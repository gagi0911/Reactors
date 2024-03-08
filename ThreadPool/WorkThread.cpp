#include "WorkThread.h"
#include <stdio.h>

WorkThread::WorkThread(int index)
{
	m_evloop = nullptr;
	m_thread = nullptr;
	m_threadID = thread::id();
	m_name = "SubThread-" + to_string(index);
}

WorkThread::~WorkThread()
{
	if (m_thread != nullptr)
	{
		delete m_thread;
	}
}

int WorkThread::Run()
{
	m_thread = new thread(&WorkThread::subThreadRunning, this);
	// let subthread initialize successfully, using mutex and cond, block until eventloop is set;
	unique_lock<mutex> locker(m_mutex);
	while (m_evloop == nullptr)
	{
		m_cond.wait(locker);
	}
	return 0;
}

void WorkThread::subThreadRunning()
{
	m_mutex.lock();
	m_evloop = new EventLoop(m_name); 
	m_mutex.unlock();
	m_cond.notify_one();
	m_evloop->Run();
}
