#include "GenerateLyricsDialog.h"

//(*InternalHeaders(GenerateLyricsDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "../include/AI.xpm"
#include "../include/E.xpm"
#include "../include/ETC.xpm"
#include "../include/FV.xpm"
#include "../include/L.xpm"
#include "../include/MBP.xpm"
#include "../include/O.xpm"
#include "../include/REST.xpm"
#include "../include/U.xpm"
#include "../include/WQ.xpm"

#include <wx/dcclient.h>
#include "NodesGridCellEditor.h"
#include <wx/wfstream.h>
#include <wx/txtstrm.h>

#ifndef wxEVT_GRID_CELL_CHANGE
//until CodeBlocks is updated to wxWidgets 3.x
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED
#endif

//(*IdInit(GenerateLyricsDialog)
const long GenerateLyricsDialog::ID_STATICTEXT1 = wxNewId();
const long GenerateLyricsDialog::ID_STATICTEXT3 = wxNewId();
const long GenerateLyricsDialog::ID_TEXTCTRL1 = wxNewId();
const long GenerateLyricsDialog::ID_STATICTEXT2 = wxNewId();
const long GenerateLyricsDialog::ID_SPINCTRL1 = wxNewId();
const long GenerateLyricsDialog::ID_PANEL1 = wxNewId();
const long GenerateLyricsDialog::ID_GRID_COROFACES = wxNewId();
const long GenerateLyricsDialog::ID_BUTTON3 = wxNewId();
const long GenerateLyricsDialog::ID_BUTTON4 = wxNewId();
const long GenerateLyricsDialog::ID_BUTTON1 = wxNewId();
const long GenerateLyricsDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(GenerateLyricsDialog,wxDialog)
	//(*EventTable(GenerateLyricsDialog)
	//*)
END_EVENT_TABLE()

GenerateLyricsDialog::GenerateLyricsDialog(wxWindow* parent, long channels, wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(GenerateLyricsDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;

	Create(parent, id, _("Generate Lyrics From Data"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("This tool will read data (typically from a data layer) and generate \na lyric phoneme track from that data. Typically this data would come \nfrom an old sequence which had 7 or 8 channel singing faces.\n\nJust as you would when setting up faces select as many nodes as you \nneed to represent the face. If multiple phenomes light up the same \nnodes then only the first one will be used ... so choose wisely.\n\nWARNING: Your sequence must be rendered for this tool to work correctly."), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer1->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer2->AddGrowableCol(1);
	StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _("Timing Name:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer2->Add(TextCtrl_Name, 1, wxALL|wxEXPAND, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("First Channel (absolute):"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_FirstChannel = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL1"));
	SpinCtrl_FirstChannel->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_FirstChannel, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer4->AddGrowableCol(1);
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxSize(36,0), wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer4->Add(Panel1, 1, wxALL|wxEXPAND, 5);
	SingleNodeGrid = new wxGrid(this, ID_GRID_COROFACES, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID_COROFACES"));
	SingleNodeGrid->CreateGrid(10,1);
	SingleNodeGrid->SetMinSize(wxDLG_UNIT(this,wxSize(-1,130)));
	SingleNodeGrid->EnableEditing(true);
	SingleNodeGrid->EnableGridLines(true);
	SingleNodeGrid->SetColLabelSize(20);
	SingleNodeGrid->SetRowLabelSize(150);
	SingleNodeGrid->SetDefaultColSize(250, true);
	SingleNodeGrid->SetColLabelValue(0, _("Node Offsets From First Channel"));
	SingleNodeGrid->SetRowLabelValue(0, _("Mouth - AI"));
	SingleNodeGrid->SetRowLabelValue(1, _("Mouth - E"));
	SingleNodeGrid->SetRowLabelValue(2, _("Mouth - etc"));
	SingleNodeGrid->SetRowLabelValue(3, _("Mouth - FV"));
	SingleNodeGrid->SetRowLabelValue(4, _("Mouth - L"));
	SingleNodeGrid->SetRowLabelValue(5, _("Mouth - MBP"));
	SingleNodeGrid->SetRowLabelValue(6, _("Mouth - O"));
	SingleNodeGrid->SetRowLabelValue(7, _("Mouth - rest"));
	SingleNodeGrid->SetRowLabelValue(8, _("Mouth - U"));
	SingleNodeGrid->SetRowLabelValue(9, _("Mouth - WQ"));
	SingleNodeGrid->SetDefaultCellFont( SingleNodeGrid->GetFont() );
	SingleNodeGrid->SetDefaultCellTextColour( SingleNodeGrid->GetForegroundColour() );
	FlexGridSizer4->Add(SingleNodeGrid, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 5, 0, 0);
	FlexGridSizer3->AddGrowableCol(2);
	Button_Save = new wxButton(this, ID_BUTTON3, _("Save Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer3->Add(Button_Save, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Load = new wxButton(this, ID_BUTTON4, _("Load Mapping"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	FlexGridSizer3->Add(Button_Load, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer3->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer3->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	Center();

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&GenerateLyricsDialog::OnTextCtrl_NameText);
	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&GenerateLyricsDialog::OnSpinCtrl_FirstChannelChange);
	Panel1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&GenerateLyricsDialog::Paint,0,this);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&GenerateLyricsDialog::OnSingleNodeGridCellLeftClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&GenerateLyricsDialog::OnSingleNodeGridCellLeftDClick);
	Connect(ID_GRID_COROFACES,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&GenerateLyricsDialog::OnSingleNodeGridCellSelect);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateLyricsDialog::OnButton_SaveClick);
	Connect(ID_BUTTON4,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateLyricsDialog::OnButton_LoadClick);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateLyricsDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&GenerateLyricsDialog::OnButton_CancelClick);
	//*)

    SpinCtrl_FirstChannel->SetRange(1,channels);

    wxArrayString names;
    names.push_back("");
    for (int x = 0; x < 32; x++) {
        names.push_back(wxString::Format("+%i", x));
    }

    NodesGridCellEditor *editor = new NodesGridCellEditor();
    editor->names = names;
    SingleNodeGrid->SetDefaultEditor(editor);

    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        SingleNodeGrid->SetReadOnly(x, 1);
    }

    SetEscapeId(Button_Cancel->GetId());

    ValidateWindow();
}

