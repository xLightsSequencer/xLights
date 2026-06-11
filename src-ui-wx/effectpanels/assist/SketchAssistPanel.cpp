#include "SketchAssistPanel.h"
#include "SketchCanvasPanel.h"

#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>

#include "UtilFunctions.h"
#include "shared/utils/wxUtilities.h"
#include <xLightsMain.h>
#include <xLightsApp.h>
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "render/SequenceMedia.h"

#include <wx/button.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/filepicker.h>
#include <wx/wfstream.h>

#include "utils/nanosvg_xl.h"

#include <algorithm>
#include <cmath>

namespace
{
    const wxString imgSelect("Select an image file");
    const wxString imgFilters("*.jpg;*.gif;*.png;*.bmp;*.jpeg;*.webp");

    const char HotkeysText[] =
        "Shift\tToggle segment type (line, one-point curve, two-point curve)\n"
        "Esc\tEnd current path\n"
        "Space\tClose current path\n"
        "Delete\tDelete point/segment\n\n"
        "Use mouse wheel and middle button to zoom and pan on sketch";
}

BEGIN_EVENT_TABLE(SketchAssistPanel, wxPanel)
END_EVENT_TABLE()

long SketchAssistPanel::ID_MENU_Delete = wxNewId();
long SketchAssistPanel::ID_MENU_Reverse = wxNewId();
long SketchAssistPanel::ID_MENU_MoveUp = wxNewId();
long SketchAssistPanel::ID_MENU_MoveDown = wxNewId();
long SketchAssistPanel::ID_MENU_MoveToTop = wxNewId();
long SketchAssistPanel::ID_MENU_MoveToBottom = wxNewId();
long SketchAssistPanel::ID_MENU_SortByDescription = wxNewId();

