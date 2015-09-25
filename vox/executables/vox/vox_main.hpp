#ifndef _VOX_EXE_VOX_MAIN_HPP_
#define _VOX_EXE_VOX_MAIN_HPP_

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#include "trace.hpp"
#include "exception.hpp"
#include "util.hpp"
#include "socket.hpp"
#include "epoll.hpp"
#include "wiring_init.hpp"


namespace vox
{

class VoxMain
{

public:
    VoxMain(const char* appname = NULL);
    virtual ~VoxMain();
    bool initialize(int argc, char** argv);
    void finalize();
    void run();
    static void signalHandler(int signo, siginfo_t* si, void* data);
    bool isExit() { return m_exit; }
    const char* getAppName() { return m_appname.c_str(); }
    int getCmdStat() { return m_stat; };
    void setCmdStat(int stat) { m_stat = (sig_atomic_t)stat; };
    sig_atomic_t getMode() { return m_mode; }

private:
    void usage();
    bool readUserInput(int argc, char** argv);
    void installSignalHandler();
    void signalDelegator(int signo, siginfo_t* si, void* data);
    bool setToDaemon();
    bool setupLog();
    bool createPidFile();
    bool writeDataFile(const std::string& datastr);
    void cpuUsage();
    
private:
    bool m_exit; // 종료 플래그
    std::string m_appname; // program name
    pid_t m_pid; // self process-id
    bool m_debug;
    std::string m_logdir;
    int m_loglevel;
    std::string m_pidfile;
    std::string m_datfile;
    sig_atomic_t m_stat;
    sig_atomic_t m_mode;
    std::string m_myip;
    float m_cpuusage;
    float m_cputemp;
    size_t m_appmem;
    uint64_t m_prvuser;
    uint64_t m_prvnice;
    uint64_t m_prvsystem;
    uint64_t m_prvidle;
    uint64_t m_prviowait;
    uint64_t m_prvirq;
    uint64_t m_prvsoftirq;
};

extern VoxMain g_main;

} // namespace vox

#endif // _VOX_EXE_VOX_MAIN_HPP_
