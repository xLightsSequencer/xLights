#ifndef MESH_H
#define MESH_H

#include <vector>
#include "../../Image.h"
#include "../tiny_obj_loader.h"
#include "../../DrawGLUtils.h"
class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;

class ModelPreview;
namespace DrawGLUtils {
    class xl3DMesh;
}

class Mesh
{
    public:
        Mesh(wxXmlNode* node, wxString _name);
        virtual ~Mesh();

        void Init(BaseObject* base, bool set_size);
        bool GetExists() { return !_objFile.empty(); }

        void AddTypeProperties(wxPropertyGridInterface* grid);
        void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Draw(BaseObject* base, ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, glm::mat4& base_matrix, glm::mat4& motion_matrix,
            bool show_empty, float pivot_offset_x = 0, float pivot_offset_y = 0, float pivot_offset_z = 0, bool rotation = false, bool use_pivot = false);

        void Serialise(wxXmlNode* root, wxFile& f, const wxString& show_dir) const;
        void Serialise(wxXmlNode* root, wxXmlNode* model_xml, const wxString& show_dir) const;

        void SetLink(Mesh* val) { link = val; }
        Mesh* GetLink() { return link; }

        float GetWidth() { return width; }
        float GetHeight() { return height; }
        void SetHalfHeight() { half_height = 0.5f; }

        // These 3 don't write to XML because currently they are only used to offset built-in models
        // like the skull which uses the offset to draw the same mesh twice.
        void SetOffsetX(float val) { offset_x = val; }  
        void SetOffsetY(float val) { offset_y = val; }
        void SetOffsetZ(float val) { offset_z = val; }

        void SetColor(const xlColor& _color, const std::string _name) { new_color = _color; update_color = true; color_name = _name; }

    protected:

        void loadObject(BaseObject* base);
        void uncacheDisplayObjects();

    private:
        wxXmlNode* node_xml;
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
        wxString base_name;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::map<std::string, Image*> textures;
        std::vector<int> lines;
        float bmin[3];
        float bmax[3];

        DrawGLUtils::xl3DMesh *mesh3d = nullptr;
        Mesh* link = nullptr;
};

#endif // MESH_H

