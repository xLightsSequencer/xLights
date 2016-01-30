#ifndef TREEPANEL_H
#define TREEPANEL_H

//(*Headers(TreePanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxSlider;
class wxStaticText;
//*)

class TreePanel: public wxPanel
{
	public:

		TreePanel(wxWindow* parent);
		virtual ~TreePanel();

		//(*Declarations(TreePanel)
		wxStaticText* StaticText86;
		wxBitmapButton* BitmapButton_TreeBranches;
		wxSlider* Slider_Tree_Branches;
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

		//*)

		DECLARE_EVENT_TABLE()
};

#endif
