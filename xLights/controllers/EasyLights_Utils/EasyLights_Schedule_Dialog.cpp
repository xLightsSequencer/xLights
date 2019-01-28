

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
#include "EasyLights_Schedule_Dialog.h"

//(*IdInit(EasyLights_Schedule_Dialog)
const long EasyLights_Schedule_Dialog::ID_STATICTEXT1 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_LISTCTRL1 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_STATICTEXT2 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_LISTCTRL2 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON2 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON3 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON4 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_STATICTEXT3 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_TEXTCTRL2 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_GRID1 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON1 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON5 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON6 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_BUTTON7 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_STATICTEXT5 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_STATICTEXT4 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_STATICTEXT6 = wxNewId();
const long EasyLights_Schedule_Dialog::ID_STATICTEXT7 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EasyLights_Schedule_Dialog,wxDialog)
	//(*EventTable(EasyLights_Schedule_Dialog)
	EVT_INIT_DIALOG(EasyLights_Schedule_Dialog::InitDialog)
	EVT_LIST_ITEM_SELECTED(EasyLights_Schedule_Dialog::ID_LISTCTRL1, EasyLights_Schedule_Dialog::OnList1_Seq_Selected)
	EVT_GRID_SELECT_CELL(EasyLights_Schedule_Dialog::OnGridCellLeftClick)
	EVT_GRID_CELL_RIGHT_CLICK(EasyLights_Schedule_Dialog::OnGridCellRightClick)

	//*)
END_EVENT_TABLE()

EasyLights_Schedule_Dialog::EasyLights_Schedule_Dialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	SchedWork_Active = 0;

	BuildContent(parent,id,pos,size);

	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_Edit_PlayList);
	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_NoSchedule);
	Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_Copy_Schedule);

	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_Add_End_Schedule);
	Connect(ID_BUTTON5, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_Remove_Schedule);
	Connect(ID_BUTTON6, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_Save_Schedule);
	Connect(ID_BUTTON7, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButton_Sanity_Check);


	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButtonCloseClick);
	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&EasyLights_Schedule_Dialog::OnButtonCloseClick);

}

