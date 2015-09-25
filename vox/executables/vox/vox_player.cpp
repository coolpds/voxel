#include "trace.hpp"
#include "util.hpp"

#include "vox_main.hpp"
#include "vox_control.hpp"
#include "vox_player.hpp"


namespace vox
{

VoxPlayer::VoxPlayer()
{
    pthread_mutex_init(&m_cs, NULL);

    mpg123_init();
    ao_initialize();

    m_playpath = "";
    m_isplaying = false;
    m_interrupt = false;

    setThreadName("PLAY");
}

VoxPlayer::~VoxPlayer()
{
    pthread_mutex_destroy(&m_cs);

    ao_shutdown();
    mpg123_exit();
}

VoxPlayer& VoxPlayer::getInstance()
{
    static VoxPlayer m_instance;
    return m_instance;
}

bool VoxPlayer::start()
{
    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);
}

void VoxPlayer::stop()
{
    resetEvent();

    if(Thread::isRunning())
    {
        if(g_main.isExit())
            Thread::kill(SIGHUP);

        Thread::stop(true);
    }
}

void VoxPlayer::easterEgg()
{
    const std::string mp3 = "/home/pi/rpi/www/starwars/Star_Wars_original_opening_crawl_1977.mp3";

    if(m_playpath == mp3)
    {
        while(m_isplaying && !g_main.isExit())
        {
            m_interrupt = true;
            delay(10);
        }

        m_interrupt = false;
        m_playpath = "";
    }
    else
    {
        m_playpath = mp3;
    }
}

void VoxPlayer::play(const std::string& path, bool async)
{
    while(m_isplaying && !g_main.isExit())
    {
        m_interrupt = true;
        delay(10);
    }

    m_interrupt = false;

    if(!g_main.isExit() && !path.empty())
    {
        if(async)
            m_playpath = path;
        else
            aoplay(path);
    }
}

void VoxPlayer::aoplay(const std::string& path)
{
    m_isplaying = true;

    ao_device* ad = NULL;
    mpg123_handle* mh = NULL;
    unsigned char* buf = NULL;
    size_t bufsz = 0;
    size_t done = 0;
    int driver = ao_default_driver_id();
    ao_sample_format asf;
    int channels = 0;
    int encoding = 0;
    long rate = 0;
    
    if(!(mh = mpg123_new(NULL, NULL)))
    {
        ERROR("mpg123_new() failed");
        goto error_success;
    }

    bufsz = mpg123_outblock(mh);
    buf = (unsigned char*)malloc(bufsz * sizeof(unsigned char));
    
    if(mpg123_open(mh, path.c_str()) != MPG123_OK)
    {
        ERROR("mpg123_open() failed");
        goto error_success;
    }

    mpg123_volume(mh, 1.4);

    if(mpg123_getformat(mh, &rate, &channels, &encoding) != MPG123_OK)
    {
        ERROR("mpg123_getformat() failed");
        goto error_success;
    }

    asf.bits = mpg123_encsize(encoding) * 8; // BITS
    asf.rate = rate;
    asf.channels = channels;
    asf.byte_format = AO_FMT_NATIVE;
    asf.matrix = 0;

    if(!(ad = ao_open_live(driver, &asf, NULL)))
    {
        ERROR("ao_open_live() failed");
        goto error_success;
    }
    
    while(!g_main.isExit() && !m_interrupt)
    {
        int ret = mpg123_read(mh, buf, bufsz, &done);

        DEBUG("ret=%d, done=%d", ret, done);

        if(ret == MPG123_OK || ret == MPG123_DONE || ret == MPG123_NEED_MORE)
        {
            if(!ao_play(ad, (char*)buf, done))
            {
                ERROR("ao_play() failed");
                break;
            }

            if(ret == MPG123_DONE)
                break;

            continue;
        }

        break;
    }

    DEBUG("play done");

error_success:

    if(buf)
    {
        free(buf);
        buf = NULL;
    }

    if(ad)
    {
        ao_close(ad);
        ad = NULL;
    }

    if(mh)
    {
        mpg123_close(mh);
        mpg123_delete(mh);
        mh = NULL;
    }

    m_isplaying = false;
}

void VoxPlayer::run()
{
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    INFORMATION("(PLAY) start of loop");

    while(!g_main.isExit())
    {
        if(!m_playpath.empty())
        {
            std::string path = m_playpath;
            NOTICE("(PLAY) async play '%s'", path.c_str());
            aoplay(path);
            m_playpath = "";
        }

        delay(100);
    }

    INFORMATION("(PLAY) end of loop");
}

} // namespace vox
