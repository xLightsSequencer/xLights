#include "CustomModelDialog.h"

#include <wx/msgdlg.h>
#include <wx/clipbrd.h>
#include <wx/graphics.h>
#include <wx/file.h>
#include <wx/numdlg.h>

//(*InternalHeaders(CustomModelDialog)
#include <wx/artprov.h>
#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include "models/CustomModel.h"
#include "WiringDialog.h"
#include "wxModelGridCellRenderer.h"
#include "UtilClasses.h"
#include "UtilFunctions.h"

//(*IdInit(CustomModelDialog)
const long CustomModelDialog::ID_SPINCTRL1 = wxNewId();
const long CustomModelDialog::ID_SPINCTRL2 = wxNewId();
const long CustomModelDialog::ID_STATICTEXT1 = wxNewId();
const long CustomModelDialog::ID_SPINCTRL3 = wxNewId();
const long CustomModelDialog::ID_BUTTON3 = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_CUT = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_COPY = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_PASTE = wxNewId();
const long CustomModelDialog::ID_BUTTON_CustomModelZoomIn = wxNewId();
const long CustomModelDialog::ID_BUTTON_CustomModelZoomOut = wxNewId();
const long CustomModelDialog::ID_FILEPICKERCTRL1 = wxNewId();
const long CustomModelDialog::ID_SLIDER_CUSTOM_LIGHTNESS = wxNewId();
const long CustomModelDialog::ID_BITMAPBUTTON_CUSTOM_BKGRD = wxNewId();
const long CustomModelDialog::ID_CHECKBOX_AUTO_NUMBER = wxNewId();
const long CustomModelDialog::ID_CHECKBOX_AUTO_INCREMENT = wxNewId();
const long CustomModelDialog::ID_SPINCTRL_NEXT_CHANNEL = wxNewId();
const long CustomModelDialog::ID_BUTTON1 = wxNewId();
const long CustomModelDialog::ID_BUTTON2 = wxNewId();
const long CustomModelDialog::ID_NOTEBOOK1 = wxNewId();
//*)

const long CustomModelDialog::CUSTOMMODELDLGMNU_CUT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPY = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_PASTE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPH = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPV = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_ROTATE90 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_REVERSE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHIFT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_INSERT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COMPRESS = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHRINKSPACE10 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHRINKSPACE50 = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHRINKSPACE99 = wxNewId();

BEGIN_EVENT_TABLE(CustomModelDialog,wxDialog)
	//(*EventTable(CustomModelDialog)
	//*)
END_EVENT_TABLE()

// Subclassing wxGrid is the only way to get keyboard copy and paste working without breaking the grid behaviour
class CopyPasteGrid : public wxGrid
{
    void DoOnChar(wxKeyEvent& event)
    {
        wxChar uc = event.GetUnicodeKey();

        switch (uc)
        {
        case 'c':
        case 'C':
        case WXK_CONTROL_C:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_COPY);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'x':
        case 'X':
        case WXK_CONTROL_X:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_CUT);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        case 'v':
        case 'V':
        case WXK_CONTROL_V:
            if (event.CmdDown() || event.ControlDown()) {
                wxCommandEvent pasteEvent(wxEVT_TEXT_PASTE);
                wxPostEvent(this, pasteEvent);
                event.StopPropagation();
            }
            break;
        default:
            wxGrid::OnChar(event);
            break;
        }
    }

    public:
    CopyPasteGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name) : wxGrid(parent, id, pos, size, style, name)
    {
        Connect(wxEVT_CHAR, (wxObjectEventFunction)&CopyPasteGrid::DoOnChar, 0, this);
    }

    virtual ~CopyPasteGrid()
    {

    }
};

