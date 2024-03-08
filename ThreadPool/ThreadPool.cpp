#include "ThreadPool.h"
#include <stdlib.h>
#include <assert.h>
#include "Log.h"

ThreadPool::ThreadPool(EventLoop* evloop, int count)
{
	m_mainLoop = evloop;
	m_index = 0;
	m_isStart = false;
	m_threadNum = count;
	m_workThreads.clear();
}

ThreadPool::~ThreadPool()
{
	for (auto it : m_workThreads)
	{
		delete it;
	}
}

void ThreadPool::Run()
{
	assert(!m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id())
	{
		exit(0);
	}
	m_isStart = true;
	if (m_threadNum > 0)
	{
		for (int i = 0; i < m_threadNum; i++)
		{
			WorkThread* subThread = new WorkThread(i);
			subThread->Run();
			m_workThreads.push_back(subThread);
		}
	}
}

EventLoop* ThreadPool::takeWorkEventLoop()
{
	assert(m_isStart);
	if (m_mainLoop->getThreadID() != this_thread::get_id())
	{
		exit(0);
	}
	EventLoop* evloop = m_mainLoop;
	if (m_threadNum > 0)
	{
		evloop = m_workThreads[m_index]->getEventLoop();
		m_index = ++m_index % m_threadNum;
	}
	return evloop;
}
