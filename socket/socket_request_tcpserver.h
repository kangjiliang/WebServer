#ifndef _SOCKET_REQUEST_TCPSERVER_H
#define _SOCKET_REQUEST_TCPSERVER_H

#include "socket_request.h"


const SWORD32 SOCKET_TCPSERVER_LISTEN_BACKLOG = 10;

// tcpserver的请求类
class CTcpServerRequest : public CSocketRequest
{
public:
    virtual CSocketRequest* clone();
    virtual BOOL initialize(const STRING& ip, const WORD16& port, const BOOL& block);
    virtual BOOL activate(const BOOL& block);
    virtual BOOL receive();
    virtual BOOL dispatch();
    virtual BOOL process();
protected:
private:
};



#endif

