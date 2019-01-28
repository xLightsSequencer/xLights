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
#include <wx/datetime.h>

#include <wx/dir.h>

#include "../../models/Model.h"
#include "../../outputs/OutputManager.h"
#include "../../outputs/Output.h"
#include "../../models/ModelManager.h"

#include "../EasyLights.h"

#include "../EasyLights.h"
#include "EasyLights_Data.h"
#include "EasyLights_PlayList_Dialog.h"
#include "../../xlightsMain.h"


//(*IdInit(EasyLights_PlayList_Dialog)
const long EasyLights_PlayList_Dialog::ID_STATICTEXT1 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_LISTCTRL1 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_STATICTEXT2 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_LISTCTRL2 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_STATICTEXT4 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_BUTTON7 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_TEXTCTRL1 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_BUTTON2 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_BUTTON3 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_STATICTEXT3 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_TEXTCTRL2 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_GRID1 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_BUTTON1 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_BUTTON5 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_BUTTON6 = wxNewId();
const long EasyLights_PlayList_Dialog::ID_STATICTEXT5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EasyLights_PlayList_Dialog,wxDialog)
	//(*EventTable(EasyLights_PlayList_Dialog)
	EVT_INIT_DIALOG(EasyLights_PlayList_Dialog::InitDialog)
	EVT_LIST_ITEM_SELECTED(EasyLights_PlayList_Dialog::ID_LISTCTRL1, EasyLights_PlayList_Dialog::OnList1_Seq_Selected)
	EVT_GRID_SELECT_CELL(EasyLights_PlayList_Dialog::OnGridCellLeftClick)
	EVT_GRID_CELL_RIGHT_CLICK(EasyLights_PlayList_Dialog::OnGridCellRightClick)
	

	//*)
END_EVENT_TABLE()

EasyLights_PlayList_Dialog::EasyLights_PlayList_Dialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	PLWork_Active = 0;
	Grid_Left_Row = 0;
	Grid_Left_Col = 0;


	BuildContent(parent,id,pos,size);

	Connect(ID_BUTTON7, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButton_New_PlayList);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButton_Edit_PlayList);
	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButton_Delete_PlayList);
	Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButton_Remove_PlayList);
	Connect(ID_BUTTON6, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButton_Save_PlayList);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButton_Add_End_PlayList);



	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButtonCloseClick);
	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&EasyLights_PlayList_Dialog::OnButtonCloseClick);

}

