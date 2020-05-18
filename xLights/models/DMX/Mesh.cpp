/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>
#include <wx/sstream.h>

#include <algorithm>

#include "Mesh.h"
#include "Servo.h"
#include "../../UtilFunctions.h"
#include "../../ModelPreview.h"
#include "../../xLightsMain.h"
#include <log4cpp/Category.hh>

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

Mesh::Mesh(wxXmlNode* node, wxString _name)
 : node_xml(node), _objFile(""), base_name(_name)
{
}

Mesh::~Mesh()
{
    for (const auto& it : textures) {
        if (it.second != nullptr) {
            delete it.second;
        }
    }
    if (mesh3d) {
        delete mesh3d;
    }
}

void Mesh::SetRenderScaling(float s) {
    rscale = s;
}

void Mesh::Init(BaseObject* base, bool set_size) {
	_objFile = FixFile("", node_xml->GetAttribute("ObjFile", ""));
    mesh_only = node_xml->GetAttribute("MeshOnly", "0") == "1";

    brightness = wxAtoi(node_xml->GetAttribute("Brightness", "100"));
    if (brightness > 100) {
        brightness = 100;
    } else if (brightness < 0) {
        brightness = 0;
    }

    offset_x = wxAtof(node_xml->GetAttribute("OffsetX", "0.0"));
    offset_y = wxAtof(node_xml->GetAttribute("OffsetY", "0.0"));
    offset_z = wxAtof(node_xml->GetAttribute("OffsetZ", "0.0"));

    scalex = wxAtof(node_xml->GetAttribute("ScaleX", "1.0"));
    scaley = wxAtof(node_xml->GetAttribute("ScaleY", "1.0"));
    scalez = wxAtof(node_xml->GetAttribute("ScaleZ", "1.0"));

    if (scalex < 0) {
        scalex = 1.0f;
    }
    if (scaley < 0) {
        scaley = 1.0f;
    }
    if (scalez < 0) {
        scalez = 1.0f;
    }

    rotatex = wxAtof(node_xml->GetAttribute("RotateX", "0.0f"));
    rotatey = wxAtof(node_xml->GetAttribute("RotateY", "0.0f"));
    rotatez = wxAtof(node_xml->GetAttribute("RotateZ", "0.0f"));

    if (rotatex < -180.0f || rotatex > 180.0f) {
        rotatex = 0.0f;
    }
    if (rotatey < -180.0f || rotatey > 180.0f) {
        rotatey = 0.0f;
    }
    if (rotatez < -180.0f || rotatez > 180.0f) {
        rotatez = 0.0f;
    }

    controls_size = set_size;
    if (controls_size) {
        width = wxAtof(node_xml->GetAttribute("Width", "1.0f"));
        height = wxAtof(node_xml->GetAttribute("Height", "1.0f"));
        depth = wxAtof(node_xml->GetAttribute("Depth", "1.0f"));
        base->GetBaseObjectScreenLocation().SetRenderSize(width * scalex, height * scaley, depth * scalez);
    }
}

void Mesh::AddTypeProperties(wxPropertyGridInterface *grid) {
    grid->Append(new wxPropertyCategory(base_name, base_name + "Properties"));
    
    wxPGProperty * prop = grid->Append(new wxFileProperty("ObjFile", base_name + "ObjFile", _objFile));
    prop->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    prop = grid->Append(new wxBoolProperty("Mesh Only", base_name + "MeshOnly", mesh_only));
    prop->SetAttribute("UseCheckbox", true);
    prop = grid->Append(new wxUIntProperty("Brightness", base_name + "Brightness", brightness));
    prop->SetAttribute("Min", 0);
    prop->SetAttribute("Max", 100);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset X", base_name + "OffsetX", offset_x));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Y", base_name + "OffsetY", offset_y));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("Offset Z", base_name + "OffsetZ", offset_z));
    prop->SetAttribute("Precision", 2);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleX", base_name + "ScaleX", scalex));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleY", base_name + "ScaleY", scaley));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("ScaleZ", base_name + "ScaleZ", scalez));
    prop->SetAttribute("Precision", 3);
    prop->SetAttribute("Step", 0.1);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateX", base_name + "RotateX", rotatex));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateY", base_name + "RotateY", rotatey));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");
    prop = grid->Append(new wxFloatProperty("RotateZ", base_name + "RotateZ", rotatez));
    prop->SetAttribute("Min", "-180");
    prop->SetAttribute("Max", "180");
    prop->SetAttribute("Precision", 8);
    prop->SetAttribute("Step", 1.0);
    prop->SetEditor("SpinCtrl");

    grid->Collapse(base_name + "Properties");
}

