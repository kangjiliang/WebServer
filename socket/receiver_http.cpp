#include "receiver_http.h"
#include "codec.h"


WORD32 CHttpReceiver::receive_message(const SOCKFD& fd, STRING& rbuff, STRMAP& infomap)
{
    if(receiveonce(fd, rbuff))
    {
        receive_reqline(rbuff, infomap);
        receive_headers(rbuff, infomap);
        return receive_reqbody(rbuff, infomap);
    }
    else
    {
        return SOCKET_RECEIVE_ERROR;
    }
}

VOID CHttpReceiver::receive_reqline(STRING& rbuff, STRMAP& infomap)
{
    if(HTTP_RECV_REQLINE == m_rcvstat)
    {
        STRING line;
        if(getline(rbuff, line))
        {
            STRVEC strvec;
            strsplit(line, " ", strvec);
            if(strvec.size() > 2)
            {
                infomap[HTTP_METHOD]  = strvec[0];
                infomap[HTTP_REQURL]  = url_decode(strvec[1]); //对url进行解码
                infomap[HTTP_VERSION]  = strvec[2];
                //URI结构 <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
                //[path][?query][#fragment]
                STRVEC urlvec;
                STRING urlinfo;
                strsplit(infomap[HTTP_REQURL], "?", urlvec);
                if(urlvec.size() > 0)
                {
                    infomap[HTTP_REQFILE] = urlvec[0];
                }
                if(urlvec.size() > 1)
                {
                    STRVEC queryvec;
                    strsplit(urlvec[1], "#", queryvec);
                    infomap[HTTP_QUERY] = queryvec[0];
                }
            }
            m_rcvstat = HTTP_RECV_HEADERS;
        }
    }
}

//接收http请求的头部 每一行结构为: key: value
VOID CHttpReceiver::receive_headers(STRING& rbuff, STRMAP& infomap)
{
    if(HTTP_RECV_HEADERS == m_rcvstat)
    {
        STRING line;
        if(!getline(rbuff, line))
        {
            return;
        }
        //如果读取到一个空行 则消息头接收完毕 开始接收消息体
        if("" == line)
        {
            m_rcvstat = HTTP_RECV_REQBODY;
        }
        else
        {
            STRVEC strvec;
            strsplit(line, ": ", strvec);
            if(strvec.size() > 1)
            {
                infomap[strvec[0]] = strvec[1];
                if(strvec[0] == "Content-Length")
                {
                    m_bodylen = atoi(strvec[1].c_str());
                }
            }
        }
    }
}

//接收http请求的消息体 长度由Content-Length确定
BOOL CHttpReceiver::receive_reqbody(STRING& rbuff, STRMAP& infomap)
{
    if(HTTP_RECV_REQBODY == m_rcvstat)
    {
        if(getsize(rbuff, infomap[HTTP_REQBODY], m_bodylen))
        {
            return SOCKET_RECEIVE_SUCCESS;
        }
    }
    return SOCKET_RECEIVE_NOTENOUGH;
}



