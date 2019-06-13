#ifndef _SOCKET_REQUEST_H
#define _SOCKET_REQUEST_H

#include "public.h"


class CSocketReceiver
{
public:
    virtual ~CSocketReceiver(){}
    virtual CSocketReceiver* clone() = 0;
    virtual WORD32 recvmessage(const SOCKFD& fd, STRMAP& info) = 0;
};

// 定义socket请求基类
class CSocketRequest
{
public:
    CSocketRequest():m_fd(INVALIDFD), m_rbuff(), m_sbuff(), m_reqinfo(), m_receiver(NULL){}
    virtual ~CSocketRequest()
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

    virtual CSocketRequest* clone() = 0;
    virtual BOOL  initialize(const STRING& ip, const WORD16& port, const BOOL& block) = 0;
    virtual BOOL  activate(const BOOL& block) = 0;
    virtual BOOL  receive() = 0;
    virtual BOOL  dispatch() = 0;
    virtual BOOL  process() = 0;
protected:
    SOCKFD  m_fd;
    STRING  m_rbuff;
    STRING  m_sbuff;
    STRMAP  m_reqinfo;
    CSocketReceiver* m_receiver;
};

#define SOCKET_RECEIVER_CLONE(classname) \
virtual CSocketReceiver* clone() \
{\
    return new classname(*this);\
}

#define SOCKET_REQUEST_CLONE(classname) \
virtual CSocketRequest* clone() \
{\
    CSocketRequest* req = new classname(*this);\
    if(req && m_receiver)\
    {\
        req->setreceiver(m_receiver->clone());\
    }\
    return req; \
}

BOOL recvinfo(const SOCKFD& fd, STRING& info);
BOOL sendinfo(const SOCKFD& fd, STRING& info);


STRING getgmttime(TIMET timestamp);
BOOL   getline(STRING& buffer, STRING& line);
BOOL   getsize(STRING& buffer, STRING& str, WORD64 size);
VOID   strsplit(const STRING& str, STRING sep, STRVEC& strvec);



#endif
