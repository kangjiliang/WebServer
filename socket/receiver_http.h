#ifndef _RECEIVER_HTTP_H
#define _RECEIVER_HTTP_H

#include "receiver.h"


//接收一个http请求的状态
enum HTTP_RECV_STATUS
{
    HTTP_RECV_REQLINE,
    HTTP_RECV_HEADERS,
    HTTP_RECV_REQBODY
};

const STRING HTTP_METHOD  = "method";
const STRING HTTP_REQURL  = "url";
const STRING HTTP_VERSION = "version";
const STRING HTTP_REQFILE = "reqfile";
const STRING HTTP_QUERY   = "query";
const STRING HTTP_REQBODY = "reqbody";

const STRING HTTP_HEAD_CONLEN  = "Content-Length";
const STRING HTTP_HEAD_UPGRADE = "Upgrade";
const STRING HTTP_HEAD_CONNECT = "Connection";
const STRING HTTP_HEAD_WSVER   = "Sec-WebSocket-Version";
const STRING HTTP_HEAD_WSKEY   = "Sec-WebSocket-Key";


class CHttpReceiver : public CTcpReceiver
{
public:
    CHttpReceiver() : CTcpReceiver(), m_bodylen(0){}
    SOCKET_RECEIVER_CLONE(CHttpReceiver);
    virtual WORD32 receive_message(const SOCKFD& fd, STRING& rbuff, STRMAP& infomap);
protected:
    VOID receive_reqline(STRING& rbuff, STRMAP& infomap);
    VOID receive_headers(STRING& rbuff, STRMAP& infomap);
    BOOL receive_reqbody(STRING& rbuff, STRMAP& infomap);
    WORD64 m_bodylen;
};

#endif
