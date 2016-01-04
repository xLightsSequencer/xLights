#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "RenderableEffect.h"


class TextEffect : public RenderableEffect
{
    public:
        TextEffect(int id);
        virtual ~TextEffect();
        virtual void SetDefaultParameters(ModelClass *cls);
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // TEXTEFFECT_H
