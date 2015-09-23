#ifndef _VOX_LIB_SOCKET_SOCKET_HPP_
#define _VOX_LIB_SOCKET_SOCKET_HPP_

#include <stdint.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/un.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <errno.h>
#include <string>


namespace vox
{

class EPoll;

class Socket
{
public:
    
    Socket();
    ~Socket();

    enum SockStat
    {
        SS_ERROR = -4,   // critical error
        SS_TIMEOUT = -3,	 // blocking 소켓에서의 timeout
        SS_RETRY = -2,   // non-bocking 소켓에서의 would-block, 또는 EINTR
        SS_CLOSED = -1,  // socket closed, broken pipe
        SS_OK = 0        // successfully read or write
    };

    bool create(int domain, int type, bool cloexec = true);
    bool create(int type, bool cloexec = true);
    bool create();
    bool attach(int sockfd, int domain, int type, bool cloexec = false);
    bool attach(int sockfd, int type, bool cloexec = false);
    bool attach(int sockfd);
    void detach();
    bool bind(const uint32_t port, bool loopback = false);
    bool bind(const char* sockpath);
    bool listen(const int backlog = 32);
    int accept(Socket* child);
    int connect(const char* host, int port, float timeout = 5.0);
    int connect(const char* sockpath, float timeout = 5.0);
    void close();

    bool getSockOpt(int name, void* value, socklen_t* len, int level = SOL_SOCKET);
    bool setSockOpt(int name, const void* value, socklen_t len, int level = SOL_SOCKET);
    bool reuseAddr(bool onoff);
    bool keepAlive(bool onoff);
    bool nonBlock(bool onoff);
    bool sendTimeout(float sec);
    bool recvTimeout(float sec);
    int send(const char* buf, const size_t bufsize, size_t* sentsize, int flags = 0);
    int recv(char* buf, const size_t bufsize, size_t* recvsize, int flags = 0);
    int sendMsg(const struct msghdr* msg, size_t* sentsize, int flags = 0);
    int recvMsg(struct msghdr* msg, size_t* recvsize, int flags = 0);
    int sendFile(int infd, off_t* offset, size_t count, size_t* sentsize);
    int getSockFd();
    bool isCreated();
    bool isConnected();
    inline uint32_t getLocalAddr() { return m_localaddr; }
    inline const char* getLocalIP() { return m_localip.c_str(); } 
    inline int getLocalPort() { return m_localport; }
    inline uint32_t getRemoteAddr() { return m_remoteaddr; }
    inline const char* getRemoteIp() { return m_remoteip.c_str(); }
    inline const char* getRemoteHost() { return m_remotehost.c_str(); }
    inline int getRemotePort() { return m_remoteport; }
    inline const char* getRemoteUrl() { return m_remoteurl.c_str(); }
    inline int getLastError() { return errno; }

private:
    void setLocalAddrInfo(uint32_t addr, int port);
    void setRemoteAddrInfo(uint32_t addr, int port);

private:
    int m_domain;
    int m_type;
    bool m_cloexec;
    int m_sockfd;
    bool m_nonblock;
    bool m_connected;
    uint32_t m_remoteaddr;
    std::string m_remoteip;
    std::string m_remotehost;
    int m_remoteport;
    std::string m_remoteurl;
    uint32_t m_localaddr;
    std::string m_localip;
    int m_localport;
    std::string m_sockpath;
};

} // namespace vox

#endif // _VOX_LIB_SOCKET_SOCKET_HPP_
