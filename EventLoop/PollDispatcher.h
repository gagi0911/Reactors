#pragma once
#include "Dispatcher.h"
#include <string>
#include <poll.h>

class PollDispatcher : public Dispatcher
{
public:
	//init data fit for the io method
	PollDispatcher(EventLoop* evloop);
	~PollDispatcher();
	//add
	int add() override;
	//remove
	int remove() override;
	//modify
	int modify() override;
	// event check
	int dispatch(int timeout = 2) override; // unit: s

private:

	int m_maxfd;
	const int m_maxNode = 1024;
	struct pollfd* m_fds;

};