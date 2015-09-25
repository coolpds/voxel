#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "wiringPi.h"
#include "trace.hpp"
#include "util.hpp"
#include "dht.hpp"


namespace vox
{

Dht::Dht()
{
    m_dhtpin = (-1);
    m_maxtimings = 83;
}

Dht::~Dht()
{
}

bool Dht::initialize(int dhtpin, int maxtimings)
{
    if(dhtpin < 0 || maxtimings < 0)
        return false;

    m_dhtpin = dhtpin;
    m_maxtimings = maxtimings;

    pinMode(m_dhtpin, OUTPUT);
    digitalWrite(m_dhtpin, LOW);

    return true;
}

void Dht::finalize()
{
    if(m_dhtpin > 0)
        digitalWrite(m_dhtpin, LOW) ;
}

bool Dht::readDhtData(float* humi, float* temp)
{
    if(m_dhtpin < 0 || m_maxtimings < 0)
        return false;

    int data[5] = {0, };
    uint8_t laststate = HIGH;
    uint8_t counter = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t flag = HIGH;
    uint8_t state = 0;

    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = 0;
    data[4] = 0;

    *humi = 0.f;
    *temp = 0.f;

    pinMode(m_dhtpin, OUTPUT);
    
    digitalWrite(m_dhtpin, LOW);
    delay(18);
    digitalWrite(m_dhtpin, HIGH);
    delayMicroseconds(30);

    pinMode(m_dhtpin, INPUT);

    for(i = 0; i < m_maxtimings; i++)
    {
        counter = 0;

        while(digitalRead(m_dhtpin) == laststate)
        {
            counter++;
            delayMicroseconds(1);

            if(counter == 200)
                break;
        }

        laststate = digitalRead(m_dhtpin);

        if(counter == 200) // if while breaked by timer, break for
            break;

        if((i >= 4) && (i % 2 == 0))
        {
            data[j / 8] <<= 1;

            if(counter > 20)
                data[j / 8] |= 1;

            j++;
        }
    }

    if((j >= 40) && (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xff)))
    {
        (*humi) = atof(strFormat("%d.%d", data[0], data[1]).c_str());
        (*temp) = atof(strFormat("%d.%d", data[2], data[3]).c_str());
        //NOTICE("humidity = %d.%d %% Temperature = %d.%d *C", data[0], data[1], data[2], data[3]);
    }
    else
        return false;

    return true;
}

} // namespace vox
