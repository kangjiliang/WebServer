#include "socket_request_tcpserver.h"


CSocketRequest* CTcpServerRequest::clone()
{
    return new CTcpServerRequest(*this);
}

// Tcpserver初始化 封装socket bind listen
BOOL CTcpServerRequest::initialize(const STRING& ip, const WORD16& port, const BOOL& block)
{
    WORD32      opt  = 1;
    SOCKADDRIN  addr = {0};
    SOCKFD      fd   = INVALIDFD;
    //创建socket
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd)
    {
        perror("socket failed");
        return FALSE;
    }
    //设置地址重用
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (CHAR*)&opt, sizeof(opt));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = ("" == ip ? htonl(INADDR_ANY) : inet_addr(ip.c_str()));
    //绑定ip地址和端口
    if(-1 == bind(fd, (SOCKADDR*)&addr, sizeof(addr)))
    {
        SOCKET_TRACE("bind failed, errno %d: %s\n", errno, strerror(errno));
        close(fd);
        return FALSE;
    }
    //开始监听
    if(-1 == listen(fd, SOCKET_TCPSERVER_LISTEN_BACKLOG))
    {
        SOCKET_TRACE("listen failed, errno %d: %s\n", errno, strerror(errno));
        close(fd);
        return FALSE;
    }
    m_sockaddr.setaddr(fd, ip, port, block);
    return TRUE;
}

// Tcpserver激活 也就是accept等待客户端连接
BOOL CTcpServerRequest::activate(const BOOL& block)
{
    SOCKFD     sockfd = -1;
    SOCKADDRIN addrin = {0};
    WORD32     addlen = sizeof(addrin);
    SOCKFD     servfd = m_sockaddr.fd;

    if(INVALIDFD == servfd)
    {
        SOCKET_TRACE("activate failed, servfd invalid\n");
        return FALSE;
    }
    sockfd = accept(servfd, (SOCKADDR*)&addrin, &addlen);
    if(-1 == sockfd)
    {
        SOCKET_TRACE("accept failed, errno %d: %s\n", errno, strerror(errno));
        return FALSE;
    }
    else
    {
        m_sockaddr.setaddr(sockfd, inet_ntoa(addrin.sin_addr), ntohs(addrin.sin_port), block);
        SOCKET_TRACE("accept from: %s\n", m_sockaddr.peeraddr().c_str());
        return TRUE;
    }
}

// Tcpserver接收数据 读取到一行为成功 成功后转process去处理
BOOL CTcpServerRequest::receive()
{
    if(recvinfo(m_sockaddr.fd, m_sockaddr.rbuff))
    {
        STRING line;
        if(getline(m_sockaddr.rbuff, line))
        {
            cout << "recv: " << line << endl;
        }
        return FALSE;
    }
    else
    {
        m_sockaddr.closefd();
        return FALSE;
    }
}
// Tcpserver应答数据处理
BOOL CTcpServerRequest::dispatch()
{
    if(!m_sockaddr.sbuff.empty())
    {
        sendinfo(m_sockaddr.fd, m_sockaddr.sbuff);
    }
    return TRUE;
}

// Tcpserver处理接收到的数据 打印接收到的一行
BOOL CTcpServerRequest::process()
{
    return TRUE;
}



