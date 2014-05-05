/***************************************************************
 * Name:      heartbeat.cpp
 * Purpose:
    Implements heartbeat API to allow crash monitoring
 *
 * Author:    Don Julien (djulien@github.com)
 * Created:   2014-02-07
 * Copyright: 2014, part of xLights by Matt Brown
 * License:
     This file is part of xLights, created by Matt Brown (dowdybrown@yahoo.com)

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
 **************************************************************/

#include <algorithm> //std::remove
#include <string>
//#include <string.h>
//#include <istream>
#include <fstream>
#include <iostream>
//#include <sstream>
#include <wx/datetime.h>
#include <wx/filename.h>

#ifdef __WXMSW__ //WINDOWS
#include <windows.h> //NOTE: this must come after a wxWidget #include in order to set up some dependent #defs
#define strncasecmp _strnicmp
#endif
#ifdef LINUX
#include <unistd.h>
#endif

#include "heartbeat.h"

//#define WANT_DEBUG  100
//#define WANT_DEBUG_IMPL
//#include "djdebug.cpp"


//config info:
static struct
{
    std::string logpath; //path name to heartbeat file
    int interval; //how often to write heartbeats
    int maxents; //max #entries to keep (limits file size)
} cfg;

//state info:
static struct
{
//    std::vector<std::string> fifo; //log file contents
    std::string fifo; //log file contents
    int numents; //#ents in log file
    wxDateTime latest; //time of last entry
} state;


//add an entry to fixed-length fifo:
static void append(std::string& fifo, const char* linebuf)
{
//    if (fifo.size() >= cfg.maxents) fifo.erase(fifo.begin()); //pop_front(); make room
//    fifo.push_back(linebuf);
    if (state.numents >= cfg.maxents)
    {
        std::string::size_type ofs = fifo.find("\n");
        if (ofs != std::string::npos)
        {
            fifo.erase(0, ofs + 1); //pop_front(); make room
            --state.numents;
        }
    }
    state.fifo += linebuf;
    state.fifo += "\n";
    ++state.numents;
}


//load fifo into memory:
static void load_fifo(void)
{
    std::ifstream stream;
    std::string linebuf;

    //create file, read previous contents:
    state.numents = 0;
    state.fifo.empty();
    stream.open(cfg.logpath);
//    debug(1, "log file exists? %d", stream.is_open());
    if (!stream.is_open()) return;
    while (std::getline(stream, linebuf))
    {
//        debug(1, "got fifo '%s', #lines %d, lf? %d", linebuf.c_str(), state.numents, linebuf.back() == '\n');
        append(state.fifo, linebuf.c_str());
//        if ((ofs = linebuf.find("]")) != std::string::npos)
//        state.latest.ParseDateTime(linebuf.substr(1, ofs - 1));
        struct tm tm; //&tmint yr, mon, day, hr, min, sec;
        if (sscanf(linebuf.c_str(), "[%d/%d/%d %d:%d:%d]", &tm.tm_mon, &tm.tm_mday, &tm.tm_year, &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) continue;
        --tm.tm_mon; //0-based month#
        tm.tm_year += 100; //2000 => 1900
//        state.latest.Set(tm);
        state.latest = mktime(&tm);
    }
    stream.close();
}


//read config options if heartbeat config file is present:
static void get_cfg(void)
{
//default no logging:
    cfg.logpath.empty();

//get config file path:
    wxString fullpath;
//    wxStringBuffer pathbuf(fullpath, MAX_PATH + 1);
#ifdef __WXMSW__
    GetModuleFileName(GetModuleHandle(NULL), wxStringBuffer(fullpath, MAX_PATH + 1), MAX_PATH);
#else //TODO; maybe use argv[0] although it's not 100% reliable
    fullpath.empty();
#endif
    fullpath = fullpath.BeforeLast(wxFileName::GetPathSeparator());
    fullpath += wxFileName::GetPathSeparator();
    fullpath += "heartbeat.cfg";
//    debug(1, "get cfg path '%s'", (const char*)fullpath);

//try to read config file, extract info:
    std::ifstream stream;
    std::string linebuf;
    stream.open(fullpath.ToStdString());
    if (!stream.is_open()) return;
    while (std::getline(stream, linebuf))
    {
        std::string::size_type ofs;
        if ((ofs = linebuf.find("#")) != std::string::npos) linebuf.erase(ofs); //remove comments
        while (linebuf.size() && (linebuf.back() == ' ')) linebuf.pop_back(); //trim
	    if (linebuf.empty()) continue;
        if ((ofs = linebuf.find("\"")) == std::string::npos) ofs = linebuf.size();
//            remove_if(str.begin(), /*str.end()*/ str.begin() + 9, isspace); //make parsing simpler, but leave spaces in file name
//        if (ofs)
//        std::string::iterator quoted = linebuf.begin();
//        quoted += ofs;
//        linebuf.erase(/*std::string::iterator end_pos =*/ std::remove(&linebuf[0] /*.begin()*/, /*str.end()*/ &linebuf[ofs] /*.begin() + ofs*/, ' '), linebuf.end()); //make parsing simpler, but leave spaces in file name
        linebuf.erase(std::remove(linebuf.begin(), linebuf.begin() + ofs, ' '), linebuf.end());
//        if (ofs)
//        {
//            std::string::iterator end_pos = std::remove(linebuf.begin(), /*str.end()*/ linebuf.begin() + ofs, ' ');
//            linebuf.erase(end_pos, linebuf.begin() + ofs); //make parsing simpler, but leave spaces in file name
//        }
//        debug(1, "got cfg line '%s'", linebuf.c_str());
        if (!strncasecmp(linebuf.c_str(), "Path=", 5)) cfg.logpath = linebuf.substr(5);
        else if (!strncasecmp(linebuf.c_str(), "Maxents=", 8)) cfg.maxents = atoi(linebuf.substr(8).c_str());
        else if (!strncasecmp(linebuf.c_str(), "Interval=", 9)) cfg.interval = atoi(linebuf.substr(9).c_str());
    }
    stream.close();

    load_fifo();
}


//write status message to heartbeat log file (if configured):
void heartbeat(const wxString& msg, bool always)
{
    static bool first = TRUE;
//    debug(1, "hb(%s) first? %d, always? %d", msg, first, always);
    if (first) get_cfg();
    first = FALSE;

    wxDateTime now = wxDateTime::Now();
//    debug(1, "cfg: path '%s', intv %d, max %d", cfg.logpath.c_str(), cfg.interval, cfg.maxents);
    if (cfg.logpath.empty()) return; //no logging
    if (!always)
    {
        wxTimeSpan elapsed = now.Subtract(state.latest);
//            if ( dt.ParseDateTime(Line, &end) )
//        debug(1, "last age: %d vs. %d", elapsed, cfg.interval);
        if (elapsed.GetMilliseconds() < cfg.interval) return; //not time to log yet
    }
    wxString linebuf = now.Format("[%F %T] ");
    linebuf += msg;
//    debug(1, "wr ent '%s'", linebuf.c_str());
    append(state.fifo, linebuf.c_str());

    FILE* hbfile = fopen(cfg.logpath.c_str(), "wt");
//    debug(1, "wr file len %d, #ents %d", hbfile? linebuf.size(): -1, state.numents);
    if (hbfile)
    {
        fputs(state.fifo.c_str(), hbfile); //CAUTION: must be atomic
        fflush(hbfile);
        fclose(hbfile);
    }
    state.latest = now;
}
