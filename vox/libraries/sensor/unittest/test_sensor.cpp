#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/HelperMacros.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include "wiringPi.h"

#include "trace.hpp"
#include "util.hpp"
#include "dht.hpp"
#include "usonic.hpp"
#include "wiring_init.hpp"

using namespace std;
using namespace vox;


class SensorTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(SensorTest);
    CPPUNIT_TEST(dht);
    //CPPUNIT_TEST(usonic);
    CPPUNIT_TEST_SUITE_END();

public:

    SensorTest() {};
    ~SensorTest() {};
    void setUp()
    {
        Trace::getInstance().open("/dev/stdout", Trace::TraceLevel::LEVEL_DEBUG);
    };

    void tearDown()
    {
        Trace::getInstance().close();
    };

    void dht();
    void usonic();
};

CPPUNIT_TEST_SUITE_REGISTRATION(SensorTest);

void SensorTest::dht()
{
    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        CPPUNIT_ASSERT(0);
        return;
    }

    Dht d;

    CPPUNIT_ASSERT(d.initialize(21, 83) == true);

    float humi = 0.f;
    float temp = 0.f;
    
    for(int i = 0; i < 10; i++)
    {
        d.readDhtData(&humi, &temp);
        NOTICE("humi=%.2f, temp=%.2f", humi, temp);
        sleep(1);
    }

    d.finalize();
}

void SensorTest::usonic()
{
    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        CPPUNIT_ASSERT(0);
        return;
    }

    USonic us_l;
    USonic us_r;


    USonic us_f;
    USonic us_b;

    //CPPUNIT_ASSERT(us_l.initialize(0, 0, 10) == true);
    CPPUNIT_ASSERT(us_l.initialize(15, 16, 10) == true);
    //CPPUNIT_ASSERT(us_r.initialize(5, 6, 10) == true);
    //CPPUNIT_ASSERT(us_r.initialize(3, 11, 10) == true);
    CPPUNIT_ASSERT(us_r.initialize(31, 11, 10) == true);

    CPPUNIT_ASSERT(us_f.initialize(26, 27, 10) == true);
    CPPUNIT_ASSERT(us_b.initialize(29, 2, 10) == true);

    for(int i = 0; i < 100; i++)
    {
        float dist_l = us_l.getDistance();
        usleep(100000);
        float dist_r = us_r.getDistance();
        usleep(100000);

        float dist_f = us_f.getDistance();
        usleep(100000);
        float dist_b = us_b.getDistance();
        usleep(100000);

        //NOTICE("L:%6.2f cm", dist_l);
          
        NOTICE("               F:%6.2f cm                                                   ", dist_f);
        NOTICE("L:%6.2f                           R:%6.2f cm                                ", dist_l, dist_r);
        NOTICE("               B:%6.2f cm                                                   ", dist_b);
        NOTICE("                                                                            ");
        sleep(1);
    }

    us_b.finalize();
    us_f.finalize();

    us_r.finalize();
    us_l.finalize();
}


