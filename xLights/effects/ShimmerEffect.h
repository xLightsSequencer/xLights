#ifndef SHIMMEREFFECT_H
#define SHIMMEREFFECT_H

#include "RenderableEffect.h"


class ShimmerEffect : public RenderableEffect
{
    public:
        ShimmerEffect(int id);
        virtual ~ShimmerEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // SHIMMEREFFECT_H
