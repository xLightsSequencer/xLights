/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "BoxedScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <glm/glm.hpp>
#include <glm/gtx/matrix_transform_2d.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "Model.h"
#include "../ModelPreview.h"
#include "../support/VectorMath.h"
#include "RulerObject.h"

#include <log4cpp/Category.hh>

#include <cmath>

extern wxCursor GetResizeCursor(int cornerIndex, int PreviewRotation);
extern glm::vec3 rotationMatrixToEulerAngles(const glm::mat3 &R);

inline void TranslatePointDoubles(float radians,float x, float y,float &x1, float &y1) {
    float s = sin(radians);
    float c = cos(radians);
    x1 = c*x-(s*y);
    y1 = s*x+(c*y);
}

#define BOUNDING_RECT_OFFSET 8

static glm::mat4 Identity(glm::mat4(1.0f));


BoxedScreenLocation::BoxedScreenLocation()
    : ModelScreenLocation(10), perspective(0.0f), centerx(0.0), centery(0.0), centerz(0.0)
{
    mSelectableHandles = 1;
    handle_aabb_min.push_back(glm::vec3(0.0f));
    handle_aabb_max.push_back(glm::vec3(0.0f));
}

BoxedScreenLocation::BoxedScreenLocation(int points)
    : ModelScreenLocation(points), perspective(0.0f), centerx(0.0), centery(0.0), centerz(0.0)
{
}

ModelScreenLocation::MSLUPGRADE BoxedScreenLocation::CheckUpgrade(wxXmlNode* node)
{
    // check for upgrade to world positioning
    int version = wxAtoi(node->GetAttribute("versionNumber", "0"));
    if (version < 2) {
        // skip first upgrade call since preview size is not set
        node->DeleteAttribute("versionNumber");
        node->AddAttribute("versionNumber", "2");
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_SKIPPED;
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
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_EXEC_DONE;
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
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED;
    }
    return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED;
}

void BoxedScreenLocation::Read(wxXmlNode *ModelNode) {
    ModelScreenLocation::MSLUPGRADE upgrade_result = CheckUpgrade(ModelNode);
    if (upgrade_result == ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED) {
        worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "200.0"));
        worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
        worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));

        if (!std::isfinite(worldPos_x)) {
            worldPos_x = 0.0F;
        }
        if (!std::isfinite(worldPos_y)) {
            worldPos_y = 0.0F;
        }
        if (!std::isfinite(worldPos_z)) {
            worldPos_z = 0.0F;
        }

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

void BoxedScreenLocation::ApplyModelViewMatrices(xlGraphicsContext *ctx) const {
    ctx->Translate(worldPos_x,
                   worldPos_y,
                   draw_3d ? worldPos_z : 0);

    ctx ->Rotate(rotatez, 0, 0, 1)
        ->Rotate(rotatey, 0, 1, 0)
        ->Rotate(rotatex, 1, 0, 0)
        ->Scale(scalex, scaley, scalez);

    if (!draw_3d && perspective != 0.0f) {
        // Give 2D tree/cube/sphere models a perspective so you can see the back nodes
        // perspective is in radians, we need degrees fro the Rotate call
        float f = perspective * 180.0f / 3.14159f;
        ctx->Rotate(f, 1, 0, 0);
    }
        
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
        TranslateMatrix)
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
        FindPlaneIntersection( x, y, preview );
        if (preview->Is3D()) {
            if (supportsZScaling && !_startOnXAxis) {
                worldPos_y = RenderHt / 2.0f;
                active_axis = MSLAXIS::Y_AXIS;
            } else if (active_axis ==  MSLAXIS::Z_AXIS) {
                rotatey = 90.0f;
            }
            handle = CENTER_HANDLE;
        } else {
            active_axis = MSLAXIS::Y_AXIS;
            centery = worldPos_y;
            centerx = worldPos_x;
            centerz = worldPos_z;
            scalex = scaley = scalez = 0.0f;
            handle = R_BOT_HANDLE;
        }
    } else {
        DisplayError("InitializeLocation: called with no preview....investigate!");
    }
    return wxCURSOR_SIZING;
}

