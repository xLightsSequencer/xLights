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

#include "EasyLights_Identify_Dialog.h"
#include "../EasyLights.h"
#include "EasyLights_Data.h"



//(*InternalHeaders(EasyLights_Identify_Dialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EasyLights_Identify_Dialog)
const long EasyLights_Identify_Dialog::ID_STATICTEXT1 = wxNewId();
const long EasyLights_Identify_Dialog::ID_LISTCTRL1 = wxNewId();
const long EasyLights_Identify_Dialog::ID_STATICTEXT2 = wxNewId();
const long EasyLights_Identify_Dialog::ID_TEXTCTRL1 = wxNewId();
const long EasyLights_Identify_Dialog::ID_STATICTEXT3 = wxNewId();
const long EasyLights_Identify_Dialog::ID_TEXTCTRL2 = wxNewId();
const long EasyLights_Identify_Dialog::ID_STATICTEXT5 = wxNewId();
const long EasyLights_Identify_Dialog::ID_CHOICE2 = wxNewId();
const long EasyLights_Identify_Dialog::ID_CHECKBOX1 = wxNewId();
const long EasyLights_Identify_Dialog::ID_CHECKBOX2 = wxNewId();
const long EasyLights_Identify_Dialog::ID_BUTTON1 = wxNewId();
const long EasyLights_Identify_Dialog::ID_BUTTON2 = wxNewId();
const long EasyLights_Identify_Dialog::ID_BUTTON3 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EasyLights_Identify_Dialog,wxDialog)
	//(*EventTable(EasyLights_Identify_Dialog)
	EVT_INIT_DIALOG(EasyLights_Identify_Dialog::InitDialog)
	//*)
END_EVENT_TABLE()

