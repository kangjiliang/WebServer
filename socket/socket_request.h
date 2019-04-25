#ifndef _SOCKET_REQUEST_H
#define _SOCKET_REQUEST_H


#include "public.h"

// 定义socket地址类 封装socket ip port
class CSocketAddress
{
public:
    CSocketAddress() : fd(INVALIDFD), ip(), port(0), rbuff(), sbuff() {}
    STRING selfaddr() const;
    STRING peeraddr() const;
    VOID   noblock();
    VOID   setaddr(const SOCKFD& fd, const STRING& ip, const WORD16& port, const BOOL& block);
    VOID   closefd();
    SOCKFD fd;    // socket
    STRING ip;    // ip地址
    WORD16 port;  // 端口号
    STRING rbuff; // 接收数据的buffer
    STRING sbuff; // 发送数据的buffer
};


// 定义socket请求基类
class CSocketRequest
{
public:
    CSocketRequest():m_sockaddr(){}
    virtual ~CSocketRequest(){}
    SOCKFD fd() {return m_sockaddr.fd;}
    BOOL   sbuffempty() {return m_sockaddr.sbuff.empty();}

    virtual CSocketRequest* clone() = 0;
    virtual BOOL  initialize(const STRING& ip, const WORD16& port, const BOOL& block) = 0;
    virtual BOOL  activate(const BOOL& block) = 0;
    virtual BOOL  receive() = 0;
    virtual BOOL  dispatch() = 0;
    virtual BOOL  process() = 0;
protected:
    CSocketAddress m_sockaddr;  //本端地址
};


BOOL recvinfo(const SOCKFD& fd, STRING& info);
BOOL sendinfo(const SOCKFD& fd, STRING& info);


STRING getgmttime(TIMET timestamp);
BOOL   getline(STRING& buffer, STRING& line);
BOOL   getsize(STRING& buffer, STRING& str, WORD64 size);
VOID   strsplit(const STRING& str, STRING sep, STRVEC& strvec);



#endif
