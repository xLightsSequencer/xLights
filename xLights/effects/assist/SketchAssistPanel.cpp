#include "SketchAssistPanel.h"
#include "SketchCanvasPanel.h"

#include <wx/filepicker.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

namespace
{
    const wxString imgSelect("Select an image file");
    const wxString imgFilters("*.jpg;*.gif;*.png;*.bmp;*.jpeg");

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

SketchAssistPanel::SketchAssistPanel(wxWindow* parent, wxWindowID id /*wxID_ANY*/, const wxPoint& pos /*wxDefaultPosition*/, const wxSize& size /*wxDefaultSize*/) :
    wxPanel(parent, id, pos, size)
{
    /*
 
    mainSizer
     \
      pathUISizer (canvas, bg-image controls, and path/sketch controls)
      |\
      | canvasFrame (m_sketchCanvasPanel)
      |\
      | bgSizer (m_filePicker, m_bgAlphaSlider)
      |
      |\
      | pathSketchCtrlsSizer
      | |\
      | | pathCtrlsSizer (m_startPathBtn, m_endPathBtn, m_closePathBtn)
      | |
      |  \
      |   sketchCtrlsSizer ("Clear" sketch button)
      |
      sketchUISizer (hotkeys text and paths listbox)
      |\
      | hotkeysSizer (Canvas hotkeys)
      |
      |\
      | pathsSizer (m_pathsListBox)

    */
    auto mainSizer = new wxFlexGridSizer(3, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);

    auto pathUISizer = new wxFlexGridSizer(3, 1, 5, 0);
    pathUISizer->AddGrowableRow(0);
    pathUISizer->AddGrowableCol(0);

    auto bgSizer = new wxFlexGridSizer(1, 3, 0, 0);
    bgSizer->AddGrowableCol(1);

    auto pathSketchCtrlsSizer = new wxFlexGridSizer(1, 3, 0, 5);
    auto sketchCtrlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Sketch");

    auto sketchUISizer = new wxFlexGridSizer(2, 1, 5, 0);
    sketchUISizer->AddGrowableRow(1);
    sketchUISizer->AddGrowableCol(0);

    auto hotkeysSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Canvas hotkeys");
    auto pathsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Paths");

    // canvas
    auto canvasFrame = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto canvasFrameSizer = new wxFlexGridSizer(1, 1, 0, 0);
    canvasFrameSizer->AddGrowableRow(0);
    canvasFrameSizer->AddGrowableCol(0);
    m_sketchCanvasPanel = new SketchCanvasPanel(this, canvasFrame, wxID_ANY, wxDefaultPosition, wxSize(400, 300));
    canvasFrameSizer->Add(m_sketchCanvasPanel, 1, wxALL | wxEXPAND);
    canvasFrame->SetSizer(canvasFrameSizer);

    // background image controls
    auto bgLabel = new wxStaticText(this, wxID_ANY, "Background:");
    m_filePicker = new wxFilePickerCtrl(this, wxID_ANY, wxEmptyString, imgSelect, imgFilters, wxDefaultPosition, wxDefaultSize, wxFLP_FILE_MUST_EXIST | wxFLP_OPEN | wxFLP_USE_TEXTCTRL);
    m_filePicker->GetTextCtrl()->SetEditable(false);
    m_bgAlphaSlider = new wxSlider(this, wxID_ANY, m_bitmapAlpha, 0x00, 0xff);

    bgSizer->Add(bgLabel, 1, wxALL | wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL, 2);
    bgSizer->Add(m_filePicker, 1, wxALL | wxEXPAND, 2);
    bgSizer->Add(m_bgAlphaSlider, 1, wxALL | wxEXPAND, 2);

    // path / sketch controls
    m_startPathBtn = new wxButton(this, wxID_ANY, "Start");
    m_endPathBtn = new wxButton(this, wxID_ANY, "End");
    m_closePathBtn = new wxButton(this, wxID_ANY, "Close");
    m_continuePathBtn = new wxButton(this, wxID_ANY, "Continue");

    m_clearSketchBtn = new wxButton(this, wxID_ANY, "Clear");

    auto pathCtrlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Path");
    pathCtrlsSizer->Add(m_startPathBtn, 1, wxALL | wxEXPAND, 3);
    pathCtrlsSizer->Add(m_endPathBtn, 1, wxALL | wxEXPAND, 3);
    pathCtrlsSizer->Add(m_closePathBtn, 1, wxALL | wxEXPAND, 3);
    pathCtrlsSizer->Add(m_continuePathBtn, 1, wxALL | wxEXPAND, 3);

    sketchCtrlsSizer->Add(m_clearSketchBtn, 1, wxALL | wxEXPAND, 3);

    pathSketchCtrlsSizer->AddGrowableCol(2);
    pathSketchCtrlsSizer->Add(pathCtrlsSizer, 1, wxALL, 2);
    pathSketchCtrlsSizer->Add(sketchCtrlsSizer, 1, wxALL, 2);

    // Hotkeys text
    hotkeysSizer->Add(new wxStaticText(hotkeysSizer->GetStaticBox(), wxID_ANY, HotkeysText), 1, wxALL | wxEXPAND, 3);

    // Paths ListBox
    m_pathsListBox = new wxListBox(this, wxID_ANY);
    pathsSizer->Add(m_pathsListBox, 1, wxALL | wxEXPAND, 3);

    sketchUISizer->Add(hotkeysSizer, 1, wxALL | wxEXPAND);
    sketchUISizer->Add(pathsSizer, 1, wxALL | wxEXPAND, 5);

    pathUISizer->Add(canvasFrame, 1, wxALL | wxEXPAND, 5);
    pathUISizer->Add(bgSizer, 1, wxALL | wxEXPAND);
    pathUISizer->Add(pathSketchCtrlsSizer, 1, wxALL | wxEXPAND, 5);

    mainSizer->Add(pathUISizer, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(sketchUISizer, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);

    Connect(m_filePicker->GetId(), wxEVT_COMMAND_FILEPICKER_CHANGED, (wxObjectEventFunction)&SketchAssistPanel::OnFilePickerCtrl_FileChanged);
    Connect(m_bgAlphaSlider->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&SketchAssistPanel::OnSlider_BgAlphaChanged);

    Connect(m_startPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_StartPath);
    Connect(m_endPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_EndPath);
    Connect(m_closePathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ClosePath);
    Connect(m_continuePathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ContinuePath);
    Connect(m_clearSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchAssistPanel::OnButton_ClearSketch);

    m_pathsListBox->Connect(wxEVT_LISTBOX, (wxObjectEventFunction)&SketchAssistPanel::OnListBox_PathSelected, nullptr, this);
    m_pathsListBox->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&SketchAssistPanel::OnListBox_ContextMenu, nullptr, this);

    m_sketchCanvasPanel->UpdatePathState(SketchCanvasPathState::Undefined);
}

void SketchAssistPanel::SetSketchDef(const std::string& sketchDef)
{
    if ( sketchDef != m_sketchDef) {
        m_sketchDef = sketchDef;
        m_sketch = SketchEffectSketch::SketchFromString(m_sketchDef);
        populatePathListBoxFromSketch();
        Refresh();
    }
}

void SketchAssistPanel::ForwardKeyEvent(wxKeyEvent& event)
{
    if (m_sketchCanvasPanel != nullptr)
        m_sketchCanvasPanel->OnSketchKeyDown(event);
}

SketchEffectSketch& SketchAssistPanel::GetSketch()
{
    return m_sketch;
}

int SketchAssistPanel::GetSelectedPathIndex()
{
    return m_pathsListBox->GetSelection();
}

void SketchAssistPanel::NotifySketchUpdated()
{
    m_sketchDef = m_sketch.toString();
    if (m_sketchUpdateCB != nullptr)
        m_sketchUpdateCB(m_sketchDef);
}

void SketchAssistPanel::NotifySketchPathsUpdated()
{
    populatePathListBoxFromSketch();
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
        m_continuePathBtn->Enable(canContinuePath());
    }
}

