#ifndef DMXMODEL_H
#define DMXMODEL_H

#include "../Model.h"
class DmxColorAbility;

class DmxModel : public ModelWithScreenLocation<BoxedScreenLocation>
{
    public:
        DmxModel(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxModel();

        virtual void GetBufferSize(const std::string &type, const std::string &camera, const std::string &transform,
                                   int &BufferWi, int &BufferHi) const override;
        virtual void InitRenderBufferNodes(const std::string &type, const std::string &camera, const std::string &transform,
                                           std::vector<NodeBaseClassPtr> &Nodes, int &BufferWi, int &BufferHi) const override;

        virtual void DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, DrawGLUtils::xlAccumulator &tva, bool is_3d = false, const xlColor *color = NULL, bool allowSelected = true) override;
        virtual void DisplayModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, DrawGLUtils::xl3Accumulator &tva, DrawGLUtils::xl3Accumulator& lva, bool is_3d = false, const xlColor *color = NULL, bool allowSelected = true, bool wiring = false, bool highlightFirst = false) override;
        virtual void DisplayEffectOnWindow(ModelPreview* preview, double pointSize) override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator& va, const xlColor* c, float& sx, float& sy, bool active) = 0;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator& va, const xlColor* c, float& sx, float& sy, float& sz, bool active) = 0;

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

        virtual bool HasColorAbility() { return false; }
        DmxColorAbility* GetColorAbility() { return color_ability; }
        virtual bool SupportsXlightsModel() override { return true; }
        virtual bool SupportsExportAsCustom() const override { return false; }
        virtual bool SupportsWiringView() const override { return false; }
        virtual void ExportXlightsModel() override = 0;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override = 0;
        virtual int GetNumPhysicalStrings() const override { return 1; }

    protected:

        DmxModel(const ModelManager &manager);
        virtual void InitModel() override;

        virtual int GetChannelValue( int channel );

        DmxColorAbility* color_ability;

    private:
};

#endif // DMXMODEL_H
