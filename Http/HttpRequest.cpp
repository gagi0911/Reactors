//#define _GNU_SOURCE

#include "HttpRequest.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>

#define HeaderSize 12

int hexToDec(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;

	return 0;
}


HttpRequest::HttpRequest()
{
	reset();
}

HttpRequest::~HttpRequest()
{
}

void HttpRequest::reset()
{
	m_curState = HttpRequestState::ParseReqLine;
	m_method = string();
	m_url = string();
	m_version = string();
	m_reqHeaders.clear();
}

void HttpRequest::addHeader(const string key, const string value)
{
	if (key.empty() || value.empty()) return;
	m_reqHeaders.insert(make_pair(key, value));
}

string HttpRequest::getHeader(const string key)
{
	if (m_reqHeaders.find(key) != m_reqHeaders.end())
	{
		return m_reqHeaders[key];
	}
	return string();
}

bool HttpRequest::parseRequestLine(Buffer* buf)
{
	char* end = buf->findCRLF();
	char* start = buf->getData() + buf->getReadPos();
	int linesize = end - start;
	if (linesize)
	{
		auto methodFunc = bind(&HttpRequest::setMethod, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", methodFunc);
		auto urlFunc = bind(&HttpRequest::seturl, this, placeholders::_1);
		start = splitRequestLine(start, end, " ", urlFunc);
		auto versionFunc = bind(&HttpRequest::setversion, this, placeholders::_1);
		splitRequestLine(start, end, NULL, versionFunc);

		// move readpos for read headers
		buf->AddtoReadPos(linesize + 2);
		m_curState = HttpRequestState::ParseReqHeaders;
		return true;
	}
	return false;
}

bool HttpRequest::parseRequestHeaders(Buffer* buf)
{
	char* end = buf->findCRLF();
	if (end != NULL)
	{
		char* start = buf->getData() + buf->getReadPos();
		int linesize = end - start;
		char* middle = static_cast<char*>(memmem(start, linesize, ": ", 2));
		if (middle != NULL)
		{
			int keyLen = middle - start;
			int valueLen = end - middle - 2;
			if (keyLen > 0 && valueLen > 0)
			{
				string key = string(start, keyLen);
				string value = string(middle + 2, valueLen);
				addHeader(key, value);
			}
			buf->AddtoReadPos(linesize + 2);
		}
		else
		{
			buf->AddtoReadPos(2);
			m_curState = HttpRequestState::ParseReqDone;
		}
		return true;
	}
	return false;
}

bool HttpRequest::parseRequest(Buffer* buf, HttpResponse* response, Buffer* sendbuf, int socket)
{
	bool flag = true;
	while (m_curState != HttpRequestState::ParseReqDone)
	{
		switch (m_curState)
		{
		case HttpRequestState::ParseReqLine:
			flag = parseRequestLine(buf);
			break;
		case HttpRequestState::ParseReqHeaders:
			flag = parseRequestHeaders(buf);
			break;
		case HttpRequestState::ParseReqBody:
			break;

		default:
			break;
		}
		if (!flag)
		{
			return flag;
		}
		// if parse is over, should prepare response data
		if (m_curState == HttpRequestState::ParseReqDone)
		{
			// deal with the data
			processHttpRequest(response);
			// try to response
			response->prepareMsg(sendbuf, socket);

		}
	}
	m_curState = HttpRequestState::ParseReqLine;
	return flag;
}

bool HttpRequest::processHttpRequest(HttpResponse* response)
{

	if (strcasecmp(m_method.data(), "get") != 0)
	{
		return -1;
	}
	m_url = decodeMsg(m_url);
	const char* file = NULL;
	if (strcmp(m_url.data(), "/") == 0)
	{
		file = "./";
	}
	else
	{
		file = m_url.data() + 1;
	}
	struct stat st;
	int ret = stat(file, &st);
	printf("filename: %s\n", file);
	//file not exist
	if (ret == -1)
	{
		// 404
		response->setFilename("404.html");
		response->setStatusCode(HttpStatusCode::NotFound);
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendFile;
		return 0;
	}
	response->setFilename(file);
	response->setStatusCode(HttpStatusCode::OK);

	if (S_ISDIR(st.st_mode))
	{
		// this is a catalog, return the content of the catalog
		response->addHeader("Content-type", getFileType(".html"));
		response->sendDataFunc = sendDir;
	}
	else
	{
		// this is a file, return the content of the file
		response->addHeader("Content-type", getFileType(file));
		response->addHeader("Content-length", to_string(st.st_size));
		response->sendDataFunc = sendFile;
	}
	return false;
}

const string HttpRequest::getFileType(const string name)
{
	// from left to right find ".", if not exist return null
	const char* dot = strrchr(name.data(), '.');
	if (dot == NULL)
		return "text/plain; charset=utf-8"; // plain text
	if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0)
		return "text/html; charset=utf-8";
	if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jepg") == 0)
		return "image/jpeg";
	if (strcmp(dot, ".gif") == 0)
		return "image/gif";
	if (strcmp(dot, ".png") == 0)
		return "image/png";
	if (strcmp(dot, ".css") == 0)
		return "text/css";
	if (strcmp(dot, ".au") == 0)
		return "audio/basic";
	if (strcmp(dot, ".wav") == 0)
		return "audio/wav";
	if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0)
		return "audio/midi";
	if (strcmp(dot, ".mp3") == 0)
		return "audio/mpeg";
	if (strcmp(dot, ".avi") == 0)
		return "video/x-msvideo";
	if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0)
		return "video/quicktime";
	if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0)
		return "video/mpeg";
	if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0)
		return "model/vrml";
	if (strcmp(dot, ".ogg") == 0)
		return "application/ogg";
	if (strcmp(dot, ".pac") == 0)
		return "application/x-ns-proxy-autoconfig";

	return "text/plain; charset=utf-8";
}

