#include "GetAppTask.h"
#include "UserModel.h"
GetAppTask::GetAppTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{

}

void GetAppTask::execute()
{
	DOCTOR_APP_REQ req;
	memcpy(&req,m_data,sizeof(req));
	int id = req.id;
	int style = req.style;
	char name[10][15];
	char time[10][15];
	int state[10];
	UserModel u;
	//cout << "进入模型" << endl;
	int count;
	u.doctor_get_meet(id,style,name,time,state,count);
	
	//resp返回
	DOCTOR_APP_RESP resp;
	resp.count = count;
	memcpy(resp.PatientName, name, sizeof(name));
	memcpy(resp.time, time, sizeof(time));
	memcpy(resp.state, state, sizeof(state));
	HEAD head;
	head.len = sizeof(resp);
	head.is_fragment = false;
	head.type = SERVICE_TYPE::DOCTOR_APP_INFO;

	char send_data[1024]="";
	memcpy(send_data, &head, sizeof(head));
	memcpy(send_data + sizeof(head), &resp, sizeof(resp));
	int len = send(m_fd, send_data, sizeof(resp) + sizeof(head), 0);
	if (len < 0)
	{
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}




}
