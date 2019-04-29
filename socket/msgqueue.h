#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "public.h"

const STRING MQ_FTOK_FNAME = ".";

typedef struct MQ_MESSAGE_INFO
{
    MQMSGTYPE msgtype;
    FUNCPTR   func;
    VOID*     param;
}MQMSG;


class CMsgQueue
{
public:
    CMsgQueue()
    {

    }
    ~CMsgQueue()
    {
        if(0 > msgctl(m_msgid, IPC_RMID, NULL))
        {
            perror("msgctl rmid");
        }
    }
    BOOL init(SWORD32 keyid)
    {
        m_keyid = keyid;
        m_mqkey = ftok(MQ_FTOK_FNAME.c_str(), m_keyid);
        if(0 > m_mqkey)
        {
            perror("ftok error");
            return FALSE;
        }
        m_msgid = msgget(m_mqkey, IPC_CREAT | IPC_EXCL | 0666);
        if(0 > m_msgid)
        {
            perror("msgget error");
            return FALSE;
        }
    }
    BOOL msgpush(const MQMSG& msg)
    {
        if(-1 == msgsnd(m_msgid, &msg, sizeof(msg), 0))
        {
            perror("msgsnd");
            return FALSE;
        }
        return TRUE;
    }
    BOOL msgpop(MQMSG& msg)
    {
        if(-1 == msgrcv(m_msgid, &msg, sizeof(msg), msg.msgtype, 0))
        {
            perror("msgsnd");
            return FALSE;
        }
        return TRUE;
    }

protected:
private:
    SWORD32 m_keyid;
    MQKEY   m_mqkey;
    SWORD32 m_msgid;
};