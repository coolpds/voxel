#include "vox_control.hpp"
#include "vox_player.hpp"
#include "vox_sensor.hpp"
#include "vox_vision.hpp"
#include "vox_main.hpp"


namespace vox
{

VoxControl::VoxControl()
{
    setThreadName("CTRL");
}

VoxControl::~VoxControl()
{
}

VoxControl& VoxControl::getInstance()
{
    static VoxControl m_instance;
    return m_instance;
}

bool VoxControl::start()
{
    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        return false;
    }

    StepPins pins;

    pins.lclk = 22;// BCM 6 , Left CLK
    pins.ldir = 23;// BCM 13, Left DIR
    pins.rclk = 24;// BCM 19, Right CLK
    pins.rdir = 25;// BCM 26, Right DIR
    pins.enab = 28;// BCM 20, Enable

    if(!m_step.initialize(pins))
        return false;

    if(!m_servo.initialize(0, 7))
        return false;

    m_servo.setLimit(60, 60, 70, 50); // L:R:U:D

    //m_step.setSpeed(60);

    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);
}

void VoxControl::stop()
{
    resetEvent();

    if(Thread::isRunning())
    {
        if(g_main.isExit())
            Thread::kill(SIGHUP);

        Thread::stop(true);
    }

    m_servo.finalize();
    m_step.finalize();
}

void VoxControl::run()
{
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    int ncmd = ST1;
    int pcmd = ST1;

    INFORMATION("(CTRL) start of loop");

    while(!g_main.isExit())
    {
        ncmd = g_main.getCmdStat();

        if((pcmd == ncmd) &&
            (pcmd == LL || pcmd == RR || pcmd == UT ||
                pcmd == ST1 || pcmd == ST2 ||
                pcmd == AC1 || pcmd == AC2 ||
                pcmd == AC3 || pcmd == AC4 ||
                pcmd == AC5))
        {
            usleep(50000); // 50ms
            continue;
        }

        if(pcmd != ST1 && pcmd != ST2)
        {
            if(ncmd != ST1 && ncmd != ST2)
                m_step.stop();
        }

        //NOTICE("ncmd=%d", ncmd);

        switch(ncmd)
        {
            case FL:
                {
                    while(g_main.getCmdStat() == FL)
                    {
                        if(VoxSensor::getInstance().isFreeze())
                        {
                            g_main.setCmdStat(ST1);
                            m_step.stop(true);
                            break;
                        }

                        m_step.fl();
                    }
                }
                break;
            
            case FF:
                {
                    while(g_main.getCmdStat() == FF)
                    {
                        if(VoxSensor::getInstance().isFreeze())
                        {
                            g_main.setCmdStat(ST1);
                            //m_step.stop(true);
                            m_step.stop();
                            break;
                        }

                        m_step.ff();
                    }
                }
                break;

            case FR:
                {
                    while(g_main.getCmdStat() == FR)
                    {
                        if(VoxSensor::getInstance().isFreeze())
                        {
                            g_main.setCmdStat(ST1);
                            m_step.stop(true);
                            break;
                        }

                        m_step.fr();
                    }
                }
                break;

            case LL:
                {
                    if(VoxSensor::getInstance().getDistL() < LIMIT_L)
                    {
                        g_main.setCmdStat(ST1);
                        m_step.stop(true);
                        break;
                    }

                    m_step.ll();
                }
                break;

            case ST1:
            case ST2:
                m_step.stop();

                if(ncmd == ST2)
                    m_servo.setNeutral();

                break;

            case RR:
                {
                    if(VoxSensor::getInstance().getDistR() < LIMIT_R)
                    {
                        g_main.setCmdStat(ST1);
                        m_step.stop(true);
                        break;
                    }

                    m_step.rr();
                }
                break;

            case BL:
                {
                    while(g_main.getCmdStat() == BL)
                    {
                        if(VoxSensor::getInstance().getDistL() < LIMIT_L)
                        {
                            g_main.setCmdStat(ST1);
                            m_step.stop(true);
                            break;
                        }

                        m_step.bl();
                    }
                }
                break;

            case BB:
                {
                    while(g_main.getCmdStat() == BB)
                        m_step.bb();
                }
                break;

            case BR:
                {
                    while(g_main.getCmdStat() == BR)
                    {
                        if(VoxSensor::getInstance().getDistR() < LIMIT_R)
                        {
                            g_main.setCmdStat(ST1);
                            m_step.stop(true);
                            break;
                        }

                        m_step.br();
                    }
                }
                break;

            case CC:
                {
                    while(g_main.getCmdStat() == CC)
                    {
                        if(VoxSensor::getInstance().getDistL() < LIMIT_L
                            || VoxSensor::getInstance().getDistR() < LIMIT_R)
                        {
                            g_main.setCmdStat(ST1);
                            m_step.stop(true);
                            break;
                        }

                        m_step.cc();
                    }
                }
                break;

            case UT:
                {
                    if(VoxSensor::getInstance().getDistL() < LIMIT_L
                        || VoxSensor::getInstance().getDistR() < LIMIT_R)
                    {
                        g_main.setCmdStat(ST1);
                        m_step.stop(true);
                        break;
                    }

                    m_step.ut();
                }
                break;

            case CW:
                {
                    while(g_main.getCmdStat() == CW)
                    {
                        if(VoxSensor::getInstance().getDistL() < LIMIT_L
                            || VoxSensor::getInstance().getDistR() < LIMIT_R)
                        {
                            g_main.setCmdStat(ST1);
                            m_step.stop(true);
                            break;
                        }

                        m_step.cw();
                    }
                }
                break;

            case CLT:
                {
                    while(g_main.getCmdStat() == CLT)
                        m_servo.left(5);
                }
                break;

            case CUP:
                {
                    while(g_main.getCmdStat() == CUP)
                        m_servo.up(5);
                }
                break;

            case CDN:
                {
                    while(g_main.getCmdStat() == CDN)
                        m_servo.down(5);
                }
                break;

            case CRT:
                {
                    while(g_main.getCmdStat() == CRT)
                        m_servo.right(5);
                }
                break;

            case AC5:
                m_step.stop();
                m_step.setSpeed(100);
                break;

            case AC4:
                m_step.stop();
                m_step.setSpeed(80);
                break;

            case AC3:
                m_step.stop();
                m_step.setSpeed(60);
                break;

            case AC2:
                m_step.stop();
                m_step.setSpeed(40);
                break;

            case AC1:
                m_step.stop();
                m_step.setSpeed(20);
                break;

            case EE:
                {
                    NOTICE("EE");
                    g_main.setCmdStat(ST1);
                    VoxPlayer::getInstance().easterEgg();
                }
                break;

            case FB:
                {
                    VoxVision::getInstance().setFollowing(
                        !VoxVision::getInstance().isFollowing());
                    g_main.setCmdStat(ST1);
                }
                break;

            default: break;
        }

        pcmd = ncmd;
    }

    INFORMATION("(CTRL) end of loop");
}

} // namespace vox
