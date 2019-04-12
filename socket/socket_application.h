#ifndef _SOCKET_APPLICATION_H
#define _SOCKET_APPLICATION_H

#include "socket_request.h"

typedef list<CSocketRequest*>           REQUESTLIST;
typedef list<CSocketRequest*>::iterator REQUESTITER;
typedef map<SOCKFD, CSocketRequest*>    REQUESTMAP;


// 定义socket应用基类
class CSocketApplication
{
public:
    CSocketApplication(CSocketRequest* req) : m_reqtype(req) {}
    virtual ~CSocketApplication();
    virtual VOID startup();
protected:
    virtual VOID working() = 0;
    CSocketRequest*  m_reqtype;  // 请求原型 (使用原型模式)
    STRING ip;
    WORD16 port;
    BOOL   m_block;
private:
};

#endif
