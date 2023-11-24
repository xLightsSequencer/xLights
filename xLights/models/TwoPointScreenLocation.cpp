/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TwoPointScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Model.h"
#include "../ModelPreview.h"
#include "../support/VectorMath.h"
#include "UtilFunctions.h"
#include "RulerObject.h"

#include <log4cpp/Category.hh>

extern void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va);
extern wxCursor GetResizeCursor(int cornerIndex, int PreviewRotation);
extern void rotate_point(float cx, float cy, float angle, float &x, float &y);

#define SNAP_RANGE                  5

static float BB_OFF = 5.0f;

static glm::mat4 Identity(glm::mat4(1.0f));

TwoPointScreenLocation::TwoPointScreenLocation() : ModelScreenLocation(3)
{
    mSelectableHandles = 3;
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
}

TwoPointScreenLocation::~TwoPointScreenLocation() {
}

ModelScreenLocation::MSLUPGRADE TwoPointScreenLocation::CheckUpgrade(wxXmlNode* node)
{
    // check for upgrade to world positioning
    int version = wxAtoi(node->GetAttribute("versionNumber", "0"));
    if (version < 2) {
        // skip first upgrade call since preview size is not set
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", "2");
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_SKIPPED;
    } else if (version == 2) {
        if (node->HasAttribute("X1")) {  // Two Point model
            float old_x1 = wxAtof(node->GetAttribute("X1", "0"));
            float old_y1 = wxAtof(node->GetAttribute("Y1", "0"));
            float old_x2 = wxAtof(node->GetAttribute("X2", "0"));
            float old_y2 = wxAtof(node->GetAttribute("Y2", "0"));
            worldPos_x = previewW * old_x1;
            worldPos_y = previewH * old_y1;
            worldPos_z = 0.0f;
            x2 = previewW * old_x2 - worldPos_x;
            y2 = previewH * old_y2 - worldPos_y;
            z2 = 0.0f;
            node->DeleteAttribute("X1");
            node->DeleteAttribute("Y1");
            node->DeleteAttribute("X2");
            node->DeleteAttribute("Y2");
            node->DeleteAttribute("Z2");
            node->DeleteAttribute("WorldPosX");
            node->DeleteAttribute("WorldPosY");
            node->DeleteAttribute("WorldPosZ");
            node->AddAttribute("WorldPosX", wxString::Format("%6.4f", worldPos_x));
            node->AddAttribute("WorldPosY", wxString::Format("%6.4f", worldPos_y));
            node->AddAttribute("WorldPosZ", wxString::Format("%6.4f", worldPos_z));
            node->AddAttribute("X2", wxString::Format("%6.4f", x2));
            node->AddAttribute("Y2", wxString::Format("%6.4f", y2));
            node->AddAttribute("Z2", wxString::Format("%6.4f", z2));
        }
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_EXEC_DONE;
    }
    return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED;
}

void TwoPointScreenLocation::Read(wxXmlNode *ModelNode) {
    ModelScreenLocation::MSLUPGRADE upgrade_result = CheckUpgrade(ModelNode);
    if (upgrade_result == ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED) {
        worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "0.0"));
        worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
        worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));
        x2 = wxAtof(ModelNode->GetAttribute("X2", "0.0"));
        y2 = wxAtof(ModelNode->GetAttribute("Y2", "0.0"));
        z2 = wxAtof(ModelNode->GetAttribute("Z2", "0.0"));
        _locked = (wxAtoi(ModelNode->GetAttribute("Locked", "0")) == 1);
    }
}

void TwoPointScreenLocation::Write(wxXmlNode *ModelXml) {
    ModelXml->DeleteAttribute("WorldPosX");
    ModelXml->DeleteAttribute("WorldPosY");
    ModelXml->DeleteAttribute("WorldPosZ");
    ModelXml->DeleteAttribute("Locked");
    ModelXml->AddAttribute("WorldPosX", wxString::Format("%6.4f", worldPos_x));
    ModelXml->AddAttribute("WorldPosY", wxString::Format("%6.4f", worldPos_y));
    ModelXml->AddAttribute("WorldPosZ", wxString::Format("%6.4f", worldPos_z));
    ModelXml->DeleteAttribute("X2");
    ModelXml->DeleteAttribute("Y2");
    ModelXml->DeleteAttribute("Z2");
    ModelXml->DeleteAttribute("Locked");
    ModelXml->AddAttribute("X2", std::to_string(x2));
    ModelXml->AddAttribute("Y2", std::to_string(y2));
    ModelXml->AddAttribute("Z2", std::to_string(z2));
    if (_locked)
    {
        ModelXml->AddAttribute("Locked", "1");
    }
}

void TwoPointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    origin = glm::vec3(worldPos_x, worldPos_y, worldPos_z);

    // if both points are exactly equal, then the line is length 0 and the scaling matrix
    // will not be usable.  We'll offset the x coord slightly so the scaling matrix
    // will not be a 0 matrix
    float x = x2;
    if (x2 == 0.0f && y2 == 0.0f && z2 == 0.0f) {
        x = 0.001f;
    }

    point2 = glm::vec3(x + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);

    glm::vec3 a = point2 - origin;
    glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector2(origin, point2);
    length = glm::length(a);
    scalex = scaley = scalez = length / RenderWi;
    glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
    TranslateMatrix = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    matrix = TranslateMatrix * rotationMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);
        ModelMatrix = TranslateMatrix * rotationMatrix;
        glm::vec4 ctr = matrix * glm::vec4(center, 1.0f);
        center = glm::vec3(ctr);
    }

    draw_3d = is_3d;
}

void TwoPointScreenLocation::TranslatePoint(float &x, float &y, float &z) const {
    glm::vec4 v = matrix * glm::vec4(glm::vec3(x, y, z), 1.0f);
    x = v.x;
    y = v.y;
    z = v.z;
}
void TwoPointScreenLocation::ApplyModelViewMatrices(xlGraphicsContext *ctx) const {
    ctx->SetModelMatrix(matrix);
}


bool TwoPointScreenLocation::IsContained(ModelPreview* preview, int x1_, int y1_, int x2_, int y2_) const {
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
            ModelMatrix);
    }
    else {
        glm::vec3 min = glm::vec3(ModelMatrix * glm::vec4(aabb_min, 1.0f));
        glm::vec3 max = glm::vec3(ModelMatrix * glm::vec4(aabb_max, 1.0f));

        if (min.x >= xs && max.x <= xf && min.y >= ys && max.y <= yf) {
            return true;
        }
        else {
            return false;
        }
    }
}

bool TwoPointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {
    // NOTE:  This routine is designed for the 2D layout model selection only

    bool return_value = false;

    if (VectorMath::TestRayOBBIntersection2D(
        ray_origin,
        aabb_min,
        aabb_max,
        mSelectableHandles == 4 ? TranslateMatrix : ModelMatrix)
        ) {
        return_value = true;
    }

    return return_value;
}

wxCursor TwoPointScreenLocation::CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const
{
    // NOTE:  This routine is designed for the 2D layout handle selection only
    wxASSERT(!preview->Is3D());

    handle = NO_HANDLE;

    if (_locked)
    {
        return wxCURSOR_DEFAULT;
    }

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;

    VectorMath::ScreenPosToWorldRay(
        x, preview->getHeight() - y,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    int num_handles = mHandlePosition.size()-1; // 2D doesn't use center handle

    float zoom = preview->GetCameraZoomForHandles();
    int scale = preview->GetHandleScale();
    float hw = GetRectHandleWidth(zoom, scale);
    for (size_t h = 0; h < num_handles; h++) {
        handle_aabb_min[h].x = mHandlePosition[h + 1].x - hw;
        handle_aabb_min[h].y = mHandlePosition[h + 1].y - hw;
        handle_aabb_min[h].z = mHandlePosition[h + 1].z - hw;
        handle_aabb_max[h].x = mHandlePosition[h + 1].x + hw;
        handle_aabb_max[h].y = mHandlePosition[h + 1].y + hw;
        handle_aabb_max[h].z = mHandlePosition[h + 1].z + hw;
    }

    // Test each each Oriented Bounding Box (OBB).
    int handles_found = 0;
    for (size_t i = 0; i < num_handles; i++)
    {
        if (VectorMath::TestRayOBBIntersection2D(
            ray_origin,
            handle_aabb_min[i],
            handle_aabb_max[i],
            Identity)
            ) {
            handle = i + 1;
            handles_found++;
            if (handles_found == 2) {
                break;  // if handles overlap we want the second sizing handle to take precedence
            }
        }
    }

    if (handle == NO_HANDLE) {
        return wxCURSOR_DEFAULT;
    }
    else if (handle == SHEAR_HANDLE) {
        return wxCURSOR_HAND;
    }
    else {
        return GetResizeCursor(handle, rotatez);
    }
}

void TwoPointScreenLocation::SetActiveHandle(int handle)
{
    active_handle = handle;
    highlighted_handle = -1;
    SetAxisTool(axis_tool);  // run logic to disallow certain tools
}

void TwoPointScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (mode != MSLTOOL::TOOL_SCALE && mode != MSLTOOL::TOOL_XY_TRANS) {
        axis_tool = mode;
    }
    else {
        axis_tool = MSLTOOL::TOOL_TRANSLATE;
    }
}

void TwoPointScreenLocation::AdvanceAxisTool()
{
    ModelScreenLocation::AdvanceAxisTool();
    if (axis_tool == MSLTOOL::TOOL_SCALE) {
        ModelScreenLocation::AdvanceAxisTool();
    }
}

