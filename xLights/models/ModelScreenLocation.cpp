/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "ModelScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <glm/glm.hpp>

#include "Model.h"
#include "../ModelPreview.h"
#include "../DrawGLUtils.h"
#include "Shapes.h"
#include "../ViewpointMgr.h"
#include "../support/VectorMath.h"
#include "UtilFunctions.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"

#include <log4cpp/Category.hh>

#define SNAP_RANGE                  5
#define BOUNDING_RECT_OFFSET        8

static float BB_OFF = 5.0f;

static glm::mat4 Identity(glm::mat4(1.0f));

static inline void TranslatePointDoubles(float radians,float x, float y,float &x1, float &y1) {
    float s = sin(radians);
    float c = cos(radians);
    x1 = c*x-(s*y);
    y1 = s*x+(c*y);
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"
static void rotate_point(float cx, float cy, float angle, float &x, float &y)
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

// used to print matrix when debugging
static void PrintMatrix(std::string name, glm::mat4& matrix)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Matrix Info: %s", name.c_str());
    logger_base.debug("Row 0: %6.8f  %6.8f  %6.8f  %6.2f", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]);
    logger_base.debug("Row 1: %6.8f  %6.8f  %6.8f  %6.2f", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]);
    logger_base.debug("Row 2: %6.8f  %6.8f  %6.8f  %6.2f", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
    logger_base.debug("Row 3: %6.2f  %6.2f  %6.2f  %6.2f", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
}

static glm::vec3 rotationMatrixToEulerAngles(const glm::mat3 &R)
{
    double x, y, z;
    int path = 0;

    double m13 = R[0][2];
    
    y = -asin(m13 < -1.0 ? -1.0 : m13 > 1 ? 1 : m13);

    if (abs(m13) < 0.9999) {

        x = atan2(R[1][2], R[2][2]);
        z = atan2(R[0][1], R[0][0]);

    }
    else {

        x = atan2(R[2][1], R[1][1]);
        z = 0;
    }

    return glm::vec3(x, y, z);
}

static void PrintRay(std::string name, glm::vec3& origin, glm::vec3& direction)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Ray Info: %s", name.c_str());
    logger_base.debug("Ray Origin: %6.2f  %6.2f  %6.2f", origin.x, origin.y, origin.z);
    logger_base.debug("Ray Direct: %6.2f  %6.2f  %6.2f", direction.x, direction.y, direction.z);
}
#pragma clang diagnostic pop


static wxCursor GetResizeCursor(int cornerIndex, int PreviewRotation) {
    int angleState;
    //LeftTop and RightBottom
    switch(cornerIndex) {
            // Left top when PreviewRotation = 0
        case L_TOP_HANDLE:
            angleState = (int)(PreviewRotation/22.5);
            break;
            // Right Top
        case R_TOP_HANDLE:
            angleState = ((int)(PreviewRotation/22.5)+4)%16;
            break;
            // Right Bottom
        case R_BOT_HANDLE:
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

glm::vec3 ModelScreenLocation::GetHandlePosition(int handle)
{
    if (handle < 0 || handle >= mHandlePosition.size()) return glm::vec3(0, 0, 0);
    return glm::vec3(mHandlePosition[handle].x, mHandlePosition[handle].y, mHandlePosition[handle].z);
}

ModelScreenLocation::ModelScreenLocation(int sz)
: RenderWi(0), RenderHt(0), RenderDp(0), previewW(-1), previewH(-1),
  worldPos_x(0.0f), worldPos_y(0.0f), worldPos_z(0.0f),
  scalex(1.0f), scaley(1.0f), scalez(1.0f), mHandlePosition(sz),
  active_handle_pos(glm::vec3(0.0f)), rotatex(0.0f), rotatey(0.0f), rotatez(0.0f),
  ModelMatrix(Identity), aabb_min(0.0f), aabb_max(0.0f), saved_intersect(0.0f),
  saved_position(0.0f), saved_size(0.0f), saved_scale(1.0f), saved_rotate(0.0f),
  active_handle(-1), highlighted_handle(-1), active_axis(-1), axis_tool(TOOL_TRANSLATE),
  supportsZScaling(false), _startOnXAxis(false), rotation_init(true), mouse_down(false)
{
    mSelectableHandles = 0;
    draw_3d = false;
    _locked = false;
}

void ModelScreenLocation::SetRenderSize(float NewWi, float NewHt, float NewDp) {
    RenderHt = NewHt;
    RenderWi = NewWi;
    RenderDp = NewDp;
}

// This function is used when the render size needs to be adjusted after a mesh is loaded during model creation
void ModelScreenLocation::AdjustRenderSize(float NewWi, float NewHt, float NewDp, wxXmlNode* node) {
    if ((NewWi != RenderWi || NewHt != RenderHt || NewDp != RenderDp) && NewWi != 1.0f) {
        RenderHt = NewHt;
        RenderWi = NewWi;
        RenderDp = NewDp;
        scalex = scaley = scalez = 1.0f;
        saved_scale = glm::vec3(scalex, scaley, scalez);
        saved_size = glm::vec3(RenderWi, RenderHt, RenderWi);
        node->DeleteAttribute("ScaleX");
        node->DeleteAttribute("ScaleY");
        node->DeleteAttribute("ScaleZ");
        node->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
        node->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
        node->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));
    }
    else {
        RenderHt = NewHt;
        RenderWi = NewWi;
        RenderDp = NewDp;
    }
}

void ModelScreenLocation::SetActiveAxis(int axis)
{
    if (axis_tool == TOOL_ROTATE && axis != -1) {
        active_axis = (axis + 1) % (NUM_TOOLS-1);
    }
    else {
        active_axis = axis;
    }
}

void ModelScreenLocation::SetActiveHandle(int handle)
{
    active_handle = handle;
    highlighted_handle = -1;
}

void ModelScreenLocation::MouseOverHandle(int handle)
{
    if (handle != active_handle) {
        highlighted_handle = handle;
    }
}

float ModelScreenLocation::GetAxisArrowLength(float zoom, int scale) const
{
    //static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    static float AXIS_ARROW_LENGTH = 60.0f;
    //logger_base.debug("zoom %f scale %d", zoom, scale);
    return std::max(AXIS_ARROW_LENGTH, AXIS_ARROW_LENGTH * zoom* scale);
}

float ModelScreenLocation::GetAxisHeadLength(float zoom, int scale) const
{
    static float AXIS_HEAD_LENGTH = 12.0f;
    return std::max(AXIS_HEAD_LENGTH, AXIS_HEAD_LENGTH * zoom* scale);
}

float ModelScreenLocation::GetAxisRadius(float zoom, int scale) const
{
    static float AXIS_RADIUS = 4.0f;
    return std::max(AXIS_RADIUS, AXIS_RADIUS * zoom* scale);
}

float ModelScreenLocation::GetRectHandleWidth(float zoom, int scale) const
{
    static float RECT_HANDLE_WIDTH = 6.0f;
    return std::max(RECT_HANDLE_WIDTH, RECT_HANDLE_WIDTH * zoom* scale);
}

void ModelScreenLocation::DrawAxisTool(glm::vec3& pos, DrawGLUtils::xl3Accumulator &va, float zoom, int scale) const
{
    int num_points = 18;
    float os = (float)GetRectHandleWidth(zoom, scale);

    if (axis_tool == TOOL_TRANSLATE) {
        xlColor ax1c = (highlighted_handle == 0x2000) ? xlYELLOW : xlRED;
        xlColor ax2c = (highlighted_handle == 0x2001) ? xlYELLOW : xlGREEN;
        xlColor ax3c = (highlighted_handle == 0x2002) ? xlYELLOW : xlBLUE;
        float tip = pos.x + GetAxisArrowLength(zoom, scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(tip, pos.y, pos.z, ax1c);
            va.AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), pos.z + GetAxisRadius(zoom,scale) * sin(2.0 * M_PI*u1), ax1c);
            va.AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), pos.z + GetAxisRadius(zoom,scale) * sin(2.0 * M_PI*u2), ax1c);
        }
        tip = pos.y + GetAxisArrowLength(zoom,scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(pos.x, tip, pos.z, ax2c);
            va.AddVertex(pos.x + GetAxisRadius(zoom,scale) * cos(2.0 * M_PI*u1), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), ax2c);
            va.AddVertex(pos.x + GetAxisRadius(zoom,scale) * cos(2.0 * M_PI*u2), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), ax2c);
        }
        tip = pos.z + GetAxisArrowLength(zoom, scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(pos.x, pos.y, tip, ax3c);
            va.AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), pos.y + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), tip - GetAxisHeadLength(zoom, scale), ax3c);
            va.AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), pos.y + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), tip - GetAxisHeadLength(zoom, scale), ax3c);
        }
        va.Finish(GL_TRIANGLES);
    }
    else if (axis_tool == TOOL_SCALE) {
        xlColor ax1c = (highlighted_handle == 0x2000) ? xlYELLOW : xlRED;
        xlColor ax2c = (highlighted_handle == 0x2001) ? xlYELLOW : xlGREEN;
        xlColor ax3c = (highlighted_handle == 0x2002) ? xlYELLOW : xlBLUE;
        DrawGLUtils::DrawCube(pos.x + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.y, pos.z, GetAxisRadius(zoom, scale) * 2, ax1c, va);
        DrawGLUtils::DrawCube(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, GetAxisRadius(zoom, scale) * 2, ax2c, va);
        DrawGLUtils::DrawCube(pos.x, pos.y, pos.z + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), GetAxisRadius(zoom, scale) * 2, ax3c, va);
        va.Finish(GL_TRIANGLES);
    }
    else if (axis_tool == TOOL_ROTATE) {
        xlColor ax1c = (highlighted_handle == 0x2000) ? xlYELLOW : xlGREEN;
        xlColor ax2c = (highlighted_handle == 0x2001) ? xlYELLOW : xlBLUE;
        xlColor ax3c = (highlighted_handle == 0x2002) ? xlYELLOW : xlRED;
        float radius = GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale);
        for (size_t i = 30; i <= 150; i += 5) {
            float u1 = (float)i;
            float u2 = (float)(i + 5);
            va.AddVertex(pos.x, pos.y + radius * cos(glm::radians(u1)), pos.z + radius * sin(glm::radians(u1)), xlRED);
            va.AddVertex(pos.x, pos.y + radius * cos(glm::radians(u2)), pos.z + radius * sin(glm::radians(u2)), xlRED);
        }
        for (size_t i = 0; i <= 120; i += 5) {
            float u1 = (float)i - 60.0f;;
            float u2 = (float)(i + 5) - 60.0f;
            va.AddVertex(pos.x + radius * cos(glm::radians(u1)), pos.y, pos.z + radius * sin(glm::radians(u1)), xlGREEN);
            va.AddVertex(pos.x + radius * cos(glm::radians(u2)), pos.y, pos.z + radius * sin(glm::radians(u2)), xlGREEN);
        }
        for (size_t i = 30; i <= 150; i += 5) {
            float u1 = (float)i;
            float u2 = (float)(i + 5);
            va.AddVertex(pos.x + radius * cos(glm::radians(u1)), pos.y + radius * sin(glm::radians(u1)), pos.z, xlBLUE);
            va.AddVertex(pos.x + radius * cos(glm::radians(u2)), pos.y + radius * sin(glm::radians(u2)), pos.z, xlBLUE);
        }
        va.Finish(GL_LINES);

        DrawGLUtils::DrawCube(pos.x + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom,scale), pos.y, pos.z, GetAxisRadius(zoom, scale) * 2, ax1c, va);
        DrawGLUtils::DrawCube(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, GetAxisRadius(zoom, scale) * 2, ax2c, va);
        DrawGLUtils::DrawCube(pos.x, pos.y, pos.z + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), GetAxisRadius(zoom, scale) * 2, ax3c, va);
        va.Finish(GL_TRIANGLES);
    }
    else if (axis_tool == TOOL_XY_TRANS) {
        xlColor a1c = (highlighted_handle == 0x2000) ? xlYELLOW : xlRED;
        xlColor a2c = (highlighted_handle == 0x2000) ? xlYELLOW : xlGREEN;
        float arrow_length = GetAxisArrowLength(zoom, scale) / 2.0f;
        va.AddVertex(pos.x, pos.y, pos.z, xlGREEN);
        va.AddVertex(pos.x, pos.y + arrow_length, pos.z, xlGREEN);
        va.AddVertex(pos.x, pos.y, pos.z, xlGREEN);
        va.AddVertex(pos.x, pos.y - arrow_length, pos.z, xlGREEN);
        va.AddVertex(pos.x, pos.y, pos.z, xlRED);
        va.AddVertex(pos.x + arrow_length, pos.y, pos.z, xlRED);
        va.AddVertex(pos.x, pos.y, pos.z, xlRED);
        va.AddVertex(pos.x - arrow_length, pos.y, pos.z, xlRED);
        va.Finish(GL_LINES);
        float tip = pos.x + arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(tip, pos.y, pos.z, a1c);
            va.AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a1c);
            va.AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a1c);
        }
        tip = pos.x - arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(tip, pos.y, pos.z, a1c);
            va.AddVertex(tip + GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a1c);
            va.AddVertex(tip + GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a1c);
        }
        tip = pos.y + arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(pos.x, tip, pos.z, a2c);
            va.AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a2c);
            va.AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a2c);
        }
        tip = pos.y - arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(pos.x, tip, pos.z, a2c);
            va.AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), tip + GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a2c);
            va.AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), tip + GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a2c);
        }
        va.Finish(GL_TRIANGLES);
    }

    if (axis_tool == TOOL_TRANSLATE || axis_tool == TOOL_SCALE) {
        va.AddVertex(pos.x + os, pos.y, pos.z, xlRED);
        va.AddVertex(pos.x + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.y, pos.z, xlRED);
        va.AddVertex(pos.x, pos.y + os, pos.z, xlGREEN);
        va.AddVertex(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, xlGREEN);
        va.AddVertex(pos.x, pos.y, pos.z + os, xlBLUE);
        va.AddVertex(pos.x, pos.y, pos.z + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), xlBLUE);
        va.Finish(GL_LINES);
    }
}

void ModelScreenLocation::AddOffset(float deltax, float deltay, float deltaz) {

    if (_locked) return;

    worldPos_x += deltax;
    worldPos_y += deltay;
    worldPos_z += deltaz;
}

void ModelScreenLocation::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (_locked) return;

    float posx = GetHcenterPos();
    float posy = GetVcenterPos();
    float posz = GetDcenterPos();

    if( angle.y != 0.0f ) {
        float offset = angle.y;
        Rotate(Y_AXIS, offset);
        rotate_point(position.x, position.z, glm::radians(-offset), posx, posz);
        SetHcenterPos(posx);
        SetDcenterPos(posz);
    }
    else if( angle.x != 0.0f ) {
        float offset = angle.x;
        Rotate(X_AXIS, offset);
        rotate_point(position.y, position.z, glm::radians(offset), posy, posz);
        SetVcenterPos(posy);
        SetDcenterPos(posz);
    }
    else if( angle.z != 0.0f ) {
        float offset = angle.z;
        Rotate(Z_AXIS, offset);
        rotate_point(position.x, position.y, glm::radians(offset), posx, posy);
        SetHcenterPos(posx);
        SetVcenterPos(posy);
    }
}

glm::vec2 ModelScreenLocation::GetScreenPosition(int screenwidth, int screenheight, ModelPreview* preview,  PreviewCamera* camera, float &sx, float &sy, float &sz) const
{
    glm::vec2 position = VectorMath::GetScreenCoord(screenwidth,
        screenheight,
        glm::vec3(sx, sy, sz),                                // X,Y,Z coords of the position when not transformed at all.
        preview->GetProjMatrix() * camera->GetViewMatrix(),    // Projection / View matrix
        Identity                                              // Points must be pre-translated
    );
    return position;
}

void ModelScreenLocation::DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va) const
{
    DrawGLUtils::DrawBoundingBox(c, aabb_min, aabb_max, ModelMatrix, va);
}

void ModelScreenLocation::TranslateVector(glm::vec3& point) const
{
    float sx = point.x;
    float sy = point.y;
    float sz = point.z;
    TranslatePoint(sx, sy, sz);
    point.x = sx;
    point.y = sy;
    point.z = sz;
}

void ModelScreenLocation::AddASAPWork(uint32_t work, const std::string& from)
{
    xLightsApp::GetFrame()->GetOutputModelManager()->AddASAPWork(work, from);
}

void ModelScreenLocation::SetDefaultMatrices() const
{
    TranslateMatrix = glm::translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    ModelMatrix = TranslateMatrix;
}

bool ModelScreenLocation::DragHandle(ModelPreview* preview, int mouseX, int mouseY, bool latch) {

    static log4cpp::Category& logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    float zoom = preview->GetCameraZoomForHandles();
    int scale = preview->GetHandleScale();

    if (latch) {
        saved_scale = glm::vec3(scalex, scaley, scalez);
        saved_size = glm::vec3(RenderWi, RenderHt, RenderWi);
        saved_rotate = glm::vec3(rotatex, rotatey, rotatez);
    }

    //Get a world position for the mouse
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    glm::vec3 point(0.0f);
    glm::vec3 normal(0.0f);
    glm::vec3 intersect(0.0f);

    if (axis_tool == TOOL_ROTATE) {
        switch (active_axis)
        {
        case X_AXIS:
            normal = glm::vec3(saved_position.x + GetAxisArrowLength(zoom, scale), 0.0f, 0.0f);
            point = glm::vec3(saved_position.x, 0.0f, 0.0f);
            break;
        case Y_AXIS:
            normal = glm::vec3(0.0f, saved_position.y + GetAxisArrowLength(zoom, scale), 0.0f);
            point = glm::vec3(0.0f, saved_position.y, 0.0f);
            break;
        case Z_AXIS:
            normal = glm::vec3(0.0f, 0.0f, saved_position.z + GetAxisArrowLength(zoom, scale));
            point = glm::vec3(0.0f, 0.0f, saved_position.z);
            break;
        default:
            wxASSERT(false);
            break;
        }
    }
    else {
        switch (active_axis)
        {
        case Z_AXIS:
            normal = glm::vec3(0.0f, saved_position.y + GetAxisArrowLength(zoom, scale), 0.0f);
            point = glm::vec3(0.0f, saved_position.y, 0.0f);
            break;
        case X_AXIS:
        case Y_AXIS:
            normal = glm::vec3(0.0f, 0.0f, saved_position.z + GetAxisArrowLength(zoom, scale));
            point = glm::vec3(0.0f, 0.0f, saved_position.z);
            break;
        default:
            wxASSERT(false);
            break;
        }
    }

    bool found = VectorMath::GetPlaneIntersect(
        ray_origin,         // Point origin  (x0, y0, z0)
        ray_direction,      // Ray direction (x,  y,  z)
        point,              // Point on the plane
        normal,             // Normal to the plane
        intersect);         // Output: intersect point

    drag_delta = glm::vec3(0.0f);

    if (found) {
        if (latch) {
            saved_intersect = intersect;
        }
        else {
            drag_delta = intersect - saved_intersect;
        }
    }
    else {
        logger_base.warn("MoveHandle3D: Intersect not found!");
    }
    return found;
}

wxCursor ModelScreenLocation::CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const
{
    handle = NO_HANDLE;

    if (_locked)
    {
        return wxCURSOR_DEFAULT;
    }

    wxCursor return_value = CheckIfOverAxisHandles3D(ray_origin, ray_direction, handle, zoom, scale);

    if (handle == NO_HANDLE) {
        float distance = 1000000000.0f;

        // Test each each Oriented Bounding Box (OBB).
        for (size_t i = 0; i < mSelectableHandles; i++)
        {
            float intersection_distance; // Output of TestRayOBBIntersection()

            if (VectorMath::TestRayOBBIntersection(
                ray_origin,
                ray_direction,
                handle_aabb_min[i],
                handle_aabb_max[i],
                ModelMatrix,
                intersection_distance)
                ) {
                if (intersection_distance < distance) {
                    distance = intersection_distance;
                    handle = i;
                    return_value = wxCURSOR_HAND;
                }
            }
        }
    }
    return return_value;
}