SketchAssistPanel::SketchAssistPanel(wxWindow* parent, wxWindowID id /*wxID_ANY*/, const wxPoint& pos /*wxDefaultPosition*/, const wxSize& size /*wxDefaultSize*/) :
    wxPanel(parent, id, pos, size)
{
    auto mainSizer = new wxBoxSizer(wxVERTICAL);
    auto controlsSizer = new wxBoxSizer(wxVERTICAL);
    auto primaryActionSizer = new wxBoxSizer(wxHORIZONTAL);
    auto fileActionSizer = new wxBoxSizer(wxHORIZONTAL);
    auto pathsSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Paths");
    auto pathsHeaderSizer = new wxBoxSizer(wxHORIZONTAL);
    auto pathToolbarSizer = new wxBoxSizer(wxHORIZONTAL);
    auto pathDescriptionSizer = new wxBoxSizer(wxVERTICAL);
    auto hotkeysSizer = new wxBoxSizer(wxVERTICAL);

    auto canvasFrame = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto canvasFrameSizer = new wxFlexGridSizer(1, 1, 0, 0);
    canvasFrameSizer->AddGrowableRow(0);
    canvasFrameSizer->AddGrowableCol(0);
    m_sketchCanvasPanel = new SketchCanvasPanel(this, canvasFrame, wxID_ANY, wxDefaultPosition, wxSize(400, 300));
    canvasFrameSizer->Add(m_sketchCanvasPanel, 1, wxALL | wxEXPAND);
    canvasFrame->SetSizer(canvasFrameSizer);

    m_startPathBtn = new wxButton(this, wxID_ANY, "Start");
    m_continuePathBtn = new wxButton(this, wxID_ANY, "Continue");
    m_endPathBtn = new wxButton(this, wxID_ANY, "End");
    m_closePathBtn = new wxButton(this, wxID_ANY, "Close");

    m_importSketchBtn = new wxButton(this, wxID_ANY, "Import Sketch");
    m_exportSketchBtn = new wxButton(this, wxID_ANY, "Export Sketch");
    m_importSVGBtn = new wxButton(this, wxID_ANY, "Import SVG");
    m_clearSketchBtn = new wxButton(this, wxID_ANY, "Clear Sketch");

    m_movePathUpBtn = new wxButton(pathsSizer->GetStaticBox(), wxID_ANY, L"▲", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    m_movePathDownBtn = new wxButton(pathsSizer->GetStaticBox(), wxID_ANY, L"▼", wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT);
    m_movePathUpBtn->SetToolTip("Move path up");
    m_movePathDownBtn->SetToolTip("Move path down");

    primaryActionSizer->Add(new wxStaticText(this, wxID_ANY, "Path Editing:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    primaryActionSizer->Add(m_startPathBtn, 0, wxALL, 3);
    primaryActionSizer->Add(m_continuePathBtn, 0, wxALL, 3);
    primaryActionSizer->Add(m_endPathBtn, 0, wxALL, 3);
    primaryActionSizer->Add(m_closePathBtn, 0, wxALL, 3);

    fileActionSizer->Add(new wxStaticText(this, wxID_ANY, "File:"), 0, wxALL | wxALIGN_CENTER_VERTICAL, 3);
    fileActionSizer->Add(m_importSketchBtn, 0, wxALL, 3);
    fileActionSizer->Add(m_exportSketchBtn, 0, wxALL, 3);
    fileActionSizer->Add(m_importSVGBtn, 0, wxALL, 3);
    fileActionSizer->Add(m_clearSketchBtn, 0, wxALL, 3);

    m_pathsHeaderLabel = new wxStaticText(pathsSizer->GetStaticBox(), wxID_ANY, "Paths (0)");
    pathsHeaderSizer->Add(m_pathsHeaderLabel, 0, wxALIGN_CENTER_VERTICAL);
    pathsHeaderSizer->AddStretchSpacer(1);
    pathToolbarSizer->Add(m_movePathUpBtn, 0, wxALL, 2);
    pathToolbarSizer->Add(m_movePathDownBtn, 0, wxALL, 2);
    pathsHeaderSizer->Add(pathToolbarSizer, 0, wxALIGN_CENTER_VERTICAL);
    pathsSizer->Add(pathsHeaderSizer, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 3);

    m_pathsListBox = new wxListBox(pathsSizer->GetStaticBox(), wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_EXTENDED);
    const int pathListHeight = m_pathsListBox->GetCharHeight() * 6 + FromDIP(12);
    m_pathsListBox->SetMinSize(wxSize(-1, pathListHeight));
    pathsSizer->Add(m_pathsListBox, 1, wxALL | wxEXPAND, 3);

    m_pathDescriptionLabel = new wxStaticText(this, wxID_ANY, "Path Description");
    m_pathDescriptionText = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    m_pathDescriptionText->SetHint("Enter path description...");
    pathDescriptionSizer->Add(m_pathDescriptionLabel, 0, wxBOTTOM, 2);
    pathDescriptionSizer->Add(m_pathDescriptionText, 0, wxEXPAND);

    hotkeysSizer->Add(new wxStaticText(this, wxID_ANY, "Canvas Hotkeys"), 0, wxBOTTOM, 2);
    auto hotkeysText = new wxStaticText(this, wxID_ANY, HotkeysText);
    hotkeysText->Wrap(260);
    hotkeysSizer->Add(hotkeysText, 0, wxEXPAND);

    controlsSizer->Add(primaryActionSizer, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
    controlsSizer->Add(fileActionSizer, 0, wxLEFT | wxRIGHT | wxTOP | wxEXPAND, 5);
    controlsSizer->Add(pathsSizer, 0, wxALL | wxEXPAND, 5);
    controlsSizer->Add(pathDescriptionSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);
    controlsSizer->Add(hotkeysSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

    mainSizer->Add(canvasFrame, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(controlsSizer, 0, wxLEFT | wxRIGHT | wxBOTTOM | wxEXPAND, 5);

    SetSizer(mainSizer);

    SetName("ID_PANEL_SKETCH_ASSIST");

    Connect(m_startPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_StartPath);
    Connect(m_endPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_EndPath);
    Connect(m_closePathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ClosePath);
    Connect(m_continuePathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ContinuePath);
    Connect(m_clearSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ClearSketch);
    Connect(m_importSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ImportSketch);
    Connect(m_exportSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ExportSketch);
    Connect(m_importSVGBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ImportSVG);
    Connect(m_movePathUpBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_MovePathUp);
    Connect(m_movePathDownBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_MovePathDown);

    m_pathsListBox->Connect(wxEVT_LISTBOX, (wxObjectEventFunction)&SketchAssistPanel::OnListBox_PathSelected, nullptr, this);
    m_pathsListBox->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&SketchAssistPanel::OnListBox_KeyDown, nullptr, this);
    m_pathsListBox->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&SketchAssistPanel::OnListBox_ContextMenu, nullptr, this);
    m_pathDescriptionText->Connect(wxEVT_TEXT, (wxObjectEventFunction)&SketchAssistPanel::OnTextCtrl_PathDescription, nullptr, this);
    m_pathDescriptionText->Connect(wxEVT_TEXT_ENTER, (wxObjectEventFunction)&SketchAssistPanel::OnTextCtrl_PathDescriptionEnter, nullptr, this);
    m_pathDescriptionText->Connect(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&SketchAssistPanel::OnTextCtrl_PathDescriptionKillFocus, nullptr, this);

    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::Undefined);
    UpdatePathLabels();
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
}

void SketchAssistPanel::SetSketchDef(const std::string& sketchDef)
{
    if ( sketchDef != m_sketchDef) {
        m_sketchDef = sketchDef;
        m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
        populatePathListBoxFromSketch();
        UpdatePathDescriptionEditorFromSelection();
        Refresh();
    }
}

void SketchAssistPanel::ForwardKeyEvent(wxKeyEvent& event)
{
    if (m_pathDescriptionText != nullptr && m_pathDescriptionText->HasFocus())
        return;

    if (event.GetKeyCode() == WXK_DELETE && m_pathsListBox != nullptr && m_pathsListBox->HasFocus() && DeleteSelectedPaths())
        return;

    if (m_sketchCanvasPanel != nullptr)
        m_sketchCanvasPanel->OnSketchKeyDown(event);
}

SketchEffectSketch& SketchAssistPanel::GetSketch()
{
    return m_sketch;
}

int SketchAssistPanel::GetSelectedPathIndex()
{
    if (m_pathsListBox == nullptr)
        return 0;
    auto selections = GetSelectedPathIndices();
    if (selections.empty())
        return wxNOT_FOUND;
    return selections.front();
}

void SketchAssistPanel::NotifySketchUpdated()
{
    m_sketchDef = m_sketch.toString();
    if (m_sketchUpdateCB != nullptr)
        m_sketchUpdateCB(m_sketchDef, m_bgImagePath, m_bitmapAlpha);
}

void SketchAssistPanel::NotifySketchPathsUpdated()
{
    populatePathListBoxFromSketch();
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
}

void SketchAssistPanel::NotifyPathStateUpdated(SketchCanvasPathState state)
{
    switch (state) {
    case SketchCanvasPathState::Undefined:
        m_startPathBtn->Enable();
        m_endPathBtn->Disable();
        m_closePathBtn->Disable();
        m_continuePathBtn->Enable(canContinuePath());
        m_clearSketchBtn->Enable();
        break;
    case SketchCanvasPathState::DefineStartPoint:
        m_startPathBtn->Disable();
        m_endPathBtn->Disable();
        m_closePathBtn->Disable();
        m_continuePathBtn->Disable();
        m_clearSketchBtn->Disable();
        break;
    case SketchCanvasPathState::LineToNewPoint:
    case SketchCanvasPathState::QuadraticCurveToNewPoint:
    case SketchCanvasPathState::CubicCurveToNewPoint:
        m_startPathBtn->Disable();
        m_endPathBtn->Enable();
        m_closePathBtn->Enable();
        m_continuePathBtn->Disable();
        m_clearSketchBtn->Disable();
        break;
    }
}

void SketchAssistPanel::SelectLastPath()
{
    unsigned n;

    if ((n = m_pathsListBox->GetCount()) != 0) {
        m_pathsListBox->SetSelection(n - 1);
        UpdatePathDescriptionEditorFromSelection();
        UpdatePathActionButtons();
        m_continuePathBtn->Enable(canContinuePath());
    }
}

void SketchAssistPanel::UpdateSketchBackground(const wxString& imagePath, int opacity)
{
    if (imagePath.IsEmpty()) {
        m_sketchCanvasPanel->clearBackgroundBitmap();
        m_bgImagePath = "";
        return;
    }

    // Resolve relative paths against the show directory for filesystem files.
    wxString resolvedPath = wxString(FileUtils::FixFile(
        xLightsFrame::CurrentDir.ToStdString(), imagePath.ToStdString()));
    if (resolvedPath.IsEmpty())
        resolvedPath = imagePath;

    if (resolvedPath == m_bgImagePath && opacity == m_bitmapAlpha)
        return;

    wxImage img;

    if (wxFileExists(resolvedPath)) {
        img = wxImage(resolvedPath);
    } else {
        // Not on disk — load from SequenceMedia (covers embedded images).
        auto* xl = xLightsApp::GetFrame();
        if (xl) {
            auto& media = xl->GetSequenceElements().GetSequenceMedia();
            auto entry = media.GetImage(imagePath.ToStdString());
            if (entry && entry->IsOk()) {
                auto frame = entry->GetFrame(0, false);
                if (frame && frame->IsOk()) {
                    int w = frame->GetWidth();
                    int h = frame->GetHeight();
                    auto* rgbData = static_cast<unsigned char*>(malloc(w * h * 3));
                    auto* alphaData = static_cast<unsigned char*>(malloc(w * h));
                    const uint8_t* src = frame->GetData();
                    for (int i = 0; i < w * h; ++i) {
                        rgbData[i * 3 + 0] = src[i * 4 + 0];
                        rgbData[i * 3 + 1] = src[i * 4 + 1];
                        rgbData[i * 3 + 2] = src[i * 4 + 2];
                        alphaData[i]        = src[i * 4 + 3];
                    }
                    img = wxImage(w, h, rgbData, alphaData, false);
                }
            }
        }
    }

    if (!img.IsOk()) {
        m_sketchCanvasPanel->clearBackgroundBitmap();
        m_bgImagePath = "";
        return;
    }

    if (!img.HasAlpha())
        img.InitAlpha();

    m_bgImagePath = resolvedPath;
    m_bgImage = img;
    m_bitmapAlpha = static_cast<unsigned char>(opacity);
    updateBgImage();
}

void SketchAssistPanel::OnButton_StartPath(wxCommandEvent& /*event*/)
{
    m_sketchCanvasPanel->ResetHandlesState(SketchCanvasPathState::DefineStartPoint);

    m_pathsListBox->DeselectAll();
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
}

void SketchAssistPanel::OnButton_EndPath(wxCommandEvent& /*event*/)
{
    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::Undefined);
    NotifySketchUpdated();
}

void SketchAssistPanel::OnButton_ClosePath(wxCommandEvent& /*event*/)
{
    m_sketchCanvasPanel->ClosePath();
    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::Undefined);
    NotifySketchUpdated();
}

void SketchAssistPanel::OnButton_ContinuePath(wxCommandEvent& /*event*/)
{
    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::LineToNewPoint);
}

