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

#include "ViewObject.h"
#include "BoxedScreenLocation.h"
#include "../graphics/xlGraphicsAccumulators.h"

class IModelPreview;

class ImageObject : public ObjectWithScreenLocation<BoxedScreenLocation>
{
public:
    ImageObject(const ViewObjectManager &manager);
    virtual ~ImageObject();

    virtual void InitModel() override;


    virtual bool Draw(IModelPreview* preview, xlGraphicsContext *ctx, xlGraphicsProgram *solid, xlGraphicsProgram *transparent, bool allowSelected = false) override;

    virtual std::list<std::string> GetFileReferences() override;
    virtual bool CleanupFileLocations(RenderContext* ctx) override;
    virtual std::list<std::string> CheckModelSettings() override;

    void SetImageFile(const std::string & imageFile);
    void SetImageFileDirect(const std::string& file) { _imageFile = file; }
    void SetTransparency(int val) { transparency = val; }
    void SetBrightness(float val) { brightness = val; }
    void ClearImages();

    const std::string& GetImageFile() const { return _imageFile; }
    int GetTransparency() const { return transparency; }
    int GetBrightness() const { return brightness; }

    void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

protected:

private:
    std::string _imageFile {""};
    int width {1};
    int height {1};
    int transparency {0};
    int brightness {100};
    std::map<std::string, xlTexture*> _images;
};

