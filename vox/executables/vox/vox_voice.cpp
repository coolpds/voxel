#include <time.h> 
#include <dirent.h>
#include <curl/curl.h>
#include <map>

#include "document.h"
#include "prettywriter.h"
#include "filestream.h"
#include "stringbuffer.h"

#include "vox_voice.hpp"
#include "vox_control.hpp"
#include "vox_player.hpp"
#include "vox_sensor.hpp"
#include "vox_main.hpp"

namespace vox
{

using namespace rapidjson;

VoxVoice::VoxVoice()
{
    pthread_mutex_init(&m_cs, NULL);

    m_callcnt = 0;
    m_ready = 0;
    m_req = "";
    m_rsp = "";
    m_calcstr = "";
    curl_global_init(CURL_GLOBAL_ALL);
    setThreadName("VOCE");

    m_vcs.push_back({READY, "시작"});
    m_vcs.push_back({READY, "준비"});
    m_vcs.push_back({READY, "지금시작"});
    m_vcs.push_back({DANCE, "춤춰봐"});
    m_vcs.push_back({DANCE, "댄스시작"});
    m_vcs.push_back({MUSIC, "음악틀어라"});
    m_vcs.push_back({MUSIC, "음악시작해라"});
    m_vcs.push_back({MUSIC, "뮤직스타트"});
    m_vcs.push_back({MUSIC, "다른음악틀어라"});
    m_vcs.push_back({MUSIC, "다른음악시작해라"});
    m_vcs.push_back({MUSIC, "다음음악시작해라"});
    m_vcs.push_back({MUSIC, "다음뮤직시작해라"});
    m_vcs.push_back({MUSIC, "딴음악시작"});
    m_vcs.push_back({MUSIC, "노래시작해라"});
    m_vcs.push_back({MUSIC, "노래틀어라"});
    m_vcs.push_back({CLEAR, "지워"});
    m_vcs.push_back({CLEAR, "클리어"});
    m_vcs.push_back({CLEAR, "화면지워"});
    m_vcs.push_back({SMILE, "웃음"});
    m_vcs.push_back({SMILE, "웃어봐"});
    m_vcs.push_back({SMILE, "하하하하하하"});
    m_vcs.push_back({CRYING, "울음"});
    m_vcs.push_back({CRYING, "울어봐"});
    m_vcs.push_back({CRYING, "으앙"});
    m_vcs.push_back({CRYING, "으아앙"});
    m_vcs.push_back({SIREN, "사이렌켜"});
    m_vcs.push_back({SIREN, "싸이렌켜"});
    m_vcs.push_back({TEMPER, "현재온도는"});
    m_vcs.push_back({TEMPER, "현재기온은"});
    m_vcs.push_back({TEMPER, "온도는몇도"});
    m_vcs.push_back({TEMPER, "기온은몇도"});
    m_vcs.push_back({HUMIDITY, "현재습도는"});
    m_vcs.push_back({HUMIDITY, "지금습도는"});
    m_vcs.push_back({HUMIDITY, "습도는"});
    m_vcs.push_back({DORIDORI, "도리도리해봐"});
    m_vcs.push_back({DORIDORI, "도리도리도리"});
    m_vcs.push_back({HEADBANG, "헤드뱅뱅"});
    m_vcs.push_back({HEADBANG, "헤드뱅잉"});
    m_vcs.push_back({HEADBANG, "해드뱅뱅"});
    m_vcs.push_back({HEADBANG, "해드뱅잉"});
    m_vcs.push_back({HEADBANG, "푸쳐핸섭"});
    m_vcs.push_back({HEADBANG, "퓨쳐핸섭"});
    m_vcs.push_back({HEADBANG, "풋쳐핸섭"});
    m_vcs.push_back({CLASSIC, "클래식음악켜"});
    m_vcs.push_back({CLASSIC, "클래식음악시작해"});
    m_vcs.push_back({CLASSIC, "클래식음악틀어"});
    m_vcs.push_back({LOVEU, "사랑해"});
    m_vcs.push_back({LOVEU, "너를사랑해"});
    m_vcs.push_back({DATE, "오늘날짜는"});
    m_vcs.push_back({DATE, "오늘몇일이야"});
    m_vcs.push_back({DATE, "오늘은무슨요일이야"});
    m_vcs.push_back({DATE, "무슨요일이야"});
    m_vcs.push_back({TIME, "현재시각은몇시야"});
    m_vcs.push_back({TIME, "지금시각은몇시야"});
    m_vcs.push_back({TIME, "현재시간은몇시야"});
    m_vcs.push_back({TIME, "지금시간은몇시야"});
    m_vcs.push_back({TIME, "지금몇시야"});
    m_vcs.push_back({ALLSTOP, "모두중지"});
    m_vcs.push_back({ALLSTOP, "그만해"});
    m_vcs.push_back({ALLSTOP, "이제그만해"});
    m_vcs.push_back({ALLSTOP, "중지해"});
    m_vcs.push_back({ALLSTOP, "음악꺼버려라"});
    m_vcs.push_back({ALLSTOP, "음악중지해라"});
    m_vcs.push_back({ALLSTOP, "시끄러워요"});
    m_vcs.push_back({ALLSTOP, "조용히해라"});
    m_vcs.push_back({SHUTDOWN, "죽어버려라"});
    m_vcs.push_back({SHUTDOWN, "시스템종료"});
    m_vcs.push_back({REBOOT, "시스템다시시작해"});
    m_vcs.push_back({REBOOT, "다시시작해"});
    m_vcs.push_back({REBOOT, "재시작해"});
    m_vcs.push_back({REBOOT, "시스템재시작해"});
    m_vcs.push_back({REBOOT, "리붓"});
    m_vcs.push_back({REBOOT, "리부트"});
}

VoxVoice::~VoxVoice()
{
    pthread_mutex_destroy(&m_cs);
    curl_global_cleanup();
}

VoxVoice& VoxVoice::getInstance()
{
    static VoxVoice m_instance;
    return m_instance;
}

std::string VoxVoice::getReq()
{
    pthread_mutex_lock(&m_cs);
    std::string r = m_req;
    strReplace(r, "|", "");
    pthread_mutex_unlock(&m_cs);

    return r;
}

std::string VoxVoice::getRsp()
{
    pthread_mutex_lock(&m_cs);
    std::string r = m_rsp;
    strReplace(r, "|", "");
    pthread_mutex_unlock(&m_cs);
    
    return r;
}

void VoxVoice::setReq(const std::string& r)
{
    pthread_mutex_lock(&m_cs);
    m_req = r;
    pthread_mutex_unlock(&m_cs);
}
void VoxVoice::setRsp(const std::string& r)
{
    pthread_mutex_lock(&m_cs);
    m_rsp = r;
    pthread_mutex_unlock(&m_cs);
}

bool VoxVoice::start()
{
    return Thread::start(true, THREAD_DEFAULT_STACKSIZE);
}

void VoxVoice::stop()
{
    resetEvent();

    if(Thread::isRunning())
    {
        if(g_main.isExit())
            Thread::kill(SIGHUP);

        Thread::stop(true);
    }
}

void VoxVoice::getPlayList(const std::string& path, std::vector<std::string>& ret)
{
    DIR* dir = NULL;
    dirent* ent = NULL;

    dir = opendir(path.c_str());

    if(!dir)
        return;

    while((ent = readdir(dir)))
    {
        const std::string name = std::string(ent->d_name);
        size_t pos = name.rfind('.');

        if(pos != std::string::npos)
        {
            const std::string ext = strToLower(name.substr(pos + 1));

            if(ext == "mp3" || ext == "flac" || ext == "wav")
            {
                ret.push_back(name);
                //const std::string title = name.substr(0, pos);
                //ret.push_back(title);
            }
        }
    }

    closedir(dir);
}

// speech to txt
std::string VoxVoice::s2t(const std::string& jstr)
{
    if(jstr.empty())
        return "";

    try
    {
        Document jdoc;

        if(jdoc.Parse<0>(jstr.c_str()).HasParseError())
        {
            ERROR("(VOCE) jdoc parse error:%zu %s",
                jdoc.GetErrorOffset(), jdoc.GetParseError());
            return "";
        }

        if(!jdoc.HasMember("result"))
        {
            ERROR("(VOCE) json no result");
            return "";
        }

        const Value& r = jdoc["result"];

        if(!r.IsArray())
        {
            ERROR("(VOCE) result member is not array");
            return "";
        }

        std::vector<std::string> transcripts;
        std::vector<double> confidences;

        for(SizeType i = 0; i < r.Size(); i++)
        {
            if(!r[i].HasMember("alternative"))
                continue;

            const Value& alt = r[i]["alternative"];

            if(!alt.IsArray())
            {
                ERROR("(VOCE) alternative member is not array");
                return "";
            }

            for(SizeType j = 0; j < alt.Size(); j++)
            {
                const Value& obj = alt[j];
                std::string trans = "";
                double cons = 0.f;

                for(Value::ConstMemberIterator it = obj.MemberBegin(); it != obj.MemberEnd(); it++)
                {
                    if(std::string(it->name.GetString()) == "transcript" && it->value.IsString())
                        trans = strTrim(it->value.GetString());
                    else if(std::string(it->name.GetString()) == "confidence" && it->value.IsDouble())
                        cons = it->value.GetDouble();
                }

                if(!trans.empty() || cons > 0.f)
                {
                    transcripts.push_back(trans);
                    confidences.push_back(cons);

                    NOTICE("trans: '%s', cons: '%.6f'", trans.c_str(), cons);
                }
            }
        }

        if(!transcripts.empty() && !confidences.empty())
        {
            //if(confidences[0] > 0.f)
                return transcripts[0];
        }
    }
    catch(...)
    {
        ERROR("(VOCE) unexpected exception caught");
        return "";
    }
   
    return "";
}

std::string VoxVoice::simsimi(const std::string& q)
{
    //std::string url = "http://sandbox.api.simsimi.com/request.p";
    //url += "?key=9a3e0abd-6b0a-42e8-bc7b-86ad1627e5e1";
    //url += "&lc=ko&ft=1.0&text=" + encodeUrl(q);
    std::string url = "http://api.simsimi.com/request.p";
    url += "?key=";
    url += "&lc=ko&ft=1.0&text=" + encodeUrl(q);

    std::string rsphdr = "";
    std::string content = "";
    std::string rsp = "";
    
    if(!httpRequest(url, std::map<std::string, std::string>{}, rsphdr, content))
    {
        ERROR("httpRequest() failed, q='%s'", q.c_str());
        return "";
    }

    INFORMATION("rsphdr=%s", rsphdr.c_str());
    INFORMATION("content=%s", content.c_str());

    try
    {
        Document jdoc;

        if(jdoc.Parse<0>(content.c_str()).HasParseError())
        {
            ERROR("(VOCE) jdoc parse error:%zu %s",
                jdoc.GetErrorOffset(), jdoc.GetParseError());
            return "";
        }

        if(!jdoc.HasMember("response"))
        {
            ERROR("(VOCE) json no result");
            return "";
        }

        const Value& r = jdoc["response"];

        if(!r.IsString())
        {
            ERROR("(VOCE) result member is not string");
            return "";
        }

        rsp = std::string(r.GetString());

        NOTICE("rsp=%s", rsp.c_str());

        return rsp;
    }
    catch(...)
    {
        ERROR("(VOCE) unexpected exception caught");
        return "";
    }

    return "";
}

bool VoxVoice::record()
{
    /** wav
    /usr/bin/sox -q -v 4 -t alsa hw:1,0 /dev/shm/cmd.wav rate 16k silence 0 1 0:00:02 3% > /dev/null 2>&1
    /usr/bin/sox -q /dev/shm/cmd.wav /dev/shm/vad.wav vad gain 6 > /dev/null 2>&1
    **/
    /** flac
    AUDIODEV=hw:1,0 AUDIODRIVER=alsa /usr/bin/rec -t flac /dev/shm/cmd.flac rate 16k silence 0 1 0:00:02 3%
    /usr/bin/sox /dev/shm/cmd.flac /dev/shm/vad.flac vad
    **/
    std::string cmd = "/usr/local/bin/record_flac.sh";
    std::string ret = "";
    
    m_ready = 1;

    if(!createProcess(cmd, ret))
    {
        m_ready = 0;
        ERROR("(VOCE) exec '%s' failed", cmd.c_str());
        return false;
    }

    m_ready = 0;

    ssize_t sz = getFileSize("/dev/shm/vad.flac");
    
    if(sz < 512) // 512k
    {
        NOTICE("(VOCE) vad size '%zd' limit", sz);

        if(sz == (-1))
            delay(200);

        return false;
    }

    return true;
}

#define USER_AGENT "Mozilla/4.0 (compatible; MSIE 5.01; Windows NT 5.0)"
#define GOOGLE_APIKEY1 ""
#define GOOGLE_APIKEY2 ""
#define GOOGLE_APIKEY3 ""

std::string VoxVoice::stt()
{
    CURL* ctx = NULL;
    CURLcode rc = CURLE_OK;
    curl_slist* rsphdr = NULL;
    std::string key = GOOGLE_APIKEY1;
    //int callcnt = ((++m_callcnt) % 3);
    //if(callcnt == 1) key = ""GOOGLE_APIKEY2;
    //else if(callcnt == 2) key = ""GOOGLE_APIKEY3;
    const std::string req = "https://www.google.com/speech-api/v2/recognize?output=json&lang=ko-KR&maxresults=10&key=" + key;
    const std::string path = "/dev/shm/cmd.json";
    std::string rstr = "";
    std::string jstr = "";
    std::string line = "";
    size_t pos = std::string::npos;
    FILE* fp = fopen(path.c_str(), "wb");

    if(!fp)
        return "";

    curl_httppost* formpost = NULL;
    curl_httppost* lastptr = NULL;
    curl_slist* headers = NULL;
 
    curl_formadd(&formpost,
               &lastptr,
               CURLFORM_COPYNAME, "sendfile",
               CURLFORM_FILE, "/dev/shm/cmd.flac",
               CURLFORM_END);

    if(!(ctx = curl_easy_init()))
        goto error_success;

    headers = curl_slist_append(headers, "Expect:");
    headers = curl_slist_append(headers, "Content-Type: audio/x-flac; rate=16000");

    curl_easy_setopt(ctx, CURLOPT_URL, req.c_str());
    curl_easy_setopt(ctx, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(ctx, CURLOPT_HTTPPOST, formpost);
    curl_easy_setopt(ctx, CURLOPT_USERAGENT, "curl/7.26.0");
    curl_easy_setopt(ctx, CURLOPT_NOPROGRESS, 1L);
    curl_easy_setopt(ctx, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(ctx, CURLOPT_WRITEHEADER , stderr);
    curl_easy_setopt(ctx, CURLOPT_WRITEDATA, fp);

    rc = curl_easy_perform(ctx);

    if(rc != CURLE_OK)
    {
        ERROR("curl error '%s'", curl_easy_strerror(rc));
        goto error_success;
    }

    fflush(fp);

    jstr = getFileContents(path);
    pos = jstr.find("\n");

    if(pos != std::string::npos)
    {
        line = jstr.substr(0, pos);

        if(line.find("{\"result\":[]}") == 0)
            jstr = jstr.substr(pos);
    }

    if(!strTrim(jstr).empty())
        rstr = s2t(jstr);

error_success:

    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }

    if(ctx)
    {
        curl_easy_cleanup(ctx);
        ctx = NULL;
    }

    if(formpost)
    {
        curl_formfree(formpost);
        formpost = NULL;
    }

    if(headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    return rstr;
}

void VoxVoice::tts(const std::string& text, bool en)
{
    if(text.empty())
        return;

    const std::string q = encodeUrl(text);
    const std::string mp3 = "/dev/shm/tts.mp3";
    std::string req = "";

#if 0 // google api
    if(en)
        req = "http://translate.google.com/translate_tts?tl=en&ie=UTF-8&client=t&q=" + q;
    else
        req = "http://translate.google.com/translate_tts?tl=ko&ie=UTF-8&client=t&q=" + q;

    NOTICE("tts_req=%s", req.c_str());
#endif

    // naver openapi
    req = "https://openapi.naver.com/v1/voice/tts.bin";

    std::string postdata = "speaker=mijin&speed=0&text=" + q;
    CURL* ctx = NULL;
    curl_slist* headers = NULL;
    CURLcode rc = CURLE_OK;
    FILE* fp = fopen(mp3.c_str(), "wb");
    long retcode = 0;

    if(!fp)
        return;
    
    if(!(ctx = curl_easy_init()))
        goto error_success;

#if 0 // google api
    curl_easy_setopt(ctx, CURLOPT_URL, req.c_str());
    curl_easy_setopt(ctx, CURLOPT_USERAGENT, USER_AGENT);
    curl_easy_setopt(ctx, CURLOPT_NOPROGRESS, 1L);
    //curl_easy_setopt(ctx, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(ctx, CURLOPT_WRITEHEADER , stderr);
    curl_easy_setopt(ctx, CURLOPT_WRITEDATA, fp);
    //curl_easy_setopt(ctx, CURLOPT_WRITEFUNCTION, VoxVoice::playStream);
#endif

    // naver openapi
    curl_easy_setopt(ctx, CURLOPT_URL, req.c_str());
    curl_easy_setopt(ctx, CURLOPT_USERAGENT, USER_AGENT);

    //curl_easy_setopt(ctx, CURLOPT_POSTFIELDSIZE, postdata.size());
    curl_easy_setopt(ctx, CURLOPT_POSTFIELDS, postdata.c_str());

    headers = curl_slist_append(headers, "X-Naver-Client-Id: EsdO2_lPUhoEdXHyujeU");
    headers = curl_slist_append(headers, "X-Naver-Client-Secret: 7zgm4WZScz");
    curl_easy_setopt(ctx, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(ctx, CURLOPT_NOPROGRESS, 1L);
    //curl_easy_setopt(ctx, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(ctx, CURLOPT_WRITEHEADER , stderr);
    curl_easy_setopt(ctx, CURLOPT_WRITEDATA, fp);
    //curl_easy_setopt(ctx, CURLOPT_WRITEFUNCTION, VoxVoice::playStream);

    rc = curl_easy_perform(ctx);
    
    if(rc != CURLE_OK)
    {
        ERROR("curl error '%s'", curl_easy_strerror(rc));
        goto error_success;
    }

    fflush(fp);

    curl_easy_getinfo(ctx, CURLINFO_HTTP_CODE, &retcode);

    NOTICE("(VOCE) HTTP response code: %d", retcode);

    if(retcode == 200)
    {
        //VoxPlayer::getInstance().play(mp3); // naver-tts mp3 포맷 이상?
        execProcess("/usr/bin/mpg123 " + mp3);
    }
    else
        WARNING("(VOCE) unexpected http status");

error_success:

    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }

    if(headers)
    {
        curl_slist_free_all(headers);
        headers = NULL;
    }

    if(ctx)
    {
        curl_easy_cleanup(ctx);
        ctx = NULL;
    }
}

// 그럭저럭..
std::string VoxVoice::calcTwoNums(const std::string& req, int op)
{
    std::string first = "";
    std::string second = "";
    double ans = 0;

    if(!extractSubString(first, req.c_str(), 0, ' '))
        return "";

    if(!extractSubString(second, req.c_str(), 1, ' '))
        return "";

    if(first.empty() || second.empty())
        return "";

    if(first.find(".") != std::string::npos || second.find(".") != std::string::npos)
    {
        if(op == PLUS)
            ans = atof(first.c_str()) + atof(second.c_str());
        else if(op == MINUS)
            ans = atof(first.c_str()) - atof(second.c_str());
        else if(op == MULTI)
            ans = atof(first.c_str()) * atof(second.c_str());
        else if(op == DIVI)
        {
            if(atof(second.c_str()) > 0.0)
                ans = atof(first.c_str()) / atof(second.c_str());
        }

        return strFormat("정답은 %.2f 입니다", ans);
    }
    else
    {
        if(op == PLUS)
            ans = atoi(first.c_str()) + atoi(second.c_str());
        else if(op == MINUS)
            ans = atoi(first.c_str()) - atoi(second.c_str());
        else if(op == MULTI)
            ans = atoi(first.c_str()) * atoi(second.c_str());
        else if(op == DIVI)
        {
             if(atoi(second.c_str()) > 0.0)
                ans = atoi(first.c_str()) / atoi(second.c_str());
        }

        return strFormat("정답은 %d 입니다", (int)ans);
    }

    return "";
}

int VoxVoice::findVoiceCommand(const std::string& nos)
{
    if(nos.empty())
        return VC::NONE;

    std::string str = nos;

    if(str.find("더하기") != std::string::npos || str.find("플러스") != std::string::npos)
    {
        strReplace(str, "더하기", " ");
        strReplace(str, "플러스", " ");
        m_calcstr = calcTwoNums(str, PLUS);
    }
    else if(str.find("빼기") != std::string::npos || str.find("마이너스") != std::string::npos)
    {
        strReplace(str, "빼기", " ");
        strReplace(str, "마이너스", " ");
        m_calcstr = calcTwoNums(str, MINUS);
    }
    else if(str.find("곱하기") != std::string::npos)
    {
        strReplace(str, "곱하기", " ");
        m_calcstr = calcTwoNums(str, MULTI);
    }
    else if(str.find("나누기") != std::string::npos)
    {
        strReplace(str, "나누기", " ");
        m_calcstr = calcTwoNums(str, DIVI);
    }

    NOTICE("(VOCE) calcstr='%s' nos='%s'", m_calcstr.c_str(), nos.c_str());

    if(!m_calcstr.empty())
        return VC::CALCULATION;

    for(size_t i = 0; i < m_vcs.size(); i++)
    {
        if(m_vcs[i].qry.find(nos) == 0)
            return m_vcs[i].cmd;
    }

    return VC::UNKNOWN;
}

void VoxVoice::run()
{
    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    std::string req = "";
    std::string rsp = "";
    std::string nos = "";
    int ncmd = VC::NONE;

    const std::string ostpath = "/home/pi/rpi/mp3/ost";
    std::vector<std::string> pls;
    getPlayList(ostpath, pls);

    INFORMATION("(VOCE) start of loop");

    while(!g_main.isExit())
    {
        if(g_main.getMode() != MD3)
        {
            if(!getReq().empty())
                setReq("");

            if(!getRsp().empty())
                setRsp("");

            delay(100);
            continue;
        }

#if 0
        if(0)
        {
            std::string text = "123456789";
            tts(text);
            VoxPlayer::getInstance().play("/dev/shm/tts.mp3");
            sleep(10);
            continue;
        }

        if(0)
        {
            std::string r = stt();
            NOTICE("R=%s", r.c_str());
            sleep(10);
            continue;
        }

        if(0)
        {
            std::string q = "안녕하세요";
            simsimi(q);
            sleep(10);
            continue;
        }
#endif

        if(!record())
        {
            delay(50);
            continue;
        }

        if(g_main.isExit())
            break;

        nos = req = stt();
        strReplace(nos, " ", "");

        if(g_main.isExit())
            break;

        if(nos.empty())
            continue;

        NOTICE("req -> '%s'", req.c_str());

        setReq(req);

        ncmd = findVoiceCommand(nos);

        switch(ncmd)
        {
            case READY:
            {
                setRsp("무엇을 도와드릴까요?");
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/korean_help.mp3");
                break;
            }
            case MUSIC:
            {
                NOTICE("pls.size()=%zu", pls.size());

                if(pls.empty())
                {
                    setRsp("아직 준비중입니다");
                    tts("아직 준비중입니다");
                }
                else
                {
                    timeval tv = {0, 0};
                    gettimeofday(&tv, NULL);

                    size_t rd = tv.tv_usec % pls.size();

                    const std::string name = pls[rd];
                    const std::string title = name.substr(0, name.rfind('.'));

                    NOTICE("name=%s", name.c_str());
                    NOTICE("title=%s", title.c_str());

                    setRsp(title);
                    tts(title, true);

                    delay(500);
                    VoxPlayer::getInstance().play(ostpath + "/" + name, true);
                }
                break;
            }
            case CLEAR:
            {
                setReq("");
                setRsp("");
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/korean_yes.mp3");
                break;
            }
            case SMILE:
            {
                setRsp("푸하하하하하~~");
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/laugh_laugh13.mp3");
                break;
            }
            case CRYING:
            {
                setRsp("으앙~~");
                VoxPlayer::getInstance().play("/home/pi/rpi/vox/mp3/primitive_cry01.mp3");
                break;
            }
            case SIREN:
            {
                setRsp("~~~~~~~~");
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/siren3.mp3");
                break;
            }
            case TEMPER:
            {
                std::string t = strFormat("현재 온도는 %.1f도 입니다",
                    VoxSensor::getInstance().getTemperature());
                setRsp(t);
                tts(t);
                break;
            }
            case HUMIDITY:
            {
                std::string h = strFormat("현재 습도는 %.1f%% 입니다",
                    VoxSensor::getInstance().getHumidity());
                setRsp(h);
                tts(h);
                break;
            }
            case DORIDORI:
            {
                Servo* servo = VoxControl::getInstance().getServo();

                servo->setNeutral();
                delay(100);

                setRsp("Put Your Hands Up~~~~");
                tts("Put Your Hands Up", true);

                servo->left(45);
                delay(100);
                
                for(int i = 0; i < 5; i++)
                {
                    servo->right(90);
                    delay(100);
                    servo->left(90);
                    delay(100);
                }

                servo->right(90);
                delay(100);
                servo->setNeutral();
                delay(100);
                break;
            }
            case DANCE:
            case HEADBANG:
            {
                Servo* servo = VoxControl::getInstance().getServo();

                servo->setNeutral();
                delay(50);

                setRsp("푸처핸섭~~~~");
                tts("푸처핸섭");
                delay(1500);

                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/run_to_u.mp3", true);
                delay(2000);

                for(int i = 0; i < 6; i++)
                {
                    servo->up(10);
                    delay(400);
                    servo->down(10);
                    delay(400);
                    servo->down(10);
                    delay(400);
                    servo->up(10);
                    delay(400);
                }

                servo->setNeutral();
                delay(100);
                servo->up(20);
                delay(800);
                servo->down(40);
                delay(800);
                servo->up(40);
                delay(400);
                servo->down(40);
                delay(400);
                servo->up(40);
                delay(400);

                servo->setNeutral();
                delay(50);
                servo->down(10);
                delay(400);

                for(int i = 0; i < 38; i++)
                {
                    servo->up(20);
                    delay(50);
                    servo->down(20);
                    delay(50);
                }

                servo->setNeutral();
                delay(50);

                servo->left(30);
                delay(50);
                servo->down(30);
                delay(50);
                
                for(int i = 0; i < 8; i++)
                {
                    servo->right(60);
                    delay(50);
                    servo->up(60);
                    delay(50);
                    servo->left(60);
                    delay(50);
                    servo->down(60);
                    delay(50);
                }

                servo->setNeutral();
                delay(100);

                servo->down(5);
                delay(50);

                for(int i = 0; i < 34; i++)
                {
                    servo->up(10);
                    delay(50);
                    servo->down(10);
                    delay(50);
                }

                VoxPlayer::getInstance().play("", false);
                delay(1000);

                servo->setNeutral();
                delay(50);

                servo->up(20);
                delay(500);

                servo->down(40);
                delay(1000);

                servo->setNeutral();
                delay(50);
                break;
            }
            case CLASSIC:
            {
                break;
            }
            case LOVEU:
            {
                setRsp("나도 사랑해!");
                tts("나도 사랑해!");
                break;
            }
            case TIME:
            {
                std::string r = "현재 시각은 " + getCurrentTime() + " 입니다";
                setRsp(r);
                tts(r);
                break;
            }
            case DATE:
            {
                std::string r = "오늘은 " + getCurrentDate() + " 입니다";
                setRsp(r);
                tts(r);
                break;
            }
            case CALCULATION:
            {
                if(m_calcstr.empty())
                {
                    setRsp("아직 준비중입니다");
                    tts("아직 준비중입니다");
                }
                else
                {
                    setRsp(m_calcstr);
                    tts(m_calcstr);
                    m_calcstr = "";
                }
                break;
            }
            case ALLSTOP:
            {
                setRsp("얼음!");
                Servo* servo = VoxControl::getInstance().getServo();
                servo->setNeutral();
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/korean_yes.mp3");
                break;
            }
            case SHUTDOWN:
            {
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/korean_yes.mp3");
                delay(1000);
                execProcess("sudo shutdown -h now");
                break;
            }
            case REBOOT:
            {
                VoxPlayer::getInstance().play("/home/pi/rpi/mp3/korean_yes.mp3");
                delay(1000);
                execProcess("sudo reboot");
                break;
            }
            default:
            {
                rsp = simsimi(req);
                setRsp(rsp);
                tts(rsp);
                break;
            }
        }

        delay(10);
    }

    m_callcnt = 0;
    m_ready = 0;

    INFORMATION("(VOCE) end of loop");
}

} // namespace vox
