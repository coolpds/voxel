#ifndef _VOX_LIB_THREAD_EVENT_HPP_
#define _VOX_LIB_THREAD_EVENT_HPP_

#include <errno.h>
#include <assert.h>

#include "autolock.hpp"

namespace vox
{

#define WAIT_FAILED    (-1) // 함수 수행 실패
#define WAIT_OBJECT_0  (0) // 이벤트 발생
#define WAIT_ABANDONED (1) // 오류로 인해 버려짐
#define WAIT_TIMEOUT   (2) // 타임아웃

class Event
{

public:
    Event();
    ~Event();

    void lock();
    void unlock();
    void setEvent();
    void resetEvent();
    void resetEventNoLock();
    int waitFor(float sec);
    bool isWaiting();

private:
    pthread_mutex_t m_cs;
    pthread_cond_t m_cv;
    bool m_waiting;
};

} // namespace vox

#endif // _VOX_LIB_THREAD_EVENT_HPP_
