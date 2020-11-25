#include "InAppPurchaseDialog.h"

//(*InternalHeaders(InAppPurchaseDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(InAppPurchaseDialog)
const long InAppPurchaseDialog::ID_STATICTEXT4 = wxNewId();
const long InAppPurchaseDialog::ID_STATICTEXT5 = wxNewId();
const long InAppPurchaseDialog::ID_STATICTEXT6 = wxNewId();
const long InAppPurchaseDialog::ID_TOGGLEBUTTON1 = wxNewId();
const long InAppPurchaseDialog::ID_TOGGLEBUTTON2 = wxNewId();
const long InAppPurchaseDialog::ID_TOGGLEBUTTON3 = wxNewId();
const long InAppPurchaseDialog::ID_TOGGLEBUTTON4 = wxNewId();
const long InAppPurchaseDialog::ID_TOGGLEBUTTON5 = wxNewId();
const long InAppPurchaseDialog::ID_TOGGLEBUTTON6 = wxNewId();
const long InAppPurchaseDialog::ID_STATICTEXT1 = wxNewId();
const long InAppPurchaseDialog::ID_STATICTEXT2 = wxNewId();
const long InAppPurchaseDialog::ID_STATICTEXT3 = wxNewId();
const long InAppPurchaseDialog::ID_STATICTEXT7 = wxNewId();
const long InAppPurchaseDialog::ID_HYPERLINKCTRL1 = wxNewId();
const long InAppPurchaseDialog::ID_BUTTON1 = wxNewId();
const long InAppPurchaseDialog::ID_BUTTON2 = wxNewId();
//*)

BEGIN_EVENT_TABLE(InAppPurchaseDialog,wxDialog)
	//(*EventTable(InAppPurchaseDialog)
	//*)
END_EVENT_TABLE()

