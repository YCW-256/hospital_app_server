#include "SetRecordTask.h"

SetRecordTask::SetRecordTask(void* data, int size, int fd)
	:BusinessTask(data, size, fd)
{
}

void SetRecordTask::execute()
{
	SET_RECORD_REQ req;
	memcpy(&req, m_data, sizeof(req));
	int doctor_id = req.doctor_id;
	int patient_id = req.patient_id;
	char diagnosis[200];
	strcpy(diagnosis, req.diagnosis);
	char treat_plan[200];
	strcpy(treat_plan, req.treat_plan);
	UserModel u;
	if (u.set_record(doctor_id, patient_id, diagnosis, treat_plan)) {
		cout << "病历设置成功" << endl;
	};


}
