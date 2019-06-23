#ifndef _SOCKET_APPLICATION_H
#define _SOCKET_APPLICATION_H

#include "handler.h"

typedef list<CSocketHandler*>           REQUESTLIST;
typedef list<CSocketHandler*>::iterator REQUESTITER;
typedef map<SOCKFD, CSocketHandler*>    REQUESTMAP;


// 定义socket应用基类
class CSocketApplication
{
public:
    CSocketApplication(STRING ip, WORD16 port, CSocketHandler* req) : m_reqtype(req), m_ip(ip), m_port(port), m_block(FALSE) {}
    virtual ~CSocketApplication();
    virtual VOID startup();
protected:
    virtual VOID working() = 0;
    CSocketHandler*  m_reqtype;  // 请求原型 (使用原型模式)
    STRING m_ip;
    WORD16 m_port;
    BOOL   m_block;
private:
};

#endif