CustomModelDialog::CustomModelDialog(wxWindow* parent)
: background_image(""),
  bkg_image(nullptr),
  bkgrd_active(true),
  lightness(80),
  autonumber(false),
  autoincrement(false),
  next_channel(1)
{
	//(*Initialize(CustomModelDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* FlexGridSizer3;
	wxFlexGridSizer* FlexGridSizer4;
	wxFlexGridSizer* FlexGridSizer5;
	wxFlexGridSizer* FlexGridSizer6;
	wxFlexGridSizer* FlexGridSizer7;
	wxFlexGridSizer* FlexGridSizer8;
	wxFlexGridSizer* Sizer2;
	wxStaticBoxSizer* StaticBoxSizer1;
	wxStaticBoxSizer* StaticBoxSizer2;
	wxStaticText* StaticText1;
	wxStaticText* StaticText2;
	wxStaticText* StaticText3;

	Create(parent, wxID_ANY, _("Custom Model"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("wxID_ANY"));
	SetClientSize(wxDLG_UNIT(parent,wxSize(450,350)));
	SetMinSize(wxDLG_UNIT(parent,wxSize(300,200)));
	Sizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	Sizer1->AddGrowableCol(1);
	Sizer1->AddGrowableRow(0);
	Sizer2 = new wxFlexGridSizer(7, 1, 0, 0);
	Sizer2->AddGrowableCol(0);
	Sizer2->AddGrowableRow(6);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText1 = new wxStaticText(this, wxID_ANY, _("Width"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	WidthSpin = new wxSpinCtrl(this, ID_SPINCTRL1, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 500, 10, _T("ID_SPINCTRL1"));
	WidthSpin->SetValue(_T("10"));
	FlexGridSizer2->Add(WidthSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	HeightSpin = new wxSpinCtrl(this, ID_SPINCTRL2, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 500, 10, _T("ID_SPINCTRL2"));
	HeightSpin->SetValue(_T("10"));
	FlexGridSizer2->Add(HeightSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText4 = new wxStaticText(this, ID_STATICTEXT1, _("Depth"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText4, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrl_Depth = new wxSpinCtrl(this, ID_SPINCTRL3, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 100, 1, _T("ID_SPINCTRL3"));
	SpinCtrl_Depth->SetValue(_T("1"));
	FlexGridSizer2->Add(SpinCtrl_Depth, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
	ButtonWiring = new wxButton(this, ID_BUTTON3, _("Wiring View"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	FlexGridSizer8->Add(ButtonWiring, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL, 5);
	FlexGridSizer5 = new wxFlexGridSizer(0, 7, 0, 0);
	BitmapButtonCustomCut = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_CUT, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_CUT")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_CUT"));
	BitmapButtonCustomCut->SetToolTip(_("Cut"));
	FlexGridSizer5->Add(BitmapButtonCustomCut, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButtonCustomCopy = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_COPY, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_COPY")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_COPY"));
	BitmapButtonCustomCopy->SetDefault();
	BitmapButtonCustomCopy->SetToolTip(_("Copy"));
	FlexGridSizer5->Add(BitmapButtonCustomCopy, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButtonCustomPaste = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_PASTE, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_PASTE")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_PASTE"));
	BitmapButtonCustomPaste->SetDefault();
	BitmapButtonCustomPaste->SetToolTip(_("Paste"));
	FlexGridSizer5->Add(BitmapButtonCustomPaste, 1, wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CustomModelZoomIn = new wxButton(this, ID_BUTTON_CustomModelZoomIn, _("+"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CustomModelZoomIn"));
	Button_CustomModelZoomIn->SetMinSize(wxSize(24,-1));
	Button_CustomModelZoomIn->SetToolTip(_("Zoom In"));
	FlexGridSizer5->Add(Button_CustomModelZoomIn, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_CustomModelZoomOut = new wxButton(this, ID_BUTTON_CustomModelZoomOut, _("-"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON_CustomModelZoomOut"));
	Button_CustomModelZoomOut->SetMinSize(wxSize(24,-1));
	Button_CustomModelZoomOut->SetToolTip(_("Zoom Out"));
	FlexGridSizer5->Add(Button_CustomModelZoomOut, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer5, 1, wxTOP|wxLEFT|wxRIGHT|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _("Background Image"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
	FlexGridSizer1->AddGrowableCol(1);
	FilePickerCtrl1 = new ImageFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, wxEmptyString, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SliderCustomLightness = new wxSlider(this, ID_SLIDER_CUSTOM_LIGHTNESS, 0, 0, 100, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_SLIDER_CUSTOM_LIGHTNESS"));
	FlexGridSizer1->Add(SliderCustomLightness, 1, wxTOP|wxBOTTOM|wxLEFT|wxEXPAND, 5);
	BitmapButtonCustomBkgrd = new wxBitmapButton(this, ID_BITMAPBUTTON_CUSTOM_BKGRD, wxArtProvider::GetBitmap(wxART_MAKE_ART_ID_FROM_STR(_T("wxART_FIND")),wxART_BUTTON), wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW, wxDefaultValidator, _T("ID_BITMAPBUTTON_CUSTOM_BKGRD"));
	BitmapButtonCustomBkgrd->SetDefault();
	BitmapButtonCustomBkgrd->SetMinSize(wxSize(24,-1));
	FlexGridSizer1->Add(BitmapButtonCustomBkgrd, 1, wxTOP|wxBOTTOM|wxRIGHT, 5);
	StaticBoxSizer2->Add(FlexGridSizer1, 1, wxEXPAND, 5);
	Sizer2->Add(StaticBoxSizer2, 1, wxEXPAND, 5);
	StaticBoxSizer1 = new wxStaticBoxSizer(wxVERTICAL, this, _("Auto Numbering"));
	FlexGridSizer3 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer6 = new wxFlexGridSizer(0, 3, 0, 0);
	CheckBoxAutoNumber = new wxCheckBox(this, ID_CHECKBOX_AUTO_NUMBER, _("Active"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_NUMBER"));
	CheckBoxAutoNumber->SetValue(false);
	FlexGridSizer6->Add(CheckBoxAutoNumber, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	CheckBoxAutoIncrement = new wxCheckBox(this, ID_CHECKBOX_AUTO_INCREMENT, _("Auto Increment"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CHECKBOX_AUTO_INCREMENT"));
	CheckBoxAutoIncrement->SetValue(false);
	FlexGridSizer6->Add(CheckBoxAutoIncrement, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer6, 1, wxEXPAND, 5);
	FlexGridSizer4 = new wxFlexGridSizer(0, 2, 0, 0);
	StaticText3 = new wxStaticText(this, wxID_ANY, _("Channel"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer4->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SpinCtrlNextChannel = new wxSpinCtrl(this, ID_SPINCTRL_NEXT_CHANNEL, _T("1"), wxDefaultPosition, wxDefaultSize, 0, 1, 9999999, 1, _T("ID_SPINCTRL_NEXT_CHANNEL"));
	SpinCtrlNextChannel->SetValue(_T("1"));
	FlexGridSizer4->Add(SpinCtrlNextChannel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer3->Add(FlexGridSizer4, 1, wxEXPAND, 5);
	StaticBoxSizer1->Add(FlexGridSizer3, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(StaticBoxSizer1, 1, wxALIGN_LEFT, 5);
	FlexGridSizer7 = new wxFlexGridSizer(0, 3, 0, 0);
	ButtonOk = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	FlexGridSizer7->Add(ButtonOk, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	ButtonCancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer7->Add(ButtonCancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Sizer2->Add(FlexGridSizer7, 1, wxALL|wxALIGN_BOTTOM|wxALIGN_CENTER_HORIZONTAL, 5);
	Sizer1->Add(Sizer2, 1, wxALL|wxEXPAND, 5);
	Notebook1 = new wxNotebook(this, ID_NOTEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_NOTEBOOK1"));
	Sizer1->Add(Notebook1, 1, wxALL|wxEXPAND, 5);
	SetSizer(Sizer1);
	SetSizer(Sizer1);
	Layout();
	Center();

	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnWidthSpinChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnHeightSpinChange);
	Connect(ID_SPINCTRL3,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnSpinCtrl_DepthChange);
	Connect(ID_BUTTON3,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButtonWiringClick);
	Connect(ID_BITMAPBUTTON_CUSTOM_CUT,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomCutClick);
	Connect(ID_BITMAPBUTTON_CUSTOM_COPY,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomCopyClick);
	Connect(ID_BITMAPBUTTON_CUSTOM_PASTE,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomPasteClick);
	Connect(ID_BUTTON_CustomModelZoomIn,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButton_CustomModelZoomInClick);
	Connect(ID_BUTTON_CustomModelZoomOut,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButton_CustomModelZoomOutClick);
	Connect(ID_FILEPICKERCTRL1,wxEVT_COMMAND_FILEPICKER_CHANGED,(wxObjectEventFunction)&CustomModelDialog::OnFilePickerCtrl1FileChanged);
	Connect(ID_SLIDER_CUSTOM_LIGHTNESS,wxEVT_COMMAND_SLIDER_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnSliderCustomLightnessCmdSliderUpdated);
	Connect(ID_BITMAPBUTTON_CUSTOM_BKGRD,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnBitmapButtonCustomBkgrdClick);
	Connect(ID_CHECKBOX_AUTO_NUMBER,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBoxAutoNumberClick);
	Connect(ID_CHECKBOX_AUTO_INCREMENT,wxEVT_COMMAND_CHECKBOX_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnCheckBoxAutoIncrementClick);
	Connect(ID_SPINCTRL_NEXT_CHANNEL,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnSpinCtrlNextChannelChange);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButtonOkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&CustomModelDialog::OnButtonCancelClick);
	Connect(ID_NOTEBOOK1,wxEVT_COMMAND_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&CustomModelDialog::OnNotebook1PageChanged);
	//*)

    name = "";

    SetEscapeId(ButtonCancel->GetId());

    ValidateWindow();
}

CustomModelDialog::~CustomModelDialog()
{
	//(*Destroy(CustomModelDialog)
	//*)

	if( bkg_image != nullptr ) {
        delete bkg_image;
    }
}

void CustomModelDialog::ValidateWindow()
{
}

void CustomModelDialog::Setup(CustomModel *m) {
    name = m->GetName();
    background_image = m->GetCustomBackground();
    FilePickerCtrl1->SetFileName(wxFileName(background_image));
    lightness = m->GetCustomLightness();
    SliderCustomLightness->SetValue(lightness);
    std::string data = m->GetCustomData();
    if (data == "") {
        WidthSpin->SetValue(5);
        HeightSpin->SetValue(5);
        SpinCtrl_Depth->SetValue(1);
        ResizeCustomGrid();
        return;
    }

    WidthSpin->SetValue(1);
    HeightSpin->SetValue(1);
    SpinCtrl_Depth->SetValue(1);
    ResizeCustomGrid();

    if (background_image != "" && wxFile::Exists(background_image)) {
        bkg_image = new wxImage(background_image);
    }

    wxArrayString layers=wxSplit(data, '|');
    for (auto layer = 0; layer < layers.size(); layer++)
    {
        AddPage();
        auto grid = GetLayerGrid(layer);
        wxArrayString rows = wxSplit(layers[layer], ';');
        for (auto row = 0; row < rows.size(); row++)
        {
            if (row >= grid->GetNumberRows()) grid->AppendRows();
            wxArrayString cols = wxSplit(rows[row], ',');
            for (auto col = 0; col < cols.size(); col++)
            {
                if (col >= grid->GetNumberCols()) grid->AppendCols();
                wxString value = cols[col];
                if (!value.IsEmpty() && value != "0")
                {
                    grid->SetCellValue(row, col, value);
                }
            }
        }

        wxFont font = grid->GetDefaultCellFont();
        grid->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
        grid->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
        for (int c = 0; c < grid->GetNumberCols(); ++c)
            grid->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 4/5);
        for (int r = 0; r < grid->GetNumberRows(); ++r)
            grid->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 4/5);
        font = grid->GetLabelFont();
        grid->SetColLabelSize(int(1.5 * (float)font.GetPixelSize().y));
        //Sizer1->Fit(this);
    }

    UpdateBackground();

    WidthSpin->SetValue(GetActiveGrid()->GetNumberCols());
    HeightSpin->SetValue(GetActiveGrid()->GetNumberRows());
    SpinCtrl_Depth->SetValue(Notebook1->GetPageCount());

    wxBookCtrlEvent e;
    e.SetSelection(0);
    OnNotebook1PageChanged(e);

    Sizer1->Layout();
    Layout();

    // This does not stop the user entering the illegal characters
    //        wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    //        wxString filter("0123456789");
    //        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    //        validator.SetCharIncludes(filter);
    //        reditor->SetValidator(validator);
    //        GridCustom->SetDefaultEditor(reditor);

    // neither does this
    //for (int r = 0; r < GridCustom->GetNumberRows(); ++r)
    //{
    //    for (int c = 0; c < GridCustom->GetNumberCols(); ++c)
    //    {
    //        wxGridCellTextEditor *reditor = new wxGridCellTextEditor();
    //        wxString filter("0123456789");
    //        wxTextValidator validator(wxFILTER_INCLUDE_CHAR_LIST);
    //        validator.SetCharIncludes(filter);
    //        reditor->SetValidator(validator);
    //        //GridCustom->SetDefaultEditor(reditor);
    //        GridCustom->SetCellEditor(r, c, reditor);
    //    }
    //}
    ValidateWindow();
}

// make grid the size specified by the spin controls
void CustomModelDialog::ResizeCustomGrid()
{
    int numCols=WidthSpin->GetValue();
    int numRows=HeightSpin->GetValue();
    for (auto grid : _grids)
    {
        int deltaCols = numCols - grid->GetNumberCols();
        int deltaRows = numRows - grid->GetNumberRows();
        if (deltaCols > 0) grid->AppendCols(deltaCols);
        if (deltaRows > 0) grid->AppendRows(deltaRows);
        if (deltaCols < 0) grid->DeleteCols(numCols, -deltaCols);
        if (deltaRows < 0) grid->DeleteRows(numRows, -deltaRows);
    }
    UpdateBackground();
}

wxString StripIllegalChars(const wxString& s)
{
    wxString res = "";

    for (auto it = s.begin(); it != s.end(); ++it)
    {
        if (*it >= '0' && *it <= '9')
        {
            res += *it;
        }
    }

    return res;
}

void CustomModelDialog::Save(CustomModel *m) {
    m->SetCustomHeight(HeightSpin->GetValue());
    m->SetCustomWidth(WidthSpin->GetValue());
    m->SetCustomDepth(SpinCtrl_Depth->GetValue());
    std::string customChannelData = "";
    for (int layer = 0; layer < Notebook1->GetPageCount(); layer++) {
        if (layer > 0) customChannelData += "|";
        auto grid = GetLayerGrid(layer);
        int numCols = grid->GetNumberCols();
        int numRows = grid->GetNumberRows();
        for (int row = 0; row < numRows; row++) {
            if (row > 0) customChannelData += ";";
            for (int col = 0; col < numCols; col++) {
                if (col > 0) customChannelData += ",";
                wxString value = StripIllegalChars(grid->GetCellValue(row, col));
                if (value == "0" || value.StartsWith("-")) value.clear();
                customChannelData += value;
            }
        }
    }
    m->SetCustomData(customChannelData);
    m->SetCustomLightness(lightness);
    m->SetCustomBackground(FilePickerCtrl1->GetFileName().GetFullPath());
}

void CustomModelDialog::OnWidthSpinChange(wxSpinEvent& event)
{
    ResizeCustomGrid();
}

void CustomModelDialog::OnHeightSpinChange(wxSpinEvent& event)
{
    ResizeCustomGrid();
}

void CustomModelDialog::OnButton_CustomModelZoomInClick(wxCommandEvent& event)
{
    for (auto grid : _grids)
    {
        grid->BeginBatch();
        wxFont font = grid->GetLabelFont();
        font.MakeLarger();
        grid->SetLabelFont(font);
        font = grid->GetDefaultCellFont();
        font.MakeLarger();
        grid->SetDefaultCellFont(font);
        for (int c = 0; c < grid->GetNumberCols(); ++c)
            grid->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
        for (int r = 0; r < grid->GetNumberRows(); ++r)
            grid->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
        grid->EndBatch();
    }
    UpdateBackground();
}

void CustomModelDialog::OnButton_CustomModelZoomOutClick(wxCommandEvent& event)
{
    for (auto grid : _grids)
    {
        grid->BeginBatch();
        wxFont font = grid->GetLabelFont();
        font.MakeSmaller();
        grid->SetLabelFont(font);
        font = grid->GetDefaultCellFont();
        font.MakeSmaller();
        grid->SetDefaultCellFont(font);
        grid->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
        grid->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
        for (int c = 0; c < grid->GetNumberCols(); ++c)
            grid->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 4/5);
        for (int r = 0; r < grid->GetNumberRows(); ++r)
            grid->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 4/5);
        grid->EndBatch();
        UpdateBackground();
    }
}

void CustomModelDialog::OnButtonCustomModelHelpClick(wxCommandEvent& event)
{
    DisplayInfo("To create a custom model, set 'Display As' to 'Custom';\nthen set the model width and model height values.\n\nA custom model allows you to represent almost any display element \ncreated from RGB lights. If your element has 12 RGB nodes in it, then \nyou will need to place the numbers 1 through 12 in the grid. \nPlace the numbers so that they are in the shape of your display \nelement. If you enter a number and then need to erase it, enter a 0.\n\nNode 1 will be assigned:\n  StartChannel\n  StartChannel+1\n  StartChannel+2\nNode 2 will be assigned:\n  StartChannel+3\n  StartChannel+4\n  StartChannel+5\netc.\n\nFor example, to model a candy cane with 12 nodes, you could \nstart with a grid 4 columns wide and 10 rows high. You would place the \nnumbers 1-9 up the right-hand side, 10 and 11 would go in the middle \ncells in the top row, and then 12 would go in column A, row 2.\n\n     11 10\n12           9\n               8\n               7\n               6\n               5\n               4\n               3\n               2\n               1");
}

void CustomModelDialog::OnGridCustomCellChange(wxGridEvent& event)
{
}

#ifdef __WXOSX__
wxString GetOSXFormattedClipboardData();
#endif

void CustomModelDialog::OnBitmapButtonCustomCutClick(wxCommandEvent& event)
{
    CutOrCopyToClipboard(true);
}

void CustomModelDialog::OnBitmapButtonCustomCopyClick(wxCommandEvent& event)
{
    CutOrCopyToClipboard(false);
}

void CustomModelDialog::CutOrCopyToClipboard(bool IsCut) {

    wxString copy_data;

    auto grid = GetActiveGrid();

    for (int i = 0; i< grid->GetNumberRows(); i++)        // step through all lines
    {
        bool something_in_this_line = false;             // nothing found yet
        for (int k = 0; k<grid->GetNumberCols(); k++)     // step through all colums
        {
            if (grid->IsInSelection(i,k))     // this field is selected!!!
            {
                if (!something_in_this_line)        // first field in this line => may need a linefeed
                {
                    if (!copy_data.IsEmpty())       // ... if it is not the very first field
                    {
                        copy_data += "\n";     // next LINE
                    }
                    something_in_this_line = true;
                }
                else                                    // if not the first field in this line we need a field seperator (TAB)
                {
                    copy_data += "\t";  // next COLUMN
                }
                copy_data += StripIllegalChars(grid->GetCellValue(i,k));    // finally we need the field value
                if (IsCut) grid->SetCellValue(i,k,wxEmptyString);
            }
        }
    }

    if (copy_data.IsEmpty())
    {
        copy_data += StripIllegalChars(grid->GetCellValue(grid->GetGridCursorRow(), grid->GetGridCursorCol()));    // finally we need the field value
        if (IsCut) grid->SetCellValue(grid->GetGridCursorRow(), grid->GetGridCursorCol(), wxEmptyString);
    }

    if (wxTheClipboard->Open())
    {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data)))
        {
            DisplayError(_("Unable to copy data to clipboard."), this);
        }
        wxTheClipboard->Close();
    }
    else
    {
        DisplayError(_("Error opening clipboard."), this);
    }

}

void CustomModelDialog::Paste()
{
    wxString copy_data = "";

#ifdef __WXOSX__
    //wxDF_TEXT gets a very strange formatted string from the clipboard if using Numbers
    //native ObjectC code can get the proper tab formatted version.
    copy_data = GetOSXFormattedClipboardData();
#endif

    if (copy_data == "") {
        if (wxTheClipboard->Open())
        {
            if (wxTheClipboard->IsSupported(wxDF_TEXT))
            {
                wxTextDataObject data;

                if (wxTheClipboard->GetData(data))
                {
                    copy_data = data.GetText();
                }
                else
                {
                    DisplayError(_("Unable to copy data from clipboard."), this);
                }
            }
            else
            {
                DisplayError(_("Non-Text data in clipboard."), this);
            }
            wxTheClipboard->Close();
        }
        else
        {
            DisplayError(_("Error opening clipboard."), this);
            return;
        }
    }

    auto grid = GetActiveGrid();

    int i = grid->GetGridCursorRow();
    int k = grid->GetGridCursorCol();
    int numrows = grid->GetNumberRows();
    int numcols = grid->GetNumberCols();
    bool errflag = false;
    wxString errdetails; //-DJ

    copy_data.Replace("\r\r", "\n");
    copy_data.Replace("\r\n", "\n");
    copy_data.Replace("\r", "\n");

    do
    {
        wxString cur_line = copy_data.BeforeFirst('\n');
        copy_data = copy_data.AfterFirst('\n');
        wxArrayString fields = wxSplit(cur_line, (cur_line.Find(',') != wxNOT_FOUND) ? ',' : '\t'); //allow comma or tab delim -DJ
        for (int fieldnum = 0; fieldnum < fields.Count(); fieldnum++)
        {
            if (i < numrows && k + fieldnum < numcols)
            {
                wxString field = fields[fieldnum].Trim(true).Trim(false);
                long val;
                if (field.IsEmpty() || field.ToLong(&val))
                {
                    grid->SetCellValue(i, k + fieldnum, fields[fieldnum].Trim(true).Trim(false)); //strip surrounding spaces -DJ
                }
                else
                {
                    errflag = true;
                    errdetails += wxString::Format("\n'%s' row %d/col %d of %d", fields[fieldnum].c_str(), i - grid->GetGridCursorRow(), fieldnum, fields.Count()); //tell the user what was wrong; show relative row#, col# (more user friendly) -DJ
                }
            }
        }
        i++;
    } while (copy_data.IsEmpty() == false);

    if (errflag)
    {
        DisplayError(_("One or more of the values were not pasted because they did not contain a number") + errdetails, this); //-DJ
    }
}

void CustomModelDialog::OnBitmapButtonCustomPasteClick(wxCommandEvent& event)
{
    Paste();
}

void CustomModelDialog::UpdateBackground()
{
    auto r = _renderers.begin();
    for (auto grid : _grids)
    {
        if (*r != nullptr)
        {
            (*r)->UpdateSize(*grid, bkgrd_active, lightness);
        }
        ++r;
    }
}

void CustomModelDialog::OnBitmapButtonCustomBkgrdClick(wxCommandEvent& event)
{
    bkgrd_active = !bkgrd_active;
    GetActiveGrid()->Refresh();
    UpdateBackground();
}

void CustomModelDialog::OnSliderCustomLightnessCmdSliderUpdated(wxScrollEvent& event)
{
    lightness = SliderCustomLightness->GetValue();
    UpdateBackground();
    Refresh();
}

void CustomModelDialog::OnCheckBoxAutoNumberClick(wxCommandEvent& event)
{
    autonumber = CheckBoxAutoNumber->GetValue();
}

void CustomModelDialog::OnCheckBoxAutoIncrementClick(wxCommandEvent& event)
{
    autoincrement = CheckBoxAutoIncrement->GetValue();
}

void CustomModelDialog::OnSpinCtrlNextChannelChange(wxSpinEvent& event)
{
    next_channel = SpinCtrlNextChannel->GetValue();
}

void CustomModelDialog::OnGridCustomCellLeftClick(wxGridEvent& event)
{
    if( autonumber ) {
        GetActiveGrid()->SetCellValue(event.GetRow(), event.GetCol(), wxString::Format("%d", next_channel) );
        if( autoincrement ) {
            next_channel++;
            SpinCtrlNextChannel->SetValue(next_channel);
        }
    }
    event.Skip();
}

void CustomModelDialog::OnCheckBox_RearViewClick(wxCommandEvent& event)
{
    for (auto grid : _grids)
    {
        // reverse the rows
        for (auto r = 0; r < grid->GetNumberRows(); r++)
        {
            std::list<wxString> vals;
            for (auto c = 0; c < grid->GetNumberCols(); ++c)
            {
                vals.push_front(grid->GetCellValue(r, c));
            }
            auto c = 0;
            for (auto it : vals)
            {
                grid->SetCellValue(r, c++, it);
            }
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::OnButtonCancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void CustomModelDialog::OnButtonOkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void CustomModelDialog::OnButtonWiringClick(wxCommandEvent& event)
{
    if (Notebook1->GetPageCount() == 1)
    {
        WiringDialog dlg(this, name);
        dlg.SetData(GetActiveGrid(), false);
        dlg.ShowModal();
    }
    else
    {
        wxMessageBox("Wiring view cannot be displayed for 3D models.");
    }
}

void CustomModelDialog::FlipHorizontal()
{
    // reverse the rows
    for (auto grid : _grids)
    {
        for (auto r = 0; r < grid->GetNumberRows(); r++)
        {
            std::list<wxString> vals;
            for (auto c = 0; c < grid->GetNumberCols(); ++c)
            {
                vals.push_front(grid->GetCellValue(r, c));
            }
            auto c = 0;
            for (auto it : vals)
            {
                grid->SetCellValue(r, c++, it);
            }
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::FlipVertical()
{
    // reverse the columns
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            std::list<wxString> vals;
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                vals.push_front(grid->GetCellValue(r, c));
            }
            auto r = 0;
            for (auto it : vals)
            {
                grid->SetCellValue(r++, c, it);
            }
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::Rotate90()
{
    for (auto grid : _grids)
    {
        if (grid->GetNumberCols() > grid->GetNumberRows())
        {
            grid->AppendRows(grid->GetNumberCols() - grid->GetNumberRows());
        }
        else if (grid->GetNumberCols() < grid->GetNumberRows())
        {
            grid->AppendCols(grid->GetNumberRows() - grid->GetNumberCols());
        }
    }

    WidthSpin->SetValue(GetActiveGrid()->GetNumberCols());
    HeightSpin->SetValue(GetActiveGrid()->GetNumberRows());
    ResizeCustomGrid();

    for (auto grid : _grids)
    {
        const int n = grid->GetNumberCols();
        for (int i = 0; i < n / 2; i++) {
            for (int j = i; j < n - i - 1; j++) {
                const wxString tmp = grid->GetCellValue(i, j);
                grid->SetCellValue(i, j, grid->GetCellValue(n - j - 1, i));
                grid->SetCellValue(n - j - 1, i, grid->GetCellValue(n - i - 1, n - j - 1));
                grid->SetCellValue(n - i - 1, n - j - 1, grid->GetCellValue(j, n - i - 1));
                grid->SetCellValue(j, n - i - 1, tmp);
            }
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::Insert(int selRow, int selCol)
{
    long val;
    auto value = GetActiveGrid()->GetCellValue(selRow, selCol);
    value.ToCLong(&val);
    wxNumberEntryDialog dlg(this, wxString::Format("Number of nodes to create a gap for prior to node %ld.", val), "Nodes to create a gap for", "Insert", 1, 1, 50);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto toinsert = dlg.GetValue();

        //Find the max value returned
        auto max = 0;
        for (auto grid : _grids)
        {
            for (auto c = 0; c < grid->GetNumberCols(); c++)
            {
                for (auto r = 0; r < grid->GetNumberRows(); ++r)
                {
                    wxString s = grid->GetCellValue(r, c);

                    if (s.IsEmpty() == false)
                    {
                        long v;
                        if (s.ToCLong(&v) == true)
                        {
                            if (v > max)max = v;
                        }
                    }
                }
            }
        }

        for (auto current = max; current >= val; current--)
        {
            AdjustNodeBy(current, toinsert);
        }
    }
}

bool CustomModelDialog::AdjustNodeBy(int node, int adjust)
{
    bool adjusted = false;
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;
                    s.ToCLong(&val);
                    if (val == node)
                    {
                        grid->SetCellValue(r, c, wxString::Format("%d", val + adjust));
                        adjusted = true;
                    }
                }
            }
        }
    }

    return adjusted;
}

void CustomModelDialog::Compress()
{
    //Find the max value returned
    auto max = 0;
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;
                    if (s.ToCLong(&val) == true)
                    {
                        if (val > max)max = val;
                    }
                }
            }
        }
    }

    int adjust = 0;
    for (int current = 1; current <= max; current++)
    {
        if (!AdjustNodeBy(current, adjust))
        {
            adjust--;
        }
    }
}

void CustomModelDialog::Reverse()
{
    auto min = 1;
    auto max = 1;

    //Find the max value returned
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;

                    if (s.ToCLong(&val) == true)
                    {
                        if (val > max)max = val;
                        if (val < min)min = val;
                    }
                }
            }
        }
    }

    max++;
    //Rewrite the grid values
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            std::list<wxString> vals;
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;

                    if (s.ToCLong(&val) == true)
                    {
                        long newVal = max - val;
                        s.Printf("%d", newVal);

                        grid->SetCellValue(r, c, s);
                    }
                }
            }
        }
    }

    UpdateBackground();
    ValidateWindow();
}

bool CustomModelDialog::CheckScale(std::list<wxPoint>& points, float scale) const
{
    std::list<wxPoint> newPoints;

    for (auto it = points.begin(); it != points.end(); ++it)
    {
        int newX = (int)((float)it->x * scale);
        int newY = (int)((float)it->y * scale);

        for (auto it2 = newPoints.begin(); it2 != newPoints.end(); ++it2)
        {
            if (it2->x == newX && it2->y == newY)
            {
                return false;
            }
        }
        newPoints.push_back(wxPoint(newX, newY));
    }

    return true;
}

void CustomModelDialog::ShrinkSpace(float min)
{
    TrimSpace();

    float scale = min;
    float scaleFactor = (1.0 - min) / 2.0;

    std::vector<std::list<wxPoint>> points;

    for (auto grid : _grids)
    {
        std::list<wxPoint> gpoints;
        for (int c = 0; c < grid->GetNumberCols(); c++)
        {
            for (int r = 0; r < grid->GetNumberRows(); r++)
            {
                if (!grid->GetCellValue(r, c).IsEmpty())
                {
                    gpoints.push_back(wxPoint(r, c));
                }
            }
        }

        float gscaleFactor = (1.0 - min) / 2.0;
        float gscale = min + gscaleFactor;

        for (int i = 0; i < 5; i++)
        {
            gscaleFactor /= 2.0;
            if (CheckScale(gpoints, gscale))
            {
                gscale -= gscaleFactor;
            }
            else
            {
                gscale += gscaleFactor;
            }
        }

        if (gscale > scale)
        {
            scale = gscale;
            scaleFactor = gscaleFactor;
        }
    }


    if (scale < min) scale = min;

    for (auto gpoints : points)
    {
        if (!CheckScale(gpoints, scale))
        {
            scale += scaleFactor;
            if (!CheckScale(gpoints, scale))
            {
                // cant scale
                return;
            }
        }
    }

    int i = 0;
    for (auto grid : _grids)
    {
        auto gpoints = points[i];
        for (auto it = gpoints.begin(); it != gpoints.end(); ++it)
        {
            int newX = (int)((float)it->x * scale);
            int newY = (int)((float)it->y * scale);

            if (newX != it->x || newY != it->y)
            {
                wxASSERT(grid->GetCellValue(newX, newY).IsEmpty());

                grid->SetCellValue(newX, newY, grid->GetCellValue(it->x, it->y));
                grid->SetCellValue(it->x, it->y, "");
            }
        }
        i++;
    }

    TrimSpace();
}

void CustomModelDialog::TrimSpace()
{
    int minRow = 99999;
    int maxRow = -99999;
    int minCol = 99999;
    int maxCol = -99999;

    //Find the max value returned
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (!s.IsEmpty())
                {
                    if (c < minCol) minCol = c;
                    if (r < minRow) minRow = r;
                    if (c > maxCol) maxCol = c;
                    if (r > maxRow) maxRow = r;
                }
            }
        }
    }

    if (minCol == 0 && minRow == 0 && maxRow == GetActiveGrid()->GetNumberRows() && maxCol == GetActiveGrid()->GetNumberCols())
    {
        // already the right size
        return;
    }

    int adjustRows = -1 * minRow;
    int adjustCols = -1 * minCol;

    for (auto grid : _grids)
    {
        for (int r = minRow; r <= maxRow; r++)
        {
            for (int c = minCol; c <= maxCol; c++)
            {
                grid->SetCellValue(r + adjustRows, c + adjustCols, grid->GetCellValue(r, c));
            }
        }
    }

    WidthSpin->SetValue(std::max(1, maxCol - minCol + 1));
    HeightSpin->SetValue(std::max(1, maxRow - minRow + 1));
    ResizeCustomGrid();
}

void CustomModelDialog::Shift()
{
    auto min = 1;
    auto max = 1;

    //Find the max value returned
    for (auto grid : _grids)
    {
        for (auto c = 0; c < grid->GetNumberCols(); c++)
        {
            for (auto r = 0; r < grid->GetNumberRows(); ++r)
            {
                wxString s = grid->GetCellValue(r, c);

                if (s.IsEmpty() == false)
                {
                    long val;

                    if (s.ToCLong(&val) == true)
                    {
                        if (val > max)max = val;
                        if (val < min)min = val;
                    }
                }
            }
        }
    }

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0)
        {
            for (auto grid : _grids)
            {
                //Rewrite the grid values
                for (auto c = 0; c < grid->GetNumberCols(); c++)
                {
                    std::list<wxString> vals;
                    for (auto r = 0; r < grid->GetNumberRows(); ++r)
                    {
                        wxString s = grid->GetCellValue(r, c);

                        if (s.IsEmpty() == false)
                        {
                            long val;

                            if (s.ToCLong(&val) == true)
                            {
                                long newVal = val + scaleFactor;
                                if (newVal > max)
                                {
                                    newVal -= max;
                                }
                                else if (newVal < min)
                                {
                                    newVal += max;
                                }
                                s.Printf("%d", newVal);

                                grid->SetCellValue(r, c, s);
                            }
                        }
                    }
                }
            }

            UpdateBackground();
            ValidateWindow();
        }
    }
}

void CustomModelDialog::OnPaste(wxCommandEvent& event)
{
    Paste();
}

void CustomModelDialog::OnGridPopup(wxCommandEvent& event)
{
    int id = event.GetId();
    if (id == CUSTOMMODELDLGMNU_CUT)
    {
        CutOrCopyToClipboard(true);
    }
    else if (id == CUSTOMMODELDLGMNU_COPY)
    {
        CutOrCopyToClipboard(false);
    }
    else if (id == CUSTOMMODELDLGMNU_PASTE)
    {
        Paste();
    }
    else if (id == CUSTOMMODELDLGMNU_FLIPH)
    {
        FlipHorizontal();
    }
    else if (id == CUSTOMMODELDLGMNU_FLIPV)
    {
        FlipVertical();
    }
    else if (id == CUSTOMMODELDLGMNU_ROTATE90)
    {
        Rotate90();
    }
    else if (id == CUSTOMMODELDLGMNU_REVERSE)
    {
        Reverse();
    }
    else if (id == CUSTOMMODELDLGMNU_SHIFT)
    {
        Shift();
    }
    else if (id == CUSTOMMODELDLGMNU_INSERT)
    {
        Insert(_selRow, _selCol);
    }
    else if (id == CUSTOMMODELDLGMNU_COMPRESS)
    {
        Compress();
    }
    else if (id == CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE)
    {
        TrimSpace();
    }
    else if (id == CUSTOMMODELDLGMNU_SHRINKSPACE10)
    {
        ShrinkSpace(0.9f);
    }
    else if (id == CUSTOMMODELDLGMNU_SHRINKSPACE50)
    {
        ShrinkSpace(0.5f);
    }
    else if (id == CUSTOMMODELDLGMNU_SHRINKSPACE99)
    {
        ShrinkSpace(0.01f);
    }
}

void CustomModelDialog::OnCut(wxCommandEvent& event)
{
    CutOrCopyToClipboard(true);
}

void CustomModelDialog::OnCopy(wxCommandEvent& event)
{
    CutOrCopyToClipboard(false);
}

void CustomModelDialog::OnFilePickerCtrl1FileChanged(wxFileDirPickerEvent& event)
{
    background_image = FilePickerCtrl1->GetFileName().GetFullPath();

    if (background_image != "") {
        if (wxFile::Exists(background_image)) {
            bkg_image = new wxImage(background_image);
        }
        else
        {
            bkg_image = nullptr;
        }
        for (auto r : _renderers)
        {
            r->SetImage(bkg_image);
        }
        UpdateBackground();
        GetActiveGrid()->Refresh();
    }
}

void CustomModelDialog::OnGridCustomCellRightClick(wxGridEvent& event)
{
    _selRow = event.GetRow();
    _selCol = event.GetCol();
    GetActiveGrid()->SetGridCursor(_selRow, _selCol);
    auto s = GetActiveGrid()->GetCellValue(_selRow, _selCol);
    bool selectedCellWithValue = !s.IsEmpty() && s.IsNumber();

    wxMenu mnu;
    // Copy / Paste / Delete
    wxMenuItem* menu_cut = mnu.Append(CUSTOMMODELDLGMNU_CUT, "Cut");
    wxMenuItem* menu_copy = mnu.Append(CUSTOMMODELDLGMNU_COPY, "Copy");
    wxMenuItem* menu_paste = mnu.Append(CUSTOMMODELDLGMNU_PASTE, "Paste");
    if (GetActiveGrid()->GetSelectedCells().size() > 0)
    {
        menu_cut->Enable(true);
        menu_copy->Enable(true);
        menu_paste->Enable(true);
    }
    else
    {
        menu_cut->Enable(false);
        menu_copy->Enable(false);
        menu_paste->Enable(false);
    }

    mnu.AppendSeparator();

    mnu.Append(CUSTOMMODELDLGMNU_FLIPH, "Horizontal Flip");
    mnu.Append(CUSTOMMODELDLGMNU_FLIPV, "Vertical Flip");
    mnu.Append(CUSTOMMODELDLGMNU_ROTATE90, "Rotate 90");
    mnu.Append(CUSTOMMODELDLGMNU_REVERSE, "Reverse");
    mnu.Append(CUSTOMMODELDLGMNU_SHIFT, "Shift");
    wxMenuItem* menu_insert = mnu.Append(CUSTOMMODELDLGMNU_INSERT, "Insert Prior");
    mnu.Append(CUSTOMMODELDLGMNU_COMPRESS, "Compress");
    mnu.Append(CUSTOMMODELDLGMNU_TRIMUNUSEDSPACE, "Trim Unused Space");
    mnu.Append(CUSTOMMODELDLGMNU_SHRINKSPACE10, "Shrink Space - Max 10%");
    mnu.Append(CUSTOMMODELDLGMNU_SHRINKSPACE50, "Shrink Space - Max 50%");
    mnu.Append(CUSTOMMODELDLGMNU_SHRINKSPACE99, "Shrink Space - Max 99%");

    if (selectedCellWithValue)
    {
        menu_insert->Enable(true);
    }
    else
    {
        menu_insert->Enable(false);
    }
    mnu.Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&CustomModelDialog::OnGridPopup, nullptr, this);
    PopupMenu(&mnu);
}

void CustomModelDialog::OnSpinCtrl_DepthChange(wxSpinEvent& event)
{
    while (Notebook1->GetPageCount() < SpinCtrl_Depth->GetValue())
    {
        AddPage();
    }

    while (Notebook1->GetPageCount() > SpinCtrl_Depth->GetValue())
    {
        RemovePage();
    }
}

void CustomModelDialog::AddPage()
{
    auto id = wxNewId();
    wxPanel* p = new wxPanel(Notebook1, wxNewId(), wxPoint(26, 52), wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    auto sizer = new wxFlexGridSizer(0, 1, 0, 0);
    sizer->AddGrowableCol(0);
    sizer->AddGrowableRow(0);
    auto grid = new CopyPasteGrid(p, id, wxDefaultPosition, wxDefaultSize, wxVSCROLL | wxHSCROLL, _T("ID_GRID_Custom"));
    grid->CreateGrid(1, 1);
    grid->EnableEditing(true);
    grid->EnableGridLines(true);
    grid->SetColLabelSize(20);
    grid->SetRowLabelSize(30);
    grid->SetDefaultColSize(30, true);
    grid->SetDefaultCellFont(grid->GetFont());
    grid->SetDefaultCellTextColour(grid->GetForegroundColour());
    sizer->Add(grid, 0, wxEXPAND, 0);
    p->SetSizer(sizer);
    sizer->Fit(p);
    sizer->SetSizeHints(p);
    Notebook1->AddPage(p, wxString::Format("%d", (int)Notebook1->GetPageCount() + 1), false);

    Connect(id, wxEVT_GRID_CELL_LEFT_CLICK, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellLeftClick);
    Connect(id, wxEVT_GRID_CELL_RIGHT_CLICK, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellRightClick);
    Connect(id, wxEVT_GRID_CELL_CHANGED, (wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellChange);

    grid->Connect(wxEVT_TEXT_CUT, (wxObjectEventFunction)&CustomModelDialog::OnCut, 0, this);
    grid->Connect(wxEVT_TEXT_COPY, (wxObjectEventFunction)&CustomModelDialog::OnCopy, 0, this);
    grid->Connect(wxEVT_TEXT_PASTE, (wxObjectEventFunction)&CustomModelDialog::OnPaste, 0, this);

    grid->EnableDragColSize(false);
    grid->EnableDragGridSize(false);
    grid->EnableDragRowSize(false);

    while (grid->GetNumberCols() < WidthSpin->GetValue())
    {
        grid->AppendCols();
    }

    while (grid->GetNumberRows() < HeightSpin->GetValue())
    {
        grid->AppendRows();
    }

    auto renderer = new wxModelGridCellRenderer(bkg_image, *grid);
    grid->SetDefaultRenderer(renderer);
    _renderers.push_back(renderer);

    _grids.push_back(grid);
}

CopyPasteGrid* CustomModelDialog::GetActiveGrid() const
{
    return _grids[Notebook1->GetSelection()];
}

CopyPasteGrid* CustomModelDialog::GetLayerGrid(int layer) const
{
    return _grids[layer];
}

void CustomModelDialog::RemovePage()
{
    _grids.pop_back();
    _renderers.pop_back();
    Notebook1->RemovePage(Notebook1->GetPageCount() - 1);
}

void CustomModelDialog::OnNotebook1PageChanged(wxNotebookEvent& event)
{
    bool first = (Notebook1->GetSelection() == 0);
    bool last = (Notebook1->GetSelection() == Notebook1->GetPageCount() - 1);

    CopyPasteGrid* prior = nullptr;
    if (!first) prior = _grids[Notebook1->GetSelection() - 1];

    CopyPasteGrid* next = nullptr;
    if (!last) next = _grids[Notebook1->GetSelection() + 1];

    wxColor priorc = wxColor(255, 200, 200);
    wxColor nextc = wxColor(200, 200, 255);
    wxColor priornextc = wxColor(255, 200, 255);

    for (auto c = 0; c < GetActiveGrid()->GetNumberCols(); c++)
    {
        for (auto r = 0; r < GetActiveGrid()->GetNumberRows(); ++r)
        {
            int state = 0;

            if (prior != nullptr && !prior->GetCellValue(r, c).IsEmpty()) state += 1;
            if (next != nullptr && !next->GetCellValue(r, c).IsEmpty()) state += 2;

            if (state == 0)
            {
                GetActiveGrid()->SetCellBackgroundColour(r, c, *wxWHITE);
            }
            else if (state == 1)
            {
                GetActiveGrid()->SetCellBackgroundColour(r, c, priorc);
            }
            else if (state == 2)
            {
                GetActiveGrid()->SetCellBackgroundColour(r, c, nextc);
            }
            else if (state == 3)
            {
                GetActiveGrid()->SetCellBackgroundColour(r, c, priornextc);
            }
        }
    }
}
