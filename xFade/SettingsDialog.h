#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

//(*Headers(SettingsDialog)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <map>

class Settings;

class SettingsDialog: public wxDialog
{
    Settings* _settings;
    std::map<int, std::string> _targetIPCopy;
    std::map<int, std::string> _targetDescCopy;
    std::map<int, std::string> _targetProtocolCopy;
    std::string _localInputIPCopy;
    std::string _localOutputIPCopy;

    void ValidateWindow();
    void LoadUniverses();
    void AddUniverseRange(int low, int high, std::string ipAddress, std::string desc, std::string protocol);
    std::list<std::string> GetMIDIDevices();
    void Apply();
    void PopulateFields();

	public:

		SettingsDialog(wxWindow* parent, Settings* settings, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SettingsDialog();

		//(*Declarations(SettingsDialog)
		wxButton* ButtonAddFE;
		wxButton* Button_Add;
		wxButton* Button_Cancel;
		wxButton* Button_Delete;
		wxButton* Button_DeleteFE;
		wxButton* Button_Edit;
		wxButton* Button_EditFE;
		wxButton* Button_ExportSettings;
		wxButton* Button_ForceInput;
		wxButton* Button_ForceOutput;
		wxButton* Button_ImportSettings;
		wxButton* Button_Ok;
		wxCheckBox* CheckBox_ArtNET;
		wxCheckBox* CheckBox_E131;
		wxChoice* Choice1;
		wxChoice* Choice_FrameTiming;
		wxListView* ListViewFadeExclude;
		wxListView* ListView_Universes;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxStaticText* StaticText7;
		wxStaticText* StaticText_InputIP;
		wxStaticText* StaticText_OutputIP;
		//*)

	protected:

		//(*Identifiers(SettingsDialog)
		static const long ID_STATICTEXT5;
		static const long ID_CHECKBOX_E131;
		static const long ID_CHECKBOX_ARTNET;
		static const long ID_STATICTEXT7;
		static const long ID_STATICTEXT8;
		static const long ID_BUTTON9;
		static const long ID_STATICTEXT2;
		static const long ID_STATICTEXT3;
		static const long ID_BUTTON12;
		static const long ID_STATICTEXT9;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT4;
		static const long ID_LISTVIEW_UNIVERSES;
		static const long ID_BUTTON3;
		static const long ID_BUTTON4;
		static const long ID_BUTTON5;
		static const long ID_STATICTEXT6;
		static const long ID_LISTVIEW1;
		static const long ID_BUTTON8;
		static const long ID_BUTTON10;
		static const long ID_BUTTON11;
		static const long ID_BUTTON6;
		static const long ID_BUTTON7;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(SettingsDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnListView_UniversesItemSelect(wxListEvent& event);
		void OnListView_UniversesItemActivated(wxListEvent& event);
		void OnButton_ForceInputClick(wxCommandEvent& event);
		void OnButton_ForceOutputClick(wxCommandEvent& event);
		void OnChoice_FrameTimingSelect(wxCommandEvent& event);
		void OnCheckBox_E131Click(wxCommandEvent& event);
		void OnCheckBox_ArtNETClick(wxCommandEvent& event);
		void OnButton_AddClick(wxCommandEvent& event);
		void OnButton_EditClick(wxCommandEvent& event);
		void OnButton_DeleteClick(wxCommandEvent& event);
		void OnButton_ImportSettingsClick(wxCommandEvent& event);
		void OnButton_ExportSettingsClick(wxCommandEvent& event);
		void OnButtonAddFEClick(wxCommandEvent& event);
		void OnButton_EditFEClick(wxCommandEvent& event);
		void OnButton_DeleteFEClick(wxCommandEvent& event);
		void OnListViewFadeExcludeItemSelect(wxListEvent& event);
		void OnListViewFadeExcludeItemActivated(wxListEvent& event);
        void LoadFadeExclude();
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