int Mesh::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked) {
    std::string name = event.GetPropertyName().ToStdString();
    if (!locked && base_name + "ObjFile" == name) {
        obj_loaded = false;
        obj_exists = false;
        for (auto it = textures.begin(); it != textures.end(); ++it) {
            if (it->second != nullptr) {
                delete it->second;
            }
        }
        if (controls_size) {
            node_xml->DeleteAttribute("Width");
            node_xml->DeleteAttribute("Height");
            node_xml->DeleteAttribute("Depth");
        }
        textures.clear();
        uncacheDisplayObjects();
        _objFile = event.GetValue().GetString();
        node_xml->DeleteAttribute("ObjFile");
        node_xml->AddAttribute("ObjFile", _objFile);
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ObjFile");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ObjFile");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ObjFile");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ObjFile");
        return 0;
    }
    else if (locked && base_name + "ObjFile" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "Brightness" == name) {
        brightness = (int)event.GetPropertyValue().GetLong();
        node_xml->DeleteAttribute("Brightness");
        node_xml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::Brightness");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::Brightness");
        return 0;
    }
    else if (locked && base_name + "Brightness" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "MeshOnly" == name) {
        node_xml->DeleteAttribute("MeshOnly");
        mesh_only = event.GetValue().GetBool();
        if (mesh_only) {
            node_xml->AddAttribute("MeshOnly", "1");
        }
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::MeshOnly");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::MeshOnly");
        return 0;
    }
    else if (locked && base_name + "MeshOnly" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleX" == name) {
        scalex = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleX");
        node_xml->AddAttribute("ScaleX", wxString::Format("%6.4f", scalex));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ScaleX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ScaleX");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleY" == name) {
        scaley = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleY");
        node_xml->AddAttribute("ScaleY", wxString::Format("%6.4f", scaley));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ScaleY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ScaleY");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "ScaleZ" == name) {
        scalez = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("ScaleZ");
        node_xml->AddAttribute("ScaleZ", wxString::Format("%6.4f", scalez));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ScaleZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ScaleZ");
        return GRIDCHANGE_SUPPRESS_HOLDSIZE;
    }
    else if (locked && base_name + "ScaleZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetX" == name) {
        offset_x = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetX");
        node_xml->AddAttribute("OffsetX", wxString::Format("%6.4f", offset_x));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ModelX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ModelX");
        return 0;
    }
    else if (locked && base_name + "OffsetX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetY" == name) {
        offset_y = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetY");
        node_xml->AddAttribute("OffsetY", wxString::Format("%6.4f", offset_y));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ModelY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ModelY");
        return 0;
    }
    else if (locked && base_name + "OffsetY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "OffsetZ" == name) {
        offset_z = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("OffsetZ");
        node_xml->AddAttribute("OffsetZ", wxString::Format("%6.4f", offset_z));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::ModelZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::ModelZ");
        return 0;
    }
    else if (locked && base_name + "OffsetZ" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateX" == name) {
        rotatex = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateX");
        node_xml->AddAttribute("RotateX", wxString::Format("%4.8f", rotatex));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::RotateX");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::RotateX");
        return 0;
    }
    else if (locked && base_name + "RotateX" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateY" == name) {
        rotatey = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateY");
        node_xml->AddAttribute("RotateY", wxString::Format("%4.8f", rotatey));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::RotateY");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::RotateY");
        return 0;
    }
    else if (locked && base_name + "RotateY" == name) {
        event.Veto();
        return 0;
    }
    else if (!locked && base_name + "RotateZ" == name) {
        rotatez = event.GetValue().GetDouble();
        node_xml->DeleteAttribute("RotateZ");
        node_xml->AddAttribute("RotateZ", wxString::Format("%4.8f", rotatez));
        base->AddASAPWork(OutputModelManager::WORK_RGBEFFECTS_CHANGE, "Mesh::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_MODELS_CHANGE_REQUIRING_RERENDER, "Mesh::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_REDRAW_LAYOUTPREVIEW, "Mesh::OnPropertyGridChange::RotateZ");
        base->AddASAPWork(OutputModelManager::WORK_RELOAD_PROPERTYGRID, "Mesh::OnPropertyGridChange::RotateZ");
        return 0;
    }
    else if (locked && base_name + "RotateZ" == name) {
        event.Veto();
        return 0;
    }

    return -1;
}

