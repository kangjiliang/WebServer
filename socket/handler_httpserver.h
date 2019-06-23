#ifndef _HANDLER_HTTPSERVER_H
#define _HANDLER_HTTPSERVER_H

#include "handler_tcpserver.h"
#include "receiver_http.h"

const STRING HTTP_RESPONSE_VERSION = "HTTP/1.1 ";
const STRING HTTP_RESPONSE_SERVER  = "httpserver/kangjiliang-117470154@qq.com";

const STRING HTTP_CGISCRIPT_PYTHON = "/usr/bin/python";

class CHttpHandler : public CTcpServerHandler
{
public:
    CHttpHandler(STRING rootdir) : CTcpServerHandler(), m_rootdir(rootdir), m_respstat(), m_respmime("text/html")
    {
        m_receiver = new CHttpReceiver();
    }
    SOCKET_REQUEST_CLONE(CHttpHandler)

    VOID show_reqinfo();
    VOID show_respinfo();
    VOID response(const CHAR* info, WORD64 size);
    VOID response(const STRING& info);
    VOID response_file(const STRING& name, const FILEST& state);
    VOID response_path(const STRING& path);

    virtual VOID process_GET();
    virtual VOID process_POST();
    virtual BOOL process();


    VOID process_cgiscript();
    VOID process_cgiscript_writereqbody(FILEFD fd, const STRING& reqbody);
    VOID process_cgiscript_readresponse(FILEFD fd);
    VOID process_cgiscript_setoneenv(const STRING& key, const WORD64& val);
    VOID process_cgiscript_setoneenv(const STRING& key, const STRING& header);
    VOID process_cgiscript_setallenv();
    VOID process_cgiscript_runcgi(const STRING& cgiscript);

protected:
    STRING m_rootdir;
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
