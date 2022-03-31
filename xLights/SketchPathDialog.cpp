#include "SketchPathDialog.h"

#include <algorithm>
#include <optional>
#include <utility>

#include <wx/affinematrix2d.h>
#include <wx/bitmap.h>
#include <wx/brush.h>
#include <wx/button.h>
#include <wx/dcbuffer.h>
#include <wx/filepicker.h>
#include <wx/graphics.h>
#include <wx/listbox.h>
#include <wx/menu.h>
#include <wx/panel.h>
#include <wx/pen.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

BEGIN_EVENT_TABLE(SketchPathDialog, wxDialog)
END_EVENT_TABLE()

namespace
{
    const int BorderWidth = 5;
    const int MouseWheelLimit = 1440;

    const wxString imgSelect("Select an image file");
    const wxString imgFilters(" *.jpg; *.gif; *.png; *.bmp; *.jpeg");

    const char HotkeysText[] =
        "Shift\tToggle segment type (line, one-point curve, two-point curve)\n"
        "Esc\tEnd current path\n"
        "Space\tClose current path\n"
        "Delete\tDelete point/segment\n\n"
        "Use mouse wheel and middle button to zoom and pan on sketch";

    struct LinearInterpolater {
        double operator()(double t) const
        {
            return t;
        }
    };

    template<class T>
    double interpolate(double x, double loIn, double loOut, double hiIn, double hiOut, const T& interpolater)
    {
        return (loIn != hiIn)
                   ? (loOut + (hiOut - loOut) * interpolater((x - loIn) / (hiIn - loIn)))
                   : ((loOut + hiOut) / 2);
    }
}

long SketchPathDialog::ID_MENU_Delete = wxNewId();

