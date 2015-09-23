#ifndef _VOX_EXE_VOX_MISC_HPP_
#define _VOX_EXE_VOX_MISC_HPP_

#include <stdint.h>

#include "thread.hpp"


namespace vox
{

class VoxMisc : public Thread
{

public:
    VoxMisc();
    virtual ~VoxMisc();

    bool start();
    void stop();
    void run();

private:

};

} // namespace vox

#endif // _VOX_EXE_VOX_MISC_HPP_
