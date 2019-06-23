#ifndef _HANDLER_WEBSOCKET_H
#define _HANDLER_WEBSOCKET_H

#include "handler_httpserver.h"

//在httpserver基础上 支持WebSocket协议
class CWSHandler : public CHttpHandler
{
public:
    CWSHandler(STRING rootdir) : CHttpHandler(rootdir){}
    SOCKET_REQUEST_CLONE(CWSHandler)

    virtual BOOL process();
    virtual VOID process_websocket();
    virtual VOID process_httprequest();

    BOOL upgrade_websocket();       //判断http请求是否是要切换到websocket
    BOOL response_upgradews();         //应答切换到websocket

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