void EasyLights_Schedule_Dialog::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EasyLights_Schedule_Dialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;

	Create(parent, wxID_ANY, _("EasyLights PlayList Manager"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("PlayLists"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl1 = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	ListCtrl1->SetMinSize(wxSize(300,200));
	FlexGridSizer2->Add(ListCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Schedules for Day of Week"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ListCtrl2 = new wxListCtrl(this, ID_LISTCTRL2, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxVSCROLL, wxDefaultValidator, _T("ID_LISTCTRL2"));
	ListCtrl2->SetMinSize(wxSize(300,150));
	FlexGridSizer2->Add(ListCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	Button2 = new wxButton(this, ID_BUTTON2, _("Edit"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer4->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("No Schedule"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer4->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button4 = new wxButton(this, ID_BUTTON4, _("Copy"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer4->Add(Button4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Schedule Details"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer3->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl2 = new wxTextCtrl(this, ID_TEXTCTRL2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	TextCtrl2->SetMinSize(wxSize(175,25));
	FlexGridSizer3->Add(TextCtrl2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
	Grid1->CreateGrid(0,4);
	Grid1->SetMinSize(wxSize(500,350));
	Grid1->EnableEditing(true);
	Grid1->EnableGridLines(true);
	Grid1->SetColLabelValue(0, _("PlayList"));
	Grid1->SetColLabelValue(1, _("Start Time"));
	Grid1->SetColLabelValue(2, _("End Time"));
	Grid1->SetColLabelValue(3, _("# Loops"));
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	FlexGridSizer3->Add(Grid1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 4, 0, 0);
	Button1 = new wxButton(this, ID_BUTTON1, _("Add End"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer5->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button5 = new wxButton(this, ID_BUTTON5, _("Remove"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer5->Add(Button5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button6 = new wxButton(this, ID_BUTTON6, _("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON6"));
	FlexGridSizer5->Add(Button6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button7 = new wxButton(this, ID_BUTTON7, _("Sanity Check"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON7"));
	FlexGridSizer5->Add(Button7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Right Mouse Clickto Insert Sequence"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer3->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT4, _("Time format is Hour minutes e.g. 3:30pm is Entered as 1530 -- Hour is 0 to 23 -- Schedule Ends at Midnight"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer3->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("PlayList becomes Active when the PlayList before it Completes"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer3->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(this, ID_STATICTEXT7, _("PlayList ends at EndTime or Loop Count is Loop Count is not Zero"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer3->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();
	//*)
}

EasyLights_Schedule_Dialog::~EasyLights_Schedule_Dialog()
{
	//(*Destroy(EasyLights_Schedule_Dialog)
	//*)
}

//(*Handlers(EasyLights_PlayList_Dialog)

void EasyLights_Schedule_Dialog::InitDialog(wxInitDialogEvent &WXUNUSED(event))
{
	wxListItem itemCol;

	itemCol.SetText(wxT("Name"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(0, itemCol);
	ListCtrl1->SetColumnWidth(0, 250);

	ListCtrl2->InsertColumn(0, itemCol);
	ListCtrl2->SetColumnWidth(0, 250);

	Display_PlayLists();

	Grid1->SetRowLabelSize(0);
	Grid1->SetColSize(0, 225);
	Grid1->SetColSize(1, 75);

	Display_DOW();
}

void EasyLights_Schedule_Dialog::OnButtonCloseClick(wxCommandEvent& event)
{

	EndDialog(0);
}


void EasyLights_Schedule_Dialog::OnButton_Edit_PlayList(wxCommandEvent& event)
{
	int IDX;

	if(SchedWork_Active)
	{
		wxMessageBox(wxT("A Schedule being Edited - Please Save First"), wxT("Schedule"), wxOK, NULL);

		return;
	}


	if(ListCtrl2->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl2->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	Sched_Work = ListCtrl2->GetItemText(IDX);
	SchedWork_Active = 1;

	TextCtrl2->SetValue(Sched_Work);
	TextCtrl2->Refresh();
	TextCtrl2->Update();

	Load_Schedule_to_Work_on();

}

void EasyLights_Schedule_Dialog::OnButton_NoSchedule(wxCommandEvent& event)
{
	int IDX;
	std::string FN;


	if(ListCtrl2->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl2->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	FN = ListCtrl2->GetItemText(IDX);

	wxString FNL = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, FN);
	std::string fn = FNL;

	FILE *fo = fopen((const char *)fn.c_str(), "w");

	if(fo)
		fclose(fo);
}

void EasyLights_Schedule_Dialog::OnButton_Copy_Schedule(wxCommandEvent& event)
{
	std::string Saved_Edit_Schedule;
	int IDX;

	if(SchedWork_Active == 0)
	{
		wxMessageBox(wxT("No Active Schedule being Edited - Only Copy Schedule Open for Edit"), wxT("Schedule"), wxOK, NULL);

		return;
	}


	if(ListCtrl2->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl2->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	Saved_Edit_Schedule = Sched_Work;

	Sched_Work = ListCtrl2->GetItemText(IDX);

	Save_Work_on_Schedule();

	Sched_Work = Saved_Edit_Schedule;

	wxMessageBox(wxT("Schedule being Edited was Copy into the requested Schedule"), wxT("Schedule"), wxOK, NULL);


}


void EasyLights_Schedule_Dialog::OnButton_Remove_Schedule(wxCommandEvent& event)
{
	if(Grid_Left_Row >= 0)
	{
		Grid1->DeleteRows(Grid_Left_Row, 1);

		Grid_Left_Row = -1;	// selection row is gone
	}

}

void EasyLights_Schedule_Dialog::OnButton_Save_Schedule(wxCommandEvent& event)
{
	if(SchedWork_Active == 0)
		return;

	Save_Work_on_Schedule();

	Sched_Work = "";

	TextCtrl2->SetValue("");
	TextCtrl2->Refresh();
	TextCtrl2->Update();

	int i = Grid1->GetNumberRows();

	if(i)
		Grid1->DeleteRows(0, i);

	SchedWork_Active = 0;

	
}

void EasyLights_Schedule_Dialog::OnButton_Add_End_Schedule(wxCommandEvent& event)
{
	if(SchedWork_Active == 0)
	{
		wxMessageBox(wxT("No Active Schedule being Edited"), wxT("Schedule"), wxOK, NULL);

		return;
	}


	if(Selected_List1_PlayList.size() < 3)
	{
		wxMessageBox(wxT("No PlayList Selected"), wxT("Selection Error"), wxOK, NULL);

		return;
	}

	int i = Grid1->GetNumberRows();

	Grid1->InsertRows(i, 1);
	Grid1->SetCellValue(i, 0, Selected_List1_PlayList);
	Grid1->SetReadOnly(i, 0, true);
	Grid1->SetCellValue(i, 1, "1");
	Grid1->SetCellValue(i, 2, "2");
	Grid1->SetCellValue(i, 3, "0");

}

void EasyLights_Schedule_Dialog::OnButton_Sanity_Check(wxCommandEvent& event)
{
	std::string T0;
	std::string T1;
	std::string T2;
	std::string T3;

	char TMBuff[20];
	int TM;


	int i = Grid1->GetNumberRows();
	int j;

	if(i == 0)	// empty sched
	{
		return;
	}


	for(j = 0; j < i; j++)
	{
		T0 = Grid1->GetCellValue(j, 0);
		T1 = Grid1->GetCellValue(j, 1);
		T2 = Grid1->GetCellValue(j, 2);
		T3 = Grid1->GetCellValue(j, 3);


		TM = atoi(T1.c_str());
		if(TM <= 0)
			TM = 1200;
		if(TM > 2355)
			TM = 2355;
		sprintf(TMBuff, "%d", TM);
		T1 = TMBuff;
		Grid1->SetCellValue(j, 1, T1);


		TM = atoi(T2.c_str());
		if(TM <= 0)
			TM = 1200;
		if(TM > 2355)
			TM = 2355;
		sprintf(TMBuff, "%d", TM);
		T2 = TMBuff;
		Grid1->SetCellValue(j, 2, T2);

		TM = atoi(T3.c_str());
		if(TM < 0)
			TM = 0;
		sprintf(TMBuff, "%d", TM);
		T3 = TMBuff;
		Grid1->SetCellValue(j, 3, T3);

	}


}

void EasyLights_Schedule_Dialog::OnList1_Seq_Selected(wxListEvent& event)
{
	int IDX;

	if(ListCtrl1->GetSelectedItemCount() == 0)
		return;

	IDX = ListCtrl1->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);

	Selected_List1_PlayList = ListCtrl1->GetItemText(IDX);

}
void EasyLights_Schedule_Dialog::OnGridCellLeftClick(wxGridEvent &event)
{
	Grid_Left_Row = event.GetRow();
	Grid_Left_Col = event.GetCol();

}

void EasyLights_Schedule_Dialog::OnGridCellRightClick(wxGridEvent &event)
{
	int Row = event.GetRow();
	int Col = event.GetCol();

	if(SchedWork_Active == 0)
	{
		wxMessageBox(wxT("No Active Schedule being Edited"), wxT("Schedule"), wxOK, NULL);

		return;
	}


	if(Selected_List1_PlayList.size() < 3)
	{
		wxMessageBox(wxT("No PlayList Selected for Insert"), wxT("Selection Error"), wxOK, NULL);

		return;
	}

	Grid1->InsertRows(Row, 1);
	Grid1->SetCellValue(Row, 0, Selected_List1_PlayList);
	Grid1->SetReadOnly(Row, 0, true);
	Grid1->SetCellValue(Row, 1, "1");
	Grid1->SetCellValue(Row, 2, "2");
	Grid1->SetCellValue(Row, 3, "0");
}

void EasyLights_Schedule_Dialog::Display_PlayLists()
{

	std::string FN_search;
	std::string FN;
	wxString T;
	int i = 0;
	wxFileName wxFN;



	FN_search = EL_Ptr->ShowDirectory + "\\EasyLightsData\\*.ply";

	FN = wxFindFirstFile(FN_search);

	while(!FN.empty())
	{
		wxFN = FN;

		T = wxString::Format("%s.%s", wxFN.GetName(), wxFN.GetExt());

		ListCtrl1->InsertItem(i, T, -1);

		i++;


		FN = wxFindNextFile();
	}


}

void EasyLights_Schedule_Dialog::Display_DOW()
{
	ListCtrl2->InsertItem(0, "Sunday.sched", -1);
	ListCtrl2->InsertItem(1, "Monday.sched", -1);
	ListCtrl2->InsertItem(2, "Tuesday.sched", -1);
	ListCtrl2->InsertItem(3, "Wednesday.sched", -1);
	ListCtrl2->InsertItem(4, "Thursday.sched", -1);
	ListCtrl2->InsertItem(5, "Friday.sched", -1);
	ListCtrl2->InsertItem(6, "Saturday.sched", -1);

}



void EasyLights_Schedule_Dialog::Load_Schedule_to_Work_on()
{
	wxString FN = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, Sched_Work);
	std::string fn = FN;

	FILE *fo = fopen((const char *)fn.c_str(), "r");

	int FSZ, r;
	char *B;
	char *p, *JF, *JV;
	char *EOL;
	std::string PL;
	std::string StartTM;
	std::string EndTM;
	std::string Loops;
	int Row = 0;

	int TM;
	char TMBuff[50];


	if(fo == 0)
	{
		return;
	}

	fseek(fo, 0L, SEEK_END);
	FSZ = ftell(fo);
	fseek(fo, 0L, SEEK_SET);

	B = (char*)malloc(FSZ + 5);
	if(B == 0)
	{
		fclose(fo);
		wxMessageBox(wxT("Failed to Acquire Memory for Schedule"), wxT("File Edit"), wxOK, NULL);

		return;
	}

	memset(B, 0, FSZ + 3);

	r = fread(B, 1, FSZ + 2, fo);

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

		PL = "";
		StartTM = "";
		EndTM = "";
		Loops = "0";

		while(p)	// get all wanted fields
		{
			p = Get_Next_JArg(p, &JF, &JV, 0, 1);

			if(p)
			{
				switch(*JF)
				{
				case 'S':	// sequence file name
					StartTM = JV;
					break;

				case 'E':
					EndTM = JV;
					break;

				case 'P':
					PL = JV;
					break;

				case 'L':
					Loops = JV;
					break;


				}

			}	// end of if p
		} // end of while fields

		// we have a jason package decoded

		Grid1->InsertRows(Row, 1);
		Grid1->SetCellValue(Row, 0, PL);
		Grid1->SetReadOnly(Row, 0, true);

		TM = atoi(StartTM.c_str());
		if(TM <= 0)
			TM = 1200;
		sprintf(TMBuff, "%d", TM);
		Grid1->SetCellValue(Row, 1, TMBuff);

		TM = atoi(EndTM.c_str());
		if(TM <= 0)
			TM = 1200;
		sprintf(TMBuff, "%d", TM);
		Grid1->SetCellValue(Row, 2, TMBuff);

		TM = atoi(Loops.c_str());
		if(TM < 0)
			TM = 0;
		sprintf(TMBuff, "%d", TM);
		Grid1->SetCellValue(Row, 3, TMBuff);

		Row++;

	}

}


void EasyLights_Schedule_Dialog::Save_Work_on_Schedule()
{
	wxString FN = wxString::Format("%s\\EasyLightsData\\%s", EL_Ptr->ShowDirectory, Sched_Work);
	std::string fn = FN;
	std::string T0;
	std::string T1;
	std::string T2;
	std::string T3;

	char TMBuff[20];
	int TM;

	FILE *fo = fopen((const char *)fn.c_str(), "w");

	if(fo == 0)
	{
		wxMessageBox(wxT("Failed to Save the Schedule"), wxT("File Save"), wxOK, NULL);

		return;
	}

	int i = Grid1->GetNumberRows();
	int j;

	if(i == 0)	// empty sched
	{
		fclose(fo);
		return;
	}

	fprintf(fo, "[");


	for(j = 0; j < i; j++)
	{
		T0 = Grid1->GetCellValue(j, 0);
		T1 = Grid1->GetCellValue(j, 1);
		T2 = Grid1->GetCellValue(j, 2);
		T3 = Grid1->GetCellValue(j, 3);


		TM = atoi(T1.c_str());
		if(TM <= 0)
			TM = 1200;
		if(TM > 2355)
			TM = 2355;
		sprintf(TMBuff, "%d", TM);
		T1 = TMBuff;

		TM = atoi(T2.c_str());
		if(TM <= 0)
			TM = 1200;
		if(TM > 2355)
			TM = 2355;
		sprintf(TMBuff, "%d", TM);
		T2 = TMBuff;

		TM = atoi(T3.c_str());
		if(TM < 0)
			TM = 0;
		sprintf(TMBuff, "%d", TM);
		T3 = TMBuff;


		fprintf(fo, "{\"PS\":\"%s\",\"S\":%s,\"E\":%s,\"L\":%s}", T0.c_str(), T1.c_str(), T2.c_str(), T3.c_str());

		if(j < (i - 1))
			fprintf(fo, ",");
	}

	fprintf(fo, "]");

	fclose(fo);

}
