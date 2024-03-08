#pragma once
#include "EventLoop/EventLoop.h"
#include "ThreadPool/ThreadPool.h"


class TcpServer
{
public:
	TcpServer(unsigned short port, int threadNum);
	~TcpServer();

	bool SetListener();

	void Run();
	static int acceptConnection(void* arg);
private:

	int m_threadNum;
	EventLoop* m_mainloop;
	ThreadPool* m_pool;
	
	int m_lfd;
	unsigned short m_port;
};
