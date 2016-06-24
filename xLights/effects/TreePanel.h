#ifndef TREEPANEL_H
#define TREEPANEL_H

//(*Headers(TreePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxStaticText;
class wxSlider;
class wxBitmapButton;
class wxFlexGridSizer;
//*)

class TreePanel: public wxPanel
{
	public:

		TreePanel(wxWindow* parent);
		virtual ~TreePanel();

		//(*Declarations(TreePanel)
		wxSlider* Slider_Tree_Branches;
		wxBitmapButton* BitmapButton_TreeBranches;
		wxStaticText* StaticText86;
		//*)

	protected:

		//(*Identifiers(TreePanel)
		static const long ID_STATICTEXT87;
		static const long ID_SLIDER_Tree_Branches;
		static const long IDD_TEXTCTRL_Tree_Branches;
		static const long ID_BITMAPBUTTON_SLIDER_Tree_Branches;
		static const long ID_SLIDER_Tree_Speed;
		static const long IDD_TEXTCTRL_Tree_Speed;
		//*)

	public:

		//(*Handlers(TreePanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
