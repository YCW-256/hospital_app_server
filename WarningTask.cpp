#include "WarningTask.h"
#include "CData.h"
WarningTask::WarningTask(void* data, int size, int fd) :BusinessTask::BusinessTask(data, size, fd)
{
}
string classes[12]={
	"Curculionidae",
	"Delphacidae",
	"Cicadellidae",
	"Phlaeothripidae",
	"Cecidomyiidae",
	"Hesperiidae",
	"Crambidae",
	"Chloropidae",
	"Ephydridae"
	"Noctuidae",
	"Thripidae"
};
void WarningTask::execute()
{
	WARING_REQ req = { 0 };
	memcpy(&req, this->m_data, sizeof(WARING_REQ));
	std::cout << "设备id" << req.id<<"虫害有";
	for (int i = 0; i < 12; i++) {
		if (CData::now_envir[req.id].isClass[i] = req.isClass[i])
			std::cout << classes[i]<<"  ";
	}
	HEAD head;
	head.len = sizeof(WARING_REQ);
	head.type = SERVICE_TYPE::WARNING;
	char send_buffer[1024] = { 0 };
	memcpy(send_buffer,&head, sizeof(HEAD));
	memcpy(send_buffer +sizeof(HEAD), &req, sizeof(WARING_REQ));
	int len = send(m_fd, send_buffer, sizeof(HEAD) + sizeof(WARING_REQ), 0);
	printf("发送%d字节到客户端\n", len);

	std::cout << endl;

}
