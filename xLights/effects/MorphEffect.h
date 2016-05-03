#ifndef MORPHEFFECT_H
#define MORPHEFFECT_H

#include "RenderableEffect.h"


class MorphEffect : public RenderableEffect
{
    public:
        MorphEffect(int id);
        virtual ~MorphEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlVertexColorAccumulator &backgrounds) override;
        virtual AssistPanel *GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame) override;
        virtual bool HasAssistPanel() override { return true; }
    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
};

#endif // MORPHEFFECT_H
