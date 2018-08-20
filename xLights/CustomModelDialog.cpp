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
#include <wx/grid.h>
#include <wx/image.h>
#include <wx/intl.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/spinctrl.h>
#include <wx/stattext.h>
#include <wx/string.h>
//*)

#include "models/CustomModel.h"
#include "WiringDialog.h"

//(*IdInit(CustomModelDialog)
const long CustomModelDialog::ID_SPINCTRL1 = wxNewId();
const long CustomModelDialog::ID_SPINCTRL2 = wxNewId();
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
const long CustomModelDialog::ID_GRID_Custom = wxNewId();
//*)

const long CustomModelDialog::CUSTOMMODELDLGMNU_CUT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COPY = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_PASTE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPH = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_FLIPV = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_REVERSE = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_SHIFT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_INSERT = wxNewId();
const long CustomModelDialog::CUSTOMMODELDLGMNU_COMPRESS = wxNewId();

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
  renderer(nullptr),
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
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	WidthSpin = new wxSpinCtrl(this, ID_SPINCTRL1, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 500, 10, _T("ID_SPINCTRL1"));
	WidthSpin->SetValue(_T("10"));
	FlexGridSizer2->Add(WidthSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, wxID_ANY, _("Height"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	HeightSpin = new wxSpinCtrl(this, ID_SPINCTRL2, _T("10"), wxDefaultPosition, wxDefaultSize, 0, 1, 500, 10, _T("ID_SPINCTRL2"));
	HeightSpin->SetValue(_T("10"));
	FlexGridSizer2->Add(HeightSpin, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	FilePickerCtrl1 = new wxFilePickerCtrl(this, ID_FILEPICKERCTRL1, wxEmptyString, _("Select a file"), _T("*.*"), wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST|wxFLP_OPEN|wxFLP_USE_TEXTCTRL, wxDefaultValidator, _T("ID_FILEPICKERCTRL1"));
	FlexGridSizer1->Add(FilePickerCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(0,0,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
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
	GridCustom = new CopyPasteGrid(this, ID_GRID_Custom, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_GRID_Custom"));
	GridCustom->CreateGrid(1,1);
	GridCustom->EnableEditing(true);
	GridCustom->EnableGridLines(true);
	GridCustom->SetColLabelSize(20);
	GridCustom->SetRowLabelSize(30);
	GridCustom->SetDefaultColSize(30, true);
	GridCustom->SetDefaultCellFont( GridCustom->GetFont() );
	GridCustom->SetDefaultCellTextColour( GridCustom->GetForegroundColour() );
	Sizer1->Add(GridCustom, 0, wxEXPAND, 0);
	SetSizer(Sizer1);
	SetSizer(Sizer1);
	Layout();
	Center();

	Connect(ID_SPINCTRL1,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnWidthSpinChange);
	Connect(ID_SPINCTRL2,wxEVT_COMMAND_SPINCTRL_UPDATED,(wxObjectEventFunction)&CustomModelDialog::OnHeightSpinChange);
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
	Connect(ID_GRID_Custom,wxEVT_GRID_CELL_LEFT_CLICK,(wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellLeftClick);
	Connect(ID_GRID_Custom,wxEVT_GRID_CELL_RIGHT_CLICK,(wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellRightClick);
	//*)
    Connect(ID_GRID_Custom,wxEVT_GRID_CELL_CHANGED,(wxObjectEventFunction)&CustomModelDialog::OnGridCustomCellChange);

    GridCustom->Connect(wxEVT_TEXT_CUT, (wxObjectEventFunction)&CustomModelDialog::OnCut, 0, this);
    GridCustom->Connect(wxEVT_TEXT_COPY, (wxObjectEventFunction)&CustomModelDialog::OnCopy, 0, this);
    GridCustom->Connect(wxEVT_TEXT_PASTE, (wxObjectEventFunction)&CustomModelDialog::OnPaste, 0, this);

    name = "";

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
        ResizeCustomGrid();
        return;
    }

    if( background_image != "" && wxFile::Exists(background_image)) {
            bkg_image = new wxImage(background_image);
    }
    renderer = new wxModelGridCellRenderer(bkg_image, *GridCustom);
    GridCustom->SetDefaultRenderer(renderer);

    wxArrayString rows=wxSplit(data, ';');
    for(size_t row=0; row < rows.size(); row++)
    {
        if (row >= GridCustom->GetNumberRows()) GridCustom->AppendRows();
        wxArrayString cols = wxSplit(rows[row],',');
        for(size_t col=0; col < cols.size(); col++)
        {
            if (col >= GridCustom->GetNumberCols()) GridCustom->AppendCols();
            wxString value=cols[col];
            if (!value.IsEmpty() && value != "0")
            {
                GridCustom->SetCellValue(row,col,value);
            }
        }
    }

    WidthSpin->SetValue(GridCustom->GetNumberCols());
    HeightSpin->SetValue(GridCustom->GetNumberRows());

    wxFont font = GridCustom->GetDefaultCellFont();
    GridCustom->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    GridCustom->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < GridCustom->GetNumberCols(); ++c)
        GridCustom->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 4/5);
    for (int r = 0; r < GridCustom->GetNumberRows(); ++r)
        GridCustom->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 4/5);
    font = GridCustom->GetLabelFont();
    GridCustom->SetColLabelSize(int(1.5 * (float)font.GetPixelSize().y));
    //Sizer1->Fit(this);
    UpdateBackground();
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
    int deltaCols=numCols - GridCustom->GetNumberCols();
    int deltaRows=numRows - GridCustom->GetNumberRows();
    if (deltaCols > 0) GridCustom->AppendCols(deltaCols);
    if (deltaRows > 0) GridCustom->AppendRows(deltaRows);
    if (deltaCols < 0) GridCustom->DeleteCols(numCols,-deltaCols);
    if (deltaRows < 0) GridCustom->DeleteRows(numRows,-deltaRows);
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
    std::string customChannelData = "";
    int numCols=GridCustom->GetNumberCols();
    int numRows=GridCustom->GetNumberRows();
    for(int row=0; row < numRows; row++) {
        if (row > 0) customChannelData+=";";
        for(int col=0; col<numCols; col++) {
            if (col > 0) customChannelData+=",";
            wxString value = StripIllegalChars(GridCustom->GetCellValue(row,col));
            if (value == "0" || value.StartsWith("-")) value.clear();
            customChannelData += value;
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
    GridCustom->BeginBatch();
    wxFont font = GridCustom->GetLabelFont();
    font.MakeLarger();
    GridCustom->SetLabelFont(font);
    font = GridCustom->GetDefaultCellFont();
    font.MakeLarger();
    GridCustom->SetDefaultCellFont(font);
    for (int c = 0; c < GridCustom->GetNumberCols(); ++c)
        GridCustom->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 5/4);
    for (int r = 0; r < GridCustom->GetNumberRows(); ++r)
        GridCustom->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 5/4);
    GridCustom->EndBatch();
    UpdateBackground();
}

void CustomModelDialog::OnButton_CustomModelZoomOutClick(wxCommandEvent& event)
{
    GridCustom->BeginBatch();
    wxFont font = GridCustom->GetLabelFont();
    font.MakeSmaller();
    GridCustom->SetLabelFont(font);
    font = GridCustom->GetDefaultCellFont();
    font.MakeSmaller();
    GridCustom->SetDefaultCellFont(font);
    GridCustom->SetRowMinimalAcceptableHeight(5); //don't need to read text, just see the shape
    GridCustom->SetColMinimalAcceptableWidth(5); //don't need to read text, just see the shape
    for (int c = 0; c < GridCustom->GetNumberCols(); ++c)
        GridCustom->SetColSize(c, 2 * font.GetPixelSize().y); //GridCustom->GetColSize(c) * 4/5);
    for (int r = 0; r < GridCustom->GetNumberRows(); ++r)
        GridCustom->SetRowSize(r, int(1.5 * (float)font.GetPixelSize().y)); //GridCustom->GetRowSize(r) * 4/5);
    GridCustom->EndBatch();
    UpdateBackground();
}

void CustomModelDialog::OnButtonCustomModelHelpClick(wxCommandEvent& event)
{
    wxMessageBox("To create a custom model, set 'Display As' to 'Custom';\nthen set the model width and model height values.\n\nA custom model allows you to represent almost any display element \ncreated from RGB lights. If your element has 12 RGB nodes in it, then \nyou will need to place the numbers 1 through 12 in the grid. \nPlace the numbers so that they are in the shape of your display \nelement. If you enter a number and then need to erase it, enter a 0.\n\nNode 1 will be assigned:\n  StartChannel\n  StartChannel+1\n  StartChannel+2\nNode 2 will be assigned:\n  StartChannel+3\n  StartChannel+4\n  StartChannel+5\netc.\n\nFor example, to model a candy cane with 12 nodes, you could \nstart with a grid 4 columns wide and 10 rows high. You would place the \nnumbers 1-9 up the right-hand side, 10 and 11 would go in the middle \ncells in the top row, and then 12 would go in column A, row 2.\n\n     11 10\n12           9\n               8\n               7\n               6\n               5\n               4\n               3\n               2\n               1");
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

    for (int i = 0; i< GridCustom->GetNumberRows(); i++)        // step through all lines
    {
        bool something_in_this_line = false;             // nothing found yet
        for (int k = 0; k<GridCustom->GetNumberCols(); k++)     // step through all colums
        {
            if (GridCustom->IsInSelection(i,k))     // this field is selected!!!
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
                copy_data += StripIllegalChars(GridCustom->GetCellValue(i,k));    // finally we need the field value
                if (IsCut) GridCustom->SetCellValue(i,k,wxEmptyString);
            }
        }
    }

    if (copy_data.IsEmpty())
    {
        copy_data += StripIllegalChars(GridCustom->GetCellValue(GridCustom->GetGridCursorRow(), GridCustom->GetGridCursorCol()));    // finally we need the field value
        if (IsCut) GridCustom->SetCellValue(GridCustom->GetGridCursorRow(), GridCustom->GetGridCursorCol(), wxEmptyString);
    }

    if (wxTheClipboard->Open())
    {
        if (!wxTheClipboard->SetData(new wxTextDataObject(copy_data)))
        {
            wxMessageBox(_("Unable to copy data to clipboard."), _("Error"));
        }
        wxTheClipboard->Close();
    }
    else
    {
        wxMessageBox(_("Error opening clipboard."), _("Error"));
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
                    wxMessageBox(_("Unable to copy data from clipboard."), _("Error"));
                }
            }
            else
            {
                wxMessageBox(_("Non-Text data in clipboard."), _("Error"));
            }
            wxTheClipboard->Close();
        }
        else
        {
            wxMessageBox(_("Error opening clipboard."), _("Error"));
            return;
        }
    }

    int i = GridCustom->GetGridCursorRow();
    int k = GridCustom->GetGridCursorCol();
    int numrows = GridCustom->GetNumberRows();
    int numcols = GridCustom->GetNumberCols();
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
                    GridCustom->SetCellValue(i, k + fieldnum, fields[fieldnum].Trim(true).Trim(false)); //strip surrounding spaces -DJ
                }
                else
                {
                    errflag = true;
                    errdetails += wxString::Format("\n'%s' row %d/col %d of %d", fields[fieldnum].c_str(), i - GridCustom->GetGridCursorRow(), fieldnum, fields.Count()); //tell the user what was wrong; show relative row#, col# (more user friendly) -DJ
                }
            }
        }
        i++;
    } while (copy_data.IsEmpty() == false);

    if (errflag)
    {
        wxMessageBox(_("One or more of the values were not pasted because they did not contain a number") + errdetails, _("Paste Error")); //-DJ
    }
}