static void CalcNormal(float N[3], float v0[3], float v1[3], float v2[3]) {
    float v10[3];
    v10[0] = v1[0] - v0[0];
    v10[1] = v1[1] - v0[1];
    v10[2] = v1[2] - v0[2];

    float v20[3];
    v20[0] = v2[0] - v0[0];
    v20[1] = v2[1] - v0[1];
    v20[2] = v2[2] - v0[2];

    N[0] = v20[1] * v10[2] - v20[2] * v10[1];
    N[1] = v20[2] * v10[0] - v20[0] * v10[2];
    N[2] = v20[0] * v10[1] - v20[1] * v10[0];

    float len2 = N[0] * N[0] + N[1] * N[1] + N[2] * N[2];
    if (len2 > 0.0f) {
        float len = sqrtf(len2);

        N[0] /= len;
        N[1] /= len;
        N[2] /= len;
    }
}

namespace  // Local utility functions
{
    struct vec3 {
        float v[3];
        vec3() {
            v[0] = 0.0f;
            v[1] = 0.0f;
            v[2] = 0.0f;
        }
    };

    void normalizeVector(vec3 &v) {
        float len2 = v.v[0] * v.v[0] + v.v[1] * v.v[1] + v.v[2] * v.v[2];
        if (len2 > 0.0f) {
            float len = sqrtf(len2);

            v.v[0] /= len;
            v.v[1] /= len;
            v.v[2] /= len;
        }
    }

    // Check if `mesh_t` contains smoothing group id.
    bool hasSmoothingGroup(const tinyobj::shape_t& shape)
    {
        for (size_t i = 0; i < shape.mesh.smoothing_group_ids.size(); i++) {
            if (shape.mesh.smoothing_group_ids[i] > 0) {
                return true;
            }
        }
        return false;
    }

    void computeSmoothingNormals(const tinyobj::attrib_t& attrib, const tinyobj::shape_t& shape,
        std::map<int, vec3>& smoothVertexNormals) {
        smoothVertexNormals.clear();
        std::map<int, vec3>::iterator iter;

        for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
            // Get the three indexes of the face (all faces are triangular)
            tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
            tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
            tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

            // Get the three vertex indexes and coordinates
            int vi[3];      // indexes
            float v[3][3];  // coordinates

            for (int k = 0; k < 3; k++) {
                vi[0] = idx0.vertex_index;
                vi[1] = idx1.vertex_index;
                vi[2] = idx2.vertex_index;
                assert(vi[0] >= 0);
                assert(vi[1] >= 0);
                assert(vi[2] >= 0);

                v[0][k] = attrib.vertices[3 * vi[0] + k];
                v[1][k] = attrib.vertices[3 * vi[1] + k];
                v[2][k] = attrib.vertices[3 * vi[2] + k];
            }

            // Compute the normal of the face
            float normal[3];
            CalcNormal(normal, v[0], v[1], v[2]);

            // Add the normal to the three vertexes
            for (size_t i = 0; i < 3; ++i) {
                iter = smoothVertexNormals.find(vi[i]);
                if (iter != smoothVertexNormals.end()) {
                    // add
                    iter->second.v[0] += normal[0];
                    iter->second.v[1] += normal[1];
                    iter->second.v[2] += normal[2];
                }
                else {
                    smoothVertexNormals[vi[i]].v[0] = normal[0];
                    smoothVertexNormals[vi[i]].v[1] = normal[1];
                    smoothVertexNormals[vi[i]].v[2] = normal[2];
                }
            }

        }  // f

