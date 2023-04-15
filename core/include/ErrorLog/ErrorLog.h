#ifndef ERRORLOG_ERRORLOG_H
#define ERRORLOG_ERRORLOG_H

//#define NO_ERROR_LOG
#ifndef NO_ERROR_LOG
/*
 * if you don't use LOG、LOG_MSG、LOG_FMT please #define NO_ERRORLOG in front of ErrorLog.h
 * if you don't need log file please #define NO_LOG_FILE in front of ErrorLog.h
 * */

#define LOG_FILE_NAME "log.log"
#define NO_LOG_FILE

#include <iostream>
#include <fstream>
#include <ctime>
#include <unordered_map>
#include <mutex>

namespace hzd {
    enum Error {
        None = 0,
        Out_Of_Bound,
        Pointer_To_Null,
        Bad_Malloc,
        Socket_Bind,
        Socket_Create,
        Socket_Accept,
        Socket_Set_Opt,
        Epoll_Create,
        Epoll_Add,
        Epoll_Wait,
        Socket_Listen,
        Epoll_Error,
        Epoll_Read,
        Epoll_Write,
        Conn_Send,
        Conn_Recv,
        Heart_Beat_Timeout,

    };

#ifdef _WIN32

    #include <windows.h>
    #include <cstdint>
    #include <cstdlib>

    //   ����̨ǰ����ɫ
    enum ConsoleForegroundColor {
        CFC_Red = FOREGROUND_RED,
        CFC_RedI = FOREGROUND_INTENSITY | FOREGROUND_RED,
        CFC_Green = FOREGROUND_INTENSITY | FOREGROUND_GREEN,
        CFC_Blue = FOREGROUND_INTENSITY | FOREGROUND_BLUE,
        CFC_Yellow = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
        CFC_Purple = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_BLUE,
        CFC_Cyan = FOREGROUND_INTENSITY | FOREGROUND_GREEN | FOREGROUND_BLUE,
        CFC_Gray = FOREGROUND_INTENSITY,
        CFC_White = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        CFC_HighWhite = FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
        CFC_Black = 0,
    };

    // ����ɫö��ֵ
    enum ConsoleBackGroundColor {
        CBC_Red = BACKGROUND_INTENSITY | BACKGROUND_RED,
        CBC_Green = BACKGROUND_INTENSITY | BACKGROUND_GREEN,
        CBC_Blue = BACKGROUND_INTENSITY | BACKGROUND_BLUE,
        CBC_Yellow = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN,
        CBC_Purple = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE,
        CBC_Cyan = BACKGROUND_INTENSITY | BACKGROUND_GREEN | BACKGROUND_BLUE,
        CBC_White = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
        CBC_HighWhite = BACKGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,
        CBC_Black = 0,
    };

    // ������ɫ����
    void SetConsoleColor(ConsoleForegroundColor foreColor = CFC_White, ConsoleBackGroundColor backColor = CBC_Black) {
        HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(handle, foreColor | backColor);
    }

    //���ú���
    void ShowMessage(std::string msg, ConsoleForegroundColor foreColor = CFC_White,
                     ConsoleBackGroundColor backColor = CBC_Black) {
        SetConsoleColor(foreColor, backColor);
        std::cout << msg << std::endl;
        SetConsoleColor(CFC_White, CBC_Black);
    }

#endif

#ifdef __unix__

    #define RESET       "\033[0m"
    #define BLACK       "\033[30m"             /* Black */
    #define RED         "\033[31m"             /* Red */
    #define GREEN       "\033[32m"             /* Green */
    #define YELLOW      "\033[33m"             /* Yellow */
    #define BLUE        "\033[34m"             /* Blue */
    #define MAGENTA     "\033[35m"             /* Magenta */
    #define CYAN        "\033[36m"             /* Cyan */
    #define WHITE       "\033[37m"             /* White */
    #define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
    #define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
    #define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
    #define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
    #define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
    #define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
    #define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
    #define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

    enum ConsoleForegroundColor
    {
        CFC_RESET = 0,
        CFC_Black,
        CFC_Red,
        CFC_Green,
        CFC_Yellow,
        CFC_Blue,
        CFC_Magenta,
        CFC_Cyan,
        CFC_White,
    };

    std::string getColor(ConsoleForegroundColor color)
    {
        switch (color)
        {
            case CFC_RESET : {
                return RESET;
            }
            case CFC_Black : {
                return BOLDBLACK;
            }
            case CFC_Red : {
                return BOLDRED;
            }
            case CFC_Green : {
                return BOLDGREEN;
            }
            case CFC_Yellow : {
                return BOLDYELLOW;
            }
            case CFC_Blue : {
                return BOLDBLUE;
            }
            case CFC_Magenta : {
                return BOLDMAGENTA;
            }
            case CFC_Cyan : {
                return BOLDCYAN;
            }
            case CFC_White : {
                return BOLDWHITE;
            }
            default : {
                return RESET;
            }
        }
    }

    //���ú���
    void ShowMessage(const std::string& msg,ConsoleForegroundColor foreColor = CFC_White) {
        std::cout << getColor(foreColor) << msg << getColor(CFC_RESET) << std::endl;
    }

#endif

    static std::string getTime() {
        time_t t = time(nullptr);
        char timeTable[64] = {0};
        strftime(timeTable, sizeof(timeTable), "[%Y-%m-%d %H:%M:%S]", localtime(&t));
        return timeTable;
    }

