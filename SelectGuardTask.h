#pragma once
#include "BusinessTask.h"
class SelectGuardTask :
    public BusinessTask
{
public:
    SelectGuardTask(void* data, int size, int fd);

    void execute();
};

