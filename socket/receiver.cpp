#include "receiver.h"



BOOL CTcpReceiver::receiveonce(const SOCKFD& fd, STRING& info)
{
    CHAR    buff[1] = {0};
    SWORD32 recvlen = 0;
    recvlen = recv(fd, buff, sizeof(buff), 0);

    if(0 < recvlen)  //接收成功
    {
        info.append(buff, recvlen);
        return TRUE;
    }
    else if(0 == recvlen)   //连接关闭
    {
        return FALSE;
    }
    else
    {
        if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return TRUE;
        }
        SOCKET_TRACE("recvinfo failed, errno %d: %s\n", errno, strerror(errno));
        return FALSE;
    }
}
