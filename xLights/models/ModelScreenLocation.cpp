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
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Model.h"
#include "../ModelPreview.h"
#include "../graphics/xlGraphicsAccumulators.h"
#include "Shapes.h"
#include "../ViewpointMgr.h"
#include "../support/VectorMath.h"
#include "UtilFunctions.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "RulerObject.h"

#include <log4cpp/Category.hh>

#define BOUNDING_RECT_OFFSET        8

constexpr float dead_zone = 10.0f;  // how many degrees close to a plane should we ignore it

static glm::mat4 Identity(glm::mat4(1.0f));

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused"
void rotate_point(float cx, float cy, float angle, float &x, float &y)
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

glm::vec3 rotationMatrixToEulerAngles(const glm::mat3 &R)
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


wxCursor GetResizeCursor(int cornerIndex, int PreviewRotation) {
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

glm::vec3 ModelScreenLocation::GetHandlePosition(int handle) const
{
    if (handle < 0 || handle >= mHandlePosition.size()) return glm::vec3(0, 0, 0);
    return glm::vec3(mHandlePosition[handle].x, mHandlePosition[handle].y, mHandlePosition[handle].z);
}

ModelScreenLocation::ModelScreenLocation(int sz) :
    ModelMatrix(Identity), mHandlePosition(sz)
{
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

ModelScreenLocation::MSLAXIS ModelScreenLocation::NextAxis(MSLAXIS axis)
{
    switch (axis) {
    case MSLAXIS::X_AXIS:
        return MSLAXIS::Y_AXIS;
    case MSLAXIS::Y_AXIS:
        return MSLAXIS::Z_AXIS;
    case MSLAXIS::Z_AXIS:
        return MSLAXIS::X_AXIS;
    default:
        break;
    }
    return MSLAXIS::X_AXIS;
}

void ModelScreenLocation::SetActiveAxis(MSLAXIS axis)
{
    if (axis_tool == MSLTOOL::TOOL_ROTATE && axis != MSLAXIS::NO_AXIS) {
        active_axis = NextAxis(axis);
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
    float rs = scale;
    rs /= 2.0;
    rs += 1.0;
    return std::max(AXIS_ARROW_LENGTH, AXIS_ARROW_LENGTH * zoom * rs);
}

float ModelScreenLocation::GetAxisHeadLength(float zoom, int scale) const
{
    static float AXIS_HEAD_LENGTH = 12.0f;
    float rs = scale;
    rs /= 2.0;
    rs += 1.0;
    return std::max(AXIS_HEAD_LENGTH, AXIS_HEAD_LENGTH * zoom * rs);
}

float ModelScreenLocation::GetAxisRadius(float zoom, int scale) const
{
    static float AXIS_RADIUS = 4.0f;
    float rs = scale;
    rs /= 2.0;
    rs += 1.0;
    return std::max(AXIS_RADIUS, AXIS_RADIUS * zoom * rs);
}

float ModelScreenLocation::GetRectHandleWidth(float zoom, int scale) const
{
    // When RECT_HANDLE_WIDTH was set to 6.0 the model handle could never draw smaller than that
    // when zoomed in and zoom is less than 1.0.  By setting this to 0.5 and then multiplying the
    // scale by 12.0 later allows the handle size to still be 6.0 when zoom is 1.0 but lets it go
    // smaller than 6.0 when zoomed in
    static float RECT_HANDLE_WIDTH = 0.5f;
    float rs = scale;
    rs /= 2.0;
    rs += 1.0;
    rs *= 12.0;
    return std::max(RECT_HANDLE_WIDTH, RECT_HANDLE_WIDTH * zoom * rs);
}

ModelScreenLocation::MSLPLANE ModelScreenLocation::GetBestIntersection( ModelScreenLocation::MSLPLANE prefer, bool& rotate, ModelPreview* preview )
{
    MSLPLANE best_plane {MSLPLANE::XZ_PLANE};

    float angleX {glm::radians(preview->GetCameraRotationX())};
    float angleY {glm::radians(preview->GetCameraRotationY())};

    bool xz_plane_ok { ((angleX > glm::radians(dead_zone)) && (angleX < glm::radians(180.0f - dead_zone))) ||
                       ((angleX > glm::radians(180.0f + dead_zone)) && (angleX < glm::radians(360.0f - dead_zone))) };

    bool xy_plane_ok { ((angleX >= glm::radians(0.0f)) && (angleX < glm::radians(90.0f - dead_zone))) ||
                       ((angleX > glm::radians(90.0f + dead_zone)) && (angleX < glm::radians(270.0f - dead_zone))) ||
                       (angleX > glm::radians(270.0f + dead_zone)) };

    if( !xz_plane_ok && !xy_plane_ok ) {
        best_plane = MSLPLANE::YZ_PLANE;
    } else if( prefer == MSLPLANE::XZ_PLANE ) {
        if( xz_plane_ok ) {
            best_plane = MSLPLANE::XZ_PLANE;
        } else if( xy_plane_ok ) {
            best_plane = MSLPLANE::XY_PLANE;
        }
    } else if( prefer == MSLPLANE::XY_PLANE ) {
        if( xy_plane_ok ) {
            best_plane = MSLPLANE::XY_PLANE;
        } else {
            best_plane = MSLPLANE::XZ_PLANE;
        }
    }

    rotate = false;
    if( (angleY > glm::radians(45.0f) && angleY < glm::radians(135.0f)) ||
        (angleY > glm::radians(225.0f) && angleY < glm::radians(315.0f)) ) {
        if( best_plane == MSLPLANE::XZ_PLANE ) {
            rotate = true;
        } else if( best_plane == MSLPLANE::XY_PLANE ) {
            rotate = true;
            best_plane = MSLPLANE::YZ_PLANE;
        }
    }

    return best_plane;
}

void ModelScreenLocation::FindPlaneIntersection( int x, int y, ModelPreview* preview )
{
    bool rotate {false};
    active_plane = MSLPLANE::XY_PLANE;  // 2D will always use the X/Y plane

    if( preview->Is3D() ) {
        active_plane = GetBestIntersection( preferred_selection_plane, rotate, preview );
    }

    if (active_plane == ModelScreenLocation::MSLPLANE::XY_PLANE) {
        active_axis = MSLAXIS::X_AXIS;
        DragHandle(preview, x, y, true);
        worldPos_x = saved_intersect.x;
        worldPos_y = saved_intersect.y;
        worldPos_z = 0.0f;
    } else if (active_plane == ModelScreenLocation::MSLPLANE::XZ_PLANE) {
        active_axis = MSLAXIS::Z_AXIS;
        DragHandle(preview, x, y, true);
        worldPos_x = saved_intersect.x;
        worldPos_y = 0.0f;
        worldPos_z = saved_intersect.z;
        if( rotate ) {
            active_axis = MSLAXIS::Z_AXIS;
        } else {
            active_axis = MSLAXIS::X_AXIS;
        }
    } else {
        active_axis = MSLAXIS::Z_AXIS;
        DragHandle(preview, x, y, true);
        worldPos_x = 0.0f;
        worldPos_y = saved_intersect.y;
        worldPos_z = saved_intersect.z;
    }
}

void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va) {
    glm::vec4 c1(min_pt.x, max_pt.y, min_pt.z, 1.0f);
    glm::vec4 c2(max_pt.x, max_pt.y, min_pt.z, 1.0f);
    glm::vec4 c3(max_pt.x, min_pt.y, min_pt.z, 1.0f);
    glm::vec4 c4(min_pt.x, min_pt.y, min_pt.z, 1.0f);
    glm::vec4 c5(min_pt.x, max_pt.y, max_pt.z, 1.0f);
    glm::vec4 c6(max_pt.x, max_pt.y, max_pt.z, 1.0f);
    glm::vec4 c7(max_pt.x, min_pt.y, max_pt.z, 1.0f);
    glm::vec4 c8(min_pt.x, min_pt.y, max_pt.z, 1.0f);

    c1 = bound_matrix * c1;
    c2 = bound_matrix * c2;
    c3 = bound_matrix * c3;
    c4 = bound_matrix * c4;
    c5 = bound_matrix * c5;
    c6 = bound_matrix * c6;
    c7 = bound_matrix * c7;
    c8 = bound_matrix * c8;

    va.AddVertex(c1.x, c1.y, c1.z, c);
    va.AddVertex(c2.x, c2.y, c2.z, c);
    va.AddVertex(c2.x, c2.y, c2.z, c);
    va.AddVertex(c3.x, c3.y, c3.z, c);
    va.AddVertex(c3.x, c3.y, c3.z, c);
    va.AddVertex(c4.x, c4.y, c4.z, c);
    va.AddVertex(c4.x, c4.y, c4.z, c);
    va.AddVertex(c1.x, c1.y, c1.z, c);

    va.AddVertex(c5.x, c5.y, c5.z, c);
    va.AddVertex(c6.x, c6.y, c6.z, c);
    va.AddVertex(c6.x, c6.y, c6.z, c);
    va.AddVertex(c7.x, c7.y, c7.z, c);
    va.AddVertex(c7.x, c7.y, c7.z, c);
    va.AddVertex(c8.x, c8.y, c8.z, c);
    va.AddVertex(c8.x, c8.y, c8.z, c);
    va.AddVertex(c5.x, c5.y, c5.z, c);

    va.AddVertex(c1.x, c1.y, c1.z, c);
    va.AddVertex(c5.x, c5.y, c5.z, c);
    va.AddVertex(c2.x, c2.y, c2.z, c);
    va.AddVertex(c6.x, c6.y, c6.z, c);
    va.AddVertex(c3.x, c3.y, c3.z, c);
    va.AddVertex(c7.x, c7.y, c7.z, c);
    va.AddVertex(c4.x, c4.y, c4.z, c);
    va.AddVertex(c8.x, c8.y, c8.z, c);
}

void ModelScreenLocation::DrawAxisTool(glm::vec3& pos, xlGraphicsProgram *program, float zoom, int scale) const {
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();

    int num_points = 18;
    float os = (float)GetRectHandleWidth(zoom, scale);

    if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
        xlColor ax1c = (highlighted_handle == HANDLE_AXIS) ? xlYELLOW : xlRED;
        xlColor ax2c = (highlighted_handle == HANDLE_AXIS + 1) ? xlYELLOW : xlGREEN;
        xlColor ax3c = (highlighted_handle == HANDLE_AXIS + 2) ? xlYELLOW : xlBLUE;
        float tip = pos.x + GetAxisArrowLength(zoom, scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(tip, pos.y, pos.z, ax1c);
            vac->AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u1), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u1), ax1c);
            vac->AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u2), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u2), ax1c);
        }
        tip = pos.y + GetAxisArrowLength(zoom, scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(pos.x, tip, pos.z, ax2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u1), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u1), ax2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u2), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u2), ax2c);
        }
        tip = pos.z + GetAxisArrowLength(zoom, scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(pos.x, pos.y, tip, ax3c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u1), pos.y + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u1), tip - GetAxisHeadLength(zoom, scale), ax3c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u2), pos.y + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u2), tip - GetAxisHeadLength(zoom, scale), ax3c);
        }
        int count = vac->getCount() - startVertex;
        program->addStep([program, vac, startVertex, count](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count);
        });
    } else if (axis_tool == MSLTOOL::TOOL_SCALE) {
        xlColor ax1c = (highlighted_handle == HANDLE_AXIS) ? xlYELLOW : xlRED;
        xlColor ax2c = (highlighted_handle == HANDLE_AXIS+1) ? xlYELLOW : xlGREEN;
        xlColor ax3c = (highlighted_handle == HANDLE_AXIS+2) ? xlYELLOW : xlBLUE;
        
        vac->AddCubeAsTriangles(pos.x + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.y, pos.z, GetAxisRadius(zoom, scale) * 2, ax1c);
        vac->AddCubeAsTriangles(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, GetAxisRadius(zoom, scale) * 2, ax2c);
        vac->AddCubeAsTriangles(pos.x, pos.y, pos.z + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), GetAxisRadius(zoom, scale) * 2, ax3c);
        int count = vac->getCount();
        program->addStep([program, vac, startVertex, count](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
    } else if (axis_tool == MSLTOOL::TOOL_ROTATE) {
        xlColor ax1c = (highlighted_handle == HANDLE_AXIS) ? xlYELLOW : xlGREEN;
        xlColor ax2c = (highlighted_handle == HANDLE_AXIS+1) ? xlYELLOW : xlBLUE;
        xlColor ax3c = (highlighted_handle == HANDLE_AXIS+2) ? xlYELLOW : xlRED;
        float radius = GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale);
        for (size_t i = 30; i <= 150; i += 5) {
            float u1 = (float)i;
            float u2 = (float)(i + 5);
            vac->AddVertex(pos.x, pos.y + radius * cos(glm::radians(u1)), pos.z + radius * sin(glm::radians(u1)), xlRED);
            vac->AddVertex(pos.x, pos.y + radius * cos(glm::radians(u2)), pos.z + radius * sin(glm::radians(u2)), xlRED);
        }
        for (size_t i = 0; i <= 120; i += 5) {
            float u1 = (float)i - 60.0f;;
            float u2 = (float)(i + 5) - 60.0f;
            vac->AddVertex(pos.x + radius * cos(glm::radians(u1)), pos.y, pos.z + radius * sin(glm::radians(u1)), xlGREEN);
            vac->AddVertex(pos.x + radius * cos(glm::radians(u2)), pos.y, pos.z + radius * sin(glm::radians(u2)), xlGREEN);
        }
        for (size_t i = 30; i <= 150; i += 5) {
            float u1 = (float)i;
            float u2 = (float)(i + 5);
            vac->AddVertex(pos.x + radius * cos(glm::radians(u1)), pos.y + radius * sin(glm::radians(u1)), pos.z, xlBLUE);
            vac->AddVertex(pos.x + radius * cos(glm::radians(u2)), pos.y + radius * sin(glm::radians(u2)), pos.z, xlBLUE);
        }
        int triangleStart = vac->getCount();
        
        vac->AddCubeAsTriangles(pos.x + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom,scale), pos.y, pos.z, GetAxisRadius(zoom, scale) * 2, ax1c);
        vac->AddCubeAsTriangles(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, GetAxisRadius(zoom, scale) * 2, ax2c);
        vac->AddCubeAsTriangles(pos.x, pos.y, pos.z + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), GetAxisRadius(zoom, scale) * 2, ax3c);
        
        int count = vac->getCount();
        program->addStep([program, vac, startVertex, triangleStart, count](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, triangleStart - startVertex);
            ctx->drawTriangles(vac, triangleStart, count - triangleStart);
        });
    } else if (axis_tool == MSLTOOL::TOOL_XY_TRANS) {
        xlColor a1c = (highlighted_handle == HANDLE_AXIS) ? xlYELLOW : xlRED;
        xlColor a2c = (highlighted_handle == HANDLE_AXIS) ? xlYELLOW : xlGREEN;
        float arrow_length = GetAxisArrowLength(zoom, scale) / 2.0f;
        vac->AddVertex(pos.x, pos.y, pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y + arrow_length, pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y, pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y - arrow_length, pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y, pos.z, xlRED);
        vac->AddVertex(pos.x + arrow_length, pos.y, pos.z, xlRED);
        vac->AddVertex(pos.x, pos.y, pos.z, xlRED);
        vac->AddVertex(pos.x - arrow_length, pos.y, pos.z, xlRED);
        
        int triangeVertex = vac->getCount();
        
        float tip = pos.x + arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(tip, pos.y, pos.z, a1c);
            vac->AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a1c);
            vac->AddVertex(tip - GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a1c);
        }
        tip = pos.x - arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(tip, pos.y, pos.z, a1c);
            vac->AddVertex(tip + GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a1c);
            vac->AddVertex(tip + GetAxisHeadLength(zoom, scale), pos.y + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a1c);
        }
        tip = pos.y + arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(pos.x, tip, pos.z, a2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a2c);
        }
        tip = pos.y - arrow_length;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(pos.x, tip, pos.z, a2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u1), tip + GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u1), a2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI*u2), tip + GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI*u2), a2c);
        }
        int count = vac->getCount();
        program->addStep([program, vac, startVertex, triangeVertex, count](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, triangeVertex - startVertex);
            ctx->drawTriangles(vac, triangeVertex, count - triangeVertex);
        });

    } else if (axis_tool == MSLTOOL::TOOL_ELEVATE) {
        xlColor ax2c = (highlighted_handle == HANDLE_AXIS) ? xlYELLOW : xlGREEN;
        float tip = pos.y + GetAxisArrowLength(zoom, scale);
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            vac->AddVertex(pos.x, tip, pos.z, ax2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u1), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u1), ax2c);
            vac->AddVertex(pos.x + GetAxisRadius(zoom, scale) * cos(2.0 * M_PI * u2), tip - GetAxisHeadLength(zoom, scale), pos.z + GetAxisRadius(zoom, scale) * sin(2.0 * M_PI * u2), ax2c);
        }
        vac->AddVertex(pos.x, pos.y + os, pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, xlGREEN);
        
        int count = vac->getCount();
        program->addStep([program, vac, startVertex, count](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex - 2);
            ctx->drawLines(vac, count - 2, 2);
        });

    }
    if (axis_tool == MSLTOOL::TOOL_TRANSLATE || axis_tool == MSLTOOL::TOOL_SCALE) {
        startVertex = vac->getCount();
        vac->AddVertex(pos.x + os, pos.y, pos.z, xlRED);
        vac->AddVertex(pos.x + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.y, pos.z, xlRED);
        vac->AddVertex(pos.x, pos.y + os, pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), pos.z, xlGREEN);
        vac->AddVertex(pos.x, pos.y, pos.z + os, xlBLUE);
        vac->AddVertex(pos.x, pos.y, pos.z + GetAxisArrowLength(zoom, scale) - GetAxisRadius(zoom, scale), xlBLUE);
        int count = vac->getCount();
        program->addStep([program, vac, startVertex, count](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, count - startVertex);
        });
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
        Rotate(MSLAXIS::Y_AXIS, offset);
        rotate_point(position.x, position.z, glm::radians(-offset), posx, posz);
        SetHcenterPos(posx);
        SetDcenterPos(posz);
    }
    else if( angle.x != 0.0f ) {
        float offset = angle.x;
        Rotate(MSLAXIS::X_AXIS, offset);
        rotate_point(position.y, position.z, glm::radians(offset), posy, posz);
        SetVcenterPos(posy);
        SetDcenterPos(posz);
    }
    else if( angle.z != 0.0f ) {
        float offset = angle.z;
        Rotate(MSLAXIS::Z_AXIS, offset);
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

    if (axis_tool == MSLTOOL::TOOL_ROTATE) {
        switch (active_axis) {
        case MSLAXIS::X_AXIS:
            normal = glm::vec3(saved_position.x + GetAxisArrowLength(zoom, scale), 0.0f, 0.0f);
            point = glm::vec3(saved_position.x, 0.0f, 0.0f);
            break;
        case MSLAXIS::Y_AXIS:
            normal = glm::vec3(0.0f, saved_position.y + GetAxisArrowLength(zoom, scale), 0.0f);
            point = glm::vec3(0.0f, saved_position.y, 0.0f);
            break;
        case MSLAXIS::Z_AXIS:
            normal = glm::vec3(0.0f, 0.0f, saved_position.z + GetAxisArrowLength(zoom, scale));
            point = glm::vec3(0.0f, 0.0f, saved_position.z);
            break;
        default:
            wxASSERT(false);
            break;
        }
    } else {
        switch (active_axis) {
        case MSLAXIS::Z_AXIS:
                if( active_plane == MSLPLANE::YZ_PLANE ) {
                    normal = glm::vec3(saved_position.x + GetAxisArrowLength(zoom, scale), 0.0f, 0.0f);
                    point = glm::vec3(saved_position.x, 0.0f, 0.0f);
                } else {
                    normal = glm::vec3(0.0f, saved_position.y + GetAxisArrowLength(zoom, scale), 0.0f);
                    point = glm::vec3(0.0f, saved_position.y, 0.0f);
                }
           break;
            case MSLAXIS::X_AXIS:
                    if( active_plane == MSLPLANE::XZ_PLANE ) {
                        normal = glm::vec3(0.0f, saved_position.y + GetAxisArrowLength(zoom, scale), 0.0f);
                        point = glm::vec3(0.0f, saved_position.y, 0.0f);
                    } else {
                        normal = glm::vec3(0.0f, 0.0f, saved_position.z + GetAxisArrowLength(zoom, scale));
                        point = glm::vec3(0.0f, 0.0f, saved_position.z);
                    }
                break;
            case MSLAXIS::Y_AXIS:
                    if( active_plane == MSLPLANE::YZ_PLANE ) {
                        normal = glm::vec3(saved_position.x + GetAxisArrowLength(zoom, scale), 0.0f, 0.0f);
                        point = glm::vec3(saved_position.x, 0.0f, 0.0f);
                    } else {
                        normal = glm::vec3(0.0f, 0.0f, saved_position.z + GetAxisArrowLength(zoom, scale));
                        point = glm::vec3(0.0f, 0.0f, saved_position.z);
                    }
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
        } else {
            drag_delta = intersect - saved_intersect;
        }
    } else {
        logger_base.warn("MoveHandle3D: Intersect not found!");
    }
    return found;
}

