#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include "socket_request_httpserver.h"
#include "socket_request_websocket.h"
#include "socket_application_select.h"
#include "socket_application_epoll.h"
#include "socket_application_multithread.h"

// webserver请求类 继承websocket类
class CWebServerRequst : public CWebSocketRequest
{
public:
    CWebServerRequst(STRING rootdir) : CWebSocketRequest(rootdir){}
};


// webserver应用类 继承select模式
class CWebServerSelect : public CSocketApplicationSelect
{
public:
    CWebServerSelect(STRING serverip, WORD16 serverport, CWebServerRequst* req) : CSocketApplicationSelect(serverip, serverport, req){}
};

// webserver应用类 继承Epoll模式
class CWebServerEpoll : public CSocketApplicationEpoll
{
public:
    CWebServerEpoll(STRING serverip, WORD16 serverport, CWebServerRequst* req) : CSocketApplicationEpoll(serverip, serverport, req){}
};

// webserver应用类 继承多线程模式
class CWebServerMultiThread : public CSocketApplicationMultiThread
{
public:
    CWebServerMultiThread(STRING serverip, WORD16 serverport, CWebServerRequst* req) : CSocketApplicationMultiThread(serverip, serverport, req){}
};

#endif
