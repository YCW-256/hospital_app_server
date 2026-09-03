#include "SetAppTask.h"
#include <chrono>
#include <ctime>
#include <sstream>
#include <iomanip>
SetAppTask::SetAppTask(void* data, int size, int fd)
	:BusinessTask(data,size,fd)
{
}

void SetAppTask::execute()
{
	PATIENT_APPOINTMENT_REQ req;
	memcpy(&req,m_data,sizeof(req));
	int patient_id = req.patient_id;
	int doctor_id = req.doctor_id;
	int ob_time = req.ob_time;
	auto now = std::chrono::system_clock::now();
	// 转换为 time_t 类型（秒）
	std::time_t now_c = std::chrono::system_clock::to_time_t(now);
	// 转换为本地时间的 tm 结构
	std::tm now_tm = *std::localtime(&now_c);
	std::stringstream ss;
	ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
	std::string date_time = ss.str();

	UserModel model;
	
	PATIENT_APPOINTMENT_RESP resp;
	resp.state=model.patient_set_app(doctor_id, patient_id, date_time, ob_time);

	HEAD head;
	head.type = SERVICE_TYPE::PATIENT_APPOINTMENT;
	head.len = sizeof(resp);

	char data[1024]="";
	memcpy(data, &head, sizeof(head));
	memcpy(data + sizeof(head), &resp, sizeof(resp));
	int len = send(m_fd, data, sizeof(resp) + sizeof(head), 0);
	if (len < 0)
	{
		cout << "send失败! errno:" << errno << " " << strerror(errno) << endl;
	}
	else
	{
		cout << "send成功，发送字节数:" << len << endl;
	}


}
