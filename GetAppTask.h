#pragma once
#include "BusinessTask.h"
class GetAppTask :
    public BusinessTask
{
public:
    GetAppTask(void* data, int size, int fd);

    void execute();
};

