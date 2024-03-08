#pragma once
#include "Buffer.h"
#include "HttpResponse.h"
#include <map>
#include <string>
#include <functional>

using namespace std;

// decode phase
enum class HttpRequestState:char
{
	ParseReqLine,
	ParseReqHeaders,
	ParseReqBody,
	ParseReqDone
};

class HttpRequest
{
public:
	HttpRequest();
	~HttpRequest();

	void reset();

	inline HttpRequestState getState()
	{
		return HttpRequestState();
	}

	inline void SetState(HttpRequestState state)
	{
		m_curState = state;
	}

	void addHeader(const string key, const string value);

	string getHeader(const string key);

	bool parseRequestLine(Buffer* buf);
	bool parseRequestHeaders(Buffer* buf);

	bool parseRequest(Buffer* buf, HttpResponse* response, Buffer* sendbuf, int socket);

	string decodeMsg(string msg);
	bool processHttpRequest(HttpResponse* response);

	const string getFileType(const string name);

	static void sendDir(const string dirName, Buffer* sendbuf, int cfd);
	static void sendFile(const string FileName, Buffer* sendbuf, int cfd);

	inline void setMethod(string method)
	{
		m_method = method;
	}

	inline void seturl(string url)
	{
		m_url = url;
	}

	inline void setversion(string version)
	{
		m_version = version;
	}

private:
	char* splitRequestLine(const char* start, const char* end, const char* sub, function<void(string)> callback);

	string m_method;
	string m_url;
	string m_version;
	map<string, string> m_reqHeaders;
	HttpRequestState m_curState;
};


