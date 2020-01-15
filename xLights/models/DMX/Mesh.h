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

        void Init(BaseObject* base, bool set_size, bool show_empty_);
        bool GetExists() { return !_objFile.empty(); }

        void AddTypeProperties(wxPropertyGridInterface* grid);
        void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Draw(BaseObject* base, ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, glm::mat4& base_matrix, glm::mat4& motion_matrix);

    protected:

        void loadObject(BaseObject* base);
        void uncacheDisplayObjects();

    private:
        wxXmlNode* node_xml;
        std::string _objFile;
        float width;
        float height;
        float depth;
        float brightness;
        bool obj_loaded;
        bool mesh_only;
        bool obj_exists;
        bool show_empty;
        bool controls_size;
        float offset_x;
        float offset_y;
        float offset_z;
        float scalex;
        float scaley;
        float scalez;
        float rotatex;
        float rotatey;
        float rotatez;
        wxString base_name;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::map<std::string, Image*> textures;
        std::vector<int> lines;
        float bmin[3];
        float bmax[3];

        DrawGLUtils::xl3DMesh *mesh3d;
};

#endif // MESH_H

