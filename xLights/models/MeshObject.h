#ifndef MESHOBJECT_H
#define MESHOBJECT_H

#include <vector>
#include "ViewObject.h"
#include "Image.h"
#include "tiny_obj_loader.h"

class ModelPreview;

class MeshObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    public:
        MeshObject(wxXmlNode *node, const ViewObjectManager &manager);
        virtual ~MeshObject();

        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);

        virtual void Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, bool allowSelected = false) override;

    protected:

    private:
        std::string _objFile;
        float width;
        float height;
        float depth;
        int transparency;
        bool obj_loaded;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        float bmin[3];
        float bmax[3];

};

#endif // MESHOBJECT_H

