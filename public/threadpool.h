#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "public.h"


class CTask
{
public:
    CTask(FUNCPTR func, VOID* param) : m_func(func), m_param(param){}
    VOID run() const {m_func(m_param);}
protected:
private:
    FUNCPTR m_func;
    VOID*   m_param;
};

typedef queue<CTask>   TASKQUEUE;   //任务队列
typedef vector<THREAD> THREADVEC;   //线程ID的数组


class CThreadPool
{
public:
    CThreadPool(WORD32 num);
    ~CThreadPool();
    static VOID* threadwork(VOID* args);
    VOID pushtask(FUNCPTR func, VOID* param);
protected:
private:
    TMUTEX    m_mutex;
    TCONDT    m_condt;
    THREADVEC m_threadvec;
    TASKQUEUE m_taskqueue;
};

#endif

