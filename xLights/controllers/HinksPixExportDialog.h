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

#include "../FSEQFile.h"

class ModelManager;
class OutputManager;
class Output;
class ControllerEthernet;

static std::vector<wxString> const DAYS{ "SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY" };

struct PlayListItem {
    PlayListItem(wxString fseq) :
        FSEQ(std::move(fseq))
    {}
    PlayListItem(wxString fseq, wxString audio) :
        FSEQ(std::move(fseq)), Audio(std::move(audio))
    {}

    PlayListItem(wxJSONValue const& json)
    {
        fromJSON(json);
    }
    wxString FSEQ;
    wxString Audio;

    wxString HSEQ;
    wxString AU{ "NONE" };

    [[nodiscard]] wxJSONValue asJSON() const
    {
        wxJSONValue vs;
        vs["f"] = FSEQ;
        vs["a"] = Audio;
        return vs;
    }

    void fromJSON(wxJSONValue const& json)
    {
        FSEQ = json.ItemAt("f").AsString();
        Audio = json.ItemAt("a").AsString();
    }

    [[nodiscard]] wxString asString() const
    {
        return wxString::Format("{\"H\":\"%s\",\"A\":\"%s\",\"D\":2}", HSEQ, AU);//D is a delay, maybe set to zero?
    }
};

struct PlayList
{
    PlayList(wxString name) :
        Name(std::move(name))
    {}

    PlayList(wxJSONValue const& json)
    {
        fromJSON(json);
    }
    wxString Name;
    std::vector<PlayListItem> Items;

    [[nodiscard]] wxJSONValue asJSON() const
    {
        wxJSONValue vs;
        vs["n"] = Name;
        for (auto const& it : Items) {
            vs["pl"].Append(it.asJSON());
        }
        return vs;
    }

    void fromJSON(wxJSONValue const& json)
    {
        Name = json.ItemAt("n").AsString();
        auto jArry = json.ItemAt("pl");
        if (jArry.IsArray()) {
            for (int x = 0; x < jArry.Size(); x++) {
                Items.emplace_back(jArry.ItemAt(x));
            }
        }
    }

    void saveAsFile(wxString const& drive) const
    {
        wxArrayString main;
        for (auto const& it : Items) {
            main.Add(it.asString());
        }
        wxString const filename = drive + wxFileName::GetPathSeparator() + Name + ".ply";
        wxFile f;
        f.Open(filename, wxFile::write);
        if (f.IsOpened()) {
            f.Write("[");
            f.Write(wxJoin(main, ',', '\0'));
            f.Write("]");
            f.Close();
        }
    }
};

struct ScheduleItem {
    ScheduleItem(wxString playlist) :
        Playlist(std::move(playlist))
    {}
    ScheduleItem(wxJSONValue const& json)
    {
        fromJSON(json);
    }

    wxString Playlist;
    int StartHour;
    int StartMin;
    int EndHour;
    int EndMin;
    bool Enabled{true};

    [[nodiscard]] wxJSONValue asJSON() const
    {
        wxJSONValue vs;
        vs["pl"] = Playlist;
        vs["sh"] = StartHour;
        vs["sm"] = StartMin;
        vs["eh"] = EndHour;
        vs["em"] = EndMin;
        vs["en"] = Enabled;
        return vs;
    }
    void fromJSON(wxJSONValue const& json)
    {
        Playlist = json.ItemAt("pl").AsString();
        StartHour = json.ItemAt("sh").AsInt();
        StartMin = json.ItemAt("sm").AsInt();
        EndHour = json.ItemAt("eh").AsInt();
        EndMin = json.ItemAt("em").AsInt();
        Enabled = json.ItemAt("en").AsBool();
    }
    [[nodiscard]] wxString asString() const
    {
        // Q is repeat count, 0 = infinite, 1 is "play once" in HSA
        return wxString::Format("{\"S\":\"%d%02d\",\"E\":\"%d%02d\",\"P\":\"%s.ply\",\"Q\":0}", StartHour, StartMin, EndHour, EndMin, Playlist);
    }
};

struct Schedule {

    Schedule(wxString day) :
        Day(std::move(day))
    {}
    Schedule(wxJSONValue const& json)
    {
        fromJSON(json);
    }

    wxString Day;
    std::vector<ScheduleItem> Items;

    [[nodiscard]] wxJSONValue asJSON() const
    {
        wxJSONValue vs;
        vs["d"] = Day;
        for (auto const& it : Items) {
            vs["sc"].Append(it.asJSON());
        }
        return vs;
    }
    void fromJSON(wxJSONValue const& json)
    {
        Day = json.ItemAt("d").AsString();
        auto jArry = json.ItemAt("sc");
        if (jArry.IsArray()) {
            for (int x = 0; x < jArry.Size(); x++) {
                Items.emplace_back(jArry.ItemAt(x));
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

    void saveAsFile(wxString const& drive) const
    {
        wxArrayString main;
        auto sItems = GetSortedSchedule();
        for (auto const& it : sItems) {
            if (it.Enabled) {
                main.Add(it.asString());
            }
        }
        wxString const filename = drive + wxFileName::GetPathSeparator() + Day + ".sched";
        wxFile f;
        f.Open(filename, wxFile::write);
        if (f.IsOpened()) {
            f.Write("[");
            f.Write(wxJoin(main, ',', '\0'));
            f.Write("]");
            f.Close();
        }
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
    static const long ID_SCROLLEDWINDOW1;
    static const long ID_STATICTEXT3;
    static const long ID_CHOICE_PLAYLISTS;
    static const long ID_BUTTON_ADD_PLAYLIST;
    static const long ID_BUTTON_REMOVE;
    static const long ID_STATICTEXT1;
    static const long ID_CHOICE_FILTER;
    static const long ID_STATICTEXT2;
    static const long ID_CHOICE_FOLDER;
    static const long ID_BITMAPBUTTON_MOVE_UP;
    static const long ID_BITMAPBUTTON_MOVE_DOWN;
    static const long ID_LISTVIEW_Sequences;
    static const long ID_PANEL1;
    static const long ID_GRID_SCHEDULE;
    static const long ID_PANEL4;
    static const long ID_NOTEBOOK_EXPORT_ITEMS;
    static const long ID_BUTTON_REFRESH;
    static const long ID_BUTTON_EXPORT;
    static const long wxID_Close;
    //*)

    static const long ID_MNU_SELECTALL;
    static const long ID_MNU_SELECTNONE;
    static const long ID_MNU_SELECTHIGH;
    static const long ID_MNU_DESELECTHIGH;

    static const long ID_MNU_SETALL;
    static const long ID_MNU_SETALLPLAY;

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

    void OnPopup(wxCommandEvent& event);
    void OnPopupGrid(wxCommandEvent& event);

    void OnChoiceSelected(wxCommandEvent& event);

    void SaveSettings();
    void LoadSettings();
    void ApplySavedSettings(wxJSONValue json);

    void AddInstanceHeader(wxString const& h);

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

    DECLARE_EVENT_TABLE()
};

#endif