void EasyLights_PlayList_Dialog::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EasyLights_PlayList_Dialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;

	Create(parent, wxID_ANY, _("EasyLights PlayList Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Sequences"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl1 = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	ListCtrl1->SetMinSize(wxSize(300,200));
	FlexGridSizer2->Add(ListCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("PlayLists"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl2 = new wxListCtrl(this, ID_LISTCTRL2, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL2"));
	ListCtrl2->SetMinSize(wxSize(300,150));
	FlexGridSizer2->Add(ListCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer6->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("New PlayList Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer6->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button7 = new wxButton(this, ID_BUTTON7, _("New Playlist"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer6->Add(Button7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl1->SetMinSize(wxSize(200,25));
	FlexGridSizer6->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	Button2 = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer4->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("Delete"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer4->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("PlayList Details"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl2 = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	TextCtrl2->SetMinSize(wxSize(250,25));
	FlexGridSizer3->Add(TextCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
	Grid1->CreateGrid(0,2);
	Grid1->SetMinSize(wxSize(300,350));
	Grid1->EnableEditing(true);
	Grid1->EnableGridLines(true);
	Grid1->SetColLabelValue(0, _("Sequence"));
	Grid1->SetColLabelValue(1, _("Delay"));
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	FlexGridSizer3->Add(Grid1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	Button1 = new wxButton(this, ID_BUTTON1, _("Add End"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button5 = new wxButton(this, ID_BUTTON5, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer5->Add(Button5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button6 = new wxButton(this, ID_BUTTON6, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer5->Add(Button6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Right Mouse Clickto Insert Sequence"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer3->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();
	//*)
}

EasyLights_PlayList_Dialog::~EasyLights_PlayList_Dialog()
{
	//(*Destroy(EasyLights_PlayList_Dialog)
	//*)
}

void EasyLights_PlayList_Dialog::InitDialog(wxInitDialogEvent &WXUNUSED(event))
{
	wxListItem itemCol;

	itemCol.SetText(wxT("Name"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(0, itemCol);
	ListCtrl1->SetColumnWidth(0, 250);

	ListCtrl2->InsertColumn(0, itemCol);
	ListCtrl2->SetColumnWidth(0, 250);

	Display_Sequences();

	Grid1->SetRowLabelSize(0);
	Grid1->SetColSize(0, 225);
	Grid1->SetColSize(1, 75);

	Display_PlayLists();
}

void EasyLights_PlayList_Dialog::OnButtonCloseClick(wxCommandEvent& event)
{

	EndDialog(0);
}

void EasyLights_PlayList_Dialog::OnButton_New_PlayList(wxCommandEvent& event)
{
	char B[200];
	char *p;
	std::string PLnew;

	if(PLWork_Active)
	{
		wxMessageBox(wxT("A PlayList is currently being worked on - Please Save first "), wxT("New PlayList"), wxOK, NULL);

		return;
	}

	PLnew = TextCtrl1->GetValue();


	if(PLnew.size() < 2)
	{
		wxMessageBox(wxT("Name of New PlayList is Missing"), wxT("New PlayList"), wxOK, NULL);

		return;
	}

	PLWork_Active = 1;

	strncpy(B, PLnew.c_str(), 199);
	p = B;

	while(*p && *p != '.') p++;
	if(*p)
		*p = 0;

	PLwork = B;
	PLwork += ".ply";

	TextCtrl2->SetValue(PLwork);
	TextCtrl2->Refresh();
	TextCtrl2->Update();

	TextCtrl1->SetValue("");
	TextCtrl1->Refresh();
	TextCtrl1->Update();

}

void EasyLights_PlayList_Dialog::OnButton_Edit_PlayList(wxCommandEvent& event)
{
	int IDX;

	if(PLWork_Active)
	{
		wxMessageBox(wxT("A PlayList being Edited - Please Save First"), wxT("PlayList"), wxOK, NULL);

		return;
	}


	if(ListCtrl2->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl2->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	PLwork = ListCtrl2->GetItemText(IDX);
	PLWork_Active = 1;

	TextCtrl2->SetValue(PLwork);
	TextCtrl2->Refresh();
	TextCtrl2->Update();

	Load_PlayList_to_Work_on();

}

void EasyLights_PlayList_Dialog::OnButton_Delete_PlayList(wxCommandEvent& event)
{
	int IDX;
	std::string FN;

	if(PLWork_Active)
	{
		wxMessageBox(wxT("A PlayList being Edited - Please Save First"), wxT("PlayList"), wxOK, NULL);

		return;
	}


	if(ListCtrl2->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl2->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	FN = ListCtrl2->GetItemText(IDX);

	wxString FNL = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, FN);


	wxRemove(FNL);

	ListCtrl2->DeleteItem(IDX);

}


void EasyLights_PlayList_Dialog::OnButton_Remove_PlayList(wxCommandEvent& event)
{
	if(Grid_Left_Row >= 0)
	{
		Grid1->DeleteRows(Grid_Left_Row, 1);

		Grid_Left_Row = -1;	// selection row is gone
	}

}

void EasyLights_PlayList_Dialog::OnButton_Save_PlayList(wxCommandEvent& event)
{
	if(PLWork_Active == 0)
		return;

	Save_Work_on_Play_List();

	Add_PlayList_to_Display_if_New(PLwork);


	PLwork = "";

	TextCtrl2->SetValue("");
	TextCtrl2->Refresh();
	TextCtrl2->Update();

	int i = Grid1->GetNumberRows();

	Grid1->DeleteRows(0, i);

	PLWork_Active = 0;

}

void EasyLights_PlayList_Dialog::OnButton_Add_End_PlayList(wxCommandEvent& event)
{
	if(PLWork_Active == 0)
	{
		wxMessageBox(wxT("No Active PlayList being Edited"), wxT("PlayList"), wxOK, NULL);

		return;
	}


	if(Selected_List1_Sequence.size() < 3)
	{
		wxMessageBox(wxT("No Sequence Selected"), wxT("Selection Error"), wxOK, NULL);

		return;
	}

	int i = Grid1->GetNumberRows();

	Grid1->InsertRows(i, 1);
	Grid1->SetCellValue(i, 0, Selected_List1_Sequence);
	Grid1->SetReadOnly(i, 0, true);
	Grid1->SetCellValue(i, 1, "4");


}

void EasyLights_PlayList_Dialog::OnList1_Seq_Selected(wxListEvent& event)
{
	int IDX;

	if(ListCtrl1->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	Selected_List1_Sequence = ListCtrl1->GetItemText(IDX);
}

void EasyLights_PlayList_Dialog::OnGridCellLeftClick(wxGridEvent &event)
{
	Grid_Left_Row = event.GetRow();
	Grid_Left_Col = event.GetCol();

}

void EasyLights_PlayList_Dialog::OnGridCellRightClick(wxGridEvent &event)
{
	int Row = event.GetRow();
	int Col = event.GetCol();

	if(PLWork_Active == 0)
	{
		wxMessageBox(wxT("No Active PlayList being Edited"), wxT("PlayList"), wxOK, NULL);

		return;
	}


	if(Selected_List1_Sequence.size() < 3)
	{
		wxMessageBox(wxT("No Sequence Selected for Insert"), wxT("Selection Error"), wxOK, NULL);

		return;
	}

	Grid1->InsertRows(Row, 1);
	Grid1->SetCellValue(Row, 0, Selected_List1_Sequence);
	Grid1->SetReadOnly(Row, 0, true);
	Grid1->SetCellValue(Row, 1, "4");
}




//[{"S":"Christmas Eve Sarajevo.fseq","A":"Christmas Eve Sarajevo.wav","D":4}]

void EasyLights_PlayList_Dialog::Load_PlayList_to_Work_on()
{
	wxString FN = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, PLwork);
	std::string fn = FN;

	FILE *fo = fopen((const char *)fn.c_str(), "r");

	int FSZ, r;
	char *B;
	char *p, *JF, *JV;
	char *EOL;
	std::string SQ;
	std::string Audio;
	std::string Delay;
	int Row = 0;


	if(fo == 0)
	{
		wxMessageBox(wxT("Failed to Open the PlayList"), wxT("File Open"), wxOK, NULL);
		return;
	}

	fseek(fo, 0L, SEEK_END);
	FSZ = ftell(fo);
	fseek(fo, 0L, SEEK_SET);

	B = (char*)malloc(FSZ+5);
	if(B == 0)
	{
		fclose(fo);
		wxMessageBox(wxT("Failed to Acquire Memory for PlayList"), wxT("File Edit"), wxOK, NULL);

		return;
	}

	memset(B, 0, FSZ+3);

	r = fread(B, 1, FSZ+2, fo);

	fclose(fo);

	EOL = B;

	while(1)	// parse file
	{
		if(*EOL == 0)
			break;

		p = EOL;

		// find start of jason line

		while(*p && *p != '{') p++;
		if(*p == 0)
			break;	// done

		p++;	// skip over {

// find end of jason line
		EOL = p;

		while(*EOL && *EOL != '}') EOL++;
		if(*EOL == 0)
			break;	// ERROR!!! - done

		*EOL = 0;
		EOL++;

		SQ = "";
		Audio = "";
		Delay = "";

		while(p)	// get all wanted fields
		{
			p = Get_Next_JArg(p, &JF, &JV, 0, 1);

			if(p)
			{
				switch(*JF)
				{
				case 'S':	// sequence file name
					SQ = JV;
					break;

				case 'A':
					Audio = JV;
					break;

				case 'D':
					Delay = JV;
					break;

				}

			}	// end of if p
		} // end of while fields

		// we have a jason package decoded

		Grid1->InsertRows(Row, 1);
		Grid1->SetCellValue(Row, 0, SQ);
		Grid1->SetReadOnly(Row, 0, true);
		Grid1->SetCellValue(Row, 1, Delay);

		Row++;

	}

}

void EasyLights_PlayList_Dialog::Save_Work_on_Play_List()
{
	wxString FN = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, PLwork);
	std::string fn = FN;
	std::string T0;
	std::string T1;
	char B[200];
	char *p;

	FILE *fo = fopen((const char *)fn.c_str(), "w");

	if(fo == 0)
	{
		wxMessageBox(wxT("Failed to Save the PlayList"), wxT("File Save"), wxOK, NULL);

		return;
	}

	int i = Grid1->GetNumberRows();
	int j;

	fprintf(fo, "[");

	// [{"H":"Steve.ezls","D":3},{"H":"Steve.ezls","D":3}]

	for(j = 0; j < i; j++)
	{
		T0 = Grid1->GetCellValue(j, 0);
		T1 = Grid1->GetCellValue(j, 1);

		strncpy(B, T0.c_str(), 199);
		p = strstr(B, ".fseq");
		if(p)
			strcpy(p, ".wav");


		fprintf(fo, "{\"S\":\"%s\",\"A\":\"%s\",\"D\":%s}", T0.c_str(), B, T1.c_str());

		if(j < (i - 1))
			fprintf(fo, ",");
	}

	fprintf(fo, "]");

	fclose(fo);

}

void EasyLights_PlayList_Dialog::Add_PlayList_to_Display_if_New(std::string PL)
{
	int i = ListCtrl2->GetItemCount();
	int j;
	std::string T;


	for(j = 0; j < i; j++)
	{
		T = ListCtrl2->GetItemText(j);
		if(T == PL)
			return;
	}

	ListCtrl2->InsertItem(i, PL, -1);
}



void EasyLights_PlayList_Dialog::Display_Sequences()
{

	std::string FN_search;
	std::string FN;
	wxString T;
	std::string TS;
	int i = 0;
	char *S;
	wxFileName wxFN;



	FN_search = EL_Ptr->ShowDirectory + "\\EasyLightsData\\*.fseq";

	FN = wxFindFirstFile(FN_search);

	while(!FN.empty())
	{
		wxFN = FN;

		T = wxString::Format("%s.%s", wxFN.GetName(), wxFN.GetExt());
		TS = T;
		S = Return_EasuLights_FSEQ_FN_Without_IP((char*)TS.c_str());
		if(S == 0)
			goto next;

		TS = S;
		free(S);

		// we can have the same fseq name but for multiple controllers

		for(auto it = Seq_Array.begin(); it != Seq_Array.end(); ++it, i++)
		{
			if(*it == TS)
				goto next;
		}

		ListCtrl1->InsertItem(i, TS, -1);

		Seq_Array.push_back(TS);
		i++;

next:
		FN = wxFindNextFile();
	}


}


void EasyLights_PlayList_Dialog::Display_PlayLists()
{

	std::string FN_search;
	std::string FN;
	wxString T;
	int i = 0;
	wxFileName wxFN;

	ListCtrl2->DeleteAllItems();


	FN_search = EL_Ptr->ShowDirectory + "\\EasyLightsData\\*.ply";

	FN = wxFindFirstFile(FN_search);

	while(!FN.empty())
	{
		wxFN = FN;

		T = wxString::Format("%s.%s", wxFN.GetName(), wxFN.GetExt());

		ListCtrl2->InsertItem(i, T, -1);

		i++;
		FN = wxFindNextFile();
	}


}

/***
bool EasyLights_PlayList_Dialog::TransferDataFromWindow()
{

	PLnew = TextCtrl1->GetValue();

	return true;
}
***/
