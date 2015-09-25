#ifndef _VOX_EXE_VOX_VOICE_HPP_
#define _VOX_EXE_VOX_VOICE_HPP_

#include <vector>
#include "thread.hpp"


namespace vox
{

typedef struct _vc_item
{
    int cmd;
    std::string qry;
} VCItem;

class VoxVoice : public Thread
{
public:
    enum VC
    {
        UNKNOWN = -1,
        NONE = 0,
        READY = 1,
        DANCE,
        MUSIC,
        CLEAR,
        SMILE,
        CRYING,
        SIREN,
        TEMPER,
        HUMIDITY,
        DORIDORI,
        HEADBANG,
        CLASSIC,
        LOVEU,
        DATE,
        TIME,
        ALLSTOP,
        CALCULATION,
        SHUTDOWN,
        REBOOT,
    };

    enum OP
    {
        PLUS = 1,
        MINUS,
        MULTI,
        DIVI,
    };

    static VoxVoice& getInstance();
    bool start();
    void stop();
    void run();
    int readyToGo() { return m_ready; }
    std::string getReq();
    std::string getRsp();
    void setReq(const std::string& r);
    void setRsp(const std::string& r);
    int findVoiceCommand(const std::string& nos);

protected:
    VoxVoice();
    virtual ~VoxVoice();

private:
    std::string s2t(const std::string& jstr);
    std::string simsimi(const std::string& q);
    bool record();
    std::string stt();
    void tts(const std::string& text, bool en = false);
    void getPlayList(const std::string& path, std::vector<std::string>& ret);
    std::string calcTwoNums(const std::string& nos, int op);

private:
    static VoxVoice m_instance;
    pthread_mutex_t m_cs;
    int m_callcnt;
    std::string m_req;
    std::string m_rsp;
    std::string m_calcstr;
    volatile int m_ready; // 레코딩 준비 상태
    std::vector<VCItem> m_vcs;
};

} // namespace vox

#endif // _VOX_EXE_VOX_VOICE_HPP_
