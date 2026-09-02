#include "SelectGuardTask.h"
#include "UserModel.h"
SelectGuardTask::SelectGuardTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{
}

void SelectGuardTask::execute()
{
	GET_GUARD_REQ req;
	memcpy(&req, this->m_data, sizeof(req));
	string department = req.department;
	string time = req.start_day;
	cout << "查询这个部门" << department<<"日期"<<req.start_day;

	UserModel model;
	GET_GUARD_RESP resp;
	model.get_guard_info(department, resp,time);
	cout << "查询成功，共 " << 3 * 7 << " 个时段：" << endl;
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 7; ++j) {
			const auto& g = resp.guards[i][j];
			cout << "  时段[" << i << "][" << j << "] "
				<< "日期:" << g.date
				<< " 姓名:" << (g.name[0] ? g.name : "(空闲)")
				<< " 空闲:" << (g.isfree ? "是" : "否")
				<< endl;
		}
	}

	char data[2048]="";
	HEAD head;
	head.type = SERVICE_TYPE::GET_GUARD;
	head.len = sizeof(resp);
	memcpy(data, &head, sizeof(head));
	memcpy(data + sizeof(head), &resp, sizeof(resp));
	HEAD head_check;
	memcpy(&head_check, data, sizeof(head));
	// 检查 head 字段是否一致
	if (head_check.type != head.type || head_check.len != head.len) {
		cout << "自检：HEAD 不一致！" << endl;
	}

	// 3. 解析 Body 到临时变量
	GET_GUARD_RESP resp_check;
	memcpy(&resp_check, data + sizeof(head), sizeof(resp));

	// 4. 用 memcmp 比较原始 resp 和解析出的 resp_check
	if (memcmp(&resp, &resp_check, sizeof(resp)) == 0) {
		cout << "自检通过：打包-解包数据完全一致" << endl;
	}
	else {
		cout << "自检失败：数据不一致，可能结构体对齐有问题" << endl;
		// 打印前几个字节的十六进制对比
		cout << "原始 resp 前 64 字节: ";
		for (int i = 0; i < 64; ++i) printf("%02x ", ((unsigned char*)&resp)[i]);
		cout << endl;
		cout << "解包 resp_check 前 64 字节: ";
		for (int i = 0; i < 64; ++i) printf("%02x ", ((unsigned char*)&resp_check)[i]);
		cout << endl;
	}

	int len = send(m_fd, data, sizeof(head)+sizeof(resp), 0);
	if (len < 0) {
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}

}
