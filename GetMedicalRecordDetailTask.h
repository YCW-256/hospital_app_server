#pragma once
#include "BusinessTask.h"

class GetMedicalRecordDetailTask : public BusinessTask
{
public:
	GetMedicalRecordDetailTask(void* data, int size, int fd);

	void execute();
};