           // Normalize the normals, that is, make them unit vectors
        for (iter = smoothVertexNormals.begin(); iter != smoothVertexNormals.end();
            iter++) {
            normalizeVector(iter->second);
        }

    }  // computeSmoothingNormals
}  // namespace

void Mesh::uncacheDisplayObjects() {
    if (mesh3d) {
        delete mesh3d;
        mesh3d = nullptr;
    }
}

void Mesh::loadObject(BaseObject* base) {
    if (wxFileExists(_objFile)) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));
        obj_exists = true;

        logger_base.debug("Loading mesh model file %s.",
                          (const char *)_objFile.c_str());
        wxFileName fn(_objFile);
        std::string base_path = fn.GetPath();
        std::string err;
        tinyobj::LoadObj(&attrib, &shapes, &lines, &materials, &err, (char *)_objFile.c_str(), (char *)base_path.c_str());
        logger_base.debug("    Loaded.");

        // Append `default` material
        materials.push_back(tinyobj::material_t());
        
        bmin[0] = bmin[1] = bmin[2] = std::numeric_limits<float>::max();
        bmax[0] = bmax[1] = bmax[2] = -std::numeric_limits<float>::max();
        
        for (auto shape : shapes) {
            // Loop over faces(polygon)
            
            for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
                tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
                tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
                tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];
                
                float v[3][3];
                for (int k = 0; k < 3; k++) {
                    int f0 = idx0.vertex_index;
                    int f1 = idx1.vertex_index;
                    int f2 = idx2.vertex_index;
                    assert(f0 >= 0);
                    assert(f1 >= 0);
                    assert(f2 >= 0);
                    
                    v[0][k] = attrib.vertices[3 * f0 + k];
                    v[1][k] = attrib.vertices[3 * f1 + k];
                    v[2][k] = attrib.vertices[3 * f2 + k];
                    bmin[k] = std::min(v[0][k], bmin[k]);
                    bmin[k] = std::min(v[1][k], bmin[k]);
                    bmin[k] = std::min(v[2][k], bmin[k]);
                    bmax[k] = std::max(v[0][k], bmax[k]);
                    bmax[k] = std::max(v[1][k], bmax[k]);
                    bmax[k] = std::max(v[2][k], bmax[k]);
                }
            }
        }
        width = std::max(std::abs(bmin[0]), bmax[0]) * 2.0f;
        height = std::max(std::abs(bmin[1]), bmax[1]) * 2.0f;
        depth = std::max(std::abs(bmin[2]), bmax[2]) * 2.0f;
        obj_loaded = true;
        
        // Load textures
        for (auto m : materials) {
            if (m.diffuse_texname.length() > 0) {
                // Only load the texture if it is not already loaded
                if (textures.find(m.diffuse_texname) == textures.end()) {
                    std::string texture_filename = m.diffuse_texname;
                    if (!wxFileExists(texture_filename)) {
                        // Append base dir.
                        wxFileName fn2(texture_filename);
                        fn2.SetPath(fn.GetPath());
                        texture_filename = fn2.GetFullPath();
                        if (!wxFileExists(texture_filename)) {
                            texture_filename = fn.GetPath() + "/" + m.diffuse_texname;
                            if (!wxFileExists(texture_filename)) {
                                logger_base.debug("Unable to find materials file: %s", (const char *)m.diffuse_texname.c_str());
                                continue;
                            }
                        }
                    }
                    textures[m.diffuse_texname] = new Image(texture_filename, false, true);
                }
            }
        }
    }
}