wxCursor ModelScreenLocation::CheckIfOverAxisHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const
{
    wxCursor return_value = wxCURSOR_DEFAULT;

    float distance = 1000000000.0f;
    handle = NO_HANDLE;

    // test for a selected axis first
    int num_axis_handles = (axis_tool == TOOL_XY_TRANS) ? 1 : 3;
    glm::vec3 axisbb_min[3];
    glm::vec3 axisbb_max[3];
    if (IsXYTransHandle()) {
        axisbb_min[0].x = active_handle_pos.x - ModelMatrix[3][0] - GetAxisArrowLength(zoom, scale) /2 - 3;
        axisbb_min[0].y = active_handle_pos.y - ModelMatrix[3][1] - GetAxisArrowLength(zoom, scale) /2 - 3;
        axisbb_min[0].z = active_handle_pos.z - ModelMatrix[3][2] - GetAxisArrowLength(zoom, scale) /2 - 3;
        axisbb_max[0].x = active_handle_pos.x - ModelMatrix[3][0] + GetAxisArrowLength(zoom, scale) /2 + 3;
        axisbb_max[0].y = active_handle_pos.y - ModelMatrix[3][1] + GetAxisArrowLength(zoom, scale) /2 + 3;
        axisbb_max[0].z = active_handle_pos.z - ModelMatrix[3][2] + GetAxisArrowLength(zoom, scale) /2 + 3;
    }
    else {
        axisbb_min[0].x = active_handle_pos.x - ModelMatrix[3][0] + GetAxisArrowLength(zoom, scale) - GetAxisHeadLength(zoom, scale) - 3;
        axisbb_min[0].y = active_handle_pos.y - ModelMatrix[3][1] - GetAxisRadius(zoom, scale);
        axisbb_min[0].z = active_handle_pos.z - ModelMatrix[3][2] - GetAxisRadius(zoom, scale);
        axisbb_min[1].x = active_handle_pos.x - ModelMatrix[3][0] - GetAxisRadius(zoom, scale);
        axisbb_min[1].y = active_handle_pos.y - ModelMatrix[3][1] + GetAxisArrowLength(zoom, scale) - GetAxisHeadLength(zoom, scale) - 3;
        axisbb_min[1].z = active_handle_pos.z - ModelMatrix[3][2] - GetAxisRadius(zoom, scale);
        axisbb_min[2].x = active_handle_pos.x - ModelMatrix[3][0] - GetAxisRadius(zoom, scale);
        axisbb_min[2].y = active_handle_pos.y - ModelMatrix[3][1] - GetAxisRadius(zoom, scale);
        axisbb_min[2].z = active_handle_pos.z - ModelMatrix[3][2] + GetAxisArrowLength(zoom, scale) - GetAxisHeadLength(zoom, scale) - 3;
        axisbb_max[0].x = active_handle_pos.x - ModelMatrix[3][0] + GetAxisArrowLength(zoom, scale) + 3;
        axisbb_max[0].y = active_handle_pos.y - ModelMatrix[3][1] + GetAxisRadius(zoom, scale);
        axisbb_max[0].z = active_handle_pos.z - ModelMatrix[3][2] + GetAxisRadius(zoom, scale);
        axisbb_max[1].x = active_handle_pos.x - ModelMatrix[3][0] + GetAxisRadius(zoom, scale);
        axisbb_max[1].y = active_handle_pos.y - ModelMatrix[3][1] + GetAxisArrowLength(zoom, scale) + 3;
        axisbb_max[1].z = active_handle_pos.z - ModelMatrix[3][2] + GetAxisRadius(zoom, scale);
        axisbb_max[2].x = active_handle_pos.x - ModelMatrix[3][0] + GetAxisRadius(zoom, scale);
        axisbb_max[2].y = active_handle_pos.y - ModelMatrix[3][1] + GetAxisRadius(zoom, scale);
        axisbb_max[2].z = active_handle_pos.z - ModelMatrix[3][2] + GetAxisArrowLength(zoom, scale) + 3;
    }

    // see if an axis handle is selected
    for (size_t i = 0; i < num_axis_handles; i++)
    {
        float intersection_distance; // Output of TestRayOBBIntersection()

        if (VectorMath::TestRayOBBIntersection(
            ray_origin,
            ray_direction,
            axisbb_min[i],
            axisbb_max[i],
            TranslateMatrix,      // axis is not rotated
            intersection_distance)
            ) {
            if (intersection_distance < distance) {
                distance = intersection_distance;
                handle = i | 0x2000;
                return_value = wxCURSOR_HAND;
            }
        }
    }

    return return_value;
}

bool ModelScreenLocation::HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const
{
    if (VectorMath::TestRayOBBIntersection(
        ray_origin,
        ray_direction,
        aabb_min,
        aabb_max,
        ModelMatrix,
        intersection_distance)
        ) {
            return true;
        }
    return false;
}

void ModelScreenLocation::UpdateBoundingBox(float width, float height, float depth)
{
    // scale the bounding box for selection logic
    aabb_max.x = width / 2.0f * scalex;
    aabb_max.y = height / 2.0f * scaley;
    aabb_max.z = depth / 2.0f * scalez;
    aabb_min.x = -aabb_max.x;
    aabb_min.y = -aabb_max.y;
    aabb_min.z = -aabb_max.z;

    // Set minimum bounding rectangle
    if (aabb_max.y - aabb_min.y < 4) {
        aabb_max.y += 5;
        aabb_min.y -= 5;
    }
    if (aabb_max.x - aabb_min.x < 4) {
        aabb_max.x += 5;
        aabb_min.x -= 5;
    }
    if (aabb_max.z - aabb_min.z < 4) {
        aabb_max.z += 5;
        aabb_min.z -= 5;
    }

}

BoxedScreenLocation::BoxedScreenLocation()
: ModelScreenLocation(10), perspective(0.0f), centerx(0.0), centery(0.0), centerz(0.0)
{
    mSelectableHandles = 1;
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
}

int BoxedScreenLocation::CheckUpgrade(wxXmlNode *node)
{
    // check for upgrade to world positioning
    int version = wxAtoi(node->GetAttribute("versionNumber", "0"));
    if (version < 2) {
        // skip first upgrade call since preview size is not set
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", "2");
        return UPGRADE_SKIPPED;
    }
    else if (version == 2) {
        if (node->HasAttribute("offsetXpct")) {
            float offsetXpct = wxAtof(node->GetAttribute("offsetXpct", "0"));
            float offsetYpct = wxAtof(node->GetAttribute("offsetYpct", "0"));
            float previewScaleX = wxAtof(node->GetAttribute("PreviewScaleX", "0"));
            float previewScaleY = wxAtof(node->GetAttribute("PreviewScaleY", "0"));
            worldPos_x = previewW * offsetXpct;
            worldPos_y = previewH * offsetYpct;
            worldPos_z = 0.0f;
            scalex = previewW / RenderWi * previewScaleX;
            scaley = previewH / RenderHt * previewScaleY;
            scalez = scaley;
            rotatex = 0.0f;
            rotatey = 0.0f;
            rotatez = wxAtof(node->GetAttribute("PreviewRotation", "0.0f"));
            node->DeleteAttribute("offsetXpct");
            node->DeleteAttribute("offsetYpct");
            node->DeleteAttribute("PreviewScaleX");
            node->DeleteAttribute("PreviewScaleY");
            node->DeleteAttribute("PreviewRotation");
            node->DeleteAttribute("WorldPosX");
            node->DeleteAttribute("WorldPosY");
            node->DeleteAttribute("WorldPosZ");
            node->DeleteAttribute("ScaleX");
            node->DeleteAttribute("ScaleY");
            node->DeleteAttribute("ScaleZ");
            node->DeleteAttribute("RotateX");
            node->DeleteAttribute("RotateY");
            node->DeleteAttribute("RotateZ");
            node->AddAttribute("WorldPosX", wxString::Format("%6.4f", worldPos_x));
            node->AddAttribute("WorldPosY", wxString::Format("%6.4f", worldPos_y));
            node->AddAttribute("WorldPosZ", wxString::Format("%6.4f", worldPos_z));
            node->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
            node->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
            node->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));
            node->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
            node->AddAttribute("RotateY", wxString::Format("%4.8f", rotatey));
            node->AddAttribute("RotateZ", wxString::Format("%4.8f", rotatez));
            node->DeleteAttribute("versionNumber");
            node->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
            glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
            rotate_quat = glm::quat_cast(rz * ry * rx);
            rotation_init = false;
        }
        return UPGRADE_EXEC_DONE;
    }
    else if (version == 3) {
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
        rotatex = -wxAtof(node->GetAttribute("RotateX", "0.0f"));
        rotatey = -wxAtof(node->GetAttribute("RotateY", "0.0f"));
        rotatez = wxAtof(node->GetAttribute("RotateZ", "0.0f"));
        node->DeleteAttribute("RotateX");
        node->DeleteAttribute("RotateY");
        node->DeleteAttribute("RotateZ");
        node->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
        node->AddAttribute("RotateY", wxString::Format("%4.8f", rotatey));
        node->AddAttribute("RotateZ", wxString::Format("%4.8f", rotatez));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        rotate_quat = glm::quat_cast(rx * ry * rz);
        rotation_init = false;
        return UPGRADE_NOT_NEEDED;
    }
    return UPGRADE_NOT_NEEDED;
}

void BoxedScreenLocation::Read(wxXmlNode *ModelNode) {
    int upgrade_result = CheckUpgrade(ModelNode);
    if (upgrade_result == UPGRADE_NOT_NEEDED) {
        worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "200.0"));
        worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
        worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));

        scalex = wxAtof(ModelNode->GetAttribute("ScaleX", "1.0"));
        scaley = wxAtof(ModelNode->GetAttribute("ScaleY", "1.0"));
        scalez = wxAtof(ModelNode->GetAttribute("ScaleZ", "1.0"));

        if (scalex < 0) {
            scalex = 1.0f;
        }
        if (scaley < 0) {
            scaley = 1.0f;
        }
        if (scalez < 0) {
            scalez = 1.0f;
        }

        rotatex = wxAtof(ModelNode->GetAttribute("RotateX", "0.0f"));
        rotatey = wxAtof(ModelNode->GetAttribute("RotateY", "0.0f"));
        rotatez = wxAtof(ModelNode->GetAttribute("RotateZ", "0.0f"));

        if (rotatex < -180.0f || rotatex > 180.0f) {
            rotatex = 0.0f;
        }
        if (rotatey < -180.0f || rotatey > 180.0f) {
            rotatey = 0.0f;
        }
        if (rotatez < -180.0f || rotatez > 180.0f) {
            rotatez = 0.0f;
        }
        if (rotation_init) {
            glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
            rotate_quat = glm::quat_cast(rz * ry * rx);
            rotation_init = false;
        }

        _locked = (wxAtoi(ModelNode->GetAttribute("Locked", "0")) == 1);
    }
}

void BoxedScreenLocation::Write(wxXmlNode *ModelXml) {
    ModelXml->DeleteAttribute("WorldPosX");
    ModelXml->DeleteAttribute("WorldPosY");
    ModelXml->DeleteAttribute("WorldPosZ");
    ModelXml->DeleteAttribute("ScaleX");
    ModelXml->DeleteAttribute("ScaleY");
    ModelXml->DeleteAttribute("ScaleZ");
    ModelXml->DeleteAttribute("RotateX");
    ModelXml->DeleteAttribute("RotateY");
    ModelXml->DeleteAttribute("RotateZ");
    ModelXml->DeleteAttribute("Locked");
    ModelXml->AddAttribute("WorldPosX", wxString::Format("%6.4f", worldPos_x));
    ModelXml->AddAttribute("WorldPosY", wxString::Format("%6.4f", worldPos_y));
    ModelXml->AddAttribute("WorldPosZ", wxString::Format("%6.4f", worldPos_z));
    ModelXml->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
    ModelXml->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
    ModelXml->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));
    ModelXml->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
    ModelXml->AddAttribute("RotateY", wxString::Format("%4.8f", rotatey));
    ModelXml->AddAttribute("RotateZ", wxString::Format("%4.8f", rotatez));
    if (_locked)
    {
        ModelXml->AddAttribute("Locked", "1");
    }
}

void BoxedScreenLocation::TranslatePoint(float &sx, float &sy, float &sz) const {
    sx = (sx*scalex);
    sy = (sy*scaley);
	sz = (sz*scalez);
    glm::vec4 v = rotate_quat * glm::vec4(glm::vec3(sx, sy, sz), 1.0f);
    sx = v.x; sy = v.y; sz = v.z;

    // Give 2D tree model its perspective
    if (!draw_3d) {
        glm::vec4 position = glm::vec4(glm::vec3(sx, sy, sz), 1.0);
        glm::mat4 rm = glm::rotate(Identity, perspective, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec4 model_position = rm * position;
        sx = model_position.x;
        sy = model_position.y;
        sz = model_position.z;
    }

    sx += worldPos_x;
    sy += worldPos_y;
	sz += worldPos_z;
}

bool BoxedScreenLocation::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const {
    int xs = x1<x2?x1:x2;
    int xf = x1>x2?x1:x2;
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;

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
        } else {
            return false;
        }
    }
}

bool BoxedScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {
    // NOTE:  This routine is designed for the 2D layout model selection only
    bool return_value = false;

    if (VectorMath::TestRayOBBIntersection2D(
        ray_origin,
        aabb_min,
        aabb_max,
        ModelMatrix)
        ) {
        return_value = true;
    }

    return return_value;
}

wxCursor BoxedScreenLocation::CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const
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

    int hw = GetRectHandleWidth(preview->GetCameraZoomForHandles(), preview->GetHandleScale());

    int num_handles = 5;
    glm::vec3 aabb_min[5];
    glm::vec3 aabb_max[5];

    for (size_t h = 0; h < num_handles; h++) {
        aabb_min[h].x = mHandlePosition[h+1].x - hw;
        aabb_min[h].y = mHandlePosition[h+1].y - hw;
        aabb_min[h].z = mHandlePosition[h+1].z - hw;
        aabb_max[h].x = mHandlePosition[h+1].x + hw;
        aabb_max[h].y = mHandlePosition[h+1].y + hw;
        aabb_max[h].z = mHandlePosition[h+1].z + hw;
    }

    // Test each each Oriented Bounding Box (OBB).
    for (size_t i = 0; i < num_handles; i++)
    {
        if (VectorMath::TestRayOBBIntersection2D(
            ray_origin,
            aabb_min[i],
            aabb_max[i],
            Identity)
            ) {
            handle = i + 1;
            break;
        }
    }

    if (handle == NO_HANDLE) {
        return wxCURSOR_DEFAULT;
    }
    else if (handle == ROTATE_HANDLE) {
        return wxCURSOR_HAND;
    }
    else {
        return GetResizeCursor(handle, rotatez);
    }
}

wxCursor BoxedScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {
    if (preview != nullptr) {
        if (preview->Is3D()) {
            if (supportsZScaling && !_startOnXAxis) {
                // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
                // of the mouse click with the ground plane
                active_axis = Z_AXIS;
                saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
                DragHandle(preview, x, y, true);
                worldPos_x = saved_intersect.x;
                worldPos_y = RenderHt / 2.0f;
                worldPos_z = saved_intersect.z;
                handle = CENTER_HANDLE;
                active_axis = Y_AXIS;
            }
            else {
                active_axis = X_AXIS;
                saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
                DragHandle(preview, x, y, true);
                worldPos_x = saved_intersect.x;
                worldPos_y = saved_intersect.y;
                worldPos_z = 0.0f;
                handle = CENTER_HANDLE;
            }
        }
        else {
            handle = R_BOT_HANDLE;
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            active_axis = Y_AXIS;
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = saved_intersect.y;
            worldPos_z = 0.0f;
            centery = worldPos_y;
            centerx = worldPos_x;
            centerz = worldPos_z;
            scalex = scaley = scalez = 0.0f;
        }
    }
    else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }
    return wxCURSOR_SIZING;
}

void BoxedScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    if (Nodes.size() > 0) {
        aabb_min = glm::vec3(100000.0f, 100000.0f, 100000.0f);
        aabb_max = glm::vec3(0.0f, 0.0f, 0.0f);

        for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
            for (auto coord = it->get()->Coords.begin(); coord != it->get()->Coords.end(); ++coord) {

                float sx = coord->screenX;
                float sy = coord->screenY;
                float sz = coord->screenZ;

                //aabb vectors need to be the untranslated / unrotated limits
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
        // scale the bounding box for selection logic
        aabb_min.x = aabb_min.x * scalex;
        aabb_min.y = aabb_min.y * scaley;
        aabb_min.z = aabb_min.z * scalez;
        aabb_max.x = aabb_max.x * scalex;
        aabb_max.y = aabb_max.y * scaley;
        aabb_max.z = aabb_max.z * scalez;

        // Set minimum bounding rectangle
        if (aabb_max.y - aabb_min.y < 4) {
            aabb_max.y += 5;
            aabb_min.y -= 5;
        }
        if (aabb_max.x - aabb_min.x < 4) {
            aabb_max.x += 5;
            aabb_min.x -= 5;
        }
        if (aabb_max.z - aabb_min.z < 4) {
            aabb_max.z += 5;
            aabb_min.z -= 5;
        }
    }
}

void BoxedScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    centerx = worldPos_x;
    centery = worldPos_y;
    centerz = worldPos_z;
    draw_3d = is_3d;
    if (allow_selected) {

        glm::mat4 Translate = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
        glm::mat4 RotationMatrix = glm::toMat4(rotate_quat);
        ModelMatrix = Translate * RotationMatrix;
        TranslateMatrix = Translate;
    }
}

void BoxedScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va, float zoom, int scale) const {
    va.PreAlloc(32 * 5);

    float sz1 = RenderDp / 2;
    float sz2 =  -RenderDp / 2;

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (_locked)
    {
        handleColor = xlREDTRANSLUCENT;
    }

    // Upper Left Handle
    float sx1 = (-RenderWi / 2) - BOUNDING_RECT_OFFSET / scalex;
    float sy1 = (RenderHt / 2) + BOUNDING_RECT_OFFSET / scaley;
    float sx = sx1;
    float sy = sy1;
    float sz = sz1;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[L_TOP_HANDLE].x = sx;
    mHandlePosition[L_TOP_HANDLE].y = sy;
    mHandlePosition[L_TOP_HANDLE].z = sz;
    sx = sx1;
    sy = sy1;
    sz = sz2;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[L_TOP_HANDLE_Z].x = sx;
    mHandlePosition[L_TOP_HANDLE_Z].y = sy;
    mHandlePosition[L_TOP_HANDLE_Z].z = sz;

    // Upper Right Handle
    sx1 = (RenderWi / 2) + BOUNDING_RECT_OFFSET / scalex;
    sy1 = (RenderHt / 2) + BOUNDING_RECT_OFFSET / scaley;
    sx = sx1;
    sy = sy1;
    sz = sz1;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[R_TOP_HANDLE].x = sx;
    mHandlePosition[R_TOP_HANDLE].y = sy;
    mHandlePosition[R_TOP_HANDLE].z = sz;
    sx = sx1;
    sy = sy1;
    sz = sz2;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[R_TOP_HANDLE_Z].x = sx;
    mHandlePosition[R_TOP_HANDLE_Z].y = sy;
    mHandlePosition[R_TOP_HANDLE_Z].z = sz;

    // Lower Right Handle
    sx1 = (RenderWi / 2) + BOUNDING_RECT_OFFSET / scalex;
    sy1 = (-RenderHt / 2) - BOUNDING_RECT_OFFSET / scaley;
    sx = sx1;
    sy = sy1;
    sz = sz1;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[R_BOT_HANDLE].x = sx;
    mHandlePosition[R_BOT_HANDLE].y = sy;
    mHandlePosition[R_BOT_HANDLE].z = sz;
    sx = sx1;
    sy = sy1;
    sz = sz2;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[R_BOT_HANDLE_Z].x = sx;
    mHandlePosition[R_BOT_HANDLE_Z].y = sy;
    mHandlePosition[R_BOT_HANDLE_Z].z = sz;

    // Lower Left Handle
    sx1 = (-RenderWi / 2) - BOUNDING_RECT_OFFSET / scalex;
    sy1 = (-RenderHt / 2) - BOUNDING_RECT_OFFSET / scaley;
    sx = sx1;
    sy = sy1;
    sz = sz1;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[L_BOT_HANDLE].x = sx;
    mHandlePosition[L_BOT_HANDLE].y = sy;
    mHandlePosition[L_BOT_HANDLE].z = sz;
    sx = sx1;
    sy = sy1;
    sz = sz2;
    TranslatePoint(sx, sy, sz);
    mHandlePosition[L_BOT_HANDLE_Z].x = sx;
    mHandlePosition[L_BOT_HANDLE_Z].y = sy;
    mHandlePosition[L_BOT_HANDLE + 5].z = sz;

    // Center Handle
    handle_aabb_min[CENTER_HANDLE].x = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[CENTER_HANDLE].y = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[CENTER_HANDLE].z = -GetRectHandleWidth(zoom, scale);
    handle_aabb_max[CENTER_HANDLE].x = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[CENTER_HANDLE].y = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[CENTER_HANDLE].z = GetRectHandleWidth(zoom, scale);
    mHandlePosition[CENTER_HANDLE].x = worldPos_x;
    mHandlePosition[CENTER_HANDLE].y = worldPos_y;
    mHandlePosition[CENTER_HANDLE].z = worldPos_z;

    LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));

    xlColor Box3dColor = xlWHITE;
    if (_locked) Box3dColor = xlREDTRANSLUCENT;

    va.AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, Box3dColor);

    va.AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, Box3dColor);

    va.AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, Box3dColor);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, Box3dColor);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);

    if (active_handle != -1) {
        active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
        DrawGLUtils::DrawSphere(mHandlePosition[CENTER_HANDLE].x, mHandlePosition[CENTER_HANDLE].y, mHandlePosition[CENTER_HANDLE].z, (double)(GetRectHandleWidth(zoom, scale)), xlORANGETRANSLUCENT, va);
        DrawAxisTool(active_handle_pos, va, zoom, scale);
        if (active_axis != -1) {
            LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
            switch (active_axis)
            {
            case X_AXIS:
                va.AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                va.AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                break;
            case Y_AXIS:
                va.AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                va.AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                break;
            case Z_AXIS:
                va.AddVertex(active_handle_pos.x, active_handle_pos.y, -1000000.0f, xlBLUETRANSLUCENT);
                va.AddVertex(active_handle_pos.x, active_handle_pos.y, +1000000.0f, xlBLUETRANSLUCENT);
                break;
            default:
                wxASSERT(false);
                break;
            }
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
        }
    }

}

void BoxedScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va, float zoom, int scale) const {
    va.PreAlloc(6 * 5);

    float w1 = worldPos_x;
    float h1 = worldPos_y;

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (_locked)
    {
        handleColor = xlREDTRANSLUCENT;
    }

    // Upper Left Handle
    float sx = -RenderWi / 2;
    float sy = RenderHt / 2;
    float sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    sx += -BOUNDING_RECT_OFFSET;
    sy += BOUNDING_RECT_OFFSET;
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) /2), sy + (GetRectHandleWidth(zoom, scale) / 2), handleColor);
    mHandlePosition[L_TOP_HANDLE].x = sx;
    mHandlePosition[L_TOP_HANDLE].y = sy;
    mHandlePosition[L_TOP_HANDLE].z = sz;
    // Upper Right Handle
    sx = RenderWi / 2;
    sy = RenderHt / 2;
    sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    sx += BOUNDING_RECT_OFFSET;
    sy += BOUNDING_RECT_OFFSET;
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) / 2), sy + (GetRectHandleWidth(zoom, scale) / 2), handleColor);
    mHandlePosition[R_TOP_HANDLE].x = sx;
    mHandlePosition[R_TOP_HANDLE].y = sy;
    mHandlePosition[R_TOP_HANDLE].z = sz;
    // Lower Right Handle
    sx = RenderWi / 2;
    sy = -RenderHt / 2;
    sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    sx += BOUNDING_RECT_OFFSET;
    sy += -BOUNDING_RECT_OFFSET;
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) / 2), sy + (GetRectHandleWidth(zoom, scale) / 2), handleColor);
    mHandlePosition[R_BOT_HANDLE].x = sx;
    mHandlePosition[R_BOT_HANDLE].y = sy;
    mHandlePosition[R_BOT_HANDLE].z = sz;
    // Lower Left Handle
    sx = -RenderWi / 2;
    sy = -RenderHt / 2;
    sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    sx += -BOUNDING_RECT_OFFSET;
    sy += -BOUNDING_RECT_OFFSET;
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) / 2), sy + (GetRectHandleWidth(zoom, scale) / 2), handleColor);
    mHandlePosition[L_BOT_HANDLE].x = sx;
    mHandlePosition[L_BOT_HANDLE].y = sy;
    mHandlePosition[L_BOT_HANDLE].z = sz;

    // Draw rotation handle square
    sx = 0.0f;
    sy = RenderHt / 2 + (50/scaley);
    sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) / 2), sy + (GetRectHandleWidth(zoom, scale) / 2), handleColor);
    // Save rotate handle
    mHandlePosition[ROTATE_HANDLE].x = sx;
    mHandlePosition[ROTATE_HANDLE].y = sy;
    mHandlePosition[ROTATE_HANDLE].z = sz;

    va.Finish(GL_TRIANGLES);

    LOG_GL_ERRORV(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));
    va.AddVertex(w1, h1, xlWHITE);
    va.AddVertex(sx, sy, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
}

void BoxedScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxBoolProperty("Locked", "Locked", _locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = propertyEditor->Append(new wxFloatProperty("X", "ModelX", worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Y", "ModelY", worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("Z", "ModelZ", worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("ScaleX", "ScaleX", scalex));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("ScaleY", "ScaleY", scaley));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("ScaleZ", "ScaleZ", scalez));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("RotateX", "RotateX", rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("RotateY", "RotateY", rotatey));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("RotateZ", "RotateZ", rotatez));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
}

int BoxedScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!_locked && "ScaleX" == name) {
        scalex = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleX");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::ScaleX");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::ScaleX");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ScaleY" == name) {
        scaley = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleY");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::ScaleY");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::ScaleY");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ScaleZ" == name) {
        scalez = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ScaleZ");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::ScaleZ");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::ScaleZ");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (_locked && "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelX" == name) {
        worldPos_x = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelX");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::ModelX");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::ModelX");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::ModelX");
        return 0;
    }
    else if (_locked && "ModelX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelY" == name) {
        worldPos_y = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelY");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::ModelY");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::ModelY");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::ModelY");
        return 0;
    }
    else if (_locked && "ModelY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelZ" == name) {
        worldPos_z = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::ModelZ");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::ModelZ");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::ModelZ");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::ModelZ");
        return 0;
    }
    else if (_locked && "ModelZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateX" == name) {
        rotatex = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateX");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::RotateX");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::RotateX");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (_locked && "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateY" == name) {
        rotatey = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateY");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::RotateY");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::RotateY");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::RotateY");
        return 0;
    }
    else if (_locked && "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateZ" == name) {
        rotatez = event.GetValue().GetDouble();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RotateZ");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::RotateZ");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::RotateZ");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::RotateZ");
        return 0;
    }
    else if (_locked && "RotateZ" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::Locked");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }

    return 0;
}

bool BoxedScreenLocation::Rotate(int axis, float factor) {
    if (_locked) return false;

    glm::quat rot;
    switch (axis) {
    case X_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case Y_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case Z_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    }
 
    rotate_quat = rot * rotate_quat;
    glm::mat4 final_matrix = glm::toMat4(rotate_quat);
    glm::vec3 final_angles = rotationMatrixToEulerAngles(glm::mat3(final_matrix));
    rotatex = glm::degrees(final_angles.x);
    rotatey = glm::degrees(final_angles.y);
    rotatez = glm::degrees(final_angles.z);

    return true;
}

bool BoxedScreenLocation::Scale(const glm::vec3& factor) {
    if (_locked) return false;

    scalex *= factor.x;
    scaley *= factor.y;
    scalez *= factor.z;
    return true;
}

int BoxedScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) {

    if (latch) {
        saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        angles = glm::vec3(0,0,0);
    }

    if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

    if (handle == CENTER_HANDLE) {

        if (axis_tool == TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case X_AXIS:
                worldPos_x = saved_position.x + drag_delta.x;
                break;
            case Y_AXIS:
                worldPos_y = saved_position.y + drag_delta.y;
                break;
            case Z_AXIS:
                worldPos_z = saved_position.z + drag_delta.z;
                break;
            }
        }
        else if (axis_tool == TOOL_ROTATE) {
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                double total_change = end_angle - start_angle;
                double delta = angles.x - total_change;
                angles.x = total_change;
                Rotate(X_AXIS, delta);
            }
                break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                double total_change = end_angle - start_angle;
                double delta = angles.y - total_change;
                angles.y = total_change;
                Rotate(Y_AXIS, -delta);
            }
                break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                double total_change = end_angle - start_angle;
                double delta = angles.z - total_change;
                angles.z = total_change;
                Rotate(Z_AXIS, -delta);
            }
                break;
            }
        }
        else if (axis_tool == TOOL_SCALE) {
            float change_x = ((saved_size.x*saved_scale.x + drag_delta.x) / (saved_size.x*saved_scale.x));
            float change_y = ((saved_size.y*saved_scale.y + drag_delta.y) / (saved_size.y*saved_scale.y));
            float change_z = ((saved_size.z*saved_scale.z + drag_delta.z) / (saved_size.z*saved_scale.z));
            if (CtrlKeyPressed) {
                switch (active_axis)
                {
                case X_AXIS:
                    scalex = saved_scale.x * change_x;
                    if (scale_z) {
                        scalez = scalex;
                    }
                    scaley = saved_scale.y * change_x;
                    break;
                case Y_AXIS:
                    scaley = saved_scale.y * change_y;
                    scalex = saved_scale.x * change_y;
                    if (scale_z) {
                        scalez = saved_scale.x * change_y;
                    }
                    break;
                case Z_AXIS:
                    if (scale_z) {
                        scalez = saved_scale.z * change_z;
                    }
                    scalex = saved_scale.z * change_z;
                    scaley = saved_scale.y * change_z;
                    break;
                }
                if (ShiftKeyPressed) {
                    float current_bottom = saved_position.y - (saved_scale.y * RenderHt / 2.0f);
                    worldPos_y = current_bottom + (scaley * RenderHt / 2.0f);
                }
            }
            else {
                switch (active_axis)
                {
                case X_AXIS:
                    scalex = saved_scale.x * change_x;
                    if (ShiftKeyPressed) {
                        if (scale_z) {
                            scalez = scalex;
                        }
                        else {
                            scaley = saved_scale.y * change_x;
                        }
                    }
                    break;
                case Y_AXIS:
                    scaley = saved_scale.y * change_y;
                    if (ShiftKeyPressed) {
                        float current_bottom = saved_position.y - (saved_scale.y * RenderHt / 2.0f);
                        worldPos_y = current_bottom + (scaley * RenderHt / 2.0f);
                    }
                    break;
                case Z_AXIS:
                    if (scale_z) {
                        scalez = saved_scale.z * change_z;
                    }
                    if (ShiftKeyPressed) {
                        scalex = saved_scale.z * change_z;
                    }
                    break;
                }
            }
        }
    }
    return 1;
}

int BoxedScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

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

    int posx = ray_origin.x;
    int posy = ray_origin.y;

    if (handle == ROTATE_HANDLE) {
        int sx = posx - centerx;
        int sy = posy - centery;
        //Calculate angle of mouse from center.
        float tan = (float)sx / (float)sy;
        int angle = -toDegrees((float)atan(tan));
        if (sy >= 0) {
            rotatez = angle;
        }
        else if (sx <= 0) {
            rotatez = 90 + (90 + angle);
        }
        else {
            rotatez = -90 - (90 - angle);
        }
        if (ShiftKeyPressed) {
            rotatez = (int)(rotatez / 5) * 5;
        }
        rotate_quat = glm::angleAxis(glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    else {
        if ((handle == L_TOP_HANDLE) || (handle == R_TOP_HANDLE)) {
            if (float(posy) <= (centery - RenderHt / 2 * scaley)) return 0;
        }
        if ((handle == L_BOT_HANDLE) || (handle == R_BOT_HANDLE)) {
            if (float(posy) >= (centery + RenderHt / 2 * scaley)) return 0;
        }
        if ((handle == R_TOP_HANDLE) || (handle == R_BOT_HANDLE)) {
            if (float(posx) <= (centerx - RenderWi / 2 * scalex)) return 0;
        }
        if ((handle == L_TOP_HANDLE) || (handle == L_BOT_HANDLE)) {
            if (float(posx) >= (centerx + RenderWi / 2 * scalex)) return 0;
        }
        float sx = float(posx) - centerx;
        float sy = float(posy) - centery;
        float radians = -glm::radians((float)rotatez); // negative angle to reverse translation
        TranslatePointDoubles(radians, sx, sy, sx, sy);
        sx = fabs(sx);
        sy = fabs(sy);
        float current_width = RenderWi * scalex;
        float current_height = RenderHt * scaley;
        float new_width = sx + (RenderWi / 2 * scalex);
        float new_height = sy + (RenderHt / 2 * scaley);
        new_width -= BOUNDING_RECT_OFFSET;
        new_height -= BOUNDING_RECT_OFFSET;
        if ((handle == L_TOP_HANDLE) || (handle == L_BOT_HANDLE)) {
            worldPos_x += (current_width - new_width) / 2;
        }
        else {
            worldPos_x -= (current_width - new_width) / 2;
        }
        if ((handle == L_TOP_HANDLE) || (handle == R_TOP_HANDLE)) {
            worldPos_y -= (current_height - new_height) / 2;
        }
        else {
            worldPos_y += (current_height - new_height) / 2;
        }
        sx = new_width / RenderWi;
        sy = new_height / RenderHt;
        SetScale(sx, sy);
        if( supportsZScaling || createWithDepth ) {
            scalez = scalex;
        }
    }
    return 0;
}

glm::vec2 BoxedScreenLocation::GetScreenOffset(ModelPreview* preview)
{
    glm::vec2 position = VectorMath::GetScreenCoord(preview->getWidth(),
                                                    preview->getHeight(),
                                                    GetWorldPosition(),              // X,Y,Z coords of the position when not transformed at all.
                                                    preview->GetProjViewMatrix(),    // Projection / View matrix
                                                    Identity                         // Transformation applied to the position
    );

    position.x = position.x / (float)preview->getWidth();
    position.y = position.y / (float)preview->getHeight();
    return position;
}

float BoxedScreenLocation::GetTop() const {
    return worldPos_y + (RenderHt * scaley / 2.0f);
}
float BoxedScreenLocation::GetLeft() const {
    return worldPos_x - (RenderWi * scalex / 2.0f);
}
float BoxedScreenLocation::GetRight() const {
    return worldPos_x + (RenderWi * scalex / 2.0f);
}
float BoxedScreenLocation::GetBottom() const {
    return worldPos_y - (RenderHt * scaley / 2.0f);
}
float BoxedScreenLocation::GetFront() const {
    return worldPos_z + (RenderWi * scalez / 2.0f);
}
float BoxedScreenLocation::GetBack() const {
    return worldPos_z - (RenderWi * scalez / 2.0f);
}

float BoxedScreenLocation::GetRestorableMWidth() const {
    return (RenderWi-1) * scalex;
}
float BoxedScreenLocation::GetRestorableMHeight() const {
    return (RenderHt-1) * scaley;
}
float BoxedScreenLocation::GetMWidth() const {
    return RenderWi * scalex;
}
float BoxedScreenLocation::GetMHeight() const {
    return RenderHt * scaley;
}
void BoxedScreenLocation::SetMWidth(float w) {
    if (RenderWi == 1)
    { 
        scalex = 1;
    }
    else
    {
        scalex = w / (RenderWi - 1);
    }
}
void BoxedScreenLocation::SetMDepth(float d) {
    if (RenderWi == 1)
    {
        scalez = 1;
    }
    else
    {
        scalez = d / (RenderWi - 1);
    }
}
float BoxedScreenLocation::GetMDepth() const {
    return scalez * RenderWi;
}
float BoxedScreenLocation::GetRestorableMDepth() const {
    return scalez * (RenderWi-1);
}
void BoxedScreenLocation::SetMHeight(float h) {
    if (RenderHt == 1 || h == 0)
    {
        scaley = 1;
    }
    else
    {
        scaley = h / (RenderHt - 1);
    }
}
void BoxedScreenLocation::SetLeft(float x) {
    worldPos_x = x + (RenderWi * scalex / 2.0f);
}
void BoxedScreenLocation::SetRight(float x) {
    worldPos_x = x - (RenderWi * scalex / 2.0f);
}
void BoxedScreenLocation::SetTop(float y) {
    worldPos_y = y - (RenderHt * scaley / 2.0f);
}
void BoxedScreenLocation::SetBottom(float y) {
    worldPos_y = y + (RenderHt * scaley / 2.0f);
}
void BoxedScreenLocation::SetFront(float z) {
    worldPos_z = z - (RenderWi * scalez / 2.0f);
}
void BoxedScreenLocation::SetBack(float z) {
    worldPos_z = z + (RenderWi * scalez / 2.0f);
}

TwoPointScreenLocation::TwoPointScreenLocation() : ModelScreenLocation(3),
    point2(glm::vec3(0.0f)), center(glm::vec3(0.0f)), minMaxSet(false), old(nullptr)
{
    x2 = 0;
    y2 = 0;
    z2 = 0;
    saved_angle = 0;
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

int TwoPointScreenLocation::CheckUpgrade(wxXmlNode *node)
{
    // check for upgrade to world positioning
    int version = wxAtoi(node->GetAttribute("versionNumber", "0"));
    if (version < 2) {
        // skip first upgrade call since preview size is not set
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", "2");
        return UPGRADE_SKIPPED;
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
        return UPGRADE_EXEC_DONE;
    }
    return UPGRADE_NOT_NEEDED;
}

void TwoPointScreenLocation::Read(wxXmlNode *ModelNode) {
    int upgrade_result = CheckUpgrade(ModelNode);
    if (upgrade_result == UPGRADE_NOT_NEEDED) {
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

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

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
    if (!is_3d) {
        // allows 2D selection to work
        origin.z = 0.0f;
        point2.z = 0.0f;
    }
    center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);

    glm::vec3 a = point2 - origin;
    glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector2(origin, point2);
    length = glm::length(a);
    scalex = scaley = scalez = length / RenderWi;
    glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
    TranslateMatrix = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    matrix = TranslateMatrix * rotationMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
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

    for (size_t h = 0; h < num_handles; h++) {
        float zoom = preview->GetCameraZoomForHandles();
        int scale = preview->GetHandleScale();
        handle_aabb_min[h].x = mHandlePosition[h + 1].x - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[h].y = mHandlePosition[h + 1].y - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[h].z = mHandlePosition[h + 1].z - GetRectHandleWidth(zoom, scale);
        handle_aabb_max[h].x = mHandlePosition[h + 1].x + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[h].y = mHandlePosition[h + 1].y + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[h].z = mHandlePosition[h + 1].z + GetRectHandleWidth(zoom, scale);
    }

    // Test each each Oriented Bounding Box (OBB).
    for (size_t i = 0; i < num_handles; i++)
    {
        if (VectorMath::TestRayOBBIntersection2D(
            ray_origin,
            handle_aabb_min[i],
            handle_aabb_max[i],
            Identity)
            ) {
            handle = i + 1;
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

void TwoPointScreenLocation::SetAxisTool(int mode)
{
    if (mode != TOOL_SCALE && mode != TOOL_XY_TRANS) {
        axis_tool = mode;
    }
    else {
        axis_tool = TOOL_TRANSLATE;
    }
}

void TwoPointScreenLocation::AdvanceAxisTool()
{
    axis_tool += 1;
    if (axis_tool == TOOL_SCALE) {
        axis_tool += 1;
    }
    axis_tool %= (NUM_TOOLS-1);
}

void TwoPointScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va, float zoom, int scale) const {
    va.PreAlloc(10);

    if (active_handle != NO_HANDLE) {
        xlColor h1c = xlBLUETRANSLUCENT;
        xlColor h2c = xlBLUETRANSLUCENT;
        xlColor h3c = xlORANGETRANSLUCENT;
        if (_locked)
        {
            h1c = xlREDTRANSLUCENT;
            h2c = xlREDTRANSLUCENT;
            h3c = xlREDTRANSLUCENT;
        }
        else {
            h1c = (highlighted_handle == START_HANDLE) ? xlYELLOWTRANSLUCENT : xlGREENTRANSLUCENT;
            h2c = (highlighted_handle == END_HANDLE) ? xlYELLOWTRANSLUCENT : xlBLUETRANSLUCENT;
            h3c = (highlighted_handle == CENTER_HANDLE) ? xlYELLOWTRANSLUCENT : xlORANGETRANSLUCENT;
        }

        DrawGLUtils::DrawSphere(worldPos_x, worldPos_y, worldPos_z, GetRectHandleWidth(zoom, scale), h1c, va);
        mHandlePosition[START_HANDLE].x = worldPos_x;
        mHandlePosition[START_HANDLE].y = worldPos_y;
        mHandlePosition[START_HANDLE].z = worldPos_z;

        DrawGLUtils::DrawSphere(point2.x, point2.y, point2.z, GetRectHandleWidth(zoom, scale), h2c, va);
        mHandlePosition[END_HANDLE].x = point2.x;
        mHandlePosition[END_HANDLE].y = point2.y;
        mHandlePosition[END_HANDLE].z = point2.z;


        DrawGLUtils::DrawSphere(center.x, center.y, center.z, GetRectHandleWidth(zoom, scale), h3c, va);
        mHandlePosition[CENTER_HANDLE].x = center.x;
        mHandlePosition[CENTER_HANDLE].y = center.y;
        mHandlePosition[CENTER_HANDLE].z = center.z;

        handle_aabb_min[START_HANDLE].x = -GetRectHandleWidth(zoom, scale);
        handle_aabb_min[START_HANDLE].y = -GetRectHandleWidth(zoom, scale);
        handle_aabb_min[START_HANDLE].z = -GetRectHandleWidth(zoom, scale);
        handle_aabb_max[START_HANDLE].x = GetRectHandleWidth(zoom, scale);
        handle_aabb_max[START_HANDLE].y = GetRectHandleWidth(zoom, scale);
        handle_aabb_max[START_HANDLE].z = GetRectHandleWidth(zoom, scale);

        handle_aabb_min[END_HANDLE].x = RenderWi * scalex - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[END_HANDLE].y = -GetRectHandleWidth(zoom, scale);
        handle_aabb_min[END_HANDLE].z = -GetRectHandleWidth(zoom, scale);
        handle_aabb_max[END_HANDLE].x = RenderWi * scalex + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[END_HANDLE].y = GetRectHandleWidth(zoom, scale);
        handle_aabb_max[END_HANDLE].z = GetRectHandleWidth(zoom, scale);

        handle_aabb_min[CENTER_HANDLE].x = (RenderWi / 2.0f) * scalex - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[CENTER_HANDLE].y = -GetRectHandleWidth(zoom, scale);
        handle_aabb_min[CENTER_HANDLE].z = -GetRectHandleWidth(zoom, scale);
        handle_aabb_max[CENTER_HANDLE].x = (RenderWi / 2.0f) * scalex + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[CENTER_HANDLE].y = GetRectHandleWidth(zoom, scale);
        handle_aabb_max[CENTER_HANDLE].z = GetRectHandleWidth(zoom, scale);

        if (!_locked) {
            active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
            DrawAxisTool(active_handle_pos, va, zoom, scale);
            if (active_axis != -1) {
                LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
                if (active_handle == SHEAR_HANDLE) {
                    va.AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va.AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va.AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                    va.AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                }
                else {
                    switch (active_axis)
                    {
                    case X_AXIS:
                        va.AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                        va.AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                        break;
                    case Y_AXIS:
                        va.AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                        va.AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                        break;
                    case Z_AXIS:
                        va.AddVertex(active_handle_pos.x, active_handle_pos.y, -1000000.0f, xlBLUETRANSLUCENT);
                        va.AddVertex(active_handle_pos.x, active_handle_pos.y, +1000000.0f, xlBLUETRANSLUCENT);
                        break;
                    }
                }
                va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
            }
        }
    }
    else {
        // the bounding box is so close to a single line don't draw it once it's selected
        xlColor Box3dColor = xlWHITETRANSLUCENT;
        if (_locked) Box3dColor = xlREDTRANSLUCENT;
        DrawGLUtils::DrawBoundingBox(Box3dColor, aabb_min, aabb_max, ModelMatrix, va);
    }
}

void TwoPointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va, float zoom, int scale) const {

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (_locked)
    {
        handleColor = xlREDTRANSLUCENT;
    }

    va.PreAlloc(16);
    if( point2.y - origin.y == 0 )
    {
        va.AddVertex(worldPos_x, worldPos_y, xlRED);
        va.AddVertex(point2.x, point2.y, xlRED);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }
    else if(point2.x - origin.x == 0 )
    {
        va.AddVertex(worldPos_x, worldPos_y, handleColor);
        va.AddVertex(point2.x, point2.y, handleColor);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }

    float sx = worldPos_x;
    float sy = worldPos_y;
    float sz = worldPos_z;
    //TranslatePoint(sx, sy, sz);
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) / 2), sy + (GetRectHandleWidth(zoom, scale) / 2), xlGREENTRANSLUCENT);
    mHandlePosition[START_HANDLE].x = sx;
    mHandlePosition[START_HANDLE].y = sy;
    mHandlePosition[START_HANDLE].z = sz;

    sx = point2.x;
    sy = point2.y;
    sz = point2.z;
    //TranslatePoint(sx, sy, sz);
    va.AddRect(sx - (GetRectHandleWidth(zoom, scale) / 2), sy - (GetRectHandleWidth(zoom, scale) / 2), sx + (GetRectHandleWidth(zoom, scale) / 2), sy + (GetRectHandleWidth(zoom, scale) / 2), handleColor);
    mHandlePosition[END_HANDLE].x = sx;
    mHandlePosition[END_HANDLE].y = sy;
    mHandlePosition[END_HANDLE].z = sz;

    va.Finish(GL_TRIANGLES);
}

int TwoPointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (latch) {
        saved_angle = 0.0f;
        angles = glm::vec3(0,0,0);

        if (handle == CENTER_HANDLE) {
            saved_position = center;
            saved_point = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        }
        else if (handle == END_HANDLE) {
            saved_position = point2;
            saved_point = glm::vec3(x2, y2, z2);
        }
        else {
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            saved_point = glm::vec3(x2, y2, z2);
        }
    }

    if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

    if (handle == CENTER_HANDLE) {

        if (axis_tool == TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case X_AXIS:
                worldPos_x = saved_point.x + drag_delta.x;
                break;
            case Y_AXIS:
                worldPos_y = saved_point.y + drag_delta.y;
                break;
            case Z_AXIS:
                worldPos_z = saved_point.z + drag_delta.z;
                break;
            }
        }
        else if (axis_tool == TOOL_ROTATE) {
            double angle = 0.0f;
            float new_angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;

            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                new_angle = saved_angle - angle;
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                new_angle = saved_angle - angle;
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                new_angle = angle - saved_angle;
            }
            break;
            }
            TwoPointScreenLocation::Rotate(active_axis, new_angle);
            saved_angle = angle;
        }
        else if (axis_tool == TOOL_SCALE) {
            double delta = 0.0f;
            glm::vec3 scaling = glm::vec3(1.0f);
            switch (active_axis)
            {
            case X_AXIS:
                delta = (drag_delta.x - saved_position.x) * 2.0f;
                scaling.x = (length + delta) / length;
                break;
            case Y_AXIS:
                delta = (drag_delta.y - saved_position.y) * 2.0f;
                scaling.y = (length + delta) / length;
                break;
            case Z_AXIS:
                delta = (drag_delta.z - saved_position.z) * 2.0f;
                scaling.z = (length + delta) / length;
                break;
            }
            saved_position = drag_delta;
            TwoPointScreenLocation::Scale(scaling);
        }
    }
    else if (handle == START_HANDLE) {

        if (axis_tool == TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case X_AXIS:
                worldPos_x = saved_position.x + drag_delta.x;
                x2 = saved_point.x - drag_delta.x;
                break;
            case Y_AXIS:
                worldPos_y = saved_position.y + drag_delta.y;
                y2 = saved_point.y - drag_delta.y;
                break;
            case Z_AXIS:
                worldPos_z = saved_position.z + drag_delta.z;
                z2 = saved_point.z - drag_delta.z;
                break;
            }
        }
        else if (axis_tool == TOOL_ROTATE) {
            double angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            glm::vec3 end_pt = point2;
            glm::mat4 translateToOrigin = glm::translate(Identity, -origin);
            glm::mat4 translateBack = glm::translate(Identity, origin);
            glm::mat4 Rotate = Identity;

            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            }
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

        if (axis_tool == TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case X_AXIS:
                x2 = saved_point.x + drag_delta.x;
                break;
            case Y_AXIS:
                y2 = saved_point.y + drag_delta.y;
                break;
            case Z_AXIS:
                z2 = saved_point.z + drag_delta.z;
                break;
            }
        }
        else if (axis_tool == TOOL_ROTATE) {
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
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(Identity, glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            }
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
        saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        active_axis = X_AXIS;
        DragHandle(preview, x, y, true);
        worldPos_x = saved_intersect.x;
        worldPos_y = saved_intersect.y;
        worldPos_z = 0.0f;
        if (preview->Is3D()) {
            // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
            // of the mouse click with the ground plane
            saved_point = glm::vec3(0.0f);
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
    }
    else if ("Locked" == name)
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
        Rotate(Y_AXIS, -offset);
        rotate_point(position.x, position.z, glm::radians(-offset), posx, posz);
        SetHcenterPos(posx);
        SetDcenterPos(posz);
    }
    else {
        ModelScreenLocation::RotateAboutPoint(position, angle);
    }
}

bool TwoPointScreenLocation::Rotate(int axis, float factor) {
    if (_locked) return false;
    glm::vec3 start_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    glm::vec3 end_pt = glm::vec3(x2 + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);
    glm::mat4 translateToOrigin = glm::translate(Identity, -center);
    glm::mat4 translateBack = glm::translate(Identity, center);
    glm::mat4 rot_mat = Identity;

    switch (axis)
    {
    case X_AXIS:
    {
        rot_mat = glm::rotate(Identity, glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    break;
    case Y_AXIS:
    {
        rot_mat = glm::rotate(Identity, glm::radians(-factor), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    break;
    case Z_AXIS:
    {
        rot_mat = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
    }
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

glm::vec2 TwoPointScreenLocation::GetScreenOffset(ModelPreview* preview)
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

ThreePointScreenLocation::ThreePointScreenLocation(): height(1.0), modelHandlesHeight(false), supportsShear(false), supportsAngle(false), angle(0), shear(0.0) {
    mHandlePosition.resize(4);
    handle_aabb_max.resize(4);
    handle_aabb_min.resize(4);
    mSelectableHandles = 4;
}

ThreePointScreenLocation::~ThreePointScreenLocation() {
}

wxCursor ThreePointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {
    if (preview != nullptr) {
        saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        if (mSelectableHandles > 3 && supportsAngle && preview->Is3D()) {
            // place Arch models on the ground in 3D mode
            active_axis = Z_AXIS;
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = 0.0f;
            worldPos_z = saved_intersect.z;
            active_axis = X_AXIS;
            point2 = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        }
        else {
            active_axis = X_AXIS;
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = saved_intersect.y;
            worldPos_z = 0.0f;
        }
        if (preview->Is3D()) {
            // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
            // of the mouse click with the ground plane
            saved_point = glm::vec3(0.0f);
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
    if (!is_3d) {
        // allows 2D selection to work
        //origin.z = 0.0f;
        //point2.z = 0.0f;
    }
    center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);

    glm::vec3 a = point2 - origin;
    scalex = scaley = scalez = glm::length(a) / RenderWi;
    glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);

    glm::mat4 scalingMatrix;
    if (modelHandlesHeight) {
        scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley, scalez));
    }
    else {
        scalingMatrix = glm::scale(Identity, glm::vec3(scalex, scaley * height, scalez));
    }
    shearMatrix = Identity;
    if (supportsShear) {
        shearMatrix = glm::mat4(glm::shearY(glm::mat3(1.0f), GetYShear()));
    }
    glm::mat4 RotateX1 = glm::rotate(Identity, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 RotateX = glm::rotate(Identity, glm::radians((float)rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
    TranslateMatrix = translate(Identity, glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    if (x2 < 0.0f && y2 != 0.0f) {
        rotationMatrix = rotationMatrix * RotateX1;
    }
    matrix = TranslateMatrix * rotationMatrix * RotateX * shearMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        ModelMatrix = TranslateMatrix * rotationMatrix;
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
    }
    else {
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
    TwoPointScreenLocation::SetMHeight(h);
}

float ThreePointScreenLocation::GetMWidth() const
{
    return TwoPointScreenLocation::GetMWidth();
}

float ThreePointScreenLocation::GetMHeight() const
{
    return TwoPointScreenLocation::GetMHeight();
}

void ThreePointScreenLocation::SetActiveHandle(int handle)
{
    active_handle = handle;
    highlighted_handle = -1;
    SetAxisTool(axis_tool);  // run logic to disallow certain tools
}

void ThreePointScreenLocation::SetAxisTool(int mode)
{
    if (active_handle == SHEAR_HANDLE) {
        axis_tool = TOOL_XY_TRANS;
    }
    else {
        TwoPointScreenLocation::SetAxisTool(mode);
    }
}

void ThreePointScreenLocation::AdvanceAxisTool()
{
    if (active_handle == SHEAR_HANDLE) {
        axis_tool = TOOL_XY_TRANS;
    }
    else {
        TwoPointScreenLocation::AdvanceAxisTool();
    }
}

void ThreePointScreenLocation::SetActiveAxis(int axis)
{
    if (active_handle == SHEAR_HANDLE) {
        if (axis != -1) {
            active_axis = X_AXIS;
        }
        else {
            active_axis = -1;
        }
    }
    else {
        ModelScreenLocation::SetActiveAxis(axis);
    }
}

void ThreePointScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va, float zoom, int scale) const {

    if (active_handle != -1) {

        float ymax = RenderHt;
        va.PreAlloc(18);

        float x = RenderWi / 2;
        if (supportsAngle) {
            ymax = RenderHt * height;
            rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, ymax);
        }

        glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 0.0f), 1.0f));
        float sx = v1.x;
        float sy = v1.y;
        float sz = v1.z;
        LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
        va.AddVertex(center.x, center.y, center.z, xlWHITE);
        va.AddVertex(sx, sy, sz, xlWHITE);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);

        xlColor h4c = xlBLUETRANSLUCENT;
        if (_locked)
        {
            h4c = xlREDTRANSLUCENT;
        }
        else {
            h4c = (highlighted_handle == SHEAR_HANDLE) ? xlYELLOWTRANSLUCENT : xlBLUETRANSLUCENT;
        }

        DrawGLUtils::DrawSphere(sx, sy, sz, GetRectHandleWidth(zoom, scale), h4c, va);
        mHandlePosition[SHEAR_HANDLE].x = sx;
        mHandlePosition[SHEAR_HANDLE].y = sy;
        mHandlePosition[SHEAR_HANDLE].z = sz;

        handle_aabb_min[SHEAR_HANDLE].x = x * scalex - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[SHEAR_HANDLE].y = ymax * scaley * (supportsShear ? height : 1.0f) - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[SHEAR_HANDLE].z = -GetRectHandleWidth(zoom, scale);
        handle_aabb_max[SHEAR_HANDLE].x = x * scalex + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[SHEAR_HANDLE].y = ymax * scaley * (supportsShear ? height : 1.0f) + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[SHEAR_HANDLE].z = GetRectHandleWidth(zoom, scale);

        if (supportsShear) {
            handle_aabb_min[SHEAR_HANDLE] = glm::vec3(shearMatrix * glm::vec4(handle_aabb_min[SHEAR_HANDLE], 1.0f));
            handle_aabb_max[SHEAR_HANDLE] = glm::vec3(shearMatrix * glm::vec4(handle_aabb_max[SHEAR_HANDLE], 1.0f));
        }
    }

    TwoPointScreenLocation::DrawHandles(va, zoom, scale);
}

void ThreePointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va, float zoom, int scale) const {

    float sx1 = center.x;
    float sy1 = center.y;

    float ymax = RenderHt;

    va.PreAlloc(18);

    float x = RenderWi / 2;
    if (supportsAngle) {
        ymax = RenderHt * height;
        rotate_point(RenderWi / 2.0, 0, toRadians(angle), x, ymax);
    }

    glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, ymax, 1), 1.0f));
    float sx = v1.x;
    float sy = v1.y;
    LOG_GL_ERRORV(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));
    va.AddVertex(sx1, sy1, xlWHITE);
    va.AddVertex(sx, sy, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (_locked)
    {
        handleColor = xlREDTRANSLUCENT;
    }

    va.AddRect(sx - GetRectHandleWidth(zoom, scale)/2.0, sy - GetRectHandleWidth(zoom, scale)/2.0, sx + GetRectHandleWidth(zoom, scale), sy + GetRectHandleWidth(zoom, scale), handleColor);
    va.Finish(GL_TRIANGLES);
    mHandlePosition[SHEAR_HANDLE].x = sx;
    mHandlePosition[SHEAR_HANDLE].y = sy;

    TwoPointScreenLocation::DrawHandles(va, zoom, scale);
}

int ThreePointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

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

    float posx = ray_origin.x - center.x;
    float posy = ray_origin.y - center.y;

    if (handle == SHEAR_HANDLE) {
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
            }
            else if (posx <= 0) {
                angle = 90 + (90 + angle1);
            }
            else {
                angle = -90 - (90 - angle1);
            }
            if (ShiftKeyPressed) {
                angle = (int)(angle / 5) * 5;
            }
            float length = std::sqrt(posy*posy + posx * posx);
            height = length / (RenderHt * scaley);
        }
        else if (supportsShear) {
            glm::mat4 m = glm::inverse(matrix);
            glm::vec3 v = glm::vec3(m * glm::vec4(ray_origin, 1.0f));
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

        if (axis_tool == TOOL_XY_TRANS) {
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
        if (axis_tool == TOOL_ROTATE) {
            if (latch) {
                saved_angle = 0.0f;
                saved_position = center;
                saved_point = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            }
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            if (active_axis == X_AXIS) {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                double angle = end_angle - start_angle;
                rotatex = saved_rotate.x - angle;
            }
        }
    }
    return TwoPointScreenLocation::MoveHandle3D(preview, handle, ShiftKeyPressed, CtrlKeyPressed, mouseX, mouseY, latch, scale_z);
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

void ThreePointScreenLocation::DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va) const
{
    DrawGLUtils::DrawBoundingBox(c, aabb_min, aabb_max, draw_3d ? ModelMatrix : TranslateMatrix, va);
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

            for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
                for (auto coord = it->get()->Coords.begin(); coord != it->get()->Coords.end(); ++coord) {

                    float sx = coord->screenX;
                    float sy = coord->screenY;
                    float sz = coord->screenZ;

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
            for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
                for (auto coord = it->get()->Coords.begin(); coord != it->get()->Coords.end(); ++coord) {

                    float sx = coord->screenX;
                    float sy = coord->screenY;
                    float sz = coord->screenZ;
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

PolyPointScreenLocation::PolyPointScreenLocation() : ModelScreenLocation(2),
   num_points(2), selected_handle(-1), selected_segment(-1) {
    mPos.resize(2);
    mPos[0].x = 0.0f;
    mPos[0].y = 0.0f;
    mPos[0].z = 0.0f;
    mPos[0].matrix = nullptr;
    mPos[0].mod_matrix = nullptr;
    mPos[0].curve = nullptr;
    mPos[0].has_curve = false;
    mPos[1].x = 0.0f;
    mPos[1].y = 0.0f;
    mPos[1].z = 0.0f;
    mPos[1].matrix = nullptr;
    mPos[1].mod_matrix = nullptr;
    mPos[1].curve = nullptr;
    mPos[1].has_curve = false;
    handle_aabb_max.resize(7);
    handle_aabb_min.resize(7);
    seg_aabb_min.resize(1);
    seg_aabb_max.resize(1);
    mSelectableHandles = 3;
}

PolyPointScreenLocation::~PolyPointScreenLocation() {
    for( int i = 0; i < mPos.size(); ++i ) {
        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        if (mPos[i].mod_matrix != nullptr) {
            delete mPos[i].mod_matrix;
        }
        if (mPos[i].curve != nullptr) {
            delete mPos[i].curve;
        }
    }
    mPos.clear();
}

void PolyPointScreenLocation::SetCurve(int seg_num, bool create) {

    if (_locked) return;

    if( create ) {
        mPos[seg_num].has_curve = true;
        if( mPos[seg_num].curve == nullptr ) {
            mPos[seg_num].curve = new BezierCurveCubic3D();
        }
        mPos[seg_num].curve->set_p0( mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z );
        mPos[seg_num].curve->set_p1( mPos[seg_num+1].x, mPos[seg_num+1].y, mPos[seg_num+1].z );
        mPos[seg_num].curve->set_cp0( mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z );
        mPos[seg_num].curve->set_cp1( mPos[seg_num+1].x, mPos[seg_num+1].y, mPos[seg_num+1].z );
    } else {
        mPos[seg_num].has_curve = false;
        if( mPos[seg_num].curve != nullptr ) {
            delete mPos[seg_num].curve;
            mPos[seg_num].curve = nullptr;
        }
    }
}

int PolyPointScreenLocation::CheckUpgrade(wxXmlNode *node)
{
    // check for upgrade to world positioning
    int version = wxAtoi(node->GetAttribute("versionNumber", "0"));
    if (version < 2) {
        // on first upgrade call need to fix point data so init model won't crash
        if (node->HasAttribute("NumPoints")) {
            int num_points = wxAtoi(node->GetAttribute("NumPoints", "2"));
            wxString point_data = node->GetAttribute("PointData", "0.0, 0.0, 0.0, 0.0, 0.0, 0.0");
            wxArrayString point_array = wxSplit(point_data, ',');
            wxString new_point_data = "";
            for (int i = 0; i < num_points; ++i) {
                float posx = wxAtof(point_array[i * 2]);
                float posy = wxAtof(point_array[i * 2 + 1]);
                new_point_data += wxString::Format("%f,", posx);
                new_point_data += wxString::Format("%f,", posy);
                new_point_data += wxString::Format("%f", 0.0f);
                if (i != num_points - 1) {
                    new_point_data += ",";
                }
            }
            wxString cpoint_data = node->GetAttribute("cPointData", "");
            wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
            int num_curves = cpoint_array.size() / 5;
            wxString new_cpoint_data = "";
            for (int i = 0; i < num_curves; ++i) {
                int seg_num = wxAtoi(cpoint_array[i * 5]);
                float cp0x = wxAtof(cpoint_array[i * 5 + 1]);
                float cp0y = wxAtof(cpoint_array[i * 5 + 2]);
                float cp1x = wxAtof(cpoint_array[i * 5 + 3]);
                float cp1y = wxAtof(cpoint_array[i * 5 + 4]);
                new_cpoint_data += wxString::Format("%d,%f,%f,%f,%f,%f,%f,", seg_num, cp0x, cp0y, 0.0f, cp1x, cp1y, 0.0f);
            }
            node->DeleteAttribute("PointData");
            node->DeleteAttribute("cPointData");
            node->AddAttribute("PointData", new_point_data);
            node->AddAttribute("cPointData", new_cpoint_data);
        }
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", "2");
        return UPGRADE_SKIPPED;
    } else if (version == 2) {
        if (node->HasAttribute("NumPoints")) {
            float worldPos_x = 0.0;
            float worldPos_y = 0.0;
            int num_points = wxAtoi(node->GetAttribute("NumPoints", "2"));
            wxString point_data = node->GetAttribute("PointData", "0.0, 0.0, 0.0, 0.0, 0.0, 0.0");
            wxArrayString point_array = wxSplit(point_data, ',');
            wxString new_point_data = "";
            for (int i = 0; i < num_points; ++i) {
                float posx = wxAtof(point_array[i * 3]);
                float posy = wxAtof(point_array[i * 3 + 1]);
                if (i == 0) {
                    worldPos_x = previewW * posx;
                    worldPos_y = previewH * posy;
                    worldPos_z = 0.0f;
                    new_point_data += wxString::Format("%f,", 0.0f);
                    new_point_data += wxString::Format("%f,", 0.0f);
                    new_point_data += wxString::Format("%f", 0.0f);
                }
                else {
                    posx = (previewW * posx - worldPos_x) / 100.0f;
                    posy = (previewH * posy - worldPos_y) / 100.0f;
                    new_point_data += wxString::Format("%f,", posx);
                    new_point_data += wxString::Format("%f,", posy);
                    new_point_data += wxString::Format("%f", 0.0f);
                }
                if (i != num_points - 1) {
                    new_point_data += ",";
                }
            }
            wxString cpoint_data = node->GetAttribute("cPointData", "");
            wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
            int num_curves = cpoint_array.size() / 7;
            wxString new_cpoint_data = "";
            for (int i = 0; i < num_curves; ++i) {
                int seg_num = wxAtoi(cpoint_array[i * 7]);
                float cp0x = wxAtof(cpoint_array[i * 7 + 1]);
                float cp0y = wxAtof(cpoint_array[i * 7 + 2]);
                float cp1x = wxAtof(cpoint_array[i * 7 + 4]);
                float cp1y = wxAtof(cpoint_array[i * 7 + 5]);
                cp0x = (previewW * cp0x - worldPos_x) / 100.0f;
                cp0y = (previewH * cp0y - worldPos_y) / 100.0f;
                cp1x = (previewW * cp1x - worldPos_x) / 100.0f;
                cp1y = (previewH * cp1y - worldPos_y) / 100.0f;
                new_cpoint_data += wxString::Format("%d,%f,%f,%f,%f,%f,%f,", seg_num, cp0x, cp0y, 0.0f, cp1x, cp1y, 0.0f);
            }
            node->DeleteAttribute("WorldPosX");
            node->DeleteAttribute("WorldPosY");
            node->DeleteAttribute("WorldPosZ");
            node->AddAttribute("WorldPosX", wxString::Format("%6.4f", worldPos_x));
            node->AddAttribute("WorldPosY", wxString::Format("%6.4f", worldPos_y));
            node->AddAttribute("WorldPosZ", wxString::Format("%6.4f", worldPos_z));
            node->DeleteAttribute("PointData");
            node->DeleteAttribute("cPointData");
            node->AddAttribute("PointData", new_point_data);
            node->AddAttribute("cPointData", new_cpoint_data);
        }
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", CUR_MODEL_POS_VER);
        return UPGRADE_EXEC_READ;
    }
    return UPGRADE_NOT_NEEDED;
}

void PolyPointScreenLocation::Read(wxXmlNode *ModelNode) {
    int upgrade_result = CheckUpgrade(ModelNode);
    if (upgrade_result == UPGRADE_NOT_NEEDED) {
        worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "0.0"));
        worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
        worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));

        scalex = wxAtof(ModelNode->GetAttribute("ScaleX", "100.0"));
        scaley = wxAtof(ModelNode->GetAttribute("ScaleY", "100.0"));
        scalez = wxAtof(ModelNode->GetAttribute("ScaleZ", "100.0"));

        if (scalex <= 0 || std::isinf(scalex)) {
            scalex = 1.0f;
        }
        if (scaley <= 0 || std::isinf(scaley)) {
            scaley = 1.0f;
        }
        if (scalez <= 0 || std::isinf(scalez)) {
            scalez = 1.0f;
        }

        num_points = wxAtoi(ModelNode->GetAttribute("NumPoints", "2"));
        if (num_points < 2) num_points = 2;
        mPos.resize(num_points);
        wxString point_data = ModelNode->GetAttribute("PointData", "0.0, 0.0, 0.0, 0.0, 0.0, 0.0");
        wxArrayString point_array = wxSplit(point_data, ',');
        while (point_array.size() < num_points * 3) point_array.push_back("0.0");
        for (int i = 0; i < num_points; ++i) {
            mPos[i].x = wxAtof(point_array[i * 3]);
            mPos[i].y = wxAtof(point_array[i * 3 + 1]);
            mPos[i].z = wxAtof(point_array[i * 3 + 2]);
            mPos[i].has_curve = false;
        }
        mHandlePosition.resize(num_points + 5);
        mSelectableHandles = num_points + 1;
        handle_aabb_min.resize(num_points + 5);
        handle_aabb_max.resize(num_points + 5);
        seg_aabb_min.resize(num_points - 1);
        seg_aabb_max.resize(num_points - 1);
        wxString cpoint_data = ModelNode->GetAttribute("cPointData", "");
        wxArrayString cpoint_array = wxSplit(cpoint_data, ',');
        int num_curves = cpoint_array.size() / 7;
        glm::vec3 scaling(scalex, scaley, scalez);
        glm::vec3 world_pos(worldPos_x, worldPos_y, worldPos_z);
        for (int i = 0; i < num_curves; ++i) {
            int seg_num = wxAtoi(cpoint_array[i * 7]);
            mPos[seg_num].has_curve = true;
            if (mPos[seg_num].curve == nullptr) {
                mPos[seg_num].curve = new BezierCurveCubic3D();
            }
            mPos[seg_num].curve->set_p0(mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z);
            mPos[seg_num].curve->set_p1(mPos[seg_num + 1].x, mPos[seg_num + 1].y, mPos[seg_num + 1].z);
            mPos[seg_num].curve->set_cp0(wxAtof(cpoint_array[i * 7 + 1]), wxAtof(cpoint_array[i * 7 + 2]), wxAtof(cpoint_array[i * 7 + 3]));
            mPos[seg_num].curve->set_cp1(wxAtof(cpoint_array[i * 7 + 4]), wxAtof(cpoint_array[i * 7 + 5]), wxAtof(cpoint_array[i * 7 + 6]));
            mPos[seg_num].curve->SetPositioning(scaling, world_pos);
            mPos[seg_num].curve->UpdatePoints();
        }
        _locked = (wxAtoi(ModelNode->GetAttribute("Locked", "0")) == 1);
    }
}

void PolyPointScreenLocation::Write(wxXmlNode *node) {
    node->DeleteAttribute("WorldPosX");
    node->DeleteAttribute("WorldPosY");
    node->DeleteAttribute("WorldPosZ");
    node->DeleteAttribute("ScaleX");
    node->DeleteAttribute("ScaleY");
    node->DeleteAttribute("ScaleZ");
    node->AddAttribute("WorldPosX", wxString::Format("%6.4f", worldPos_x));
    node->AddAttribute("WorldPosY", wxString::Format("%6.4f", worldPos_y));
    node->AddAttribute("WorldPosZ", wxString::Format("%6.4f", worldPos_z));
    node->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
    node->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
    node->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));

    node->DeleteAttribute("NumPoints");
    node->DeleteAttribute("PointData");
    node->DeleteAttribute("cPointData");
    node->DeleteAttribute("Locked");
    wxString point_data = "";
    for( int i = 0; i < num_points; ++i ) {
        point_data += wxString::Format("%f,", mPos[i].x);
        point_data += wxString::Format("%f,", mPos[i].y);
        point_data += wxString::Format("%f", mPos[i].z);
        if( i != num_points-1 ) {
            point_data += ",";
        }
    }
    wxString cpoint_data = "";
    for( int i = 0; i < num_points; ++i ) {
        if( mPos[i].has_curve ) {
            cpoint_data += wxString::Format( "%d,%f,%f,%f,%f,%f,%f,", i, mPos[i].curve->get_cp0x(), mPos[i].curve->get_cp0y(), mPos[i].curve->get_cp0z(),
                                                                         mPos[i].curve->get_cp1x(), mPos[i].curve->get_cp1y(), mPos[i].curve->get_cp1z());
        }
    }
    node->AddAttribute("NumPoints", std::to_string(num_points));
    node->AddAttribute("PointData", point_data);
    node->AddAttribute("cPointData", cpoint_data);
    if (_locked)
    {
        node->AddAttribute("Locked", "1");
    }
}

void PolyPointScreenLocation::DrawBoundingBox(xlColor c, DrawGLUtils::xlAccumulator &va) const
{
    if (selected_segment != -1) {
        if (mPos[selected_segment].has_curve) {
            mPos[selected_segment].curve->DrawBoundingBox(c, va);
        } else {
            DrawGLUtils::DrawBoundingBox(c, seg_aabb_min[selected_segment], seg_aabb_max[selected_segment], *mPos[selected_segment].mod_matrix, va);
        }
    }
}

void PolyPointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    std::unique_lock<std::mutex> locker(_mutex);
    minX = 100000.0;
    minY = 100000.0;
    minZ = 100000.0;
    maxX = 0.0;
    maxY = 0.0;
    maxZ = 0.0;

    glm::vec3 scaling(scalex, scaley, scalez);
    glm::vec3 world_pos(worldPos_x, worldPos_y, worldPos_z);

    for( int i = 0; i < num_points-1; ++i ) {
        float x1p = mPos[i].x * scalex + worldPos_x;
        float x2p = mPos[i+1].x * scalex + worldPos_x;
        float y1p = mPos[i].y * scaley + worldPos_y;
        float y2p = mPos[i+1].y * scaley + worldPos_y;
        float z1p = mPos[i].z * scalez + worldPos_z;
        float z2p = mPos[i+1].z * scalez + worldPos_z;

        if (!is_3d) {
            // allows 2D selection to work
            z1p = 0.0f;
            z2p = 0.0f;
        }

        if( mPos[i].x < minX ) minX = mPos[i].x;
        if (mPos[i].y < minY) minY = mPos[i].y;
        if (mPos[i].z < minZ) minZ = mPos[i].z;
        if( mPos[i].x > maxX ) maxX = mPos[i].x;
        if (mPos[i].y > maxY) maxY = mPos[i].y;
        if (mPos[i].z > maxZ) maxZ = mPos[i].z;

        if( mPos[i].has_curve ) {
            mPos[i].curve->check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
        }

        if( i == num_points-2 ) {
            if( mPos[i+1].x < minX ) minX = mPos[i+1].x;
            if( mPos[i+1].y < minY ) minY = mPos[i+1].y;
            if( mPos[i+1].z < minZ ) minZ = mPos[i+1].z;
            if( mPos[i+1].x > maxX ) maxX = mPos[i+1].x;
            if( mPos[i+1].y > maxY ) maxY = mPos[i+1].y;
            if( mPos[i+1].z > maxZ ) maxZ = mPos[i+1].z;
        }

        if (x1p == x2p && y1p == y2p && z1p == z2p) {
            x2p += 0.001f;
        }
        glm::vec3 pt1(x1p, y1p, z1p);
        glm::vec3 pt2(x2p, y2p, z2p);
        glm::vec3 a = pt2 - pt1;
        float scale = glm::length(a) / RenderWi;
        glm::mat4 rotationMatrix = VectorMath::rotationMatrixFromXAxisToVector(a);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scale, 1.0f, 1.0f));
        glm::mat4 translateMatrix = glm::translate(Identity, glm::vec3(x1p, y1p, z1p));
        glm::mat4 mat = translateMatrix * rotationMatrix * scalingMatrix;

        if (mPos[i].matrix != nullptr) {
            delete mPos[i].matrix;
        }
        mPos[i].matrix = new glm::mat4(mat);

        if (allow_selected) {
            glm::mat4 mod_mat = translateMatrix * rotationMatrix;
            if (mPos[i].mod_matrix != nullptr) {
                delete mPos[i].mod_matrix;
            }
            mPos[i].mod_matrix = new glm::mat4(mod_mat);
            mPos[i].seg_scale = scale;
        }

        // update curve points
        if( mPos[i].has_curve ) {
            mPos[i].curve->SetPositioning(scaling, world_pos);
            mPos[i].curve->UpdatePoints();
        }
    }
    float yscale = (maxY - minY) * scaley;
    if (RenderHt > 1.0f && (maxY-minY < RenderHt)) {
        yscale = RenderHt;
    }
    glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3((maxX - minX) * scalex, yscale, (maxZ - minZ) * scalez));
    TranslateMatrix = glm::translate(Identity, glm::vec3(minX * scalex + worldPos_x, minY * scaley + worldPos_y, minZ * scalez + worldPos_z));
    main_matrix = TranslateMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        ModelMatrix = TranslateMatrix;
    }

    draw_3d = is_3d;
}

void PolyPointScreenLocation::TranslatePoint(float &x, float &y, float &z) const {
    glm::vec3 v = glm::vec3(main_matrix * glm::vec4(x, y, z, 1));
    x = v.x;
    y = v.y;
    z = v.z;
}

bool PolyPointScreenLocation::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const {
    int sx1 = std::min(x1,x2);
    int sx2 = std::max(x1,x2);
    int sy1 = std::min(y1,y2);
    int sy2 = std::max(y1,y2);
    float x1p = minX * scalex + worldPos_x;
    float x2p = maxX * scalex + worldPos_x;
    float y1p = minY * scaley + worldPos_y;
    float y2p = maxY * scaley + worldPos_y;

    if (draw_3d) {
        glm::vec3 aabb_min_pp(x1p, y1p, 0.0f);
        glm::vec3 aabb_max_pp(x2p, y2p, 0.0f);
        return VectorMath::TestVolumeOBBIntersection(
            sx1, sy1, sx2, sy2,
            preview->getWidth(),
            preview->getHeight(),
            aabb_min_pp, aabb_max_pp,
            preview->GetProjViewMatrix(),
            Identity);
    } else {
        if( x1p >= sx1 && x1p <= sx2 &&
            x2p >= sx1 && x2p <= sx2 &&
            y1p >= sy1 && y1p <= sy2 &&
            y2p >= sy1 && y2p <= sy2 ) {
            return true;
        }
    }
    return false;
}

bool PolyPointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const {

    bool ret_value = false;
    selected_segment = -1;

    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve) {
            if (mPos[i].curve->HitTest(ray_origin)) {
                selected_segment = i;
                ret_value = true;
                break;
            }
        } else {
            // perform normal line segment hit detection
            if (mPos[i].mod_matrix != nullptr)
            {
                if (VectorMath::TestRayOBBIntersection2D(
                    ray_origin,
                    seg_aabb_min[i],
                    seg_aabb_max[i],
                    *mPos[i].mod_matrix)
                    ) {
                    selected_segment = i;
                    ret_value = true;
                    break;
                }
            }
        }
    }
    // check if inside boundary handles
    float sx1 = (ray_origin.x - worldPos_x) / scalex;
    float sy1 = (ray_origin.y - worldPos_y) / scaley;
    if (sx1 >= minX && sx1 <= maxX && sy1 >= minY && sy1 <= maxY) {
        ret_value = true;
    }
    return ret_value;
}

