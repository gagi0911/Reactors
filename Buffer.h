#pragma once
#include <string>
using namespace std;

class Buffer
{
public:
	Buffer(int size);
	~Buffer();

	void extentRoom(int size);

	inline int writeableSize()
	{
		return m_Capacity - m_writePos;
	}

	inline int readableSize()
	{
		return m_writePos - m_readPos;
	}

	int appendString(const char* data, int size);
	int appendString(const char* data);

	int socketRead(int fd);

	int sendData(int socket);

	char* findCRLF();

	inline char* getData()
	{
		return m_data;
	}

	inline int getReadPos()
	{
		return m_readPos;
	}

	inline void AddtoReadPos(int append)
	{
		m_readPos += append;
	}

private:
	char* m_data;
	int m_readPos;
	int m_writePos;
	int m_Capacity;
};