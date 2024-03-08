#pragma once
#include "WorkThread.h"
#include "EventLoop/EventLoop.h"
#include <vector>

class ThreadPool
{
public:
	ThreadPool(EventLoop* evloop, int count);
	~ThreadPool();

	void Run();
	EventLoop* takeWorkEventLoop();
private:
	EventLoop* m_mainLoop;
	bool m_isStart;
	int m_threadNum;
	vector<WorkThread*> m_workThreads;
	int m_index;
};

