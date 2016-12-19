#ifndef BARSEFFECT_H
#define BARSEFFECT_H

#include "RenderableEffect.h"

class BarsEffect : public RenderableEffect
{
    public:
        BarsEffect(int id);
        virtual ~BarsEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsSpatialColorCurves() override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        void GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer &buffer, bool gradient, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index);
    private:
};

#endif // BARSEFFECT_H
