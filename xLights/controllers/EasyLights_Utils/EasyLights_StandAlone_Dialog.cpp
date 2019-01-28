

//(*InternalHeaders(EasyLights_StandAlone_Dialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)


#include <wx/regex.h>
#include <wx/volume.h>
#include <wx/progdlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include "../../xLightsMain.h"
#include "../FPP.h"
#include "../../xLightsXmlFile.h"
#include "../../outputs/Output.h"
#include "../../outputs/OutputManager.h"
#include "../../UtilFunctions.h"

#include <log4cpp/Category.hh>
#include "../../../xSchedule/wxJSON/jsonreader.h"

#include "../../../include/spxml-0.5/spxmlparser.hpp"
#include "../../../include/spxml-0.5/spxmlevent.hpp"
#include "../../FSEQFile.h"
#include "../../Parallel.h"

#include "EasyLights_Data.h"
#include "EasyLights_StandAlone_Dialog.h"




//(*IdInit(EasyLights_StandAlone_Dialog)
const long EasyLights_StandAlone_Dialog::ID_STATICTEXT1 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_LISTCTRL1 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_STATICTEXT2 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_LISTCTRL2 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON4 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON6 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON2 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON3 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON1 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON5 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON7 = wxNewId();
const long EasyLights_StandAlone_Dialog::ID_BUTTON8 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EasyLights_StandAlone_Dialog,wxDialog)
	//(*EventTable(EasyLights_StandAlone_Dialog)
	EVT_INIT_DIALOG(EasyLights_StandAlone_Dialog::InitDialog)
	//*)
END_EVENT_TABLE()

EasyLights_StandAlone_Dialog *EL_Ptr = 0;

//EasyLights_StandAlone_Dialog::EasyLights_StandAlone_Dialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
EasyLights_StandAlone_Dialog::EasyLights_StandAlone_Dialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
	// init class data

	EndPoints_Dirty = 0;
	ELC_Master = 0;
	ELC_Audio = 0;
	EL_Ptr = this;

	Selected_EndPoint = 0;
	Selected_EndPoint_Index = 0;


	BuildContent(parent,id,pos,size);

	Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_ModifySlaves);
	Connect(ID_BUTTON6, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_fseq);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_PlayList);
	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_Upload_fseq);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_EasyLights_Controller);
	Connect(ID_BUTTON7, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_Schedule);
	Connect(ID_BUTTON8, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_Upload_PlayList);
	Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButton_Controller_Management);

	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButtonCloseClick);
	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&EasyLights_StandAlone_Dialog::OnButtonCloseClick);


	/***
	EVT_COMMAND(wxID_OK, wxEVT_BUTTON, wxMyDialog::onOkPressed)
		EVT_COMMAND(wxID_CANCEL, wxEVT_BUTTON, wxMyDialog::onCancelPressed)

		plus a close event handler
		EVT_CLOSE(wxMyDialog::onClosePressed)
***/

}

