#ifndef _SOCKET_APPLICATION_MULTITHREAD_H
#define _SOCKET_APPLICATION_MULTITHREAD_H

#include "socket_application.h"

/* 多线程 阻塞 一个连接起一个线程 */
class CSocketApplicationMultiThread : public CSocketApplication
{
public:
    CSocketApplicationMultiThread(STRING serverip, WORD16 serverport, CSocketHandler* req) : CSocketApplication(serverip, serverport, req)
    {
        m_block = TRUE;
    }
    static  VOID* callback(VOID* args);
    virtual VOID  working();
protected:
};

#endif
