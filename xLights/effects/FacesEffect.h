#ifndef FACESEFFECT_H
#define FACESEFFECT_H

#include "RenderableEffect.h"


class FacesEffect : public RenderableEffect
{
    public:
        FacesEffect(int id);
        virtual ~FacesEffect();
    
        virtual void SetDefaultParameters(ModelClass *cls);
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // FACESEFFECT_H
