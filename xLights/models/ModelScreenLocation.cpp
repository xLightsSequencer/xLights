#include "ModelScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "../ModelPreview.h"
#include "../DrawGLUtils.h"
#include "../support/VectorMath.h"
#include <log4cpp/Category.hh>
#include <glm/glm.hpp>

#define SNAP_RANGE                  5
#define RECT_HANDLE_WIDTH           6
#define BOUNDING_RECT_OFFSET        8

static float AXIS_RADIUS = 4.0f;
static float AXIS_ARROW_LENGTH = 60.0f;

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

// used to print matrix when debugging
static void PrintMatrix(std::string name, glm::mat4& matrix)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
    logger_base.debug("Matrix Info: %s", name);
    logger_base.debug("Row 0: %6.2f  %6.2f  %6.2f  %6.2f", matrix[0][0], matrix[0][1], matrix[0][2], matrix[0][3]);
    logger_base.debug("Row 1: %6.2f  %6.2f  %6.2f  %6.2f", matrix[1][0], matrix[1][1], matrix[1][2], matrix[1][3]);
    logger_base.debug("Row 2: %6.2f  %6.2f  %6.2f  %6.2f", matrix[2][0], matrix[2][1], matrix[2][2], matrix[2][3]);
    logger_base.debug("Row 3: %6.2f  %6.2f  %6.2f  %6.2f", matrix[3][0], matrix[3][1], matrix[3][2], matrix[3][3]);
}

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

ModelScreenLocation::ModelScreenLocation(int sz)
: RenderWi(0), RenderHt(0), RenderDp(0), previewW(800), previewH(600),
  worldPos_x(0.0f), worldPos_y(0.0f), worldPos_z(0.0f),
  scalex(1.0f), scaley(1.0f), scalez(1.0f), mHandlePosition(sz),
  rotatex(0), rotatey(0), rotatez(0),
  ModelMatrix(glm::mat4(1.0f)), aabb_min(0.0f), aabb_max(0.0f), saved_intersect(0.0f),
  saved_position(0.0f), saved_size(0.0f), saved_scale(1.0f), saved_rotate(0.0f),
  active_handle(-1), active_axis(-1), axis_tool(TOOL_TRANSLATE)
{
    draw_3d = false;
    _locked = false;
}

void ModelScreenLocation::SetRenderSize(float NewWi, float NewHt, float NewDp) {
    RenderHt = NewHt;
    RenderWi = NewWi;
    RenderDp = NewDp;
}

void ModelScreenLocation::SetActiveAxis(int axis)
{
    if (axis_tool == TOOL_ROTATE && axis != -1) {
        active_axis = (axis + 1) % NUM_TOOLS;
    }
    else {
        active_axis = axis;
    }
}


void ModelScreenLocation::DrawAxisTool(float x, float y, float z, DrawGLUtils::xl3Accumulator &va) const
{
    int num_points = 18;
    float head_length = 12.0f;
    float os = (float)RECT_HANDLE_WIDTH;

    if (axis_tool == TOOL_TRANSLATE) {
        float tip = x + AXIS_ARROW_LENGTH;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(tip, y, z, xlRED);
            va.AddVertex(tip - head_length, y + AXIS_RADIUS * cos(2.0 * M_PI*u1), z + AXIS_RADIUS * sin(2.0 * M_PI*u1), xlRED);
            va.AddVertex(tip - head_length, y + AXIS_RADIUS * cos(2.0 * M_PI*u2), z + AXIS_RADIUS * sin(2.0 * M_PI*u2), xlRED);
        }
        tip = y + AXIS_ARROW_LENGTH;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(x, tip, z, xlGREEN);
            va.AddVertex(x + AXIS_RADIUS * cos(2.0 * M_PI*u1), tip - head_length, z + AXIS_RADIUS * sin(2.0 * M_PI*u1), xlGREEN);
            va.AddVertex(x + AXIS_RADIUS * cos(2.0 * M_PI*u2), tip - head_length, z + AXIS_RADIUS * sin(2.0 * M_PI*u2), xlGREEN);
        }
        tip = z + AXIS_ARROW_LENGTH;
        for (size_t i = 0; i < num_points; i++) {
            float u1 = i / (float)num_points;
            float u2 = (i + 1) / (float)num_points;
            va.AddVertex(x, y, tip, xlBLUE);
            va.AddVertex(x + AXIS_RADIUS * cos(2.0 * M_PI*u1), y + AXIS_RADIUS * sin(2.0 * M_PI*u1), tip - head_length, xlBLUE);
            va.AddVertex(x + AXIS_RADIUS * cos(2.0 * M_PI*u2), y + AXIS_RADIUS * sin(2.0 * M_PI*u2), tip - head_length, xlBLUE);
        }
        va.Finish(GL_TRIANGLES);
    }
    else if (axis_tool == TOOL_SCALE) {
        DrawGLUtils::DrawCube(x + AXIS_ARROW_LENGTH - AXIS_RADIUS, y, z, AXIS_RADIUS * 2, xlRED, va);
        DrawGLUtils::DrawCube(x, y + AXIS_ARROW_LENGTH - AXIS_RADIUS, z, AXIS_RADIUS * 2, xlGREEN, va);
        DrawGLUtils::DrawCube(x, y, z + AXIS_ARROW_LENGTH - AXIS_RADIUS, AXIS_RADIUS * 2, xlBLUE, va);
        va.Finish(GL_TRIANGLES);
    }
    else if (axis_tool == TOOL_ROTATE) {
        float radius = AXIS_ARROW_LENGTH - AXIS_RADIUS;
        for (size_t i = 30; i <= 150; i += 5) {
            float u1 = (float)i;
            float u2 = (float)(i + 5);
            va.AddVertex(x, y + radius * cos(glm::radians(u1)), z + radius * sin(glm::radians(u1)), xlRED);
            va.AddVertex(x, y + radius * cos(glm::radians(u2)), z + radius * sin(glm::radians(u2)), xlRED);
        }
        for (size_t i = 0; i <= 120; i += 5) {
            float u1 = (float)i - 60.0f;;
            float u2 = (float)(i + 5) - 60.0f;
            va.AddVertex(x + radius * cos(glm::radians(u1)), y, z + radius * sin(glm::radians(u1)), xlGREEN);
            va.AddVertex(x + radius * cos(glm::radians(u2)), y, z + radius * sin(glm::radians(u2)), xlGREEN);
        }
        for (size_t i = 30; i <= 150; i += 5) {
            float u1 = (float)i;
            float u2 = (float)(i + 5);
            va.AddVertex(x + radius * cos(glm::radians(u1)), y + radius * sin(glm::radians(u1)), z, xlBLUE);
            va.AddVertex(x + radius * cos(glm::radians(u2)), y + radius * sin(glm::radians(u2)), z, xlBLUE);
        }
        va.Finish(GL_LINES);

        DrawGLUtils::DrawCube(x + AXIS_ARROW_LENGTH - AXIS_RADIUS, y, z, AXIS_RADIUS * 2, xlGREEN, va);
        DrawGLUtils::DrawCube(x, y + AXIS_ARROW_LENGTH - AXIS_RADIUS, z, AXIS_RADIUS * 2, xlBLUE, va);
        DrawGLUtils::DrawCube(x, y, z + AXIS_ARROW_LENGTH - AXIS_RADIUS, AXIS_RADIUS * 2, xlRED, va);
        va.Finish(GL_TRIANGLES);
    }

    if (axis_tool == TOOL_TRANSLATE || axis_tool == TOOL_SCALE) {
        va.AddVertex(x + os, y, z, xlRED);
        va.AddVertex(x + AXIS_ARROW_LENGTH, y, z, xlRED);
        va.AddVertex(x, y + os, z, xlGREEN);
        va.AddVertex(x, y + AXIS_ARROW_LENGTH, z, xlGREEN);
        va.AddVertex(x, y, z + os, xlBLUE);
        va.AddVertex(x, y, z + AXIS_ARROW_LENGTH, xlBLUE);
        va.Finish(GL_LINES);
    }
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

