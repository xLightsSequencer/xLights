/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "xlGridCanvasMorph.h"
#include "../../BitmapCache.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "../../ValueCurveButton.h"

#include <log4cpp/Category.hh>

BEGIN_EVENT_TABLE(xlGridCanvasMorph, xlGridCanvas)
    EVT_PAINT(xlGridCanvasMorph::render)
    EVT_MOTION(xlGridCanvasMorph::mouseMoved)
    EVT_LEFT_DOWN(xlGridCanvasMorph::mouseLeftDown)
    EVT_LEFT_UP(xlGridCanvasMorph::mouseLeftUp)
    EVT_RIGHT_DOWN(xlGridCanvasMorph::mouseRightDown)
    EVT_RIGHT_UP(xlGridCanvasMorph::mouseRightUp)
    EVT_LEFT_DCLICK(xlGridCanvasMorph::mouseLeftDClick)
END_EVENT_TABLE()

#define CORNER_NOT_SELECTED     0
#define CORNER_1A_SELECTED      1
#define CORNER_1B_SELECTED      2
#define CORNER_2A_SELECTED      3
#define CORNER_2B_SELECTED      4
#define CORNER_ALL_SELECTED      5

xlGridCanvasMorph::xlGridCanvasMorph(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style, const wxString& name)
    : xlGridCanvas(parent, id, pos, size, style, name),
    x1a(0),
    x1b(0),
    x2a(0),
    x2b(0),
    y1a(0),
    y1b(0),
    y2a(0),
    y2b(0),
    mSelectedCorner(CORNER_NOT_SELECTED),
    mMorphStartLinked(false),
    mMorphEndLinked(false)
{
    for (int i = 0; i < 6; i++) {
        mCornerTextures[i] = nullptr;
    }
}

xlGridCanvasMorph::~xlGridCanvasMorph()
{
    for (int i = 0; i < 6; i++) {
        if (mCornerTextures[i]) {
            delete mCornerTextures[i];
        }
    }
}

void xlGridCanvasMorph::SetEffect(Effect* effect_)
{
    mEffect = effect_;
}

int xlGridCanvasMorph::CheckForCornerHit(int x, int y) const
{
    float mCellSize = getCellSize();
    float corner_size = std::max((float)mMinCornerSize, mCellSize - 2);
    float half = corner_size / 2;
    if (x > x1a - half && x < x1a + half && y > y1a - half && y < y1a + half) {
        return CORNER_1A_SELECTED;
    }
    if (x > x1b - half && x < x1b + half && y > y1b - half && y < y1b + half) {
        return CORNER_1B_SELECTED;
    }
    if (x > x2a - half && x < x2a + half && y > y2a - half && y < y2a + half) {
        return CORNER_2A_SELECTED;
    }
    if (x > x2b - half && x < x2b + half && y > y2b - half && y < y2b + half) {
        return CORNER_2B_SELECTED;
    }
    return CORNER_NOT_SELECTED;
}

