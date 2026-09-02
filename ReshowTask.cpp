#include "ReshowTask.h"

ReshowTask::ReshowTask(void* data, int size, int fd):BusinessTask(data,size,fd)
{

}

ReshowTask::~ReshowTask()
{
	cout << "~ReshowTask" << endl;
}

void ReshowTask::execute()
{
	char send_buf[1024] = { 0 };
	ENVIR_REQ envir_req;
	memcpy(&envir_req, this->m_data, sizeof(ENVIR_REQ));
	HEAD head;
	head.len = sizeof(ENVIR_RESP);
	head.type = SERVICE_TYPE::ENVIRONMENT;
	memcpy(send_buf, &head, sizeof(HEAD));
	ENVIR_RESP envir_resp;
	envir_resp.temperature = envir_req.temperature;
	envir_resp.wet = envir_req.wet;
	memcpy(send_buf + sizeof(HEAD), &envir_resp, sizeof(ENVIR_RESP));
	
	DetectorModel dect;
	dect.db_save(1001,envir_resp.wet, envir_resp.temperature);

	int len = send(this->m_fd,send_buf,sizeof(ENVIR_RESP)+sizeof(HEAD),0);
	printf("【服务端】向  客户端%d  回显%d个字节", this->m_fd, len);



	
}
