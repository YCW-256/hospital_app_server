#pragma once
#include "BusinessTask.h"
class BackTask :
    public BusinessTask
{
public:
    BackTask(void* data, int size, int fd);

    void execute();
};

