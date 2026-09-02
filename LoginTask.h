#pragma once
#include "BusinessTask.h"
class LoginTask :
    public BusinessTask
{
public:
    LoginTask(void* data, int size, int fd);

    ~LoginTask();

    void execute() ;
};

