#ifndef MORPHEFFECT_H
#define MORPHEFFECT_H

#include "RenderableEffect.h"

class MorphEffect : public RenderableEffect
{
    public:
        MorphEffect(int id);
        virtual ~MorphEffect();

        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
        virtual int DrawEffectBackground(const Effect *e, int x1, int y1, int x2, int y2, DrawGLUtils::xlVertexColorAccumulator &backgrounds, xlColor* colorMask) override;
        virtual AssistPanel *GetAssistPanel(wxWindow *parent, xLightsFrame* xl_frame) override;
        virtual bool HasAssistPanel() override { return true; }
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual void SetDefaultParameters(Model *cls) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
};

#endif // MORPHEFFECT_H
