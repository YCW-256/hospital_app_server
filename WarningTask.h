#pragma once
#include "BusinessTask.h"
class WarningTask :
    public BusinessTask
{
public:
    WarningTask(void* data, int size, int fd);
    void execute();

};

