#include "AIImageDialog.h"

//(*InternalHeaders(AIImageDialog)
#include <wx/string.h>
//*)
#include <wx/mstream.h>

//#define BYPASS_GEMINI // set this to use a local image for testing local functions

//(*IdInit(AIImageDialog)
const wxWindowID AIImageDialog::ID_STATICTEXT1 = wxNewId();
const wxWindowID AIImageDialog::ID_PROMPT = wxNewId();
const wxWindowID AIImageDialog::ID_STATICTEXT3 = wxNewId();
const wxWindowID AIImageDialog::ID_DIRBUTTON = wxNewId();
const wxWindowID AIImageDialog::ID_DIRECTORY = wxNewId();
const wxWindowID AIImageDialog::ID_STATICTEXT6 = wxNewId();
const wxWindowID AIImageDialog::ID_FNAMEBUTTON = wxNewId();
const wxWindowID AIImageDialog::ID_FILENAME = wxNewId();
const wxWindowID AIImageDialog::ID_STATICTEXT2 = wxNewId();
const wxWindowID AIImageDialog::ID_AIPROMPT = wxNewId();
const wxWindowID AIImageDialog::ID_HTMLWINDOW1 = wxNewId();
const wxWindowID AIImageDialog::ID_GENERATE = wxNewId();
const wxWindowID AIImageDialog::ID_OK = wxNewId();
const wxWindowID AIImageDialog::ID_CANCEL = wxNewId();
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
#include "../ai/gemini.h"
#include "ServiceManager.h"
#include "TempFileManager.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>


