

#include <wx/msgdlg.h>
#include <wx/regex.h>
#include <wx/xml/xml.h>
#include <wx/progdlg.h>

#include <wx/intl.h>
#include <wx/string.h>


#include "../../models/Model.h"
#include "../../outputs/OutputManager.h"
#include "../../outputs/Output.h"
#include "../../models/ModelManager.h"


#include <log4cpp/Category.hh>
#include "UtilFunctions.h"
#include <wx/filename.h>

#include <wx/dir.h>

#include "../../models/Model.h"
#include "../../outputs/OutputManager.h"
#include "../../outputs/Output.h"
#include "../../models/ModelManager.h"

#include "EasyLights_Add_Slaves_Dialog.h"
#include "EasyLights_Identify_Dialog.h"
#include "../EasyLights.h"
#include "EasyLights_Data.h"
#include <wx/textctrl.h>



//(*IdInit(EasyLights_Add_Slaves_Dialog)
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT1 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_LISTCTRL1 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT2 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT3 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL1 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT4 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL2 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT5 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL3 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT6 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL4 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT7 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL5 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT16 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL12 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT8 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT9 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL6 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT12 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL8 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT14 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL10 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_BUTTON1 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_BUTTON2 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT10 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT11 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL7 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT13 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL9 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_STATICTEXT15 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_TEXTCTRL11 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_BUTTON3 = wxNewId();
const long EasyLights_Add_Slaves_Dialog::ID_BUTTON4 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EasyLights_Add_Slaves_Dialog,wxDialog)
	//(*EventTable(EasyLights_Add_Slaves_Dialog)
	EVT_INIT_DIALOG(EasyLights_Add_Slaves_Dialog::InitDialog)
	//*)
END_EVENT_TABLE()

EasyLights_Add_Slaves_Dialog::EasyLights_Add_Slaves_Dialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	Selected_EndPoint = 0;
	Dirty = 0;
	EL_EndPoint = 0;
	Channels_Avail_To_Slave = 0;
	S1_EndPoint = 0;
	S2_EndPoint = 0;
	S1_Selected_EndPoint_Index = 0;
	S2_Selected_EndPoint_Index = 0;




	BuildContent(parent,id,pos,size);


	wxListItem itemCol;

	itemCol.SetText(wxT("Name"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(0, itemCol);
	ListCtrl1->SetColumnWidth(0, 200);

	itemCol.SetText(wxT("IP"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(1, itemCol);
	ListCtrl1->SetColumnWidth(1, 100);

	itemCol.SetText(wxT("Pixels"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(2, itemCol);
	ListCtrl1->SetColumnWidth(2, 100);


	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Add_Slaves_Dialog::OnButton_S1_Add);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Add_Slaves_Dialog::OnButton_S1_Remove);
	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Add_Slaves_Dialog::OnButton_S2_Add);
	Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Add_Slaves_Dialog::OnButton_S2_Remove);

	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Add_Slaves_Dialog::OnButtonCloseClick);
	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&EasyLights_Add_Slaves_Dialog::OnButtonCloseClick);


}

