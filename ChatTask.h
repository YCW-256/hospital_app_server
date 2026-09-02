#pragma once
#include "BusinessTask.h"
class ChatTask :
    public BusinessTask
{
public:
    ChatTask(void *data,int size,int fd);

    ~ChatTask();

    void execute();
};

