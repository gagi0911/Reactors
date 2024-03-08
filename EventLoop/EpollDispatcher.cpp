#include "EpollDispatcher.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int EpollDispatcher::epollCTL(int op)
{
	struct epoll_event ev;
	ev.data.fd = m_channel->getFd();
	int event = 0;
	if (m_channel->getEvents() & static_cast<int>(FDEvent::ReadEvent))
	{
		event |= EPOLLIN;
	}
	if (m_channel->getEvents() & static_cast<int>(FDEvent::WriteEvent))
	{
		event |= EPOLLOUT;
	}
	ev.events = event;
	int ret = epoll_ctl(m_epfd, op, m_channel->getFd(), &ev);
	return ret;
}

EpollDispatcher::EpollDispatcher(EventLoop* evloop)
	:Dispatcher(evloop)
{
	m_epfd = epoll_create(10);
	if (m_epfd == -1)
	{
		perror("epoll create\n");
		exit(0);
	}
	m_events = new struct epoll_event[m_maxNode];
	m_name = "Epoll";
}

EpollDispatcher::~EpollDispatcher()
{
	close(m_epfd);
	delete[] m_events;
}

int EpollDispatcher::add()
{
	int ret = epollCTL(EPOLL_CTL_ADD);
	if (ret == -1)
	{
		perror("epollAdd");
		exit(0);
	}
	return ret;
}

int EpollDispatcher::remove()
{
	int ret = epollCTL(EPOLL_CTL_DEL);
	if (ret == -1)
	{
		perror("epollRemove");
		exit(0);
	}
	// using channel to destroy tcpconnection
	m_channel->destroyCallback(const_cast<void*>(m_channel->getArgs()));
	return ret;
}

int EpollDispatcher::modify()
{
	int ret = epollCTL(EPOLL_CTL_MOD);
	if (ret == -1)
	{
		perror("epollModify");
		exit(0);
	}
	return ret;
}

int EpollDispatcher::dispatch(int timeout)
{
	int count = epoll_wait(m_epfd, m_events, m_maxNode, timeout * 1000);
	for (int i = 0; i < count; i++)
	{
		int events = m_events[i].events;
		int fd = m_events[i].data.fd;
		if (events & EPOLLERR || events & EPOLLOUT)
		{
			//epollRemove(channel, evloop);
			continue;
		}
		if (events & EPOLLIN)
		{
			m_evLoop->eventActivate(fd, (int)FDEvent::ReadEvent);
		}
		if (events & EPOLLOUT)
		{
			m_evLoop->eventActivate(fd, (int)FDEvent::WriteEvent);
		}
	}
	return 0;
}
