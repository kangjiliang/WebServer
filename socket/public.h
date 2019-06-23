#ifndef _SOCKET_PUBLIC_H
#define _SOCKET_PUBLIC_H


#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/ipc.h>
#include <sys/msg.h>
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
#include <queue>
#include <list>
#include <map>
#if __cplusplus >= 201103L
#include <unordered_map>
#endif


using namespace std;



/* 重定义一些数据类型 */
/*------------------------------------------------*/
typedef void                           VOID;
typedef char                           CHAR;
typedef uint8_t                        BYTE;
typedef uint8_t                        BOOL;
typedef uint16_t                       WORD16;
typedef uint32_t                       WORD32;
typedef uint64_t                       WORD64;
typedef int32_t                        SWORD32;
typedef stringstream                   SSTREAM;
typedef std::string                    STRING;
typedef std::vector<string>            STRVEC;
#if __cplusplus >= 201103L
typedef unordered_map<string, string>  STRMAP;
#else
typedef std::map<string, string>       STRMAP;
#endif
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
typedef key_t                     MQKEY;
typedef long                      MQMSGTYPE;
/*------------------------------------------------*/
typedef VOID* (*FUNCPTR)(VOID* param);
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


BOOL sendinfo(const SOCKFD& fd, STRING& info);


STRING getgmttime(TIMET timestamp);
BOOL   getline(STRING& buffer, STRING& line);
BOOL   getsize(STRING& buffer, STRING& str, WORD64 size);
VOID   strsplit(const STRING& str, STRING sep, STRVEC& strvec);


#endif
