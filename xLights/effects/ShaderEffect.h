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
    SHADER_PARM_AUDIO,
    SHADER_PARM_AUDIOFFT
};

enum class ShaderCtrlType
{
    SHADER_CTRL_STATIC,
    SHADER_CTRL_SLIDER,
    SHADER_CTRL_CHECKBOX,
    SHADER_CTRL_TEXTCTRL
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
    float _min;
    float _max;
    float _default;

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
        }
        wxASSERT(false);
        return "NONAME";
    }
    // These are the labels that will be in the settings map
    wxString GetUndecoratedId(ShaderCtrlType ctrl) const
    {
        return GetId(ctrl).AfterFirst('_');
    }
    wxString GetLabel() const { if (_label != "") return _label; return _name; }
    bool ShowParm() const 
    { 
        return _type == ShaderParmType::SHADER_PARM_FLOAT || _type == ShaderParmType::SHADER_PARM_BOOL; }
};

class ShaderConfig
{
    std::list<ShaderParm> _parms;
    std::string _filename;
    std::string _description;
    std::list<ShaderPass> _passes;
    std::string _code;

public:
    ShaderConfig(const wxString& filename, const wxString& code, const wxString& json);
    std::list<ShaderPass> GetPasses() const { return _passes; }
    std::list<ShaderParm> GetParms() const { return _parms; }
    std::string GetFilename() const { return _filename; }
    std::string GetDescription() const { return _description; }
    std::string GetCode() const { return _code; }
};

class ShaderEffect : public RenderableEffect
{
    public:
        ShaderEffect(int id);
        virtual ~ShaderEffect();
        virtual bool CanBeRandom() override {return false;}
        virtual void Render(Effect *effect, SettingsMap &settings, RenderBuffer &buffer) override;
        virtual bool CanRenderOnBackgroundThread(Effect *effect, const SettingsMap &settings, RenderBuffer &buffer) override { return false; }
        virtual bool SupportsLinearColorCurves(const SettingsMap &SettingsMap) override { return false; }
        virtual void SetDefaultParameters() override;
        virtual bool needToAdjustSettings(const std::string &version) override;
        virtual void adjustSettings(const std::string &version, Effect *effect, bool removeDefaults = true) override;
        virtual std::list<std::string> CheckEffectSettings(const SettingsMap& settings, AudioManager* media, Model* model, Effect* eff) override;
        static ShaderConfig* ParseShader(const std::string& filename);

    protected:
        struct VertexTex
        {
           float v[2];
           float t[2];
        };
        virtual void RemoveDefaults(const std::string &version, Effect *effect) override;
        virtual wxPanel *CreatePanel(wxWindow *parent) override;

        void sizeForRenderBuffer(const RenderBuffer& rb);
        void recompileFromShaderConfig( const ShaderConfig* cfg );

        static bool s_shadersInit;
        static unsigned s_vertexArrayId;
        static unsigned s_vertexBufferId;
        static unsigned s_fbId;
        static unsigned s_rbId;
        static unsigned s_rbTex;
        static unsigned s_programId;
        static int s_rbWidth;
        static int s_rbHeight;
        static std::string s_psCode;
};

#endif // SHADEREFFECT_H
