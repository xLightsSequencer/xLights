#ifndef TEXTEFFECT_H
#define TEXTEFFECT_H

#include "RenderableEffect.h"

class wxString;
class TextDrawingContext;
class FontManager;
class wxImage;

class TextEffect : public RenderableEffect
{
    public:
        TextEffect(int id);
        virtual ~TextEffect();
        virtual void SetDefaultParameters() override;
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual void SetPanelStatus(Model* cls) override;
#ifdef LINUX
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override {return false;};
#endif
        virtual bool CanBeRandom() override {return false;}
        virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }

        virtual bool needToAdjustSettings(const std::string &version) override { return true; }
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
        virtual bool AppropriateOnNodes() const override { return false; }
        virtual std::list<std::string> GetFileReferences(const SettingsMap &SettingsMap) const override;
        virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap &SettingsMap) override;

    protected:
        virtual wxPanel *CreatePanel(wxWindow *parent) override;
    private:
        void SelectTextColor(std::string& palette, int index) const;
        void FormatCountdown(int Countdown, int state, wxString& Line, RenderBuffer &buffer, wxString& msg, wxString Line_orig) const;

        wxImage *RenderTextLine(RenderBuffer &buffer,
                            TextDrawingContext* dc,
                            const wxString& Line_orig,
                            const std::string &fontString,
                            int dir,
                            bool center, int Effect, int Countdown, int tspeed,
                            int startx, int starty, int endx, int endy,
                            bool isPixelBased) const;
        void RenderXLText(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer);
        void AddMotions( int& OffsetLeft, int& OffsetTop, const SettingsMap& settings,
                         RenderBuffer &buffer, int txtwidth, int txtheight, int endx, int endy, bool pixelOffset,
                         int PreOffsetLeft, int PreOffsetTop ) const;
        FontManager& font_mgr;
};

#endif // TEXTEFFECT_H
