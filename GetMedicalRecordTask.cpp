#include "GetMedicalRecordTask.h"

GetMedicalRecordTask::GetMedicalRecordTask(void* data, int size, int fd)
	: BusinessTask(data, size, fd)
{
}

void GetMedicalRecordTask::execute()
{
	GET_MEDICAL_RECORD_RESP resp;
	memset(&resp, 0, sizeof(resp));

	if (m_data == nullptr || size < static_cast<int>(sizeof(MEDICAL_RECORD_REQ)))
		return;

	MEDICAL_RECORD_REQ req;
	memcpy(&req, m_data, sizeof(req));

	UserModel model;
	model.get_medical_records(req, resp);

	HEAD head;
	memset(&head, 0, sizeof(head));
	head.len = sizeof(resp);
	head.type = SERVICE_TYPE::GET_MEDICAL_RECORD;

	char send_data[sizeof(HEAD) + sizeof(GET_MEDICAL_RECORD_RESP)];
	memcpy(send_data, &head, sizeof(head));
	memcpy(send_data + sizeof(head), &resp, sizeof(resp));
	int len = send(m_fd, send_data, sizeof(send_data), 0);
	if (len < 0)
		cout << "发送病历列表失败! errno:" << errno << " " << strerror(errno) << endl;
}
