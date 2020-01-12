#ifndef DMX_IMAGE_H
#define DMX_IMAGE_H

#include <vector>
#include "../../Image.h"
#include "../../DrawGLUtils.h"

class wxPropertyGridInterface;
class wxPropertyGridEvent;
class BaseObject;

class ModelPreview;
class wxXmlNode;

class DmxImage
{
    public:
        DmxImage(wxXmlNode* node, wxString _name);
        virtual ~DmxImage();

        void Init(BaseObject* base, bool set_size, bool show_empty_);
        bool GetExists() { return obj_exists; }

        void AddTypeProperties(wxPropertyGridInterface* grid);
        void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Draw(BaseObject* base, ModelPreview* preview, DrawGLUtils::xlAccumulator& va,
            glm::mat4& base_matrix, glm::mat4& motion_matrix,
            int transparency, float brightness,
            int pivot_offset_x, int pivot_offset_y, bool use_pivot);

    protected:

    private:
        wxXmlNode* node_xml;
        std::string _imageFile;
        int width;
        int height;
        std::map<std::string, Image*> _images;
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

};

#endif // DMX_IMAGE_H

