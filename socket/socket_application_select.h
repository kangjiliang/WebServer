#ifndef _SOCKET_APPLICATION_SELECT_H
#define _SOCKET_APPLICATION_SELECT_H

#include "socket_application.h"

//单线程 单连接 非阻塞 select模式
class CSocketApplicationSelect : public CSocketApplication
{
public:
    CSocketApplicationSelect(STRING serverip, WORD16 serverport, CSocketRequest* req) : CSocketApplication(req)
    {
        ip    = serverip;
        port  = serverport;
        m_block = FALSE;      //select模式使用非阻塞socket
    }    
protected:
    virtual VOID working();
    virtual VOID select_fdset();
    virtual VOID select_self();
    virtual VOID select_peer(); 
private:
    SOCKFD      m_maxfd;
    fd_set      m_readset;
    fd_set      m_writeset;
    TIMEVAL     m_timeval;
    REQUESTLIST m_requests;
};

#endif