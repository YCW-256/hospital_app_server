#pragma once
#include "BusinessTask.h"

class SendPicTask : public BusinessTask
{
public:
	SendPicTask(void* data, int size, int fd);

	void execute();
};
