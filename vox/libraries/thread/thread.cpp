#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "util.hpp"
#include "exception.hpp"
#include "thread.hpp"

namespace vox
{

Thread::Thread()
{
    m_ptid = (pthread_t)0;
    m_detach = false;
    m_started = false;
    m_retptr = NULL;
    m_name = "";
}

Thread::~Thread()
{
}

bool Thread::start(bool detach, int stacksize)
{
    pthread_attr_t attr;

    TRY_EXCEPTIONS;

    lock();

    if(m_started)
    {
        unlock();
        THROW_ERRORS(EINVAL, "thread is already started. started(%d) m_ptid(%lu)",
            m_started, m_ptid);
    }

    pthread_attr_init(&attr);

    if(stacksize > 1024 * 1024)
    {
        pthread_attr_setstacksize(&attr, stacksize);
    }

    if(pthread_create(&m_ptid, &attr, threadFunc, this) != 0)
    {
        pthread_attr_destroy(&attr);

        unlock();
        THROW_ERRORS(errno, "pthread_create() failed. ptid(%lu)", m_ptid);
    }

    pthread_attr_destroy(&attr);

    m_started = true;
    m_detach = detach;

    if(m_detach)
    {
        if(pthread_detach(m_ptid) != 0)
        {
            unlock();
            THROW_ERRORS(errno, "pthread_detach() failed. name(%s) ptid(%lu) detach(%u)",
                m_name.c_str(), m_ptid, m_detach);
        }
    }

    unlock();

    CATCH_EXCEPTIONS(false);

    return true;
}

void Thread::stop(bool force)
{
    bool started;
    timespec tscur, tsprev;

    TRY_EXCEPTIONS;

    clock_gettime(CLOCK_REALTIME, &tsprev);

    while(true)
    {
        clock_gettime(CLOCK_REALTIME, &tscur);

        lock();
        started = m_started;
        unlock();

        if(!started) break;

        if(timeSpan(tsprev, tscur) > (float)THREAD_KILLTO)
        {
            //if(!m_detach)
            {
                if(force)
                {
                    THROW_EXCEPTIONS("thread force terminate. name(%s) ptid(%lu) detach(%u)",
                        m_name.c_str(), m_ptid, m_detach);
                    terminate();
                }
            }
        }

        waitFor(0.05);
    }

    CATCH_EXCEPTIONS_BEGIN;

    join();

    CATCH_EXCEPTIONS_END();
}

void* Thread::threadFunc(void* pArg)
{
    Thread* pSelf = (Thread *)pArg;

    // lock을 획득하는 시점은 start() 에서 부가적인 작업이 끝난 직후이다.
    pSelf->lock();
    pSelf->unlock();

    pSelf->run();

    pSelf->lock();

    // detach일 경우 바로 종료
    if(pSelf->m_detach)
        pSelf->m_started = false;

    pSelf->resetEventNoLock();

    pSelf->unlock();

    return NULL;
}

void Thread::kill(int signo)
{
    if(!isRunning())
        return;
    
    pthread_kill(m_ptid, signo);
}

void Thread::yield()
{
    pthread_yield();
}

bool Thread::isRunning()
{
    lock();
    bool ret = m_started;
    unlock();

    return ret;
}

void Thread::join()
{
    bool started;

    lock();
    started = m_started;
    unlock();

    if(!started)
        return;
    
    if(m_detach)
    {
        lock();
        m_started = false;
        unlock();
        return;
    }

#ifndef NDEBUG
    int ret =
#endif
    pthread_join(m_ptid, &m_retptr);
    assert(ret == 0);

    lock();
    m_started = false;
    unlock();
}

void Thread::terminate()
{
    // 동작중인 쓰레드를 강제로 종료시키면 스택 위치에 따라서 deadlock등의 문제를 야기할 수 있다
    // 종료되지 않는 쓰레드가 있다면 exit() 호출로 프로세스를 종료한다
    // 만약, 차일드 프로세스가 존재한다면 zombie 상태를 막기위해 waitpid()도 수행한다
    while(true)
    {
        int stat;
        pid_t pid = waitpid(-1, &stat, 0);;
        
        if(pid < 0)
        {
            if(errno == ECHILD)
                break;
            else if(errno == EINTR)
                continue;
        }
        else if(pid == 0)
        {
            break;
        }
    }

    exit(1);

    //lock();
    //pthread_cancel(m_ptid);
    //m_started = false;
    //unlock();
}

pthread_t Thread::getCurrentThreadId()
{
    return m_ptid;
    //return pthread_self();
}

void Thread::setThreadName(const char* name)
{
    char tmp[256] = {0, };

    if(name)
    {
        m_name = std::string(name);
    }
    else
    {
        snprintf(tmp, 255, "noname_%lu", m_ptid);
        m_name = std::string(tmp);
    }
}

const char* Thread::getThreadName()
{
    return m_name.c_str();
}

} // namespace vox