void CustomModelDialog::OnBitmapButtonCustomPasteClick(wxCommandEvent& event)
{
    Paste();
}

void CustomModelDialog::UpdateBackground()
{
    if (renderer != nullptr) renderer->UpdateSize(*GridCustom, bkgrd_active, lightness);
}

void CustomModelDialog::OnBitmapButtonCustomBkgrdClick(wxCommandEvent& event)
{
    bkgrd_active = !bkgrd_active;
    GridCustom->Refresh();
    UpdateBackground();
}

void CustomModelDialog::OnSliderCustomLightnessCmdSliderUpdated(wxScrollEvent& event)
{
    lightness = SliderCustomLightness->GetValue();
    UpdateBackground();
    Refresh();
}

wxModelGridCellRenderer::wxModelGridCellRenderer(wxImage* image_, wxGrid& grid)
: image(image_),
  draw_picture(true),
  lightness(0)
{
    UpdateSize(grid, true, lightness);
}

void wxModelGridCellRenderer::Draw(wxGrid &grid, wxGridCellAttr &attr, wxDC &dc, const wxRect &rect, int row, int col, bool isSelected)
{
    // erase only this cells background
    if( !isSelected ) {
        wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
    }

    // draw bitmap slice
    if( image != nullptr && draw_picture )
    {
        if( bmp.IsOk() )
        {
            if( (rect.x + rect.width ) <= bmp.GetWidth() &&
                (rect.y + rect.height) <= bmp.GetHeight() )
            {
                dc.DrawBitmap(bmp.GetSubBitmap(rect), rect.x, rect.y);
            }
        }
    }

    // draw selection color over image if needed
    if( isSelected ) {
        wxGridCellRenderer::Draw(grid, attr, dc, rect, row, col, isSelected);
    }

    // draw the text
    SetTextColoursAndFont(grid, attr, dc, isSelected);
    grid.DrawTextRectangle(dc, grid.GetCellValue(row, col), rect,  wxALIGN_CENTRE,  wxALIGN_CENTRE);
}