void SketchAssistPanel::OnButton_ClearSketch(wxCommandEvent& /*event*/)
{
    m_sketch = SketchEffectSketch();

    m_pathsListBox->Clear();

    m_sketchCanvasPanel->ResetHandlesState();
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
    NotifySketchUpdated();
}

void SketchAssistPanel::OnButton_ImportSketch(wxCommandEvent& event)
{
    wxString filename = wxFileSelector(_("Choose xLights Sketch File"), xLightsFrame::CurrentDir, wxEmptyString, wxEmptyString, "Sketch Files (*.xsketch)|*.xsketch", wxFD_OPEN | wxFD_FILE_MUST_EXIST);

    if (!filename.empty()) {
        nlohmann::json data;

        try {
            std::ifstream inputFile(filename.ToStdString());
            inputFile >> data;

        } catch (std::exception& ex) {
            DisplayError(wxString::Format("Could not open xLights Sketch file %s.\nError: %s", filename, ex.what()).ToStdString());
            return;
        }

        std::string bgImagePath;
        unsigned char bitmapAlpha = m_bitmapAlpha;

        if (data.contains("imagepath") && data["imagepath"].is_string()) {
            bgImagePath = data["imagepath"].get<std::string>();
        }
        if (data.contains("bitmapalpha") && data["bitmapalpha"].is_number_integer()) {
            bitmapAlpha = data["bitmapalpha"].get<int>();
        }
        if (!bgImagePath.empty()) {
            UpdateSketchBackground(bgImagePath, bitmapAlpha);
        }
        if (data.contains("sketchdata") && data["sketchdata"].is_string()) {
            SetSketchDef(data["sketchdata"].get<std::string>());
            NotifySketchUpdated();
        }
    }
}

