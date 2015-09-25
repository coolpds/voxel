#ifndef _VOX_EXE_VOX_SENSOR_HPP_
#define _VOX_EXE_VOX_SENSOR_HPP_

#include "dht.hpp"
#include "usonic.hpp"
#include "thread.hpp"


namespace vox
{

#define LIMIT_F (40.0f)
#define LIMIT_B (4.0f)
#define LIMIT_L (20.0f)
#define LIMIT_R (20.0f)

class VoxSensor : public Thread
{

public:
    static VoxSensor& getInstance();
    bool start();
    void stop();
    void run();
    float getTemperature() { return m_temp; }
    float getHumidity() { return m_humi; }
    float getDistF() { return m_dist_f; }
    float getDistB() { return m_dist_b; }
    float getDistL() { return m_dist_l; }
    float getDistR() { return m_dist_r; }
    bool isFreeze() { return m_freeze; }

protected:
    VoxSensor();
    virtual ~VoxSensor();

private:
    static VoxSensor m_instance;
    Dht m_dht;
    USonic m_us_f;
    USonic m_us_b;
    USonic m_us_l;
    USonic m_us_r;

    volatile float m_temp; // 온도
    volatile float m_humi; // 습도
    volatile float m_dist_f;
    volatile float m_dist_b;
    volatile float m_dist_l;
    volatile float m_dist_r;
    volatile bool m_freeze;

};

} // namespace vox

#endif // _VOX_EXE_VOX_SENSOR_HPP_
