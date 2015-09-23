#ifndef _VOX_LIB_MOTOR_STEP_HPP_
#define _VOX_LIB_MOTOR_STEP_HPP_

#include <string>

namespace vox
{

#define MIN_PULSES (10) // 최소 펄스(0.9 x 10)

typedef struct _step_pins
{
    int lclk;
    int ldir;
    int rclk;
    int rdir;
    int enab;
} StepPins;

class Step
{

public:
    Step();
    virtual ~Step();
    bool initialize(const StepPins& pins);
    void finalize();

    void stop(bool force = false);
    void fl();
    void ff();
    void fr();
    void ll();
    void rr();
    void bl();
    void bb();
    void br();
    void cc();
    void ut();
    void cw();
    void setSpeed(float speed); // 1~100%

private:
    void pulse(int ldir, int rdir, int lhalf, int rhalf, int cnt = MIN_PULSES);

private:
    StepPins m_pins;
    int m_acccnt;
    int m_ldir;
    int m_rdir;
    int m_lhalf;
    int m_rhalf;
    float m_speed;
};

} // namespace vox

#endif // _VOX_LIB_MOTOR_STEP_HPP_
