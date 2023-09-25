/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/filename.h>
#include <wx/config.h>
#include <wx/regex.h>
#include <wx/sckaddr.h>
#include <wx/dir.h>
#include <wx/socket.h>
#include <wx/mimetype.h>
#include <wx/display.h>
#include <wx/protocol/http.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>

#include <random>
#include <time.h>
#include <thread>

#include "UtilFunctions.h"
#include "xLightsVersion.h"
#include "ExternalHooks.h"


#include "../xSchedule/wxJSON/json_defs.h"
#include "../xSchedule/wxJSON/jsonreader.h"
#include "../xSchedule/wxJSON/jsonval.h"

#include "utils/Curl.h"
#include "utils/string_utils.h"

#include <mutex>
#include <string_view>

#ifdef __WXMSW__
#include <psapi.h>
#include <iphlpapi.h>
#else
#include <sys/socket.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#endif

#ifdef __WXOSX__
#include <sys/sysctl.h>
#endif

#ifdef LINUX
#include <sys/sysinfo.h>
#endif

#include <log4cpp/Category.hh>

#if defined (_MSC_VER)  // Visual studio
#define thread_local __declspec( thread )
#elif defined (__GCC__) // GCC
#define thread_local __thread
#endif


void DisplayError(const std::string& err, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.error("DisplayError: %s", (const char*)err.c_str());
    wxMessageBox(err, "Error", wxICON_ERROR | wxOK, win);
}

void DisplayWarning(const std::string& warn, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.warn("DisplayWarning: %s", (const char*)warn.c_str());
    wxMessageBox(warn, "Warning", wxICON_WARNING | wxOK, win);
}

void DisplayInfo(const std::string& info, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.info("DisplayInfo: %s", (const char*)info.c_str());
    wxMessageBox(info, "Information", wxICON_INFORMATION | wxOK, win);
}

void DisplayCrit(const std::string& crit, wxWindow* win)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.crit("DisplayCrit: %s", (const char*)crit.c_str());
    wxMessageBox(crit, "CRITICAL", wxICON_ERROR | wxOK, win);
}

std::string DecodeIPError(wxSocketError err)
{
    switch (err)
    {
    case wxSOCKET_NOERROR:
        return "No Error";
    case wxSOCKET_INVOP:
        return "Invalid Operation";
    case wxSOCKET_IOERR:
        return "IO Error";
    case wxSOCKET_INVADDR:
        return "Invalid Address";
    case wxSOCKET_INVSOCK:
        return "Invalid Socket";
    case wxSOCKET_NOHOST:
        return "No Host";
    case wxSOCKET_INVPORT:
        return "Invalid Port";
    case wxSOCKET_WOULDBLOCK:
        return "Would Block";
    case wxSOCKET_TIMEDOUT:
        return "Timeout";
    case wxSOCKET_MEMERR:
        return "Memory Error";
    case wxSOCKET_OPTERR:
        return "Option Error";
    default:
        return "God knows what happened";
    }
}

std::string DecodeMidi(int midi)
{
    int n = midi % 12;
    int o = midi / 12 - 1;
    // dont go below zero ... if so move it up an octave ... the user will never know
    while (o < 0)
    {
        o++;
    }

    bool sharp = false;
    char note = '?';
    switch (n)
    {
    case 9:
        note = 'A';
        break;
    case 10:
        note = 'A';
        sharp = true;
        break;
    case 11:
        note = 'B';
        break;
    case 0:
        note = 'C';
        break;
    case 1:
        note = 'C';
        sharp = true;
        break;
    case 2:
        note = 'D';
        break;
    case 3:
        note = 'D';
        sharp = true;
        break;
    case 4:
        note = 'E';
        break;
    case 5:
        note = 'F';
        break;
    case 6:
        note = 'F';
        sharp = true;
        break;
    case 7:
        note = 'G';
        break;
    case 8:
        note = 'G';
        sharp = true;
        break;
    default:
        break;
    }

    if (sharp)
    {
        return wxString::Format("%c#%d", note, o).ToStdString();
    }
    return wxString::Format("%c%d", note, o).ToStdString();
}


static std::list<std::string> SearchDirectories;
void SetFixFileDirectories(const std::list<std::string>& dirs) {
    SearchDirectories = dirs;
}

wxArrayString Split(const wxString& s, const std::vector<char>& delimiters)
{
    wxArrayString res;

    wxString w;
    for (const auto it : s)
    {
        bool delim = false;
        for (auto it2 : delimiters)
        {
            if (it == it2)
            {
                delim = true;
                break;
            }
        }
        if (delim)
        {
            res.Add(w);
            w = "";
        }
        else
        {
            w += it;
        }
    }
    res.Add(w);

    return res;
}


