#include <cstdlib>
#include <cstring>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>

#include "trace.hpp"
#include "exception.hpp"


namespace vox
{

void Exception::backtrace()
{
    if (Trace::getInstance().getLevel() >= Trace::LEVEL_ERROR)
    {
        m_trace = std::string{"\nCall Stack:"};

        constexpr int MAX_TRACE_DEPTH{32};
        void* trace[MAX_TRACE_DEPTH];
        const auto size = ::backtrace(trace, MAX_TRACE_DEPTH);

        for (auto i = 2; i < size; ++i) // 진입점 출력 제한
        {
            Dl_info dlinfo;
            if (!dladdr(trace[i], &dlinfo))
                continue;

            int stat;
            const char* symname = dlinfo.dli_sname;
            char* demangled = abi::__cxa_demangle(symname, nullptr, 0, &stat);
            if (stat == 0 && demangled != nullptr)
                symname = demangled;

            m_trace += "\nobj: ";
            m_trace += dlinfo.dli_fname != nullptr ? dlinfo.dli_fname : "??";

            if (symname != nullptr)
            {
                m_trace += ", func: ";
                m_trace += symname;
            }

            if (demangled != nullptr)
                std::free(demangled);
        }
    }
}

void Exception::reason() const
{
    if (m_code != 0)
        ERROR("%d:%s. %s%s", m_code, std::strerror(m_code), m_msg.c_str(),
                m_trace.c_str());
    else
        ERROR("%s%s", m_msg.c_str(), m_trace.c_str());
}

} // namespace vox

