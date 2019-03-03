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

        int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual void Draw(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va3, DrawGLUtils::xl3Accumulator &tva3, bool allowSelected = false) override;

        virtual std::list<std::string> GetFileReferences() override;
        virtual bool CleanupFileLocations(xLightsFrame* frame) override;
        virtual std::list<std::string> CheckModelSettings() override;

    protected:

    private:
        std::string _imageFile;
        int width;
        int height;
        std::map<std::string, Image*> _images;
        int transparency;
        float brightness;

};

#endif // IMAGEOBJECT_H
