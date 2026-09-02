#pragma once
#include "BusinessTask.h"


#include "ThreadPool.h"
class TaskController
{
public:
	static BusinessTask* create_task(SERVICE_TYPE,void *data,int size,int fd);

	static ThreadPool thread_pool;

};

