#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/file.h>
#include <vector>
#include <glm/glm.hpp>
#include "../../graphics/tiny_obj_loader.h"
#include "../../Color.h"

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class wxXmlNode;
class BaseObject;

class ModelPreview;
class xlGraphicsContext;
class xlGraphicsProgram;
class xlMesh;

class Mesh
{
public:
    Mesh(wxXmlNode* node, wxString _name);
    virtual ~Mesh();

    void Init(BaseObject* base, bool set_size);
    bool GetExists(BaseObject* base, xlGraphicsContext *ctx);
    bool HasObjFile() const { return !_objFile.empty(); }

    void AddTypeProperties(wxPropertyGridInterface* grid);
    void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

    int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

    void Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram,
            glm::mat4& base_matrix, glm::mat4& motion_matrix,
            bool show_empty, float pivot_offset_x = 0, float pivot_offset_y = 0, float pivot_offset_z = 0,
            bool rotation = false, bool use_pivot = false);

    void Draw(BaseObject* base, ModelPreview* preview, xlGraphicsProgram *sprogram, xlGraphicsProgram *tprogram,     glm::mat4& base_matrix, glm::mat4& trans_matrix, float xrot, float yrot, float zrot,
            bool show_empty, float pivot_offset_x, float pivot_offset_y, float pivot_offset_z,
            bool rotation, bool use_pivot);

    void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
    void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

    void SetLink(Mesh* val) { link = val; }
    Mesh* GetLink() { return link; }

    float GetWidth() const { return width; }
    float GetHeight() const { return height; }
    float GetDepth() const { return depth; }
    void SetHalfHeight() { half_height = 0.5f; }

    std::string GetName() const { return base_name; }
    std::string GetObjFile() const { return _objFile; }
    float GetBrightness() const { return brightness; }
    bool GetMeshOnly() const { return mesh_only; }
    float GetOffsetX() const { return offset_x; }
    float GetOffsetY() const { return offset_y; }
    float GetOffsetZ() const { return offset_z; }
    float GetScaleX() const { return scalex; }
    float GetScaleY() const { return scaley; }
    float GetScaleZ() const { return scalez; }
    float GetRotateX() const { return rotatex; }
    float GetRotateY() const { return rotatey; }
    float GetRotateZ() const { return rotatez; }

    void SetRenderScaling(float s);
    void SetMeshOnly(bool val) { mesh_only = val; }

    // These 3 don't write to XML because currently they are only used to offset built-in models
    // like the skull which uses the offset to draw the same mesh twice.
    void SetOffsetX(float val) { offset_x = val; }
    void SetOffsetY(float val) { offset_y = val; }
    void SetOffsetZ(float val) { offset_z = val; }

    void SetColor(const xlColor& _color, const std::string _name) { new_color = _color; update_color = true; color_name = _name; }

protected:

    void loadObject(BaseObject* base, xlGraphicsContext *ctx);
    void uncacheDisplayObjects();

private:
    wxXmlNode* node_xml = nullptr;
    std::string _objFile;
    float width = 1.0f;
    float height = 1.0f;
    float depth = 1.0f;
    float brightness = 100.0f;
    bool obj_loaded = false;
    bool mesh_only = false;
    bool obj_exists = false;
    bool controls_size = false;
    bool update_color = false;
    std::string color_name;
    xlColor new_color = xlBLACK;
    xlColor last_color = xlBLACK;
    float offset_x = 0.0f;
    float offset_y = 0.0f;
    float offset_z = 0.0f;
    float scalex = 1.0f;
    float scaley = 1.0f;
    float scalez = 1.0f;
    float rotatex = 0.0f;
    float rotatey = 0.0f;
    float rotatez = 0.0f;
    float half_height = 1.0f;
    float rscale = 1.0f;
    wxString base_name;

    float bmin[3] = { 0.0, 0.0, 0.0 };
    float bmax[3] = { 0.0, 0.0, 0.0 };

    Mesh* link = nullptr;
    xlMesh *mesh3d = nullptr;
};

