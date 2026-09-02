#pragma once
#include "BusinessTask.h"
class PatientLoginTask :
    public BusinessTask
{
public:
    PatientLoginTask(void* data, int size, int fd);

    void execute();
};

