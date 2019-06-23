#ifndef _RECEIVER_H
#define _RECEIVER_H

#include "public.h"

/* 接收结果 */
enum SOCKET_RECEIVE_RESULT
{
    SOCKET_RECEIVE_ERROR,
    SOCKET_RECEIVE_NOTENOUGH,
    SOCKET_RECEIVE_SUCCESS,
};

/* 虚基类    socket接收处理基类 */
class CSocketReceiver
{
public:
    CSocketReceiver(){}
    virtual ~CSocketReceiver(){}
    virtual CSocketReceiver* clone() = 0;
    virtual VOID reset(STRMAP& infomap)
    {
        m_rcvstat = 0;
        infomap.clear();
    }
    virtual WORD32 receive_message(const SOCKFD& fd, STRING& rbuff, STRMAP& info) = 0;
protected:
    WORD32 m_rcvstat;
};

/* clone宏 */
#define SOCKET_RECEIVER_CLONE(classname) \
virtual CSocketReceiver* clone() \
{\
    return (CSocketReceiver*)(new classname(*this));\
}

/* 虚基类    tcp接收处理基类 */
class CTcpReceiver : public CSocketReceiver
{
public:
    virtual CSocketReceiver* clone() = 0;
    BOOL receiveonce(const SOCKFD& fd, STRING& info);
protected:
};

#endif
