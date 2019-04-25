#ifndef _SOCKET_APPLICATION_THREADPOOL_H
#define _SOCKET_APPLICATION_THREADPOOL_H

#include "socket_application.h"
#include "threadpool.h"

/* 线程池 */
class CSocketApplicationThreadPool : public CSocketApplication
{
public:
    CSocketApplicationThreadPool(WORD32 tnum, STRING serverip, WORD16 serverport, CSocketRequest* req) : \
                                 CSocketApplication(serverip, serverport, req), m_threadpool(tnum)
    {
        m_block = TRUE;
    }
    static  VOID* callback(VOID* args);
    virtual VOID  working();
protected:
    CThreadPool m_threadpool;
};

#endif