bool TwoPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase)
    {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else
    if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }
    
    auto va = program->getAccumulator();
    int startVert = va->getCount();

    va->PreAlloc(16);
    if (point2.y - origin.y == 0) {
        va->AddVertex(worldPos_x, worldPos_y, xlRED);
        va->AddVertex(point2.x, point2.y, xlRED);
    } else if (point2.x - origin.x == 0) {
        va->AddVertex(worldPos_x, worldPos_y, handleColor);
        va->AddVertex(point2.x, point2.y, handleColor);
    }
    int startTriangles = va->getCount();

    float sx = worldPos_x;
    float sy = worldPos_y;
    float sz = worldPos_z;
    //TranslatePoint(sx, sy, sz);
    float hw = GetRectHandleWidth(zoom, scale);
    va->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), xlGREENTRANSLUCENT);
    mHandlePosition[START_HANDLE].x = sx;
    mHandlePosition[START_HANDLE].y = sy;
    mHandlePosition[START_HANDLE].z = sz;

    sx = point2.x;
    sy = point2.y;
    sz = point2.z;
    //TranslatePoint(sx, sy, sz);
    va->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), handleColor);
    mHandlePosition[END_HANDLE].x = sx;
    mHandlePosition[END_HANDLE].y = sy;
    mHandlePosition[END_HANDLE].z = sz;
    int count = va->getCount();

    program->addStep([startVert, startTriangles, count, va, program](xlGraphicsContext *ctx) {
        if (startVert != startTriangles) {
            ctx->drawLines(va, startVert, startTriangles - startVert);
        }
        ctx->drawTriangles(va, startTriangles, count - startTriangles);
    });
    return true;
}


