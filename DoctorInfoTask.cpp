#include "DoctorInfoTask.h"
#include "UserModel.h"
DoctorInfoTask::DoctorInfoTask(void* data, int size, int fd)
	:BusinessTask(data, size, fd)
{
}

void DoctorInfoTask::execute()
{
	vector<DOCCTOR_INFO>res;
	DOCTOR_INFO_REQ req;
	memcpy(&req,this->m_data ,sizeof(req));
	UserModel model;
	model.get_depart_info(req.department, res);
	
	char send_buf[2048];
	DOCCTOR_INFO* doc;
	HEAD head;
	head.type = SERVICE_TYPE::SELECT_DOCTOR;
	head.len = 0;
	head.frag_total = res.size();
	int cur_size = sizeof(head);
	
	for (int i = 0;i < res.size();i++) {
		DOCCTOR_INFO* doc = &res[i];
		memcpy(send_buf + cur_size, (char*)doc, sizeof(DOCCTOR_INFO));
		cur_size += sizeof(DOCCTOR_INFO);
	}
	head.len = cur_size - sizeof(head);
	head.type = SERVICE_TYPE::SELECT_DOCTOR;
	memcpy(send_buf,&head,sizeof(HEAD));
	int len = send(m_fd, send_buf, cur_size, 0);
	if (len < 0){
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}


	

}
