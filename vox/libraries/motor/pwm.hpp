#ifndef _VOX_LIB_MOTOR_PWM_HPP_
#define _VOX_LIB_MOTOR_PWM_HPP_

#include <string>
#include <pthread.h>
#include <sys/time.h>

#include "thread.hpp"


namespace vox
{

class Pwm : public Thread
{

public:
    Pwm();
    ~Pwm();

    bool start(int pin, int initval, int range);
    void stop();
    void run();
    void write(int val);
    void suspend();
    void resume();

private:
    bool m_exit;
    bool m_wait;
    int m_pin;
    int m_mark;
    int m_range;
};

} // namespace vox

#endif // _VOX_LIB_MOTOR_PWM_HPP_
