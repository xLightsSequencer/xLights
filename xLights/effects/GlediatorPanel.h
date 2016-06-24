#ifndef GLEDIATORPANEL_H
#define GLEDIATORPANEL_H

//(*Headers(GlediatorPanel)
#include <wx/panel.h>
class wxTextCtrl;
class wxFlexGridSizer;
class wxBitmapButton;
class wxButton;
//*)

class GlediatorPanel: public wxPanel
{
	public:

		GlediatorPanel(wxWindow* parent);
		virtual ~GlediatorPanel();

		//(*Declarations(GlediatorPanel)
		wxBitmapButton* BitmapButton_Glediator_Filename;
		wxButton* ButtonGledFile;
		wxTextCtrl* TextCtrl_Glediator_Filename;
		//*)
    
        wxString defaultDir;

	protected:

		//(*Identifiers(GlediatorPanel)
		static const long ID_BUTTON_GLED_FILE;
		static const long ID_BITMAPBUTTON10;
		static const long ID_TEXTCTRL_Glediator_Filename;
		//*)

	public:

		//(*Handlers(GlediatorPanel)
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void OnButton_Glediator_FilenameClick(wxCommandEvent& event);
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