    #define GET_LOGGER() hzd::ErrorLog& logger = hzd::ErrorLog::getLogger()

    #define FORMAT_ARGS(format,...) do{ \
        size_t __length = std::snprintf(nullptr,0,format,##__VA_ARGS__);   \
        if(__length >= 0)                                                \
        {                                                                \
            char* __buf = new char[__length + 1];                        \
            std::snprintf(__buf,__length+1,format,##__VA_ARGS__);          \
            __str = __buf;                                               \
            delete []__buf;                                              \
        }                                                                \
    }while(0)

    /**
     * @brief log msg
     * @note log msg
     * @param msg std::string
     * @retval None
     */

    #define LOG_MSG(msg) do {           \
        GET_LOGGER();                   \
        std::string logmsg = hzd::getTime() + "[INFO]:File:" + __FILE__ + "\tLine:" + std::to_string(__LINE__) + "\t" + "Function:" + __FUNCTION__ + "\tMsg:" + (msg);                     \
        logger << logmsg;               \
    }while(0)

    /**
     * @brief log error with msg
     * @note log error with msg
     * @param error hzd::enum Error {
        None,
        Out_Of_Bound,
        Pointer_To_Null,
        Bad_Malloc
        }
       @param msg std::string
     * @retval None
     */
    #define LOG(error,msg) do { \
        GET_LOGGER();           \
        logger[error];            \
        std::string logmsg = hzd::getTime() + "[ERROR]:" + logger.errorMsg[error] + "\tFile:"+__FILE__+"\tLine:" + std::to_string(__LINE__) + "\t" + "Function:" + __FUNCTION__ + "\tMsg:" + (msg);                     \
        logger << logmsg;                            \
    }while(0)

    /**
     * @brief log with error and format
     * @note log with error and format
     * @param error hzd::enum Error {
        None,
        Out_Of_Bound,
        Pointer_To_Null,
        Bad_Malloc,
        }
     * @param msg std::string
     * @param format std::string
     * @param ... args
     * @retval None
     */
    #define LOG_FMT(error,msg,format,...) do{   \
        std::string __str;                      \
        FORMAT_ARGS(format,##__VA_ARGS__);      \
        GET_LOGGER();                           \
        logger[error];                          \
        std::string logmsg = hzd::getTime() + "[ERROR]:" + logger.errorMsg[error] + "\tFile:"+__FILE__+"\tLine:" + std::to_string(__LINE__) + "\t" + "Function:" + __FUNCTION__ + "\tMsg:" + (msg) + "\tArgs={" + __str + "}";                     \
        logger << logmsg;             \
    }while(0)

    class ErrorLog {
        class Watcher
        {
        public:
            ~Watcher()
            {
                delete logger;
                logger = nullptr;
            }
        };

        static ErrorLog* logger;
        static Watcher watcher;
        static std::mutex mtx;
        #ifndef NO_LOG_FILE
        std::ofstream out;
        #endif
        Error error;

        ErrorLog()
        {
            #ifndef NO_LOG_FILE
            out.open(LOG_FILE_NAME,std::ios::app);
            #endif
            error = None;
        }

    public:
        std::unordered_map<Error,std::string> errorMsg
                {
                        {None,"* No Error *"},
                        {Out_Of_Bound,"* Out Of Bound *"},
                        {Pointer_To_Null,"* Null Pointer *"},
                        {Bad_Malloc,"* Bad Malloc *"},
                        {Socket_Bind,"* Socket Bind *"},
                        {Socket_Create,"* Socket Create *"},
                        {Socket_Listen,"* Socket Listen *"},
                        {Socket_Accept,"* Socket Accept *"},
                        {Socket_Set_Opt,"* Socket Set Options *"},
                        {Epoll_Create,"* Epoll Create *"},
                        {Epoll_Add,"* Epoll Add *"},
                        {Epoll_Wait,"* Epoll Wait *"},
                        {Epoll_Error,"* Epoll Error *"},
                        {Epoll_Read,"* Epoll Read *"},
                        {Epoll_Write,"* Epoll Write *"},
                        {Conn_Send,"* Conn Send *"},
                        {Conn_Recv,"* Conn Recv *"},
                        {Heart_Beat_Timeout,"* Heart Beat Timeout *"}

                };
        static ErrorLog& getLogger()
        {
            std::lock_guard<std::mutex> guard(mtx);
            if(!logger)
            {
                logger = new ErrorLog;
            }
            return *logger;
        }

        void operator <<(const std::string& msg)
        {
            std::lock_guard<std::mutex> guard(mtx);
            ShowMessage(msg,error == None ? CFC_Cyan : CFC_Red);
            #ifndef NO_LOG_FILE
            out << msg << std::endl;
            #endif
            error = None;
        }

        void operator[](Error error_)
        {
            error = error_;
        }

        ~ErrorLog()
        {
            #ifndef NO_LOG_FILE
            out.close();
            #endif
        }
    };

    ErrorLog* ErrorLog::logger = nullptr;
    ErrorLog::Watcher ErrorLog::watcher;
    std::mutex ErrorLog::mtx;
    using Error = hzd::Error;
}
#ifdef LOG_ERROR_ONLY
#undef LOG_MSG
#define LOG_MSG(msg)
#endif

#else
#define LOG_MSG(msg)
#define LOG(error,msg)
#define LOG_FMT(error,msg,fmt,...)
#endif

#endif //ERRORLOG_ERRORLOG_H
