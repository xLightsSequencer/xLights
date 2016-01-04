#ifndef PICTURESEFFECT_H
#define PICTURESEFFECT_H

#include "RenderableEffect.h"


class PicturesEffect : public RenderableEffect
{
    public:
        PicturesEffect(int id);
        virtual ~PicturesEffect();
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // PICTURESEFFECT_H
