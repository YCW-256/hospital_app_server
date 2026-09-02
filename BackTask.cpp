#include "BackTask.h"

BackTask::BackTask(void* data, int size, int fd):BusinessTask(data,size,fd)
{
}

void BackTask::execute()
{
	cout << "开始执行任务" << endl;
	GETENVIR_REQ req;
	memcpy(&req, this->m_data, sizeof(GETENVIR_REQ));
	int id = req.id;
	cout << "任务2 id" << id<< endl;
	HEAD head;
	head.type = SERVICE_TYPE::ENVIRONMENT;
	head.len = sizeof(ENVIR_RESP);
	
	pthread_mutex_lock(&CData::mtx_envir);
	ENVIR_RESP resp=CData::now_envir[id];
	pthread_mutex_unlock(&CData::mtx_envir);

	cout << "发送温度" << resp.temperature << "湿度" << resp.wet;
	for (int i = 0; i < 12; i++) {
		cout << "  " << resp.isClass[i];
	}
	cout << endl;
	char send_buffer[1024] = {0};
	memcpy(send_buffer,&head,sizeof(HEAD));
	memcpy(send_buffer+sizeof(HEAD),&resp, sizeof(ENVIR_RESP));
	cout << "任务3" << endl;
	int len = send(m_fd, send_buffer, sizeof(HEAD) + sizeof(ENVIR_RESP), 0);
	printf("发送%d字节到客户端\n", len); 
	

}
