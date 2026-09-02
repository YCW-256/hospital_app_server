#pragma once
#include "BusinessTask.h"
#include "CData.h"
class RandomTask :
    public BusinessTask
{
public:
    RandomTask(void* data, int size, int fd);

    ~RandomTask();

    void execute();
};

