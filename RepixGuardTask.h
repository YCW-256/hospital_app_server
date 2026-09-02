#pragma once
#include "BusinessTask.h"
class RepixGuardTask :
    public BusinessTask
{
private:
    int num;
public:
    RepixGuardTask(void* data, int size, int fd);
    RepixGuardTask(void* data, int size, int fd,int num);
    void execute();
};

