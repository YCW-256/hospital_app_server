#pragma once
#include "BusinessTask.h"
#include "DetectorModel.h"
class ReshowTask :
    public BusinessTask
{
public:
    ReshowTask(void* data, int size, int fd);

    ~ReshowTask();

    void execute();

};

