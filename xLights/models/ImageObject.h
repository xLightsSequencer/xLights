#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include "ViewObject.h"
#include "Image.h"

class ImageObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    public:
        ImageObject(wxXmlNode *node, const ViewObjectManager &manager);
        virtual ~ImageObject();

        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);

        virtual void Draw(DrawGLUtils::xl3Accumulator &va3, bool allowSelected = false) override;

    protected:
    void DrawObjectOnWindow(/*ModelPreview* preview,*/ DrawGLUtils::xl3Accumulator &va,
        float &x1, float &y1, float &z1,
        float &x2, float &y2, float &z2,
        float &x3, float &y3, float &z3,
        float &x4, float &y4, float &z4);

    private:
        std::string _imageFile;
        Image* _image;
        int width;
        int height;
        //std::map<std::string, Image*> _images;
        int transparency;

};

#endif // IMAGEOBJECT_H