void wxModelGridCellRenderer::UpdateSize(wxGrid& grid, bool draw_picture_, int lightness_)
{
    draw_picture = draw_picture_;
    lightness = lightness_;
    DetermineGridSize(grid);
    CreateImage();
}

void wxModelGridCellRenderer::CreateImage()
{
    if( image != nullptr )
    {
        wxImage img(*image);

        for(int x = 0; x < img.GetWidth(); x++)
        {
            for(int y = 0; y < img.GetHeight(); y++)
            {
                unsigned char red = img.GetRed(x,y);
                unsigned char green = img.GetGreen(x,y);
                unsigned char blue = img.GetBlue(x,y);
                xlColor pixel(red, green, blue);
                HSLValue hsl(pixel);

               if (lightness > 0.0)
                    hsl.lightness = lightness/100.0 * (1.0 - hsl.lightness) + hsl.lightness;
                else if (lightness < 0.0)
                    hsl.lightness *= (1.0 + lightness/100.0);

                pixel.fromHSL(hsl);
                img.SetRGB(x,y,pixel.red,pixel.green,pixel.blue);
            }
        }
        img.Rescale(width, height);
        bmp = wxBitmap(img);
    }
}

void wxModelGridCellRenderer::SetImage(wxImage* image_)
{
    image = image_;
    CreateImage();
}

