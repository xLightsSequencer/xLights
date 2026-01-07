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

#include "Model.h"

class xlTexture;

class ImageModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        ImageModel(const ModelManager &manager);
        virtual ~ImageModel();

        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi, int stagger) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi, int stagger, bool deep = false) const override;
    
        virtual void DisplayModelOnWindow(ModelPreview* preview, xlGraphicsContext *ctx,
                                          xlGraphicsProgram *solidProgram, xlGraphicsProgram *transparentProgram, bool is_3d = false,
                                          const xlColor* color = nullptr, bool allowSelected = false, bool wiring = false,
                                          bool highlightFirst = false, int highlightpixel = 0,
                                          float *boundingBox = nullptr) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;
    
        virtual void AddTypeProperties(wxPropertyGridInterface* grid, OutputManager* outputManager) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        [[nodiscard]] virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        [[nodiscard]] virtual bool SupportsXlightsModel() override { return true; }
        [[nodiscard]] virtual bool SupportsExportAsCustom() const override { return false; }
        [[nodiscard]] virtual bool SupportsWiringView() const override { return false; }
        [[nodiscard]] virtual int GetNumPhysicalStrings() const override { return 1; }
        [[nodiscard]] virtual std::list<std::string> GetFileReferences() override;
        [[nodiscard]] virtual bool CleanupFileLocations(xLightsFrame* frame) override;
        [[nodiscard]] virtual std::list<std::string> CheckModelSettings() override;
        [[nodiscard]] std::string GetImageFile() const { return _imageFile; }
        [[nodiscard]] bool IsWhiteAsAlpha() const { return _whiteAsAlpha; }
        [[nodiscard]] int GetOffBrightness() const { return _offBrightness; }

        void SetImageFile(const std::string & imageFile);
        void SetWhiteAsAlpha(bool alpha) { _whiteAsAlpha = alpha; }
        void SetOffBrightness(int brightness) { _offBrightness = brightness; }

        [[nodiscard]] virtual bool SupportsVisitors() const override { return true; }
        void Accept(BaseObjectVisitor& visitor) const override { return visitor.Visit(*this); }

    protected:
        int GetChannelValue(int channel);

        virtual void InitModel() override;

        bool _whiteAsAlpha;
        std::string _imageFile;
        std::map<std::string, xlTexture*> _images;
        int _offBrightness;
    
        int width = 0;
        int height = 0;
        bool hasAlpha = false;
};

