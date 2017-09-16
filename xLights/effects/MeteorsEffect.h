#ifndef METEORSEFFECT_H
#define METEORSEFFECT_H

#include "RenderableEffect.h"


class MeteorsEffect : public RenderableEffect
{
    public:
        MeteorsEffect(int id);
        virtual ~MeteorsEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
    
    void RenderMeteorsVertical(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsHorizontal(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsImplode(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed, int xoffset, int yoffset, bool fadeWithDistance);
    void RenderIcicleDrip(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int MeteorsEffect, int SwirlIntensity, int mspeed);
    void RenderMeteorsExplode(RenderBuffer &buffer, int ColorScheme, int Count, int Length, int SwirlIntensity, int mspeed, int xoffset, int yoffset, bool fadeWithDistance);
};

#endif // METEORSEFFECT_H
