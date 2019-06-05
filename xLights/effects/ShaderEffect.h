#ifndef SHADEREFFECT_H
#define SHADEREFFECT_H

#include "RenderableEffect.h"

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
    SHADER_CTRL_VALUECURVE
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
    float _min = 0.0f;
    float _max = 0.0f;
    float _default = 0.0f;
    wxRealPoint _minPt = { 0,0 };
    wxRealPoint _maxPt = { 0,0 };
    wxRealPoint _defaultPt = { 0,0 };
    std::map<int, wxString> _valueOptions;

    std::vector<wxString> GetChoices() const
    {
        std::vector<wxString> res;

        for (auto it : _valueOptions)
        {
            res.push_back(it.second);
        }

        return res;
    }

    int EncodeChoice(const wxString& value)
    {
        for (auto it : _valueOptions)
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

    ShaderParm(const wxString& name, const wxString& label, ShaderParmType type, float min, float max, float dfault)
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
            return wxString::Format("ID_CHECKBOX_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_SLIDER:
            return wxString::Format("ID_SLIDER_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_TEXTCTRL:
            return wxString::Format("IDD_TEXTCTRL_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_STATIC:
            return wxString::Format("ID_STATICTEXT_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_VALUECURVE:
            return wxString::Format("ID_VALUECURVE_%s", _name);
        case ShaderCtrlType::SHADER_CTRL_CHOICE:
            return wxString::Format("ID_CHOICE_%s", _name);
        }
        wxASSERT(false);
        return "NONAME";
    }
    // These are the labels that will be in the settings map
    wxString GetUndecoratedId(ShaderCtrlType ctrl) const
    {
        if (ctrl == ShaderCtrlType::SHADER_CTRL_VALUECURVE) return _name;

        return GetId(ctrl).AfterFirst('_');
    }
    wxString GetLabel() const { if (_label != "") return _label; return _name; }
    bool ShowParm() const 
    { 
        return _type == ShaderParmType::SHADER_PARM_FLOAT || 
            _type == ShaderParmType::SHADER_PARM_BOOL || 
            _type == ShaderParmType::SHADER_PARM_LONGCHOICE ||
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
    bool _hasRendersize = false;
    bool _hasTime = false;

public:
    ShaderConfig(const wxString& filename, const wxString& code, const wxString& json);
    std::list<ShaderPass> GetPasses() const { return _passes; }
    std::list<ShaderParm> GetParms() const { return _parms; }
    std::string GetFilename() const { return _filename; }
    std::string GetDescription() const { return _description; }
    std::string GetCode() const { return _code; }
    bool IsCanvasShader() const { return _canvasMode; }
    bool HasRendersize() const { return _hasRendersize; }
    bool HasTime() const { return _hasTime; }
};

class ShaderEffect : public RenderableEffect
{
public:
    ShaderEffect(int id);
    virtual ~ShaderEffect();
    virtual bool CanBeRandom() override { return false; }
    virtual void Render(Effect* effect, SettingsMap& settings, RenderBuffer& buffer) override;
    virtual bool CanRenderOnBackgroundThread(Effect* effect, const SettingsMap& settings, RenderBuffer& buffer) override { return false; }
    virtual bool SupportsLinearColorCurves(const SettingsMap& SettingsMap) override { return false; }
    virtual void SetDefaultParameters() override;
    static ShaderConfig* ParseShader(const std::string& filename);
    virtual std::list<std::string> GetFileReferences(const SettingsMap& SettingsMap) override;
    virtual bool CleanupFileLocations(xLightsFrame* frame, SettingsMap& SettingsMap) override;
    virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;

protected:
    virtual void RemoveDefaults(const std::string& version, Effect* effect) override;
    virtual wxPanel* CreatePanel(wxWindow* parent) override;

    void sizeForRenderBuffer(const RenderBuffer& rb,
        bool& s_shadersInit,
        unsigned& s_vertexArrayId, unsigned& s_vertexBufferId, unsigned& s_rbId, unsigned& s_fbId,
        unsigned& s_rbTex, int& s_rbWidth, int& s_rbHeight);
    void recompileFromShaderConfig(const ShaderConfig* cfg, unsigned& s_programId);

    typedef struct _VertexTex
    {
        float v[2];
        float t[2];
    } VertexTex;
};
#endif // SHADEREFFECT_H
