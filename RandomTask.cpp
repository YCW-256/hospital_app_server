#include "RandomTask.h"

RandomTask::RandomTask(void* data, int size, int fd) :BusinessTask(data, size, fd)
{

}

RandomTask::~RandomTask()
{

	cout << "~RondomTask" << endl;
}

void RandomTask::execute()
{	
	WARING_REQ req;
	memcpy(&req, this->m_data, sizeof(req));
	int id = req.id;
	int wet = std::rand() % 100;
	// [1,100]
	int temperature = -20+std::rand() % 70 ;
	char send_buffer[1024];
	HEAD head;
	head.len = sizeof(ENVIR_RESP);
	head.type = SERVICE_TYPE::ENVIRONMENT;
	ENVIR_RESP resp;
	resp.id = id;
	resp.wet = wet;
	resp.temperature = temperature;
	for (int i = 0; i < 12; i++) {
		resp.isClass[i] = req.isClass[i];
	}

	cout << "设备" << id << "有数据";
	cout << resp.wet << "     " << resp.temperature << endl;


	pthread_mutex_lock(&CData::mtx_envir);
	CData::now_envir[id] = resp;
	pthread_mutex_unlock(&CData::mtx_envir);
	
	/*for (int i = 0; i < 12; i++) {
		cout << "  " << (CData::now_envir[req.id].isClass[i]);
	}cout << endl;
	for (int i = 0; i < 12; i++) {
		cout << "  " << resp.isClass[i];
	}cout << endl;*/
	
	/*memcpy(send_buffer, &head, sizeof(head));
	memcpy(send_buffer+sizeof(HEAD), &resp,sizeof(resp));
	int len = send(m_fd, send_buffer, sizeof(HEAD) + sizeof(ENVIR_RESP), 0);
	printf("发送%d字节到客户端\n", len);*/

}