void SketchAssistPanel::OnButton_ExportSketch(wxCommandEvent& event)
{
    wxString filename = wxFileSelector(_("Save xLights Sketch File"), xLightsFrame::CurrentDir, wxEmptyString, wxEmptyString, "Sketch Files (*.xsketch)|*.xsketch", wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

    if (!filename.IsEmpty()) {
        nlohmann::json data;
        data["sketchdata"] = m_sketch.toString();
        data["imagepath"] = m_bgImagePath.ToStdString();
        data["bitmapalpha"] = m_bitmapAlpha;

        try {
            std::ofstream o(filename.ToStdString());
            if (o.is_open()) {
                o << std::setw(4) << data << std::endl;
            }
        } catch (const std::exception&) {
        }
    }
}

void SketchAssistPanel::OnButton_ImportSVG(wxCommandEvent& event)
{
    wxString filename = wxFileSelector(_("Choose SVG File"), xLightsFrame::CurrentDir, wxEmptyString, wxEmptyString, "SVG Files (*.svg)|*.svg", wxFD_OPEN | wxFD_FILE_MUST_EXIST);
    if (!filename.IsEmpty()) {
        m_sketch = SketchEffectSketch();

        //100px scaling seems to scale the SVG parser right
        auto* image = nsvgParseFromFile(filename.ToStdString().c_str(), "px", 100);

        if ( nullptr == image) {
            DisplayError(wxString::Format("Could not open SVG image %s.", filename).ToStdString());
            return;
        }

        float h = image->height;
        float w = image->width;
        for (NSVGshape* shape = image->shapes; shape != nullptr; shape = shape->next) {
            for (NSVGpath* path = shape->paths; path != nullptr; path = path->next) {
                auto skpath = std::make_shared<SketchEffectPath>();
                for (int i = 0; i < path->npts - 1; i += 3) {
                    float* p = &path->pts[i * 2];
                    //sketch points are 0-1, need to scale from pixel xy
                    //1-h, everything was upside down for some reason
                    xlPointD start(p[0] / w, 1 - (p[1] / h));
                    xlPointD cp1(p[2] / w, 1 - (p[3] / h));
                    xlPointD cp2(p[4] / w, 1 - (p[5] / h));
                    xlPointD end(p[6] / w, 1 - (p[7] / h));
                    if (areCollinear(start,cp1,end, 0.001f) && areCollinear(start,cp2,end, 0.001f)) {//check if its a straight line
                        skpath->appendSegment(std::make_shared<SketchLine>(start, end));
                    } else if (areSame(end.x, cp2.x, 0.001f) && areSame(end.y, cp2.y, 0.001f)) { // check if control points2 is the end
                        skpath->appendSegment(std::make_shared<SketchQuadraticBezier>(start, cp1, end));
                    } else {
                        skpath->appendSegment(std::make_shared<SketchCubicBezier>(start, cp1, cp2, end));
                    }
                }
                m_sketch.appendPath(skpath);
            }
        }
        // Delete svg pointer
        nsvgDelete(image);
        //redraw screen
        m_sketchCanvasPanel->ResetHandlesState();
        populatePathListBoxFromSketch();
        UpdatePathDescriptionEditorFromSelection();
        UpdatePathActionButtons();
        Refresh();
        NotifySketchUpdated();
    }
}

void SketchAssistPanel::OnButton_MovePathUp(wxCommandEvent& /*event*/)
{
    auto selections = GetSelectedPathIndices();
    if (selections.size() != 1)
        return;

    const int index = selections.front();
    if (index <= 0 || index >= (int)m_sketch.pathCount())
        return;

    m_sketch.swapPaths(index, index - 1);
    m_sketchCanvasPanel->ResetHandlesState();
    populatePathListBoxFromSketch();
    m_pathsListBox->SetSelection(index - 1);
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
    NotifySketchUpdated();
}

void SketchAssistPanel::OnButton_MovePathDown(wxCommandEvent& /*event*/)
{
    auto selections = GetSelectedPathIndices();
    if (selections.size() != 1)
        return;

    const int index = selections.front();
    if (index < 0 || index >= (int)m_sketch.pathCount() - 1)
        return;

    m_sketch.swapPaths(index, index + 1);
    m_sketchCanvasPanel->ResetHandlesState();
    populatePathListBoxFromSketch();
    m_pathsListBox->SetSelection(index + 1);
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
    NotifySketchUpdated();
}

void SketchAssistPanel::OnListBox_PathSelected(wxCommandEvent& /*event*/)
{
    auto selections = GetSelectedPathIndices();
    if (selections.size() == 1) {
        m_sketchCanvasPanel->UpdateHandlesForPath(selections.front());
    } else {
        m_sketchCanvasPanel->ResetHandlesState();
    }
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
    m_continuePathBtn->Enable(canContinuePath());
}

void SketchAssistPanel::OnListBox_KeyDown(wxKeyEvent& event)
{
    if (event.GetKeyCode() == WXK_DELETE && DeleteSelectedPaths())
        return;
    event.Skip();
}

void SketchAssistPanel::OnTextCtrl_PathDescription(wxCommandEvent& /*event*/)
{
    if (m_ignoreDescriptionTextEvent)
        return;

    auto selections = GetSelectedPathIndices();
    if (selections.size() != 1)
        return;

    const int index = selections.front();
    if (index < 0 || index >= (int)m_sketch.pathCount())
        return;

    auto& path = m_sketch.paths()[index];
    if (path == nullptr)
        return;

    path->SetDescription(m_pathDescriptionText->GetValue().ToStdString());
    wxString text;
    text.sprintf("Path %d", index + 1);
    if (!path->GetDescription().empty()) {
        text += " - ";
        text += path->GetDescription();
    }
    m_pathsListBox->SetString(index, text);
}

void SketchAssistPanel::OnTextCtrl_PathDescriptionEnter(wxCommandEvent& /*event*/)
{
    if (m_ignoreDescriptionTextEvent)
        return;
    if (m_lastCommittedPathDescription != m_pathDescriptionText->GetValue().ToStdString()) {
        m_lastCommittedPathDescription = m_pathDescriptionText->GetValue().ToStdString();
        NotifySketchUpdated();
    }
}

void SketchAssistPanel::OnTextCtrl_PathDescriptionKillFocus(wxFocusEvent& event)
{
    if (!m_ignoreDescriptionTextEvent) {
        const std::string description = m_pathDescriptionText->GetValue().ToStdString();
        if (m_lastCommittedPathDescription != description) {
            m_lastCommittedPathDescription = description;
            NotifySketchUpdated();
        }
    }
    event.Skip();
}

void SketchAssistPanel::OnListBox_ContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt(m_pathsListBox->ScreenToClient(event.GetPosition()));
    m_pathIndexToDelete = m_pathsListBox->HitTest(pt);

    wxString str;
    wxMenu mnu;

    if (m_pathIndexToDelete >= 0) {
        str.sprintf("Reverse Path %d", 1 + m_pathIndexToDelete);
        mnu.Append(ID_MENU_Reverse, str);
        str.sprintf("Delete Path %d", 1 + m_pathIndexToDelete);
        mnu.Append(ID_MENU_Delete, str);
        if (m_pathIndexToDelete != 0) {
            str.sprintf("Move Path %d Up", 1 + m_pathIndexToDelete);
            mnu.Append(ID_MENU_MoveUp, str);
        }
        if (m_pathIndexToDelete != (int)m_pathsListBox->GetCount() - 1) {
            str.sprintf("Move Path %d Down", 1 + m_pathIndexToDelete);
            mnu.Append(ID_MENU_MoveDown, str);
        }
        if (m_pathIndexToDelete != 0) {
            str.sprintf("Move Path %d to Top", 1 + m_pathIndexToDelete);
            mnu.Append(ID_MENU_MoveToTop, str);
        }
        if (m_pathIndexToDelete != (int)m_pathsListBox->GetCount() - 1) {
            str.sprintf("Move Path %d to Bottom", 1 + m_pathIndexToDelete);
            mnu.Append(ID_MENU_MoveToBottom, str);
        }
    }

    if (m_pathsListBox->GetCount() > 1) {
        if (mnu.GetMenuItemCount() > 0)
            mnu.AppendSeparator();
        mnu.Append(ID_MENU_SortByDescription, "Sort All by Description");
    }

    if (mnu.GetMenuItemCount() == 0)
        return;
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SketchAssistPanel::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void SketchAssistPanel::OnPopupCommand(wxCommandEvent& event)
{
    if (event.GetId() == ID_MENU_SortByDescription) {
        std::sort(m_sketch.paths().begin(), m_sketch.paths().end(),
                  [](const auto& a, const auto& b) {
                      return a->GetDescription() < b->GetDescription();
                  });
        m_sketchCanvasPanel->ResetHandlesState();
        populatePathListBoxFromSketch();
        UpdatePathDescriptionEditorFromSelection();
        UpdatePathActionButtons();
        NotifySketchUpdated();
        return;
    }

    if (m_pathIndexToDelete < 0 || m_pathIndexToDelete >= (int)m_sketch.pathCount())
        return;
    bool update = false;

    if (event.GetId() == ID_MENU_Delete) {
        m_sketch.deletePath(m_pathIndexToDelete);
        update = true;
    } else if (event.GetId() == ID_MENU_Reverse) {
        m_sketch.reversePath(m_pathIndexToDelete);
        update = true;
    } else if (event.GetId() == ID_MENU_MoveUp) {
        m_sketch.swapPaths(m_pathIndexToDelete, m_pathIndexToDelete - 1);
        update = true;
    } else if (event.GetId() == ID_MENU_MoveDown) {
        m_sketch.swapPaths(m_pathIndexToDelete, m_pathIndexToDelete + 1);
        update = true;
    } else if (event.GetId() == ID_MENU_MoveToTop) {
        for (int i = m_pathIndexToDelete; i > 0; --i)
            m_sketch.swapPaths(i, i - 1);
        update = true;
    } else if (event.GetId() == ID_MENU_MoveToBottom) {
        const int last = (int)m_sketch.pathCount() - 1;
        for (int i = m_pathIndexToDelete; i < last; ++i)
            m_sketch.swapPaths(i, i + 1);
        update = true;
    }

    if (update) {
        m_sketchCanvasPanel->ResetHandlesState();

        populatePathListBoxFromSketch();
        UpdatePathDescriptionEditorFromSelection();
        UpdatePathActionButtons();
        NotifySketchUpdated();
    }
}

void SketchAssistPanel::updateBgImage()
{
    if (!m_bgImage.IsOk()) {
        m_sketchCanvasPanel->clearBackgroundBitmap();
        return;
    }

    int w = m_bgImage.GetWidth();
    int h = m_bgImage.GetHeight();
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            m_bgImage.SetAlpha(x, y, m_bitmapAlpha);

    m_sketchCanvasPanel->setBackgroundBitmap(std::make_unique<wxBitmap>(m_bgImage));
}

void SketchAssistPanel::populatePathListBoxFromSketch()
{
    m_pathsListBox->Clear();

    for (int i = 0; i < (int)m_sketch.paths().size(); ++i) {
        wxString text;
        text.sprintf("Path %d", i + 1);
        const auto& description = m_sketch.paths()[i]->GetDescription();
        if (!description.empty()) {
            text += " - ";
            text += description;
        }
        m_pathsListBox->Insert(text, i);
    }
    UpdatePathLabels();
}

bool SketchAssistPanel::DeleteSelectedPaths()
{
    auto selections = GetSelectedPathIndices();
    if (selections.empty())
        return false;

    std::sort(selections.begin(), selections.end(), std::greater<int>());
    for (const auto index : selections) {
        if (index >= 0 && index < (int)m_sketch.pathCount())
            m_sketch.deletePath(index);
    }

    m_sketchCanvasPanel->ResetHandlesState();
    populatePathListBoxFromSketch();
    UpdatePathDescriptionEditorFromSelection();
    UpdatePathActionButtons();
    NotifySketchUpdated();
    m_continuePathBtn->Enable(canContinuePath());
    return true;
}

std::vector<int> SketchAssistPanel::GetSelectedPathIndices() const
{
    std::vector<int> selections;
    if (m_pathsListBox == nullptr)
        return selections;

    wxArrayInt indices;
    m_pathsListBox->GetSelections(indices);
    selections.reserve(indices.GetCount());
    for (auto i = 0u; i < indices.GetCount(); ++i)
        selections.push_back(indices[i]);
    return selections;
}

void SketchAssistPanel::UpdatePathDescriptionEditorFromSelection()
{
    if (m_pathDescriptionText == nullptr)
        return;

    auto selections = GetSelectedPathIndices();
    if (selections.size() != 1 || selections.front() < 0 || selections.front() >= (int)m_sketch.pathCount()) {
        m_ignoreDescriptionTextEvent = true;
        m_pathDescriptionText->ChangeValue(wxEmptyString);
        m_ignoreDescriptionTextEvent = false;
        m_lastCommittedPathDescription.clear();
        m_pathDescriptionText->Enable(false);
        if (m_pathDescriptionLabel != nullptr)
            m_pathDescriptionLabel->SetLabel("Path Description");
        return;
    }

    const int index = selections.front();
    m_ignoreDescriptionTextEvent = true;
    m_pathDescriptionText->ChangeValue(wxString(m_sketch.paths()[index]->GetDescription()));
    m_ignoreDescriptionTextEvent = false;
    m_lastCommittedPathDescription = m_sketch.paths()[index]->GetDescription();
    m_pathDescriptionText->Enable(true);
    if (m_pathDescriptionLabel != nullptr)
        m_pathDescriptionLabel->SetLabel("Path Description");
}

void SketchAssistPanel::UpdatePathLabels()
{
    if (m_pathsHeaderLabel != nullptr)
        m_pathsHeaderLabel->SetLabel(wxString::Format("Paths (%d)", (int)m_sketch.pathCount()));
}

void SketchAssistPanel::UpdatePathActionButtons()
{
    auto selections = GetSelectedPathIndices();
    const bool singleSelection = selections.size() == 1;
    const int selectedIndex = singleSelection ? selections.front() : -1;
    const int lastIndex = (int)m_sketch.pathCount() - 1;

    if (m_movePathUpBtn != nullptr)
        m_movePathUpBtn->Enable(singleSelection && selectedIndex > 0);
    if (m_movePathDownBtn != nullptr)
        m_movePathDownBtn->Enable(singleSelection && selectedIndex >= 0 && selectedIndex < lastIndex);
}

bool SketchAssistPanel::canContinuePath() const
{
    auto selections = GetSelectedPathIndices();
    if (selections.size() != 1)
        return false;
    const int index = selections.front();
    if (index < 0 || index >= (int)m_sketch.pathCount())
        return false;
    return !m_sketch.paths()[index]->isClosed();
}

bool SketchAssistPanel::areSame(double a, double b, float eps) const
{
    return std::fabs(a - b) < eps;
}

bool SketchAssistPanel::areCollinear(const xlPointD& a, const xlPointD& b, const xlPointD& c, double eps) const
{
    //use dot product to determine if point are in a strait line
    auto [a_x, a_y] = a;
    auto [b_x, b_y] = b;
    auto [c_x, c_y] = c;

    auto test = (b_x - a_x) * (c_y - a_y) - (c_x - a_x) * (b_y - a_y);
    return std::abs(test) < eps;
}