SketchPathDialog::SketchPathDialog(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size)
{
    Create(parent, wxID_ANY, "Define Sketch", wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER | wxCLOSE_BOX, _T("wxID_ANY"));

    auto mainSizer = new wxFlexGridSizer(3, 1, 0, 0);
    mainSizer->AddGrowableCol(0);
    mainSizer->AddGrowableRow(0);

    auto pathUISizer = new wxFlexGridSizer(3, 1, 5, 0);
    pathUISizer->AddGrowableRow(0);
    pathUISizer->AddGrowableCol(0);

    // canvas
    auto frame = new wxStaticBox(this, wxID_ANY, wxEmptyString);
    auto frameSizer = new wxFlexGridSizer(1, 1, 0, 0);
    frameSizer->AddGrowableRow(0);
    frameSizer->AddGrowableCol(0);
    m_sketchPanel = new wxPanel(frame, wxID_ANY, wxDefaultPosition, wxSize(400, 300), wxALL | wxEXPAND);
    m_sketchPanel->SetBackgroundStyle(wxBG_STYLE_PAINT);
    frameSizer->Add(m_sketchPanel, 1, wxALL | wxEXPAND);
    frame->SetSizer(frameSizer);
    pathUISizer->Add(frame, 1, wxALL | wxEXPAND, 5);

    // background image controls
    auto bgSizer = new wxFlexGridSizer(1, 3, 0, 0);
    bgSizer->AddGrowableCol(1);
    pathUISizer->Add(bgSizer, 1, wxALL | wxEXPAND);
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
    auto clearSketchBtn = new wxButton(this, wxID_ANY, "Clear");

    auto pathCtrlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Path");
    pathCtrlsSizer->Add(m_startPathBtn, 1, wxALL | wxEXPAND, 3);
    pathCtrlsSizer->Add(m_endPathBtn, 1, wxALL | wxEXPAND, 3);
    pathCtrlsSizer->Add(m_closePathBtn, 1, wxALL | wxEXPAND, 3);

    auto sketchCtrlsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Sketch");
    sketchCtrlsSizer->Add(clearSketchBtn, 1, wxALL | wxEXPAND, 3);

    auto pathSketchCtrlsSizer = new wxFlexGridSizer(1, 3, 0, 5);
    pathSketchCtrlsSizer->AddGrowableCol(2);
    pathSketchCtrlsSizer->Add(pathCtrlsSizer, 1, wxALL, 2);
    pathSketchCtrlsSizer->Add(sketchCtrlsSizer, 1, wxALL, 2);
    pathUISizer->Add(pathSketchCtrlsSizer, 1, wxALL | wxEXPAND);

    mainSizer->Add(pathUISizer, 1, wxALL | wxEXPAND, 5);

    // Sketch UI
    auto sketchUISizer = new wxFlexGridSizer(2, 1, 5, 0);
    sketchUISizer->AddGrowableRow(1);
    sketchUISizer->AddGrowableCol(0);
    auto hotkeysSizer = new wxStaticBoxSizer(wxVERTICAL, this, "Canvas hotkeys");
    hotkeysSizer->Add(new wxStaticText(hotkeysSizer->GetStaticBox(), wxID_ANY, HotkeysText), 1, wxALL | wxEXPAND, 3);

    // Paths UI
    m_pathsListBox = new wxListBox(this, wxID_ANY);
    auto pathsSizer = new wxStaticBoxSizer(wxHORIZONTAL, this, "Paths");
    pathsSizer->Add(m_pathsListBox, 1, wxALL | wxEXPAND, 3);

    sketchUISizer->Add(hotkeysSizer, 1, wxALL | wxEXPAND);
    sketchUISizer->Add(pathsSizer, 1, wxALL | wxEXPAND, 5);
    mainSizer->Add(sketchUISizer, 1, wxALL | wxEXPAND, 5);

    // Ok / Cancel
    auto okCancelSizer = new wxFlexGridSizer(1, 3, 0, 0);
    okCancelSizer->AddGrowableCol(0);
    okCancelSizer->AddStretchSpacer();
    auto okButton = new wxButton(this, wxID_ANY, "Ok");
    okCancelSizer->Add(okButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    auto cancelButton = new wxButton(this, wxID_ANY, "Cancel");
    okCancelSizer->Add(cancelButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, 5);
    mainSizer->Add(okCancelSizer, 1, wxALL | wxEXPAND, 5);

    SetSizer(mainSizer);
    mainSizer->Fit(this);
    mainSizer->SetSizeHints(this);

    m_sketchPanel->Connect(wxEVT_PAINT, (wxObjectEventFunction)&SketchPathDialog::OnSketchPaint, nullptr, this);
    m_sketchPanel->Connect(wxEVT_KEY_DOWN, (wxObjectEventFunction)&SketchPathDialog::OnSketchKeyDown, nullptr, this);
    m_sketchPanel->Connect(wxEVT_LEFT_DOWN, (wxObjectEventFunction)&SketchPathDialog::OnSketchLeftDown, nullptr, this);
    m_sketchPanel->Connect(wxEVT_LEFT_UP, (wxObjectEventFunction)&SketchPathDialog::OnSketchLeftUp, nullptr, this);
    m_sketchPanel->Connect(wxEVT_MOTION, (wxObjectEventFunction)&SketchPathDialog::OnSketchMouseMove, nullptr, this);
    m_sketchPanel->Connect(wxEVT_ENTER_WINDOW, (wxObjectEventFunction)&SketchPathDialog::OnSketchEntered, nullptr, this);
    m_sketchPanel->Connect(wxEVT_MOUSEWHEEL, (wxObjectEventFunction)&SketchPathDialog::OnSketchMouseWheel, nullptr, this);
    m_sketchPanel->Connect(wxEVT_MIDDLE_DOWN, (wxObjectEventFunction)&SketchPathDialog::OnSketchMidDown, nullptr, this);

    Connect(m_filePicker->GetId(), wxEVT_COMMAND_FILEPICKER_CHANGED, (wxObjectEventFunction)&SketchPathDialog::OnFilePickerCtrl_FileChanged);
    Connect(m_bgAlphaSlider->GetId(), wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&SketchPathDialog::OnSlider_BgAlphaChanged);

    Connect(m_startPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_StartPath);
    Connect(m_endPathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_EndPath);
    Connect(m_closePathBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_ClosePath);
    Connect(clearSketchBtn->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_ClearSketch);

    Connect(m_pathsListBox->GetId(), wxEVT_LISTBOX, (wxObjectEventFunction)&SketchPathDialog::OnListBox_PathSelected);
    m_pathsListBox->Connect(wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&SketchPathDialog::OnListBox_ContextMenu, nullptr, this);

    Connect(okButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Ok);
    Connect(cancelButton->GetId(), wxEVT_COMMAND_BUTTON_CLICKED, (wxObjectEventFunction)&SketchPathDialog::OnButton_Cancel);

    UpdatePathState(Undefined);
}

void SketchPathDialog::setSketch(const std::string& sketchStr)
{
    m_sketch = SketchEffectSketch::SketchFromString(sketchStr);

    PopulatePathListBoxFromSketch();
}

std::string SketchPathDialog::sketchDefString() const
{
    return m_sketch.toString();
}

void SketchPathDialog::OnSketchPaint(wxPaintEvent& event)
{
    wxAutoBufferedPaintDC pdc(m_sketchPanel);
    wxSize sz(m_sketchPanel->GetSize());
    wxRect borderRect(BorderWidth, BorderWidth, sz.GetWidth() - 2 * BorderWidth - 2, sz.GetHeight() - 2 * BorderWidth - 2);
    wxRect bgRect(wxRect(borderRect).Deflate(1, 1));

    pdc.SetPen(*wxWHITE_PEN);
    pdc.SetBrush(*wxWHITE_BRUSH);
    pdc.DrawRectangle(wxPoint(0, 0), sz);

    pdc.SetPen(*wxLIGHT_GREY_PEN);
    pdc.DrawRectangle(borderRect);

    {
        std::unique_ptr<wxGraphicsContext> gc(wxGraphicsContext::Create(pdc));
        double zoomLevel = 1.;
        if (m_wheelRotation) {
            zoomLevel = interpolate(m_wheelRotation, 0, 1., MouseWheelLimit, 8., LinearInterpolater());
            wxGraphicsMatrix m = gc->CreateMatrix();
            wxPoint2DDouble pt(NormalizedToUI(m_normalizedZoomPt));
            m.Translate(-m_canvasTranslation.m_x, -m_canvasTranslation.m_y);
            m.Translate(pt.m_x, pt.m_y);
            m.Scale(zoomLevel, zoomLevel);
            m.Translate(-pt.m_x, -pt.m_y);
            gc->SetTransform(m);

            m.Get(m_matrixComponents, m_matrixComponents + 1, m_matrixComponents + 2,
                  m_matrixComponents + 3, m_matrixComponents + 4, m_matrixComponents + 5);
        }

        // First, draw the background
        if (m_bgBitmap != nullptr)
            gc->DrawBitmap(*(m_bgBitmap.get()), bgRect.x, bgRect.y, bgRect.width, bgRect.height);

        // Next, draw the unselected path(s)
        wxGraphicsPen pen = gc->CreatePen(wxGraphicsPenInfo(*wxLIGHT_GREY, 1. / zoomLevel));
        gc->SetPen(pen);

        long selectedPathIndex = m_pathsListBox->GetSelection();
        long pathIndex = 0;
        for (const auto& path : m_sketch.paths() ) {
            if (pathIndex++ == selectedPathIndex)
                continue;
            wxGraphicsPath graphicsPath(gc->CreatePath());
            const auto& firstSegment( path->segments().front() );
            wxPoint2DDouble startPt(NormalizedToUI(firstSegment->StartPoint()));
            graphicsPath.MoveToPoint(startPt);

            for (const auto& segment : path->segments()) {
                std::shared_ptr<SketchQuadraticBezier> quadratic;
                std::shared_ptr<SketchCubicBezier> cubic;

                if (std::dynamic_pointer_cast<SketchLine>(segment) != nullptr) {
                    graphicsPath.AddLineToPoint(NormalizedToUI(segment->EndPoint()));
                } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segment)) != nullptr) {
                    wxPoint2DDouble ctrlPt(NormalizedToUI(quadratic->ControlPoint()));
                    wxPoint2DDouble endPt(NormalizedToUI(quadratic->EndPoint()));
                    graphicsPath.AddQuadCurveToPoint(ctrlPt.m_x, ctrlPt.m_y, endPt.m_x, endPt.m_y);
                } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segment)) != nullptr) {
                    wxPoint2DDouble ctrlPt1(NormalizedToUI(cubic->ControlPoint1()));
                    wxPoint2DDouble ctrlPt2(NormalizedToUI(cubic->ControlPoint2()));
                    wxPoint2DDouble endPt(NormalizedToUI(cubic->EndPoint()));
                    graphicsPath.AddCurveToPoint(ctrlPt1, ctrlPt2, endPt);
                }
            }
            if (path->isClosed())
                graphicsPath.CloseSubpath();

            gc->DrawPath(graphicsPath);
        }

        // Next, draw the selected path
        pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 1. / zoomLevel));
        gc->SetPen(pen);
        std::vector<wxPoint>::size_type n = m_handles.size();
        wxGraphicsPath path( gc->CreatePath() );
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0)
                path.MoveToPoint(pt);
            else if (m_handles[i].handlePointType == Point)
                path.AddLineToPoint(pt);
            else if (m_handles[i].handlePointType == QuadraticControlPt) {
                auto endPt = NormalizedToUI(m_handles[i + 1].pt);
                path.AddQuadCurveToPoint(pt.m_x, pt.m_y, endPt.m_x, endPt.m_y);
                i += 1;
            }
            else if (m_handles[i].handlePointType == CubicControlPt1)
            {
                auto cp2 = NormalizedToUI(m_handles[i + 1].pt);
                auto endPt = NormalizedToUI(m_handles[i + 2].pt);
                path.AddCurveToPoint(pt, cp2, endPt);
                i += 2;
            }
        }

        if (m_pathClosed && m_handles.size() > 2)
            path.CloseSubpath();
        gc->DrawPath(path);

        // Next, if we are in a segment-adding state, draw that potential segment
        if ((m_pathState == LineToNewPoint || m_pathState == QuadraticCurveToNewPoint || m_pathState == CubicCurveToNewPoint) && !m_handles.empty())
        {
            switch (m_pathState) {
            case LineToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            case QuadraticCurveToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxRED, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            case CubicCurveToNewPoint:
                pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLUE, 2. / zoomLevel, wxPENSTYLE_SHORT_DASH));
                break;
            }
            gc->SetPen(pen);

            wxAffineMatrix2D m;
            if (m_wheelRotation) {
                wxMatrix2D m2d(m_matrixComponents[0], m_matrixComponents[1],
                               m_matrixComponents[2], m_matrixComponents[3]);
                wxPoint2DDouble mt(m_matrixComponents[4], m_matrixComponents[5]);
                m.Set(m2d, mt);
                m.Invert();
            }

            auto ptFrom = NormalizedToUI(m_handles.back().pt);
            auto ptTo(m.TransformPoint(m_mousePos));
            gc->StrokeLine(ptFrom.m_x, ptFrom.m_y, ptTo.m_x, ptTo.m_y);

            pen = gc->CreatePen(wxGraphicsPenInfo(*wxBLACK, 1. / zoomLevel, wxPENSTYLE_SOLID));
            gc->SetPen(pen);
        }

        // Next, draw the handles
        for (std::vector<wxPoint>::size_type i = 0; i < n; ++i)
        {
            wxPoint2DDouble pt(NormalizedToUI(m_handles[i].pt));
            if (i == 0 && !m_handles[i].state)
                gc->SetBrush(*wxGREEN_BRUSH);
            else if (i == n - 1 && !m_handles[i].state)
                gc->SetBrush(*wxRED_BRUSH);
            else
                gc->SetBrush((m_handles[i].state) ? (*wxYELLOW_BRUSH)
                                                  : (isControlPoint(m_handles[i]) ? (*wxBLUE_BRUSH) : (*wxLIGHT_GREY_BRUSH)));

             gc->DrawEllipse(pt.m_x - 4.5 / zoomLevel, pt.m_y - 4.5 / zoomLevel, 9 / zoomLevel, 9 / zoomLevel);
        }
    }
}

