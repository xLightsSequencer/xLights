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

#include <wx/bmpbndl.h>
#include <string>
#include "../Color.h"
#include "assist/AssistPanel.h"
#include "../graphics/xlGraphicsAccumulators.h"

class wxPanel;
class wxWindow;
class wxBitmap;
class Model;
class SequenceElements;
class Effect;
class SettingsMap;
class RenderBuffer;
class wxSlider;
class wxCheckBox;
class AudioManager;
class wxSpinCtrl;
class xlEffectPanel;
class EffectManager;
class EffectLayer;

class RenderableEffect
{
public:
    RenderableEffect(int id,
                     std::string n,
                     const char** data16,
                     const char** data24,
                     const char** data32,
                     const char** data48,
                     const char** data64);
    virtual ~RenderableEffect();

    virtual const std::string& Name() const
    {
        return name;
    };

    virtual const std::string& ToolTip() const
    {
        return tooltip;
    };

    virtual const wxBitmapBundle& GetEffectIcon(int defSize = 16) const;
    virtual int GetId() const
    {
        return id;
    }
    virtual int GetColorSupportedCount() const
    {
        return -1;
    } // -1 is no limit
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const
    {
        return false;
    }
    virtual bool SupportsRadialColorCurves(const SettingsMap& SettingsMap) const
    {
        return false;
    }
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const
    {
        return std::list<std::string>();
    }
    virtual std::list<std::string> GetFacesUsed(const SettingsMap& SettingsMap) const
    {
        return std::list<std::string>();
    }
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap)
    {
        return false;
    }
    virtual bool AppropriateOnNodes() const
    {
        return true;
    }
    virtual bool CanRenderPartialTimeInterval() const
    {
        return false;
    }
    virtual bool PressButton(const std::string& id, SettingsMap& paletteMap, SettingsMap& settings)
    {
        return false;
    }

    virtual void SetSequenceElements(SequenceElements* els)
    {
        mSequenceElements = els;
    }

    xlEffectPanel* GetPanel(wxWindow* parent);
    virtual void SetDefaultParameters()
    {}
    virtual void SetPanelStatus(Model* cls)
    {}
    virtual wxString GetEffectString();

    // Methods for rendering the effect
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer)
    {
        return true;
    }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const;
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) = 0;
    virtual void RenameTimingTrack(std::string oldname, std::string newname, Effect* effect)
    {}
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache)
    {
        std::list<std::string> res;
        return res;
    };

    virtual bool CanBeRandom()
    {
        return true;
    }

    // return 0 if this is completely drawin the effect background
    // return 1 to have the grid place a normal icon
    // return 2 to have the grid place a smaller icon
    virtual int DrawEffectBackground(const Effect* e, int x1, int y1, int x2, int y2,
                                     xlVertexColorAccumulator& backgrounds, xlColor* colorMask, bool ramps);

    virtual bool needToAdjustSettings(const std::string& version);
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true);

    virtual AssistPanel* GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame);
    virtual bool HasAssistPanel()
    {
        return false;
    }

    static std::string UpgradeValueCurve(EffectManager* effectManager, const std::string& name, const std::string& value, const std::string& effectName);

protected:
    static void SetSliderValue(wxSlider* slider, int value);
    static void SetSpinValue(wxSpinCtrl* spin, int value);
    static void SetChoiceValue(wxChoice* choice, std::string value);
    static void SetRadioValue(wxRadioButton* radio);
    static void SetTextValue(wxTextCtrl* choice, std::string value);
    static void SetCheckBoxValue(wxCheckBox* w, bool b);

    virtual double GetSettingVCMin(const std::string& name) const
    {
        wxASSERT(false);
        return 0.0;
    }
    virtual double GetSettingVCMax(const std::string& name) const
    {
        wxASSERT(false);
        return 100.0;
    }
    virtual int GetSettingVCDivisor(const std::string& name) const
    {
        return 1;
    }

    double GetValueCurveDouble(const std::string& name, double def, const SettingsMap& SettingsMap, float offset, double min, double max, long startMS, long endMS, int divisor = 1);
    int GetValueCurveInt(const std::string& name, int def, const SettingsMap& SettingsMap, float offset, int min, int max, long startMS, long endMS, int divisor = 1);
    int GetValueCurveIntMax(const std::string& name, int def, const SettingsMap& SettingsMap, int min, int max, int divisor = 1);
    EffectLayer* GetTiming(const std::string& timingtrack) const;
    Effect* GetCurrentTiming(const RenderBuffer& buffer, const std::string& timingtrack) const;
    std::string GetTimingTracks(const int maxLayers = 0, const int absoluteLayers = 0) const;
    bool IsVersionOlder(const std::string& compare, const std::string& version);
    void AdjustSettingsToBeFitToTime(int effectIdx, SettingsMap& settings, int startMS, int endMS, xlColorVector& colors);
    virtual void RemoveDefaults(const std::string& version, Effect* effect);

    void initBitmaps(const char** data16,
                     const char** data24,
                     const char** data32,
                     const char** data48,
                     const char** data64);

    virtual xlEffectPanel* CreatePanel(wxWindow* parent) = 0;
    std::string name;
    std::string tooltip;
    int id;
    xlEffectPanel* panel;
    SequenceElements* mSequenceElements;
    wxBitmapBundle icon16;
    wxBitmapBundle icon24;
    wxBitmapBundle icon32;
    wxBitmapBundle icon48;

private:
};
