#ifndef _VOX_LIB_EXCEPTION_EXCEPTION_HPP_
#define _VOX_LIB_EXCEPTION_EXCEPTION_HPP_

#include <cstdio>
#include <string>

namespace vox
{

#define TRY_EXCEPTIONS try {

#define THROW_ERRORS(err, format, ...) \
    { \
        char str[2048]; \
        std::snprintf(str, sizeof(str), format, ## __VA_ARGS__); \
        char str2[256]; \
        std::snprintf(str2, sizeof(str2), " {%s:%d}", __FILE__, __LINE__); \
        throw Exception{err, std::string{str} + str2, false}; \
    }

#define THROW_EXCEPTIONS(format, ...) \
        THROW_ERRORS(0, format, ## __VA_ARGS__) \

#define CATCH_EXCEPTIONS(retval) \
    } \
    catch(const Exception& ex) \
    { \
        ex.reason(); \
        return retval; \
    } \
    catch(const std::bad_alloc& ba) \
    { \
        Exception{ba.what(), true}.reason(); \
        return retval; \
    }

#define CATCH_EXCEPTIONS_BEGIN \
    } \
    catch(const Exception& ex) \
    { \
        ex.reason();

#define CATCH_EXCEPTIONS_END(retval) \
        return retval; \
    }

#define THROW(format, ...) \
    { \
        char str[2048]; \
        std::snprintf(str, sizeof(str), format, ## __VA_ARGS__); \
        throw Exception{str, false}; \
    }

class Exception
{
public:
    Exception(int code, const std::string& msg, bool trace)
    {
        m_code = code;
        m_msg = msg;

        if (trace == true)
            backtrace();
    }

    Exception(int code, const char* msg, bool trace)
    {
        Exception(code, msg ? std::string(msg) : "", trace);
    }

    Exception(const char* msg, bool trace)
    {
        Exception(0, msg, trace);
    }
    
    Exception(const std::string& msg, bool trace)
    {
        Exception(0, msg, trace);
    }

    ~Exception() { };

    int code() const { return m_code; }
    const std::string& message() const { return m_msg; }
    const char* what() const { return m_msg.c_str(); }
    virtual void reason() const;

private:
    void backtrace();

private:
    int m_code;
    std::string m_msg;
    std::string m_trace;
};

} // namespace vox

#endif // _VOX_LIB_EXCEPTION_EXCEPTION_HPP_
