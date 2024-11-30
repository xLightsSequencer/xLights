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
#include "../UtilFunctions.h"

class SequenceElements;

#define SHADER_SPEED_MIN -1000
#define SHADER_SPEED_MAX 1000
#define SHADER_SPEED_DIVISOR 100

#define SHADER_OFFSET_X_MIN -100
#define SHADER_OFFSET_X_MAX 100

#define SHADER_OFFSET_Y_MIN -100
#define SHADER_OFFSET_Y_MAX 100

#define SHADER_ZOOM_MIN -100
#define SHADER_ZOOM_MAX 100

enum class ShaderParmType
{
    SHADER_PARM_IMAGE,
    SHADER_PARM_FLOAT,
    SHADER_PARM_EVENT,
    SHADER_PARM_COLOUR,
    SHADER_PARM_BOOL,
    SHADER_PARM_POINT2D,
    SHADER_PARM_LONG,
    SHADER_PARM_LONGCHOICE,
    SHADER_PARM_AUDIO,
    SHADER_PARM_AUDIOFFT
};

enum class ShaderCtrlType
{
    SHADER_CTRL_STATIC,
    SHADER_CTRL_SLIDER,
    SHADER_CTRL_CHECKBOX,
    SHADER_CTRL_TEXTCTRL,
    SHADER_CTRL_CHOICE,
    SHADER_CTRL_VALUECURVE,
    SHADER_CTRL_TIMING
};

struct ShaderPass
{
    wxString _target;
    bool _persistent;
};

struct ShaderParm
{
    wxString _name;
    wxString _label;
    ShaderParmType _type;
    double _min = 0.0f;
    double _max = 0.0f;
    double _default = 0.0f;
    wxRealPoint _minPt = { 0,0 };
    wxRealPoint _maxPt = { 0,0 };
    wxRealPoint _defaultPt = { 0,0 };
    std::map<int, wxString> _valueOptions;

    std::vector<wxString> GetChoices() const
    {
        std::vector<wxString> res;

        for (const auto& it : _valueOptions)
        {
            res.push_back(it.second);
        }

        return res;
    }

    int EncodeChoice(const wxString& value) const
    {
        for (const auto& it : _valueOptions)
        {
            if (it.second == value) return it.first;
        }
        return -1;
    }

    ShaderParm(const wxString& name, const wxString& label, ShaderParmType type)
    {
        _name = name;
        _label = label;
        _type = type;
    }

    ShaderParm(const wxString& name, const wxString& label, ShaderParmType type, double min, double max, double dfault)
    {
        _name = name;
        _label = label;
        _type = type;
        _min = min;
        _max = max;
        _default = dfault;
    }

    ShaderParm(const wxString& name, const wxString& label, ShaderParmType type, wxRealPoint min, wxRealPoint max, wxRealPoint dfault)
    {
        _name = name;
        _label = label;
        _type = type;
        _minPt = min;
        _maxPt = max;
        _defaultPt = dfault;
    }

