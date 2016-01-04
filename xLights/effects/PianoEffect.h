#ifndef PIANOEFFECT_H
#define PIANOEFFECT_H

#include "RenderableEffect.h"


class PianoEffect : public RenderableEffect
{
    public:
        PianoEffect(int id);
        virtual ~PianoEffect();
    
        virtual void SetSequenceElements(SequenceElements *els);

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // PIANOEFFECT_H
