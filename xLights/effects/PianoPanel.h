#ifndef PIANOPANEL_H
#define PIANOPANEL_H

//(*Headers(PianoPanel)
#include <wx/panel.h>
class wxSpinEvent;
class wxCheckBox;
class wxStaticText;
class wxBitmapButton;
class wxFlexGridSizer;
class wxSpinCtrl;
//*)

class PianoPanel: public wxPanel
{
	public:

		PianoPanel(wxWindow* parent);
		virtual ~PianoPanel();

        wxString defaultDir;
    
		//(*Declarations(PianoPanel)
		wxStaticText* StaticText_Piano_NumKeys;
		wxCheckBox* CheckBox_Piano_ShowSharps;
		wxSpinCtrl* SpinCtrl_Piano_StartMIDI;
		wxStaticText* StaticText1;
		wxStaticText* StaticText_Piano_NumRows;
		wxBitmapButton* BitmapButton_Piano_Style;
		wxSpinCtrl* SpinCtrl_Piano_EndMIDI;
		wxBitmapButton* BitmapButton_Piano_NumKeys;
		wxBitmapButton* BitmapButton_Piano_NumRows;
		//*)

	protected:

		//(*Identifiers(PianoPanel)
		static const long ID_STATICTEXT1;
		static const long ID_SPINCTRL_Piano_StartMIDI;
		static const long ID_BITMAPBUTTON_SLIDER_Piano_Style;
		static const long ID_STATICTEXT_Piano_NumKeys;
		static const long ID_SPINCTRL_Piano_EndMIDI;
		static const long ID_BITMAPBUTTON_Piano_NumKeys;
		static const long ID_CHECKBOX_Piano_ShowSharps;
		static const long ID_STATICTEXT_Piano_NumRows;
		static const long ID_BITMAPBUTTON_Piano_NumRows;
		//*)

	public:

		//(*Handlers(PianoPanel)
		void UpdateLinkedSliderFloat(wxCommandEvent& event);
		void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
		void UpdateLinkedTextCtrl360(wxScrollEvent& event);
		void UpdateLinkedSlider360(wxCommandEvent& event);
		void UpdateLinkedTextCtrl(wxScrollEvent& event);
		void UpdateLinkedSlider(wxCommandEvent& event);
		void OnLockButtonClick(wxCommandEvent& event);
		void OnPiano_StyleSelect(wxCommandEvent& event);
		void OnSlider_Piano_NumKeysCmdScroll(wxScrollEvent& event);
		void OnTextCtrl_Piano_FilenameText(wxCommandEvent& event);
		void OnButton_Piano_FilenameClick(wxCommandEvent& event);
		void OnSlider_Piano_KeyWidthCmdScroll(wxScrollEvent& event);
		void OnButton_Piano_CueFilenameClick(wxCommandEvent& event);
		void OnButton_Piano_MapFilenameClick(wxCommandEvent& event);
		void OnButton_Piano_ShapeFilenameClick(wxCommandEvent& event);
		void OnTextCtrl_Piano_CueFilenameText(wxCommandEvent& event);
		void OnTextCtrl_Piano_MapFilenameText(wxCommandEvent& event);
		void OnTextCtrl_Piano_ShapeFilenameText(wxCommandEvent& event);
		void OnSlider_Piano_NumRowsCmdScroll(wxScrollEvent& event);
		void OnSpinCtrl_Piano_StartMIDIChange(wxSpinEvent& event);
		void OnSpinCtrl_Piano_EndMIDIChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
