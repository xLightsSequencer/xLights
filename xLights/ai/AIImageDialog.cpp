#include "AIImageDialog.h"

//(*InternalHeaders(AIImageDialog)
#include <wx/string.h>
//*)
#include <wx/mstream.h>
#include <wx/numdlg.h>
#include <wx/dcbuffer.h>
#include "../ResizeImageDialog.h"

//#define BYPASS_GEMINI // set this to use a local image for testing local functions

// CropPanel implementation
wxBEGIN_EVENT_TABLE(CropPanel, wxPanel)
    EVT_PAINT(CropPanel::OnPaint)
    EVT_LEFT_DOWN(CropPanel::OnLeftDown)
    EVT_LEFT_UP(CropPanel::OnLeftUp)
    EVT_MOTION(CropPanel::OnMouseMove)
wxEND_EVENT_TABLE()

CropPanel::CropPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                     const wxSize& size,
                     long style,
                     const wxString& name)
    : wxPanel(parent, id, pos, size, style, name)
    , _isDragging(false)
    , _hasSelection(false)
    , _scaleFactor(1.0)
    , _offset(0, 0)
{
    SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void CropPanel::SetImage(const wxImage& image)
{
    _image = image;
    _hasSelection = false;
    _isDragging = false;
    
    if (image.IsOk()) {
        // Set minimum size to constrain the panel
        SetMinSize(wxSize(400, 400));
        SetMaxSize(wxSize(400, 400));
        
        // Scale image to fit within panel while maintaining aspect ratio
        wxSize targetSize(400, 400);
        wxImage scaledImage = image.Copy();
        
        double imageAspect = (double)image.GetWidth() / image.GetHeight();
        double targetAspect = (double)targetSize.GetWidth() / targetSize.GetHeight();
        
        int scaledWidth, scaledHeight;
        if (imageAspect > targetAspect) {
            // Image is wider - fit to width
            scaledWidth = targetSize.GetWidth();
            scaledHeight = (int)(targetSize.GetWidth() / imageAspect);
        } else {
            // Image is taller - fit to height
            scaledHeight = targetSize.GetHeight();
            scaledWidth = (int)(targetSize.GetHeight() * imageAspect);
        }
        
        scaledImage.Rescale(scaledWidth, scaledHeight, wxIMAGE_QUALITY_HIGH);
        _bitmap = wxBitmap(scaledImage);
        
        // Calculate scale factor and offset for coordinate transformation
        _scaleFactor = (double)scaledWidth / image.GetWidth();
        _offset.x = (targetSize.GetWidth() - scaledWidth) / 2;
        _offset.y = (targetSize.GetHeight() - scaledHeight) / 2;
    }
    
    Refresh();
}

void CropPanel::ClearSelection()
{
    _hasSelection = false;
    _isDragging = false;
    Refresh();
    
    if (OnSelectionChanged) {
        OnSelectionChanged(false);
    }
}

void CropPanel::OnPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC dc(this);
    dc.Clear();
    
    // Draw the image centered
    dc.DrawBitmap(_bitmap, _offset.x, _offset.y, false);
    
    // Draw selection rectangle if exists
    if (_hasSelection || _isDragging) {
        wxPoint topLeft(std::min(_startPoint.x, _endPoint.x), std::min(_startPoint.y, _endPoint.y));
        wxPoint bottomRight(std::max(_startPoint.x, _endPoint.x), std::max(_startPoint.y, _endPoint.y));
        
        // Draw semi-transparent overlay outside selection
        wxRegion outsideRegion(GetClientRect());
        wxRect selectionRect(topLeft, bottomRight);
        outsideRegion.Subtract(selectionRect);
        
        dc.SetDeviceClippingRegion(outsideRegion);
        dc.SetBrush(wxBrush(wxColour(0, 0, 0, 128)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRectangle(GetClientRect());
        dc.DestroyClippingRegion();
        
        // Draw selection border
        dc.SetPen(wxPen(*wxWHITE, 2, wxPENSTYLE_SOLID));
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.DrawRectangle(selectionRect);
        
        // Draw dashed inner line
        dc.SetPen(wxPen(*wxBLACK, 1, wxPENSTYLE_SHORT_DASH));
        dc.DrawRectangle(selectionRect);
    }
}

void CropPanel::OnLeftDown(wxMouseEvent& event)
{
    wxPoint pos = event.GetPosition();
    
    // Adjust for offset and clamp to bitmap bounds
    _startPoint.x = std::max(_offset.x, std::min(pos.x, _offset.x + _bitmap.GetWidth()));
    _startPoint.y = std::max(_offset.y, std::min(pos.y, _offset.y + _bitmap.GetHeight()));
    _endPoint = _startPoint;
    _isDragging = true;
    _hasSelection = false;
    CaptureMouse();
    Refresh();
}

void CropPanel::OnLeftUp(wxMouseEvent& event)
{
    if (_isDragging) {
        _isDragging = false;
        
        // Check if we have a valid selection (width and height > threshold)
        wxPoint topLeft(std::min(_startPoint.x, _endPoint.x), std::min(_startPoint.y, _endPoint.y));
        wxPoint bottomRight(std::max(_startPoint.x, _endPoint.x), std::max(_startPoint.y, _endPoint.y));
        wxRect rect(topLeft, bottomRight);
        
        // If the selection is too small (likely a click), clear any existing selection
        const int minSelectionSize = 5; // pixels
        if (rect.GetWidth() < minSelectionSize || rect.GetHeight() < minSelectionSize) {
            _hasSelection = false;
        } else {
            _hasSelection = true;
        }
        
        ReleaseMouse();
        Refresh();
        
        if (OnSelectionChanged) {
            OnSelectionChanged(_hasSelection);
        }
    }
}

void CropPanel::OnMouseMove(wxMouseEvent& event)
{
    if (_isDragging) {
        wxPoint pos = event.GetPosition();
        
        // Clamp to bitmap bounds
        _endPoint.x = std::max(_offset.x, std::min(pos.x, _offset.x + _bitmap.GetWidth()));
        _endPoint.y = std::max(_offset.y, std::min(pos.y, _offset.y + _bitmap.GetHeight()));
        
        Refresh();
    }
}

bool CropPanel::GetCropRect(wxRect& rect) const
{
    if (!_hasSelection) {
        return false;
    }
    
    wxPoint topLeft(std::min(_startPoint.x, _endPoint.x), std::min(_startPoint.y, _endPoint.y));
    wxPoint bottomRight(std::max(_startPoint.x, _endPoint.x), std::max(_startPoint.y, _endPoint.y));
    
    // Convert from screen coordinates to original image coordinates
    topLeft.x = (int)((topLeft.x - _offset.x) / _scaleFactor);
    topLeft.y = (int)((topLeft.y - _offset.y) / _scaleFactor);
    bottomRight.x = (int)((bottomRight.x - _offset.x) / _scaleFactor);
    bottomRight.y = (int)((bottomRight.y - _offset.y) / _scaleFactor);
    
    rect = wxRect(topLeft, bottomRight);
    
    // Ensure we have a valid rectangle
    return rect.GetWidth() > 0 && rect.GetHeight() > 0;
}

//(*IdInit(AIImageDialog)
const wxWindowID AIImageDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID AIImageDialog::ID_PROMPT = wxNewId();
const wxWindowID AIImageDialog::ID_PANEL2 = wxNewId();
const wxWindowID AIImageDialog::ID_STATICTEXT4 = wxNewId();
const wxWindowID AIImageDialog::ID_RESIZE = wxNewId();
const wxWindowID AIImageDialog::ID_CROP = wxNewId();
const wxWindowID AIImageDialog::ID_RESET = wxNewId();
const wxWindowID AIImageDialog::ID_PANEL1 = wxNewId();
const wxWindowID AIImageDialog::ID_STATICTEXT2 = wxNewId();
const wxWindowID AIImageDialog::ID_GENERATE = wxNewId();
const wxWindowID AIImageDialog::ID_BUTTON1 = wxNewId();
const wxWindowID AIImageDialog::ID_OK = wxNewId();
//*)

BEGIN_EVENT_TABLE(AIImageDialog,wxDialog)
    //(*EventTable(AIImageDialog)
    //*)
END_EVENT_TABLE()

#include "../utils/CurlManager.h"
#include "../xLightsMain.h"
#include "../xLightsApp.h"
#include "../sequencer/MainSequencer.h"
#include "../ai/aiBase.h"
#include "ServiceManager.h"
#include "TempFileManager.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>


AIImageDialog::AIImageDialog(wxWindow* parent, aiBase* service, wxWindowID id)
    : wxDialog()
{
    //(*Initialize(AIImageDialog)
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticText* SizeLabel1;

    Create(parent, id, _T("Generate Image"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxFULL_REPAINT_ON_RESIZE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    MainSizer = new wxFlexGridSizer(0, 1, 0, 0);
    MainSizer->AddGrowableCol(0);
    MainSizer->AddGrowableRow(1);
    ParametersBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Parameters"));
    ParametersSizer = new wxFlexGridSizer(0, 2, 0, 0);
    ParametersSizer->AddGrowableCol(1);
    ParametersSizer->AddGrowableRow(0);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _T("Prompt"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    ParametersSizer->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    PromptBox = new wxTextCtrl(this, ID_PROMPT, wxEmptyString, wxDefaultPosition, wxSize(400,50), wxTE_MULTILINE|wxVSCROLL, wxDefaultValidator, _T("ID_PROMPT"));
    PromptBox->SetFocus();
    PromptBox->SetHelpText(_T("Enter a prompt"));
    ParametersSizer->Add(PromptBox, 1, wxALL|wxEXPAND, 2);
    ParametersBox->Add(ParametersSizer, 1, wxALL|wxEXPAND, 0);
    MainSizer->Add(ParametersBox, 0, wxALL|wxEXPAND, 5);
    ResultsBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Results"));
    ResultBoxSizer = new wxFlexGridSizer(0, 3, 0, 0);
    ResultBoxSizer->AddGrowableCol(1);
    ResultBoxSizer->AddGrowableRow(0);
    ImagePanel = new wxPanel(this, ID_PANEL1, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL1"));
    ImagePanel->SetMinSize(wxSize(500,420));
    FlexGridSizer1 = new wxFlexGridSizer(1, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    cropPanel = new CropPanel(ImagePanel, ID_PANEL2, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("ID_PANEL2"));
    cropPanel->SetMinSize(wxSize(400,400));
    FlexGridSizer1->Add(cropPanel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    FlexGridSizer2 = new wxFlexGridSizer(0, 1, 0, 0);
    SizeLabel1 = new wxStaticText(ImagePanel, wxID_ANY, _T("Size:"), wxDefaultPosition, wxDefaultSize, 0, _T("wxID_ANY"));
    FlexGridSizer2->Add(SizeLabel1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    SizeLabel = new wxStaticText(ImagePanel, ID_STATICTEXT4, _T("0x0"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT4"));
    FlexGridSizer2->Add(SizeLabel, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
    ResizeButton = new wxButton(ImagePanel, ID_RESIZE, _T("Resize"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RESIZE"));
    FlexGridSizer2->Add(ResizeButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CropButton = new wxButton(ImagePanel, ID_CROP, _T("Crop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CROP"));
    FlexGridSizer2->Add(CropButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ResetButton = new wxButton(ImagePanel, ID_RESET, _T("Reset"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_RESET"));
    FlexGridSizer2->Add(ResetButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    ImagePanel->SetSizer(FlexGridSizer1);
    ResultBoxSizer->Add(ImagePanel, 1, wxALL|wxEXPAND, 5);
    ErrorText = new wxTextCtrl(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE|wxTE_READONLY|wxTE_WORDWRAP|wxVSCROLL, wxDefaultValidator, _T("ID_STATICTEXT2"));
    ErrorText->SetMinSize(wxSize(500,400));
    ResultBoxSizer->Add(ErrorText, 1, wxALL|wxEXPAND, 5);
    ResultsBox->Add(ResultBoxSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    MainSizer->Add(ResultsBox, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(1, 4, 0, 0);
    FlexGridSizer3->AddGrowableCol(1);
    GenerateButton = new wxButton(this, ID_GENERATE, _T("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GENERATE"));
    GenerateButton->SetDefault();
    FlexGridSizer3->Add(GenerateButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxEXPAND, 10);
    SaveButton = new wxButton(this, ID_BUTTON1, _T("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(SaveButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CloseButton = new wxButton(this, ID_OK, _T("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_OK"));
    FlexGridSizer3->Add(CloseButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MainSizer->Add(FlexGridSizer3, 1, wxALL|wxEXPAND, 2);
    SetSizer(MainSizer);
    MainSizer->SetSizeHints(this);
    Center();

    Connect(ID_RESIZE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnResizeButtonClick);
    Connect(ID_CROP, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnCropButtonClick);
    Connect(ID_RESET, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnResetButtonClick);
    Connect(ID_GENERATE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnGenerateButtonClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnSaveButtonClicked);
    Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnCloseButtonClick);
    Connect(wxEVT_SIZE, (wxObjectEventFunction)&AIImageDialog::OnResize);
    //*)

    SaveButton->Disable();
    ResizeButton->Disable();
    CropButton->Disable();
    ResetButton->Disable();
        
    // Set up selection changed callback
    cropPanel->OnSelectionChanged = [this](bool hasSelection) {
        CropButton->Enable(hasSelection);
    };
    
    if (service) {
        ImagePanel->Hide();
        generator = service->createAIImageGenerator();
        generator->addControls(this, ParametersSizer);

        MainSizer->SetSizeHints(this);
        Center();
    }
}

AIImageDialog::~AIImageDialog()
{
    //(*Destroy(AIImageDialog)
    //*)
    if (generator) {
        delete generator;
    }
}

void AIImageDialog::OnGenerateButtonClick(wxCommandEvent& event)
{
    if (generator) {
        ImagePanel->Hide();
        ErrorText->SetValue("Generating image....");
        ErrorText->Show();
        MainSizer->SetSizeHints(this);
        Center();

        generator->generateImage(PromptBox->GetValue().ToStdString(), [this](const wxBitmap &i, const std::string &err) {
            CallAfter([this, i, err] {
                if (err.empty()) {
                    _originalImage = i.ConvertToImage();
                    _currentImage = _originalImage.Copy();
                    cropPanel->SetImage(_currentImage);
                    cropPanel->ClearSelection();
                    cropPanel->Show();
                    UpdateSizeLabel();
                    ImagePanel->Show();
                    ErrorText->Hide();
                    MainSizer->SetSizeHints(this);
                    Center();
                    SaveButton->Enable();
                    ResizeButton->Enable();
                    CropButton->Disable();  // Will be enabled when user makes a selection
                    ResetButton->Enable();
                } else {
                    ImagePanel->Hide();
                    ErrorText->SetValue(err);
                    ErrorText->Show();
                    MainSizer->SetSizeHints(this);
                    Center();
                    SaveButton->Disable();
                    ResizeButton->Disable();
                    CropButton->Disable();
                    ResetButton->Disable();
                }
            });
        });
    }
}

void AIImageDialog::OnResize(wxSizeEvent& event) {
    OnSize(event);
    Refresh();
}

void AIImageDialog::UpdateSizeLabel() {
    if (_currentImage.IsOk()) {
        SizeLabel->SetLabel(wxString::Format("%dx%d", _currentImage.GetWidth(), _currentImage.GetHeight()));
    } else {
        SizeLabel->SetLabel("0x0");
    }
}


wxImage RemoveBlackBackground(const wxImage& src) {
    if (!src.IsOk()) {
        return src;  // Return invalid image as-is
    }
    wxImage result = src.Copy();

    // Ensure we have an alpha channel
    if (!result.HasAlpha()) {
        result.InitAlpha();
        // Initialize alpha to fully opaque by default
        memset(result.GetAlpha(), 255, result.GetWidth() * result.GetHeight());
    }

    int width = result.GetWidth();
    int height = result.GetHeight();

    std::vector<bool> visited(width * height, false);
    std::queue<std::pair<int, int>> q;

    // 4-directional neighbors
    const int dx[4] = {0, 0, -1, 1};
    const int dy[4] = {-1, 1, 0, 0};

    auto isBlack = [&](int x, int y) -> bool {
        unsigned char r = result.GetRed(x, y);
        unsigned char g = result.GetGreen(x, y);
        unsigned char b = result.GetBlue(x, y);

        const unsigned char tolerance = 25;

        return r <= tolerance && g <= tolerance && b <= tolerance;
    };

    // Seed flood fill from all black pixels on the image borders
    for (int x = 0; x < width; ++x) {
        if (isBlack(x, 0)) {               // Top row
            q.push({x, 0});
            visited[x] = true;
            result.SetAlpha(x, 0, 0);
        }
        if (isBlack(x, height - 1)) {      // Bottom row
            q.push({x, height - 1});
            visited[x + (height - 1) * width] = true;
            result.SetAlpha(x, height - 1, 0);
        }
    }
    for (int y = 0; y < height; ++y) {
        if (isBlack(0, y)) {               // Left column
            q.push({0, y});
            visited[y * width] = true;
            result.SetAlpha(0, y, 0);
        }
        if (isBlack(width - 1, y)) {       // Right column
            q.push({width - 1, y});
            visited[(width - 1) + y * width] = true;
            result.SetAlpha(width - 1, y, 0);
        }
    }
    for (int y = 0; y < height; ++y) {
        if (isBlack(0, y)) {               // Left column
            q.push({0, y});
            visited[y * width] = true;
            result.SetAlpha(0, y, 0);
        }
        if (isBlack(width - 1, y)) {       // Right column
            q.push({width - 1, y});
            visited[(width - 1) + y * width] = true;
            result.SetAlpha(width - 1, y, 0);
        }
    }

    while (!q.empty()) {
        auto [cx, cy] = q.front();
        q.pop();

        for (int d = 0; d < 4; ++d) {
            int nx = cx + dx[d];
            int ny = cy + dy[d];

            if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                int idx = nx + ny * width;
                if (!visited[idx] && isBlack(nx, ny)) {
                    visited[idx] = true;
                    result.SetAlpha(nx, ny, 0);
                    q.push({nx, ny});
                }
            }
        }
    }
    return result;
}

void AIImageDialog::OnResizeButtonClick(wxCommandEvent& event)
{
    if (!_currentImage.IsOk()) {
        return;
    }

    int currentWidth = _currentImage.GetWidth();
    int currentHeight = _currentImage.GetHeight();

    ResizeImageDialog dlg(this);
    dlg.WidthSpinCtrl->SetRange(1, 10000);
    dlg.WidthSpinCtrl->SetValue(currentWidth);
    dlg.HeightSpinCtrl->SetRange(1, 10000);
    dlg.HeightSpinCtrl->SetValue(currentHeight);
    
    if (dlg.ShowModal() != wxID_OK) {
        return; // User cancelled
    }

    int newWidth = dlg.WidthSpinCtrl->GetValue();
    int newHeight = dlg.HeightSpinCtrl->GetValue();
    int scalingType = dlg.ResizeChoice->GetSelection();

    // Map scaling type to wxImageResizeQuality
    wxImageResizeQuality quality = wxIMAGE_QUALITY_HIGH;
    switch (scalingType) {
        case 0: // Normal
            quality = wxIMAGE_QUALITY_NORMAL;
            break;
        case 1: // Bilinear
            quality = wxIMAGE_QUALITY_BILINEAR;
            break;
        case 2: // Bicubic
            quality = wxIMAGE_QUALITY_BICUBIC;
            break;
        case 3: // Box Average
            quality = wxIMAGE_QUALITY_BOX_AVERAGE;
            break;
        case 4: // Crop/Border
            quality = wxIMAGE_QUALITY_HIGH; // Use high quality for crop/border
            break;
        default:
            quality = wxIMAGE_QUALITY_HIGH;
            break;
    }

    // Resize the image
    _currentImage = _currentImage.Scale(newWidth, newHeight, quality);
    
    // Update the display
    cropPanel->SetImage(_currentImage);
    cropPanel->ClearSelection();
    UpdateSizeLabel();
    MainSizer->SetSizeHints(this);
    Center();
}

void AIImageDialog::OnCropButtonClick(wxCommandEvent& event)
{
    if (!_currentImage.IsOk() || !cropPanel->HasSelection()) {
        return;
    }

    wxRect cropRect;
    if (cropPanel->GetCropRect(cropRect)) {
        // Crop the image
        _currentImage = _currentImage.GetSubImage(cropRect);
        
        // Update the display
        cropPanel->SetImage(_currentImage);
        cropPanel->ClearSelection();
        UpdateSizeLabel();
        MainSizer->SetSizeHints(this);
        Center();
    }
}

void AIImageDialog::OnResetButtonClick(wxCommandEvent& event)
{
    if (_originalImage.IsOk()) {
        _currentImage = _originalImage.Copy();
        cropPanel->SetImage(_currentImage);
        cropPanel->ClearSelection();
        UpdateSizeLabel();
        MainSizer->SetSizeHints(this);
        Center();
    }
}

void AIImageDialog::OnSaveButtonClicked(wxCommandEvent& event)
{
    wxImage img = RemoveBlackBackground(_currentImage);
    if (img.IsOk()) {
        wxFileDialog dlg(this, "Save Image", wxEmptyString, "Image.png", "*.png", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
        if (dlg.ShowModal() == wxID_OK) {
            img.SaveFile(dlg.GetPath());
        }
    }
}

void AIImageDialog::OnCloseButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

