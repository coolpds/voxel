#include "trace.hpp"
#include "util.hpp"

#include "vox_main.hpp"
#include "vox_misc.hpp"


namespace vox
{

VoxMisc::VoxMisc()
{
    setThreadName("VOCE");
}

VoxMisc::~VoxMisc()
{
}

bool VoxMisc::start()
{
    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);
}

void VoxMisc::stop()
{
    resetEvent();

    if(Thread::isRunning())
    {
        if(g_main.isExit())
            Thread::kill(SIGHUP);

        Thread::stop(true);
    }
}

void VoxMisc::run()
{
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    INFORMATION("(MISC) start of loop");

    while(!g_main.isExit())
    {
        // TODO


        delay(100);
    }

    INFORMATION("(MISC) end of loop");
}

} // namespace vox
