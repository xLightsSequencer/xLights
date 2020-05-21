/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelDimmingCurveDialog.h"
#include "xLightsXmlFile.h"
#include "osxMacUtils.h"

//(*InternalHeaders(ModelDimmingCurveDialog)
#include <wx/button.h>
#include <wx/string.h>
#include <wx/intl.h>
#include "UtilFunctions.h"
//*)

//(*IdInit(ModelDimmingCurveDialog)
const long ModelDimmingCurveDialog::ID_TEXTCTRL1 = wxNewId();
const long ModelDimmingCurveDialog::IDD_SLIDER_SINGLEBRIGHTNESS = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL_SINGLEBRIGHTNESS = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL1 = wxNewId();
const long ModelDimmingCurveDialog::ID_FILEPICKERCTRL1 = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL2 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT6 = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL2 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT7 = wxNewId();
const long ModelDimmingCurveDialog::IDD_SLIDER_RGBRED = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL_RGBRED = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT8 = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL4 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT9 = wxNewId();
const long ModelDimmingCurveDialog::IDD_SLIDER_RGBGREEN = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL_RGBGREEN = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT10 = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL6 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT11 = wxNewId();
const long ModelDimmingCurveDialog::IDD_SLIDER_RGBBLUE = wxNewId();
const long ModelDimmingCurveDialog::ID_TEXTCTRL_RGBBLUE = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL3 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT3 = wxNewId();
const long ModelDimmingCurveDialog::ID_FILEPICKERCTRL2 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT4 = wxNewId();
const long ModelDimmingCurveDialog::ID_FILEPICKERCTRL3 = wxNewId();
const long ModelDimmingCurveDialog::ID_STATICTEXT5 = wxNewId();
const long ModelDimmingCurveDialog::ID_FILEPICKERCTRL4 = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL4 = wxNewId();
const long ModelDimmingCurveDialog::ID_CHOICEBOOK1 = wxNewId();
const long ModelDimmingCurveDialog::ID_GLCANVAS2 = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL5 = wxNewId();
const long ModelDimmingCurveDialog::ID_GLCANVAS1 = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL6 = wxNewId();
const long ModelDimmingCurveDialog::ID_GLCANVAS3 = wxNewId();
const long ModelDimmingCurveDialog::ID_PANEL7 = wxNewId();
//*)

BEGIN_EVENT_TABLE(ModelDimmingCurveDialog,wxDialog)
	//(*EventTable(ModelDimmingCurveDialog)
	//*)
END_EVENT_TABLE()

