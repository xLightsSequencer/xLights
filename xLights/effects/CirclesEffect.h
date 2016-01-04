#ifndef CIRCLESEFFECT_H
#define CIRCLESEFFECT_H

#include "RenderableEffect.h"


class CirclesEffect : public RenderableEffect
{
    public:
        CirclesEffect(int id);
        virtual ~CirclesEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // CIRCLESEFFECT_H
