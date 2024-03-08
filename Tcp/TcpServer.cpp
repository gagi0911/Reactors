#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include "TcpServer.h"
#include "TcpConnection.h"
#include "Channel.h"

TcpServer::TcpServer(unsigned short port, int threadNum)
{
	m_port = port;
	m_mainloop = new EventLoop();
	m_threadNum = threadNum;
	m_pool = new ThreadPool(m_mainloop, threadNum);
	SetListener();
}

TcpServer::~TcpServer()
{
	delete m_mainloop;
	delete m_pool;
}

bool TcpServer::SetListener()
{
	// create listen fd
	int lfd = socket(AF_INET, SOCK_STREAM, 0);
	if (lfd == -1)
	{
		perror("socket");
		return false;
	}
	// port reuse
	int opt = 1;
	int ret = setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret == -1)
	{
		perror("setsockopt");
		return false;
	}
	//bind
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(lfd, (struct sockaddr*)&addr, sizeof(addr));
	if (ret == -1)
	{
		perror("bind");
		return false;
	}
	// set listen
	ret = listen(lfd, 128);
	if (ret == -1)
	{
		perror("listen");
		return false;
	}
	m_lfd = lfd;
	return true;
}

void TcpServer::Run()
{
	m_pool->Run();
	Channel* channel = new Channel(m_lfd, FDEvent::ReadEvent, acceptConnection, nullptr, nullptr, this);
	m_mainloop->AddTask(channel, ElementType::ADD);
	m_mainloop->Run();

}

int TcpServer::acceptConnection(void* arg)
{
	TcpServer* server = static_cast<TcpServer*>(arg);
	int cfd = accept(server->m_lfd, NULL, NULL);
	// get a reactor model from threadpool
	EventLoop* evloop = server->m_pool->takeWorkEventLoop();
	// tcpconnection
	new TcpConnection(cfd, evloop);
	return 0;
}