static bool doesFileExist(const wxString &dir, const wxString &origFileWin, const wxString &origFileUnix, wxString &path) {
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (origFileWin != "") {
        wxFileName fn3(dir, origFileWin);
        if (FileExists(fn3, false)) {
            logger_base.debug("File location fixed: " + origFileWin + " -> " + fn3.GetFullPath());
            path = fn3.GetFullPath();
            return true;
        }
    }
    if (origFileUnix != "") {
        wxFileName fn4(dir, origFileUnix);
        if (FileExists(fn4, false)) {
            logger_base.debug("File location fixed: " + origFileWin + " -> " + fn4.GetFullPath());
            path = fn4.GetFullPath();
            return true;
        }
    }
    return false;
}
static bool doesFileExist(const wxString &sd, const wxString &append,
                          const wxString &nameWin, const wxString &nameUnix, wxString &newPath) {
    if (doesFileExist(sd + append, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        return true;
    }
    for (auto &fd : SearchDirectories) {
        if (doesFileExist(fd + append, nameWin, nameUnix, newPath)) {
            // file exists in one of the resource directories
            return true;
        }
    }
    return false;
}
static bool doesFileExist(const wxString &sd, const wxString &appendWin, const wxString &appendUnx,
                          const wxString &nameWin, const wxString &nameUnix, wxString &newPath) {
    if (doesFileExist(sd + appendWin, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        return true;
    }
    if (doesFileExist(sd + appendUnx, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        return true;
    }
    for (auto &fd : SearchDirectories) {
        if (doesFileExist(fd + appendUnx, nameWin, nameUnix, newPath)) {
            // file exists in one of the resource directories
            return true;
        }
        if (doesFileExist(fd + appendUnx, nameWin, nameUnix, newPath)) {
            // file exists in one of the resource directories
            return true;
        }
    }
    return false;
}


static wxString RememberShowDir;
void SetFixFileShowDir(const wxString& ShowDir) {
    RememberShowDir = ShowDir;
}

static std::recursive_mutex __fixFilesMutex;
static std::vector<std::string> __nonExistentFiles;

void ClearNonExistentFiles()
{
    std::unique_lock<std::recursive_mutex> lock(__fixFilesMutex);
    __nonExistentFiles.clear();
}

wxString FixFile(const wxString& ShowDir, const wxString& file)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    static std::map<wxString, wxString> __fileMap;

    if (ShowDir != RememberShowDir && ShowDir != "" && RememberShowDir == "") {
        RememberShowDir = ShowDir;
    }

    if (file == "") {
        return file;
    }

    if (FileExists(file, false)) {
        return file;
    }

    std::unique_lock<std::recursive_mutex> lock(__fixFilesMutex);

    // Lookup previous mappings as this is faster
    if (__fileMap.find(file) != __fileMap.end()) {
        return __fileMap[file];
    }

    if (std::find(begin(__nonExistentFiles), end(__nonExistentFiles), file.ToStdString()) != end(__nonExistentFiles)) {
        // we have looked before and this file does not exist ... so dont look again
        return file;
    }

    wxString sd;
    if (ShowDir == "") {
        sd = RememberShowDir;
    } else {
        sd = ShowDir;
    }
    // done with __nonExistentFiles and __fileMap for right now, we'll unlock
    // so other threads can access them, but we'll need to relock when we add entries later
    lock.unlock();
    logger_base.debug("File not found ... attempting to fix location (" + sd + ") : " + file);

    // I dont know what this is trying to fix but it blows up on windows
    wxFileName fnUnix(file, wxPATH_UNIX);
    wxFileName fnWin(file, wxPATH_WIN);
    wxString nameUnix = fnUnix.GetFullName();
#ifdef __WXMSW__
    // This should stop the blowup but to be honest it wont produce any different result to fnWin so it all looks like wasted effort
    if (nameUnix.Contains("\\")) {
        nameUnix = nameUnix.AfterLast('\\');
    }
#endif
    wxString nameWin = fnWin.GetFullName();
    wxString newPath;
    if (doesFileExist(sd, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        lock.lock();
        __fileMap[file] = newPath;
        return newPath;
    }
    for (auto &fd : SearchDirectories) {
        if (doesFileExist(fd, nameWin, nameUnix, newPath)) {
            // file exists in one of the resource directories
            lock.lock();
            __fileMap[file] = newPath;
            return newPath;
        }
    }

    wxDir dir(sd);
    if (dir.IsOpened()) {
        wxString foldername;
        // Should we follow symbolic links ... doing do would allow common folders shared between folders which seems desirable ... but I have seen people put
        // stupid links there which has then massively slowed down processing
        bool cont = dir.GetFirst(&foldername, "*", wxDIR_DIRS/* | wxDIR_NO_FOLLOW*/);
        while ( cont ) {
            if (foldername.Lower() != "backup") { // dont look in backup folder
                auto const folder = sd + wxFileName::GetPathSeparator() + foldername;
                if (doesFileExist(folder, nameWin, nameUnix, newPath)) {
                    lock.lock();
                    __fileMap[file] = newPath;
                    return newPath;
                }
            }
            cont = dir.GetNext(&foldername);
        }
    }

    wxString flc = file;
    flc.LowerCase();

    wxString path;
    wxString fname;
    wxString ext;
    wxFileName::SplitPath(sd, &path, &fname, &ext, wxPATH_WIN);
    if (fname == "" || fname.Contains("/")) {
        wxFileName::SplitPath(sd, &path, &fname, &ext, wxPATH_UNIX);
    }
    if (fname == "") {
        // no subdirectory
        lock.lock();
        __nonExistentFiles.push_back(file.ToStdString());
        return file;
    }

    wxString showfolder = fname;
    wxString sflc = showfolder;
    sflc.LowerCase();

    wxString appendWin;
    wxString appendUnx;

    bool appending = false;
    for (size_t x = 0; x < fnWin.GetDirs().size(); x++) {
        if (fnWin.GetDirs()[x].Lower() == sflc) {
            appending = true;
        } else if (appending) {
            appendWin += wxFileName::GetPathSeparator();
            appendWin += fnWin.GetDirs()[x];
        }
    }
    appending = false;
    for (size_t x = 0; x < fnUnix.GetDirs().size(); x++) {
        if (fnUnix.GetDirs()[x].Lower() == sflc) {
            appending = true;
        } else if (appending) {
            appendUnx += wxFileName::GetPathSeparator();
            appendUnx += fnUnix.GetDirs()[x];
        }
    }
    if (doesFileExist(sd, appendWin, appendUnx, nameWin, nameUnix, newPath)) {
        // file exists
        lock.lock();
        __fileMap[file] = newPath;
        return newPath;
    }

    if (flc.Contains(sflc)) {
        int offset = flc.Find(sflc) + showfolder.Length();
        wxString relative = file.SubString(offset, file.Length());
        wxFileName sdFn =  wxFileName::DirName(sd);
        if (FileExists(relative, false)) {
            lock.lock();
            __fileMap[file] = relative;
            return relative;
        }
    }
    // if we get here, the show folders could not be matched, let's try the reverse and traverse up the
    // the files directories to see if we can get a match
    appendWin = "";
    for (int x = fnWin.GetDirs().size()-1; x >= 0;x--) {
        appendWin = wxFileName::GetPathSeparator() + fnWin.GetDirs()[x] + appendWin;
        if (doesFileExist(sd, appendWin, nameWin, nameUnix, newPath)) {
            // file exists
            lock.lock();
            __fileMap[file] = newPath;
            return newPath;
        }
    }
    appendUnx = "";
    for (int x = fnUnix.GetDirs().size()-1; x >= 0;x--) {
        appendUnx = wxFileName::GetPathSeparator() + fnUnix.GetDirs()[x] + appendUnx;
        if (doesFileExist(sd, appendUnx, nameWin, nameUnix, newPath)) {
            // file exists
            lock.lock();
            __fileMap[file] = newPath;
            return newPath;
        }
    }

    if (ShowDir == "" && fnWin.GetDirCount() > 0) {
        return FixFile(sd + "\\" + fnWin.GetDirs().Last(), file);
    }
    if (ShowDir == "" && fnUnix.GetDirCount() > 0) {
        return FixFile(sd + "/" + fnUnix.GetDirs().Last(), file);
    }
    logger_base.debug("   could not find a fixed file location for : " + file);
    logger_base.debug("   We will not look for this file again until a new sequence is loaded.");
    lock.lock();
    __nonExistentFiles.push_back(file.ToStdString());
    return file;
}
bool IsFileInShowDir(const wxString& showDir, const std::string filename)
{
    wxString fixedFile = FixFile(showDir, filename);

#ifdef __WXMSW__
    fixedFile = fixedFile.Lower();
#endif

    if (fixedFile.StartsWith(showDir
#ifdef __WXMSW__
        .Lower()
#endif
    )) {
        return true;
    }
    for (auto &d : SearchDirectories) {
#ifdef __WXMSW__
        transform(d.begin(), d.end(), d.begin(), ::tolower);
#endif
        if (fixedFile.StartsWith(d)) {
            return true;
        }
    }
    return false;
}
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir)
{
    int startparamname = parametervalue.Find(paramname);
    int endparamname = parametervalue.find("=", startparamname) - 1;
    int startvalue = endparamname + 2;
    int endvalue = parametervalue.find(",", startvalue) - 1;
    wxString file = parametervalue.SubString(startvalue, endvalue);
    wxString newfile = FixFile(ShowDir, file);
    wxString rc = parametervalue.Left(startvalue) + newfile + parametervalue.Right(parametervalue.Length() - endvalue - 1);
    return rc;
}

std::string UnXmlSafe(const std::string &res)
{
    if (Contains(res, "&")) {
        std::string r2(res);
        for (int i = 0; i< 32; ++i) {
            std::string ss = "&#" + std::to_string(i);
            char buf[2] = {(char)i, 0};
            Replace(r2, ss, buf);
        }
        Replace(r2, "&lt;", "<");
        Replace(r2, "&gt;", ">");
        Replace(r2, "&apos;", "'");
        Replace(r2, "&quot;", "\"");
        Replace(r2, "&amp;", "&");
        return r2;
    }
    return res;
}

std::string XmlSafe(const std::string& s)
{
    std::string res = "";
    for (auto c = s.begin(); c != s.end(); ++c)
    {
        if ((int)(*c) < 32)
        {
            //res += wxString::Format("&#x%x;", (int)(*c));
            int cc = (int)*c;
            if (cc == 9 || cc == 10 || cc == 13)
            {
                res += wxString::Format("&#%d;", (int)(*c)).ToStdString();
            }
        }
        else if (*c == '&')
        {
            res += "&amp;";
        }
        else if (*c == '<')
        {
            res += "&lt;";
        }
        else if (*c == '>')
        {
            res += "&gt;";
        }
        else if (*c == '\'')
        {
            res += "&apos;";
        }
        else if (*c == '\"')
        {
            res += "&quot;";
        }
        else
        {
            res += (*c);
        }
    }

    return res;
}

// This takes a string and removes all problematic characters from it for an XML file
std::string RemoveUnsafeXmlChars(const std::string& s)
{
    std::string res;

    for (int i = 0; i < s.size(); i++) {
        if ((int)s[i] < 32 || s[i] > 127) {
            // strip it
        }
        else if (s[i] == '=' || s[i] == '<' || s[i] == '>' || s[i] == '&' || s[i] == '"' || s[i] == '\'') {
            // strip them too
        }
        else res += s[i];
    }

    return res;
}

std::string EscapeCSV(const std::string& s)
{
    std::string res = "";
    for (auto c : s)
    {
        if (c == '\"')
        {
            res += "\"\"";
        }
        else
        {
            res += c;
        }
    }

    return res;
}

std::string EscapeRegex(const std::string& s)
{
    // \,*,+,?,|,{,[, (,),^,$,.,#,
    constexpr std::string_view BADREX {R"(\*+?|{[()^$,#)"};
    std::string safe;
    for (auto& c : s) {
        if (BADREX.find(c) != std::string::npos) {
            safe += "\\";
        }
        safe += c;
    }
    return safe;
}

wxString GetXmlNodeAttribute(wxXmlNode* parent, const std::string& path, const std::string& attribute, const std::string& def)
{
    wxXmlNode* curr = parent;
    auto pe = wxSplit(path, '/');

    for (const auto& it : pe)
    {
        for (wxXmlNode* n = curr->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == it)
            {
                if (it == pe.back())
                {
                    return n->GetAttribute(attribute);
                }

                curr = n;
                break;
            }
        }
    }

    return def;
}

wxString GetXmlNodeContent(wxXmlNode* parent, const std::string& path, const std::string& def)
{
    wxXmlNode* curr = parent;
    auto pe = wxSplit(path, '/');

    for (const auto& it : pe)
    {
        for (wxXmlNode* n = curr->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == it)
            {
                if (it == pe.back())
                {
                    if (n->GetChildren() != nullptr) return n->GetChildren()->GetContent();
                    return def;
                }

                curr = n;
                break;
            }
        }
    }

    return def;
}

