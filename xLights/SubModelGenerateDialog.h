#ifndef SUBMODELGENERATEDIALOG_H
#define SUBMODELGENERATEDIALOG_H

//(*Headers(SubModelGenerateDialog)
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/dialog.h>
//*)

class SubModelGenerateDialog: public wxDialog
{
    int _modelWidth;
    int _modelHeight;
    int _modelNodes;
    int _lastCount;

    int SetSpinValue(int step, bool down);
    void ValidateWindow();

	public:

		SubModelGenerateDialog(wxWindow* parent, int modelWidth, int modelHeight, int modelNodes, wxWindowID id=wxID_ANY,const wxPoint& pos=wxDefaultPosition,const wxSize& size=wxDefaultSize);
		virtual ~SubModelGenerateDialog();

		//(*Declarations(SubModelGenerateDialog)
		wxStaticText* StaticText2;
		wxSpinCtrl* SpinCtrl_Count;
		wxStaticText* StaticText1;
		wxStaticText* StaticText3;
		wxTextCtrl* TextCtrl_BaseName;
		wxChoice* Choice1;
		//*)

        int GetCount() const;
        wxString GetBaseName() const;
        wxString GetType() const;

	protected:

		//(*Identifiers(SubModelGenerateDialog)
		static const long ID_STATICTEXT1;
		static const long ID_TEXTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_CHOICE1;
		static const long ID_STATICTEXT3;
		static const long ID_SPINCTRL1;
		//*)

	private:

		//(*Handlers(SubModelGenerateDialog)
		void OnChoice1Select(wxCommandEvent& event);
		void OnSpinCtrl_CountChange(wxSpinEvent& event);
		//*)

		DECLARE_EVENT_TABLE()
};

#endif
