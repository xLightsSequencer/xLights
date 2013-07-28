#include "ModelDialog.h"
#include "CustomModelDialog.h"
#include "ModelClass.h"
#include <wx/msgdlg.h>

//(*InternalHeaders(ModelDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(ModelDialog)
const long ModelDialog::ID_STATICTEXT1 = wxNewId();
const long ModelDialog::ID_TEXTCTRL1 = wxNewId();
const long ModelDialog::ID_STATICTEXT5 = wxNewId();
const long ModelDialog::ID_CHOICE1 = wxNewId();
const long ModelDialog::ID_STATICTEXT2 = wxNewId();
const long ModelDialog::ID_SPINCTRL1 = wxNewId();
const long ModelDialog::ID_STATICTEXT3 = wxNewId();
const long ModelDialog::ID_SPINCTRL2 = wxNewId();
const long ModelDialog::ID_STATICTEXT4 = wxNewId();
const long ModelDialog::ID_SPINCTRL3 = wxNewId();
const long ModelDialog::ID_STATICTEXT6 = wxNewId();
const long ModelDialog::ID_SPINCTRL4 = wxNewId();
const long ModelDialog::ID_STATICTEXT7 = wxNewId();
const long ModelDialog::ID_CHOICE2 = wxNewId();
const long ModelDialog::ID_STATICTEXT8 = wxNewId();
const long ModelDialog::ID_RADIOBUTTON1 = wxNewId();
const long ModelDialog::ID_RADIOBUTTON2 = wxNewId();
const long ModelDialog::ID_STATICTEXT11 = wxNewId();
const long ModelDialog::ID_RADIOBUTTON4 = wxNewId();
const long ModelDialog::ID_RADIOBUTTON3 = wxNewId();
const long ModelDialog::ID_STATICTEXT9 = wxNewId();
const long ModelDialog::ID_CHOICE3 = wxNewId();
const long ModelDialog::ID_STATICTEXT10 = wxNewId();
const long ModelDialog::ID_CHECKBOX1 = wxNewId();
const long ModelDialog::ID_STATICTEXT12 = wxNewId();
const long ModelDialog::ID_CHECKBOX2 = wxNewId();
const long ModelDialog::ID_STATICTEXT13 = wxNewId();
const long ModelDialog::ID_BUTTON1 = wxNewId();
const long ModelDialog::ID_GRID1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelDialog,wxDialog)
    //(*EventTable(ModelDialog)
    //*)
END_EVENT_TABLE()

