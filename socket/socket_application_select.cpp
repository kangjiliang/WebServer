#include "socket_application_select.h"

//单线程 单连接 非阻塞 select模式
VOID CSocketApplicationSelect::working()
{
    SWORD32 ret = 0;

    while(NULL != m_reqtype)
    {
        m_timeval.tv_sec  = 1;
        m_timeval.tv_usec = 0;

        select_fdset();

        ret = select(m_maxfd+1, &m_readset, &m_writeset, NULL, &m_timeval);
        if(ret < 0)
        {
            SOCKET_TRACE("select failed, errno %d: %s\n", errno, strerror(errno));
            break;
        }
        else if(0 == ret)
        {
            continue;
        }
        else
        {
            select_self();
            select_peer();
        }
    }
}

//设置select可读可写的fdset
VOID CSocketApplicationSelect::select_fdset()
{
    SOCKFD peerfd = INVALIDFD;
    SOCKFD selffd = INVALIDFD;

    FD_ZERO(&m_readset);
    FD_ZERO(&m_writeset);
    for(REQUESTITER it = m_requests.begin(); it != m_requests.end(); ++it)
    {
        CSocketHandler* req = *it;
        if(NULL != req)
        {
            peerfd = req->fd();
            if(INVALIDFD != peerfd)
            {
                //设置可读fdset
                FD_SET(peerfd, &m_readset);
                m_maxfd = peerfd > m_maxfd ? peerfd : m_maxfd;
                //当有数据要发送时 设置可写fdset
                if(!req->sbuffempty())
                {
                    FD_SET(peerfd, &m_writeset);
                }
            }
        }
    }
    // 如果是tcpserver 则本端socket可读会触发accept
    // 其他类型的 如tcpclient udp之类的 selffd为无效值
    if(NULL != m_reqtype)
    {
        selffd = m_reqtype->fd();
        if(INVALIDFD != selffd)
        {
            FD_SET(selffd, &m_readset);
            m_maxfd = selffd > m_maxfd ? selffd : m_maxfd;
        }
    }
}

VOID CSocketApplicationSelect::select_self()
{
    SOCKFD fd = m_reqtype->fd();
    if(INVALIDFD != fd && FD_ISSET(fd, &m_readset))
    {
        CSocketHandler* req = m_reqtype->clone();
        if(NULL != req)
        {
            if(req->activate(m_block))
            {
                m_requests.push_back(req);
            }
            else
            {
                SOCKET_TRACE("activate failed\n");
                delete req;
            }
        }
        else
        {
            SOCKET_TRACE("clone failed\n");
        }
    }

}

VOID CSocketApplicationSelect::select_peer()
{
    for(REQUESTITER it = m_requests.begin(); it != m_requests.end();)
    {
        CSocketHandler* req = *it;
        if(NULL != req)
        {
            SOCKFD peerfd = req->fd();
            if(INVALIDFD != peerfd)
            {
                //如果socket可读 则调用接收接口 接收成功则进行处理
                if(FD_ISSET(peerfd, &m_readset))
                {
                    if(req->receive())
                    {
                        req->process();
                    }
                }
                //如果是socket可写 则调用发送接口 发送数据
                if(FD_ISSET(peerfd, &m_writeset))
                {
                    req->dispatch();
                }
            }
            else
            {
                delete req;
                it = m_requests.erase(it);
                continue;
            }
        }
        else
        {
            SOCKET_TRACE("m_requests has NULL req\n");
        }
        ++it;
    }
}
