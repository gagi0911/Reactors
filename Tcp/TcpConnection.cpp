#include "TcpConnection.h"
#include "Log.h"
#include <stdlib.h>
#include <stdio.h>

TcpConnection::TcpConnection(int cfd, EventLoop* evloop)
{
	m_evloop = evloop;
	m_readbuf = new Buffer(10240);
	m_writebuf = new Buffer(10240);
	m_request = new HttpRequest();
	m_response = new HttpResponse();
	m_name = "Connection-" + to_string(cfd);
	m_channel = new Channel(cfd, FDEvent::ReadEvent, processRead, processWrite, destroy, this);
	evloop->AddTask(m_channel, ElementType::ADD); 
	Debug("connected with server, threadID: %s, connName: %s", evloop->getThreadID(), m_name);

}

TcpConnection::~TcpConnection()
{
	if (m_readbuf && m_readbuf->readableSize() == 0 &&
		m_writebuf && m_readbuf->readableSize() == 0)
	{
		delete m_readbuf;
		delete m_writebuf;
		m_evloop->destroyChannel(m_channel);
		delete m_request;
		delete m_response;
	}
}


int TcpConnection::processRead(void* arg)
{
	TcpConnection* conn = (TcpConnection*)arg;
	int socket = conn->m_channel->getFd();
	int count = conn->m_readbuf->socketRead(socket);

	if (count > 0)
	{
#ifdef MSG_SEND_AUTO
		conn->m_channel->WriteEventEnable(true);
		conn->m_evloop->AddTask(conn->m_channel, ElementType::MODIFY);

#endif // MSG_SEND_AUTO

		bool flag = conn->m_request->parseRequest(conn->m_readbuf, conn->m_response, conn->m_writebuf, socket);
		if (!flag)
		{
			char* err = "HTTP/1.1 400 Bad Request\r\n\r\n";
			conn->m_writebuf->appendString(err);
		}
		// decode http requests
	}
	else
	{
#ifdef MSG_SEND_AUTO
		conn->m_evloop->AddTask(conn->m_channel, ElementType::DELETE);

#endif
	}
	// close the connection
#ifndef MSG_SEND_AUTO
	conn->m_evloop->AddTask(conn->m_channel, ElementType::DELETE);

#endif // !MSG_SEND_DATA
	return 0;
}

int TcpConnection::processWrite(void* arg)
{
	TcpConnection* conn = (TcpConnection*)arg;
	int count = conn->m_writebuf->sendData(conn->m_channel->getFd());
	if (count > 0)
	{
		// if the data is all sent
		if (conn->m_writebuf->readableSize() == 0)
		{
			// stop check write event
			conn->m_channel->WriteEventEnable(false);

			conn->m_evloop->AddTask(conn->m_channel, ElementType::MODIFY);
			conn->m_evloop->AddTask(conn->m_channel, ElementType::DELETE);

		}
	}
	return 0;
}

int TcpConnection::destroy(void* arg)
{
	TcpConnection* conn = (TcpConnection*)arg;
	if (conn != nullptr)
	{
		delete conn;
	}
	return 0;
}
