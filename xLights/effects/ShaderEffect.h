#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H

#include "RenderableEffect.h"

class ShaderEffect : public RenderableEffect
{
    public:
        ShaderEffect(int id);
        virtual ~ShaderEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override { return false; }
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return false; }
        virtual void SetDefaultParameters() override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;

    protected:
        struct VertexTex
        {
           float v[2];
           float t[2];
        };
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;

        void sizeForRenderBuffer(const RenderBuffer& rb);

        static bool s_shadersInit;
        static unsigned s_vertexArrayId;
        static unsigned s_vertexBufferId;
        static unsigned s_fbId;
        static unsigned s_rbId;
        static unsigned s_rbTex;
        static unsigned s_programId;
        static int s_rbWidth;
        static int s_rbHeight;
};

#endif // SHADEREFFECT_H
