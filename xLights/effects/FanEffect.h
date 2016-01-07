#ifndef FANEFFECT_H
#define FANEFFECT_H

#include "RenderableEffect.h"


class FanEffect : public RenderableEffect
{
    public:
        FanEffect(int id);
        virtual ~FanEffect();
    
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    
    
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // FANEFFECT_H
