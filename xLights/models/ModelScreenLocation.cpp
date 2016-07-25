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
static inline void RotatePoint(float radians, float &x1, float &y1) {
    float x = x1;
    float y = y1;
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
    offsetXpct(0.5), offsetYpct(0.5), singleScale(false),
    PreviewScaleX(0.333), PreviewScaleY(0.333),
    PreviewRotation(0) {
}
void BoxedScreenLocation::Read(wxXmlNode *ModelNode) {
    offsetXpct = wxAtof(ModelNode->GetAttribute("offsetXpct","0"));
    if(offsetXpct<0 || offsetXpct>1) {
        offsetXpct = .5;
    }
    offsetYpct = wxAtof(ModelNode->GetAttribute("offsetYpct","0"));
    if(offsetYpct<0 || offsetYpct>1) {
        offsetYpct = .5;
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
    if(PreviewScaleX<0 || PreviewScaleX>1) {
        PreviewScaleX = .33;
    }
    if(PreviewScaleY<0 || PreviewScaleY>1) {
        PreviewScaleY = .33;
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
   num_points(2) {
    mPos.resize(2);
    mPos[0].x = 0.4f;
    mPos[0].y = 0.6f;
    mPos[0].matrix = nullptr;
    mPos[1].x = 0.4f;
    mPos[1].y = 0.6f;
    mPos[1].matrix = nullptr;
}
PolyPointScreenLocation::~PolyPointScreenLocation() {
    for( int i = 0; i < mPos.size(); ++i ) {
        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
    }
    mPos.clear();
}
void PolyPointScreenLocation::Read(wxXmlNode *ModelNode) {
    num_points = wxAtoi(ModelNode->GetAttribute("NumPoints", "2"));
    mPos.resize(num_points);
    wxString point_data = ModelNode->GetAttribute("PointData", "0.4, 0.6, 0.4, 0.6");
    wxArrayString point_array = wxSplit(point_data, ',');
    for( int i = 0; i < num_points; ++i ) {
        mPos[i].x = wxAtof(point_array[i*2]);
        mPos[i].y = wxAtof(point_array[i*2+1]);
    }
    mHandlePosition.resize(num_points);
}
void PolyPointScreenLocation::Write(wxXmlNode *node) {
    node->DeleteAttribute("NumPoints");
    node->DeleteAttribute("PointData");
    wxString point_data = "";
    for( int i = 0; i < num_points; ++i ) {
        point_data += wxString::Format( "%f,", mPos[i].x );
        point_data += wxString::Format( "%f", mPos[i].y );
        if( i != num_points-1 ) {
            point_data += ",";
        }
    }
    node->AddAttribute("NumPoints", std::to_string(num_points));
    node->AddAttribute("PointData", point_data);
}

void PolyPointScreenLocation::PrepareToDraw() const {
    for( int i = 0; i < num_points-1; ++i ) {
        float x1p = mPos[i].x * (float)previewW;
        float x2p = mPos[i+1].x * (float)previewW;
        float y1p = mPos[i].y * (float)previewH;
        float y2p = mPos[i+1].y * (float)previewH;

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

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, scale * GetVScaleFactor()));
        glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
        glm::mat3 shearMatrix = glm::shearY(glm::mat3(1.0f), GetYShear());
        glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(mPos[i].x*previewW, mPos[i].y*previewH));
        glm::mat3 mat3 = translateMatrix * rotationMatrix * shearMatrix * scalingMatrix;

        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        mPos[i].matrix = new glm::mat3(mat3);
    }
}
void PolyPointScreenLocation::TranslatePoint(float &x, float &y) const {
    glm::vec3 v = *mPos[y].matrix * glm::vec3(x, 0, 1);
    x = v.x;
    y = v.y;
}

bool PolyPointScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
    for( int i = 0; i < num_points-1; ++i ) {
        float min = 0;
        float max = RenderHt;

        //invert the matrix, get into render space
        glm::vec3 v1 = *mPos[i].matrix * glm::vec3(0, min, 1);
        glm::vec3 v2 = *mPos[i].matrix * glm::vec3(0, max, 1);
        glm::vec3 v3 = *mPos[i].matrix * glm::vec3(RenderWi, min, 1);
        glm::vec3 v4 = *mPos[i].matrix * glm::vec3(RenderWi, max, 1);

        int xsi = mPos[i].x<mPos[i+1].x?mPos[i].x:mPos[i+1].x;
        int xfi = mPos[i].x>mPos[i+1].x?mPos[i].x:mPos[i+1].x;
        int ysi = mPos[i].y<mPos[i+1].y?mPos[i].y:mPos[i+1].y;
        int yfi = mPos[i].y>mPos[i+1].y?mPos[i].y:mPos[i+1].y;

        float xs = std::min(std::min(v1.x, v2.x), std::min(v3.x, v4.x));
        float xf = std::max(std::max(v1.x, v2.x), std::max(v3.x, v4.x));
        float ys = std::min(std::min(v1.y, v2.y), std::min(v3.y, v4.y));
        float yf = std::max(std::max(v1.y, v2.y), std::max(v3.y, v4.y));

        if( xsi < xs && xfi > xf && ysi < ys && yfi > yf ) {
            return true;
        }
    }

    return false;
}

