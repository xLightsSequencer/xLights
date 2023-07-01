/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TipOfTheDayDialog.h"

//(*InternalHeaders(TipOfTheDayDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/stdpaths.h>
#include <wx/config.h>

#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "utils/Curl.h"
#include "CachedFileDownloader.h"

#include <log4cpp/Category.hh>

#ifdef USE_WEBVIEW_FOR_TOD
#include <wx/webview.h>
static const std::string TOD_BASE_URL = "https://raw.githack.com/smeighan/xLights/master/TipOfDay/";
#else
static const std::string TOD_BASE_URL = "https://raw.githubusercontent.com/smeighan/xLights/master/TipOfDay/";
#endif

#define USE_GITHUB_HOSTED_TOD

//(*IdInit(TipOfTheDayDialog)
const long TipOfTheDayDialog::ID_HTMLWINDOW1 = wxNewId();
const long TipOfTheDayDialog::ID_ShowTips_CHECKBOX = wxNewId();
const long TipOfTheDayDialog::ID_BUTTON1 = wxNewId();
const long TipOfTheDayDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TipOfTheDayDialog,wxDialog)
	//(*EventTable(TipOfTheDayDialog)
	//*)
END_EVENT_TABLE()

class TODTracker
{
    wxConfig *config = new wxConfig("xLights-TOD");
public:
    TODTracker()
    {
    }

    void ClearVisited()
    {
        config->DeleteAll();
        config->Flush();
    }

    int GetVisited(const std::string& tod)
    {
        return config->ReadLong(tod, 0);
    }

    void AddVisited(const std::string& tod)
    {
        int i = config->ReadLong(tod, 0);
        config->Write(tod, ++i);
        config->Flush();
    }
};

class TipOfDayThread : public wxThread
{
    wxWindow* _notify = nullptr;
    std::string _downloadTo;

public:
    TipOfDayThread(wxWindow* notify) :
        _notify(notify)
    {}

    virtual void* Entry() override
    {
        // download Tip of day content here
        CachedFileDownloader cache;
        auto file = cache.GetFile(wxURI(TOD_BASE_URL + "tod.xml"), CACHEFOR::CACHETIME_DAY);

        wxCommandEvent e(EVT_TIPOFDAY_READY);
        e.SetString(file);
        wxPostEvent(_notify, e);

        return nullptr;
    }
};

class xlCachedHtmlWindow : public wxHtmlWindow {
public:
    xlCachedHtmlWindow(wxWindow *parent, wxWindowID id=wxID_ANY, const wxPoint &pos=wxDefaultPosition, const wxSize &size=wxDefaultSize, long style=wxHW_DEFAULT_STYLE, const wxString &name="htmlWindow") : wxHtmlWindow(parent, id, pos, size, style, name) {
    }
    virtual ~xlCachedHtmlWindow() {
    }

    void Reset() {
        baseURL = "";
        baseFileLocation = "";
    }
    virtual bool LoadPage(const wxString& location) override {
        Reset();
        return wxHtmlWindow::LoadPage(location);
    }
    virtual wxHtmlOpeningStatus OnHTMLOpeningURL(wxHtmlURLType type, const wxString &u, wxString *redirect) const override {
        wxString url = u;

        if (baseURL == "") {
            baseURL = url.substr(0, url.find_last_of("/"));
        } else if (baseFileLocation != "") {
            url = baseURL + url.substr(baseFileLocation.size());
        }
        wxURI uri(url);
        auto file = cache.GetFile(uri, CACHEFOR::CACHETIME_LONG);
        if (file != "") {
            *redirect = file;
            if (baseFileLocation == "") {
                baseFileLocation = file.substr(0, file.find_last_of(wxFileName::GetPathSeparator()));
            }
            return wxHTML_REDIRECT;
        }
        return wxHtmlWindow::OnHTMLOpeningURL(type, url, redirect);
    }
    mutable CachedFileDownloader cache;
    mutable std::string baseURL;
    mutable std::string baseFileLocation;
};

