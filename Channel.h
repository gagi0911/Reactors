#pragma once
#include <functional>

using namespace std;
//typedef int (*handleFunc)(void* args);
//using handleFunc = int(*)(void* args);

enum class FDEvent {
	ReadEvent = 0x01,
	WriteEvent = 0x02,
	TimeOut = 0x04
};

class Channel
{
public:
	using handleFunc = function<int(void*)>;
	Channel(int fd, FDEvent event, handleFunc readFunc, handleFunc writeFunc, handleFunc destroyFunc, void* args);
	void WriteEventEnable(bool flag);
	bool isWriteEventEnable();

	inline int getFd() 
	{
		return m_fd;
	}
	inline int getEvents()
	{
		return m_events;
	}
	inline const void* getArgs()
	{
		return m_args;
	}

	handleFunc readCallback;
	handleFunc writeCallback;
	handleFunc destroyCallback;

private:
	int m_fd;
	int m_events;
	void* m_args;
};