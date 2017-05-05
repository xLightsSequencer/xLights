#include "ModelScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "../ModelPreview.h"
#include "../DrawGLUtils.h"

#define SNAP_RANGE                  5
#define RECT_HANDLE_WIDTH           6
#define BOUNDING_RECT_OFFSET        8

static inline void TranslatePointDoubles(float radians,float x, float y,float &x1, float &y1) {
    float s = sin(radians);
    float c = cos(radians);
    x1 = c*x-(s*y);
    y1 = s*x+(c*y);
}

static inline float toRadians(long degrees) {
    return 2.0*M_PI*float(degrees)/360.0f;
}

static inline int toDegrees(float radians) {
    return (radians/(2*M_PI))*360.0;
}

static wxCursor GetResizeCursor(int cornerIndex, int PreviewRotation) {
    int angleState;
    //LeftTop and RightBottom
    switch(cornerIndex) {
            // Left top when PreviewRotation = 0
        case 0:
            angleState = (int)(PreviewRotation/22.5);
            break;
            // Right Top
        case 1:
            angleState = ((int)(PreviewRotation/22.5)+4)%16;
            break;
            // Right Bottom
        case 2:
            angleState = ((int)(PreviewRotation/22.5)+8)%16;
            break;
            // Right Bottom
        default:
            angleState = ((int)(PreviewRotation/22.5)+12)%16;
            break;
    }
    switch(angleState) {
        case 0:
            return wxCURSOR_SIZENWSE;
        case 1:
            return wxCURSOR_SIZEWE;
        case 2:
            return wxCURSOR_SIZEWE;
        case 3:
            return wxCURSOR_SIZENESW;
        case 4:
            return wxCURSOR_SIZENESW;
        case 5:
            return wxCURSOR_SIZENS;
        case 6:
            return wxCURSOR_SIZENS;
        case 7:
            return wxCURSOR_SIZENWSE;
        case 8:
            return wxCURSOR_SIZENWSE;
        case 9:
            return wxCURSOR_SIZEWE;
        case 10:
            return wxCURSOR_SIZEWE;
        case 11:
            return wxCURSOR_SIZENESW;
        case 12:
            return wxCURSOR_SIZENESW;
        case 13:
            return wxCURSOR_SIZENS;
        case 14:
            return wxCURSOR_SIZENS;
        default:
            return wxCURSOR_SIZENWSE;
    }

}

ModelScreenLocation::ModelScreenLocation(int sz) : RenderWi(0), RenderHt(0), previewW(800), previewH(600), mHandlePosition(sz) {
}


BoxedScreenLocation::BoxedScreenLocation() : ModelScreenLocation(5),
    offsetXpct(0.5f), offsetYpct(0.5f), singleScale(false),
    PreviewScaleX(0.333f), PreviewScaleY(0.333f),
    PreviewRotation(0) {
}
void BoxedScreenLocation::Read(wxXmlNode *ModelNode) {
    offsetXpct = wxAtof(ModelNode->GetAttribute("offsetXpct","0"));
    if(offsetXpct<0 || offsetXpct>1) {
        offsetXpct = 0.5f;
    }
    offsetYpct = wxAtof(ModelNode->GetAttribute("offsetYpct","0"));
    if(offsetYpct<0 || offsetYpct>1) {
        offsetYpct = 0.5f;
    }
    wxString tempstr = ModelNode->GetAttribute("PreviewScale");
    singleScale = false;
    if (tempstr == "") {
        PreviewScaleX = wxAtof(ModelNode->GetAttribute("PreviewScaleX", "0.3333"));
        PreviewScaleY = wxAtof(ModelNode->GetAttribute("PreviewScaleY", "0.3333"));
    } else {
        singleScale = true;
        PreviewScaleX = PreviewScaleY = wxAtof(tempstr);
    }
    if(PreviewScaleX<0) {
        PreviewScaleX = 0.33f;
    }
    if(PreviewScaleY<0) {
        PreviewScaleY = 0.33f;
    }
    PreviewRotation=wxAtoi(ModelNode->GetAttribute("PreviewRotation","0"));
}
void BoxedScreenLocation::Write(wxXmlNode *ModelXml) {
    ModelXml->DeleteAttribute("offsetXpct");
    ModelXml->DeleteAttribute("offsetYpct");
    ModelXml->DeleteAttribute("PreviewScale");
    ModelXml->DeleteAttribute("PreviewScaleX");
    ModelXml->DeleteAttribute("PreviewScaleY");
    ModelXml->DeleteAttribute("PreviewRotation");
    ModelXml->AddAttribute("offsetXpct", wxString::Format("%6.4f",offsetXpct));
    ModelXml->AddAttribute("offsetYpct", wxString::Format("%6.4f",offsetYpct));
    if (singleScale) {
        ModelXml->AddAttribute("PreviewScale", wxString::Format("%6.4f",PreviewScaleX));
    } else {
        ModelXml->AddAttribute("PreviewScaleX", wxString::Format("%6.4f",PreviewScaleX));
        ModelXml->AddAttribute("PreviewScaleY", wxString::Format("%6.4f",PreviewScaleY));
    }
    ModelXml->AddAttribute("PreviewRotation", wxString::Format("%d",PreviewRotation));
}


void BoxedScreenLocation::TranslatePoint(float &sx, float &sy) const {
    sx = (sx*scalex);
    sy = (sy*scaley);
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx += centerx;
    sy += centery;
}

bool BoxedScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
    int xs = x1<x2?x1:x2;
    int xf = x1>x2?x1:x2;
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;

    if (mMinScreenX>=xs && mMaxScreenX<=xf && mMinScreenY>=ys && mMaxScreenY<=yf) {
        return true;
    } else {
        return false;
    }
}

bool BoxedScreenLocation::HitTest(int x, int y) const {
    if (x>=mMinScreenX && x<=mMaxScreenX && y>=mMinScreenY && y <= mMaxScreenY) {
        return true;
    } else {
        return false;
    }
}

wxCursor BoxedScreenLocation::CheckIfOverHandles(int &handle, wxCoord x,wxCoord y) const {
    if (x>mHandlePosition[0].x && x<mHandlePosition[0].x+RECT_HANDLE_WIDTH &&
        y>mHandlePosition[0].y && y<mHandlePosition[0].y+RECT_HANDLE_WIDTH) {
        handle = OVER_L_TOP_HANDLE;
        return GetResizeCursor(0, PreviewRotation);
    } else if (x>mHandlePosition[1].x && x<mHandlePosition[1].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[1].y && y<mHandlePosition[1].y+RECT_HANDLE_WIDTH) {
        handle = OVER_R_TOP_HANDLE;
        return GetResizeCursor(1, PreviewRotation);
    } else if (x>mHandlePosition[2].x && x<mHandlePosition[2].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[2].y && y<mHandlePosition[2].y+RECT_HANDLE_WIDTH) {
        handle = OVER_R_BOTTOM_HANDLE;
        return GetResizeCursor(2, PreviewRotation);
    } else if (x>mHandlePosition[3].x && x<mHandlePosition[3].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[3].y && y<mHandlePosition[3].y+RECT_HANDLE_WIDTH) {
        handle = OVER_R_BOTTOM_HANDLE;
        return GetResizeCursor(3, PreviewRotation);
    } else if (x>mHandlePosition[4].x && x<mHandlePosition[4].x+RECT_HANDLE_WIDTH &&
               y>mHandlePosition[4].y && y<mHandlePosition[4].y+RECT_HANDLE_WIDTH) {
        handle = OVER_ROTATE_HANDLE;
        return wxCURSOR_HAND;
    } else {
        handle = OVER_NO_HANDLE;
    }
    return wxCURSOR_DEFAULT;
}
wxCursor BoxedScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) {
    offsetXpct = (float)x/(float)previewW;
    offsetYpct = (float)y/(float)previewH;
    SetPreviewSize(previewW, previewH, Nodes);
    handle = OVER_R_BOTTOM_HANDLE;
    return wxCURSOR_SIZING;
}


void BoxedScreenLocation::PrepareToDraw() const {
    radians = toRadians(PreviewRotation);
    scalex = (float)previewW / (float)RenderWi * PreviewScaleX;
    scaley = (float)previewH / (float)RenderHt * PreviewScaleY;
    centerx = int(offsetXpct*(float)previewW);
    centery = int(offsetYpct*(float)previewH);
}