void xlGridCanvasMorph::mouseLeftDown(wxMouseEvent& event)
{
    if (mEffect == nullptr) return;
    mSelectedCorner = CheckForCornerHit(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
    if (mSelectedCorner == CORNER_NOT_SELECTED) {
        if (CheckForInsideHit(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()))) {
            mSelectedCorner = CORNER_ALL_SELECTED;
        } else {
            return;
        }
        _startPoint = wxPoint(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
        _starta1 = wxPoint(x1a, y1a);
        _startb1 = wxPoint(x1b, y1b);
        _starta2 = wxPoint(x2a, y2a);
        _startb2 = wxPoint(x2b, y2b);
    }
    mDragging = true;
    CaptureMouse();
    SetCursor(wxCURSOR_HAND);
    render();
    SetUndoPoint();
}

int Cross(int x, int y, int x1, int y1, int x2, int y2)
{
    if (y < y1 && y < y2) return 0;
    if (y > y1 && y > y2) return 0;
    if (x > x1 && x > x2) return 0;
    if (x < x1 && x < x2) return 1;

    if (x1 == x2 || y2 == y1) return 1;

    double slope = static_cast<double>(y2 - y1) / static_cast<double>(x2 - x1);
    double b = static_cast<double>(y1) - slope * static_cast<double>(x1);
    int xx = (static_cast<double>(y) - b) / slope;
    if (xx >= x) return 1;
    return 0;
}

// Inside if there are an odd number of crosses
bool xlGridCanvasMorph::CheckForInsideHit(int x, int y) const
{
    int crossings = Cross(x, y, x1a, y1a, x1b, y1b) +
        Cross(x, y, x1b, y1b, x2b, y2b) +
        Cross(x, y, x2b, y2b, x2a, y2a) +
        Cross(x, y, x2a, y2a, x1a, y1a);
    return crossings % 2 != 0;
}

void xlGridCanvasMorph::mouseRightDown(wxMouseEvent& event)
{
    if (mEffect == nullptr) return;

    mSelectedCorner = CheckForCornerHit(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
    if (mSelectedCorner == CORNER_NOT_SELECTED) {
        mRightDragging = true;
        SetUndoPoint();
        if (event.ControlDown() || event.CmdDown()) {
            // dragging out end
            SetMorphCorner2a(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
            StoreUpdatedMorphPositions();
            SetMorphCorner2b(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
            //StoreUpdatedMorphPositions();
        } else {
            // dragging out start
            SetMorphCorner1a(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
            StoreUpdatedMorphPositions();
            SetMorphCorner1b(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
            //StoreUpdatedMorphPositions();
        }
        CaptureMouse();
        SetCursor(wxCURSOR_HAND);
        render();
    }
}

void xlGridCanvasMorph::SetUndoPoint() const
{
    if (mEffect == nullptr) return;
    auto& undoManager = xLightsApp::GetFrame()->GetSequenceElements().get_undo_mgr();
    undoManager.CreateUndoStep();
    undoManager.CaptureModifiedEffect(mEffect->GetParentEffectLayer()->GetParentElement()->GetName(),
        mEffect->GetParentEffectLayer()->GetIndex(), mEffect->GetID(), mEffect->GetSettingsAsString(), mEffect->GetPaletteAsString());
}

void xlGridCanvasMorph::mouseMoved(wxMouseEvent& event)
{
    if (mEffect == nullptr) return;

    float mCellSize = getCellSize();
    if (!mDragging && !mRightDragging) {
        if (CheckForCornerHit(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY())) == CORNER_NOT_SELECTED) {
            if (CheckForInsideHit(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()))) {
                if (event.ControlDown() || event.CmdDown()) {
                    if (event.ShiftDown()) {
                        SetCursor(wxCURSOR_SIZENS);
                    } else {
                        SetCursor(wxCURSOR_SIZEWE);
                    }
                } else {
                    SetCursor(wxCURSOR_SIZING);
                }
            } else {
                SetCursor(wxCURSOR_DEFAULT);
            }
        } else {
            SetCursor(wxCURSOR_HAND);
        }
    } else {
        if (mSelectedCorner == CORNER_ALL_SELECTED) {
            int dx = (mapLogicalToAbsolute(event.GetX()) - _startPoint.x) / mCellSize * mCellSize;
            int dy = (mapLogicalToAbsolute(event.GetY()) - _startPoint.y) / mCellSize * mCellSize;

            if (event.ControlDown() || event.CmdDown()) {
                if (event.ShiftDown()) {
                    // only move in y direction
                    dx = 0;
                } else {
                    // only move in x direction
                    dy = 0;
                }
            }

            x1a = _starta1.x + dx;
            if (x1a < 0) x1a = 0;
            if (x1a >= mColumns * mCellSize) x1a = mColumns * mCellSize;
            y1a = _starta1.y + dy;
            if (y1a < 0) y1a = 0;
            if (y1a >= mRows * mCellSize) y1a = mRows * mCellSize;

            x1b = _startb1.x + dx;
            if (x1b < 0) x1b = 0;
            if (x1b >= mColumns * mCellSize) x1b = mColumns * mCellSize;
            y1b = _startb1.y + dy;
            if (y1b < 0) y1b = 0;
            if (y1b >= mRows * mCellSize) y1b = mRows * mCellSize;

            x2a = _starta2.x + dx;
            if (x2a < 0) x2a = 0;
            if (x2a >= mColumns * mCellSize) x2a = mColumns * mCellSize;
            y2a = _starta2.y + dy;
            if (y2a < 0) y2a = 0;
            if (y2a >= mRows * mCellSize) y2a = mRows * mCellSize;

            x2b = _startb2.x + dx;
            if (x2b < 0) x2b = 0;
            if (x2b >= mColumns * mCellSize) x2b = mColumns * mCellSize;
            y2b = _startb2.y + dy;
            if (y2b < 0) y2b = 0;
            if (y2b >= mRows * mCellSize) y2b = mRows * mCellSize;
        } else {
            if (event.ControlDown() || event.CmdDown()) {
                if (event.ShiftDown()) {
                    // y only
                    UpdateSelectedMorphCorner(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()), false, true);
                } else {
                    // x only
                    UpdateSelectedMorphCorner(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()), true, false);
                }
            } else {
                UpdateSelectedMorphCorner(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
            }
            if (!mRightDragging) {
                StoreUpdatedMorphPositions();
            }
        }
        render();
    }

    SetTooltip(mapLogicalToAbsolute(event.GetX()), mapLogicalToAbsolute(event.GetY()));
}

void xlGridCanvasMorph::mouseLeftUp(wxMouseEvent& event)
{
    SetTooltip(-1, -1);
    if (mEffect == nullptr) return;
    if (mDragging) {
        StoreUpdatedMorphPositions();
        mSelectedCorner = CORNER_NOT_SELECTED;
        ReleaseMouse();
        mDragging = false;
    }
}

void xlGridCanvasMorph::CalcDistanceTo(float targetX, float targetY, float* distance)
{
    distance[0] = sqrt((x1a - targetX) * (x1a - targetX) + (y1a - targetY) * (y1a - targetY));
    distance[1] = sqrt((x2a - targetX) * (x2a - targetX) + (y2a - targetY) * (y2a - targetY));
    distance[2] = sqrt((x1b - targetX) * (x1b - targetX) + (y1b - targetY) * (y1b - targetY));
    distance[3] = sqrt((x2b - targetX) * (x2b - targetX) + (y2b - targetY) * (y2b - targetY));
}

void xlGridCanvasMorph::ProcessNearest(int targetX, int targetY, bool* done)
{
    float distanceTo[4];
    CalcDistanceTo(targetX, targetY, distanceTo);

    int least = -1;
    for (int i = 0; i < 4; i++) {
        if (least == -1 && done[i] == false) {
            least = i;
        } else if (done[i] == false) {
            if (distanceTo[i] < distanceTo[least]) {
                least = i;
            }
        }
    }
    switch (least) {
    case 0:
        x1a = targetX;
        y1a = targetY;
        break;
    case 1:
        x2a = targetX;
        y2a = targetY;
        break;
    case 2:
        x1b = targetX;
        y1b = targetY;
        break;
    case 3:
        x2b = targetX;
        y2b = targetY;
        break;
    }
    done[least] = true;
}

void xlGridCanvasMorph::mouseLeftDClick(wxMouseEvent& event)
{
    bool done[4] = { false,false,false,false };

    float mCellSize = getCellSize();
    ProcessNearest(0, 0, done);
    ProcessNearest(0, mRows * mCellSize, done);
    ProcessNearest(mColumns * mCellSize, 0, done);
    ProcessNearest(mColumns * mCellSize, mRows * mCellSize, done);
    mSelectedCorner = CORNER_ALL_SELECTED;
    StoreUpdatedMorphPositions();
    mSelectedCorner = CORNER_NOT_SELECTED;
    render();
}

void xlGridCanvasMorph::mouseRightUp(wxMouseEvent& event)
{
    SetTooltip(-1, -1);
    if (mEffect == nullptr) return;
    if (mRightDragging) {
        StoreUpdatedMorphPositions();
        mSelectedCorner = CORNER_NOT_SELECTED;
        ReleaseMouse();
        mRightDragging = false;
    }
}

void xlGridCanvasMorph::UpdateSelectedMorphCorner(int x_pos, int y_pos, bool updateX, bool updateY)
{
    float offset = mapLogicalToAbsolute(5.0);
    if (x_pos < offset) {
        x_pos = offset;
    }
    if (y_pos < offset) {
        y_pos = offset;
    }
    if (x_pos >= (mWindowWidth-offset)) {
        x_pos = mWindowWidth-offset - 1;
    }
    if (y_pos >= (mWindowHeight-offset)) {
        y_pos = mWindowHeight-offset - 1;
    }

    if (mSelectedCorner == CORNER_1A_SELECTED) {
        if (updateX) x1a = x_pos;
        if (updateY) y1a = y_pos;
        if (mMorphStartLinked) {
            if (updateX) x1b = x_pos;
            if (updateY) y1b = y_pos;
        }
    } else if (mSelectedCorner == CORNER_1B_SELECTED) {
        if (updateX) x1b = x_pos;
        if (updateY) y1b = y_pos;
    } else if (mSelectedCorner == CORNER_2A_SELECTED) {
        if (updateX) x2a = x_pos;
        if (updateY) y2a = y_pos;
        if (mMorphEndLinked) {
            if (updateX) x2b = x_pos;
            if (updateY) y2b = y_pos;
        }
    } else if (mSelectedCorner == CORNER_2B_SELECTED) {
        if (updateX) x2b = x_pos;
        if (updateY) y2b = y_pos;
    }
    render();
}

void xlGridCanvasMorph::SetMorphCorner1a(int x_pos, int y_pos)
{
    float offset = mapLogicalToAbsolute(5.0);
    if (x_pos < offset) {
        x_pos = offset;
    }
    if (y_pos < offset) {
        y_pos = offset;
    }
    if (x_pos >= (mWindowWidth-offset)) {
        x_pos = mWindowWidth-offset - 1;
    }
    if (y_pos >= (mWindowHeight-offset)) {
        y_pos = mWindowHeight-offset - 1;
    }

    x1a = x_pos;
    y1a = y_pos;
    if (mMorphStartLinked) {
        x1b = x_pos;
        y1b = y_pos;
    }
    mSelectedCorner = CORNER_1A_SELECTED;
}

void xlGridCanvasMorph::SetMorphCorner1b(int x_pos, int y_pos)
{
    float offset = mapLogicalToAbsolute(5.0);
    if (x_pos < offset) {
        x_pos = offset;
    }
    if (y_pos < offset) {
        y_pos = offset;
    }
    if (x_pos >= (mWindowWidth-offset)) {
        x_pos = mWindowWidth-offset - 1;
    }
    if (y_pos >= (mWindowHeight-offset)) {
        y_pos = mWindowHeight-offset - 1;
    }
    
    x1b = x_pos;
    y1b = y_pos;
    if (!mMorphStartLinked) {
        mSelectedCorner = CORNER_1B_SELECTED;
    }
}

void xlGridCanvasMorph::SetMorphCorner2a(int x_pos, int y_pos)
{
    float offset = mapLogicalToAbsolute(5.0);
    if (x_pos < offset) {
        x_pos = offset;
    }
    if (y_pos < offset) {
        y_pos = offset;
    }
    if (x_pos >= (mWindowWidth-offset)) {
        x_pos = mWindowWidth-offset - 1;
    }
    if (y_pos >= (mWindowHeight-offset)) {
        y_pos = mWindowHeight-offset - 1;
    }

    x2a = x_pos;
    y2a = y_pos;
    if (mMorphEndLinked) {
        x2b = x_pos;
        y2b = y_pos;
    }
    mSelectedCorner = CORNER_2A_SELECTED;
}

void xlGridCanvasMorph::SetMorphCorner2b(int x_pos, int y_pos)
{
    float offset = mapLogicalToAbsolute(5.0);
    if (x_pos < offset) {
        x_pos = offset;
    }
    if (y_pos < offset) {
        y_pos = offset;
    }
    if (x_pos >= (mWindowWidth-offset)) {
        x_pos = mWindowWidth-offset - 1;
    }
    if (y_pos >= (mWindowHeight-offset)) {
        y_pos = mWindowHeight-offset - 1;
    }
    x2b = x_pos;
    y2b = y_pos;
    if (!mMorphStartLinked) {
        mSelectedCorner = CORNER_2B_SELECTED;
    }
}


void xlGridCanvasMorph::SetTooltip(int x, int y)
{
    float mCellSize = getCellSize();
    float offset = mapLogicalToAbsolute(5.0);
    x -= offset;
    y -= offset;
    if (x < 0 || y < 0 || mCellSize == 0 || mRows == 0 || mColumns == 0) {
        UnsetToolTip();
        return;
    }
    if ((x > mColumns * mCellSize) || (y > mRows * mCellSize)) {
        UnsetToolTip();
        return;
    }

    x = ((float)x) / mCellSize * 100.0f / mColumns;
    y = 100 - (((float)y) / mCellSize * 100.0f / mRows);

    wxString tt = wxString::Format("%d%%, %d%%", x, y);
    SetToolTip(tt);
}

void DeactivateValueCurve(SettingsMap& settings, const std::string& setting)
{
    if (settings.Contains(setting)) {
        wxString s = wxString(settings[setting]);
        s.Replace("Active=TRUE", "Active=FALSE");
        settings[setting] = s;
    }
}

void xlGridCanvasMorph::StoreUpdatedMorphPositions()
{
    SettingsMap& settings = mEffect->GetSettings();
    if (mSelectedCorner == CORNER_1A_SELECTED || mSelectedCorner == CORNER_ALL_SELECTED) {
        settings["E_SLIDER_Morph_Start_X1"] = wxString::Format("%d", SetColumnCenter(x1a));
        settings["E_SLIDER_Morph_Start_Y1"] = wxString::Format("%d", SetRowCenter(y1a));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_X1");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_Y1");
    }
    if ((mSelectedCorner == CORNER_1B_SELECTED || mSelectedCorner == CORNER_ALL_SELECTED) && !mMorphStartLinked) {
        settings["E_SLIDER_Morph_Start_X2"] = wxString::Format("%d", SetColumnCenter(x1b));
        settings["E_SLIDER_Morph_Start_Y2"] = wxString::Format("%d", SetRowCenter(y1b));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_X2");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_Start_Y2");
    }
    if (mSelectedCorner == CORNER_2A_SELECTED || mSelectedCorner == CORNER_ALL_SELECTED) {
        settings["E_SLIDER_Morph_End_X1"] = wxString::Format("%d", SetColumnCenter(x2a));
        settings["E_SLIDER_Morph_End_Y1"] = wxString::Format("%d", SetRowCenter(y2a));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_X1");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_Y1");
    }
    if ((mSelectedCorner == CORNER_2B_SELECTED || mSelectedCorner == CORNER_ALL_SELECTED) && !mMorphEndLinked) {
        settings["E_SLIDER_Morph_End_X2"] = wxString::Format("%d", SetColumnCenter(x2b));
        settings["E_SLIDER_Morph_End_Y2"] = wxString::Format("%d", SetRowCenter(y2b));
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_X2");
        DeactivateValueCurve(settings, "E_VALUECURVE_Morph_End_Y2");
    }

    wxCommandEvent eventVCChange(EVT_VC_CHANGED);
    eventVCChange.SetEventObject(this);
    wxPostEvent(GetParent(), eventVCChange);

    wxCommandEvent eventEffectChanged(EVT_EFFECT_CHANGED);
    eventEffectChanged.SetClientData(mEffect);
    wxPostEvent(GetParent(), eventEffectChanged);
}

