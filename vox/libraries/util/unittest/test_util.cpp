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

using namespace std;
using namespace vox;


class UtilTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(UtilTest);
    CPPUNIT_TEST(case1);
/*    CPPUNIT_TEST(case2);
    CPPUNIT_TEST(case3);
    CPPUNIT_TEST(case4);
    CPPUNIT_TEST(case5);
    CPPUNIT_TEST(case6);
    CPPUNIT_TEST(case7);
    CPPUNIT_TEST(case8);
    CPPUNIT_TEST(case9);*/
    CPPUNIT_TEST_SUITE_END();

public:

    UtilTest() {};
    ~UtilTest() {};
    void setUp()
    {
        /*std::string cmdline = getFileContents(strFormat("/proc/%d/cmdline", getpid()));
        setlinebuf(stdout);

        if(cmdline.find("UtilTest") != std::string::npos)*/
            Trace::getInstance().open("/dev/stdout", Trace::TraceLevel::LEVEL_DEBUG);
    };

    void tearDown()
    {
        Trace::getInstance().close();
    };

    void case1();
    void case2();
    void case3();
    void case4();
    void case5();
    void case6();
    void case7();
    void case8();
    void case9();
};

CPPUNIT_TEST_SUITE_REGISTRATION(UtilTest);

void UtilTest::case1()
{
    NOTICE("sizeof(size_t)=%zu", sizeof(size_t));
    NOTICE("(%zu)", humanizeNums2Bytes("4gb"));

    CPPUNIT_ASSERT(humanizeNums(1) == "1");
    CPPUNIT_ASSERT(humanizeNums(2. * 1024) == "2k");
    CPPUNIT_ASSERT(humanizeNums(3. * 1024 * 1024) == "3m");
    CPPUNIT_ASSERT(humanizeNums(4. * 1024 * 1024 * 1024) == "4g");
    CPPUNIT_ASSERT(humanizeNums(5. * 1024 * 1024 * 1024 * 1024) == "5t");
    CPPUNIT_ASSERT(humanizeNums(6. * 1024 * 1024 * 1024 * 1024 * 1024) == "6p");
    CPPUNIT_ASSERT(humanizeNums(7. * 1024 * 1024 * 1024 * 1024 * 1024 * 1024) == "7e");

    CPPUNIT_ASSERT(humanizeNums2Bytes("unlimited") == 0);
    CPPUNIT_ASSERT(humanizeNums2Bytes("-1") == 0);
    CPPUNIT_ASSERT(humanizeNums2Bytes("0") == 0);
    CPPUNIT_ASSERT(humanizeNums2Bytes("1") == 1);
    CPPUNIT_ASSERT(humanizeNums2Bytes("2k") == 2000);
    CPPUNIT_ASSERT(humanizeNums2Bytes("2kb") == 2048);
    CPPUNIT_ASSERT(humanizeNums2Bytes("3m") == 3000000);
    CPPUNIT_ASSERT(humanizeNums2Bytes("3mb") == 3145728);
    CPPUNIT_ASSERT(humanizeNums2Bytes("4g") == 4000000000);
    
    // 32-bit
    /*CPPUNIT_ASSERT(humanizeNums2Bytes("4gb") == 4294967296);
    CPPUNIT_ASSERT(humanizeNums2Bytes("5t") == 5000000000000);
    CPPUNIT_ASSERT(humanizeNums2Bytes("5tb") == 5497558138880);
    CPPUNIT_ASSERT(humanizeNums2Bytes("6p") == 6000000000000000);
    CPPUNIT_ASSERT(humanizeNums2Bytes("6pb") == 6755399441055744);
    CPPUNIT_ASSERT(humanizeNums2Bytes("7e") == 7000000000000000000);
    CPPUNIT_ASSERT(humanizeNums2Bytes("7eb") == 8070450532247928832);*/

    CPPUNIT_ASSERT(isPositiveNumber("1234") == true);
    CPPUNIT_ASSERT(isPositiveNumber("-5678") == false);
    CPPUNIT_ASSERT(isPositiveNumber("abcd") == false);
    CPPUNIT_ASSERT(isPositiveNumber("-efgh") == false);
}


