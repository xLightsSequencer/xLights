#ifndef DMXMOVINGHEAD_H
#define DMXMOVINGHEAD_H

#include "DmxModel.h"
#include "DmxColorAbility.h"
#include "DmxPanTiltAbility.h"

class DmxMovingHead : public DmxModel, public DmxColorAbility, public DmxPanTiltAbility
{
    public:
        DmxMovingHead(wxXmlNode *node, const ModelManager &manager, bool zeroBased = false);
        virtual ~DmxMovingHead();

        virtual bool HasColorAbility() override { return true; }

        virtual void AddTypeProperties(wxPropertyGridInterface *grid) override;
        virtual int OnPropertyGridChange(wxPropertyGridInterface *grid, wxPropertyGridEvent& event) override;

    protected:
        DmxMovingHead(const ModelManager &manager);
        virtual void InitModel() override;

        virtual void ExportXlightsModel() override;
        virtual void ImportXlightsModel(std::string filename, xLightsFrame* xlights, float& min_x, float& max_x, float& min_y, float& max_y) override;

        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xlAccumulator &va, const xlColor *c, float &sx, float &sy, bool active) override;
        virtual void DrawModelOnWindow(ModelPreview* preview, DrawGLUtils::xl3Accumulator &va, const xlColor *c, float &sx, float &sy, float &sz, bool active) override;

        bool hide_body = false;
        bool style_changed;
        std::string dmx_style;
        int dmx_style_val;
        int shutter_channel;
        int shutter_threshold;
        float beam_length;

    private:
};

#endif // DMXMOVINGHEAD_H
