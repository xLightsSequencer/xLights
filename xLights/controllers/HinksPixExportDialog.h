#ifndef HINKSPIXEXPORTDIALOG_H
#define HINKSPIXEXPORTDIALOG_H

#include <algorithm>
#include <array>
#include <list>
#include <map>
#include <optional>
#include <vector>

//(*Headers(HinksPixExportDialog)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/grid.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <nlohmann/json.hpp>
#include "../FSEQFile.h"

class ModelManager;
class OutputManager;
class Output;
class ControllerEthernet;

static std::vector<std::string> const DAYS{ "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };

struct PlayListItem {
    explicit PlayListItem(std::string fseq) :
        FSEQ(std::move(fseq))
    {}
    PlayListItem(std::string fseq, std::string audio) :
        FSEQ(std::move(fseq)), Audio(std::move(audio))
    {}

    explicit PlayListItem(nlohmann::json const& json)
    {
        fromJSON(json);
    }
    std::string FSEQ;
    std::string Audio;

    std::string HSEQ;
    std::string AU{ "NONE" };

    [[nodiscard]] nlohmann::json asJSON() const
    {
        nlohmann::json vs;
        vs["f"] = FSEQ;
        vs["a"] = Audio;
        return vs;
    }

    void fromJSON(nlohmann::json const& json)
    {
        FSEQ = json.at("f").get<std::string>();
        Audio = json.at("a").get<std::string>();
    }

    [[nodiscard]] wxString asString() const
    {
        // switch to std::format if gcc supports it
        return wxString::Format("{\"H\":\"%s\",\"A\":\"%s\",\"D\":2}", HSEQ, AU);//D is a delay, maybe set to zero?
    }
};

struct PlayList
{
    explicit PlayList(std::string name) :
        Name(std::move(name))
    {}

    explicit PlayList(nlohmann::json const& json)
    {
        fromJSON(json);
    }
    std::string Name;
    std::vector<PlayListItem> Items;

    [[nodiscard]] nlohmann::json asJSON() const
    {
        nlohmann::json vs;
        vs["n"] = Name;
        for (auto const& it : Items) {
            vs["pl"].push_back(it.asJSON());
        }
        return vs;
    }

    void fromJSON(nlohmann::json const& json)
    {
        Name = json.at("n").get<std::string>();
        auto jArry = json.at("pl");
        if (jArry.is_array()) {
            for (int x = 0; x < jArry.size(); x++) {
                Items.emplace_back(jArry.at(x));
            }
        }
    }

    [[nodiscard]] std::string getFileName() const {
        return Name + ".ply";
    }

    void saveAsFile(wxString const& filename) const {
        wxArrayString main;
        for (auto const& it : Items) {
            main.Add(it.asString());
        }
        wxFile f;
        f.Open(filename, wxFile::write);
        if (f.IsOpened()) {
            f.Write("[");
            f.Write(wxJoin(main, ',', '\0'));
            f.Write("]");
            f.Close();
        }
    }
    void saveToDrive(wxString const& drive) const {
        wxString const filename = drive + wxFileName::GetPathSeparator() + getFileName();
        saveAsFile(filename);
    }
};

struct ScheduleItem {
    explicit ScheduleItem(std::string playlist) :
        Playlist(std::move(playlist))
    {}
    explicit ScheduleItem(nlohmann::json const& json)
    {
        fromJSON(json);
    }

    std::string Playlist;
    int StartHour;
    int StartMin;
    int EndHour;
    int EndMin;
    bool Enabled{true};
    int Repeat{ 0 };

    [[nodiscard]] nlohmann::json asJSON() const
    {
        nlohmann::json vs;
        vs["pl"] = Playlist;
        vs["sh"] = StartHour;
        vs["sm"] = StartMin;
        vs["eh"] = EndHour;
        vs["em"] = EndMin;
        vs["en"] = Enabled;
        vs["rp"] = Repeat;
        return vs;
    }
    void fromJSON(nlohmann::json const& json) {
        Playlist = json.at("pl").get<std::string>();
        StartHour = json.at("sh").get<int>();
        StartMin = json.at("sm").get<int>();
        EndHour = json.at("eh").get<int>();
        EndMin = json.at("em").get<int>();
        Enabled = json.at("en").get<bool>();
        if (json.contains("rp")) {
            Repeat = json.at("rp").get<int>();
        }
    }
    [[nodiscard]] wxString asString() const
    {
        // Q is repeat count, 0 = infinite, 1 is "play once" in HSA
        return wxString::Format("{\"S\":\"%d%02d\",\"E\":\"%d%02d\",\"P\":\"%s.ply\",\"Q\":%d}", StartHour, StartMin, EndHour, EndMin, Playlist, Repeat);
    }

    [[nodiscard]] static bool isValidRange(int val, int start, int end)
    {
        return (val >= start && val <= end);
    }

    [[nodiscard]] bool isValidTimes() const
    {
        return isValidRange(StartHour, 0, 23) &&
               isValidRange(StartMin, 0, 59) &&
               isValidRange(EndHour, 0, 23) &&
               isValidRange(EndMin, 0, 59);
    }

    [[nodiscard]] bool isValid(wxString& reason) const
    {
        if (!isValidTimes()) {
            reason = Playlist + " Hour/Minutes not valid range 0-23, 0-59";
            return false;
        }
        if (EndHour < StartHour) {
            reason = Playlist + "End Hour is Less Then Start Hour";
            return false;
        }
        if (EndHour == StartHour && EndMin <= StartMin) {
            reason = Playlist + "End Minute is Less Then Start Minute";
            return false;
        }
        return true;
    }
};

struct Schedule {

    explicit Schedule(std::string day) :
        Day(std::move(day))
    {}
    explicit Schedule(nlohmann::json const& json)
    {
        fromJSON(json);
    }

    std::string Day;
    std::vector<ScheduleItem> Items;

    [[nodiscard]] nlohmann::json asJSON() const
    {
        nlohmann::json vs;
        vs["d"] = Day;
        for (auto const& it : Items) {
            vs["sc"].push_back(it.asJSON());
        }
        return vs;
    }
    void fromJSON(nlohmann::json const& json)
    {
        Day = json.at("d").get<std::string>();
        auto jArry = json.at("sc");
        if (jArry.is_array()) {
            for (int x = 0; x < jArry.size(); x++) {
                Items.emplace_back(jArry.at(x));
            }
        }
    }
    void clearItems()
    {
        Items.clear();
    }

    void removePlaylist(wxString const& playlist)
    {
        Items.erase(std::remove_if(Items.begin(), Items.end(), [&playlist](auto const& it) { return it.Playlist == playlist; }), Items.end());
        //std::erase_if(Items, [&playlist](auto const& it) { return it.Playlist == playlist; });
    }

    [[nodiscard]] std::vector<ScheduleItem> GetSortedSchedule() const
    {
        std::vector<ScheduleItem> sorted = Items;
        std::sort(sorted.begin(), sorted.end(), [](const auto& lhs, const auto& rhs) {
            if (lhs.StartHour == rhs.StartHour) {
                return lhs.StartMin < rhs.StartMin;
            }
            return lhs.StartHour < rhs.StartHour;
        });
        return sorted;
    }

    [[nodiscard]] wxString getFileName() const {
        return Day + ".sched";
    }

    void saveAsFile(wxString const& filename) const
    {
        wxArrayString main;
        auto sItems = GetSortedSchedule();
        for (auto const& it : sItems) {
            if (it.Enabled) {
                main.Add(it.asString());
            }
        }
        wxFile f;
        f.Open(filename, wxFile::write);
        if (f.IsOpened()) {
            f.Write("[");
            f.Write(wxJoin(main, ',', '\0'));
            f.Write("]");
            f.Close();
        }
    }

    void saveToDrive(wxString const& drive) const {
        wxString const filename = drive + wxFileName::GetPathSeparator() + getFileName();
        saveAsFile(filename);
    }

    [[nodiscard]] bool isValid(wxString &reason) const
    {
        auto sItems = GetSortedSchedule();
        for (auto const& it : sItems) {
            if (!it.Enabled) {
                continue;
            }
            if (!it.isValid(reason)) {
                return false;
            }
        }
        //check for overlapping times
        //for (auto it1 = std::cbegin(sItems); it1 != std::cend(sItems); ++it1) {
        //    auto& schItm1 = *it1;
        //    if (!schItm1.Enabled) {
        //        continue;
        //    }
        //    for (auto it2 = std::next(it1); it2 != std::cend(sItems); ++it2) {
        //        auto& schItm2 = *it2;
        //        if (!schItm2.Enabled) {
        //            continue;
        //        }
        //        if (schItm1.EndHour > schItm2.StartHour) {
        //            reason = wxString::Format("%s End Hour overlaps %s Start Hour", schItm1.Playlist, schItm2.Playlist);
        //            return false;
        //        }
        //        if (schItm1.EndHour == schItm2.StartHour && schItm1.EndMin >= schItm1.StartMin) {
        //            reason = wxString::Format("%s End Minute overlaps %s Start Minute", schItm1.Playlist, schItm2.Playlist);
        //            return false;
        //        }
        //    }
        //}
        return true;
    }
};

class HSEQFile : public V1FSEQFile {
public:
    HSEQFile(std::string const& fn, ControllerEthernet* hinx, ControllerEthernet* slave1, ControllerEthernet* slave2, uint32_t orgChannelCount) :
        V1FSEQFile(fn), m_hinx(hinx), m_slave1(slave1), m_slave2(slave2), m_orgChannelCount(orgChannelCount){};

    void writeHeader() override;

private:
    ControllerEthernet* m_hinx;
    ControllerEthernet* m_slave1;
    ControllerEthernet* m_slave2;
    uint32_t m_orgChannelCount;
};

struct HinksChannelMap {
    HinksChannelMap(int32_t orgStartChannel, int32_t ChannelCount, int32_t hinksStartChannel) :
        OrgStartChannel(orgStartChannel), ChannelCount(ChannelCount), HinksStartChannel(hinksStartChannel) {
    }
    int32_t const OrgStartChannel;
    int32_t const ChannelCount;
    int32_t const HinksStartChannel;

    void Dump() const;
};

class HinksPixExportDialog : public wxDialog {
public:
    HinksPixExportDialog(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~HinksPixExportDialog();

    //(*Declarations(HinksPixExportDialog)
    wxBitmapButton* BitmapButtonMoveDown;
    wxBitmapButton* BitmapButtonMoveUp;
    wxButton* AddRefreshButton;
    wxButton* ButtonAddPlaylist;
    wxButton* ButtonRemove;
    wxButton* ButtonUpload;
    wxButton* Button_Export;
    wxChoice* ChoiceFilter;
    wxChoice* ChoiceFolder;
    wxChoice* ChoicePlaylists;
    wxFlexGridSizer* HinkControllerSizer;
    wxGrid* GridSchedule;
    wxListView* CheckListBox_Sequences;
    wxNotebook* NotebookExportItems;
    wxPanel* Panel1;
    wxPanel* Panel4;
    wxScrolledWindow* HinkControllerList;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText3;
    //*)
protected:
    //(*Identifiers(HinksPixExportDialog)
    static const wxWindowID ID_SCROLLEDWINDOW1;
    static const wxWindowID ID_STATICTEXT3;
    static const wxWindowID ID_CHOICE_PLAYLISTS;
    static const wxWindowID ID_BUTTON_ADD_PLAYLIST;
    static const wxWindowID ID_BUTTON_REMOVE;
    static const wxWindowID ID_STATICTEXT1;
    static const wxWindowID ID_CHOICE_FILTER;
    static const wxWindowID ID_STATICTEXT2;
    static const wxWindowID ID_CHOICE_FOLDER;
    static const wxWindowID ID_BITMAPBUTTON_MOVE_UP;
    static const wxWindowID ID_BITMAPBUTTON_MOVE_DOWN;
    static const wxWindowID ID_LISTVIEW_Sequences;
    static const wxWindowID ID_PANEL1;
    static const wxWindowID ID_GRID_SCHEDULE;
    static const wxWindowID ID_PANEL4;
    static const wxWindowID ID_NOTEBOOK_EXPORT_ITEMS;
    static const wxWindowID ID_BUTTON_REFRESH;
    static const wxWindowID ID_BUTTON_EXPORT;
    static const wxWindowID ID_BUTTON_UPLOAD;
    static const wxWindowID wxID_Close;
    //*)

    static const long ID_MNU_SELECTALL;
    static const long ID_MNU_SELECTNONE;
    static const long ID_MNU_SELECTHIGH;
    static const long ID_MNU_DESELECTHIGH;

    static const long ID_MNU_SETALL;
    static const long ID_MNU_SETALLPLAY;
    static const long ID_MNU_SETALLDAYS;

    static const long ID_MNU_SETMASTER;
    static const long ID_MNU_SETREMOTE;
    static const long ID_MNU_SETTIME;
    static const long ID_MNU_UPLOADFILE;
    static const long ID_MNU_UPLOADFIRM;
    static const long ID_MNU_UPLOADSCHEDULE;

    ModelManager* m_modelManager = nullptr;
    OutputManager* m_outputManager = nullptr;

    std::vector<ControllerEthernet*> m_hixControllers;
    std::vector<ControllerEthernet*> m_otherControllers;
    wxArrayString m_drives;

    std::string m_selectedPlayList;
    std::vector<PlayList> m_playLists;
    std::vector<Schedule> m_schedules;

private:
    //(*Handlers(HinksPixExportDialog)
    void OnClose(wxCloseEvent& event);
    void SequenceListPopup(wxListEvent& event);
    void OnAddRefreshButtonClick(wxCommandEvent& event);
    void OnButton_ExportClick(wxCommandEvent& event);
    void OnBitmapButtonMoveDownClick(wxCommandEvent& event);
    void OnBitmapButtonMoveUpClick(wxCommandEvent& event);
    void OnButtonAddPlaylistClick(wxCommandEvent& event);
    void OnButtonRemoveClick(wxCommandEvent& event);
    void OnButton_CloseClick(wxCommandEvent& event);
    void OnGridScheduleCellChanged(wxGridEvent& event);
    void OnGridScheduleCellRightClick(wxGridEvent& event);
    void OnButtonUploadClick(wxCommandEvent& event);
    //*)

    void CreateDriveList();
    void LoadSequencesFromFolder(wxString dir) const;
    void LoadSequences();
    void PopulateControllerList(OutputManager* outputManager);

    void GetFolderList(const wxString& folder);

    void moveSequenceItem(int to, int from, bool select = true);

    void createModeFile(wxString const& drive, int mode) const;

    bool Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortHSEQName, ControllerEthernet* hix, ControllerEthernet* slave1, ControllerEthernet* slave2, wxString& errorMsg);

    [[nodiscard]] wxString createUniqueShortName(wxString const& fseqName, std::vector<wxString> const& names);

    bool Make_AU_From_ProcessedAudio(const std::vector<int16_t>& processedAudio, wxString const& AU_File, wxString& errorMsg);

    [[nodiscard]] int getMaxSlaveControllerUniverses(ControllerEthernet* controller) const;

    void ControllerPopupMenu(wxContextMenuEvent& event);
    void OnPopup(wxCommandEvent& event);
    void OnPopupGrid(wxCommandEvent& event);

    void OnChoiceSelected(wxCommandEvent& event);

    void SaveSettings();
    void LoadSettings();
    void ApplySavedSettings(nlohmann::json controllers);

    wxPanel* AddInstanceHeader(wxString const& h);

    [[nodiscard]] bool GetCheckValue(wxString const& col) const;
    [[nodiscard]] wxString GetChoiceValue(wxString const& col) const;
    [[nodiscard]] int GetChoiceValueIndex(wxString const& col) const;

    void SetChoiceValue(wxString const& col, std::string const& value);
    void SetChoiceValueIndex(wxString const& col, int i);
    void SetCheckValue(wxString const& col, bool b);
    void SetDropDownItems(wxString const& col, wxArrayString const& items);

    bool CheckSlaveControllerSizes(ControllerEthernet* controller, ControllerEthernet* slave1, ControllerEthernet* slave2);

    ControllerEthernet* getSlaveController(std::string const& name);

    [[nodiscard]] std::vector<HinksChannelMap> getModelChannelMap(ControllerEthernet* hinks, int32_t& chanCount) const;
    
    [[nodiscard]] bool createTestFile(wxString const& drive) const;

    [[nodiscard]] std::optional<std::reference_wrapper<PlayList>> GetPlayList(wxString const& name);
    void DeletePlayList(wxString const& name);

    [[nodiscard]] std::optional<std::reference_wrapper<Schedule>> GetSchedule(wxString const& day);

    void StoreToObjectPlayList(wxString const& playlist);
    void RedrawPlayList(wxString const& new_playlist, bool save_old = true);
    void StoreToObjectSchedule();
    void RedrawSchedules();
    bool CheckSchedules();

    void UploadFile(ControllerEthernet* controller);
    void ExtractFirmware(ControllerEthernet* controller);
    void UploadSchedules(ControllerEthernet* controller);

    DECLARE_EVENT_TABLE()
};

#endif
