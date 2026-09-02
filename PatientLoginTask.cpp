#include "PatientLoginTask.h"
#include "UserModel.h"
PatientLoginTask::PatientLoginTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{

}

void PatientLoginTask::execute()
{
	PATIENT_LOGIN_REQ req;
	memcpy(&req, m_data, sizeof(req));
	int type = 1;
	string account = req.account;
	string pwd = req.pwd;
	int id = 0;
	string name = "";
	UserModel model;
	bool flag=model.patient_login(account,pwd,id,name,type);

	PATIENT_LOGIN_RESP resp;
	HEAD head;
	head.len = sizeof(resp);
	head.type = SERVICE_TYPE::PATIENT_LOGIN;
	if (flag) {
		resp.result = flag;
		resp.id = id;
		strcpy(resp.name, name.c_str());
	}
	else {
		resp.result = flag;
	}
	char buf[204] = "";
	memcpy(buf,&head,sizeof(head));
	memcpy(buf+sizeof(head), &resp, sizeof(resp));
	int len = send(m_fd, buf, sizeof(resp) + sizeof(head), 0);
	if (len < 0)
	{
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}
	


}