bool ModelScreenLocation::DragHandle(ModelPreview* preview, int mouseX, int mouseY, bool latch) {

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
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
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
            normal = glm::vec3(saved_position.x + AXIS_ARROW_LENGTH, 0.0f, 0.0f);
            point = glm::vec3(saved_position.x, 0.0f, 0.0f);
            break;
        case Y_AXIS:
            normal = glm::vec3(0.0f, saved_position.y + AXIS_ARROW_LENGTH, 0.0f);
            point = glm::vec3(0.0f, saved_position.y, 0.0f);
            break;
        case Z_AXIS:
            normal = glm::vec3(0.0f, 0.0f, saved_position.z + AXIS_ARROW_LENGTH);
            point = glm::vec3(0.0f, 0.0f, saved_position.z);
            break;
        }
    }
    else {
        switch (active_axis)
        {
        case X_AXIS:
        case Z_AXIS:
            normal = glm::vec3(0.0f, saved_position.y + AXIS_ARROW_LENGTH, 0.0f);
            point = glm::vec3(0.0f, saved_position.y, 0.0f);
            break;
        case Y_AXIS:
            normal = glm::vec3(0.0f, 0.0f, saved_position.z + AXIS_ARROW_LENGTH);
            point = glm::vec3(0.0f, 0.0f, saved_position.z);
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
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        logger_base.warn("MoveHandle3D: Intersect not found!");
    }
    return found;
}

BoxedScreenLocation::BoxedScreenLocation()
: ModelScreenLocation(10), perspective(0.0f)
{
    mSelectableHandles.push_back(CENTER_HANDLE);
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
}

void BoxedScreenLocation::Read(wxXmlNode *ModelNode) {
    worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "200.0"));
    worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
    worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));

    scalex = wxAtof(ModelNode->GetAttribute("ScaleX", "1.0"));
	scaley = wxAtof(ModelNode->GetAttribute("ScaleY", "1.0"));
	scalez = wxAtof(ModelNode->GetAttribute("ScaleZ", "1.0"));

	if (scalex<0) {
		scalex = 1.0f;
	}
	if (scaley<0) {
		scaley = 1.0f;
	}
	if (scalez<0) {
		scalez = 1.0f;
	}

    rotatex = wxAtoi(ModelNode->GetAttribute("RotateX", "0"));
    rotatey = wxAtoi(ModelNode->GetAttribute("RotateY", "0"));
    rotatez = wxAtoi(ModelNode->GetAttribute("RotateZ", "0"));

    if (rotatex < -180 || rotatex > 180) {
        rotatex = 0;
    }
    if (rotatey < -180 || rotatey > 180) {
        rotatey = 0;
    }
    if (rotatez < -180 || rotatez > 180) {
        rotatez = 0;
    }

    _locked = (wxAtoi(ModelNode->GetAttribute("Locked", "0")) == 1);
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
    ModelXml->AddAttribute("RotateX", wxString::Format("%d", rotatex));
    ModelXml->AddAttribute("RotateY", wxString::Format("%d", rotatey));
    ModelXml->AddAttribute("RotateZ", wxString::Format("%d", rotatez));
    if (_locked)
    {
        ModelXml->AddAttribute("Locked", "1");
    }
}

void BoxedScreenLocation::TranslatePoint(float &sx, float &sy, float &sz) const {
    sx = (sx*scalex);
    sy = (sy*scaley);
	sz = (sz*scalez);
    TranslatePointDoubles(glm::radians((float)rotatez), sx, sy, sx, sy);
    TranslatePointDoubles(glm::radians((float)rotatey), sx, sz, sx, sz);
    TranslatePointDoubles(glm::radians((float)rotatex), sz, sy, sz, sy);

    // Give 2D tree model its perspective
    if (!draw_3d) {
        glm::vec4 position = glm::vec4(glm::vec3(sx, sy, sz), 1.0);
        glm::mat4 rm = glm::rotate(glm::mat4(1.0f), perspective, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec4 model_position = rm * position;
        sx = model_position.x;
        sy = model_position.y;
        sz = model_position.z;
    }

    sx += worldPos_x;
    sy += worldPos_y;
	sz += worldPos_z;
}

bool BoxedScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
    int xs = x1<x2?x1:x2;
    int xf = x1>x2?x1:x2;
    int ys = y1<y2?y1:y2;
    int yf = y1>y2?y1:y2;

    if (aabb_min.x >= xs && aabb_max.x <= xf && aabb_min.y >= ys && aabb_max.y <= yf) {
        return true;
    } else {
        return false;
    }
}

bool BoxedScreenLocation::HitTest(ModelPreview* preview, int x, int y) const {
    // NOTE:  This routine is designed for the 2D layout model selection only

    bool return_value = false;
    glm::vec3 origin;
    glm::vec3 direction;

    VectorMath::ScreenPosToWorldRay(
        x, preview->getHeight() - y,
        preview->getWidth(), preview->getHeight(),
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
        origin,
        direction
    );

    glm::mat4 flipy = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 flipx = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 matrix = ModelMatrix2D * flipy * flipx;

    float intersection_distance; // Output of TestRayOBBIntersection()

    if (VectorMath::TestRayOBBIntersection(
        origin,
        direction,
        aabb_min,
        aabb_max,
        matrix,
        intersection_distance)
        ) {
        return_value = true;
    }

    return return_value;
}

wxCursor BoxedScreenLocation::CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const
{
    // NOTE:  This routine is designed for the 2D layout handle selection only

    if (_locked)
    {
        handle = NO_HANDLE;
        return wxCURSOR_DEFAULT;
    }

    glm::vec3 origin;
    glm::vec3 direction;

    VectorMath::ScreenPosToWorldRay(
        x, preview->getHeight() - y,
        preview->getWidth(), preview->getHeight(),
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
        origin,
        direction
    );

    float distance = 1000000000.0f;
    int which_handle = NO_HANDLE;
    int hw = RECT_HANDLE_WIDTH;

    int num_handles = 5;
    glm::vec3 aabb_min[5];
    glm::vec3 aabb_max[5];

    glm::mat4 flipy = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 flipx = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 matrix = flipy * flipx;

    for (size_t h = 0; h < num_handles; h++) {
        aabb_min[h].x = mHandlePosition[h+1].x - hw;
        aabb_min[h].y = mHandlePosition[h + 1].y - hw;
        aabb_min[h].z = 0.0f;
        aabb_max[h].x = mHandlePosition[h + 1].x + hw;
        aabb_max[h].y = mHandlePosition[h + 1].y + hw;
        aabb_max[h].z = 0.0f;
    }

    // Test each each Oriented Bounding Box (OBB).
    for (size_t i = 0; i < num_handles; i++)
    {
        float intersection_distance; // Output of TestRayOBBIntersection()

        if (VectorMath::TestRayOBBIntersection(
            origin,
            direction,
            aabb_min[i],
            aabb_max[i],
            matrix,
            intersection_distance)
            ) {
            if (intersection_distance < distance) {
                distance = intersection_distance;
                which_handle = i+1;
            }
        }
    }
    handle = which_handle;

    if (handle == NO_HANDLE) {
        return wxCURSOR_DEFAULT;
    }
    else if (handle == ROTATE_HANDLE) {
        return wxCURSOR_HAND;
    }
    else {
        return GetResizeCursor(which_handle, rotatez);
    }
}

