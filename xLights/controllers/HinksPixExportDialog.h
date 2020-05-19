#ifndef HINKSPIXEXPORTDIALOG_H
#define HINKSPIXEXPORTDIALOG_H

#include <list>
#include <map>
#include <array>

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

class OutputManager;
class Output;
class ControllerEthernet;

class HSEQFile : public V1FSEQFile {
public:
	HSEQFile(const std::string& fn, ControllerEthernet* hinx, ControllerEthernet* slave1, ControllerEthernet* slave2, uint32_t orgChannelCount)
		: V1FSEQFile(fn), _hinx(hinx), _slave1(slave1), _slave2(slave2), _orgChannelCount(orgChannelCount){
	};


	virtual void writeHeader() override;
private:
	ControllerEthernet* _hinx;
	ControllerEthernet* _slave1;
	ControllerEthernet* _slave2;
	uint32_t      _orgChannelCount;
};

class HinksPixExportDialog: public wxDialog
{
    void SaveSettings();

	public:

		HinksPixExportDialog(wxWindow* parent, OutputManager* outputManager, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
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

		std::vector <ControllerEthernet*> _hixControllers;
		std::vector <ControllerEthernet*> _otherControllers;
		wxArrayString _drives;

	private:

		//(*Handlers(HinksPixExportDialog)
		void OnClose(wxCloseEvent& event);
		void SequenceListPopup(wxListEvent& event);
		void OnChoiceFolderSelect(wxCommandEvent& event);
		void OnChoiceFilterSelect(wxCommandEvent& event);
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

		void createPlayList(std::vector<std::tuple<wxString, wxString>> const& songs, wxString const& drive);

		void createSchedule(wxString const& drive);

		void createModeFile(wxString const& drive, int mode);

		bool Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortFSEQName, ControllerEthernet* hix, ControllerEthernet* slave1, ControllerEthernet* slave2, wxString& errorMsg);

		wxString createUniqueShortName(wxString const& fseqName, std::vector<wxString> const& names);

		bool Make_AU_From_ProcessedAudio( const std::vector<int16_t>& processedAudio, wxString const& AU_File, wxString& errorMsg );

		int howManySlaveUniverses(ControllerEthernet* controller);

        void OnPopup(wxCommandEvent &event);

		void OnChoiceSelected(wxCommandEvent& event);

		void ApplySavedSettings();

		void AddInstanceHeader(const wxString& h);

		bool GetCheckValue(const wxString& col);
		wxString GetChoiceValue(const wxString& col);
		int GetChoiceValueIndex(const wxString& col);

		void SetChoiceValue(const wxString& col, const std::string& value);
		void SetChoiceValueIndex(const wxString& col, int i);
		void SetCheckValue(const wxString& col, bool b);
		void SetDropDownItems(const wxString& col, const wxArrayString& items);

		bool CheckSlaveSizes(ControllerEthernet* controller, ControllerEthernet* slave1, ControllerEthernet* slave2);

		ControllerEthernet* getSlaveController(const std::string& name)
		{
			auto contrl = std::find_if(_otherControllers.begin(), _otherControllers.end(), [&name](auto po) {return po->GetName() == name; });
			if (contrl != _otherControllers.end())
				return *contrl;
			return nullptr;
		}

		DECLARE_EVENT_TABLE()
};

#endif
