#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include "MeshObject.h"
#include "DrawGLUtils.h"
#include "UtilFunctions.h"
#include "ModelPreview.h"
#include "../xLightsMain.h"

#include <log4cpp/Category.hh>


#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

MeshObject::MeshObject(wxXmlNode *node, const ViewObjectManager &manager)
 : ObjectWithScreenLocation(manager), _objFile(""),
    width(100), height(100), depth(100), brightness(100),
    obj_loaded(false), mesh_only(false), diffuse_colors(false),
    mesh3d(nullptr)
{
    SetFromXml(node);
    screenLocation.SetSupportsZScaling(true);
}

MeshObject::~MeshObject()
{
    for (auto it : textures) {
        if (it.second != nullptr) {
            delete it.second;
        }
    }
    if (mesh3d) {
        delete mesh3d;
    }
}

void MeshObject::InitModel() {
	_objFile = FixFile("", ModelXml->GetAttribute("ObjFile", ""));
    mesh_only = ModelXml->GetAttribute("MeshOnly", "0") == "1";
    diffuse_colors = ModelXml->GetAttribute("Diffuse", "0") == "1";

    if (ModelXml->HasAttribute("Brightness")) {
        brightness = wxAtoi(ModelXml->GetAttribute("Brightness"));
        if (brightness > 100) {
            brightness = 100;
        } else if (brightness < 0) {
            brightness = 0;
        }
    }

    screenLocation.SetRenderSize(width, height, depth);
}

void MeshObject::AddTypeProperties(wxPropertyGridInterface *grid) {
	wxPGProperty *p = grid->Append(new wxFileProperty("ObjFile",
                                             "ObjFile",
                                             _objFile));
    p->SetAttribute(wxPG_FILE_WILDCARD, "Wavefront files|*.obj|All files (*.*)|*.*");

    p = grid->Append(new wxUIntProperty("Brightness", "Brightness", brightness));
    p->SetAttribute("Min", 0);
    p->SetAttribute("Max", 100);
    p->SetEditor("SpinCtrl");

    p = grid->Append(new wxBoolProperty("Mesh Only", "MeshOnly", mesh_only));
    p->SetAttribute("UseCheckbox", true);
    p = grid->Append(new wxBoolProperty("Diffuse Colors", "Diffuse", diffuse_colors));
    p->SetAttribute("UseCheckbox", true);
}