wxCursor ModelScreenLocation::CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int &handle, float zoom, int scale) const
{
    handle = NO_HANDLE;

    if (_locked) {
        return wxCURSOR_DEFAULT;
    }

    wxCursor return_value = CheckIfOverAxisHandles3D(ray_origin, ray_direction, handle, zoom, scale);

    if (handle == NO_HANDLE) {
        float distance = 1000000000.0f;

        // Test each each Oriented Bounding Box (OBB).
        int handles_found = 0;
        for (size_t i = 0; i < mSelectableHandles; i++) {
            float intersection_distance; // Output of TestRayOBBIntersection()

            if (VectorMath::TestRayOBBIntersection(
                ray_origin,
                ray_direction,
                handle_aabb_min[i],
                handle_aabb_max[i],
                ModelMatrix,
                intersection_distance)
                ) {
                // this is designed to go for the 3rd handle if several handles overlap to
                // prevent not being able to enlarge a model where someone just clicked
                // without a drag during creation and have a tiny model
                if (intersection_distance - distance < 0.001f) {
                    handles_found++;
                    if (intersection_distance - distance >= 0.0f) {
                        if (handles_found < 4) {
                            handle = i;
                        }
                    } else {
                        handle = i;
                    }
                    distance = intersection_distance;
                    return_value = wxCURSOR_HAND;
                }
            }
        }
    }
    return return_value;
}

