#ifndef MIDIASSOCIATEDIALOG_H
#define MIDIASSOCIATEDIALOG_H

//(*Headers(MIDIAssociateDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

class MIDIListener;

class MIDIAssociateDialog: public wxDialog
{
    std::list<MIDIListener*>& _midiListeners;

    void SetTempWindow(wxWindow* window);

    public:

		MIDIAssociateDialog(wxWindow* parent, std::list<MIDIListener*>& midiListeners, std::string controlName, int status, int channel, int data1, const std::string& midiDevice, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~MIDIAssociateDialog();

		//(*Declarations(MIDIAssociateDialog)
		wxButton* Button_Cancel;
		wxButton* Button_Ok;
		wxButton* Button_Scan;
		wxChoice* Choice_Channel;
		wxChoice* Choice_Data1;
		wxChoice* Choice_MIDIDevice;
		wxChoice* Choice_Status;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		wxStaticText* StaticText3;
		wxStaticText* StaticText4;
		wxStaticText* StaticText5;
		wxTextCtrl* TextCtrl_KeyCode;
		//*)

	protected:

		//(*Identifiers(MIDIAssociateDialog)
		static const long ID_STATICTEXT5;
		static const long ID_CHOICE4;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT1;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE3;
		static const long ID_BUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(MIDIAssociateDialog)
		void OnButton_CancelClick(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_ScanClick(wxCommandEvent& event);
		//*)

        void OnMIDIEvent(wxCommandEvent& event);

		DECLARE_EVENT_TABLE()
};

#endif
