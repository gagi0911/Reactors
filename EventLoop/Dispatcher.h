#pragma once
#include "Channel.h"
#include "EventLoop.h"

#include<string>

using namespace std;

class EventLoop;

class Dispatcher
{
public:
	//init data fit for the io method
	Dispatcher(EventLoop* evloop);
	virtual ~Dispatcher();
	//add
	virtual int add();
	//remove
	virtual int remove();
	//modify
	virtual int modify();
	// event check
	virtual int dispatch(int timeout = 2); // unit: s
	inline void setChannel(Channel* channel)
	{
		m_channel = channel;
	}
protected:
	string m_name = string();
	Channel* m_channel;
	EventLoop* m_evLoop;
};