void SketchPathDialog::OnSketchKeyDown(wxKeyEvent& event)
{
    int keycode = event.GetKeyCode();
    if (keycode == WXK_DELETE && (m_pathState == Undefined || m_pathState == DefineStartPoint)) {
        int pathIndex = m_pathsListBox->GetSelection();
        std::optional<std::pair<int,int>> toErase;
        int index = 0;
        for (auto iter = m_handles.begin(); iter != m_handles.end(); ++iter, ++index) {
            if (iter->state) {
                switch (iter->handlePointType) {
                case Point:
                    toErase = std::make_pair(index, 1);
                    break;
                case QuadraticControlPt:
                    toErase = std::make_pair(index, 2);
                    break;
                case QuadraticCurveEnd:
                    toErase = std::make_pair(index-1, 2);
                    break;
                case CubicControlPt1:
                    toErase = std::make_pair(index, 3);
                    break;
                case CubicControlPt2:
                    toErase = std::make_pair(index - 1, 3);
                    break;
                case CubicCurveEnd:
                    toErase = std::make_pair(index - 2, 3);
                    break;
                }
                break;
            }
        }
        if (toErase.has_value()) {
            auto startIter = m_handles.cbegin();
            auto endIter = m_handles.cbegin();
            auto p = toErase.value();
            std::advance(startIter, p.first);
            std::advance(endIter, p.second + p.first);
            m_handles.erase(startIter, endIter);
            if (m_handles.size() == 1)
                m_handles.clear();

            if (m_handles.empty()) {
                m_sketch.deletePath(pathIndex);
                PopulatePathListBoxFromSketch();
            } else
                UpdatePathFromHandles();
        }

        m_sketchPanel->Refresh();
    }
    else if (keycode == WXK_ESCAPE) {
        UpdatePathState(Undefined);
    } else if (keycode == WXK_SPACE) {
        m_pathClosed = true;
        UpdatePathState(Undefined);
    } else if (keycode == WXK_SHIFT) {
        switch (m_pathState) {
        case LineToNewPoint:
            m_pathState = QuadraticCurveToNewPoint;
            break;
        case QuadraticCurveToNewPoint:
            m_pathState = CubicCurveToNewPoint;
            break;
        case CubicCurveToNewPoint:
            m_pathState = LineToNewPoint;
            break;
        }
        m_sketchPanel->Refresh();
    }
}

