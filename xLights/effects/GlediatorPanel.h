#ifndef GLEDIATORPANEL_H
#define GLEDIATORPANEL_H

//(*Headers(GlediatorPanel)
#include <wx/panel.h>
class wxFilePickerCtrl;
class wxStaticText;
class wxFlexGridSizer;
class wxChoice;
//*)

class GlediatorPanel: public wxPanel
{
	public:

		GlediatorPanel(wxWindow* parent);
		virtual ~GlediatorPanel();

		//(*Declarations(GlediatorPanel)
		wxStaticText* StaticText2;
		wxStaticText* StaticText1;
		wxFilePickerCtrl* FilePickerCtrl_Glediator_Filename;
		wxChoice* Choice_Glediator_DurationTreatment;
		//*)
    
        wxString defaultDir;

	protected:

		//(*Identifiers(GlediatorPanel)
		static const long ID_STATICTEXT2;
		static const long ID_FILEPICKERCTRL_Glediator_Filename;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE_Glediator_DurationTreatment;
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
    void UpdateLinkedTextCtrlVC(wxScrollEvent& event);
    void UpdateLinkedTextCtrlFloatVC(wxScrollEvent& event);
    void OnVCButtonClick(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    void UpdateLinkedSliderFloat2(wxCommandEvent& event);
    void UpdateLinkedTextCtrlFloat2(wxScrollEvent& event);
    //*)

		DECLARE_EVENT_TABLE()
};

#endif
