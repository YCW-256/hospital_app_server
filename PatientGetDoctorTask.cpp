#include "PatientGetDoctorTask.h"
#include "UserModel.h"
PatientGetDoctorTask::PatientGetDoctorTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{
}

void PatientGetDoctorTask::execute()
{
	UserModel model;
	vector<patient_doctor_infoo> doctors;
	model.patient_get_doctor(doctors);
	HEAD head;
	int pre = sizeof(head);
	char data[4048]="";
	for (int i = 0;i<doctors.size();i++) {
		memcpy(data + pre,&doctors[i], sizeof(patient_doctor_infoo));
		pre += sizeof(patient_doctor_infoo);
	}
	head.len = pre-sizeof(head);
	head.type = SERVICE_TYPE::PATIENT_GET_DOCTOR_INFO;
	memcpy(data, &head, sizeof(head));
	int len = send(m_fd, data, pre + sizeof(head), 0);
	if (len < 0)
	{
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}

}
