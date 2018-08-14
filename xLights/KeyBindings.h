//
//  KeyBindings.h
//  xLights
//

#ifndef __xLights__KeyBindings__
#define __xLights__KeyBindings__

#include "wx/wx.h"
#include "wx/filename.h"
#include <vector>

const std::string KeyBindingTypes[] =  {
    "TIMING_ADD",
    "TIMING_SPLIT",
    "ZOOM_IN",
    "ZOOM_OUT",
    "RANDOM",
    "RENDER_ALL",
    "LIGHTS_TOGGLE",
    "OPEN_SEQUENCE",
    "CLOSE_SEQUENCE",
    "SAVE_SEQUENCE",
    "NEW_SEQUENCE",
    "SAVEAS_SEQUENCE",
    "PASTE_BY_CELL",
    "PASTE_BY_TIME",
    "EFFECT_SETTINGS_TOGGLE",
    "EFFECT_ASSIST_TOGGLE",
    "COLOR_TOGGLE",
    "LAYER_SETTING_TOGGLE",
    "LAYER_BLENDING_TOGGLE",
    "MODEL_PREVIEW_TOGGLE",
    "HOUSE_PREVIEW_TOGGLE",
    "EFFECTS_TOGGLE",
    "DISPLAY_ELEMENTS_TOGGLE",
    "JUKEBOX_TOGGLE",
    "SEQUENCE_SETTINGS",
    "PLAY_LOOP",
    "PLAY",
    "STOP",
    "PAUSE",
    "EFFECT"
};

class KeyBinding {
    bool IsShiftedKey(unsigned char ch);
    void FixKey()
    {
        if (_key >= 'a' && _key <= 'z')
        {
            _key -= 32;
            _shift = false;
        }
        else if ((_key >= 'A' && _key <= 'Z') || IsShiftedKey(_key))
        {
            _shift = true;
        }
    }
public:
    KeyBinding(unsigned char k, std::string type, bool control = false, bool alt = false) : 
    _key(k), _type(type), _effectName(""), _effectString(""), _effectDataVersion(""), _control(control), 
    _alt(alt), _shift(false) {
        FixKey();
    }
    KeyBinding(unsigned char k, const wxString &name, const wxString &eff, const wxString &ver, bool control = false, bool alt = false)
        : _key(k), _type("EFFECT"), _effectName(name), _effectString(eff), _effectDataVersion(ver), _control(control), _alt(alt), _shift(false)
    {
        FixKey();
    }
    
    wxString GetType() const { return _type; }
    unsigned char GetKey() const { return _key; }
    const std::string &GetEffectString() const { return _effectString;}
    const std::string &GetEffectName() const { return _effectName;}
    const std::string &GetEffectDataVersion() const { return _effectDataVersion;}
    bool RequiresControl() const { return _control; }
    bool RequiresAlt() const { return _alt; }
    bool RequiresShift() const { return _shift; }
    
    
private:
    unsigned char _key;
    std::string _type;
    std::string _effectName;
    std::string _effectString;
    std::string _effectDataVersion;
    bool _control;
    bool _alt;
    bool _shift;
};

class KeyBindingMap {
public:
    KeyBindingMap() {}
    
    void LoadDefaults();
    
    void Load(wxFileName &file);
    void Save(wxFileName &file);
    
    KeyBinding *Find(wxKeyEvent& event);
    KeyBinding *Find(unsigned char key, bool control, bool alt);
    
private:
    std::vector<KeyBinding> bindings;
};

#endif /* defined(__xLights__KeyBindings__) */