#pragma once
#include "BusinessTask.h"
class SetRecordTask :
    public BusinessTask
{
public:
    SetRecordTask(void* data, int size, int fd);
   
    void execute();
};

