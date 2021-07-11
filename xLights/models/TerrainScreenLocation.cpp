/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "TerrianScreenLocation.h"
#include "../DrawGLUtils.h"
#include <wx/xml/xml.h>
#include "../support/VectorMath.h"

#include <log4cpp/Category.hh>

#define BOUNDING_RECT_OFFSET        8
#define NUM_TERRAIN_HANDLES   861   // default number of points for a 40x20 grid

static glm::mat4 Identity(glm::mat4(1.0f));

TerrianScreenLocation::TerrianScreenLocation()
    : BoxedScreenLocation(NUM_TERRAIN_HANDLES+ 9), num_points_wide(41), num_points_deep(21),
    edit_active(false)
{
    mSelectableHandles = NUM_TERRAIN_HANDLES;
    handle_aabb_max.resize(NUM_TERRAIN_HANDLES);
    handle_aabb_min.resize(NUM_TERRAIN_HANDLES);
}

void TerrianScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator& va, float zoom, int scale, bool drawBounding) const {
    va.PreAlloc((mSelectableHandles + 5) * 5);

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (_locked) {
        handleColor = xlREDTRANSLUCENT;
    }

    float x_offset = (num_points_wide - 1) * spacing / 2;
    float z_offset = (num_points_deep - 1) * spacing / 2;
    float y_min = -10.0f;
    float y_max = 10.0f;
    float handle_width = GetRectHandleWidth(zoom, scale);
    float sx, sy, sz;
    if (!edit_active) {
        // Center Handle
        handle_aabb_min[CENTER_HANDLE].x = -handle_width;
        handle_aabb_min[CENTER_HANDLE].y = -handle_width;
        handle_aabb_min[CENTER_HANDLE].z = -handle_width;
        handle_aabb_max[CENTER_HANDLE].x = handle_width;
        handle_aabb_max[CENTER_HANDLE].y = handle_width;
        handle_aabb_max[CENTER_HANDLE].z = handle_width;
        mHandlePosition[CENTER_HANDLE].x = worldPos_x;
        mHandlePosition[CENTER_HANDLE].y = worldPos_y;
        mHandlePosition[CENTER_HANDLE].z = worldPos_z;
        if (active_handle == CENTER_HANDLE) {
            DrawGLUtils::DrawSphere(mHandlePosition[CENTER_HANDLE].x, mHandlePosition[CENTER_HANDLE].y, mHandlePosition[CENTER_HANDLE].z, (double)(handle_width), xlORANGETRANSLUCENT, va);
        }
        // calculate elevation boundary
        for (int j = 0; j < num_points_deep; ++j) {
            for (int i = 0; i < num_points_wide; ++i) {
                int abs_point = j * num_points_wide + i;
                sy = mPos[abs_point];
                if (sy > y_min)
                {
                    y_min = sy;
                }
                if (sy < y_max)
                {
                    y_max = sy;
                }
            }
        }
    } else {
        for (int j = 0; j < num_points_deep; ++j) {
            for (int i = 0; i < num_points_wide; ++i) {
                int abs_point = j * num_points_wide + i;
                sx = i * spacing - x_offset;
                sz = j * spacing - z_offset;
                sy = mPos[abs_point];
                if (sy > y_min)
                {
                    y_min = sy;
                }
                if (sy < y_max)
                {
                    y_max = sy;
                }
                handle_aabb_min[abs_point + 1].x = sx * scalex - handle_width;
                handle_aabb_min[abs_point + 1].y = sy * scaley - handle_width;
                handle_aabb_min[abs_point + 1].z = sz * scalez - handle_width;
                handle_aabb_max[abs_point + 1].x = sx * scalex + handle_width;
                handle_aabb_max[abs_point + 1].y = sy * scaley + handle_width;
                handle_aabb_max[abs_point + 1].z = sz * scalez + handle_width;
                TranslatePoint(sx, sy, sz);
                mHandlePosition[abs_point + 1].x = sx;
                mHandlePosition[abs_point + 1].y = sy;
                mHandlePosition[abs_point + 1].z = sz;
                if (highlighted_handle == abs_point + 1 || active_handle == abs_point + 1) {
                    if (!_locked) {
                        if (active_handle == abs_point + 1)
                        {
                            handleColor = xlYELLOWTRANSLUCENT;
                        }
                        else {
                            handleColor = xlBLUETRANSLUCENT;
                        }
                    }
                    DrawGLUtils::DrawSphere(mHandlePosition[abs_point + 1].x, mHandlePosition[abs_point + 1].y, mHandlePosition[abs_point + 1].z, (double)(handle_width), handleColor, va);
                }
            }
        }
    }

    LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));

    xlColor Box3dColor = xlWHITE;
    if (_locked) Box3dColor = xlREDTRANSLUCENT;

    minX = -x_offset * scalex - BOUNDING_RECT_OFFSET;
    maxX = x_offset * scalex + BOUNDING_RECT_OFFSET;
    minY = y_min * scaley - BOUNDING_RECT_OFFSET;
    maxY = y_max * scaley + BOUNDING_RECT_OFFSET;
    minZ = -z_offset * scalez - BOUNDING_RECT_OFFSET;
    maxZ = z_offset * scalez + BOUNDING_RECT_OFFSET;
    glm::vec3 bound_min(minX, minY, minZ);
    glm::vec3 bound_max(maxX, maxY, maxZ);
    DrawGLUtils::DrawBoundingBox(Box3dColor, bound_min, bound_max, ModelMatrix, va);

    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);

    if (active_handle != -1) {
        active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
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

wxCursor TerrianScreenLocation::CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int& handle, float zoom, int scale) const
{
    wxCursor return_value = wxCURSOR_DEFAULT;
    handle = NO_HANDLE;

    if (_locked) {
        return wxCURSOR_DEFAULT;
    }

    return_value = CheckIfOverAxisHandles3D(ray_origin, ray_direction, handle, zoom, scale);

    if (handle == NO_HANDLE) {
        float distance = 1000000000.0f;
        handle = -1;

        // Test each each Oriented Bounding Box (OBB).
        for (size_t i = edit_active ? 1 : 0; edit_active ? i < mSelectableHandles : i < 1; i++)
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

int TerrianScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (_locked) return 0;

    if (handle != CENTER_HANDLE) {
        if (axis_tool == TOOL_ELEVATE) {
            if (latch) {
                saved_position.y = active_handle_pos.y;
            }

            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

            if (scaley == 0) scaley = 0.001f;
            if (isnan(scaley)) scaley = 1.0f;

            float newy = (saved_position.y + drag_delta.y - worldPos_y) / scaley;

            int point = handle - 1;
            if (point < mPos.size()) {
                switch (active_axis) {
                case Y_AXIS:
                    mPos[point] = newy;
                    if (tool_size > 1) {
                        int row = point / num_points_wide;
                        int col = point % num_points_wide;
                        int start_row = row - tool_size + 1;
                        int end_row = row + tool_size - 1;
                        int start_col = col - tool_size + 1;
                        int end_col = col + tool_size - 1;
                        if (start_row < 0) start_row = 0;
                        if (end_row > num_points_deep - 1) end_row = num_points_deep - 1;
                        if (start_col < 0) start_col = 0;
                        if (end_col > num_points_wide - 1) end_col = num_points_wide - 1;
                        for (int j = start_row; j <= end_row; ++j) {
                            for (int i = start_col; i <= end_col; ++i) {
                                int abs_point = j * num_points_wide + i;
                                mPos[abs_point] = newy;
                            }
                        }
                    }
                    break;
                }
            }
        }
    }
    else {
        BoxedScreenLocation::MoveHandle3D(preview, handle, ShiftKeyPressed, CtrlKeyPressed, mouseX, mouseY, latch, scale_z);
    }
    return 1;
}

void TerrianScreenLocation::SetActiveHandle(int handle)
{
    active_handle = handle;
    highlighted_handle = -1;
    SetAxisTool(axis_tool);  // run logic to disallow certain tools
}

void TerrianScreenLocation::SetAxisTool(int mode)
{
    if (active_handle > 0) {
        axis_tool = TOOL_ELEVATE;
    }
    else {
        if (axis_tool == TOOL_ELEVATE)
        {
            axis_tool = TOOL_TRANSLATE;
        }
        ModelScreenLocation::SetAxisTool(mode);
    }
}

void TerrianScreenLocation::AdvanceAxisTool()
{
    if (active_handle > 0) {
        axis_tool = TOOL_ELEVATE;
    }
    else {
        ModelScreenLocation::AdvanceAxisTool();
    }
}

void TerrianScreenLocation::SetActiveAxis(int axis)
{
    if (active_handle > 0) {
        if (axis != -1) {
            active_axis = Y_AXIS;
        }
        else {
            active_axis = -1;
        }
    }
    else {
        ModelScreenLocation::SetActiveAxis(axis);
    }
}

void TerrianScreenLocation::Read(wxXmlNode* node) {
    BoxedScreenLocation::Read(node);
    spacing = wxAtoi(node->GetAttribute("TerrianLineSpacing", "50"));
    num_points_wide = wxAtoi(node->GetAttribute("TerrianWidth", "1000")) / spacing + 1;
    num_points_deep = wxAtoi(node->GetAttribute("TerrianDepth", "1000")) / spacing + 1;
    num_points = num_points_wide * num_points_deep;
    mPos.resize(num_points);
    wxString point_data = node->GetAttribute("PointData");
    wxArrayString point_array = wxSplit(point_data, ',');
    int i = 0;
    for (int p = 2; p < point_array.size() && i < num_points; ) {
        float val = wxAtof(point_array[p]);
        if (val == 0.0f) {
            p++;
            float num_zeroes = wxAtof(point_array[p]);
            for (int j = 0; j < num_zeroes; ++j) {
                if (i == num_points) {
                    break;
                }
                mPos[i++] = 0.0f;
            }
        } else {
            mPos[i++] = val;
        }
        p++;
    }
    while (i < num_points) mPos[i++] = 0.0f;
    mHandlePosition.resize(num_points + 1);
    mSelectableHandles = num_points + 1;
    handle_aabb_min.resize(num_points + 1);
    handle_aabb_max.resize(num_points + 1);
}

void TerrianScreenLocation::Write(wxXmlNode* node) {
    BoxedScreenLocation::Write(node);

    node->DeleteAttribute("PointData");
    wxString point_data = "";
    // store the number of points in each axis to allow for smart resizing  
    // when grid is altered after terrain points have already been established
    point_data += wxString::Format("%f,%f,", (float)num_points_wide, (float)num_points_deep);
    int num_zeroes = 0;
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i] != 0) {
            if (num_zeroes > 0) {
                point_data += wxString::Format("%f,%f,", 0.0f, (float)num_zeroes);
                num_zeroes = 0;
            }
            point_data += wxString::Format("%f", mPos[i]);
            if (i != num_points - 1) {
                point_data += ",";
            }
        }
        else {
            num_zeroes++;
            if (i == num_points - 1) {
                point_data += wxString::Format("%f,%f", 0.0f, (float)num_zeroes);
            }
        }
    }
    node->AddAttribute("PointData", point_data);
}

