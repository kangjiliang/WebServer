#include "socket_application_threadpool.h"


//请求激活后的处理函数 阻塞处理
VOID* CSocketApplicationThreadPool::callback(VOID* args)
{
    CSocketRequest* req = (CSocketRequest*)args;
    while(NULL != req)
    {
        if(INVALIDFD != req->fd())
        {
            if(req->receive())   //没有新数据发来 会阻塞
            {
                req->process();
            }
            req->dispatch();
        }
        else
        {
            delete req;
            req = NULL;
        }
    }
    return NULL;
}

VOID CSocketApplicationThreadPool::working()
{
    while(NULL != m_reqtype)
    {
        CSocketRequest* req = m_reqtype->clone();
        if(NULL != req)
        {
            if(req->activate(m_block))  //有新连接后 插入线程池去处理
            {
                m_threadpool.pushtask(callback, req);
            }
            else
            {
                delete req;
            }
        }
    }
}