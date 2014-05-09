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

#include <wx/datetime.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/msgdlg.h>

#include "heartbeat.h"

//config info:
static struct
{
    wxString logpath; //path name to heartbeat file
    long interval;    //how often to write heartbeats
    long maxents;     //max #entries to keep (limits file size)
} cfg;

//state info:
static struct
{
    wxString fifo;     //log file contents
    int numents;       //#ents in log file
    wxDateTime latest; //time of last entry
} state;


//add an entry to fixed-length fifo:
static void append(const wxString& linebuf)
{
    if (cfg.maxents <= 0) return;
    while (state.numents >= cfg.maxents)
    {
        int ofs = state.fifo.Find("\n");
        if (ofs == wxNOT_FOUND)
        {
            state.fifo.clear();
            state.numents=0;
        }
        else
        {
            state.fifo.erase(0, ofs + 1); // make room
            --state.numents;
        }
    }
    state.fifo += linebuf + "\n";
    ++state.numents;
}


//read config options if heartbeat config file is present:
static void get_cfg()
{
    cfg.logpath.clear();
    cfg.maxents=0;
    cfg.interval=50;
    wxFileName cfgFile = wxFileName::FileName(wxStandardPaths::Get().GetExecutablePath());
    cfgFile.SetFullName("heartbeat.cfg");
    if (!wxFile::Exists(cfgFile.GetFullPath())) {
        return;
    }
    wxFile f(cfgFile.GetFullPath());
    if (!f.IsOpened()) {
        return;
    }
    wxString AllLines;
    f.ReadAll(&AllLines);
    f.Close();
    wxArrayString arrLines=wxSplit(AllLines,'\n');
    for (int i=0; i<arrLines.Count(); i++)
    {
        if (arrLines[i].StartsWith("Path=")) cfg.logpath = arrLines[i].Mid(5).Trim();
        if (arrLines[i].StartsWith("Maxents=")) arrLines[i].Mid(8).ToLong(&cfg.maxents);
        if (arrLines[i].StartsWith("Interval=")) arrLines[i].Mid(9).ToLong(&cfg.interval);
    }

    state.numents = 0;
    state.fifo.empty();
    state.latest=wxDateTime::Today();
}


//write status message to heartbeat log file (if configured):
void heartbeat(const wxString& msg, bool always)
{
    static bool first = TRUE;
//    debug(1, "hb(%s) first? %d, always? %d", msg, first, always);
    if (first)
    {
        get_cfg();
        first = FALSE;
        //wxMessageBox("heartbeat first log="+cfg.logpath);
    }

    wxDateTime now = wxDateTime::Now();
//    debug(1, "cfg: path '%s', intv %d, max %d", cfg.logpath.c_str(), cfg.interval, cfg.maxents);
    if (cfg.logpath.IsEmpty()) return; //no logging
    if (!always)
    {
        wxTimeSpan elapsed = now.Subtract(state.latest);
        if (elapsed.GetMilliseconds() < cfg.interval) return; //not time to log yet
    }
    wxString linebuf = now.Format("[%F %T] ") + msg;
    append(linebuf);

    wxFile f;
    if (f.Open(cfg.logpath,wxFile::OpenMode::write))
    {
        f.Write(state.fifo);
        f.Close();
    }
    state.latest = now;
}
