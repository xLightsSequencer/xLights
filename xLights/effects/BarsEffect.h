#ifndef BARSEFFECT_H
#define BARSEFFECT_H

#include "RenderableEffect.h"

#define BARCOUNT_MIN 1
//#define BARCOUNT_MAX 50
#define BARCOUNT_MAX 5

#define BARCYCLES_MIN 0
//#define BARCYCLES_MAX 500
#define BARCYCLES_MAX 300

#define BARCENTER_MIN -100
#define BARCENTER_MAX 100

class BarsEffect : public RenderableEffect
{
    public:
        BarsEffect(int id);
        virtual ~BarsEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return true; }
        virtual bool CanRenderPartialTimeInterval() const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        void GetSpatialColor(xlColor& color, size_t colorIndex, float x, float y, RenderBuffer &buffer, bool gradient, bool highlight, bool show3d, int BarHt, int n, float pct, int color2Index);
    private:
};

#endif // BARSEFFECT_H
