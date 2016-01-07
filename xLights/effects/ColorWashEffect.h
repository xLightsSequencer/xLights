#ifndef COLORWASHEFFECT_H
#define COLORWASHEFFECT_H

#include "RenderableEffect.h"


class ColorWashEffect : public RenderableEffect
{
    public:
        ColorWashEffect(int id);
        virtual ~ColorWashEffect();
    
        virtual void SetDefaultParameters(ModelClass *cls);
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // COLORWASHEFFECT_H