void Mesh::Draw(BaseObject* base, ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, glm::mat4& base_matrix, glm::mat4& motion_matrix,
                bool show_empty, float pivot_offset_x, float pivot_offset_y, float pivot_offset_z, bool rotation, bool use_pivot)
{
    if (!obj_loaded) {
        loadObject(base);
        base->GetBaseObjectScreenLocation().SetSupportsZScaling(true);  // set here instead of constructor so model creation doesn't go up like a tree
    }

    if (obj_loaded) {
        glm::mat4 Identity = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(Identity, glm::vec3(scalex * rscale, scaley * rscale, scalez * rscale));
        glm::mat4 rx = glm::rotate(Identity, glm::radians(rotatex), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 ry = glm::rotate(Identity, glm::radians(rotatey), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rz = glm::rotate(Identity, glm::radians(rotatez), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::quat rotate_quat = glm::quat_cast(rx * ry * rz);
        glm::mat4 translationMatrix = glm::translate(Identity, glm::vec3(offset_x, offset_y, offset_z));
        glm::mat4 m;
        if (rotation) {
            glm::mat4 pivotToZero = glm::translate(Identity, glm::vec3(-pivot_offset_x, -pivot_offset_y, -pivot_offset_z));
            glm::mat4 pivotBack = glm::translate(Identity, glm::vec3(pivot_offset_x, pivot_offset_y, pivot_offset_z));
            m = base_matrix * translationMatrix * pivotBack * motion_matrix * pivotToZero * glm::toMat4(rotate_quat) * scalingMatrix;
        }
        else
        {
            m = base_matrix * translationMatrix * motion_matrix * glm::toMat4(rotate_quat) * scalingMatrix;
        }

        if (controls_size) {
            if (!node_xml->HasAttribute("Width")) {
                base->GetBaseObjectScreenLocation().AdjustRenderSize(width * scalex, height * scaley * half_height, depth * scalez, base->GetModelXml());
                node_xml->AddAttribute("Width", std::to_string(width));
                node_xml->AddAttribute("Height", std::to_string(height));
                node_xml->AddAttribute("Depth", std::to_string(depth));
            }
            base->GetBaseObjectScreenLocation().UpdateBoundingBox(width, height, depth);  // FIXME: Modify to only call this when position changes
        }

        // skip updating color if it hasn't changed.
        if (update_color) {
            if (new_color != last_color) {
                last_color = new_color;
            }
            else {
                update_color = false;
            }
        }
        if (!mesh3d || update_color) {
            if (update_color) {
                textures.clear();
                uncacheDisplayObjects();
            }
            mesh3d = DrawGLUtils::createMesh();
            // Loop over shapes
            for (auto shape : shapes) {
                // Loop over faces(polygon)

                // Check for smoothing group and compute smoothing normals
                std::map<int, vec3> smoothVertexNormals;
                if (hasSmoothingGroup(shape)) {
                    computeSmoothingNormals(attrib, shape, smoothVertexNormals);
                }

                int last_material_id = -1;
                GLuint image_id = 0;
                for (size_t f = 0; f < shape.mesh.indices.size() / 3; f++) {
                    tinyobj::index_t idx0 = shape.mesh.indices[3 * f + 0];
                    tinyobj::index_t idx1 = shape.mesh.indices[3 * f + 1];
                    tinyobj::index_t idx2 = shape.mesh.indices[3 * f + 2];

                    int current_material_id = shape.mesh.material_ids[f];

                    if ((current_material_id < 0) ||
                        (current_material_id >= static_cast<int>(materials.size()))) {
                        // Invalid material ID. Use default material.
                        current_material_id = materials.size() - 1;  // Default material is added to the last item in `materials`.
                    }

                    if (current_material_id != last_material_id) {
                        std::string diffuse_texname = materials[current_material_id].diffuse_texname;
                        if (textures.find(diffuse_texname) != textures.end()) {
                            image_id = textures[diffuse_texname]->getID();
                        }
                        else {
                            image_id = -1;
                        }
                    }
                    last_material_id = current_material_id;

                    float diffuse[3];
                    if (update_color && materials[current_material_id].name == color_name) {
                        diffuse[0] = new_color.red / 255.0f;
                        diffuse[1] = new_color.green / 255.0f;
                        diffuse[2] = new_color.blue / 255.0f;
                    } else {
                        for (size_t i = 0; i < 3; i++) {
                            diffuse[i] = materials[current_material_id].diffuse[i];
                        }
                    }

                    float tc[3][2];
                    if (attrib.texcoords.size() > 0) {
                        if ((idx0.texcoord_index < 0) || (idx1.texcoord_index < 0) ||
                            (idx2.texcoord_index < 0)) {
                            // face does not contain valid uv index.
                            tc[0][0] = 0.0f;
                            tc[0][1] = 0.0f;
                            tc[1][0] = 0.0f;
                            tc[1][1] = 0.0f;
                            tc[2][0] = 0.0f;
                            tc[2][1] = 0.0f;
                        }
                        else {
                            assert(attrib.texcoords.size() >
                                size_t(2 * idx0.texcoord_index + 1));
                            assert(attrib.texcoords.size() >
                                size_t(2 * idx1.texcoord_index + 1));
                            assert(attrib.texcoords.size() >
                                size_t(2 * idx2.texcoord_index + 1));

                            tc[0][0] = attrib.texcoords[2 * idx0.texcoord_index];
                            tc[0][1] = attrib.texcoords[2 * idx0.texcoord_index + 1];
                            tc[1][0] = attrib.texcoords[2 * idx1.texcoord_index];
                            tc[1][1] = attrib.texcoords[2 * idx1.texcoord_index + 1];
                            tc[2][0] = attrib.texcoords[2 * idx2.texcoord_index];
                            tc[2][1] = attrib.texcoords[2 * idx2.texcoord_index + 1];

                            //tc[0][1] = 1.0f - attrib.texcoords[2 * idx0.texcoord_index + 1];
                            //tc[1][1] = 1.0f - attrib.texcoords[2 * idx1.texcoord_index + 1];
                            //tc[2][1] = 1.0f - attrib.texcoords[2 * idx2.texcoord_index + 1];
                        }
                    }
                    else {
                        tc[0][0] = 0.0f;
                        tc[0][1] = 0.0f;
                        tc[1][0] = 0.0f;
                        tc[1][1] = 0.0f;
                        tc[2][0] = 0.0f;
                        tc[2][1] = 0.0f;
                    }

                    float v[3][3];
                    for (int k = 0; k < 3; k++) {
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;

                        v[0][k] = attrib.vertices[3 * f0 + k];
                        v[1][k] = attrib.vertices[3 * f1 + k];
                        v[2][k] = attrib.vertices[3 * f2 + k];
                    }

                    float n[3][3];
                    bool invalid_normal_index = false;
                    if (attrib.normals.size() > 0) {
                        int nf0 = idx0.normal_index;
                        int nf1 = idx1.normal_index;
                        int nf2 = idx2.normal_index;

                        if ((nf0 < 0) || (nf1 < 0) || (nf2 < 0)) {
                            // normal index is missing from this face.
                            invalid_normal_index = true;
                        }
                        else {
                            for (int k = 0; k < 3; k++) {
                                assert(size_t(3 * nf0 + k) < attrib.normals.size());
                                assert(size_t(3 * nf1 + k) < attrib.normals.size());
                                assert(size_t(3 * nf2 + k) < attrib.normals.size());
                                n[0][k] = attrib.normals[3 * nf0 + k];
                                n[1][k] = attrib.normals[3 * nf1 + k];
                                n[2][k] = attrib.normals[3 * nf2 + k];
                            }
                        }
                    }
                    else {
                        invalid_normal_index = true;
                    }

                    if (invalid_normal_index && !smoothVertexNormals.empty()) {
                        // Use smoothing normals
                        int f0 = idx0.vertex_index;
                        int f1 = idx1.vertex_index;
                        int f2 = idx2.vertex_index;

                        if (f0 >= 0 && f1 >= 0 && f2 >= 0) {
                            n[0][0] = smoothVertexNormals[f0].v[0];
                            n[0][1] = smoothVertexNormals[f0].v[1];
                            n[0][2] = smoothVertexNormals[f0].v[2];

                            n[1][0] = smoothVertexNormals[f1].v[0];
                            n[1][1] = smoothVertexNormals[f1].v[1];
                            n[1][2] = smoothVertexNormals[f1].v[2];

                            n[2][0] = smoothVertexNormals[f2].v[0];
                            n[2][1] = smoothVertexNormals[f2].v[1];
                            n[2][2] = smoothVertexNormals[f2].v[2];

                            invalid_normal_index = false;
                        }
                    }

                    if (invalid_normal_index) {
                        // compute geometric normal
                        CalcNormal(n[0], v[0], v[1], v[2]);
                        n[1][0] = n[0][0];
                        n[1][1] = n[0][1];
                        n[1][2] = n[0][2];
                        n[2][0] = n[0][0];
                        n[2][1] = n[0][1];
                        n[2][2] = n[0][2];
                    }

                    uint8_t colors[3][4];
                    for (int k = 0; k < 3; k++) {

                        // Combine normal and diffuse to get color.
                        float normal_factor = 0.2f;
                        float diffuse_factor = 1 - normal_factor;
                        float c[3] = { n[k][0] * normal_factor + diffuse[0] * diffuse_factor,
                            n[k][1] * normal_factor + diffuse[1] * diffuse_factor,
                            n[k][2] * normal_factor + diffuse[2] * diffuse_factor };
                        float len2 = c[0] * c[0] + c[1] * c[1] + c[2] * c[2];
                        if (len2 > 0.0f) {
                            float len = sqrtf(len2);

                            c[0] /= len;
                            c[1] /= len;
                            c[2] /= len;
                        }
                        float red = diffuse[0];
                        float green = diffuse[1];
                        float blue = diffuse[2];

                        float trans = materials[current_material_id].dissolve * 255.0f;
                        xlColor color(red * 255, green * 255, blue * 255, trans);
                        colors[k][0] = color.red;
                        colors[k][1] = color.green;
                        colors[k][2] = color.blue;
                        colors[k][3] = trans;
                    }
                    mesh3d->addSurface(v, tc, n, colors, image_id);
                }
            }

            // process any edge lines
            if (lines.size() > 0 && attrib.vertices.size() > 0) {
                for (size_t l = 0; l < lines.size() / 2; l++) {
                    float v[2][3];
                    for (int k = 0; k < 3; k++) {
                        int f0 = lines[l * 2 + 0];
                        int f1 = lines[l * 2 + 1];

                        v[0][k] = attrib.vertices[3 * f0 + k];
                        v[1][k] = attrib.vertices[3 * f1 + k];
                    }
                    mesh3d->addLine(v);
                }
            }
            update_color = false;
        }
        
        mesh3d->setMatrix(m);
        va.AddMesh(mesh3d, mesh_only, brightness, false);

        // draw the pivot location
        if (use_pivot && rotation) {
            xlColor pink = xlColor(255, 0, 255);
            glm::vec3 scale = base->GetBaseObjectScreenLocation().GetScaleMatrix();
            float x1 = pivot_offset_x;
            float y1 = pivot_offset_y;
            float z1 = pivot_offset_z;
            float offx = width * scalex * scale.x * rscale;
            float offz = depth * scalez * scale.z * rscale;
            glm::vec4 v = base_matrix * translationMatrix * glm::vec4(glm::vec3(x1, y1, z1), 1.0f);
            x1 = v.x; y1 = v.y; z1 = v.z;
            va.AddVertex(x1 - offx, y1, z1, pink);
            va.AddVertex(x1 + offx, y1, z1, pink);
            va.AddVertex(x1, y1, z1 - offz, pink);
            va.AddVertex(x1, y1, z1 + offz, pink);
            va.Finish(GL_LINES);
        }

    }
    else if( show_empty ) {
        float rw = base->GetBaseObjectScreenLocation().GetRenderWi();
        float rh = base->GetBaseObjectScreenLocation().GetRenderHt();
        float x1 = -0.5f * rw;
        float x2 = -0.5f * rw;
        float x3 = 0.5f * rw;
        float x4 = 0.5f * rw;
        float y1 = -0.5f * rh;
        float y2 = 0.5f * rh;
        float y3 = 0.5f * rh;
        float y4 = -0.5f * rh;
        float z1 = 0.0f;
        float z2 = 0.0f;
        float z3 = 0.0f;
        float z4 = 0.0f;

        base->GetBaseObjectScreenLocation().TranslatePoint(x1, y1, z1);
        base->GetBaseObjectScreenLocation().TranslatePoint(x2, y2, z2);
        base->GetBaseObjectScreenLocation().TranslatePoint(x3, y3, z3);
        base->GetBaseObjectScreenLocation().TranslatePoint(x4, y4, z4);

        va.AddVertex(x1, y1, z1, *wxRED);
        va.AddVertex(x2, y2, z2, *wxRED);
        va.AddVertex(x2, y2, z2, *wxRED);
        va.AddVertex(x3, y3, z3, *wxRED);
        va.AddVertex(x3, y3, z3, *wxRED);
        va.AddVertex(x4, y4, z4, *wxRED);
        va.AddVertex(x4, y4, z4, *wxRED);
        va.AddVertex(x1, y1, z1, *wxRED);
        va.AddVertex(x1, y1, z1, *wxRED);
        va.AddVertex(x3, y3, z3, *wxRED);
        va.AddVertex(x2, y2, z2, *wxRED);
        va.AddVertex(x4, y4, z4, *wxRED);
        va.Finish(GL_LINES, GL_LINE_SMOOTH, 5.0f);
    }
}

// Serialize for output
void Mesh::Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const
{
    wxString res = "";

    wxXmlNode* child = root->GetChildren();
    while (child != nullptr) {
        if (child->GetName() == base_name) {
            wxXmlDocument new_doc;
            new_doc.SetRoot(new wxXmlNode(*child));
            wxStringOutputStream stream;
            new_doc.Save(stream);
            wxString s = stream.GetString();
            s = s.SubString(s.Find("\n") + 1, s.Length()); // skip over xml format header
            int index = s.Find(show_dir);
            while (index != wxNOT_FOUND) {
                s = s.SubString(0, index-1) + s.SubString(index + show_dir.Length() + 1, s.Length());
                index = s.Find(show_dir);
            }
            res += s;
        }
        child = child->GetNext();
    }

    if (res != "")
    {
        f.Write(res);
    }
}

// Serialise for input
void Mesh::Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const
{
    wxXmlNode* node = nullptr;
    for (wxXmlNode* n = model_xml->GetChildren(); n != nullptr; n = n->GetNext())
    {
        if (n->GetName() == base_name)
        {
            node = n;
            break;
        }
    }

    if (node != nullptr) {
        // add new attributes from import
        for (wxXmlNode* n = root->GetChildren(); n != nullptr; n = n->GetNext())
        {
            if (n->GetName() == base_name)
            {
                for (auto a = n->GetAttributes(); a != nullptr; a = a->GetNext())
                {
                    wxString s = a->GetValue();
                    if (a->GetName() == "ObjFile") {
                        s = show_dir + wxFileName::GetPathSeparator() + s;
                    }
                    if (node->HasAttribute(a->GetName())) {
                        node->DeleteAttribute(a->GetName());
                    }
                    node->AddAttribute(a->GetName(), s);
                }
                return;
            }
        }
    }
}
