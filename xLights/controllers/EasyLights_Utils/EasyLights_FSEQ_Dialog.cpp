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
#include "EasyLights_FSEQ_Dialog.h"
#include "../../xlightsMain.h"


//(*IdInit(EasyLights_FSEQ_Dialog)
const long EasyLights_FSEQ_Dialog::ID_STATICTEXT1 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_STATICTEXT2 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_GRID1 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_LISTCTRL1 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_BUTTON1 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_BUTTON2 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_BUTTON3 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_BUTTON4 = wxNewId();
const long EasyLights_FSEQ_Dialog::ID_TEXTCTRL1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EasyLights_FSEQ_Dialog,wxDialog)
	//(*EventTable(EasyLights_FSEQ_Dialog)
	EVT_GRID_SELECT_CELL(EasyLights_FSEQ_Dialog::OnGridCellLeftClick)
	EVT_INIT_DIALOG(EasyLights_FSEQ_Dialog::InitDialog)
	//*)
END_EVENT_TABLE()

EasyLights_FSEQ_Dialog *EasyLights_FSEQ_Dialog_Ptr = 0;

EasyLights_FSEQ_Dialog::EasyLights_FSEQ_Dialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	Quit_Now = 0;
	FSEQ_Busy = 0;
	Button_Active = 0;
	Process_Selected_Active = 0;


	EasyLights_FSEQ_Dialog_Ptr = this;


	BuildContent(parent,id,pos,size);

	Grid1->SetRowLabelSize(0);
	Grid1->HideCol(0);
	Grid1->HideCol(1);
	Grid1->SetColSize(2, 200);
	Grid1->SetColSize(3, 100);
	Grid1->SetColSize(4, 150);
	Grid1->SetColSize(5, 100);
	Grid1->SetColSize(6, 100);

	Connect(wxID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_FSEQ_Dialog::OnButtonCloseClick);
	Connect(wxID_ANY, wxEVT_CLOSE_WINDOW, (wxObjectEventFunction)&EasyLights_FSEQ_Dialog::OnButtonCloseClick);
	Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_FSEQ_Dialog::OnButtonCloseClick);
	Connect(ID_BUTTON2, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_FSEQ_Dialog::OnButton_Fseq_Selected);
	Connect(ID_BUTTON3, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_FSEQ_Dialog::OnButton_Fseq_All);
	Connect(ID_BUTTON4, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&EasyLights_FSEQ_Dialog::OnButton_Fseq_Required);

	Set_Busy_Status(0);

}

