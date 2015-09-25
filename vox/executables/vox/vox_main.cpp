#include "vox_main.hpp"
#include "vox_control.hpp"
#include "vox_player.hpp"
#include "vox_sensor.hpp"
#include "vox_vision.hpp"
#include "vox_voice.hpp"

namespace vox
{

VoxMain g_main("vox");

VoxMain::VoxMain(const char* appname)
{
    m_appname = (appname) ? std::string(appname) : "";
    m_exit = false;
    m_debug = false;
    m_mode = MD1;
    m_stat = ST1;
    m_logdir = "";
    m_loglevel = 0;
    m_pidfile = "/tmp/vox.pid";
    m_datfile = "/tmp/vox.dat";
    m_myip = "127.0.0.1";
    m_cpuusage = 0.0f;
    m_cputemp = 40.0f;
    m_appmem = 0;
    m_prvuser = 0;
    m_prvnice = 0;
    m_prvsystem = 0;
    m_prvidle = 0;
    m_prviowait = 0;
    m_prvirq = 0;
    m_prvsoftirq = 0;
}

VoxMain::~VoxMain()
{
}

void VoxMain::usage()
{
    fprintf(stdout, "Usage: %s [OPTION]     Build: %s\n", m_appname.c_str(), __DATE__);
    fprintf(stdout, "                             Start %s\n", m_appname.c_str());
    fprintf(stdout, "\n");
    fprintf(stdout, "  -x, --no-daemon            Run voxel in debug mode\n");
    fprintf(stdout, "  -l, --log-dir=LOGDIR       Set default log directory\n");
    fprintf(stdout, "  -d, --debug=LEVEL          Set debug level to LEVEL\n");
    fprintf(stdout, "  -h, --help                 Display this help and exit\n");
    fprintf(stdout, "      --version              Output version information and exit\n");
    fprintf(stdout, "\n");
}

bool VoxMain::readUserInput(int argc, char **argv)
{
    int c = 0;

    optarg = NULL;
    optind = 0;
    opterr = 0;
    optopt = 0;

    while(!m_exit)
    {
        int option_index = 0;
        static struct option long_options[] =
            {
                {"no-daemon", 0, 0, 'x'},
                {"log-dir", 1, 0, 'l'},
                {"debug", 1, 0, 'd'},
                {"help", 0, 0, 'h'},
                {"version", 0, 0, 1000},
                {0, 0, 0, 0}
            };

        optarg = NULL;

        c = getopt_long(argc, argv, "xl:d:h", long_options, &option_index);

        if (c == -1)
            break;

        switch(c)
        {
            case 'x':
                m_debug = true;
                break;

            case 'l':
                if(optarg)
                    m_logdir = std::string(optarg);
                break;

            case 'd':
                m_loglevel = atoi(optarg);
                break;

            case 'h':
                usage();
                exit(0);
                return false;

            case 1000: // version
                fprintf(stdout, "%s (Build: %s)\n", m_appname.c_str(), __DATE__);
                exit(0);
                return false;

            default:
                {
                    const int curind = optind - 1;

                    if(curind > 0 && curind < argc)
                        fprintf(stderr, "Invalid option argument '%s'\n\n", argv[curind]);
                    else
                        fprintf(stderr, "Invalid option argument\n\n");
                }
                return false;
        }
    }

    return true;
}

void VoxMain::signalDelegator(int signo, siginfo_t* si, void* data)
{
    pid_t pid;
    int stat;

    //fprintf(stderr, "signo(%d) si_signo(%d) si_code(%d) si_errno(%d) si_pid(%d) si_status(%d)\n",
    //    signo, si->si_signo, si->si_code, si->si_errno, si->si_pid, si->si_status);

    switch(signo)
    {
        case SIGTERM:
        case SIGINT:
            m_exit = true;
            break;

        case SIGCHLD:
            while(true)
            {
                pid = waitpid(-1, &stat, WNOHANG);

                if(pid < 0)
                {
                    if(errno == ECHILD)
                        break;
                    else if(errno == EINTR)
                        continue;
                }
                else if(pid == 0)
                    break;
            }
            break;

        case SIGUSR1: break;
        case SIGUSR2: break;

        // 모드
        case MD1: m_mode = MD1; break;
        case MD2: m_mode = MD2; break;
        case MD3: m_mode = MD3; break;
        case MD4: m_mode = MD4; break;
        case MD5: m_mode = MD5; break;

        // 커맨드
        case FL:  m_stat = FL;  break;
        case FF:  m_stat = FF;  break;
        case FR:  m_stat = FR;  break;
        case LL:  m_stat = LL;  break;
        case ST1: m_stat = ST1; break;
        case ST2: m_stat = ST2; break;
        case RR:  m_stat = RR;  break;
        case BL:  m_stat = BL;  break;
        case BB:  m_stat = BB;  break;
        case BR:  m_stat = BR;  break;
        case CC:  m_stat = CC;  break;
        case UT:  m_stat = UT;  break;
        case CW:  m_stat = CW;  break;
        case UP:  m_stat = UP;  break;
        case DN:  m_stat = DN;  break;
        case AC1: m_stat = AC1; break;
        case AC2: m_stat = AC2; break;
        case AC3: m_stat = AC3; break;
        case AC4: m_stat = AC4; break;
        case AC5: m_stat = AC5; break;
        case CLT: m_stat = CLT; break;
        case CRT: m_stat = CRT; break;
        case CUP: m_stat = CUP; break;
        case CDN: m_stat = CDN; break;
        case EE:  m_stat = EE; break;
        case FB:  m_stat = FB; break;

        case SIGQUIT:
        case SIGILL:
        case SIGABRT:
        case SIGFPE:
        case SIGSEGV:
        case SIGBUS:
        case SIGSYS:
        case SIGTRAP:
        case SIGXCPU:
        case SIGXFSZ:
            {
                signal(signo, SIG_DFL);
                kill(getpid(), signo);
            }
            break;

        default:
            break;
    }
}

void VoxMain::signalHandler(int signo, siginfo_t* si, void* data)
{
    g_main.signalDelegator(signo, si, data);
}

void VoxMain::installSignalHandler()
{
    struct sigaction sa;
    int i;

    sigemptyset(&sa.sa_mask);

    for(i = SIGHUP; i <= SIGUNUSED; i++)
    {
        if(i == SIGKILL || i == SIGSTOP || i == SIGPROF)
            continue;
        
        sigaddset(&sa.sa_mask, i);
    }

    for(i = SIGRTMIN; i <= SIGRTMAX; i++)
        sigaddset(&sa.sa_mask, i);

    sa.sa_sigaction = VoxMain::signalHandler;
    sa.sa_flags = SA_SIGINFO | SA_NOCLDSTOP;

    for(i = SIGHUP; i <= SIGUNUSED; i++)
    {
        if(i == SIGKILL || i == SIGSTOP || i == SIGPROF)
            continue;

        sigaction(i, &sa, NULL);
    }

    for(i = SIGRTMIN; i <= SIGRTMAX; i++)
        sigaction(i, &sa, NULL);
}

bool VoxMain::setToDaemon()
{
    if(m_debug)
        return true;

    char buf[1024] = {0, };
    ssize_t len = (-1);
    std::string pathout = std::string("/proc/self/fd/1");
    std::string patherr = std::string("/proc/self/fd/2");
    std::string redirectout = readLink(pathout);
    std::string redirecterr = readLink(patherr);
    struct stat fsout;
    struct stat fserr;

    memset(&fsout, 0, sizeof(fsout));
    memset(&fserr, 0, sizeof(fserr));

    freopen("/dev/null", "r", stdin);

    if(lstat(redirectout.c_str(), &fsout) == 0)
    {
        if(!S_ISREG(fsout.st_mode))
            redirectout.clear();
    }

    if(lstat(redirecterr.c_str(), &fserr) == 0)
    {
        if(!S_ISREG(fserr.st_mode))
            redirecterr.clear();
    }

    if(fork() != 0)
        exit(0);

    setsid();

    setlinebuf(stdout);

    if(redirectout.empty())
        freopen("/dev/null", "a+", stdout);

    if(redirecterr.empty())
        freopen("/dev/null", "a+", stderr);

    return true;
}

bool VoxMain::setupLog()
{
    if(m_logdir.empty())
    {
        if(m_debug)
            Trace::getInstance().open("/dev/stdout", (Trace::TraceLevel)m_loglevel);
    }
    else
    {
        std::string logpath = strTrimRight(m_logdir, "/") + "/vox.log";
        Trace::getInstance().open(logpath, (Trace::TraceLevel)m_loglevel);
    }

    setlinebuf(stdout);

    return true;
}

bool VoxMain::createPidFile()
{
    FILE* fp = fopen(m_pidfile.c_str(), "w");

    if(!fp)
        return false;

    fprintf(fp, "%d\n", (int)getpid());
    fclose(fp);

    return true;
}

bool VoxMain::writeDataFile(const std::string& datastr)
{
    FILE* fp = fopen(m_datfile.c_str(), "w+");

    if(!fp)
        return false;

    fprintf(fp, "%s\n", datastr.c_str());
    fclose(fp);

    return true;
}

bool VoxMain::initialize(int argc, char** argv)
{
    if(!readUserInput(argc, argv))
        return false;

    installSignalHandler();

    if(!setToDaemon())
        return false;

    if(!setupLog())
        return false;

    if(!INIT_WIRING)
    {
        ERROR("INIT_WIRING failed!");
        return false;
    }

    if(!VoxControl::getInstance().start())
    {
        ERROR("(MAIN) failed to start controller thread");
        return false;
    }

    if(!VoxSensor::getInstance().start())
    {
        ERROR("(MAIN) failed to start sensor thread");
        return false;
    }

    if(!VoxPlayer::getInstance().start())
    {
        ERROR("(MAIN) failed to start player thread");
        return false;
    }

    if(!VoxVision::getInstance().start())
    {
        ERROR("(MAIN) failed to start vision thread");
        return false;
    }

    if(!VoxVoice::getInstance().start())
    {
        ERROR("(MAIN) failed to start voice thread");
        return false;
    }

    if(!createPidFile())
    {
        ERROR("(MAIN) pid file '%s' create failed", m_pidfile.c_str());
        return false;
    }

    return true;
}

void VoxMain::finalize()
{
    m_exit = true;

    VoxVoice::getInstance().stop();
    VoxVision::getInstance().stop();
    VoxPlayer::getInstance().stop();
    VoxSensor::getInstance().stop();
    VoxControl::getInstance().stop();

    if(!m_datfile.empty())
        unlink(m_datfile.c_str());

    if(!m_pidfile.empty())
        unlink(m_pidfile.c_str());

    Trace::getInstance().close();
}

void VoxMain::cpuUsage()
{
    FILE* fp = fopen("/proc/stat", "r");

    if(!fp)
        return;

    char curcpu[8] = {0, };
    uint64_t curuser = 0;
    uint64_t curnice = 0;
    uint64_t cursystem = 0;
    uint64_t curidle = 0;
    uint64_t curiowait = 0;
    uint64_t curirq = 0;
    uint64_t cursoftirq = 0;
    uint64_t spanuser = 0;
    uint64_t spannice = 0;
    uint64_t spansystem = 0;
    uint64_t spanidle = 0;
    uint64_t spaniowait = 0;
    uint64_t spanirq = 0;
    uint64_t spansoftirq = 0;
    uint64_t spanall = 0;
    
    int n = fscanf(fp, "%7s %lu %lu %lu %lu %lu %lu %lu%*[^\n]",
                curcpu, &curuser, &curnice, &cursystem, &curidle, &curiowait, &curirq, &cursoftirq);

    if(strcasecmp(curcpu, "cpu") == 0 && n > 7)
    {
        spanuser = curuser - m_prvuser;
        spannice = curnice - m_prvnice;
        spansystem = cursystem - m_prvsystem;
        spanidle = curidle - m_prvidle;
        spaniowait = curiowait - m_prviowait;
        spanirq = curirq - m_prvirq;
        spansoftirq = cursoftirq - m_prvsoftirq;

        spanall = spanuser + spannice + spansystem + spanidle + spaniowait + spanirq + spansoftirq;

        if(spanall > 0 && m_prvuser > 0)
            m_cpuusage = (float)(100.0 - (float)(100.0 * spanidle / spanall));
        else
            m_cpuusage = 0.f;

        m_prvuser = curuser;
        m_prvnice = curnice;
        m_prvsystem = cursystem;
        m_prvidle = curidle;
        m_prviowait = curiowait;
        m_prvirq = curirq;
        m_prvsoftirq = cursoftirq;
    }

    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }
}