void BoxedScreenLocation::UpdateBoundingBox(const std::vector<NodeBaseClassPtr> &Nodes)
{
    if (Nodes.size() > 0) {
        aabb_min = glm::vec3(100000.0f, 100000.0f, 100000.0f);
        aabb_max = glm::vec3(0.0f, 0.0f, 0.0f);

        for (const auto& it : Nodes) {
            for (const auto& coord : it.get()->Coords) {

                float sx = coord.screenX;
                float sy = coord.screenY;
                float sz = coord.screenZ;

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
void BoxedScreenLocation::UpdateBoundingBox(float width, float height, float depth) {
    ModelScreenLocation::UpdateBoundingBox(width, height, depth);
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

bool BoxedScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();
    vac->PreAlloc(6 * 5 + 2);

    float w1 = worldPos_x;
    float h1 = worldPos_y;

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase)
    {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    }
    else if (_locked) {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    }

    float hw = GetRectHandleWidth(zoom, scale);
    
    // Upper Left Handle
    float sx = -RenderWi / 2;
    float sy = RenderHt / 2;
    float sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    sx += -BOUNDING_RECT_OFFSET;
    sy += BOUNDING_RECT_OFFSET;
    vac->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw /2), sy + (hw / 2), handleColor);
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
    vac->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), handleColor);
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
    vac->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), handleColor);
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
    vac->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), handleColor);
    mHandlePosition[L_BOT_HANDLE].x = sx;
    mHandlePosition[L_BOT_HANDLE].y = sy;
    mHandlePosition[L_BOT_HANDLE].z = sz;

    // Draw rotation handle square
    sx = 0.0f;
    sy = RenderHt / 2 + (50/scaley);
    sz = 0.0f;
    TranslatePoint(sx, sy, sz);
    vac->AddRectAsTriangles(sx - (hw / 2), sy - (hw / 2), sx + (hw / 2), sy + (hw / 2), handleColor);
    // Save rotate handle
    mHandlePosition[ROTATE_HANDLE].x = sx;
    mHandlePosition[ROTATE_HANDLE].y = sy;
    mHandlePosition[ROTATE_HANDLE].z = sz;
        
    vac->AddVertex(w1, h1, xlWHITE);
    vac->AddVertex(sx, sy, xlWHITE);

    int count = vac->getCount() - startVertex;
    program->addStep([this, startVertex, count, vac, program](xlGraphicsContext *ctx) {
        ctx->drawTriangles(vac, startVertex, count - 2);
        ctx->drawLines(vac, startVertex + count - 2, 2);
    });
    return true;
}
bool BoxedScreenLocation::DrawHandles(xlGraphicsProgram* program, float zoom, int scale, bool drawBounding, bool fromBase) const
{
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();
    vac->PreAlloc(32 * 5);

    float sz1 = RenderDp / 2;
    float sz2 =  -RenderDp / 2;

    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase) {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
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
    float hw = GetRectHandleWidth(zoom, scale);
    handle_aabb_min[CENTER_HANDLE].x = -hw;
    handle_aabb_min[CENTER_HANDLE].y = -hw;
    handle_aabb_min[CENTER_HANDLE].z = -hw;
    handle_aabb_max[CENTER_HANDLE].x = hw;
    handle_aabb_max[CENTER_HANDLE].y = hw;
    handle_aabb_max[CENTER_HANDLE].z = hw;
    mHandlePosition[CENTER_HANDLE].x = worldPos_x;
    mHandlePosition[CENTER_HANDLE].y = worldPos_y;
    mHandlePosition[CENTER_HANDLE].z = worldPos_z;

    xlColor Box3dColor = xlWHITE;
    if (fromBase)
        Box3dColor = FROM_BASE_HANDLES_COLOUR;
    else if (_locked)
        Box3dColor = LOCKED_HANDLES_COLOUR;

    vac->AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, Box3dColor);

    vac->AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, Box3dColor);

    vac->AddVertex(mHandlePosition[L_TOP_HANDLE].x, mHandlePosition[L_TOP_HANDLE].y, mHandlePosition[L_TOP_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_TOP_HANDLE_Z].x, mHandlePosition[L_TOP_HANDLE_Z].y, mHandlePosition[L_TOP_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_TOP_HANDLE].x, mHandlePosition[R_TOP_HANDLE].y, mHandlePosition[R_TOP_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_TOP_HANDLE_Z].x, mHandlePosition[R_TOP_HANDLE_Z].y, mHandlePosition[R_TOP_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_BOT_HANDLE].x, mHandlePosition[R_BOT_HANDLE].y, mHandlePosition[R_BOT_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[R_BOT_HANDLE_Z].x, mHandlePosition[R_BOT_HANDLE_Z].y, mHandlePosition[R_BOT_HANDLE_Z].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_BOT_HANDLE].x, mHandlePosition[L_BOT_HANDLE].y, mHandlePosition[L_BOT_HANDLE].z, Box3dColor);
    vac->AddVertex(mHandlePosition[L_BOT_HANDLE_Z].x, mHandlePosition[L_BOT_HANDLE_Z].y, mHandlePosition[L_BOT_HANDLE_Z].z, Box3dColor);
    
    int lineCount = vac->getCount();
    program->addStep([this, lineCount, startVertex, program, vac](xlGraphicsContext *ctx) {
        ctx->drawLines(vac, startVertex, lineCount - startVertex);
    });


    if (active_handle != -1) {
        active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
        int startTriangles = vac->getCount();
        vac->AddSphereAsTriangles(mHandlePosition[CENTER_HANDLE].x, mHandlePosition[CENTER_HANDLE].y, mHandlePosition[CENTER_HANDLE].z, (double)(GetRectHandleWidth(zoom, scale)), xlORANGETRANSLUCENT);
        int count = vac->getCount();
        program->addStep([startTriangles, count, program, vac](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startTriangles, count - startTriangles);
        });

        DrawAxisTool(active_handle_pos, program, zoom, scale);
        if (active_axis != MSLAXIS::NO_AXIS) {
            startVertex = vac->getCount();
            switch (active_axis) {
            case MSLAXIS::X_AXIS:
                vac->AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                vac->AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                break;
            case MSLAXIS::Y_AXIS:
                vac->AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                vac->AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                break;
            case MSLAXIS::Z_AXIS:
                vac->AddVertex(active_handle_pos.x, active_handle_pos.y, -1000000.0f, xlBLUETRANSLUCENT);
                vac->AddVertex(active_handle_pos.x, active_handle_pos.y, +1000000.0f, xlBLUETRANSLUCENT);
                break;
            default:
                wxASSERT(false);
                break;
            }
            lineCount = vac->getCount();
            program->addStep([lineCount, startVertex, program, vac](xlGraphicsContext *ctx) {
                ctx->drawLines(vac, startVertex, lineCount - startVertex);
            });

        }
    }
    return true;
}

