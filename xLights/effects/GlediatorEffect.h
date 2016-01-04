#ifndef GLEDIATOREFFECT_H
#define GLEDIATOREFFECT_H

#include "RenderableEffect.h"


class GlediatorEffect : public RenderableEffect
{
    public:
        GlediatorEffect(int id);
        virtual ~GlediatorEffect();
    
        virtual void SetSequenceElements(SequenceElements *els);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // GLEDIATOREFFECT_H
