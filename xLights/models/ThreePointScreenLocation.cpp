/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ThreePointScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "../ModelPreview.h"
#include "../support/VectorMath.h"
#include "UtilFunctions.h"
#include "RulerObject.h"

#include <log4cpp/Category.hh>

extern void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va);
extern void rotate_point(float cx, float cy, float angle, float &x, float &y);

static glm::mat4 Identity(glm::mat4(1.0f));

ThreePointScreenLocation::ThreePointScreenLocation()
{
    mHandlePosition.resize(4);
    handle_aabb_max.resize(4);
    handle_aabb_min.resize(4);
    mSelectableHandles = 4;
}

ThreePointScreenLocation::~ThreePointScreenLocation() {
}

wxCursor ThreePointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {
    if (preview != nullptr) {
        FindPlaneIntersection( x, y, preview );
        if( preview->Is3D() ) {
            active_handle = END_HANDLE;
        }
    }
    else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }
    x2 = y2 = z2 = 0.0f;
    handle = END_HANDLE;
    return wxCURSOR_SIZING;
}

void ThreePointScreenLocation::Read(wxXmlNode *node) {
    TwoPointScreenLocation::Read(node);
    height = wxAtof(node->GetAttribute("Height", std::to_string(height)));
    angle = wxAtoi(node->GetAttribute("Angle", "0"));
    shear = wxAtof(node->GetAttribute("Shear", "0.0"));
    rotatex = wxAtof(node->GetAttribute("RotateX", "0"));
}

void ThreePointScreenLocation::Write(wxXmlNode *node) {
    TwoPointScreenLocation::Write(node);
    node->DeleteAttribute("Height");
    node->DeleteAttribute("Locked");
    node->DeleteAttribute("RotateX");
    node->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
    node->AddAttribute("Height", std::to_string(height));
    if (supportsAngle) {
        node->DeleteAttribute("Angle");
        node->AddAttribute("Angle", std::to_string(angle));
    }
    if (supportsShear) {
        node->DeleteAttribute("Shear");
        node->AddAttribute("Shear", std::to_string(shear));
    }
    if (_locked)
    {
        node->AddAttribute("Locked", "1");
    }
}

void ThreePointScreenLocation::AddDimensionProperties(wxPropertyGridInterface* propertyEditor, float factor) const
{
    TwoPointScreenLocation::AddDimensionProperties(propertyEditor, 1.0);
    float width = RulerObject::Measure(origin, point2);
    wxPGProperty* prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Height (%s)", RulerObject::GetUnitDescription()), "RealHeight", 
                                                                     RulerObject::Measure((width * height) / 2.0 * factor * 100.0)
                                                                    ));
    prop->ChangeFlag(wxPG_PROP_READONLY, true);
    prop->SetAttribute("Precision", 2);
    prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
}

std::string ThreePointScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    float width = RulerObject::Measure(origin, point2);
    return wxString::Format("Length %s Height %s", RulerObject::MeasureLengthDescription(origin, point2), 
        RulerObject::PrescaledMeasureDescription((width * height) / 2.0 * factor)).ToStdString();
}

float ThreePointScreenLocation::GetRealWidth() const
{
    return RulerObject::Measure(origin, point2);
}

float ThreePointScreenLocation::GetRealHeight() const
{
    float width = RulerObject::Measure(origin, point2);
    return RulerObject::Measure((width * height) / 2.0 * 1.0 * 100.0);
}

void ThreePointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    TwoPointScreenLocation::AddSizeLocationProperties(propertyEditor);
    wxPGProperty *prop = propertyEditor->Append(new wxFloatProperty("Height", "ModelHeight", height));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    if (supportsShear) {
        prop = propertyEditor->Append(new wxFloatProperty("Shear", "ModelShear", shear));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.1);
        prop->SetEditor("SpinCtrl");
    }
    prop = propertyEditor->Append(new wxFloatProperty("RotateX", "RotateX", rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1);
    prop->SetEditor("SpinCtrl");
}

int ThreePointScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    wxString name = event.GetPropertyName();
    if (!_locked && "ModelHeight" == name) {
        height = event.GetValue().GetDouble();
        if (std::abs(height) < 0.01f) {
            if (height < 0.0f) {
                height = -0.01f;
            }
            else {
                height = 0.01f;
            }
        }
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ThreePointScreenLocation::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ThreePointScreenLocation::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ThreePointScreenLocation::OnPropertyGridChange::ModelHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ThreePointScreenLocation::OnPropertyGridChange::ModelHeight");
        return 0;
    }
    else if (_locked && "ModelHeight" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelShear" == name) {
        shear = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "ThreePointScreenLocation::OnPropertyGridChange::ModelShear");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "ThreePointScreenLocation::OnPropertyGridChange::ModelShear");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "ThreePointScreenLocation::OnPropertyGridChange::ModelShear");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "ThreePointScreenLocation::OnPropertyGridChange::ModelShear");
        return 0;
    }
    else if (_locked && "ModelShear" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateX" == name) {
        rotatex = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::RotateX");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::RotateX");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::RotateX");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (_locked && "RotateX" == name) {
        event.Veto();
        return 0;
    }

    return TwoPointScreenLocation::OnPropertyGridChange(grid, event);
}

inline float toRadians(int degrees) {
    return 2.0*M_PI*float(degrees) / 360.0;
}

void ThreePointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    
    origin = glm::vec3(worldPos_x, worldPos_y, worldPos_z);

    // if both points are exactly equal, then the line is length 0 and the scaling matrix
    // will not be usable.  We'll offset the x coord slightly so the scaling matrix
    // will not be a 0 matrix
    float x = x2;
    if (x2 == 0.0f && y2 == 0.0f && z2 == 0.0f) {
        x = 0.001f;
    }

    point2 = glm::vec3(x + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);

    glm::vec3 point2_calc = point2;
    glm::vec3 origin_calc = origin;
    bool swapped = false;
    if( x2 < 0.0f ) { // green square right of blue square
        point2_calc = origin;
        origin_calc = point2;
        swapped = true;
    }
    
    glm::vec3 a = point2_calc - origin_calc;
    scalex = scaley = scalez = glm::length(a) / RenderWi;
    glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);

    glm::mat4 scalingMatrix;
    if (modelHandlesHeight) {
        scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
    } else {
        scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley * height, scalez));
    }
    shearMatrix = Identity;
    if (supportsShear) {
        shearMatrix = glm::mat4(glm::shearY(glm::mat3(1.0f), GetYShear()));
    }
    glm::mat4 RotateY = glm::rotate(Identity, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 RotateX = glm::rotate(Identity, glm::radians((float)rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
    TranslateMatrix = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    if (swapped) {
        rotationMatrix = rotationMatrix * RotateY;
    }
    matrix = TranslateMatrix * rotationMatrix * RotateX * shearMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);
        ModelMatrix = TranslateMatrix * rotationMatrix * RotateX;
        glm::vec4 ctr = matrix * glm::vec4(center, 1.0f);
        center = glm::vec3(ctr);
    }

    draw_3d = is_3d;
}

bool ThreePointScreenLocation::IsContained(ModelPreview* preview, int x1_, int y1_, int x2_, int y2_) const {
    int xs = x1_ < x2_ ? x1_ : x2_;
    int xf = x1_ > x2_ ? x1_ : x2_;
    int ys = y1_ < y2_ ? y1_ : y2_;
    int yf = y1_ > y2_ ? y1_ : y2_;

    if (draw_3d) {
        return VectorMath::TestVolumeOBBIntersection(
            xs, ys, xf, yf,
            preview->getWidth(),
            preview->getHeight(),
            aabb_min, aabb_max,
            preview->GetProjViewMatrix(),
            TranslateMatrix);
    } else {
        glm::vec3 min = glm::vec3(TranslateMatrix * glm::vec4(aabb_min, 1.0f));
        glm::vec3 max = glm::vec3(TranslateMatrix * glm::vec4(aabb_max, 1.0f));

        if (min.x >= xs && max.x <= xf && min.y >= ys && max.y <= yf) {
            return true;
        }
        else {
            return false;
        }
    }
}

bool ThreePointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {
    return TwoPointScreenLocation::HitTest(ray_origin, ray_direction);
}

void ThreePointScreenLocation::SetMWidth(float w)
{
    TwoPointScreenLocation::SetMWidth(w);
}

void ThreePointScreenLocation::SetMHeight(float h)
{
    height = h;
}

float ThreePointScreenLocation::GetMWidth() const
{
    return TwoPointScreenLocation::GetMWidth();
}

float ThreePointScreenLocation::GetMHeight() const
{
    return height;
}

void ThreePointScreenLocation::SetActiveHandle(int handle)
{
    active_handle = handle;
    highlighted_handle = -1;
    SetAxisTool(axis_tool);  // run logic to disallow certain tools
}

void ThreePointScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (active_handle == SHEAR_HANDLE) {
        axis_tool = MSLTOOL::TOOL_XY_TRANS;
    } else {
        TwoPointScreenLocation::SetAxisTool(mode);
    }
}

