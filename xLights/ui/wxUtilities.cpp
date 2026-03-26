/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <cstring>

#include <wx/button.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/display.h>
#include <wx/filename.h>
#include <wx/mimetype.h>
#include <wx/protocol/http.h>
#include <wx/regex.h>
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>

#include <fstream>

#include "wxUtilities.h"
#include "../ExternalHooks.h"
#include "../xLightsVersion.h"
#include "../utils/Curl.h"
#include "../utils/string_utils.h"

#include <log.h>

#include <mutex>

const wxString xlEMPTY_WXSTRING{ "" };

void DisplayError(const std::string& err, wxWindow* win) {
    spdlog::error("DisplayError: {}", err);
    wxMessageBox(err, "Error", wxICON_ERROR | wxOK, win);
}

void DisplayWarning(const std::string& warn, wxWindow* win) {
    spdlog::warn("DisplayWarning: {}", warn);
    wxMessageBox(warn, "Warning", wxICON_WARNING | wxOK, win);
}

void DisplayInfo(const std::string& info, wxWindow* win) {
    spdlog::info("DisplayInfo: {}", info);
    wxMessageBox(info, "Information", wxICON_INFORMATION | wxOK, win);
}

void DisplayCrit(const std::string& crit, wxWindow* win) {
    spdlog::critical("DisplayCrit: {}", crit);
    wxMessageBox(crit, "CRITICAL", wxICON_ERROR | wxOK, win);
}

