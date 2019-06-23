#ifndef _HANDLER_TCPSERVER_H
#define _HANDLER_TCPSERVER_H

#include "handler.h"


const SWORD32 TCPSERVER_LISTEN_BACKLOG = 10;

// tcpserver的处理类
class CTcpServerHandler : public CTcpHandler
{
public:
    virtual BOOL initialize(const STRING& ip, const WORD16& port, const BOOL& block);
    virtual BOOL activate(const BOOL& block);
protected:
private:
};



#endif

