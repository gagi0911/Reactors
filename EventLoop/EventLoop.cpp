#include "EventLoop.h"
#include "SelectDispatcher.h"
#include "EpollDispatcher.h"
#include "PollDispatcher.h"

#include <stdlib.h>
#include <assert.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>


EventLoop::EventLoop()
	:EventLoop("MainThread")
{
}

EventLoop::EventLoop(const string threadname)
{
	m_isQuit = true;
	m_threadID = this_thread::get_id();
	m_threadName = threadname;
	m_dispatcher = new EpollDispatcher(this);
	m_channelMap.clear();
	int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, m_socketPair);
	if (ret == -1)
	{
		perror("socket pair");
		exit(0);
	}
#if 0
	Channel* tmpchannel = new Channel(m_socketPair[1], FDEvent::ReadEvent, readLocalMessage, nullptr, nullptr, this);
#else
	auto obj = bind(&EventLoop::readMessage, this);
	Channel* tmpchannel = new Channel(m_socketPair[1], FDEvent::ReadEvent, obj, nullptr, nullptr, this);
#endif

	AddTask(tmpchannel, ElementType::ADD);
}

EventLoop::~EventLoop()
{
}

int EventLoop::Run()
{
	m_isQuit = false;
	if (m_threadID != this_thread::get_id())
	{
		return -1;
	}
	while (!m_isQuit)
	{
		m_dispatcher->dispatch();
		ProcessTaskQ();
	}
	return 0;
}

int EventLoop::eventActivate(int fd, int event)
{
	assert(fd >= 0);
	Channel* channel = m_channelMap[fd];
	assert(channel->getFd() == fd);
	if (event & (int)FDEvent::ReadEvent && channel->readCallback)
	{
		channel->readCallback(const_cast<void*>(channel->getArgs()));
	}
	if (event & (int)FDEvent::WriteEvent && channel->writeCallback)
	{
		channel->writeCallback(const_cast<void*>(channel->getArgs()));
	}
	return 0;
}

int EventLoop::AddTask(Channel* channel, ElementType type)
{
	m_mutex.lock();
	ChannelElement* node = new ChannelElement;
	node->channel = channel;
	node->type = type;
	m_taskQ.push(node);
	m_mutex.unlock();

	if (m_threadID == this_thread::get_id())
	{
		ProcessTaskQ();
	}
	else
	{
		taskWakeup();
	}
	return 0;
}

int EventLoop::ProcessTaskQ()
{
	while (!m_taskQ.empty())
	{
		m_mutex.lock();
		ChannelElement* head = m_taskQ.front();
		m_taskQ.pop();
		m_mutex.unlock();
		Channel* channel = head->channel;
		if (head->type == ElementType::ADD)
		{
			add(channel);
		}
		else if (head->type == ElementType::DELETE)
		{
			remove(channel);
		}
		else if (head->type == ElementType::MODIFY)
		{
			modify(channel);
		}
		delete head;
	}
	return 0;
}

int EventLoop::add(Channel* channel)
{
	int fd = channel->getFd();
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		m_channelMap.insert(make_pair(fd, channel));
		m_dispatcher->setChannel(channel);
		return m_dispatcher->add();
	}
	return -1;
}

int EventLoop::remove(Channel* channel)
{
	int fd = channel->getFd();
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->remove();
	return ret;
}
int EventLoop::modify(Channel* channel)
{
	int fd = channel->getFd();
	if (m_channelMap.find(fd) == m_channelMap.end())
	{
		return -1;
	}
	m_dispatcher->setChannel(channel);
	int ret = m_dispatcher->modify();
	return ret;
}

int EventLoop::destroyChannel(Channel* channel)
{
	int fd = channel->getFd();
	auto it = m_channelMap.find(fd);
	if (it != m_channelMap.end())
	{
		m_channelMap.erase(it);
		close(fd);
		delete channel;
	}
	return 0;
}

int EventLoop::readLocalMessage(void* args)
{
	EventLoop* evloop = static_cast<EventLoop*>(args);
	char buf[256];
	read(evloop->m_socketPair[1], buf, sizeof(buf));
	return 0;
}

int EventLoop::readMessage()
{
	char buf[256];
	read(m_socketPair[1], buf, sizeof(buf));
	return 0;
}

void EventLoop::taskWakeup()
{
	const char* str = "hello";
	write(m_socketPair[0], str, strlen(str));
}
