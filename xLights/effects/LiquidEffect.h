#ifndef LIQUIDEFFECT_H
#define LIQUIDEFFECT_H

#include "RenderableEffect.h"
#include "../RenderBuffer.h"
#include <string>
#include <list>

class wxString;
class b2World;
class b2ParticleSystem;

#define LIQUID_LIFETIME_MIN 0
#define LIQUID_LIFETIME_MAX 1000

#define LIQUID_DIRECTION_MIN 0
#define LIQUID_DIRECTION_MAX 360

#define LIQUID_FLOW_MIN 0
#define LIQUID_FLOW_MAX 1000

#define LIQUID_VELOCITY_MIN 0
#define LIQUID_VELOCITY_MAX 1000

#define LIQUID_X_MIN 0
#define LIQUID_X_MAX 100

#define LIQUID_Y_MIN 0
#define LIQUID_Y_MAX 100

#define LIQUID_SOURCESIZE_MIN 0
#define LIQUID_SOURCESIZE_MAX 100

class LiquidEffect : public RenderableEffect
{
    public:
        LiquidEffect(int id);
        virtual ~LiquidEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        virtual bool AppropriateOnNodes() const override { return false; }
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        void Render(RenderBuffer &buffer,
            bool top, bool bottom, bool left, bool right,
            int lifetime, bool holdcolor, bool mixcolors, int size, int warmUpFrames,
            int direction1, int x1, int y1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
            bool enabled2, int direction2, int x2, int y2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
            bool enabled3, int direction3, int x3, int y3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
            bool enabled4, int direction4, int x4, int y4, int velocity4, int flow4, int sourceSize4, bool flowMusic4,
            const std::string& particleType, int despeckle, float gravity);
        void CreateBarrier(b2World* world, float x, float y, float width, float height);
        void Draw(RenderBuffer& buffer, b2ParticleSystem* ps, const xlColor& color, bool mixColors, int despeckle);
        void CreateParticles(b2ParticleSystem* ps, int x, int y, int direction, int velocity, int flow, bool flowMusic, int lifetime, int width, int height, const xlColor& c, const std::string& particleType, bool mixcolors, float audioLevel, int sourceSize);
        void CreateParticleSystem(b2World* world, int lifetime, int size);
        void Step(b2World* world, RenderBuffer &buffer, bool enabled[], int lifetime, const std::string& particleType, bool mixcolors,
            int x1, int y1, int direction1, int velocity1, int flow1, int sourceSize1, bool flowMusic1,
            int x2, int y2, int direction2, int velocity2, int flow2, int sourceSize2, bool flowMusic2,
            int x3, int y3, int direction3, int velocity3, int flow3, int sourceSize3, bool flowMusic3,
            int x4, int y4, int direction4, int velocity4, int flow4, int sourceSize4, bool flowMusic4, float time
        );
        xlColor GetDespeckleColor(RenderBuffer& buffer, size_t x, size_t y, int despeckle) const;
};

#endif // LIQUIDEFFECT_H
