#include "ThreadPool.h"



ThreadPool::ThreadPool(int threadCount)
{
	this->m_threadCount = threadCount;
	pthread_mutex_init(&m_mutex, nullptr);
	pthread_cond_init(&m_cond, nullptr);
	//创建初始线程
	m_workers.resize(threadCount);
	for (int i = 0; i < this->m_threadCount; i++) {
		int ret = pthread_create(&m_workers[i], nullptr, workerThread, this);
		if (ret != 0)
			perror("pthread create error");
	}

}

void ThreadPool::submit(BusinessTask* task)
{
	if (task == nullptr) {
		return;
	}
	//将任务存入任务队列，唤醒一个等待中的线程去处理任务
	pthread_mutex_lock(&m_mutex);
	m_tasks.push(task);
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
}

void* ThreadPool::workerThread(void* arg)
{
	//在这个方法，它就相当于this指针
	ThreadPool* pool = static_cast<ThreadPool*>(arg);
	while (true) {
		BusinessTask* task = nullptr;
		pthread_mutex_lock(&pool->m_mutex);
		while (pool->m_tasks.empty()) {
			pthread_cond_wait(&pool->m_cond, &pool->m_mutex);
		}
		//取出任务
		task = pool->m_tasks.front();
		pool->m_tasks.pop();
		pthread_mutex_unlock(&pool->m_mutex);
		if (task != nullptr) {
			task->execute();
			delete task;
		}
	}
	return nullptr;
}
