/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "wxUtilities.h"

#include <wx/button.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/file.h>
#include <wx/display.h>
#include <wx/filename.h>
#include <wx/gifdecod.h>
#include <wx/mimetype.h>
#include <wx/protocol/http.h>
#include <wx/regex.h>
#include <wx/sckaddr.h>
#include <wx/socket.h>
#include <wx/sstream.h>
#include <wx/stdpaths.h>
#include <wx/wfstream.h>

#include "wxUtilities.h"
#include "../ExternalHooks.h"
#include "../xLightsVersion.h"
#include "../utils/CurlManager.h"
#include "../utils/string_utils.h"

#include <log.h>

#include <mutex>

wxDEFINE_EVENT(EVT_SETTIMINGTRACKS, wxCommandEvent);

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
    auto resultString = CurlManager::HTTPSPost(url, message, "", "", "application/json", 30 * 60, {}, &responseCode);
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
    result = CurlManager::HTTPSGet(url, "", "", 30 * 60, {}, &responseCode);
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

AnimatedImageData LoadGIFAnimationDataWx(const std::string& filename)
{
    AnimatedImageData result;
    wxLogNull logNo;

    wxFileInputStream stream(filename);
    if (!stream.IsOk()) {
        spdlog::warn("LoadGIFAnimationDataWx: cannot open {}", filename);
        return result;
    }

    wxGIFDecoder decoder;
    if (decoder.LoadGIF(stream) != wxGIF_OK) {
        spdlog::warn("LoadGIFAnimationDataWx: failed to decode {}", filename);
        return result;
    }

    int frameCount = (int)decoder.GetFrameCount();
    if (frameCount == 0) return result;

    // Background color
    wxColour bgColor = decoder.GetBackgroundColour();
    result.backgroundColor = bgColor.IsOk() ? wxColourToXlColor(bgColor) : xlBLACK;

    // Logical GIF canvas size
    wxSize gifSize(0, 0);
    for (int i = 0; i < frameCount; i++) {
        wxSize  fs = decoder.GetFrameSize(i);
        wxPoint fo = decoder.GetFramePosition(i);
        gifSize.SetWidth( std::max(gifSize.GetWidth(),  fo.x + fs.GetWidth()));
        gifSize.SetHeight(std::max(gifSize.GetHeight(), fo.y + fs.GetHeight()));
    }
    result.width  = gifSize.GetWidth();
    result.height = gifSize.GetHeight();

    // Frame delays
    long totalTime = 0;
    for (int i = 0; i < frameCount; i++) {
        long delay = decoder.GetDelay(i);
        result.frameTimes.push_back(delay);
        totalTime += delay;
    }
    if (totalTime == 0) {
        result.frameTimes.assign(frameCount, 100);
    }

    // Helper: copy a sub-image onto canvas at offset, skipping transparent pixels
    auto blitFrame = [](wxImage& canvas, const wxImage& src, wxPoint offset, bool skipTransparent) {
        int tox = std::min(src.GetWidth(),  canvas.GetWidth()  - offset.x);
        int toy = std::min(src.GetHeight(), canvas.GetHeight() - offset.y);
        for (int y = 0; y < toy; y++) {
            for (int x = 0; x < tox; x++) {
                if (skipTransparent && src.IsTransparent(x, y)) continue;
                canvas.SetRGB(x + offset.x, y + offset.y,
                              src.GetRed(x, y), src.GetGreen(x, y), src.GetBlue(x, y));
                canvas.SetAlpha(x + offset.x, y + offset.y,
                                src.IsTransparent(x, y) ? 0 : 255);
            }
        }
    };

    auto fillSolid = [](wxImage& img, unsigned char r, unsigned char g, unsigned char b) {
        for (int y = 0; y < img.GetHeight(); y++) {
            for (int x = 0; x < img.GetWidth(); x++) {
                img.SetRGB(x, y, r, g, b);
                img.SetAlpha(x, y, 255);
            }
        }
    };

    auto clearTransparent = [](wxImage& img) {
        img.UnShare();
        img.Clear();
        for (int y = 0; y < img.GetHeight(); y++)
            for (int x = 0; x < img.GetWidth(); x++)
                img.SetAlpha(x, y, 0);
    };

    // Build two composite streams: one with background filled, one transparent
    wxImage canvasBG(gifSize);
    canvasBG.InitAlpha();
    wxImage canvasNoBG(gifSize);
    canvasNoBG.InitAlpha();

    wxAnimationDisposal lastDispose = wxANIM_TOBACKGROUND;

    for (int i = 0; i < frameCount; i++) {
        wxSize  frameSize = decoder.GetFrameSize(i);
        wxPoint offset    = decoder.GetFramePosition(i);
        wxAnimationDisposal dispose = decoder.GetDisposalMethod(i);

        wxImage newframe(frameSize);
        decoder.ConvertToImage(i, &newframe);

        if (i == 0 || lastDispose == wxANIM_TOBACKGROUND) {
            fillSolid(canvasBG,
                      result.backgroundColor.red,
                      result.backgroundColor.green,
                      result.backgroundColor.blue);
            clearTransparent(canvasNoBG);
        }

        blitFrame(canvasBG,   newframe, offset, true);
        blitFrame(canvasNoBG, newframe, offset, true);

        result.frames.push_back(wxImageToXlImage(canvasBG));
        result.framesNoBG.push_back(wxImageToXlImage(canvasNoBG));

        lastDispose = dispose;
    }

    return result;
}
