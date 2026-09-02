#include "LoginTask.h"
#include "UserModel.h"
LoginTask::LoginTask(void* data, int size, int fd):BusinessTask(data,size,fd)
{

}

LoginTask::~LoginTask()
{
	cout << "~LoginTask" << endl;
}

void LoginTask::execute()
{
	char send_buffer[1024];
	LOGIN_REQ login_req = { 0 };
	memcpy(&login_req, this->m_data, sizeof(LOGIN_REQ));
	cout << "【服务器】接收到 nike=" << login_req.nike
		<< ", passward=" << login_req.password << endl;
	//数据库中查询用户名，密码




	LOGIN_RESP login_resp = { 0 };
	UserModel login_task;
	login_resp.result = login_task.db_login(login_req.nike, login_req.password);
	if (login_resp.result) {
		CData::online_users[login_req.nike] = this->m_fd;
	}
	else {
		cout << "用户名密码错误" << endl;
	}
	HEAD head;
	head.len = sizeof(LOGIN_RESP);
	head.type = SERVICE_TYPE::LOGIN;
	memcpy(send_buffer, &head, sizeof(head));
	memcpy(send_buffer + sizeof(HEAD), &login_resp, sizeof(login_resp));
	int len = send(m_fd, send_buffer, sizeof(HEAD)+sizeof(LOGIN_RESP), 0);
	cout << "【服务端】发送：" << len << "字节" << endl;


}
