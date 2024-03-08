#pragma once
#include "Buffer.h"
#include "EventLoop/EventLoop.h"
#include "Channel.h"
#include "Http/HttpRequest.h"
#include "Http/HttpResponse.h"

//#define MSG_SEND_AUTO
class TcpConnection
{
public:
	TcpConnection(int cfd, EventLoop* evloop);
	~TcpConnection();

	static int processRead(void* arg);
	static int processWrite(void* arg);
	static int destroy(void* arg);
private:
	EventLoop* m_evloop;
	Channel* m_channel;
	Buffer* m_readbuf;
	Buffer* m_writebuf;

	string m_name;
	HttpRequest* m_request;
	HttpResponse* m_response;
};