#pragma once

#include "RenderableEffect.h"

class wxImage;

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
    void Render( Effect* effect, SettingsMap& settings, RenderBuffer& buffer ) override;
    bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override
    {
        return false;
    }
    void SetDefaultParameters() override;
    bool needToAdjustSettings( const std::string& version ) override;
    void adjustSettings( const std::string& version, Effect* effect, bool removeDefaults = true ) override;
    std::list<std::string> CheckEffectSettings( const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache ) override;

    AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) override;
    bool HasAssistPanel() override
    {
        return true;
    }

protected:
    void RemoveDefaults( const std::string& version, Effect* effect ) override;
    xlEffectPanel* CreatePanel( wxWindow* parent ) override;

    void renderSketch(const SketchEffectSketch& sketch,
                      wxImage& img, double progress,
                      double drawPercentage, int lineThickness, bool hasMotion, double motionPercentage,
                      const xlColorVector& colors);

    SketchPanel* m_panel = nullptr;
};
