#ifndef _SOCKET_REQUEST_H
#define _SOCKET_REQUEST_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <assert.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <string>
#include <vector>
#include <list>
#include <map>

using namespace std;


/* 重定义一些数据类型 */
/*------------------------------------------------*/
typedef void                      VOID;
typedef char                      CHAR;
typedef uint8_t                   BYTE;
typedef uint8_t                   BOOL;
typedef uint16_t                  WORD16;
typedef uint32_t                  WORD32;
typedef uint64_t                  WORD64;
typedef int32_t                   SWORD32;
typedef stringstream              SSTREAM;
typedef std::string               STRING;
typedef std::vector<string>       STRVEC;
typedef std::map<string, string>  STRMAP;
/*------------------------------------------------*/
typedef int                       SOCKFD;
typedef struct sockaddr_in        SOCKADDRIN;
typedef struct sockaddr           SOCKADDR;
typedef struct epoll_event        EPEVENT;
/*------------------------------------------------*/
typedef ofstream                  OFSTREAM;
typedef ifstream                  IFSTREAM;
typedef int                       FILEFD;
typedef struct stat               FILEST;
typedef struct dirent             DIRENT;
/*------------------------------------------------*/
typedef pthread_t                 THREAD;
typedef pthread_mutex_t           TMUTEX;
typedef pthread_cond_t            TCONDT;
/*------------------------------------------------*/
typedef pid_t                     PID;
typedef int                       PSTAT;
/*------------------------------------------------*/
typedef time_t                    TIMET;
typedef tm                        TIMETM;
typedef struct timeval            TIMEVAL;
/*------------------------------------------------*/
#define TRUE                      (1)
#define FALSE                     (0)
#define FDREAD                    0
#define FDWRITE                   1
#define STDIN                     0
#define STDOUT                    1
#define INVALIDFD                 ((SOCKFD)-1)
/*------------------------------------------------*/
#define SOCKET_TRACE              printf
/*------------------------------------------------*/


// 定义socket地址类 封装socket ip port
class CSocketAddress
{
public:
    CSocketAddress() : fd(INVALIDFD), ip(), port(0) {}
    STRING addr() const;
    VOID   noblock();
    VOID   setaddr(const SOCKFD& fd, const STRING& ip, const WORD16& port, const BOOL& block);
    VOID   closefd();
    SOCKFD fd;    // socket
    STRING ip;    // ip地址
    WORD16 port;  // 端口号
};


// 定义socket请求基类
class CSocketRequest
{
public:
    CSocketRequest():m_rbuff(), m_sbuff(), m_saddr(), m_paddr(){}
    virtual ~CSocketRequest(){}
    SOCKFD selffd() {return m_saddr.fd;}
    SOCKFD peerfd() {return m_paddr.fd;}
    BOOL   sbuffempty() {return m_sbuff.empty();}


    virtual CSocketRequest* clone() = 0;
    virtual BOOL  initialize(const STRING& ip, const WORD16& port, const BOOL& block) = 0;
    virtual BOOL  activate(const BOOL& block) = 0;
    virtual BOOL  receive() = 0;
    virtual BOOL  dispatch() = 0;
    virtual BOOL  process() = 0;
protected:
    STRING         m_rbuff;  //接收数据的buffer
    STRING         m_sbuff;  //发送数据的buffer
    CSocketAddress m_saddr;  //本端地址
    CSocketAddress m_paddr;  //对端地址
};


BOOL recvinfo(const SOCKFD& fd, STRING& info);
BOOL sendinfo(const SOCKFD& fd, STRING& info);


STRING getgmttime(TIMET timestamp);
BOOL   getline(STRING& buffer, STRING& line);
BOOL   getsize(STRING& buffer, STRING& str, WORD64 size);
VOID   strsplit(const STRING& str, STRING sep, STRVEC& strvec);



#endif
