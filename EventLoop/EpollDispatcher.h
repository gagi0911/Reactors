#pragma once
#include "Dispatcher.h"
#include <string>
#include <sys/epoll.h>

class EpollDispatcher : public Dispatcher
{
public:
	//init data fit for the io method
	EpollDispatcher(EventLoop* evloop);
	~EpollDispatcher();
	//add
	int add() override;
	//remove
	int remove() override;
	//modify
	int modify() override;
	// event check
	int dispatch(int timeout = 2) override; // unit: s

private:
	int epollCTL(int op);

	int m_epfd;
	struct epoll_event* m_events;
	const int m_maxNode = 520;

};