#pragma once
#include "BusinessTask.h"
class DoctorLoginTask :
    public BusinessTask
{
public:
    DoctorLoginTask(void* data, int size, int fd);
       
    void execute();
};

