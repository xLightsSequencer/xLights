#include "ModelScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "../ModelPreview.h"
#include "../DrawGLUtils.h"


#define RECT_HANDLE_WIDTH           6
#define BOUNDING_RECT_OFFSET        8

static inline void TranslatePointDoubles(double radians,double x, double y,double &x1, double &y1) {
    x1 = cos(radians)*x-(sin(radians)*y);
    y1 = sin(radians)*x+(cos(radians)*y);
}
static inline void RotatePoint(double radians, float &x1, float &y1) {
    float x = x1;
    float y = y1;
    x1 = cos(radians)*x-(sin(radians)*y);
    y1 = sin(radians)*x+(cos(radians)*y);
}

inline double toRadians(long degrees) {
    return 2.0*M_PI*double(degrees)/360.0;
}

inline long toDegrees(double radians) {
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

ModelScreenLocation::ModelScreenLocation() : RenderWi(0), RenderHt(0), previewW(800), previewH(600) {
}


BoxedScreenLocation::BoxedScreenLocation() :
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
        tempstr.ToDouble(&PreviewScaleX);
        tempstr.ToDouble(&PreviewScaleY);
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


void BoxedScreenLocation::TranslatePoint(double &sx, double &sy) const {
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
    scalex = (double)previewW / (double)RenderWi * PreviewScaleX;
    scaley = (double)previewH / (double)RenderHt * PreviewScaleY;
    centerx = int(offsetXpct*(double)previewW);
    centery = int(offsetYpct*(double)previewH);
}

void BoxedScreenLocation::SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) {
    previewW = w;
    previewH = h;
    
    if (singleScale) {
        //we now have the virtual size so we can flip to non-single scale
        singleScale = false;
        if (RenderHt > RenderWi) {
            PreviewScaleX = double(RenderWi) * double(previewH) / (double(previewW) * RenderHt) * PreviewScaleY;
        } else {
            PreviewScaleY = double(RenderHt) * double(previewW) / (double(previewH) * RenderWi) * PreviewScaleX;
        }
    }
    PrepareToDraw();
    
    double sx,sy;
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


void BoxedScreenLocation::DrawHandles() const {
    double w1 = centerx;
    double h1 = centery;
    
    double sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    double sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    mHandlePosition[0].x = sx;
    mHandlePosition[0].y = sy;
    // Upper Right Handle
    sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
    sy = (RenderHt*scaley/2) + BOUNDING_RECT_OFFSET;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    mHandlePosition[1].x = sx;
    mHandlePosition[1].y = sy;
    // Lower Right Handle
    sx =  (RenderWi*scalex/2) + BOUNDING_RECT_OFFSET;
    sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    mHandlePosition[2].x = sx;
    mHandlePosition[2].y = sy;
    // Lower Left Handle
    sx =  (-RenderWi*scalex/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    sy = (-RenderHt*scaley/2) - BOUNDING_RECT_OFFSET-RECT_HANDLE_WIDTH;
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx = sx + w1;
    sy = sy + h1;
    DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    mHandlePosition[3].x = sx;
    mHandlePosition[3].y = sy;
    
    // Draw rotation handle square
    sx = -RECT_HANDLE_WIDTH/2;
    sy = ((RenderHt*scaley/2) + 50);
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx += w1;
    sy += h1;
    DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    // Save rotate handle
    mHandlePosition[4].x = sx;
    mHandlePosition[4].y = sy;
    // Draw rotation handle from center to 25 over rendered height
    sx = 0;
    sy = ((RenderHt*scaley/2) + 50);
    TranslatePointDoubles(radians,sx,sy,sx,sy);
    sx += w1;
    sy += h1;
    DrawGLUtils::DrawLine(xlWHITE,255,w1,h1,sx,sy,1.0);
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
void BoxedScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX,int mouseY) {
    if (handle == OVER_ROTATE_HANDLE) {
        int sx,sy;
        sx = mouseX-centerx;
        sy = mouseY-centery;
        //Calculate angle of mouse from center.
        float tan = (float)sx/(float)sy;
        int angle = -toDegrees((double)atan(tan));
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
        double sx,sy;
        sx = double(mouseX)-centerx;
        sy = double(mouseY)-centery;
        double radians=-toRadians(PreviewRotation); // negative angle to reverse translation
        TranslatePointDoubles(radians,sx,sy,sx,sy);
        sx = fabs(sx) - RECT_HANDLE_WIDTH;
        sy = fabs(sy) - RECT_HANDLE_WIDTH;
        SetScale( (double)(sx*2.0)/double(previewW), (double)(sy*2.0)/double(previewH));
    }
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

TwoPointScreenLocation::TwoPointScreenLocation() : x1(0.4), y1(0.4), x2(0.6), y2(0.6), old(nullptr), matrix(nullptr), minMaxSet(false) {
    
}
TwoPointScreenLocation::~TwoPointScreenLocation() {
    if (matrix != nullptr) {
        delete matrix;
    }
}
void TwoPointScreenLocation::Read(wxXmlNode *ModelNode) {
    if (!ModelNode->HasAttribute("X1")) {
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
    float angle = M_PI/2;
    if (x2 != x1) {
        float slope = (y2 - y1)/(x2 - x1);
        angle = std::atan(slope);
        if (x1 > x2) {
            angle += M_PI;
        }
    } else if (y2 < y1) {
        angle += M_PI;
    }
    float scale = std::sqrt((y2 - y1)*(y2 - y1) + (x2 - x1)*(x2 - x1))/RenderWi;

    glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2(scale, scale));
    glm::mat3 rotationMatrix = glm::rotate(glm::mat3(1.0f), (float)angle);
    glm::mat3 translateMatrix = glm::translate(glm::mat3(1.0f), glm::vec2(x1, y1));
    glm::mat3 scaling2Matrix = glm::scale(glm::mat3(1.0f), glm::vec2(previewW, previewH));
    
    glm::mat3 mat3 = scaling2Matrix * translateMatrix * rotationMatrix * scalingMatrix;
    
    if (matrix != nullptr) {
        delete matrix;
    }
    matrix = new glm::mat3(mat3);
}
void TwoPointScreenLocation::TranslatePoint(double &x, double &y) const {
    glm::vec3 v = *matrix * glm::vec3(x, y, 1);
    x = (v[0]);
    y = (v[1]);
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
    
    float xs = std::min(std::min(v1[0], v2[0]), std::min(v3[0], v4[0]));
    float xf = std::max(std::max(v1[0], v2[0]), std::max(v3[0], v4[0]));
    float ys = std::min(std::min(v1[1], v2[1]), std::min(v3[1], v4[1]));
    float yf = std::max(std::max(v1[1], v2[1]), std::max(v3[1], v4[1]));
    
    return xsi < xs && xfi > xf && ysi < ys && yfi > yf;
}

bool TwoPointScreenLocation::HitTest(int sx,int sy) const {
    //invert the matrix, get into render space
    glm::mat3 m = glm::inverse(*matrix);
    glm::vec3 v = m * glm::vec3(sx, sy, 1);
    glm::vec3 v2 = m * glm::vec3(sx + 1, sy + 1, 1);
    
    float min = ymin;
    float max = ymax;
    if (!minMaxSet) {
        if (RenderHt < 4) {
            min = -std::abs(v2[1] - v[1]) * 6.0;
            max = -min;
        } else {
            min = -1;
            max = RenderHt;
        }
    }

    float y = v[1];
    return (v[0] >= -1 && v[0] <= (RenderWi+1) && y >= min && y <= max);
}

wxCursor TwoPointScreenLocation::CheckIfOverHandles(int &handle, int x, int y) const {
    for (int h = 0; h < 2; h++) {
        if (x>mHandlePosition[h].x && x<mHandlePosition[h].x+RECT_HANDLE_WIDTH &&
            y>mHandlePosition[h].y && y<mHandlePosition[h].y+RECT_HANDLE_WIDTH) {
            handle = h;
            return wxCURSOR_SIZING;
        }
    }
    handle = -1;
    return wxCURSOR_DEFAULT;
}
void TwoPointScreenLocation::DrawHandles() const {
    float sx = x1 * previewW - RECT_HANDLE_WIDTH / 2;
    float sy = y1 * previewH - RECT_HANDLE_WIDTH / 2;
    DrawGLUtils::DrawFillRectangle(xlGREEN,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    mHandlePosition[0].x = sx;
    mHandlePosition[0].y = sy;
    
    sx = x2 * previewW - RECT_HANDLE_WIDTH / 2;
    sy = y2 * previewH - RECT_HANDLE_WIDTH / 2;
    DrawGLUtils::DrawFillRectangle(xlBLUE,255,sx,sy,RECT_HANDLE_WIDTH,RECT_HANDLE_WIDTH);
    mHandlePosition[1].x = sx;
    mHandlePosition[1].y = sy;
}

void TwoPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {
    float newx = (float)mouseX / (float)previewW;
    float newy = (float)mouseY / (float)previewH;
    if (handle) {
        x2 = newx;
        y2 = newy;
    } else {
        x1 = newx;
        y1 = newy;
    }
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
        BoxedScreenLocation box;
        box.Read(old);
        std::vector<NodeBaseClassPtr> Nodes;
        box.SetPreviewSize(previewW, previewH, Nodes);
        box.SetRenderSize(RenderWi, RenderHt);
        box.PrepareToDraw();
        
        double sx = - float(RenderWi) / 2.0; double sy = 0;
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
        Write(old);
        old = nullptr;
    }
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

void TwoPointScreenLocation::SetOffset(double xPct, double yPct) {
    float diffx = (x1 + x2) / 2.0 - xPct;
    float diffy = (y1 + y2) / 2.0 - yPct;
    
    y1 -= diffy;
    y2 -= diffy;
    x1 -= diffx;
    x2 -= diffx;
}
void TwoPointScreenLocation::AddOffset(double xPct, double yPct) {
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


