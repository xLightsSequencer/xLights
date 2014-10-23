#ifndef _DEBUG_H
#define _DEBUG_H

#include <stdarg.h>
#include <stdio.h>
#include "xLightsApp.h"

#define DEBUG_FILE  "C:\\Users\\%USERID%\\Documents\\djdebug.log" //"C:\\debug.log"


#ifdef WANT_DEBUG
 #define debug(level, ...)  MyDebug::Message(level, 0, __LINE__, __VA_ARGS__)
 #define debug_more(level, ...)  MyDebug::Message(level, -1, __LINE__, __VA_ARGS__)
 #define debug_function(level)  MyDebug debug_func(level, __func__, __LINE__)
#else
 #define debug(level, ...)
 #define debug_more(level, ...)
 #define debug_function(level)
#endif // WANT_DEBUG

#ifndef ABS
 #define ABS(thingy)  (((thingy) < 0)? -(thingy): (thingy)) //CAUTION: don't use with side effects
#endif

//int DebugLevel = 10; //set higher for more detail, lower for less detail

#ifdef WANT_DEBUG
#ifdef WANT_DEBUG_IMPL
class MyDebug
{
public:
    int svlevel, svline;
    std::string svname;
    static std::string nesting;
public:
    MyDebug(int level, const char* name, int line): svlevel(level), svline(line), svname(name) { Message(svlevel, 0, svline, "enter %s", svname.c_str()); nesting += "    "; }
    ~MyDebug(void) { nesting.resize(nesting.size() - 4); Message(svlevel, 0, svline, "exit %s", svname.c_str()); }
public:
    static FILE* outf; //= fopen(DEBUG_FILE, "wt");
    static void Message(int level, int goback, int line, ...)
    {
        static FILE* outf = 0; //= fopen(DEBUG_FILE, "w");
        if (level > ABS(WANT_DEBUG)) return; //caller doesn't want this much detail
        if (!outf && !xLightsApp::DebugPath.IsEmpty()) outf = fopen((const char*)xLightsApp::DebugPath.c_str(), "wt");
        if (!outf)
        {
            wxString path = DEBUG_FILE;
//            path.Replace(wxT("%USERID%"), wxGetUserId());
            int ofs = path.Find(wxT("%USERID%"));
            if (ofs != wxNOT_FOUND) path = path.Left(ofs) + wxGetUserId() + path.Mid(ofs + 8);
            outf = fopen((const char*)path.c_str(), "wt");
        }
        if (!outf) outf = stdout;
        static int seqnum = 0;
        if (!seqnum)
        {
            struct tm* timeinfo;
            char timebuf[20];
            time_t now;
            time(&now);
            timeinfo = localtime(&now);
            strftime(timebuf, sizeof(timebuf), "%x %X", timeinfo);
            fprintf(outf, "\ndebug log started %s\n", timebuf);
        }
        va_list argp;
        va_start(argp, line);
        const char* fmt = va_arg(argp, const char*);
        if (goback) fseek(outf, goback, SEEK_CUR); //kludge: overwrite newline to join with prev line
        else fprintf(outf, "[DEBUG@%d %d]%s ", line, seqnum++, nesting.c_str());
        vfprintf(outf, fmt, argp);
        fputs("\n", outf);
        if (WANT_DEBUG < 0) fflush(outf); //slower performance, but helps to ensure we get the info
        va_end(argp);
    }
};
std::string MyDebug::nesting;
#else
class MyDebug
{
public:
    int svlevel, svline;
    std::string svname;
    static std::string nesting;
public:
    MyDebug(int level, const char* name, int line);
    ~MyDebug(void) {}
public:
    static FILE* outf; //= fopen(DEBUG_FILE, "wt");
    static void Message(int level, int goback, int line, ...);
};
#endif
#endif


#if 0
//don't write message until object goes out of scope:
//allows concat additional info for more concise log messages
class DebugFunction
{
public:
    std::string func;
public:
    Debug(const char* fmt, ...)
    {
        va_list argp;
        va_start(argp, fmt);
        update(fmt, argp);
        va_end(argp);
        write(false); //log message at start of scope, but don't flush
    }
    ~Debug() { Append(" ... exit"); write(true); } //log message at end if scope
public:
    Debug& Append(const char* fmt, ...)
    {
        va_list argp;
        va_start(argp, fmt);
        update(fmt, argp);
        va_end(argp);
        return *this; //chainable
    }
    Debug& Flush(bool want_write) { if (want_write) write(true); else msg.clear(); return *this; } //chainable
private:
    void update(const char* fmt, va_list argp)
    {
#if 1
        char buf[1024];
        vsnprintf(buf, sizeof(buf), fmt, argp);
        msg += buf;
#else
        if (msg.capacity() < msg.size() + 512) msg.reserve(msg.size() + 1024); //make room if needed
        int buflen = vsnprintf((char*)msg.data() + msg.size(), 512, fmt, argp);
        msg.resize(msg.size() + buflen);
#endif // 1
    }
    void write(bool flush)
    {
        if (!msg.empty())
        {
            static FILE* outf = fopen(DEBUG_FILE, "wt");

            if (!outf) outf = stdout;
            fputs(msg.c_str(), outf);
            fputs("\n", outf);
            fflush(outf);
        }
        if (flush) msg.clear();
    }
};

class NoDebug
{
public:
    std::string msg;
public:
    NoDebug(const char* fmt, ...) {}
    ~NoDebug() {}
public:
    NoDebug& Append(const char* fmt, ...) {}
    NoDebug& Flush(bool want_write) {}
};

#endif // 0
#endif // _DEBUG_H