int MeshObject::OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) {
    if ("ObjFile" == event.GetPropertyName()) {
        obj_loaded = false;
        for (auto it = textures.begin(); it != textures.end(); ++it) {
            if (it->second != nullptr) {
                delete it->second;
            }
        }
        textures.clear();
        uncacheDisplayObjects();
        _objFile = event.GetValue().GetString();
        ModelXml->DeleteAttribute("ObjFile");
        ModelXml->AddAttribute("ObjFile", _objFile);
        SetFromXml(ModelXml);
        return 3;
    } else if ("Brightness" == event.GetPropertyName()) {
        brightness = (int)event.GetPropertyValue().GetLong();
        ModelXml->DeleteAttribute("Brightness");
        ModelXml->AddAttribute("Brightness", wxString::Format("%d", (int)brightness));
        return 3 | 0x0008;
    } else if ("MeshOnly" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("MeshOnly");
        mesh_only = event.GetValue().GetBool();
        if (mesh_only) {
            ModelXml->AddAttribute("MeshOnly", "1");
        }
        return 3 | 0x0008;
    } else if ("Diffuse" == event.GetPropertyName()) {
        ModelXml->DeleteAttribute("Diffuse");
        diffuse_colors = event.GetValue().GetBool();
        if (diffuse_colors) {
            ModelXml->AddAttribute("Diffuse", "1");
        }
        uncacheDisplayObjects();
        return 3 | 0x0008;
    }

    return ViewObject::OnPropertyGridChange(grid, event);
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

bool MeshObject::CleanupFileLocations(xLightsFrame* frame)
{
    bool rc = false;
    if (wxFile::Exists(_objFile))
    {
        if (!frame->IsInShowFolder(_objFile))
        {
            auto fr = GetFileReferences();
            for (auto f: fr)
            {
                if (f != _objFile)
                {
                    frame->MoveToShowFolder(f, wxString(wxFileName::GetPathSeparator()) + "3D");
                }
            }

            _objFile = frame->MoveToShowFolder(_objFile, wxString(wxFileName::GetPathSeparator()) + "3D");

            ModelXml->DeleteAttribute("ObjFile");
            ModelXml->AddAttribute("ObjFile", _objFile);
            SetFromXml(ModelXml);
            rc = true;
        }
    }

    return BaseObject::CleanupFileLocations(frame) || rc;
}

std::list<std::string> MeshObject::CheckModelSettings()
{
    std::list<std::string> res;

    if (_objFile == "" || !wxFile::Exists(_objFile))
    {
        res.push_back(wxString::Format("    ERR: Mesh object '%s' cant find obj file '%s'", GetName(), _objFile).ToStdString());
    }
    else
    {
        if (!IsFileInShowDir(xLightsFrame::CurrentDir, _objFile))
        {
            res.push_back(wxString::Format("    WARN: Mesh object '%s' obj file '%s' not under show directory.", GetName(), _objFile).ToStdString());
        }

        wxFileName fn(_objFile);
        fn.SetExt("mtl");
        if (!fn.Exists())
        {
            res.push_back(wxString::Format("    WARN: Mesh object '%s' does not have a material file '%s'.", GetName(), fn.GetFullPath()).ToStdString());
        }

        std::string base_path = fn.GetPath();

        tinyobj::attrib_t attr;
        std::vector<int> lin;
        std::vector<tinyobj::shape_t> shap;
        std::vector<tinyobj::material_t> mater;
        std::string err;
        tinyobj::LoadObj(&attr, &shap, &lin, &mater, &err, (char *)_objFile.c_str(), (char *)base_path.c_str());

        for (auto m : mater) {
            if (m.diffuse_texname.length() > 0) {
                wxFileName tex(m.diffuse_texname);
                tex.SetPath(fn.GetPath());
                if (!tex.Exists())
                {
                    wxFileName tex2(fn.GetPath() + "/" + m.diffuse_texname);
                    if (!tex2.Exists())
                    {
                        res.push_back(wxString::Format("    ERR: Mesh object '%s' cant find texture file '%s'", GetName(), tex.GetFullPath()).ToStdString());
                    }
                }
            }
        }
    }

    return res;
}

std::list<std::string> MeshObject::GetFileReferences()
{
    std::list<std::string> res;
    if (wxFile::Exists(_objFile))
    {
        res.push_back(_objFile);

        wxFileName mtl(_objFile);
        mtl.SetExt("mtl");

        if (mtl.Exists())
        {
            res.push_back(mtl.GetFullPath());
        }

        wxFileName fn(_objFile);
        std::string base_path = fn.GetPath();

        tinyobj::attrib_t attr;
        std::vector<int> lin;
        std::vector<tinyobj::shape_t> shap;
        std::vector<tinyobj::material_t> mater;
        std::string err;
        tinyobj::LoadObj(&attr, &shap, &lin, &mater, &err, (char *)_objFile.c_str(), (char *)base_path.c_str());

        for (auto m : mater) {
            if (m.diffuse_texname.length() > 0) {
                wxFileName tex(m.diffuse_texname);
                tex.SetPath(fn.GetPath());
                if (tex.Exists())
                {
                    res.push_back(tex.GetFullPath());
                }
                else
                {
                    wxFileName tex2(fn.GetPath() + "/" + m.diffuse_texname);
                    if (tex2.Exists())
                    {
                        res.push_back(tex2.GetFullPath());
                    }
                }
            }
        }
    }
    return res;
}

void MeshObject::IncrementChangeCount() {
    uncacheDisplayObjects();
    ObjectWithScreenLocation<BoxedScreenLocation>::IncrementChangeCount();
}

void MeshObject::uncacheDisplayObjects() {
    if (mesh3d) {
        delete mesh3d;
        mesh3d = nullptr;
    }
}

void MeshObject::loadObject() {
    if (wxFileExists(_objFile)) {
        static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

        logger_base.debug("Loading mesh model %s file %s.",
                          (const char *)GetName().c_str(),
                          (const char *)_objFile.c_str());
        wxFileName fn(_objFile);
        std::string base_path = fn.GetPath();
        std::string err;
        tinyobj::LoadObj(&attrib, &shapes, &lines, &materials, &err, (char *)_objFile.c_str(), (char *)base_path.c_str());
        
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
        screenLocation.SetRenderSize(width, height, depth);
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

void MeshObject::Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, bool allowSelected)
{
    if( !active ) { return; }

    GetObjectScreenLocation().PrepareToDraw(true, allowSelected);

    if (!obj_loaded) {
        loadObject();
    }

    GetObjectScreenLocation().UpdateBoundingBox(width, height);  // FIXME: Modify to only call this when position changes
    
    if (obj_loaded) {
        glm::mat4 m = glm::mat4(1.0f);
        glm::mat4 scalingMatrix = glm::scale(m, GetObjectScreenLocation().GetScaleMatrix());
        glm::vec3 rot = GetObjectScreenLocation().GetRotation();
        glm::mat4 RotateX = glm::rotate(m, glm::radians((float)-rot.x), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 RotateY = glm::rotate(m, glm::radians((float)-rot.y), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 RotateZ = glm::rotate(m, glm::radians((float)rot.z), glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 translationMatrix = glm::translate(m, GetObjectScreenLocation().GetWorldPosition());
        m = translationMatrix * RotateX * RotateY * RotateZ * scalingMatrix;
        
        if (!mesh3d) {
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
                        // Invaid material ID. Use default material.
                        current_material_id = materials.size() - 1;  // Default material is added to the last item in `materials`.
                    }

                    if (current_material_id != last_material_id) {
                        std::string diffuse_texname = materials[current_material_id].diffuse_texname;
                        if (textures.find(diffuse_texname) != textures.end()) {
                            image_id = textures[diffuse_texname]->getID();
                        } else {
                            image_id = -1;
                        }
                    }
                    last_material_id = current_material_id;

                    float diffuse[3];
                    for (size_t i = 0; i < 3; i++) {
                        diffuse[i] = materials[current_material_id].diffuse[i];
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
                        } else {
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
                    } else {
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
                        } else {
                            for (int k = 0; k < 3; k++) {
                                assert(size_t(3 * nf0 + k) < attrib.normals.size());
                                assert(size_t(3 * nf1 + k) < attrib.normals.size());
                                assert(size_t(3 * nf2 + k) < attrib.normals.size());
                                n[0][k] = attrib.normals[3 * nf0 + k];
                                n[1][k] = attrib.normals[3 * nf1 + k];
                                n[2][k] = attrib.normals[3 * nf2 + k];
                            }
                        }
                    } else {
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
                        float red = c[0] * 0.5 + 0.5;
                        float green = c[1] * 0.5 + 0.5;
                        float blue = c[2] * 0.5 + 0.5;

                        if (diffuse_colors) {
                            // just use diffuse color for now
                            red = diffuse[0];
                            green = diffuse[1];
                            blue = diffuse[2];
                        }
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
            if (lines.size() > 0) {
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
        }
        mesh3d->setMatrix(m);
        va3.AddMesh(mesh3d, mesh_only, brightness);
    }

    if ((Selected || Highlighted) && allowSelected) {
        GetObjectScreenLocation().DrawHandles(va3);
    }
}
