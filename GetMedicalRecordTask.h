#pragma once
#include "BusinessTask.h"

class GetMedicalRecordTask : public BusinessTask
{
public:
	GetMedicalRecordTask(void* data, int size, int fd);

	void execute();
};
