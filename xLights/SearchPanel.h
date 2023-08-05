#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

//(*Headers(SearchPanel)
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include <vector>

class MainSequencer;
class SequenceElements;
class Effect;
class Element;

class SearchPanel: public wxPanel
{
	public:

		SearchPanel(SequenceElements* elements, MainSequencer* sequencer, wxWindow* parent,wxWindowID id=wxID_ANY);
		virtual ~SearchPanel();

		void ClearData();

		//(*Declarations(SearchPanel)
		wxButton* ButtonSelectAll;
		wxButton* Button_Search_Find;
		wxCheckBox* CheckBox_Search_Regex;
		wxComboBox* ComboBox_Search_Model;
		wxListCtrl* ListCtrl_Results;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText_Count;
		wxTextCtrl* TextCtrlSearch;
		//*)

	protected:

		//(*Identifiers(SearchPanel)
		static const long ID_STATICTEXT1;
		static const long ID_COMBOBOX_SEARCH_MODEL;
		static const long ID_CHECKBOX_SEARCH_REGEX;
		static const long ID_STATICTEXT2;
		static const long ID_TEXTCTRL_SEARCH;
		static const long ID_BUTTON_SEARCH_FIND;
		static const long ID_LISTCTRL_Results;
		static const long ID_BUTTON_SELECT_ALL;
		static const long ID_STATICTEXT_COUNT;
		//*)

	private:

        SequenceElements * mSequenceElements;
        MainSequencer* mainSequencer;


		std::vector<wxString> GetModelList() const;
        void PopulateModelsList();
        void FindSettings();
        void SelectEffects();

		bool ContainsSetting(Effect* eff, std::string const& search, bool regex, std::string& value) const;

		//(*Handlers(SearchPanel)
		void OnButton_Search_FindClick(wxCommandEvent& event);
		void OnComboBox_Search_ModelDropdown(wxCommandEvent& event);
		void OnButtonSelectAllClick(wxCommandEvent& event);
		void OnTextCtrlSearchTextEnter(wxCommandEvent& event);
		void OnListBoxResultsSelect(wxCommandEvent& event);
		void OnListCtrl_ResultsItemSelect(wxListEvent& event);
		void OnListCtrl_ResultsItemActivated(wxListEvent& event);
		void OnListCtrl_ResultsItemFocused(wxListEvent& event);
		void OnListCtrl_ResultsItemDeselect(wxListEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};
