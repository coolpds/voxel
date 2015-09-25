#ifndef _VOX_LIB_UTIL_UTIL_HPP_
#define _VOX_LIB_UTIL_UTIL_HPP_

#include <stdint.h>
#include <math.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>


namespace vox
{

inline unsigned char toHex(const unsigned char& x)
{
  return x > 9 ? x + 55: x + 48;
}

inline bool isHexDigit(int c)
{
  switch(c)
  {
    case '0': return true;
    case '1': return true;
    case '2': return true;
    case '3': return true;
    case '4': return true;
    case '5': return true;
    case '6': return true;
    case '7': return true;
    case '8': return true;
    case '9': return true;
    case 'A': return true;
    case 'B': return true;
    case 'C': return true;
    case 'D': return true;
    case 'E': return true;
    case 'F': return true;
    case 'a': return true;
    case 'b': return true;
    case 'c': return true;
    case 'd': return true;
    case 'e': return true;
    case 'f': return true;
    default: return false;
  }

  return false;
}

const std::string base64Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline bool isBase64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

inline std::string double2StringPrecision(const double input, const uint32_t precision)
{
    char fmt[3 + (10 * 2) + 1] = {0, };
    char output[32] = {0, };

    if(precision < 30)
    {
        snprintf(fmt, 3 + (10 * 2), "%%.%uf", precision);
        snprintf(output, 31, fmt, input);
    }
    else
    {
        snprintf(output, 31,"%f", input);
    }

    return std::string(output);
}

inline std::string digit2String(const double input)
{
    char output[32] = {0, };
    snprintf(output, 31, "%.2f", input);
    return std::string(output);
}

template<typename T>
const extern inline std::string humanizeNums(const T val, const uint32_t precision = 0)
{
    const register long double absval = fabs(val);

    if(absval > (1LL << 60))
        return double2StringPrecision(double(val) / (1ULL << 60), precision) + "e";
    else if(absval > (1LL << 50))
        return double2StringPrecision(double(val) / (1ULL << 50), precision) + "p";
    else if(absval > (1LL << 40))
        return double2StringPrecision(double(val) / (1ULL << 40), precision) + "t";
    if(absval > (1 << 30))
        return double2StringPrecision(double(val) / (1 << 30), precision) + "g";
    else if(absval > (1 << 20))
        return double2StringPrecision(double(val) / (1 << 20), precision) + "m";
    else if(absval > (1 << 10))
        return double2StringPrecision(double(val) / (1 << 10), precision) + "k";

    return double2StringPrecision(val, precision);
}

extern size_t humanizeNums2Bytes(const std::string& nums);
extern bool isFileExistence(const char* path);
extern bool isPositiveNumber(const std::string& str);
extern int makeSubDirectory(const std::string& path, mode_t mode = 0755);
extern void removeDirectory(const std::string& path);
extern std::string readLink(const std::string& path);
extern std::string realPath(const std::string& path);
extern int fileLock(int fd);
extern int fileUnlock(int fd);
extern std::string strTrim(const std::string& s, const char* target = NULL);
extern std::string strTrimLeft(const std::string& s, const char* target = NULL);
extern std::string strTrimRight(const std::string& s, const char* target = NULL);
extern std::string strToLower(const std::string& instr);
extern std::string strToUpper(const std::string& instr);
extern bool extractSubString(std::string& str, const char* src, int idx, char sep);
extern void splitString(const std::string& str, std::vector<std::string>& result, const std::string& delm = " \t\r\n");
extern void splitStringWithQuot(const std::string& str, std::vector<std::string>& result, const std::string& delm = " \t\r\n");
extern void parsePostParam(const std::string& str, std::map<std::string, std::string>& result);
extern bool strReplace(std::string& str, std::string from, std::string to);
extern void replaceAmpChar(std::string& str);
extern void restoreAmpChar(std::string& str);
extern void strRemove(std::string& str, const char* chars);
extern std::string strFormat(const char* format, ...);
extern std::string strFormatV(const char* format, va_list ap);
extern int aToIntOpt(const char* str);
extern bool wildMatch(const char* wild, const char* text);
extern int encodeUrl(const char* instr, std::string& outstr);
extern std::string encodeUrl(const std::string& instr);
extern int decodeUrl(const char* instr, std::string& outstr);
extern std::string decodeUrl(const std::string& instr);
extern int encodeUtf8(const char* instr, std::string& outstr);
extern std::string encodeUtf8(const std::string& instr);
extern int decodeUtf8(const char* instr, std::string& outstr);
extern std::string decodeUtf8(const std::string& instr);
extern std::string encodeBase64(const std::string& str);
extern std::string decodeBase64(const std::string& str);
extern void encodeBase64(const std::vector<uint8_t>& inbytes, std::string& outstr);
extern void decodeBase64(const std::string& instr, std::vector<uint8_t>& outbytes);
extern float timeSpan(const timespec& start, const timespec& end);
extern float timeSpan(const timeval& start, const timeval& end);
extern float timeSpan(const time_t start, const time_t end);
extern std::string getCurrentTimeString();
extern std::string getTimeString(const timeval& tv);
extern std::string getTimeString(const timespec& ts);
extern std::string getTimeString(time_t t); // 초단위까지만 표시
extern std::string getCurrentDate();
extern std::string getCurrentTime();
extern time_t timeString2EpochTime(const char* tmstr);
extern int getRuntimeCpuCnt();
extern size_t getRss();
extern bool checkIoError(size_t bufferlen);
extern bool coreLimit(bool enable = true, int64_t limitsize = 4294967296); // 4gb
extern int coreCntFromCurrentDir();
extern bool fadviseClear(const char* filepath);
extern int64_t getFileSize(const char* filepath);
extern int64_t getFileSize(int fd);
extern int64_t fileInCore(const char* filepath);
extern int64_t fileInCore(int fd);
extern std::string host2IPString(const std::string& host);
extern void getLocalIPString(std::vector<std::string>& ipaddrs);
extern void url2HostAddress(const std::string& url, std::string& host, int* port);
extern void addrToIpString(const in_addr_t clientaddr, std::string& ipstr);
extern int randomPicker(const std::vector<int>& nums);
extern std::string dumpString(const char* s, int n, int unit = 16);
extern void unlinkFormatFile(time_t exptime, const char* wildpath);
extern bool execProcess(const std::string& cmdstr);
extern bool createProcess(const std::string& cmdstr, std::string& retstr);
extern std::string getFileContents(const std::string& path);
extern bool httpRequest(const std::string& url, const std::map<std::string, std::string>& opts,
    std::string& rsphdr, std::string& content, bool post = false);

} // namespace vox

#endif // _VOX_LIB_UTIL_UTIL_HPP_
