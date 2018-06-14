#ifndef TREEPANEL_H
#define TREEPANEL_H

//(*Headers(TreePanel)
#include <wx/panel.h>
class wxBitmapButton;
class wxCheckBox;
class wxFlexGridSizer;
class wxSlider;
class wxStaticText;
class wxTextCtrl;
//*)

#include "../BulkEditControls.h"

class TreePanel: public wxPanel
{
	public:

		TreePanel(wxWindow* parent);
		virtual ~TreePanel();

		//(*Declarations(TreePanel)
		BulkEditCheckBox* CheckBox1;
		BulkEditSlider* Slider_Tree_Branches;
		BulkEditSlider* Slider_Tree_Speed;
		wxStaticText* StaticText182;
		wxStaticText* StaticText86;
		xlLockButton* BitmapButton_TreeBranches;
		//*)

	protected:

		//(*Identifiers(TreePanel)
		static const long ID_STATICTEXT_Tree_Branches;
		static const long ID_SLIDER_Tree_Branches;
		static const long IDD_TEXTCTRL_Tree_Branches;
		static const long ID_BITMAPBUTTON_SLIDER_Tree_Branches;
		static const long ID_STATICTEXT_Tree_Speed;
		static const long ID_SLIDER_Tree_Speed;
		static const long IDD_TEXTCTRL_Tree_Speed;
		static const long ID_CHECKBOX_Tree_ShowLights;
		//*)

	public:

		//(*Handlers(TreePanel)
    void OnLockButtonClick(wxCommandEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
