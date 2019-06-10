#ifndef _CGIBASE_H
#define _CGIBASE_H

#include "public.h"
#include "json.h"

class CCgiBase
{
public:
    CCgiBase();
    virtual ~CCgiBase(){}
    virtual VOID process();
protected:
    VOID get_request();
    VOID get_request_head();
    VOID get_request_body();
    VOID response(const STRING& info);


    Json::Value m_request;
};



#endif
