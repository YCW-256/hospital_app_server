#include "TaskController.h"
#include "DoctorLoginTask.h"
#include "GetAppTask.h"
#include "DoctorInfoTask.h"
#include "RepixGuardTask.h"
#include "SelectGuardTask.h"
#include "PatientRegTask.h"
#include "PatientLoginTask.h"
#include "PatientGetDoctorTask.h"
#include "SetAppTask.h"
ThreadPool TaskController::thread_pool(5);

BusinessTask* TaskController::create_task(SERVICE_TYPE type, void* data, int size, int fd)
{   
	
	cout << "创建任务" << endl;
	BusinessTask* task = nullptr;
	
	if (type == SERVICE_TYPE::DOCTOR_LOGIN) {
		task = new DoctorLoginTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::DOCTOR_APP_INFO) {
		cout <<"约会信息" << endl;
		task = new GetAppTask(data,size,fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::SELECT_DOCTOR) {
		cout << "查询科室医生"<<endl;
		task = new DoctorInfoTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::REPIX_GUARD) {
		cout << "识别到了"<<endl;
		task = new RepixGuardTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::GET_GUARD) {
		cout << "获得值班信息" << endl;
		task = new SelectGuardTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::PATIENT_RESIGN) {
		cout << "注册患者" << endl;
		task = new PatientRegTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::PATIENT_LOGIN) {
		cout << "患者登录" << endl;
		task = new PatientLoginTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::PATIENT_GET_DOCTOR_INFO) {
		cout << "获得医生信息" << endl;
		task = new PatientGetDoctorTask(data, size, fd);
		thread_pool.submit(task);
	}
	else if (type == SERVICE_TYPE::PATIENT_APPOINTMENT) {
		cout << "挂号" << endl;
		task = new SetAppTask(data, size, fd);
		thread_pool.submit(task);
	}
	



	//if (type == SERVICE_TYPE::WARNING) {

	//	task = new WarningTask(data,size,fd);
	//	thread_pool.submit(task);
	//	BusinessTask* task2 = nullptr;
	//	char *data2=new char[1024];
	//	memcpy(data2, data, size);
	//	task2 = new RandomTask(data2, size, fd);
	//	thread_pool.submit(task2);

	//}
	//else if (type == SERVICE_TYPE::GETENVIR) {
	//	
	//	task = new BackTask(data, size, fd);
	//	thread_pool.submit(task);

	//}
	//if (type == SERVICE_TYPE::LOGIN) {
	//	task =new LoginTask(data, size, fd);
	//	thread_pool.submit(task);
	//	
	//	//task.execute();
	//	

	//}
	//else if (type == SERVICE_TYPE::CHAT) {
	//
	//	task=new ChatTask(data, size, fd);
	//	thread_pool.submit(task);
	///*	BusinessTask* tasks[100];
	//	for (int i = 0; i < 100; i++) 
	//		tasks[i] = new ChatTask(data, size, fd);
	//	for (int i = 0; i < 100; i++) {
	//		thread_pool.submit(tasks[i]);
	//	}*/
	//}
	//else if (type == SERVICE_TYPE::GETENVIR) {
	//	task = new RandomTask(data, size, fd);
	//	thread_pool.submit(task);
	//}
	

    return task;
}