static std::vector<std::vector<wxImage>> CORNER_IMAGES;
void xlGridCanvasMorph::CreateCornerTextures(xlGraphicsContext *ctx)
{
    if (CORNER_IMAGES.empty()) {
        CORNER_IMAGES.resize(6);
        for (int x = 0; x < 6; x++) {
            CORNER_IMAGES[x].emplace_back(BitmapCache::GetCornerIcon(x, 64));
            CORNER_IMAGES[x].emplace_back(BitmapCache::GetCornerIcon(x, 32));
            CORNER_IMAGES[x].emplace_back(BitmapCache::GetCornerIcon(x, 16));
            CORNER_IMAGES[x].emplace_back(CORNER_IMAGES[x][2].Scale(8, 8, wxIMAGE_QUALITY_HIGH));
            CORNER_IMAGES[x].emplace_back(CORNER_IMAGES[x][2].Scale(4, 4, wxIMAGE_QUALITY_HIGH));
            CORNER_IMAGES[x].emplace_back(CORNER_IMAGES[x][2].Scale(2, 2, wxIMAGE_QUALITY_HIGH));
            CORNER_IMAGES[x].emplace_back(CORNER_IMAGES[x][2].Scale(1, 1, wxIMAGE_QUALITY_HIGH));
        }
    }
    for (int i = 0; i < 6; i++) {
        mCornerTextures[i] = ctx->createTextureMipMaps(CORNER_IMAGES[i]);
    }
}
void xlGridCanvasMorph::render(wxPaintEvent& event)
{
    wxPaintDC p(this);
    render();
}
void xlGridCanvasMorph::render()
{
    if(!IsShownOnScreen()) return;
    if(!mIsInitialized) {
        PrepareCanvas();
    }

    xlGraphicsContext *ctx = PrepareContextForDrawing();
    if (ctx == nullptr) {
        return;
    }
    ctx->SetViewport(0, 0, mWindowWidth, mWindowHeight);

    if (!mCornerTextures[0]) {
        CreateCornerTextures(ctx);
    }

    if (mEffect != nullptr) {
        DrawBaseGrid(ctx);
        DrawMorphEffect(ctx);
    }
    FinishDrawing(ctx);
}

