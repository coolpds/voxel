#ifndef _VOX_LIB_LOG_TRACE_HPP_
#define _VOX_LIB_LOG_TRACE_HPP_

#include <cstdio>
#include <string>
#include <mutex>


namespace vox
{

#ifdef DOTRACE
#undef DOTRACE
#endif
#define DOTRACE(level, file, line, format, ...) \
    Trace::getInstance().print(level, file, line, format, ## __VA_ARGS__)

#ifdef DODUMP
#undef DODUMP
#endif
#define DODUMP(level, file, line, buf, buflen, format, ...) \
    Trace::getInstance().dump(level, file, line, buf, buflen, format, ## __VA_ARGS__)

#ifdef ERROR
#undef ERROR
#endif
#define ERROR(format, ...) \
    DOTRACE(Trace::LEVEL_ERROR, __FILE__, __LINE__, format, ## __VA_ARGS__)

#ifdef WARNING
#undef WARNING
#endif
#define WARNING(format, ...) \
    DOTRACE(Trace::LEVEL_WARN, __FILE__, __LINE__, format, ## __VA_ARGS__)

#ifdef NOTICE
#undef NOTICE
#endif
#define NOTICE(format, ...) \
    DOTRACE(Trace::LEVEL_NOTICE, __FILE__, __LINE__, format, ## __VA_ARGS__)

#ifdef INFORMATION
#undef INFORMATION
#endif
#define INFORMATION(format, ...) \
    DOTRACE(Trace::LEVEL_INFO, __FILE__, __LINE__, format, ## __VA_ARGS__)

#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG(format, ...) \
    DOTRACE(Trace::LEVEL_DEBUG, __FILE__, __LINE__, format, ## __VA_ARGS__)

#ifdef ERRORDUMP
#undef ERRORDUMP
#endif
#define ERRORDUMP(buf, buflen, format, ...) \
    DODUMP(Trace::LEVEL_ERROR, __FILE__, __LINE__, buf, buflen, format, ## __VA_ARGS__)

#ifdef NOTICEDUMP
#undef NOTICEDUMP
#endif
#define NOTICEDUMP(buf, buflen, format, ...) \
    DODUMP(Trace::LEVEL_NOTICE, __FILE__, __LINE__, buf, buflen, format, ## __VA_ARGS__)

#ifdef DEBUGDUMP
#undef DEBUGDUMP
#endif
#define DEBUGDUMP(buf, buflen, format, ...) \
    DODUMP(Trace::LEVEL_DEBUG, __FILE__, __LINE__, buf, buflen, format, ## __VA_ARGS__)

#ifdef NETERROR
#undef NETERROR
#endif
#define NETERROR(errcode, format, ...) \
    do { m_errcode = errcode; m_errmsg = strFormat(format " {%s:%d}", ## __VA_ARGS__, __FILE__, __LINE__); } while(0)

#ifdef ALERT_CRIT
#undef ALERT_CRIT
#endif
#define ALERT_CRIT(code, name, url, format, ...) \
    Trace::getInstance().alert(Trace::ALERT_CRIT, code, name, url, format, ## __VA_ARGS__)

#ifdef ALERT_ERROR
#undef ALERT_ERROR
#endif
#define ALERT_ERROR(code, name, url, format, ...) \
    Trace::getInstance().alert(Trace::ALERT_ERROR, code, name, url, format, ## __VA_ARGS__)

#ifdef ALERT_WARN
#undef ALERT_WARN
#endif
#define ALERT_WARN(code, name, url, format, ...) \
    Trace::getInstance().alert(Trace::ALERT_WARN, code, name, url, format, ## __VA_ARGS__)


// Trace 로그를 출력하기 위한 클래스
class Trace
{
public:
    enum TraceLevel
    {
        LEVEL_HOLD = -1,    // 이전 레벨 유지(reopen)
        LEVEL_QUIET = 0,    // 안찍어
        LEVEL_ERROR = 1,    // 에러
        LEVEL_WARN,         // 경고
        LEVEL_NOTICE,       // 상태
        LEVEL_INFO,         // 정보
        LEVEL_DEBUG         // 디버그
    };

    enum AlertLevel
    {
        ALERT_NONE = 0,
        ALERT_CRIT = 1,
        ALERT_ERROR,
        ALERT_WARN
    };

    static Trace& getInstance()
    {
        static Trace t;
        return t;
    }

    bool open(const std::string& filename, TraceLevel level = LEVEL_QUIET,
            bool cloexec = true);
    bool reopen();
    bool reopen(const std::string& filename, TraceLevel level = LEVEL_HOLD);
    void close();
    bool print(TraceLevel level, const char* file, int line,
            const char* format, ...);
    bool dump(TraceLevel level, const char* file, int line,
            const char* buf, int buflen, const char* format, ...);
    int getLevel() const;
    bool setLevel(TraceLevel level);
    bool setLogPath(const char* logpath);
    const char* getLogPath() const;

private:
    Trace();
    ~Trace();
    void closeWithoutLock();

private:
    bool m_enabled;
    TraceLevel m_level;
    std::string m_filename;
    std::FILE* m_file;
    bool m_cloexec;
    mutable std::mutex m_mutex;
    //AlertLevel m_alert;
    //std::string m_alerturl;
};

} // namespace vox

#endif // _VOX_LIB_LOG_TRACE_HPP_
