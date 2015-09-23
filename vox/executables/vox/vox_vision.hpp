#ifndef _VOX_EXE_VOX_VISION_HPP_
#define _VOX_EXE_VOX_VISION_HPP_

#include <vector>
#include "thread.hpp"
#include "opencv/cvaux.h"
#include "opencv/highgui.h"
#include "opencv/cxcore.h"

//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/core/core.hpp"

namespace vox
{

typedef struct _ball_info
{
    float posx;
    float posy;
    float radi;
} BallInfo;

class VoxVision : public Thread
{

public:
    static VoxVision& getInstance();
    bool start();
    void stop();
    void run();
    int getPosX() { return (int)m_ball_x; }
    int getPosY() { return (int)m_ball_y; }
    float getRadius() { return m_ball_r; }
    int getBallCount() { return m_ball_cnt; }
    void setFollowing(int onoff) { m_follow = onoff; }
    int isFollowing() { return m_follow; }

protected:
    VoxVision();
    virtual ~VoxVision();

private:
    bool detectBalls(std::vector<BallInfo>& balls);

private:
    static VoxVision m_instance;
    //CvCapture* m_cap;
    //cv::VideoCapture* m_cap;
    std::vector<BallInfo> m_balls;
    volatile float m_ball_x;
    volatile float m_ball_y;
    volatile float m_ball_r; // 반경
    volatile int m_ball_cnt;
    volatile int m_follow;
};

} // namespace vox

#endif // _VOX_EXE_VOX_VISION_HPP_
