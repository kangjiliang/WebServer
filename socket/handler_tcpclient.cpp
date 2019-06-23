#include "handler_tcpclient.h"



BOOL CTcpClientHandler::initialize(const STRING& ip, const WORD16& port, const BOOL& block)
{
    WORD32       opt  = 1;
    SOCKFD       fd   = INVALIDFD;

    fd = socket(AF_INET, SOCK_STREAM, 0);
    if(-1 == fd)
    {
        cout << "tcp server create socket failed, errno:" << errno << endl;
        return FALSE;
    }
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (CHAR*)&opt, sizeof(opt));
    m_saddr.fd = fd;
    m_paddr.fd = fd;
    return TRUE;
}

BOOL CTcpClientHandler::activate(const BOOL& block)
{
    SOCKADDRIN addr = {0};

    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(m_paddr.port);
    addr.sin_addr.s_addr = ("" == m_paddr.ip ? htonl(INADDR_ANY) : inet_addr(m_paddr.ip.c_str()));

    if(-1 == connect(m_paddr.fd, (SOCKADDR*)&addr, sizeof(addr)))
    {
        perror("tcp client connect failed");
        close(m_paddr.fd);
        m_paddr.fd = INVALIDFD;
        m_saddr.fd = INVALIDFD;
        return FALSE;
    }
    else
    {
        cout << "tcp client connect success" << endl;
        return TRUE;
    }
}

