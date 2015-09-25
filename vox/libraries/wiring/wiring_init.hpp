#ifndef _VOX_LIB_WIRING_WIRING_INIT_HPP_
#define _VOX_LIB_WIRING_WIRING_INIT_HPP_

#include <cstdio>
#include <string>
#include "wiringPi.h"

namespace vox
{

#define INIT_WIRING WiringInit::getInstance().isInitialized()

class WiringInit
{
public:
    static WiringInit& getInstance()
    {
        static WiringInit instance;
        return instance;
    }

    bool isInitialized() { return m_initialized; }

private:
    WiringInit()
    {
        if(wiringPiSetup() == -1)
            m_initialized = false;
        else
            m_initialized = true;
    }

    ~WiringInit()
    {
    }

private:
    bool m_initialized;
};

} // namespace vox

#endif // _VOX_LIB_WIRING_WIRING_INIT_HPP_