wxCursor BoxedScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    if (preview != nullptr) {
        if (preview->Is3D()) {
            // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
            // of the mouse click with the ground plane
            active_axis = X_AXIS;
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = RenderHt / 2.0f;
            worldPos_z = saved_intersect.z;
            handle = CENTER_HANDLE;
            active_axis = Y_AXIS;
        }
        else {
            //worldPos_x = (float)x / (float)previewW;
            //worldPos_y = (float)y / (float)previewH;
            handle = R_BOT_HANDLE;
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            active_axis = Y_AXIS;
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = (float)previewH - saved_intersect.y;
            worldPos_z = 0.0f;
        }
    }
    else {
        wxMessageBox("InitializeLocation: called with no preview....investigate!", "Error", wxICON_ERROR | wxOK);
    }
    SetPreviewSize(previewW, previewH, Nodes);
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

                //aabb vectors need to be the untranslated / unscaled limits
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
            aabb_max.y += 2;
            aabb_min.y -= 2;
        }
        if (aabb_max.x - aabb_min.x < 4) {
            aabb_max.x += 2;
            aabb_min.x -= 2;
        }
        if (aabb_max.z - aabb_min.z < 4) {
            aabb_max.z += 4;
            aabb_min.z -= 4;
        }
    }

}

void BoxedScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
    centerx = worldPos_x;
    centery = worldPos_y;
    draw_3d = is_3d;
    if (allow_selected) {
        glm::mat4 RotateX = glm::rotate(glm::mat4(1.0f), glm::radians((float)rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 RotateY = glm::rotate(glm::mat4(1.0f), glm::radians((float)rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 RotateZ = glm::rotate(glm::mat4(1.0f), glm::radians((float)rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 Translate = translate(glm::mat4(1.0f), glm::vec3(worldPos_x, worldPos_y, worldPos_z));
        ModelMatrix = Translate * RotateZ * RotateY * RotateX;
        glm::mat4 Translate2D = translate(glm::mat4(1.0f), glm::vec3(worldPos_x, worldPos_y, 0.0f));
        ModelMatrix2D = Translate2D * RotateZ * RotateY * RotateX;
        TranslateMatrix = Translate;
    }
}

void BoxedScreenLocation::SetPreviewSize(int w, int h, const std::vector<NodeBaseClassPtr> &Nodes) {
    previewW = w;
    previewH = h;

    PrepareToDraw(draw_3d, false);

    for (auto it = Nodes.begin(); it != Nodes.end(); ++it) {
        for (auto coord = it->get()->Coords.begin(); coord != it->get()->Coords.end(); ++coord) {
            // draw node on screen
            float sx = coord->screenX;
            float sy = coord->screenY;
            float sz = coord->screenZ;

            TranslatePoint(sx, sy, sz);
        }
    }
}

void BoxedScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va) const {
    va.PreAlloc(32 * 5);

    float color1[4] = { 1, 0, 0, 1 };

    float sz1 = RenderDp / 2;
    float sz2 =  -RenderDp / 2;

    xlColor handleColor = xlBLUE;
    if (_locked)
    {
        handleColor = xlRED;
    }

    xlColor color = handleColor;

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
    handle_aabb_min[CENTER_HANDLE].x = -RECT_HANDLE_WIDTH;
    handle_aabb_min[CENTER_HANDLE].y = -RECT_HANDLE_WIDTH;
    handle_aabb_min[CENTER_HANDLE].z = -RECT_HANDLE_WIDTH;
    handle_aabb_max[CENTER_HANDLE].x = RECT_HANDLE_WIDTH;
    handle_aabb_max[CENTER_HANDLE].y = RECT_HANDLE_WIDTH;
    handle_aabb_max[CENTER_HANDLE].z = RECT_HANDLE_WIDTH;
    mHandlePosition[CENTER_HANDLE].x = worldPos_x;
    mHandlePosition[CENTER_HANDLE].y = worldPos_y;
    mHandlePosition[CENTER_HANDLE].z = worldPos_z;

    LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
    va.AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, xlWHITE);

    va.AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, xlWHITE);

    va.AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, xlWHITE);
    va.AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);

    if (active_handle != -1) {
        DrawGLUtils::DrawSphere(mHandlePosition[CENTER_HANDLE].x, mHandlePosition[CENTER_HANDLE].y, mHandlePosition[CENTER_HANDLE].z, (double)(RECT_HANDLE_WIDTH), xlORANGE, va);
        DrawAxisTool(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z, va);
        if (active_axis != -1) {
            LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
            switch (active_axis)
            {
            case X_AXIS:
                va.AddVertex(-1000000.0f, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z, xlRED);
                va.AddVertex(+1000000.0f, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z, xlRED);
                break;
            case Y_AXIS:
                va.AddVertex(mHandlePosition[active_handle].x, -1000000.0f, mHandlePosition[active_handle].z, xlGREEN);
                va.AddVertex(mHandlePosition[active_handle].x, +1000000.0f, mHandlePosition[active_handle].z, xlGREEN);
                break;
            case Z_AXIS:
                va.AddVertex(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, -1000000.0f, xlBLUE);
                va.AddVertex(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, +1000000.0f, xlBLUE);
                break;
            }
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
        }
    }
}

void BoxedScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {
    va.PreAlloc(6 * 5);

    float w1 = worldPos_x;
    float h1 = worldPos_y;

    xlColor handleColor = xlBLUE;
    if (_locked)
    {
        handleColor = xlRED;
    }

    // Upper Left Handle
    float sx = -RenderWi / 2;
    float sy = RenderHt / 2;
    float sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    sx += -BOUNDING_RECT_OFFSET;
    sy += BOUNDING_RECT_OFFSET;
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH/2), sy + (RECT_HANDLE_WIDTH / 2), handleColor);
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
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH / 2), sy + (RECT_HANDLE_WIDTH / 2), handleColor);
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
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH / 2), sy + (RECT_HANDLE_WIDTH / 2), handleColor);
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
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH / 2), sy + (RECT_HANDLE_WIDTH / 2), handleColor);
    mHandlePosition[L_BOT_HANDLE].x = sx;
    mHandlePosition[L_BOT_HANDLE].y = sy;
    mHandlePosition[L_BOT_HANDLE].z = sz;

    // Draw rotation handle square
    sx = 0.0f;
    sy = RenderHt / 2 + (50/scaley);
    sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH / 2), sy + (RECT_HANDLE_WIDTH / 2), handleColor);
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
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("ScaleY", "ScaleY", scaley));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxFloatProperty("ScaleZ", "ScaleZ", scalez));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxIntProperty("RotateX", "RotateX", rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxIntProperty("RotateY", "RotateY", rotatey));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetEditor("SpinCtrl");
    prop = propertyEditor->Append(new wxIntProperty("RotateZ", "RotateZ", rotatez));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetEditor("SpinCtrl");
}

int BoxedScreenLocation::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!_locked && "ScaleX" == name) {
        scalex = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ScaleY" == name) {
        scaley = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ScaleZ" == name) {
        scalez = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelX" == name) {
        worldPos_x = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelY" == name) {
        worldPos_y = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelZ" == name) {
        worldPos_z = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateX" == name) {
        rotatex = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateY" == name) {
        rotatey = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "RotateZ" == name) {
        rotatez = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "RotateZ" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        return 3;
    }

    return 0;
}

void BoxedScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z) {
    
    if (latch) {
        saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
    }

    if (!DragHandle(preview, mouseX, mouseY, latch)) return;

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
            double angle = 0.0f;
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                rotatex = saved_rotate.x + angle;
            }
                break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                rotatey = saved_rotate.y - angle;
            }
                break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                rotatez = saved_rotate.z + angle;
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
                    if (ShiftKeyPressed && scale_z) {
                        scalez = scalex;
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
}

int BoxedScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

    glm::vec3 origin;
    glm::vec3 direction;

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
        origin,
        direction
    );

    int posx = origin.x;
    int posy = origin.y;

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
    }
    return 0;
}

void BoxedScreenLocation::AddOffset(float xPct, float yPct, float zPct) {

    if (_locked) return;

    worldPos_x += xPct * previewW;
    worldPos_y += yPct * previewH;
    worldPos_z += zPct * previewH;
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
    return RenderWi*scalex;
}

int BoxedScreenLocation::GetMHeight() const {
    return RenderHt*scaley;
}

void BoxedScreenLocation::SetMWidth(int w)
{
    scalex = (float)w / (float)previewW;
}

void BoxedScreenLocation::SetMHeight(int h)
{
    scaley = (float)h / (float)previewH;
}

void BoxedScreenLocation::SetLeft(int x) {
    worldPos_x = x + RenderWi / 2;
}
void BoxedScreenLocation::SetRight(int i) {
    worldPos_x = i - RenderWi / 2;
}
void BoxedScreenLocation::SetTop(int y) {
    worldPos_y = y - RenderHt / 2;
}
void BoxedScreenLocation::SetBottom(int y) {
    worldPos_y = y + RenderHt / 2;
}

TwoPointScreenLocation::TwoPointScreenLocation() : ModelScreenLocation(3),
    old(nullptr), minMaxSet(false), point2(glm::vec3(0.0f)), center(glm::vec3(0.0f))
{
    mSelectableHandles.push_back(CENTER_HANDLE);
    mSelectableHandles.push_back(START_HANDLE);
    mSelectableHandles.push_back(END_HANDLE);
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
}
TwoPointScreenLocation::~TwoPointScreenLocation() {
}

