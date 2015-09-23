#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include "trace.hpp"
#include "util.hpp"
#include "step.hpp"
#include "servo.hpp"
#include "wiring_init.hpp"

using namespace std;
using namespace vox;


class MotorTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(MotorTest);
    CPPUNIT_TEST(stepTest);
    CPPUNIT_TEST(servoTest);
    CPPUNIT_TEST_SUITE_END();

public:
    MotorTest() {};
    ~MotorTest() {};
    void setUp()
    {
        Trace::getInstance().open("/dev/stdout", Trace::TraceLevel::LEVEL_DEBUG);
    };

    void tearDown()
    {
        Trace::getInstance().close();
    };

    void stepTest();
    void servoTest();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MotorTest);

void MotorTest::stepTest()
{
    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        CPPUNIT_ASSERT(0);
        return;
    }

    StepPins pins;

    pins.lclk = 22;// BCM 6 , Left CLK
    pins.ldir = 23;// BCM 13, Left DIR
    pins.rclk = 24;// BCM 19, Right CLK
    pins.rdir = 25;// BCM 26, Right DIR
    //pins.smm0 = 26;// BCM 12, Mode 0
    //pins.smm1 = 27;// BCM 16, Mode 1
    pins.enab = 28;// BCM 20, Enable

    Step s;

    CPPUNIT_ASSERT(s.initialize(pins) == true);
    
    //s.setSpeed(80);
    //sleep(3);

    for(int i=0; i < 400; i++)
        s.ff();

    s.stop();

    s.finalize();
}

void MotorTest::servoTest()
{
    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        CPPUNIT_ASSERT(0);
        return;
    }

    Servo s;
    
    if(!s.initialize(0, 7))
    {
        CPPUNIT_ASSERT(0);
        return;
    }

    // L, R, D, U
    s.setLimit(60, 60, 70, 50);

    sleep(1);

    int x = 0;
    int y = 0;

    s.setNeutral();

    sleep(1);

    for(int i = 0; i < 1; i++)
    {
        NOTICE("LEFT_TEST");
        while(true)
        {
            s.left(10);

            if(s.isLimitL())
                break;
        }

        NOTICE("DOWN_TEST");
        while(true)
        {
            s.down(10);

            if(s.isLimitD())
                break;
        }

        NOTICE("RIGHT_TEST");
        while(true)
        {
            s.right(10);

            if(s.isLimitR())
                break;
        }

        NOTICE("UP_TEST");
        while(true)
        {
            s.up(10);

            if(s.isLimitU())
                break;
        }
    }

    sleep(1);

    s.setNeutral();
    
    sleep(1);

    s.finalize();
}