std::vector<std::string> GetXmlNodeListContent(wxXmlNode* parent, const std::string& path, const std::string& listNodeName)
{
    std::vector<std::string> res;

    wxXmlNode* curr = parent;
    auto pe = wxSplit(path, '/');

    for (const auto& it : pe)
    {
        for (wxXmlNode* n = curr->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == it)
            {
                curr = n;
                break;
            }
        }
    }

    if (curr != nullptr)
    {
        for (wxXmlNode* n = curr->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == listNodeName && n->GetChildren() != nullptr)
            {
                res.push_back(n->GetChildren()->GetContent());
            }
        }
    }

    return res;
}

bool DoesXmlNodeExist(wxXmlNode* parent, const std::string& path)
{
    wxXmlNode* curr = parent;
    auto pe = wxSplit(path, '/');

    for (const auto& it : pe)
    {
        for (wxXmlNode* n = curr->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == it)
            {
                if (it == pe.back()) return true;

                curr = n;
                break;
            }
        }
    }

    return false;
}

void SetXmlNodeAttribute(wxXmlNode* node, wxString const& property, wxString const& value)
{
    if (node->HasAttribute(property)) {
        node->DeleteAttribute(property);
    }
    node->AddAttribute(property, value);
}

void DownloadVamp()
{
    wxMessageBox("We are about to download the Queen Mary Vamp plugins for your platform. Once downloaded please install them and then close and reopen xLights to use them.");
#ifdef __WXMSW__
    if (GetBitness() == "64bit") {
        ::wxLaunchDefaultBrowser("https://code.soundsoftware.ac.uk/attachments/download/2623/qm-vamp-plugins-1.8.0-win64.msi");
    } else {
        ::wxLaunchDefaultBrowser("https://code.soundsoftware.ac.uk/attachments/download/2621/qm-vamp-plugins-1.8.0-win32.zip");
    }
#elif defined(__WXOSX__) && defined(__aarch64__)
    // plugin pack is only for x86_64 and won't run native on M1 macs, download build of QM
    ::wxLaunchDefaultBrowser("https://dankulp.com/xlights/archive/qm-vamp-plugins-1.8.dmg");
#else
    // likely can/should be used for all platforms
    ::wxLaunchDefaultBrowser("https://code.soundsoftware.ac.uk/projects/vamp-plugin-pack");
#endif
}