bool PolyPointScreenLocation::HitTest3D(glm::vec3& ray_origin, glm::vec3& ray_direction, float& intersection_distance) const
{
    float distance = 1000000000.0f;
    bool ret_value = false;

    // FIXME: Speed up by having initial check for overall boundaries?

    selected_segment = -1;
    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve) {
            if (mPos[i].curve->HitTest3D(ray_origin, ray_direction, distance)) {
                if (distance < intersection_distance) {
                    intersection_distance = distance;
                    selected_segment = i;
                }
                ret_value = true;
            }
        } else {
            if (mPos[i].mod_matrix == nullptr) {
                continue;
            }

            // perform normal line segment hit detection
            if (VectorMath::TestRayOBBIntersection(
                ray_origin,
                ray_direction,
                seg_aabb_min[i],
                seg_aabb_max[i],
                *mPos[i].mod_matrix,
                distance)
                ) {
                if (distance < intersection_distance) {
                    intersection_distance = distance;
                    selected_segment = i;
                }
                ret_value = true;
            }
        }
    }

    return ret_value;
}

wxCursor PolyPointScreenLocation::CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const
{
    wxCursor return_value = wxCURSOR_DEFAULT;
    handle = NO_HANDLE;

    if (_locked) {
        return wxCURSOR_DEFAULT;
    }

    return_value = CheckIfOverAxisHandles3D(ray_origin, ray_direction, handle, zoom, scale);

    // test control point handles
    if (handle == NO_HANDLE) {
        float distance = 1000000000.0f;

        if (selected_segment != -1) {
            // add control point handles for selected segments
            int s = selected_segment;
            if (mPos.size() > s && mPos[s].has_curve) {
                glm::vec3 cp_handle_aabb_min[2];
                glm::vec3 cp_handle_aabb_max[2];
                cp_handle_aabb_min[0].x = (mPos[s].curve->get_cp0x() - minX)*scalex - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[0].y = (mPos[s].curve->get_cp0y() - minY)*scaley - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[0].z = (mPos[s].curve->get_cp0z() - minZ)*scalez - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[0].x = (mPos[s].curve->get_cp0x() - minX)*scalex + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[0].y = (mPos[s].curve->get_cp0y() - minY)*scaley + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[0].z = (mPos[s].curve->get_cp0z() - minZ)*scalez + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[1].x = (mPos[s].curve->get_cp1x() - minX)*scalex - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[1].y = (mPos[s].curve->get_cp1y() - minY)*scaley - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[1].z = (mPos[s].curve->get_cp1z() - minZ)*scalez - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[1].x = (mPos[s].curve->get_cp1x() - minX)*scalex + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[1].y = (mPos[s].curve->get_cp1y() - minY)*scaley + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[1].z = (mPos[s].curve->get_cp1z() - minZ)*scalez + GetRectHandleWidth(zoom, scale);

                // Test each each Oriented Bounding Box (OBB).
                for (size_t i = 0; i < 2; i++)
                {
                    float intersection_distance; // Output of TestRayOBBIntersection()

                    if (VectorMath::TestRayOBBIntersection(
                        ray_origin,
                        ray_direction,
                        cp_handle_aabb_min[i],
                        cp_handle_aabb_max[i],
                        ModelMatrix,
                        intersection_distance)
                        ) {
                        if (intersection_distance < distance) {
                            distance = intersection_distance;
                            handle = ((i == 0) ? s | 0x4000 : s | 0x8000);
                            return_value = wxCURSOR_HAND;
                        }
                    }
                }
            }
        }
    }

    // test the normal handles
    if (handle == NO_HANDLE) {
        float distance = 1000000000.0f;
        handle = -1;

        // Test each each Oriented Bounding Box (OBB).
        for (size_t i = 0; i < mSelectableHandles; i++)
        {
            float intersection_distance; // Output of TestRayOBBIntersection()

            if (VectorMath::TestRayOBBIntersection(
                ray_origin,
                ray_direction,
                handle_aabb_min[i],
                handle_aabb_max[i],
                ModelMatrix,
                intersection_distance)
                ) {
                if (intersection_distance < distance) {
                    distance = intersection_distance;
                    handle = i;
                    return_value = wxCURSOR_HAND;
                }
            }
        }
    }

    // test for clicking a new segment
    if (handle == NO_HANDLE) {
        float distance = 1000000000.0f;
        float intersection_distance = 1000000000.0f;
        // FIXME: Speed up by having initial check for overall boundaries?

        for (int i = 0; i < num_points - 1; ++i) {
            if (mPos[i].has_curve) {
                if (mPos[i].curve->HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    handle = i | 0x10000;
                    return_value = wxCURSOR_DEFAULT;
                }
            }
            else {
                // perform normal line segment hit detection
                if (mPos[i].mod_matrix != nullptr)
                {
                    if (VectorMath::TestRayOBBIntersection(
                        ray_origin,
                        ray_direction,
                        seg_aabb_min[i],
                        seg_aabb_max[i],
                        *mPos[i].mod_matrix,
                        distance)
                        ) {
                        if (distance < intersection_distance) {
                            intersection_distance = distance;
                            handle = i | 0x10000;
                            return_value = wxCURSOR_DEFAULT;
                        }
                    }
                }
            }
        }
    }

    return return_value;
}

wxCursor PolyPointScreenLocation::CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const
{
    wxASSERT(!preview->Is3D());

    wxCursor return_value = wxCURSOR_DEFAULT;
    handle = NO_HANDLE;
    float zoom = preview->GetCameraZoomForHandles();
    int scale = preview->GetHandleScale();

    if (_locked)
    {
        return wxCURSOR_DEFAULT;
    }

    //Get a world position for the mouse
    glm::vec3 ray_origin;
    glm::vec3 ray_direction;

    VectorMath::ScreenPosToWorldRay(
        x, preview->getHeight() - y,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    // test control point handles
    if (handle == NO_HANDLE) {
        if (selected_segment != -1) {
            int s = selected_segment;
            if (mPos[s].has_curve) {
                glm::vec3 cp_handle_aabb_min[2];
                glm::vec3 cp_handle_aabb_max[2];
                cp_handle_aabb_min[0].x = (mPos[s].curve->get_cp0x() - minX)*scalex - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[0].y = (mPos[s].curve->get_cp0y() - minY)*scaley - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[0].z = (mPos[s].curve->get_cp0z() - minZ)*scalez - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[0].x = (mPos[s].curve->get_cp0x() - minX)*scalex + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[0].y = (mPos[s].curve->get_cp0y() - minY)*scaley + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[0].z = (mPos[s].curve->get_cp0z() - minZ)*scalez + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[1].x = (mPos[s].curve->get_cp1x() - minX)*scalex - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[1].y = (mPos[s].curve->get_cp1y() - minY)*scaley - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[1].z = (mPos[s].curve->get_cp1z() - minZ)*scalez - GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[1].x = (mPos[s].curve->get_cp1x() - minX)*scalex + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[1].y = (mPos[s].curve->get_cp1y() - minY)*scaley + GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_max[1].z = (mPos[s].curve->get_cp1z() - minZ)*scalez + GetRectHandleWidth(zoom, scale);

                // Test each each Oriented Bounding Box (OBB).
                for (size_t i = 0; i < 2; i++)
                {
                    if (VectorMath::TestRayOBBIntersection2D(
                        ray_origin,
                        cp_handle_aabb_min[i],
                        cp_handle_aabb_max[i],
                        ModelMatrix)
                        ) {
                        handle = ((i == 0) ? s | 0x4000 : s | 0x8000);
                        return_value = wxCURSOR_HAND;
                        break;
                    }
                }
            }
        }
    }

    // test the normal handles
    if (handle == NO_HANDLE) {
        // Test each each Oriented Bounding Box (OBB).
        for (size_t i = 1; i < mSelectableHandles; i++)
        {
            if (VectorMath::TestRayOBBIntersection2D(
                ray_origin,
                handle_aabb_min[i],
                handle_aabb_max[i],
                ModelMatrix)
                ) {
                handle = i;
                return_value = wxCURSOR_HAND;
                break;
            }
        }
    }

    // test for over a segment
    if (handle == NO_HANDLE) {
        for (int i = 0; i < num_points - 1; ++i) {
            if (mPos[i].has_curve) {
                if (mPos[i].curve->HitTest(ray_origin)) {
                    if (i != selected_segment) {
                        handle = i | 0x10000;
                        return_value = wxCURSOR_BULLSEYE;
                    }
                    break;
                }
            } else {
                if (VectorMath::TestRayOBBIntersection2D(
                    ray_origin,
                    seg_aabb_min[i],
                    seg_aabb_max[i],
                    *mPos[i].mod_matrix)
                    ) {
                    if (i != selected_segment) {
                        handle = i | 0x10000;
                        return_value = wxCURSOR_BULLSEYE;
                    }
                    break;
                }
            }
        }
    }

    // test for clicking a boundary handle
    if (handle == NO_HANDLE) {
        for (size_t h = num_points + 1; h < num_points + 5; h++) {
            handle_aabb_min[h].x = mHandlePosition[h].x - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[h].y = mHandlePosition[h].y - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[h].z = mHandlePosition[h].z - GetRectHandleWidth(zoom, scale);
            handle_aabb_max[h].x = mHandlePosition[h].x + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[h].y = mHandlePosition[h].y + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[h].z = mHandlePosition[h].z + GetRectHandleWidth(zoom, scale);

            // Test each each Oriented Bounding Box (OBB).
            if (VectorMath::TestRayOBBIntersection2D(
                ray_origin,
                handle_aabb_min[h],
                handle_aabb_max[h],
                Identity)
                ) {
                handle = h;
                return_value = wxCURSOR_HAND;
                break;
            }
        }
    }

    return return_value;
}

void PolyPointScreenLocation::SetActiveHandle(int handle)
{
    selected_handle = handle;
    active_handle = handle;
    highlighted_handle = -1;
    SetAxisTool(axis_tool);  // run logic to disallow certain tools
}

void PolyPointScreenLocation::SetAxisTool(int mode)
{
    if (active_handle == CENTER_HANDLE) {
        ModelScreenLocation::SetAxisTool(mode);
    }
    else {
        if (mode == TOOL_TRANSLATE || mode == TOOL_XY_TRANS) {
            axis_tool = mode;
        }
        else {
            axis_tool = TOOL_TRANSLATE;
        }
    }
}

void PolyPointScreenLocation::AdvanceAxisTool()
{
    if (active_handle == CENTER_HANDLE) {
        ModelScreenLocation::AdvanceAxisTool();
    }
    else {
        axis_tool = TOOL_TRANSLATE;
    }
}

void PolyPointScreenLocation::SetActiveAxis(int axis)
{
   ModelScreenLocation::SetActiveAxis(axis);
}

void PolyPointScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va, float zoom, int scale) const {
    std::unique_lock<std::mutex> locker(_mutex);

    if (active_handle != NO_HANDLE || mouse_down) {
        va.PreAlloc(10 * num_points + 12);
        xlColor h1c, h2c, h3c;
        if (_locked)
        {
            h1c = xlREDTRANSLUCENT;
            h2c = xlREDTRANSLUCENT;
            h3c = xlREDTRANSLUCENT;
        }
        else {
            h1c = (highlighted_handle == START_HANDLE) ? xlYELLOWTRANSLUCENT : xlGREENTRANSLUCENT;
            h2c = xlBLUETRANSLUCENT;
            h3c = (highlighted_handle == CENTER_HANDLE) ? xlYELLOWTRANSLUCENT : xlORANGETRANSLUCENT;
        }

        // add center handle
        float cx, cy, cz;
        if (!mouse_down) {
            cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
            cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
            cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;
            mHandlePosition[CENTER_HANDLE].x = cx;
            mHandlePosition[CENTER_HANDLE].y = cy;
            mHandlePosition[CENTER_HANDLE].z = cz;
            handle_aabb_min[CENTER_HANDLE].x = (maxX - minX) * scalex / 2.0f - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[CENTER_HANDLE].y = (maxY - minY) * scaley / 2.0f - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[CENTER_HANDLE].z = (maxZ - minZ) * scalez / 2.0f - GetRectHandleWidth(zoom, scale);
            handle_aabb_max[CENTER_HANDLE].x = (maxX - minX) * scalex / 2.0f + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[CENTER_HANDLE].y = (maxY - minY) * scaley / 2.0f + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[CENTER_HANDLE].z = (maxZ - minZ) * scalez / 2.0f + GetRectHandleWidth(zoom, scale);
        }
        DrawGLUtils::DrawSphere(mHandlePosition[CENTER_HANDLE].x, mHandlePosition[CENTER_HANDLE].y, mHandlePosition[CENTER_HANDLE].z, GetRectHandleWidth(zoom, scale), h3c, va);

        for (int i = 0; i < num_points - 1; ++i) {
            int x1_pos = mPos[i].x * scalex + worldPos_x;
            int x2_pos = mPos[i + 1].x * scalex + worldPos_x;
            int y1_pos = mPos[i].y * scaley + worldPos_y;
            int y2_pos = mPos[i + 1].y * scaley + worldPos_y;
            int z1_pos = mPos[i].z * scalez + worldPos_z;
            int z2_pos = mPos[i + 1].z * scalez + worldPos_z;

            if (i == selected_segment) {
                va.Finish(GL_TRIANGLES);
                if (!mPos[i].has_curve) {
                    va.AddVertex(x1_pos, y1_pos, z1_pos, xlMAGENTA);
                    va.AddVertex(x2_pos, y2_pos, z2_pos, xlMAGENTA);
                }
                else {
                    // draw bezier curve
                    x1_pos = mPos[i].curve->get_px(0) * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_py(0) * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_pz(0) * scalez + worldPos_z;
                    for (int x = 1; x < mPos[i].curve->GetNumPoints(); ++x) {
                        x2_pos = mPos[i].curve->get_px(x) * scalex + worldPos_x;
                        y2_pos = mPos[i].curve->get_py(x) * scaley + worldPos_y;
                        z2_pos = mPos[i].curve->get_pz(x) * scalez + worldPos_z;
                        va.AddVertex(x1_pos, y1_pos, z1_pos, xlMAGENTA);
                        va.AddVertex(x2_pos, y2_pos, z2_pos, xlMAGENTA);
                        x1_pos = x2_pos;
                        y1_pos = y2_pos;
                        z1_pos = z2_pos;
                    }
                    // draw control lines
                    x1_pos = mPos[i].curve->get_p0x() * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_p0y() * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_p0z() * scalez + worldPos_z;
                    x2_pos = mPos[i].curve->get_cp0x() * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_cp0y() * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_cp0z() * scalez + worldPos_z;
                    va.AddVertex(x1_pos, y1_pos, z1_pos, xlRED);
                    va.AddVertex(x2_pos, y2_pos, z2_pos, xlRED);
                    x1_pos = mPos[i].curve->get_p1x() * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_p1y() * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_p1z() * scalez + worldPos_z;
                    x2_pos = mPos[i].curve->get_cp1x() * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_cp1y() * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_cp1z() * scalez + worldPos_z;
                    va.AddVertex(x1_pos, y1_pos, z1_pos, xlRED);
                    va.AddVertex(x2_pos, y2_pos, z2_pos, xlRED);
                }
                va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
            }

            // add handle for start of this vector
            float sx = mPos[i].x * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
            float sy = mPos[i].y * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
            float sz = mPos[i].z * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
            int hpos = i + 1;
            DrawGLUtils::DrawSphere(sx, sy, sz, GetRectHandleWidth(zoom, scale), i == 0 ? h1c : (hpos == highlighted_handle ? xlYELLOW : h2c), va);
            mHandlePosition[hpos].x = sx;
            mHandlePosition[hpos].y = sy;
            mHandlePosition[hpos].z = sz;
            handle_aabb_min[hpos].x = (mPos[i].x - minX)*scalex - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[hpos].y = (mPos[i].y - minY)*scaley - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[hpos].z = (mPos[i].z - minZ)*scalez - GetRectHandleWidth(zoom, scale);
            handle_aabb_max[hpos].x = (mPos[i].x - minX)*scalex + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[hpos].y = (mPos[i].y - minY)*scaley + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[hpos].z = (mPos[i].z - minZ)*scalez + GetRectHandleWidth(zoom, scale);

            // add final handle
            if (i == num_points - 2) {
                hpos++;
                sx = mPos[i + 1].x * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
                sy = mPos[i + 1].y * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
                sz = mPos[i + 1].z * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
                DrawGLUtils::DrawSphere(sx, sy, sz, GetRectHandleWidth(zoom, scale), (hpos == highlighted_handle ? xlYELLOW : h2c), va);
                mHandlePosition[hpos].x = sx;
                mHandlePosition[hpos].y = sy;
                mHandlePosition[hpos].z = sz;
                handle_aabb_min[hpos].x = (mPos[i + 1].x - minX)*scalex - GetRectHandleWidth(zoom, scale);
                handle_aabb_min[hpos].y = (mPos[i + 1].y - minY)*scaley - GetRectHandleWidth(zoom, scale);
                handle_aabb_min[hpos].z = (mPos[i + 1].z - minZ)*scalez - GetRectHandleWidth(zoom, scale);
                handle_aabb_max[hpos].x = (mPos[i + 1].x - minX)*scalex + GetRectHandleWidth(zoom, scale);
                handle_aabb_max[hpos].y = (mPos[i + 1].y - minY)*scaley + GetRectHandleWidth(zoom, scale);
                handle_aabb_max[hpos].z = (mPos[i + 1].z - minZ)*scalez + GetRectHandleWidth(zoom, scale);
            }
        }

        glm::vec3 cp_handle_pos[2];
        if (selected_segment != -1) {
            // add control point handles for selected segments
            int i = selected_segment;
            if (mPos[i].has_curve) {
                float cxx = mPos[i].curve->get_cp0x() * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
                float cyy = mPos[i].curve->get_cp0y() * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
                float czz = mPos[i].curve->get_cp0z() * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
                h2c = highlighted_handle & 0x4000 ? ((highlighted_handle & 0xFFF) == i ? xlYELLOW : xlRED) : xlRED;
                DrawGLUtils::DrawSphere(cxx, cyy, czz, GetRectHandleWidth(zoom, scale), h2c, va);
                mPos[i].cp0.x = mPos[i].curve->get_cp0x();
                mPos[i].cp0.y = mPos[i].curve->get_cp0y();
                mPos[i].cp0.z = mPos[i].curve->get_cp0z();
                cp_handle_pos[0].x = cxx;
                cp_handle_pos[0].y = cyy;
                cp_handle_pos[0].z = czz;
                cxx = mPos[i].curve->get_cp1x() * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
                cyy = mPos[i].curve->get_cp1y() * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
                czz = mPos[i].curve->get_cp1z() * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
                h2c = highlighted_handle & 0x8000 ? ((highlighted_handle & 0xFFF) == i ? xlYELLOW : xlRED) : xlRED;
                DrawGLUtils::DrawSphere(cxx, cyy, czz, GetRectHandleWidth(zoom, scale), h2c, va);
                mPos[i].cp1.x = mPos[i].curve->get_cp1x();
                mPos[i].cp1.y = mPos[i].curve->get_cp1y();
                mPos[i].cp1.z = mPos[i].curve->get_cp1z();
                cp_handle_pos[1].x = cxx;
                cp_handle_pos[1].y = cyy;
                cp_handle_pos[1].z = czz;
            }
        }

        if (!_locked) {
            if ((active_handle & 0x4000) > 0) {
                active_handle_pos = cp_handle_pos[0];
            }
            else if ((active_handle & 0x8000) > 0) {
                active_handle_pos = cp_handle_pos[1];
            }
            else {
                active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
            }
            DrawAxisTool(active_handle_pos, va, zoom, scale);
            if (active_axis != -1) {
                LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
                if (axis_tool == TOOL_XY_TRANS) {
                    va.AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va.AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va.AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                    va.AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                }
                switch (active_axis)
                {
                case X_AXIS:
                    va.AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    va.AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    break;
                case Y_AXIS:
                    va.AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                    va.AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                    break;
                case Z_AXIS:
                    va.AddVertex(active_handle_pos.x, active_handle_pos.y, -1000000.0f, xlBLUETRANSLUCENT);
                    va.AddVertex(active_handle_pos.x, active_handle_pos.y, +1000000.0f, xlBLUETRANSLUCENT);
                    break;
                }
                va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
            }
        }
    }
    else {
        // draw bounding box for each segment if model is highlighted
        xlColor Box3dColor = xlWHITETRANSLUCENT;
        if (_locked) Box3dColor = xlREDTRANSLUCENT;
        for (int i = 0; i < num_points - 1; ++i) {
            if (mPos[i].has_curve) {
                mPos[i].curve->DrawBoundingBoxes(Box3dColor, va);
            }
            else {
                DrawGLUtils::DrawBoundingBox(Box3dColor, seg_aabb_min[i], seg_aabb_max[i], *mPos[i].mod_matrix, va);
            }
        }
    }

    va.Finish(GL_TRIANGLES);
}

void PolyPointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va, float zoom, int scale) const {
    std::unique_lock<std::mutex> locker(_mutex);
    va.PreAlloc(10*num_points+12);

    // add boundary handles
    float boundary_offset = 2.0f * GetRectHandleWidth(zoom, scale);
    float x1 = minX * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2 - boundary_offset;
    float y1 = minY * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2 - boundary_offset;
    float z1 = minZ * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2 - boundary_offset;
    float x2 = maxX * scalex + worldPos_x + GetRectHandleWidth(zoom, scale) / 2 + boundary_offset;
    float y2 = maxY * scaley + worldPos_y + GetRectHandleWidth(zoom, scale) / 2 + boundary_offset;
    float z2 = maxZ * scalez + worldPos_z + GetRectHandleWidth(zoom, scale) / 2 + boundary_offset;
    xlColor handleColor = xlBLUETRANSLUCENT;
    if (_locked)
    {
        handleColor = xlREDTRANSLUCENT;
    }
    va.AddRect(x1, y1, x1 + GetRectHandleWidth(zoom, scale), y1 + GetRectHandleWidth(zoom, scale), handleColor);
    va.AddRect(x1, y2, x1 + GetRectHandleWidth(zoom, scale), y2 + GetRectHandleWidth(zoom, scale), handleColor);
    va.AddRect(x2, y1, x2 + GetRectHandleWidth(zoom, scale), y1 + GetRectHandleWidth(zoom, scale), handleColor);
    va.AddRect(x2, y2, x2 + GetRectHandleWidth(zoom, scale), y2 + GetRectHandleWidth(zoom, scale), handleColor);
    while (mHandlePosition.size() < num_points + 5) // not sure this is the best way to do this but it stops a crash
    {
        xlPoint pt;
        mHandlePosition.push_back(pt);
    }
    mHandlePosition[num_points+1].x = x1;
    mHandlePosition[num_points+1].y = y1;
    mHandlePosition[num_points+1].z = z1;
    mHandlePosition[num_points+2].x = x1;
    mHandlePosition[num_points+2].y = y2;
    mHandlePosition[num_points+2].z = z2;
    mHandlePosition[num_points+3].x = x2;
    mHandlePosition[num_points+3].y = y1;
    mHandlePosition[num_points+3].z = z1;
    mHandlePosition[num_points+4].x = x2;
    mHandlePosition[num_points+4].y = y2;
    mHandlePosition[num_points+4].z = z2;

    for( int i = 0; i < num_points-1; ++i ) {
        int x1_pos = mPos[i].x * scalex + worldPos_x;
        int x2_pos = mPos[i+1].x * scalex + worldPos_x;
        int y1_pos = mPos[i].y * scaley + worldPos_y;
        int y2_pos = mPos[i+1].y * scaley + worldPos_y;
        int z1_pos = mPos[i].z * scalez + worldPos_z;
        int z2_pos = mPos[i+1].z * scalez + worldPos_z;

        if( i == selected_segment ) {
            va.Finish(GL_TRIANGLES);
            if( !mPos[i].has_curve ) {
                va.AddVertex(x1_pos, y1_pos, xlMAGENTA);
                va.AddVertex(x2_pos, y2_pos, xlMAGENTA);
            } else {
                // draw bezier curve
                x1_pos = mPos[i].curve->get_px(0) * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_py(0) * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_pz(0) * scalez + worldPos_z;
                for( int x = 1; x < mPos[i].curve->GetNumPoints(); ++x ) {
                    x2_pos = mPos[i].curve->get_px(x) * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_py(x) * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_pz(x) * scalez + worldPos_z;
                    va.AddVertex(x1_pos, y1_pos, xlMAGENTA);
                    va.AddVertex(x2_pos, y2_pos, xlMAGENTA);
                    x1_pos = x2_pos;
                    y1_pos = y2_pos;
                    z1_pos = z2_pos;
                }
                // draw control lines
                x1_pos = mPos[i].curve->get_p0x() * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_p0y() * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_p0z() * scalez + worldPos_z;
                x2_pos = mPos[i].curve->get_cp0x() * scalex + worldPos_x;
                y2_pos = mPos[i].curve->get_cp0y() * scaley + worldPos_y;
                z2_pos = mPos[i].curve->get_cp0z() * scalez + worldPos_z;
                va.AddVertex(x1_pos, y1_pos, xlRED);
                va.AddVertex(x2_pos, y2_pos, xlRED);
                x1_pos = mPos[i].curve->get_p1x() * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_p1y() * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_p1z() * scalez + worldPos_z;
                x2_pos = mPos[i].curve->get_cp1x() * scalex + worldPos_x;
                y2_pos = mPos[i].curve->get_cp1y() * scaley + worldPos_y;
                z2_pos = mPos[i].curve->get_cp1z() * scalez + worldPos_z;
                va.AddVertex(x1_pos, y1_pos, xlRED);
                va.AddVertex(x2_pos, y2_pos, xlRED);
            }
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
        }

        // add handle for start of this vector
        float sx = mPos[i].x * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
        float sy = mPos[i].y * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
        float sz = mPos[i].z * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
        va.AddRect(sx, sy, sx + GetRectHandleWidth(zoom, scale), sy + GetRectHandleWidth(zoom, scale), i == (selected_handle-1) ? xlMAGENTATRANSLUCENT : (i == 0 ? xlGREENTRANSLUCENT : handleColor));
        int hpos = i + 1;
        mHandlePosition[hpos].x = sx;
        mHandlePosition[hpos].y = sy;
        mHandlePosition[hpos].z = sz;
        handle_aabb_min[hpos].x = (mPos[i].x - minX)*scalex - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[hpos].y = (mPos[i].y - minY)*scaley - GetRectHandleWidth(zoom, scale);
        handle_aabb_min[hpos].z = (mPos[i].z - minZ)*scalez - GetRectHandleWidth(zoom, scale);
        handle_aabb_max[hpos].x = (mPos[i].x - minX)*scalex + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[hpos].y = (mPos[i].y - minY)*scaley + GetRectHandleWidth(zoom, scale);
        handle_aabb_max[hpos].z = (mPos[i].z - minZ)*scalez + GetRectHandleWidth(zoom, scale);

        // add final handle
        if( i == num_points-2 ) {
            sx = mPos[i+1].x * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
            sy = mPos[i+1].y * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
            sz = mPos[i+1].z * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
            va.AddRect(sx, sy, sx + GetRectHandleWidth(zoom, scale), sy + GetRectHandleWidth(zoom, scale), i+1 == (selected_handle - 1) ? xlMAGENTATRANSLUCENT : handleColor);
            hpos++;
            mHandlePosition[hpos].x = sx;
            mHandlePosition[hpos].y = sy;
            mHandlePosition[hpos].z = sz;
            handle_aabb_min[hpos].x = (mPos[i+1].x - minX)*scalex - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[hpos].y = (mPos[i+1].y - minY)*scaley - GetRectHandleWidth(zoom, scale);
            handle_aabb_min[hpos].z = (mPos[i+1].z - minZ)*scalez - GetRectHandleWidth(zoom, scale);
            handle_aabb_max[hpos].x = (mPos[i+1].x - minX)*scalex + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[hpos].y = (mPos[i+1].y - minY)*scaley + GetRectHandleWidth(zoom, scale);
            handle_aabb_max[hpos].z = (mPos[i+1].z - minZ)*scalez + GetRectHandleWidth(zoom, scale);
        }
    }

    if( selected_segment != -1 ) {
        // add control point handles for selected segments
        int i = selected_segment;
        if( mPos[i].has_curve ) {
            float cx = mPos[i].curve->get_cp0x() * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
            float cy = mPos[i].curve->get_cp0y() * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
            float cz = mPos[i].curve->get_cp0z() * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
            va.AddRect(cx, cy, cx + GetRectHandleWidth(zoom, scale), cy + GetRectHandleWidth(zoom, scale), xlREDTRANSLUCENT);
            mPos[i].cp0.x = mPos[i].curve->get_cp0x();
            mPos[i].cp0.y = mPos[i].curve->get_cp0y();
            mPos[i].cp0.z = mPos[i].curve->get_cp0z();
            cx = mPos[i].curve->get_cp1x() * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
            cy = mPos[i].curve->get_cp1y() * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
            cz = mPos[i].curve->get_cp1z() * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
            va.AddRect(cx, cy, cx + GetRectHandleWidth(zoom, scale), cy + GetRectHandleWidth(zoom, scale), xlREDTRANSLUCENT);
            mPos[i].cp1.x = mPos[i].curve->get_cp1x();
            mPos[i].cp1.y = mPos[i].curve->get_cp1y();
            mPos[i].cp1.z = mPos[i].curve->get_cp1z();
        }
    }

    va.Finish(GL_TRIANGLES);
}

int PolyPointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (_locked) return 0;
    std::unique_lock<std::mutex> locker(_mutex);

    if (handle != CENTER_HANDLE) {

        if (axis_tool == TOOL_TRANSLATE) {
            if (latch) {
                saved_position.x = active_handle_pos.x;
                saved_position.y = active_handle_pos.y;
                saved_position.z = active_handle_pos.z;
            }

            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

            float newx = (saved_position.x + drag_delta.x - worldPos_x) / scalex;
            float newy = (saved_position.y + drag_delta.y - worldPos_y) / scaley;
            float newz = (saved_position.z + drag_delta.z - worldPos_z) / scalez;

            // check for control point handles
            if (handle & 0x4000) {
                int seg = handle & 0x0FFF;
                switch (active_axis) {
                case X_AXIS:
                    mPos[seg].cp0.x = newx;
                    break;
                case Y_AXIS:
                    mPos[seg].cp0.y = newy;
                    break;
                case Z_AXIS:
                    mPos[seg].cp0.z = newz;
                    break;
                }
                mPos[seg].curve->set_cp0(mPos[seg].cp0.x, mPos[seg].cp0.y, mPos[seg].cp0.z);
            }
            else if (handle & 0x8000) {
                int seg = handle & 0x0FFF;
                switch (active_axis) {
                case X_AXIS:
                    mPos[seg].cp1.x = newx;
                    break;
                case Y_AXIS:
                    mPos[seg].cp1.y = newy;
                    break;
                case Z_AXIS:
                    mPos[seg].cp1.z = newz;
                    break;
                }
                mPos[seg].curve->set_cp1(mPos[seg].cp1.x, mPos[seg].cp1.y, mPos[seg].cp1.z);
                FixCurveHandles();
            }
            else {
                int point = handle - 1;
                switch (active_axis) {
                case X_AXIS:
                    mPos[point].x = newx;
                    break;
                case Y_AXIS:
                    mPos[point].y = newy;
                    break;
                case Z_AXIS:
                    mPos[point].z = newz;
                    break;
                }
                FixCurveHandles();
            }
        }
        else if (axis_tool == TOOL_XY_TRANS) {
            if (latch) {
                saved_position.x = mHandlePosition[1].x * scalex + worldPos_x;
                saved_position.y = mHandlePosition[1].y * scaley + worldPos_y;
                saved_position.z = 0.0f;
            }

            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

            float newx = (saved_position.x + drag_delta.x - worldPos_x) / scalex;
            float newy = (saved_position.y + drag_delta.y - worldPos_y) / scaley;
            int point = handle - 1;
            mPos[point].x = newx;
            mPos[point].y = newy;
        }

    }
    else {
        if (axis_tool == TOOL_TRANSLATE) {
            if (latch) {
                saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            }
            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;
            switch (active_axis)
            {
            case X_AXIS:
                worldPos_x = saved_position.x + drag_delta.x;
                break;
            case Y_AXIS:
                worldPos_y = saved_position.y + drag_delta.y;
                break;
            case Z_AXIS:
                worldPos_z = saved_position.z + drag_delta.z;
                break;
            }
        }
        else if (axis_tool == TOOL_ROTATE) {
            if (latch) {
                center.x = mHandlePosition[CENTER_HANDLE].x;
                center.y = mHandlePosition[CENTER_HANDLE].y;
                center.z = mHandlePosition[CENTER_HANDLE].z;
                saved_position = center;
                saved_angle = 0.0f;
            }
            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;
            double angle = 0.0f;
            float new_angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;

            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.x = angle;
                new_angle = saved_angle - angle;
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.y = angle;
                new_angle = angle - saved_angle;
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                angles.z = angle;
                new_angle = angle - saved_angle;
            }
            break;
            }
            rotate_pt = center;
            Rotate(active_axis, new_angle);
            saved_angle = angle;
        }
        else if (axis_tool == TOOL_SCALE) {
            if (latch) {
                saved_position = glm::vec3(mHandlePosition[0].x, mHandlePosition[0].y, mHandlePosition[0].z);
                saved_point = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            }
            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;
            float change_x = (saved_position.x - worldPos_x + drag_delta.x) / (saved_position.x - worldPos_x);
            float change_y = (saved_position.y - worldPos_y + drag_delta.y) / (saved_position.y - worldPos_y);
            float change_z = (saved_position.z - worldPos_z + drag_delta.z) / (saved_position.z - worldPos_z);

            if (ShiftKeyPressed) {
                float change = 1.0f;
                switch (active_axis)
                {
                case X_AXIS:
                    change = change_x;
                    break;
                case Y_AXIS:
                    change = change_y;
                    break;
                case Z_AXIS:
                    change = change_z;
                    break;
                }
                float new_half_size_x = (saved_position.x - saved_point.x) * change;
                if (new_half_size_x < 0.0f) return 0;
                float new_half_size_y = (saved_position.y - saved_point.y) * change;
                if (new_half_size_y < 0.0f) return 0;
                float new_half_size_z = (saved_position.z - saved_point.z) * change;
                if (new_half_size_z < 0.0f) return 0;
                scalex = saved_scale.x * change;
                scaley = saved_scale.y * change;
                scalez = saved_scale.z * change;
                if (!CtrlKeyPressed) {
                    float change_size_x = new_half_size_x - saved_position.x + saved_point.x;
                    worldPos_x = saved_point.x - change_size_x;
                    float change_size_y = new_half_size_y - saved_position.y + saved_point.y;
                    worldPos_y = saved_point.y - change_size_y;
                    float change_size_z = new_half_size_z - saved_position.z + saved_point.z;
                    worldPos_z = saved_point.z - change_size_z;
                }
            }

            switch (active_axis)
            {
            case X_AXIS:
            {
                float new_half_size_x = (saved_position.x - saved_point.x) * change_x;
                if (new_half_size_x < 0.0f) return 0;
                scalex = saved_scale.x * change_x;
                if (!CtrlKeyPressed) {
                    float change_size_x = new_half_size_x - saved_position.x + saved_point.x;
                    worldPos_x = saved_point.x - change_size_x;
                }
            }
            break;
            case Y_AXIS:
            {
                float new_half_size_y = (saved_position.y - saved_point.y) * change_y;
                if (new_half_size_y < 0.0f) return 0;
                scaley = saved_scale.y * change_y;
                if (!CtrlKeyPressed) {
                    float change_size_y = new_half_size_y - saved_position.y + saved_point.y;
                    worldPos_y = saved_point.y - change_size_y;
                }
            }
                break;
            case Z_AXIS:
            {
                float new_half_size_z = (saved_position.z - saved_point.z) * change_z;
                if (new_half_size_z < 0.0f) return 0;
                scalez = saved_scale.z * change_z;
                if (!CtrlKeyPressed) {
                    float change_size_z = new_half_size_z - saved_position.z + saved_point.z;
                    worldPos_z = saved_point.z - change_size_z;
                }
            }
                break;
            }
        }
    }
    return 1;
}

int PolyPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    float zoom = preview->GetCameraZoomForHandles();
    int scale = preview->GetHandleScale();

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    float newx = (ray_origin.x - worldPos_x) / scalex;
    float newy = (ray_origin.y - worldPos_y) / scaley;
    float newz = 0.0f;

    // check for control point handles
    if( handle & 0x4000 ) {
        int seg = handle & 0x0FFF;
        mPos[seg].cp0.x = ray_origin.x - GetRectHandleWidth(zoom, scale) / 2;
        mPos[seg].cp0.y = ray_origin.y - GetRectHandleWidth(zoom, scale) / 2;
        mPos[seg].cp0.z = newz;
        mPos[seg].curve->set_cp0( newx, newy, newz );
    } else if( handle & 0x8000 ) {
        int seg = handle & 0x0FFF;
        mPos[seg].cp1.x = ray_origin.x - GetRectHandleWidth(zoom, scale) / 2;
        mPos[seg].cp1.y = ray_origin.y - GetRectHandleWidth(zoom, scale) / 2;
        mPos[seg].cp1.z = newz;
        mPos[seg].curve->set_cp1( newx, newy, newz );

    // check normal handles
    } else if( handle < num_points+1 ) {
        mPos[handle-1].x = newx;
        mPos[handle-1].y = newy;
        FixCurveHandles();
    } else {
        float boundary_offset = 2.0f * GetRectHandleWidth(zoom, scale);
        // move a boundary handle
        float trans_x = 0.0f;
        float trans_y = 0.0f;
        float scale_x = 1.0f;
        float scale_y = 1.0f;
        if( handle == num_points+1 ) {  // bottom-left corner
            newx = (ray_origin.x + boundary_offset - worldPos_x) / scalex;
            newy = (ray_origin.y + boundary_offset - worldPos_y) / scaley;
            if( newx >= maxX-0.01f || newy >= maxY-0.01f ) return 0;
            if (maxX - minX != 0.0f) {
                trans_x = newx - minX;
                scale_x -= trans_x / (maxX - minX);
            }
            if (maxY - minY != 0.0f) {
                trans_y = newy - minY;
                scale_y -= trans_y / (maxY - minY);
            }
        } else if( handle == num_points+2 ) {  // top left corner
            newx = (ray_origin.x + boundary_offset - worldPos_x) / scalex;
            newy = (ray_origin.y - boundary_offset - worldPos_y) / scaley;
            if( newx >= maxX-0.01f || newy <= minY+0.01f ) return 0;
            if (maxX - minX != 0.0f) {
                trans_x = newx - minX;
                scale_x -= trans_x / (maxX - minX);
            }
            if (maxY - minY != 0.0f) {
                scale_y = (newy - minY) / (maxY - minY);
            }
        } else if( handle == num_points+3 ) {  // bottom right corner
            newx = (ray_origin.x - boundary_offset - worldPos_x) / scalex;
            newy = (ray_origin.y + boundary_offset - worldPos_y) / scaley;
            if( newx <= minX+0.01f|| newy >= maxY-0.01f ) return 0;
            if (maxX - minX != 0.0f) {
                scale_x = (newx - minX) / (maxX - minX);
            }
            if (maxY - minY != 0.0f) {
                trans_y = newy - minY;
                scale_y -= trans_y / (maxY - minY);
            }
        } else if( handle == num_points+4 ) {  // top right corner
            newx = (ray_origin.x - boundary_offset - worldPos_x) / scalex;
            newy = (ray_origin.y - boundary_offset - worldPos_y) / scaley;
            if( newx <= minX+0.01f || newy <= minY+0.01f ) return 0;
            if (maxX - minX != 0.0f) {
                scale_x = (newx - minX) / (maxX - minX);
            }
            if (maxY - minY != 0.0f) {
                scale_y = (newy - minY) / (maxY - minY);
            }
        } else {
            return 0;
        }

        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scale_x, scale_y, 1.0f));
        glm::mat4 translateMatrix = glm::translate(Identity, glm::vec3( minX + trans_x, minY + trans_y, 0.0f));
        glm::mat4 mat4 = translateMatrix * scalingMatrix;

        AdjustAllHandles(mat4);
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
    std::unique_lock<std::mutex> locker(_mutex);

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    float zoom = preview->GetCameraZoomForHandles();
    int scale = preview->GetHandleScale();

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    xlPolyPoint new_point;
    new_point.x = (ray_origin.x - worldPos_x) / scalex;
    new_point.y = (ray_origin.y - worldPos_y) / scaley;
    new_point.z = 0.0f;

    if (draw_3d) {
        // use drag handle function to find plane intersection
        glm::vec3 backup = saved_position;
        glm::vec3 backup2 = saved_intersect;
        saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        DragHandle(preview, mouseX, mouseY, true);
        new_point.x = (saved_intersect.x - worldPos_x) / scalex;
        new_point.y = (saved_intersect.y - worldPos_y) / scaley;
        saved_position = backup;
        saved_intersect = backup2;
    }

    new_point.matrix = nullptr;
    new_point.mod_matrix = nullptr;
    new_point.curve = nullptr;
    new_point.has_curve = false;
    new_point.seg_scale = 1.0f;
    mPos.push_back(new_point);
    xlPoint new_handle;
    float sx = new_point.x * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
    float sy = new_point.y * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
    float sz = new_point.z * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
    new_handle.x = sx;
    new_handle.y = sy;
    new_handle.z = sz;
    mHandlePosition.insert(mHandlePosition.begin() + num_points + 1, new_handle);
    num_points++;
    mSelectableHandles++;
    handle_aabb_max.resize(num_points+5);
    handle_aabb_min.resize(num_points+5);
    seg_aabb_min.resize(num_points - 1);
    seg_aabb_max.resize(num_points - 1);

    handle_aabb_min[num_points].x = sx - GetRectHandleWidth(zoom, scale);
    handle_aabb_min[num_points].y = sy - GetRectHandleWidth(zoom, scale);
    handle_aabb_min[num_points].z = sz - GetRectHandleWidth(zoom, scale);
    handle_aabb_max[num_points].x = sx + GetRectHandleWidth(zoom, scale);
    handle_aabb_max[num_points].y = sy + GetRectHandleWidth(zoom, scale);
    handle_aabb_max[num_points].z = sz + GetRectHandleWidth(zoom, scale);
}

