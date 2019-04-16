#include "socket_application_epoll.h"


VOID CSocketApplicationEpoll::epoll_setevent(WORD32 events, SOCKFD fd, VOID* req, SWORD32 op)
{
    m_epev.events   = events;
    m_epev.data.ptr = req;
    epoll_ctl(m_epfd, op, fd, (op == EPOLL_CTL_DEL) ? NULL : &m_epev);
}


VOID CSocketApplicationEpoll::working()
{
    SWORD32 timeout = -1;
    SWORD32 retnum  = 0;

    m_epfd = epoll_create(SOCKET_APPLICATION_EPOLL_EVSIZE);
    if(m_epfd < 0)
    {
        perror("epoll_create failed");
        return;
    }

    epoll_setevent(EPOLLIN, m_reqtype->selffd(), m_reqtype, EPOLL_CTL_ADD);

    while(NULL != m_reqtype)
    {
        retnum = epoll_wait(m_epfd, m_epevs, sizeof(m_epevs)/sizeof(EPEVENT), timeout);
        if(retnum < 0)
        {
            perror("epoll_wait");
            cout << m_epfd << endl;
            break;
        }
        else if(retnum == 0)
        {
            continue;
        }
        else
        {
            for(SWORD32 i = 0; i < retnum; i++)
            {
                CSocketRequest* req = (CSocketRequest*)m_epevs[i].data.ptr;
                if(NULL != req)
                {
                    if(req == m_reqtype)
                    {
                        epoll_self(m_epevs[i].events, req);
                    }
                    else
                    {
                        epoll_peer(m_epevs[i].events, req);
                    }
                }
            }
        }

    }
}

VOID CSocketApplicationEpoll::epoll_self(WORD32 events, CSocketRequest* req)
{
    if(events & EPOLLIN)
    {
        CSocketRequest* newreq = m_reqtype->clone();
        if(NULL != newreq)
        {
            if(newreq->activate(m_block))
            {
                epoll_setevent(EPOLLIN | EPOLLOUT, newreq->peerfd(), newreq, EPOLL_CTL_ADD);
            }
            else
            {
                delete newreq;
            }
        }
    }
}

VOID CSocketApplicationEpoll::epoll_peer(WORD32 events, CSocketRequest* req)
{
    if(events & EPOLLIN)
    {
        if(req->receive())
        {
            req->process();
        }
    }
    if(events & EPOLLOUT)
    {
        req->dispatch();
    }

    if(INVALIDFD == req->peerfd())
    {
        epoll_setevent(events, req->peerfd(), req, EPOLL_CTL_DEL);
        delete req;
        return;
    }

    events = req->sbuffempty() ? EPOLLIN : (EPOLLIN | EPOLLOUT);
    epoll_setevent(events, req->peerfd(), req, EPOLL_CTL_MOD);
}