void BoxedScreenLocation::SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) {
    previewW = w;
    previewH = h;

    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = float(RenderWi) * float(previewH) / (float(previewW) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = float(RenderHt) * float(previewW) / (float(previewH) * RenderWi) * PreviewScaleX;
        }
    }
    PrepareToDraw();

    float sx,sy;
    mMinScreenX = w;
    mMinScreenY = h;
    mMaxScreenX = 0;
    mMaxScreenY = 0;
    for (auto it = Nodes.begin(); it != Nodes.end(); it++) {
        for (auto coord = it->get()->Coords.begin(); coord != it->get()->Coords.end(); coord++) {
            // draw node on screen
            sx=coord->screenX;
            sy=coord->screenY;

            TranslatePoint(sx, sy);

            if (sx<mMinScreenX) {
                mMinScreenX = sx;
            }
            if (sx>mMaxScreenX) {
                mMaxScreenX = sx;
            }
            if (sy<mMinScreenY) {
                mMinScreenY = sy;
            }
            if (sy>mMaxScreenY) {
                mMaxScreenY = sy;
            }
        }
    }
    // Set minimum bounding rectangle
    if(mMaxScreenY-mMinScreenY<4) {
        mMaxScreenY+=2;
        mMinScreenY-=2;
    }
    if(mMaxScreenX-mMinScreenX<4) {
        mMaxScreenX+=2;
        mMinScreenX-=2;
    }
}


void BoxedScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {
    va.PreAlloc(6 * 5);

    float w1 = centerx;
    float h1 = centery;

    float sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    float sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);

    mHandlePosition[0].x = sx;
    mHandlePosition[0].y = sy;
    // Upper Right Handle
    sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
    sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
    mHandlePosition[1].x = sx;
    mHandlePosition[1].y = sy;
    // Lower Right Handle
    sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
    sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
    mHandlePosition[2].x = sx;
    mHandlePosition[2].y = sy;
    // Lower Left Handle
    sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
    mHandlePosition[3].x = sx;
    mHandlePosition[3].y = sy;

    // Draw rotation handle square
    sx = -RECT_HANDLE_WIDTH/2;
    sy = ((RenderHt*scaley/2) + 50);
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx += w1;
    sy += h1;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
    // Save rotate handle
    mHandlePosition[4].x = sx;
    mHandlePosition[4].y = sy;
    // Draw rotation handle from center to 25 over rendered height
    sx = 0;
    sy = ((RenderHt*scaley/2) + 50);
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx += w1;
    sy += h1;

    va.Finish(GL_TRIANGLES);

    LOG_GL_ERRORV(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));
    va.AddVertex(w1,h1, xlWHITE);
    va.AddVertex(sx, sy, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
}

void BoxedScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxFloatProperty("X (%)", "ModelX", offsetXpct * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Y (%)", "ModelY", offsetYpct * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Width", "ModelWidth", PreviewScaleX * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Height", "ModelHeight", PreviewScaleY * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxIntProperty("Rotation", "ModelRotation", PreviewRotation));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetEditor("SpinCtrl");
}
int BoxedScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if ("ModelRotation" == name) {
        PreviewRotation = event.GetValue().GetInteger();
    } else if ("ModelWidth" == name) {
        PreviewScaleX = event.GetValue().GetDouble() / 100.0;
        return 3;
    } else if ("ModelHeight" == name) {
        PreviewScaleY = event.GetValue().GetDouble() / 100.0;
        return 3;
    } else if ("ModelX" == name) {
        offsetXpct = event.GetValue().GetDouble() / 100.0f;
        return 3;
    } else if ("ModelY" == name) {
        offsetYpct = event.GetValue().GetDouble() / 100.0f;
        return 3;
    }
    return 0;
}
int BoxedScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX,int mouseY) {
    if (handle == OVER_ROTATE_HANDLE) {
        int sx,sy;
        sx = mouseX-centerx;
        sy = mouseY-centery;
        //Calculate angle of mouse from center.
        float tan = (float)sx/(float)sy;
        int angle = -toDegrees((float)atan(tan));
        if(sy>=0) {
            PreviewRotation = angle;
        } else if (sx<=0) {
            PreviewRotation = 90+(90+angle);
        } else {
            PreviewRotation = -90-(90-angle);
        }
        if(ShiftKeyPressed) {
            PreviewRotation = (int)(PreviewRotation/5) * 5;
        }
    } else {
        // Get mouse point in model space/ not screen space
        float sx,sy;
        sx = float(mouseX)-centerx;
        sy = float(mouseY)-centery;
        float radians=-toRadians(PreviewRotation); // negative angle to reverse translation
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = fabs(sx) - RECT_HANDLE_WIDTH;
        sy = fabs(sy) - RECT_HANDLE_WIDTH;
        SetScale( (float)(sx*2.0)/float(previewW), (float)(sy*2.0)/float(previewH));
    }
    return 0;
}

int BoxedScreenLocation::GetTop() const {
    return centery+(RenderHt*scaley/2);
}
int BoxedScreenLocation::GetLeft() const {
    return centerx-(RenderWi*scalex/2);
}
int BoxedScreenLocation::GetRight() const {
    return centerx+(RenderWi*scalex/2);
}
int BoxedScreenLocation::GetBottom() const {
    return centery-(RenderHt*scaley/2);
}

int BoxedScreenLocation::GetMWidth() const {
    return previewW*PreviewScaleX;
}

int BoxedScreenLocation::GetMHeight() const {
    return previewH*PreviewScaleY;
}

void BoxedScreenLocation::SetMWidth(int w)
{
    PreviewScaleX = (float)w / (float)previewW;
}

void BoxedScreenLocation::SetMHeight(int h)
{
    PreviewScaleY = (float)h / (float)previewH;
}

void BoxedScreenLocation::SetLeft(int x) {
    float screenCenterX = previewW*offsetXpct;
    float newCenterX = screenCenterX + (x-mMinScreenX);
    offsetXpct = newCenterX/(float)previewW;
}
void BoxedScreenLocation::SetRight(int i) {
    float screenCenterX = previewW * offsetXpct;
    float newCenterX = screenCenterX + (i-mMaxScreenX);
    offsetXpct = newCenterX/(float)previewW;
}
void BoxedScreenLocation::SetTop(int y) {
    float screenCenterY = previewH*offsetYpct;
    float newCenterY = screenCenterY + (y-mMaxScreenY);
    offsetYpct = ((float)newCenterY/(float)previewH);
}
void BoxedScreenLocation::SetBottom(int y) {
    float screenCenterY = previewH*offsetYpct;
    float newCenterY = screenCenterY + (y-mMinScreenY);
    offsetYpct = ((float)newCenterY/(float)previewH);
}

TwoPointScreenLocation::TwoPointScreenLocation() : ModelScreenLocation(2),
    x1(0.4f), y1(0.4f), x2(0.6f), y2(0.6f),
    old(nullptr), matrix(nullptr), minMaxSet(false) {
}
TwoPointScreenLocation::~TwoPointScreenLocation() {
    if (matrix != nullptr) {
        delete matrix;
    }
}
void TwoPointScreenLocation::Read(wxXmlNode *ModelNode) {
    if (!ModelNode->HasAttribute("X1") && ModelNode->HasAttribute("offsetXpct")) {
        old = ModelNode;
    } else {
        x1 = wxAtof(ModelNode->GetAttribute("X1", ".4"));
        x2 = wxAtof(ModelNode->GetAttribute("X2", ".6"));
        y1 = wxAtof(ModelNode->GetAttribute("Y1", ".4"));
        y2 = wxAtof(ModelNode->GetAttribute("Y2", ".6"));
    }
}
void TwoPointScreenLocation::Write(wxXmlNode *node) {
    node->DeleteAttribute("X1");
    node->DeleteAttribute("Y1");
    node->DeleteAttribute("X2");
    node->DeleteAttribute("Y2");
    node->AddAttribute("X1", std::to_string(x1));
    node->AddAttribute("Y1", std::to_string(y1));
    node->AddAttribute("X2", std::to_string(x2));
    node->AddAttribute("Y2", std::to_string(y2));
}
#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>

void TwoPointScreenLocation::PrepareToDraw() const {
    float x1p = x1 * (float)previewW;
    float x2p = x2 * (float)previewW;
    float y1p = y1 * (float)previewH;
    float y2p = y2 * (float)previewH;

    float angle = (float)M_PI/2.0f;
    if (x2 != x1) {
        float slope = (y2p - y1p)/(x2p - x1p);
        angle = std::atan(slope);
        if (x1 > x2) {
            angle += (float)M_PI;
        }
    } else if (y2 < y1) {
        angle += (float)M_PI;
    }
    float scale = std::sqrt((y2p - y1p)*(y2p - y1p) + (x2p - x1p)*(x2p - x1p));
    scale /= RenderWi;

    glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, scale * GetVScaleFactor()));
    glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
    glm::mat3 shearMatrix = glm::shearY(glm::mat3(1.0f), GetYShear());
    glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1*previewW, y1*previewH));
    glm::mat3 mat3 = translateMatrix * rotationMatrix * shearMatrix * scalingMatrix;

    if (matrix != nullptr) {
        delete matrix;
    }
    matrix = new glm::mat3(mat3);
}
void TwoPointScreenLocation::TranslatePoint(float &x, float &y) const {
    glm::vec3 v = *matrix * glm::vec3(x, y, 1);
    x = v.x;
    y = v.y;
}