void TwoPointScreenLocation::Read(wxXmlNode *ModelNode) {
    if (!ModelNode->HasAttribute("X1") && ModelNode->HasAttribute("worldPos_x")) {
        old = ModelNode;
    } else {
        worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "0.0"));
        worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
        worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));
        x2 = wxAtof(ModelNode->GetAttribute("X2", "0.0"));
        y2 = wxAtof(ModelNode->GetAttribute("Y2", "0.0"));
        z2 = wxAtof(ModelNode->GetAttribute("Z2", "0.0"));
    }
    _locked = (wxAtoi(ModelNode->GetAttribute("Locked", "0")) == 1);
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
    point2 = glm::vec3(x2 + worldPos_x, y2 + worldPos_y, z2 + worldPos_z);
    if (!is_3d) {
        // allows 2D selection to work
        origin.z = 0.0f;
        point2.z = 0.0f;
    }
    center = glm::vec3(RenderWi / 2.0f, 0.0f, 0.0f);

    glm::vec3 a = point2 - origin;
    glm::mat4 rotationMatrix = glm::mat4(1.0f);
    if (origin.y != point2.y || origin.z != point2.z) {
        glm::vec3 v = glm::vec3(0, -a.z, a.y);
        float angle = acos(a.x / glm::length(a));
        rotationMatrix = glm::rotate(angle, v);
    }
    scalex = scaley = scalez = glm::length(a) / RenderWi;

    glm::mat4 scalingMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(scalex, scaley, scalez));
    glm::mat3 shear = glm::shearY(glm::mat3(1.0f), GetYShear());
    glm::mat4 shearMatrix = glm::mat4(shear);
    TranslateMatrix = translate(glm::mat4(1.0f), glm::vec3(worldPos_x, worldPos_y, worldPos_z));
    matrix = TranslateMatrix * rotationMatrix * shearMatrix * scalingMatrix;

    if (allow_selected) {
        // save processing time by skipping items not needed for view only preview
        ModelMatrix = TranslateMatrix * rotationMatrix * shearMatrix;
        glm::mat4 Translate2D = translate(glm::mat4(1.0f), glm::vec3(worldPos_x, worldPos_y, 0.0f));
        ModelMatrix2D = Translate2D * rotationMatrix * shearMatrix;
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

bool TwoPointScreenLocation::IsContained(int x1, int y1, int x2, int y2) const {
 /*   float min = ymin;
    float max = ymax;
    if (!minMaxSet) {
        min = 0;
        max = RenderHt;
    }
    //invert the matrix, get into render space
    glm::vec4 v1 = *matrix * glm::vec4(glm::vec3(0, min, ), 1.0f);
    glm::vec4 v2 = *matrix * glm::vec4(glm::vec3(0, max, 1), 1.0f);
    glm::vec4 v3 = *matrix * glm::vec4(glm::vec3(RenderWi, min, 1), 1.0f);
    glm::vec4 v4 = *matrix * glm::vec4(glm::vec3(RenderWi, max, 1), 1.0f);

    int xsi = x1<x2?x1:x2;
    int xfi = x1>x2?x1:x2;
    int ysi = y1<y2?y1:y2;
    int yfi = y1>y2?y1:y2;

    float xs = std::min(std::min(v1.x, v2.x), std::min(v3.x, v4.x));
    float xf = std::max(std::max(v1.x, v2.x), std::max(v3.x, v4.x));
    float ys = std::min(std::min(v1.y, v2.y), std::min(v3.y, v4.y));
    float yf = std::max(std::max(v1.y, v2.y), std::max(v3.y, v4.y));

    return xsi < xs && xfi > xf && ysi < ys && yfi > yf;*/
    return false;
}

bool TwoPointScreenLocation::HitTest(ModelPreview* preview, int sx,int sy) const {
    // NOTE:  This routine is designed for the 2D layout model selection only

    bool return_value = false;
    glm::vec3 origin;
    glm::vec3 direction;

    VectorMath::ScreenPosToWorldRay(
        sx, preview->getHeight() - sy,
        preview->getWidth(), preview->getHeight(),
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
        origin,
        direction
    );

    glm::mat4 flipy = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 flipx = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 matrix2d = ModelMatrix2D * flipy * flipx;

    PrintMatrix("ModelMatrix2D", ModelMatrix2D);

    float intersection_distance; // Output of TestRayOBBIntersection()

    if (VectorMath::TestRayOBBIntersection(
        origin,
        direction,
        aabb_min,
        aabb_max,
        matrix2d,
        intersection_distance)
        ) {
        return_value = true;
    }

    return return_value;
}

wxCursor TwoPointScreenLocation::CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const
{
    // NOTE:  This routine is designed for the 2D layout handle selection only

    if (_locked)
    {
        handle = NO_HANDLE;
        return wxCURSOR_DEFAULT;
    }

    glm::vec3 origin;
    glm::vec3 direction;

    VectorMath::ScreenPosToWorldRay(
        x, preview->getHeight() - y,
        preview->getWidth(), preview->getHeight(),
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
        origin,
        direction
    );

    float distance = 1000000000.0f;
    int which_handle = NO_HANDLE;
    int hw = RECT_HANDLE_WIDTH;

    int num_handles = mHandlePosition.size()-1; // 2D doesn't use center handle
    glm::vec3 aabb_min[3];
    glm::vec3 aabb_max[3];

    glm::mat4 flipy = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 flipx = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 matrix2D = flipy * flipx;

    for (size_t h = 0; h < num_handles; h++) {
        aabb_min[h].x = mHandlePosition[h + 1].x - hw;
        aabb_min[h].y = mHandlePosition[h + 1].y - hw;
        aabb_min[h].z = 0.0f;
        aabb_max[h].x = mHandlePosition[h + 1].x + hw;
        aabb_max[h].y = mHandlePosition[h + 1].y + hw;
        aabb_max[h].z = 0.0f;
    }

    // Test each each Oriented Bounding Box (OBB).
    for (size_t i = 0; i < num_handles; i++)
    {
        float intersection_distance; // Output of TestRayOBBIntersection()

        if (VectorMath::TestRayOBBIntersection(
            origin,
            direction,
            aabb_min[i],
            aabb_max[i],
            matrix2D,
            intersection_distance)
            ) {
            if (intersection_distance < distance) {
                distance = intersection_distance;
                which_handle = i + 1;
            }
        }
    }
    handle = which_handle;

    if (handle == NO_HANDLE) {
        return wxCURSOR_DEFAULT;
    }
    else if (handle == ROTATE_HANDLE) {
        return wxCURSOR_HAND;
    }
    else {
        return GetResizeCursor(which_handle, rotatez);
    }
}

void TwoPointScreenLocation::SetActiveHandle(int handle)
{
    active_handle = handle;
    SetAxisTool(axis_tool);  // run logic to disallow certain tools
}

void TwoPointScreenLocation::SetAxisTool(int mode)
{
    if (mode != TOOL_SCALE) {
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
    axis_tool %= NUM_TOOLS;
}

void TwoPointScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va) const {
    va.PreAlloc(10);

    xlColor h1c = xlBLUE;
    xlColor h2c = xlBLUE;
    xlColor h3c = xlORANGE;
    if (_locked)
    {
        h1c = xlRED;
        h2c = xlRED;
        h3c = xlRED;
    }
    else {
        h1c = (active_handle == START_HANDLE) ? xlCYAN : xlGREEN;
        h2c = (active_handle == END_HANDLE) ? xlCYAN : xlBLUE;
    }

    DrawGLUtils::DrawSphere(worldPos_x, worldPos_y, worldPos_z, RECT_HANDLE_WIDTH, h1c, va);
    mHandlePosition[START_HANDLE].x = worldPos_x;
    mHandlePosition[START_HANDLE].y = worldPos_y;
    mHandlePosition[START_HANDLE].z = worldPos_z;

    DrawGLUtils::DrawSphere(point2.x, point2.y, point2.z, RECT_HANDLE_WIDTH, h2c, va);
    mHandlePosition[END_HANDLE].x = point2.x;
    mHandlePosition[END_HANDLE].y = point2.y;
    mHandlePosition[END_HANDLE].z = point2.z;

    DrawGLUtils::DrawSphere(center.x, center.y, center.z, RECT_HANDLE_WIDTH, h3c, va);
    mHandlePosition[CENTER_HANDLE].x = center.x;
    mHandlePosition[CENTER_HANDLE].y = center.y;
    mHandlePosition[CENTER_HANDLE].z = center.z;

    handle_aabb_min[START_HANDLE].x = -RECT_HANDLE_WIDTH;
    handle_aabb_min[START_HANDLE].y = -RECT_HANDLE_WIDTH;
    handle_aabb_min[START_HANDLE].z = -RECT_HANDLE_WIDTH;
    handle_aabb_max[START_HANDLE].x = RECT_HANDLE_WIDTH;
    handle_aabb_max[START_HANDLE].y = RECT_HANDLE_WIDTH;
    handle_aabb_max[START_HANDLE].z = RECT_HANDLE_WIDTH;

    handle_aabb_min[END_HANDLE].x = RenderWi * scalex - RECT_HANDLE_WIDTH;
    handle_aabb_min[END_HANDLE].y = - RECT_HANDLE_WIDTH;
    handle_aabb_min[END_HANDLE].z = - RECT_HANDLE_WIDTH;
    handle_aabb_max[END_HANDLE].x = RenderWi * scalex + RECT_HANDLE_WIDTH;
    handle_aabb_max[END_HANDLE].y = RECT_HANDLE_WIDTH;
    handle_aabb_max[END_HANDLE].z = RECT_HANDLE_WIDTH;

    handle_aabb_min[CENTER_HANDLE].x = (RenderWi / 2.0f) * scalex - RECT_HANDLE_WIDTH;
    handle_aabb_min[CENTER_HANDLE].y = - RECT_HANDLE_WIDTH;
    handle_aabb_min[CENTER_HANDLE].z = - RECT_HANDLE_WIDTH;
    handle_aabb_max[CENTER_HANDLE].x = (RenderWi / 2.0f) * scalex + RECT_HANDLE_WIDTH;
    handle_aabb_max[CENTER_HANDLE].y = RECT_HANDLE_WIDTH;
    handle_aabb_max[CENTER_HANDLE].z = RECT_HANDLE_WIDTH;

    // the bounding box is so close to a single line don't draw it once it's selected
    if (active_handle == -1) {
        glm::vec4 c1(aabb_min.x, aabb_max.y, aabb_min.z, 1.0f);
        glm::vec4 c2(aabb_max.x, aabb_max.y, aabb_min.z, 1.0f);
        glm::vec4 c3(aabb_max.x, aabb_min.y, aabb_min.z, 1.0f);
        glm::vec4 c4(aabb_min.x, aabb_min.y, aabb_min.z, 1.0f);
        glm::vec4 c5(aabb_min.x, aabb_max.y, aabb_max.z, 1.0f);
        glm::vec4 c6(aabb_max.x, aabb_max.y, aabb_max.z, 1.0f);
        glm::vec4 c7(aabb_max.x, aabb_min.y, aabb_max.z, 1.0f);
        glm::vec4 c8(aabb_min.x, aabb_min.y, aabb_max.z, 1.0f);

        c1 = ModelMatrix * c1;
        c2 = ModelMatrix * c2;
        c3 = ModelMatrix * c3;
        c4 = ModelMatrix * c4;
        c5 = ModelMatrix * c5;
        c6 = ModelMatrix * c6;
        c7 = ModelMatrix * c7;
        c8 = ModelMatrix * c8;

        LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
        va.AddVertex(c1.x, c1.y, c1.z, xlWHITE);
        va.AddVertex(c2.x, c2.y, c2.z, xlWHITE);
        va.AddVertex(c2.x, c2.y, c2.z, xlWHITE);
        va.AddVertex(c3.x, c3.y, c3.z, xlWHITE);
        va.AddVertex(c3.x, c3.y, c3.z, xlWHITE);
        va.AddVertex(c4.x, c4.y, c4.z, xlWHITE);
        va.AddVertex(c4.x, c4.y, c4.z, xlWHITE);
        va.AddVertex(c1.x, c1.y, c1.z, xlWHITE);

        va.AddVertex(c5.x, c5.y, c5.z, xlWHITE);
        va.AddVertex(c6.x, c6.y, c6.z, xlWHITE);
        va.AddVertex(c6.x, c6.y, c6.z, xlWHITE);
        va.AddVertex(c7.x, c7.y, c7.z, xlWHITE);
        va.AddVertex(c7.x, c7.y, c7.z, xlWHITE);
        va.AddVertex(c8.x, c8.y, c8.z, xlWHITE);
        va.AddVertex(c8.x, c8.y, c8.z, xlWHITE);
        va.AddVertex(c5.x, c5.y, c5.z, xlWHITE);

        va.AddVertex(c1.x, c1.y, c1.z, xlWHITE);
        va.AddVertex(c5.x, c5.y, c5.z, xlWHITE);
        va.AddVertex(c2.x, c2.y, c2.z, xlWHITE);
        va.AddVertex(c6.x, c6.y, c6.z, xlWHITE);
        va.AddVertex(c3.x, c3.y, c3.z, xlWHITE);
        va.AddVertex(c7.x, c7.y, c7.z, xlWHITE);
        va.AddVertex(c4.x, c4.y, c4.z, xlWHITE);
        va.AddVertex(c8.x, c8.y, c8.z, xlWHITE);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
    }

    if (active_handle != -1 && !_locked) {
        DrawAxisTool(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z, va);
        if (active_axis != -1) {
            LOG_GL_ERRORV(glHint(GL_LINE_SMOOTH_HINT, GL_NICEST));
            switch (active_axis)
            {
            case X_AXIS:
                va.AddVertex(-1000000.0f, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z, xlRED);
                va.AddVertex(+1000000.0f, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z, xlRED);
                break;
            case Y_AXIS:
                va.AddVertex(mHandlePosition[active_handle].x, -1000000.0f, mHandlePosition[active_handle].z, xlGREEN);
                va.AddVertex(mHandlePosition[active_handle].x, +1000000.0f, mHandlePosition[active_handle].z, xlGREEN);
                break;
            case Z_AXIS:
                va.AddVertex(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, -1000000.0f, xlBLUE);
                va.AddVertex(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, +1000000.0f, xlBLUE);
                break;
            }
            va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);
        }
    }
}

void TwoPointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {

    xlColor handleColor = xlBLUE;
    if (_locked)
    {
        handleColor = xlRED;
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
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH / 2), sy + (RECT_HANDLE_WIDTH / 2), xlGREEN);
    mHandlePosition[START_HANDLE].x = sx;
    mHandlePosition[START_HANDLE].y = sy;
    mHandlePosition[START_HANDLE].z = sz;

    sx = point2.x;
    sy = point2.y;
    sz = point2.z;
    //TranslatePoint(sx, sy, sz);
    va.AddRect(sx - (RECT_HANDLE_WIDTH / 2), sy - (RECT_HANDLE_WIDTH / 2), sx + (RECT_HANDLE_WIDTH / 2), sy + (RECT_HANDLE_WIDTH / 2), handleColor);
    mHandlePosition[END_HANDLE].x = sx;
    mHandlePosition[END_HANDLE].y = sy;
    mHandlePosition[END_HANDLE].z = sz;

    va.Finish(GL_TRIANGLES);
}

void TwoPointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (latch) {
        saved_angle = 0.0f;
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

    if (!DragHandle(preview, mouseX, mouseY, latch)) return;

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
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            glm::vec3 start_pt = origin;
            glm::vec3 end_pt = point2;
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -center);
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), center);
            glm::mat4 Rotate = glm::mat4(1.0f);

            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
            }
            break;
            }
            start_pt = glm::vec3(translateBack * Rotate * translateToOrigin* glm::vec4(start_pt, 1.0f));
            end_pt = glm::vec3(translateBack * Rotate * translateToOrigin* glm::vec4(end_pt, 1.0f));
            worldPos_x = start_pt.x;
            worldPos_y = start_pt.y;
            worldPos_z = start_pt.z;
            x2 = end_pt.x - worldPos_x;
            y2 = end_pt.y - worldPos_y;
            z2 = end_pt.z - worldPos_z;
            saved_angle = angle;
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
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -origin);
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), origin);
            glm::mat4 Rotate = glm::mat4(1.0f);

            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
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
            glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), -point2);
            glm::mat4 translateBack = glm::translate(glm::mat4(1.0f), point2);
            glm::mat4 Rotate = glm::mat4(1.0f);

            switch (active_axis)
            {
            case X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = saved_angle - angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(1.0f, 0.0f, 0.0f));
            }
            break;
            case Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            break;
            case Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                angle = end_angle - start_angle;
                float new_angle = angle - saved_angle;
                Rotate = glm::rotate(glm::mat4(1.0f), glm::radians((float)new_angle), glm::vec3(0.0f, 0.0f, 1.0f));
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
}

int TwoPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

    glm::vec3 origin;
    glm::vec3 direction;

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetViewMatrix(),
        preview->GetProjMatrix(),
        origin,
        direction
    );

    float newx = origin.x;
    float newy = origin.y;

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

    glm::vec3 ray_origin;
    glm::vec3 ray_direction;
    if (preview != nullptr) {
        if (preview->Is3D()) {
            // what we do here is define a position at origin so that the DragHandle function will calculate the intersection
            // of the mouse click with the ground plane
            active_axis = X_AXIS;
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = 0.0f;
            worldPos_z = saved_intersect.z;
        }
        else {
            saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            active_axis = Y_AXIS;
            DragHandle(preview, x, y, true);
            worldPos_x = saved_intersect.x;
            worldPos_y = (float)previewH - saved_intersect.y;
            worldPos_z = 0.0f;
        }
    }
    else {
        wxMessageBox("InitializeLocation: called with no preview....investigate!", "Error", wxICON_ERROR | wxOK);
    }
    x2 = y2 = z2 = 0.0f;
    handle = END_HANDLE;
    SetPreviewSize(previewW, previewH, Nodes);
    return wxCURSOR_SIZING;
}


void TwoPointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxBoolProperty("Locked", "Locked", _locked));
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
    if (!_locked && "ModelX1" == name) {
        worldPos_x = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelX1" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelY1" == name) {
        worldPos_y = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelY1" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelZ1" == name) {
        worldPos_z = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelZ1" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelX2" == name) {
        x2 = event.GetValue().GetDouble() - worldPos_x;
        return 3;
    }
    else if (_locked && "ModelX2" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelY2" == name) {
        y2 = event.GetValue().GetDouble() - worldPos_y;
        return 3;
    }
    else if (_locked && "ModelY2" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelZ2" == name) {
        z2 = event.GetValue().GetDouble() - worldPos_z;
        return 3;
    }
    else if (_locked && "ModelZ2" == name) {
        event.Veto();
        return 0;
    }
    else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        return 3;
    }

    return 0;
}

void TwoPointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    glm::vec3 a = point2 - origin;
    aabb_min = glm::vec3(0.0f);
    aabb_max = glm::vec3(RenderWi, 0.0f, 0.0f);

    // scale the bounding box for selection logic
    aabb_min.x = aabb_min.x * scalex;
    aabb_min.y = aabb_min.y * scaley;
    aabb_min.z = aabb_min.z * scalez;
    aabb_max.x = aabb_max.x * scalex;
    aabb_max.y = aabb_max.y * scaley;
    aabb_max.z = aabb_max.z * scalez;

    // Set minimum bounding rectangle
    if (aabb_max.y - aabb_min.y < 8) {
        aabb_max.y += 10;
        aabb_min.y -= 10;
    }
    if (aabb_max.x - aabb_min.x < 8) {
        aabb_max.x += 10;
        aabb_min.x -= 10;
    }
    if (aabb_max.z - aabb_min.z < 8) {
        aabb_max.z += 10;
        aabb_min.z -= 10;
    }
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
    box.PrepareToDraw(false, false);

    float sx = - float(RenderWi) / 2.0;
    float sy = 0;
    float sz = 0;
    box.TranslatePoint(sx, sy, sz);
    worldPos_x = sx / (float)previewW;
    worldPos_y = sy / (float)previewH;

    sx = float(RenderWi) / 2.0;
    sy = 0;
    box.TranslatePoint(sx, sy, sz);

    x2 = sx / (float)previewW;
    y2 = sy / (float)previewH;

    old->DeleteAttribute("worldPos_x");
    old->DeleteAttribute("worldPos_y");
    old->DeleteAttribute("scalex");
    old->DeleteAttribute("scaley");
    old->DeleteAttribute("PreviewRotation");
}

float TwoPointScreenLocation::GetHcenterOffset() const {
    return x2 / 2.0;
}
float TwoPointScreenLocation::GetVcenterOffset() const {
    return y2 / 2.0;
}

void TwoPointScreenLocation::SetHcenterOffset(float f) {
    float diffx = x2 / 2.0 - f;
    worldPos_x -= diffx;
}
void TwoPointScreenLocation::SetVcenterOffset(float f) {
    float diffy = y2 / 2.0 - f;
    worldPos_y -= diffy;
}

void TwoPointScreenLocation::SetOffset(float xPct, float yPct) {

    if (_locked) return;

    float diffx = x2 / 2.0 - xPct;
    float diffy = y2 / 2.0 - yPct;

    worldPos_y -= diffy;
    worldPos_x -= diffx;
}