wxCursor ModelScreenLocation::CheckIfOverAxisHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int& handle, float zoom, int scale) const
{
    wxCursor return_value = wxCURSOR_DEFAULT;

    float distance = 1000000000.0f;
    handle = NO_HANDLE;

    // test for a selected axis first
    int num_axis_handles = (axis_tool == MSLTOOL::TOOL_XY_TRANS || axis_tool == MSLTOOL::TOOL_ELEVATE) ? 1 : 3;
    glm::vec3 axisbb_min[3];
    glm::vec3 axisbb_max[3];
    if (IsXYTransHandle()) {
        float arrow_offset = GetAxisArrowLength(zoom, scale) / 2 - 3;
        axisbb_min[0].x = active_handle_pos.x - ModelMatrix[3][0] - arrow_offset;
        axisbb_min[0].y = active_handle_pos.y - ModelMatrix[3][1] - arrow_offset;
        axisbb_min[0].z = active_handle_pos.z - ModelMatrix[3][2] - arrow_offset;
        axisbb_max[0].x = active_handle_pos.x - ModelMatrix[3][0] + arrow_offset;
        axisbb_max[0].y = active_handle_pos.y - ModelMatrix[3][1] + arrow_offset;
        axisbb_max[0].z = active_handle_pos.z - ModelMatrix[3][2] + arrow_offset;
    } else if (IsElevationHandle()) {
        float axis_radius = GetAxisRadius(zoom, scale);
        float arrow_length = GetAxisArrowLength(zoom, scale);
        float head_length = GetAxisHeadLength(zoom, scale);
        axisbb_min[0].x = active_handle_pos.x - ModelMatrix[3][0] - axis_radius;
        axisbb_min[0].y = active_handle_pos.y - ModelMatrix[3][1] + arrow_length - head_length - 3;
        axisbb_min[0].z = active_handle_pos.z - ModelMatrix[3][2] - axis_radius;
        axisbb_max[0].x = active_handle_pos.x - ModelMatrix[3][0] + axis_radius;
        axisbb_max[0].y = active_handle_pos.y - ModelMatrix[3][1] + arrow_length + 3;
        axisbb_max[0].z = active_handle_pos.z - ModelMatrix[3][2] + axis_radius;
    } else {
        float axis_radius = GetAxisRadius(zoom, scale);
        float arrow_length = GetAxisArrowLength(zoom, scale);
        float head_length = GetAxisHeadLength(zoom, scale);
        axisbb_min[0].x = active_handle_pos.x - ModelMatrix[3][0] + arrow_length - head_length - 3;
        axisbb_min[0].y = active_handle_pos.y - ModelMatrix[3][1] - axis_radius;
        axisbb_min[0].z = active_handle_pos.z - ModelMatrix[3][2] - axis_radius;
        axisbb_min[1].x = active_handle_pos.x - ModelMatrix[3][0] - axis_radius;
        axisbb_min[1].y = active_handle_pos.y - ModelMatrix[3][1] + arrow_length - head_length - 3;
        axisbb_min[1].z = active_handle_pos.z - ModelMatrix[3][2] - axis_radius;
        axisbb_min[2].x = active_handle_pos.x - ModelMatrix[3][0] - axis_radius;
        axisbb_min[2].y = active_handle_pos.y - ModelMatrix[3][1] - axis_radius;
        axisbb_min[2].z = active_handle_pos.z - ModelMatrix[3][2] + arrow_length - head_length - 3;
        axisbb_max[0].x = active_handle_pos.x - ModelMatrix[3][0] + arrow_length + 3;
        axisbb_max[0].y = active_handle_pos.y - ModelMatrix[3][1] + axis_radius;
        axisbb_max[0].z = active_handle_pos.z - ModelMatrix[3][2] + axis_radius;
        axisbb_max[1].x = active_handle_pos.x - ModelMatrix[3][0] + axis_radius;
        axisbb_max[1].y = active_handle_pos.y - ModelMatrix[3][1] + arrow_length + 3;
        axisbb_max[1].z = active_handle_pos.z - ModelMatrix[3][2] + axis_radius;
        axisbb_max[2].x = active_handle_pos.x - ModelMatrix[3][0] + axis_radius;
        axisbb_max[2].y = active_handle_pos.y - ModelMatrix[3][1] + axis_radius;
        axisbb_max[2].z = active_handle_pos.z - ModelMatrix[3][2] + arrow_length + 3;
    }

    // see if an axis handle is selected
    for (size_t i = 0; i < num_axis_handles; i++) {
        float intersection_distance; // Output of TestRayOBBIntersection()

        if (VectorMath::TestRayOBBIntersection(
                ray_origin,
                ray_direction,
                axisbb_min[i],
                axisbb_max[i],
                TranslateMatrix, // axis is not rotated
                intersection_distance)) {
            if (intersection_distance < distance) {
                distance = intersection_distance;
                handle = i | HANDLE_AXIS;
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
            intersection_distance)) {
        return true;
    }
    return false;
}

void ModelScreenLocation::UpdateBoundingBox(float width, float height, float depth)
{
    // scale the bounding box for selection logic
    aabb_max.x = (width * scalex) / 2.0f;
    aabb_max.y = (height * scaley) / 2.0f;
    aabb_max.z = (depth * scalez) / 2.0f;
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
