#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "wiringPi.h"
#include "trace.hpp"
#include "util.hpp"
#include "usonic.hpp"


namespace vox
{

USonic::USonic()
{
    m_trig = (-1);
    m_echo = (-1);
    m_burst = 10;
}

USonic::~USonic()
{
}

bool USonic::initialize(int echo, int trig, unsigned int burst)
{
    if(echo < 0 || trig < 0)
        return false;

    m_echo = echo;
    m_trig = trig;

    if(burst < 10 || burst > 500)
        m_burst = 10;
    else
        m_burst = burst;

    // free run mode 사용 안함
    pinMode(m_trig, OUTPUT);
    pinMode(m_echo, INPUT);

    digitalWrite(m_trig, LOW);

    return true;
}

void USonic::finalize()
{
    if(m_trig >= 0)
    {
        pinMode(m_trig, OUTPUT);
        digitalWrite(m_trig, LOW);
    }

    if(m_echo >= 0)
    {
        pinMode(m_echo, OUTPUT);
        digitalWrite(m_echo, LOW);
    }
}

float USonic::getDistance()
{
    int begin = 0;
    int end = 0;
    float distance = 0.f;

    if(m_echo < 0 || m_trig < 0)
        return -1.f;

    digitalWrite(m_trig, HIGH);
    delayMicroseconds(m_burst);
    digitalWrite(m_trig, LOW);

    begin = micros();

    while(digitalRead(m_echo) == LOW)
    {
        if((micros() - begin) > DEF_USNIC_TIMEOUT)
            return -1.f;
    }

    begin = micros();

    while(digitalRead(m_echo) == HIGH)
    {
        if((micros() - begin) > DEF_USNIC_TIMEOUT)
            return -1.f;
    }

    end = micros();

    // 오차 보정: C = 331.5 + (0.6 * t)
    distance = (end - begin) / 58.f;

    return distance;
}

} // namespace vox
