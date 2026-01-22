#include "AIImageDialog.h"

//(*InternalHeaders(AIImageDialog)
#include <wx/string.h>
//*)
#include <wx/mstream.h>

//#define BYPASS_GEMINI // set this to use a local image for testing local functions

//(*IdInit(AIImageDialog)
const wxWindowID AIImageDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID AIImageDialog::ID_PROMPT = wxNewId();
const wxWindowID AIImageDialog::ID_STATICBITMAP1 = wxNewId();
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
    wxFlexGridSizer* FlexGridSizer3;

    Create(parent, id, _T("Generate Image"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxFULL_REPAINT_ON_RESIZE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    MainSizer = new wxFlexGridSizer(0, 1, 0, 0);
    MainSizer->AddGrowableCol(0);
    ParametersBox = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Parameters"));
    ParametersSizer = new wxFlexGridSizer(0, 2, 0, 0);
    ParametersSizer->AddGrowableCol(1);
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
    ResultBitmap = new wxStaticBitmap(this, ID_STATICBITMAP1, wxNullBitmap, wxDefaultPosition, wxSize(400,400), 0, _T("ID_STATICBITMAP1"));
    ResultBitmap->SetMaxSize(wxSize(400,400));
    ResultBoxSizer->Add(ResultBitmap, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    ErrorText = new wxStaticText(this, ID_STATICTEXT2, wxEmptyString, wxDefaultPosition, wxSize(400,400), 0, _T("ID_STATICTEXT2"));
    ErrorText->SetMinSize(wxSize(400,400));
    ErrorText->SetMaxSize(wxSize(400,400));
    ResultBoxSizer->Add(ErrorText, 1, wxALL|wxEXPAND, 5);
    ResultsBox->Add(ResultBoxSizer, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 0);
    MainSizer->Add(ResultsBox, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 5, 0, 0);
    GenerateButton = new wxButton(this, ID_GENERATE, _T("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GENERATE"));
    GenerateButton->SetDefault();
    FlexGridSizer3->Add(GenerateButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    SaveButton = new wxButton(this, ID_BUTTON1, _T("Save"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
    FlexGridSizer3->Add(SaveButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer3->Add(-1,-1,1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 10);
    CloseButton = new wxButton(this, ID_OK, _T("Close"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_OK"));
    FlexGridSizer3->Add(CloseButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    MainSizer->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    SetSizer(MainSizer);
    MainSizer->SetSizeHints(this);
    Center();

    Connect(ID_GENERATE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnGenerateButtonClick);
    Connect(ID_BUTTON1, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnSaveButtonClicked);
    Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnCloseButtonClick);
    Connect(wxEVT_SIZE, (wxObjectEventFunction)&AIImageDialog::OnResize);
    //*)

    SaveButton->Disable();
    ResultBitmap->SetScaleMode(wxStaticBitmapBase::Scale_AspectFit);
    if (service) {
        ResultBitmap->Hide();
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
        ResultBitmap->Hide();
        ErrorText->SetLabel("Generating image....");
        ErrorText->Show();
        MainSizer->SetSizeHints(this);
        Center();

        generator->generateImage(PromptBox->GetValue().ToStdString(), [this](const wxBitmap &i, const std::string &err) {
            CallAfter([this, i, err] {
                if (err.empty()) {
                    ResultBitmap->SetBitmap(i);
                    ResultBitmap->Show();
                    ErrorText->Hide();
                    MainSizer->SetSizeHints(this);
                    Center();
                    SaveButton->Enable();
                } else {
                    ResultBitmap->Hide();
                    ErrorText->SetLabel(err);
                    ErrorText->Show();
                    MainSizer->SetSizeHints(this);
                    Center();
                    SaveButton->Disable();
                }
            });
        });
    }
}

void AIImageDialog::OnResize(wxSizeEvent& event) {
    OnSize(event);
    Refresh();
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

void AIImageDialog::OnSaveButtonClicked(wxCommandEvent& event)
{
    wxImage img = RemoveBlackBackground(ResultBitmap->GetBitmap().ConvertToImage());
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

