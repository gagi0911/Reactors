#pragma once
#include "EventLoop/EventLoop.h"
#include <thread>
#include <string>
#include <condition_variable>
#include <mutex>
using namespace std;

class WorkThread
{
public:
	WorkThread(int index);
	~WorkThread();

	inline EventLoop* getEventLoop()
	{
		return m_evloop;
	}

	int Run();
private:
	void subThreadRunning();

private:
	thread* m_thread;
	thread::id m_threadID;
	string m_name;
	mutex m_mutex;
	condition_variable m_cond;
	EventLoop* m_evloop;
};