void ThreePointScreenLocation::AdvanceAxisTool()
{
    if (active_handle == SHEAR_HANDLE) {
        axis_tool = MSLTOOL::TOOL_XY_TRANS;
    } else {
        TwoPointScreenLocation::AdvanceAxisTool();
    }
}

void ThreePointScreenLocation::SetActiveAxis(MSLAXIS axis)
{
    if (active_handle == SHEAR_HANDLE) {
        if (axis != MSLAXIS::NO_AXIS) {
            active_axis = MSLAXIS::X_AXIS;
        } else {
            active_axis = MSLAXIS::NO_AXIS;
        }
    } else {
        ModelScreenLocation::SetActiveAxis(axis);
    }
}
bool ThreePointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {
    if (active_handle != -1) {

        float ymax = RenderHt;
        
        auto vac = program->getAccumulator();
        int startVertex = vac->getCount();
        vac->PreAlloc(38);

        float x = RenderWi / 2;
        if (supportsAngle) {
            ymax = RenderHt * height;
            rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, ymax);
        }

        glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 0.0f), 1.0f));
        float sx = v1.x;
        float sy = v1.y;
        float sz = v1.z;
        vac->AddVertex(center.x, center.y, center.z, xlWHITE);
        vac->AddVertex(sx, sy, sz, xlWHITE);

        xlColor h4c = xlBLUETRANSLUCENT;
        if (fromBase)
        {
            h4c = FROM_BASE_HANDLES_COLOUR;
        } else
        if (_locked) {
            h4c = LOCKED_HANDLES_COLOUR;
        } else {
            h4c = (highlighted_handle == SHEAR_HANDLE) ? xlYELLOWTRANSLUCENT : xlBLUETRANSLUCENT;
        }

        float hw = GetRectHandleWidth(zoom, scale);
        vac->AddSphereAsTriangles(sx, sy, sz, hw, h4c);
        mHandlePosition[SHEAR_HANDLE].x = sx;
        mHandlePosition[SHEAR_HANDLE].y = sy;
        mHandlePosition[SHEAR_HANDLE].z = sz;

        handle_aabb_min[SHEAR_HANDLE].x = x * scalex - hw;
        handle_aabb_min[SHEAR_HANDLE].y = ymax * scaley * (supportsShear ? height : 1.0f) - hw;
        handle_aabb_min[SHEAR_HANDLE].z = -hw;
        handle_aabb_max[SHEAR_HANDLE].x = x * scalex + hw;
        handle_aabb_max[SHEAR_HANDLE].y = ymax * scaley * (supportsShear ? height : 1.0f) + hw;
        handle_aabb_max[SHEAR_HANDLE].z = hw;

        if (supportsShear) {
            handle_aabb_min[SHEAR_HANDLE] = glm::vec3(shearMatrix * glm::vec4(handle_aabb_min[SHEAR_HANDLE], 1.0f));
            handle_aabb_max[SHEAR_HANDLE] = glm::vec3(shearMatrix * glm::vec4(handle_aabb_max[SHEAR_HANDLE], 1.0f));
        }
        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, 2);
            ctx->drawTriangles(vac, startVertex + 2, count - startVertex - 2);
        });
    }

    TwoPointScreenLocation::DrawHandles(program, zoom, scale, drawBounding, fromBase);
    return true;
}

bool ThreePointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    float sx1 = center.x;
    float sy1 = center.y;

    float ymax = RenderHt;

    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();
    vac->PreAlloc(18);

    float x = RenderWi / 2;
    if (supportsAngle) {
        ymax = RenderHt * height;
        rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, ymax);
    }

    glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 1), 1.0f));
    float sx = v1.x;
    float sy = v1.y;
    vac->AddVertex(sx1, sy1, xlWHITE);
    vac->AddVertex(sx, sy, xlWHITE);

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase)
    {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else
    if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }
    float hw = GetRectHandleWidth(zoom, scale);
    vac->AddRectAsTriangles(sx - hw/2.0, sy - hw/2.0, sx + hw, sy + hw, handleColor);
    mHandlePosition[SHEAR_HANDLE].x = sx;
    mHandlePosition[SHEAR_HANDLE].y = sy;
    int count = vac->getCount();
    program->addStep([=](xlGraphicsContext *ctx) {
        ctx->drawLines(vac, startVertex, 2);
        ctx->drawTriangles(vac, startVertex + 2, count - 2);
    });

    TwoPointScreenLocation::DrawHandles(program, zoom, scale, fromBase);
    return true;
}

