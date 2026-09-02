#pragma once
#include "BusinessTask.h"

#include <vector>
#include <queue>
using namespace std;
class ThreadPool
{
public:
	ThreadPool(int threadCount);
	void submit(BusinessTask* task);//提交任务

private:
	static void* workerThread(void* arg);
	int m_threadCount;//线程池核心线程数量
	vector<pthread_t>m_workers;//工作线程
	queue<BusinessTask*>m_tasks;//任务队列
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;


};

