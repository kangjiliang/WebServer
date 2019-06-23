#ifndef _SOCKET_APPLICATION_EPOLL_H
#define _SOCKET_APPLICATION_EPOLL_H

#include "socket_application.h"

//单线程 非阻塞 epoll模式

const WORD32 SOCKET_APPLICATION_EPOLL_EVSIZE = 10;

class CSocketApplicationEpoll : public CSocketApplication
{
public:
    CSocketApplicationEpoll(STRING serverip, WORD16 serverport, CSocketHandler* req) :\
                             CSocketApplication(serverip, serverport, req){}
    virtual VOID working();
    virtual VOID epoll_setevent(WORD32 events, SOCKFD fd, VOID* req, SWORD32 op);
    virtual VOID epoll_self(WORD32 events, CSocketHandler* req);
    virtual VOID epoll_peer(WORD32 events, CSocketHandler* req);
protected:
    SOCKFD  m_epfd;
    EPEVENT m_epev;
    EPEVENT m_epevs[SOCKET_APPLICATION_EPOLL_EVSIZE];
};

#endif