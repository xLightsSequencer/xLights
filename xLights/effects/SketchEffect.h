#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "RenderableEffect.h"

class wxImage;

class SketchAssistPanel;
class SketchEffectSketch;
class SketchPanel;

class SketchEffect : public RenderableEffect
{
public: 
    SketchEffect( int id );
    virtual ~SketchEffect();

    bool CanBeRandom() override
    {
        return false;
    }
    void Render( Effect* effect, const SettingsMap& settings, RenderBuffer& buffer ) override;

    void SetDefaultParameters() override;
    bool needToAdjustSettings( const std::string& version ) override;
    void adjustSettings( const std::string& version, Effect* effect, bool removeDefaults = true ) override;
    std::list<std::string> CheckEffectSettings( const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache ) override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;

    AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) override;
    bool HasAssistPanel() override
    {
        return true;
    }

    virtual double GetSettingVCMin(const std::string& name) const override;
    virtual double GetSettingVCMax(const std::string& name) const override;

protected:
    void RemoveDefaults( const std::string& version, Effect* effect ) override;
    xlEffectPanel* CreatePanel( wxWindow* parent ) override;

    void renderSketch(const SketchEffectSketch& sketch,
                      wxImage& img, double progress,
                      double drawPercentage, int lineThickness, bool hasMotion, double motionPercentage,
                      const xlColorVector& colors);

    void updateSketchAssistBackground() const;

    // Since SketchEffect and SketchPanel are more-or-less singletons,
    // having a raw pointer to the effect's panel is sufficient
    SketchPanel* m_panel = nullptr;

    // Assist panels come and go and are owned by the window passed into
    // GetAssistPanel()... so we just store the latest one as a weak_ptr
    SketchAssistPanel* m_sketchAssistPanel = nullptr;
};
