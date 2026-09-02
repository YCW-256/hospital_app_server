#include "PatientRegTask.h"
#include "UserModel.h"
PatientRegTask::PatientRegTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{
}

void PatientRegTask::execute()
{
	PATIENT_RESIGN_REQ req;
	memcpy(&req,m_data,sizeof(req));
	string name = req.name;
	string card = req.card;
	string phone = req.phone;
	string pwd = req.pwd;
	UserModel model;
	for (unsigned char c : name) printf("%02X ", c);
	model.patient_reg(name,card,phone,pwd);

}
