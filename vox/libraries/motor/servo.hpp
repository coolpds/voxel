#ifndef _VOX_LIB_MOTOR_SERVO_HPP_
#define _VOX_LIB_MOTOR_SERVO_HPP_

#include "pwm.hpp"

namespace vox
{

#define DEF_SERVO_SPEED (1.666667f) // msec/1도
#define DEF_MIN_PLUSE_DURATION (20) // 20msec

#define DEF_DUTY_L (600) // usec, -90도
#define DEF_DUTY_N (1500) // usec, 0도, neutral
#define DEF_DUTY_R (2400) // usec, 90도


class Servo
{

public:
    Servo();
    ~Servo();
    bool initialize(int pin_x, int pin_y);
    void finalize();
    void setLimit(int left, int right, int up, int down);
    void setNeutral(bool x = true, bool y = true);
    void left(unsigned int angle = 5);
    void right(unsigned int angle = 5);
    void up(unsigned int angle = 5);
    void down(unsigned int angle = 5);
    void getPosition(int* x, int* y);
    bool isLimitR() { return (m_cur_x <= m_limit_right); }
    bool isLimitL() { return (m_cur_x >= m_limit_left); }
    bool isLimitU() { return (m_cur_y >= m_limit_up); }
    bool isLimitD() { return (m_cur_y <= m_limit_down); }

private:
    void setNeutralEx(bool x, bool y);

private:
    int m_pin_x;
    int m_pin_y;
    Pwm m_pwm_x;
    Pwm m_pwm_y;

    int m_limit_left;
    int m_limit_right;
    int m_limit_up;
    int m_limit_down;

    int m_cur_x;
    int m_cur_y;
    int m_cur_duty_x;
    int m_cur_duty_y;
};

} // namespace vox

#endif // _VOX_LIB_MOTOR_SERVO_HPP_
