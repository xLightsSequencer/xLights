#include "ChannelLayoutDialog.h"

//(*InternalHeaders(ChannelLayoutDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/html/htmlwin.h>
#include <wx/html/htmprint.h>

//(*IdInit(ChannelLayoutDialog)
const long ChannelLayoutDialog::ID_BUTTON1 = wxNewId();
const long ChannelLayoutDialog::ID_HTMLWINDOW1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ChannelLayoutDialog,wxDialog)
    //(*EventTable(ChannelLayoutDialog)
    //*)
END_EVENT_TABLE()

ChannelLayoutDialog::ChannelLayoutDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    //(*Initialize(ChannelLayoutDialog)
    wxBoxSizer* BoxSizer1;
    wxButton* Button_Print;
    wxFlexGridSizer* FlexGridSizer1;
    wxStdDialogButtonSizer* StdDialogButtonSizer1;

    Create(parent, id, _("Node Layout"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(1);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
    StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
    StdDialogButtonSizer1->Realize();
    BoxSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    Button_Print = new wxButton(this, ID_BUTTON1, _("Print"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    BoxSizer1->Add(Button_Print, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    HtmlWindow1 = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxDefaultSize, wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    HtmlWindow1->SetMinSize(wxSize(500,400));
    FlexGridSizer1->Add(HtmlWindow1, 1, wxALL|wxEXPAND, 5);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&ChannelLayoutDialog::OnButton_PrintClick);
    //*)
    HtmlEasyPrint=new wxHtmlEasyPrinting("xLights Printing", this);
}

ChannelLayoutDialog::~ChannelLayoutDialog()
{
    //(*Destroy(ChannelLayoutDialog)
    //*)
    delete HtmlEasyPrint;
}

void ChannelLayoutDialog::OnButton_PrintClick(wxCommandEvent& event)
{
    HtmlEasyPrint->SetStandardFonts(7, "Arial");
    HtmlEasyPrint->PageSetup();
    HtmlEasyPrint->PreviewText(HtmlSource);
}

void ChannelLayoutDialog::SetHtmlSource(wxString& html)
{
    HtmlSource=html;
    HtmlWindow1->SetPage(html);
}
