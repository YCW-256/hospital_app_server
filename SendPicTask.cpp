#include "SendPicTask.h"
#include <stddef.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utility>

namespace {

/* 收齐后的图片落盘目录（相对服务端进程的工作目录）。
 * 服务端在虚拟机里跑，进程工作目录不一定是源码目录，实际落地路径以写盘日志里打印的绝对路径为准。 */
const char* kPicDir = "./received_pics";

/* 同时处于“未收齐”状态的图片数量上限。
 * 每张未完成的图常驻一个 PIC_T（约 1MB），此上限约束住服务端内存占用。 */
const int kMaxPendingPics = 32;

/* 文件名净化：只放行字母/数字/下划线/短横/点。
 * 文件名来自网络，而收齐后要拿它拼路径写盘，必须挡掉 '/'、'\' 与 ".." 防止越权写文件。 */
bool isSafeFileName(const string& name)
{
	if (name.empty() || name.size() > PIC_FILE_NAME_LEN - 1)
		return false;
	if (name == "." || name == "..")
		return false;
	for (size_t i = 0; i < name.size(); ++i) {
		char c = name[i];
		bool ok = (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
			|| c == '_' || c == '-' || c == '.';
		if (!ok)
			return false;
	}
	return true;
}

/* 确保落盘目录存在（已存在不算失败） */
bool ensurePicDir(const string& dir)
{
	if (mkdir(dir.c_str(), 0755) == 0)
		return true;
	return errno == EEXIST;
}

/* 收齐整图后写盘：逐包按“本包有效长度”写出，末包的补零不计入，
 * 因此还原出的文件与客户端原始图片逐字节一致。filen 已过 isSafeFileName 净化。 */
bool savePicture(const string& file_name, const PIC_T& picture, int total, int file_size)
{
	if (!ensurePicDir(kPicDir)) {
		perror("创建图片目录失败");
		return false;
	}

	const string path = string(kPicDir) + "/" + file_name;
	FILE* fp = fopen(path.c_str(), "wb");
	if (fp == nullptr) {
		perror("打开图片文件失败");
		return false;
	}

	bool ok = true;
	int written = 0;
	for (int i = 0; i < total; ++i) {
		int chunk = PIC_CHUNK_SIZE;
		if (i == total - 1)
			chunk = file_size - i * PIC_CHUNK_SIZE;  // 末包只写真实字节数，丢掉补零
		if (chunk <= 0 || chunk > PIC_CHUNK_SIZE)
			break;
		if (fwrite(picture.pic[i], 1, static_cast<size_t>(chunk), fp) != static_cast<size_t>(chunk)) {
			ok = false;
			break;
		}
		written += chunk;
	}
	fclose(fp);

	char cwd[1024] = { 0 };
	if (getcwd(cwd, sizeof(cwd)) == nullptr)
		cwd[0] = '\0';
	cout << (ok ? "图片已保存：" : "图片写盘失败：") << cwd << "/" << path
		<< "（" << written << "/" << file_size << " 字节）" << endl;
	return ok;
}

} // namespace

SendPicTask::SendPicTask(void* data, int size, int fd)
	: BusinessTask(data, size, fd)
{
}

void SendPicTask::execute()
{
	// 整包体必须完整到达，否则字段可能只拷贝了一半
	if (m_data == nullptr || size < static_cast<int>(sizeof(PIC_SIGLE_PACK)))
		return;

	PIC_SIGLE_PACK packet;
	memset(&packet, 0, sizeof(packet));
	memcpy(&packet, m_data, sizeof(packet));

	// ---- 包内字段校验：任一不合法即整包丢弃，绝不写入聚合缓冲 ----
	if (packet.index < 0 || packet.index >= PIC_MAX_CHUNKS)
		return;
	if (packet.total <= 0 || packet.total > PIC_MAX_CHUNKS)
		return;
	if (packet.index >= packet.total)
		return;
	if (packet.data_len <= 0 || packet.data_len > PIC_CHUNK_SIZE)
		return;
	// 非末包必须是满包：否则按 index*PIC_CHUNK_SIZE 归位后中间会留下空洞，拼出的图会错位
	if (packet.index < packet.total - 1 && packet.data_len != PIC_CHUNK_SIZE)
		return;

	size_t name_len = 0;
	while (name_len < sizeof(packet.file_name) && packet.file_name[name_len] != '\0')
		++name_len;
	const string file_name(packet.file_name, name_len);
	if (!isSafeFileName(file_name)) {
		cout << "非法的图片文件名，已丢弃该分包" << endl;
		return;
	}

	const size_t body_offset = offsetof(PIC_SIGLE_PACK, width);

	pthread_mutex_lock(&CData::mtx_pic);

	// 未收齐的图每张常驻约 1MB，这里限制同时待收的图片数量
	unordered_map<string, PIC_T>::iterator it = CData::pics.find(file_name);
	if (it == CData::pics.end()) {
		if (static_cast<int>(CData::pics.size()) >= kMaxPendingPics) {
			pthread_mutex_unlock(&CData::mtx_pic);
			cout << "待收图片过多，丢弃本次分包：" << file_name << endl;
			return;
		}
		it = CData::pics.insert(make_pair(file_name, PIC_T())).first;
	}

	PIC_T& picture = it->second;

	if (picture.total == 0) {
		picture.total = packet.total;      // 首包决定本图总包数
	}
	else if (picture.total != packet.total) {
		pthread_mutex_unlock(&CData::mtx_pic);
		cout << "总包数与首包不一致，丢弃本包：" << file_name << endl;
		return;
	}

	// 按 index 去重：重复包直接忽略，避免计数虚高导致“提前完成”或“永远完不成”
	if (picture.received[packet.index]) {
		pthread_mutex_unlock(&CData::mtx_pic);
		return;
	}

	memcpy(picture.pic[packet.index],
		static_cast<const char*>(m_data) + body_offset,
		static_cast<size_t>(packet.data_len));
	picture.received[packet.index] = true;
	++picture.received_count;

	if (packet.index == packet.total - 1)
		picture.file_size = packet.index * PIC_CHUNK_SIZE + packet.data_len;

	cout << "收到第" << picture.received_count << "/" << picture.total << "包（" << file_name << "）" << endl;

	if (picture.received_count != picture.total) {
		pthread_mutex_unlock(&CData::mtx_pic);
		return;
	}

	// ---- 收齐：先把聚合体摘出来再解锁落盘，避免持锁做磁盘 I/O 阻塞其它收包线程 ----
	PIC_T* finished = new PIC_T;
	memcpy(finished->pic, picture.pic, static_cast<size_t>(picture.file_size));
	const int finished_total = picture.total;
	const int finished_size = picture.file_size;
	CData::pics.erase(it);   // 落盘后即刻释放约 1MB，避免内存只增不减

	pthread_mutex_unlock(&CData::mtx_pic);

	cout << "图片接收完成：" << file_name << " 共 " << finished_size << " 字节" << endl;
	savePicture(file_name, *finished, finished_total, finished_size);
	delete finished;
}