void TwoPointScreenLocation::AddOffset(float xPct, float yPct, float zPct) {

    if (_locked) return;

    worldPos_x += xPct * previewW;
    worldPos_y += yPct * previewH;
    worldPos_z += zPct * previewH;
}

int TwoPointScreenLocation::GetTop() const {
    return std::max(std::round(worldPos_y), std::round(y2 + worldPos_y));
}

int TwoPointScreenLocation::GetLeft() const {
    return std::min(std::round(worldPos_x), std::round(x2 + worldPos_x));
}

int TwoPointScreenLocation::GetMWidth() const
{
    return std::abs(x2 / 2);
}

int TwoPointScreenLocation::GetMHeight() const
{
    return std::abs(y2 / 2);
}

int TwoPointScreenLocation::GetRight() const {
    return std::max(std::round(worldPos_x), std::round(x2 + worldPos_x));
}
int TwoPointScreenLocation::GetBottom() const {
    return std::min(std::round(worldPos_y), std::round(y2 + worldPos_y));
}
void TwoPointScreenLocation::SetTop(int i) {
    float newtop = (float)i;
    if (worldPos_y > y2 + worldPos_y) {
        float diff = worldPos_y - newtop;
        worldPos_y = newtop;
        y2 -= diff;
    } else {
        float diff = y2 - newtop;
        y2 = newtop;
        worldPos_y -= diff;
    }
}
void TwoPointScreenLocation::SetLeft(int i) {
    float newx = (float)i / (float)previewW;
    if (worldPos_x < x2) {
        float diff = worldPos_x - newx;
        worldPos_x = newx;
        x2 -= diff;
    } else {
        float diff = x2 - newx;
        x2 = newx;
        worldPos_x -= diff;
    }
}
void TwoPointScreenLocation::SetRight(int i) {
    float newx = (float)i / (float)previewW;
    if (worldPos_x > x2) {
        float diff = worldPos_x - newx;
        worldPos_x = newx;
        x2 -= diff;
    } else {
        float diff = x2 - newx;
        x2 = newx;
        worldPos_x -= diff;
    }
}

void TwoPointScreenLocation::SetMWidth(int w)
{
    if (worldPos_x > x2)
    {
        worldPos_x = x2 + (float)w / previewW;
    }
    else
    {
        x2 = worldPos_x + (float)w / previewW;
    }
}

void TwoPointScreenLocation::SetMHeight(int h)
{
    if (worldPos_y > y2)
    {
        worldPos_y = y2 + (float)h / previewH;
    }
    else
    {
        y2 = worldPos_y + (float)h / previewH;
    }
}

void TwoPointScreenLocation::SetBottom(int i) {
    float newbot = (float)i / (float)previewH;
    if (worldPos_y < y2) {
        float diff = worldPos_y - newbot;
        worldPos_y = newbot;
        y2 -= diff;
    } else {
        float diff = y2 - newbot;
        y2 = newbot;
        worldPos_y -= diff;
    }
}

void TwoPointScreenLocation::FlipCoords() {
    std::swap(worldPos_x, x2);
    std::swap(worldPos_y, y2);
}


ThreePointScreenLocation::ThreePointScreenLocation(): height(1.0), modelHandlesHeight(false), supportsShear(false), supportsAngle(false), angle(0), shear(0.0) {
    mHandlePosition.resize(4);
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
    node->DeleteAttribute("Locked");
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
        return 3;
    }
    else if (_locked && "ModelHeight" == name) {
        event.Veto();
        return 0;
    }
    else if (!_locked && "ModelShear" == name) {
        shear = event.GetValue().GetDouble();
        return 3;
    }
    else if (_locked && "ModelShear" == name) {
        event.Veto();
        return 0;
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
  /*  float min = ymin;
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

    return xsi < xs && xfi > xf && ysi < ys && yfi > yf;*/
    return false;
}

bool ThreePointScreenLocation::HitTest(ModelPreview* preview, int sx,int sy) const {
    return TwoPointScreenLocation::HitTest(preview, sx, sy);
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
}

void ThreePointScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va) const {
    TwoPointScreenLocation::DrawHandles(va);
}

void ThreePointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {

    float sx1 = center.x;
    float sy1 = center.y;

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

    glm::vec3 v1 = glm::vec3(matrix * glm::vec4(glm::vec3(x, max, 1), 1.0f));
    float sx = v1.x;
    float sy = v1.y;
    LOG_GL_ERRORV(glHint( GL_LINE_SMOOTH_HINT, GL_NICEST ));
    va.AddVertex(sx1, sy1, xlWHITE);
    va.AddVertex(sx, sy, xlWHITE);
    va.Finish(GL_LINES, GL_LINE_SMOOTH, 1.7f);

    TwoPointScreenLocation::DrawHandles(va);

    xlColor handleColor = xlBLUE;
    if (_locked)
    {
        handleColor = xlRED;
    }

    va.AddRect(sx - RECT_HANDLE_WIDTH/2.0, sy - RECT_HANDLE_WIDTH/2.0, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, handleColor);
    va.Finish(GL_TRIANGLES);
    mHandlePosition[SHEAR_HANDLE].x = sx;
    mHandlePosition[SHEAR_HANDLE].y = sy;
}

int ThreePointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

    if (handle == SHEAR_HANDLE) {
        glm::mat4 m = glm::inverse(matrix);
        glm::vec3 v = glm::vec3(m * glm::vec4(mouseX, mouseY, 1, 1));
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

void ThreePointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    glm::vec3 a = point2 - origin;
    aabb_min = glm::vec3(0.0f);
    aabb_max = glm::vec3(RenderWi, RenderHt, 0.0f);

    // scale the bounding box for selection logic
    aabb_min.x = aabb_min.x * scalex;
    aabb_min.y = aabb_min.y * scaley;
    aabb_min.z = aabb_min.z * scalez;
    aabb_max.x = aabb_max.x * scalex;
    aabb_max.y = aabb_max.y * scaley;
    aabb_max.z = aabb_max.z * scalez;

    // Set minimum bounding rectangle
    if (aabb_max.y - aabb_min.y < 8) {
        aabb_max.y += 10;
        aabb_min.y -= 10;
    }
    if (aabb_max.x - aabb_min.x < 8) {
        aabb_max.x += 10;
        aabb_min.x -= 10;
    }
    if (aabb_max.z - aabb_min.z < 8) {
        aabb_max.z += 10;
        aabb_min.z -= 10;
    }
}

void ThreePointScreenLocation::ProcessOldNode(wxXmlNode *old) {
    BoxedScreenLocation box;
    box.Read(old);
    std::vector<NodeBaseClassPtr> Nodes;
    box.SetPreviewSize(previewW, previewH, Nodes);
    box.SetRenderSize(RenderWi, RenderHt);
    box.PrepareToDraw(false, false);

    float x1 = RenderWi / 2.0;
    float y1 = RenderHt;
    float z1 = 0;
    box.TranslatePoint(x1, y1, z1);

    TwoPointScreenLocation::ProcessOldNode(old);

    height = 1.0;
    PrepareToDraw(false, false);
    glm::mat4 m = glm::inverse(matrix);
    glm::vec4 v = m * glm::vec4(glm::vec3(x1, y1, 1), 1.0f);
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

    if (_locked) return;

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
    _locked = (wxAtoi(ModelNode->GetAttribute("Locked", "0")) == 1);
}

void PolyPointScreenLocation::Write(wxXmlNode *node) {
    node->DeleteAttribute("NumPoints");
    node->DeleteAttribute("PointData");
    node->DeleteAttribute("cPointData");
    node->DeleteAttribute("Locked");
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
    if (_locked)
    {
        node->AddAttribute("Locked", "1");
    }
}

void PolyPointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const {
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
    draw_3d = is_3d;
}

void PolyPointScreenLocation::TranslatePoint(float &x, float &y, float &z) const {
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

bool PolyPointScreenLocation::HitTest(ModelPreview* preview, int sx,int sy) const {
    for( int i = 0; i < num_points-1; ++i ) {
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
            float max_y = std::max(std::max(v2.y, v3.y), std::max(v4.y, v5.y));
            float min_y = std::min(std::min(v2.y, v3.y), std::min(v4.y, v5.y));

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

wxCursor PolyPointScreenLocation::CheckIfOverHandles(ModelPreview* preview, int &handle, int x, int y) const
{
    if (_locked)
    {
        handle = NO_HANDLE;
        return wxCURSOR_DEFAULT;
    }
    /*
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
    }*/
    handle = -1;
    return wxCURSOR_DEFAULT;
}

void PolyPointScreenLocation::DrawHandles(DrawGLUtils::xl3Accumulator &va) const {
}

void PolyPointScreenLocation::DrawHandles(DrawGLUtils::xlAccumulator &va) const {
    va.PreAlloc(10*num_points+12);

    // add boundary handles
    float x1 = minX * previewW - RECT_HANDLE_WIDTH / 2;
    float y1 = minY * previewH - RECT_HANDLE_WIDTH / 2;
    float x2 = maxX * previewW - RECT_HANDLE_WIDTH / 2;
    float y2 = maxY * previewH - RECT_HANDLE_WIDTH / 2;
    xlColor handleColor = xlBLUE;
    if (_locked)
    {
        handleColor = xlRED;
    }
    va.AddRect(x1, y1, x1 + RECT_HANDLE_WIDTH, y1 + RECT_HANDLE_WIDTH, handleColor);
    va.AddRect(x1, y2, x1 + RECT_HANDLE_WIDTH, y2 + RECT_HANDLE_WIDTH, handleColor);
    va.AddRect(x2, y1, x2 + RECT_HANDLE_WIDTH, y1 + RECT_HANDLE_WIDTH, handleColor);
    va.AddRect(x2, y2, x2 + RECT_HANDLE_WIDTH, y2 + RECT_HANDLE_WIDTH, handleColor);
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
        va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, i == selected_handle ? xlMAGENTA : (i == 0 ? xlGREEN : handleColor));
        mHandlePosition[i].x = sx;
        mHandlePosition[i].y = sy;

        // add final handle
        if( i == num_points-2 ) {
            sx = mPos[i+1].x * previewW - RECT_HANDLE_WIDTH / 2;
            sy = mPos[i+1].y * previewH - RECT_HANDLE_WIDTH / 2;
            va.AddRect(sx, sy, sx + RECT_HANDLE_WIDTH, sy + RECT_HANDLE_WIDTH, i+1 == selected_handle ? xlMAGENTA : handleColor);
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

void PolyPointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{

}

int PolyPointScreenLocation::MoveHandle(ModelPreview* preview, int handle, bool ShiftKeyPressed, int mouseX, int mouseY) {

    if (_locked) return 0;

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
        float scalex = 1.0f;
        float scaley = 1.0f;
        if( handle == num_points ) {  // bottom-left corner
            if( newx >= maxX-0.01f || newy >= maxY-0.01f ) return 0;
            trans_x = newx - minX;
            trans_y = newy - minY;
            scalex -= trans_x / (maxX - minX);
            scaley -= trans_y / (maxY - minY);
        } else if( handle == num_points+1 ) {  // top left corner
            if( newx >= maxX-0.01f || newy <= minY+0.01f ) return 0;
            trans_x = newx - minX;
            scalex -= trans_x / (maxX - minX);
            scaley = (newy - minY) / (maxY - minY);
        } else if( handle == num_points+2 ) {  // bottom right corner
            if( newx <= minX+0.01f|| newy >= maxY-0.01f ) return 0;
            trans_y = newy - minY;
            scalex = (newx - minX) / (maxX - minX);
            scaley -= trans_y / (maxY - minY);
        } else if( handle == num_points+3 ) {  // bottom right corner
            if( newx <= minX+0.01f || newy <= minY+0.01f ) return 0;
            scalex = (newx - minX) / (maxX - minX);
            scaley = (newy - minY) / (maxY - minY);
        } else {
            return 0;
        }

        glm::mat3 scalingMatrix = glm::scale(glm::mat3(1.0f), glm::vec2( scalex, scaley));
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

wxCursor PolyPointScreenLocation::InitializeLocation(int &handle, int x, int y, const std::vector<NodeBaseClassPtr> &Nodes, ModelPreview* preview) {
    mPos[0].x = (float)x/(float)previewW;
    mPos[0].y = (float)y/(float)previewH;
    mPos[1].x = (float)x/(float)previewW;
    mPos[1].y = (float)y/(float)previewH;
    handle = 1;
    return wxCURSOR_SIZING;
}

void PolyPointScreenLocation::AddSizeLocationProperties(wxPropertyGridInterface *propertyEditor) const {
    wxPGProperty *prop = propertyEditor->Append(new wxBoolProperty("Locked", "Locked", _locked));
    prop->SetAttribute("UseCheckbox", 1);
    prop = propertyEditor->Append(new wxFloatProperty("X1 (%)", "ModelX1", mPos[0].x * 100.0));
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
        if (!_locked && name.find("ModelX") != std::string::npos) {
            mPos[selected_handle].x = event.GetValue().GetDouble() / 100.0;
            return 3;
        }
        else if (_locked && name.find("ModelX") != std::string::npos) {
            event.Veto();
            return 0;
        }
        else if (!_locked && name.find("ModelY") != std::string::npos) {
            mPos[selected_handle].y = event.GetValue().GetDouble() / 100.0;
            return 3;
        }
        else if (_locked && name.find("ModelY") != std::string::npos) {
            event.Veto();
            return 0;
        }
    }
    else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        return 3;
    }

    return 0;
}

void PolyPointScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
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

    if (_locked) return;

    SetHcenterOffset(xPct);
    SetVcenterOffset(yPct);
}

void PolyPointScreenLocation::AddOffset(float xPct, float yPct, float zPct) {
// FIXME:  update for zPct
    if (_locked) return;

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

    if (_locked) return;

    float newtop = (float)i / (float)previewH;
    float topy = mPos[0].y * (float)previewH;
    for(int j = 1; j < num_points; ++j ) {
        float newy = mPos[j].y * (float)previewH;
        topy = std::max(topy, newy);
    }
    float diff = topy - newtop;
    SetVcenterOffset(diff);
}

void PolyPointScreenLocation::SetLeft(int i) {

    if (_locked) return;

    float newleft = (float)i / (float)previewW;
    float leftx = mPos[0].x * (float)previewW;
    for(int j = 1; j < num_points; ++j ) {
        float newx = mPos[j].x * (float)previewW;
        leftx = std::max(leftx, newx);
    }
    float diff = leftx - newleft;
    SetHcenterOffset(diff);
}

void PolyPointScreenLocation::SetRight(int i) {

    if (_locked) return;

    float newright = (float)i / (float)previewW;
    float rightx = mPos[0].x * (float)previewW;
    for(int j = 1; j < num_points; ++j ) {
        float newx = mPos[j].x * (float)previewW;
        rightx = std::max(rightx, newx);
    }
    float diff = rightx - newright;
    SetHcenterOffset(diff);
}

void PolyPointScreenLocation::SetBottom(int i) {

    if (_locked) return;

    float newbot = (float)i / (float)previewH;
    float boty = mPos[0].y * (float)previewH;
    for(int j = 1; j < num_points; ++j ) {
        float newy = mPos[j].y * (float)previewH;
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