void EasyLights_FSEQ_Dialog::BuildContent(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EasyLights_FSEQ_Dialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, wxID_ANY, _("EasyLights FSEQ Management"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Xlights FSEQ related to EasyLights StandAlone FSEQ by Controller"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Status / Build Information"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer1->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Grid1 = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, wxVSCROLL, _T("ID_GRID1"));
	Grid1->CreateGrid(0,7);
	Grid1->EnableEditing(false);
	Grid1->EnableGridLines(true);
	Grid1->SetColLabelValue(0, _("Hidden"));
	Grid1->SetColLabelValue(1, _("Hidden"));
	Grid1->SetColLabelValue(2, _("Xlights FSEQ"));
	Grid1->SetColLabelValue(3, _("FSEQ Date"));
	Grid1->SetColLabelValue(4, _("EL Controller Name"));
	Grid1->SetColLabelValue(5, _("ELC IP Address"));
	Grid1->SetColLabelValue(6, _("ELC FSEQ Date"));
	Grid1->SetDefaultCellFont( Grid1->GetFont() );
	Grid1->SetDefaultCellTextColour( Grid1->GetForegroundColour() );
	FlexGridSizer1->Add(Grid1, 1, wxALL, 5);
	ListCtrl1 = new wxListCtrl(this, ID_LISTCTRL1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT|wxLC_NO_HEADER|wxVSCROLL|wxHSCROLL, wxDefaultValidator, _T("ID_LISTCTRL1"));
	ListCtrl1->SetMinSize(wxSize(250,400));
	ListCtrl1->SetMaxSize(wxSize(-1,-1));
	FlexGridSizer1->Add(ListCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 4, 0, 0);
	Button1 = new wxButton(this, ID_BUTTON1, _("Quit"), wxDefaultPosition, wxSize(150,25), 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer2->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button2 = new wxButton(this, ID_BUTTON2, _("Update Selected"), wxDefaultPosition, wxSize(150,25), 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("Update ALL"), wxDefaultPosition, wxSize(150,25), 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer2->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button4 = new wxButton(this, ID_BUTTON4, _("Update only those Required"), wxDefaultPosition, wxSize(150,25), 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer2->Add(Button4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl1 = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_READONLY, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	TextCtrl1->SetMinSize(wxSize(200,25));
	FlexGridSizer1->Add(TextCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
}

EasyLights_FSEQ_Dialog::~EasyLights_FSEQ_Dialog()
{
	//(*Destroy(EasyLights_FSEQ_Dialog)
	//*)
}


void EasyLights_FSEQ_Dialog::InitDialog(wxInitDialogEvent &WXUNUSED(event))
{
	Get_All_Xlights_FSEQ_Files();
}

void EasyLights_FSEQ_Dialog::Set_Busy_Status(int Busy)
{
	wxColour CR(255, 150, 150);
	wxColour CG(150, 255, 150);
	std::string T;

	FSEQ_Busy = Busy;

	if(Busy)
	{
		TextCtrl1->SetBackgroundColour(CR);
		TextCtrl1->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

		T = "Busy Working";	// if blank color won't change
		TextCtrl1->SetValue(T);
		TextCtrl1->Refresh();
		TextCtrl1->Update();

	}
	else
	{
		TextCtrl1->SetBackgroundColour(CG);
		TextCtrl1->SetStyle(0, -1, TextCtrl1->GetDefaultStyle());

		T = "Waiting for a Request";	// if blank color won't change
		TextCtrl1->SetValue(T);
		TextCtrl1->Refresh();
		TextCtrl1->Update();

	}




}

void EasyLights_FSEQ_Dialog::OnButtonCloseClick(wxCommandEvent& event)
{
	Quit_Now = 1;

	while(FSEQ_Busy)
	{
		wxMilliSleep(100);
	}

	// release memory acquired

	for(auto e = FSEQ_Array.begin(); e != FSEQ_Array.end(); ++e)
		delete *e;

	for(auto x = FSEQ_Array_Xlights.begin(); x != FSEQ_Array_Xlights.end(); ++x)
		delete *x;

	FSEQ_Array.clear();
	FSEQ_Array_Xlights.clear();

	EasyLights_FSEQ_Dialog_Ptr = 0;

	EndDialog(0);

}


void EasyLights_FSEQ_Dialog::OnButton_Fseq_Selected(wxCommandEvent& event)
{
	if(Button_Active)
		return;

	Button_Active = 1;

	Process_Selected_Active = 1;

	Button_Active = 0;

	OnButton_Fseq_Required(event);	// this rebuilds as required


}

void EasyLights_FSEQ_Dialog::OnButton_Fseq_All(wxCommandEvent& event)
{
	wxString T;

	if(Button_Active)
		return;

	Button_Active = 1;

	if(wxNO == wxMessageBox("Are you Sure you want to Delete all the EasyLights FSEQs and Rebuild them", "Delete and Rebuild All", wxICON_QUESTION | wxYES_NO))
	{
		Button_Active = 0;

		return;
	}

	Set_Busy_Status(1);

	int i = 0;

	for(auto e = FSEQ_Array.begin(); e != FSEQ_Array.end(); ++e, i++)
	{
		if(Quit_Now)
			break;

		if((*e)->ELC_FN_DT_Value != 0)	// valid file
		{
			T = wxString::Format("Removing %s", (*e)->ELC_FN_short);
			Write_Msg_to_Display(T);

			wxRemove((*e)->ELC_FN_long);
			(*e)->ELC_FN_DT_Value = 0;
			(*e)->ELC_FN_DT = 0;
			Grid1->SetCellValue(i, 6, "");
		}
	}

// now remove PCM files

	std::string FN_search;
	std::string FN;


	FN_search = EL_Ptr->ShowDirectory + "\\EasyLightsData\\*.wav";

	FN = wxFindFirstFile(FN_search);

	while(!FN.empty())
	{
		T = wxString::Format("Removing %s", FN);
		Write_Msg_to_Display(T);

		wxRemove(FN);
		FN = wxFindNextFile();
	}



	Button_Active = 0;

	OnButton_Fseq_Required(event);	// this rebuilds as required


}

void EasyLights_FSEQ_Dialog::OnButton_Fseq_Required(wxCommandEvent& event)
{
	int i = 0;
	wxString T;
	std::string EL_Dir_Path;
	std::string EL_WAV;
	std::string EL_PCM;
	wxFileName FN;
	int r;
	int Create_WAV = 0;
	uint32_t Size_PCM_Bytes;
	uint32_t Sample_Rate_Used;
	wxColour CW(255, 255, 255);



	if(Button_Active)
		return;

	Button_Active = 1;
	Set_Busy_Status(1);

	EL_Dir_Path = EL_Ptr->ShowDirectory + "\\EasyLightsData\\";


	for(auto e = FSEQ_Array.begin(); e != FSEQ_Array.end(); ++e, i++)
	{
		if(Quit_Now)
			break;

		if(Process_Selected_Active && (*e)->Process == 0)
			continue;

		if(Process_Selected_Active && (*e)->Process)
		{
			(*e)->Process = 0;	// kill process flag as we are doing it
			Grid1->SetCellBackgroundColour((*e)->Grid_Row, 2, CW);
			Grid1->SetCellValue((*e)->Grid_Row, 2, (*e)->IX->FN_short);
		}

		if(Process_Selected_Active || ((*e)->ELC_FN_DT_Value < (*e)->IX->FN_DT_Value))	// xlights fseq newer
		{
			if((*e)->ELC_FN_DT_Value != 0)	// valid file
			{
				T = wxString::Format("Removing %s", (*e)->ELC_FN_short);
				Write_Msg_to_Display(T);

				wxRemove((*e)->ELC_FN_long);
				(*e)->ELC_FN_DT_Value = 0;
				(*e)->ELC_FN_DT = 0;
				Grid1->SetCellValue(i, 6, "");
			}

			// check if audio is needed

			FN = EL_Dir_Path + (*e)->IX->FN_short + ".wav";

			EL_PCM = EL_Dir_Path + "PCM";

			Create_WAV = 0;

			if(!FN.Exists())	// we need to make it
				Create_WAV = 1;
			else if(FN.GetSize() < 2048)	// previous conversion failed
				Create_WAV = 1;


			if(Create_WAV || Process_Selected_Active)	// we need to make it
			{
				T = wxString::Format("Creating Audio PCM file for %s.fseq", (*e)->IX->FN_short);
				Write_Msg_to_Display(T);

				EL_WAV = FN.GetFullPath();

				r = Create_EasyLights_PCM_File((*e)->IX->FN_long, EL_PCM, this, &Size_PCM_Bytes, &Sample_Rate_Used);	// path to xlights fseq and wxfilename to pcm

				if(r == 0)
				{
					r = Make_WAV_From_PCM(EL_PCM, EL_WAV, Size_PCM_Bytes, Sample_Rate_Used);

					if(r == 0)
					{
						Write_Msg_to_Display("ELC Audio File Completed - no Errors");
					}
				}

			}

			// check if we need to build a EL fseq

			FN = (*e)->ELC_FN_long;

			if(!FN.Exists())	// we need to make it
			{
				r = Create_EasyLights_FSEQ_File(*e, this);	// path to xlights fseq and wxfilename to pcm
				if(r == 0)
				{
					(*e)->ELC_FN_DT = FN.GetModificationTime();

					if((*e)->ELC_FN_DT.IsValid())
						(*e)->ELC_FN_DT_Value = (*e)->ELC_FN_DT.GetValue();
					else
						(*e)->ELC_FN_DT_Value = 0;	// should never happen

					if((*e)->ELC_FN_DT_Value != 0)
						Grid1->SetCellValue((*e)->Grid_Row, 6, (*e)->ELC_FN_DT.FormatDate());


					Write_Msg_to_Display("ELC FSEQ File Completed - no Errors");
				}



			}



		}
	}




	Process_Selected_Active = 0;
	Set_Busy_Status(0);
	Button_Active = 0;

}



void EasyLights_FSEQ_Dialog::Get_All_Xlights_FSEQ_Files()
{

	wxFileName fseq_file = "*.fseq";
	std::string FN;
	std::string fsearch;
	EL_FSEQ_Info *I;
	EL_FSEQ_Info_Xlights *IX;
	std::string EL_Dir_Path;
	wxFileName ELC_fseq;
	std::string IPdash;
	int i;

	// set up list display

	wxListItem itemCol;

	itemCol.SetText(wxT("Name"));
	itemCol.SetImage(-1);
	ListCtrl1->InsertColumn(0, itemCol);
	ListCtrl1->SetColumnWidth(0, 1000);


	Write_Msg_to_Display("Fseq Searching Now");

	// Only Look for FSEQ file in FSEQ FOLDER, if folder are unlinked
	if(wxFileName(EL_Ptr->FseqDirectory) != wxFileName(EL_Ptr->ShowDirectory))
	{
		fseq_file.SetPath(EL_Ptr->FseqDirectory);
	}
	else
	{
		fseq_file.SetPath(EL_Ptr->ShowDirectory);
	}


	fsearch = fseq_file.GetFullPath();

	FN = wxFindFirstFile(fsearch);

/***
	std::string FN_long;
	std::string FN_short;
	wxDateTime FN_DT;
	int FN_DT_Value;

	
	EL_FSEQ_Info_Xlights *X;
	std::string ELC_IP;
	std::string ELC_FN_long;
	std::string ELC_FN_short;
	wxDateTime ELC_FN_DT;
	wxLongLong ELC_FN_DT_Value;
**/

	while(!FN.empty())
	{
		fseq_file = FN;

		IX = new EL_FSEQ_Info_Xlights;
		IX->FN_long = FN;
		IX->FN_short = fseq_file.GetName();

		IX->Version = Return_FSEQ_Version(FN);

		IX->FN_DT = fseq_file.GetModificationTime();

		if(IX->FN_DT.IsValid())
			IX->FN_DT_Value = IX->FN_DT.GetValue();
		else
			IX->FN_DT_Value = 0;	// should never happen

		FSEQ_Array_Xlights.push_back(IX);

		// now add ELC's

		for(auto it = EL_Ptr->EL_EndPoints.begin(); it != EL_Ptr->EL_EndPoints.end(); ++it)
		{
			if((*it)->Model != "Undefined")	// we have a ELC
			{
				I = new EL_FSEQ_Info;
				I->IX = IX;
				I->EP = *it;
				I->ELC_IP = (*it)->IP;
				I->ELC_FN_DT = 0;
				I->ELC_FN_DT_Value = 0;
				I->ELC_Name = (*it)->Name;
				I->Process = 0;
				I->Grid_Row = 0;

				FSEQ_Array.push_back(I);
			}
		}

		FN = wxFindNextFile();
	}

	// get here with all Xlights FSEQs in and array and an array of ELC fseq that may need to be created

	EL_Dir_Path = EL_Ptr->ShowDirectory + "\\EasyLightsData\\";

	for(auto e = FSEQ_Array.begin(); e != FSEQ_Array.end(); ++e)
	{
		// construct base file name for this IP's fseq file

		IPdash = (*e)->ELC_IP;

		for(i = 0; i < IPdash.size(); i++)
		{
			if(IPdash[i] == '.')
				IPdash[i] = '_';
		}

		FN = (*e)->IX->FN_short + " " + IPdash + ".fseq";

		ELC_fseq = EL_Dir_Path + FN;

		// test for well formed file name

		if(!ELC_fseq.IsOk())
		{
			wxString Z = wxString::Format("Bad File Name %s", ELC_fseq.GetFullPath());
			Write_Msg_to_Display(Z);
			continue;
		}

		(*e)->ELC_FN_short = FN;

		(*e)->ELC_FN_long = ELC_fseq.GetFullPath();

		if(ELC_fseq.FileExists())	// we have a real file out there, get values
		{
			(*e)->ELC_FN_DT = ELC_fseq.GetModificationTime();

			if((*e)->ELC_FN_DT.IsValid())
				(*e)->ELC_FN_DT_Value = (*e)->ELC_FN_DT.GetValue();
			else
				(*e)->ELC_FN_DT_Value = 0;	// should never happen
		}
		else
		{
			(*e)->ELC_FN_DT = 0;
			(*e)->ELC_FN_DT_Value = 0;
		}


	}

	// we can now build the grid

	i = 0;


	//wxColour CR(255, 150, 150);





	for(auto e = FSEQ_Array.begin(); e != FSEQ_Array.end(); ++e, i++)
	{
		// have two hidden rows - not using them right now -- using vector instead

		Grid1->InsertRows(i, 1);
		Grid1->SetCellValue(i, 2, (*e)->IX->FN_short);
		Grid1->SetCellValue(i, 3, (*e)->IX->FN_DT.FormatDate());
		Grid1->SetCellValue(i, 4, (*e)->ELC_Name);
		Grid1->SetCellValue(i, 5, (*e)->ELC_IP);

		(*e)->Grid_Row = i;

		//if((*e)->IX->Version != 1)
		//	Grid1->SetCellBackgroundColour(i, 2, CR);

		if((*e)->ELC_FN_DT_Value != 0)
			Grid1->SetCellValue(i, 6, (*e)->ELC_FN_DT.FormatDate());
	}
}


void EasyLights_FSEQ_Dialog::Write_Msg_to_Display(char *S)
{
	int r = ListCtrl1->GetItemCount();

	ListCtrl1->InsertItem(r, S);
}

void EasyLights_FSEQ_Dialog::Write_Msg_to_Display(std::string &S)
{
	int r = ListCtrl1->GetItemCount();

	ListCtrl1->InsertItem(r, S);

}


void EasyLights_FSEQ_Dialog::Write_Msg_to_Display(wxString &S)
{
	int r = ListCtrl1->GetItemCount();

	ListCtrl1->InsertItem(r, S);

}


void EasyLights_FSEQ_Dialog::OnGridCellLeftClick(wxGridEvent &event)
{
	int Row = event.GetRow();
	int Col = event.GetCol();
	wxColour CB(200, 200, 255);
	wxColour CW(255, 255, 255);
	EL_FSEQ_Info *e;


	if(Col != 2)
		return;


	if(Row < 0 || Row > EasyLights_FSEQ_Dialog_Ptr->FSEQ_Array.size())
		return;

	e = EasyLights_FSEQ_Dialog_Ptr->FSEQ_Array[Row];

	if(e->Process == 0)
	{
		e->Process = 1;

		EasyLights_FSEQ_Dialog_Ptr->Grid1->SetCellBackgroundColour(Row, 2, CB);
		EasyLights_FSEQ_Dialog_Ptr->Grid1->SetCellValue(Row, 2, e->IX->FN_short);
	}
	else
	{
		e->Process = 0;

		EasyLights_FSEQ_Dialog_Ptr->Grid1->SetCellBackgroundColour(Row, 2, CW);
		EasyLights_FSEQ_Dialog_Ptr->Grid1->SetCellValue(Row, 2, e->IX->FN_short);
	}


}