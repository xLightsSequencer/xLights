#ifndef IMAGEOBJECT_H
#define IMAGEOBJECT_H

#include "ViewObject.h"
#include "Image.h"

class ModelPreview;

class ImageObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
    public:
        ImageObject(wxXmlNode *node, const ViewObjectManager &manager);
        virtual ~ImageObject();

        virtual void InitModel() override;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event);

        virtual void Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, bool allowSelected = false) override;

    protected:

    private:
        std::string _imageFile;
        int width;
        int height;
        std::map<std::string, Image*> _images;
        int transparency;

};

#endif // IMAGEOBJECT_H
