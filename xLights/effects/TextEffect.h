#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "RenderableEffect.h"

class wxString;
class TextDrawingContext;

class TextEffect : public RenderableEffect
{
    public:
        TextEffect(int id);
        virtual ~TextEffect();
        virtual void SetDefaultParameters(Model *cls) override;
        virtual void Render(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override;
#ifdef LINUX
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override {return false;};
#endif
        virtual bool CanBeRandom() override {return false;}

        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void SelectTextColor(std::string& palette, int index);

        void RenderTextLine(RenderBuffer &buffer,
                            TextDrawingContext* dc, const wxString& Line_orig, int dir,
                            bool center, int Effect, int Countdown, int tspeed,
                            int startx, int starty, int endx, int endy,
                            bool isPixelBased);

};

#endif // TEXTEFFECT_H
