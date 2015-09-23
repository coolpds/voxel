#include "autolock.hpp"

#include <assert.h>

namespace vox
{

Autolock::Autolock(pthread_mutex_t& cs) : m_cs(cs)
{
#ifndef NDEBUG
    int e =
#endif
        pthread_mutex_lock(&m_cs);
    assert(e == 0);
}

Autolock::~Autolock()
{
#ifndef NDEBUG
    int e =
#endif
        pthread_mutex_unlock(&m_cs);
    assert(e == 0);
}

} // namespace vox