void SketchPathDialog::OnSketchLeftDown(wxMouseEvent& event)
{
    wxAffineMatrix2D m;
    if (m_wheelRotation) {
        wxMatrix2D m2d(m_matrixComponents[0], m_matrixComponents[1],
                       m_matrixComponents[2], m_matrixComponents[3]);
        wxPoint2DDouble mt(m_matrixComponents[4], m_matrixComponents[5]);
        m.Set(m2d, mt);
        m.Invert();
    }
    wxPoint2DDouble ptUI(m.TransformPoint(event.GetPosition()));
    wxPoint2DDouble pt(UItoNormalized(ptUI));

    // Defining-new-segment stuff
    if (m_pathState == DefineStartPoint)
    {
        m_handles.push_back(pt);
        UpdatePathState(LineToNewPoint);
        return;
    }
    else if (m_pathState == LineToNewPoint) {
        m_handles.push_back(pt);
        m_sketchPanel->Refresh();
        return;
    } else if ( m_pathState == QuadraticCurveToNewPoint ) {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = pt;
        wxPoint2DDouble cp = 0.5 * startPt + 0.5 * endPt;

        m_handles.push_back(HandlePoint(cp, QuadraticControlPt));
        m_handles.push_back(HandlePoint(endPt, QuadraticCurveEnd));
        m_sketchPanel->Refresh();
        return;
    } else if (m_pathState == CubicCurveToNewPoint) {
        wxPoint2DDouble startPt = m_handles.back().pt;
        wxPoint2DDouble endPt = pt;
        wxPoint2DDouble cp1 = 0.75 * startPt + 0.25 * endPt;
        wxPoint2DDouble cp2 = 0.25 * startPt + 0.75 * endPt;

        m_handles.push_back(HandlePoint(cp1, CubicControlPt1));
        m_handles.push_back(HandlePoint(cp2, CubicControlPt2));
        m_handles.push_back(HandlePoint(endPt, CubicCurveEnd));
        m_sketchPanel->Refresh();
        return;
    }

    // Updating 'grabbed' handle
    for (std::vector<HandlePoint>::size_type i = 0; i < m_handles.size(); ++i) {
        if (m_handles[i].state) {
            m_grabbedHandleIndex = i;
            break;
        }
    }
}

