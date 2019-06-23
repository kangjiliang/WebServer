#include "receiver_websocket.h"
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
VOID CWSReceiver::receive_frame_byte1(STRING& rbuff)
{
    if(WS_RECV_BYTE1 == m_rcvstat)
    {
        STRING info;
        if(getsize(rbuff, info, 1))
        {
            m_fin = (info[0] & 0x80) == 0x80;
            m_opcode = info[0] & 0x0F;
            m_rcvstat = WS_RECV_BYTE2;
        }
    }
}

//接收websocket一帧的第二个字节 解析出其中的MASK 和 Payload len
VOID CWSReceiver::receive_frame_byte2(STRING& rbuff)
{
    if(WS_RECV_BYTE2 == m_rcvstat)
    {
        STRING info;
        if(getsize(rbuff, info, 1))
        {
            m_mask     = (info[0] & 0x80) == 0x80;
            m_framelen = info[0] & 0x7F;
            m_rcvstat  = WS_RECV_LENGTH;
        }
    }
}

//如果Payload len是126 则实际消息长度为后面的2个字节
//如果Payload len是127 则实际消息长度为后面的8个字节
VOID CWSReceiver::receive_frame_length(STRING& rbuff)
{
    if(WS_RECV_LENGTH == m_rcvstat)
    {
        STRING info;
        if(126 == m_framelen)
        {
            if(getsize(rbuff, info, 2))
            {
                m_framelen = info[0];
                m_framelen = (m_framelen << 8) | info[1];
                m_rcvstat = WS_RECV_MASKEY;
            }
        }
        else if(127 == m_framelen)
        {
            if(getsize(rbuff, info, 8))
            {
                m_framelen = info[0];
                m_framelen = (m_framelen << 8) | info[1];
                m_framelen = (m_framelen << 8) | info[2];
                m_framelen = (m_framelen << 8) | info[3];
                m_framelen = (m_framelen << 8) | info[4];
                m_framelen = (m_framelen << 8) | info[5];
                m_framelen = (m_framelen << 8) | info[6];
                m_framelen = (m_framelen << 8) | info[7];
                m_rcvstat = WS_RECV_MASKEY;
            }
        }
        else
        {
            m_rcvstat = WS_RECV_MASKEY;
        }
    }
}

//如果MASK为1 则后面4个字节是Masking-key
VOID CWSReceiver::receive_frame_maskey(STRING& rbuff)
{
    STRING info;
    if(WS_RECV_MASKEY == m_rcvstat)
    {
        if(m_mask)
        {
            if(getsize(rbuff, info, 4))
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
}

//接收到的消息体的每个字节是经过Masking-key异或的 接收之后再进行一次异或 还原消息体
VOID CWSReceiver::umask(CHAR *data, WORD64 len, CHAR *mask)
{
    for (WORD64 i = 0; i < len; ++i)
    {
        *(data+i) ^= *(mask + (i % 4));
    }
}

//根据Payload len获取消息体 并进行异或还原
WORD32 CWSReceiver::receive_frame_reqbody(STRING& rbuff, STRMAP& infomap)
{
    if(WS_RECV_DATA == m_rcvstat)
    {
        STRING framebody;
        if(getsize(rbuff, framebody, m_framelen))
        {
            umask((char*)framebody.c_str(), framebody.size(), m_maskey);
            infomap[WS_REQBODY].append(framebody);
            m_rcvstat = WS_RECV_BYTE1;
            if(m_fin)
            {
                return SOCKET_RECEIVE_SUCCESS;
            }
        }
    }
    return SOCKET_RECEIVE_NOTENOUGH;
}


WORD32 CWSReceiver::receive_message(const SOCKFD& fd, STRING& rbuff, STRMAP& infomap)
{
    if(receiveonce(fd, rbuff))
    {
        receive_frame_byte1(rbuff);
        receive_frame_byte2(rbuff);
        receive_frame_length(rbuff);
        receive_frame_maskey(rbuff);
        return receive_frame_reqbody(rbuff, infomap);
    }
    else
    {
        return SOCKET_RECEIVE_ERROR;
    }
}





