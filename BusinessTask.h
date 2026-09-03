#pragma once
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include "protecol.h"
#include "CData.h"
#include <string.h>
#include <vector>
#include "UserModel.h"
using namespace std;

class BusinessTask
{
public:
	BusinessTask(void* data, int size, int fd);
	virtual~BusinessTask();
	virtual void execute() = 0;
protected:
	void* m_data;
	int m_fd;
	int size;

};

