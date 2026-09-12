#include "GetTongueImgTask.h"
#include <algorithm>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
	bool read_ppm(const string& path, vector<unsigned char>& pixels, int& width, int& height)
	{
		ifstream input(path, ios::binary);
		if (!input)
			return false;

		string magic;
		int max_value = 0;
		input >> magic >> width >> height >> max_value;
		if (!input || magic != "P6" || width <= 0 || height <= 0 || max_value != 255)
			return false;

		input.get();
		pixels.assign(istreambuf_iterator<char>(input), istreambuf_iterator<char>());
		size_t expected_size = static_cast<size_t>(width) * static_cast<size_t>(height) * 3;
		if (pixels.size() < expected_size)
			return false;
		if (pixels.size() > expected_size)
			pixels.resize(expected_size);
		return true;
	}

	string file_name_from_path(const string& path)
	{
		size_t separator = path.find_last_of("/\\");
		return separator == string::npos ? path : path.substr(separator + 1);
	}
}

GetTongueImgTask::GetTongueImgTask(void* data, int size, int fd)
	: BusinessTask(data, size, fd)
{
}

void GetTongueImgTask::execute()
{
	if (m_data == nullptr || size < static_cast<int>(sizeof(GET_TONGUE_IMG_REQ)))
		return;

	GET_TONGUE_IMG_REQ req;
	memcpy(&req, m_data, sizeof(req));
	string date(req.date, strnlen(req.date, sizeof(req.date)));
	if (req.patient_id <= 0 || date.empty())
		return;

	UserModel model;
	string path;
	if (!model.get_img_path(req.patient_id, date, path)) {
		cout << "没有找到舌诊图片" << endl;
		return;
	}

	vector<unsigned char> pixels;
	int width = 0;
	int height = 0;
	if (!read_ppm(path, pixels, width, height)) {
		cout << "舌诊图片读取失败: " << path << endl;
		return;
	}

	const size_t payload_size = sizeof(((IMG_T*)nullptr)->img_data);
	int total = static_cast<int>((pixels.size() + payload_size - 1) / payload_size);
	string file_name = file_name_from_path(path);

	for (int index = 0; index < total; ++index) {
		IMG_T packet;
		memset(&packet, 0, sizeof(packet));
		packet.index = index;
		packet.total = total;
		packet.width = width;
		packet.height = height;
		packet.id = req.patient_id;
		strncpy(packet.file_name, file_name.c_str(), sizeof(packet.file_name) - 1);

		size_t offset = static_cast<size_t>(index) * payload_size;
		size_t copy_size = min(payload_size, pixels.size() - offset);
		memcpy(packet.img_data, pixels.data() + offset, copy_size);

		HEAD head;
		memset(&head, 0, sizeof(head));
		head.type = SERVICE_TYPE::GET_TONGUE_IMG;
		head.is_fragment = total > 1;
		head.frag_index = index;
		head.frag_total = total;
		head.len = sizeof(packet);

		char send_data[sizeof(HEAD) + sizeof(IMG_T)];
		memcpy(send_data, &head, sizeof(head));
		memcpy(send_data + sizeof(head), &packet, sizeof(packet));
		int sent = send(m_fd, send_data, sizeof(send_data), 0);
		if (sent < 0) {
			cout << "发送舌诊图片失败! errno:" << errno << " " << strerror(errno) << endl;
			return;
		}
	}

	cout << "舌诊图片发送完成，共 " << total << " 包" << endl;
}
