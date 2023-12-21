/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "PolyPointScreenLocation.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include "../ModelPreview.h"
#include "Shapes.h"
#include "../support/VectorMath.h"
#include "UtilFunctions.h"
#include "RulerObject.h"

#include <log4cpp/Category.hh>

extern void DrawBoundingBoxLines(const xlColor &c, glm::vec3& min_pt, glm::vec3& max_pt, glm::mat4& bound_matrix, xlVertexColorAccumulator &va);

static float BB_OFF = 5.0f;

static glm::mat4 Identity(glm::mat4(1.0f));

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

void PolyPointScreenLocation::SetCurve(int seg_num, bool create)
{

    if (_locked) return;

    if (create) {
        mPos[seg_num].has_curve = true;
        if (mPos[seg_num].curve == nullptr) {
            mPos[seg_num].curve = new BezierCurveCubic3D();
        }
        mPos[seg_num].curve->set_p0(mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z);
        mPos[seg_num].curve->set_p1(mPos[seg_num + 1].x, mPos[seg_num + 1].y, mPos[seg_num + 1].z);
        mPos[seg_num].curve->set_cp0(mPos[seg_num].x, mPos[seg_num].y, mPos[seg_num].z);
        mPos[seg_num].curve->set_cp1(mPos[seg_num + 1].x, mPos[seg_num + 1].y, mPos[seg_num + 1].z);
    }
    else {
        mPos[seg_num].has_curve = false;
        if (mPos[seg_num].curve != nullptr) {
            delete mPos[seg_num].curve;
            mPos[seg_num].curve = nullptr;
        }
    }
}

ModelScreenLocation::MSLUPGRADE PolyPointScreenLocation::CheckUpgrade(wxXmlNode* node)
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
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_SKIPPED;
    }
    else if (version == 2) {
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
        return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_EXEC_READ;
    }
    return ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED;
}

