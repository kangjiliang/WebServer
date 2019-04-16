#include "socket_application.h"


CSocketApplication::~CSocketApplication()
{
    if(NULL != m_reqtype)
    {
        delete m_reqtype;
    }
}

VOID CSocketApplication::startup()
{
    if(NULL != m_reqtype)
    {
        if(m_reqtype->initialize(m_ip, m_port, m_block))
        {
            working();  // working一般都会有个循环在一直执行
        }
    }
    else
    {
        SOCKET_TRACE("CSocketApplication startup failed, NULL == m_reqtype\n");
    }
}