bool TwoPointScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
    float min = ymin;
    float max = ymax;
    if (!minMaxSet) {
        min = 0;
        max = RenderHt;
    }
    //invert the matrix, get into render space
    glm::vec3 v1 = *matrix * glm::vec3(0, min, 1);
    glm::vec3 v2 = *matrix * glm::vec3(0, max, 1);
    glm::vec3 v3 = *matrix * glm::vec3(RenderWi, min, 1);
    glm::vec3 v4 = *matrix * glm::vec3(RenderWi, max, 1);

    int xsi = x1<x2?x1:x2;
    int xfi = x1>x2?x1:x2;
    int ysi = y1<y2?y1:y2;
    int yfi = y1>y2?y1:y2;

    float xs = std::min(std::min(v1.x, v2.x), std::min(v3.x, v4.x));
    float xf = std::max(std::max(v1.x, v2.x), std::max(v3.x, v4.x));
    float ys = std::min(std::min(v1.y, v2.y), std::min(v3.y, v4.y));
    float yf = std::max(std::max(v1.y, v2.y), std::max(v3.y, v4.y));

    return xsi < xs && xfi > xf && ysi < ys && yfi > yf;
}

bool TwoPointScreenLocation::HitTest(int sx,int sy) const {
    //invert the matrix, get into render space
    glm::mat3 m = glm::inverse(*matrix);
    glm::vec3 v = m * glm::vec3(sx, sy, 1);

    float min = ymin;
    float max = ymax;
    if (!minMaxSet) {
        if (RenderHt < 4) {
            float sx1 = (x1 + x2) * previewW / 2.0;
            float sy1 = (y1 + y2) * previewH / 2.0;

            glm::vec3 v2 = m * glm::vec3(sx1 + 3, sy1 + 3, 1);
            glm::vec3 v3 = m * glm::vec3(sx1 + 3, sy1 - 3, 1);
            glm::vec3 v4 = m * glm::vec3(sx1 - 3, sy1 + 3, 1);
            glm::vec3 v5 = m * glm::vec3(sx1 - 3, sy1 - 3, 1);
            max = std::max(std::max(v2.y, v3.y), std::max(v4.y, v5.y));
            min = std::min(std::min(v2.y, v3.y), std::min(v4.y, v5.y));
        } else {
            min = -1;
            max = RenderHt;
        }
    }

    float y = v.y;
    return (v.x >= -1 && v.x <= (RenderWi+1) && y >= min && y <= max);
}

wxCursor TwoPointScreenLocation::CheckIfOverHandles(int &handle, int x, int y) const {
    for (size_t h = 0; h < mHandlePosition.size(); h++) {
        if (x>mHandlePosition[h].x && x<mHandlePosition[h].x+RECT_HANDLE_WIDTH &&
            y>mHandlePosition[h].y && y<mHandlePosition[h].y+RECT_HANDLE_WIDTH) {
            handle = h;
            return wxCURSOR_SIZING;
        }
    }
    handle = -1;
    return wxCURSOR_DEFAULT;
}
void TwoPointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {
    int x1_pos = x1 * previewW;
    int x2_pos = x2 * previewW;
    int y1_pos = y1 * previewH;
    int y2_pos = y2 * previewH;

    va.PreAlloc(10);
    if( y2_pos - y1_pos == 0 )
    {
        va.AddVertex(x1_pos, y1_pos, xlRED);
        va.AddVertex(x2_pos, y2_pos, xlRED);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }
    else if( x2_pos - x1_pos == 0 )
    {
        va.AddVertex(x1_pos, y1_pos, xlBLUE);
        va.AddVertex(x2_pos, y2_pos, xlBLUE);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }

    float sx = x1 * previewW - RECT_HANDLE_WIDTH / 2;
    float sy = y1 * previewH - RECT_HANDLE_WIDTH / 2;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlGREEN);
    mHandlePosition[0].x = sx;
    mHandlePosition[0].y = sy;

    sx = x2 * previewW - RECT_HANDLE_WIDTH / 2;
    sy = y2 * previewH - RECT_HANDLE_WIDTH / 2;
    va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
    mHandlePosition[1].x = sx;
    mHandlePosition[1].y = sy;
    va.Finish(GL_TRIANGLES);
}

int TwoPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    float newx = (float)mouseX / (float)previewW;
    float newy = (float)mouseY / (float)previewH;

    if (ShiftKeyPressed) {
        if (handle) {
            int x1_pos = x1 * previewW;
            int y1_pos = y1 * previewH;
            if (std::abs(mouseX - x1_pos) <= SNAP_RANGE) {
                newx = x1;
            }
            if (std::abs(mouseY - y1_pos) <= SNAP_RANGE) {
                newy = y1;
            }
        } else {
            int x2_pos = x2 * previewW;
            int y2_pos = y2 * previewH;
            if (std::abs(mouseX - x2_pos) <= SNAP_RANGE) {
                newx = x2;
            }
            if (std::abs(mouseY - y2_pos) <= SNAP_RANGE) {
                newy = y2;
            }
        }
    }

    if (handle) {
        x2 = newx;
        y2 = newy;
    } else {
        x1 = newx;
        y1 = newy;
    }
    return 0;
}
wxCursor TwoPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) {
    x1 = x2 = (float)x/(float)previewW;
    y1 = y2 = (float)y/(float)previewH;
    handle = 1;
    return wxCURSOR_SIZING;
}


void TwoPointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxFloatProperty("X1 (%)", "ModelX1", x1 * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = propertyEditor->Append(new wxFloatProperty("Y1 (%)", "ModelY1", y1 * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    prop = propertyEditor->Append(new wxFloatProperty("X2 (%)", "ModelX2", x2 * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxBLUE);
    prop = propertyEditor->Append(new wxFloatProperty("Y2 (%)", "ModelY2", y2 * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxBLUE);
}
int TwoPointScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if ("ModelX1" == name) {
        x1 = event.GetValue().GetDouble() / 100.0;
        return 3;
    } else if ("ModelY1" == name) {
        y1 = event.GetValue().GetDouble() / 100.0;
        return 3;
    } else if ("ModelX2" == name) {
        x2 = event.GetValue().GetDouble() / 100.0;
        return 3;
    } else if ("ModelY2" == name) {
        y2 = event.GetValue().GetDouble() / 100.0;
        return 3;
    }
    return 0;
}

void TwoPointScreenLocation::SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) {
    previewH = h;
    previewW = w;

    if (old) {
        //need to update to latest code
        ProcessOldNode(old);
        Write(old);
        old = nullptr;
    }
}
void TwoPointScreenLocation::ProcessOldNode(wxXmlNode *old) {
    BoxedScreenLocation box;
    box.Read(old);
    std::vector<NodeBaseClassPtr> Nodes;
    box.SetPreviewSize(previewW, previewH, Nodes);
    box.SetRenderSize(RenderWi, RenderHt);
    box.PrepareToDraw();

    float sx = - float(RenderWi) / 2.0;
    float sy = 0;
    box.TranslatePoint(sx, sy);
    x1 = sx / (float)previewW;
    y1 = sy / (float)previewH;

    sx = float(RenderWi) / 2.0;
    sy = 0;
    box.TranslatePoint(sx, sy);

    x2 = sx / (float)previewW;
    y2 = sy / (float)previewH;

    old->DeleteAttribute("offsetXpct");
    old->DeleteAttribute("offsetYpct");
    old->DeleteAttribute("PreviewScaleX");
    old->DeleteAttribute("PreviewScaleY");
    old->DeleteAttribute("PreviewRotation");
}

float TwoPointScreenLocation::GetHcenterOffset() const {
    return (x1 + x2) / 2.0;
}
float TwoPointScreenLocation::GetVcenterOffset() const {
    return (y1 + y2) / 2.0;
}

void TwoPointScreenLocation::SetHcenterOffset(float f) {
    float diffx = (x1 + x2) / 2.0 - f;
    x1 -= diffx;
    x2 -= diffx;
}
void TwoPointScreenLocation::SetVcenterOffset(float f) {
    float diffy = (y1 + y2) / 2.0 - f;
    y1 -= diffy;
    y2 -= diffy;
}

void TwoPointScreenLocation::SetOffset(float xPct, float yPct) {
    float diffx = (x1 + x2) / 2.0 - xPct;
    float diffy = (y1 + y2) / 2.0 - yPct;

    y1 -= diffy;
    y2 -= diffy;
    x1 -= diffx;
    x2 -= diffx;
}
void TwoPointScreenLocation::AddOffset(float xPct, float yPct) {
    y1 += yPct;
    y2 += yPct;
    x1 += xPct;
    x2 += xPct;
}
int TwoPointScreenLocation::GetTop() const {
    float y1i = y1 * previewH;
    float y2i = y2 * previewH;
    return std::max(std::round(y1i), std::round(y2i));
}

int TwoPointScreenLocation::GetLeft() const {
    float x1i = x1 * previewW;
    float x2i = x2 * previewW;
    return std::min(std::round(x1i), std::round(x2i));
}

int TwoPointScreenLocation::GetMWidth() const
{
    return std::abs((x1 - x2) * previewW);
}

int TwoPointScreenLocation::GetMHeight() const
{
    return std::abs((y1 - y2) * previewH);
}

int TwoPointScreenLocation::GetRight() const {
    float x1i = x1 * previewW;
    float x2i = x2 * previewW;
    return std::max(std::round(x1i), std::round(x2i));
}
int TwoPointScreenLocation::GetBottom() const {
    float y1i = y1 * previewH;
    float y2i = y2 * previewH;
    return std::min(std::round(y1i), std::round(y2i));
}
void TwoPointScreenLocation::SetTop(int i) {
    float newtop = (float)i / (float)previewH;
    if (y1 > y2) {
        float diff = y1 - newtop;
        y1 = newtop;
        y2 -= diff;
    } else {
        float diff = y2 - newtop;
        y2 = newtop;
        y1 -= diff;
    }
}
void TwoPointScreenLocation::SetLeft(int i) {
    float newx = (float)i / (float)previewW;
    if (x1 < x2) {
        float diff = x1 - newx;
        x1 = newx;
        x2 -= diff;
    } else {
        float diff = x2 - newx;
        x2 = newx;
        x1 -= diff;
    }
}
void TwoPointScreenLocation::SetRight(int i) {
    float newx = (float)i / (float)previewW;
    if (x1 > x2) {
        float diff = x1 - newx;
        x1 = newx;
        x2 -= diff;
    } else {
        float diff = x2 - newx;
        x2 = newx;
        x1 -= diff;
    }
}

void TwoPointScreenLocation::SetMWidth(int w)
{
    if (x1 > x2)
    {
        x1 = x2 + (float)w / previewW;
    }
    else
    {
        x2 = x1 + (float)w / previewW;
    }
}

void TwoPointScreenLocation::SetMHeight(int h)
{
    if (y1 > y2)
    {
        y1 = y2 + (float)h / previewH;
    }
    else
    {
        y2 = y1 + (float)h / previewH;
    }
}

void TwoPointScreenLocation::SetBottom(int i) {
    float newbot = (float)i / (float)previewH;
    if (y1 < y2) {
        float diff = y1 - newbot;
        y1 = newbot;
        y2 -= diff;
    } else {
        float diff = y2 - newbot;
        y2 = newbot;
        y1 -= diff;
    }
}

void TwoPointScreenLocation::FlipCoords() {
    std::swap(x1, x2);
    std::swap(y1, y2);
}


ThreePointScreenLocation::ThreePointScreenLocation(): height(1.0), modelHandlesHeight(false), supportsShear(false), supportsAngle(false), angle(0), shear(0.0) {
    mHandlePosition.resize(3);
}
ThreePointScreenLocation::~ThreePointScreenLocation() {
}
void ThreePointScreenLocation::Read(wxXmlNode *node) {
    TwoPointScreenLocation::Read(node);
    height = wxAtof(node->GetAttribute("Height", std::to_string(height)));
    angle = wxAtoi(node->GetAttribute("Angle", "0"));
    shear = wxAtof(node->GetAttribute("Shear", "0.0"));
}
void ThreePointScreenLocation::Write(wxXmlNode *node) {
    TwoPointScreenLocation::Write(node);
    node->DeleteAttribute("Height");
    node->AddAttribute("Height", std::to_string(height));
    if (supportsAngle) {
        node->DeleteAttribute("Angle");
        node->AddAttribute("Angle", std::to_string(angle));
    }
    if (supportsShear) {
        node->DeleteAttribute("Shear");
        node->AddAttribute("Shear", std::to_string(shear));
    }
}

void ThreePointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *grid) const {
    TwoPointScreenLocation::AddSizeLocationProperties(grid);
    wxPGProperty *prop = grid->Append(new wxFloatProperty("Height", "ModelHeight", height));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    if (supportsShear) {
        prop = grid->Append(new wxFloatProperty("Shear", "ModelShear", shear));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.1);
        prop->SetEditor("SpinCtrl");
    }
}
int ThreePointScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("ModelHeight" == event.GetPropertyName()) {
        height = event.GetValue().GetDouble();
        if (std::abs(height) < 0.01f) {
            if (height < 0.0f) {
                height = -0.01f;
            } else {
                height = 0.01f;
            }
        }
        return 3;
    } else if ("ModelShear" == event.GetPropertyName()) {
        shear = event.GetValue().GetDouble();
        return 3;
    }
    return TwoPointScreenLocation::OnPropertyGridChange(grid, event);
}

inline float toRadians(int degrees) {
    return 2.0*M_PI*float(degrees)/360.0;
}
static void rotate_point(float cx,float cy, float angle, float &x, float &y)
{
    float s = sin(angle);
    float c = cos(angle);

    // translate point back to origin:
    x -= cx;
    y -= cy;

    // rotate point
    float xnew = x * c - y * s;
    float ynew = x * s + y * c;

    // translate point back:
    x = xnew + cx;
    y = ynew + cy;
}

bool ThreePointScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
    float min = ymin;
    float max = ymax;
    if (!minMaxSet) {
        if( angle > -270 && angle < -90 ) {
            min = -RenderHt;
            max = 0;
        } else {
            min = 0;
            max = RenderHt;
        }
    }
    //invert the matrix, get into render space
    glm::vec3 v1 = *matrix * glm::vec3(0, min, 1);
    glm::vec3 v2 = *matrix * glm::vec3(0, max, 1);
    glm::vec3 v3 = *matrix * glm::vec3(RenderWi, min, 1);
    glm::vec3 v4 = *matrix * glm::vec3(RenderWi, max, 1);

    int xsi = x1<x2?x1:x2;
    int xfi = x1>x2?x1:x2;
    int ysi = y1<y2?y1:y2;
    int yfi = y1>y2?y1:y2;

    float xs = std::min(std::min(v1.x, v2.x), std::min(v3.x, v4.x));
    float xf = std::max(std::max(v1.x, v2.x), std::max(v3.x, v4.x));
    float ys = std::min(std::min(v1.y, v2.y), std::min(v3.y, v4.y));
    float yf = std::max(std::max(v1.y, v2.y), std::max(v3.y, v4.y));

    return xsi < xs && xfi > xf && ysi < ys && yfi > yf;
}

bool ThreePointScreenLocation::HitTest(int sx,int sy) const {
    //invert the matrix, get into render space
    glm::mat3 m = glm::inverse(*matrix);
    glm::vec3 v = m * glm::vec3(sx, sy, 1);

    float min = ymin;
    float max = ymax;
    if (!minMaxSet) {
        if (RenderHt < 4) {
            float sx1 = (x1 + x2) * previewW / 2.0;
            float sy1 = (y1 + y2) * previewH / 2.0;

            glm::vec3 v2 = m * glm::vec3(sx1 + 3, sy1 + 3, 1);
            glm::vec3 v3 = m * glm::vec3(sx1 + 3, sy1 - 3, 1);
            glm::vec3 v4 = m * glm::vec3(sx1 - 3, sy1 + 3, 1);
            glm::vec3 v5 = m * glm::vec3(sx1 - 3, sy1 - 3, 1);
            max = std::max(std::max(v2.y, v3.y), std::max(v4.y, v5.y));
            min = std::min(std::min(v2.y, v3.y), std::min(v4.y, v5.y));
        } else {
            if( angle > -270 && angle < -90 ) {
                min = -RenderHt;
                max = 1;
            } else {
                min = -1;
                max = RenderHt;
            }
        }
    }

    float y = v.y;
    return (v.x >= -1 && v.x <= (RenderWi+1) && y >= min && y <= max);
}