bool PolyPointScreenLocation::HitTest(int sx,int sy) const {
    bool return_value = false;
    for( int i = 0; i < num_points-1; ++i ) {
        //invert the matrix, get into render space
        glm::mat3 m = glm::inverse(*mPos[i].matrix);
        glm::vec3 v = m * glm::vec3(sx, sy, 1);

        float min, max;

        if (RenderHt < 4) {
            float sx1 = (mPos[i].x + mPos[i+1].x) * previewW / 2.0;
            float sy1 = (mPos[i].y + mPos[i+1].y) * previewH / 2.0;

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

        float y = v.y;
        if (v.x >= -1 && v.x <= (RenderWi+1) && y >= min && y <= max) {
            return_value = true;
        }
    }
    return return_value;
}

wxCursor PolyPointScreenLocation::CheckIfOverHandles(int &handle, int x, int y) const {
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
    va.PreAlloc(10*num_points);
    for( int i = 0; i < num_points-1; ++i ) {
        int x1_pos = mPos[i].x * previewW;
        int x2_pos = mPos[i+1].x * previewW;
        int y1_pos = mPos[i].y * previewH;
        int y2_pos = mPos[i+1].y * previewH;

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

        // add handle for start of this vector
        float sx = mPos[i].x * previewW - RECT_HANDLE_WIDTH / 2;
        float sy = mPos[i].y * previewH - RECT_HANDLE_WIDTH / 2;
        va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, i == 0 ? xlGREEN : xlBLUE);
        mHandlePosition[i].x = sx;
        mHandlePosition[i].y = sy;

        // add final handle
        if( i == num_points-2 ) {
            sx = mPos[i+1].x * previewW - RECT_HANDLE_WIDTH / 2;
            sy = mPos[i+1].y * previewH - RECT_HANDLE_WIDTH / 2;
            va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, xlBLUE);
            mHandlePosition[i+1].x = sx;
            mHandlePosition[i+1].y = sy;
        }
    }
    va.Finish(GL_TRIANGLES);
}

int PolyPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    float newx = (float)mouseX / (float)previewW;
    float newy = (float)mouseY / (float)previewH;

    mPos[handle].x = newx;
    mPos[handle].y = newy;

    return 0;
}

void PolyPointScreenLocation::AddHandle(ModelPreview* preview, int mouseX, int mouseY) {
    xlPolyPoint new_point;
    new_point.x = (float)mouseX/(float)previewW;
    new_point.y = (float)mouseY/(float)previewH;
    new_point.matrix = nullptr;
    mPos.push_back(new_point);
    xlPoint new_handle;
    float sx = mPos[num_points-1].x * previewW - RECT_HANDLE_WIDTH / 2;
    float sy = mPos[num_points-1].y * previewH - RECT_HANDLE_WIDTH / 2;
    new_handle.x = sx;
    new_handle.y = sy;
    mHandlePosition.push_back(new_handle);
    num_points++;
}

void PolyPointScreenLocation::DeleteHandle(int handle) {
    mPos.erase(mPos.begin() + handle);
    mHandlePosition.erase(mHandlePosition.begin() + handle);
    num_points--;
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
/*    wxPGProperty *prop = propertyEditor->Append(new wxFloatProperty("X1 (%)", "ModelX1", x1 * 100.0));
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
    prop->SetTextColour(*wxBLUE);*/
}
int PolyPointScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
   /* std::string name = event.GetPropertyName().ToStdString();
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
    }*/
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
    }
}
void PolyPointScreenLocation::SetVcenterOffset(float f) {
    float diffy = GetVcenterOffset() - f;
    for(int i = 0; i < num_points; ++i ) {
        mPos[i].y -= diffy;
    }
}

void PolyPointScreenLocation::SetOffset(float xPct, float yPct) {
    SetHcenterOffset(xPct);
    SetVcenterOffset(yPct);
}
void PolyPointScreenLocation::AddOffset(float xPct, float yPct) {
    for(int i = 0; i < num_points; ++i ) {
        mPos[i].x += xPct;
        mPos[i].y += yPct;
    }
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