inline bool is_base64(unsigned char c)
{
    return (isalnum(c) || (c == '+') || (c == '/'));
}

//returns number of chars at the end that couldn't be decoded
int base64_decode(const wxString& encoded_string, std::vector<unsigned char> &data)
{
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t in_len = encoded_string.size();
    int i = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_]))
    {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4)
        {
            for (i = 0; i <4; i++)
            {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++)
            {
                data.resize(data.size() + 1);
                data[data.size() - 1] = char_array_3[i];
            }
            i = 0;
        }
    }

    if (i && encoded_string[in_] == '=')
    {
        for (int j = i; j <4; j++)
        {
            char_array_4[j] = 0;
        }

        for (int j = 0; j <4; j++)
        {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int j = 0; (j < i - 1); j++)
        {
            data.resize(data.size() + 1);
            data[data.size() - 1] = char_array_3[j];
        }
    }
    return i;
}

bool IsVersionOlder(const std::string &compare, const std::string &version)
{
    wxArrayString compare_parts = wxSplit(compare, '.');
    wxArrayString version_parts = wxSplit(version, '.');
    if (wxAtoi(version_parts[0]) < wxAtoi(compare_parts[0])) return true;
    if (wxAtoi(version_parts[0]) > wxAtoi(compare_parts[0])) return false;
    if (wxAtoi(version_parts[1]) < wxAtoi(compare_parts[1])) return true;
    if (wxAtoi(version_parts[1]) > wxAtoi(compare_parts[1])) return false;
    // From 2016 versions only have 2 parts
    if (version_parts.Count() == 2 || compare_parts.Count() == 2)
    {
        if (version_parts.Count() > 2)
        {
            return false; // remote version has 2 components but local has three so local must be newer
        }
        return true;
    }
    else
    {
        if (wxAtoi(version_parts[2]) < wxAtoi(compare_parts[2])) return true;
    }
    return false;
}

std::string JSONSafe(const std::string& s)
{
    std::string safe;
    for (auto& c : s) {
        if (c == '\\')
            safe += "\\\\";
        else if (c == '"')
            safe += "\\\"";
        else
            safe += c;
    }
    return safe;
}

void SaveInt(const std::string& tag, int value)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write(tag, value);
}

int LoadInt(const std::string& tag, int defaultValue)
{
    wxConfigBase* config = wxConfigBase::Get();
    return config->ReadLong(tag, defaultValue);
}

void SaveWindowPosition(const std::string& tag, wxWindow* window)
{
    wxConfigBase* config = wxConfigBase::Get();
    if (window != nullptr)
    {
        wxPoint position = window->GetPosition();
        wxSize size = window->GetSize();
        config->Write(tag, wxString::Format("%d|%d|%d|%d", position.x, position.y, size.GetWidth(), size.GetHeight()));
    }
    else
    {
        config->DeleteEntry(tag);
    }
}