void ThreePointScreenLocation::SetMWidth(int w)
{
    TwoPointScreenLocation::SetMWidth(w);
}
void ThreePointScreenLocation::SetMHeight(int h)
{
    SetHeight((float)h / RenderHt);
    //TwoPointScreenLocation::SetMHeight(h);
}
int ThreePointScreenLocation::GetMWidth() const
{
    return TwoPointScreenLocation::GetMWidth();
}
int ThreePointScreenLocation::GetMHeight() const
{
    return GetHeight() * RenderHt;
    //return TwoPointScreenLocation::GetMHeight();
}

void ThreePointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {

    float sx1 = (x1 + x2) * previewW / 2.0;
    float sy1 = (y1 + y2) * previewH / 2.0;

    float max = ymax;
    if (!minMaxSet) {
        max = RenderHt;
    }
    va.PreAlloc(18);

    float x = RenderWi / 2;
    if (supportsAngle) {
        max = RenderHt * height;
        rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, max);
    }

    glm::vec3 v1 = *matrix * glm::vec3(x, max, 1);
    float sx = v1.x;
    float sy = v1.y;
    LOG_GL_ERRORV(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));
    va.AddVertex(sx1, sy1, xlWHITE);
    va.AddVertex(sx, sy, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);


    TwoPointScreenLocation::DrawHandles(va);


    va.AddRect(sx - RECT_HANDLE_WIDTH/2.0, sy - RECT_HANDLE_WIDTH/2.0, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
    va.Finish(GL_TRIANGLES);
    mHandlePosition[2].x = sx;
    mHandlePosition[2].y = sy;
}
int ThreePointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {
    if (handle == 2) {
        glm::mat3 m = glm::inverse(*matrix);
        glm::vec3 v = m * glm::vec3(mouseX, mouseY, 1);
        float max = ymax;
        if (!minMaxSet) {
            max = RenderHt;
        }
        if (max < 0.01f) {
            max = 0.01f;
        }
        float newy = v.y;
        if (supportsAngle) {
            float newx = v.x - RenderWi/2.0f;
            float nheight = std::sqrt(newy*newy + newx*newx);
            height = nheight / RenderHt;
            float newa = std::atan2(newy, newx) - M_PI/2;
            angle = toDegrees(newa);
        } else if (supportsShear) {
            height = height * newy / max;
            shear -= (v.x - (RenderWi / 2.0f)) / RenderWi;
            if (shear < -3.0f) {
                shear = -3.0f;
            } else if (shear > 3.0f) {
                shear = 3.0f;
            }
        } else {
            height = height * newy / max;
        }
        if (std::abs(height) < 0.01f) {
            if (height < 0.0f) {
                height = -0.01f;
            } else {
                height = 0.01f;
            }
        }
        return 1;
    }
    return TwoPointScreenLocation::MoveHandle(preview, handle, ShiftKeyPressed, mouseX, mouseY);
}

float ThreePointScreenLocation::GetVScaleFactor() const {
    if (modelHandlesHeight) {
        return 1.0;
    }
    return height;
}
float ThreePointScreenLocation::GetYShear() const {
    if (supportsShear) {
        return shear;
    }
    return 0.0;
}
void ThreePointScreenLocation::ProcessOldNode(wxXmlNode *old) {
    BoxedScreenLocation box;
    box.Read(old);
    std::vector<NodeBaseClassPtr> Nodes;
    box.SetPreviewSize(previewW, previewH, Nodes);
    box.SetRenderSize(RenderWi, RenderHt);
    box.PrepareToDraw();

    float x1 = RenderWi / 2.0;
    float y1 = RenderHt;
    box.TranslatePoint(x1, y1);

    TwoPointScreenLocation::ProcessOldNode(old);

    height = 1.0;
    PrepareToDraw();
    glm::mat3 m = glm::inverse(*matrix);
    glm::vec3 v = m * glm::vec3(x1, y1, 1);
    height = height * v.y / RenderHt;

}

PolyPointScreenLocation::PolyPointScreenLocation() : ModelScreenLocation(2),
   num_points(2), selected_handle(-1), selected_segment(-1) {
    mPos.resize(2);
    mPos[0].x = 0.4f;
    mPos[0].y = 0.6f;
    mPos[0].matrix = nullptr;
    mPos[0].curve = nullptr;
    mPos[0].has_curve = false;
    mPos[1].x = 0.4f;
    mPos[1].y = 0.6f;
    mPos[1].matrix = nullptr;
    mPos[1].curve = nullptr;
    mPos[1].has_curve = false;
    main_matrix = nullptr;
}
PolyPointScreenLocation::~PolyPointScreenLocation() {
    for( int i = 0; i < mPos.size(); ++i ) {
        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        if (mPos[i].curve != nullptr) {
            delete mPos[i].curve;
        }
    }
    mPos.clear();
    if (main_matrix != nullptr) {
        delete main_matrix;
    }
}

void PolyPointScreenLocation::SetCurve(int seg_num, bool create) {
    if( create ) {
        mPos[seg_num].has_curve = true;
        if( mPos[seg_num].curve == nullptr ) {
            mPos[seg_num].curve = new BezierCurveCubic();
        }
        mPos[seg_num].curve->set_p0( mPos[seg_num].x, mPos[seg_num].y );
        mPos[seg_num].curve->set_p1( mPos[seg_num+1].x, mPos[seg_num+1].y );
        mPos[seg_num].curve->set_cp0( mPos[seg_num].x, mPos[seg_num].y );
        mPos[seg_num].curve->set_cp1( mPos[seg_num+1].x, mPos[seg_num+1].y );
    } else {
        mPos[seg_num].has_curve = false;
        if( mPos[seg_num].curve != nullptr ) {
            delete mPos[seg_num].curve;
            mPos[seg_num].curve = nullptr;
        }
    }
}

void PolyPointScreenLocation::Read(wxXmlNode *ModelNode) {
    num_points = wxAtoi(ModelNode->GetAttribute("NumPoints", "2"));
    mPos.resize(num_points);
    wxString point_data = ModelNode->GetAttribute("PointData", "0.4, 0.6, 0.4, 0.6");
    wxArrayString point_array = wxSplit(point_data, ',');
    for( int i = 0; i < num_points; ++i ) {
        mPos[i].x = wxAtof(point_array[i*2]);
        mPos[i].y = wxAtof(point_array[i*2+1]);
        mPos[i].has_curve = false;
    }
    mHandlePosition.resize(num_points+4);
    wxString cpoint_data = ModelNode->GetAttribute("cPointData", "");
    wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
    int num_curves = cpoint_array.size() / 5;
    for( int i = 0; i < num_curves; ++i ) {
        int seg_num = wxAtoi(cpoint_array[i*5]);
        mPos[seg_num].has_curve = true;
        if( mPos[seg_num].curve == nullptr ) {
            mPos[seg_num].curve = new BezierCurveCubic();
        }
        mPos[seg_num].curve->set_p0( mPos[seg_num].x, mPos[seg_num].y );
        mPos[seg_num].curve->set_p1( mPos[seg_num+1].x, mPos[seg_num+1].y );
        mPos[seg_num].curve->set_cp0( wxAtof(cpoint_array[i*5+1]), wxAtof(cpoint_array[i*5+2]) );
        mPos[seg_num].curve->set_cp1( wxAtof(cpoint_array[i*5+3]), wxAtof(cpoint_array[i*5+4]) );
        mPos[seg_num].curve->SetScale(previewW, previewH, RenderWi);
        mPos[seg_num].curve->UpdatePoints();
    }
}

void PolyPointScreenLocation::Write(wxXmlNode *node) {
    node->DeleteAttribute("NumPoints");
    node->DeleteAttribute("PointData");
    node->DeleteAttribute("cPointData");
    wxString point_data = "";
    for( int i = 0; i < num_points; ++i ) {
        point_data += wxString::Format( "%f,", mPos[i].x );
        point_data += wxString::Format( "%f", mPos[i].y );
        if( i != num_points-1 ) {
            point_data += ",";
        }
    }
    wxString cpoint_data = "";
    for( int i = 0; i < num_points; ++i ) {
        if( mPos[i].has_curve ) {
            cpoint_data += wxString::Format( "%d,%f,%f,%f,%f,", i, mPos[i].curve->get_cp0x(), mPos[i].curve->get_cp0y(),
                                                                   mPos[i].curve->get_cp1x(), mPos[i].curve->get_cp1y() );
        }
    }
    node->AddAttribute("NumPoints", std::to_string(num_points));
    node->AddAttribute("PointData", point_data);
    node->AddAttribute("cPointData", cpoint_data);
}

