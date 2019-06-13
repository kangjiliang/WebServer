#include "socket_request_websocket.h"
#include "codec.h"

/*------------------ WebSocket 帧头 ----------------------------+
|0               1               2               3              |
|0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7 0 1 2 3 4 5 6 7|
+-+-+-+-+-------+-+-------------+-------------------------------+
|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
|N|V|V|V|       |S|             |   (if payload len==126/127)   |
| |1|2|3|       |K|             |                               |
+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
|     Extended payload length continued, if payload len == 127  |
+ - - - - - - - - - - - - - - - +-------------------------------+
|                               |Masking-key, if MASK set to 1  |
+-------------------------------+-------------------------------+
| Masking-key (continued)       |          Payload Data         |
+-------------------------------- - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
|                     Payload Data continued ...                |
+---------------------------------------------------------------+
*/

//接收websocket一帧的第一个字节 并解析出其中的FIN OPCODE
BOOL CWebSocketRequest::receive_frame_byte1()
{
    if(WS_RECV_BYTE1 == m_rcvstat)
    {
        STRING info;
        if(getsize(m_rbuff, info, 1))
        {
            m_fin = (info[0] & 0x80) == 0x80;
            m_opcode = info[0] & 0x0F;
            m_rcvstat = WS_RECV_BYTE2;
        }
    }
    return TRUE;
}

//接收websocket一帧的第二个字节 解析出其中的MASK 和 Payload len
BOOL CWebSocketRequest::receive_frame_byte2()
{
    if(WS_RECV_BYTE2 == m_rcvstat)
    {
        STRING info;
        if(getsize(m_rbuff, info, 1))
        {
            m_mask = (info[0] & 0x80) == 0x80;
            m_bodylen = info[0] & 0x7F;
            m_rcvstat = WS_RECV_LENGTH;
        }
    }
    return TRUE;
}

//如果Payload len是126 则实际消息长度为后面的2个字节
//如果Payload len是127 则实际消息长度为后面的8个字节
BOOL CWebSocketRequest::receive_frame_length()
{
    if(WS_RECV_LENGTH == m_rcvstat)
    {
        STRING info;
        if(126 == m_bodylen)
        {
            if(getsize(m_rbuff, info, 2))
            {
                m_bodylen = info[0];
                m_bodylen = (m_bodylen << 8) | info[1];
                m_rcvstat = WS_RECV_MASKEY;
            }
        }
        else if(127 == m_bodylen)
        {
            if(getsize(m_rbuff, info, 8))
            {
                m_bodylen = info[0];
                m_bodylen = (m_bodylen << 8) | info[1];
                m_bodylen = (m_bodylen << 8) | info[2];
                m_bodylen = (m_bodylen << 8) | info[3];
                m_bodylen = (m_bodylen << 8) | info[4];
                m_bodylen = (m_bodylen << 8) | info[5];
                m_bodylen = (m_bodylen << 8) | info[6];
                m_bodylen = (m_bodylen << 8) | info[7];
                m_rcvstat = WS_RECV_MASKEY;
            }
        }
        else
        {
            m_rcvstat = WS_RECV_MASKEY;
        }
    }
    return TRUE;
}

//如果MASK为1 则后面4个字节是Masking-key
BOOL CWebSocketRequest::receive_frame_maskey()
{
    STRING info;
    if(WS_RECV_MASKEY == m_rcvstat)
    {
        if(m_mask)
        {
            if(getsize(m_rbuff, info, 4))
            {
                memcpy(m_maskey, info.c_str(), 4);
                m_rcvstat = WS_RECV_DATA;
            }
        }
        else
        {
            m_rcvstat = WS_RECV_DATA;
        }
    }
    return TRUE;
}

//接收到的消息体的每个字节是经过Masking-key异或的 接收之后再进行一次异或 还原消息体
VOID CWebSocketRequest::umask(CHAR *data, WORD64 len, CHAR *mask)
{
    for (WORD64 i = 0; i < len; ++i)
    {
        *(data+i) ^= *(mask + (i % 4));
    }
}
//根据Payload len获取消息体 并进行异或还原
BOOL CWebSocketRequest::receive_frame_reqbody()
{
    if(WS_RECV_DATA == m_rcvstat)
    {
        if(getsize(m_rbuff, m_reqbody, m_bodylen))
        {
            umask((char*)m_reqbody.c_str(), m_reqbody.size(), m_maskey);
            m_rcvstat = WS_RECV_BYTE1;
            if(m_fin)
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

//应答一帧数据
BOOL CWebSocketRequest::response_frame(const STRING& data)
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

//接收websocket一帧
BOOL CWebSocketRequest::receive_frame()
{
    if(recvinfo(m_fd, m_rbuff))
    {
        receive_frame_byte1();
        receive_frame_byte2();
        receive_frame_length();
        receive_frame_maskey();
        return receive_frame_reqbody();
    }
    return FALSE;
}

//接收一个websocket请求
BOOL CWebSocketRequest::receive()
{
    //如果已经切换到websocket则按帧结构接收请求 否则继续按http请求接收
    if(m_iswebsocket)
    {
        return receive_frame();
    }
    else
    {
        return CHttpServerRequest::receive();
    }
}

// 应答WebSocket握手请求
BOOL CWebSocketRequest::response_upgrade_websocket()
{
    m_sbuff.append("HTTP/1.1 101 Switching Protocols\r\n");
    m_sbuff.append("Upgrade: websocket\r\n");
    m_sbuff.append("Connection: Upgrade\r\n");
    m_sbuff.append("Sec-WebSocket-Accept: ");

    CHAR sha1str[20] = {0};
    STRING wskey = m_headers["Sec-WebSocket-Key"] + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    sha1(wskey.c_str(), wskey.size(), sha1str);

    m_sbuff.append(base64_encode(STRING(sha1str,sizeof(sha1str))));
    m_sbuff.append("\r\n\r\n");

    return TRUE;
}

//判断是否要切换到 WebSocket 协议
BOOL CWebSocketRequest::is_upgrade_websocket()
{
    STRMAP::iterator itupgr = m_headers.find("Upgrade");
    STRMAP::iterator itconn = m_headers.find("Connection");
    STRMAP::iterator itwsve = m_headers.find("Sec-WebSocket-Version");
    STRMAP::iterator itwske = m_headers.find("Sec-WebSocket-Key");
    if(itupgr != m_headers.end() && itconn != m_headers.end() &&
       itwsve != m_headers.end() && itwske != m_headers.end())
    {
        return TRUE;
    }
    return FALSE;
}

//处理一个websocket请求
BOOL CWebSocketRequest::process()
{
    if(m_iswebsocket)
    {
        show_reqinfo();
        response_frame(m_reqbody);  //直接回显收到的消息
        show_respinfo();
        return TRUE;
    }
    else if(is_upgrade_websocket())
    {
        show_reqinfo();
        response_upgrade_websocket();
        m_iswebsocket = TRUE;
        m_rcvstat = WS_RECV_BYTE1;
        reset_reqinfo();
        show_respinfo();
    }
    else
    {
        CHttpServerRequest::process();
    }
    return TRUE;
}