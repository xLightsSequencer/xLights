#include "ColorGradientDialog.h"

//(*InternalHeaders(ColorGradientDialog)
#include <wx/intl.h>
#include <wx/button.h>
#include <wx/string.h>
//*)

//(*IdInit(ColorGradientDialog)
const long ColorGradientDialog::ID_PANEL1 = wxNewId();
const long ColorGradientDialog::ID_GRID1 = wxNewId();
const long ColorGradientDialog::ID_CHOICE1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ColorGradientDialog,wxDialog)
	//(*EventTable(ColorGradientDialog)
	//*)
END_EVENT_TABLE()

// work around wxWIDGETS bug
#define wxEVT_GRID_CELL_CHANGE wxEVT_GRID_CELL_CHANGED

ColorGradientDialog::ColorGradientDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(ColorGradientDialog)
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer1;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;

	Create(parent, id, _("Color Curve"), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	Panel1 = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2->Add(Panel1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	GridColours = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_GRID1"));
	GridColours->CreateGrid(10,4);
	GridColours->EnableEditing(true);
	GridColours->EnableGridLines(true);
	GridColours->SetColLabelValue(0, _("Color"));
	GridColours->SetColLabelValue(1, _("Start"));
	GridColours->SetColLabelValue(2, _("Fade In"));
	GridColours->SetColLabelValue(3, _("Fade Out"));
	GridColours->SetDefaultCellFont( GridColours->GetFont() );
	GridColours->SetDefaultCellTextColour( GridColours->GetForegroundColour() );
	FlexGridSizer3->Add(GridColours, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	Choice1 = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
	Choice1->SetSelection( Choice1->Append(_("Gradient")) );
	Choice1->Append(_("Sharp"));
	Choice1->Append(_("Fade"));
	FlexGridSizer1->Add(Choice1, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Panel1->Connect(wxEVT_PAINT,(wxObjectEventFunction)&ColorGradientDialog::OnPanel1Paint,0,this);
	Panel1->Connect(wxEVT_LEFT_DOWN,(wxObjectEventFunction)&ColorGradientDialog::OnPanel1LeftDown,0,this);
	Panel1->Connect(wxEVT_LEFT_UP,(wxObjectEventFunction)&ColorGradientDialog::OnPanel1LeftUp,0,this);
	Panel1->Connect(wxEVT_MOTION,(wxObjectEventFunction)&ColorGradientDialog::OnPanel1MouseMove,0,this);
	Panel1->Connect(wxEVT_ENTER_WINDOW,(wxObjectEventFunction)&ColorGradientDialog::OnPanel1MouseEnter,0,this);
	Panel1->Connect(wxEVT_LEAVE_WINDOW,(wxObjectEventFunction)&ColorGradientDialog::OnPanel1MouseLeave,0,this);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&ColorGradientDialog::OnGridColoursCellLeftClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&ColorGradientDialog::OnGridColoursCellRightClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_LEFT_DCLICK,(wxObjectEventFunction)&ColorGradientDialog::OnGridColoursCellLeftDClick);
	Connect(ID_GRID1,wxEVT_GRID_LABEL_RIGHT_CLICK,(wxObjectEventFunction)&ColorGradientDialog::OnGridColoursLabelRightClick);
	Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ColorGradientDialog::OnGridColoursCellChange);
	Connect(ID_GRID1,wxEVT_GRID_SELECT_CELL,(wxObjectEventFunction)&ColorGradientDialog::OnGridColoursCellSelect);
	Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ColorGradientDialog::OnChoice1Select);
	Connect(wxEVT_SIZE,(wxObjectEventFunction)&ColorGradientDialog::OnResize);
	//*)
}

ColorGradientDialog::~ColorGradientDialog()
{
	//(*Destroy(ColorGradientDialog)
	//*)
}


void ColorGradientDialog::OnGridColoursCellChange(wxGridEvent& event)
{
}

void ColorGradientDialog::OnGridColoursCellLeftClick(wxGridEvent& event)
{
}

void ColorGradientDialog::OnGridColoursCellLeftDClick(wxGridEvent& event)
{
}

void ColorGradientDialog::OnGridColoursCellRightClick(wxGridEvent& event)
{
}

void ColorGradientDialog::OnGridColoursLabelRightClick(wxGridEvent& event)
{
}

void ColorGradientDialog::OnChoice1Select(wxCommandEvent& event)
{
}

void ColorGradientDialog::OnGridColoursCellSelect(wxGridEvent& event)
{
}

void ColorGradientDialog::OnPanel1Paint(wxPaintEvent& event)
{
}

void ColorGradientDialog::OnPanel1LeftDown(wxMouseEvent& event)
{
}

void ColorGradientDialog::OnPanel1LeftUp(wxMouseEvent& event)
{
}

void ColorGradientDialog::OnPanel1MouseMove(wxMouseEvent& event)
{
}

void ColorGradientDialog::OnPanel1MouseEnter(wxMouseEvent& event)
{
}

void ColorGradientDialog::OnPanel1MouseLeave(wxMouseEvent& event)
{
}

void ColorGradientDialog::OnResize(wxSizeEvent& event)
{
}
