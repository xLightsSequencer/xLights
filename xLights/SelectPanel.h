#ifndef SELECTPANEL_H
#define SELECTPANEL_H

//(*Headers(SelectPanel)
#include <wx/button.h>
#include <wx/combobox.h>
#include <wx/listbox.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class MainSequencer;
class SequenceElements;
class Effect;
class Element;

class SelectPanel: public wxPanel
{
	public:

		SelectPanel(SequenceElements* elements, MainSequencer* sequencer, wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SelectPanel();

        void ReloadModels();

		//(*Declarations(SelectPanel)
		wxButton* Button_Select_All_Time;
		wxButton* Button_Select_Effect_All;
		wxButton* Button_Select_Model_All;
		wxButton* Button_Select_Refresh;
		wxButton* Button_Select_Search;
		wxComboBox* ComboBox_Select_Models;
		wxListBox* ListBox_Select_Found;
		wxListBox* ListBox_Select_Types;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxStaticText* StaticText6;
		wxTextCtrl* TextCtrl_Select_EndTime;
		wxTextCtrl* TextCtrl_Select_StartTime;
		//*)

	protected:

		//(*Identifiers(SelectPanel)
		static const long ID_STATICTEXT1;
		static const long ID_COMBOBOX_SELECT_MODELS;
		static const long ID_BUTTON_SELECT_SEARCH;
		static const long ID_STATICTEXT2;
		static const long ID_LISTBOX_SELECT_FOUND;
		static const long ID_BUTTON_SELECT_MODEL_ALL;
		static const long ID_STATICTEXT6;
		static const long ID_STATICTEXT3;
		static const long ID_TEXTCTRL_SELECT_STARTTIME;
		static const long ID_STATICTEXT5;
		static const long ID_TEXTCTRL_SELECT_ENDTIME;
		static const long ID_BUTTON_SELECT_ALL_TIME;
		static const long ID_STATICTEXT4;
		static const long ID_LISTBOX_SELECT_TYPES;
		static const long ID_BUTTON_SELECT_EFFECT_ALL;
		static const long ID_BUTTON_SELECT_REFRESH;
		//*)

	private:

        SequenceElements * mSequenceElements;
        MainSequencer* mainSequencer;

        void populateModelsList(const std::string& elementName);
        void populateEffectsList();
        void SelectEffects();

        std::vector< Element * > GetElements(const std::string& elementName);

        std::pair< int, int > GetStartAndEndTime();

		//(*Handlers(SelectPanel)
		void OnListBox_Select_TypesSelect(wxCommandEvent& event);
		void OnButton_Select_RefreshClick(wxCommandEvent& event);
		void OnComboBox_Select_ModelsTextEnter(wxCommandEvent& event);
		void OnListBox_Select_FoundSelect(wxCommandEvent& event);
		void OnButton_Select_SearchClick(wxCommandEvent& event);
		void OnButton_Select_Model_AllClick(wxCommandEvent& event);
		void OnButton_Select_Effect_AllClick(wxCommandEvent& event);
		void OnTextCtrl_Select_StartTimeText(wxCommandEvent& event);
		void OnTextCtrl_Select_EndTimeText(wxCommandEvent& event);
		void OnButton_Select_All_TimeClick(wxCommandEvent& event);
		void OnComboBox_Select_ModelsSelected(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
