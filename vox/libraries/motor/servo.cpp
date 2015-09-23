#include <cassert>
#include <math.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <softPwm.h>
#include <wiringPi.h>

#include "trace.hpp"
#include "util.hpp"
#include "servo.hpp"
#include "pwm.hpp"


namespace vox
{

#define DEF_SERVO_CLOCK (20000) //20ms

Servo::Servo()
{
    m_pin_x = 0;
    m_pin_y = 7;

    m_limit_left = 60;
    m_limit_right = (-60);
    m_limit_up = (70);
    m_limit_down = (-50);

    m_cur_duty_x = DEF_DUTY_N; // 1500
    m_cur_duty_y = DEF_DUTY_N; // 1500
    m_cur_x = (0);
    m_cur_y = (0);
}

Servo::~Servo()
{
}

bool Servo::initialize(int pin_x, int pin_y)
{
    if(pin_x < 0 || pin_y < 0)
        return false;

    m_pin_x = pin_x;
    m_pin_y = pin_y;

    if(!m_pwm_x.start(m_pin_x, 0, DEF_SERVO_CLOCK))
        return false;

    if(!m_pwm_y.start(m_pin_y, 0, DEF_SERVO_CLOCK))
        return false;

    setNeutral(); // 중립

    return true;
}

void Servo::finalize()
{
    m_pwm_x.stop();
    m_pwm_y.stop();
}

void Servo::setLimit(int left, int right, int up, int down)
{
    m_limit_left = left;
    m_limit_right = (-1) * right;
    m_limit_up = up;
    m_limit_down = (-1) * down;
}

void Servo::getPosition(int* x, int* y)
{
    (*x) = (int)((-1) * m_cur_x);
    (*y) = m_cur_y;
}

void Servo::setNeutral(bool x, bool y)
{
    setNeutralEx(x, y);
    delay(100);
    setNeutralEx(x, y);
}

void Servo::setNeutralEx(bool x, bool y)
{
    assert(m_pin_x >= 0 && m_pin_y >= 0);

    if(x)
    {
        m_pwm_x.resume();
        m_pwm_x.write(DEF_DUTY_N);
        m_cur_duty_x = DEF_DUTY_N;
        m_cur_x = 0;
        delay(50);
        m_pwm_x.suspend();
    }

    if(y)
    {
        m_pwm_y.resume();
        m_pwm_y.write(DEF_DUTY_N);
        m_cur_duty_y = DEF_DUTY_N;
        m_cur_y = 0;
        delay(50);
        m_pwm_y.suspend();
    }
}

void Servo::left(unsigned int angle)
{
    assert(m_pin_x >= 0);

    if(m_limit_left >= (int)(m_cur_x + angle))
    {
        m_cur_duty_x += (10 * angle);
        m_cur_x += angle;

        m_pwm_x.resume();
        m_pwm_x.write(m_cur_duty_x);
        delay(DEF_MIN_PLUSE_DURATION + (unsigned int)ceil(DEF_SERVO_SPEED * angle));
        m_pwm_x.suspend();
    }
}

void Servo::right(unsigned int angle)
{
    assert(m_pin_x >= 0);

    if(m_limit_right <= (int)(m_cur_x - angle))
    {
        m_cur_duty_x -= (10 * angle);
        m_cur_x -= angle;

        m_pwm_x.resume();
        m_pwm_x.write(m_cur_duty_x);
        delay(DEF_MIN_PLUSE_DURATION + (unsigned int)ceil(DEF_SERVO_SPEED * angle));
        m_pwm_x.suspend();
    }
}

void Servo::up(unsigned int angle)
{
    assert(m_pin_y >= 0);

    if(m_limit_up >= (int)(m_cur_y + angle))
    {
        m_cur_duty_y += (10 * angle);
        m_cur_y += angle;

        m_pwm_y.resume();
        m_pwm_y.write(m_cur_duty_y);
        delay(DEF_MIN_PLUSE_DURATION + (unsigned int)ceil(DEF_SERVO_SPEED * angle));
        m_pwm_y.suspend();
    }
}

void Servo::down(unsigned int angle)
{
    assert(m_pin_y >= 0);

    if(m_limit_down <= (int)(m_cur_y - angle))
    {
        m_cur_duty_y -= (10 * angle);
        m_cur_y -= angle;

        m_pwm_y.resume();
        m_pwm_y.write(m_cur_duty_y);
        delay(DEF_MIN_PLUSE_DURATION + (unsigned int)ceil(DEF_SERVO_SPEED * angle));
        m_pwm_y.suspend();
    }
}

} // namespace vox
