#pragma once
#include "BusinessTask.h"
class DoctorInfoTask :
    public BusinessTask
{
public:
    DoctorInfoTask(void* data, int size, int fd);

    void execute();

};

