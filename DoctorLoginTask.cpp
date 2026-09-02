#include "DoctorLoginTask.h"
#include "UserModel.h"
DoctorLoginTask::DoctorLoginTask(void* data, int size, int fd):BusinessTask::BusinessTask(data, size, fd)
{

}

void DoctorLoginTask::execute()
{

	DOCTOR_LOGIN_REQ login_req;
	memcpy(&login_req, this->m_data, sizeof(login_req));
	string account = login_req.account;
	string pwd = login_req.pwd;
	cout << "账号" << account << "密码" << pwd <<"类型"<<login_req.login_style <<endl;
	UserModel login_model;
	int doctor_id = -1;
	int result = 0;
	if (result=login_model.doctor_login(account, pwd, doctor_id)) {
		cout << "登录成功" <<"result" <<result<< endl;
	}
	DOCTOR_LOGIN_RESP resp;
	resp.id = doctor_id;
	resp.result = result;
	HEAD head;
	head.len = sizeof(resp);
	head.type = SERVICE_TYPE::DOCTOR_LOGIN;
	char buf[1024]="";
	memcpy(buf,&head,sizeof(head));
	memcpy(buf+sizeof(head), &resp, sizeof(resp));
	int len = send(m_fd, buf, sizeof(resp)+sizeof(head), 0);
	if (len < 0)
	{
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}

}
