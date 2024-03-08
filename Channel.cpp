#include "Channel.h"

Channel::Channel(int fd, FDEvent event, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* args)
{
	m_fd = fd;
	m_events = (int)event;
	m_args = args;
	readCallback = readFunc;
	writeCallback = writeFunc;
	destroyCallback = destroyFunc;
}

void Channel::WriteEventEnable(bool flag)
{
	//if (flag)	m_events |= (int)FDEvent::WriteEvent;
	if (flag)	m_events |= static_cast<int>(FDEvent::WriteEvent);
	else		m_events = m_events & ~static_cast<int>(FDEvent::WriteEvent);
}

bool Channel::isWriteEventEnable()
{
	return m_events & static_cast<int>(FDEvent::WriteEvent);
}

