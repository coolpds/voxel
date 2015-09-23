#include "vox_vision.hpp"
#include "vox_control.hpp"
#include "vox_sensor.hpp"
#include "vox_main.hpp"

namespace vox
{

using namespace cv;

VoxVision::VoxVision()
{
    m_ball_x = (-1.0);
    m_ball_y = (-1.0);
    m_ball_r = (-1.0);
    m_ball_cnt = 0;
    m_follow = 0;

    setThreadName("VISI");
}

VoxVision::~VoxVision()
{
}

VoxVision& VoxVision::getInstance()
{
    static VoxVision m_instance;
    return m_instance;
}

bool VoxVision::start()
{
    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);
}

void VoxVision::stop()
{
    resetEvent();

    if(Thread::isRunning())
    {
        if(g_main.isExit())
            Thread::kill(SIGHUP);

        Thread::stop(true);
    }
}

bool VoxVision::detectBalls(std::vector<BallInfo>& balls)
{
    try
    {
        if(!balls.empty())
            balls.clear();

        FILE* fp = fopen("/dev/shm/picam.jpg", "rb");
        if(!fp)
            return false;

        int fd = fileno(fp);

        fileLock(fd);
        IplImage* frame = cvLoadImage("/dev/shm/picam.jpg", CV_LOAD_IMAGE_COLOR);
        fileUnlock(fd);

        fclose(fp);

        if(!frame)
        {
            ERROR("(VISI) frame is null");
            return false;
        }

        //CvSize sz = cvSize(1280, 960);
        CvSize sz = cvSize(640, 480);
        CvScalar hsv_min = cvScalar(43, 105, 103, 0); // hsv
        CvScalar hsv_max = cvScalar(76, 255, 255, 0);

        IplImage* hsv_frame = cvCreateImage(sz, IPL_DEPTH_8U, 3);
        IplImage* threshold = cvCreateImage(sz, IPL_DEPTH_8U, 1);

        cvCvtColor(frame, hsv_frame, CV_BGR2HSV);
        cvInRangeS(hsv_frame, hsv_min, hsv_max, threshold);
        CvMemStorage* storage = cvCreateMemStorage(0);
        cvSmooth(threshold, threshold, CV_GAUSSIAN, 9, 9);
        CvSeq* circles = cvHoughCircles(threshold, storage, CV_HOUGH_GRADIENT, 2,
                            threshold->height / 4, 100, 50, 5, 120);

        for(int i = 0; i < circles->total; i++)
        {
            float* p = (float*)cvGetSeqElem(circles, i);
            BallInfo bi = { p[0], p[1], p[2] };
            balls.push_back(bi);

            INFORMATION("(VISI) ball[%d/%d]: x(%f) y(%f) r(%f)",
                i, circles->total, p[0], p[1], p[2]);
        }

        if(storage)
        {
            cvReleaseMemStorage(&storage);
            storage = NULL;
        }

        if(threshold)
        {
            cvReleaseImage(&threshold);
            threshold = NULL;
        }

        if(hsv_frame)
        {
            cvReleaseImage(&hsv_frame);
            hsv_frame = NULL;
        }

        if(frame)
        {
            cvReleaseImage(&frame);
            frame = NULL;
        }
    }
    catch(const cv::Exception& e)
    {
        WARNING("(VISI) cv::exception caught, '%s'", e.what());
        return false;
    }
    catch(...)
    {
        WARNING("(VISI) unexpected exception caught");
        return false;
    }

    return true;
}

void VoxVision::run()
{
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    std::vector<BallInfo> balls;

    INFORMATION("(VISI) start of loop");

    while(!g_main.isExit())
    {

        if(g_main.getMode() != MD2)
        {
            m_ball_x = (-1.0);
            m_ball_y = (-1.0);
            m_ball_r = (-1.0);
            m_ball_cnt = 0;
            m_follow = 0;

            delay(200);
            continue;
        }

        if(!detectBalls(balls))
        {
            delay(200);
            continue;
        }

        if(balls.empty())
        {
            m_ball_x = (-1.0);
            m_ball_y = (-1.0);
            m_ball_r = (-1.0);
            m_ball_cnt = 0;
        }
        else
        {
            // 첫 번째 오브젝트만 검출..
            m_ball_x = balls[0].posx;
            m_ball_y = balls[0].posy;
            m_ball_r = balls[0].radi;
            m_ball_cnt = (int)balls.size();
        }

        if(m_follow && m_ball_cnt == 1)
        {
            // 640 x 480
            if(m_ball_r > 10 && m_ball_r < 80)
            {
                Servo* servo = VoxControl::getInstance().getServo();

                if(m_ball_y > 380)
                {
                    servo->down(10);
                }

                else if(m_ball_y < 100)
                {
                    servo->up(10);
                }

                else if(m_ball_x < 200)
                {
                    servo->left(10);
                }

                else if(m_ball_x > 440)
                {
                    servo->right(10);
                }
            }
            /*else
            {
                Step* step = VoxControl::getInstance().getStep();

                for(int i = 0; i < 2; i++)
                {
                    if(!VoxSensor::getInstance().isFreeze())
                        step->ff();
                    else
                        step->stop(true);
                }
            }*/
        }

        delay(50);
    }

    INFORMATION("(VISI) end of loop");
}

} // namespace vox
