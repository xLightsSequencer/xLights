#ifndef VUMETEREFFECT_H
#define VUMETEREFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>

class VUMeterEffect : public RenderableEffect
{
    public:
        VUMeterEffect(int id);
        virtual ~VUMeterEffect();
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void Render(RenderBuffer &buffer,
                    int bars);
        virtual bool CanRenderOnBackgroundThread();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
};

#endif // VUMETEREFFECT_H