void xlGridCanvasMorph::UpdateMorphPositionsFromEffect()
{
    wxString settings = mEffect->GetSettingsAsString();
    wxArrayString all_settings = wxSplit(settings, ',');
    for (int s = 0; s < all_settings.size(); s++) {
        wxArrayString parts = wxSplit(all_settings[s], '=');
        if (parts.size() > 1) {
            if (parts[0] == "E_SLIDER_Morph_Start_X1") {
                x1a = GetColumnCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_Start_X2") {
                x1b = GetColumnCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_Start_Y1") {
                y1a = GetRowCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_Start_Y2") {
                y1b = GetRowCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_End_X1") {
                x2a = GetColumnCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_End_X2") {
                x2b = GetColumnCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_End_Y1") {
                y2a = GetRowCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_SLIDER_Morph_End_Y2") {
                y2b = GetRowCenter(wxAtoi(parts[1]));
            } else if (parts[0] == "E_CHECKBOX_Morph_Start_Link") {
                mMorphStartLinked = wxAtoi(parts[1]) > 0;
            } else if (parts[0] == "E_CHECKBOX_Morph_End_Link") {
                mMorphEndLinked = wxAtoi(parts[1]) > 0;
            }
        }
    }
    if (mMorphStartLinked) {
        x1b = x1a;
        y1b = y1a;
    }
    if (mMorphEndLinked) {
        x2b = x2a;
        y2b = y2a;
    }
}

