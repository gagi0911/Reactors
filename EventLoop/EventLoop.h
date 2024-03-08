#pragma once
#include "Dispatcher.h"
#include "Channel.h"
#include <thread>
#include <queue>
#include <map>
#include <mutex>
#include <string>

using namespace std;

enum class ElementType:char{ ADD, DELETE, MODIFY };

class ChannelElement
{
	friend class EventLoop;
	ElementType type;
	Channel* channel;
};

class Dispatcher;

class EventLoop
{
public:
	EventLoop();
	EventLoop(const string threadname);
	~EventLoop();

	int Run();

	int eventActivate(int fd, int event);

	int AddTask(Channel* channel, ElementType type);

	int ProcessTaskQ();

	inline thread::id getThreadID()
	{
		return m_threadID;
	}

	// free channel
	int destroyChannel(Channel* channel);
	static int readLocalMessage(void* args);
	int readMessage();

private:
	// task solve funcs(add,delete,modify)
	int add(Channel* channel);
	int remove(Channel* channel);
	int modify(Channel* channel);

	void taskWakeup();

private:
	bool m_isQuit;
	// this ptr pointed to child class
	Dispatcher* m_dispatcher;

	//taskqueue
	queue<ChannelElement*> m_taskQ;

	// channel map
	map<int, Channel*> m_channelMap;
	thread::id m_threadID;
	string m_threadName;

	mutex m_mutex;
	int m_socketPair[2]; // store fd pair to activate blocked io func
};

