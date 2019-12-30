#ifndef HINKSPIXEXPORTDIALOG_H
#define HINKSPIXEXPORTDIALOG_H

#include <list>
#include <map>
#include <array>

//(*Headers(HinksPixExportDialog)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/spinctrl.h>
#include <wx/splitter.h>
#include <wx/stattext.h>
//*)

#include "../FSEQFile.h"

class OutputManager;
class Output;

class HSEQFile : public V1FSEQFile {
public:
	HSEQFile(const std::string& fn, wxString ipAdress, uint32_t orgChannelCount) : V1FSEQFile(fn), _ipAdress(ipAdress), _orgChannelCount(orgChannelCount){
	};


	virtual void writeHeader() override;
private:
	wxString const _ipAdress;
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
		wxCheckListBox* CheckListBoxControllers;
		wxChoice* ChoiceFilter;
		wxChoice* ChoiceFolder;
		wxChoice* ChoiceSDCards;
		wxFlexGridSizer* FlexGridSizer5;
		wxListView* CheckListBox_Sequences;
		wxPanel* Panel1;
		wxPanel* Panel2;
		wxSpinCtrl* SpinCtrlEndHour;
		wxSpinCtrl* SpinCtrlEndMin;
		wxSpinCtrl* SpinCtrlStartHour;
		wxSpinCtrl* SpinCtrlStartMin;
		wxSplitterWindow* SplitterWindow1;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		//*)

	protected:

		//(*Identifiers(HinksPixExportDialog)
		static const long ID_STATICTEXT4;
		static const long ID_CHECKLISTBOX_CONTROLLERS;
		static const long ID_PANEL2;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_FILTER;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE_FOLDER;
		static const long ID_BITMAPBUTTON_MOVE_UP;
		static const long ID_BITMAPBUTTON_MOVE_DOWN;
		static const long ID_LISTVIEW_Sequences;
		static const long ID_PANEL1;
		static const long ID_SPLITTERWINDOW1;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE_SD_CARDS;
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

        OutputManager* _outputManager;

		std::vector <std::string> _hixControllers;

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
        void PopulateControllerList(wxString const& savedIPs);

        void GetFolderList(const wxString& folder);

		void moveSequenceItem(int to, int from, bool select = true);

		void createPlayList(std::vector<std::tuple<wxString, wxString>> const& songs, wxString const& drive);

		void createSchedule(wxString const& drive);

		bool Create_HinksPix_HSEQ_File(wxString const& fseqFile, wxString const& shortFSEQName, wxString const ipAddress, int const startChan, int const endChan, wxString& errorMsg);

		wxString createUniqueShortName(wxString const& fseqName, std::vector<wxString> const& names);

		bool Create_HinksPix_PCM_File(wxString const& inAudioFile, wxString& tmpPCMFile, uint32_t* Size_PCM_Bytes, uint32_t* Sample_Rate_Used, wxString& errorMsg);
		bool Make_AU_From_PCM(wxString const& PCM_File, wxString const& AU_File, uint32_t Size_PCM_Bytes, uint32_t Sample_Rate_Used, wxString& errorMsg);
        void OnPopup(wxCommandEvent &event);

		DECLARE_EVENT_TABLE()
};

#endif
