#include "ChatTask.h"
#include "unistd.h"
ChatTask::ChatTask(void* data, int size, int fd):BusinessTask::BusinessTask(data,size,fd)
{

}

ChatTask::~ChatTask()
{
	cout << "~ChatTask" << endl;
}

void ChatTask::execute()
{
	CHAT_REQ chat_req = { 0 };
	

	memcpy(&chat_req, this->m_data, sizeof(CHAT_REQ));
	//消息转发给除了自身之外的所有客户端
	HEAD head;
	head.len = sizeof(CHAT_REQ);
	head.type = SERVICE_TYPE::CHAT;
	char send_buf[1024] = { 0 };
	//搭建转发信息
	memcpy(send_buf, &head, sizeof(HEAD));
	memcpy(send_buf + sizeof(HEAD), &chat_req, sizeof(CHAT_REQ));
	int len;
	cout << "【服务端】线程:"<< pthread_self()<<endl;
	
	for (const auto& u : CData::online_users) {

		if (strcmp(u.first.c_str(), chat_req.send_name) == 0)continue;
		int client_fd = u.second;
		int send_len = sizeof(HEAD) + sizeof(CHAT_REQ);
		len = send(client_fd, send_buf, send_len, 0);

		if (len < 0) {
			break;
		}
		cout << "【服务端】转发：端" << client_fd << "  " << len << "字节" << endl;

	}

}
