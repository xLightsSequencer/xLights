#include "TipOfTheDayDialog.h"

//(*InternalHeaders(TipOfTheDayDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/xml/xml.h>
#include <wx/stdpaths.h>

#include "UtilFunctions.h"
#include "xLightsMain.h"
#include "../xSchedule/xSMSDaemon/Curl.h"
#include "CachedFileDownloader.h"

#include <log4cpp/Category.hh>

//#define USE_GITHUB_HOSTED_TOD

//(*IdInit(TipOfTheDayDialog)
const long TipOfTheDayDialog::ID_HTMLWINDOW1 = wxNewId();
const long TipOfTheDayDialog::ID_BUTTON1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(TipOfTheDayDialog,wxDialog)
	//(*EventTable(TipOfTheDayDialog)
	//*)
END_EVENT_TABLE()

class TODTracker
{
    std::string _file;
    std::map<std::string, uint32_t> _visited;

public:
    TODTracker()
    {
        _file = GetTrackerFile();

        if (wxFile::Exists(_file)) {
            wxTextFile f(_file);
            uint32_t line = 0;
            if (f.Open()) {
                while (!f.Eof()) {
                    auto l = f.GetLine(line);
                    if (l == "END")
                        break;
                    if (l != "" && l.Contains(",")) {
                        auto ll = wxSplit(l, ',');
                        _visited[ll[0]] = wxAtoi(ll[1]);
                    }
                    ++line;
                }
            }
        }
    }

    static std::string GetTrackerFile()
    {
        return xLightsFrame::CurrentDir + "/tod.dat";
    }

    static void ClearVisited()
    {
        if (wxFile::Exists(GetTrackerFile()))
            wxRemoveFile(GetTrackerFile());
    }

    int GetVisited(const std::string& tod)
    {
        if (_visited.find(tod) == _visited.end())
            return 0;
        return _visited[tod];
    }

    void AddVisited(const std::string& tod)
    {
        if (_visited.find(tod) == _visited.end()) {
            _visited[tod] = 1;
        } else {
            ++_visited[tod];
        }
    }

    void Save()
    {
        wxFile f(_file, wxFile::OpenMode::write);
        if (f.IsOpened()) {
            for (const auto& it : _visited) {
                f.Write(it.first + "," + std::to_string(it.second) + "\n");
            }
            f.Write("END\n");
            f.Close();
        }
    }
};

class TipOfDayThread : public wxThread
{
    wxWindow* _notify = nullptr;
    std::string _downloadTo;

public:
    TipOfDayThread(wxWindow* notify, const std::string& downloadTo) :
        _notify(notify), _downloadTo(downloadTo)
    {}

    virtual void* Entry() override
    {
        // TODO download Tip of day content here

        CachedFileDownloader cache;

        auto file = cache.GetFile(wxURI("https://raw.githubusercontent.com/smeighan/xLights/master/TipOfDay/tod.xml"), CACHEFOR::CACHETIME_DAY);
        if (_downloadTo != "")
            wxCopyFile(file, _downloadTo, true);

        wxCommandEvent e(EVT_TIPOFDAY_READY);
        wxPostEvent(_notify, e);

        return nullptr;
    }
};

TipOfTheDayDialog::TipOfTheDayDialog(const std::string& url, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    //(*Initialize(TipOfTheDayDialog)
    Create(parent, id, _("Tip of the day"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxCLOSE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    HtmlWindow1 = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(1200,1200), wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    HtmlWindow1->SetPage(_("<html><body><p>Loading...</p></body></html>"));
    FlexGridSizer1->Add(HtmlWindow1, 1, wxALL|wxEXPAND, 5);
    Button_Next = new wxButton(this, ID_BUTTON1, _("Give me another one"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer1->Add(Button_Next, 1, wxALL|wxALIGN_RIGHT|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&TipOfTheDayDialog::OnButton_NextClick);
    //*)

    SetSize(1200, 800);
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
        if (StartsWith(url, "file://")) {
            auto file = url.substr(7);
            if (!wxFile::Exists(file)) {
                logger_base.warn("Tip Of Day unable to load file: %s", (const char*)file.c_str());
                logger_base.info("Current directory: %s", (const char*)wxGetCwd().c_str());
            } else {
                HtmlWindow1->LoadFile(wxFileName(file));
            }
        } else {
            HtmlWindow1->LoadPage(url);
        }
    }
    Layout();
}

std::string TipOfTheDayDialog::GetTODXMLFile() const
{
#ifdef USE_GITHUB_HOSTED_TOD
    // tempdir/tod.xml
    wxFileName f(wxStandardPaths::Get().GetTempDir());
    return f.GetPath() + "/tod.xml";
#else
    // exe location/TipOfDay/tod.xml
    wxFileName f(wxStandardPaths::Get().GetExecutablePath());
    return f.GetPath() + "/TipOfDay/tod.xml";
#endif
}

std::string TipOfTheDayDialog::BuildURL(const std::string& url) const
{
#ifdef USE_GITHUB_HOSTED_TOD
    return "https://raw.githubusercontent.com/smeighan/xLights/master/TipOfDay/" + url;
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

bool TipOfTheDayDialog::DoTipOfDay()
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
        logger_base.debug("Tip of the day disabled.");
        return false;
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
        uint32_t unvisited = 0;
        uint32_t of = 0;

        for (auto n = doc.GetRoot()->GetChildren(); n != nullptr; n = n->GetNext()) {
            if (n->GetName() == "tip") {
                auto url = n->GetAttribute("url", "");
                if (url != "") {
                    auto level = n->GetAttribute("level", "Beginner");

                    if (IsLevelGreaterOrEqualTo(level, mintiplevel)) {
                        ++of;
                        if (tracker.GetVisited(url) == 0) {
                            unvisited++;
                        }
                    }
                }
            }
        }

        if (of == 0) {
            logger_base.debug("No tips of sufficient level found to display.");
            return false;
        }

        if (unvisited == 0) {

            logger_base.debug("All tips have been displayed.");

            // if everything has been seen then clear visited and start again
            if (!onlyshowunseen) {
                logger_base.debug("Clearing viewed tips.");
                ClearVisited();
                return DoTipOfDay();
            }

            return false;
        }

        uint32_t choice = rand01() * (unvisited - 1);

        auto n = doc.GetRoot()->GetChildren();
        do {
            auto url = n->GetAttribute("url", "");
            if (url != "") {
                auto level = n->GetAttribute("level", "Beginner");
                if (tracker.GetVisited(url) == 0 && IsLevelGreaterOrEqualTo(level, mintiplevel)) {
                    if (choice == 0) {
                        tracker.AddVisited(url);
                        LoadURL(BuildURL(url));

                        if (!IsShown())
                            Show();

                        tracker.Save();
                        return true;
                    }
                    --choice;
                }
            }
        } while (choice != 0);

        logger_base.warn("Thats odd ... i did not find my selected tip.");
    }

    return false;
}

void TipOfTheDayDialog::ClearVisited()
{
    TODTracker::ClearVisited();
}

void TipOfTheDayDialog::PrepTipOfDay(wxWindow* notify)
{
    // if we want to now we can spin up a thread and download any tip of day content. When done we should send to the notify within a EVT_TIPOFDAY_READY message
#ifdef USE_GITHUB_HOSTED_TOD
    _thread = new TipOfDayThread(notify, GetTODXMLFile());
    _thread->Run();
#else
    wxCommandEvent e(EVT_TIPOFDAY_READY);
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
    if (!DoTipOfDay()) {
        wxBell();
        Button_Next->Disable();
    }
}