void PolyPointScreenLocation::PrepareToDraw() const {
    minX = 100.0;
    minY = 100.0;
    maxX = 0.0;
    maxY = 0.0;

    for( int i = 0; i < num_points-1; ++i ) {
        float x1p = mPos[i].x * (float)previewW;
        float x2p = mPos[i+1].x * (float)previewW;
        float y1p = mPos[i].y * (float)previewH;
        float y2p = mPos[i+1].y * (float)previewH;

        if( mPos[i].x < minX ) minX = mPos[i].x;
        if( mPos[i].y < minY ) minY = mPos[i].y;
        if( mPos[i].x > maxX ) maxX = mPos[i].x;
        if( mPos[i].y > maxY ) maxY = mPos[i].y;

        if( mPos[i].has_curve ) {
            mPos[i].curve->check_min_max(minX, maxX, minY, maxY);
        }

        if( i == num_points-2 ) {
            if( mPos[i+1].x < minX ) minX = mPos[i+1].x;
            if( mPos[i+1].y < minY ) minY = mPos[i+1].y;
            if( mPos[i+1].x > maxX ) maxX = mPos[i+1].x;
            if( mPos[i+1].y > maxY ) maxY = mPos[i+1].y;
        }

        float angle = (float)M_PI/2.0f;
        if (mPos[i+1].x != mPos[i].x) {
            float slope = (y2p - y1p)/(x2p - x1p);
            angle = std::atan(slope);
            if (mPos[i].x > mPos[i+1].x) {
                angle += (float)M_PI;
            }
        } else if (mPos[i+1].y < mPos[i].y) {
            angle += (float)M_PI;
        }
        float scale = std::sqrt((y2p - y1p)*(y2p - y1p) + (x2p - x1p)*(x2p - x1p));
        scale /= RenderWi;

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, scale));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1p, y1p));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * scalingMatrix;

        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        mPos[i].matrix = new glm::mat3(mat3);

        // update curve points
        if( mPos[i].has_curve ) {
            mPos[i].curve->SetScale(previewW, previewH, RenderWi);
            mPos[i].curve->UpdatePoints();
        }
    }
    glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2((maxX-minX) * previewW, (maxY-minY) * previewH));
    glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(minX * previewW, minY * previewH));
    glm::mat3 mat3 = translateMatrix * scalingMatrix;
    if (main_matrix != nullptr) {
        delete main_matrix;
    }
    main_matrix = new glm::mat3(mat3);
}

void PolyPointScreenLocation::TranslatePoint(float &x, float &y) const {
    glm::vec3 v = *main_matrix * glm::vec3(x, y, 1);
    x = v.x;
    y = v.y;
}

bool PolyPointScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
    int sx1 = std::min(x1,x2);
    int sx2 = std::max(x1,x2);
    int sy1 = std::min(y1,y2);
    int sy2 = std::max(y1,y2);
    float x1p = minX * (float)previewW;
    float x2p = maxX * (float)previewW;
    float y1p = minY * (float)previewH;
    float y2p = maxY * (float)previewH;

    if( x1p >= sx1 && x1p <= sx2 &&
        x2p >= sx1 && x2p <= sx2 &&
        y1p >= sy1 && y1p <= sy2 &&
        y2p >= sy1 && y2p <= sy2 ) {
        return true;
    }

    return false;
}

bool PolyPointScreenLocation::HitTest(int sx,int sy) const {
    for( int i = 0; i < num_points-1; ++i ) {

        float min_y, max_y;

        if( mPos[i].has_curve ) {
            if( mPos[i].curve->HitTest(sx, sy) ) {
                selected_segment = i;
                return true;
            }
        } else {
            //invert the matrix, get into render space
            glm::mat3 m = glm::inverse(*mPos[i].matrix);
            glm::vec3 v = m * glm::vec3(sx, sy, 1);

            // perform normal line segment hit detection
            float sx1 = (mPos[i].x + mPos[i+1].x) * previewW / 2.0;
            float sy1 = (mPos[i].y + mPos[i+1].y) * previewH / 2.0;

            glm::vec3 v2 = m * glm::vec3(sx1 + 3, sy1 + 3, 1);
            glm::vec3 v3 = m * glm::vec3(sx1 + 3, sy1 - 3, 1);
            glm::vec3 v4 = m * glm::vec3(sx1 - 3, sy1 + 3, 1);
            glm::vec3 v5 = m * glm::vec3(sx1 - 3, sy1 - 3, 1);
            max_y = std::max(std::max(v2.y, v3.y), std::max(v4.y, v5.y));
            min_y = std::min(std::min(v2.y, v3.y), std::min(v4.y, v5.y));

            if (v.x >= 0.0 && v.x <= 1.0 && v.y >= min_y && v.y <= max_y) {
                selected_segment = i;
                return true;
            }
        }
    }
    selected_segment = -1;

    // check if inside boundary handles
    float sx1 = (float)sx / (float)previewW;
    float sy1 = (float)sy / (float)previewH;
    if( sx1 >= minX && sx1 <= maxX && sy1 >= minY && sy1 <= maxY ) {
        return true;
    }

    return false;
}

wxCursor PolyPointScreenLocation::CheckIfOverHandles(int &handle, int x, int y) const {
    // check handle of selected curve first
    if( selected_segment != -1 ) {
        if( mPos[selected_segment].has_curve ) {
            if (x>mPos[selected_segment].cp0.x && x<mPos[selected_segment].cp0.x+RECT_HANDLE_WIDTH &&
                y>mPos[selected_segment].cp0.y && y<mPos[selected_segment].cp0.y+RECT_HANDLE_WIDTH) {
                handle = 0x4000 | selected_segment;
                return wxCURSOR_SIZING;
            }
            if (x>mPos[selected_segment].cp1.x && x<mPos[selected_segment].cp1.x+RECT_HANDLE_WIDTH &&
                y>mPos[selected_segment].cp1.y && y<mPos[selected_segment].cp1.y+RECT_HANDLE_WIDTH) {
                handle = 0x8000 | selected_segment;
                return wxCURSOR_SIZING;
            }
        }
    }

    for (size_t h = 0; h < mHandlePosition.size(); h++) {
        if (x>mHandlePosition[h].x && x<mHandlePosition[h].x+RECT_HANDLE_WIDTH &&
            y>mHandlePosition[h].y && y<mHandlePosition[h].y+RECT_HANDLE_WIDTH) {
            handle = h;
            return wxCURSOR_SIZING;
        }
    }
    handle = -1;
    return wxCURSOR_DEFAULT;
}
void PolyPointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {
    va.PreAlloc(10*num_points+12);

    // add boundary handles
    float x1 = minX * previewW - RECT_HANDLE_WIDTH / 2;
    float y1 = minY * previewH - RECT_HANDLE_WIDTH / 2;
    float x2 = maxX * previewW - RECT_HANDLE_WIDTH / 2;
    float y2 = maxY * previewH - RECT_HANDLE_WIDTH / 2;
    va.AddRect(x1, y1, x1 + RECT_HANDLE_WIDTH, y1 + RECT_HANDLE_WIDTH, xlBLUE);
    va.AddRect(x1, y2, x1 + RECT_HANDLE_WIDTH, y2 + RECT_HANDLE_WIDTH, xlBLUE);
    va.AddRect(x2, y1, x2 + RECT_HANDLE_WIDTH, y1 + RECT_HANDLE_WIDTH, xlBLUE);
    va.AddRect(x2, y2, x2 + RECT_HANDLE_WIDTH, y2 + RECT_HANDLE_WIDTH, xlBLUE);
    mHandlePosition[num_points].x = x1;
    mHandlePosition[num_points].y = y1;
    mHandlePosition[num_points+1].x = x1;
    mHandlePosition[num_points+1].y = y2;
    mHandlePosition[num_points+2].x = x2;
    mHandlePosition[num_points+2].y = y1;
    mHandlePosition[num_points+3].x = x2;
    mHandlePosition[num_points+3].y = y2;

    for( int i = 0; i < num_points-1; ++i ) {
        int x1_pos = mPos[i].x * previewW;
        int x2_pos = mPos[i+1].x * previewW;
        int y1_pos = mPos[i].y * previewH;
        int y2_pos = mPos[i+1].y * previewH;

        if( i == selected_segment ) {
            va.Finish(GL_TRIANGLES);
            if( !mPos[i].has_curve ) {
                va.AddVertex(x1_pos, y1_pos, xlMAGENTA);
                va.AddVertex(x2_pos, y2_pos, xlMAGENTA);
            } else {
                // draw bezier curve
                x1_pos = mPos[i].curve->get_px(0) * previewW;
                y1_pos = mPos[i].curve->get_py(0) * previewH;
                for( int x = 1; x < mPos[i].curve->GetNumPoints(); ++x ) {
                    x2_pos = mPos[i].curve->get_px(x) * previewW;
                    y2_pos = mPos[i].curve->get_py(x) * previewH;
                    va.AddVertex(x1_pos, y1_pos, xlMAGENTA);
                    va.AddVertex(x2_pos, y2_pos, xlMAGENTA);
                    x1_pos = x2_pos;
                    y1_pos = y2_pos;
                }
                // draw control lines
                x1_pos = mPos[i].curve->get_p0x() * previewW;
                y1_pos = mPos[i].curve->get_p0y() * previewH;
                x2_pos = mPos[i].curve->get_cp0x() * previewW;
                y2_pos = mPos[i].curve->get_cp0y() * previewH;
                va.AddVertex(x1_pos, y1_pos, xlRED);
                va.AddVertex(x2_pos, y2_pos, xlRED);
                x1_pos = mPos[i].curve->get_p1x() * previewW;
                y1_pos = mPos[i].curve->get_p1y() * previewH;
                x2_pos = mPos[i].curve->get_cp1x() * previewW;
                y2_pos = mPos[i].curve->get_cp1y() * previewH;
                va.AddVertex(x1_pos, y1_pos, xlRED);
                va.AddVertex(x2_pos, y2_pos, xlRED);
            }
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
        }

        // add handle for start of this vector
        float sx = mPos[i].x * previewW - RECT_HANDLE_WIDTH / 2;
        float sy = mPos[i].y * previewH - RECT_HANDLE_WIDTH / 2;
        va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, i == selected_handle ? xlMAGENTA : (i == 0 ? xlGREEN : xlBLUE));
        mHandlePosition[i].x = sx;
        mHandlePosition[i].y = sy;

        // add final handle
        if( i == num_points-2 ) {
            sx = mPos[i+1].x * previewW - RECT_HANDLE_WIDTH / 2;
            sy = mPos[i+1].y * previewH - RECT_HANDLE_WIDTH / 2;
            va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, i+1 == selected_handle ? xlMAGENTA : xlBLUE);
            mHandlePosition[i+1].x = sx;
            mHandlePosition[i+1].y = sy;
        }
    }

    if( selected_segment != -1 ) {
        // add control point handles for selected segments
        int i = selected_segment;
        if( mPos[i].has_curve ) {
            float cx = mPos[i].curve->get_cp0x() * previewW - RECT_HANDLE_WIDTH / 2;
            float cy = mPos[i].curve->get_cp0y() * previewH - RECT_HANDLE_WIDTH / 2;
            va.AddRect(cx, cy, cx + RECT_HANDLE_WIDTH, cy + RECT_HANDLE_WIDTH, xlRED);
            mPos[i].cp0.x = cx;
            mPos[i].cp0.y = cy;
            cx = mPos[i].curve->get_cp1x() * previewW - RECT_HANDLE_WIDTH / 2;
            cy = mPos[i].curve->get_cp1y() * previewH - RECT_HANDLE_WIDTH / 2;
            va.AddRect(cx, cy, cx + RECT_HANDLE_WIDTH, cy + RECT_HANDLE_WIDTH, xlRED);
            mPos[i].cp1.x = cx;
            mPos[i].cp1.y = cy;
        }
    }

    va.Finish(GL_TRIANGLES);
}

int PolyPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {
    float newx = (float)mouseX / (float)previewW;
    float newy = (float)mouseY / (float)previewH;

    // check for control point handles
    if( handle & 0x4000 ) {
        int seg = handle & 0x0FFF;
        mPos[seg].cp0.x = newx;
        mPos[seg].cp0.y = newy;
        mPos[seg].curve->set_cp0( newx, newy );
    } else if( handle & 0x8000 ) {
        int seg = handle & 0x0FFF;
        mPos[seg].cp1.x = newx;
        mPos[seg].cp1.y = newy;
        mPos[seg].curve->set_cp1( newx, newy );

    // check normal handles
    } else if( handle < num_points ) {
        mPos[handle].x = newx;
        mPos[handle].y = newy;
        FixCurveHandles();
    } else {
        // move a boundary handle
        float trans_x = 0.0f;
        float trans_y = 0.0f;
        float scale_x = 1.0f;
        float scale_y = 1.0f;
        if( handle == num_points ) {  // bottom-left corner
            if( newx >= maxX-0.01f || newy >= maxY-0.01f ) return 0;
            trans_x = newx - minX;
            trans_y = newy - minY;
            scale_x -= trans_x / (maxX - minX);
            scale_y -= trans_y / (maxY - minY);
        } else if( handle == num_points+1 ) {  // top left corner
            if( newx >= maxX-0.01f || newy <= minY+0.01f ) return 0;
            trans_x = newx - minX;
            scale_x -= trans_x / (maxX - minX);
            scale_y = (newy - minY) / (maxY - minY);
        } else if( handle == num_points+2 ) {  // bottom right corner
            if( newx <= minX+0.01f|| newy >= maxY-0.01f ) return 0;
            trans_y = newy - minY;
            scale_x = (newx - minX) / (maxX - minX);
            scale_y -= trans_y / (maxY - minY);
        } else if( handle == num_points+3 ) {  // bottom right corner
            if( newx <= minX+0.01f || newy <= minY+0.01f ) return 0;
            scale_x = (newx - minX) / (maxX - minX);
            scale_y = (newy - minY) / (maxY - minY);
        } else {
            return 0;
        }

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2( scale_x, scale_y));
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2( minX + trans_x, minY + trans_y));
        glm::mat3 mat3 = translateMatrix * scalingMatrix;

        for( int i = 0; i < num_points; ++i ) {
            glm::vec3 v = mat3 * glm::vec3(mPos[i].x - minX, mPos[i].y - minY, 1);
            mPos[i].x = v.x;
            mPos[i].y = v.y;
            if( mPos[i].has_curve ) {
                float x1 = mPos[i].curve->get_cp0x();
                float y1 = mPos[i].curve->get_cp0y();
                v = mat3 * glm::vec3(x1 - minX, y1 - minY, 1);
                mPos[i].curve->set_cp0( v.x, v.y );
                x1 = mPos[i].curve->get_cp1x();
                y1 = mPos[i].curve->get_cp1y();
                v = mat3 * glm::vec3(x1 - minX, y1 - minY, 1);
                mPos[i].curve->set_cp1( v.x, v.y );
            }
        }
        FixCurveHandles();
    }

    return 1;
}

void PolyPointScreenLocation::SelectHandle(int handle) {
    selected_handle = handle;
    if( handle != -1 && handle < 0x4000 ) {
        selected_segment = -1;
    }
}

void PolyPointScreenLocation::SelectSegment(int segment) {
    selected_segment = segment;
    if( segment != -1 ) {
        selected_handle = -1;
    }
}

void PolyPointScreenLocation::AddHandle(ModelPreview* preview, int mouseX, int mouseY) {
    xlPolyPoint new_point;
    new_point.x = (float)mouseX/(float)previewW;
    new_point.y = (float)mouseY/(float)previewH;
    new_point.matrix = nullptr;
    new_point.curve = nullptr;
    new_point.has_curve = false;
    mPos.push_back(new_point);
    xlPoint new_handle;
    float sx = mPos[num_points-1].x * previewW - RECT_HANDLE_WIDTH / 2;
    float sy = mPos[num_points-1].y * previewH - RECT_HANDLE_WIDTH / 2;
    new_handle.x = sx;
    new_handle.y = sy;
    mHandlePosition.insert(mHandlePosition.begin() + num_points, new_handle);
    num_points++;
}

void PolyPointScreenLocation::InsertHandle(int after_handle) {
    float x1_pos = mPos[after_handle].x;
    float x2_pos = mPos[after_handle+1].x;
    float y1_pos = mPos[after_handle].y;
    float y2_pos = mPos[after_handle+1].y;
    xlPolyPoint new_point;
    new_point.x = (x1_pos+x2_pos)/2.0;
    new_point.y = (y1_pos+y2_pos)/2.0;
    new_point.matrix = nullptr;
    new_point.curve = nullptr;
    new_point.has_curve = false;
    mPos.insert(mPos.begin() + after_handle + 1, new_point);
    xlPoint new_handle;
    float sx = mPos[after_handle+1].x * previewW - RECT_HANDLE_WIDTH / 2;
    float sy = mPos[after_handle+1].y * previewH - RECT_HANDLE_WIDTH / 2;
    new_handle.x = sx;
    new_handle.y = sy;
    mHandlePosition.insert(mHandlePosition.begin() + after_handle + 1, new_handle);
    num_points++;
    selected_handle = after_handle+1;
    selected_segment = -1;
}

