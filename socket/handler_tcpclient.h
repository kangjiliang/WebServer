#ifndef _HANDLER_TCPCLIENT_H
#define _HANDLER_TCPCLIENT_H

#include "handler.h"



// tcpclient的处理类
class CTcpClientHandler : public CTcpHandler
{
public:
    virtual BOOL initialize(const STRING& ip, const WORD16& port, const BOOL& block);
    virtual BOOL activate(const BOOL& block);
protected:
private:
};



#endif


