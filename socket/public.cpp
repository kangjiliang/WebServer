#include "public.h"

//向socket fd上发送数据
BOOL sendinfo(const SOCKFD& fd, STRING& info)
{
    SWORD32 sendlen = 0;

    if(0 == info.size())
    {
        return TRUE;
    }
    //向一个已关闭的socket发送数据时 会触发SIGPIPE信号 该信号默认会退出进程
    //MSG_NOSIGNAL可以禁止send函数向系统发送异常消息
    sendlen = send(fd, info.data(), info.size(), MSG_NOSIGNAL);
    if(0 < sendlen)
    {
        info.erase(0, sendlen);
        return TRUE;
    }
    else
    {
        if(errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
        {
            return TRUE;
        }
        SOCKET_TRACE("sendinfo failed, errno %d: %s\n", errno, strerror(errno));
        return FALSE;
    }
}


/* 从buffer中读取一行 \n或\r\n 并且去掉了\n 或\r\n */
BOOL getline(STRING& buffer, STRING& line)
{
    WORD64 linepos = buffer.find('\n');
    if(string::npos == linepos)
    {
        return FALSE;
    }
    line.clear();
    if(linepos > 0 && buffer[linepos-1] == '\r')
    {
        line.assign(buffer, 0, linepos-1);
    }
    else
    {
        line.assign(buffer, 0, linepos);
    }
    buffer.erase(0, linepos+1);
    return TRUE;
}

/* 从buffer中读取前size个字节 放入str 并从buffer中删除前size个字节 */
BOOL getsize(STRING& buffer, STRING& str, WORD64 size)
{
    str.clear();
    if(buffer.size() < size)
    {
        return FALSE;
    }
    str.assign(buffer, 0 , size);
    buffer.erase(0, size);
    return TRUE;
}

/* 以sep分割str 放入strvec中
   strvec.size() 肯定 >= 1
   如果sep位于str的开始或结尾 则strvec的第一个或最后一个为空字符串 */
VOID strsplit(const STRING& str, STRING sep, STRVEC& strvec)
{
    WORD64 pos1 = 0;
    WORD64 pos2 = 0;

    strvec.clear();
    pos2 = str.find(sep);
    while(string::npos != pos2)
    {
        strvec.push_back(str.substr(pos1, pos2 - pos1));
        pos1 = pos2 + sep.size();
        pos2 = str.find(sep, pos1);
    }
    strvec.push_back(str.substr(pos1));
}

