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

class CHttpServerRequest : public CTcpServerRequest
{
public:
    CHttpServerRequest(STRING rootdir) : CTcpServerRequest(), m_rcvstat(HTTP_RECV_REQLINE), m_method(),
        m_requrl(), m_version(), m_reqfile(), m_query(), m_headers(), m_reqbody(), m_bodylen(0),
        m_rootdir(rootdir), m_respstat(), m_respmime("text/html") {}

    VOID show_reqinfo();
    VOID show_respinfo();
    virtual CSocketRequest* clone();
    virtual BOOL receive();
    virtual BOOL process();

    virtual BOOL receive_reqline();
    virtual BOOL receive_headers();
    virtual BOOL receive_reqbody();

    virtual VOID response(const CHAR* info, WORD64 size);
    virtual VOID response(const STRING& info);
    virtual VOID response_file(const STRING& name, const FILEST& state);
    virtual VOID response_path(const STRING& path);

    virtual VOID process_GET();
    virtual VOID process_POST();
    virtual VOID process_cgiscript();
    virtual VOID process_cgiscript_writereqbody(FILEFD fd, const STRING& reqbody);
    virtual VOID process_cgiscript_readresponse(FILEFD fd);
    virtual VOID process_cgiscript_setoneenv(const STRING& key, const WORD64& val);
    virtual VOID process_cgiscript_setoneenv(const STRING& key, const STRING& header);
    virtual VOID process_cgiscript_setallenv();
    virtual VOID process_cgiscript_runcgi();
protected:
    WORD16 m_rcvstat;  //http请求接收的状态
    STRING m_method;   //
    STRING m_requrl;   //
    STRING m_version;
    STRING m_reqfile;
    STRING m_query;
    STRMAP m_headers;  //http请求信息保存到map结构
    STRING m_reqbody;
    WORD64 m_bodylen;  //body长度

    STRING m_rootdir;
    STRING m_respstat;
    STRING m_respmime;

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
