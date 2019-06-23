#include "handler_websocket.h"
#include "receiver_websocket.h"
#include "codec.h"


//应答一帧数据
BOOL CWSHandler::response_frame(const STRING& data)
{
    WORD64 length = data.size();
    if(length < 126)
    {
        m_sbuff.append(1, (CHAR)0x81);
        m_sbuff.append(1, (CHAR)length);
    }
    else if(length < 0xFFFF)
    {
        m_sbuff.append(1, (CHAR)0x81);
        m_sbuff.append(1, (CHAR)126);
        m_sbuff.append(1, (CHAR)(length >> 8));
        m_sbuff.append(1, (CHAR)length);
    }
    else
    {
        m_sbuff.append(1, (CHAR)0x81);
        m_sbuff.append(1, (CHAR)127);
        m_sbuff.append(1, (CHAR)(length >> 56));
        m_sbuff.append(1, (CHAR)(length >> 48));
        m_sbuff.append(1, (CHAR)(length >> 40));
        m_sbuff.append(1, (CHAR)(length >> 32));
        m_sbuff.append(1, (CHAR)(length >> 24));
        m_sbuff.append(1, (CHAR)(length >> 16));
        m_sbuff.append(1, (CHAR)(length >>  8));
        m_sbuff.append(1, (CHAR)(length));
    }
    m_sbuff.append(data, 0 , data.size());
    return TRUE;
}


// 应答WebSocket握手请求
BOOL CWSHandler::response_upgradews()
{
    m_sbuff.append("HTTP/1.1 101 Switching Protocols\r\n");
    m_sbuff.append("Upgrade: websocket\r\n");
    m_sbuff.append("Connection: Upgrade\r\n");
    m_sbuff.append("Sec-WebSocket-Accept: ");

    CHAR sha1str[20] = {0};
    STRING wskey = m_reqinfo[HTTP_HEAD_WSKEY] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1(wskey.c_str(), wskey.size(), sha1str);

    m_sbuff.append(base64_encode(STRING(sha1str,sizeof(sha1str))));
    m_sbuff.append("\r\n\r\n");

    return TRUE;
}

//判断是否要切换到 WebSocket 协议
BOOL CWSHandler::upgrade_websocket()
{
    STRMAP::iterator itupgr = m_reqinfo.find("Upgrade");
    STRMAP::iterator itconn = m_reqinfo.find("Connection");
    STRMAP::iterator itwsve = m_reqinfo.find("Sec-WebSocket-Version");
    STRMAP::iterator itwske = m_reqinfo.find("Sec-WebSocket-Key");
    if(itupgr != m_reqinfo.end() && itconn != m_reqinfo.end() &&
       itwsve != m_reqinfo.end() && itwske != m_reqinfo.end())
    {
        return TRUE;
    }
    return FALSE;
}

//处理一个websocket请求
BOOL CWSHandler::process()
{
    if(upgrade_websocket() && m_receiver)
    {
        response_upgradews();
        delete m_receiver;
        m_receiver = new CWSReceiver();
        m_receiver->reset(m_reqinfo);
        m_iswebsocket = TRUE;
    }
    else
    {
        if(m_iswebsocket)
        {
            show_reqinfo();
            process_websocket();
            show_respinfo();
            return TRUE;
        }
        else
        {
            process_httprequest();
        }
    }
    return TRUE;
}

VOID CWSHandler::process_websocket()
{
    response_frame(m_reqinfo[WS_REQBODY]);  //直接回显收到的消息
}

VOID CWSHandler::process_httprequest()
{
    CHttpHandler::process();
}
