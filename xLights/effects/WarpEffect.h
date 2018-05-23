#ifndef WARPEFFECT_H
#define WARPEFFECT_H

#include "RenderableEffect.h"

class WarpEffect : public RenderableEffect
{
    public:
        WarpEffect(int id);
        virtual ~WarpEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return false; }
        virtual void SetDefaultParameters(Model *cls) override;
        virtual std::string GetEffectString() override;
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;

    protected:
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;

        struct VertexTex
        {
           float v[2];
           float t[2];
        };

        static void sizeForRenderBuffer( const RenderBuffer& rb );

        static bool s_shadersInit;
        static unsigned s_programId_dissolve_in;
        static unsigned s_programId_dissolve_out;
        static unsigned s_programId_ripple_in;
        static unsigned s_programId_ripple_out;
        static unsigned s_noiseTexId;
        static unsigned s_vertexArrayId;
        static unsigned s_vertexBufferId;
        static unsigned s_fbId;
        static unsigned s_rbId;
        static unsigned s_rbTex;
        static int s_rbWidth;
        static int s_rbHeight;
};

#endif // WARPEFFECT_H
