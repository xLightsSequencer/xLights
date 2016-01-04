#ifndef CURTAINEFFECT_H
#define CURTAINEFFECT_H

#include "RenderableEffect.h"


class CurtainEffect : public RenderableEffect
{
    public:
        CurtainEffect(int id);
        virtual ~CurtainEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // CURTAINEFFECT_H
