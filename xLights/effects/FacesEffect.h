#ifndef FACESEFFECT_H
#define FACESEFFECT_H

#include "RenderableEffect.h"


class FacesEffect : public RenderableEffect
{
    public:
        FacesEffect(int id);
        virtual ~FacesEffect();
    
        virtual void SetDefaultParameters(ModelClass *cls);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // FACESEFFECT_H
