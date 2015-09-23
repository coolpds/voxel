#ifndef _VOX_LIB_SOCKET_EPOLL_HPP_
#define _VOX_LIB_SOCKET_EPOLL_HPP_

#include <errno.h>
#include <sys/epoll.h>


namespace vox
{

class EPoll
{

public:
    EPoll(bool edge = false);
    ~EPoll();

    bool create(int size, bool cloexec = true);
    void close();
    bool control(int op, int fd, struct epoll_event* ev);
    bool add(int fd, void* data, int events);
    bool add(int fd, int data, int events);
    bool modify(int fd, void* data, int events);
    bool modify(int fd, int data, int events);
    bool remove(int fd);
    int wait(float timeout = (-1.0));
    bool isCreated();
    inline void edgeTrigger(bool edge = false) { m_edge = ((edge) ? EPOLLET : 0); }
    inline epoll_event* getEventList() { return m_epev; }
    inline int getEventSize() { return m_epsz; }
    inline int getLastError() { return errno; }

private:
    bool m_cloexec;
    int m_edge;
    int m_epfd;
    epoll_event* m_epev;
    int m_epmaxsz;
    int m_epsz;
};

} // namespace vox

#endif // _VOX_LIB_SOCKET_EPOLL_HPP_