void BoxedScreenLocation::AddDimensionProperties(wxPropertyGridInterface* propertyEditor, float factor) const
{
    wxPGProperty* prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Width (%s)", RulerObject::GetUnitDescription()), "RealWidth", GetRealWidth()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Min", 0.01);
    prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Height (%s)", RulerObject::GetUnitDescription()), "RealHeight", GetRealHeight()));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Min", 0.01);
    if (supportsZScaling) {
        prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Depth (%s)", RulerObject::GetUnitDescription()), "RealDepth", GetRealDepth()));
        prop->SetAttribute("Precision", 2);
        prop->SetAttribute("Min", 0.01);
    }
}

float BoxedScreenLocation::GetRealWidth() const
{
    return RulerObject::Measure(GetRestorableMWidth());
}

float BoxedScreenLocation::GetRealHeight() const 
{
    return RulerObject::Measure(GetRestorableMHeight());
}

float BoxedScreenLocation::GetRealDepth() const
{
    if (supportsZScaling) {
        return RulerObject::Measure(GetRestorableMDepth());
    }
    return 0;
}

std::string BoxedScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    if (supportsZScaling) {
        return wxString::Format("Width %s Height %s Depth %s",
            RulerObject::MeasureDescription(GetMWidth()),
            RulerObject::MeasureDescription(GetMHeight()),
            RulerObject::MeasureDescription(GetMDepth())).ToStdString();
    }
    return wxString::Format("Width %s Height %s",
        RulerObject::MeasureDescription(GetMWidth()),
        RulerObject::MeasureDescription(GetMHeight())).ToStdString();
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
    } else if (!_locked && "RealWidth" == name) {
        SetMWidth(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealWidth");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::RealWidth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::RealWidth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::RealWidth");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (_locked && "RealWidth" == name) {
        event.Veto();
        return 0;
    } else if (!_locked && "RealHeight" == name) {
        SetMHeight(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealHeight");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::RealHeight");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::RealHeight");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::RealHeight");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (_locked && "RealHeight" == name) {
        event.Veto();
        return 0;
    } else if (!_locked && "RealDepth" == name) {
        SetMDepth(RulerObject::UnMeasure(event.GetValue().GetDouble()));
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::RealDepth");
        AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "BoxedScreenLocation::OnPropertyGridChange::RealDepth");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::RealDepth");
        AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "BoxedScreenLocation::OnPropertyGridChange::RealDepth");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    } else if (_locked && "RealDepth" == name) {
        event.Veto();
        return 0;
    } else if ("Locked" == name)
    {
        _locked = event.GetValue().GetBool();
        AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "BoxedScreenLocation::OnPropertyGridChange::Locked");
        AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "BoxedScreenLocation::OnPropertyGridChange::Locked");
        return 0;
    }

    return 0;
}