void SketchPathDialog::OnSketchLeftUp(wxMouseEvent& /*event*/)
{
    if (m_grabbedHandleIndex != -1) {
        UpdatePathFromHandles(m_grabbedHandleIndex);
        // temp for debugging handles-path synchronization
        //UpdateHandlesForPath(m_pathsListView->GetFirstSelected());
    }

    m_grabbedHandleIndex = -1;
}

void SketchPathDialog::OnSketchMouseMove(wxMouseEvent& event)
{
    // handling drag of canvas as a special case for now...
    if ( event.ButtonIsDown(wxMOUSE_BTN_MIDDLE) ) {
        m_canvasTranslation += m_mousePos - event.GetPosition();
        m_mousePos = event.GetPosition();
        m_sketchPanel->Refresh();
        return;
    }

    m_mousePos = event.GetPosition();

    if (m_pathState == LineToNewPoint || m_pathState == QuadraticCurveToNewPoint || m_pathState == CubicCurveToNewPoint)
    {
        m_sketchPanel->Refresh();
        return;
    }

    wxAffineMatrix2D m;
    if (m_wheelRotation) {
        wxMatrix2D m2d(m_matrixComponents[0], m_matrixComponents[1],
                       m_matrixComponents[2], m_matrixComponents[3]);
        wxPoint2DDouble mt(m_matrixComponents[4], m_matrixComponents[5]);
        m.Set(m2d, mt);
    }

    if ( m_grabbedHandleIndex != -1 )
    {
        m.Invert();
        m_handles[m_grabbedHandleIndex].pt = UItoNormalized(m.TransformPoint(m_mousePos));
        m_sketchPanel->Refresh();
    }
    else
    {
        bool somethingChanged = false;
        for (auto& handle : m_handles)
        {
            wxPoint2DDouble handlePos(NormalizedToUI(handle.pt));
            wxPoint2DDouble transformedHandlePos(m.TransformPoint(handlePos));
            bool state = m_mousePos.GetDistanceSquare(transformedHandlePos) <= 20.25;
            if (state != handle.state)
            {
                handle.state = state;
                somethingChanged = true;
            }
        }

        if (somethingChanged)
            m_sketchPanel->Refresh();
    }
}

