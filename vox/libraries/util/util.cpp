#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <fcntl.h>
#include <glob.h>
#include <errno.h>
#include <iconv.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/klog.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <cstdlib>
#include <iconv.h>
#include <regex.h>
#include <cassert>
#include <cstring>
//#include <boost/tr1/random.hpp>

#include "trace.hpp"
#include "socket.hpp"
#include "util.hpp"


namespace vox
{

size_t humanizeNums2Bytes(const std::string& nums)
{
    const std::string s = strToLower(nums);
    const double t = std::atof(nums.c_str());
    
    if(s.find("kb") != std::string::npos)
        return (size_t)(t * 1024);
    else if(s.find("mb") != std::string::npos)
        return (size_t)(t * 1024 * 1024);
    else if(s.find("gb") != std::string::npos)
        return (size_t)(t * 1024 * 1024 * 1024);
    else if(s.find("tb") != std::string::npos)
        return (size_t)(t * 1024 * 1024 * 1024 * 1024);
    else if(s.find("pb") != std::string::npos)
        return (size_t)(t * 1024 * 1024 * 1024 * 1024 * 1024);
    else if(s.find("eb") != std::string::npos)
        return (size_t)(t * 1024 * 1024 * 1024 * 1024 * 1024 * 1024);
    else if (s.find("k") != std::string::npos)
        return (size_t)(t * 1000);
    else if (s.find("m") != std::string::npos)
        return (size_t)(t * 1000 * 1000);
    else if (s.find("g") != std::string::npos)
        return (size_t)(t * 1000 * 1000 * 1000);
    else if (s.find("t") != std::string::npos)
        return (size_t)(t * 1000 * 1000 * 1000 * 1000);
    else if (s.find("p") != std::string::npos)
        return (size_t)(t * 1000 * 1000 * 1000 * 1000 * 1000);
    else if (s.find("e") != std::string::npos)
        return (size_t)(t * 1000 * 1000 * 1000 * 1000 * 1000 * 1000);
    else if(s == "0" || s == "unlimited" || t < 0.f)
        return 0;
    else if(s != "0" && t > 0.f)
        return (size_t)(t);
    
    return 0;
}

bool isFileExistence(const char* path)
{
    if(!path)
        return false;

    struct stat fs;

    if(lstat(path, &fs) < 0)
        return false;

    return true;
}

bool isPositiveNumber(const std::string& str)
{
    if(str.find_first_not_of("0123456789") == std::string::npos)
        return true;

    return false;
}

int makeSubDirectory(const std::string& path, mode_t mode)
{
    size_t pre = 0;
    size_t pos = 0;
    std::string p = path;
    std::string dir = "";
    int ret = 0;

    if(p[p.size() - 1] != '/')
        p += '/';

    while((pos = p.find_first_of('/', pre)) != std::string::npos)
    {
        dir = p.substr(0, pos++);
        pre = pos;
        
        if(dir.size() == 0)
            continue;
        
        if((ret = mkdir(dir.c_str(), mode)) && errno != EEXIST)
            return ret;
    }

    return ret;
}

void removeDirectory(const std::string& path)
{
    DIR* dir = NULL;
    dirent* ent = NULL;

    dir = opendir(path.c_str());

    if(dir)
    {
        while((ent = readdir(dir)))
        {
            if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
                continue;

            std::string fname = path + "/" + std::string(ent->d_name);
            struct stat fs;
            
            if(lstat(fname.c_str(), &fs) == (-1))
            {
                WARNING("lstat(%s) failed(%d,%s)",
                    fname.c_str(), errno, strerror(errno));
                continue;
            }

            if(S_ISDIR(fs.st_mode))
            {
                removeDirectory(fname);
            }
            else
            {
                if(unlink(fname.c_str()) == (-1))
                    WARNING("unlink(%s) failed(%d,%s)",
                        fname.c_str(), errno, strerror(errno));
            }
        }

        if(rmdir(path.c_str()) == (-1))
            WARNING("rmdir(%s) failed(%d,%s)",
                path.c_str(), errno, strerror(errno));

        closedir(dir);
    }
}

std::string readLink(const std::string& path)
{
    std::string retpath = path;
    struct stat fs;

    if(lstat(path.c_str(), &fs) == (-1))
        return retpath;
    
    if(S_ISLNK(fs.st_mode))
    {
        char link[1024] = {0, };
        ssize_t len = (-1);

        if((len = readlink(retpath.c_str(), link, 1023)) == (-1))
            return retpath;

        link[len] = '\0';

        retpath = std::string(link);
    }

    return retpath;
}

std::string realPath(const std::string& path)
{
    std::string retpath = path;
    char pathbuf[PATH_MAX + 1] = {0, };
    
    if(realpath(path.c_str(), pathbuf))
        retpath = std::string(pathbuf);

    return retpath;
}

int fileLock(int fd)
{
    int e = 0;

    while(true)
    {
        e = flock(fd, LOCK_EX);
        
        if(e < 0)
        {
            if(errno == EINTR)
                continue;
        }

        break;
    }

    return e;
}

int fileUnlock(int fd)
{
    int e = 0;

    while(true)
    {
        e = flock(fd, LOCK_UN);
        
        if(e < 0)
        {
            if(errno == EINTR)
                continue;
        }

        break;
    }

    return e;
}

std::string strTrim(const std::string& s, const char* target)
{
    if(s.length() == 0)
        return s;

    return (strTrimLeft(strTrimRight(s, target), target));
}

std::string strTrimLeft(const std::string& s, const char* target)
{
    uint32_t f;

    if(s.length() == 0)
        return s;

    if(target == NULL)
        f = (uint32_t)(s.find_first_not_of(" \t\r\n"));
    else
        f = (uint32_t)(s.find_first_not_of(target));

    if(f == (uint32_t)std::string::npos)
        return "";

    return std::string(s, f);
}

std::string strTrimRight(const std::string& s, const char* target)
{
    uint32_t e;

    if(s.length() == 0)
        return s;

    if(target == NULL)
        e = (uint32_t)(s.find_last_not_of(" \t\r\n"));
    else
        e = (uint32_t)(s.find_last_not_of(target));

    if(e == (uint32_t)std::string::npos)
        return "";

    return std::string(s, 0, e + 1);
}

std::string strToLower(const std::string& instr)
{
    std::string outstr(instr);

    for(std::string::iterator it = outstr.begin(); it != outstr.end(); ++it)
    {
        if(isupper(*it))
            *it = tolower(*it);
    }

    return outstr;
}

std::string strToUpper(const std::string& instr)
{
    std::string outstr(instr);

    for(std::string::iterator it = outstr.begin(); it != outstr.end(); ++it)
    {
        if(islower(*it))
            *it = toupper(*it);
    }

    return outstr;
}

bool extractSubString(std::string& str, const char* src, int idx, char sep)
{
    const char* end = NULL;
    int len = 0;

    str.clear();

    if(src == NULL)
        return false;

    while(idx--)
    {
        src = strchr(src, sep);

        if(src == NULL)
            return false;

        src++;
    }

    end = strchr((char*)src, sep);
    len = (end == NULL) ? (int)(strlen(src)) : (int)(end - src);

    str.insert(0, src, len * sizeof(char));

    return true;
}

void splitString(const std::string& str, std::vector<std::string>& result, const std::string& delm)
{
    size_t idx1 = str.find_first_not_of(delm, 0);
    size_t idx2 = str.find_first_of(delm, idx1);

    if(!result.empty())
        result.clear();

    while(std::string::npos != idx2 || std::string::npos != idx1)
    {
        result.push_back(str.substr(idx1, idx2 - idx1));
        idx1 = str.find_first_not_of(delm, idx2);
        idx2 = str.find_first_of(delm, idx1);
    }
}

void splitStringWithQuot(const std::string& str, std::vector<std::string>& result, const std::string& delm)
{
    size_t len = str.length();
    size_t pos = 0;
    std::string token = "";
    char ch1 = 0;
    char ch2 = 0;
    bool sq_open = false;
    bool dq_open = false;
    bool ac_open = false;

    if(!result.empty())
        result.clear();
    
    while(len > pos)
    {
        ch1 = str.at(pos);

        if(ch1 == '"')
        {
            if(sq_open || ac_open)
            {
                token.push_back(ch1);
                ++pos;
                continue;
            }

            if(dq_open)
            {
                result.push_back(token);
                token = "";

                dq_open = false;
            }
            else
            {
                dq_open = true;
            }
        }
        else if(ch1 == '\'')
        {
            if(dq_open || ac_open)
            {
                token.push_back(ch1);
                ++pos;
                continue;
            }

            if(sq_open)
            {
                result.push_back(token);
                token = "";

                sq_open = false;
            }
            else
            {
                sq_open = true;
            }
        }
        else if(ch1 == '`')
        {
            if(dq_open || sq_open)
            {
                token.push_back(ch1);
                ++pos;
                continue;
            }

            if(ac_open)
            {
                result.push_back(token);
                token = "";

                ac_open = false;
            }
            else
            {
                ac_open = true;
            }
        }
        else if(ch1 == '\\')
        {
            if(sq_open || dq_open || ac_open)
            {
                ++pos;
                ch2 = (len > pos) ? str.at(pos) : 0;

                if(ch2 == '"' && dq_open)
                {
                    token.push_back('"');
                }
                else if(ch2 == '\'' && sq_open)
                {
                    token.push_back('\'');
                }
                else if(ch2 == '`' && ac_open)
                {
                    token.push_back('`');
                }
                else if(ch2 == '\\')
                {
                    token.push_back('\\');
                    token.push_back('\\');
                }
                else
                {
                    token.push_back(ch1);
                    token.push_back(ch2);
                }
            }
            else
            {
                token.push_back(ch1);
            }
        }
        else
        {
            if(sq_open || dq_open || ac_open)
            {
                token.push_back(ch1);
            }
            else
            {

                if(std::string::npos == delm.find_first_of(ch1))
                {
                    token.push_back(ch1);
                }
                else
                {
                    if(!token.empty())
                    {
                        result.push_back(token);
                        token = "";
                    }
                }
            }
        }

        ++pos;
    }

    if(!token.empty())
    {
        result.push_back(token);
        token = "";
    }
}

void parsePostParam(const std::string& str, std::map<std::string, std::string>& result)
{
    if(!result.empty())
        result.clear();

    std::vector<std::string> params;
    
    splitString(str, params, "&");

    for(size_t i = 0; i < params.size(); i++)
    {
        std::string name = "";
        std::string value = "";

        extractSubString(name, params[i].c_str(), 0, '=');
        extractSubString(value, params[i].c_str(), 1, '=');

        name = decodeUrl(name);
        value = decodeUrl(value);

        result[name] = value;
    }
}

bool strReplace(std::string& str, std::string from, std::string to)
{
    std::string::size_type pos = 0;

    int fromlen = from.size();
    int tolen = to.size();

    bool r = false;
    while((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, fromlen, to);
        pos += tolen;
        if (r == false)
            r = true;
    }
    return r;
}

void replaceAmpChar(std::string& str)
{
    strReplace(str, "&", "&amp;");
    strReplace(str, "<", "&lt;");
    strReplace(str, ">", "&gt;");
    strReplace(str, "\"", "&quot;");
    strReplace(str, "'", "&apos;");
}

void restoreAmpChar(std::string& str)
{
    strReplace(str, "&apos;", "'");
    strReplace(str, "&quot;", "\"");
    strReplace(str, "&gt;", ">");
    strReplace(str, "&lt;", "<");
    strReplace(str, "&amp;", "&");
}

void strRemove(std::string& str, const char* chars)
{
    std::string::size_type pos = 0;
    
    while((pos = str.find_first_of(chars, pos)) != std::string::npos)
        str.erase(pos, 1);
}

std::string strFormat(const char* format, ...)
{
    std::string result = "";
    va_list ap;

    if(format == NULL)
        return "";

    va_start(ap, format);
    result = strFormatV(format, ap);
    va_end(ap);

    return result;
}

std::string strFormatV(const char* format, va_list ap)
{
    std::string result = "";
    int n = 0;
    int len = 8192;
    char* buf = NULL;
    va_list apdst;

    if((buf = (char *)malloc(len)) == NULL)
        return "";

    while(1)
    {
        va_copy(apdst, ap);
        n = vsnprintf(buf, len, format, apdst);
        va_end(apdst);

        if(n > -1 && n < len)
        {
            result = std::string(buf);
            break;
        }

        if(n > -1) // glibc 2.1
            len = n + 1;
        else // glibc 2.0
            len *= 2;

        if((buf = (char *)realloc(buf, len)) == NULL)
            break;
    }

    if(buf)
    {
        free(buf);
        buf = NULL;
    }

    return result;
}

int aToIntOpt(const char* str)
{
    int ret = 0;
    
    if(!str)
        return 0;
    
    if(strlen(str) > 2 && strncmp(str, "0x", 2) == 0)
        sscanf(str, "%x", &ret);
    else
        ret = atoi(str);

    return ret;
}

bool wildMatch(const char* wild, const char* text)
{
    const char* cp = NULL;
    const char* mp = NULL;

    while((*text) && (*wild != '*'))
    {
        if ((*wild != *text) && (*wild != '?'))
            return false;

        wild++;
        text++;
    }

    while(*text)
    {
        if(*wild == '*')
        {
            if (!*++wild)
                return true;

            mp = wild;
            cp = text + 1;
        }
        else if((*wild == *text) || (*wild == '?'))
        {
            wild++;
            text++;
        }
        else
        {
            wild = mp;
            text = cp++;
        }
    }

    while(*wild == '*')
        wild++;

    return !*wild;
}

int encodeUrl(const char* instr, std::string& outstr)
{
    register unsigned char* outtmp = NULL;
    register unsigned char* intmp = NULL;
    unsigned char* outbuf = NULL;
    unsigned char* inbuf = (unsigned char*)instr;
    const int len = strlen(instr) + 1;

    outstr.clear();

    try
    {
        outstr.resize(len * 3 - 2, ' ');
        outbuf = (unsigned char*)outstr.c_str();

        if(outbuf)
        {
            intmp = inbuf;
            outtmp = outbuf;

            while(*intmp)
            {
                if(isalnum(*intmp))
                {
                    *outtmp++ = *intmp;
                }
                else
                {
                    *outtmp++ = '%';
                    *outtmp++ = toHex(*intmp >> 4);
                    *outtmp++ = toHex(*intmp % 16);
                }
                intmp++;
            }
            *outtmp = '\0';
        }
    }
    catch(...)
    {
        return 0;
    }

    outstr.resize(strlen(outstr.c_str()));

    return (int)outstr.size();
}

std::string encodeUrl(const std::string& instr)
{
    std::string outstr = "";

    if(encodeUrl(instr.c_str(), outstr) > 0)
        return outstr;

    return "";
}

int decodeUrl(const char* instr, std::string& outstr)
{
    std::string tmpstr(instr);
    outstr.clear();

    for(int x = 0; x < (int)tmpstr.size(); ++x)
    {
        if(tmpstr.at(x) == '%' && x + 2 < (int)tmpstr.size() &&
            isHexDigit(tmpstr.at(x + 1)) && isHexDigit(tmpstr.at(x + 2)))
        {
            char hexstr[3];

            hexstr[2] = 0;
            strncpy(hexstr, tmpstr.substr(x + 1, 2).c_str(), 2);
            x = x + 2;
            outstr.append(1, (unsigned char)strtoul(hexstr, NULL, 16));
        }
        else
        {
            if(tmpstr.at(x) == '+')
                outstr.append(1, ' ');
            else
                outstr.append(1, tmpstr.at(x));
        }
    }

    return (int)outstr.size();
}

std::string decodeUrl(const std::string& instr)
{
    std::string outstr = "";

    if(decodeUrl(instr.c_str(), outstr) > 0)
        return outstr;

    return "";
}

int encodeUtf8(const char* instr, std::string& outstr)
{
    iconv_t icvd;
    size_t inleft;
    size_t outleft;
    char* inbuf;
    char* outbuf;

    inleft = strlen(instr);

    if(inleft <= 0)
        return 0;

    outstr.clear();
    outstr.resize(inleft * 3);

    outleft = outstr.size();

    inbuf = (char*)instr;
    outbuf = (char*)outstr.c_str();

    icvd = iconv_open("UTF-8", "CP949");
    if(icvd == (iconv_t)(-1))
        return 0;

    if(iconv(icvd, &inbuf, &inleft, &outbuf, &outleft) == (size_t)(-1))
    {
        iconv_close(icvd);
        return 0;
    }

    iconv_close(icvd);

    outstr.resize(strlen(outstr.c_str()));

    return outstr.size();
}

std::string encodeUtf8(const std::string& instr)
{
    std::string outstr = "";

    if(encodeUtf8(instr.c_str(), outstr) > 0)
        return outstr;

    return "";
}

int decodeUtf8(const char* instr, std::string& outstr)
{
    iconv_t icvd;
    size_t inleft;
    size_t outleft;
    char* inbuf;
    char* outbuf;

    inleft = strlen(instr);

    if(inleft <= 0)
        return 0;

    outstr.clear();
    outstr.resize(inleft * 3);

    outleft = outstr.size();

    inbuf = (char*)instr;
    outbuf = (char*)outstr.c_str();

    icvd = iconv_open("CP949", "UTF-8");
    if(icvd == (iconv_t)(-1))
        return 0;

    if(iconv(icvd, &inbuf, &inleft, &outbuf, &outleft) == (size_t)(-1))
    {
        iconv_close(icvd);
        return 0;
    }

    iconv_close(icvd);

    outstr.resize(strlen(outstr.c_str()));

    return outstr.size();
}

std::string decodeUtf8(const std::string& instr)
{
    std::string outstr = "";

    if(decodeUtf8(instr.c_str(), outstr) > 0)
        return outstr;

    return "";
}

std::string encodeBase64(const std::string& str)
{
    std::string retstr = "";
    std::vector<uint8_t> bytes;

    bytes.assign(str.begin(), str.end());

    encodeBase64(bytes, retstr);

    return retstr;
}

std::string decodeBase64(const std::string& str)
{
    std::string retstr = "";
    std::vector<uint8_t> bytes;

    decodeBase64(str, bytes);

    retstr.assign(bytes.begin(), bytes.end());

    return retstr;
}

void encodeBase64(const std::vector<uint8_t>& inbytes, std::string& outstr)
{
    const uint8_t* bytes = (const uint8_t*)&inbytes[0];
    size_t len = inbytes.size();
    int i = 0;
    int j = 0;
    uint8_t ch3[3] = {0, };
    uint8_t ch4[4] = {0, };

    if(!outstr.empty())
        outstr.clear();

    while(len--)
    {
        ch3[i++] = *(bytes++);

        if(i == 3)
        {
            ch4[0] = (ch3[0] & 0xfc) >> 2;
            ch4[1] = ((ch3[0] & 0x03) << 4) + ((ch3[1] & 0xf0) >> 4);
            ch4[2] = ((ch3[1] & 0x0f) << 2) + ((ch3[2] & 0xc0) >> 6);
            ch4[3] = ch3[2] & 0x3f;

            for(i = 0; i < 4; i++)
                outstr += base64Chars[ch4[i]];

            i = 0;
        }
    }

    if(i > 0)
    {
        for(j = i; j < 3; j++)
            ch3[j] = '\0';

        ch4[0] = (ch3[0] & 0xfc) >> 2;
        ch4[1] = ((ch3[0] & 0x03) << 4) + ((ch3[1] & 0xf0) >> 4);
        ch4[2] = ((ch3[1] & 0x0f) << 2) + ((ch3[2] & 0xc0) >> 6);
        ch4[3] = ch3[2] & 0x3f;

        for(j = 0; j < (i + 1); j++)
            outstr += base64Chars[ch4[j]];

        while((i++ < 3))
            outstr += '=';
    }
}

void decodeBase64(const std::string& instr, std::vector<uint8_t>& outbytes)
{
    size_t len = instr.size();
    int i = 0;
    int j = 0;
    int idx = 0;
    uint8_t ch3[3] = {0, };
    uint8_t ch4[4] = {0, };

    if(!outbytes.empty())
        outbytes.clear();

    while(len-- && (instr[idx] != '=') && isBase64(instr[idx]))
    {
        ch4[i++] = instr[idx]; idx++;
        
        if(i == 4)
        {
            for(i = 0; i < 4; i++)
                ch4[i] = base64Chars.find(ch4[i]);

            ch3[0] = (ch4[0] << 2) + ((ch4[1] & 0x30) >> 4);
            ch3[1] = ((ch4[1] & 0xf) << 4) + ((ch4[2] & 0x3c) >> 2);
            ch3[2] = ((ch4[2] & 0x3) << 6) + ch4[3];

            for(i = 0; i < 3; i++)
                outbytes.push_back(ch3[i]);
            
            i = 0;
        }
    }

    if(i > 0)
    {
        for(j = i; j < 4; j++)
            ch4[j] = 0;

        for(j = 0; j <4; j++)
            ch4[j] = base64Chars.find(ch4[j]);

        ch3[0] = (ch4[0] << 2) + ((ch4[1] & 0x30) >> 4);
        ch3[1] = ((ch4[1] & 0xf) << 4) + ((ch4[2] & 0x3c) >> 2);
        ch3[2] = ((ch4[2] & 0x3) << 6) + ch4[3];

        for(j = 0; j < (i - 1); j++)
            outbytes.push_back(ch3[j]);
    }
}

float timeSpan(const timespec& start, const timespec& end)
{
    return (float)((end.tv_sec - start.tv_sec) + ((end.tv_nsec - start.tv_nsec) / 1.e9));
}

float timeSpan(const timeval& start, const timeval& end)
{
    return (float)((end.tv_sec - start.tv_sec) + ((end.tv_usec - start.tv_usec) / 1.e6));
}

float timeSpan(const time_t start, const time_t end)
{
    return (float)(end - start);
}

std::string getCurrentTimeString()
{
    timeval tvcur = {0, 0};

    if(gettimeofday(&tvcur, NULL) == (-1))
        return std::string("");

    return getTimeString(tvcur);
}

std::string getTimeString(const timeval& tv)
{
    struct tm now;
    char tmbuf[32];

    if(!localtime_r(&tv.tv_sec, &now))
        return std::string("");

    snprintf(tmbuf, 31, "[%04d-%02d-%02d %02d:%02d:%02d.%06d]",
        1900 + now.tm_year, 1 + now.tm_mon, now.tm_mday,
        now.tm_hour, now.tm_min, now.tm_sec, (int)tv.tv_usec);

    return std::string(tmbuf);
}

std::string getTimeString(const timespec& ts)
{
    struct tm now;
    char tmbuf[32];

    if(!localtime_r(&ts.tv_sec, &now))
        return std::string("");

    snprintf(tmbuf, 31, "[%04d-%02d-%02d %02d:%02d:%02d.%06d]",
        1900 + now.tm_year, 1 + now.tm_mon, now.tm_mday,
        now.tm_hour, now.tm_min, now.tm_sec, (int)(ts.tv_nsec / 1.e3));

    return std::string(tmbuf);
}

std::string getTimeString(time_t t)
{
    timeval tv = {t, 0};
    struct tm now;
    char tmbuf[32];

    if(!localtime_r(&tv.tv_sec, &now))
        return std::string("");

    snprintf(tmbuf, 31, "%04d-%02d-%02d %02d:%02d:%02d",
        1900 + now.tm_year, 1 + now.tm_mon, now.tm_mday,
        now.tm_hour, now.tm_min, now.tm_sec);

    return std::string(tmbuf);
}

std::string getCurrentDate()
{
    time_t t = time(NULL);
    struct tm now;
    char tmbuf[80];

    if(!localtime_r(&t, &now))
        return std::string("");

    snprintf(tmbuf, 80, "%d년 %d월 %d일",
        1900 + now.tm_year, 1 + now.tm_mon, now.tm_mday);

    return std::string(tmbuf);
}

std::string getCurrentTime()
{
    time_t t = time(NULL);
    struct tm now;
    char tmbuf[80];

    if(!localtime_r(&t, &now))
        return std::string("");

    if(now.tm_hour == 0)
        snprintf(tmbuf, 80, "오전 %d시 %d분 %d초",
            12, now.tm_min, now.tm_sec);
    else if(now.tm_hour > 11)
        snprintf(tmbuf, 80, "오후 %d시 %d분 %d초",
            (now.tm_hour > 12) ? now.tm_hour - 12 : now.tm_hour, now.tm_min, now.tm_sec);
    else
        snprintf(tmbuf, 80, "오전 %d시 %d분 %d초",
            now.tm_hour, now.tm_min, now.tm_sec);

    return std::string(tmbuf);
}

// 시간 문자열로 부터 epoch-time을 반환한다.
// 시간 문자열은 다음 형식으로 지정할 수 있다.
// 20100121(YYYYmmdd), 20100121000000(YYYYmmddHHMMSS),
// 2010-01-21(YYYY-mm-dd), 2010-01-21 00:00:00(YYYY-mm-dd HH:MM:SS)
time_t timeString2EpochTime(const char* tmstr)
{
    if(!tmstr)
        return -1;

    std::string instr = std::string(tmstr);
    std::string tmpstr = "";
    struct tm tm;
    time_t epoch = -1;

    // YYYY-mm-dd
    if(instr.size() == 10 && instr.at(4) == '-' && instr.at(7) == '-')
    {
        tmpstr = instr.substr(0, 4);
        if(atoi(tmpstr.c_str()) < 1900 || atoi(tmpstr.c_str()) > 2100)
            return -1;
        tm.tm_year = atoi(tmpstr.c_str()) - 1900;

        tmpstr = instr.substr(5, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 12)
            return -1;
        tm.tm_mon = atoi(tmpstr.c_str()) - 1;

        tmpstr = instr.substr(8, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 31)
            return -1;
        tm.tm_mday = atoi(tmpstr.c_str());

        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        tm.tm_isdst = -1;

        if((epoch = mktime(&tm)) == (-1))
            return -1;
    }
    // YYYY-mm-dd HH:MM:SS
    else if(instr.size() == 19 && instr.at(4) == '-' && instr.at(7) == '-' &&
        instr.at(4) == '-' && instr.at(13) == ':' && instr.at(16) == ':')
    {
        tmpstr = instr.substr(0, 4);
        if(atoi(tmpstr.c_str()) < 1900 || atoi(tmpstr.c_str()) > 2100)
            return -1;
        tm.tm_year = atoi(tmpstr.c_str()) - 1900;

        tmpstr = instr.substr(5, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 12)
            return -1;
        tm.tm_mon = atoi(tmpstr.c_str()) - 1;

        tmpstr = instr.substr(8, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 31)
            return -1;
        tm.tm_mday = atoi(tmpstr.c_str());

        tmpstr = instr.substr(11, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 24)
            return -1;
        tm.tm_hour = atoi(tmpstr.c_str());
        
        tmpstr = instr.substr(14, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 60)
            return -1;
        tm.tm_min = atoi(tmpstr.c_str());

        tmpstr = instr.substr(17, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 60)
            return -1;
        tm.tm_sec = atoi(tmpstr.c_str());
        
        tm.tm_isdst = -1;

        if((epoch = mktime(&tm)) == (-1))
            return -1;
    }
    // date (YYYYmmdd, 1900~2100)
    else if(atof(instr.c_str()) > 19000000 && atof(instr.c_str()) < 21000000)
    {
        tmpstr = instr.substr(0, 4);
        if(atoi(tmpstr.c_str()) < 1900 || atoi(tmpstr.c_str()) > 2100)
            return -1;
        tm.tm_year = atoi(tmpstr.c_str()) - 1900;

        tmpstr = instr.substr(4, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 12)
            return -1;
        tm.tm_mon = atoi(tmpstr.c_str()) - 1;

        tmpstr = instr.substr(6, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 31)
            return -1;
        tm.tm_mday = atoi(tmpstr.c_str());

        tm.tm_hour = 0;
        tm.tm_min = 0;
        tm.tm_sec = 0;
        tm.tm_isdst = -1;

        if((epoch = mktime(&tm)) == (-1))
            return -1;
    }
    // datetime (YYYYmmddHHMMSS, 1900~2100)
    else if(atof(instr.c_str()) > 19000000000000 && atof(instr.c_str()) < 21000000000000)
    {
        tmpstr = instr.substr(0, 4);
        if(atoi(tmpstr.c_str()) < 1900 || atoi(tmpstr.c_str()) > 2100)
            return -1;
        tm.tm_year = atoi(tmpstr.c_str()) - 1900;

        tmpstr = instr.substr(4, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 12)
            return -1;
        tm.tm_mon = atoi(tmpstr.c_str()) - 1;

        tmpstr = instr.substr(6, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 31)
            return -1;
        tm.tm_mday = atoi(tmpstr.c_str());

        tmpstr = instr.substr(8, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 24)
            return -1;
        tm.tm_hour = atoi(tmpstr.c_str());
        
        tmpstr = instr.substr(10, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 60)
            return -1;
        tm.tm_min = atoi(tmpstr.c_str());

        tmpstr = instr.substr(12, 2);
        if(atoi(tmpstr.c_str()) < 0 || atoi(tmpstr.c_str()) > 60)
            return -1;
        tm.tm_sec = atoi(tmpstr.c_str());
        
        tm.tm_isdst = -1;
        
        if((epoch = mktime(&tm)) == (-1))
            return -1;
    }

    return epoch;
}

int getRuntimeCpuCnt()
{
    int corecnt = sysconf(_SC_NPROCESSORS_ONLN);

    if(corecnt <= 0)
        return 1;

    return corecnt;
}

size_t getRss()
{
    int page = sysconf(_SC_PAGESIZE);
    size_t rss = 0;
    const std::string statfile = strFormat("/proc/%d/stat", getpid());
    char buf[4096] = {0, };
    int fd = (-1);
    int count = 23; // rss 필드(24번째)
    char* p = NULL;
    char* x = NULL;

    if((fd = open(statfile.c_str(), O_RDONLY)) == -1)
        return 0;
    
    if(read(fd, buf, 4096) <= 0)
    {
        close(fd);
        return 0;
    }

    close(fd);

    p = buf;
    while(p && count--)
    {
        p = strchr(p,' ');
        if(p) p++;
    }
    
    if(!p)
        return 0;
    
    x = strchr(p,' ');
    if(!x)
        return 0;
    
    *x = '\0';
    rss = strtoll(p, NULL, 10);
    rss *= page;

    return rss;
}

bool checkIoError(size_t bufferlen)
{
    static const size_t numpattern = 7;
    static char* pattern[] = {
        (char*)"I/O error", // 커널 로그의 디스크 오류 패턴
        (char*)"fs error", // FSERR
        (char*)"megasas: failed", // DEVERR
        (char*)"scsi: Device offlined", // DEVERR
        (char*)"SCSI error", // DEVERR
        (char*)"CHECK CONDITION", // ETCERR
        (char*)"Unrecovered read error"}; // bad sectors
    std::vector<char> ring_buffer;

    if(bufferlen > 0)
    {
        ring_buffer.reserve(bufferlen);

        int n = klogctl(3, &ring_buffer[0], (int)(bufferlen - 4));
        if(n < 0) // err
        {
            return false;
        }
        
        ring_buffer[n] = '\0';

        for(size_t i = 0; i < numpattern; i++)
        {
            if(strstr(&ring_buffer[0], pattern[i]) != NULL)
            {
                // io error
                return true;
            }
        }
    }

    return false;
}

bool coreLimit(bool enable, int64_t limitsize)
{
    struct rlimit rl;

    if(getrlimit(RLIMIT_CORE, &rl) < 0)
        return false;

    if(enable)
    {
        if(limitsize > 0)
            rl.rlim_cur = limitsize;
        else
            rl.rlim_cur = RLIM_INFINITY;
    }
    else
    {
        rl.rlim_cur = 0;
    }

    if(setrlimit(RLIMIT_CORE, &rl) < 0)
        return false;

    return true;
}

int coreCntFromCurrentDir()
{
    int cnt = 0;
    DIR* dir = NULL;
    dirent* ent = NULL;
    char buf[PATH_MAX + 1] = {0, };

    if(!getcwd(buf, PATH_MAX))
        return -1;

    dir = opendir(buf);

    if(!dir)
        return -1;

    while((ent = readdir(dir)))
    {
        if(!strcmp(ent->d_name, "core") || !strncmp(ent->d_name, "core.", 4))
            cnt++;
    }

    closedir(dir);

    return cnt;
}

bool fadviseClear(const char* filepath)
{
    int fd;

    if(filepath == NULL || strlen(filepath) <= 0)
        return false;

    fd = open(filepath, O_RDONLY);

    if(fd < 0)
        return false;

    if(fdatasync(fd) < 0)
    {
        close(fd);
        return false;
    }

    if(posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED) != 0)
    {
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

int64_t getFileSize(const char* filepath)
{
    if(!filepath)
        return -1;

    struct stat fs;
    memset(&fs, 0, sizeof(fs));
    
    if(stat(filepath, &fs) < 0)
        return -1;

    if(!S_ISREG(fs.st_mode))
        return -1;
    
    return fs.st_size;
}

int64_t getFileSize(int fd)
{
    if(fd < 0)
        return -1;

    struct stat fs;
    memset(&fs, 0, sizeof(fs));

    if(fstat(fd, &fs) < 0)
        return -1;

    if(!S_ISREG(fs.st_mode))
        return -1;

    return fs.st_size;
}

int64_t fileInCore(const char* filepath)
{
    if(!filepath)
        return -1;

    int64_t cached_size = 0;
    int fd = -1;
    struct stat fs;
    memset(&fs, 0, sizeof(fs));
    
    if(stat(filepath, &fs) < 0)
        return -1;

    if(!S_ISREG(fs.st_mode))
        return -1;
    
    fd = open(filepath, O_RDONLY);

    if(fd < 0)
        return -1;

    cached_size = fileInCore(fd);

    if(fd >= 0)
    {
        close(fd);
        fd = -1;
    }

    return cached_size;
}

int64_t fileInCore(int fd)
{
    if(fd < 0)
        return -1;

    int64_t cached_size = 0;
    struct stat fs;
    void* fmap = MAP_FAILED;
    unsigned char* mcorevec = NULL;
    size_t pgsize = getpagesize();
    size_t pgidx = 0;
    size_t pgtotal = 0;
    size_t pgcached = 0;

    memset(&fs, 0, sizeof(fs));

    if(fstat(fd, &fs) < 0)
        return -1;

    if(fs.st_size <= 0)
        return -1;

    if(!S_ISREG(fs.st_mode))
        return -1;

    fmap = mmap((void *)0, fs.st_size, PROT_NONE, MAP_SHARED, fd, 0);

    if(fmap == MAP_FAILED)
    {
        WARNING("mmap() failed. st_size(%zd) fd(%d) error(%d:%s)",
            fs.st_size, fd, errno, strerror(errno));
        goto cleanup;
    }

    mcorevec = (unsigned char*)calloc(1, (fs.st_size + pgsize - 1) / pgsize);

    if(mcorevec == NULL)
    {
        WARNING("alloc() failed. st_size(%zd) pgsize(%zd)",
            fs.st_size, pgsize);
        goto cleanup;
    }

    if(mincore(fmap, fs.st_size, mcorevec) != 0)
    {
        WARNING("mincore() failed. st_size(%zd) error(%d:%s)",
            fs.st_size, errno, strerror(errno));
        goto cleanup;
    }
    
    pgtotal = (size_t)ceil((double)fs.st_size / (double)pgsize);

    for(pgidx = 0; pgidx <= fs.st_size / pgsize; pgidx++)
    {
        if(mcorevec[pgidx] & 1)
            ++pgcached;
    }

    if(pgcached == pgtotal && pgtotal > 0)
        cached_size = (int64_t)fs.st_size;
    else
        cached_size = (int64_t)(pgcached * pgsize);

cleanup:

    if(mcorevec)
    {
        free(mcorevec);
        mcorevec = NULL;
    }

    if(fmap != MAP_FAILED)
    {
        munmap(fmap, fs.st_size);
        fmap = MAP_FAILED;
    }

    return cached_size;
}

std::string host2IPString(const std::string& host)
{
    std::string ipstr = host;
    std::vector<std::string> arr;
    bool conv = false;

    splitString(host, arr, ".");

    if(arr.size() == 4)
    {
        for(size_t i = 0; i < arr.size(); i++)
        {
            if(arr[i].empty() || arr[i].length() > 3)
            {
                conv = true;
                break;
            }

            int num = atoi(arr[i].c_str());
            
            if(num < 0 || num > 255)
            {
                conv = true;
                break;
            }
            else if(num == 0)
            {
                if(arr[i] != "0")
                {
                    conv = true;
                    break;
                }
            }
        }
    }
    else
    {
        conv = true;
    }

    if(conv)
    {
        struct hostent* he = NULL;
        struct hostent hentry;
        char reserved[1024];
        int herr;

        gethostbyname_r(host.c_str(), &hentry, reserved, sizeof(reserved), &he, &herr);
    
        if(!he)
            return ipstr;

        ipstr = std::string(inet_ntoa(*((struct in_addr *)he->h_addr)));
    }

    return ipstr;
}

void getLocalIPString(std::vector<std::string>& ipstrs)
{
    struct ifaddrs* ifaddr = NULL;
    struct ifaddrs* ifa = NULL;
    void* tmpaddr = NULL;      

    ipstrs.clear();

    if(getifaddrs(&ifaddr) == -1)
        return;

    for(ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa ->ifa_addr->sa_family==AF_INET) // ipv4
        {
            tmpaddr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;

            char buf[INET_ADDRSTRLEN] = {0, };
            inet_ntop(AF_INET, tmpaddr, buf, INET_ADDRSTRLEN);
            ipstrs.push_back(buf);
        }
        else if(ifa->ifa_addr->sa_family==AF_INET6) // ipv6
        {
            tmpaddr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;

            char buf[INET6_ADDRSTRLEN] = {0, };
            inet_ntop(AF_INET6, tmpaddr, buf, INET6_ADDRSTRLEN);
            ipstrs.push_back(buf);
        } 
    }

    if(ifaddr) 
        freeifaddrs(ifaddr);
}

void url2HostAddress(const std::string& url, std::string& host, int* port)
{
    std::string tmp = "";
    
    host = "";
    *port = (-1);
    
    if(extractSubString(tmp, url.c_str(), 0, ':'))
        host = tmp;

    if(extractSubString(tmp, url.c_str(), 1, ':'))
        *port = atoi(tmp.c_str());
}

void addrToIpString(const in_addr_t clientaddr, std::string& ipstr)
{
    ipstr = strFormat("%d.%d.%d.%d", (clientaddr / 256 / 256 / 256) % 256,
                (clientaddr / 256 / 256) % 256, (clientaddr / 256) % 256, clientaddr % 256);
}

int randomPicker(const std::vector<int>& nums)
{
    if(nums.empty())
        return 0;

    timeval tv = {0, 0};
    gettimeofday(&tv, NULL);

    std::default_random_engine gen(tv.tv_usec);
    std::uniform_real_distribution<double> dist(0.0, (float)nums.size());
    
    double d = dist(gen);
    int rd = (int)std::floor(d);

    if(rd == (int)nums.size())
        rd = nums.size() - 1;

    return nums[rd];
}

std::string dumpString(const char* s, int n, int unit)
{
    assert(unit != 0);

    std::string r;
    char tmp[260] = {0, };
    int lineno = 0;
    int j = 7;
    int i = 0;
    for(; i < n; ++i)
    {
        if((i % unit) == 0)
            std::memset(tmp, 0x20, ((unit * 3) + unit + 1 + 7 + 3));

        if(j == 7)
            std::snprintf(tmp, sizeof (tmp), "[%4d] ", lineno);

        std::snprintf(tmp + j, sizeof (tmp) - j, "%02x", (unsigned char)(s[i]));
        tmp[j + 2] = 0x20;

        if((unsigned char)(s[i]) >= (unsigned char)0x20)
            tmp[(j / 3) + (unit * 3) + 1 + 6] = s[i];
        else
            tmp[(j / 3) + (unit * 3) + 1 + 6] = '.';
        j += 3;

        if((i % unit) == (unit - 1))
        {
            r += std::string{tmp} + "\n";
            j = 7;
            ++lineno;
        }
    }

    if((i % unit) != 0)
        r += std::string{tmp} + "\n";
    return r;
}

void unlinkFormatFile(time_t exptime, const char* wildpath)
{
    if(wildpath == NULL || exptime <= 0)
        return;

    glob_t paths;
    struct stat fs;
    time_t cur = time(NULL);

    paths.gl_pathc = 0;
    paths.gl_pathv = NULL;
    paths.gl_offs = 0;

    if(glob(wildpath, GLOB_APPEND, NULL, &paths) == 0)
    {
        for(size_t i = 0; i < paths.gl_pathc; i++)
        {
            if(lstat(paths.gl_pathv[i], &fs) == (-1))
                continue;

            if(fs.st_mtime + exptime < cur)
                unlink(paths.gl_pathv[i]);
        }
    }

    globfree(&paths);
}

bool execProcess(const std::string& cmdstr)
{
    int e = system(cmdstr.c_str());

    if(!WIFEXITED(e) || WEXITSTATUS(e))
    {
        WARNING("system('%s') failed, exited(%d) retval(%d)",
            cmdstr.c_str(), WIFEXITED(e), WEXITSTATUS(e));
        return false;
    }
    
    return true;
}

bool createProcess(const std::string& cmdstr, std::string& retstr)
{
    FILE* fp = popen(cmdstr.c_str(), "r");

    if(!fp)
        return false;

    char buf[1024];

    while(fgets(buf, 1024, fp) != NULL)
        retstr += std::string(buf);

    pclose(fp);
    
    return true;
}

std::string getFileContents(const std::string& path)
{
    FILE* fp = fopen(path.c_str(), "r");
    char buf[4096] = {0, };
    size_t n = 0;

    if(!fp)
        return "";

    std::string retstr = "";

    while((n = fread(buf, sizeof(char), sizeof(buf), fp)) > 0)
        retstr += std::string(buf, n);

    if(fp)
        fclose(fp);

    return retstr;
}

bool httpRequest(const std::string& url, const std::map<std::string, std::string>& opts,
    std::string& rsphdr, std::string& content, bool post)
{
    std::string curl = "";
    std::string domain = "";
    std::string host = "";
    int port = (-1);
    std::string path = "";
    std::string param = "";
    std::string res = "";
    std::string reqhdr = "";
    std::string status = "";
    Socket sock;
    size_t offset = 0;
    size_t len = 0;
    int ss = 0;
    char buf[32768] = {0, };

    rsphdr = "";
    content = "";

    try
    {
        if(!strncasecmp(url.c_str(), "http://", 7))
        {
            curl = url.substr(7);
            port = 80;
        }
        else if(!strncasecmp(url.c_str(), "https://", 8))
        {
            curl = url.substr(8);
            port = 443;
        }
        else
        {
            curl = url;
            port = 80;
        }

        size_t pos = curl.find("/");

        if(pos != std::string::npos)
        {
            domain = curl.substr(0, pos);
            path = curl.substr(pos);
        }
        else
        {
            domain = curl;
            path = "/";
        }

        pos = domain.find(":");

        if(pos != std::string::npos)
        {
            host = domain.substr(0, pos);

            if(atoi(domain.substr(pos + 1).c_str()) > 0)
                port = atoi(domain.substr(pos + 1).c_str());
        }
        else
        {
            host = domain;
        }

        if(path.find("/") != 0)
            path = "/" + path;
        
        for(const std::pair<std::string,std::string>& p : opts)
        {
            param += "&";
            param += encodeUrl(p.first);
            param += "=";
            param += encodeUrl(p.second);
        }

        param = strTrimLeft(param, "&");
        DEBUG("util: param=%s", param.c_str());

        if(post)
        {
            reqhdr = strFormat("POST %s HTTP/1.0\r\n", path.c_str());
            reqhdr += strFormat("Host: %s\r\n", domain.c_str());
            reqhdr += strFormat("Content-Length: %d\r\n", param.size());
            reqhdr += strFormat("Content-Type: application/x-www-form-urlencoded\r\n");
            reqhdr += strFormat("User-Agent: curl/7.26.0\r\n");
            reqhdr += strFormat("Connection: close\r\n");
            reqhdr += strFormat("\r\n");
            reqhdr += strFormat("%s\r\n", param.c_str());
            reqhdr += strFormat("\r\n");
        }
        else
        {
            // HTTP/1.1 지원하려면 "Transfer-Encoding: chunked" 구현해야함
            if(path.find("?") == std::string::npos)
                reqhdr = strFormat("GET %s?%s HTTP/1.0\r\n", path.c_str(), param.c_str());
            else
                reqhdr = strFormat("GET %s&%s HTTP/1.0\r\n", path.c_str(), param.c_str());
            reqhdr += strFormat("Host: %s\r\n", domain.c_str());
            reqhdr += strFormat("Accept: text/html\r\n");
            reqhdr += strFormat("User-Agent: curl/7.26.0\r\n");
            reqhdr += strFormat("Connection: close\r\n");
            reqhdr += strFormat("\r\n");
        }

        if(!sock.create())
        {
            ERROR("util: socket creation failed");
            return false;
        }

        sock.sendTimeout(10.0);
        sock.recvTimeout(10.0);

        // connect
        while(1)
        {
            ss = sock.connect(host.c_str(), port);

            if(ss == Socket::SS_RETRY)
                continue;

            if(ss == Socket::SS_OK)
                break;

            ERROR("util: connection failed");
            sock.close();
            return false;
        }

        DEBUG("util: reqhdr=%s", reqhdr.c_str());

        // send
        while(1)
        {
            ss = sock.send(reqhdr.c_str() + offset, reqhdr.size() - offset, &len);

            if(ss == Socket::SS_RETRY)
                continue;

            if(ss == Socket::SS_OK)
            {
                offset += len;

                if(offset >= reqhdr.size())
                    break;

                continue;
            }

            ERROR("util: send failed");
            sock.close();
            return false;
        }

        // recv
        while(1)
        {
            ss = sock.recv(buf, sizeof(buf), &len);

            if(ss == Socket::SS_RETRY)
                continue;

            if(ss == Socket::SS_OK)
            {
                res += std::string(buf, len);
                continue;
            }

            break;
        }

        sock.close();

        if(res.empty())
        {
            ERROR("util: no response");
            return false;
        }

        pos = res.find("\r\n\r\n");
    
        if(pos == std::string::npos)
        {
            ERROR("util: wrong http format");
            return false;
        }

        rsphdr = res.substr(0, pos);
        content = res.substr(pos + 4);

        extractSubString(status, rsphdr.c_str(), 1, ' ');

        if(atoi(status.c_str()) != 200)
        {
            WARNING("util: unrecognized status code, '%s'", status.c_str());
            return false;
        }
    }
    catch(...)
    {
        WARNING("util: unexpected exception");
        return false;
    }

    return true;
}

} // namespace vox