ModelDimmingCurveDialog::ModelDimmingCurveDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
    AdjustModalDialogParent(parent);
	//(*Initialize(ModelDimmingCurveDialog)
	wxStaticBoxSizer* StaticBoxSizer2;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer11;
	wxFlexGridSizer* FlexGridSizer4;
	wxStaticText* StaticText1;
	wxStaticBoxSizer* RedPanelBox;
	wxFlexGridSizer* FlexGridSizer9;
	wxStaticBoxSizer* StaticBoxSizer3;
	wxFlexGridSizer* FlexGridSizer6;
	wxStaticBoxSizer* BluePanelBox;
	wxFlexGridSizer* FlexGridSizer3;
	wxStaticBoxSizer* GreenPanelBox;
	wxFlexGridSizer* FlexGridSizer10;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer13;
	wxStaticText* StaticText2;
	wxFlexGridSizer* FlexGridSizer12;
	wxFlexGridSizer* FlexGridSizer5;
	wxStdDialogButtonSizer* StdDialogButtonSizer1;
	wxStaticBoxSizer* StaticBoxSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	DimmingTypeChoice = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
	GammaPanel = new wxPanel(DimmingTypeChoice, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
	FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2->AddGrowableCol(0);
	FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer3->AddGrowableCol(1);
	StaticText2 = new wxStaticText(GammaPanel, wxID_ANY, _("Gamma"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SingleGammaText = new wxTextCtrl(GammaPanel, ID_TEXTCTRL1, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL1"));
	FlexGridSizer3->Add(SingleGammaText, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText1 = new wxStaticText(GammaPanel, wxID_ANY, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer3->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SingleBrightnessSlider = new wxSlider(GammaPanel, IDD_SLIDER_SINGLEBRIGHTNESS, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_SINGLEBRIGHTNESS"));
	FlexGridSizer3->Add(SingleBrightnessSlider, 1, wxALL|wxEXPAND, 5);
	SingleBrightnessBox = new wxTextCtrl(GammaPanel, ID_TEXTCTRL_SINGLEBRIGHTNESS, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_SINGLEBRIGHTNESS"));
	SingleBrightnessBox->SetMaxLength(4);
	FlexGridSizer3->Add(SingleBrightnessBox, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 5);
	GammaPanel->SetSizer(FlexGridSizer2);
	FlexGridSizer2->Fit(GammaPanel);
	FlexGridSizer2->SetSizeHints(GammaPanel);
	FromFilePanel = new wxPanel(DimmingTypeChoice, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer8->AddGrowableCol(0);
	SingleFilePicker = new wxFilePickerCtrl(FromFilePanel, ID_FILEPICKERCTRL1, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer8->Add(SingleFilePicker, 1, wxALL|wxEXPAND, 5);
	FromFilePanel->SetSizer(FlexGridSizer8);
	FlexGridSizer8->Fit(FromFilePanel);
	FlexGridSizer8->SetSizeHints(FromFilePanel);
	RGBGammaPanel = new wxPanel(DimmingTypeChoice, ID_PANEL3, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL3"));
	FlexGridSizer10 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer10->AddGrowableCol(0);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, RGBGammaPanel, _("Red"));
	FlexGridSizer11 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer11->AddGrowableCol(1);
	StaticText6 = new wxStaticText(RGBGammaPanel, ID_STATICTEXT6, _("Gamma"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
	FlexGridSizer11->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBRedGammaTextCtrl = new wxTextCtrl(RGBGammaPanel, ID_TEXTCTRL2, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL2"));
	FlexGridSizer11->Add(RGBRedGammaTextCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer11->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText7 = new wxStaticText(RGBGammaPanel, ID_STATICTEXT7, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT7"));
	FlexGridSizer11->Add(StaticText7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBRedSlider = new wxSlider(RGBGammaPanel, IDD_SLIDER_RGBRED, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_RGBRED"));
	FlexGridSizer11->Add(RGBRedSlider, 1, wxALL|wxEXPAND, 5);
	RGBRedTextCtrl = new wxTextCtrl(RGBGammaPanel, ID_TEXTCTRL_RGBRED, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_RGBRED"));
	RGBRedTextCtrl->SetMaxLength(4);
	FlexGridSizer11->Add(RGBRedTextCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer1->Add(FlexGridSizer11, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer10->Add(StaticBoxSizer1, 1, wxALL|wxEXPAND, 0);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, RGBGammaPanel, _("Green"));
	FlexGridSizer12 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer12->AddGrowableCol(1);
	StaticText8 = new wxStaticText(RGBGammaPanel, ID_STATICTEXT8, _("Gamma"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT8"));
	FlexGridSizer12->Add(StaticText8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBGreenGammaTextCtrl = new wxTextCtrl(RGBGammaPanel, ID_TEXTCTRL4, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL4"));
	FlexGridSizer12->Add(RGBGreenGammaTextCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer12->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText9 = new wxStaticText(RGBGammaPanel, ID_STATICTEXT9, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT9"));
	FlexGridSizer12->Add(StaticText9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBGreenSlider = new wxSlider(RGBGammaPanel, IDD_SLIDER_RGBGREEN, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_RGBGREEN"));
	FlexGridSizer12->Add(RGBGreenSlider, 1, wxALL|wxEXPAND, 5);
	RGBGreenTextCtrl = new wxTextCtrl(RGBGammaPanel, ID_TEXTCTRL_RGBGREEN, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_RGBGREEN"));
	RGBGreenTextCtrl->SetMaxLength(4);
	FlexGridSizer12->Add(RGBGreenTextCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2->Add(FlexGridSizer12, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer10->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
	StaticBoxSizer3 = new wxStaticBoxSizer(wxHORIZONTAL, RGBGammaPanel, _("Blue"));
	FlexGridSizer13 = new wxFlexGridSizer(0, 3, 0, 0);
	FlexGridSizer13->AddGrowableCol(1);
	StaticText10 = new wxStaticText(RGBGammaPanel, ID_STATICTEXT10, _("Gamma"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT10"));
	FlexGridSizer13->Add(StaticText10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBBlueGammaTextCtrl = new wxTextCtrl(RGBGammaPanel, ID_TEXTCTRL6, _("1.0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL6"));
	FlexGridSizer13->Add(RGBBlueGammaTextCtrl, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer13->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText11 = new wxStaticText(RGBGammaPanel, ID_STATICTEXT11, _("Brightness"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT11"));
	FlexGridSizer13->Add(StaticText11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBBlueSlider = new wxSlider(RGBGammaPanel, IDD_SLIDER_RGBBLUE, 0, -100, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("IDD_SLIDER_RGBBLUE"));
	FlexGridSizer13->Add(RGBBlueSlider, 1, wxALL|wxEXPAND, 5);
	RGBBlueTextCtrl = new wxTextCtrl(RGBGammaPanel, ID_TEXTCTRL_RGBBLUE, _("0"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_TEXTCTRL_RGBBLUE"));
	RGBBlueTextCtrl->SetMaxLength(4);
	FlexGridSizer13->Add(RGBBlueTextCtrl, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer3->Add(FlexGridSizer13, 1, wxALL|wxEXPAND, 1);
	FlexGridSizer10->Add(StaticBoxSizer3, 1, wxALL|wxEXPAND, 1);
	RGBGammaPanel->SetSizer(FlexGridSizer10);
	FlexGridSizer10->Fit(RGBGammaPanel);
	FlexGridSizer10->SetSizeHints(RGBGammaPanel);
	RGBFromFilePanel = new wxPanel(DimmingTypeChoice, ID_PANEL4, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL4"));
	FlexGridSizer9 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer9->AddGrowableCol(1);
	StaticText3 = new wxStaticText(RGBFromFilePanel, ID_STATICTEXT3, _("Red"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
	FlexGridSizer9->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBRedFilePicker = new wxFilePickerCtrl(RGBFromFilePanel, ID_FILEPICKERCTRL2, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL2"));
	FlexGridSizer9->Add(RGBRedFilePicker, 1, wxALL|wxEXPAND, 5);
	StaticText4 = new wxStaticText(RGBFromFilePanel, ID_STATICTEXT4, _("Green"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
	FlexGridSizer9->Add(StaticText4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBGreenFilePicker = new wxFilePickerCtrl(RGBFromFilePanel, ID_FILEPICKERCTRL3, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL3"));
	FlexGridSizer9->Add(RGBGreenFilePicker, 1, wxALL|wxEXPAND, 5);
	StaticText5 = new wxStaticText(RGBFromFilePanel, ID_STATICTEXT5, _("Blue"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT5"));
	FlexGridSizer9->Add(StaticText5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	RGBBlueFilePicker = new wxFilePickerCtrl(RGBFromFilePanel, ID_FILEPICKERCTRL4, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL4"));
	FlexGridSizer9->Add(RGBBlueFilePicker, 1, wxALL|wxEXPAND, 5);
	RGBFromFilePanel->SetSizer(FlexGridSizer9);
	FlexGridSizer9->Fit(RGBFromFilePanel);
	FlexGridSizer9->SetSizeHints(RGBFromFilePanel);
	DimmingTypeChoice->AddPage(GammaPanel, _("Single Brightness/Gamma"), true);
	DimmingTypeChoice->AddPage(FromFilePanel, _("Single Curve From File"), false);
	DimmingTypeChoice->AddPage(RGBGammaPanel, _("RGB Brightness/Gamma"), false);
	DimmingTypeChoice->AddPage(RGBFromFilePanel, _("RGB From File"), false);
	FlexGridSizer1->Add(DimmingTypeChoice, 1, wxALL|wxEXPAND, 5);
	BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
	RedPanel = new wxPanel(this, ID_PANEL5, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL5"));
	FlexGridSizer4 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer4->AddGrowableCol(0);
	FlexGridSizer4->AddGrowableRow(0);
	RedPanelBox = new wxStaticBoxSizer(wxHORIZONTAL, RedPanel, _("Red"));
	int GLCanvasAttributes_1[] = {
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE,      16,
		WX_GL_STENCIL_SIZE,    0,
		0, 0 };
	redDCPanel = new DimmingCurvePanel(RedPanel, ID_GLCANVAS2, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_GLCANVAS2"), GLCanvasAttributes_1);
	redDCPanel->SetMinSize(wxSize(100,100));
	RedPanelBox->Add(redDCPanel, 1, wxEXPAND, 0);
	FlexGridSizer4->Add(RedPanelBox, 1, wxALL|wxEXPAND, 1);
	RedPanel->SetSizer(FlexGridSizer4);
	FlexGridSizer4->Fit(RedPanel);
	FlexGridSizer4->SetSizeHints(RedPanel);
	BoxSizer1->Add(RedPanel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	GreenPanel = new wxPanel(this, ID_PANEL6, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL6"));
	FlexGridSizer5 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer5->AddGrowableCol(0);
	FlexGridSizer5->AddGrowableRow(0);
	GreenPanelBox = new wxStaticBoxSizer(wxHORIZONTAL, GreenPanel, _("Green"));
	int GLCanvasAttributes_2[] = {
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE,      16,
		WX_GL_STENCIL_SIZE,    0,
		0, 0 };
	greenDCPanel = new DimmingCurvePanel(GreenPanel, ID_GLCANVAS1, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_GLCANVAS1"), GLCanvasAttributes_2);
	greenDCPanel->SetMinSize(wxSize(100,100));
	GreenPanelBox->Add(greenDCPanel, 1, wxEXPAND, 0);
	FlexGridSizer5->Add(GreenPanelBox, 1, wxALL|wxEXPAND, 1);
	GreenPanel->SetSizer(FlexGridSizer5);
	FlexGridSizer5->Fit(GreenPanel);
	FlexGridSizer5->SetSizeHints(GreenPanel);
	BoxSizer1->Add(GreenPanel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BluePanel = new wxPanel(this, ID_PANEL7, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL7"));
	FlexGridSizer6 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6->AddGrowableCol(0);
	FlexGridSizer6->AddGrowableRow(0);
	BluePanelBox = new wxStaticBoxSizer(wxHORIZONTAL, BluePanel, _("Blue"));
	int GLCanvasAttributes_3[] = {
		WX_GL_RGBA,
		WX_GL_DOUBLEBUFFER,
		WX_GL_DEPTH_SIZE,      16,
		WX_GL_STENCIL_SIZE,    0,
		0, 0 };
	blueDCPanel = new DimmingCurvePanel(BluePanel, ID_GLCANVAS3, wxDefaultPosition, wxDefaultSize, wxNO_BORDER, _T("ID_GLCANVAS3"), GLCanvasAttributes_3);
	blueDCPanel->SetMinSize(wxSize(100,100));
	BluePanelBox->Add(blueDCPanel, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
	FlexGridSizer6->Add(BluePanelBox, 1, wxALL|wxEXPAND, 1);
	BluePanel->SetSizer(FlexGridSizer6);
	FlexGridSizer6->Fit(BluePanel);
	FlexGridSizer6->SetSizeHints(BluePanel);
	BoxSizer1->Add(BluePanel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(BoxSizer1, 1, wxALL|wxEXPAND, 5);
	StdDialogButtonSizer1 = new wxStdDialogButtonSizer();
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_OK, wxEmptyString));
	StdDialogButtonSizer1->AddButton(new wxButton(this, wxID_CANCEL, wxEmptyString));
	StdDialogButtonSizer1->Realize();
	FlexGridSizer1->Add(StdDialogButtonSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_TEXTCTRL1,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnSingleGammaText);
	Connect(IDD_SLIDER_SINGLEBRIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_SINGLEBRIGHTNESS,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnSingleBrightnessBoxText);
	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnSingleFilePickerFileChanged);
	Connect(ID_TEXTCTRL2,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBGammaText);
	Connect(IDD_SLIDER_RGBRED,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_RGBRED,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBTextCtrlText);
	Connect(ID_TEXTCTRL4,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBGammaText);
	Connect(IDD_SLIDER_RGBGREEN,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_RGBGREEN,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBTextCtrlText);
	Connect(ID_TEXTCTRL6,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBGammaText);
	Connect(IDD_SLIDER_RGBBLUE,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::UpdateLinkedTextCtrl);
	Connect(ID_TEXTCTRL_RGBBLUE,wxEVT_COMMAND_TEXT_UPDATED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBTextCtrlText);
	Connect(ID_FILEPICKERCTRL2,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBFilePickerFileChanged);
	Connect(ID_FILEPICKERCTRL3,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBFilePickerFileChanged);
	Connect(ID_FILEPICKERCTRL4,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&ModelDimmingCurveDialog::OnRGBFilePickerFileChanged);
	//*)

    redDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(0, 0.25), 0);
    greenDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(0, 0.5), 0);
    blueDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(0, 1.5), 0);
    
    brightnessValidator.SetRange(-100, 100);
    gammaValidator.SetRange(0.0, 50.0);
    gammaValidator.SetStyle(wxNUM_VAL_NO_TRAILING_ZEROES);
    
    SingleBrightnessBox->SetValidator(brightnessValidator);
    RGBRedTextCtrl->SetValidator(brightnessValidator);
    RGBGreenTextCtrl->SetValidator(brightnessValidator);
    RGBBlueTextCtrl->SetValidator(brightnessValidator);
    
    SingleGammaText->SetValidator(gammaValidator);
    RGBRedGammaTextCtrl->SetValidator(gammaValidator);
    RGBGreenGammaTextCtrl->SetValidator(gammaValidator);
    RGBBlueGammaTextCtrl->SetValidator(gammaValidator);

    SetEscapeId(wxID_CANCEL);
}

ModelDimmingCurveDialog::~ModelDimmingCurveDialog()
{
	//(*Destroy(ModelDimmingCurveDialog)
	//*)
}

void ModelDimmingCurveDialog::UpdateLinkedTextCtrl(wxScrollEvent& event)
{
    wxSlider * slider = (wxSlider*)event.GetEventObject();
    wxString name = slider->GetName();
    if (name.Contains("ID_")) {
        name.Replace("ID_SLIDER_", "IDD_TEXTCTRL_");
    } else {
        name.Replace("IDD_SLIDER_", "ID_TEXTCTRL_");
    }
    wxTextCtrl *txt = (wxTextCtrl*)slider->GetParent()->FindWindowByName(name);
    txt->SetValue(wxString::Format("%d",slider->GetValue()));
}

void ModelDimmingCurveDialog::UpdateLinkedSlider(wxCommandEvent& event)
{
    wxTextCtrl * txt = (wxTextCtrl*)event.GetEventObject();
    wxString name = txt->GetName();
    if (name.Contains("IDD_")) {
        name.Replace("IDD_TEXTCTRL_", "ID_SLIDER_");
    } else {
        name.Replace("ID_TEXTCTRL_", "IDD_SLIDER_");
    }
    wxSlider *slider = (wxSlider*)txt->GetParent()->FindWindowByName(name);
    if (slider == nullptr) {
        return;
    }
    int value = wxAtoi(txt->GetValue());

    if (value < slider->GetMin()) {
        value = slider->GetMin();
        wxString val_str;
        val_str << value;
        txt->ChangeValue(val_str);
    }
    else if (value > slider->GetMax()) {
        value = slider->GetMax();
        wxString val_str;
        val_str << value;
        txt->ChangeValue(val_str);
    }
    slider->SetValue(value);
}

void ModelDimmingCurveDialog::Init(std::map<std::string, std::map<std::string,std::string>> &dimmingInfo) {
    if (dimmingInfo.find("all") != dimmingInfo.end()) {
        if (dimmingInfo["all"].find("filename") != dimmingInfo["all"].end()) {
            DimmingTypeChoice->SetSelection(1);
            SingleFilePicker->SetPath(dimmingInfo["all"]["filename"]);
            redDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(SingleFilePicker->GetPath()), 0);
            greenDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(SingleFilePicker->GetPath()), 0);
            blueDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(SingleFilePicker->GetPath()), 0);
        } else {
            DimmingTypeChoice->SetSelection(0);
            SingleGammaText->ChangeValue(dimmingInfo["all"]["gamma"]);
            if (wxAtof(SingleGammaText->GetValue()) > 50.0)
            {
                SingleGammaText->ChangeValue("50.0");
            }
            SingleBrightnessBox->SetValue(dimmingInfo["all"]["brightness"]);
        }
    } else {
        if (dimmingInfo["red"].find("filename") != dimmingInfo["red"].end()) {
            DimmingTypeChoice->SetSelection(3);
            RGBRedFilePicker->SetPath(dimmingInfo["red"]["filename"]);
            RGBGreenFilePicker->SetPath(dimmingInfo["green"]["filename"]);
            RGBBlueFilePicker->SetPath(dimmingInfo["blue"]["filename"]);
            
            redDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(FixFile("",RGBRedFilePicker->GetPath())), 0);
            greenDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(FixFile("", RGBGreenFilePicker->GetPath())), 0);
            blueDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(FixFile("", RGBBlueFilePicker->GetPath())), 0);
        } else {
            DimmingTypeChoice->SetSelection(2);
            RGBRedGammaTextCtrl->ChangeValue(dimmingInfo["red"]["gamma"]);
            RGBGreenGammaTextCtrl->ChangeValue(dimmingInfo["green"]["gamma"]);
            RGBBlueGammaTextCtrl->ChangeValue(dimmingInfo["blue"]["gamma"]);
            RGBRedTextCtrl->SetValue(dimmingInfo["red"]["brightness"]);
            RGBGreenTextCtrl->SetValue(dimmingInfo["green"]["brightness"]);
            RGBBlueTextCtrl->SetValue(dimmingInfo["blue"]["brightness"]);
        }
    }
}
static const wxString &validate(const wxString &in, const wxString &def) {
    if (in == "") {
        return def;
    }
    return in;
}
void ModelDimmingCurveDialog::Update(std::map<std::string, std::map<std::string,std::string>> &dimmingInfo) {
    switch (DimmingTypeChoice->GetSelection()) {
    case 0:
        dimmingInfo["all"]["brightness"] = validate(SingleBrightnessBox->GetValue(), "0");
        dimmingInfo["all"]["gamma"] = validate(SingleGammaText->GetValue(), "1.0");
        break;
    case 1:
        dimmingInfo["all"]["filename"] = SingleFilePicker->GetPath();
        break;
    case 2:
        dimmingInfo["red"]["brightness"] = validate(RGBRedTextCtrl->GetValue(), "0");
        dimmingInfo["red"]["gamma"] = validate(RGBRedGammaTextCtrl->GetValue(), "1.0");
        dimmingInfo["green"]["brightness"] = validate(RGBGreenTextCtrl->GetValue(), "0");
        dimmingInfo["green"]["gamma"] = validate(RGBGreenGammaTextCtrl->GetValue(), "1.0");
        dimmingInfo["blue"]["brightness"] = validate(RGBBlueTextCtrl->GetValue(), "0");
        dimmingInfo["blue"]["gamma"] = validate(RGBBlueGammaTextCtrl->GetValue(), "1.0");
        break;
    case 3:
        dimmingInfo["red"]["filename"] = RGBRedFilePicker->GetPath();
        dimmingInfo["green"]["filename"] = RGBGreenFilePicker->GetPath();
        dimmingInfo["blue"]["filename"] = RGBBlueFilePicker->GetPath();
        break;
    }
}

void ModelDimmingCurveDialog::OnSingleGammaText(wxCommandEvent& event)
{
    float f = wxAtof(validate(SingleGammaText->GetValue(), "1.0"));
    int i = wxAtoi(validate(SingleBrightnessBox->GetValue(), "0"));
    redDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(i, f), 0);
    greenDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(i, f), 0);
    blueDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(i, f), 0);
}

void ModelDimmingCurveDialog::OnSingleBrightnessBoxText(wxCommandEvent& event)
{
    UpdateLinkedSlider(event);
    OnSingleGammaText(event);
}

void ModelDimmingCurveDialog::OnRGBGammaText(wxCommandEvent& event)
{
    float f = wxAtof(validate(RGBRedGammaTextCtrl->GetValue(), "1.0"));
    int i = wxAtoi(validate(RGBRedTextCtrl->GetValue(), "0"));
    redDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(i, f), 0);
    
    f = wxAtof(validate(RGBGreenGammaTextCtrl->GetValue(), "1.0"));
    i = wxAtoi(validate(RGBGreenTextCtrl->GetValue(), "0"));
    greenDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(i, f), 0);

    f = wxAtof(validate(RGBBlueGammaTextCtrl->GetValue(), "1.0"));
    i = wxAtoi(validate(RGBBlueTextCtrl->GetValue(), "0"));
    blueDCPanel->SetDimmingCurve(DimmingCurve::createBrightnessGamma(i, f), 0);
}

void ModelDimmingCurveDialog::OnRGBTextCtrlText(wxCommandEvent& event)
{
    UpdateLinkedSlider(event);
    OnRGBGammaText(event);
}

void ModelDimmingCurveDialog::OnRGBFilePickerFileChanged(wxFileDirPickerEvent& event)
{
    redDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(FixFile("",RGBRedFilePicker->GetPath())), 0);
    greenDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(FixFile("", RGBGreenFilePicker->GetPath())), 0);
    blueDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(FixFile("", RGBBlueFilePicker->GetPath())), 0);
}

void ModelDimmingCurveDialog::OnSingleFilePickerFileChanged(wxFileDirPickerEvent& event)
{
    redDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(SingleFilePicker->GetPath()), 0);
    greenDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(SingleFilePicker->GetPath()), 0);
    blueDCPanel->SetDimmingCurve(DimmingCurve::createFromFile(SingleFilePicker->GetPath()), 0);
}