EasyLights_Identify_Dialog::EasyLights_Identify_Dialog(wxWindow* parent, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	Selected_EndPoint = 0;
	Dirty = 0;
	Selected_EndPoint_Index = 0;

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

	itemCol.SetText(wxT("Type"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(2, itemCol);
	ListCtrl1->SetColumnWidth(2, 150);

	itemCol.SetText(wxT("Channels"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(3, itemCol);
	ListCtrl1->SetColumnWidth(3, 150);


	itemCol.SetText(wxT("Master"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(4, itemCol);
	ListCtrl1->SetColumnWidth(4, 100);


	itemCol.SetText(wxT("Audio"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(5, itemCol);
	ListCtrl1->SetColumnWidth(5, 100);

	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Identify_Dialog::OnButton_Cancel);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Identify_Dialog::OnButton_Edit);
	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Identify_Dialog::OnButton_Apply);


}

void EasyLights_Identify_Dialog::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EasyLights_Identify_Dialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("E131 Controllers on your Network"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl1 = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxSize(850,250), wxLC_REPORT|wxLC_SINGLE_SEL|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	FlexGridSizer1->Add(ListCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(5, 2, 0, 0);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("IP Address"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxSize(100,25), wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Controller\'s Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl2 = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxSize(200,25), 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer2->Add(TextCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Controller Model"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_TOP|wxALIGN_CENTER_HORIZONTAL, 5);
	Choice2 = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxSize(200,88), 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
	FlexGridSizer2->Add(Choice2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox1 = new wxCheckBox(this, ID_CHECKBOX1, _("StandAlone Master"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
	CheckBox1->SetValue(false);
	FlexGridSizer2->Add(CheckBox1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CheckBox2 = new wxCheckBox(this, ID_CHECKBOX2, _("Plays the Audio"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
	CheckBox2->SetValue(false);
	FlexGridSizer2->Add(CheckBox2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	Button1 = new wxButton(this, ID_BUTTON1, _("Cancel Change"), wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button1, 1, wxALL|wxEXPAND, 5);
	Button2 = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxSize(75,25), 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button2, 1, wxALL|wxEXPAND, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("Accept Change"), wxDefaultPosition, wxSize(100,25), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer3->Add(Button3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

EasyLights_Identify_Dialog::~EasyLights_Identify_Dialog()
{
	//(*Destroy(EasyLights_Identify_Dialog)
	//*)
}

void EasyLights_Identify_Dialog::InitDialog(wxInitDialogEvent &WXUNUSED(event))
{
	Load_Combo();

	Load_I_List();

}

void EasyLights_Identify_Dialog::Load_I_List()
{
	wxString T;
	int i = 0;

	for(auto it = EL_Ptr->EL_EndPoints.begin(); it != EL_Ptr->EL_EndPoints.end(); ++it, i++)
	{
		T.Printf("%s", (*it)->Name);
		ListCtrl1->InsertItem(i, T, -1);
		ListCtrl1->SetItemData(i, i);

		T.Printf("%s", (*it)->IP);
		ListCtrl1->SetItem(i, 1, T);

		if((*it)->Model.size() == 0)
			(*it)->Model = "Undefined";

		T.Printf("%s", (*it)->Model);
		ListCtrl1->SetItem(i, 2, T);

		T.Printf("%d", (*it)->Channels_Used);
		ListCtrl1->SetItem(i, 3, T);

		if((*it)->Master)
			T = "Yes";
		else
			T = "No";
		ListCtrl1->SetItem(i, 4, T);

		if((*it)->Audio)
			T = "Yes";
		else
			T = "No";
		ListCtrl1->SetItem(i, 5, T);


	}
}



void EasyLights_Identify_Dialog::Load_Combo()
{
	wxString T;


	Choice2->Append(wxT("Undefined"));

	for(auto it = EL_Ptr->EL_Controller.begin(); it != EL_Ptr->EL_Controller.end(); ++it)
	{
		T = (*it)->Model;
		Choice2->Append(T);
	}
}

void EasyLights_Identify_Dialog::OnButton_Cancel(wxCommandEvent& event)
{
	Selected_EndPoint = 0;
	TransferDataToWindow();

}

void EasyLights_Identify_Dialog::OnButton_Edit(wxCommandEvent& event)
{
	if(ListCtrl1->GetSelectedItemCount() == 0)
		return;

	Selected_EndPoint_Index = ListCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(Selected_EndPoint_Index < 0)
		return;

	Selected_EndPoint = (EL_Ptr->EL_EndPoints)[Selected_EndPoint_Index];

	if(EL_Ptr->ELC_Master && Selected_EndPoint != EL_Ptr->ELC_Master)	// can't allow Master to be modified
	{
		CheckBox1->Enable(FALSE);
	}
	else // allow modification
	{
		CheckBox1->Enable(TRUE);
	}

	if(EL_Ptr->ELC_Audio && Selected_EndPoint != EL_Ptr->ELC_Audio)	// can't allow Master to be modified
	{
		CheckBox2->Enable(FALSE);
	}
	else // allow modification
	{
		CheckBox2->Enable(TRUE);
	}


	TransferDataToWindow();

}

void EasyLights_Identify_Dialog::OnButton_Apply(wxCommandEvent& event)
{
	if(Selected_EndPoint == 0)
		return;

	TransferDataFromWindow();

// record any changes to Master and Audio

	if(Selected_EndPoint->Master && EL_Ptr->ELC_Master == 0)
		EL_Ptr->ELC_Master = Selected_EndPoint;

	if(Selected_EndPoint->Audio && EL_Ptr->ELC_Audio == 0)
		EL_Ptr->ELC_Audio = Selected_EndPoint;

	if(Selected_EndPoint == EL_Ptr->ELC_Master && Selected_EndPoint->Master == 0)
		EL_Ptr->ELC_Master = 0;

	if(Selected_EndPoint == EL_Ptr->ELC_Audio && Selected_EndPoint->Audio == 0)
		EL_Ptr->ELC_Audio = 0;



	Update_Selected_List_Item();

	Dirty = 1;

	Selected_EndPoint = 0;	// make the input blank
	TransferDataToWindow();

}

bool EasyLights_Identify_Dialog::TransferDataToWindow()
{
	wxString T;
	int Choice;
	wxColour CR(255, 150, 150);
	wxColour CG(150, 255, 150);

	if(Selected_EndPoint == 0)
	{
		T = "";
		TextCtrl1->SetValue(T);

		TextCtrl2->SetBackgroundColour(CR);
		TextCtrl2->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

		T = "test";
		TextCtrl2->SetValue(T);
		T = "";
		TextCtrl2->SetValue(T);

		Choice2->SetSelection(0);

		CheckBox1->SetValue(0);
		CheckBox2->SetValue(0);

		return true;
	}

	T = Selected_EndPoint->IP;
	TextCtrl1->SetValue(T);

	TextCtrl2->SetBackgroundColour(CG);
	TextCtrl2->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

	T = "test";	// if blank color won't change
	TextCtrl2->SetValue(T);

	T = Selected_EndPoint->Name;
	TextCtrl2->SetValue(T);

	Choice = Return_Model_Selection_Index(Selected_EndPoint->Model);
	Choice2->SetSelection(Choice);

	CheckBox1->SetValue(Selected_EndPoint->Master);
	CheckBox2->SetValue(Selected_EndPoint->Audio);

	return true;
}

bool EasyLights_Identify_Dialog::TransferDataFromWindow()
{
	int Choice;

	if(Selected_EndPoint == 0)
		return true;

	Selected_EndPoint->Name = TextCtrl2->GetValue();

	Choice = Choice2->GetSelection();
	Set_EndPoint_Model_String_From_Choice_Index(Choice);

	// Choice == 0 for slave

	if(Choice)	// only for ELC
	{
		Selected_EndPoint->Master = CheckBox1->GetValue();
		Selected_EndPoint->Audio = CheckBox2->GetValue();
	}

	return true;
}


int EasyLights_Identify_Dialog::Return_Model_Selection_Index(std::string &M)
{
	int i = 1;

	if(M == "Undefined")
		return 0;

	for(auto it = EL_Ptr->EL_Controller.begin(); it != EL_Ptr->EL_Controller.end(); ++it, i++)
	{
		if((*it)->Model == M)
			return i;
	}

	return 0;

}

void EasyLights_Identify_Dialog::Set_EndPoint_Model_String_From_Choice_Index(int I)
{
	if(Selected_EndPoint == 0)
		return;

	if(I == 0)
		Selected_EndPoint->Model = "Undefined";
	else
	{
		Selected_EndPoint->Model = (EL_Ptr->EL_Controller)[I-1]->Model;
	}
}


void EasyLights_Identify_Dialog::Update_Selected_List_Item()
{
	wxString T;

	if(Selected_EndPoint == 0)
		return;

	T.Printf("%s", Selected_EndPoint->Name);
	ListCtrl1->SetItem(Selected_EndPoint_Index, 0, T);

	if(Selected_EndPoint->Model.size() == 0)
		Selected_EndPoint->Model = "Undefined";

	T.Printf("%s", Selected_EndPoint->Model);
	ListCtrl1->SetItem(Selected_EndPoint_Index, 2, T);

	if(Selected_EndPoint->Model != "Undefined")
	{
		if(Selected_EndPoint->Master)
			T = "Yes";
		else
			T = "No";
		ListCtrl1->SetItem(Selected_EndPoint_Index, 4, T);

		if(Selected_EndPoint->Audio)
			T = "Yes";
		else
			T = "No";
		ListCtrl1->SetItem(Selected_EndPoint_Index, 5, T);

		// we have a easylights controller - update all controller max values

		Set_EndPoint_EL_Max_Controller_Values(Selected_EndPoint);

	}
	else
	{
		T = "No";
		ListCtrl1->SetItem(Selected_EndPoint_Index, 4, T);
		ListCtrl1->SetItem(Selected_EndPoint_Index, 5, T);
	}


}
