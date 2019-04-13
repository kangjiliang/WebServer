#include "socket_request_httpserver.h"
#include "codec.h"


CSocketRequest* CHttpServerRequest::clone()
{
    return new CHttpServerRequest(*this);
}

VOID CHttpServerRequest::show_reqinfo()
{
    SOCKET_TRACE("********************************************\n");
    SOCKET_TRACE("%s %s %s\n", m_method.c_str(), m_requrl.c_str(), m_version.c_str());
    STRMAP::iterator it = m_headers.begin();
    while(it != m_headers.end())
    {
        SOCKET_TRACE("%s: %s\n", it->first.c_str(), it->second.c_str());
        it++;
    }
    SOCKET_TRACE("%s\n", m_reqbody.c_str());
    SOCKET_TRACE("********************************************\n");
}


/* 接收一次http请求，结构如下
   请求行     reqline  方法 空格 URL 空格 协议版本
   请求头部   headers  每行都是 冒号: 值value
   空行       ""       CRLF
   请求消息体 reqbody */
BOOL CHttpServerRequest::receive()
{
    if(recvinfo(m_paddr.fd, m_rbuff))
    {
        receive_reqline();
        receive_headers();
        return receive_reqbody();
    }
    else
    {
        m_paddr.closefd();
        return FALSE;
    }
}

BOOL CHttpServerRequest::receive_reqline()
{
    if(HTTP_RECV_REQLINE == m_rcvstat)
    {
        STRING line;
        if(getline(m_rbuff, line))
        {
            STRVEC strvec;
            strsplit(line, " ", strvec);
            if(strvec.size() > 2)
            {
                m_method  = strvec[0];
                m_requrl  = url_decode(strvec[1]);
                m_version = strvec[2];
                //URL结构 <scheme>://<user>:<password>@<host>:<port>/<path>;<params>?<query>#<frag>
                STRVEC urlvec;
                STRING urlinfo;
                strsplit(m_requrl, "#", urlvec);
                urlinfo = urlvec[0];
                strsplit(urlinfo, "?", urlvec);
                if(urlvec.size() > 0)
                {
                    m_reqfile = urlvec[0];
                }
                if(urlvec.size() > 1)
                {
                    m_query = urlvec[1];
                }
            }
            m_rcvstat = HTTP_RECV_HEADERS;
        }
    }
    return TRUE;
}

BOOL CHttpServerRequest::receive_headers()
{
    if(HTTP_RECV_HEADERS == m_rcvstat)
    {
        STRING line;
        if(!getline(m_rbuff, line))
        {
            return FALSE;
        }
        //如果读取到一个空行 则消息头接收完毕 开始接收消息体
        if("" == line)
        {
            m_rcvstat = HTTP_RECV_REQBODY;
        }
        else
        {
            //消息头以冒号分割
            WORD64 splitpos = line.find(": ");
            if(string::npos != splitpos)
            {
                STRING key = line.substr(0, splitpos);
                STRING val =  line.substr(splitpos+2);
                m_headers[key] = val;
                if(key == "Content-Length")
                {
                    m_bodylen = atoi(val.c_str());
                }
            }
        }
    }
    return TRUE;
}