void LoadWindowPosition(const std::string& tag, wxSize& size, wxPoint& position)
{
    wxConfigBase* config = wxConfigBase::Get();

    wxString pos = config->Read(tag, "");

    position.x = -1;
    position.y = -1;
    size.SetWidth(-1);
    size.SetHeight(-1);

    wxArrayString ss = wxSplit(pos, '|');

    if (ss.size() == 4)
    {
        position.x = wxAtoi(ss[0]);
        position.y = wxAtoi(ss[1]);
        size.SetWidth(wxAtoi(ss[2]));
        size.SetHeight(wxAtoi(ss[3]));
    }
}

// Extract all chars before the first number in the string ... strip it from the input string
std::string BeforeInt(std::string& s)
{
    size_t i = 0;
    while (i < s.size() && (s[i] > '9' || s[i] < '0')) {
        i++;
    }
    if (i == 0) {
        return "";
    }

    std::string res = s.substr(0, i);
    s = s.substr(i);
    return res;
}

int intRand(const int& min, const int& max) {
    static thread_local std::mt19937* generator = nullptr;
    if (!generator) generator = new std::mt19937(clock() + std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(*generator);
}

// Extract any leading number ... strip it from the input string
int ExtractInt(std::string& s)
{
    size_t i = 0;
    while (i < s.size() && s[i] <= '9' && s[i] >= '0') {
        i++;
    }

    if (i == 0) {
        return -1;
    }

    int res = std::stoi(s.substr(0, i));
    s = s.substr(i);
    return res;
}

int NumberAwareStringCompare(const std::string &a, const std::string &b)
{
    std::string aa = a;
    std::string bb = b;

    while (true) {
        std::string abi = BeforeInt(aa);
        std::string bbi = BeforeInt(bb);

        if (abi == bbi) {
            int ia = ExtractInt(aa);
            int ib = ExtractInt(bb);

            if (ia == ib) {
                if (aa == bb) {
                    return 0;
                }
            } else {
                if (ia < ib) {
                    return -1;
                }
                return 1;
            }
        } else {
            if (abi < bbi) {
                return -1;
            }
            return 1;
        }
    }
}

bool IsExcessiveMemoryUsage(double physicalMultiplier)
{
#if defined(__WXMSW__) && defined(__WIN64__)
    ULONGLONG physical;
    if (GetPhysicallyInstalledSystemMemory(&physical) != 0) {
        PROCESS_MEMORY_COUNTERS_EX mc;
        if (::GetProcessMemoryInfo(::GetCurrentProcess(), (PPROCESS_MEMORY_COUNTERS)&mc, sizeof(mc)) != 0) {
            // if we are using more ram than the machine has times the multiplier
            if (mc.PagefileUsage / 1024 > physicalMultiplier * physical) {
                return true;
            }
        }
    }
    return false;
#elif defined(__WXOSX__)
    // max of 24G of swap used
    constexpr size_t MAX = 24l * 1024 * 1024 * 1024;
    return wxDir::GetTotalSize("/System/Volumes/VM") > MAX;
#else
    // test memory availability by allocating 200MB ... if it fails then treat this as a low memory problem
    //void* test = malloc(200 * 1024 * 1024);
    //if (test == nullptr)
    //{
    //    return true;
    //}
    //free(test);
    return false;
#endif
}

std::list<std::string> GetLocalIPs()
{
    std::list<std::string> res;

#ifdef __WXMSW__
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == nullptr) {
        logger_base.error("Error getting adapter info.");
        return res;
    }

    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
        if (pAdapterInfo == nullptr) {
            logger_base.error("Error getting adapter info.");
            return res;
        }
    }

    PIP_ADAPTER_INFO pAdapter = nullptr;
    DWORD dwRetVal = 0;

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {
        pAdapter = pAdapterInfo;
        while (pAdapter) {

            auto ip = &pAdapter->IpAddressList;
            while (ip != nullptr) {
                if (wxString(ip->IpAddress.String) != "0.0.0.0") {
                    res.push_back(std::string(ip->IpAddress.String));
                }
                ip = ip->Next;
            }

            pAdapter = pAdapter->Next;
        }
    }
    free(pAdapterInfo);
#else
    struct ifaddrs* interfaces, * tmp;
    getifaddrs(&interfaces);
    tmp = interfaces;
    //loop through all the interfaces
    while (tmp) {
        if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET) {
            struct sockaddr_in* address = (struct sockaddr_in*)tmp->ifa_addr;
            std::string ip = inet_ntoa(address->sin_addr);
            if (ip != "0.0.0.0") {
                res.push_back(ip);
            }
        }
        else if (tmp->ifa_addr && tmp->ifa_addr->sa_family == AF_INET6) {
            //LogDebug(VB_SYNC, "   Inet6 interface %s\n", tmp->ifa_name);
        }
        tmp = tmp->ifa_next;
    }
    freeifaddrs(interfaces);
#endif

    return res;
}

bool IsValidLocalIP(const std::string& ip)
{
    for (const auto& it : GetLocalIPs()) {
        if (it == ip) return true;
    }

    return false;
}

bool IsValidLocalIP(const wxIPV4address& ip)
{
    return IsValidLocalIP(ip.IPAddress().ToStdString());
}

bool IsInSameSubnet(const std::string& ip1, const std::string& ip2, const std::string& mask)
{
    wxIPV4address i1;
    wxIPV4address i2;
    wxIPV4address m;

    i1.Hostname(ip1);
    i2.Hostname(ip2);
    m.Hostname(mask);

    if (i1.GetAddressData() == nullptr || i2.GetAddressData() == nullptr || m.GetAddressData() == nullptr) return false;

    return ((((sockaddr_in*)i1.GetAddressData())->sin_addr.s_addr & ((sockaddr_in*)m.GetAddressData())->sin_addr.s_addr) ==
        (((sockaddr_in*)i2.GetAddressData())->sin_addr.s_addr & ((sockaddr_in*)m.GetAddressData())->sin_addr.s_addr));
}