AIImageDialog::AIImageDialog(wxWindow* parent, ServiceManager* sm, const wxString& defaultPrompt,  const wxString& AIPrompt, wxWindowID id)
    : wxDialog()
    , _serviceManager(sm)
{
    //(*Initialize(AIImageDialog)
    wxBoxSizer* BoxSizer1;
    wxBoxSizer* BoxSizer2;
    wxFlexGridSizer* FlexGridSizer1;
    wxFlexGridSizer* FlexGridSizer2;
    wxFlexGridSizer* FlexGridSizer3;
    wxStaticBoxSizer* StaticBoxSizer1;
    wxStaticBoxSizer* StaticBoxSizer2;

    Create(parent, id, _T("Generate Image"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE|wxRESIZE_BORDER|wxCLOSE_BOX|wxFULL_REPAINT_ON_RESIZE, _T("id"));
    SetClientSize(wxDefaultSize);
    Move(wxDefaultPosition);
    FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    StaticBoxSizer1 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Parameters"));
    FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer2->AddGrowableCol(1);
    StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _T("Prompt"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
    FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FreeFormText = new wxTextCtrl(this, ID_PROMPT, wxEmptyString, wxDefaultPosition, wxSize(400,50), wxTE_MULTILINE|wxVSCROLL, wxDefaultValidator, _T("ID_PROMPT"));
    FreeFormText->SetFocus();
    FreeFormText->SetHelpText(_T("Enter a prompt"));
    FlexGridSizer2->Add(FreeFormText, 1, wxALL|wxEXPAND, 2);
    StaticText3 = new wxStaticText(this, ID_STATICTEXT3, _T("Directory"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT3"));
    FlexGridSizer2->Add(StaticText3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer1 = new wxBoxSizer(wxHORIZONTAL);
    DirButton = new wxButton(this, ID_DIRBUTTON, _T("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_DIRBUTTON"));
    BoxSizer1->Add(DirButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    DirectoryCtrl = new wxTextCtrl(this, ID_DIRECTORY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_DIRECTORY"));
    BoxSizer1->Add(DirectoryCtrl, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(BoxSizer1, 0, wxEXPAND, 5);
    StaticText6 = new wxStaticText(this, ID_STATICTEXT6, _T("Filename"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT6"));
    FlexGridSizer2->Add(StaticText6, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    BoxSizer2 = new wxBoxSizer(wxHORIZONTAL);
    FilenameButton = new wxButton(this, ID_FNAMEBUTTON, _T("Select"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_FNAMEBUTTON"));
    BoxSizer2->Add(FilenameButton, 0, wxALL|wxALIGN_CENTER_VERTICAL, 2);
    FilenameCtrl = new wxTextCtrl(this, ID_FILENAME, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_FILENAME"));
    BoxSizer2->Add(FilenameCtrl, 1, wxALL|wxEXPAND, 2);
    FlexGridSizer2->Add(BoxSizer2, 0, wxEXPAND, 5);
    StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _T("AI Prompt"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
    FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    TextCtrl1 = new wxTextCtrl(this, ID_AIPROMPT, wxEmptyString, wxDefaultPosition, wxSize(400,100), wxTE_MULTILINE|wxTE_READONLY|wxVSCROLL, wxDefaultValidator, _T("ID_AIPROMPT"));
    TextCtrl1->SetHelpText(_T("Default system prompt"));
    FlexGridSizer2->Add(TextCtrl1, 1, wxALL|wxEXPAND, 2);
    StaticBoxSizer1->Add(FlexGridSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(StaticBoxSizer1, 0, wxALL|wxEXPAND, 5);
    StaticBoxSizer2 = new wxStaticBoxSizer(wxHORIZONTAL, this, _T("Results"));
    ResultHTMLCtrl = new wxHtmlWindow(this, ID_HTMLWINDOW1, wxDefaultPosition, wxSize(-1,260), wxHW_SCROLLBAR_AUTO, _T("ID_HTMLWINDOW1"));
    StaticBoxSizer2->Add(ResultHTMLCtrl, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer1->Add(StaticBoxSizer2, 1, wxALL|wxEXPAND, 5);
    FlexGridSizer3 = new wxFlexGridSizer(0, 3, 0, 0);
    GenerateButton = new wxButton(this, ID_GENERATE, _T("Generate"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_GENERATE"));
    GenerateButton->SetDefault();
    FlexGridSizer3->Add(GenerateButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    OkButton = new wxButton(this, ID_OK, _T("OK"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_OK"));
    FlexGridSizer3->Add(OkButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    CancelButton = new wxButton(this, ID_CANCEL, _T("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_CANCEL"));
    FlexGridSizer3->Add(CancelButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
    FlexGridSizer1->Add(FlexGridSizer3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 2);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->SetSizeHints(this);
    Center();

    Connect(ID_DIRBUTTON, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnDirectoryButtonClick);
    Connect(ID_FNAMEBUTTON, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnFilenameButtonClick);
    Connect(ID_GENERATE, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnGenerateButtonClick);
    Connect(ID_OK, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnOkButtonClick);
    Connect(ID_CANCEL, wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&AIImageDialog::OnCancelButtonClick);
    Connect(wxEVT_SIZE, (wxObjectEventFunction)&AIImageDialog::OnResize);
    //*)

    FreeFormText->SetValue(defaultPrompt);
    FreeFormText->SetFocus();
    FreeFormText->SetSelection(-1, -1);

    wxString showDir = xLightsFrame::CurrentDir;
    wxConfigBase* config = wxConfigBase::Get();
    wxString imageDir = "";
    if (config != nullptr) {
        config->Read("xLightsAIImageDir", &imageDir, showDir);
    }
    if (imageDir.empty()) {
        imageDir = showDir;
    }

    DirectoryCtrl->SetValue(imageDir);
    TextCtrl1->SetValue(AIPrompt);

    wxDateTime now = wxDateTime::Now();
    wxString filename = wxString::Format("image-%04d-%02d-%02d-%02d%02d.png",
        now.GetYear(), now.GetMonth() + 1, now.GetDay(),
        now.GetHour(), now.GetMinute());

    FilenameCtrl->SetValue(filename);

    // MANDATORY OUTPUT REQUIREMENTS: Background: Pure Black background (#000000) with no watermarks or border.
    // Design: 2D Vector Style, Colors aligned to the character design.
    // The design features bold, clean outlines, simple cel-shading, and a limited vibrant color palette with clean edges and no gradients.
    // Character features should be simplified with large blocky shapes, a large head, and a small body (Chibi proportions).
    // Ensure the lines are perfectly horizontal and vertical with no diagonal smoothing.  A minimalist, flat 2D pixel art illustration using a 32-bit kawaii style.
}

AIImageDialog::~AIImageDialog()
{
    //(*Destroy(AIImageDialog)
    //*)
}

struct Point {
    int y, x;
    Point(int y, int x) : y(y), x(x) {}
};

wxImage RemoveBlackBackground(const wxImage& src)
{
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

void AIImageDialog::OnGenerateButtonClick(wxCommandEvent& event)
{
    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    wxString prompt = FreeFormText->GetValue().Trim();
    if (prompt.IsEmpty()) {
        ResultHTMLCtrl->SetPage("<html><body><b>Error:</b> Prompt cannot be empty.</body></html>");
        return;
    }
    wxString AIPrompt = TextCtrl1->GetValue().Trim();
    if (AIPrompt.IsEmpty()) {
        ResultHTMLCtrl->SetPage("<html><body><b>Error:</b> System Prompt cannot be empty.</body></html>");
        return;
    }
    gemini* geminiService = dynamic_cast<gemini*>(_serviceManager->getService("Gemini"));
    if (!geminiService || !geminiService->IsAvailable()) {
        ResultHTMLCtrl->SetPage("<html><body><b>Error:</b> Gemini service is not available or not configured.</body></html>");
        return;
    }
    std::string actualKey = _serviceManager->getSecretServiceToken("GeminiApiKey");

    if (actualKey.empty()) {
        ResultHTMLCtrl->SetPage("<html><body><b>Error:</b> Gemini API key is not set. Go to Preferences Services to configure it.</body></html>");
        return;
    }

#ifdef BYPASS_GEMINI
    // === TEST MODE: Load local image file instead of calling Gemini ===
    std::vector<uint8_t> imageData;
    std::string localFile = "F:\\ShowFolderQA\\image-2026-01-04-1848.png";
    wxFile localImageFile(localFile, wxFile::read);
    if (localImageFile.IsOpened()) {
        size_t len = localImageFile.Length();
        imageData.resize(len);
        localImageFile.Read(imageData.data(), len);
        localImageFile.Close();
        logger_base.debug("Loaded local test image: %s (%zu bytes)", localFile.c_str(), len);
    } else {
        logger_base.error("Failed to open local test image: %s", localFile.c_str());
    }

    std::string html = "<html><body>";
    if (imageData.empty()) {
        html += "<b>Failed to load local test image.</b>";
    } else {
        html += "<b>Test Mode:</b> Loaded local image successfully.<br>";

        wxMemoryInputStream stream(imageData.data(), imageData.size());
        wxImage originalImage(stream, wxBITMAP_TYPE_PNG);

        if (!originalImage.IsOk()) {
            html += "<b>Invalid PNG data received.</b>";
        } else {
            wxImage processedImage = ::RemoveBlackBackground(originalImage);

            wxString filename = FilenameCtrl->GetValue();
            if(filename.empty()) {
                ResultHTMLCtrl->SetPage("<html><body><b>Error:</b> Filename cannot be empty.</body></html>");
                return;
            }

            wxString imageDir = DirectoryCtrl->GetValue();
            if(imageDir.empty()) {
                imageDir = xLightsFrame::CurrentDir;
            } else {
                wxConfigBase* config = wxConfigBase::Get();
                if (config != nullptr) {
                    config->Write("xLightsAIImageDir", imageDir);
                }
            }

            wxFileName fullPath(imageDir, filename);
            generatedImagePath = fullPath.GetFullPath();

            if (processedImage.SaveFile(generatedImagePath, wxBITMAP_TYPE_PNG)) {
                html += "<b>Success!</b> Image saved!<br><br>";

                wxImage thumbnail = processedImage.Copy();
                thumbnail.Rescale(128, 128);

                wxString thumbFilename = filename.BeforeLast('.') + "_thumb.png";

                wxFileName thumbPath(imageDir, thumbFilename);
                wxString thumbFullPath = thumbPath.GetFullPath();
                TempFileManager::GetTempFileManager().AddTempFile(thumbFullPath);

                if (thumbnail.SaveFile(thumbFullPath, wxBITMAP_TYPE_PNG)) {
                    html += "<img src='file:///" + thumbFullPath.ToStdString() +
                        "' style='border:3px solid #0f0; border-radius:5px;'><br>";
                    html += "<small><i>128x128 thumbnail shown above. Full PNG saved.</i></small><br><br>";
                }

                html += "<small><b>File:</b> " + filename.ToStdString() + "<br>";
                html += "<b>Path:</b> " + generatedImagePath.ToStdString() + "</small>";
            } else {
                html += "<b>Failed to save PNG file.</b>";
                generatedImagePath = "";
            }
        }
    }
    html += "</body></html>";
    ResultHTMLCtrl->SetPage(html);

#else
    // === NORMAL MODE: Call Gemini API ===

    prompt = AIPrompt + " " + prompt;

    ResultHTMLCtrl->SetPage("<html><body><i>Generating image with Gemini...<br>This may take 10-30 seconds.</i></body></html>");
    ResultHTMLCtrl->Refresh();
    wxYield();

    std::string model = "gemini-2.5-flash-image";
    model = _serviceManager->getServiceSetting("GeminiModel", model);
    std::string url = "https://generativelanguage.googleapis.com/v1beta/models/" + model + ":generateContent?key=" + actualKey;

    wxString jsonPayload = wxString::Format(
        "{"
        "  \"contents\": [{"
        "    \"parts\": [{"
        "      \"text\": \"%s\""
        "    }]"
        "  }],"
        "  \"generationConfig\": {"
        "    \"responseModalities\": [\"IMAGE\"]"
        "  }"
        "}", prompt);

    std::string postData = jsonPayload.ToStdString();

    CurlManager::INSTANCE.addPost(
        url,
        postData,
        "application/json",
        [this](int rc, const std::string& resp)
        {
            static log4cpp::Category& logger_base = log4cpp::Category::getInstance("log_base");
            std::string html = "<html><body>";
            std::vector<unsigned char> imageData;

            if (rc != 200) {
                html += wxString::Format("<b>HTTP Error %d</b><br><pre>%s</pre>", rc, resp.c_str());
            } else {
                std::string base64Data;
                try {
                    auto jsonResp = nlohmann::json::parse(resp);
                    logger_base.debug("Gemini Response: %s", resp.c_str());

                    if (jsonResp.contains("candidates") && jsonResp["candidates"].is_array() && !jsonResp["candidates"].empty()) {
                        auto parts = jsonResp["candidates"][0]["content"]["parts"];
                        for (const auto& part : parts) {
                            if (part.contains("inlineData") && part["inlineData"].contains("data")) {
                                base64Data = part["inlineData"]["data"].get<std::string>();
                                break;
                            }
                        }
                    }
                } catch (const std::exception& e) {
                    html += wxString::Format("<b>JSON Parse Error:</b> %s<br><pre>%s</pre>", e.what(), resp.c_str());
                }

                if (base64Data.empty()) {
                    html += "<b>No image data found in response.</b>";
                } else {
                    base64Data.erase(std::remove_if(base64Data.begin(), base64Data.end(), ::isspace), base64Data.end());

                    wxMemoryBuffer decoded = wxBase64Decode(base64Data.c_str(), base64Data.length());
                    if (decoded.GetDataLen() > 0) {
                        imageData.resize(decoded.GetDataLen());
                        memcpy(imageData.data(), decoded.GetData(), decoded.GetDataLen());
                    }
                }
            }

            if (imageData.empty()) {
                html += "<b>Failed to obtain image data.</b>";
            } else {
                wxMemoryInputStream stream(imageData.data(), imageData.size());
                wxImage originalImage(stream, wxBITMAP_TYPE_PNG);

                if (!originalImage.IsOk()) {
                    html += "<b>Invalid PNG data received.</b>";
                } else {
                    wxImage processedImage = ::RemoveBlackBackground(originalImage);

                    wxString filename = FilenameCtrl->GetValue();
                    if(filename.empty()) {
                        ResultHTMLCtrl->SetPage("<html><body><b>Error:</b> Filename cannot be empty.</body></html>");
                        return;
                    }

                    wxString imageDir = DirectoryCtrl->GetValue();
                    if(imageDir.empty()) {
                        imageDir = xLightsFrame::CurrentDir;
                    } else {
                        wxConfigBase* config = wxConfigBase::Get();
                        if (config != nullptr) {
                            config->Write("xLightsAIImageDir", imageDir);
                        }
                    }

                    wxFileName fullPath(imageDir, filename);
                    generatedImagePath = fullPath.GetFullPath();

                    if (processedImage.SaveFile(generatedImagePath, wxBITMAP_TYPE_PNG)) {
                        html += "<b>Success!</b> Image saved!<br><br>";

                        wxImage thumbnail = processedImage.Copy();
                        thumbnail.Rescale(128, 128);

                        wxString thumbFilename = filename.BeforeLast('.') + "_thumb.png";
                        wxFileName thumbPath(imageDir, thumbFilename);
                        wxString thumbFullPath = thumbPath.GetFullPath();
                        TempFileManager::GetTempFileManager().AddTempFile(thumbFullPath);

                        if (thumbnail.SaveFile(thumbFullPath, wxBITMAP_TYPE_PNG)) {
                            html += "<img src='file:///" + thumbFullPath.ToStdString() +
                                "' style='border:3px solid #0f0; border-radius:5px;'><br>";
                            html += "<small><i>128x128 thumbnail shown above. Full PNG saved.</i></small><br><br>";
                        }

                        html += "<small><b>File:</b> " + filename.ToStdString() + "<br>";
                        html += "<b>Path:</b> " + generatedImagePath.ToStdString() + "</small>";
                    } else {
                        html += "<b>Failed to save PNG file.</b>";
                        generatedImagePath = "";
                    }
                }
            }

            html += "</body></html>";
            wxTheApp->CallAfter([this, html]() {
                ResultHTMLCtrl->SetPage(html);
                });
        }
    );
#endif
}

void AIImageDialog::OnOkButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_OK);
}

void AIImageDialog::OnCancelButtonClick(wxCommandEvent& event)
{
    EndModal(wxID_CANCEL);
}


void AIImageDialog::OnResize(wxSizeEvent& event) {
    OnSize(event);
    Refresh();
}

void AIImageDialog::OnDirectoryButtonClick(wxCommandEvent& event) {
    wxDirDialog dirDialog(this, _("Select Image Directory"), generatedImagePath,
                          wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dirDialog.ShowModal() == wxID_OK) {
        wxString selectedPath = dirDialog.GetPath();
        wxLogMessage(wxString::Format("Selected directory: %s", selectedPath));
        DirectoryCtrl->SetValue(selectedPath);
    }
}

void AIImageDialog::OnFilenameButtonClick(wxCommandEvent& event) {
    wxLogMessage("fname");
    wxString directory = DirectoryCtrl->GetValue();
    wxString defaultFilename = FilenameCtrl->GetValue();

    if (defaultFilename.IsEmpty()) {
        wxDateTime now = wxDateTime::Now();
        wxString filename = wxString::Format("image-%04d-%02d-%02d-%02d%02d.png",
                                             now.GetYear(), now.GetMonth() + 1, now.GetDay(),
                                             now.GetHour(), now.GetMinute());
    }

    wxString wildcard = _("PNG files (*.png)|*.png|All files (*.*)|*.*");

    wxFileDialog saveDialog(this,
                            _("Enter filename to save"),
                            directory,
                            defaultFilename,
                            wildcard,
                            wxFD_SAVE);

    if (saveDialog.ShowModal() == wxID_OK) {
        wxString fullPath = saveDialog.GetPath();

        if (wxFileExists(fullPath)) {
            wxMessageDialog confirm(this,
                                    wxString::Format(_("%s already exists.\nDo you want to overwrite it?"), fullPath),
                                    _("Confirm Overwrite"),
                                    wxYES_NO | wxICON_QUESTION);

            if (confirm.ShowModal() != wxID_YES)
                return; // user cancelled
        }

        FilenameCtrl->SetValue(saveDialog.GetFilename()); // or fullPath as needed
    }
}
