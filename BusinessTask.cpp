#include "BusinessTask.h"

BusinessTask::BusinessTask(void* data, int size, int fd)
{
	if (data != nullptr && size > 0) {
		this->m_data = new char[size];
		memset(this->m_data, 0, size);
		memcpy(this->m_data, data, size);
		this->m_fd = fd;
		this->size = size;
	}
}

BusinessTask::~BusinessTask()
{
	if (this->m_data != nullptr) {
		delete []this->m_data;
	}
	cout <<"~BusinessTask()" << endl;
}


