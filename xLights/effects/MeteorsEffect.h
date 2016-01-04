#ifndef METEORSEFFECT_H
#define METEORSEFFECT_H

#include "RenderableEffect.h"


class MeteorsEffect : public RenderableEffect
{
    public:
        MeteorsEffect(int id);
        virtual ~MeteorsEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // METEORSEFFECT_H
