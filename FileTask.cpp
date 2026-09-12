#include "FileTask.h"

#include <fstream>     // ★ 新增：ofstream / ifstream
#include <iostream>    // ★ 顺手加上：cout / cerr（你现在也在用）
#include <map>
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>     // ★ memcpy / strnlen
FileTask::FileTask(void* data, int size, int fd)
	:BusinessTask(data, size, fd)
{
}

bool FileTask::savePPM(const std::string& path,
    const std::vector<uint8_t>& data,
    int w, int h)
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) return false;
    ofs << "P6\n" << w << " " << h << "\n255\n";
    ofs.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}


void FileTask::execute()
{
    // ---------- 1. 解析本分片 ----------
    IMG_T req;
    memcpy(&req, m_data, sizeof(IMG_T));   // 注意别再用 size

    std::string file_name(req.file_name,
        strnlen(req.file_name, sizeof(req.file_name)));
    if (file_name.empty()) file_name = "unnamed";

    //MY_FILE& recv = CData::my_files[file_name];

    int is_con = 1;
    int is_recv = 0;
    pthread_mutex_lock(&CData::mtx_my_files);
    MY_FILE& recv = CData::my_files[file_name];
    CData::total++;
    // 首片记录元数据
    if (recv.total_frags == 0) {
        recv.total_frags = req.total;
        recv.width = req.width;
        recv.height = req.height;
    }

    // ---------- 2. 存下这一片的像素 ----------
    recv.frags[req.index] =
        std::vector<uint8_t>(req.img_data, req.img_data + sizeof(req.img_data));
    //recv.now_size = (int)recv.frags.size();
    recv.now_size++;
    // ---------- 3. 没齐就返回 ----------
    if (recv.now_size < recv.total_frags) is_con=0;
	is_recv = recv.now_size;
	//cout << "当前已收到 "<< "/" << CData::total << " 个分片\n";

    std::cout << "收到包 " << req.index
        << " 总共收到 " << is_recv << "/" << recv.total_frags
        << " 文件=" << file_name <<"当前已收到 " << "/" << CData::total << " 个分片 " << std::endl;
    pthread_mutex_unlock(&CData::mtx_my_files);
    
    if (!is_con)return;
    // ---------- 4. 收齐了，按 index 顺序拼接 ----------
	int patient_id = req.id;
    std::vector<uint8_t> full;
    full.reserve((size_t)recv.total_frags * sizeof(req.img_data));

    for (int i = 0; i < recv.total_frags; ++i) {
        auto it = recv.frags.find(i);
        if (it == recv.frags.end()) {
            std::cerr << "缺少分片 " << i << "，放弃\n";
            CData::my_files.erase(file_name);
            return;
        }
        full.insert(full.end(), it->second.begin(), it->second.end());
    }

    // 裁掉多余填充
    const size_t expect = (size_t)recv.width * recv.height * recv.channels;
    if (full.size() > expect) full.resize(expect);

    // ---------- 5. 落盘 ----------
    std::string outPath = "/MyImgs/" + file_name + ".ppm";
    UserModel m;
    m.set_img_record(outPath, patient_id);
    if (savePPM(outPath, full, recv.width, recv.height)) {
        std::cout << "✔ 图片已保存: " << outPath
            << " 大小=" << full.size() << std::endl;
    }
    else {
        std::cerr << "✘ 保存失败\n";
    }

    // ---------- 6. 清理，防止内存泄漏 ----------
    CData::my_files.erase(file_name);
}
