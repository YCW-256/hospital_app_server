#pragma once
#include "BusinessTask.h"
class PatientGetDoctorTask :
    public BusinessTask
{
public:
    PatientGetDoctorTask(void* data, int size, int fd);

    void execute();
};