GenerateLyricsDialog::~GenerateLyricsDialog()
{
	//(*Destroy(GenerateLyricsDialog)
	//*)
}

std::list<long> GenerateLyricsDialog::GetChannels(const std::string& phenome) const
{
    std::list<long> res;
    std::string label = "Mouth - " + phenome;

    for (int x = 0; x < SingleNodeGrid->GetNumberRows(); x++) {
        if (SingleNodeGrid->GetRowLabelValue(x) == label)
        {
            wxString value = SingleNodeGrid->GetCellValue(x, 0);

            wxArrayString nodes = wxSplit(value, ',');

            for (auto it = nodes.begin(); it != nodes.end(); ++it)
            {
                long node = SpinCtrl_FirstChannel->GetValue() + wxAtoi(it->SubString(1, it->size()));
                res.push_back(node);
            }

            break;
        }
    }

    return res;
}

std::string GenerateLyricsDialog::GetLyricName() const
{
    return TextCtrl_Name->GetValue().Trim(false).Trim(true).ToStdString();
}

void GenerateLyricsDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void GenerateLyricsDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void GenerateLyricsDialog::OnSingleNodeGridCellLeftClick(wxGridEvent& event)
{
    event.ResumePropagation(1);
    event.Skip();
}

void GenerateLyricsDialog::OnSingleNodeGridCellLeftDClick(wxGridEvent& event)
{
}

void GenerateLyricsDialog::OnSingleNodeGridCellChange(wxGridEvent& event)
{
}

void GenerateLyricsDialog::OnSingleNodeGridCellSelect(wxGridEvent& event)
{
    event.ResumePropagation(1);
    event.Skip();
}

void GenerateLyricsDialog::OnSpinCtrl_FirstChannelChange(wxSpinEvent& event)
{
}

void GenerateLyricsDialog::OnTextCtrl_NameText(wxCommandEvent& event)
{
    ValidateWindow();
}