void EasyLights_Add_Slaves_Dialog::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EasyLights_Add_Slaves_Dialog)
	wxFlexGridSizer* FlexGridSizer10;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer9;

	Create(parent, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("E131 Controllers that May be Set as Slaves to the Selected EasyLights Controller"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl1 = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxSize(400,250), wxLC_REPORT|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	FlexGridSizer1->Add(ListCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Selected EasyLights Controller"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer5->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer5->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer5->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl2 = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(200,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer5->Add(TextCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Model"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer5->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl3 = new wxTextCtrl(this, ID_TEXTCTRL3, wxEmptyString, wxDefaultPosition, wxSize(150,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL3"));
	FlexGridSizer5->Add(TextCtrl3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Number of Pixels Used"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer5->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl4 = new wxTextCtrl(this, ID_TEXTCTRL4, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer5->Add(TextCtrl4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("Pixels Available to Slaves"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer5->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl5 = new wxTextCtrl(this, ID_TEXTCTRL5, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL5"));
	FlexGridSizer5->Add(TextCtrl5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText16 = new wxStaticText(this, ID_STATICTEXT16, _("Status"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT16"));
	FlexGridSizer5->Add(StaticText16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl12 = new wxTextCtrl(this, ID_TEXTCTRL12, wxEmptyString, wxDefaultPosition, wxSize(200,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL12"));
	FlexGridSizer5->Add(TextCtrl12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText8 = new wxStaticText(this, ID_STATICTEXT8, _("Attachment 1"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer6->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText9 = new wxStaticText(this, ID_STATICTEXT9, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer7->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl6 = new wxTextCtrl(this, ID_TEXTCTRL6, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL6"));
	FlexGridSizer7->Add(TextCtrl6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText12 = new wxStaticText(this, ID_STATICTEXT12, _("Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
	FlexGridSizer7->Add(StaticText12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl8 = new wxTextCtrl(this, ID_TEXTCTRL8, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL8"));
	FlexGridSizer7->Add(TextCtrl8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText14 = new wxStaticText(this, ID_STATICTEXT14, _("Pixels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT14"));
	FlexGridSizer7->Add(StaticText14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl10 = new wxTextCtrl(this, ID_TEXTCTRL10, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL10"));
	FlexGridSizer7->Add(TextCtrl10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer7->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button1 = new wxButton(this, ID_BUTTON1, _("Add Selected"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer7->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button2 = new wxButton(this, ID_BUTTON2, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer7->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6->Add(FlexGridSizer7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer9 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText10 = new wxStaticText(this, ID_STATICTEXT10, _("Attachment 2"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer9->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText11 = new wxStaticText(this, ID_STATICTEXT11, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer10->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl7 = new wxTextCtrl(this, ID_TEXTCTRL7, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL7"));
	FlexGridSizer10->Add(TextCtrl7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText13 = new wxStaticText(this, ID_STATICTEXT13, _("Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
	FlexGridSizer10->Add(StaticText13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl9 = new wxTextCtrl(this, ID_TEXTCTRL9, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL9"));
	FlexGridSizer10->Add(TextCtrl9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText15 = new wxStaticText(this, ID_STATICTEXT15, _("Pixels"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT15"));
	FlexGridSizer10->Add(StaticText15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl11 = new wxTextCtrl(this, ID_TEXTCTRL11, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL11"));
	FlexGridSizer10->Add(TextCtrl11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer10->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("Add Selected"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer10->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button4 = new wxButton(this, ID_BUTTON4, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer10->Add(Button4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer9->Add(FlexGridSizer10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8->Add(FlexGridSizer9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

EasyLights_Add_Slaves_Dialog::~EasyLights_Add_Slaves_Dialog()
{
	//(*Destroy(EasyLights_Add_Slaves_Dialog)
	//*)
}

void EasyLights_Add_Slaves_Dialog::InitDialog(wxInitDialogEvent &WXUNUSED(event))
{
	Load_S_List(0);

	TransferDataToWindow();
}


void EasyLights_Add_Slaves_Dialog::Load_S_List(int Ignore_Slaves_Attached)
{
	wxString T;
	int i = 0;
	int ListIndex = 0;



	for(auto it = EL_Ptr->EL_EndPoints.begin(); it != EL_Ptr->EL_EndPoints.end(); ++it, i++)
	{
		if(((*it)->Model.size() == 0 || (*it)->Model == "Undefined") && (*it)->DDP_Network == 0)	// no slave DDP
		{
			if(Ignore_Slaves_Attached || (*it)->Slave_Attached == 0)
			{
				T.Printf("%s", (*it)->Name);
				ListCtrl1->InsertItem(ListIndex, T, -1);
				ListCtrl1->SetItemData(ListIndex, i);

				T.Printf("%s", (*it)->IP);
				ListCtrl1->SetItem(ListIndex, 1, T);

				T.Printf("%d", (*it)->Channels_Used / 3);
				ListCtrl1->SetItem(ListIndex, 2, T);

				ListIndex++;
			}
		}
	}
}



bool EasyLights_Add_Slaves_Dialog::TransferDataToWindow()
{
	wxString T;

	wxColour CR(255, 150, 150);
	wxColour CG(150, 255, 150);

	int V;

	Button1->Enable(true);
	Button3->Enable(true);

	V = ELC_Status_To_Support_Slaves(EL_EndPoint);
	if(V == ELC_Support_NO_Slaves)
	{
		Channels_Avail_To_Slave = 0;
		Button1->Enable(false);
		Button3->Enable(false);
	}
	else if(V == ELC_Support_One_Slave)
	{
		Channels_Avail_To_Slave = (510 * 16) / 3;
		//if(EL_EndPoint->S1_IP.size() > 3)
		//	Button3->Enable(false);
		//else if(EL_EndPoint->S2_IP.size() > 3)
		//	Button1->Enable(false);
	}
	else
		Channels_Avail_To_Slave = (510 * 32) / 3;






	T = EL_EndPoint->IP;
	TextCtrl1->SetValue(T);

	T = EL_EndPoint->Name;
	TextCtrl2->SetValue(T);

	T = EL_EndPoint->Model;
	TextCtrl3->SetValue(T);


	T = wxString::Format("%d", EL_EndPoint->Channels_Used / 3);
	TextCtrl4->SetValue(T);

// now check for any slaves

	if(EL_EndPoint->S1_IP.size() > 1)
	{
		S1_EndPoint = Find_EndPoint_By_IP(EL_EndPoint->S1_IP);
		if(S1_EndPoint)
		{
			T = S1_EndPoint->IP;
			TextCtrl6->SetValue(T);

			T = S1_EndPoint->Name;
			TextCtrl8->SetValue(T);

			T = wxString::Format("%d", S1_EndPoint->Channels_Used / 3);
			TextCtrl10->SetValue(T);

		}
	}

	if(S1_EndPoint == 0)
	{
		T = "";
		TextCtrl6->SetValue(T);
		TextCtrl8->SetValue(T);
		TextCtrl10->SetValue(T);
	}


	if(EL_EndPoint->S2_IP.size() > 1)
	{
		S2_EndPoint = Find_EndPoint_By_IP(EL_EndPoint->S2_IP);
		if(S2_EndPoint)
		{
			T = S2_EndPoint->IP;
			TextCtrl7->SetValue(T);

			T = S2_EndPoint->Name;
			TextCtrl9->SetValue(T);

			T = wxString::Format("%d", S2_EndPoint->Channels_Used / 3);
			TextCtrl11->SetValue(T);

		}
	}

	if(S2_EndPoint == 0)
	{
		T = "";
		TextCtrl7->SetValue(T);
		TextCtrl9->SetValue(T);
		TextCtrl11->SetValue(T);
	}



	if(Channels_Avail_To_Slave == 0)
	{
		TextCtrl5->SetBackgroundColour(CR);
		TextCtrl5->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

		T = wxString::Format("%d", Channels_Avail_To_Slave);
		TextCtrl5->SetValue(T);

		T = "No More Slaves Allowed";
		TextCtrl12->SetBackgroundColour(CR);
		TextCtrl12->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());


	}
	else
	{
		TextCtrl5->SetBackgroundColour(CG);
		TextCtrl5->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

		T = wxString::Format("%d", Channels_Avail_To_Slave);
		TextCtrl5->SetValue(T);

		T = "Acceptable";
		TextCtrl12->SetBackgroundColour(CG);
		TextCtrl12->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

	}

	TextCtrl12->SetValue(T);


	return true;
}

bool EasyLights_Add_Slaves_Dialog::TransferDataFromWindow()
{
	return true;
}



void EasyLights_Add_Slaves_Dialog::OnButton_S1_Add(wxCommandEvent& event)
{
	wxString T;
	int ItemData;
	wxColour C(150, 150, 255);
	int V, SV;

	V = ELC_Status_To_Support_Slaves(EL_EndPoint);
	if(V == ELC_Support_NO_Slaves)
	{
		wxBell();
		return;
	}

	if(ListCtrl1->GetSelectedItemCount() == 0)
	{
		wxBell();
		return;
	}

	S1_Selected_EndPoint_Index = ListCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(S1_Selected_EndPoint_Index < 0)
	{
		wxBell();
		return;
	}
	
	ItemData = (int)ListCtrl1->GetItemData(S1_Selected_EndPoint_Index);

	S1_EndPoint = (EL_Ptr->EL_EndPoints)[ItemData];

	SV = Slave_Consumes_This_Number_Of_Controllers(S1_EndPoint);
	if(SV > 2)
	{
		wxBell();
		S1_EndPoint = 0;
		return;
	}

	if(V == ELC_Support_One_Slave && SV > 1)
	{
		wxBell();
		S1_EndPoint = 0;
		return;
	}

	if(EL_EndPoint->S1_IP.size() > 3 || EL_EndPoint->S1_IP == S1_EndPoint->IP || EL_EndPoint->S2_IP == S1_EndPoint->IP)	// already used?
	{
		S1_Selected_EndPoint_Index = 0;
		S1_EndPoint = 0;
		wxMessageBox(wxT("That Attachment has Already Been Selected"), wxT("Selection Error"), wxOK, NULL);
		wxBell();
		return;
	}

	EL_EndPoint->S1_IP = S1_EndPoint->IP;
	S1_EndPoint->Slave_Attached = 1;

	TransferDataToWindow();

	Dirty = 1;

	ListCtrl1->SetItemState(S1_Selected_EndPoint_Index, 0, wxLIST_STATE_SELECTED);	// deselect so color change visible

	ListCtrl1->SetItemBackgroundColour(S1_Selected_EndPoint_Index, C);

// this used to force color change

	T.Printf("%s", S1_EndPoint->IP);
	ListCtrl1->SetItem(S1_Selected_EndPoint_Index, 1, T);



}

void EasyLights_Add_Slaves_Dialog::OnButton_S1_Remove(wxCommandEvent& event)
{
	wxColour C(255, 255, 255);
	wxString T;

	if(S1_EndPoint == 0)
		return;

	ListCtrl1->SetItemBackgroundColour(S1_Selected_EndPoint_Index, C);

	// this forces color change
	T.Printf("%s", S1_EndPoint->IP);
	ListCtrl1->SetItem(S1_Selected_EndPoint_Index, 1, T);


	S1_EndPoint->Slave_Attached = 0;
	EL_EndPoint->S1_IP = "";

	S1_Selected_EndPoint_Index = 0;
	S1_EndPoint = 0;

	TransferDataToWindow();

	Dirty = 1;
}

void EasyLights_Add_Slaves_Dialog::OnButton_S2_Add(wxCommandEvent& event)
{
	wxString T;
	int ItemData;
	wxColour C(150, 150, 255);
	int V, SV;

	V = ELC_Status_To_Support_Slaves(EL_EndPoint);
	if(V == ELC_Support_NO_Slaves)
	{
		wxBell();
		return;
	}


	if(ListCtrl1->GetSelectedItemCount() == 0)
	{
		wxBell();
		return;
	}

	S2_Selected_EndPoint_Index = ListCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(S2_Selected_EndPoint_Index < 0)
	{
		wxBell();
		return;
	}

	ItemData = (int)ListCtrl1->GetItemData(S2_Selected_EndPoint_Index);

	S2_EndPoint = (EL_Ptr->EL_EndPoints)[ItemData];

	SV = Slave_Consumes_This_Number_Of_Controllers(S2_EndPoint);
	if(SV > 2)
	{
		wxBell();
		S2_EndPoint = 0;
		return;
	}

	if(V == ELC_Support_One_Slave && SV > 1)
	{
		wxBell();
		S2_EndPoint = 0;
		return;
	}


	if(EL_EndPoint->S2_IP.size() > 3 || EL_EndPoint->S1_IP == S2_EndPoint->IP || EL_EndPoint->S2_IP == S2_EndPoint->IP)	// already used?
	{
		S2_Selected_EndPoint_Index = 0;
		S2_EndPoint = 0;
		wxMessageBox(wxT("That Attachment has Already Been Selected"), wxT("Selection Error"), wxOK, NULL);
		wxBell();
		return;
	}

	EL_EndPoint->S2_IP = S2_EndPoint->IP;
	S2_EndPoint->Slave_Attached = 1;

	TransferDataToWindow();

	Dirty = 1;

	ListCtrl1->SetItemState(S2_Selected_EndPoint_Index, 0, wxLIST_STATE_SELECTED);	// deselect so color change visible

	ListCtrl1->SetItemBackgroundColour(S2_Selected_EndPoint_Index, C);

	// this used to force color change

	T.Printf("%s", S2_EndPoint->IP);
	ListCtrl1->SetItem(S2_Selected_EndPoint_Index, 1, T);



}

void EasyLights_Add_Slaves_Dialog::OnButton_S2_Remove(wxCommandEvent& event)
{
	wxColour C(255, 255, 255);
	wxString T;

	if(S2_EndPoint == 0)
		return;

	ListCtrl1->SetItemBackgroundColour(S2_Selected_EndPoint_Index, C);

	// this forces color change
	T.Printf("%s", S2_EndPoint->IP);
	ListCtrl1->SetItem(S2_Selected_EndPoint_Index, 1, T);


	S2_EndPoint->Slave_Attached = 0;
	EL_EndPoint->S2_IP = "";

	S2_Selected_EndPoint_Index = 0;
	S2_EndPoint = 0;

	TransferDataToWindow();

	Dirty = 1;

}

void EasyLights_Add_Slaves_Dialog::OnButtonCloseClick(wxCommandEvent& event)
{
	/**
	if(Error_No_Save)
	{
		wxMessageBox(wxT("You can NOT Close until you REMOVE an Attachment that has caused More Channels to be Used then Available"), wxT("Close Error"), wxOK, NULL);
		return;
	}
	**/

	EndDialog(0);
}
