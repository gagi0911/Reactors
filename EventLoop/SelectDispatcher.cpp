#include "SelectDispatcher.h"
#include <sys/select.h>
#include <stdlib.h>
#include <stdio.h>


SelectDispatcher::SelectDispatcher(EventLoop* evloop)
	:Dispatcher(evloop)
{
	FD_ZERO(&m_readSet);
	FD_ZERO(&m_writeSet);
	m_name = "Select";
}

SelectDispatcher::~SelectDispatcher()
{
}

int SelectDispatcher::add()
{
	if (m_channel->getFd() >= m_maxNode)
	{
		return -1;
	}
	setFdSet();
	return 0;
}

int SelectDispatcher::remove()
{
	clrFdSet();
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArgs()));
	return 0;
}

int SelectDispatcher::modify()
{
	if (m_channel->getEvents() & (int)FDEvent::ReadEvent)
	{
		FD_SET(m_channel->getFd(), &m_readSet);
		FD_CLR(m_channel->getFd(), &m_writeSet);
	}
	if (m_channel->getEvents() & (int)FDEvent::WriteEvent)
	{
		FD_SET(m_channel->getFd(), &m_writeSet);
		FD_CLR(m_channel->getFd(), &m_readSet);
	}
	return 0;
}

int SelectDispatcher::dispatch(int timeout)
{
	struct timeval val;
	val.tv_sec = timeout;
	val.tv_usec = 0;
	fd_set readtmp = m_readSet;
	fd_set writetmp = m_writeSet;
	int count = select(m_maxNode, &readtmp, &writetmp, NULL, &val);
	if (count == -1)
	{
		perror("select");
		exit(0);
	}
	for (int i = 0; i < m_maxNode; i++)
	{
		if (FD_ISSET(i, &readtmp))
		{
			m_evLoop->eventActivate(i, (int)FDEvent::ReadEvent);
		}
		if (FD_ISSET(i, &writetmp))
		{
			m_evLoop->eventActivate(i, (int)FDEvent::WriteEvent);
		}
	}
	return 0;
}

void SelectDispatcher::setFdSet()
{
	if (m_channel->getEvents() & (int)FDEvent::ReadEvent)
	{
		FD_SET(m_channel->getFd(), &m_readSet);
	}
	if (m_channel->getEvents() & (int)FDEvent::WriteEvent)
	{
		FD_SET(m_channel->getFd(), &m_writeSet);
	}
}

void SelectDispatcher::clrFdSet()
{
	if (m_channel->getEvents() & (int)FDEvent::ReadEvent)
	{
		FD_CLR(m_channel->getFd(), &m_readSet);
	}
	if (m_channel->getEvents() & (int)FDEvent::WriteEvent)
	{
		FD_CLR(m_channel->getFd(), &m_writeSet);
	}
}
