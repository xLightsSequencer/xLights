#ifndef BARSEFFECT_H
#define BARSEFFECT_H

#include "RenderableEffect.h"

class BarsEffect : public RenderableEffect
{
    public:
        BarsEffect(int id);
        virtual ~BarsEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // BARSEFFECT_H