void HttpRequest::sendDir(const string dirName, Buffer* sendbuf, int cfd)
{
	char buf[4096] = { 0 };
	sprintf(buf, "<html><head><title>%s</title></head><body><table>", dirName.data());
	struct dirent** namelist;
	int num = scandir(dirName.data(), &namelist, NULL, alphasort);
	for (int i = 0; i < num; i++)
	{
		char* name = namelist[i]->d_name;
		struct stat st;
		char relapath[1024] = { 0 };
		sprintf(relapath, "%s/%s", dirName.data(), name);
		stat(relapath, &st);
		if (S_ISDIR(st.st_mode))
		{
			sprintf(buf + strlen(buf), "<tr><td><a href=\"%s/\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		else
		{
			sprintf(buf + strlen(buf), "<tr><td><a href=\"%s\">%s</a></td><td>%ld</td></tr>", name, name, st.st_size);
		}
		//send(cfd, buf, strlen(buf), 0);
		sendbuf->appendString(buf);
#ifndef MSG_SEND_AUTO
		sendbuf->sendData(cfd);
#endif // !MSG_SEND_AUTO		
		memset(buf, 0, sizeof(buf));
		free(namelist[i]);

	}
	sprintf(buf, "</table></body></html>");
	//send(cfd, buf, strlen(buf), 0);
	sendbuf->appendString(buf);
#ifndef MSG_SEND_AUTO
	sendbuf->sendData(cfd);
#endif // !MSG_SEND_AUTO

	free(namelist);
}

void HttpRequest::sendFile(const string FileName, Buffer* sendbuf, int cfd)
{
	int fd = open(FileName.data(), O_RDONLY);
	assert(fd > 0);
#if 1
	while (1)
	{
		char buf[1024];
		int len = read(fd, buf, sizeof(buf));
		if (len > 0)
		{
			//send(cfd, buf, len, 0);
			sendbuf->appendString(buf, len);
#ifndef MSG_SEND_AUTO
			sendbuf->sendData(cfd);
#endif // !MSG_SEND_AUTO			usleep(10);
		}
		else if (len == 0)
		{
			break;
		}
		else
		{
			close(fd);
			perror("read");
		}
	}
#else
	off_t offset = 0;
	int size = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	while (offset < size)
	{
		int ret = sendfile(cfd, fd, &offset, size - offset);
		printf("ret value: %d\n", ret);
		if (ret == -1)
		{
			perror("sendfile");
		}
	}
#endif
	close(fd);
}

char* HttpRequest::splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback)
{
	char* space = const_cast<char*>(end);
	if (sub != NULL)
	{
		space = static_cast<char*>(memmem(start, end - start, sub, strlen(sub)));
		assert(space != nullptr);
	}
	int size = space - start;
	callback(string(start, size));
	return space + 1;
}

string HttpRequest::decodeMsg(string msg)
{
	string str = string();
	const char* from = msg.data();
	for (; *from != '\0'; ++from)
	{
		// isxdigit -> judge if the char hex
		// Linux%E5%86%85%E6%A0%B8.jpg
		if (from[0] == '%' && isxdigit(from[1]) && isxdigit(from[2]))
		{
			// hex -> dec  int -> char
			// B2 == 178
			// bring 3 char to 1 char
			str.append(1, hexToDec(from[1]) * 16 + hexToDec(from[2]));

			// jump from[1] and from[2] since they are solved
			from += 2;
		}
		else
		{
			// copy
			str.append(1, *from);
		}

	}
	str.append(1, '\0');
	return str;
}