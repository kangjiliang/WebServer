#ifndef _HANDLER_H
#define _HANDLER_H

#include "public.h"
#include "receiver.h"


// 定义socket请求基类
class CSocketHandler
{
public:
    CSocketHandler():m_fd(INVALIDFD), m_rbuff(), m_sbuff(), m_reqinfo(), m_receiver(NULL){}
    virtual ~CSocketHandler()
    {
        if(m_receiver)
        {
            delete m_receiver;
        }
    }
    SOCKFD  fd() {return m_fd;}
    BOOL    sbuffempty() {return m_sbuff.empty();}
    STRING  selfip() const;
    STRING  peerip() const;
    STRING  selfport() const;
    STRING  selfaddr() const;
    STRING  peeraddr() const;
    VOID    closefd();
    VOID    setreceiver(CSocketReceiver* receiver) {m_receiver = receiver;}

    virtual CSocketHandler* clone() = 0;
    virtual BOOL  initialize(const STRING& ip, const WORD16& port, const BOOL& block) = 0;
    virtual BOOL  activate(const BOOL& block) = 0;
    virtual BOOL  receive();
    virtual BOOL  dispatch() = 0;
    virtual BOOL  process() = 0;
protected:
    SOCKFD  m_fd;
    STRING  m_rbuff;
    STRING  m_sbuff;
    STRMAP  m_reqinfo;
    CSocketReceiver* m_receiver;
};


#define SOCKET_REQUEST_CLONE(classname) \
virtual CSocketHandler* clone() \
{\
    CSocketHandler* req = new classname(*this);\
    if(req && m_receiver)\
    {\
        req->setreceiver(m_receiver->clone());\
    }\
    return req; \
}


class CTcpHandler : public CSocketHandler
{
public:
    virtual BOOL dispatch();
};


#endif
