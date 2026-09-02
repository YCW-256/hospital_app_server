#pragma once
#include "BusinessTask.h"
class PatientRegTask :
    public BusinessTask
{
public:
    PatientRegTask(void* data, int size, int fd);

    void execute();
};