void ThreePointScreenLocation::DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const {
    xlColor Box3dColor = xlWHITETRANSLUCENT;
    if (fromBase)
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    else if (_locked)
        Box3dColor = LOCKED_HANDLES_COLOUR;
    DrawBoundingBoxLines(Box3dColor, aabb_min, aabb_max, draw_3d ? ModelMatrix : TranslateMatrix, *vac);
}

int ThreePointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

    if (handle == SHEAR_HANDLE) {
        glm::vec3 ray_origin;
        glm::vec3 ray_direction;

        VectorMath::ScreenPosToWorldRay(
            mouseX, preview->getHeight() - mouseY,
            preview->getWidth(), preview->getHeight(),
            preview->GetProjViewMatrix(),
            ray_origin,
            ray_direction
        );

        float posx = ray_origin.x - center.x;
        float posy = ray_origin.y - center.y;

        float ymax = RenderHt;
        if (ymax < 0.01f) {
            ymax = 0.01f;
        }
        //Calculate angle of mouse from center.
        if (supportsAngle) {
            if (posy == 0.0f) return 0;
            float tan = (float)posx / (float)posy;
            int angle1 = -toDegrees((float)atan(tan));
            if (x2 < 0.0f) {
                angle1 = -angle1;
            }
            if (posy >= 0) {
                angle = angle1;
            } else if (posx <= 0) {
                angle = 90 + (90 + angle1);
            } else {
                angle = -90 - (90 - angle1);
            }
            if (ShiftKeyPressed) {
                angle = (int)(angle / 5) * 5;
            }
            float length = std::sqrt(posy*posy + posx * posx);
            height = length / (RenderHt * scaley);
        } else if (supportsShear) {
            glm::mat4 m = glm::inverse(matrix);
            glm::vec3 v = glm::vec3(m * glm::vec4(ray_origin, 1.0f));
            if (height < 0.0f) {
                shear -= (v.x - ((RenderWi) / 2.0f)) / (RenderWi);
            } else {
                shear += (v.x - ((RenderWi) / 2.0f)) / (RenderWi);
            }
            if (shear < -3.0f) {
                shear = -3.0f;
            } else if (shear > 3.0f) {
                shear = 3.0f;
            }
            height = posy / (RenderHt * scaley);
        } else {
            height = height * posy / ymax;
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

int ThreePointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (_locked) return 0;

    if (handle == SHEAR_HANDLE) {
        if (latch) {
            saved_position.x = mHandlePosition[SHEAR_HANDLE].x;
            saved_position.y = mHandlePosition[SHEAR_HANDLE].y;
            saved_position.z = mHandlePosition[SHEAR_HANDLE].z;
        }

        if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

        if (axis_tool == MSLTOOL::TOOL_XY_TRANS) {
            glm::vec3 a = point2 - origin;
            glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
            glm::mat4 inv_rotation = glm::inverse(rotationMatrix);
            glm::vec3 cent = glm::vec3(inv_rotation * glm::vec4(center, 1.0f));
            glm::vec3 origin = glm::vec3(inv_rotation * glm::vec4(saved_position + drag_delta, 1.0f));

            float posx = origin.x - cent.x;
            float posy = origin.y - cent.y;

            //float posx = saved_position.x + drag_delta.x - center.x;
            //float posy = saved_position.y + drag_delta.y - center.y;

            float ymax = RenderHt;
            if (ymax < 0.01f) {
                ymax = 0.01f;
            }

            //Calculate angle of mouse from center.
            if (supportsAngle) {
                if (posy == 0.0f) return 0;
                float tan = (float)posx / (float)posy;
                int angle1 = -toDegrees((float)atan(tan));
                if (posy >= 0) {
                    angle = angle1;
                }
                else if (posx <= 0) {
                    angle = 180 + angle1;
                }
                else {
                    angle = -180 + angle1;
                }
                if (ShiftKeyPressed) {
                    angle = (int)(angle / 5) * 5;
                }
                float length = std::sqrt(posy*posy + posx * posx);
                height = length / (RenderHt * scaley);
            }
            else if (supportsShear) {
                glm::mat4 m = glm::inverse(matrix);
                glm::vec3 v = glm::vec3(m * glm::vec4(saved_position + drag_delta, 1.0f));
                if (height < 0.0f) {
                    shear -= (v.x - ((RenderWi) / 2.0f)) / (RenderWi);
                }
                else {
                    shear += (v.x - ((RenderWi) / 2.0f)) / (RenderWi);
                }
                if (shear < -3.0f) {
                    shear = -3.0f;
                }
                else if (shear > 3.0f) {
                    shear = 3.0f;
                }
                height = posy / (RenderHt * scaley);
            }
            else {
                height = height * posy / ymax;
            }
            if (std::abs(height) < 0.01f) {
                if (height < 0.0f) {
                    height = -0.01f;
                }
                else {
                    height = 0.01f;
                }
            }
            return 1;
        }
    }
    else if (handle == CENTER_HANDLE ) {
        if (axis_tool == MSLTOOL::TOOL_ROTATE) {
            if (latch) {
                saved_angle = 0.0f;
                saved_position = center;
                saved_point = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            }
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            if (active_axis == MSLAXIS::X_AXIS) {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                double angle = end_angle - start_angle;
                rotatex = saved_rotate.x - angle;
            }
        }
    }
    return TwoPointScreenLocation::MoveHandle3D(preview, handle, ShiftKeyPressed, CtrlKeyPressed, mouseX, mouseY, latch, scale_z);
}
int ThreePointScreenLocation::MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) {
    if (handle == SHEAR_HANDLE) {
        //we'll handle move, ignore rotations
        if (supportsAngle) {
            angle -= mov.x*10.0f;
            height += -mov.z;
        } else if (supportsShear) {
            shear -= mov.x*10.0f;
            height += -mov.z;
        } else {
            height += -mov.z;
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
    return TwoPointScreenLocation::MoveHandle3D(scale, handle, rot, mov);
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

void ThreePointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    if (Nodes.size() > 0) {
        aabb_min = glm::vec3(100000.0f, 100000.0f, 100000.0f);
        aabb_max = glm::vec3(0.0f, 0.0f, 0.0f);

        if (draw_3d) {
            shearMatrix = glm::mat4(glm::shearY(glm::mat3(1.0f), GetYShear()));
            glm::mat4 scalingMatrix;
            if (modelHandlesHeight) {
                scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
            }
            else {
                scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley * height, scalez));
            }

            for (const auto& it : Nodes) {
                for (const auto& coord : it.get()->Coords) {

                    float sx = coord.screenX;
                    float sy = coord.screenY;
                    float sz = coord.screenZ;

                    glm::vec3 shear_point = glm::vec3(shearMatrix * scalingMatrix * glm::vec4(glm::vec3(sx, sy, sz), 1.0f));
                    sx = shear_point.x;
                    sy = shear_point.y;
                    sz = shear_point.z;

                    //aabb vectors need to be the untranslated / unrotated
                    if (sx < aabb_min.x) {
                        aabb_min.x = sx;
                    }
                    if (sy < aabb_min.y) {
                        aabb_min.y = sy;
                    }
                    if (sz < aabb_min.z) {
                        aabb_min.z = sz;
                    }
                    if (sx > aabb_max.x) {
                        aabb_max.x = sx;
                    }
                    if (sy > aabb_max.y) {
                        aabb_max.y = sy;
                    }
                    if (sz > aabb_max.z) {
                        aabb_max.z = sz;
                    }
                }
            }
        }
        else {
            aabb_min.z = 0.0f;
            aabb_max.z = 0.0f;
            for (const auto& it : Nodes) {
                for (const auto& coord : it.get()->Coords) {

                    float sx = coord.screenX;
                    float sy = coord.screenY;
                    float sz = coord.screenZ;
                    glm::vec4 v = matrix * glm::vec4(glm::vec3(sx, sy, sz), 1.0f);
                    v.x -= worldPos_x;
                    v.y -= worldPos_y;

                    //aabb vectors need to be the untranslated / unrotated
                    if (v.x < aabb_min.x) {
                        aabb_min.x = v.x;
                    }
                    if (v.y < aabb_min.y) {
                        aabb_min.y = v.y;
                    }
                    if (v.x > aabb_max.x) {
                        aabb_max.x = v.x;
                    }
                    if (v.y > aabb_max.y) {
                        aabb_max.y = v.y;
                    }
                }
            }
        }

        // Set minimum bounding rectangle
        if (aabb_max.x - aabb_min.x < 4) {
            aabb_max.x += 5;
            aabb_min.x -= 5;
        }
        if (aabb_max.y - aabb_min.y < 4) {
            aabb_max.y += 5;
            aabb_min.y -= 5;
        }
        if (aabb_max.z - aabb_min.z < 4) {
            aabb_max.z += 5;
            aabb_min.z -= 5;
        }
    }
}