bool DeleteDirectory(std::string directory)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    bool res = true;
    logger_base.debug("  Processing directory: %s.", (const char*)directory.c_str());
    if (wxDirExists(directory)) {
        wxDir d;
        if (d.Open(directory)) {
            wxString filename;
            bool found = d.GetFirst(&filename, "", wxDIR_FILES | wxDIR_HIDDEN | wxDIR_NO_FOLLOW);
            if (!found) {
                logger_base.debug("  No files found.");
            }
            while (found && res) {
                auto ff = directory + wxFileName::GetPathSeparator() + filename;
                logger_base.debug("  Deleting file: %s.", (const char*)ff.c_str());
                if (!wxRemoveFile(ff)) {
                    logger_base.error("    Could not delete file %s.", (const char*)ff.c_str());
                    res = false;
                }
                found = d.GetNext(&filename);
            }

            found = d.GetFirst(&filename, "", wxDIR_DIRS | wxDIR_HIDDEN | wxDIR_NO_FOLLOW);
            if (!found) {
                logger_base.debug("  No subdirectories found.");
            }
            while (found && res) {
                auto dd = directory + wxFileName::GetPathSeparator() + filename;
                logger_base.debug("  Deleting directory: %s.", (const char*)dd.c_str());
                res &= DeleteDirectory(dd);
                found = d.GetNext(&filename);
            }

            if (!wxRmdir(directory)) {
                logger_base.error("    Could not delete folder %s.", (const char*)directory.c_str());
                res = false;
            }
        }
        else {
            logger_base.error("  Thats odd ... the directory cannot be opened: %s.", (const char*)directory.c_str());
            res = false;
        }
    }
    else {
        logger_base.error("  Thats odd ... the directory cannot be found: %s.", (const char*)directory.c_str());
        res = false;
    }

    return res;
}

std::string Ordinal(int i)
{
    wxString ii = wxString::Format("%d", i);

    if (ii.EndsWith("11") || ii.EndsWith("12") || ii.EndsWith("13")) {
        return (ii + "th").ToStdString();
    }
    else if (ii.EndsWith("1")) {
        return (ii + "st").ToStdString();
    }
    else if (ii.EndsWith("2")) {
        return (ii + "nd").ToStdString();
    }
    else if (ii.EndsWith("3")) {
        return (ii + "rd").ToStdString();
    }
    else {
        return (ii + "th").ToStdString();
    }
}

bool IsEmailValid(const std::string& email)
{
    wxString e = wxString(email).Trim(false).Trim(true);
    if (e == "") {
        return false;
    }
    else {
        static wxRegEx regxEmail("^([a-zA-Z0-9\\.!#$%&+\\/=?^_`{|}~\\-]*@[a-zA-Z0-9\\-]+\\.[a-zA-Z0-9\\=\\.]+)$");

        if (regxEmail.Matches(e)) {
            return true;
        }
    }
    return false;
}

int GetxFadePort(int xfp)
{
    if (xfp == 0) return 0;
    return xfp + 49912;
}

void EnsureWindowHeaderIsOnScreen(wxWindow* win)
{
    int headerHeight = wxSystemSettings::GetMetric(wxSystemMetric::wxSYS_CAPTION_Y, win);
    wxSize size = win->GetSize();
    wxPoint pos = win->GetPosition();

    if (wxDisplay::GetFromPoint(wxPoint(pos.x, pos.y)) < 0 &&
        wxDisplay::GetFromPoint(wxPoint(pos.x, pos.y + headerHeight)) < 0 &&
        wxDisplay::GetFromPoint(wxPoint(pos.x + size.x, pos.y)) < 0 &&
        wxDisplay::GetFromPoint(wxPoint(pos.x + size.x, pos.y + headerHeight)) < 0) {

        // window header is not on screen
        win->Move(0, 0);
    }
}

void OptimiseDialogPosition(wxDialog* dlg)
{
    wxPoint pos = wxGetMousePosition();
    wxSize sz = dlg->GetSize();
    pos.x -= sz.GetWidth() / 2;
    pos.y -= sz.GetHeight() / 2;

    // ensure it is on a single screen
    int d = wxDisplay::GetFromPoint(wxGetMousePosition());
    if (d < 0) d = 0;
    wxDisplay display(d);
    if (display.IsOk()) {
        wxRect displayRect = display.GetClientArea();
        if (pos.y < displayRect.GetTop()) pos.y = displayRect.GetTop();
        if (pos.y + sz.GetHeight() > displayRect.GetBottom()) pos.y = displayRect.GetBottom() - sz.GetHeight();
        if (pos.x < displayRect.GetLeft()) pos.x = displayRect.GetLeft();
        if (pos.x + sz.GetWidth() > displayRect.GetRight()) pos.x = displayRect.GetRight() - sz.GetWidth();
    }

    dlg->SetPosition(pos);
    EnsureWindowHeaderIsOnScreen(dlg);
}