void SketchPathDialog::OnSketchEntered(wxMouseEvent& /*event*/)
{
    m_sketchPanel->SetFocus();
}

void SketchPathDialog::OnSketchMouseWheel(wxMouseEvent& event)
{
    m_wheelRotation += event.GetWheelRotation();
    m_wheelRotation = std::clamp(m_wheelRotation, 0, MouseWheelLimit);
    if (!m_wheelRotation)
        m_canvasTranslation = wxPoint2DDouble();

    // todo? - take zoom and/or canvas translation into account
    m_normalizedZoomPt = UItoNormalized(event.GetPosition());

    m_sketchPanel->Refresh();
}

void SketchPathDialog::OnSketchMidDown(wxMouseEvent& event)
{
    m_mousePos = event.GetPosition();
}

void SketchPathDialog::OnFilePickerCtrl_FileChanged(wxCommandEvent& /*event*/)
{
    UpdateBgBitmap(ImageUpdate);
}

void SketchPathDialog::OnSlider_BgAlphaChanged(wxCommandEvent& event)
{
    m_bitmapAlpha = static_cast<unsigned char>(m_bgAlphaSlider->GetValue());
    UpdateBgBitmap(AlphaUpdate);
}

void SketchPathDialog::OnButton_StartPath(wxCommandEvent& /*event*/)
{
    ResetHandlesState(DefineStartPoint);

    m_pathsListBox->DeselectAll();
}

void SketchPathDialog::OnButton_EndPath(wxCommandEvent& /*event*/)
{
    UpdatePathState(Undefined);
}

void SketchPathDialog::OnButton_ClosePath(wxCommandEvent& /*event*/)
{
    m_pathClosed = true;
    UpdatePathState(Undefined);
}

void SketchPathDialog::OnButton_ClearSketch(wxCommandEvent& /*event*/)
{
    m_sketch = SketchEffectSketch();

    m_pathsListBox->Clear();

    ResetHandlesState();
}

void SketchPathDialog::OnListBox_PathSelected(wxCommandEvent& /*event*/)
{
    UpdateHandlesForPath( m_pathsListBox->GetSelection() );
}

void SketchPathDialog::OnListBox_ContextMenu(wxContextMenuEvent& event)
{
    wxPoint pt(m_pathsListBox->ScreenToClient(event.GetPosition()));
    m_pathIndexToDelete = m_pathsListBox->HitTest(pt);
    if (m_pathIndexToDelete < 0)
        return;

    wxString str;
    str.sprintf("Delete Path %d", 1 + m_pathIndexToDelete);

    wxMenu mnu;
    mnu.Append(ID_MENU_Delete, str);
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&SketchPathDialog::OnPopupCommand, nullptr, this);
    PopupMenu(&mnu);
}

void SketchPathDialog::OnPopupCommand(wxCommandEvent& event)
{
    if (event.GetId() == ID_MENU_Delete && m_pathIndexToDelete < m_sketch.pathCount()) {
        m_sketch.deletePath(m_pathIndexToDelete);

        ResetHandlesState();

        PopulatePathListBoxFromSketch();
    }
}

void SketchPathDialog::OnButton_Ok(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_OK);
}

void SketchPathDialog::OnButton_Cancel(wxCommandEvent& /*event*/)
{
    EndDialog(wxID_CANCEL);
}

// Usable area rect: BorderWidth+1, BorderWidth+1, sz.GetWidth()-2*BorderWidth-2, sz.GetHeight()-2*BorderWidth-2;

