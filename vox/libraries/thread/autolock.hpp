#ifndef _VOX_LIB_THREAD_AUTOLOCK_HPP_
#define _VOX_LIB_THREAD_AUTOLOCK_HPP_

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include <pthread.h>

#ifndef __USE_BSD
#define __USE_BSD
#endif

namespace vox
{

class Autolock
{

public:
    Autolock(pthread_mutex_t& cs);
    ~Autolock();

private:
    pthread_mutex_t& m_cs;
};


} // namespace vox

#endif // _VOX_LIB_THREAD_AUTOLOCK_HPP_
