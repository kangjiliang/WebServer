#include "cgibase.h"


CCgiBase::CCgiBase()
{
    get_request();
}


VOID CCgiBase::get_request()
{
    get_request_head();
    get_request_body();
}

VOID CCgiBase::get_request_head()
{
    CHAR* envstr = getenv("CONTENT_LENGTH");
    m_request["CONTENT_LENGTH"] = Json::Value(envstr);
}

VOID CCgiBase::get_request_body()
{
    STRING reqbody;
    Json::Reader reader;
    SSTREAM lenstream;
    WORD32 contentlen = 0;
    lenstream << getenv("CONTENT_LENGTH");
    lenstream >> contentlen;

    while(contentlen != reqbody.size())
    {
        reqbody.append(1, cin.get());
    }
    if (!reader.parse(reqbody, m_request))
    {
        response("parse json string failed");
    }
}

VOID CCgiBase::response(const STRING& info)
{
    cout << "HTTP/1.1 200 OK\r\n";
    cout << "Content-Type: text/html\r\n";
    cout << "Content-Length: " << info.size() << "\r\n";
    cout << "\r\n";
    cout << info;
}

VOID CCgiBase::process()
{
    response(m_request.toStyledString());
}
