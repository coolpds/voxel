#include <unistd.h>
#include <fcntl.h>

#include "exception.hpp"
#include "trace.hpp"
#include "epoll.hpp"
#include "socket.hpp"


namespace vox
{

Socket::Socket()
{
    m_domain = AF_INET;
    m_type = SOCK_STREAM;
    m_cloexec = true;
    m_sockfd = -1;
    m_nonblock = false;
    m_connected = false;
    m_remoteaddr = 0;
    m_remoteip = "";
    m_remotehost = "";
    m_remoteport = -1;
    m_remoteurl = "";
    m_localaddr = 0x7F000001; // 2130706433
    m_localip = "127.0.0.1";
    m_localport = -1;
    m_sockpath = "";
}

Socket::~Socket()
{
}

int Socket::getSockFd()
{
    return m_sockfd;
}

bool Socket::isCreated()
{
    return (m_sockfd >= 0);
}

bool Socket::isConnected()
{
    return m_connected;
}

bool Socket::create()
{
    return create(AF_INET, SOCK_STREAM, true);
}

bool Socket::create(int type, bool cloexec)
{
    return create(AF_INET, type, cloexec);
}

bool Socket::create(int domain, int type, bool cloexec)
{
    TRY_EXCEPTIONS;

    if(m_sockfd >= 0)
    {
        THROW_EXCEPTIONS("sockfd is already created.");
    }
    
    if((m_sockfd = socket(domain, type, 0)) < 0)
    {
        THROW_ERRORS(errno, "socket create failed. domain(%d) type(%d) cloexec(%u)",
            domain, type, cloexec);
    }

    m_domain = domain;
    m_type = type;
    
    if(cloexec)
    {
        m_cloexec = true;

        int val = fcntl(m_sockfd, F_GETFD, 0);

        if(val < 0)
        {
            THROW_ERRORS(errno, "fcntl(F_GETFL) failed.");
        }

        if(fcntl(m_sockfd, F_SETFD, val | FD_CLOEXEC) < 0)
        {
            THROW_ERRORS(errno, "fcntl(F_SETFL) failed.");
        }
    }
    else
    {
        m_cloexec = false;
    }

    if(m_domain == AF_UNIX)
    {
        m_remoteaddr = 0x7F000001;
        m_remoteip = "127.0.0.1";

        m_localaddr = 0x7F000001;
        m_localip = "127.0.0.1";
    }
        
    CATCH_EXCEPTIONS(false);

    return true;
}

bool Socket::attach(int sockfd)
{
    return attach(sockfd, AF_INET, SOCK_STREAM, false);
}

bool Socket::attach(int sockfd, int type, bool cloexec)
{
    return attach(sockfd, AF_INET, type, cloexec);
}

bool Socket::attach(int sockfd, int domain, int type, bool cloexec)
{
    TRY_EXCEPTIONS;

    if(sockfd < 0 || (domain != AF_INET && domain != AF_UNIX))
    {
        THROW_EXCEPTIONS("invalid parameter. sockfd(%d) domain(%d)", sockfd, domain);
    }

    if(m_sockfd >= 0)
        detach();

    m_sockfd = sockfd;
    m_domain = domain;
    m_type = type;

    if(cloexec)
    {
        m_cloexec = true;

        int	val = fcntl(m_sockfd, F_GETFD, 0);

        if(val < 0)
        {
            THROW_ERRORS(errno, "fcntl(F_GETFL) failed.");
        }

        if(fcntl(m_sockfd, F_SETFD, val | FD_CLOEXEC) < 0)
        {
            THROW_ERRORS(errno, "fcntl(F_SETFL) failed.");
        }
    }
    else
    {
        m_cloexec = false;
    }
    
    if(m_domain == AF_UNIX)
    {
        m_remoteaddr = 0x7F000001;
        m_remoteip = "127.0.0.1";

        m_localaddr = 0x7F000001;
        m_localip = "127.0.0.1";
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

void Socket::detach()
{
    Socket::close();
}

// for AF_INET
bool Socket::bind(const uint32_t port, bool loopback)
{
    struct sockaddr_in addr_in;

    TRY_EXCEPTIONS;

    if(m_domain != AF_INET)
    {
        THROW_EXCEPTIONS("invalid domain. domain(%d)", m_domain);
    }

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd. sockfd(%d)", m_sockfd);
    }

    if(port > 65535)
    {
        THROW_EXCEPTIONS("invalid listen-port number. port(%d)", port);
    }

    bzero(&addr_in, sizeof(addr_in));

    addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = (loopback) ? htonl(INADDR_LOOPBACK) : htonl(INADDR_ANY);
    addr_in.sin_port = htons((uint16_t)port);

    if(::bind(m_sockfd, (struct sockaddr*)&addr_in, (socklen_t)sizeof(addr_in)) < 0)
    {
        THROW_ERRORS(errno, "bind() failed.");
    }

    if(port == 0)
    {
        struct sockaddr_in addr2;
        int addr2_len = sizeof(struct sockaddr_in);
        bzero(&addr2, sizeof(addr2));
        
        if(getsockname(m_sockfd, (struct sockaddr*)&addr2, (socklen_t*)&addr2_len) < 0)
        {
            THROW_ERRORS(errno, "getsockname() failed.");
        }

        setLocalAddrInfo(ntohl(addr2.sin_addr.s_addr), ntohs(addr2.sin_port));
    }
    else
    {
        setLocalAddrInfo(ntohl(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

// for AF_UNIX
bool Socket::bind(const char* sockpath)
{
    struct sockaddr_un addr_un;

    TRY_EXCEPTIONS;

    if(m_domain != AF_UNIX)
    {
        THROW_EXCEPTIONS("invalid domain. domain(%d)", m_domain);
    }

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd. sockfd(%d)", m_sockfd);
    }

    if(!sockpath)
    {
        THROW_EXCEPTIONS("invalid sockpath.");
    }

    m_sockpath = std::string(sockpath);

    bzero(&addr_un, sizeof(addr_un));
    addr_un.sun_family = AF_UNIX;
    strncpy(addr_un.sun_path, m_sockpath.c_str(), sizeof(addr_un.sun_path) - 1);

    if(::bind(m_sockfd, (struct sockaddr*)&addr_un, (socklen_t)sizeof(addr_un)) < 0)
    {
        THROW_ERRORS(errno, "bind() failed.");
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

bool Socket::listen(const int backlog)
{
    TRY_EXCEPTIONS;

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd.");
    }

    if(backlog <= 0)
    {
        THROW_EXCEPTIONS("invalid backlog size.");
    }

    if(::listen(m_sockfd, backlog) < 0)
    {
        THROW_ERRORS(errno, "listen failed.");
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

int Socket::accept(Socket* child)
{
    int ss = SS_ERROR;
    int connfd = -1;
    struct sockaddr_in addr_in;
    struct sockaddr_un addr_un;
    socklen_t len_in = sizeof(sockaddr_in);
    socklen_t len_un = sizeof(sockaddr_un);

    TRY_EXCEPTIONS;

    if(!child)
    {
        THROW_EXCEPTIONS("invalid parameter. child(%p)", child);
    }

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd.");
    }

    if(m_type != SOCK_STREAM && m_type != SOCK_SEQPACKET && m_type != SOCK_RDM)
    {
        THROW_EXCEPTIONS("invalid socket type.");
    }

    if(m_domain == AF_INET)	// AF_INET
    {
        bzero(&addr_in, sizeof(addr_in));

        if((connfd = ::accept(m_sockfd, (struct sockaddr*)&addr_in, &len_in)) < 0)
        {
            if(errno == EAGAIN || errno == EINTR)
                return SS_RETRY;		

            THROW_ERRORS(errno, "accept failed. sockfd(%d) child(%p)", m_sockfd, child);
        }

        child->setRemoteAddrInfo(ntohl(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));

        // localaddrinfo
        {
            struct sockaddr_in addr2;
            int addr2_len = sizeof(struct sockaddr_in);
            
            bzero(&addr2, sizeof(addr2));
                
            if(getsockname(connfd, (struct sockaddr*)&addr2, (socklen_t*)&addr2_len) == 0)
            {
                child->setLocalAddrInfo(ntohl(addr2.sin_addr.s_addr), ntohs(addr2.sin_port));
            }
        }
    }
    else // AF_UNIX
    {
        bzero(&addr_un, sizeof(addr_un));

        if((connfd = ::accept(m_sockfd, (struct sockaddr*)&addr_un, &len_un)) < 0)
        {
            if(errno == EAGAIN || errno == EINTR)
                return SS_RETRY;		

            THROW_ERRORS(errno, "accept failed. sockfd(%d) child(%p)", m_sockfd, child);
        }
    }

    if(m_cloexec)
    {
        int val = fcntl(connfd, F_GETFD, 0);

        if(val < 0)
        {
            THROW_ERRORS(errno, "fcntl(F_GETFL) failed.");
        }

        if(fcntl(connfd, F_SETFD, val | FD_CLOEXEC) < 0)
        {
            THROW_ERRORS(errno, "fcntl(F_SETFL) failed.");
        }

        child->m_cloexec = true;
    }
    else
    {
        child->m_cloexec = false;
    }

    child->m_connected = true;
    child->m_sockfd = connfd;
    child->m_domain = (m_domain);

    if(m_domain == AF_UNIX)
    {
        child->m_remoteaddr = 0x7F000001;
        child->m_remoteip = "127.0.0.1";

        child->m_localaddr = 0x7F000001;
        child->m_localip = "127.0.0.1";
    }

    CATCH_EXCEPTIONS(ss);
    
    return SS_OK;
}

int Socket::connect(const char* host, int port, float timeout)
{
    struct sockaddr_in addr_in;
    int ss = SS_ERROR;
    struct hostent* he = NULL;
    int hlen = 0;
    const char* hstr = NULL;
    struct hostent hentry;
    char reserved[1024];
    int herr;
    bool nonblock = m_nonblock;

    TRY_EXCEPTIONS;

    m_remotehost = "";
    m_remoteurl = "";

    if(m_domain != AF_INET)
    {
        THROW_EXCEPTIONS("invalid domain. domain(%d)", m_domain);
    }

    if(m_sockfd <= 0)
    {
        THROW_EXCEPTIONS("invalid sockfd. sockfd(%d)", m_sockfd);
    }

    if(!host || (hlen = (int)strlen(host)) <= 0)
    {
        THROW_EXCEPTIONS("host is empty. sockfd(%d) port(%d)", m_sockfd, port);
    }

    hstr = host + hlen - 1;
    while(hstr > host && *(hstr - 1) != '.')
        hstr--;

    if(port <= 0 || port > 65535)
    {
        THROW_EXCEPTIONS("invalid port number. sockfd(%d) host(%s) port(%d)",
            m_sockfd, host, port);
    }

    if(m_type != SOCK_STREAM && m_type != SOCK_SEQPACKET && m_type != SOCK_RDM)
    {
        THROW_EXCEPTIONS("invalid socket type. sockfd(%d) host(%s) port(%d)",
            m_sockfd, host, port);
    }

    m_remotehost = host;
    m_remoteurl = m_remotehost + ":" + std::to_string(port);

    gethostbyname_r(host, &hentry, reserved, sizeof(reserved), &he, &herr);
    
    if(!he)
    {
        THROW_ERRORS(errno, "gethostbyname_r() failed. sockfd(%d) host(%s) port(%d)",
            m_sockfd, host, port);
    }

    bzero((char*)&addr_in, sizeof(addr_in));

    addr_in.sin_family = AF_INET;
    addr_in.sin_port = htons((uint16_t)port);
    addr_in.sin_addr = *((struct in_addr*)he->h_addr);

    if(!nonblock)
        nonBlock(true);

    if(::connect(m_sockfd, (struct sockaddr*)&addr_in, (socklen_t)sizeof(addr_in)) == 0)
    {
        setRemoteAddrInfo(ntohl(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));

        // localaddrinfo
        {
            struct sockaddr_in addr2;
            int addr2_len = sizeof(struct sockaddr_in);
            bzero(&addr2, sizeof(addr2));
                
            if(getsockname(m_sockfd, (struct sockaddr*)&addr2, (socklen_t*)&addr2_len) == 0)
            {
                setLocalAddrInfo(ntohl(addr2.sin_addr.s_addr), ntohs(addr2.sin_port));
            }
        }

        if(!nonblock)
            nonBlock(false);

        m_connected = true;

        return SS_OK;
    }
    else
    {
        if(errno == EINPROGRESS)
        {
            EPoll epoll;
            int sz = 0;
            epoll_event* ev = NULL;
            int sockerr = 0;
            socklen_t sockerrlen = sizeof(int);

            if(!epoll.create(1))
            {
                THROW_ERRORS(errno, "epoll_create() failed. sockfd(%d) host(%s) port(%d)",
                    m_sockfd, host, port);
            }

            if(!epoll.add(m_sockfd, this, EPOLLOUT))
            {
                epoll.close();
                THROW_ERRORS(errno, "epoll_add() failed. sockfd(%d) host(%s) port(%d)",
                    m_sockfd, host, port);
            }

            sz = epoll.wait(timeout);

            if(sz > 0)
            {
                ev = epoll.getEventList();

                if(!ev)
                {
                    epoll.close();
                    THROW_ERRORS(errno, "geteventlist() failed. sockfd(%d) host(%s) port(%d)",
                        m_sockfd, host, port);
                }

                if((ev[0].data.ptr != (void*)this) || ((ev[0].events & EPOLLOUT) != EPOLLOUT) ||
                    ((ev[0].events & EPOLLERR) == EPOLLERR) || ((ev[0].events & EPOLLHUP) == EPOLLHUP))
                {
                    epoll.close();
                    THROW_ERRORS(errno, "invalid epoll event. this(%p) sockfd(%d) host(%s) port(%d)",
                        (void*)this, m_sockfd, host, port);
                }

                if(!getSockOpt(SO_ERROR, &sockerr, &sockerrlen))
                {
                    epoll.close();
                    THROW_ERRORS(errno, "setsockopt() failed. this(%p) sockfd(%d) host(%s) port(%d)",
                        (void*)this, m_sockfd, host, port);
                }

                if(sockerr != 0)
                {
                    epoll.close();
                    THROW_ERRORS(errno, "socket error dectected. this(%p) sockfd(%d) host(%s) port(%d) sockerr(%d)",
                        (void*)this, m_sockfd, host, port, sockerr);
                }

                epoll.close();

                setRemoteAddrInfo(ntohl(addr_in.sin_addr.s_addr), ntohs(addr_in.sin_port));

                // localaddrinfo
                {
                    struct sockaddr_in	addr2;
                    int	addr2_len = sizeof(struct sockaddr_in);
                    bzero(&addr2, sizeof(addr2));
                        
                    if(getsockname(m_sockfd, (struct sockaddr*)&addr2, (socklen_t*)&addr2_len) == 0)
                    {
                        setLocalAddrInfo(ntohl(addr2.sin_addr.s_addr), ntohs(addr2.sin_port));
                    }
                }

                if(!nonblock)
                    nonBlock(false);

                m_connected = true;

                return SS_OK;
            }
            else if(sz == 0)
            {
                epoll.close();

                ss = SS_TIMEOUT;
                THROW_ERRORS(errno, "connection timed out. sockfd(%d) host(%s) port(%d) timeout(%f)",
                    m_sockfd, host, port, timeout);
            }
            else
            {
                epoll.close();

                if(errno == EINTR)
                {
                    if(!nonblock)
                        nonBlock(false);

                    return SS_RETRY;
                }
                
                ss = SS_ERROR;
                THROW_ERRORS(errno, "epoll_wait() failed. sockfd(%d) host(%s) port(%d) timeout(%f)",
                    m_sockfd, host, port, timeout);
            }

            epoll.close();

            ss = SS_ERROR;
            THROW_ERRORS(errno, "connect() failed. nonblock(%u) sockfd(%d) host(%s) port(%d) timeout(%f)",
                m_nonblock, m_sockfd, host, port, timeout);
        }
        else if(errno == ETIMEDOUT)	// blocking-socket
        {
            ss = SS_TIMEOUT;
            THROW_ERRORS(errno, "connection timed out. sockfd(%d) host(%s) port(%d) timeout(%f)",
                m_sockfd, host, port, timeout);
        }

        ss = SS_ERROR;
        THROW_ERRORS(errno, "connect() failed. sockfd(%d) host(%s) port(%d) timeout(%f)",
            m_sockfd, host, port, timeout);
    }

    CATCH_EXCEPTIONS_BEGIN;

    if(!nonblock)
        nonBlock(false);

    CATCH_EXCEPTIONS_END(ss);

    return SS_ERROR;
}

// for AF_UNIX
int Socket::connect(const char* sockpath, float timeout)
{
    struct sockaddr_un addr_un;
    int ss = SS_ERROR;
    bool nonblock = m_nonblock;

    TRY_EXCEPTIONS;

    m_remotehost = "";
    m_remoteurl = "";

    if(m_domain != AF_UNIX)
    {
        THROW_EXCEPTIONS("invalid domain. domain(%d)", m_domain);
    }

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd. sockfd(%d)", m_sockfd);
    }

    if(!sockpath)
    {
        THROW_EXCEPTIONS("sockpath is empty. sockpath(%s)", sockpath);
    }

    m_sockpath = std::string(sockpath);

    m_remotehost = "localhost";
    m_remoteurl = m_remotehost + ":" + m_sockpath;

    bzero((char*)&addr_un, sizeof(addr_un));
    addr_un.sun_family = AF_UNIX;
    strncpy(addr_un.sun_path, m_sockpath.c_str(), sizeof(addr_un.sun_path) - 1);

    if(!nonblock)
        nonBlock(true);

    if(::connect(m_sockfd, (struct sockaddr*)&addr_un, (socklen_t)sizeof(addr_un)) == 0)
    {
        m_remoteaddr = 0x7F000001;
        m_remoteip = "127.0.0.1";

        m_localaddr = 0x7F000001;
        m_localip = "127.0.0.1";

        if(!nonblock)
            nonBlock(false);

        m_connected = true;

        return SS_OK;
    }
    else
    {
        if(errno == EINPROGRESS)
        {
            EPoll epoll;
            int sz = 0;
            epoll_event* ev = NULL;
            int sockerr = 0;
            socklen_t sockerrlen = sizeof(int);

            if(!epoll.create(1))
            {
                THROW_ERRORS(errno, "epoll_create() failed. sockfd(%d) sockpath(%s)",
                    m_sockfd, m_sockpath.c_str());
            }

            if(!epoll.add(m_sockfd, this, EPOLLOUT))
            {
                epoll.close();
                THROW_ERRORS(errno, "epoll_add() failed. sockfd(%d) sockpath(%s)",
                    m_sockfd, m_sockpath.c_str());
            }

            sz = epoll.wait(timeout);

            if(sz > 0)
            {
                ev = epoll.getEventList();

                if(!ev)
                {
                    epoll.close();
                    THROW_ERRORS(errno, "geteventlist() failed. sockfd(%d) sockpath(%s)",
                        m_sockfd, m_sockpath.c_str());
                }

                if((ev[0].data.ptr != (void*)this) || ((ev[0].events & EPOLLOUT) != EPOLLOUT) ||
                    ((ev[0].events & EPOLLERR) == EPOLLERR) || ((ev[0].events & EPOLLHUP) == EPOLLHUP))
                {
                    epoll.close();
                    THROW_ERRORS(errno, "invalid epoll event. this(%p) sockfd(%d) sockpath(%s)",
                        (void*)this, m_sockfd, m_sockpath.c_str());
                }

                if(!getSockOpt(SO_ERROR, &sockerr, &sockerrlen))
                {
                    epoll.close();
                    THROW_ERRORS(errno, "setsockopt() failed. this(%p) sockfd(%d) sockpath(%s)",
                        (void*)this, m_sockfd, m_sockpath.c_str());
                }

                if(sockerr != 0)
                {
                    epoll.close();
                    THROW_ERRORS(errno, "socket error dectected. this(%p) sockfd(%d) sockpath(%s) sockerr(%d)",
                        (void*)this, m_sockfd, m_sockpath.c_str(), sockerr);
                }

                epoll.close();

                m_remoteaddr = 0x7F000001;
                m_remoteip = "127.0.0.1";

                m_localaddr = 0x7F000001;
                m_localip = "127.0.0.1";

                if(!nonblock)
                    nonBlock(false);

                m_connected = true;

                return SS_OK;
            }
            else if(sz == 0)
            {
                epoll.close();

                ss = SS_TIMEOUT;
                THROW_ERRORS(errno, "connection timed out. sockfd(%d) sockpath(%s)",
                    m_sockfd, m_sockpath.c_str());
            }
            else
            {
                epoll.close();

                if(errno == EINTR)
                {
                    if(!nonblock)
                        nonBlock(false);

                    return SS_RETRY;
                }
                    
                ss = SS_ERROR;
                THROW_ERRORS(errno, "epoll_wait() failed. sockfd(%d) sockpath(%s)",
                    m_sockfd, m_sockpath.c_str());
            }

            epoll.close();

            ss = SS_ERROR;
            THROW_ERRORS(errno, "connect() failed. nonblock(%u) sockfd(%d) sockpath(%s)",
                m_nonblock, m_sockfd, m_sockpath.c_str());
        }
        else if(errno == ETIMEDOUT)	// blocking-socket
        {
            ss = SS_TIMEOUT;
            THROW_ERRORS(errno, "connection timed out. sockfd(%d) sockpath(%s)",
                m_sockfd, m_sockpath.c_str());
        }

        ss = SS_ERROR;
        THROW_ERRORS(errno, "connect() failed. sockfd(%d) sockpath(%s)",
            m_sockfd, m_sockpath.c_str());
    }

    CATCH_EXCEPTIONS_BEGIN;

    if(!nonblock)
        nonBlock(false);

    CATCH_EXCEPTIONS_END(ss);

    return SS_ERROR;
}

void Socket::close()
{
    m_connected = false;

    if(m_sockfd >= 0)
    {
        ::close(m_sockfd);
        m_sockfd = -1;
    }

    m_sockpath = "";

    m_remoteaddr = 0;
    m_remoteip = "";
    m_remotehost = "";
    m_remoteurl = "";
    m_remoteport = -1;

    m_localaddr = 0;
    m_localip = "";
    m_localport = -1;

    m_domain = AF_INET;
    m_type = SOCK_STREAM;
    m_nonblock = false;
}

void Socket::setLocalAddrInfo(uint32_t addr, int port)
{
    m_localaddr = addr;
    m_localport = port;
    m_localip = std::to_string((addr / 256 / 256 / 256) % 256) + "."
        + std::to_string((addr / 256 / 256) % 256) + "."
        + std::to_string((addr / 256) % 256) + "."
        + std::to_string(addr % 256);
}

void Socket::setRemoteAddrInfo(uint32_t addr, int port)
{
    m_remoteaddr = addr;
    m_remoteport = port;
    m_remoteip = std::to_string((addr / 256 / 256 / 256) % 256) + "."
        + std::to_string((addr / 256 / 256) % 256) + "."
        + std::to_string((addr / 256) % 256) + "."
        + std::to_string(addr % 256);
    m_remoteurl = m_remoteip + ":" + std::to_string(port);
}

bool Socket::getSockOpt(int name, void* value, socklen_t* len, int level)
{
    TRY_EXCEPTIONS;

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd.");
    }

    if(getsockopt(m_sockfd, level, name, value, len) < 0)
    {
        THROW_ERRORS(errno, "getsockopt() failed.");
    }

    CATCH_EXCEPTIONS(false);
    
    return true;
}

bool Socket::setSockOpt(int name, const void* value, socklen_t len, int level)
{
    TRY_EXCEPTIONS;

    if(m_sockfd < 0)
    {
        THROW_EXCEPTIONS("invalid sockfd.");
    }

    if(setsockopt(m_sockfd, level, name, value, len) < 0)
    {
        THROW_ERRORS(errno, "setsockopt() failed.");
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

bool Socket::reuseAddr(bool onoff)
{
    int on = (onoff) ? 1 : 0;

    return setSockOpt(SO_REUSEADDR, &on, (socklen_t)sizeof(on));
}

bool Socket::keepAlive(bool onoff)
{
    int on = (onoff) ? 1 : 0;

    return setSockOpt(SO_KEEPALIVE, &on, (socklen_t)sizeof(on));
}

bool Socket::nonBlock(bool onoff)
{
    int val = 1;

    TRY_EXCEPTIONS;
    
    val = fcntl(m_sockfd, F_GETFL, 0);

    if(val < 0)
    {
        THROW_ERRORS(errno, "fcntl(%d,F_GETFL) failed.", m_sockfd);
    }

    if(onoff)
    {
        if(fcntl(m_sockfd, F_SETFL, val | O_NONBLOCK) < 0)
        {
            THROW_ERRORS(errno, "fcntl(%d,F_SETFL,0x%x|O_NONBLOCK) failed.", m_sockfd, val);
        }

        m_nonblock = true;
    }
    else
    {
        if(fcntl(m_sockfd, F_SETFL, val & ~O_NONBLOCK) < 0)
        {
            THROW_ERRORS(errno, "fcntl(%d,F_SETFL,0x%x&~O_NONBLOCK) failed.", m_sockfd, val);
        }

        m_nonblock = false;
    }

    CATCH_EXCEPTIONS(false);

    return true;
}

bool Socket::sendTimeout(float sec)
{
    struct timeval tv;

    if(sec <= 0.0)	// off
    {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
    else
    {
        tv.tv_sec = (long)sec;
        tv.tv_usec = ((long)(sec * 1000) % 1000) * 1000;
    }

    return setSockOpt(SO_SNDTIMEO, &tv, (int)sizeof(tv));
}

bool Socket::recvTimeout(float sec)
{
    struct timeval tv;

    if(sec <= 0.0)	// off
    {
        tv.tv_sec = 0;
        tv.tv_usec = 0;
    }
    else
    {
        // millisec resolution
        tv.tv_sec = (long)sec;
        tv.tv_usec = ((long)(sec * 1000) % 1000) * 1000;
    }

    return setSockOpt(SO_RCVTIMEO, &tv, (socklen_t)sizeof(tv));
}

int Socket::send(const char* buf, const size_t bufsize, size_t* sentsize, int flags)
{
    int ss = SS_OK;
    ssize_t len = 0;

    TRY_EXCEPTIONS;

    len = ::send(m_sockfd, buf, bufsize, flags);

    if(len > 0)
    {
        *sentsize = len;
        return SS_OK;
    }
    else if(len == 0)
    {
        *sentsize = 0;

        if(bufsize == 0)
            return SS_OK;

        ss = SS_CLOSED;
        THROW_EXCEPTIONS("abnormally connection closed.");
    }
    else
    {
        *sentsize = 0;

        if(errno == EINTR)
            return SS_RETRY;

        if(m_nonblock)
        {
            if(errno == EAGAIN)
                return SS_RETRY;
        }
        else
        {
            if(errno == EAGAIN)	// blocking socket timeout
            {
                ss = SS_TIMEOUT;
                THROW_ERRORS(errno, "send timeout. len(%ld) sockfd(%d) bufsize(%ld) nonblock(%u)",
                    len, m_sockfd, bufsize, m_nonblock);
            }
        }

        ss = SS_ERROR;
        THROW_ERRORS(errno, "send failed. len(%ld) sockfd(%d) bufsize(%ld) nonblock(%u)",
            len, m_sockfd, bufsize, m_nonblock);
    }

    CATCH_EXCEPTIONS(ss);

    return ss;
}

int Socket::recv(char* buf, const size_t bufsize, size_t* recvsize, int flags)
{
    int ss = SS_OK;
    ssize_t len = 0;

    TRY_EXCEPTIONS;

    len = ::recv(m_sockfd, buf, bufsize, flags);

    if(len > 0)
    {
        *recvsize = len;
        return SS_OK;
    }
    else if(len == 0)
    {
        *recvsize = 0;
        return SS_CLOSED;
    }
    else
    {
        *recvsize = 0;

        if(errno == EINTR)
            return SS_RETRY;

        if(m_nonblock)
        {
            if(errno == EAGAIN)
                return SS_RETRY;
        }
        else
        {
            if(errno == EAGAIN)	// blocking socket timeout
            {
                ss = SS_TIMEOUT;
                THROW_ERRORS(errno, "recv timeout. len(%ld) sockfd(%d) bufsize(%ld) nonblock(%u)",
                    len, m_sockfd, bufsize, m_nonblock);
            }
        }

        if(errno == 104)
            return SS_ERROR;

        ss = SS_ERROR;
        THROW_ERRORS(errno, "recv failed. len(%ld) sockfd(%d) bufsize(%ld) nonblock(%u)",
            len, m_sockfd, bufsize, m_nonblock);
    }

    CATCH_EXCEPTIONS(ss);

    return ss;
}

int Socket::sendMsg(const struct msghdr* msg, size_t* sentsize, int flags)
{
    int ss = SS_OK;
    ssize_t len = 0;

    TRY_EXCEPTIONS;

    len = ::sendmsg(m_sockfd, msg, flags);

    if(len > 0)
    {
        *sentsize = len;
        return SS_OK;
    }
    else if(len == 0)
    {
        *sentsize = 0;

        ss = SS_CLOSED;
        THROW_EXCEPTIONS("abnormally connection closed.");
    }
    else
    {
        *sentsize = 0;

        if(errno == EINTR)
            return SS_RETRY;

        if(m_nonblock)
        {
            if(errno == EAGAIN)
                return SS_RETRY;
        }
        else
        {
            if(errno == EAGAIN)
            {
                ss = SS_TIMEOUT;
                THROW_ERRORS(errno, "sendmsg timeout. len(%ld) sockfd(%d) msg(%p) nonblock(%u)",
                    len, m_sockfd, msg, m_nonblock);
            }
        }

        ss = SS_ERROR;
        THROW_ERRORS(errno, "sendmsg failed. len(%ld) sockfd(%d) msg(%p) nonblock(%u)",
            len, m_sockfd, msg, m_nonblock);
    }

    CATCH_EXCEPTIONS(ss);

    return ss;
}

int Socket::recvMsg(struct msghdr* msg, size_t* recvsize, int flags)
{
    int ss = SS_OK;
    ssize_t len = 0;

    TRY_EXCEPTIONS;

    len = ::recvmsg(m_sockfd, msg, flags);

    if(len > 0)
    {
        *recvsize = len;
        return SS_OK;
    }
    else if(len == 0)
    {
        *recvsize = 0;
        return SS_CLOSED;
    }
    else
    {
        *recvsize = 0;

        if(errno == EINTR)
            return SS_RETRY;

        if(m_nonblock)
        {
            if(errno == EAGAIN)
                return SS_RETRY;
        }
        else
        {
            if(errno == EAGAIN)
                return SS_TIMEOUT;
        }

        if(errno == 104)
            return SS_ERROR;

        ss = SS_ERROR;
        THROW_ERRORS(errno, "recvmsg failed. len(%ld) sockfd(%d) msg(%p) nonblock(%u)",
            len, m_sockfd, msg, m_nonblock);
    }

    CATCH_EXCEPTIONS(ss);

    return ss;
}

int Socket::sendFile(int infd, off_t* offset, size_t count, size_t* sentsize)
{
    int ss = SS_OK;
    ssize_t len = 0;
    
    *sentsize = 0;

    TRY_EXCEPTIONS;

    if((len = ::sendfile(m_sockfd, infd, offset, count)) < 0)
    {
        if(errno == EINTR)
            return SS_RETRY;

        if(m_nonblock)
        {
            if(errno == EAGAIN)
                return SS_RETRY;
        }
        else
        {
            if(errno == EAGAIN)
            {
                ss = SS_TIMEOUT;
                THROW_ERRORS(errno, "sendfile timeout. len(%ld) sockfd(%d) infd(%d) nonblock(%u)",
                    len, m_sockfd, infd, m_nonblock);
            }
        }

        ss = SS_ERROR;
        THROW_ERRORS(errno, "sendfile failed. len(%ld) sockfd(%d) infd(%d) nonblock(%u)",
            len, m_sockfd, infd, m_nonblock);
    }

    *sentsize = len;

    CATCH_EXCEPTIONS(ss);

    return ss;
}

} // namespace vox