void SketchAssistPanel::OnFilePickerCtrl_FileChanged(wxCommandEvent& /*event*/)
{
    wxImage img(m_filePicker->GetFileName().GetFullPath());
    if (img.IsOk()) {
        if (!img.HasAlpha())
            img.InitAlpha();

        m_bgImage = img;
        updateBgImage();
    }
}

void SketchAssistPanel::OnSlider_BgAlphaChanged(wxCommandEvent& event)
{
    m_bitmapAlpha = static_cast<unsigned char>(m_bgAlphaSlider->GetValue());
    updateBgImage();
}

void SketchAssistPanel::OnButton_StartPath(wxCommandEvent& /*event*/)
{
    m_sketchCanvasPanel->ResetHandlesState(SketchCanvasPathState::DefineStartPoint);

    m_pathsListBox->DeselectAll();
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
    NotifySketchUpdated();
}

void SketchAssistPanel::OnListBox_PathSelected(wxCommandEvent& /*event*/)
{
    int index = m_pathsListBox->GetSelection();
    m_sketchCanvasPanel->UpdateHandlesForPath(index);
    m_continuePathBtn->Enable(canContinuePath());
}

void SketchAssistPanel::OnListBox_ContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt(m_pathsListBox->ScreenToClient(event.GetPosition()));
    m_pathIndexToDelete = m_pathsListBox->HitTest(pt);
    if (m_pathIndexToDelete < 0)
        return;

    wxString str;
    str.sprintf("Delete Path %d", 1 + m_pathIndexToDelete);

    wxMenu mnu;
    mnu.Append(ID_MENU_Delete, str);
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SketchAssistPanel::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void SketchAssistPanel::OnPopupCommand(wxCommandEvent& event)
{
    if (event.GetId() == ID_MENU_Delete && m_pathIndexToDelete < m_sketch.pathCount()) {
        m_sketch.deletePath(m_pathIndexToDelete);

        m_sketchCanvasPanel->ResetHandlesState();

        populatePathListBoxFromSketch();
        NotifySketchUpdated();
    }
}

void SketchAssistPanel::updateBgImage()
{
    if (!m_bgImage.IsOk())
        return;

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

    for (int i = 0; i < m_sketch.paths().size(); ++i) {
        wxString text;
        text.sprintf("Path %d", i + 1);
        m_pathsListBox->Insert(text, i);
    }
}

bool SketchAssistPanel::canContinuePath() const
{
    int index = m_pathsListBox->GetSelection();
    if (index < 0)
        return false;

    if (index >= m_sketch.pathCount())
        return false;
    auto paths(m_sketch.paths());
    return !paths[index]->isClosed();
}
