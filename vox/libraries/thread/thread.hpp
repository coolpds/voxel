#ifndef _VOX_LIB_THREAD_THREAD_HPP_
#define _VOX_LIB_THREAD_THREAD_HPP_

#include <string>

#include "event.hpp"

namespace vox
{

#define THREAD_KILLTO (30.0) // 쓰레드 강제 종료 대기시간(30초)
#define THREAD_DEFAULT_STACKSIZE (4 * 1024 * 1024) // 기본 스택 사이즈


class Thread : public Event
{

public:
    Thread();
    ~Thread();
    virtual void run() = 0;
    bool start(bool detach = false, int stacksize = -1);
    void stop(bool force = true);
    bool isRunning();
    pthread_t getCurrentThreadId();
    void setThreadName(const char* name);
    const char* getThreadName();
    void kill(int signo);
    void yield();

protected:
    static void* threadFunc(void* pArg);

protected:
    pthread_t m_ptid;
    bool m_detach;
    bool m_started;

private:
    void join();
    void terminate();

private:
    void* m_retptr;
    std::string m_name;
};

} // namespace vox

#endif // _VOX_LIB_THREAD_THREAD_HPP_