wxJSONValue xLightsRequest(int xFadePort, const wxString& message, const wxString& ipAddress)
{
    std::string url = "http://" + ipAddress + ":" + std::to_string(GetxFadePort(xFadePort)) + "/xlDoAutomation";
    int responseCode = 0;
    auto resultString = Curl::HTTPSPost(url, message, "", "", "application/json", 30*60, {}, &responseCode);
    if (resultString != "" && (responseCode == 200 || responseCode >= 500)) {
        wxJSONValue result;
        wxJSONReader reader;
        reader.Parse(resultString, &result);
        result["res"] = (int)responseCode;
        return result;
    }

    wxString msg = "{\"res\":504,\"msg\":\"Unable to connect.\"}";
    wxJSONValue result;
    wxJSONReader reader;
    reader.Parse(msg, &result);

    return result;
}
bool xLightsRequest(std::string &result, int xFadePort, const wxString& request, const wxString& ipAddress) {
    std::string url = "http://" + ipAddress + ":" + std::to_string(GetxFadePort(xFadePort)) + "/" + request;
    int responseCode = 0;
    result = Curl::HTTPSGet(url, "", "", 30*60, {}, &responseCode);
    return responseCode == 200;
}

void ViewTempFile(const wxString& content, const wxString& name, const wxString& type)
{
    wxFile f;
    const wxString filename = wxFileName::CreateTempFileName(name) + "." + type;

    f.Open(filename, wxFile::write);
    if (!f.IsOpened()) {
        DisplayError("Unable to create " + filename + " file. skip.");
        return;
    }

    if (f.IsOpened()) {
        f.Write(content);

        f.Close();

        wxFileType* ft = wxTheMimeTypesManager->GetFileTypeFromExtension(type);
        if (ft != nullptr) {
            const wxString command = ft->GetOpenCommand(filename);

            if (command.IsEmpty()) {
                DisplayError(wxString::Format("Unable to show " + name + " file '%s'.", filename).ToStdString());
            }
            else {
                wxUnsetEnv("LD_PRELOAD");
                wxExecute(command);
            }
            delete ft;
        }
    }
}

uint64_t GetPhysicalMemorySizeMB() {
uint64_t ret = 0;
#if defined(__WXOSX__)
    int mib [] = { CTL_HW, HW_MEMSIZE };
    size_t length = sizeof(ret);
    sysctl(mib, 2, &ret, &length, NULL, 0);
    ret /= 1024; // -> KB
#elif defined(__WXMSW__)
    GetPhysicallyInstalledSystemMemory(&ret);
    // already in KB
#else
    ret = get_phys_pages();
    ret *= getpagesize();
    ret /= 1024; // -> KB
#endif
    ret /= 1024; // -> MB
    return ret;
}


void CheckMemoryUsage(const std::string& reason, bool onchangeOnly)
{
#if defined(TURN_THIS_OFF) && defined(__WXMSW__)
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static long lastPrivate = 0;
    static long lastWorking = 0;
    PROCESS_MEMORY_COUNTERS_EX memoryCounters;
    memoryCounters.cb = sizeof(memoryCounters);
    ::GetProcessMemoryInfo(::GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&memoryCounters, sizeof(memoryCounters));
    long privateMem = (long)(memoryCounters.PrivateUsage / 1024);
    long workingMem = (long)(memoryCounters.WorkingSetSize / 1024);
    if (!onchangeOnly || privateMem != lastPrivate)
    {
        logger_base.debug("Memory Usage: %s : private %ldKB (%ldKB) working %ldKB (%ldKB).",
            (const char*)reason.c_str(),
            privateMem,
            privateMem - lastPrivate,
            workingMem,
            workingMem - lastWorking
        );
    }
    lastPrivate = privateMem;
    lastWorking = workingMem;
#endif
}

bool IsxLights()
{
    // Allows functions common to multiple xLights programs to know if they are running in xLights itself
    return wxTheApp->GetAppName().Lower() == "xlights";
}

std::string ReverseCSV(const std::string& csv)
{
    std::string res;
    auto a = wxSplit(csv, ',');
    for (auto it = a.rbegin(); it != a.rend(); ++it) {
        if (res != "") res += ",";
        res += *it;
    }
    return res;
}

void DumpBinary(uint8_t* buffer, size_t sz)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    for (size_t i = 0; i < (sz + 15) / 16; i++) {
        std::string out;
        for (size_t j = i * 16; j < std::min(sz, (i + 1) * 16); j++) {
            out += wxString::Format("%02x ", buffer[j]);
        }
        out += "    ";
        for (size_t j = i * 16; j < std::min(sz, (i + 1) * 16); j++) {
            if (buffer[j] < 32 || buffer[j] > 126) {
                out += '.';
            }
            else {
                out += char(buffer[j]);
            }
        }
        logger_base.debug(out);
    }
}

wxColor CyanOrBlue()
{
    if (IsDarkMode()) {
        // In Dark Mode blue is hard to read
        return *wxCYAN;
    } else {
        return *wxBLUE;
    }
}
wxColor LightOrMediumGrey()
{
    if (IsDarkMode()) {
        static const wxColor medGray(128, 128, 128);
        return medGray;
    } else {
        return *wxLIGHT_GREY;
    }
}
void CleanupIpAddress(wxString& IpAddr)
{
    static wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
    if (leadingzero1.Matches(IpAddr))
    {
        wxString s0 = leadingzero1.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero1.GetMatch(IpAddr, 1);
        leadingzero1.ReplaceFirst(&IpAddr, "" + s0.Right(s0.size() - s1.size()));
    }
    static wxRegEx leadingzero2("(\\.0+)(?:[1-9]|0\\.|0$)", wxRE_ADVANCED);
    while (leadingzero2.Matches(IpAddr)) // need to do it several times because the results overlap
    {
        wxString s0 = leadingzero2.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero2.GetMatch(IpAddr, 1);
        leadingzero2.ReplaceFirst(&IpAddr, "." + s0.Right(s0.size() - s1.size()));
    }
}