void PolyPointScreenLocation::Read(wxXmlNode* ModelNode)
{
    ModelScreenLocation::MSLUPGRADE upgrade_result = CheckUpgrade(ModelNode);
    if (upgrade_result == ModelScreenLocation::MSLUPGRADE::MSLUPGRADE_NOT_NEEDED) {
        worldPos_x = wxAtof(ModelNode->GetAttribute("WorldPosX", "0.0"));
        if (isnan(worldPos_x)) worldPos_x = 0.0;
        worldPos_y = wxAtof(ModelNode->GetAttribute("WorldPosY", "0.0"));
        if (isnan(worldPos_y)) worldPos_y = 0.0;
        worldPos_z = wxAtof(ModelNode->GetAttribute("WorldPosZ", "0.0"));
        if (isnan(worldPos_z)) worldPos_z = 0.0;

        scalex = wxAtof(ModelNode->GetAttribute("ScaleX", "100.0"));
        scaley = wxAtof(ModelNode->GetAttribute("ScaleY", "100.0"));
        scalez = wxAtof(ModelNode->GetAttribute("ScaleZ", "100.0"));

        if (scalex <= 0 || std::isinf(scalex) || isnan(scalex)) {
            scalex = 1.0f;
        }
        if (scaley <= 0 || std::isinf(scaley) || isnan(scaley)) {
            scaley = 1.0f;
        }
        if (scalez <= 0 || std::isinf(scalez) || isnan(scalez)) {
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

void PolyPointScreenLocation::Write(wxXmlNode* node)
{
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
    for (int i = 0; i < num_points; ++i) {
        point_data += wxString::Format("%f,", mPos[i].x);
        point_data += wxString::Format("%f,", mPos[i].y);
        point_data += wxString::Format("%f", mPos[i].z);
        if (i != num_points - 1) {
            point_data += ",";
        }
    }
    wxString cpoint_data = "";
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            cpoint_data += wxString::Format("%d,%f,%f,%f,%f,%f,%f,", i, mPos[i].curve->get_cp0x(), mPos[i].curve->get_cp0y(), mPos[i].curve->get_cp0z(),
                mPos[i].curve->get_cp1x(), mPos[i].curve->get_cp1y(), mPos[i].curve->get_cp1z());
        }
    }
    node->AddAttribute("NumPoints", std::to_string(num_points));
    node->AddAttribute("PointData", point_data);
    node->AddAttribute("cPointData", cpoint_data);
    if (_locked) {
        node->AddAttribute("Locked", "1");
    }
}

void PolyPointScreenLocation::PrepareToDraw(bool is_3d, bool allow_selected) const
{
    std::unique_lock<std::mutex> locker(_mutex);
    minX = 100000.0;
    minY = 100000.0;
    minZ = 100000.0;
    maxX = 0.0;
    maxY = 0.0;
    maxZ = 0.0;

    glm::vec3 scaling(scalex, scaley, scalez);
    glm::vec3 world_pos(worldPos_x, worldPos_y, worldPos_z);

    for (int i = 0; i < num_points - 1; ++i) {
        float x1p = mPos[i].x * scalex + worldPos_x;
        float x2p = mPos[i + 1].x * scalex + worldPos_x;
        float y1p = mPos[i].y * scaley + worldPos_y;
        float y2p = mPos[i + 1].y * scaley + worldPos_y;
        float z1p = mPos[i].z * scalez + worldPos_z;
        float z2p = mPos[i + 1].z * scalez + worldPos_z;

        if (!is_3d) {
            // allows 2D selection to work
            z1p = 0.0f;
            z2p = 0.0f;
        }

        if (mPos[i].x < minX) minX = mPos[i].x;
        if (mPos[i].y < minY) minY = mPos[i].y;
        if (mPos[i].z < minZ) minZ = mPos[i].z;
        if (mPos[i].x > maxX) maxX = mPos[i].x;
        if (mPos[i].y > maxY) maxY = mPos[i].y;
        if (mPos[i].z > maxZ) maxZ = mPos[i].z;

        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->check_min_max(minX, maxX, minY, maxY, minZ, maxZ);
        }

        if (i == num_points - 2) {
            if (mPos[i + 1].x < minX) minX = mPos[i + 1].x;
            if (mPos[i + 1].y < minY) minY = mPos[i + 1].y;
            if (mPos[i + 1].z < minZ) minZ = mPos[i + 1].z;
            if (mPos[i + 1].x > maxX) maxX = mPos[i + 1].x;
            if (mPos[i + 1].y > maxY) maxY = mPos[i + 1].y;
            if (mPos[i + 1].z > maxZ) maxZ = mPos[i + 1].z;
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
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->SetPositioning(scaling, world_pos);
            mPos[i].curve->UpdatePoints();
        }
    }
    float yscale = (maxY - minY) * scaley;
    if (RenderHt > 1.0f && (maxY - minY < RenderHt)) {
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
void PolyPointScreenLocation::ApplyModelViewMatrices(xlGraphicsContext *ctx) const {
    ctx->SetModelMatrix(main_matrix);
}

bool PolyPointScreenLocation::IsContained(ModelPreview* preview, int x1, int y1, int x2, int y2) const
{
    int sx1 = std::min(x1, x2);
    int sx2 = std::max(x1, x2);
    int sy1 = std::min(y1, y2);
    int sy2 = std::max(y1, y2);
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
    }
    else {
        if (x1p >= sx1 && x1p <= sx2 &&
            x2p >= sx1 && x2p <= sx2 &&
            y1p >= sy1 && y1p <= sy2 &&
            y2p >= sy1 && y2p <= sy2) {
            return true;
        }
    }
    return false;
}

bool PolyPointScreenLocation::HitTest(glm::vec3& ray_origin, glm::vec3& ray_direction) const
{

    bool ret_value = false;
    selected_segment = -1;

    for (int i = 0; i < num_points - 1; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            if (mPos[i].curve->HitTest(ray_origin)) {
                selected_segment = i;
                ret_value = true;
                break;
            }
        }
        else {
            // perform normal line segment hit detection
            if (mPos[i].mod_matrix != nullptr) {
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
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            if (mPos[i].curve->HitTest3D(ray_origin, ray_direction, distance)) {
                if (distance < intersection_distance) {
                    intersection_distance = distance;
                    selected_segment = i;
                }
                ret_value = true;
            }
        }
        else {
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

wxCursor PolyPointScreenLocation::CheckIfOverHandles3D(glm::vec3& ray_origin, glm::vec3& ray_direction, int& handle, float zoom, int scale) const
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

        float hw = GetRectHandleWidth(zoom, scale);
        if (selected_segment != -1) {
            // add control point handles for selected segments
            int s = selected_segment;
            if (mPos.size() > s && mPos[s].has_curve && mPos[s].curve != nullptr) {
                glm::vec3 cp_handle_aabb_min[2];
                glm::vec3 cp_handle_aabb_max[2];
                cp_handle_aabb_min[0].x = (mPos[s].curve->get_cp0x() - minX) * scalex - hw;
                cp_handle_aabb_min[0].y = (mPos[s].curve->get_cp0y() - minY) * scaley - hw;
                cp_handle_aabb_min[0].z = (mPos[s].curve->get_cp0z() - minZ) * scalez - hw;
                cp_handle_aabb_max[0].x = (mPos[s].curve->get_cp0x() - minX) * scalex + hw;
                cp_handle_aabb_max[0].y = (mPos[s].curve->get_cp0y() - minY) * scaley + hw;
                cp_handle_aabb_max[0].z = (mPos[s].curve->get_cp0z() - minZ) * scalez + hw;
                cp_handle_aabb_min[1].x = (mPos[s].curve->get_cp1x() - minX) * scalex - hw;
                cp_handle_aabb_min[1].y = (mPos[s].curve->get_cp1y() - minY) * scaley - hw;
                cp_handle_aabb_min[1].z = (mPos[s].curve->get_cp1z() - minZ) * scalez - hw;
                cp_handle_aabb_max[1].x = (mPos[s].curve->get_cp1x() - minX) * scalex + hw;
                cp_handle_aabb_max[1].y = (mPos[s].curve->get_cp1y() - minY) * scaley + hw;
                cp_handle_aabb_max[1].z = (mPos[s].curve->get_cp1z() - minZ) * scalez + hw;

                // Test each each Oriented Bounding Box (OBB).
                for (size_t i = 0; i < 2; i++) {
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
                            handle = ((i == 0) ? s | HANDLE_CP0 : s | HANDLE_CP1);
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
            if (mPos[i].has_curve && mPos[i].curve != nullptr) {
                if (mPos[i].curve->HitTest3D(ray_origin, ray_direction, intersection_distance)) {
                    handle = i | HANDLE_SEGMENT;
                    return_value = wxCURSOR_DEFAULT;
                }
            }
            else {
                // perform normal line segment hit detection
                if (mPos[i].mod_matrix != nullptr) {
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
                            handle = i | HANDLE_SEGMENT;
                            return_value = wxCURSOR_DEFAULT;
                        }
                    }
                }
            }
        }
    }

    return return_value;
}

wxCursor PolyPointScreenLocation::CheckIfOverHandles(ModelPreview* preview, int& handle, int x, int y) const
{
    wxASSERT(!preview->Is3D());

    wxCursor return_value = wxCURSOR_DEFAULT;

    if (preview == nullptr) return return_value;

    handle = NO_HANDLE;
    float zoom = preview->GetCameraZoomForHandles();
    int scale = preview->GetHandleScale();

    if (_locked) {
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
            if (mPos[s].has_curve && mPos[s].curve != nullptr) {
                glm::vec3 cp_handle_aabb_min[2];
                glm::vec3 cp_handle_aabb_max[2];

                float hw = GetRectHandleWidth(zoom, scale);
                cp_handle_aabb_min[0].x = (mPos[s].curve->get_cp0x() - minX) * scalex - hw;
                cp_handle_aabb_min[0].y = (mPos[s].curve->get_cp0y() - minY) * scaley - hw;
                cp_handle_aabb_min[0].z = (mPos[s].curve->get_cp0z() - minZ) * scalez - hw;
                cp_handle_aabb_max[0].x = (mPos[s].curve->get_cp0x() - minX) * scalex + hw;
                cp_handle_aabb_max[0].y = (mPos[s].curve->get_cp0y() - minY) * scaley + hw;
                cp_handle_aabb_max[0].z = (mPos[s].curve->get_cp0z() - minZ) * scalez + hw;
                cp_handle_aabb_min[1].x = (mPos[s].curve->get_cp1x() - minX) * scalex - hw;
                cp_handle_aabb_min[1].y = (mPos[s].curve->get_cp1y() - minY) * scaley - hw;
                cp_handle_aabb_min[1].z = (mPos[s].curve->get_cp1z() - minZ) * scalez - hw;
                cp_handle_aabb_max[1].x = (mPos[s].curve->get_cp1x() - minX) * scalex + hw;
                cp_handle_aabb_max[1].y = (mPos[s].curve->get_cp1y() - minY) * scaley + hw;
                cp_handle_aabb_max[1].z = (mPos[s].curve->get_cp1z() - minZ) * scalez + hw;

                // Test each each Oriented Bounding Box (OBB).
                for (size_t i = 0; i < 2; i++) {
                    if (VectorMath::TestRayOBBIntersection2D(
                        ray_origin,
                        cp_handle_aabb_min[i],
                        cp_handle_aabb_max[i],
                        ModelMatrix)
                        ) {
                        handle = ((i == 0) ? s | HANDLE_CP0 : s | HANDLE_CP1);
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
        for (size_t i = 1; i < mSelectableHandles; i++) {
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
            if (mPos[i].has_curve && mPos[i].curve != nullptr) {
                if (mPos[i].curve->HitTest(ray_origin)) {
                    if (i != selected_segment) {
                        handle = i | HANDLE_SEGMENT;
                        return_value = wxCURSOR_BULLSEYE;
                    }
                    break;
                }
            }
            else {
                if (mPos[i].mod_matrix != nullptr) {
                    if (VectorMath::TestRayOBBIntersection2D(
                        ray_origin,
                        seg_aabb_min[i],
                        seg_aabb_max[i],
                        *mPos[i].mod_matrix)
                        ) {
                        if (i != selected_segment) {
                            handle = i | HANDLE_SEGMENT;
                            return_value = wxCURSOR_BULLSEYE;
                        }
                        break;
                    }
                }
            }
        }
    }

    // test for clicking a boundary handle
    if (handle == NO_HANDLE) {
        float hw = GetRectHandleWidth(zoom, scale);
        for (size_t h = num_points + 1; h < num_points + 5; h++) {
            handle_aabb_min[h].x = mHandlePosition[h].x - hw;
            handle_aabb_min[h].y = mHandlePosition[h].y - hw;
            handle_aabb_min[h].z = mHandlePosition[h].z - hw;
            handle_aabb_max[h].x = mHandlePosition[h].x + hw;
            handle_aabb_max[h].y = mHandlePosition[h].y + hw;
            handle_aabb_max[h].z = mHandlePosition[h].z + hw;

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

void PolyPointScreenLocation::SetAxisTool(MSLTOOL mode)
{
    if (active_handle == CENTER_HANDLE) {
        ModelScreenLocation::SetAxisTool(mode);
    }
    else {
        if (mode == MSLTOOL::TOOL_TRANSLATE || mode == MSLTOOL::TOOL_XY_TRANS) {
            axis_tool = mode;
        }
        else {
            axis_tool = MSLTOOL::TOOL_TRANSLATE;
        }
    }
}

void PolyPointScreenLocation::AdvanceAxisTool()
{
    if (active_handle == CENTER_HANDLE) {
        ModelScreenLocation::AdvanceAxisTool();
    }
    else {
        axis_tool = MSLTOOL::TOOL_TRANSLATE;
    }
}

void PolyPointScreenLocation::SetActiveAxis(MSLAXIS axis)
{
   ModelScreenLocation::SetActiveAxis(axis);
}
bool PolyPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool drawBounding, bool fromBase) const {
    std::unique_lock<std::mutex> locker(_mutex);

    if (active_handle != NO_HANDLE || mouse_down) {
        auto vac = program->getAccumulator();
        int startVertex = vac->getCount();
        vac->PreAlloc(10 * num_points + 12);
        xlColor h1c, h2c, h3c;
        if (fromBase)
        {
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
            h2c = xlBLUETRANSLUCENT;
            h3c = (highlighted_handle == CENTER_HANDLE) ? xlYELLOWTRANSLUCENT : xlORANGETRANSLUCENT;
        }

        // add center handle
        float cx, cy, cz;
        float hw = GetRectHandleWidth(zoom, scale);
        if (!mouse_down) {
            cx = (maxX + minX) * scalex / 2.0f + worldPos_x;
            cy = (maxY + minY) * scaley / 2.0f + worldPos_y;
            cz = (maxZ + minZ) * scalez / 2.0f + worldPos_z;
            mHandlePosition[CENTER_HANDLE].x = cx;
            mHandlePosition[CENTER_HANDLE].y = cy;
            mHandlePosition[CENTER_HANDLE].z = cz;
            handle_aabb_min[CENTER_HANDLE].x = (maxX - minX) * scalex / 2.0f - hw;
            handle_aabb_min[CENTER_HANDLE].y = (maxY - minY) * scaley / 2.0f - hw;
            handle_aabb_min[CENTER_HANDLE].z = (maxZ - minZ) * scalez / 2.0f - hw;
            handle_aabb_max[CENTER_HANDLE].x = (maxX - minX) * scalex / 2.0f + hw;
            handle_aabb_max[CENTER_HANDLE].y = (maxY - minY) * scaley / 2.0f + hw;
            handle_aabb_max[CENTER_HANDLE].z = (maxZ - minZ) * scalez / 2.0f + hw;
        }
        vac->AddSphereAsTriangles(mHandlePosition[CENTER_HANDLE].x, mHandlePosition[CENTER_HANDLE].y, mHandlePosition[CENTER_HANDLE].z, hw, h3c);
        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
        startVertex = count;

        for (int i = 0; i < num_points - 1; ++i) {
            int x1_pos = mPos[i].x * scalex + worldPos_x;
            int x2_pos = mPos[i + 1].x * scalex + worldPos_x;
            int y1_pos = mPos[i].y * scaley + worldPos_y;
            int y2_pos = mPos[i + 1].y * scaley + worldPos_y;
            int z1_pos = mPos[i].z * scalez + worldPos_z;
            int z2_pos = mPos[i + 1].z * scalez + worldPos_z;

            if (i == selected_segment) {
                count = vac->getCount();
                if (count != startVertex) {
                    program->addStep([=](xlGraphicsContext *ctx) {
                        ctx->drawTriangles(vac, startVertex, count - startVertex);
                    });
                    startVertex = count;
                }
                
                if (!mPos[i].has_curve || mPos[i].curve == nullptr) {
                    vac->AddVertex(x1_pos, y1_pos, z1_pos, xlMAGENTA);
                    vac->AddVertex(x2_pos, y2_pos, z2_pos, xlMAGENTA);
                } else {
                    // draw bezier curve
                    x1_pos = mPos[i].curve->get_px(0) * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_py(0) * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_pz(0) * scalez + worldPos_z;
                    for (int x = 1; x < mPos[i].curve->GetNumPoints(); ++x) {
                        x2_pos = mPos[i].curve->get_px(x) * scalex + worldPos_x;
                        y2_pos = mPos[i].curve->get_py(x) * scaley + worldPos_y;
                        z2_pos = mPos[i].curve->get_pz(x) * scalez + worldPos_z;
                        vac->AddVertex(x1_pos, y1_pos, z1_pos, xlMAGENTA);
                        vac->AddVertex(x2_pos, y2_pos, z2_pos, xlMAGENTA);
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
                    vac->AddVertex(x1_pos, y1_pos, z1_pos, xlRED);
                    vac->AddVertex(x2_pos, y2_pos, z2_pos, xlRED);
                    x1_pos = mPos[i].curve->get_p1x() * scalex + worldPos_x;
                    y1_pos = mPos[i].curve->get_p1y() * scaley + worldPos_y;
                    z1_pos = mPos[i].curve->get_p1z() * scalez + worldPos_z;
                    x2_pos = mPos[i].curve->get_cp1x() * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_cp1y() * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_cp1z() * scalez + worldPos_z;
                    vac->AddVertex(x1_pos, y1_pos, z1_pos, xlRED);
                    vac->AddVertex(x2_pos, y2_pos, z2_pos, xlRED);
                }
                count = vac->getCount();
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawLines(vac, startVertex, count - startVertex);
                });
                startVertex = count;
            }

            // add handle for start of this vector
            float sx = mPos[i].x * scalex + worldPos_x - hw / 2;
            float sy = mPos[i].y * scaley + worldPos_y - hw / 2;
            float sz = mPos[i].z * scalez + worldPos_z - hw / 2;
            int hpos = i + 1;
            vac->AddSphereAsTriangles(sx, sy, sz, hw, i == 0 ? h1c : (hpos == highlighted_handle ? xlYELLOW : h2c));
            mHandlePosition[hpos].x = sx;
            mHandlePosition[hpos].y = sy;
            mHandlePosition[hpos].z = sz;
            handle_aabb_min[hpos].x = (mPos[i].x - minX) * scalex - hw;
            handle_aabb_min[hpos].y = (mPos[i].y - minY) * scaley - hw;
            handle_aabb_min[hpos].z = (mPos[i].z - minZ) * scalez - hw;
            handle_aabb_max[hpos].x = (mPos[i].x - minX) * scalex + hw;
            handle_aabb_max[hpos].y = (mPos[i].y - minY) * scaley + hw;
            handle_aabb_max[hpos].z = (mPos[i].z - minZ) * scalez + hw;

            // add final handle
            if (i == num_points - 2) {
                hpos++;
                sx = mPos[i + 1].x * scalex + worldPos_x - hw / 2;
                sy = mPos[i + 1].y * scaley + worldPos_y - hw / 2;
                sz = mPos[i + 1].z * scalez + worldPos_z - hw / 2;
                vac->AddSphereAsTriangles(sx, sy, sz, hw, (hpos == highlighted_handle ? xlYELLOW : h2c));
                mHandlePosition[hpos].x = sx;
                mHandlePosition[hpos].y = sy;
                mHandlePosition[hpos].z = sz;
                handle_aabb_min[hpos].x = (mPos[i + 1].x - minX) * scalex - hw;
                handle_aabb_min[hpos].y = (mPos[i + 1].y - minY) * scaley - hw;
                handle_aabb_min[hpos].z = (mPos[i + 1].z - minZ) * scalez - hw;
                handle_aabb_max[hpos].x = (mPos[i + 1].x - minX) * scalex + hw;
                handle_aabb_max[hpos].y = (mPos[i + 1].y - minY) * scaley + hw;
                handle_aabb_max[hpos].z = (mPos[i + 1].z - minZ) * scalez + hw;
            }
        }

        glm::vec3 cp_handle_pos[2];
        if (selected_segment != -1) {
            // add control point handles for selected segments
            int i = selected_segment;
            if (mPos[i].has_curve && mPos[i].curve != nullptr) {
                float cxx = mPos[i].curve->get_cp0x() * scalex + worldPos_x - hw / 2;
                float cyy = mPos[i].curve->get_cp0y() * scaley + worldPos_y - hw / 2;
                float czz = mPos[i].curve->get_cp0z() * scalez + worldPos_z - hw / 2;
                h2c = highlighted_handle & HANDLE_CP0 ? ((highlighted_handle & HANDLE_MASK) == i ? xlYELLOW : xlRED) : xlRED;
                vac->AddSphereAsTriangles(cxx, cyy, czz, hw, h2c);
                mPos[i].cp0.x = mPos[i].curve->get_cp0x();
                mPos[i].cp0.y = mPos[i].curve->get_cp0y();
                mPos[i].cp0.z = mPos[i].curve->get_cp0z();
                cp_handle_pos[0].x = cxx;
                cp_handle_pos[0].y = cyy;
                cp_handle_pos[0].z = czz;
                cxx = mPos[i].curve->get_cp1x() * scalex + worldPos_x - hw / 2;
                cyy = mPos[i].curve->get_cp1y() * scaley + worldPos_y - hw / 2;
                czz = mPos[i].curve->get_cp1z() * scalez + worldPos_z - hw / 2;
                h2c = highlighted_handle & HANDLE_CP0 ? ((highlighted_handle & HANDLE_MASK) == i ? xlYELLOW : xlRED) : xlRED;
                vac->AddSphereAsTriangles(cxx, cyy, czz, hw, h2c);
                mPos[i].cp1.x = mPos[i].curve->get_cp1x();
                mPos[i].cp1.y = mPos[i].curve->get_cp1y();
                mPos[i].cp1.z = mPos[i].curve->get_cp1z();
                cp_handle_pos[1].x = cxx;
                cp_handle_pos[1].y = cyy;
                cp_handle_pos[1].z = czz;
            }
        }
        count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
        startVertex = count;

        if (!_locked) {
            if ((active_handle & HANDLE_CP0) > 0) {
                active_handle_pos = cp_handle_pos[0];
            }
            else if ((active_handle & HANDLE_CP1) > 0) {
                active_handle_pos = cp_handle_pos[1];
            }
            else {
                active_handle_pos = glm::vec3(mHandlePosition[active_handle].x, mHandlePosition[active_handle].y, mHandlePosition[active_handle].z);
            }
            DrawAxisTool(active_handle_pos, program, zoom, scale);
            if (active_axis != MSLAXIS::NO_AXIS) {
                if (axis_tool == MSLTOOL::TOOL_XY_TRANS) {
                    vac->AddVertex(-1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    vac->AddVertex(+1000000.0f, active_handle_pos.y, active_handle_pos.z, xlREDTRANSLUCENT);
                    vac->AddVertex(active_handle_pos.x, -1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                    vac->AddVertex(active_handle_pos.x, +1000000.0f, active_handle_pos.z, xlGREENTRANSLUCENT);
                }
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
                    break;
                }
                count = vac->getCount();
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawLines(vac, startVertex, count - startVertex);
                });
            }
        }
    } else if (drawBounding) {
        // draw bounding box for each segment if model is highlighted
        auto vac = program->getAccumulator();
        int startVertex = vac->getCount();
        xlColor Box3dColor = xlWHITETRANSLUCENT;
        if (fromBase)
            Box3dColor = FROM_BASE_HANDLES_COLOUR;
        else if (_locked)
            Box3dColor = LOCKED_HANDLES_COLOUR;
        for (int i = 0; i < num_points - 1; ++i) {
            if (mPos[i].has_curve && mPos[i].curve != nullptr) {
                mPos[i].curve->DrawBoundingBoxes(Box3dColor, vac);
            } else {
                DrawBoundingBoxLines(Box3dColor, seg_aabb_min[i], seg_aabb_max[i], *mPos[i].mod_matrix, *vac);
            }
        }
        int count = vac->getCount();
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawLines(vac, startVertex, count - startVertex);
        });
    }
    return true;
}

bool PolyPointScreenLocation::DrawHandles(xlGraphicsProgram *program, float zoom, int scale, bool fromBase) const {
    std::unique_lock<std::mutex> locker(_mutex);
    
    auto vac = program->getAccumulator();
    int startVertex = vac->getCount();

    vac->PreAlloc(10 * num_points + 12);

    // add boundary handles
    float hw = GetRectHandleWidth(zoom, scale);
    float boundary_offset = 2.0f * hw;
    float x1 = minX * scalex + worldPos_x - hw / 2 - boundary_offset;
    float y1 = minY * scaley + worldPos_y - hw / 2 - boundary_offset;
    float z1 = minZ * scalez + worldPos_z - hw / 2 - boundary_offset;
    float x2 = maxX * scalex + worldPos_x + hw / 2 + boundary_offset;
    float y2 = maxY * scaley + worldPos_y + hw / 2 + boundary_offset;
    float z2 = maxZ * scalez + worldPos_z + hw / 2 + boundary_offset;
    xlColor handleColor = xlBLUETRANSLUCENT;
    if (fromBase) {
        handleColor = FROM_BASE_HANDLES_COLOUR;
    } else
    if (_locked) {
        handleColor = LOCKED_HANDLES_COLOUR;
    }
    vac->AddRectAsTriangles(x1, y1, x1 + hw, y1 + hw, handleColor);
    vac->AddRectAsTriangles(x1, y2, x1 + hw, y2 + hw, handleColor);
    vac->AddRectAsTriangles(x2, y1, x2 + hw, y1 + hw, handleColor);
    vac->AddRectAsTriangles(x2, y2, x2 + hw, y2 + hw, handleColor);
    while (mHandlePosition.size() < num_points + 5) { // not sure this is the best way to do this but it stops a crash
        xlPoint pt;
        mHandlePosition.push_back(pt);
    }
    mHandlePosition[num_points + 1].x = x1;
    mHandlePosition[num_points + 1].y = y1;
    mHandlePosition[num_points + 1].z = z1;
    mHandlePosition[num_points + 2].x = x1;
    mHandlePosition[num_points + 2].y = y2;
    mHandlePosition[num_points + 2].z = z2;
    mHandlePosition[num_points + 3].x = x2;
    mHandlePosition[num_points + 3].y = y1;
    mHandlePosition[num_points + 3].z = z1;
    mHandlePosition[num_points + 4].x = x2;
    mHandlePosition[num_points + 4].y = y2;
    mHandlePosition[num_points + 4].z = z2;


    for (int i = 0; i < num_points - 1; ++i) {
        int x1_pos = mPos[i].x * scalex + worldPos_x;
        int x2_pos = mPos[i + 1].x * scalex + worldPos_x;
        int y1_pos = mPos[i].y * scaley + worldPos_y;
        int y2_pos = mPos[i + 1].y * scaley + worldPos_y;
        int z1_pos = mPos[i].z * scalez + worldPos_z;
        int z2_pos = mPos[i + 1].z * scalez + worldPos_z;

        if (i == selected_segment) {
            int count = vac->getCount();
            if (count != startVertex) {
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawTriangles(vac, startVertex, count - startVertex);
                });
                startVertex = count;
            }
            if (!mPos[i].has_curve || mPos[i].curve == nullptr) {
                vac->AddVertex(x1_pos, y1_pos, xlMAGENTA);
                vac->AddVertex(x2_pos, y2_pos, xlMAGENTA);
            } else {
                // draw bezier curve
                x1_pos = mPos[i].curve->get_px(0) * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_py(0) * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_pz(0) * scalez + worldPos_z;
                for (int x = 1; x < mPos[i].curve->GetNumPoints(); ++x) {
                    x2_pos = mPos[i].curve->get_px(x) * scalex + worldPos_x;
                    y2_pos = mPos[i].curve->get_py(x) * scaley + worldPos_y;
                    z2_pos = mPos[i].curve->get_pz(x) * scalez + worldPos_z;
                    vac->AddVertex(x1_pos, y1_pos, xlMAGENTA);
                    vac->AddVertex(x2_pos, y2_pos, xlMAGENTA);
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
                vac->AddVertex(x1_pos, y1_pos, xlRED);
                vac->AddVertex(x2_pos, y2_pos, xlRED);
                x1_pos = mPos[i].curve->get_p1x() * scalex + worldPos_x;
                y1_pos = mPos[i].curve->get_p1y() * scaley + worldPos_y;
                z1_pos = mPos[i].curve->get_p1z() * scalez + worldPos_z;
                x2_pos = mPos[i].curve->get_cp1x() * scalex + worldPos_x;
                y2_pos = mPos[i].curve->get_cp1y() * scaley + worldPos_y;
                z2_pos = mPos[i].curve->get_cp1z() * scalez + worldPos_z;
                vac->AddVertex(x1_pos, y1_pos, xlRED);
                vac->AddVertex(x2_pos, y2_pos, xlRED);
            }
            count = vac->getCount();
            if (count != startVertex) {
                program->addStep([=](xlGraphicsContext *ctx) {
                    ctx->drawLines(vac, startVertex, count - startVertex);
                });
                startVertex = count;
            }
        }

        // add handle for start of this vector
        float sx = mPos[i].x * scalex + worldPos_x - hw / 2;
        float sy = mPos[i].y * scaley + worldPos_y - hw / 2;
        float sz = mPos[i].z * scalez + worldPos_z - hw / 2;
        vac->AddRectAsTriangles(sx, sy, sx + hw, sy + hw, i == (selected_handle - 1) ? xlMAGENTATRANSLUCENT : (i == 0 ? xlGREENTRANSLUCENT : handleColor));
        int hpos = i + 1;
        mHandlePosition[hpos].x = sx;
        mHandlePosition[hpos].y = sy;
        mHandlePosition[hpos].z = sz;
        handle_aabb_min[hpos].x = (mPos[i].x - minX) * scalex - hw;
        handle_aabb_min[hpos].y = (mPos[i].y - minY) * scaley - hw;
        handle_aabb_min[hpos].z = (mPos[i].z - minZ) * scalez - hw;
        handle_aabb_max[hpos].x = (mPos[i].x - minX) * scalex + hw;
        handle_aabb_max[hpos].y = (mPos[i].y - minY) * scaley + hw;
        handle_aabb_max[hpos].z = (mPos[i].z - minZ) * scalez + hw;

        // add final handle
        if (i == num_points - 2) {
            sx = mPos[i + 1].x * scalex + worldPos_x - hw / 2;
            sy = mPos[i + 1].y * scaley + worldPos_y - hw / 2;
            sz = mPos[i + 1].z * scalez + worldPos_z - hw / 2;
            vac->AddRectAsTriangles(sx, sy, sx + hw, sy + hw, i + 1 == (selected_handle - 1) ? xlMAGENTATRANSLUCENT : handleColor);
            hpos++;
            mHandlePosition[hpos].x = sx;
            mHandlePosition[hpos].y = sy;
            mHandlePosition[hpos].z = sz;
            handle_aabb_min[hpos].x = (mPos[i + 1].x - minX) * scalex - hw;
            handle_aabb_min[hpos].y = (mPos[i + 1].y - minY) * scaley - hw;
            handle_aabb_min[hpos].z = (mPos[i + 1].z - minZ) * scalez - hw;
            handle_aabb_max[hpos].x = (mPos[i + 1].x - minX) * scalex + hw;
            handle_aabb_max[hpos].y = (mPos[i + 1].y - minY) * scaley + hw;
            handle_aabb_max[hpos].z = (mPos[i + 1].z - minZ) * scalez + hw;
        }
    }

    if (selected_segment != -1) {
        // add control point handles for selected segments
        int i = selected_segment;
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            float cx = mPos[i].curve->get_cp0x() * scalex + worldPos_x - hw / 2;
            float cy = mPos[i].curve->get_cp0y() * scaley + worldPos_y - hw / 2;
            float cz = mPos[i].curve->get_cp0z() * scalez + worldPos_z - hw / 2;
            vac->AddRectAsTriangles(cx, cy, cx + hw, cy + hw, xlREDTRANSLUCENT);
            mPos[i].cp0.x = mPos[i].curve->get_cp0x();
            mPos[i].cp0.y = mPos[i].curve->get_cp0y();
            mPos[i].cp0.z = mPos[i].curve->get_cp0z();
            cx = mPos[i].curve->get_cp1x() * scalex + worldPos_x - hw / 2;
            cy = mPos[i].curve->get_cp1y() * scaley + worldPos_y - hw / 2;
            cz = mPos[i].curve->get_cp1z() * scalez + worldPos_z - hw / 2;
            vac->AddRectAsTriangles(cx, cy, cx + hw, cy + hw, xlREDTRANSLUCENT);
            mPos[i].cp1.x = mPos[i].curve->get_cp1x();
            mPos[i].cp1.y = mPos[i].curve->get_cp1y();
            mPos[i].cp1.z = mPos[i].curve->get_cp1z();
        }
    }
    int count = vac->getCount();
    if (count != startVertex) {
        program->addStep([=](xlGraphicsContext *ctx) {
            ctx->drawTriangles(vac, startVertex, count - startVertex);
        });
        startVertex = count;
    }
    return true;
}

int PolyPointScreenLocation::MoveHandle3D(ModelPreview* preview, int handle, bool ShiftKeyPressed, bool CtrlKeyPressed, int mouseX, int mouseY, bool latch, bool scale_z)
{
    if (_locked) return 0;
    std::unique_lock<std::mutex> locker(_mutex);

    if (handle != CENTER_HANDLE) {

        if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
            if (latch) {
                saved_position.x = active_handle_pos.x;
                saved_position.y = active_handle_pos.y;
                saved_position.z = active_handle_pos.z;
            }

            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

            if (scalex == 0) scalex = 0.001f;
            if (scaley == 0) scaley = 0.001f;
            if (scalez == 0) scalez = 0.001f;
            if (isnan(scalex)) scalex = 1.0f;
            if (isnan(scaley)) scaley = 1.0f;
            if (isnan(scalez)) scalez = 1.0f;

            float newx = (saved_position.x + drag_delta.x - worldPos_x) / scalex;
            float newy = (saved_position.y + drag_delta.y - worldPos_y) / scaley;
            float newz = (saved_position.z + drag_delta.z - worldPos_z) / scalez;

            // check for control point handles
            if (handle & HANDLE_CP0) {
                int seg = handle & HANDLE_MASK;
                if (seg < mPos.size()) {
                    switch (active_axis) {
                    case MSLAXIS::X_AXIS:
                        mPos[seg].cp0.x = newx;
                        break;
                    case MSLAXIS::Y_AXIS:
                        mPos[seg].cp0.y = newy;
                        break;
                    case MSLAXIS::Z_AXIS:
                        mPos[seg].cp0.z = newz;
                        break;
                    default:
                        break;
                    }
                    if (mPos[seg].curve != nullptr) mPos[seg].curve->set_cp0(mPos[seg].cp0.x, mPos[seg].cp0.y, mPos[seg].cp0.z);
                }
            }
            else if (handle & HANDLE_CP1) {
                int seg = handle & HANDLE_MASK;
                if (seg < mPos.size()) {
                    switch (active_axis) {
                    case MSLAXIS::X_AXIS:
                        mPos[seg].cp1.x = newx;
                        break;
                    case MSLAXIS::Y_AXIS:
                        mPos[seg].cp1.y = newy;
                        break;
                    case MSLAXIS::Z_AXIS:
                        mPos[seg].cp1.z = newz;
                        break;
                    default:
                        break;
                    }
                    if (mPos[seg].curve != nullptr) mPos[seg].curve->set_cp1(mPos[seg].cp1.x, mPos[seg].cp1.y, mPos[seg].cp1.z);
                }
                FixCurveHandles();
            }
            else {
                int point = handle - 1;
                if (point < mPos.size()) {
                    switch (active_axis) {
                    case MSLAXIS::X_AXIS:
                        mPos[point].x = newx;
                        break;
                    case MSLAXIS::Y_AXIS:
                        mPos[point].y = newy;
                        break;
                    case MSLAXIS::Z_AXIS:
                        mPos[point].z = newz;
                        break;
                    default:
                        break;
                    }
                }
                FixCurveHandles();
            }
        } else if (axis_tool == MSLTOOL::TOOL_XY_TRANS) {
            if (latch && mHandlePosition.size() > 1) {
                saved_position.x = mHandlePosition[1].x * scalex + worldPos_x;
                saved_position.y = mHandlePosition[1].y * scaley + worldPos_y;
                saved_position.z = 0.0f;
            }

            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;

            if (scalex == 0) scalex = 0.001f;
            if (scaley == 0) scaley = 0.001f;
            if (isnan(scalex)) scalex = 1.0f;
            if (isnan(scaley)) scaley = 1.0f;

            float newx = (saved_position.x + drag_delta.x - worldPos_x) / scalex;
            float newy = (saved_position.y + drag_delta.y - worldPos_y) / scaley;
            int point = handle - 1;
            if (point < mPos.size()) {
                mPos[point].x = newx;
                mPos[point].y = newy;
            }
        }

    }
    else {
        if (axis_tool == MSLTOOL::TOOL_TRANSLATE) {
            if (latch) {
                saved_position = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            }
            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;
            switch (active_axis) {
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

            switch (active_axis) {
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
                new_angle = angle - saved_angle;
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
            rotate_pt = center;
            Rotate(active_axis, new_angle);
            saved_angle = angle;
        } else if (axis_tool == MSLTOOL::TOOL_SCALE) {
            if (latch) {
                saved_position = glm::vec3(mHandlePosition[0].x, mHandlePosition[0].y, mHandlePosition[0].z);
                saved_point = glm::vec3(worldPos_x, worldPos_y, worldPos_z);
            }
            if (!DragHandle(preview, mouseX, mouseY, latch)) return 0;
            if (saved_position.x == worldPos_x) saved_position.x += 0.001f;
            if (saved_position.y == worldPos_y) saved_position.y += 0.001f;
            if (saved_position.z == worldPos_z) saved_position.z += 0.001f;
            float change_x = (saved_position.x - worldPos_x + drag_delta.x) / (saved_position.x - worldPos_x);
            float change_y = (saved_position.y - worldPos_y + drag_delta.y) / (saved_position.y - worldPos_y);
            float change_z = (saved_position.z - worldPos_z + drag_delta.z) / (saved_position.z - worldPos_z);

            if (ShiftKeyPressed) {
                float change = 1.0f;
                switch (active_axis) {
                case MSLAXIS::X_AXIS:
                    change = change_x;
                    break;
                case MSLAXIS::Y_AXIS:
                    change = change_y;
                    break;
                case MSLAXIS::Z_AXIS:
                    change = change_z;
                    break;
                default:
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

            switch (active_axis) {
            case MSLAXIS::X_AXIS:
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
            case MSLAXIS::Y_AXIS:
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
            case MSLAXIS::Z_AXIS:
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
            default:
                break;
            }
        }
    }
    return 1;
}
int PolyPointScreenLocation::MoveHandle3D(float scale, int handle, glm::vec3 &rot, glm::vec3 &mov) {
    if (handle == CENTER_HANDLE) {
        constexpr float rscale = 10; //10 degrees per full 1.0 aka: max speed
        Rotate(ModelScreenLocation::MSLAXIS::X_AXIS, rot.x * rscale);
        Rotate(ModelScreenLocation::MSLAXIS::Y_AXIS, -rot.z * rscale);
        Rotate(ModelScreenLocation::MSLAXIS::Z_AXIS, rot.y * rscale);
        AddOffset(mov.x * scale, -mov.z * scale, mov.y * scale);
    } else {
        if (handle & HANDLE_CP0) {
            int seg = handle & HANDLE_MASK;
            if (seg < mPos.size()) {
                mPos[seg].cp0.x += mov.x * scale;
                mPos[seg].cp0.y -= mov.z * scale;
                mPos[seg].cp0.z += mov.y * scale;
                if (mPos[seg].curve != nullptr) mPos[seg].curve->set_cp0(mPos[seg].cp0.x, mPos[seg].cp0.y, mPos[seg].cp0.z);
                FixCurveHandles();
            }
        } else if (handle & HANDLE_CP1) {
            int seg = handle & HANDLE_MASK;
            if (seg < mPos.size()) {
                mPos[seg].cp1.x += mov.x * scale;
                mPos[seg].cp1.y -= mov.z * scale;
                mPos[seg].cp1.z += mov.y * scale;
                if (mPos[seg].curve != nullptr) mPos[seg].curve->set_cp1(mPos[seg].cp1.x, mPos[seg].cp1.y, mPos[seg].cp1.z);
                FixCurveHandles();
            }
        } else {
            int point = handle - 1;
            if (point < mPos.size()) {
                mPos[point].x += mov.x * scale;
                mPos[point].y -= mov.z * scale;
                mPos[point].z += mov.y * scale;
                FixCurveHandles();
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
    float hw = GetRectHandleWidth(zoom, scale);

    VectorMath::ScreenPosToWorldRay(
        mouseX, preview->getHeight() - mouseY,
        preview->getWidth(), preview->getHeight(),
        preview->GetProjViewMatrix(),
        ray_origin,
        ray_direction
    );

    if (scalex == 0) scalex = 0.001f;
    if (scaley == 0) scaley = 0.001f;
    if (isnan(scalex)) scalex = 1.0f;
    if (isnan(scaley)) scaley = 1.0f;

    float newx = (ray_origin.x - worldPos_x) / scalex;
    float newy = (ray_origin.y - worldPos_y) / scaley;
    float newz = 0.0f;

    // check for control point handles
    if( handle & HANDLE_CP0) {
        int seg = handle & HANDLE_MASK;
        mPos[seg].cp0.x = ray_origin.x - hw / 2;
        mPos[seg].cp0.y = ray_origin.y - hw / 2;
        mPos[seg].cp0.z = newz;
        if (mPos[seg].curve != nullptr) mPos[seg].curve->set_cp0( newx, newy, newz );
    } else if( handle & HANDLE_CP1) {
        int seg = handle & HANDLE_MASK;
        mPos[seg].cp1.x = ray_origin.x - hw / 2;
        mPos[seg].cp1.y = ray_origin.y - hw / 2;
        mPos[seg].cp1.z = newz;
        if (mPos[seg].curve != nullptr) mPos[seg].curve->set_cp1( newx, newy, newz );

    // check normal handles
    } else if( handle < num_points+1 ) {
        mPos[handle-1].x = newx;
        mPos[handle-1].y = newy;
        FixCurveHandles();
    } else {
        float boundary_offset = 2.0f * hw;
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
    if( handle != -1 && handle < HANDLE_CP0) {
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
    float hw = GetRectHandleWidth(zoom, scale);

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
    float sx = new_point.x * scalex + worldPos_x - hw / 2;
    float sy = new_point.y * scaley + worldPos_y - hw / 2;
    float sz = new_point.z * scalez + worldPos_z - hw / 2;
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

    handle_aabb_min[num_points].x = sx - hw;
    handle_aabb_min[num_points].y = sy - hw;
    handle_aabb_min[num_points].z = sz - hw;
    handle_aabb_max[num_points].x = sx + hw;
    handle_aabb_max[num_points].y = sy + hw;
    handle_aabb_max[num_points].z = sz + hw;
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
    float hw = GetRectHandleWidth(zoom, scale);
    float sx = mPos[pos+1].x * scalex + worldPos_x - hw / 2;
    float sy = mPos[pos+1].y * scaley + worldPos_y - hw / 2;
    float sz = mPos[pos+1].z * scalez + worldPos_z - hw / 2;
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

    handle_aabb_min[num_points].x = sx - hw;
    handle_aabb_min[num_points].y = sy - hw;
    handle_aabb_min[num_points].z = sz - hw;
    handle_aabb_max[num_points].x = sx + hw;
    handle_aabb_max[num_points].y = sy + hw;
    handle_aabb_max[num_points].z = sz + hw;
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
        active_axis = MSLAXIS::X_AXIS;
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

    float hw = GetRectHandleWidth(zoom, scale);
    handle_aabb_min[0].x = -hw;
    handle_aabb_min[0].y = -hw;
    handle_aabb_min[0].z = -hw;
    handle_aabb_max[0].x = hw;
    handle_aabb_max[0].y = hw;
    handle_aabb_max[0].z = hw;
    handle_aabb_min[1].x = -hw;
    handle_aabb_min[1].y = -hw;
    handle_aabb_min[1].z = -hw;
    handle_aabb_max[1].x = hw;
    handle_aabb_max[1].y = hw;
    handle_aabb_max[1].z = hw;
    handle_aabb_min[2].x = -hw;
    handle_aabb_min[2].y = -hw;
    handle_aabb_min[2].z = -hw;
    handle_aabb_max[2].x = hw;
    handle_aabb_max[2].y = hw;
    handle_aabb_max[2].z = hw;

    handle = 2;
    return wxCURSOR_SIZING;
}

void PolyPointScreenLocation::AddDimensionProperties(wxPropertyGridInterface* propertyEditor, float factor) const
{
    float len = 0;
    auto last = mPos[0].AsVector();
    for (int i = 1; i < mPos.size(); i++) {
        len += RulerObject::Measure(last, mPos[i].AsVector());
        last = mPos[i].AsVector();
    }
    wxPGProperty* prop = propertyEditor->Append(new wxFloatProperty(wxString::Format("Length (%s)", RulerObject::GetUnitDescription()), "RealLength", len));
    prop->ChangeFlag(wxPG_PROP_READONLY, true);
    prop->SetAttribute("Precision", 2);
    prop->SetTextColour(wxSystemSettings::GetColour(wxSYS_COLOUR_GRAYTEXT));
    
    last = mPos[0].AsVector();
    for (int i = 1; i < mPos.size(); i++) {
        len = RulerObject::Measure(last, mPos[i].AsVector());
        last = mPos[i].AsVector();

        auto seg = wxString::Format("Segment %d (%s)", i, RulerObject::GetUnitDescription());
        prop = propertyEditor->Append(new wxFloatProperty(seg, "REAL" + seg, len));
        prop->SetAttribute("Precision", 2);
    }
}

std::string PolyPointScreenLocation::GetDimension(float factor) const
{
    if (RulerObject::GetRuler() == nullptr) return "";
    float len = 0;
    auto last = mPos[0].AsVector();
    for (int i = 1; i < mPos.size(); i++) {
        len += RulerObject::Measure(last, mPos[i].AsVector());
        last = mPos[i].AsVector();
    }
    return wxString::Format("Length %s", RulerObject::PrescaledMeasureDescription(len)).ToStdString();
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
    if (StartsWith(name, "REALSegment ")) {
        if (_locked)
        {
            event.Veto();
            return 0;
        }
        else {
            auto o = name.find(" ", 12);
            wxASSERT(o != std::string::npos);

            selected_handle = wxAtoi(name.substr(12, o - 12)) - 1;

            wxASSERT(selected_handle + 1 < mPos.size());

            float oldLen = 0.0f;
            oldLen = RulerObject::UnMeasure(RulerObject::Measure(mPos[selected_handle].AsVector(), mPos[selected_handle + 1].AsVector()));
            float len = RulerObject::UnMeasure(event.GetValue().GetDouble());

            float dx = (mPos[selected_handle + 1].x - mPos[selected_handle].x) * len / oldLen - (mPos[selected_handle + 1].x - mPos[selected_handle].x);
            float dy = (mPos[selected_handle + 1].y - mPos[selected_handle].y) * len / oldLen - (mPos[selected_handle + 1].y - mPos[selected_handle].y);
            float dz = (mPos[selected_handle + 1].z - mPos[selected_handle].z) * len / oldLen - (mPos[selected_handle + 1].z - mPos[selected_handle].z);

            // if this resulted in a divide by zero then set it to one ... setting it to zero leaves you stuck unable to change it further ... this will be weird but fixable
            if (isnan(dx))
                dx = 1.0f;
            if (isnan(dy))
                dy = 1.0f;
            if (isnan(dz))
                dz = 1.0f;

            for (auto i = selected_handle + 1; i < mPos.size(); i++) {
                    mPos[i].x += dx;
                    mPos[i].y += dy;
                    mPos[i].z += dz;
                }

            AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "PolyPointScreenLocation::OnPropertyGridChange::REALSegment");
            AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "PolyPointScreenLocation::OnPropertyGridChange::REALSegment");
            AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "PolyPointScreenLocation::OnPropertyGridChange::REALSegment");
            AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "PolyPointScreenLocation::OnPropertyGridChange::REALSegment");
            return 0;
        }
    }
    else if( name.length() > 6 ) {
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
         Rotate(MSLAXIS::Y_AXIS, offset);
    }
    else if (angle.x != 0.0f) {
        float offset = angle.x;
        Rotate(MSLAXIS::X_AXIS, offset);
    }
    else if (angle.z != 0.0f) {
        float offset = angle.z;
        Rotate(MSLAXIS::Z_AXIS, offset);
    }
}

bool PolyPointScreenLocation::Rotate(MSLAXIS axis, float factor)
{
    if (_locked) return false;

    // Rotate all the points
    glm::mat4 translateToOrigin = glm::translate(Identity, -rotate_pt);
    glm::mat4 translateBack = glm::translate(Identity, rotate_pt);
    glm::mat4 Rotate = Identity;
    glm::vec3 pt(worldPos_x, worldPos_y, worldPos_z);

    switch (axis)
    {
    case MSLAXIS::X_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(1.0f, 0.0f, 0.0f));
    }
    break;
    case MSLAXIS::Y_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 1.0f, 0.0f));
    }
    break;
    case MSLAXIS::Z_AXIS:
    {
        Rotate = glm::rotate(Identity, glm::radians(factor), glm::vec3(0.0f, 0.0f, 1.0f));
    }
    break;
    default:
        break;
    }

    pt = glm::vec3(translateBack * Rotate * translateToOrigin * glm::vec4(pt, 1.0f));
    glm::vec3 world_new(pt.x, pt.y, pt.z);
    for (int i = 0; i < num_points; ++i) {
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
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
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
            mPos[i].curve->UpdateBoundingBox(draw_3d);
        }
        else {
            // create normal line segment bounding boxes
            seg_aabb_min[i] = glm::vec3(0.0f, -BB_OFF, -BB_OFF);
            seg_aabb_max[i] = glm::vec3(RenderWi * mPos[i].seg_scale, BB_OFF, BB_OFF);
        }
    }
}

glm::vec2 PolyPointScreenLocation::GetScreenOffset(ModelPreview* preview) const
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
        if( mPos[i].has_curve && mPos[i].curve != nullptr) {
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
        if (mPos[i].has_curve && mPos[i].curve != nullptr) {
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
