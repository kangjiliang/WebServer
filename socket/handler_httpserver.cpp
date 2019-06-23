#include "handler_httpserver.h"
#include "codec.h"

//显示http请求的信息
VOID CHttpHandler::show_reqinfo()
{
    SOCKET_TRACE("********************************************\n");
    STRMAP::iterator it = m_reqinfo.begin();
    while(it != m_reqinfo.end())
    {
        SOCKET_TRACE("%s: %s\n", it->first.c_str(), it->second.c_str());
        it++;
    }
    SOCKET_TRACE("********************************************\n");
}

VOID CHttpHandler::show_respinfo()
{
    SOCKET_TRACE("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    SOCKET_TRACE("%s\n", m_sbuff.c_str());
    SOCKET_TRACE("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
}

//处理一次http请求
BOOL CHttpHandler::process()
{
    show_reqinfo();
    STRING method = m_reqinfo[HTTP_METHOD];
    STRING query  = m_reqinfo[HTTP_QUERY];
    if("GET" == method && query == "")
    {
        process_GET();
    }
    else if(("GET" == method && query != "") || ("POST" == method))
    {
        process_POST();
    }
    else
    {
        m_respstat = HTTP_RESPONSE_VERSION + "501 Not Implemented";
        response(method + " Not Implement");
    }
    m_receiver->reset(m_reqinfo); //重置http请求的信息 准备接收下一次请求
    show_respinfo();
    return TRUE;
}

//响应一次http请求
VOID CHttpHandler::response(const CHAR* info, WORD64 size)
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

VOID CHttpHandler::response(const STRING& info)
{
    response(info.c_str(), info.size());
}

//处理GET方法 url是一个文件时 读取文件内容 发送给客户端
VOID CHttpHandler::response_file(const STRING& filename, const FILEST& state)
{
    IFSTREAM filestream;
    SSTREAM  filebuffer;
    filestream.open(filename.c_str(), ios::in | ios::binary);
    filebuffer << filestream.rdbuf();
    m_respstat = HTTP_RESPONSE_VERSION + "200 OK";
    m_respmime = CHttpMIMEType::get(filename);
    response(filebuffer.str());
}

//处理GET方法 url是一个目录时 获取目录下的文件列表 发送给客户端
VOID CHttpHandler::response_path(const STRING& path)
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

//处理GET方法
//如果url是文件 则读取文件内容 发送给客户端
//如果url是目录 则查找有没有index.html文件 有则发送文件内容 没有则发送文件列表
VOID CHttpHandler::process_GET()
{
    FILEST state;
    STRING reqfile = m_rootdir + "/" + m_reqinfo[HTTP_REQFILE];
    if(-1 == stat(reqfile.c_str(), &state))
    {
        m_respstat = HTTP_RESPONSE_VERSION + "404 Not Found";
        response(reqfile + " Not Found");
        return;
    }
    if(S_ISDIR(state.st_mode))
    {
        STRING file = reqfile + "/" + "index.html";
        if(0 == stat(file.c_str(), &state) && !S_ISDIR(state.st_mode))
        {
            return response_file(file, state);
        }
        return response_path(reqfile);
    }
    else
    {
        return response_file(reqfile, state);
    }
}

//处理POST方法 暂仅支持使用cgi脚本处理
VOID CHttpHandler::process_POST()
{
    process_cgiscript();
}

//处理cgi脚本 创建子进程执行脚本 读取脚本的输出 应答给客户端
VOID CHttpHandler::process_cgiscript()
{
    FILEFD input[2]  = {0};  //子进程的读通道
    FILEFD output[2] = {0};  //子进程的写通道
    PID    pid;
    STRING cgiscript = m_rootdir + "/" + m_reqinfo[HTTP_REQFILE];
    //创建管道和子进程
    if(pipe(input) < 0 || pipe(output) < 0 || (pid = fork()) < 0)
    {
        SOCKET_TRACE("pipe or fork failed, errno %d: %s\n", errno, strerror(errno));
        //close pipe fd
        return ;
    }
    //子进程执行cgi脚本
    if(0 == pid)
    {
        dup2(input[FDREAD], STDIN);
        dup2(output[FDWRITE], STDOUT);
        close(input[FDWRITE]);    //关闭子进程 读通道 的写方向
        close(output[FDREAD]);    //关闭子进程 写通道 的读方向
        process_cgiscript_setallenv();
        process_cgiscript_runcgi(cgiscript);
        exit(0);
    }
    //父进程读取cgi的输出 应答客户端
    else
    {
        PSTAT st;
        close(input[FDREAD]);   //父进程 在子进程的读通道上 可写
        close(output[FDWRITE]); //父进城 在子进程的写通道上 可读
        process_cgiscript_writereqbody(input[FDWRITE], m_reqinfo[HTTP_REQBODY]);
        process_cgiscript_readresponse(output[FDREAD]);
        close(input[FDWRITE]);
        close(output[FDREAD]);
        waitpid(pid, &st, 0);
    }
}

//需要循环写 待实现
VOID CHttpHandler::process_cgiscript_writereqbody(FILEFD fd, const STRING& reqbody)
{
    write(fd, reqbody.c_str(), reqbody.size());
}

VOID CHttpHandler::process_cgiscript_readresponse(FILEFD fd)
{
    CHAR  ch;
    while(0 < read(fd, &ch, 1))
    {
        m_sbuff.append(1, ch);
    }
}


VOID CHttpHandler::process_cgiscript_setoneenv(const STRING& key, const WORD64& val)
{
    SSTREAM valstr;
    valstr << val;
    setenv(key.c_str(), valstr.str().c_str(), 1);
}
VOID CHttpHandler::process_cgiscript_setoneenv(const STRING& key, const STRING& val)
{
    setenv(key.c_str(), val.c_str(), 1);
}


VOID CHttpHandler::process_cgiscript_setallenv()
{
    /*
    cgiscript环境变量名称        说明
    REQUEST_METHOD        请求类型，如“GET”或“POST”
    CONTENT_TYPE          被发送数据的类型
    CONTENT_LENGTH        客户端向标准输入设备发送的数据长度，单位为字节
    QUERY_STRING          查询参数，如“id=10010&sn=liigo”
    SCRIPT_NAME           cgiscript脚本程序名称
    PATH_INFO             cgiscript脚本程序附加路径
    PATH_TRANSLATED       PATH_INFO对应的绝对路径
    REMOTE_ADDR           发送此次请求的主机IP
    REMOTE_HOST           发送此次请求的主机名
    REMOTE_USER           已被验证合法的用户名
    REMOTE_IDENT          WEB服务器的登录用户名
    AUTH_TYPE             验证类型
    GATEWAY_INTERFACE     服务器遵守的cgiscript版本，如：cgiscript/1.1
    SERVER_NAME           服务器主机名、域名或IP
    SERVER_PORT           服务器端口号
    SERVER_PROTOCOL       服务器协议，如：HTTP/1.1
    DOCUMENT_ROOT         文档根目录
    SERVER_SOFTWARE       服务器软件的描述文本
    HTTP_ACCEPT           客户端可以接收的MIME类型，以逗号分隔
    HTTP_USER_AGENT       发送此次请求的web浏览器
    HTTP_REFERER          调用此脚本程序的文档
    HTTP_COOKIE           获取COOKIE键值对，多项之间以分号分隔，如：key1=value1;key2=value2
    */
    process_cgiscript_setoneenv("REQUEST_METHOD",   m_reqinfo[HTTP_METHOD]);
    process_cgiscript_setoneenv("CONTENT_TYPE",     m_reqinfo["Content-Type"]);
    process_cgiscript_setoneenv("CONTENT_LENGTH",   m_reqinfo["Content-Length"]);
    process_cgiscript_setoneenv("QUERY_STRING",     m_reqinfo[HTTP_QUERY]);
    //process_cgiscript_setoneenv("SCRIPT_NAME",      m_reqfile);
    //process_cgiscript_setoneenv("REMOTE_ADDR",      peerip());
    //process_cgiscript_setoneenv("SERVER_NAME",      selfaddr());
    //process_cgiscript_setoneenv("SERVER_PORT",      selfport());
    //process_cgiscript_setoneenv("SERVER_PROTOCOL",  HTTP_RESPONSE_VERSION);
    //process_cgiscript_setoneenv("DOCUMENT_ROOT",    m_rootdir);
    //process_cgiscript_setoneenv("SERVER_SOFTWARE",  HTTP_RESPONSE_SERVER);
    //process_cgiscript_setoneenv("HTTP_ACCEPT",      m_headers["Accept"]);
    //process_cgiscript_setoneenv("HTTP_USER_AGENT",  m_headers["User-Agent"]);
    //process_cgiscript_setoneenv("HTTP_REFERER",     m_headers["Referer"]);
    //process_cgiscript_setoneenv("HTTP_COOKIE",      m_headers["Cookie"]);
}


VOID CHttpHandler::process_cgiscript_runcgi(const STRING& cgiscript)
{
    if(0 != access(cgiscript.c_str(), X_OK))
    {
        SOCKET_TRACE("cgiscript is not executable, name:%s\n", cgiscript.c_str());
        return;
    }
    STRVEC strvec;
    strsplit(cgiscript, ".", strvec);
    if(1 < strvec.size() && "py" == strvec[strvec.size() - 1])
    {
        execl(HTTP_CGISCRIPT_PYTHON.c_str(),
              HTTP_CGISCRIPT_PYTHON.c_str(),
              cgiscript.c_str(),
              NULL);
    }
    else
    {
        execl(cgiscript.c_str(),
              cgiscript.c_str(),
              NULL);
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