wxString CompressNodes(const wxString& nodes)
{
    wxString res = "";
    if (nodes.IsEmpty())
        return res;
    // make sure it is fully expanded first
    auto s = ExpandNodes(nodes);
    int dir = 0;
    int start = -1;
    int last = -1;
    auto as = wxSplit(s, ',');

    // There is no difference between empty row and row with one blank pixel (shrug)
    // We will take removal of the last comma approach

    for (const auto& i : as)
    {
        if (i.empty() || i == "0") {
            // Flush out start/last if any
            if (start != -1) {
                if (last != start) {
                    res += wxString::Format("%d-%d,", start, last);
                } else {
                    res += wxString::Format("%d,", start);
                }
            }
            // Add empty and separator
            res += i+",";
            start = last = -1;
            dir = 0;
            continue;
        }
        if (start == -1)
        {
            start = wxAtoi(i);
            last = start;
            dir = 0;
        }
        else
        {
            int j = wxAtoi(i);
            if (dir == 0)
            {
                if (j == last + 1)
                {
                    dir = 1;
                }
                else if (j == last - 1)
                {
                    dir = -1;
                }
                else
                {
                    res += wxString::Format("%d,", start);
                    start = j;
                    dir = 0;
                }
                last = j;
            }
            else
            {
                if (j == last + dir)
                {
                }
                else
                {
                    res += wxString::Format("%d-%d,", start, last);
                    start = j;
                    dir = 0;
                }
                last = j;
            }
        }
    }

    if (start == -1)
    {
        // nothing to do
    }
    else if (start == last)
    {
        res += wxString::Format("%d,", start);
    }
    else
    {
        res += wxString::Format("%d-%d,", start, last);
    }

    if (!res.empty())
        res = res.substr(0, res.length() - 1); // Chop last comma

    return res;
}

wxString ExpandNodes(const wxString& nodes)
{
    wxString res = "";

    auto as = wxSplit(nodes, ',');

    bool first = true;
    for (const auto& i : as)
    {
        if (i.Contains("-"))
        {
            auto as2 = wxSplit(i, '-');
            if (as2.size() == 2)
            {
                int start = wxAtoi(as2[0]);
                int end = wxAtoi(as2[1]);
                if (start < end)
                {
                    for (int j = start; j <= end; j++)
                    {
                        if (!first || res != "") res += ",";
                        res += wxString::Format("%d", j);
                    }
                }
                else if (start == end)
                {
                    if (!first || res != "") res += ",";
                    res += wxString::Format("%d", start);
                }
                else
                {
                    for (int j = start; j >= end; j--)
                    {
                        if (!first || res != "") res += ",";
                        res += wxString::Format("%d", j);
                    }
                }
            }
        }
        else
        {
            if (!first || res != "") res += ",";
            res += i;
        }
        first = false;
    }
    return res;
}

void ShiftNodes(std::map<std::string, std::string> & nodes, int shift, int min, int max)
{
    for (auto& line : nodes) {
        if(line.second.empty())
            continue;
        if(Contains(line.first, "Color"))
            continue;
        if(Contains(line.first, "Name"))
            continue;
        if(Contains(line.first, "Type"))
            continue;
        auto const oldnodes = ExpandNodes(line.second);
        auto const oldNodeArray = wxSplit(oldnodes, ',');
        wxArrayString newNodeArray;
        for (auto const& node: oldNodeArray) {
            long val;
            if (node.ToCLong(&val) == true) {
                long newVal = val + shift;
                if (newVal > max) {
                    newVal -= max;
                }
                else if (newVal < min) {
                    newVal += max;
                }
                newNodeArray.Add( wxString::Format("%ld", newVal) );
            }
        }
        if(newNodeArray.size() > 0)
            line.second = CompressNodes(wxJoin(newNodeArray, ','));
    }
}

void ReverseNodes(std::map<std::string, std::string> & nodes, int max)
{
    for (auto& line : nodes) {
        if(line.second.empty())
            continue;
        if(Contains(line.first, "Color"))
            continue;
        if(Contains(line.first, "Name"))
            continue;
        if(Contains(line.first, "Type"))
            continue;
        auto const oldnodes = ExpandNodes(line.second);
        auto const oldNodeArray = wxSplit(oldnodes, ',');
        wxArrayString newNodeArray;
        for (auto const& node: oldNodeArray) {
            long val;
            if (node.ToCLong(&val) == true) {
                long newVal = max - val;
                newNodeArray.Add( wxString::Format("%ld", newVal) );
            }
        }
        if(newNodeArray.size() > 0)
            line.second = CompressNodes(wxJoin(newNodeArray, ','));
    }
}

// returns true if the string contains what looks like a floating point number
bool IsFloat(const std::string& number)
{
    // it cant be blank
    if (number == "")
        return false;
    // if it contains a - it must be the first character and there must only be one of them
    if (CountChar(number, '-') > 1 || (Contains(number, "-") && number[0] != '-'))
        return false;
    // it must contain zero or 1 '.'
    if (CountChar(number, '.') > 1)
        return false;
    // all other characters must be 0-9
    for (const auto it : number)
    {
        if (it != '.' && it != '-' && (it < '0' || it > '9'))
            return false;
    }
    return true;
}

#ifdef __WXMSW__
bool IsSuppressDarkMode()
{
    wxConfigBase* config = wxConfigBase::Get();
    return config->ReadBool("SuppressDarkMode", false);
}

void SetSuppressDarkMode(bool suppress)
{
    if (IsSuppressDarkMode() != suppress) {
        wxConfigBase* config = wxConfigBase::Get();
        config->Write("SuppressDarkMode", suppress);
        wxMessageBox("Restart " + wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetName() + " to enable/disable dark mode properly.");
    }
}
#endif

bool IsDarkMode()
{
    return wxSystemSettings::GetAppearance().IsDark() 
#ifdef __WXMSW__
        && !IsSuppressDarkMode()
#endif
        ;
}