bool BoxedScreenLocation::Rotate(MSLAXIS axis, float factor)
{
    if (_locked) return false;

    glm::quat rot;
    switch (axis) {
    case MSLAXIS::X_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    case MSLAXIS::Y_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    case MSLAXIS::Z_AXIS:
        rot = glm::angleAxis(glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    default:
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

        if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
                worldPos_x = saved_position.x + drag_delta.x;
                break;
            case MSLAXIS::Y_AXIS:
                worldPos_y = saved_position.y + drag_delta.y;
                break;
            case MSLAXIS::Z_AXIS:
                worldPos_z = saved_position.z + drag_delta.z;
                break;
            default:
                break;
            }
        } else if (axis_tool == MSLTOOL::TOOL_ROTATE) {
            glm::vec3 start_vector = saved_intersect - saved_position;
            glm::vec3 end_vector = start_vector + drag_delta;
            switch (active_axis)
            {
            case MSLAXIS::X_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.z) * 180.0 / M_PI;
                double total_change = end_angle - start_angle;
                double delta = angles.x - total_change;
                angles.x = total_change;
                Rotate(MSLAXIS::X_AXIS, delta);
            }
                break;
            case MSLAXIS::Y_AXIS:
            {
                double start_angle = atan2(start_vector.x, start_vector.z) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.x, end_vector.z) * 180.0 / M_PI;
                double total_change = end_angle - start_angle;
                double delta = angles.y - total_change;
                angles.y = total_change;
                Rotate(MSLAXIS::Y_AXIS, -delta);
            }
                break;
            case MSLAXIS::Z_AXIS:
            {
                double start_angle = atan2(start_vector.y, start_vector.x) * 180.0 / M_PI;
                double end_angle = atan2(end_vector.y, end_vector.x) * 180.0 / M_PI;
                double total_change = end_angle - start_angle;
                double delta = angles.z - total_change;
                angles.z = total_change;
                Rotate(MSLAXIS::Z_AXIS, -delta);
            }
                break;
            default:
                break;
            }
        } else if (axis_tool == MSLTOOL::TOOL_SCALE) {
            float change_x = ((saved_size.x*saved_scale.x + drag_delta.x) / (saved_size.x*saved_scale.x));
            float change_y = ((saved_size.y*saved_scale.y + drag_delta.y) / (saved_size.y*saved_scale.y));
            float change_z = ((saved_size.z*saved_scale.z + drag_delta.z) / (saved_size.z*saved_scale.z));
            
            if (CtrlKeyPressed) {
                switch (active_axis)
                {
                case MSLAXIS::X_AXIS:
                    scalex = saved_scale.x * change_x;
                    if (scale_z) {
                        scalez = scalex;
                    }
                    scaley = saved_scale.y * change_x;
                    break;
                case MSLAXIS::Y_AXIS:
                    scaley = saved_scale.y * change_y;
                    scalex = saved_scale.x * change_y;
                    if (scale_z) {
                        scalez = saved_scale.x * change_y;
                    }
                    break;
                case MSLAXIS::Z_AXIS:
                    if (scale_z) {
                        scalez = saved_scale.z * change_z;
                    }
                    scalex = saved_scale.z * change_z;
                    scaley = saved_scale.y * change_z;
                    break;
                default:
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
                case MSLAXIS::X_AXIS:
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
                case MSLAXIS::Y_AXIS:
                    scaley = saved_scale.y * change_y;
                    if (ShiftKeyPressed) {
                        float current_bottom = saved_position.y - (saved_scale.y * RenderHt / 2.0f);
                        worldPos_y = current_bottom + (scaley * RenderHt / 2.0f);
                    }
                    break;
                case MSLAXIS::Z_AXIS:
                    if (scale_z) {
                        scalez = saved_scale.z * change_z;
                    }
                    if (ShiftKeyPressed) {
                        scalex = saved_scale.z * change_z;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    }
    return 1;
}
int BoxedScreenLocation::MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) {
    if (handle == CENTER_HANDLE) {
        constexpr float rscale = 10; //10 degrees per full 1.0 aka: max speed
        Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, rot.x * rscale);
        Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -rot.z * rscale);
        Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, rot.y * rscale);
        AddOffset(mov.x * scale, -mov.z * scale, mov.y * scale);
    } else {
        float change_x = mov.x * scale;
        float change_y = -mov.z * scale;
        float change_z = mov.y * scale;
        scalex = saved_scale.x * change_x;
        scaley = saved_scale.y * change_y;
        scalez = saved_scale.z * change_z;
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

glm::vec2 BoxedScreenLocation::GetScreenOffset(ModelPreview* preview) const
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
