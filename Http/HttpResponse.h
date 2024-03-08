#pragma once
#include "Buffer.h"
#include <string>
#include <map>
#include <functional>

using namespace std;

// 定义状态码枚举
enum class HttpStatusCode
{
    Unknown,
    OK = 200,
    MovedPermanently = 301,
    MovedTemporarily = 302,
    BadRequest = 400,
    NotFound = 404
};

// 定义结构体
class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void addHeader(const string key, const string value);
    
    void prepareMsg(Buffer* sendBuf, int socket);

    inline void setFilename(string filename)
    {
        m_fileName = filename;
    }

    inline void setStatusCode(HttpStatusCode code)
    {
        m_statusCode = code;
    }
     

public:
    function<void(const string, Buffer*, int)> sendDataFunc;

private:
    HttpStatusCode m_statusCode;
    string m_fileName;
    // response headers
    map<string, string> m_headers;

    const map<int, string> m_info = {
        {200, "OK"},
        {301, "MovedPermanently"},
        {302, "MovedTemporarily"},
        {400, "BadRequest"},
        {404, "NotFound"}
    };
};

// 初始化
