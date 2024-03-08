#pragma once
#include "Dispatcher.h"
#include <string>
#include <sys/select.h>

class SelectDispatcher : public Dispatcher
{
public:
	//init data fit for the io method
	SelectDispatcher(EventLoop* evloop);
	~SelectDispatcher();
	//add
	int add() override;
	//remove
	int remove() override;
	//modify
	int modify() override;
	// event check
	int dispatch(int timeout = 2) override; // unit: s

private:

	void setFdSet();
	void clrFdSet();
	const int m_maxNode = 1024;
	fd_set m_readSet;
	fd_set m_writeSet;

};