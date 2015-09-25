#ifndef _VOX_LIB_SENSOR_DHT_HPP_
#define _VOX_LIB_SENSOR_DHT_HPP_

#include <string>

namespace vox
{

class Dht
{

public:
    Dht();
    virtual ~Dht();
    bool initialize(int dhtpin, int maxtimings = 83);
    void finalize();
    bool readDhtData(float* humi, float* temp);

private:
    int m_dhtpin; // 입,출력
    int m_maxtimings;
};

} // namespace vox

#endif // _VOX_LIB_SENSOR_DHT_HPP_