wxPoint2DDouble SketchPathDialog::UItoNormalized(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(m_sketchPanel->GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = double(pt.m_x - o.x) / sz.GetWidth();
    double y = double(pt.m_y - o.y) / sz.GetHeight();

    return wxPoint2DDouble(x, 1. - y);
}

wxPoint2DDouble SketchPathDialog::NormalizedToUI(const wxPoint2DDouble& pt) const
{
    wxPoint o(BorderWidth + 1, BorderWidth + 1);
    wxSize sz(m_sketchPanel->GetSize() - wxSize(2 * BorderWidth - 2, 2 * BorderWidth - 2));

    double x = pt.m_x * sz.GetWidth();
    double y = pt.m_y * sz.GetHeight();
    return wxPoint2DDouble(o.x + x, o.y + (sz.GetHeight() - y - 1));
}

void SketchPathDialog::UpdateHandlesForPath(long pathIndex)
{
    ResetHandlesState();

    if (pathIndex < 0 || pathIndex >= m_sketch.paths().size())
        return;

    auto iter = m_sketch.paths().cbegin();
    std::advance(iter, pathIndex);

    auto pathSegments((*iter)->segments());
    m_handles.push_back(HandlePoint(pathSegments.front()->StartPoint()));
    for (auto iter = pathSegments.cbegin(); iter != pathSegments.cend(); ++iter) {
        std::shared_ptr<SketchPathSegment> pathSegment = *iter;

        std::shared_ptr<SketchQuadraticBezier> quadratic;
        std::shared_ptr<SketchCubicBezier> cubic;
        if (std::dynamic_pointer_cast<SketchLine>(pathSegment) != nullptr) {
            m_handles.push_back(HandlePoint(pathSegment->EndPoint()));
        } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(pathSegment)) != nullptr) {
            m_handles.push_back(HandlePoint(quadratic->ControlPoint(), QuadraticControlPt));
            m_handles.push_back(HandlePoint(quadratic->EndPoint(), QuadraticCurveEnd));
        } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(pathSegment)) != nullptr) {
            m_handles.push_back(HandlePoint(cubic->ControlPoint1(), CubicControlPt1));
            m_handles.push_back(HandlePoint(cubic->ControlPoint2(), CubicControlPt2));
            m_handles.push_back(HandlePoint(cubic->EndPoint(), CubicCurveEnd));
        }
    }

    if ((*iter)->isClosed() && m_handles.size() >= 3) {
        m_handles.pop_back();
        m_pathClosed = true;
    }

    m_sketchPanel->Refresh();
}

void SketchPathDialog::UpdatePathFromHandles()
{
    if (m_handles.size() < 2)
        return;

    auto pathIndex = m_pathsListBox->GetSelection();
    if (pathIndex < 0 || pathIndex >= m_sketch.pathCount())
        return;

    auto path = std::make_shared<SketchEffectPath>();
    auto startPt = m_handles.front().pt;
    for ( size_t i = 1; i < m_handles.size(); ) {
        std::shared_ptr<SketchPathSegment> segment;
        switch (m_handles[i].handlePointType) {
        case Point:
            segment = std::make_shared<SketchLine>(startPt, m_handles[i].pt);
            ++i;
            break;
        case QuadraticControlPt:
            segment = std::make_shared<SketchQuadraticBezier>(startPt, m_handles[i].pt, m_handles[i + 1].pt);
            i += 2;
            break;
        case CubicControlPt1:
            segment = std::make_shared<SketchCubicBezier>(startPt, m_handles[i].pt, m_handles[i + 1].pt, m_handles[i + 2].pt);
            i += 3;
            break;
        default: {
            int x = 1;
        }
        }
        path->appendSegment(segment);
        startPt = segment->EndPoint();
    }

    m_sketch.updatePath(pathIndex, path);
}

void SketchPathDialog::UpdatePathFromHandles(long handleIndex)
{
    auto paths(m_sketch.paths());
    auto pathIndex = m_pathsListBox->GetSelection();
    if (pathIndex < 0 || pathIndex >= paths.size())
        return;

    auto iter = paths.cbegin();
    std::advance(iter, pathIndex);

    auto segments = (*iter)->segments();
    if (segments.empty())
        return;

    int index = 0;
    auto normalizedHandlePt(m_handles[handleIndex].pt);

    // Can early-return when adjusting the start-point of the path
    if (handleIndex == index++) {
        segments.front()->SetStartPoint(normalizedHandlePt);

        // yuck... closed paths have an extra SketchLine to close the path, so if we update
        //         the start point, we need to update that last 'extra' segment
        if ((*iter)->isClosed())
            segments.back()->SetEndPoint(normalizedHandlePt);
        return;
    }

    for (int segmentIndex = 0; segmentIndex < segments.size(); ++segmentIndex) {
        std::shared_ptr<SketchPathSegment> segment = segments[segmentIndex];
        std::shared_ptr<SketchQuadraticBezier> quadratic;
        std::shared_ptr<SketchCubicBezier> cubic;


        if (std::dynamic_pointer_cast<SketchLine>(segment) != nullptr) {
            if (handleIndex == index++) {
                segment->SetEndPoint(normalizedHandlePt);
                if (segmentIndex < segments.size() - 1)
                    segments[segmentIndex + 1]->SetStartPoint(normalizedHandlePt);
                break;
            }
        } else if ((quadratic = std::dynamic_pointer_cast<SketchQuadraticBezier>(segment)) != nullptr) {
            if (handleIndex == index++) {
                quadratic->SetControlPoint(normalizedHandlePt);
                break;
            }
            if (handleIndex == index++) {
                segment->SetEndPoint(normalizedHandlePt);
                if (segmentIndex < segments.size() - 1)
                    segments[segmentIndex + 1]->SetStartPoint(normalizedHandlePt);
                break;
            }
        } else if ((cubic = std::dynamic_pointer_cast<SketchCubicBezier>(segment)) != nullptr) {
            if (handleIndex == index++) {
                cubic->SetControlPoint1(normalizedHandlePt);
                break;
            }
            if (handleIndex == index++) {
                cubic->SetControlPoint2(normalizedHandlePt);
                break;
            }
            if (handleIndex == index++) {
                segment->SetEndPoint(normalizedHandlePt);
                if (segmentIndex < segments.size() - 1)
                    segments[segmentIndex + 1]->SetStartPoint(normalizedHandlePt);
                break;
            }
        }
    }
}

