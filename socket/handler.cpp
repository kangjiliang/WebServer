#include "handler.h"


STRING CSocketHandler::selfip() const
{
    SOCKADDRIN addr;
    WORD32     size = sizeof(addr);
    getsockname(m_fd, (SOCKADDR*)&addr, &size);
    return STRING(inet_ntoa(addr.sin_addr));
}

STRING CSocketHandler::peerip() const
{
    SOCKADDRIN addr;
    WORD32     size = sizeof(addr);
    getpeername(m_fd, (SOCKADDR*)&addr, &size);
    return STRING(inet_ntoa(addr.sin_addr));
}

STRING CSocketHandler::selfport() const
{
    SOCKADDRIN addr;
    WORD32     size = sizeof(addr);
    getsockname(m_fd, (SOCKADDR*)&addr, &size);
    SSTREAM ssaddr;
    ssaddr << ntohs(addr.sin_port);
    return ssaddr.str();
}

//返回 ip地址:端口号:socket 格式的字符串
STRING CSocketHandler::selfaddr() const
{
    SOCKADDRIN addr;
    WORD32     size = sizeof(addr);
    getsockname(m_fd, (SOCKADDR*)&addr, &size);
    SSTREAM ssaddr;
    ssaddr << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << ":" << m_fd;
    return ssaddr.str();
}

STRING CSocketHandler::peeraddr() const
{
    SOCKADDRIN addr;
    WORD32     size = sizeof(addr);
    getpeername(m_fd, (SOCKADDR*)&addr, &size);
    SSTREAM ssaddr;
    ssaddr << inet_ntoa(addr.sin_addr) << ":" << ntohs(addr.sin_port) << ":" << m_fd;
    return ssaddr.str();
}

//关闭socket
VOID CSocketHandler::closefd()
{
    if(INVALIDFD != m_fd)
    {
        SOCKET_TRACE("disconnect: %s\n", peeraddr().c_str());
        close(m_fd);
        m_fd = INVALIDFD;
    }
}

BOOL CSocketHandler::receive()
{
    if(NULL == m_receiver)
    {
        SOCKET_TRACE("receive failed, NULL == m_receiver\n");
        return FALSE;
    }
    WORD32 ret = m_receiver->receive_message(m_fd, m_rbuff, m_reqinfo);
    if(SOCKET_RECEIVE_ERROR == ret)
    {
        closefd();
        return FALSE;
    }
    return ((SOCKET_RECEIVE_SUCCESS == ret) ? TRUE : FALSE);
}



BOOL CTcpHandler::dispatch()
{
    if(!m_sbuff.empty())
    {
        sendinfo(m_fd, m_sbuff);
    }
    return TRUE;
}


