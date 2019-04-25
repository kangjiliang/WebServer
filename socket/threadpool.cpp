#include "threadpool.h"

//构造函数 初始化互斥变量和条件变量 创建线程池
CThreadPool::CThreadPool(WORD32 num) :m_threadvec(num)
{
    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_condt, NULL);
    for(WORD32 i = 0; i < num; i++)
    {
        pthread_create(&m_threadvec[i], NULL, threadwork, this);
    }
}

//析构函数
CThreadPool::~CThreadPool()
{
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_condt);
    for(WORD32 i = 0; i < m_threadvec.size(); i++)
    {
        pthread_cancel(m_threadvec[i]);
    }
}

//工作线程回调函数
VOID* CThreadPool::threadwork(VOID* args)
{
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);   //设置线程取消 使能
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);  //线程延时退出，执行到下一个取消点才会退出
    CThreadPool* threadpool = (CThreadPool*)args;
    while(NULL != threadpool)
    {
        pthread_mutex_lock(&threadpool->m_mutex);
        while(threadpool->m_taskqueue.empty())
        {
            pthread_cond_wait(&threadpool->m_condt, &threadpool->m_mutex);
        }
        threadpool->m_taskqueue.front().run();
        threadpool->m_taskqueue.pop();
        pthread_testcancel();
    }
    return NULL;
}

//添加一个task到队列中 并
VOID CThreadPool::pushtask(FUNCPTR func, VOID* param)
{
    pthread_mutex_lock(&m_mutex);
    m_taskqueue.push(CTask(func, param));
    pthread_cond_signal(&m_condt);
    pthread_mutex_unlock(&m_mutex);
}




