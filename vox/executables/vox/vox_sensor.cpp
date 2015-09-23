#include "vox_sensor.hpp"
#include "vox_main.hpp"


namespace vox
{

VoxSensor::VoxSensor()
{
    m_temp = 0.f;
    m_humi = 0.f;
    m_dist_f = 0.f;
    m_dist_b = 0.f;
    m_dist_l = 0.f;
    m_dist_r = 0.f;
    m_freeze = false;

    setThreadName("SENS");
}

VoxSensor::~VoxSensor()
{
}

VoxSensor& VoxSensor::getInstance()
{
    static VoxSensor m_instance;
    return m_instance;
}

bool VoxSensor::start()
{
    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        return false;
    }

    if(!m_dht.initialize(21, 83))
        goto error;

    if(!m_us_f.initialize(26, 27))
        goto error;

    if(!m_us_b.initialize(29, 2))
        goto error;

    if(!m_us_l.initialize(15, 16))
        goto error;

    if(!m_us_r.initialize(31, 11))
        goto error;

    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);

error:
    stop();

    return false;
}

void VoxSensor::stop()
{
    resetEvent();

    if(Thread::isRunning())
    {
        if(g_main.isExit())
            Thread::kill(SIGHUP);

        Thread::stop(true);
    }

    m_us_r.finalize();
    m_us_l.finalize();
    m_us_b.finalize();
    m_us_f.finalize();
    m_dht.finalize();
}

void VoxSensor::run()
{
    float humi = 0.f;
    float temp = 0.f;
    bool freeze = false;
    uint64_t cnt = 0;

    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    INFORMATION("(SENS) start of loop");

    while(!g_main.isExit())
    {
        if((cnt % 100) == 0)
        {
            if(m_dht.readDhtData(&humi, &temp))
            {
                m_humi = humi;
                m_temp = temp;
            }
            
            delay(1);
        }

        m_dist_f = m_us_f.getDistance();
        delay(40);

        m_dist_b = m_us_b.getDistance();

        if(m_dist_f < LIMIT_F || m_dist_b > LIMIT_B)
            m_freeze = true;
        else
            m_freeze = false;

        delay(40);

        m_dist_l = m_us_l.getDistance();
        delay(40);

        m_dist_r = m_us_r.getDistance();
        delay(40);

        ++cnt;
    }

    INFORMATION("(SENS) end of loop");
}

} // namespace vox
