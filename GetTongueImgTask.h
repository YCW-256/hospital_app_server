#pragma once
#include "BusinessTask.h"

class GetTongueImgTask : public BusinessTask
{
public:
	GetTongueImgTask(void* data, int size, int fd);

	void execute();
};
