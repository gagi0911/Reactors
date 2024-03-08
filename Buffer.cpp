//#define _GNU_SOURCE
#include "Buffer.h"
#include <stdlib.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

Buffer::Buffer(int size)
{
	m_Capacity = size;
	m_data = (char*)malloc(sizeof(char)*size);
	m_readPos = 0;
	m_writePos = 0;
	bzero(m_data, size);
}

Buffer::~Buffer()
{
	if (m_data!=nullptr)
	free(m_data);
}

void Buffer::extentRoom(int size)
{
	if (writeableSize() >= size) return;
	// enough space but not continuouns
	else if (m_readPos + writeableSize() >= size)
	{
		int readable = readableSize();
		memcpy(m_data, m_data + m_readPos, readable);
		m_readPos = 0;
		m_writePos = readable;
	}
	// no enough space
	else
	{
		void* tmp = realloc(m_data, m_Capacity + size);
		if (tmp == NULL)
		{
			return;
		}
		memset((char*)tmp + m_Capacity, 0, size);
		m_data = static_cast<char*>(tmp);
		m_Capacity += size;
	}
}

int Buffer::appendString(const char* data, int size)
{
	if (data == nullptr || size <= 0) return -1;
	extentRoom(size);
	memcpy(m_data + m_writePos, data, size);
	m_writePos += size;
	return 0;
}

int Buffer::appendString(const char* data)
{
	int size = strlen(data);
	int ret = appendString(data, size);
	return ret;
}

int Buffer::socketRead(int fd)
{
	struct iovec vec[2];
	int writeable = writeableSize();
	vec[0].iov_base = m_data + m_writePos;
	vec[0].iov_len = writeable;
	char* tmp = (char*)malloc(40960);
	vec[1].iov_base = tmp;
	vec[1].iov_len = 40960;
	int result = readv(fd, vec, 2);
	if (result == -1) return -1;
	else if (result <= writeable) m_writePos += result;
	else
	{
		m_writePos = m_Capacity;
		appendString(tmp, result - writeable);
	}
	free(tmp);
	return result;
}

int Buffer::sendData(int socket)
{
	int readable = readableSize();
	if (readable > 0)
	{
		//int count = send(socket, buf->data + buf->readPos, readable, 0);
		int count = send(socket, m_data + m_readPos, readable, MSG_NOSIGNAL);
		if (count)
		{
			m_readPos += count;
			usleep(1);
		}
		return count;
	}
	return 0;
}

char* Buffer::findCRLF()
{
	char* ptr = static_cast<char*>(memmem(m_data + m_readPos, readableSize(), "\r\n", 2));
	return ptr;
}