    wxString GetId(ShaderCtrlType ctrl) const
    {
        switch (ctrl)
        {
        case ShaderCtrlType::SHADER_CTRL_CHECKBOX:
            return wxString::Format("ID_CHECKBOX_SHADERXYZZY_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_SLIDER:
            return wxString::Format("ID_SLIDER_SHADERXYZZY_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_TEXTCTRL:
            return wxString::Format("IDD_TEXTCTRL_SHADERXYZZY_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_STATIC:
            return wxString::Format("ID_STATICTEXT_SHADERXYZZY_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_VALUECURVE:
            return wxString::Format("ID_VALUECURVE_SHADERXYZZY_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_CHOICE:
            return wxString::Format("ID_CHOICE_SHADERXYZZY_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_TIMING:
            return wxString::Format("ID_CHOICE_SHADERXYZZY_%s", _name);
        }
        wxASSERT(false);
        return "NONAME";
    }
    // These are the labels that will be in the settings map
    wxString GetUndecoratedId(ShaderCtrlType ctrl) const
    {
        if (ctrl == ShaderCtrlType::SHADER_CTRL_VALUECURVE) return "SHADERXYZZY_" + _name;

        return GetId(ctrl).AfterFirst('_');
    }
    wxString GetLabel() const { if (_label != "") return _label; return _name; }
    bool ShowParm() const
    {
        return _type == ShaderParmType::SHADER_PARM_FLOAT ||
            _type == ShaderParmType::SHADER_PARM_BOOL ||
            _type == ShaderParmType::SHADER_PARM_LONGCHOICE ||
            _type == ShaderParmType::SHADER_PARM_EVENT ||
            _type == ShaderParmType::SHADER_PARM_POINT2D;
    }
};

class ShaderConfig
{
    std::list<ShaderParm> _parms;
    std::string _filename;
    std::string _description;
    std::list<ShaderPass> _passes;
    std::string _code;
    bool _canvasMode = false;
    bool _audioFFTMode = false;
    bool _audioIntensityMode = false;
    bool _hasRendersize = false;
    bool _hasTime = false;
    bool _hasCoord = false;

public:
    ShaderConfig(const wxString& filename, const wxString& code, const wxString& json, SequenceElements* sequenceElements);
    const std::list<ShaderPass> &GetPasses() const { return _passes; }
    const std::list<ShaderParm> &GetParms() const { return _parms; }
    const std::string &GetFilename() const { return _filename; }
    const std::string &GetDescription() const { return _description; }
    const std::string &GetCode() const { return _code; }
    bool IsCanvasShader() const { return _canvasMode; }
    bool IsAudioFFTShader() const { return _audioFFTMode; }
    bool IsAudioIntensityShader() const { return _audioIntensityMode; }
    bool HasRendersize() const { return _hasRendersize; }
    bool HasTime() const { return _hasTime; }
    bool HasCoord() const { return _hasCoord; }
    bool UsesEvents() const;
};
class ShaderRenderCache;

class ShaderEffect : public RenderableEffect
{
public:
    ShaderEffect(int id);
    virtual ~ShaderEffect();
    virtual bool CanBeRandom() override { return false; }
    virtual bool AppropriateOnNodes() const override { return false; }
    virtual void Render(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) const override { return false; }
    virtual bool SupportsRenderCache(const SettingsMap& settings) const override { return true; }
    virtual void SetDefaultParameters() override;
    virtual std::list<std::string> GetFileReferences(Model* model, const SettingsMap& SettingsMap) const override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff, bool renderCache) override;
    virtual bool needToAdjustSettings(const std::string& version) override;
    virtual void adjustSettings(const std::string& version, Effect* effect, bool removeDefaults = true) override;
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override;

    static ShaderConfig* ParseShader(const std::string& filename, SequenceElements* sequenceElements);
    static bool IsShaderFile(std::string filename);

    static void SetBackgroundRender(bool b) { useBackgroundRender = b; }
    static bool IsBackgroundRender() { return useBackgroundRender; }

    virtual double GetSettingVCMin(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Shader_Speed")
            return SHADER_SPEED_MIN;
        if (name == "E_VALUECURVE_Shader_Offset_X")
            return SHADER_OFFSET_X_MIN;
        if (name == "E_VALUECURVE_Shader_Offset_Y")
            return SHADER_OFFSET_Y_MIN;
        if (name == "E_VALUECURVE_Shader_Zoom")
            return SHADER_ZOOM_MIN;
        return RenderableEffect::GetSettingVCMin(name);
    }

    virtual double GetSettingVCMax(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_Shader_Speed")
            return SHADER_SPEED_MAX;
        if (name == "E_VALUECURVE_Shader_Offset_X")
            return SHADER_OFFSET_X_MAX;
        if (name == "E_VALUECURVE_Shader_Offset_Y")
            return SHADER_OFFSET_Y_MAX;
        if (name == "E_VALUECURVE_Shader_Zoom")
            return SHADER_ZOOM_MAX;
        return RenderableEffect::GetSettingVCMax(name);
    }

    virtual int GetSettingVCDivisor(const std::string& name) const override
    {
        if (name == "E_VALUECURVE_")
            return SHADER_SPEED_DIVISOR;
        // this tells the caller these settings cannot be scaled and min/max/divisor are unknown
        if (StartsWith(name, "E_"))
            return 0xFFFF;
        return RenderableEffect::GetSettingVCDivisor(name);
    }
    static unsigned programIdForShaderCode(ShaderConfig* cfg, ShaderRenderCache *cache);

protected:
    bool SetGLContext(ShaderRenderCache*);
    void UnsetGLContext(ShaderRenderCache*);

    virtual void RemoveDefaults(const std::string& version, Effect* effect) override;
    virtual xlEffectPanel* CreatePanel(wxWindow* parent) override;

    void sizeForRenderBuffer(const RenderBuffer& rb,
        bool& s_shadersInit,
        unsigned& s_vertexArrayId, unsigned& s_vertexBufferId, unsigned& s_rbId, unsigned& s_fbId,
        unsigned& s_rbTex, int& s_rbWidth, int& s_rbHeight);

    struct VertexTex
    {
        float v[2];
        float t[2];
    };

    static bool useBackgroundRender;
};
