#ifndef DMXFLOODLIGHT_H
#define DMXFLOODLIGHT_H

#include "DmxModel.h"
#include "DmxColorAbility.h"

class DmxFloodlight : public DmxModel, DmxColorAbility
{
    public:
        DmxFloodlight(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxFloodlight();

    protected:
        DmxFloodlight(const ModelManager &manager);
        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual bool HasColorAbility() override { return true; }

        virtual void AddTypeProperties(wxPropertyGridInterface* grid) override;
        virtual void DisableUnusedProperties(wxPropertyGridInterface* grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface* grid, wxPropertyGridEvent& event) override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

    private:
};

#endif // DMXFLOODLIGHT_H