ModelDialog::ModelDialog(wxWindow* parent,wxWindowID id)
{
    //(*Initialize(ModelDialog)
    wxFlexGridSizer* FlexGridSizer3;
    wxFlexGridSizer* FlexGridSizer2;
    wxBoxSizer* BoxSizer2;
    wxBoxSizer* BoxSizer1;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, wxID_ANY, _("Model"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
    SetHelpText(_("Pixels Start in the upper left and go right or down depending on Vertical or Horizontal orientation.  Trees are always Vertical."));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Model Name"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl_Name = new wxTextCtrl(this, ID_TEXTCTRL1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
    FlexGridSizer2->Add(TextCtrl_Name, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText5 = new wxStaticText(this, ID_STATICTEXT5, _("Display As"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
    FlexGridSizer2->Add(StaticText5, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_DisplayAs = new wxChoice(this, ID_CHOICE1, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE1"));
    Choice_DisplayAs->SetSelection( Choice_DisplayAs->Append(_("Tree 360")) );
    Choice_DisplayAs->Append(_("Tree 270"));
    Choice_DisplayAs->Append(_("Tree 180"));
    Choice_DisplayAs->Append(_("Tree 90"));
    Choice_DisplayAs->Append(_("Vert Matrix"));
    Choice_DisplayAs->Append(_("Horiz Matrix"));
    Choice_DisplayAs->Append(_("Single Line"));
    Choice_DisplayAs->Append(_("Arches"));
    Choice_DisplayAs->Append(_("Window Frame"));
    Choice_DisplayAs->Append(_("Custom"));
    FlexGridSizer2->Add(Choice_DisplayAs, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Strings = new wxStaticText(this, ID_STATICTEXT2, _("Actual # of Strings"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText_Strings, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_parm1 = new wxSpinCtrl(this, ID_SPINCTRL1, _T("1"), wxDefaultPosition, wxSize(60,21), 0, 1, 100, 1, _T("ID_SPINCTRL1"));
    SpinCtrl_parm1->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_parm1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Nodes = new wxStaticText(this, ID_STATICTEXT3, _("# of RGB Nodes per String"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText_Nodes, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_parm2 = new wxSpinCtrl(this, ID_SPINCTRL2, _T("50"), wxDefaultPosition, wxSize(60,21), 0, 1, 300, 50, _T("ID_SPINCTRL2"));
    SpinCtrl_parm2->SetValue(_T("50"));
    FlexGridSizer2->Add(SpinCtrl_parm2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText_Strands = new wxStaticText(this, ID_STATICTEXT4, _("# of Strands per String"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    StaticText_Strands->SetHelpText(_("How many times is a string folded\?"));
    FlexGridSizer2->Add(StaticText_Strands, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_parm3 = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxSize(60,21), 0, 1, 100, 1, _T("ID_SPINCTRL3"));
    SpinCtrl_parm3->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_parm3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _("Start Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SpinCtrl_StartChannel = new wxSpinCtrl(this, ID_SPINCTRL4, _T("1"), wxDefaultPosition, wxSize(90,21), 0, 1, 99999, 1, _T("ID_SPINCTRL4"));
    SpinCtrl_StartChannel->SetValue(_T("1"));
    FlexGridSizer2->Add(SpinCtrl_StartChannel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT7, _("Channel Order"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Order = new wxChoice(this, ID_CHOICE2, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE2"));
    Choice_Order->SetSelection( Choice_Order->Append(_("RGB")) );
    Choice_Order->Append(_("RBG"));
    Choice_Order->Append(_("GBR"));
    Choice_Order->Append(_("GRB"));
    Choice_Order->Append(_("BRG"));
    Choice_Order->Append(_("BGR"));
    FlexGridSizer2->Add(Choice_Order, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT8, _("Starting Corner"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
    StaticText3->SetHelpText(_("The point at which pixels in your model start."));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    RadioButton_TopLeft = new wxRadioButton(this, ID_RADIOBUTTON1, _("Top Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON1"));
    RadioButton_TopLeft->SetValue(true);
    BoxSizer1->Add(RadioButton_TopLeft, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RadioButton_TopRight = new wxRadioButton(this, ID_RADIOBUTTON2, _("Top Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON2"));
    BoxSizer1->Add(RadioButton_TopRight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText8 = new wxStaticText(this, ID_STATICTEXT11, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
    FlexGridSizer2->Add(StaticText8, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    RadioButton_BotLeft = new wxRadioButton(this, ID_RADIOBUTTON4, _("Bottom Left"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON4"));
    BoxSizer2->Add(RadioButton_BotLeft, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    RadioButton_BotRight = new wxRadioButton(this, ID_RADIOBUTTON3, _("Bottom Right"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RADIOBUTTON3"));
    BoxSizer2->Add(RadioButton_BotRight, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer2->Add(BoxSizer2, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText4 = new wxStaticText(this, ID_STATICTEXT9, _("Smooth Edges (antialias)"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
    FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    Choice_Antialias = new wxChoice(this, ID_CHOICE3, wxDefaultPosition, wxDefaultSize, 0, 0, 0, wxDefaultValidator, _T("ID_CHOICE3"));
    Choice_Antialias->SetSelection( Choice_Antialias->Append(_("None")) );
    Choice_Antialias->Append(_("2x"));
    FlexGridSizer2->Add(Choice_Antialias, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText7 = new wxStaticText(this, ID_STATICTEXT10, _("Part of my display"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
    FlexGridSizer2->Add(StaticText7, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    CheckBox_MyDisplay = new wxCheckBox(this, ID_CHECKBOX1, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX1"));
    CheckBox_MyDisplay->SetValue(true);
    FlexGridSizer2->Add(CheckBox_MyDisplay, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText9 = new wxStaticText(this, ID_STATICTEXT12, _("Individual Start Chans"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT12"));
    FlexGridSizer2->Add(StaticText9, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    cbIndividualStartNumbers = new wxCheckBox(this, ID_CHECKBOX2, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX2"));
    cbIndividualStartNumbers->SetValue(false);
    FlexGridSizer2->Add(cbIndividualStartNumbers, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    StaticText10 = new wxStaticText(this, ID_STATICTEXT13, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT13"));
    FlexGridSizer2->Add(StaticText10, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    btCustomModelConfig = new wxButton(this, ID_BUTTON1, _("Configure Custom Model"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    btCustomModelConfig->Disable();
    FlexGridSizer2->Add(btCustomModelConfig, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    gridStartChannels = new wxGrid(this, ID_GRID1, wxDefaultPosition, wxSize(150,348), wxVSCROLL|wxFULL_REPAINT_ON_RESIZE, _T("ID_GRID1"));
    gridStartChannels->CreateGrid(0,1);
    gridStartChannels->EnableEditing(true);
    gridStartChannels->EnableGridLines(true);
    gridStartChannels->SetRowLabelSize(25);
    gridStartChannels->SetDefaultColSize(90, true);
    gridStartChannels->SetColLabelValue(0, _("Start Channel"));
    gridStartChannels->SetDefaultCellFont( gridStartChannels->GetFont() );
    gridStartChannels->SetDefaultCellTextColour( gridStartChannels->GetForegroundColour() );
    FlexGridSizer3->Add(gridStartChannels, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_CHOICE1,wxEVT_COMMAND_CHOICE_SELECTED,(wxObjectEventFunction)&ModelDialog::OnChoice_DisplayAsSelect);
    Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ModelDialog::OnSpinCtrl_parm1Change);
    Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ModelDialog::OnSpinCtrl_parm2Change);
    Connect(ID_SPINCTRL4,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&ModelDialog::OnSpinCtrl_StartChannelChange);
    Connect(ID_CHECKBOX2,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&ModelDialog::OncbIndividualStartNumbersClick);
    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ModelDialog::OnbtCustomModleConfigClick);
    Connect(ID_GRID1,wxEVT_GRID_CELL_CHANGE,(wxObjectEventFunction)&ModelDialog::OngridStartChannelsCellChange);
    //*)
    UpdateStartChannels();
}

ModelDialog::~ModelDialog()
{
    //(*Destroy(ModelDialog)
    //*)
}


void ModelDialog::UpdateLabels()
{
    wxString choice;
    choice=Choice_DisplayAs->GetStringSelection();
    btCustomModelConfig->Enable(false);
    if (choice == wxT("Arches"))
    {
        StaticText_Strings->SetLabelText(_("# of Arches"));
        StaticText_Nodes->SetLabelText(_("# of RGB Nodes per Arch"));
        StaticText_Strands->SetLabelText(_("n/a"));
        SpinCtrl_parm3->SetValue(1);
        SpinCtrl_parm3->Enable(false);
    }
    else if (choice == wxT("Window Frame"))
    {
        StaticText_Strings->SetLabelText(_("# of RGB Nodes Top"));
        StaticText_Nodes->SetLabelText(_("# of RGB Nodes Left/Right"));
        StaticText_Strands->SetLabelText(_("# of RGB Nodes Bottom"));
        SpinCtrl_parm3->Enable(true);
    }
    else if (choice == wxT("Single Line"))
    {
        StaticText_Strings->SetLabelText(_("Actual # of Strings"));
        StaticText_Nodes->SetLabelText(_("# of RGB Nodes per String"));
        StaticText_Strands->SetLabelText(_("n/a"));
        SpinCtrl_parm3->SetValue(1);
        SpinCtrl_parm3->Enable(false);
    }
    else if (choice == wxT("Custom"))
    {
        StaticText_Strings->SetLabelText(_("Model Width"));
        StaticText_Nodes->SetLabelText(_("Model Height"));
        StaticText_Strands->SetLabelText(_("n/a"));
        SpinCtrl_parm3->SetValue(1);
        SpinCtrl_parm3->Enable(false);
        btCustomModelConfig->Enable(true);
    }
    else
    {
        // matrix or tree
        StaticText_Strings->SetLabelText(_("Actual # of Strings"));
        StaticText_Nodes->SetLabelText(_("# of RGB Nodes per String"));
        StaticText_Strands->SetLabelText(_("# of Strands per String"));
        SpinCtrl_parm3->Enable(true);
        UpdateStartChannels();
    }
}


void ModelDialog::OnChoice_DisplayAsSelect(wxCommandEvent& event)
{
    UpdateLabels();
}

void ModelDialog::OncbIndividualStartNumbersClick(wxCommandEvent& event)
{
    UpdateStartChannels();
}

void ModelDialog::UpdateStartChannels()
{
    int stringnum;
    int strings = SpinCtrl_parm1->GetValue();
    int startchan = SpinCtrl_StartChannel->GetValue();
    int nodesPerString = SpinCtrl_parm2->GetValue();
    int curRowCnt = gridStartChannels->GetNumberRows();

    UpdateRowCount();

    if(!cbIndividualStartNumbers->IsChecked())
    {
        SetDefaultStartChannels();
        SetReadOnly(true);
    }
    else
    {
        SetReadOnly(false);
    }

}

void ModelDialog::SetReadOnly(bool readonly)
{
    int ii;

    for(ii=0; ii < gridStartChannels->GetNumberRows(); ii++)
    {
        gridStartChannels->SetReadOnly(ii,0,readonly);
    }
}
void ModelDialog::OnSpinCtrl_parm1Change(wxSpinEvent& event)
{
    UpdateStartChannels();
}

void ModelDialog::UpdateRowCount()
{
    int stringnum;
    int strings = SpinCtrl_parm1->GetValue();
    int startchan = SpinCtrl_StartChannel->GetValue();
    int nodesPerString = SpinCtrl_parm2->GetValue();
    int curRowCnt = gridStartChannels->GetNumberRows();

    if (strings > curRowCnt )
    {
        gridStartChannels->AppendRows(strings - curRowCnt);
    }
    else if ( strings < curRowCnt )
    {
        gridStartChannels->DeleteRows(strings, curRowCnt - strings);
    }
}
void ModelDialog::SetDefaultStartChannels()
{
    int stringnum;
    int strings = SpinCtrl_parm1->GetValue();
    int startchan = SpinCtrl_StartChannel->GetValue();
    int nodesPerString = SpinCtrl_parm2->GetValue();
    int curRowCnt = gridStartChannels->GetNumberRows();

    for (stringnum=0; stringnum<strings; stringnum++)
    {
        gridStartChannels->SetCellValue(stringnum,0, wxString::Format(wxT("%i"),startchan + (stringnum*nodesPerString*3)));
    }
}

void ModelDialog::OnSpinCtrl_parm2Change(wxSpinEvent& event)
{
     UpdateStartChannels();
}

void ModelDialog::OnSpinCtrl_StartChannelChange(wxSpinEvent& event)
{
    UpdateStartChannels();
}

void ModelDialog::OngridStartChannelsCellChange(wxGridEvent& event)
{
    int row = event.GetRow(),
        col = event.GetCol();
    wxString tmpStr;
    long val;

    if (col==0)
    {
        tmpStr = gridStartChannels->GetCellValue(row,col);
        if ( (!tmpStr.ToLong(&val) || val < 0) )
        {
            wxMessageBox(_("Cell value must be a positive numeric value"));
            gridStartChannels->SetCellValue(row,col,wxT("0"));
        }
    }
    event.Skip();
}

void ModelDialog::OnbtCustomModleConfigClick(wxCommandEvent& event)
{
    CustomModelDialog dialog(this);
    int numRows, numCols, row, col, idx;
    unsigned long val;
    uint8_t *chanData;
    int dlgResult;
    wxString value;

    numCols=SpinCtrl_parm1->GetValue();
    numRows=SpinCtrl_parm2->GetValue();

    dialog.gdModelChans->AppendCols(numCols - dialog.gdModelChans->GetNumberCols());
    dialog.gdModelChans->AppendRows(numRows - dialog.gdModelChans->GetNumberRows());

    if(customChannelData.size() != 0)
    {
        chanData = base64_decode(customChannelData, numRows, numCols);
    }
    else
    {
        chanData = (uint8_t *) calloc(numRows*numCols*2, sizeof(uint8_t));
    }
    idx = 0;
    for(row=numRows-1; row >= 0 ; row--)
    {
        for( col=0; col<numCols; col++)
        {
            val = chanData[idx] << 8 | chanData[idx+1];
            if (val != 0)
                dialog.gdModelChans->SetCellValue(row,col,wxString::Format(wxT("%d"),val));
            idx+=2;
        }
    }
    dlgResult = dialog.ShowModal();
    if ( wxID_OK == dlgResult)
    {
        idx = 0;
        for(row=numRows-1; row >= 0; row--)
        {
            for( col=0; col<numCols; col++)
            {
                value = dialog.gdModelChans->GetCellValue(row,col);
                if (value == wxT(""))
                {
                    val = 0;
                }
                else
                {
                    value.ToULong(&val);
                }
                chanData[idx] = (uint8_t) (val >> 8) & 0xFF;
                chanData[idx+1] = (uint8_t) val & 0xFF;
                idx+=2;
            }
        }
        customChannelData = base64_encode(chanData,numRows*numCols*2);
    }
    free(chanData);
}