void xlGridCanvasMorph::DrawMorphEffect(xlGraphicsContext *ctx)
{
    if (!mDragging && !mRightDragging) {
        UpdateMorphPositionsFromEffect();
    }
    xlColor yellowLine = xlYELLOW;
    xlColor redLine = xlRED;
    xlVertexColorAccumulator *va = ctx->createVertexColorAccumulator();
    va->PreAlloc(8);

    if (!mMorphStartLinked) {
        va->AddVertex(x1a, y1a, 0, redLine);
        va->AddVertex(x1b, y1b, 0, redLine);
    }
    if (!mMorphEndLinked) {
        va->AddVertex(x2a, y2a, 0, redLine);
        va->AddVertex(x2b, y2b, 0, redLine);
    }
    va->AddVertex(x1a, y1a, 0, yellowLine);
    va->AddVertex(x2a, y2a, 0, yellowLine);
    va->AddVertex(x1b, y1b, 0, yellowLine);
    va->AddVertex(x2b, y2b, 0, yellowLine);
    ctx->drawLines(va);
    delete va;

    // draw the corners
    ctx->enableBlending();
    int cellSize = getCellSize();
    int corner_size = std::max(mMinCornerSize, cellSize - 2);
    if (mMorphEndLinked) {
        ctx->drawTexture(mCornerTextures[5], x2a - corner_size / 2, y2a - corner_size / 2, x2a + corner_size / 2, y2a + corner_size / 2);
    } else {
        ctx->drawTexture(mCornerTextures[3], x2b - corner_size / 2, y2b - corner_size / 2, x2b + corner_size / 2, y2b + corner_size / 2);
        ctx->drawTexture(mCornerTextures[2], x2a - corner_size / 2, y2a - corner_size / 2, x2a + corner_size / 2, y2a + corner_size / 2);
    }
    if (mMorphStartLinked) {
        ctx->drawTexture(mCornerTextures[4], x1a - corner_size / 2, y1a - corner_size / 2, x1a + corner_size / 2, y1a + corner_size / 2);
    } else {
        ctx->drawTexture(mCornerTextures[1], x1b - corner_size / 2, y1b - corner_size / 2, x1b + corner_size / 2, y1b + corner_size / 2);
        ctx->drawTexture(mCornerTextures[0], x1a - corner_size / 2, y1a - corner_size / 2, x1a + corner_size / 2, y1a + corner_size / 2);
    }
    ctx->disableBlending();
}
