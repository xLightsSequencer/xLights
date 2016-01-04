#ifndef SPIRALSEFFECT_H
#define SPIRALSEFFECT_H

#include "RenderableEffect.h"


class SpiralsEffect : public RenderableEffect {
    public:
        SpiralsEffect(int id);
        virtual ~SpiralsEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // SPIRALSEFFECT_H
