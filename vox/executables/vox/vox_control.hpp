#ifndef _VOX_EXE_VOX_CONTROL_HPP_
#define _VOX_EXE_VOX_CONTROL_HPP_

#include "step.hpp"
#include "servo.hpp"
#include "thread.hpp"


namespace vox
{

// command
#define FL  34 // SIGRTMIN
#define FF  35
#define FR  36
#define LL  37
#define ST1 38
#define ST2 39 // force
#define RR  40
#define BL  41
#define BB  42
#define BR  43
#define CC  44
#define UT  45
#define CW  46
#define UP  47
#define AC5 48
#define AC4 49
#define AC3 50 // SIGRTMAX-14
#define AC2 51
#define AC1 52
#define DN  53
#define CLT 54
#define CRT 55
#define CUP 56
#define CDN 57
#define EE  58 // easter egg: toggle
#define FB  59 // follow ball: toggle

// mode
#define MD1  (64) // SIGRTMAX
#define MD2  (63)
#define MD3  (62)
#define MD4  (61)
#define MD5  (60)

class VoxControl : public Thread
{

public:
    static VoxControl& getInstance();
    bool start();
    void stop();
    void run();
    bool isExit();
    Step* getStep() { return &m_step; }
    Servo* getServo() { return &m_servo; }

protected:
    VoxControl();
    virtual ~VoxControl();

private:
    static VoxControl m_instance;
    Step m_step;
    Servo m_servo;
};

} // namespace vox

#endif // _VOX_EXE_VOX_CONTROL_HPP_