TipOfTheDayDialog::TipOfTheDayDialog(const std::string& url, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    //(*Initialize(TipOfTheDayDialog)
    wxFlexGridSizer* FlexGridSizer2;

    Create(parent, id, _("Tip of the day"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
    SetClientSize(wxSize(1000,800));
    Move(wxDefaultPosition);
    SetMinSize(wxSize(1000,800));
    FlexGridSizer1 = new wxFlexGridSizer(2, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    HtmlWindow1 = new xlCachedHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    HtmlWindow1->SetPage(_("<html><body><p>Loading...</p></body></html>"));
    FlexGridSizer1->Add(HtmlWindow1, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer2 = new wxFlexGridSizer(1, 3, 0, 0);
    FlexGridSizer2->AddGrowableCol(0);
    ShowTipsCheckbox = new wxCheckBox(this, ID_ShowTips_CHECKBOX, _("Show Tips on Startup"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_ShowTips_CHECKBOX"));
    ShowTipsCheckbox->SetValue(true);
    FlexGridSizer2->Add(ShowTipsCheckbox, 1, wxALL|wxEXPAND, 5);
    Button_Next = new wxButton(this, ID_BUTTON1, _("Give me another one"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer2->Add(Button_Next, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    CloseButton = new wxButton(this, ID_BUTTON2, _("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
    CloseButton->SetDefault();
    FlexGridSizer2->Add(CloseButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    Layout();

    Connect(ID_ShowTips_CHECKBOX,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&TipOfTheDayDialog::OnShowTipsCheckboxClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TipOfTheDayDialog::OnButton_NextClick);
    Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TipOfTheDayDialog::OnCloseButtonClick);
    //*)

#ifdef USE_WEBVIEW_FOR_TOD
    webView = wxWebView::New(this, wxID_ANY);
    FlexGridSizer1->Replace(HtmlWindow1, webView);
#endif
    
    SetSize(1200, 800);
    Layout();
    
    wxPoint loc;
    wxSize sz;
    LoadWindowPosition("xLightsTipOfTheDay", sz, loc);
    if (loc.x != -1) {
        if (sz.GetWidth() < 400)
            sz.SetWidth(400);
        if (sz.GetHeight() < 300)
            sz.SetHeight(300);
        SetPosition(loc);
        SetSize(sz);
        Layout();
    }
    EnsureWindowHeaderIsOnScreen(this);

    LoadURL(url);
}

void TipOfTheDayDialog::LoadURL(const std::string& url)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    if (url != "") {
        logger_base.warn("Tip Of Day: %s", (const char*)url.c_str());
        if (StartsWith(url, "file://")) {
            auto file = url.substr(7);
            if (!wxFile::Exists(file)) {
                logger_base.warn("Tip Of Day unable to load file: %s", (const char*)file.c_str());
                logger_base.info("Current directory: %s", (const char*)wxGetCwd().c_str());
            } else {
#ifdef USE_WEBVIEW_FOR_TOD
                webView->LoadURL(url);
#else
                HtmlWindow1->Reset();
                HtmlWindow1->LoadFile(wxFileName(file));
#endif
            }
        } else {
#ifdef USE_WEBVIEW_FOR_TOD
            webView->LoadURL(url);
#else
            HtmlWindow1->LoadPage(url);
#endif
        }
    }
    Layout();
}

std::string TipOfTheDayDialog::BuildURL(const std::string& url) const
{
#ifdef USE_GITHUB_HOSTED_TOD
    return TOD_BASE_URL + url;
#else
    // exe location/TipOfDay/tod.xml
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    return f.GetPath() + "/TipOfDay/" + url;
#endif
}

// levels are beginner, intermediate, advanced, expert
bool TipOfTheDayDialog::IsLevelGreaterOrEqualTo(const std::string& act, const std::string& min)
{
    if (act == min)
        return true;

    if (min == "Beginner")
        return true;

    if (min == "Intermediate") {
        return act == "Advanced" || act == "Expert";
    }

    if (min == "Advanced") {
        return act == "Expert";
    }

    return false;
}

bool TipOfTheDayDialog::GetTODAtLevel(wxXmlDocument& doc, TODTracker& tracker, const std::string& level)
{
    uint32_t unvisited = 0;
    uint32_t of = 0;
    #ifdef __WXMSW__
        #define PLAT "Windows"
    #elif defined __WXOSX__
        #define PLAT "OSX"
    #elif defined LINUX
        #define PLAT "Linux"
    #else
        #define PLAT "UNKNOWN"
    #endif

    for (auto n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
        if (n->GetName() == "tip") {
            auto url = n->GetAttribute("url", "");
            if (url != "") {
                auto exclude = n->GetAttribute("exclude", "");

                if (!Contains(exclude, PLAT)) {
                    auto tiplevel = n->GetAttribute("level", "Beginner");

                    if (level == tiplevel) {
                        ++of;
                        if (tracker.GetVisited(url) == 0) {
                            unvisited++;
                        }
                    }
                }
            }
        }
    }

    if (of == 0) {
        return false;
    }

    if (unvisited == 0) {
        return false;
    }

    uint32_t choice = rand01() * (unvisited - 1);

    auto n = doc.GetRoot()->GetChildren();
    do {
        auto url = n->GetAttribute("url", "");
        if (url != "") {
            auto exclude = n->GetAttribute("exclude", "");
            if (!Contains(exclude, PLAT)) {
                auto tiplevel = n->GetAttribute("level", "Beginner");
                if (tracker.GetVisited(url) == 0 && level == tiplevel) {
                    if (choice == 0) {
                        tracker.AddVisited(url);
                        LoadURL(BuildURL(url));

                        if (!IsShown()) {
                            Show();
                            Raise();
                        }

                        return true;
                    }
                    --choice;
                }
            }
        }
        n = n->GetNext();
    } while (n != nullptr);

    return false;
}

bool TipOfTheDayDialog::DoTipOfDay(bool force)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    logger_base.debug("Getting tip of the day");

    wxXmlDocument doc;
#ifdef USE_GITHUB_HOSTED_TOD
    _thread = nullptr;
#else
#endif

    wxConfigBase* config = wxConfigBase::Get();
    auto mintiplevel = config->Read("MinTipLevel", "Beginner");
    auto onlyshowunseen = config->Read("OnlyShowUnseenTips", true);

    // if tips are disabled then just exit
    if (mintiplevel == "Off") {
        if (force) {
            mintiplevel = "Beginner";
        } else {
            logger_base.debug("Tip of the day disabled.");
            return false;
        }
        ShowTipsCheckbox->SetValue(0);
    } else {
        ShowTipsCheckbox->SetValue(1);
    }

    
    
    std::string file = GetTODXMLFile();
    if (!wxFile::Exists(file)) {
        logger_base.warn("Tip Of Day unable to load file: %s", (const char*)file.c_str());
        logger_base.info("Current directory: %s", (const char*)wxGetCwd().c_str());
        return false;
    } else {
        logger_base.debug("Loading tip of the day xml file %s.", (const char*)file.c_str());
        if (!doc.Load(file)) {
            logger_base.warn("Error loading xml file: %s", (const char*)file.c_str());
            return false;
        }

        TODTracker tracker;
        if (mintiplevel == "Beginner") {
            if (GetTODAtLevel(doc, tracker, "Beginner")) {
                return true;
            }
            mintiplevel = "Intermediate";
        }

        if (mintiplevel == "Intermediate") {
            if (GetTODAtLevel(doc, tracker, "Intermediate")) {
                return true;
            }
            mintiplevel = "Advanced";
        }

        if (mintiplevel == "Advanced") {
            if (GetTODAtLevel(doc, tracker, "Advanced")) {
                return true;
            }
            mintiplevel = "Expert";
        }

        if (mintiplevel == "Expert") {
            if (GetTODAtLevel(doc, tracker, "Expert")) {
                return true;
            }
        }

        // if everything has been seen then clear visited and start again
        if (!onlyshowunseen) {
            logger_base.debug("Clearing viewed tips.");
            ClearVisited();
            return DoTipOfDay(force);
        }
    }

    return false;
}

void TipOfTheDayDialog::ClearVisited()
{
    TODTracker tracker;
    tracker.ClearVisited();
}

void TipOfTheDayDialog::PrepTipOfDay(wxWindow* notify)
{
    // if we want to now we can spin up a thread and download any tip of day content. When done we should send to the notify within a EVT_TIPOFDAY_READY message
#ifdef USE_GITHUB_HOSTED_TOD
    _thread = new TipOfDayThread(notify);
    _thread->Run();
#else
    // exe location/TipOfDay/tod.xml
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    std::string fname = f.GetPath() + "/TipOfDay/tod.xml";

    wxCommandEvent e(EVT_TIPOFDAY_READY);
    e.SetString(fname);
    wxPostEvent(notify, e);
#endif
}

TipOfTheDayDialog::~TipOfTheDayDialog()
{
	//(*Destroy(TipOfTheDayDialog)
	//*)
    SaveWindowPosition("xLightsTipOfTheDay", this);
}

void TipOfTheDayDialog::OnButton_NextClick(wxCommandEvent& event)
{
    if (!DoTipOfDay(true)) {
        wxBell();
        Button_Next->Disable();
    }
}

void TipOfTheDayDialog::OnCloseButtonClick(wxCommandEvent& event)
{
    Close();
}

void TipOfTheDayDialog::OnShowTipsCheckboxClick(wxCommandEvent& event)
{
    wxConfigBase* config = wxConfigBase::Get();
    config->Write("MinTipLevel", ShowTipsCheckbox->GetValue() ? "Beginner" : "Off");
    config->Flush();
}