void PolyPointScreenLocation::InsertHandle(int after_handle, float zoom, int scale) {
    std::unique_lock<std::mutex> locker(_mutex);

    int pos = after_handle;
    float x1_pos = mPos[pos].x;
    float x2_pos = mPos[pos+1].x;
    float y1_pos = mPos[pos].y;
    float y2_pos = mPos[pos+1].y;
    float z1_pos = mPos[pos].z;
    float z2_pos = mPos[pos+1].z;
    xlPolyPoint new_point;
    new_point.x = (x1_pos+x2_pos)/2.0;
    new_point.y = (y1_pos+y2_pos)/2.0;
    new_point.z = (z1_pos+z2_pos)/2.0;
    new_point.matrix = nullptr;
    new_point.mod_matrix = nullptr;
    new_point.curve = nullptr;
    new_point.has_curve = false;
    mPos.insert(mPos.begin() + pos + 1, new_point);
    xlPoint new_handle;
    float sx = mPos[pos+1].x * scalex + worldPos_x - GetRectHandleWidth(zoom, scale) / 2;
    float sy = mPos[pos+1].y * scaley + worldPos_y - GetRectHandleWidth(zoom, scale) / 2;
    float sz = mPos[pos+1].z * scalez + worldPos_z - GetRectHandleWidth(zoom, scale) / 2;
    new_handle.x = sx;
    new_handle.y = sy;
    new_handle.z = sz;
    mHandlePosition.insert(mHandlePosition.begin() + after_handle + 1, new_handle);
    num_points++;
    selected_handle = after_handle+1;
    selected_segment = -1;
    mSelectableHandles++;
    handle_aabb_max.resize(num_points+5);
    handle_aabb_min.resize(num_points+5);
    seg_aabb_min.resize(num_points - 1);
    seg_aabb_max.resize(num_points - 1);

    handle_aabb_min[num_points].x = sx - GetRectHandleWidth(zoom, scale);
    handle_aabb_min[num_points].y = sy - GetRectHandleWidth(zoom, scale);
    handle_aabb_min[num_points].z = sz - GetRectHandleWidth(zoom, scale);
    handle_aabb_max[num_points].x = sx + GetRectHandleWidth(zoom, scale);
    handle_aabb_max[num_points].y = sy + GetRectHandleWidth(zoom, scale);
    handle_aabb_max[num_points].z = sz + GetRectHandleWidth(zoom, scale);
}

void PolyPointScreenLocation::DeleteHandle(int handle) {
    
    // this can happen if you click one one of the box handles
    if (handle >= mPos.size()) return;

    // delete any curves associated with this handle
    if( mPos[handle].has_curve ) {
        mPos[handle].has_curve = false;
        if( mPos[handle].curve != nullptr ) {
            delete mPos[handle].curve;
            mPos[handle].curve = nullptr;
        }
    }
    if(handle > 0 ) {
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
    mHandlePosition.erase(mHandlePosition.begin() + handle + 1);
    num_points--;
    selected_handle = -1;
    selected_segment = -1;
}

wxCursor PolyPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {
    float zoom = 1.0;
    int scale = 1;
    if (preview != nullptr) {
        zoom = preview->GetCameraZoomForHandles();
        scale = preview->GetHandleScale();
        active_axis = X_AXIS;
        saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
        DragHandle(preview, x, y, true);
        worldPos_x = saved_intersect.x;
        worldPos_y = saved_intersect.y;
        worldPos_z = 0.0f;
        if (preview->Is3D()) {
            // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
            // of the mouse click with the ground plane
            active_handle = 2;
        }
    }
    else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }

    mPos[0].x = 0.0f;
    mPos[0].y = 0.0f;
    mPos[0].z = 0.0f;
    mPos[1].x = 0.0f;
    mPos[1].y = 0.0f;
    mPos[1].z = 0.0f;

    handle_aabb_min[0].x = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[0].y = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[0].z = -GetRectHandleWidth(zoom, scale);
    handle_aabb_max[0].x = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[0].y = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[0].z = GetRectHandleWidth(zoom, scale);
    handle_aabb_min[1].x = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[1].y = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[1].z = -GetRectHandleWidth(zoom, scale);
    handle_aabb_max[1].x = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[1].y = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[1].z = GetRectHandleWidth(zoom, scale);
    handle_aabb_min[2].x = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[2].y = -GetRectHandleWidth(zoom, scale);
    handle_aabb_min[2].z = -GetRectHandleWidth(zoom, scale);
    handle_aabb_max[2].x = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[2].y = GetRectHandleWidth(zoom, scale);
    handle_aabb_max[2].z = GetRectHandleWidth(zoom, scale);

    handle = 2;
    return wxCURSOR_SIZING;
}

void PolyPointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxBoolProperty("Locked", "Locked", _locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = propertyEditor->Append(new wxFloatProperty("X1", "ModelX1", mPos[0].x + worldPos_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = propertyEditor->Append(new wxFloatProperty("Y1", "ModelY1", mPos[0].y + worldPos_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);
    prop = propertyEditor->Append(new wxFloatProperty("Z1", "ModelZ1", mPos[0].z + worldPos_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.5);
    prop->SetEditor("SpinCtrl");
    prop->SetTextColour(*wxGREEN);

    for( int i = 1; i < num_points; ++i ) {
        prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("X%d",i+1), wxString::Format("ModelX%d",i+1), mPos[i].x + worldPos_x));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(*wxBLUE);
        prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Y%d", i+1), wxString::Format("ModelY%d", i+1), mPos[i].y + worldPos_y));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Step", 0.5);
        prop->SetEditor("SpinCtrl");
        prop->SetTextColour(*wxBLUE);
        prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Z%d", i+1), wxString::Format("ModelZ%d", i+1), mPos[i].z + worldPos_z));
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
        if (!_locked && name.find("ModelX") != std::string::npos) {
            mPos[selected_handle].x = event.GetValue().GetDouble() - worldPos_x;
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelX");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyPointScreenLocation::OnPropertyGridChange::ModelX");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyPointScreenLocation::OnPropertyGridChange::ModelX");
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyPointScreenLocation::OnPropertyGridChange::ModelX");
            return 0;
        }
        else if (_locked && name.find("ModelX") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!_locked && name.find("ModelY") != std::string::npos) {
            mPos[selected_handle].y = event.GetValue().GetDouble() - worldPos_y;
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelY");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyPointScreenLocation::OnPropertyGridChange::ModelY");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyPointScreenLocation::OnPropertyGridChange::ModelY");
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyPointScreenLocation::OnPropertyGridChange::ModelY");
            return 0;
        }
        else if (_locked && name.find("ModelY") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!_locked && name.find("ModelZ") != std::string::npos) {
            mPos[selected_handle].z = event.GetValue().GetDouble() - worldPos_z;
            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::ModelZ");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyPointScreenLocation::OnPropertyGridChange::ModelZ");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyPointScreenLocation::OnPropertyGridChange::ModelZ");
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyPointScreenLocation::OnPropertyGridChange::ModelZ");
            return 0;
        }
        else if (_locked && name.find("ModelZ") != std::string::npos) {
            event.Veto();
            return 0;
        }
    }
    else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::Locked");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyPointScreenLocation::OnPropertyGridChange::Locked");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyPointScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }

    return 0;
}

void PolyPointScreenLocation::RotateAboutPoint(glm::vec3 position, glm::vec3 angle) {
    if (_locked) return;
    
    rotate_pt = position;
     if (angle.y != 0.0f) {
        float offset = angle.y;
        Rotate(Y_AXIS, offset);
    }
    else if (angle.x != 0.0f) {
        float offset = angle.x;
        Rotate(X_AXIS, offset);
    }
    else if (angle.z != 0.0f) {
        float offset = angle.z;
        Rotate(Z_AXIS, offset);
    }
}

bool PolyPointScreenLocation::Rotate(int axis, float factor) {
    if (_locked) return false;

    // Rotate all the points
    glm::mat4 translateToOrigin = glm::translate(Identity, -rotate_pt);
    glm::mat4 translateBack = glm::translate(Identity, rotate_pt);
    glm::mat4 Rotate = Identity;
    glm::vec3 pt(worldPos_x, worldPos_y, worldPos_z);

    switch (axis)
    {
    case X_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    break;
    case Y_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    break;
    case Z_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    break;
    }

    pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
    glm::vec3 world_new(pt.x, pt.y, pt.z);
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i].has_curve) {
            pt = glm::vec3(mPos[i].curve->get_cp0x() * scalex + worldPos_x, mPos[i].curve->get_cp0y() * scaley + worldPos_y, mPos[i].curve->get_cp0z() * scalez + worldPos_z);
            pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
            mPos[i].curve->set_cp0((pt.x - world_new.x) / scalex, (pt.y - world_new.y) / scaley, (pt.z - world_new.z) / scalez);
            pt = glm::vec3(mPos[i].curve->get_cp1x() * scalex + worldPos_x, mPos[i].curve->get_cp1y() * scaley + worldPos_y, mPos[i].curve->get_cp1z() * scalez + worldPos_z);
            pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
            mPos[i].curve->set_cp1((pt.x - world_new.x) / scalex, (pt.y - world_new.y) / scaley, (pt.z - world_new.z) / scalez);
        }
        pt = glm::vec3(mPos[i].x * scalex + worldPos_x, mPos[i].y * scaley + worldPos_y, mPos[i].z * scalez + worldPos_z);
        pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
        mPos[i].x = (pt.x - world_new.x) / scalex;
        mPos[i].y = (pt.y - world_new.y) / scaley;
        mPos[i].z = (pt.z - world_new.z) / scalez;
    }
    worldPos_x = world_new.x;
    worldPos_y = world_new.y;
    worldPos_z = world_new.z;

    return true;
}

bool PolyPointScreenLocation::Scale(const glm::vec3& factor) {
    if (_locked) return false;

    glm::vec3 world_pt = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    float cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
    float cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
    float cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;
    glm::mat4 translateToOrigin = glm::translate(Identity, -glm::vec3(cx, cy, cz));
    glm::mat4 translateBack = glm::translate(Identity, glm::vec3(cx, cy, cz));
    glm::mat4 scalingMatrix = glm::scale(Identity, factor);
    glm::mat4 m = translateBack * scalingMatrix * translateToOrigin;
    AdjustAllHandles(m);
    world_pt = glm::vec3(translateBack * scalingMatrix * translateToOrigin * glm::vec4(world_pt, 1.0f));
    worldPos_x = world_pt.x;
    worldPos_y = world_pt.y;
    worldPos_z = world_pt.z;

    return true;
}

void PolyPointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve) {
            mPos[i].curve->UpdateBoundingBox(draw_3d);
        }
        else {
            // create normal line segment bounding boxes
            seg_aabb_min[i] = glm::vec3(0.0f, -BB_OFF, -BB_OFF);
            seg_aabb_max[i] = glm::vec3(RenderWi * mPos[i].seg_scale, BB_OFF, BB_OFF);
        }
    }
}

glm::vec2 PolyPointScreenLocation::GetScreenOffset(ModelPreview* preview)
{
    float cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
    float cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
    float cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;

    glm::vec2 position = VectorMath::GetScreenCoord(preview->getWidth(),
        preview->getHeight(),
        glm::vec3(cx, cy, cz),           // X,Y,Z coords of the position when not transformed at all.
        preview->GetProjViewMatrix(),    // Projection / View matrix
        Identity                         // Transformation applied to the position
    );

    position.x = position.x / (float)preview->getWidth();
    position.y = position.y / (float)preview->getHeight();
    return position;
}

float PolyPointScreenLocation::GetHcenterPos() const {
    return mHandlePosition[CENTER_HANDLE].x;
}

float PolyPointScreenLocation::GetVcenterPos() const {
    return mHandlePosition[CENTER_HANDLE].y;
}

float PolyPointScreenLocation::GetDcenterPos() const {
    return mHandlePosition[CENTER_HANDLE].z;
}

void PolyPointScreenLocation::SetHcenterPos(float f) {
    worldPos_x += f - GetHcenterPos();
}

void PolyPointScreenLocation::SetVcenterPos(float f) {
    worldPos_y += f - GetVcenterPos();
}

void PolyPointScreenLocation::SetDcenterPos(float f) {
    worldPos_z += f - GetDcenterPos();
}

void PolyPointScreenLocation::SetPosition(float posx, float posy) {

    if (_locked) return;

    SetHcenterPos(posx);
    SetVcenterPos(posy);
}

float PolyPointScreenLocation::GetTop() const {
    return maxY * scaley + worldPos_y;
}

float PolyPointScreenLocation::GetLeft() const {
    return minX * scalex + worldPos_x;
}

float PolyPointScreenLocation::GetMHeight() const
{
    return maxY - minY;
}

float PolyPointScreenLocation::GetMWidth() const
{
    return maxX - minX;
}

void PolyPointScreenLocation::SetMWidth(float w)
{
    if (maxX == minX)
    {
        scalex = 1;
    }
    else
    {
        scalex = w / (maxX - minX);
    }
}

void PolyPointScreenLocation::SetMDepth(float d)
{
    if (maxZ == minZ)
    {
        scalez = 1;
    }
    else
    {
        scalez = d / (maxZ - minZ);
    }
}

float PolyPointScreenLocation::GetMDepth() const
{
    return scalez * (maxZ - minZ);
}

void PolyPointScreenLocation::SetMHeight(float h)
{
    if (maxY == minY || h == 0)
    {
        scaley = 1;
    }
    else
    {
        scaley = h / (maxY - minY);
    }
}

float PolyPointScreenLocation::GetRight() const {
    return maxX * scalex + worldPos_x;
}

float PolyPointScreenLocation::GetBottom() const {
    return minY * scaley + worldPos_y;
}

float PolyPointScreenLocation::GetFront() const {
    return maxZ * scalez + worldPos_z;
}

float PolyPointScreenLocation::GetBack() const {
    return minZ * scalez + worldPos_z;
}

void PolyPointScreenLocation::SetTop(float i) {

    if (_locked) return;

    float newtop = i;
    float topy = maxY * scaley + worldPos_y;
    float diff = newtop - topy;
    worldPos_y += diff;
}

void PolyPointScreenLocation::SetLeft(float i) {

    if (_locked) return;

    float newleft = i;
    float leftx = minX * scalex + worldPos_x;
    float diff = newleft - leftx;
    worldPos_x += diff;
}

void PolyPointScreenLocation::SetRight(float i) {

    if (_locked) return;

    float newright = i;
    float rightx = maxX * scalex + worldPos_x;
    float diff = newright - rightx;
    worldPos_x += diff;
}

void PolyPointScreenLocation::SetBottom(float i) {

    if (_locked) return;

    float newbot = i;
    float boty = minY * scaley + worldPos_y;
    float diff = newbot - boty;
    worldPos_y += diff;
}

void PolyPointScreenLocation::SetFront(float i) {

    if (_locked) return;

    float newfront = i;
    float frontz = maxZ * scalez + worldPos_z;
    float diff = newfront - frontz;
    worldPos_z += diff;
}

void PolyPointScreenLocation::SetBack(float i) {

    if (_locked) return;

    float newback = i;
    float backz = minZ * scalez + worldPos_z;
    float diff = newback - backz;
    worldPos_z += diff;
}

void PolyPointScreenLocation::FixCurveHandles() {
    for(int i = 0; i < num_points; ++i ) {
        if( mPos[i].has_curve ) {
            mPos[i].curve->set_p0( mPos[i].x, mPos[i].y, mPos[i].z );
            mPos[i].curve->set_p1( mPos[i+1].x, mPos[i+1].y, mPos[i+1].z );
            mPos[i].curve->UpdatePoints();
        }
    }
}

void PolyPointScreenLocation::AdjustAllHandles(glm::mat4& mat)
{
    for (int i = 0; i < num_points; ++i) {
        glm::vec3 v = glm::vec3(mat * glm::vec4(mPos[i].x - minX, mPos[i].y - minY, mPos[i].z - minZ, 1.0f));
        mPos[i].x = v.x;
        mPos[i].y = v.y;
        mPos[i].z = v.z;
        if (mPos[i].has_curve) {
            float x1 = mPos[i].curve->get_cp0x();
            float y1 = mPos[i].curve->get_cp0y();
            float z1 = mPos[i].curve->get_cp0z();
            v = glm::vec3(mat * glm::vec4(x1 - minX, y1 - minY, z1 - minZ, 1.0f));
            mPos[i].curve->set_cp0(v.x, v.y, v.z);
            x1 = mPos[i].curve->get_cp1x();
            y1 = mPos[i].curve->get_cp1y();
            z1 = mPos[i].curve->get_cp1z();
            v = glm::vec3(mat * glm::vec4(x1 - minX, y1 - minY, z1 - minZ, 1.0f));
            mPos[i].curve->set_cp1(v.x, v.y, v.z);
        }
    }
    FixCurveHandles();
}