BOOL CHttpServerRequest::receive_reqbody()
{
    if(HTTP_RECV_REQBODY == m_rcvstat)
    {
        if(getsize(m_rbuff, m_reqbody, m_bodylen))
        {
            m_rcvstat = HTTP_RECV_REQLINE;
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CHttpServerRequest::process()
{
    show_reqinfo();

    if("GET" == m_method && m_query == "")
    {
        process_GET();
    }
    else if(("GET" == m_method && m_query != "") || ("POST" == m_method))
    {
        //process_POST();
    }
    else
    {
        m_respstat = HTTP_RESPONSE_VERSION + "501 Not Implemented";
        response(m_method + " Not Implement");
    }
    return TRUE;
}

VOID CHttpServerRequest::response(const CHAR* info, WORD64 size)
{
    m_sbuff.append(m_respstat);
    m_sbuff.append("\r\n");

    m_sbuff.append("SERVER: ");
    m_sbuff.append(HTTP_RESPONSE_SERVER);
    m_sbuff.append("\r\n");

    m_sbuff.append("Content-Type: ");
    m_sbuff.append(m_respmime);
    m_sbuff.append("\r\n");

    m_sbuff.append("Content-Length: ");
    SSTREAM sslength;
    sslength << size;
    m_sbuff.append(sslength.str());
    m_sbuff.append("\r\n\r\n");

    m_sbuff.append(info, size);
}

VOID CHttpServerRequest::response(const STRING& info)
{
    response(info.c_str(), info.size());
}

VOID CHttpServerRequest::response_file(const STRING& filename, const FILEST& state)
{
    IFSTREAM filestream;
    SSTREAM  filebuffer;
    filestream.open(filename.c_str(), ios::in | ios::binary);
    filebuffer << filestream.rdbuf();
    m_respstat = HTTP_RESPONSE_VERSION + "200 OK";
    m_respmime = CHttpMIMEType::get(filename);
    response(filebuffer.str());
}

VOID CHttpServerRequest::response_path(const STRING& path)
{
    STRING  files;
    DIR*    dir  = NULL;
    DIRENT* item = NULL;

    m_respstat = HTTP_RESPONSE_VERSION + "200 OK";

    dir = opendir(path.c_str());
    while(NULL != (item = readdir(dir)))
    {
        files += "<a href='";
        files += item->d_name;
        files += "'>";
        files += item->d_name;
        files += "</a></br>";
    }
    closedir(dir);
    response(files);
}

VOID CHttpServerRequest::process_GET()
{
    FILEST state;
    STRING temp = m_rootdir + "/" + m_reqfile;

    if(-1 == stat(temp.c_str(), &state))
    {
        m_respstat = HTTP_RESPONSE_VERSION + "404 Not Found";
        response(temp + " Not Found");
        return;
    }
    if(S_ISDIR(state.st_mode))
    {
        STRING file = temp + "/" + "index.html";
        if(0 == stat(file.c_str(), &state) && !S_ISDIR(state.st_mode))
        {
            return response_file(file, state);
        }
        return response_path(temp);
    }
    else
    {
        return response_file(temp, state);
    }
}



//http相应头中Content-Type 根据文件的扩展名获取
static STRMAP::value_type g_mimetype[] =
{
    STRMAP::value_type(""       , "application/octet-stream"),
    STRMAP::value_type("323"    , "text/h323"),
    STRMAP::value_type("acx"    , "application/internet-property-stream"),
    STRMAP::value_type("ai"     , "application/postscript"),
    STRMAP::value_type("aif"    , "audio/x-aiff"),
    STRMAP::value_type("aifc"   , "audio/x-aiff"),
    STRMAP::value_type("aiff"   , "audio/x-aiff"),
    STRMAP::value_type("asf"    , "video/x-ms-asf"),
    STRMAP::value_type("asr"    , "video/x-ms-asf"),
    STRMAP::value_type("asx"    , "video/x-ms-asf"),
    STRMAP::value_type("au"     , "audio/basic"),
    STRMAP::value_type("avi"    , "video/x-msvideo"),
    STRMAP::value_type("axs"    , "application/olescript"),
    STRMAP::value_type("bas"    , "text/plain"),
    STRMAP::value_type("bcpio"  , "application/x-bcpio"),
    STRMAP::value_type("bin"    , "application/octet-stream"),
    STRMAP::value_type("bmp"    , "image/bmp"),
    STRMAP::value_type("c"      , "text/plain"),
    STRMAP::value_type("cat"    , "application/vnd.ms-pkiseccat"),
    STRMAP::value_type("cdf"    , "application/x-cdf"),
    STRMAP::value_type("cer"    , "application/x-x509-ca-cert"),
    STRMAP::value_type("class"  , "application/octet-stream"),
    STRMAP::value_type("clp"    , "application/x-msclip"),
    STRMAP::value_type("cmx"    , "image/x-cmx"),
    STRMAP::value_type("cod"    , "image/cis-cod"),
    STRMAP::value_type("cpio"   , "application/x-cpio"),
    STRMAP::value_type("crd"    , "application/x-mscardfile"),
    STRMAP::value_type("crl"    , "application/pkix-crl"),
    STRMAP::value_type("crt"    , "application/x-x509-ca-cert"),
    STRMAP::value_type("csh"    , "application/x-csh"),
    STRMAP::value_type("css"    , "text/css"),
    STRMAP::value_type("dcr"    , "application/x-director"),
    STRMAP::value_type("der"    , "application/x-x509-ca-cert"),
    STRMAP::value_type("dir"    , "application/x-director"),
    STRMAP::value_type("dll"    , "application/x-msdownload"),
    STRMAP::value_type("dms"    , "application/octet-stream"),
    STRMAP::value_type("doc"    , "application/msword"),
    STRMAP::value_type("dot"    , "application/msword"),
    STRMAP::value_type("dvi"    , "application/x-dvi"),
    STRMAP::value_type("dxr"    , "application/x-director"),
    STRMAP::value_type("eps"    , "application/postscript"),
    STRMAP::value_type("etx"    , "text/x-setext"),
    STRMAP::value_type("evy"    , "application/envoy"),
    STRMAP::value_type("exe"    , "application/octet-stream"),
    STRMAP::value_type("fif"    , "application/fractals"),
    STRMAP::value_type("flr"    , "x-world/x-vrml"),
    STRMAP::value_type("gif"    , "image/gif"),
    STRMAP::value_type("gtar"   , "application/x-gtar"),
    STRMAP::value_type("gz"     , "application/x-gzip"),
    STRMAP::value_type("h"      , "text/plain"),
    STRMAP::value_type("hdf"    , "application/x-hdf"),
    STRMAP::value_type("hlp"    , "application/winhlp"),
    STRMAP::value_type("hqx"    , "application/mac-binhex40"),
    STRMAP::value_type("hta"    , "application/hta"),
    STRMAP::value_type("htc"    , "text/x-component"),
    STRMAP::value_type("htm"    , "text/html"),
    STRMAP::value_type("html"   , "text/html"),
    STRMAP::value_type("htt"    , "text/webviewhtml"),
    STRMAP::value_type("ico"    , "image/x-icon"),
    STRMAP::value_type("ief"    , "image/ief"),
    STRMAP::value_type("iii"    , "application/x-iphone"),
    STRMAP::value_type("ins"    , "application/x-internet-signup"),
    STRMAP::value_type("isp"    , "application/x-internet-signup"),
    STRMAP::value_type("jfif"   , "image/pipeg"),
    STRMAP::value_type("jpe"    , "image/jpeg"),
    STRMAP::value_type("jpeg"   , "image/jpeg"),
    STRMAP::value_type("jpg"    , "image/jpeg"),
    STRMAP::value_type("js"     , "application/x-javascript"),
    STRMAP::value_type("latex"  , "application/x-latex"),
    STRMAP::value_type("lha"    , "application/octet-stream"),
    STRMAP::value_type("lsf"    , "video/x-la-asf"),
    STRMAP::value_type("lsx"    , "video/x-la-asf"),
    STRMAP::value_type("lzh"    , "application/octet-stream"),
    STRMAP::value_type("m13"    , "application/x-msmediaview"),
    STRMAP::value_type("m14"    , "application/x-msmediaview"),
    STRMAP::value_type("m3u"    , "audio/x-mpegurl"),
    STRMAP::value_type("man"    , "application/x-troff-man"),
    STRMAP::value_type("mdb"    , "application/x-msaccess"),
    STRMAP::value_type("me"     , "application/x-troff-me"),
    STRMAP::value_type("mht"    , "message/rfc822"),
    STRMAP::value_type("mhtml"  , "message/rfc822"),
    STRMAP::value_type("mid"    , "audio/mid"),
    STRMAP::value_type("mny"    , "application/x-msmoney"),
    STRMAP::value_type("mov"    , "video/quicktime"),
    STRMAP::value_type("movie"  , "video/x-sgi-movie"),
    STRMAP::value_type("mp2"    , "video/mpeg"),
    STRMAP::value_type("mp3"    , "audio/mpeg"),
    STRMAP::value_type("mpa"    , "video/mpeg"),
    STRMAP::value_type("mpe"    , "video/mpeg"),
    STRMAP::value_type("mpeg"   , "video/mpeg"),
    STRMAP::value_type("mpg"    , "video/mpeg"),
    STRMAP::value_type("mpp"    , "application/vnd.ms-project"),
    STRMAP::value_type("mpv2"   , "video/mpeg"),
    STRMAP::value_type("ms"     , "application/x-troff-ms"),
    STRMAP::value_type("mvb"    , "application/x-msmediaview"),
    STRMAP::value_type("nws"    , "message/rfc822"),
    STRMAP::value_type("oda"    , "application/oda"),
    STRMAP::value_type("p10"    , "application/pkcs10"),
    STRMAP::value_type("p12"    , "application/x-pkcs12"),
    STRMAP::value_type("p7b"    , "application/x-pkcs7-certificates"),
    STRMAP::value_type("p7c"    , "application/x-pkcs7-mime"),
    STRMAP::value_type("p7m"    , "application/x-pkcs7-mime"),
    STRMAP::value_type("p7r"    , "application/x-pkcs7-certreqresp"),
    STRMAP::value_type("p7s"    , "application/x-pkcs7-signature"),
    STRMAP::value_type("pbm"    , "image/x-portable-bitmap"),
    STRMAP::value_type("pdf"    , "application/pdf"),
    STRMAP::value_type("pfx"    , "application/x-pkcs12"),
    STRMAP::value_type("pgm"    , "image/x-portable-graymap"),
    STRMAP::value_type("pko"    , "application/ynd.ms-pkipko"),
    STRMAP::value_type("pma"    , "application/x-perfmon"),
    STRMAP::value_type("pmc"    , "application/x-perfmon"),
    STRMAP::value_type("pml"    , "application/x-perfmon"),
    STRMAP::value_type("pmr"    , "application/x-perfmon"),
    STRMAP::value_type("pmw"    , "application/x-perfmon"),
    STRMAP::value_type("pnm"    , "image/x-portable-anymap"),
    STRMAP::value_type("pot"    , "application/vnd.ms-powerpoint"),
    STRMAP::value_type("ppm"    , "image/x-portable-pixmap"),
    STRMAP::value_type("pps"    , "application/vnd.ms-powerpoint"),
    STRMAP::value_type("ppt"    , "application/vnd.ms-powerpoint"),
    STRMAP::value_type("prf"    , "application/pics-rules"),
    STRMAP::value_type("ps"     , "application/postscript"),
    STRMAP::value_type("pub"    , "application/x-mspublisher"),
    STRMAP::value_type("qt"     , "video/quicktime"),
    STRMAP::value_type("ra"     , "audio/x-pn-realaudio"),
    STRMAP::value_type("ram"    , "audio/x-pn-realaudio"),
    STRMAP::value_type("ras"    , "image/x-cmu-raster"),
    STRMAP::value_type("rgb"    , "image/x-rgb"),
    STRMAP::value_type("rmi"    , "audio/mid"),
    STRMAP::value_type("roff"   , "application/x-troff"),
    STRMAP::value_type("rtf"    , "application/rtf"),
    STRMAP::value_type("rtx"    , "text/richtext"),
    STRMAP::value_type("scd"    , "application/x-msschedule"),
    STRMAP::value_type("sct"    , "text/scriptlet"),
    STRMAP::value_type("setpay" , "application/set-payment-initiation"),
    STRMAP::value_type("setreg" , "application/set-registration-initiation"),
    STRMAP::value_type("sh"     , "application/x-sh"),
    STRMAP::value_type("shar"   , "application/x-shar"),
    STRMAP::value_type("sit"    , "application/x-stuffit"),
    STRMAP::value_type("snd"    , "audio/basic"),
    STRMAP::value_type("spc"    , "application/x-pkcs7-certificates"),
    STRMAP::value_type("spl"    , "application/futuresplash"),
    STRMAP::value_type("src"    , "application/x-wais-source"),
    STRMAP::value_type("sst"    , "application/vnd.ms-pkicertstore"),
    STRMAP::value_type("stl"    , "application/vnd.ms-pkistl"),
    STRMAP::value_type("stm"    , "text/html"),
    STRMAP::value_type("svg"    , "image/svg+xml"),
    STRMAP::value_type("sv4cpio", "application/x-sv4cpio"),
    STRMAP::value_type("sv4crc" , "application/x-sv4crc"),
    STRMAP::value_type("swf"    , "application/x-shockwave-flash"),
    STRMAP::value_type("t"      , "application/x-troff"),
    STRMAP::value_type("tar"    , "application/x-tar"),
    STRMAP::value_type("tcl"    , "application/x-tcl"),
    STRMAP::value_type("tex"    , "application/x-tex"),
    STRMAP::value_type("texi"   , "application/x-texinfo"),
    STRMAP::value_type("texinfo", "application/x-texinfo"),
    STRMAP::value_type("tgz"    , "application/x-compressed"),
    STRMAP::value_type("tif"    , "image/tiff"),
    STRMAP::value_type("tiff"   , "image/tiff"),
    STRMAP::value_type("tr"     , "application/x-troff"),
    STRMAP::value_type("trm"    , "application/x-msterminal"),
    STRMAP::value_type("tsv"    , "text/tab-separated-values"),
    STRMAP::value_type("txt"    , "text/plain"),
    STRMAP::value_type("uls"    , "text/iuls"),
    STRMAP::value_type("ustar"  , "application/x-ustar"),
    STRMAP::value_type("vcf"    , "text/x-vcard"),
    STRMAP::value_type("vrml"   , "x-world/x-vrml"),
    STRMAP::value_type("wav"    , "audio/x-wav"),
    STRMAP::value_type("wcm"    , "application/vnd.ms-works"),
    STRMAP::value_type("wdb"    , "application/vnd.ms-works"),
    STRMAP::value_type("wks"    , "application/vnd.ms-works"),
    STRMAP::value_type("wmf"    , "application/x-msmetafile"),
    STRMAP::value_type("wps"    , "application/vnd.ms-works"),
    STRMAP::value_type("wri"    , "application/x-mswrite"),
    STRMAP::value_type("wrl"    , "x-world/x-vrml"),
    STRMAP::value_type("wrz"    , "x-world/x-vrml"),
    STRMAP::value_type("xaf"    , "x-world/x-vrml"),
    STRMAP::value_type("xbm"    , "image/x-xbitmap"),
    STRMAP::value_type("xla"    , "application/vnd.ms-excel"),
    STRMAP::value_type("xlc"    , "application/vnd.ms-excel"),
    STRMAP::value_type("xlm"    , "application/vnd.ms-excel"),
    STRMAP::value_type("xls"    , "application/vnd.ms-excel"),
    STRMAP::value_type("xlt"    , "application/vnd.ms-excel"),
    STRMAP::value_type("xlw"    , "application/vnd.ms-excel"),
    STRMAP::value_type("xof"    , "x-world/x-vrml"),
    STRMAP::value_type("xpm"    , "image/x-xpixmap"),
    STRMAP::value_type("xwd"    , "image/x-xwindowdump"),
    STRMAP::value_type("z"      , "application/x-compress"),
    STRMAP::value_type("zip"    , "application/zip"),
};
static const WORD32 g_mimesize = sizeof(g_mimetype)/sizeof(STRMAP::value_type);
STRMAP CHttpMIMEType::m_mimetype(g_mimetype, g_mimetype+g_mimesize);

STRING CHttpMIMEType::get(const STRING& filename)
{
    STRVEC namevec;
    STRING fileext;
    strsplit(filename, ".", namevec);
    if(1 < namevec.size())
    {
        fileext = namevec[namevec.size()-1];
    }

    STRMAP::iterator iter = m_mimetype.find(fileext);
    if(iter == m_mimetype.end())
    {
        return "application/octet-stream";
    }
    return iter->second;
}



/* http相应状态码
"100 Continue"
"101 Switching Protocols"
"200 OK"
"201 Created"
"202 Accepted"
"203 Non-Authoritative Information"
"204 No Content"
"205 Reset Content"
"206 Partial Content"
"300 Multiple Choices"
"301 Moved Permanently"
"302 Found"
"303 See Other"
"304 Not Modified"
"305 Use Proxy"
"307 Temporary Redirect"
"400 Bad Request"
"401 Unauthorized"
"402 Payment Required"
"403 Forbidden"
"404 Not Found"
"405 Method Not Allowed"
"406 Not Acceptable"
"407 Proxy Authentication Required"
"408 Request Timeout"
"409 Conflict"
"410 Gone"
"411 Length Required"
"412 Precondition Failed"
"413 Request Entity Too Large"
"414 Request-URI Too Long"
"415 Unsupported Media Type"
"416 Requested Range Not Satisfiable"
"417 Expectation Failed"
"500 Internal Server Error"
"501 Not Implemented"
"502 Bad Gateway"
"503 Service Unavailable"
"504 Gateway Timeout"
"505 HTTP Version Not Supported"
*/