bool TwoPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {
    auto va = program->getAccumulator();
    int startVert = va->getCount();
    va->PreAlloc(10);

    if (active_handle != NO_HANDLE) {
        xlColor h1c = xlBLUETRANSLUCENT;
        xlColor h2c = xlBLUETRANSLUCENT;
        xlColor h3c = xlORANGETRANSLUCENT;
        if (fromBase) {
            h1c = FROM_BASE_HANDLES_COLOUR;
            h2c = FROM_BASE_HANDLES_COLOUR;
            h3c = FROM_BASE_HANDLES_COLOUR;
        } else
        if (_locked) {
            h1c = LOCKED_HANDLES_COLOUR;
            h2c = LOCKED_HANDLES_COLOUR;
            h3c = LOCKED_HANDLES_COLOUR;
        } else {
            h1c = (highlighted_handle == START_HANDLE) ? xlYELLOWTRANSLUCENT : xlGREENTRANSLUCENT;
            h2c = (highlighted_handle == END_HANDLE) ? xlYELLOWTRANSLUCENT : xlBLUETRANSLUCENT;
            h3c = (highlighted_handle == CENTER_HANDLE) ? xlYELLOWTRANSLUCENT : xlORANGETRANSLUCENT;
        }

        float hw = GetRectHandleWidth(zoom, scale);
        va->AddSphereAsTriangles(worldPos_x, worldPos_y, worldPos_z, hw, h1c);
        mHandlePosition[START_HANDLE].x = worldPos_x;
        mHandlePosition[START_HANDLE].y = worldPos_y;
        mHandlePosition[START_HANDLE].z = worldPos_z;

        va->AddSphereAsTriangles(point2.x, point2.y, point2.z, hw, h2c);
        mHandlePosition[END_HANDLE].x = point2.x;
        mHandlePosition[END_HANDLE].y = point2.y;
        mHandlePosition[END_HANDLE].z = point2.z;


        va->AddSphereAsTriangles(center.x, center.y, center.z, hw, h3c);
        mHandlePosition[CENTER_HANDLE].x = center.x;
        mHandlePosition[CENTER_HANDLE].y = center.y;
        mHandlePosition[CENTER_HANDLE].z = center.z;

        handle_aabb_min[START_HANDLE].x = -hw;
        handle_aabb_min[START_HANDLE].y = -hw;
        handle_aabb_min[START_HANDLE].z = -hw;
        handle_aabb_max[START_HANDLE].x = hw;
        handle_aabb_max[START_HANDLE].y = hw;
        handle_aabb_max[START_HANDLE].z = hw;

        handle_aabb_min[END_HANDLE].x = RenderWi * scalex - hw;
        handle_aabb_min[END_HANDLE].y = -hw;
        handle_aabb_min[END_HANDLE].z = -hw;
        handle_aabb_max[END_HANDLE].x = RenderWi * scalex + hw;
        handle_aabb_max[END_HANDLE].y = hw;
        handle_aabb_max[END_HANDLE].z = hw;

        handle_aabb_min[CENTER_HANDLE].x = (RenderWi / 2.0f) * scalex - hw;
        handle_aabb_min[CENTER_HANDLE].y = -hw;
        handle_aabb_min[CENTER_HANDLE].z = -hw;
        handle_aabb_max[CENTER_HANDLE].x = (RenderWi / 2.0f) * scalex + hw;
        handle_aabb_max[CENTER_HANDLE].y = hw;
        handle_aabb_max[CENTER_HANDLE].z = hw;

        int endTriangles = va->getCount();
        int startLines = va->getCount();
        if (!_locked) {
            active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
            DrawAxisTool(active_handle_pos, program, zoom, scale);
            if (active_axis != MSLAXIS::NO_AXIS) {
                startLines = va->getCount();
                if (active_handle == SHEAR_HANDLE) {
                    va->AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va->AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va->AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                    va->AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                } else {
                    switch (active_axis) {
                    case MSLAXIS::X_AXIS:
                        va->AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                        va->AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                        break;
                    case MSLAXIS::Y_AXIS:
                        va->AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                        va->AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                        break;
                    case MSLAXIS::Z_AXIS:
                        va->AddVertex(active_handle_pos.x, active_handle_pos.y, -1000000.0f, xlBLUETRANSLUCENT);
                        va->AddVertex(active_handle_pos.x, active_handle_pos.y, +1000000.0f, xlBLUETRANSLUCENT);
                        break;
                    default:
                        break;
                    }
                }
            }
        }
        int endLines = va->getCount();
        program->addStep([startVert, endTriangles, startLines, endLines, program, va](xlGraphicsContext *ctx) {
            ctx->drawTriangles(va, startVert, endTriangles - startVert);
            if (endLines != startLines) {
                ctx->drawLines(va, startLines, endLines - startLines);
            }
        });
    } else if (drawBounding) {
        // the bounding box is so close to a single line don't draw it once it's selected
        DrawBoundingBox(va, fromBase);
        int endLines = va->getCount();
        program->addStep([startVert, endLines, program, va](xlGraphicsContext *ctx) {
            ctx->drawLines(va, startVert, endLines - startVert);
        });
    }
    return true;
}
void TwoPointScreenLocation::DrawBoundingBox(xlVertexColorAccumulator *vac, bool fromBase) const {
    xlColor Box3dColor = xlWHITETRANSLUCENT;
    if (fromBase)
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    else if (_locked)
        Box3dColor = LOCKED_HANDLES_COLOUR;
    
    glm::vec3 start = origin - glm::vec3(5, 5, 5);
    glm::vec3 end = point2 + glm::vec3(5, 5, 5);
    
    glm::vec3 start2 = origin + glm::vec3(5, 5, 5);
    glm::vec3 end2 = point2 - glm::vec3(5, 5, 5);
    
    float len1 = std::abs(glm::length(end - start));
    float len2 = std::abs(glm::length(end2 - start2));
    if (len2 > len1) {
        start = start2;
        end = end2;
    }

    if (abs(start.x) < 4) {
        start.x = -5;
    }
    if (abs(start.y) < 4) {
        start.y = 5;
    }
    if (abs(start.z) < 4) {
        start.z = 5;
    }

    if (abs(end.x) < 4) {
        end.x = 5;
    }
    if (abs(end.y) < 4) {
        end.y = 5;
    }
    if (abs(end.z) < 4) {
        end.z = 5;
    }

    glm::mat4 mat;
    mat[0] = glm::vec4(1, 0, 0, 0);
    mat[1] = glm::vec4(0, 1, 0, 0);
    mat[2] = glm::vec4(0, 0, 1, 0);
    mat[3] = glm::vec4(0, 0, 0, 1);
    
    DrawBoundingBoxLines(Box3dColor, start, end, mat, *vac);
}

int TwoPointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (latch) {
        saved_angle = 0.0f;
        angles = glm::vec3(0,0,0);

        if (handle == CENTER_HANDLE) {
            saved_position = center;
            saved_point = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        } else if (handle == END_HANDLE) {
            saved_position = point2;
            saved_point = glm::vec3(x2, y2, z2);
        } else {
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            saved_point = glm::vec3(x2, y2, z2);
        }
    }

    if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

    if (handle == CENTER_HANDLE) {

        if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
                worldPos_x = saved_point.x + drag_delta.x;
                break;
            case MSLAXIS::Y_AXIS:
                worldPos_y = saved_point.y + drag_delta.y;
                break;
            case MSLAXIS::Z_AXIS:
                worldPos_z = saved_point.z + drag_delta.z;
                break;
            default:
                break;
            }
        } else if (axis_tool == MSLTOOL::TOOL_ROTATE) {
            double angle = 0.0f;
            float new_angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;

            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                new_angle = saved_angle - angle;
            }
            break;
            case MSLAXIS::Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                new_angle = saved_angle - angle;
            }
            break;
            case MSLAXIS::Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                new_angle = angle - saved_angle;
            }
            break;
            default:
                break;
            }
            TwoPointScreenLocation::Rotate(active_axis, new_angle);
            saved_angle = angle;
        } else if (axis_tool == MSLTOOL::TOOL_SCALE) {
            double delta = 0.0f;
            glm::vec3 scaling = glm::vec3(1.0f);
            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
                delta = (drag_delta.x - saved_position.x) * 2.0f;
                scaling.x = (length + delta) / length;
                break;
            case MSLAXIS::Y_AXIS:
                delta = (drag_delta.y - saved_position.y) * 2.0f;
                scaling.y = (length + delta) / length;
                break;
            case MSLAXIS::Z_AXIS:
                delta = (drag_delta.z - saved_position.z) * 2.0f;
                scaling.z = (length + delta) / length;
                break;
            default:
                break;
            }
            saved_position = drag_delta;
            TwoPointScreenLocation::Scale(scaling);
        }
    }
    else if (handle == START_HANDLE) {

        if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
                worldPos_x = saved_position.x + drag_delta.x;
                x2 = saved_point.x - drag_delta.x;
                break;
            case MSLAXIS::Y_AXIS:
                worldPos_y = saved_position.y + drag_delta.y;
                y2 = saved_point.y - drag_delta.y;
                break;
            case MSLAXIS::Z_AXIS:
                worldPos_z = saved_position.z + drag_delta.z;
                z2 = saved_point.z - drag_delta.z;
                break;
            default:
                break;
            }
        } else if (axis_tool == MSLTOOL::TOOL_ROTATE) {
            double angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            glm::vec3 end_pt = point2;
            glm::mat4 translateToOrigin = glm::translate(Identity, -origin);
            glm::mat4 translateBack = glm::translate(Identity, origin);
            glm::mat4 Rotate = Identity;

            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case MSLAXIS::Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case MSLAXIS::Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            break;
            default:
                break;
            }
            end_pt = glm::vec3(translateBack * Rotate * translateToOrigin* glm::vec4(end_pt, 1.0f));
            x2 = end_pt.x - worldPos_x;
            y2 = end_pt.y - worldPos_y;
            z2 = end_pt.z - worldPos_z;
            saved_angle = angle;
        }
    }
    else if (handle == END_HANDLE) {

        if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
                x2 = saved_point.x + drag_delta.x;
                break;
            case MSLAXIS::Y_AXIS:
                y2 = saved_point.y + drag_delta.y;
                break;
            case MSLAXIS::Z_AXIS:
                z2 = saved_point.z + drag_delta.z;
                break;
            default:
                break;
            }
        } else if (axis_tool == MSLTOOL::TOOL_ROTATE) {
            double angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            glm::vec3 start_pt = origin;
            glm::vec3 end_pt = point2;
            glm::mat4 translateToOrigin = glm::translate(Identity, -point2);
            glm::mat4 translateBack = glm::translate(Identity, point2);
            glm::mat4 Rotate = Identity;

            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case MSLAXIS::Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case MSLAXIS::Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            break;
            default:
                break;
            }
            start_pt = glm::vec3(translateBack * Rotate * translateToOrigin* glm::vec4(start_pt, 1.0f));
            worldPos_x = start_pt.x;
            worldPos_y = start_pt.y;
            worldPos_z = start_pt.z;
            x2 = end_pt.x - worldPos_x;
            y2 = end_pt.y - worldPos_y;
            z2 = end_pt.z - worldPos_z;
            saved_angle = angle;
        }
    }
    return 1;
}
int TwoPointScreenLocation::MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) {
    if (handle == CENTER_HANDLE) {
        constexpr float rscale = 10; //10 degrees per full 1.0 aka: max speed
        Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, rot.x * rscale);
        Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -rot.z * rscale);
        Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, rot.y * rscale);
        AddOffset(mov.x * scale, -mov.z * scale, mov.y * scale);
    } else if (handle == START_HANDLE) {
        worldPos_x += mov.x * scale;
        worldPos_y += -mov.z * scale;
        worldPos_z += mov.y * scale;
        
        x2 -= mov.x * scale;
        y2 -= -mov.z * scale;
        z2 -= mov.y * scale;
        
        
        glm::vec3 sp = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        glm::vec3 ep = glm::vec3(x2, y2, z2);
        
        glm::mat4 translateToOrigin = glm::translate(Identity, -sp);
        glm::mat4 translateBack = glm::translate(Identity, sp);

        glm::mat4 Rotate = glm::rotate(Identity, glm::radians(rot.x*10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        Rotate = glm::rotate(Rotate, glm::radians(-rot.z*10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        Rotate = glm::rotate(Rotate, glm::radians(rot.y*10.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        ep = glm::vec3(translateBack * Rotate * translateToOrigin* glm::vec4(ep, 1.0f));
        x2 = ep.x;
        y2 = ep.y;
        z2 = ep.z;
    } else if (handle == END_HANDLE) {
        x2 += mov.x * scale;
        y2 += -mov.z * scale;
        z2 += mov.y * scale;
        
        glm::vec3 sp = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        glm::vec3 ep = glm::vec3(x2, y2, z2);
        
        glm::mat4 translateToOrigin = glm::translate(Identity, -ep);
        glm::mat4 translateBack = glm::translate(Identity, ep);

        glm::mat4 Rotate = glm::rotate(Identity, glm::radians(rot.x*10.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        Rotate = glm::rotate(Rotate, glm::radians(-rot.z*10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        Rotate = glm::rotate(Rotate, glm::radians(rot.y*10.0f), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 sp2 = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(sp, 1.0f));
        worldPos_x = sp2.x;
        worldPos_y = sp2.y;
        worldPos_z = sp2.z;
        
        x2 += sp.x - worldPos_x;
        y2 += sp.y - worldPos_y;
        z2 += sp.z - worldPos_z;
    }
    return 1;
}

int TwoPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    float newx = ray_origin.x;
    float newy = ray_origin.y;

    if (ShiftKeyPressed) {
        if (handle == START_HANDLE) {
            if (std::abs(newx - point2.x) <= SNAP_RANGE) {
                newx = point2.x;
            }
            if (std::abs(newy - point2.y) <= SNAP_RANGE) {
                newy = point2.y;
            }
        }
        else if (handle == END_HANDLE) {
            if (std::abs(newx - worldPos_x) <= SNAP_RANGE) {
                newx = worldPos_x;
            }
            else if (std::abs(newy - worldPos_y) <= SNAP_RANGE) {
                newy = worldPos_y;
            }
        }
    }

    if (handle == START_HANDLE) {
        worldPos_x = newx;
        worldPos_y = newy;
        x2 = point2.x - worldPos_x;
        y2 = point2.y - worldPos_y;
    }
    else if (handle == END_HANDLE) {
        x2 = newx - worldPos_x;
        y2 = newy - worldPos_y;
    }

    return 0;
}

wxCursor TwoPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {
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

void TwoPointScreenLocation::AddDimensionProperties(wxPropertyGridInterface* propertyEditor, float factor) const
{
    wxPGProperty* prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Length (%s)", RulerObject::GetUnitDescription()), "RealLength", RulerObject::Measure(origin, point2)));
    //prop->ChangeFlag(wxPG_PROP_READONLY, true);
    prop->SetAttribute("Precision", 2);
    //prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
}

std::string TwoPointScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    return wxString::Format("Length %s", RulerObject::MeasureLengthDescription(origin, point2)).ToStdString();
}

void TwoPointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxBoolProperty("Locked", "Locked", _locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = propertyEditor->Append(new wxFloatProperty("WorldX", "WorldX", worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("WorldY", "WorldY", worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("WorldZ", "WorldZ", worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");

    prop->SetAttribute("UseCheckbox", 1);
    prop = propertyEditor->Append(new wxFloatProperty("X1", "ModelX1", worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = propertyEditor->Append(new wxFloatProperty("Y1", "ModelY1", worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = propertyEditor->Append(new wxFloatProperty("Z1", "ModelZ1", worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    prop = propertyEditor->Append(new wxFloatProperty("X2", "ModelX2", x2 + worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxBLUE);
    prop = propertyEditor->Append(new wxFloatProperty("Y2", "ModelY2", y2 + worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxBLUE);
    prop = propertyEditor->Append(new wxFloatProperty("Z2", "ModelZ2", z2 + worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxBLUE);
}

int TwoPointScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!_locked && "WorldX" == name) {
        worldPos_x = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldX");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::WorldX");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::WorldX");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::WorldX");
        return 0;
    }
    else if (_locked && "WorldX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "WorldY" == name) {
        worldPos_y = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldY");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::WorldY");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::WorldY");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::WorldY");
        return 0;
    }
    else if (_locked && "WorldY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "WorldZ" == name) {
        worldPos_z = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::WorldZ");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::WorldZ");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::WorldZ");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::WorldZ");
        return 0;
    }
    else if (_locked && "WorldZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelX1" == name) {
        float old_world_x = worldPos_x;
        worldPos_x = event.GetValue().GetDouble();
        x2 += old_world_x - worldPos_x;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelX1");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::ModelX1");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::ModelX1");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::ModelX1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ModelX1" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelY1" == name) {
        float old_world_y = worldPos_y;
        worldPos_y = event.GetValue().GetDouble();
        y2 += old_world_y - worldPos_y;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelY1");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::ModelY1");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::ModelY1");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::ModelY1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ModelY1" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelZ1" == name) {
        float old_world_z = worldPos_z;
        worldPos_z = event.GetValue().GetDouble();
        z2 += old_world_z - worldPos_z;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ1");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ1");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ1");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ1");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ModelZ1" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelX2" == name) {
        x2 = event.GetValue().GetDouble() - worldPos_x;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelX2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::ModelX2");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::ModelX2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::ModelX2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ModelX2" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelY2" == name) {
        y2 = event.GetValue().GetDouble() - worldPos_y;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelY2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::ModelY2");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::ModelY2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::ModelY2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ModelY2" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelZ2" == name) {
        z2 = event.GetValue().GetDouble() - worldPos_z;
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ2");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ2");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ2");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::ModelZ2");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ModelZ2" == name) {
        event.Veto();
        return 0;
    } else if (!_locked && "RealLength" == name) {
        auto origLen = RulerObject::UnMeasure(RulerObject::Measure(origin, point2));
        auto len = RulerObject::UnMeasure(event.GetValue().GetDouble());
        x2 = (x2 * len) / origLen;
        y2 = (y2 * len) / origLen;
        z2 = (z2 * len) / origLen;

        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::RealLength");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "TwoPointScreenLocation::OnPropertyGridChange::RealLength");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::RealLength");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::RealLength");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (_locked && "RealLength" == name) {
        event.Veto();
        return 0;
    } else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "TwoPointScreenLocation::OnPropertyGridChange::Locked");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "TwoPointScreenLocation::OnPropertyGridChange::Locked");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "TwoPointScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }

    return 0;
}

void TwoPointScreenLocation::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (_locked) return;
 
    if (angle.y != 0.0f) {
        float posx = GetHcenterPos();
        //float posy = GetVcenterPos();
        float posz = GetDcenterPos();

        float offset = angle.y;
        Rotate(MSLAXIS::Y_AXIS, -offset);
        rotate_point(position.x, position.z, glm::radians(-offset), posx, posz);
        SetHcenterPos(posx);
        SetDcenterPos(posz);
    }
    else {
        ModelScreenLocation::RotateAboutPoint(position, angle);
    }
}

void TwoPointScreenLocation::UpdateBoundingBox()
{
    glm::vec3 start = origin - glm::vec3(5, 5, 5);
    glm::vec3 end = point2 + glm::vec3(5, 5, 5);

    if (abs(start.x) < 4) {
        start.x = -5;
    }
    if (abs(start.y) < 4) {
        start.y = 5;
    }
    if (abs(start.z) < 4) {
        start.z = 5;
    }

    if (abs(end.x) < 4) {
        end.x = 5;
    }
    if (abs(end.y) < 4) {
        end.y = 5;
    }
    if (abs(end.z) < 4) {
        end.z = 5;
    }

    start = start - origin;
    end = end - origin;

    glm::vec4 c1(std::min(start.x, end.x), std::min(start.y, end.y), std::min(start.z, end.z), 1.0f);
    glm::vec4 c2(std::max(end.x, start.x), std::max(end.y, start.y), std::max(end.z, start.z), 1.0f);

    glm::mat4 mat;
    mat[0] = glm::vec4(1, 0, 0, 0);
    mat[1] = glm::vec4(0, 1, 0, 0);
    mat[2] = glm::vec4(0, 0, 1, 0);
    mat[3] = glm::vec4(0, 0, 0, 1);

    c1 = mat * c1;
    c2 = mat * c2;

    aabb_min = glm::vec3(c1.x, c1.y, c1.z);
    aabb_max = glm::vec3(c2.x, c2.y, c2.z);
}

bool TwoPointScreenLocation::Rotate(MSLAXIS axis, float factor) {
    if (_locked) return false;
    glm::vec3 start_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 end_pt = glm::vec3(x2 + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);
    glm::vec3 center(x2/2.0 + worldPos_x, y2/2.0 + worldPos_y, z2/2.0 + worldPos_z);
    glm::mat4 translateToOrigin = glm::translate(Identity, -center);
    glm::mat4 translateBack = glm::translate(Identity, center);
    glm::mat4 rot_mat = Identity;

    switch (axis) {
    case MSLAXIS::X_AXIS:
        rot_mat = glm::rotate(Identity, glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case MSLAXIS::Y_AXIS:
        rot_mat = glm::rotate(Identity, glm::radians(-factor), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case MSLAXIS::Z_AXIS:
        rot_mat = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    default:
        break;
    }
    start_pt = glm::vec3(translateBack * rot_mat * translateToOrigin* glm::vec4(start_pt, 1.0f));
    end_pt = glm::vec3(translateBack * rot_mat * translateToOrigin* glm::vec4(end_pt, 1.0f));
    worldPos_x = start_pt.x;
    worldPos_y = start_pt.y;
    worldPos_z = start_pt.z;
    x2 = end_pt.x - worldPos_x;
    y2 = end_pt.y - worldPos_y;
    z2 = end_pt.z - worldPos_z;
    return true;
}

bool TwoPointScreenLocation::Scale(const glm::vec3& factor) {
    if (_locked) return false;
    glm::vec3 start_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 end_pt = glm::vec3(x2 + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);
    glm::mat4 translateToOrigin = glm::translate(Identity, -center);
    glm::mat4 translateBack = glm::translate(Identity, center);
    glm::mat4 scalingMatrix = glm::scale(Identity, factor);
    start_pt = glm::vec3(translateBack * scalingMatrix * translateToOrigin * glm::vec4(start_pt, 1.0f));
    end_pt = glm::vec3(translateBack * scalingMatrix * translateToOrigin * glm::vec4(end_pt, 1.0f));
    worldPos_x = start_pt.x;
    worldPos_y = start_pt.y;
    worldPos_z = start_pt.z;
    x2 = end_pt.x - worldPos_x;
    y2 = end_pt.y - worldPos_y;
    z2 = end_pt.z - worldPos_z;
    return true;
}

void TwoPointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    aabb_min = glm::vec3(0.0f, -BB_OFF, -BB_OFF);
    aabb_max = glm::vec3(RenderWi * scalex, BB_OFF, BB_OFF);
}

glm::vec2 TwoPointScreenLocation::GetScreenOffset(ModelPreview* preview) const
{
    glm::vec2 position = VectorMath::GetScreenCoord(preview->getWidth(),
        preview->getHeight(),
        center,                          // X,Y,Z coords of the position when not transformed at all.
        preview->GetProjViewMatrix(),    // Projection / View matrix
        Identity                         // Transformation applied to the position
    );

    position.x = position.x / (float)preview->getWidth();
    position.y = position.y / (float)preview->getHeight();
    return position;
}

float TwoPointScreenLocation::GetHcenterPos() const {
    return worldPos_x + (x2 / 2.0f);
}

float TwoPointScreenLocation::GetVcenterPos() const {
    return worldPos_y + (y2 / 2.0f);
}

float TwoPointScreenLocation::GetDcenterPos() const {
    return worldPos_z + (z2 / 2.0f);
}

void TwoPointScreenLocation::SetHcenterPos(float f) {
    worldPos_x = f - (x2 / 2.0f);
}

void TwoPointScreenLocation::SetVcenterPos(float f) {
    worldPos_y = f - (y2 / 2.0f);
}

void TwoPointScreenLocation::SetDcenterPos(float f) {
    worldPos_z = f - (z2 / 2.0f);
}

void TwoPointScreenLocation::SetPosition(float posx, float posy) {

    if (_locked) return;

    float diffx = x2 / 2.0 - posx;
    float diffy = y2 / 2.0 - posy;

    worldPos_x -= diffx;
    worldPos_y -= diffy;
}

float TwoPointScreenLocation::GetTop() const {
    return std::max(worldPos_y, y2 + worldPos_y);
}

float TwoPointScreenLocation::GetLeft() const {
    return std::min(worldPos_x, x2 + worldPos_x);
}

float TwoPointScreenLocation::GetMWidth() const
{
    return x2;
}

float TwoPointScreenLocation::GetMHeight() const
{
    return y2;
}

float TwoPointScreenLocation::GetRight() const {
    return std::max(worldPos_x, x2 + worldPos_x);
}

float TwoPointScreenLocation::GetBottom() const {
    return std::min(worldPos_y, y2 + worldPos_y);
}

float TwoPointScreenLocation::GetFront() const {
    return std::max(worldPos_z, z2 + worldPos_z);
}

float TwoPointScreenLocation::GetBack() const {
    return std::min(worldPos_z, z2 + worldPos_z);
}

void TwoPointScreenLocation::SetTop(float i) {
    float newtop = i;
    if (y2 < 0) {
        worldPos_y = newtop;
    } else {
        worldPos_y = newtop - y2;
    }
}

void TwoPointScreenLocation::SetBottom(float i) {
    float newbot = i;
    if (y2 > 0) {
        worldPos_y = newbot;
    }
    else {
        worldPos_y = newbot - y2;
    }
}

void TwoPointScreenLocation::SetLeft(float i) {
    float newx = i;
    if (x2 > 0) {
        worldPos_x = newx;
    } else {
        worldPos_x = newx - x2;
    }
}

void TwoPointScreenLocation::SetRight(float i) {
    float newx = i;
    if (x2 < 0) {
        worldPos_x = newx;
    } else {
        worldPos_x = newx - x2;
    }
}

void TwoPointScreenLocation::SetFront(float i) {
    float newfront = i;
    if (z2 < 0) {
        worldPos_z = newfront;
    }
    else {
        worldPos_z = newfront - z2;
    }
}

void TwoPointScreenLocation::SetBack(float i) {
    float newback = i;
    if (z2 > 0) {
        worldPos_z = newback;
    }
    else {
        worldPos_z = newback - z2;
    }
}

void TwoPointScreenLocation::SetMWidth(float w)
{
    x2 = w;
}

void TwoPointScreenLocation::SetMDepth(float w)
{
}

float TwoPointScreenLocation::GetMDepth() const
{
    return 1.0;
}

void TwoPointScreenLocation::SetMHeight(float h)
{
    y2  = h;
}
