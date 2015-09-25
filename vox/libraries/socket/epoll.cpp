#include <unistd.h>
#include <fcntl.h>

#include "trace.hpp"
#include "exception.hpp"
#include "epoll.hpp"

namespace vox
{

EPoll::EPoll(bool edge)
{
    m_cloexec = true;
    m_edge = ((edge) ? EPOLLET : 0);
    m_epfd = -1;
    m_epev = NULL;
    m_epmaxsz = 0;
    m_epsz = 0;
}

EPoll::~EPoll()
{
}

bool EPoll::create(int size, bool cloexec)
{
    TRY_EXCEPTIONS;

    if(m_epfd >= 0 || m_epev)
    {
        THROW_EXCEPTIONS("epoll descriptor is already created.");
    }

    if(size <= 0)
    {
        THROW_EXCEPTIONS("invalid backing store size.");
    }

    if((m_epfd = epoll_create(size)) < 0)
    {
        THROW_ERRORS(errno, "epoll_create() failed.");
    }

    m_epev = new(std::nothrow) epoll_event[size];
    
    if(!m_epev)
    {
        EPoll::close();
        THROW_EXCEPTIONS("insufficient memory.");
    }

    m_epmaxsz = size;

    if(cloexec)
    {
        m_cloexec = true;

        int val = fcntl(m_epfd, F_GETFD, 0);

        if(val < 0)
        {
            EPoll::close();
            THROW_ERRORS(errno, "fcntl(F_GETFL) failed.");
        }

        if(fcntl(m_epfd, F_SETFD, val | FD_CLOEXEC) < 0)
        {
            EPoll::close();
            THROW_ERRORS(errno, "fcntl(F_SETFL) failed.");
        }

    }
    else
    {
        m_cloexec = false;
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

void EPoll::close()
{
    if(m_epev)
    {
        delete[] m_epev;
        m_epev = NULL;
    }

    if(m_epfd >= 0)
    {
        ::close(m_epfd);
        m_epfd = -1;
    }

    m_epmaxsz = 0;
    m_epsz = 0;
}

bool EPoll::isCreated()
{
    return (m_epfd >= 0);
}

bool EPoll::control(int op, int fd, struct epoll_event* ev)
{
    TRY_EXCEPTIONS;

    if(m_epfd < 0)
    {
        THROW_EXCEPTIONS("epoll descriptor has not been created yet.");
    }

    if(fd < 0)
    {
        THROW_EXCEPTIONS("invalid file descriptor. fd(%d)", fd);
    }

    if(epoll_ctl(m_epfd, op, fd, ev) < 0)
    {
        if(errno == ENOENT)
        {
            if((op & EPOLL_CTL_DEL) || (op & EPOLL_CTL_MOD))
            {
                ERROR("ENOENT, epoll_ctl, epfd(%d) op(%d) fd(%d) ev(%p)", m_epfd, op, fd, ev);
                return false;
            }
        }
        else if(errno == EEXIST)
        {
            if(op & EPOLL_CTL_ADD) 
            {
                ERROR("EEXIST, epoll_ctl, epfd(%d) op(%d) fd(%d) ev(%p)", m_epfd, op, fd, ev);
                return false;
            }
        }

        THROW_ERRORS(errno, "epoll_ctl failed. epfd(%d) op(%d) fd(%d) ev(%p)", m_epfd, op, fd, ev);
    }
    
    CATCH_EXCEPTIONS(false);

    return true;
}

bool EPoll::add(int fd, void* data, int events)
{
    struct epoll_event ev = {0, {0}};
    
    ev.data.ptr = data;
    ev.events = events | m_edge;

    return control(EPOLL_CTL_ADD, fd, &ev);
}

bool EPoll::add(int fd, int data, int events)
{
    struct epoll_event ev = {0, {0}};
    
    ev.data.fd = data;
    ev.events = events | m_edge;

    return control(EPOLL_CTL_ADD, fd, &ev);
}

bool EPoll::modify(int fd, void* data, int events)
{
    struct epoll_event ev = {0, {0}};
    
    ev.data.ptr = data;
    ev.events =  events | m_edge;

    return control(EPOLL_CTL_MOD, fd, &ev);
}

bool EPoll::modify(int fd, int data, int events)
{
    struct epoll_event ev = {0, {0}};
    
    ev.data.fd = data;
    ev.events =  events | m_edge;

    return control(EPOLL_CTL_MOD, fd, &ev);
}

bool EPoll::remove(int fd)
{
    return control(EPOLL_CTL_DEL, fd, NULL);
}

int EPoll::wait(float timeout)
{
    int millisec = (-1);

    TRY_EXCEPTIONS;

    if(m_epfd < 0 || !m_epev)
    {
        THROW_EXCEPTIONS("epoll descriptor has not been created yet. epfd(%d) epev(%p)", m_epfd, m_epev);
    }

    if(timeout >= 0.0)
        millisec = (int)(timeout * 1000.0);

    m_epsz = epoll_wait(m_epfd, m_epev, m_epmaxsz, millisec);

    if(m_epsz < 0)
    {
        if(errno != EINTR)
        {
            THROW_ERRORS(errno, "epoll_wait(%d, %p, %d, %d) failed.", m_epfd, m_epev, m_epsz, millisec);
        }
    }

    CATCH_EXCEPTIONS(-1);

    return m_epsz;
}

} // namespace vox
