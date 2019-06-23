#ifndef _RECEIVER_WEBSOCKET_H
#define _RECEIVER_WEBSOCKET_H

#include "receiver.h"


//websocket接收的状态
enum WEBSOCKET_RECV_STATUS
{
    WS_RECV_BYTE1,
    WS_RECV_BYTE2,
    WS_RECV_LENGTH,
    WS_RECV_MASKEY,
    WS_RECV_DATA,
};

const STRING WS_REQBODY = "wsreqbody";

class CWSReceiver: public CTcpReceiver
{
public:
    CWSReceiver() : CTcpReceiver(){}
    SOCKET_RECEIVER_CLONE(CWSReceiver);
    virtual WORD32 receive_message(const SOCKFD& fd, STRING& rbuff, STRMAP& infomap);
protected:
    VOID   receive_frame();         //接收websocke一帧
    VOID   receive_frame_byte1(STRING& rbuff);   //接收第一个字节
    VOID   receive_frame_byte2(STRING& rbuff);   //接收第二个字节
    VOID   receive_frame_length(STRING& rbuff);  //接收消息长度
    VOID   receive_frame_maskey(STRING& rbuff);  //接收掩码
    WORD32 receive_frame_reqbody(STRING& rbuff, STRMAP& infomap); //接收消息体
    VOID   umask(CHAR *data, WORD64 len, CHAR *mask); //解掩码
private:
    CHAR   m_fin;
    CHAR   m_opcode;
    CHAR   m_mask;
    CHAR   m_maskey[4];
    WORD64 m_framelen;
};

#endif