void VoxMain::run()
{
    int modesw = 3;
    uint64_t cnt = 0;
    timeval tvchk = {0, 0};
    timeval tvcur = {0, 0};
    std::string datastr = "";

    INFORMATION("(MAIN) start of loop");

    pinMode(modesw, INPUT);
    pullUpDnControl(modesw, PUD_UP); 
    gettimeofday(&tvchk, NULL);

    while(!m_exit)
    {
        int onoff = digitalRead(modesw);

        gettimeofday(&tvcur, NULL);

        if(onoff == LOW)
        {
            std::string retstr = "";

            if(m_mode > MD5)
                --m_mode;
            else
                m_mode = MD1;

            NOTICE("mode changed: mode=%d", m_mode);

            if(m_mode == MD1)  // control
            {
            }
            else if(m_mode == MD2) // vision
            {
                //if(!createProcess("/home/pi/mjpg-streamer/stop_mjpg.sh", retstr))
                //    ERROR("(MAIN) failed to stop mjpg-streamer");
            }
            else if(m_mode == MD3) // voice
            {
            }

            Servo* servo = VoxControl::getInstance().getServo();
            servo->setNeutral();
            VoxPlayer::getInstance().play("", false);

            delay(300);
        }

        if(timeSpan(tvchk, tvcur) >= 1.0f)
        {
            cpuUsage();
            tvchk.tv_sec = tvcur.tv_sec;
            tvchk.tv_usec = tvcur.tv_usec;
        }

        if((cnt % 100) == 0)
        {
            std::vector<std::string> addrs;
            getLocalIPString(addrs);

            m_myip = "";
            for(size_t i = 0; i < addrs.size(); i++)
            {
                std::string acls = "";
                extractSubString(acls, addrs[i].c_str(), 0, '.');

                if(isPositiveNumber(acls))
                {
                    if(acls != "127")
                        m_myip += addrs[i] + ", ";
                }
            }

            m_myip = strTrim(m_myip, " ,");

            std::string retstr = "";
            
            if(createProcess("/usr/bin/vcgencmd measure_temp", retstr))
            {
                strRemove(retstr, "temp=");
                strRemove(retstr, "'C");
                m_cputemp = atof(retstr.c_str());
            }

            m_appmem = getRss();
        }

        if((cnt % 2) == 0)
        {
            bool voice = VoxVoice::getInstance().isRunning();

            datastr = strFormat("%s|%d|%d|%d|%.1f|%.1f|%.1f|%.1f|%.1f|%.1f|%.1f|%.1f|%.1f|%d|%d|%d|%.1f|%d|%d|%s|%s",
                m_myip.c_str(),
                abs(MD1 - m_mode) + 1,
                m_stat,
                VoxSensor::getInstance().isFreeze(),
                VoxSensor::getInstance().getTemperature(),
                VoxSensor::getInstance().getHumidity(),
                VoxSensor::getInstance().getDistF(),
                VoxSensor::getInstance().getDistB(),
                VoxSensor::getInstance().getDistL(),
                VoxSensor::getInstance().getDistR(),
                m_cpuusage,
                m_cputemp,
                (float)(m_appmem / 1024.0 / 1024.0),
                VoxVision::getInstance().isFollowing(),
                VoxVision::getInstance().getPosX(),
                VoxVision::getInstance().getPosY(),
                VoxVision::getInstance().getRadius(),
                VoxVision::getInstance().getBallCount(),
                (voice) ? VoxVoice::getInstance().readyToGo() : 0,
                (voice) ? VoxVoice::getInstance().getReq().c_str() : "",
                (voice) ? VoxVoice::getInstance().getRsp().c_str() : ""
                );

            writeDataFile(datastr);
        }

        delay(150);
        ++cnt;
    }

    pinMode(modesw, OUTPUT);
    digitalWrite(modesw, LOW);

    INFORMATION("(MAIN) end of loop");
}

} // namespace vox

using namespace vox;

int main(int argc, char** argv)
{
    coreLimit(true);

    if(!g_main.initialize(argc, argv))
    {
        g_main.finalize();
        return 1;
    }

    g_main.run();

    g_main.finalize();

    return 0;
}
