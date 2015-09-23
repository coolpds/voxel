#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#include "wiringPi.h"
#include "trace.hpp"
#include "util.hpp"
#include "step.hpp"


namespace vox
{

#define MAX_ACC (400)

const unsigned int g_accTable[MAX_ACC]=
{
//[Accelator = 2]
   31953,  13235,  10155,   8561,   7543,   6819,   6271,   5837,   5482,   5185, 
    4931,   4712,   4519,   4349,   4196,   4058,   3933,   3819,   3714,   3618, 
    3528,   3445,   3368,   3295,   3227,   3164,   3103,   3046,   2992,   2941, 
    2893,   2846,   2802,   2760,   2720,   2681,   2644,   2609,   2574,   2542, 
    2510,   2480,   2450,   2422,   2395,   2368,   2342,   2318,   2294,   2270, 
    2248,   2226,   2205,   2184,   2164,   2144,   2125,   2106,   2088,   2071, 
    2054,   2037,   2020,   2004,   1989,   1974,   1959,   1944,   1930,   1916, 
    1902,   1889,   1876,   1863,   1851,   1838,   1826,   1814,   1803,   1791, 
    1780,   1769,   1758,   1748,   1738,   1727,   1717,   1707,   1698,   1688, 
    1679,   1670,   1661,   1652,   1643,   1634,   1626,   1618,   1609,   1601, 
    1593,   1585,   1578,   1570,   1562,   1555,   1548,   1540,   1533,   1526, 
    1519,   1513,   1506,   1499,   1493,   1486,   1480,   1473,   1467,   1461, 
    1455,   1449,   1443,   1437,   1431,   1426,   1420,   1414,   1409,   1403, 
    1398,   1393,   1387,   1382,   1377,   1372,   1367,   1362,   1357,   1352, 
    1347,   1343,   1338,   1333,   1329,   1324,   1319,   1315,   1311,   1306, 
    1302,   1298,   1293,   1289,   1285,   1281,   1277,   1273,   1269,   1265, 
    1261,   1257,   1253,   1249,   1245,   1241,   1238,   1234,   1230,   1227, 
    1223,   1219,   1216,   1212,   1209,   1206,   1202,   1199,   1195,   1192, 
    1189,   1185,   1182,   1179,   1176,   1173,   1169,   1166,   1163,   1160, 
    1157,   1154,   1151,   1148,   1145,   1142,   1139,   1136,   1133,   1131, 
    1128,   1125,   1122,   1119,   1117,   1114,   1111,   1109,   1106,   1103, 
    1101,   1098,   1095,   1093,   1090,   1088,   1085,   1083,   1080,   1078, 
    1075,   1073,   1071,   1068,   1066,   1063,   1061,   1059,   1056,   1054, 
    1052,   1050,   1047,   1045,   1043,   1041,   1038,   1036,   1034,   1032, 
    1030,   1028,   1025,   1023,   1021,   1019,   1017,   1015,   1013,   1011, 
    1009,   1007,   1005,   1003,   1001,    999,    997,    995,    993,    991, 
     989,    987,    986,    984,    982,    980,    978,    976,    975,    973, 
     971,    969,    967,    966,    964,    962,    960,    959,    957,    955, 
     953,    952,    950,    948,    947,    945,    943,    942,    940,    938, 
     937,    935,    934,    932,    930,    929,    927,    926,    924,    923, 
     921,    920,    918,    917,    915,    914,    912,    911,    909,    908, 
     906,    905,    903,    902,    900,    899,    898,    896,    895,    893, 
     892,    891,    889,    888,    886,    885,    884,    882,    881,    880, 
     878,    877,    876,    874,    873,    872,    870,    869,    868,    867, 
     865,    864,    863,    862,    860,    859,    858,    857,    855,    854, 
     853,    852,    850,    849,    848,    847,    846,    844,    843,    842, 
     841,    840,    839,    837,    836,    835,    834,    833,    832,    831, 
     830,    828,    827,    826,    825,    824,    823,    822,    821,    820, 
     819,    817,    816,    815,    814,    813,    812,    811,    810,    809, 
     808,    807,    806,    805,    804,    803,    802,    801,    800,    799, 
};

Step::Step()
{
    m_acccnt = 0;
    m_ldir = 0;
    m_rdir = 0;
    m_lhalf = 0;
    m_rhalf = 0;
    m_speed = 0.f;
    //m_speed = 0.6f;
}

Step::~Step()
{
}

bool Step::initialize(const StepPins& pins)
{
    m_pins.lclk = pins.lclk; // BCM 6 , Left CLK
    m_pins.ldir = pins.ldir; // BCM 13, Left DIR
    m_pins.rclk = pins.rclk; // BCM 19, Right CLK
    m_pins.rdir = pins.rdir; // BCM 26, Right DIR
    m_pins.enab = pins.enab; // BCM 20, Enable

    pinMode(m_pins.lclk, OUTPUT);
    pinMode(m_pins.ldir, OUTPUT);
    pinMode(m_pins.rclk, OUTPUT);
    pinMode(m_pins.rdir, OUTPUT);
    pinMode(m_pins.enab, OUTPUT);

    digitalWrite(m_pins.lclk, LOW);
    digitalWrite(m_pins.ldir, HIGH);
    digitalWrite(m_pins.rclk, LOW);
    digitalWrite(m_pins.rdir, LOW);
    digitalWrite(m_pins.enab, LOW);

    // half step (HH, 0.9도), quarter step (HL, 0.45도) ...
    //digitalWrite(m_pins.smm0, HIGH);
    //digitalWrite(m_pins.smm1, HIGH);

    return true;
}                           

void Step::finalize()
{                           
    digitalWrite(m_pins.lclk, LOW);
    digitalWrite(m_pins.ldir, HIGH);
    digitalWrite(m_pins.rclk, LOW);
    digitalWrite(m_pins.rdir, LOW);
    digitalWrite(m_pins.enab, LOW);
}

void Step::setSpeed(float speed)
{
    if(speed < 0 || speed >= 100) // 100%
        m_speed = 0;
    else
        m_speed = 100.0 / (100.f - speed);
}

void Step::pulse(int ldir, int rdir, int lhalf, int rhalf, int cnt)
{
    digitalWrite(m_pins.ldir, ldir);
    digitalWrite(m_pins.rdir, !rdir);

    if(m_lhalf != lhalf)
        m_lhalf = lhalf;

    if(m_rhalf != rhalf)
        m_rhalf = rhalf;

    digitalWrite(m_pins.enab, HIGH);

    for(int i = 0; i < cnt; i++)
    {
        bool l = (!lhalf || (i % 2));
        bool r = (!rhalf || (i % 2));

        if(m_acccnt > MAX_ACC - 1)
            m_acccnt = MAX_ACC - 1;

        int dura = (int)(g_accTable[m_acccnt] + ((float)g_accTable[m_acccnt] * m_speed));

        if(l) digitalWrite(m_pins.lclk, HIGH);
        if(r) digitalWrite(m_pins.rclk, HIGH);

        delayMicroseconds(dura);

        if(l) digitalWrite(m_pins.lclk, LOW);
        if(r) digitalWrite(m_pins.rclk, LOW);

        m_acccnt++;
    }

    digitalWrite(m_pins.enab, LOW);
}

void Step::stop(bool force)
{
    if(m_acccnt <= 0)
        return;

    if(force)
    {
        m_acccnt = 0;
        digitalWrite(m_pins.enab, LOW);
        digitalWrite(m_pins.ldir, HIGH);
        digitalWrite(m_pins.rdir, LOW);
        return;
    }

    digitalWrite(m_pins.enab, HIGH);

    while(m_acccnt > 0)
    {
        bool l = (!m_lhalf || (m_acccnt % 2));
        bool r = (!m_rhalf || (m_acccnt % 2));

        m_acccnt--;

        int dura = (int)(g_accTable[m_acccnt] + ((float)g_accTable[m_acccnt] * m_speed));

        if(l) digitalWrite(m_pins.lclk, HIGH);
        if(r) digitalWrite(m_pins.rclk, HIGH);

        delayMicroseconds(dura);

        if(l) digitalWrite(m_pins.lclk, LOW);
        if(r) digitalWrite(m_pins.rclk, LOW);
    }

    digitalWrite(m_pins.enab, LOW);
    digitalWrite(m_pins.ldir, HIGH);
    digitalWrite(m_pins.rdir, LOW);
}

void Step::fl()
{
    pulse(1, 1, 1, 0, 10);
}

void Step::ff()
{
    pulse(1, 1, 0, 0, 10);
}

void Step::fr()
{
    pulse(1, 1, 0, 1, 10);
}

void Step::ll()
{
    pulse(0, 1, 0, 0, 90);
    stop();
}

void Step::rr()
{
    pulse(1, 0, 0, 0, 90);
    stop();
}

void Step::bl()
{
    pulse(0, 0, 1, 0, 10);
}

void Step::bb()
{
    pulse(0, 0, 0, 0, 10);
}

void Step::br()
{
    pulse(0, 0, 0, 1, 10);
}

void Step::cc()
{
    pulse(0, 1, 0, 0, 10);
}

void Step::ut()
{
    pulse(0, 1, 0, 0, 180);
    stop();
}

void Step::cw()
{
    pulse(1, 0, 0, 0, 10);
}

} // namespace vox
