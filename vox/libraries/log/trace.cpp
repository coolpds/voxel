#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <cstdarg>
#include <cassert>
#include <array>

#include "trace.hpp"
#include "util.hpp"


namespace vox
{

Trace::Trace()
{
    m_enabled = false;
    m_level = LEVEL_QUIET;
    m_filename = "";
    m_file = NULL;
    m_cloexec = true;
}

Trace::~Trace()
{
}

void Trace::closeWithoutLock()
{
    if(m_enabled == true)
    {
        if(m_file != nullptr)
        {
            std::fflush(m_file);
            std::fclose(m_file);
            m_file = nullptr;
        }
        if(m_level != LEVEL_QUIET)
            m_level = LEVEL_QUIET;
        m_enabled = false;
    }
    assert(m_file == nullptr);
    assert(m_level == LEVEL_QUIET);
}

bool Trace::open(const std::string& filename, TraceLevel level, bool cloexec)
{
    std::lock_guard<std::mutex> lock{m_mutex};

    if(level == LEVEL_HOLD)
        level = m_level;

    closeWithoutLock();
    assert(m_enabled == false);

    if(level == LEVEL_QUIET || filename.empty() == true)
        return false;

    std::FILE*const file = std::fopen(filename.c_str(), "a+");
    if (file == nullptr)
        return false;

    if(cloexec == true)
    {
        const auto fd = fileno(file);
        auto val = fcntl(fd, F_GETFD, 0);
        
        if(val < 0)
        {
            std::fclose(file);
            return false;
        }

        if(fcntl(fd, F_SETFD, val | FD_CLOEXEC) < 0)
        {
            std::fclose(file);
            return false;
        }

        if(std::setvbuf(file, nullptr, _IOLBF, 0) != 0)
        {
            std::fclose(file);
            return false;
        }
    }

    m_enabled = true;
    m_level = level;
    m_filename = filename;
    m_file = file;
    m_cloexec = cloexec;
    return true;
}

bool Trace::reopen()
{
    return open(m_filename, m_level);
}

bool Trace::reopen(const std::string& filename, TraceLevel level)
{
    return open(filename, (level != LEVEL_HOLD) ? level : m_level);
}

void Trace::close()
{
    std::lock_guard<std::mutex> lock{m_mutex};
    closeWithoutLock();
}

int Trace::getLevel() const
{
    std::lock_guard<std::mutex> lock{m_mutex};
    const auto level = (int)m_level;
    return level;
}

bool Trace::setLevel(TraceLevel level)
{
    if(level == LEVEL_HOLD)
        return true;

    if(level <= LEVEL_QUIET)
    {
        close();
        return true;
    }

    if(m_enabled == false)
    {
        if(m_filename.empty() == false)
            return open(m_filename, level, m_cloexec);
    }
    else
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        m_level = ((level > LEVEL_DEBUG) ? LEVEL_DEBUG : level);
    }
    return true;
}

bool Trace::setLogPath(const char* logpath)
{
    if(logpath == nullptr || m_enabled == true)
        return false;

    m_filename = logpath != nullptr ? logpath : "";
    return true;
}

const char* Trace::getLogPath() const
{
    return m_filename.c_str();
}

namespace
{
    constexpr std::array<const char*, 6> LVSTR
    {{" (Q) ", " (E) ", " (W) ", " (N) ", " (I) ", " (D) "}};
}

bool Trace::print(TraceLevel level, const char* file, int line,
                  const char* format, ...)
{
    if(m_enabled == false || level < LEVEL_ERROR || level > m_level)
        return false;

    std::va_list ap;
    va_start(ap, format);
    char buf[8156];
    std::vsnprintf(buf, sizeof (buf), format, ap);
    va_end(ap);

    char srcbuf[256];
    std::snprintf(srcbuf, sizeof (srcbuf), " {%s:%d}", file, line);

    assert(level >= 0 && level < LVSTR.size());
    const auto msg = getCurrentTimeString() + LVSTR[level] +
        buf + srcbuf + "\n";

    std::size_t n{0};
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        if(m_file != nullptr)
            n = std::fwrite(msg.c_str(), sizeof (char), msg.size(), m_file);
    }
    return (n >= msg.size());
}

bool Trace::dump(TraceLevel level, const char* file, int line,
                 const char* buf, int buflen, const char* format, ...)
{
    if(m_enabled == false || level < LEVEL_ERROR || level > m_level)
        return false;

    std::va_list ap;
    va_start(ap, format);
    char msgbuf[8156];
    std::vsnprintf(msgbuf, sizeof (msgbuf), format, ap);
    va_end(ap);

    char srcbuf[256];
    std::snprintf(srcbuf, sizeof (srcbuf), " {%s:%d}", file, line);

    assert(level >= 0 && level < LVSTR.size());
    const auto msg = getCurrentTimeString() + LVSTR[level] +
        msgbuf + srcbuf + "\n" + dumpString(buf, buflen, 16) + "\n";

    std::size_t n{0};
    {
        std::lock_guard<std::mutex> lock{m_mutex};
        if(m_file != nullptr)
            n = std::fwrite(msg.c_str(), sizeof (char), msg.size(), m_file);
    }
    return (n >= msg.size());
}

} // namespace vox
