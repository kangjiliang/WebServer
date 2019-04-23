#ifndef _SOCKET_REQUEST_HTTP_H
#define _SOCKET_REQUEST_HTTP_H

#include "socket_request_tcpserver.h"


//接收一个http请求的状态
enum HTTP_RECV_STATUS
{
    HTTP_RECV_REQLINE,
    HTTP_RECV_HEADERS,
    HTTP_RECV_REQBODY
};

const STRING HTTP_RESPONSE_VERSION = "HTTP/1.1 ";
const STRING HTTP_RESPONSE_SERVER  = "httpserver/kangjiliang-117470154@qq.com";

const CHAR* HTTP_CGISCRIPT_PYTHON = "/usr/bin/python";

class CHttpServerRequest : public CTcpServerRequest
{
public:
    CHttpServerRequest(STRING rootdir) : CTcpServerRequest(), m_rootdir(rootdir), m_rcvstat(HTTP_RECV_REQLINE), m_method(),
        m_requrl(), m_version(), m_reqfile(), m_query(), m_headers(), m_reqbody(), m_bodylen(0),
        m_respstat(), m_respmime("text/html") {}

    virtual CSocketRequest* clone();
    virtual BOOL receive();
    virtual BOOL process();

    VOID show_reqinfo();
    VOID show_respinfo();
    VOID reset_reqinfo();

    BOOL receive_reqline();
    BOOL receive_headers();
    BOOL receive_reqbody();

    VOID response(const CHAR* info, WORD64 size);
    VOID response(const STRING& info);
    VOID response_file(const STRING& name, const FILEST& state);
    VOID response_path(const STRING& path);

    VOID process_GET();
    VOID process_POST();
    VOID process_cgiscript();
    VOID process_cgiscript_writereqbody(FILEFD fd, const STRING& reqbody);
    VOID process_cgiscript_readresponse(FILEFD fd);
    VOID process_cgiscript_setoneenv(const STRING& key, const WORD64& val);
    VOID process_cgiscript_setoneenv(const STRING& key, const STRING& header);
    VOID process_cgiscript_setallenv();
    VOID process_cgiscript_runcgi();

protected:
    STRING m_rootdir;

    WORD16 m_rcvstat;  //http请求接收的状态
    STRING m_method;   //方法
    STRING m_requrl;   //url
    STRING m_version;  //http版本
    STRING m_reqfile;  //请求的文件名
    STRING m_query;    //在url中的query信息
    STRMAP m_headers;  //http请求信息保存到map结构
    STRING m_reqbody;  //http请求的消息体
    WORD64 m_bodylen;  //body长度

    STRING m_respstat; //http响应状态行
    STRING m_respmime; //content type

private:
};

class CHttpMIMEType
{
public:
    static STRING get(const STRING& filename);
private:
    static STRMAP m_mimetype;
};


#endif