void GenerateLyricsDialog::ValidateWindow()
{
    if (TextCtrl_Name->GetValue().Trim(true).Trim(false) == "")
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}

void GenerateLyricsDialog::Paint(wxPaintEvent& event)
{
    wxPanel* p = (wxPanel*)event.GetEventObject();
    wxPaintDC dc(p);

    int x = 2;
    int y = SingleNodeGrid->GetColLabelSize();

    for (int i = 0; i < SingleNodeGrid->GetNumberRows(); i++)
    {
        wxString CellValue = SingleNodeGrid->GetRowLabelValue(i);

        if (CellValue.EndsWith("AI"))
        {
            PaintFace(dc, x, y, AI_xpm);
        }
        else if (CellValue.EndsWith("E"))
        {
            PaintFace(dc, x, y, E_xpm);
        }
        else if (CellValue.EndsWith("etc"))
        {
            PaintFace(dc, x, y, ETC_xpm);
        }
        else if (CellValue.EndsWith("FV"))
        {
            PaintFace(dc, x, y, FV_xpm);
        }
        else if (CellValue.EndsWith("L"))
        {
            PaintFace(dc, x, y, L_xpm);
        }
        else if (CellValue.EndsWith("MBP"))
        {
            PaintFace(dc, x, y, MBP_xpm);
        }
        else if (CellValue.EndsWith("O"))
        {
            PaintFace(dc, x, y, O_xpm);
        }
        else if (CellValue.EndsWith("rest"))
        {
            PaintFace(dc, x, y, REST_xpm);
        }
        else if (CellValue.EndsWith("U"))
        {
            PaintFace(dc, x, y, U_xpm);
        }
        else if (CellValue.EndsWith("WQ"))
        {
            PaintFace(dc, x, y, WQ_xpm);
        }
        y += SingleNodeGrid->GetRowHeight(i);
    }
}

void GenerateLyricsDialog::PaintFace(wxDC& dc, int x, int y, const char* xpm[])
{
    wxImage i(xpm);
    wxBitmap bmp(i);

    dc.DrawBitmap(bmp, x, y);
}

void GenerateLyricsDialog::OnButton_SaveClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Save mapping", wxEmptyString, "mapping", "Mapping Files (*.xfacemap)|*.xfacemap", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
    if (dlg.ShowModal() == wxID_OK) {
        wxFileOutputStream output(dlg.GetPath());
        wxTextOutputStream text(output);
        
        text.WriteString(wxString::Format("firstchannel\t%d\n", SpinCtrl_FirstChannel->GetValue()));

        for (int i = 0; i < SingleNodeGrid->GetNumberRows(); i++)
        {
            text.WriteString(wxString::Format("%s\t%s\n", SingleNodeGrid->GetRowLabelValue(i), ((SingleNodeGrid->GetCellValue(i,0) == "") ? "NULL" : SingleNodeGrid->GetCellValue(i, 0))));
        }
    }
}

void GenerateLyricsDialog::OnButton_LoadClick(wxCommandEvent& event)
{
    wxFileDialog dlg(this, "Load mapping", wxEmptyString, wxEmptyString, "Mapping Files (*.xfacemap)|*.xfacemap", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (dlg.ShowModal() == wxID_OK) {

        wxFileInputStream input(dlg.GetPath());
        wxTextInputStream text(input, "\t");

        wxString word = text.ReadWord();
        while (word != "") {
            if (word == "firstchannel")
            {
                int fc = wxAtoi(text.ReadWord());
                SpinCtrl_FirstChannel->SetValue(fc);
            }
            else
            {
                for (int i = 0; i < SingleNodeGrid->GetNumberRows(); i++)
                {
                    if (SingleNodeGrid->GetRowLabelValue(i) == word)
                    {
                        wxString map = text.ReadWord();
                        if (map == "NULL")
                        {
                            SingleNodeGrid->SetCellValue(i, 0, "");
                        }
                        else
                        {
                            SingleNodeGrid->SetCellValue(i, 0, map);
                        }
                        break;
                    }
                }
            }
            word = text.ReadWord();
        }
    }
}