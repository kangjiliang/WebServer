#include "socket_application_multithread.h"


/* 多线程多链接 阻塞         */
VOID* CSocketApplicationMultiThread::callback(VOID* args)
{
    CSocketRequest* req = (CSocketRequest*)args;
    while(NULL != req)
    {
        if(INVALIDFD != req->peerfd())
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

VOID CSocketApplicationMultiThread::working()
{
    while(NULL != m_reqtype)
    {
        CSocketRequest* req = m_reqtype->clone();
        if(NULL != req)
        {
            if(req->activate(m_block))  //没有新链接 会阻塞
            {
                THREAD tid;
                pthread_create(&tid, NULL, callback, req);
                pthread_detach(tid); //默认创建的线程是可结合的(joinable) 必须被其他线程回收(调用pthread_join)  或 调用pthread_detach使之分离
            }
            else
            {
                delete req;
            }
        }
    }
}