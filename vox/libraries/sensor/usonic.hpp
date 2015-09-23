#ifndef _VOX_LIB_SENSOR_USONIC_HPP_
#define _VOX_LIB_SENSOR_USONIC_HPP_

#include <string>

namespace vox
{

#define DEF_USNIC_TIMEOUT (50000) // 0.05s

class USonic
{

public:
    USonic();
    virtual ~USonic();
    bool initialize(int echo, int trig, unsigned int  burst = 10 /*us*/);
    void finalize();
    float getDistance();

private:
    int m_echo; // 입력
    int m_trig; // 출력
    int m_burst; // 에코 트리거 burst 시간

};

} // namespace vox

#endif // _VOX_LIB_SENSOR_USONIC_HPP_
