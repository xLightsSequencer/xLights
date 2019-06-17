#ifndef KaleidoscopeEFFECT_H
#define KaleidoscopeEFFECT_H

#include <vector>

#include "RenderableEffect.h"

#define KALEIDOSCOPE_X_MIN 0
#define KALEIDOSCOPE_X_MAX 100

#define KALEIDOSCOPE_Y_MIN 0
#define KALEIDOSCOPE_Y_MAX 100

#define KALEIDOSCOPE_SIZE_MIN 2
#define KALEIDOSCOPE_SIZE_MAX 100

#define KALEIDOSCOPE_ROTATION_MIN 0
#define KALEIDOSCOPE_ROTATION_MAX 359

struct KaleidoscopeEdge
{
    wxPoint _p1;
    wxPoint _p2;
    KaleidoscopeEdge(const wxPoint &p1, const wxPoint& p2) { _p1 = p1; _p2 = p2; }
};

class KaleidoscopeEffect : public RenderableEffect
{
    public:
        KaleidoscopeEffect(int id);
        virtual ~KaleidoscopeEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) const override { return false; }
        virtual void SetDefaultParameters() override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
        bool KaleidoscopeDone(const std::vector<std::vector<bool>>& current);
        std::pair<int, int> GetSourceLocation(int x, int y, const KaleidoscopeEdge& edge, int width, int height);
};

#endif // KaleidoscopeEFFECT_H