void wxModelGridCellRenderer::DetermineGridSize(wxGrid& grid)
{
    wxFont font = grid.GetDefaultCellFont();
    width = 0;
    height = 0;
    for (int c = 0; c < grid.GetNumberCols(); ++c) {
        width += 2 * font.GetPixelSize().y;
    }
    for (int r = 0; r < grid.GetNumberRows(); ++r) {
        height += int(1.5 * (float)font.GetPixelSize().y);
    }
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
        GridCustom->SetCellValue(event.GetRow(), event.GetCol(), wxString::Format("%d", next_channel) );
        if( autoincrement ) {
            next_channel++;
            SpinCtrlNextChannel->SetValue(next_channel);
        }
    }
    event.Skip();
}

void CustomModelDialog::OnCheckBox_RearViewClick(wxCommandEvent& event)
{
    // reverse the rows
    for(size_t r = 0; r < GridCustom->GetNumberRows(); r++)
    {
        std::list<wxString> vals;
        for (size_t c = 0; c < GridCustom->GetNumberCols(); ++c)
        {
            vals.push_front(GridCustom->GetCellValue(r, c));
        }
        size_t c = 0;
        for (auto it = vals.begin(); it != vals.end(); ++it)
        {
            GridCustom->SetCellValue(r, c++, *it);
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
    WiringDialog dlg(this, name);
    dlg.SetData(GridCustom, false);
    dlg.ShowModal();
}

void CustomModelDialog::FlipHorizontal()
{
    // reverse the rows
    for(size_t r = 0; r < GridCustom->GetNumberRows(); r++)
    {
        std::list<wxString> vals;
        for (size_t c = 0; c < GridCustom->GetNumberCols(); ++c)
        {
            vals.push_front(GridCustom->GetCellValue(r, c));
        }
        size_t c = 0;
        for (auto it = vals.begin(); it != vals.end(); ++it)
        {
            GridCustom->SetCellValue(r, c++, *it);
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::FlipVertical()
{
    // reverse the columns
    for(size_t c = 0; c < GridCustom->GetNumberCols(); c++)
    {
        std::list<wxString> vals;
        for (size_t r = 0; r < GridCustom->GetNumberRows(); ++r)
        {
            vals.push_front(GridCustom->GetCellValue(r, c));
        }
        size_t r = 0;
        for (auto it = vals.begin(); it != vals.end(); ++it)
        {
            GridCustom->SetCellValue(r++, c, *it);
        }
    }

    UpdateBackground();

    ValidateWindow();
}

void CustomModelDialog::Insert()
{
    long val;
    int x = GridCustom->GetGridCursorCol();
    int y = GridCustom->GetGridCursorRow();
    GridCustom->GetCellValue(x, y).ToCLong(&val);
    wxNumberEntryDialog dlg(this, wxString::Format("Number of nodes to create a gap for prior to node %ld.", val), "Nodes to create a gap for", "Insert", 1, 1, 50);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto toinsert = dlg.GetValue();

        //Find the max value returned
        auto max = 0;
        for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
        {
            for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
            {
                wxString s = GridCustom->GetCellValue(r, c);

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

        for (auto current = max; current >= val; current--)
        {
            AdjustNodeBy(current, toinsert);
        }
    }
}

bool CustomModelDialog::AdjustNodeBy(int node, int adjust)
{
    bool adjusted = false;
    for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
    {
        for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
        {
            wxString s = GridCustom->GetCellValue(r, c);

            if (s.IsEmpty() == false)
            {
                long val;
                s.ToCLong(&val);
                if (val == node)
                {
                    GridCustom->SetCellValue(r, c, wxString::Format("%d", val + adjust));
                    adjusted = true;
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
    for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
    {
        for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
        {
            wxString s = GridCustom->GetCellValue(r, c);

            if (s.IsEmpty() == false)
            {
                long val;
                if (s.ToCLong(&val) == true)
                {
                    if (val>max)max = val;
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
    for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
    {
        for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
        {
            wxString s = GridCustom->GetCellValue(r, c);

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

    max++;
    //Rewrite the grid values
    for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
    {
        std::list<wxString> vals;
        for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
        {
            wxString s = GridCustom->GetCellValue(r, c);

            if (s.IsEmpty() == false)
            {
                long val;

                if (s.ToCLong(&val) == true)
                {
                    long newVal = max - val;
                    s.Printf("%d", newVal);

                    GridCustom->SetCellValue(r, c, s);
                }
            }
        }
    }

    UpdateBackground();
    ValidateWindow();
}

void CustomModelDialog::Shift()
{
    auto min = 1;
    auto max = 1;

    //Find the max value returned
    for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
    {
        for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
        {
            wxString s = GridCustom->GetCellValue(r, c);

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

    wxNumberEntryDialog dlg(this, "Enter Increase/Decrease Value", "", "Increment/Decrement Value", 0, -(max - 1), max - 1);
    if (dlg.ShowModal() == wxID_OK)
    {
        auto scaleFactor = dlg.GetValue();
        if (scaleFactor != 0)
        {
            //Rewrite the grid values
            for (auto c = 0; c < GridCustom->GetNumberCols(); c++)
            {
                std::list<wxString> vals;
                for (auto r = 0; r < GridCustom->GetNumberRows(); ++r)
                {
                    wxString s = GridCustom->GetCellValue(r, c);

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

                            GridCustom->SetCellValue(r, c, s);
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
        Insert();
    }
    else if (id == CUSTOMMODELDLGMNU_COMPRESS)
    {
        Compress();
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
        renderer->SetImage(bkg_image);
        UpdateBackground();
        GridCustom->Refresh();
    }
}

void CustomModelDialog::OnGridCustomCellRightClick(wxGridEvent& event)
{
    GridCustom->SetGridCursor(event.GetRow(), event.GetCol());
    auto s = GridCustom->GetCellValue(event.GetRow(), event.GetCol());
    bool selectedCellWithValue = !s.IsEmpty() && s.IsNumber();

    wxMenu mnu;
    // Copy / Paste / Delete
    wxMenuItem* menu_cut = mnu.Append(CUSTOMMODELDLGMNU_CUT, "Cut");
    wxMenuItem* menu_copy = mnu.Append(CUSTOMMODELDLGMNU_COPY, "Copy");
    wxMenuItem* menu_paste = mnu.Append(CUSTOMMODELDLGMNU_PASTE, "Paste");
    if (GridCustom->GetSelectedCells().size() > 0)
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

    wxMenuItem* menu_fliphoriz = mnu.Append(CUSTOMMODELDLGMNU_FLIPH, "Horizontal Flip");
    wxMenuItem* menu_flipvert = mnu.Append(CUSTOMMODELDLGMNU_FLIPV, "Vertical Flip");
    wxMenuItem* menu_reverse = mnu.Append(CUSTOMMODELDLGMNU_REVERSE, "Reverse");
    wxMenuItem* menu_shift = mnu.Append(CUSTOMMODELDLGMNU_SHIFT, "Shift");
    wxMenuItem* menu_insert = mnu.Append(CUSTOMMODELDLGMNU_INSERT, "Insert Prior");
    wxMenuItem* menu_compress = mnu.Append(CUSTOMMODELDLGMNU_COMPRESS, "Compress");

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