std::string DecodeIPError(wxSocketError err) {
    switch (err) {
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

wxArrayString Split(const wxString& s, const std::vector<char>& delimiters) {
    wxArrayString res;

    wxString w;
    for (const auto it : s) {
        bool delim = false;
        for (auto it2 : delimiters) {
            if (it == it2) {
                delim = true;
                break;
            }
        }
        if (delim) {
            res.Add(w);
            w = "";
        } else {
            w += it;
        }
    }
    res.Add(w);

    return res;
}

static std::list<std::string> SearchDirectories;
void SetFixFileDirectories(const std::list<std::string>& dirs) {
    SearchDirectories = dirs;
}

static bool doesFileExist(const wxString& dir, const wxString& origFileWin, const wxString& origFileUnix, wxString& path) {

    if (origFileWin != "") {
        wxFileName fn3(dir, origFileWin);
        if (FileExists(fn3, false)) {
            spdlog::debug("File location fixed: {} -> {}", origFileWin.ToStdString(), fn3.GetFullPath().ToStdString());
            path = fn3.GetFullPath();
            return true;
        }
    }
    if (origFileUnix != "") {
        wxFileName fn4(dir, origFileUnix);
        if (FileExists(fn4, false)) {
            spdlog::debug("File location fixed: {} -> {}", origFileWin.ToStdString(), fn4.GetFullPath().ToStdString());
            path = fn4.GetFullPath();
            return true;
        }
    }
    return false;
}
static bool doesFileExist(const wxString& sd, const wxString& append,
                          const wxString& nameWin, const wxString& nameUnix, wxString& newPath) {
    if (doesFileExist(sd + append, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        return true;
    }
    for (auto& fd : SearchDirectories) {
        if (doesFileExist(fd + append, nameWin, nameUnix, newPath)) {
            // file exists in one of the resource directories
            return true;
        }
    }
    return false;
}
static bool doesFileExist(const wxString& sd, const wxString& appendWin, const wxString& appendUnx,
                          const wxString& nameWin, const wxString& nameUnix, wxString& newPath) {
    if (doesFileExist(sd + appendWin, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        return true;
    }
    if (doesFileExist(sd + appendUnx, nameWin, nameUnix, newPath)) {
        // file exists in the new show dir
        return true;
    }
    for (auto& fd : SearchDirectories) {
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

wxString MakeRelativeFile(const wxString& file)
{
    if (file.IsEmpty()) return {};
    wxFileName fn(file);
    if (!fn.IsAbsolute()) return {};  // already relative

    wxString f(file);
    f.Replace("\\", "/");

    // Helper: strip base prefix from f and return the relative portion, or ""
    auto stripPrefix = [&](wxString base) -> wxString {
        base.Replace("\\", "/");
#ifdef __WXMSW__
        base.MakeLower();
        wxString fl = f.Lower();
#else
        const wxString& fl = f;
#endif
        if (!base.EndsWith("/")) base += "/";
        if (fl.StartsWith(base))
            return f.Mid(base.Length());
        return {};
    };

    wxString rel = stripPrefix(RememberShowDir);
    if (!rel.IsEmpty()) return rel;

    for (const auto& dir : SearchDirectories) {
        rel = stripPrefix(wxString(dir));
        if (!rel.IsEmpty()) return rel;
    }

    return {};
}

static std::recursive_mutex __fixFilesMutex;
static std::vector<std::string> __nonExistentFiles;

void ClearNonExistentFiles() {
    std::unique_lock<std::recursive_mutex> lock(__fixFilesMutex);
    __nonExistentFiles.clear();
}

wxImage ApplyOrientation(const wxImage& img, int orient) {
    wxImage res = img.Copy();
    switch (orient) {
    case 2: return res.Mirror(true);  // horizontal flip
    case 3: return res.Rotate180();
    case 4: return res.Mirror(false); // vertical flip
    case 5: return res.Mirror(true).Rotate90(false); // horizontal flip + 90 CCW
    case 6: return res.Rotate90(true);  // 90 CW
    case 7: return res.Mirror(true).Rotate90(true);  // horizontal flip + 90 CW
    case 8: return res.Rotate90(false); // 90 CCW
    default: return res;
    }
}

int GetExifOrientation(const std::string& filename) {

    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        spdlog::debug("Failed to open file: {}", filename);
        file.close();
        return 1; // Default orientation
    }

    unsigned char byte1, byte2;
    file.read(reinterpret_cast<char*>(&byte1), 1);
    file.read(reinterpret_cast<char*>(&byte2), 1);
    if (byte1 != 0xFF || byte2 != 0xD8) {
        file.close();
        return 1;
    }

    while (file) {
        file.read(reinterpret_cast<char*>(&byte1), 1);
        if (byte1 != 0xFF) break;
        file.read(reinterpret_cast<char*>(&byte2), 1);
        if (byte2 == 0xD9 || byte2 == 0xDA) break;

        unsigned short len;
        file.read(reinterpret_cast<char*>(&len), 2);
        len = ((len >> 8) & 0xFF) | ((len << 8) & 0xFF00); // big-endian
        if (len < 2) break;

        if (byte2 == 0xE1) { // APP1 segment
            std::vector<char> data(len - 2);
            file.read(data.data(), len - 2);

            if (data.size() < 14) continue; // too small to hold Exif header

            if (memcmp(data.data(), "Exif\0\0", 6) != 0) {
                continue;
            }

            size_t tiff_header = 6; // TIFF header starts right after Exif\0\0
            bool littleEndian = (data[tiff_header] == 'I' && data[tiff_header + 1] == 'I');
            unsigned short fortytwo = littleEndian ?
                ((unsigned char)data[tiff_header + 3] << 8) | (unsigned char)data[tiff_header + 2] :
                ((unsigned char)data[tiff_header + 2] << 8) | (unsigned char)data[tiff_header + 3];

            if (fortytwo != 42) {
                spdlog::debug("Invalid TIFF header identifier in {}", filename);
                return 1;
            }

            // Read offset to IFD0
            unsigned int ifd_offset;
            if (littleEndian) {
                ifd_offset =  (unsigned char)data[tiff_header + 4]       |
                    ((unsigned char)data[tiff_header + 5] << 8) |
                    ((unsigned char)data[tiff_header + 6] << 16)|
                    ((unsigned char)data[tiff_header + 7] << 24);
            } else {
                ifd_offset = ((unsigned char)data[tiff_header + 4] << 24)|
                    ((unsigned char)data[tiff_header + 5] << 16)|
                    ((unsigned char)data[tiff_header + 6] << 8) |
                    (unsigned char)data[tiff_header + 7];
            }

            size_t pos = tiff_header + ifd_offset;
            if (pos + 2 > data.size()) return 1;

            unsigned short num_entries = littleEndian ?
                ((unsigned char)data[pos + 1] << 8) | (unsigned char)data[pos] :
                ((unsigned char)data[pos] << 8) | (unsigned char)data[pos + 1];
            pos += 2;

            for (unsigned short i = 0; i < num_entries; ++i) {
                if (pos + 12 > data.size()) break;

                unsigned short tag = littleEndian ?
                    ((unsigned char)data[pos + 1] << 8) | (unsigned char)data[pos] :
                    ((unsigned char)data[pos] << 8) | (unsigned char)data[pos + 1];

                if (tag == 0x0112) { // Orientation
                    unsigned short orient = littleEndian ?
                        ((unsigned char)data[pos + 9] << 8) | (unsigned char)data[pos + 8] :
                        ((unsigned char)data[pos + 8] << 8) | (unsigned char)data[pos + 9];
                    return static_cast<int>(orient);
                }
                pos += 12;
            }
        } else {
            file.seekg(len - 2, std::ios::cur);
        }
    }

    // Fallback: wxImage may know the orientation
    wxLogNull logNo;
    wxImage img;
    if (img.LoadFile(filename, wxBITMAP_TYPE_JPEG)) {
        if (img.HasOption("Orientation")) {
            int orient = img.GetOptionInt("Orientation");
            return orient;
        }
    }
    return 1; // default
}

std::string GetResourcesDirectory() {
    std::string dir = GetResourcesDir();
    if (dir.empty()) {
#ifndef __WXMSW__
        dir = wxStandardPaths::Get().GetResourcesDir().ToStdString();
#else
        auto exec = wxStandardPaths::Get().GetExecutablePath().ToStdString();
        dir = exec.substr(0, exec.find_last_of("/\\"));
#endif
        SetResourcesDir(dir);
    }
    return dir;
}


wxString FixFile(const wxString& ShowDir, const wxString& file) {


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

    if (file.find("/meshobjects/") != std::string::npos) {
#ifndef __WXMSW__
            return wxStandardPaths::Get().GetResourcesDir() + file.substr(file.find("/meshobjects/"));
#else
            wxStandardPaths stdp = wxStandardPaths::Get();
            return wxFileName(stdp.GetExecutablePath()).GetPath() + file.substr(file.find("/meshobjects/"));
#endif
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
    spdlog::debug("File not found ... attempting to fix location ({}) : {}", sd.ToStdString(), file.ToStdString());

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
    for (auto& fd : SearchDirectories) {
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
        bool cont = dir.GetFirst(&foldername, "*", wxDIR_DIRS /* | wxDIR_NO_FOLLOW*/);
        while (cont) {
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
        wxFileName sdFn = wxFileName::DirName(sd);
        if (FileExists(relative, false)) {
            lock.lock();
            __fileMap[file] = relative;
            return relative;
        }
    }
    // if we get here, the show folders could not be matched, let's try the reverse and traverse up the
    // the files directories to see if we can get a match
    appendWin = "";
    for (int x = fnWin.GetDirs().size() - 1; x >= 0; x--) {
        appendWin = wxFileName::GetPathSeparator() + fnWin.GetDirs()[x] + appendWin;
        if (doesFileExist(sd, appendWin, nameWin, nameUnix, newPath)) {
            // file exists
            lock.lock();
            __fileMap[file] = newPath;
            return newPath;
        }
    }
    appendUnx = "";
    for (int x = fnUnix.GetDirs().size() - 1; x >= 0; x--) {
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
    spdlog::debug("   could not find a fixed file location for : " + file.ToStdString());
    spdlog::debug("   We will not look for this file again until a new sequence is loaded.");
    lock.lock();
    __nonExistentFiles.push_back(file.ToStdString());
    return file;
}
bool IsFileInShowDir(const wxString& showDir, const std::string filename) {
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
    for (auto& d : SearchDirectories) {
#ifdef __WXMSW__
        transform(d.begin(), d.end(), d.begin(), ::tolower);
#endif
        if (fixedFile.StartsWith(d)) {
            return true;
        }
    }
    return false;
}
wxString FixEffectFileParameter(const wxString& paramname, const wxString& parametervalue, const wxString& ShowDir) {
    int startparamname = parametervalue.Find(paramname);
    int endparamname = parametervalue.find("=", startparamname) - 1;
    int startvalue = endparamname + 2;
    int endvalue = parametervalue.find(",", startvalue) - 1;
    wxString file = parametervalue.SubString(startvalue, endvalue);
    wxString newfile = FixFile(ShowDir, file);
    wxString rc = parametervalue.Left(startvalue) + newfile + parametervalue.Right(parametervalue.Length() - endvalue - 1);
    return rc;
}

void DownloadVamp() {
    wxMessageBox("We are about to download the Queen Mary Vamp plugins for your platform. Once downloaded please install them and then close and reopen xLights to use them.");
#ifdef __WXMSW__
    //::wxLaunchDefaultBrowser("https://code.soundsoftware.ac.uk/attachments/download/2623/qm-vamp-plugins-1.8.0-win64.msi");
    ::wxLaunchDefaultBrowser("https://github.com/vamp-plugins/vamp-plugin-pack/releases/download/v2.0/Vamp.Plugin.Pack.Installer.2.0.exe");
#else
    // likely can/should be used for all platforms
    ::wxLaunchDefaultBrowser("https://www.vamp-plugins.org/pack.html");
#endif
}

inline bool is_base64(unsigned char c) {
    return (isalnum(c) || (c == '+') || (c == '/'));
}

// returns number of chars at the end that couldn't be decoded
int base64_decode(const wxString& encoded_string, std::vector<unsigned char>& data) {
    static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t in_len = encoded_string.size();
    int i = 0;
    int in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_];
        in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) {
                char_array_4[i] = base64_chars.find(char_array_4[i]);
            }

            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

            for (i = 0; (i < 3); i++) {
                data.resize(data.size() + 1);
                data[data.size() - 1] = char_array_3[i];
            }
            i = 0;
        }
    }

    if (i && encoded_string[in_] == '=') {
        for (int j = i; j < 4; j++) {
            char_array_4[j] = 0;
        }

        for (int j = 0; j < 4; j++) {
            char_array_4[j] = base64_chars.find(char_array_4[j]);
        }

        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

        for (int j = 0; (j < i - 1); j++) {
            data.resize(data.size() + 1);
            data[data.size() - 1] = char_array_3[j];
        }
    }
    return i;
}

void SaveInt(const std::string& tag, int value) {
    wxConfigBase* config = wxConfigBase::Get();
    config->Write(tag, value);
}

int LoadInt(const std::string& tag, int defaultValue) {
    wxConfigBase* config = wxConfigBase::Get();
    return config->ReadLong(tag, defaultValue);
}

void SaveWindowPosition(const std::string& tag, wxWindow* window) {
    wxConfigBase* config = wxConfigBase::Get();
    if (window != nullptr) {
        wxPoint position = window->GetPosition();
        wxSize size = window->GetSize();
        config->Write(tag, wxString::Format("%d|%d|%d|%d", position.x, position.y, size.GetWidth(), size.GetHeight()));
    } else {
        config->DeleteEntry(tag);
    }
}

void LoadWindowPosition(const std::string& tag, wxSize& size, wxPoint& position) {
    if (wxGetKeyState(WXK_COMMAND) || wxGetKeyState(WXK_CONTROL)) {
        return;
    }
    wxConfigBase* config = wxConfigBase::Get();

    wxString pos = config->Read(tag, "");

    position.x = -1;
    position.y = -1;
    size.SetWidth(-1);
    size.SetHeight(-1);

    wxArrayString ss = wxSplit(pos, '|');

    if (ss.size() == 4) {
        position.x = wxAtoi(ss[0]);
        position.y = wxAtoi(ss[1]);
        size.SetWidth(wxAtoi(ss[2]));
        size.SetHeight(wxAtoi(ss[3]));
    }
}

void EnsureWindowHeaderIsOnScreen(wxWindow* win) {
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

void OptimiseDialogPosition(wxDialog* dlg) {
    wxPoint pos = wxGetMousePosition();
    wxSize sz = dlg->GetSize();
    pos.x -= sz.GetWidth() / 2;
    pos.y -= sz.GetHeight() / 2;

    // ensure it is on a single screen
    int d = wxDisplay::GetFromPoint(wxGetMousePosition());
    if (d < 0)
        d = 0;
    wxDisplay display(d);
    if (display.IsOk()) {
        wxRect displayRect = display.GetClientArea();
        if (pos.y < displayRect.GetTop())
            pos.y = displayRect.GetTop();
        if (pos.y + sz.GetHeight() > displayRect.GetBottom())
            pos.y = displayRect.GetBottom() - sz.GetHeight();
        if (pos.x < displayRect.GetLeft())
            pos.x = displayRect.GetLeft();
        if (pos.x + sz.GetWidth() > displayRect.GetRight())
            pos.x = displayRect.GetRight() - sz.GetWidth();
    }

    dlg->SetPosition(pos);
    EnsureWindowHeaderIsOnScreen(dlg);
}

nlohmann::json xLightsRequest(int xFadePort, const wxString& message, const std::string& ipAddress) {
    std::string url = "http://" + ipAddress + ":" + std::to_string(GetxFadePort(xFadePort)) + "/xlDoAutomation";
    int responseCode = 0;
    auto resultString = Curl::HTTPSPost(url, message, "", "", "application/json", 30 * 60, {}, &responseCode);
    if (resultString != "" && (responseCode == 200 || responseCode >= 500)) {
        nlohmann::json result = nlohmann::json::parse(resultString);
        result["res"] = (int)responseCode;
        return result;
    }

    std::string const msg = "{\"res\":504,\"msg\":\"Unable to connect.\"}";
    nlohmann::json result = nlohmann::json::parse(msg);
    return result;
}
bool xLightsRequest(std::string& result, int xFadePort, const wxString& request, const std::string& ipAddress) {
    std::string url = "http://" + ipAddress + ":" + std::to_string(GetxFadePort(xFadePort)) + "/" + request;
    int responseCode = 0;
    result = Curl::HTTPSGet(url, "", "", 30 * 60, {}, &responseCode);
    return responseCode == 200;
}

void ViewTempFile(const wxString& content, const wxString& name, const wxString& type) {
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
            } else {
                wxUnsetEnv("LD_PRELOAD");
                wxExecute(command);
            }
            delete ft;
        }
    }
}

bool IsValidLocalIP(const wxIPV4address& ip) {
    return IsValidLocalIP(ip.IPAddress().ToStdString());
}

wxColor CyanOrBlue() {
    if (IsDarkMode()) {
        // In Dark Mode blue is hard to read, use cyan for some things
        return *wxCYAN;
    } else {
        return *wxBLUE;
    }
}
wxColor BlueOrLightBlue() {
    if (IsDarkMode()) {
        // In Dark Mode blue is hard to read, use a lighter blue if
        // what is displayed really needs to be blue
        // Use Royal Blue RGB
        static const wxColor lightBlue(0x41, 0x69, 0xE1);
        return lightBlue;
    } else {
        return *wxBLUE;
    }
}
wxColor LightOrMediumGrey() {
    if (IsDarkMode()) {
        static const wxColor medGray(128, 128, 128);
        return medGray;
    } else {
        return *wxLIGHT_GREY;
    }
}
wxColor RedOrLightRed() {
    if (IsDarkMode()) {
        // In Dark Mode pure red is hard to read on grey, use a lighter salmon/coral red
        static const wxColor lightRed(0xFF, 0x6B, 0x6B);
        return lightRed;
    } else {
        return *wxRED;
    }
}
void CleanupIpAddress(wxString& IpAddr) {
    static wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
    if (leadingzero1.Matches(IpAddr)) {
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

#ifdef __WXMSW__
bool IsSuppressDarkMode() {
    wxConfigBase* config = wxConfigBase::Get();
    return config->ReadBool("SuppressDarkMode", false);
}

void SetSuppressDarkMode(bool suppress) {
    if (IsSuppressDarkMode() != suppress) {
        wxConfigBase* config = wxConfigBase::Get();
        config->Write("SuppressDarkMode", suppress);
        wxMessageBox("Restart " + wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetName() + " to enable/disable dark mode properly.");
    }
}
#endif

bool IsDarkMode() {
    return wxSystemSettings::GetAppearance().IsDark()
#ifdef __WXMSW__
           && !IsSuppressDarkMode()
#endif
        ;
}

// ExternalHooks wx-dependent fallback implementations (non-macOS)
#ifndef __APPLE__
bool FileExists(const wxString &s, bool waitForDownload) {
    return wxFile::Exists(s);
}
bool FileExists(const wxFileName &fn, bool waitForDownload) {
    return fn.FileExists();
}
void GetAllFilesInDir(const wxString& dir, wxArrayString& files, const wxString& filespec, int flags) {
    wxDir::GetAllFiles(dir, &files, filespec, flags);
}
void SetButtonBackground(wxButton* b, const wxColour& c, int bgType) {
    b->SetBackgroundColour(c);
    b->Refresh();
}
void AdjustColorToDeviceColorspace(const wxColor& c, uint8_t &r, uint8_t &g, uint8_t &b, uint8_t &a) {
    r = c.Red(); g = c.Green(); b = c.Blue(); a = c.Alpha();
}
bool DoInAppPurchases(wxWindow*) { return false; }
wxString GetOSFormattedClipboardData() { return ""; }
#endif
