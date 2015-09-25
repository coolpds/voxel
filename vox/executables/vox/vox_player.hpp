#ifndef _VOX_EXE_VOX_PLAYER_HPP_
#define _VOX_EXE_VOX_PLAYER_HPP_

#include <stdint.h>
#include <mpg123.h>
#include <ao/ao.h>

#include "thread.hpp"


namespace vox
{

class VoxPlayer : public Thread
{

public:
    static VoxPlayer& getInstance();
    bool start();
    void stop();
    void run();
    void play(const std::string& path, bool async = false);
    bool isPlaying() { return m_isplaying; }
    void easterEgg();

protected:
    VoxPlayer();
    virtual ~VoxPlayer();

private:
    void aoplay(const std::string& path);

private:
    static VoxPlayer m_instance;
    pthread_mutex_t m_cs;
    std::string m_playpath;
    volatile bool m_isplaying;
    volatile bool m_interrupt;

};

} // namespace vox

#endif // _VOX_EXE_VOX_PLAYER_HPP_
