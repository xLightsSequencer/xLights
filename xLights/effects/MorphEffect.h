#ifndef MORPHEFFECT_H
#define MORPHEFFECT_H

#include "RenderableEffect.h"


class MorphEffect : public RenderableEffect
{
    public:
        MorphEffect(int id);
        virtual ~MorphEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2);
        virtual AssistPanel *GetAssistPanel(wxWindow *parent);
        virtual bool HasAssistPanel() { return true; }
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent);
    private:
};

#endif // MORPHEFFECT_H