void SketchPathDialog::UpdatePathState(PathState state)
{
    m_pathState = state;

    switch ( m_pathState )
    {
    case Undefined:
        m_startPathBtn->Enable();
        m_endPathBtn->Disable();
        m_closePathBtn->Disable();
        break;
    case DefineStartPoint:
        m_startPathBtn->Disable();
        m_endPathBtn->Disable();
        m_closePathBtn->Disable();
        break;
    case LineToNewPoint:
    case QuadraticCurveToNewPoint:
    case CubicCurveToNewPoint:
        m_startPathBtn->Disable();
        m_endPathBtn->Enable();
        m_closePathBtn->Enable();
        break;
    }

    m_sketchPanel->Refresh();

    // If we're Undefined, have some handles, and no path
    // selected, I think we've added a new one!!
    if (m_pathState == Undefined && !m_handles.empty() && m_pathsListBox->GetSelection() < 0) {
        auto path = CreatePathFromHandles();
        if (path != nullptr) {
            m_sketch.appendPath(path);
            int n = static_cast<int>(m_sketch.pathCount());
            wxString str;
            str.sprintf("Path %d", n);
            m_pathsListBox->Insert(str, n - 1);
            m_pathsListBox->Select(n - 1);
        }
    }
}

bool SketchPathDialog::isControlPoint(const HandlePoint& handlePt)
{
    auto hpt = handlePt.handlePointType;
    return hpt == QuadraticControlPt || hpt == CubicControlPt1 || hpt == CubicControlPt2;
}

std::shared_ptr<SketchEffectPath> SketchPathDialog::CreatePathFromHandles() const
{
    if (m_handles.size() < 2)
        return nullptr;

    auto path = std::make_shared<SketchEffectPath>();
    for (size_t index = 0; index < m_handles.size() - 1;) {
        std::shared_ptr<SketchPathSegment> segment;

        switch (m_handles[index + 1].handlePointType) {
        case Point:
            segment = std::make_shared<SketchLine>(m_handles[index].pt, m_handles[index + 1].pt);
            ++index;
            break;
        case QuadraticControlPt:
            segment = std::make_shared<SketchQuadraticBezier>(m_handles[index].pt,
                                                              m_handles[index + 1].pt,
                                                              m_handles[index + 2].pt);
            index += 2;
            break;
        case CubicControlPt1:
            segment = std::make_shared<SketchCubicBezier>(m_handles[index].pt,
                                                          m_handles[index + 1].pt,
                                                          m_handles[index + 2].pt,
                                                          m_handles[index + 3].pt);
            index += 3;
            break;
        }

        if (segment != nullptr)
            path->appendSegment(segment);
    }
    if (m_pathClosed)
        path->closePath();

    return path;
}

void SketchPathDialog::ResetHandlesState(PathState state /*Undefined*/)
{
    m_handles.clear();
    m_grabbedHandleIndex = -1;
    m_pathClosed = false;
    UpdatePathState(state);
}

void SketchPathDialog::PopulatePathListBoxFromSketch()
{
    m_pathsListBox->Clear();

    int i = 0;
    for (const auto& path : m_sketch.paths()) {
        wxString text;
        text.sprintf("Path %d", i + 1);
        m_pathsListBox->Insert(text, i++);
    }
}

void SketchPathDialog::UpdateBgBitmap(BgUpdateType updateType)
{
    if (updateType == ImageUpdate) {
        wxImage img(m_filePicker->GetFileName().GetFullPath());
        if (img.IsOk()) {
            img.InitAlpha();
            m_bgImage = img;
        }
    }

    int w = m_bgImage.GetWidth();
    int h = m_bgImage.GetHeight();
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
            m_bgImage.SetAlpha(x, y, m_bitmapAlpha);

    m_bgBitmap.reset(new wxBitmap(m_bgImage));

    m_sketchPanel->Refresh();
}
