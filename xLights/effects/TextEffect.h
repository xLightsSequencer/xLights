#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

#include <vector>

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
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual void SetPanelStatus(Model* cls) override;
#ifdef LINUX
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override { return false; };
#endif
    virtual bool CanBeRandom() override { return false; }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override;

    virtual bool needToAdjustSettings(const std::string& version) override { return true; }
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool AppropriateOnNodes() const override { return false; }
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;

protected:
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;
private:
    void SelectTextColor(std::string& palette, int index) const;
    void FormatCountdown(int Countdown, int state, wxString& Line, RenderBuffer& buffer, wxString& msg, wxString Line_orig) const;
    std::vector<std::string> WordSplit(const std::string& text) const;
    std::string FlipWord(const SettingsMap& settings, const std::string& text, RenderBuffer& buffer) const;

    void ReplaceVaribles(wxString& msg, RenderBuffer& buffer) const;

    wxImage* RenderTextLine(RenderBuffer& buffer,
        TextDrawingContext* dc,
        const wxString& Line_orig,
        const std::string& fontString,
        int dir,
        bool center, bool norepeat, int Effect, int Countdown, int tspeed,
        int startx, int starty, int endx, int endy,
        bool isPixelBased, bool perWord) const;
    void RenderXLText(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer);
    void AddMotions(int& OffsetLeft, int& OffsetTop, const SettingsMap& settings, RenderBuffer& buffer,
        int txtLen, int endx, int endy, bool pixelOffsets, int PreOffsetLeft, int PreOffsetTop, int text_len, int char_width, int char_height, bool vertical, bool rotate_90) const;
    FontManager& font_mgr;
};
