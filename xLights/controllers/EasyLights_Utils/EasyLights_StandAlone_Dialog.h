#ifndef EASYLIGHTS_STANDALONE_DIALOG_H
#define EASYLIGHTS_STANDALONE_DIALOG_H


#include <wx/progdlg.h>
#include <list>

//(*Headers(EasyLights_StandAlone_Dialog)
#include <wx/button.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

#include <wx/dataview.h>
#include "../FPP.h"

class EL_Controller_Info;
class EasyLights_EndPoint;

class EasyLights_StandAlone_Dialog: public wxDialog
{
	public:

		EasyLights_StandAlone_Dialog(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
		virtual ~EasyLights_StandAlone_Dialog();

		//(*Declarations(EasyLights_StandAlone_Dialog)
		wxButton* Button1;
		wxButton* Button2;
		wxButton* Button3;
		wxButton* Button4;
		wxButton* Button5;
		wxButton* Button6;
		wxButton* Button7;
		wxButton* Button8;
		wxListCtrl* ListCtrl1;
		wxListCtrl* ListCtrl2;
		wxStaticText* StaticText1;
		wxStaticText* StaticText2;
		//*)

		std::vector<EL_Controller_Info*> EL_Controller;
		std::vector<EasyLights_EndPoint*> EL_EndPoints;
		int EndPoints_Dirty;
		
		std::string ShowDirectory;
		std::string FseqDirectory;

		OutputManager *OutPutMgr;

		EasyLights_EndPoint *Selected_EndPoint;
		int Selected_EndPoint_Index;

		EasyLights_EndPoint *ELC_Master;	// can only be one for each
		EasyLights_EndPoint *ELC_Audio;


		void Display_SA_FSEQ_and_Audio();

	protected:

		//(*Identifiers(EasyLights_StandAlone_Dialog)
		static const long ID_STATICTEXT1;
		static const long ID_LISTCTRL1;
		static const long ID_STATICTEXT2;
		static const long ID_LISTCTRL2;
		static const long ID_BUTTON4;
		static const long ID_BUTTON6;
		static const long ID_BUTTON2;
		static const long ID_BUTTON3;
		static const long ID_BUTTON1;
		static const long ID_BUTTON5;
		static const long ID_BUTTON7;
		static const long ID_BUTTON8;
		//*)

	private:

		//(*Handlers(EasyLights_StandAlone_Dialog)
		void OnButton_ModifySlaves(wxCommandEvent& event);
		void OnButton_EasyLights_Controller(wxCommandEvent& event);
		void OnButton_fseq(wxCommandEvent& event);
		void OnButton_PlayList(wxCommandEvent& event);
		void OnButton_Schedule(wxCommandEvent& event);
		void OnButton_Upload_fseq(wxCommandEvent& event);
		void OnButton_Upload_PlayList(wxCommandEvent& event);
		void OnButtonCloseClick(wxCommandEvent& event);
		void OnButton_Controller_Management(wxCommandEvent& event);
		
		void Load_SA_List();
		void InitDialog(wxInitDialogEvent &event);


		void Initialize_EndPoint_View();
		//*)

	protected:

		void BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size);

		DECLARE_EVENT_TABLE()
};

#endif
