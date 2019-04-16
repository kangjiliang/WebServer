#ifndef _WEBSERVER_H
#define _WEBSERVER_H

#include "socket_request_httpserver.h"
#include "socket_application_select.h"
#include "socket_application_epoll.h"


// webserver请求类 继承http请求类
class CWebServerRequst : public CHttpServerRequest
{
public:
    CWebServerRequst(STRING rootdir) : CHttpServerRequest(rootdir){}
};

// webserver应用类 继承select模式
class CWebServerSelect : public CSocketApplicationSelect
{
public:
    CWebServerSelect(STRING serverip, WORD16 serverport, CWebServerRequst* req) : CSocketApplicationSelect(serverip, serverport, req){}
};

// webserver应用类 继承select模式
class CWebServerEpoll : public CSocketApplicationEpoll
{
public:
    CWebServerEpoll(STRING serverip, WORD16 serverport, CWebServerRequst* req) : CSocketApplicationEpoll(serverip, serverport, req){}
};

#endif
