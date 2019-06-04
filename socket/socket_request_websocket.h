#ifndef _SOCKET_REQUEST_WEBSOCKET_H
#define _SOCKET_REQUEST_WEBSOCKET_H

#include "socket_request_httpserver.h"

enum WEBSOCKET_RECVSTATE
{
    WS_RECV_BYTE1,
    WS_RECV_BYTE2,
    WS_RECV_LENGTH,
    WS_RECV_MASKEY,
    WS_RECV_DATA,
};

//在httpserver基础上 支持WebSocket协议
class CWebSocketRequest : public CHttpServerRequest
{
public:
    CWebSocketRequest(STRING rootdir) : CHttpServerRequest(rootdir){}
    SOCKET_REQUEST_CLONE(CWebSocketRequest)

    virtual BOOL receive();
    virtual BOOL process();

    BOOL receive_frame();         //接收websocke一帧
    BOOL receive_frame_byte1();   //接收第一个字节
    BOOL receive_frame_byte2();   //接收第二个字节
    BOOL receive_frame_length();  //接收消息长度
    BOOL receive_frame_maskey();  //接收掩码
    BOOL receive_frame_reqbody(); //接收消息体
    VOID umask(CHAR *data, WORD64 len, CHAR *mask); //解掩码

    BOOL is_upgrade_websocket();       //判断http请求是否是要切换到websocket
    BOOL response_upgrade_websocket(); //应答切换到websocket

    BOOL response_frame(const STRING& data);  //websocket响应一帧数据


protected:
    BOOL m_iswebsocket;
    CHAR m_fin;
    CHAR m_opcode;
    CHAR m_mask;
    CHAR m_maskey[4];
    //STRING m_reqbody;  //http基类已有声明
    //WORD64 m_bodylen;  //http基类已有声明
};


#endif