InAppPurchaseDialog::InAppPurchaseDialog(wxWindow* parent)
{
	//(*Initialize(InAppPurchaseDialog)
	wxButton* CancelButton;
	wxButton* PurchaseButton;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxGridBagSizer* GridBagSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticText* StaticText1;

	Create(parent, wxID_ANY, _("Donate/Tip the Developers"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(1);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Tip the xLights and FPP Content Creators!"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer3->AddGrowableCol(0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("One Time Purchase"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 3, 0, 0);
	SmallLabel = new wxStaticText(this, ID_STATICTEXT4, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer5->Add(SmallLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MediumLabel = new wxStaticText(this, ID_STATICTEXT5, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer5->Add(MediumLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LargeLabel = new wxStaticText(this, ID_STATICTEXT6, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer5->Add(LargeLabel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SmallButton = new wxToggleButton(this, ID_TOGGLEBUTTON1, _("Small"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON1"));
	FlexGridSizer5->Add(SmallButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MediumButton = new wxToggleButton(this, ID_TOGGLEBUTTON2, _("Medium"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON2"));
	MediumButton->SetValue(true);
	FlexGridSizer5->Add(MediumButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LargeButton = new wxToggleButton(this, ID_TOGGLEBUTTON3, _("Large"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON3"));
	FlexGridSizer5->Add(LargeButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer5, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Monthly Subscription"));
	GridBagSizer1 = new wxGridBagSizer(0, 0);
	SmallMonthlyButton = new wxToggleButton(this, ID_TOGGLEBUTTON4, _("Small"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON4"));
	GridBagSizer1->Add(SmallMonthlyButton, wxGBPosition(1, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MediumMonthlyButton = new wxToggleButton(this, ID_TOGGLEBUTTON5, _("Medium"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON5"));
	GridBagSizer1->Add(MediumMonthlyButton, wxGBPosition(1, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LargeMonthlyButton = new wxToggleButton(this, ID_TOGGLEBUTTON6, _("Large"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TOGGLEBUTTON6"));
	GridBagSizer1->Add(LargeMonthlyButton, wxGBPosition(1, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SmallMonthlyLabel = new wxStaticText(this, ID_STATICTEXT1, _("/month"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	GridBagSizer1->Add(SmallMonthlyLabel, wxGBPosition(0, 0), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	MediumMonthlyLabel = new wxStaticText(this, ID_STATICTEXT2, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	GridBagSizer1->Add(MediumMonthlyLabel, wxGBPosition(0, 1), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	LargeMonthlyLabel = new wxStaticText(this, ID_STATICTEXT3, _("Label"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	GridBagSizer1->Add(LargeMonthlyLabel, wxGBPosition(0, 2), wxDefaultSpan, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT7, _("Subscriptions can be cancelled at any time \nfrom within your Apple iCloud/iTunes \naccount.  See:"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	GridBagSizer1->Add(StaticText2, wxGBPosition(2, 0), wxGBSpan(1, 3), wxALL|wxEXPAND, 5);
	HyperlinkCtrl1 = new wxHyperlinkCtrl(this, ID_HYPERLINKCTRL1, _("https://support.apple.com/en-us/HT202039"), wxEmptyString, wxDefaultPosition, wxDefaultSize, wxHL_CONTEXTMENU|wxHL_ALIGN_CENTRE|wxNO_BORDER, _T("ID_HYPERLINKCTRL1"));
	GridBagSizer1->Add(HyperlinkCtrl1, wxGBPosition(3, 0), wxGBSpan(1, 3), wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2->Add(GridBagSizer1, 1, wxALL|wxEXPAND, 0);
	FlexGridSizer3->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 3, 0, 0);
	PurchaseButton = new wxButton(this, ID_BUTTON1, _("Purchase"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer4->Add(PurchaseButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	CancelButton = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	CancelButton->SetDefault();
	FlexGridSizer4->Add(CancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TOGGLEBUTTON1,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnSmallButtonToggle);
	Connect(ID_TOGGLEBUTTON2,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnMediumButtonToggle);
	Connect(ID_TOGGLEBUTTON3,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnLargeButtonToggle);
	Connect(ID_TOGGLEBUTTON4,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnSmallMonthlyButtonToggle);
	Connect(ID_TOGGLEBUTTON5,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnMediumMonthlyButtonToggle);
	Connect(ID_TOGGLEBUTTON6,wxEVT_COMMAND_TOGGLEBUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnLargeMonthlyButtonToggle);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnPurchaseButtonClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&InAppPurchaseDialog::OnCancelButtonClick);
	//*)
}

InAppPurchaseDialog::~InAppPurchaseDialog()
{
	//(*Destroy(InAppPurchaseDialog)
	//*)
}

int InAppPurchaseDialog::getSelection() {
    if (SmallButton->GetValue()) {
        return 1;
    }
    if (MediumButton->GetValue()) {
        return 2;
    }
    if (LargeButton->GetValue()) {
        return 3;
    }
    if (SmallMonthlyButton->GetValue()) {
        return 4;
    }
    if (MediumMonthlyButton->GetValue()) {
        return 5;
    }
    if (LargeMonthlyButton->GetValue()) {
        return 6;
    }
    return 0;
}


void InAppPurchaseDialog::setPrices() {
    SmallLabel->SetLabel(prices[0]);
    if (prices[0] == "") {
        SmallButton->Disable();
    }
    MediumLabel->SetLabel(prices[1]);
    if (prices[1] == "") {
        MediumButton->Disable();
    }
    LargeLabel->SetLabel(prices[2]);
    if (prices[2] == "") {
        LargeButton->Disable();
    }
    SmallMonthlyLabel->SetLabel(prices[3] + "/month");
    if (prices[3] == "") {
        SmallMonthlyButton->Disable();
    }
    MediumMonthlyLabel->SetLabel(prices[4] + "/month");
    if (prices[4] == "") {
        MediumMonthlyButton->Disable();
    }
    LargeMonthlyLabel->SetLabel(prices[5] + "/month");
    if (prices[5] == "") {
        LargeMonthlyButton->Disable();
    }
    
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

void InAppPurchaseDialog::OnSmallButtonToggle(wxCommandEvent& event)
{
    bool b = true;
    SmallButton->SetValue(b);
    MediumButton->SetValue(!b);
    LargeButton->SetValue(!b);
    SmallMonthlyButton->SetValue(!b);
    MediumMonthlyButton->SetValue(!b);
    LargeMonthlyButton->SetValue(!b);
}

void InAppPurchaseDialog::OnMediumButtonToggle(wxCommandEvent& event)
{
    bool b = true;
    SmallButton->SetValue(!b);
    MediumButton->SetValue(b);
    LargeButton->SetValue(!b);
    SmallMonthlyButton->SetValue(!b);
    MediumMonthlyButton->SetValue(!b);
    LargeMonthlyButton->SetValue(!b);
}

void InAppPurchaseDialog::OnLargeButtonToggle(wxCommandEvent& event)
{
    bool b = true;
    SmallButton->SetValue(!b);
    MediumButton->SetValue(!b);
    LargeButton->SetValue(b);
    SmallMonthlyButton->SetValue(!b);
    MediumMonthlyButton->SetValue(!b);
    LargeMonthlyButton->SetValue(!b);
}

void InAppPurchaseDialog::OnSmallMonthlyButtonToggle(wxCommandEvent& event)
{
    bool b = true;
    SmallButton->SetValue(!b);
    MediumButton->SetValue(!b);
    LargeButton->SetValue(!b);
    SmallMonthlyButton->SetValue(b);
    MediumMonthlyButton->SetValue(!b);
    LargeMonthlyButton->SetValue(!b);
}

void InAppPurchaseDialog::OnMediumMonthlyButtonToggle(wxCommandEvent& event)
{
    bool b = true;
    SmallButton->SetValue(!b);
    MediumButton->SetValue(!b);
    LargeButton->SetValue(!b);
    SmallMonthlyButton->SetValue(!b);
    MediumMonthlyButton->SetValue(b);
    LargeMonthlyButton->SetValue(!b);
}

void InAppPurchaseDialog::OnLargeMonthlyButtonToggle(wxCommandEvent& event)
{
    bool b = true;
    SmallButton->SetValue(!b);
    MediumButton->SetValue(!b);
    LargeButton->SetValue(!b);
    SmallMonthlyButton->SetValue(!b);
    MediumMonthlyButton->SetValue(!b);
    LargeMonthlyButton->SetValue(b);
}

void InAppPurchaseDialog::OnCancelButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}

void InAppPurchaseDialog::OnPurchaseButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}
