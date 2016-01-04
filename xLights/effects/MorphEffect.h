#ifndef MORPHEFFECT_H
#define MORPHEFFECT_H

#include "RenderableEffect.h"


class MorphEffect : public RenderableEffect
{
    public:
        MorphEffect(int id);
        virtual ~MorphEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // MORPHEFFECT_H
