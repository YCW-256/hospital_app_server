#pragma once
#include "BusinessTask.h"
class SetAppTask :
    public BusinessTask
{
public:
    SetAppTask(void* data, int size, int fd);

    void execute();
};

