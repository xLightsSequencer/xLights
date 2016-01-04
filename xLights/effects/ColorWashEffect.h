#ifndef COLORWASHEFFECT_H
#define COLORWASHEFFECT_H

#include "RenderableEffect.h"


class ColorWashEffect : public RenderableEffect
{
    public:
        ColorWashEffect(int id);
        virtual ~ColorWashEffect();
    
        virtual void SetDefaultParameters(ModelClass *cls);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // COLORWASHEFFECT_H
