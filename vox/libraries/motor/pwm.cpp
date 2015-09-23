#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <wiringPi.h>

#include "trace.hpp"
#include "util.hpp"
#include "servo.hpp"
#include "pwm.hpp"


namespace vox
{

Pwm::Pwm()
{
    m_exit = false;
    m_wait = true;
    m_pin = (-1);
    m_mark = 0;
    m_range = 20000; // 20ms
}

Pwm::~Pwm()
{
}

bool Pwm::start(int pin, int initval, int range)
{
    if(pin < 0 || initval < 0 || range < 0)
        return false;

    m_pin = pin;
    m_mark = initval;
    m_range = range;

    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);
}

void Pwm::stop()
{
    m_exit = true;

    resetEvent();

    if(Thread::isRunning())
        Thread::stop(true);

    m_pin = (-1);
    m_mark = 0;
    m_range = 0;
}

void Pwm::run()
{
    //struct sched_param param;
    //param.sched_priority = sched_get_priority_max(SCHED_RR);
    //pthread_setschedparam(pthread_self(), SCHED_RR, &param);

    int space = 0;

    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);

    while(!m_exit)
    {
        if(m_wait)
        {
            if(waitFor(0.1) == WAIT_TIMEOUT)
            {
                yield();
                continue;
            }

            if(m_wait)
                continue;
        }

        space = m_range - m_mark;

        if(m_mark != 0)
            digitalWrite(m_pin, HIGH);

        delayMicroseconds(m_mark);

        if(space != 0)
            digitalWrite(m_pin, LOW);

        delayMicroseconds(space);
    }

    pinMode(m_pin, OUTPUT);
    digitalWrite(m_pin, LOW);
}

void Pwm::write(int val)
{
    if(val < 0)
        val = 0;
    else if(val > m_range)
        val = m_range;

    m_mark = val;
}

void Pwm::suspend()
{
    m_wait = true;
}

void Pwm::resume()
{
    m_wait = false;
    setEvent();
}

} // namespace vox
