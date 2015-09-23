#include "event.hpp"

namespace vox
{

Event::Event()
{
    m_waiting = false;

    pthread_mutex_init(&m_cs, NULL);
    pthread_cond_init(&m_cv, NULL);
}

Event::~Event()
{
    pthread_cond_destroy(&m_cv);
    pthread_mutex_destroy(&m_cs);
}

void Event::lock()
{
    pthread_mutex_lock(&m_cs);
}

void Event::unlock()
{
    pthread_mutex_unlock(&m_cs);
}

void Event::setEvent()
{
    lock();
    
    if(m_waiting)
    {
        pthread_cond_signal(&m_cv);
    }

    unlock();
}

void Event::resetEvent()
{
    lock();
    pthread_cond_signal(&m_cv);
    unlock();
}

void Event::resetEventNoLock()
{
    pthread_cond_signal(&m_cv);
}

int Event::waitFor(float sec)
{
    int e;
    struct timespec ts;

    if(sec > 0.0f)
    {
        clock_gettime(CLOCK_REALTIME, &ts);

        ts.tv_sec += (time_t)(sec);
        ts.tv_nsec += (long)(((long)(sec * 1000) % 1000) * 1000000);

        if(ts.tv_nsec > 1000000000)
        {
            ts.tv_sec += 1;
            ts.tv_nsec -= 1000000000;
        }

        lock();
        m_waiting = true;
        e = pthread_cond_timedwait(&m_cv, &m_cs, &ts);
        m_waiting = false;
        unlock();

        if(e == ETIMEDOUT)
            return WAIT_TIMEOUT;
        else if(e == EINVAL || e == EPERM)
            return WAIT_ABANDONED;

        return WAIT_OBJECT_0;
    }
    else if(sec == 0.0f)
    {
        return WAIT_TIMEOUT;
    }
    else
    {
        lock();
        m_waiting = true;
        e = pthread_cond_wait(&m_cv, &m_cs);
        m_waiting = false;
        unlock();

        if(e == EINVAL || e == EPERM)
            return WAIT_ABANDONED;

        return WAIT_OBJECT_0;
    }

    return WAIT_FAILED;
}

bool Event::isWaiting()
{
    lock();
    bool ret = m_waiting;
    unlock();

    return ret;
}

} // namespace vox
