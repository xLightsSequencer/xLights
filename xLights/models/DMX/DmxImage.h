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

        void Init(BaseObject* base, bool set_size);
        bool GetExists() { return obj_exists; }

        void AddTypeProperties(wxPropertyGridInterface* grid);
        void UpdateTypeProperties(wxPropertyGridInterface* grid) {}

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event, BaseObject* base, bool locked);

        void Draw(BaseObject* base, ModelPreview* preview, DrawGLUtils::xlAccumulator& va,
            glm::mat4& base_matrix, glm::mat4& motion_matrix,
            int transparency, float brightness, bool only_image,
            float pivot_offset_x, float pivot_offset_y, bool rotation, bool use_pivot);

        void SetOffsetZ(float value, BaseObject* base);
        bool ImageSelected() const { return image_selected; }
        void ClearImageSelected() { image_selected = false; }
        int GetWidth() const { return width; }
        int GetHeight() const { return height; }
        void SetScaleX(float value, BaseObject* base);
        void SetScaleY(float value, BaseObject* base);

    protected:

    private:
        wxXmlNode* node_xml;
        std::string _imageFile;
        int width;
        int height;
        std::map<std::string, Image*> _images;
        bool obj_exists;
        bool image_selected;

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