void EasyLights_StandAlone_Dialog::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{

	//(*Initialize(EasyLights_StandAlone_Dialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxGridSizer* GridSizer1;

	Create(parent, wxID_ANY, _("EasyLights StandAlone"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	Move(wxPoint(-1,-1));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("EasyLights Controllers and Attachments"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl1 = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxSize(850,250), wxLC_REPORT|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	FlexGridSizer1->Add(ListCtrl1, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Sequences"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl2 = new wxListCtrl(this, ID_LISTCTRL2, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL2"));
	ListCtrl2->SetMinSize(wxSize(400,200));
	FlexGridSizer1->Add(ListCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GridSizer1 = new wxGridSizer(2, 4, 0, 0);
	Button4 = new wxButton(this, ID_BUTTON4, _("Add / Modify Attachment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	GridSizer1->Add(Button4, 1, wxALL|wxEXPAND, 5);
	Button6 = new wxButton(this, ID_BUTTON6, _("Generate FSEQ"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	GridSizer1->Add(Button6, 1, wxALL|wxEXPAND, 5);
	Button2 = new wxButton(this, ID_BUTTON2, _("PlayList"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	GridSizer1->Add(Button2, 1, wxALL|wxEXPAND, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("Upload Sequence"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	GridSizer1->Add(Button3, 1, wxALL|wxEXPAND, 5);
	Button1 = new wxButton(this, ID_BUTTON1, _("EasyLights Controllers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	GridSizer1->Add(Button1, 1, wxALL|wxEXPAND, 5);
	Button5 = new wxButton(this, ID_BUTTON5, _("Controller Management"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	GridSizer1->Add(Button5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button7 = new wxButton(this, ID_BUTTON7, _("Schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	GridSizer1->Add(Button7, 1, wxALL|wxEXPAND, 5);
	Button8 = new wxButton(this, ID_BUTTON8, _("Upload Schedule Playlist"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON8"));
	GridSizer1->Add(Button8, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(GridSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();
	//*)
}

void EasyLights_StandAlone_Dialog::InitDialog(wxInitDialogEvent &WXUNUSED(event))
{

	Get_EasyLights_Specifications(ShowDirectory);
	Build_EL_EndPoint_Array(ShowDirectory);
	EndPoints_Dirty = Add_IPs_From_Output_To_EndPoint_Array();

	Initialize_EndPoint_View();

	Load_SA_List();
	Display_SA_FSEQ_and_Audio();

}


EasyLights_StandAlone_Dialog::~EasyLights_StandAlone_Dialog()
{
	//(*Destroy(EasyLights_StandAlone_Dialog)
	//*)
}



void EasyLights_StandAlone_Dialog::OnButton_ModifySlaves(wxCommandEvent& event)    // add - Modify Slave
{
	EasyLights_Add_Slaves_Dialog dlg(this);

	if(ListCtrl1->GetSelectedItemCount() == 0)
		return;

	if(ListCtrl1->GetSelectedItemCount() > 1)
	{
		wxMessageBox(wxT("Only ONE Selection Allowed for this Task"), wxT("Selection Error"), wxOK, NULL);

		return;
	}

	Selected_EndPoint_Index = ListCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(Selected_EndPoint_Index < 0)
		return;

	Selected_EndPoint = EL_EndPoints[Selected_EndPoint_Index];

	if(Selected_EndPoint->Model.size() == 0 || Selected_EndPoint->Model == "Undefined")
	{
		wxMessageBox(wxT("Action Requires Selection of an EasyLights Controller - Not an Attachment"), wxT("Selection Error"), wxOK, NULL);

		return;
	}



	dlg.EL_EndPoint = Selected_EndPoint;

	dlg.ShowModal();

	if(dlg.Dirty)
	{
		EndPoints_Dirty = 1;

		ListCtrl1->DeleteAllItems();

		Load_SA_List();
	}

}

void EasyLights_StandAlone_Dialog::OnButton_EasyLights_Controller(wxCommandEvent& event)    // easylights controller
{
	EasyLights_Identify_Dialog dlg(this);

	dlg.ShowModal();

	if(dlg.Dirty)
	{
		EndPoints_Dirty = 1;

		ListCtrl1->DeleteAllItems();

		Load_SA_List();
	}

}



void EasyLights_StandAlone_Dialog::OnButton_fseq(wxCommandEvent& event)    // generate fseq
{
	EasyLights_FSEQ_Dialog dlg(this);

	dlg.ShowModal();

	Display_SA_FSEQ_and_Audio();
	
}

void EasyLights_StandAlone_Dialog::OnButton_PlayList(wxCommandEvent& event)    // playlist
{
	
	EasyLights_PlayList_Dialog dlg(this);

	dlg.ShowModal();

}

void EasyLights_StandAlone_Dialog::OnButton_Schedule(wxCommandEvent& event)    // schedule
{

	EasyLights_Schedule_Dialog dlg(this);

	dlg.ShowModal();


	
}

void EasyLights_StandAlone_Dialog::OnButton_Upload_fseq(wxCommandEvent& event)    // upload sequence
{

}

void EasyLights_StandAlone_Dialog::OnButton_Upload_PlayList(wxCommandEvent& event)    // upload schedule playlist
{

}

void EasyLights_StandAlone_Dialog::OnButton_Controller_Management(wxCommandEvent& event)
{

}


void EasyLights_StandAlone_Dialog::OnButtonCloseClick(wxCommandEvent& event)
{
	if(EndPoints_Dirty)
		Save_EndPoint_Array_to_XML(ShowDirectory);

	EL_Ptr = 0;

// release memory

	for(auto e = EL_EndPoints.begin(); e != EL_EndPoints.end(); ++e)
		delete *e;

	for(auto x = EL_Controller.begin(); x != EL_Controller.end(); ++x)
		delete *x;

	EL_EndPoints.clear();
	EL_Controller.clear();

	EndDialog(0);
}

void EasyLights_StandAlone_Dialog::Initialize_EndPoint_View()
{
	// view has 5 columns

	wxListItem itemCol;

	itemCol.SetText(wxT("Name"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(0, itemCol);
	ListCtrl1->SetColumnWidth(0, 200);

	itemCol.SetText(wxT("IP"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(1, itemCol);
	ListCtrl1->SetColumnWidth(1, 100);

	itemCol.SetText(wxT("Type"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(2, itemCol);
	ListCtrl1->SetColumnWidth(2, 150);

	itemCol.SetText(wxT("Master"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(3, itemCol);
	ListCtrl1->SetColumnWidth(3, 75);

	itemCol.SetText(wxT("Audio"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(4, itemCol);
	ListCtrl1->SetColumnWidth(4, 75);


	itemCol.SetText(wxT("Attachment 1 IP"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(5, itemCol);
	ListCtrl1->SetColumnWidth(5, 100);

	itemCol.SetText(wxT("Attachment 2 IP"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(6, itemCol);
	ListCtrl1->SetColumnWidth(6, 100);


	itemCol.SetText(wxT("Name"));
	itemCol.SetImage(-1);
	ListCtrl2->InsertColumn(0, itemCol);
	ListCtrl2->SetColumnWidth(0, 400);


	
}

void EasyLights_StandAlone_Dialog::Load_SA_List()
{
	wxString T;
	int i = 0;
	wxColor CG(150, 255, 150);
	wxColor CB(200, 200, 255);
	int V;

	ELC_Master = 0;
	ELC_Audio = 0;
	
	for(auto it = EL_EndPoints.begin(); it != EL_EndPoints.end(); ++it, i++)
	{
		if((*it)->Master)
			ELC_Master = *it;

		if((*it)->Audio)
			ELC_Audio = *it;


		T.Printf("%s", (*it)->Name);
		ListCtrl1->InsertItem(i, T, -1);
		ListCtrl1->SetItemData(i, i);

		T.Printf("%s", (*it)->IP);
		ListCtrl1->SetItem(i, 1, T);

		if((*it)->Model.size() == 0)
			(*it)->Model = "Undefined";

		if((*it)->Model == "Undefined" && (*it)->Slave_Attached)
			T.Printf("%s", "Attached");
		else
			T.Printf("%s", (*it)->Model);

		ListCtrl1->SetItem(i, 2, T);
		if((*it)->Model != "Undefined")
		{
			V = ELC_Status_To_Support_Slaves(*it);
			if(V == ELC_Support_NO_Slaves)
				ListCtrl1->SetItemBackgroundColour(i, CB);
			else
				ListCtrl1->SetItemBackgroundColour(i, CG);
		}

		if((*it)->Master)
			T = "Yes";
		else
			T = "No";
		ListCtrl1->SetItem(i, 3, T);

		if((*it)->Audio)
			T = "Yes";
		else
			T = "No";
		ListCtrl1->SetItem(i, 4, T);

		T.Printf("%s", (*it)->S1_IP);
		ListCtrl1->SetItem(i, 5, T);

		T.Printf("%s", (*it)->S2_IP);
		ListCtrl1->SetItem(i, 6, T);

	}
}

void EasyLights_StandAlone_Dialog::Display_SA_FSEQ_and_Audio()
{
	ListCtrl2->DeleteAllItems();

	std::string FN_search;
	std::string FN;
	wxString T;
	int i = 0;
	wxFileName wxFN;



	FN_search = ShowDirectory + "\\EasyLightsData\\*.fseq";

	FN = wxFindFirstFile(FN_search);

	while(!FN.empty())
	{
		wxFN = FN;

		ListCtrl2->InsertItem(i, T, -1);


		T = wxString::Format("%s.%s", wxFN.GetName(), wxFN.GetExt());
		ListCtrl2->SetItem(i, 0, T);

		i++;
		FN = wxFindNextFile();
	}

	/***
	FN_search = showDirectory + "\\EasyLightsData\\*.wav";

	FN = wxFindFirstFile(FN_search);

	while(!FN.empty())
	{
		wxFN = FN;

		ListCtrl2->InsertItem(i, T, -1);


		T = wxString::Format("%s.%s", wxFN.GetName(), wxFN.GetExt());
		ListCtrl2->SetItem(i, 0, T);

		i++;
		FN = wxFindNextFile();
	}
	**/

}


