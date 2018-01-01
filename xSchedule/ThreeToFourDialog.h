#ifndef THREETOFOURDIALOG_H
#define THREETOFOURDIALOG_H

//(*Headers(ThreeToFourDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/button.h>
#include <wx/dialog.h>
//*)

class OutputManager;

class ThreeToFourDialog: public wxDialog
{
    std::string& _startChannel;
    size_t& _nodes;
    std::string& _colourOrder;
    std::string& _description;
    bool& _enabled;
    OutputManager* _outputManager;

    void ValidateWindow();

    public:

		ThreeToFourDialog(wxWindow* parent, OutputManager* outputManager, std::string& startChannel, size_t& channels, std::string& colourOrder, std::string& description, bool& enabled, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~ThreeToFourDialog();

		//(*Declarations(ThreeToFourDialog)
		wxStaticText* StaticText_StartChannel;
		wxCheckBox* CheckBox_Enabled;
		wxButton* Button_Ok;
		wxTextCtrl* TextCtrl_Description;
		wxChoice* Choice_ColourOrder;
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Nodes;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxButton* Button_Cancel;
		wxTextCtrl* TextCtrl_StartChannel;
		wxStaticText* StaticText4;
		//*)

	protected:

		//(*Identifiers(ThreeToFourDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL2;
		static const long ID_STATICTEXT5;
		static const long ID_STATICTEXT2;
		static const long ID_SPINCTRL2;
		static const long ID_STATICTEXT3;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT4;
		static const long ID_TEXTCTRL1;
		static const long ID_CHECKBOX1;
		static const long ID_BUTTON1;
		static const long ID_BUTTON2;
		//*)

	private:

		//(*Handlers(ThreeToFourDialog)
		void OnTextCtrl_StartChannelText(wxCommandEvent& event);
		void OnButton_OkClick(wxCommandEvent& event);
		void OnButton_CancelClick(wxCommandEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