void PolyPointScreenLocation::DeleteHandle(int handle) {
    // delete any curves associated with this handle
    if( mPos[handle].has_curve ) {
        mPos[handle].has_curve = false;
        if( mPos[handle].curve != nullptr ) {
            delete mPos[handle].curve;
            mPos[handle].curve = nullptr;
        }
    }
    if( handle > 0 ) {
        if( mPos[handle-1].has_curve ) {
            mPos[handle-1].has_curve = false;
            if( mPos[handle-1].curve != nullptr ) {
                delete mPos[handle-1].curve;
                mPos[handle-1].curve = nullptr;
            }
        }
    }

    // now delete the handle
    mPos.erase(mPos.begin() + handle);
    mHandlePosition.erase(mHandlePosition.begin() + handle);
    num_points--;
    selected_handle = -1;
}

wxCursor PolyPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes) {
    mPos[0].x = (float)x/(float)previewW;
    mPos[0].y = (float)y/(float)previewH;
    mPos[1].x = (float)x/(float)previewW;
    mPos[1].y = (float)y/(float)previewH;
    handle = 1;
    return wxCURSOR_SIZING;
}

void PolyPointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxFloatProperty("X1 (%)", "ModelX1", mPos[0].x * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = propertyEditor->Append(new wxFloatProperty("Y1 (%)", "ModelY1", mPos[0].y * 100.0));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    for( int i = 1; i < num_points; ++i ) {
        prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("X%d (%%)",i+1), wxString::Format("ModelX%d",i+1), mPos[i].x * 100.0));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(*wxBLUE);
        prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Y%d (%%)",i+1), wxString::Format("ModelY%d",i+1), mPos[i].y * 100.0));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(*wxBLUE);
    }
}

int PolyPointScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if( name.length() > 6 ) {
        selected_handle = wxAtoi(name.substr(6, name.length()-6)) - 1;
        selected_segment = -1;
        if (name.find("ModelX") != std::string::npos) {
            mPos[selected_handle].x = event.GetValue().GetDouble() / 100.0;
            return 3;
        }
        else if (name.find("ModelY") != std::string::npos) {
            mPos[selected_handle].y = event.GetValue().GetDouble() / 100.0;
            return 3;
        }
    }
    return 0;
}

void PolyPointScreenLocation::SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) {
    previewH = h;
    previewW = w;
}

float PolyPointScreenLocation::GetHcenterOffset() const {
    float x_total = 0.0f;
    for(int i = 0; i < num_points; ++i ) {
        x_total += mPos[i].x;
    }
    return x_total / (float)num_points;
}
float PolyPointScreenLocation::GetVcenterOffset() const {
    float y_total = 0.0f;
    for(int i = 0; i < num_points; ++i ) {
        y_total += mPos[i].y;
    }
    return y_total / (float)num_points;
}

void PolyPointScreenLocation::SetHcenterOffset(float f) {
    float diffx = GetHcenterOffset() - f;
    for(int i = 0; i < num_points; ++i ) {
        mPos[i].x -= diffx;
        if( mPos[i].has_curve ) {
            mPos[i].curve->OffsetX(-diffx);
        }
    }
    FixCurveHandles();
}
void PolyPointScreenLocation::SetVcenterOffset(float f) {
    float diffy = GetVcenterOffset() - f;
    for(int i = 0; i < num_points; ++i ) {
        mPos[i].y -= diffy;
        if( mPos[i].has_curve ) {
            mPos[i].curve->OffsetY(-diffy);
        }
    }
    FixCurveHandles();
}

void PolyPointScreenLocation::SetOffset(float xPct, float yPct) {
    SetHcenterOffset(xPct);
    SetVcenterOffset(yPct);
}
void PolyPointScreenLocation::AddOffset(float xPct, float yPct) {
    for(int i = 0; i < num_points; ++i ) {
        mPos[i].x += xPct;
        mPos[i].y += yPct;
        if( mPos[i].has_curve ) {
            mPos[i].curve->OffsetX(xPct);
            mPos[i].curve->OffsetY(yPct);
        }
    }
    FixCurveHandles();
}
int PolyPointScreenLocation::GetTop() const {
    int topy = std::round(mPos[0].y * previewH);
    for(int i = 1; i < num_points; ++i ) {
        int newy = std::round(mPos[i].y * previewH);
        topy = std::max(topy, newy);
    }
    return topy;
}
int PolyPointScreenLocation::GetLeft() const {
    int leftx = std::round(mPos[0].x * previewH);
    for(int i = 1; i < num_points; ++i ) {
        int newx = std::round(mPos[i].x * previewW);
        leftx = std::min(leftx, newx);
    }
    return leftx;
}

int PolyPointScreenLocation::GetMHeight() const
{
    int maxy = 0;
    int miny = 0xFFFF;

    for (int i = 0; i < num_points; ++i) {
        int y = std::round(mPos[i].y * previewH);
        if (y > maxy) maxy = y;
        if (y < miny) miny = y;
    }

    return maxy - miny;
}

int PolyPointScreenLocation::GetMWidth() const
{
    int maxx = 0;
    int minx = 0xFFFF;

    for (int i = 0; i < num_points; ++i) {
        int x = std::round(mPos[i].x * previewW);
        if (x > maxx) maxx = x;
        if (x < minx) minx = x;
    }

    return maxx - minx;
}

void PolyPointScreenLocation::SetMWidth(int w)
{
    int currw = GetMWidth();
    float scale = w / currw;

    for (int i = 1; i < num_points; ++i)
    {
        float diff = mPos[i].x - mPos[0].x;
        mPos[i].x = mPos[0].x + diff * scale;
    }
}

void PolyPointScreenLocation::SetMHeight(int h)
{
    int currh = GetMHeight();
    float scale = h / currh;

    for (int i = 1; i < num_points; ++i)
    {
        float diff = mPos[i].y - mPos[0].y;
        mPos[i].y = mPos[0].y + diff * scale;
    }
}

int PolyPointScreenLocation::GetRight() const {
    int rightx = std::round(mPos[0].x * previewH);
    for(int i = 1; i < num_points; ++i ) {
        int newx = std::round(mPos[i].x * previewW);
        rightx = std::max(rightx, newx);
    }
    return rightx;
}
int PolyPointScreenLocation::GetBottom() const {
    int boty = std::round(mPos[0].y * previewH);
    for(int i = 1; i < num_points; ++i ) {
        int newy = std::round(mPos[i].y * previewH);
        boty = std::min(boty, newy);
    }
    return boty;

}
void PolyPointScreenLocation::SetTop(int i) {
    float newtop = (float)i / (float)previewH;
    float topy = mPos[0].y * (float)previewH;
    for(int i = 1; i < num_points; ++i ) {
        float newy = mPos[i].y * (float)previewH;
        topy = std::max(topy, newy);
    }
    float diff = topy - newtop;
    SetVcenterOffset(diff);
}
void PolyPointScreenLocation::SetLeft(int i) {
    float newleft = (float)i / (float)previewW;
    float leftx = mPos[0].x * (float)previewW;
    for(int i = 1; i < num_points; ++i ) {
        float newx = mPos[i].x * (float)previewW;
        leftx = std::max(leftx, newx);
    }
    float diff = leftx - newleft;
    SetHcenterOffset(diff);
}
void PolyPointScreenLocation::SetRight(int i) {
    float newright = (float)i / (float)previewW;
    float rightx = mPos[0].x * (float)previewW;
    for(int i = 1; i < num_points; ++i ) {
        float newx = mPos[i].x * (float)previewW;
        rightx = std::max(rightx, newx);
    }
    float diff = rightx - newright;
    SetHcenterOffset(diff);
}
void PolyPointScreenLocation::SetBottom(int i) {
    float newbot = (float)i / (float)previewH;
    float boty = mPos[0].y * (float)previewH;
    for(int i = 1; i < num_points; ++i ) {
        float newy = mPos[i].y * (float)previewH;
        boty = std::max(boty, newy);
    }
    float diff = boty - newbot;
    SetVcenterOffset(diff);
}

void PolyPointScreenLocation::FixCurveHandles() {
    for(int i = 0; i < num_points; ++i ) {
        if( mPos[i].has_curve ) {
            mPos[i].curve->set_p0( mPos[i].x, mPos[i].y );
            mPos[i].curve->set_p1( mPos[i+1].x, mPos[i+1].y );
            mPos[i].curve->UpdatePoints();
        }
    }
}
