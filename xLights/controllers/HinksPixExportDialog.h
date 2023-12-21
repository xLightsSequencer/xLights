#ifndef HINKSPIXEXPORTDIALOG_H
#define HINKSPIXEXPORTDIALOG_H

#include <array>
#include <list>
#include <map>

//(*Headers(HinksPixExportDialog)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
//*)

#include "../FSEQFile.h"

class ModelManager;
class OutputManager;
class Output;
class ControllerEthernet;

class HSEQFile : public V1FSEQFile {
public:
    HSEQFile(const std::string& fn, ControllerEthernet* hinx, ControllerEthernet* slave1, ControllerEthernet* slave2, uint32_t orgChannelCount) :
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
    void SaveSettings();

public:
    HinksPixExportDialog(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~HinksPixExportDialog();

    //(*Declarations(HinksPixExportDialog)
    wxBitmapButton* BitmapButtonMoveDown;
    wxBitmapButton* BitmapButtonMoveUp;
    wxButton* AddRefreshButton;
    wxButton* Button_Export;
    wxChoice* ChoiceFilter;
    wxChoice* ChoiceFolder;
    wxFlexGridSizer* HinkControllerSizer;
    wxListView* CheckListBox_Sequences;
    wxPanel* Panel1;
    wxScrolledWindow* HinkControllerList;
    wxSpinCtrl* SpinCtrlEndHour;
    wxSpinCtrl* SpinCtrlEndMin;
    wxSpinCtrl* SpinCtrlStartHour;
    wxSpinCtrl* SpinCtrlStartMin;
    wxSplitterWindow* SplitterWindow1;
    wxStaticText* StaticText1;
    wxStaticText* StaticText2;
    wxStaticText* StaticText5;
    wxStaticText* StaticText6;
    //*)
protected:
    //(*Identifiers(HinksPixExportDialog)
    static const long ID_SCROLLEDWINDOW1;
    static const long ID_STATICTEXT1;
    static const long ID_CHOICE_FILTER;
    static const long ID_STATICTEXT2;
    static const long ID_CHOICE_FOLDER;
    static const long ID_BITMAPBUTTON_MOVE_UP;
    static const long ID_BITMAPBUTTON_MOVE_DOWN;
    static const long ID_LISTVIEW_Sequences;
    static const long ID_PANEL1;
    static const long ID_SPLITTERWINDOW1;
    static const long ID_BUTTON_REFRESH;
    static const long ID_STATICTEXT5;
    static const long ID_SPINCTRL_START_HOUR;
    static const long ID_SPINCTRL_START_MIN;
    static const long ID_STATICTEXT6;
    static const long ID_SPINCTRL_END_HOUR;
    static const long ID_SPINCTRL_END_MIN;
    static const long ID_BUTTON_EXPORT;
    //*)

    static const long ID_MNU_SELECTALL;
    static const long ID_MNU_SELECTNONE;
    static const long ID_MNU_SELECTHIGH;
    static const long ID_MNU_DESELECTHIGH;

    ModelManager* m_modelManager = nullptr;
    OutputManager* m_outputManager = nullptr;

    std::vector<ControllerEthernet*> m_hixControllers;
    std::vector<ControllerEthernet*> m_otherControllers;
    wxArrayString m_drives;

private:
    //(*Handlers(HinksPixExportDialog)
    void OnClose(wxCloseEvent& event);
    void SequenceListPopup(wxListEvent& event);
    void OnAddRefreshButtonClick(wxCommandEvent& event);
    void OnButton_ExportClick(wxCommandEvent& event);
    void OnBitmapButtonMoveDownClick(wxCommandEvent& event);
    void OnBitmapButtonMoveUpClick(wxCommandEvent& event);
    //*)

    void CreateDriveList();
    void LoadSequencesFromFolder(wxString dir) const;
    void LoadSequences();
    void PopulateControllerList(OutputManager* outputManager);

    void GetFolderList(const wxString& folder);

    void moveSequenceItem(int to, int from, bool select = true);

    void createPlayList(std::vector<std::tuple<wxString, wxString>> const& songs, wxString const& drive) const;

    void createSchedule(wxString const& drive) const;

    void createModeFile(wxString const& drive, int mode) const;

    bool Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortHSEQName, ControllerEthernet* hix, ControllerEthernet* slave1, ControllerEthernet* slave2, wxString& errorMsg);

    [[nodiscard]] wxString createUniqueShortName(wxString const& fseqName, std::vector<wxString> const& names);

    bool Make_AU_From_ProcessedAudio(const std::vector<int16_t>& processedAudio, wxString const& AU_File, wxString& errorMsg);

    [[nodiscard]] int getMaxSlaveControllerUniverses(ControllerEthernet* controller) const;

    void OnPopup(wxCommandEvent& event);

    void OnChoiceSelected(wxCommandEvent& event);

    void ApplySavedSettings();

    void AddInstanceHeader(const wxString& h);

    [[nodiscard]] bool GetCheckValue(const wxString& col) const;
    [[nodiscard]] wxString GetChoiceValue(const wxString& col) const;
    [[nodiscard]] int GetChoiceValueIndex(const wxString& col) const;

    void SetChoiceValue(const wxString& col, const std::string& value);
    void SetChoiceValueIndex(const wxString& col, int i);
    void SetCheckValue(const wxString& col, bool b);
    void SetDropDownItems(const wxString& col, const wxArrayString& items);

    bool CheckSlaveControllerSizes(ControllerEthernet* controller, ControllerEthernet* slave1, ControllerEthernet* slave2);

    ControllerEthernet* getSlaveController(const std::string& name);

    [[nodiscard]] std::vector<HinksChannelMap> getModelChannelMap(ControllerEthernet* hinks, int32_t& chanCount) const;
    
    [[nodiscard]] bool createTestFile(wxString const& drive) const;

    DECLARE_EVENT_TABLE()
};

#endif
