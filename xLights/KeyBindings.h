//
//  KeyBindings.h
//  xLights
//

#ifndef __xLights__KeyBindings__
#define __xLights__KeyBindings__

#include "wx/wx.h"
#include "wx/filename.h"
#include <vector>
#include <map>

typedef enum 
{
    KBSCOPE_ALL,
    KBSCOPE_SETUP,
    KBSCOPE_LAYOUT,
    KBSCOPE_SEQUENCE
} KBSCOPE;

const std::map<std::string, KBSCOPE> KeyBindingTypes = {
    { "TIMING_ADD", KBSCOPE_SEQUENCE },
    { "TIMING_SPLIT", KBSCOPE_SEQUENCE },
{ "ZOOM_IN", KBSCOPE_SEQUENCE },
{ "ZOOM_OUT", KBSCOPE_SEQUENCE },
{ "RANDOM", KBSCOPE_SEQUENCE },
{ "RENDER_ALL", KBSCOPE_ALL },
{ "SAVE_CURRENT_TAB", KBSCOPE_ALL },
{ "LIGHTS_TOGGLE", KBSCOPE_ALL },
{ "OPEN_SEQUENCE", KBSCOPE_ALL },
{ "CLOSE_SEQUENCE", KBSCOPE_ALL },
{ "NEW_SEQUENCE", KBSCOPE_ALL },
{ "PASTE_BY_CELL", KBSCOPE_ALL },
{ "PASTE_BY_TIME", KBSCOPE_ALL },
{ "SAVEAS_SEQUENCE", KBSCOPE_SEQUENCE },
{   "SAVE_SEQUENCE", KBSCOPE_SEQUENCE },
{   "EFFECT_SETTINGS_TOGGLE", KBSCOPE_SEQUENCE },
{   "EFFECT_ASSIST_TOGGLE", KBSCOPE_SEQUENCE },
{   "COLOR_TOGGLE", KBSCOPE_SEQUENCE },
{   "LAYER_SETTING_TOGGLE", KBSCOPE_SEQUENCE },
{   "LAYER_BLENDING_TOGGLE", KBSCOPE_SEQUENCE },
{   "MODEL_PREVIEW_TOGGLE", KBSCOPE_SEQUENCE },
{   "HOUSE_PREVIEW_TOGGLE", KBSCOPE_SEQUENCE },
{   "EFFECTS_TOGGLE", KBSCOPE_SEQUENCE },
{   "DISPLAY_ELEMENTS_TOGGLE", KBSCOPE_SEQUENCE },
{   "JUKEBOX_TOGGLE", KBSCOPE_SEQUENCE },
{   "SEQUENCE_SETTINGS", KBSCOPE_ALL },
{    "LOCK_EFFECT", KBSCOPE_SEQUENCE },
{"UNLOCK_EFFECT", KBSCOPE_SEQUENCE },
{"MARK_SPOT", KBSCOPE_SEQUENCE },
{"RETURN_TO_SPOT", KBSCOPE_SEQUENCE },
{"EFFECT_DESCRIPTION", KBSCOPE_SEQUENCE },
{"EFFECT_ALIGN_START", KBSCOPE_SEQUENCE },
{"EFFECT_ALIGN_END", KBSCOPE_SEQUENCE },
{"EFFECT_ALIGN_BOTH", KBSCOPE_SEQUENCE },
{"INSERT_LAYER_ABOVE", KBSCOPE_SEQUENCE },
{"INSERT_LAYER_BELOW", KBSCOPE_SEQUENCE },
{"TOGGLE_ELEMENT_EXPAND", KBSCOPE_SEQUENCE },
{"SHOW_PRESETS", KBSCOPE_SEQUENCE },
{"SEARCH_TOGGLE", KBSCOPE_SEQUENCE },
{"PERSPECTIVES_TOGGLE", KBSCOPE_SEQUENCE },
{"PLAY_LOOP", KBSCOPE_ALL },
{   "PLAY", KBSCOPE_ALL },
{  "STOP", KBSCOPE_ALL },
{ "PAUSE", KBSCOPE_ALL },
{ "EFFECT", KBSCOPE_SEQUENCE },
{ "LOCK_MODEL", KBSCOPE_LAYOUT},
{ "UNLOCK_MODEL", KBSCOPE_LAYOUT },
{ "GROUP_MODELS", KBSCOPE_LAYOUT},
{ "WIRING_VIEW", KBSCOPE_LAYOUT},
{ "NODE_LAYOUT", KBSCOPE_LAYOUT},
{ "SAVE_LAYOUT", KBSCOPE_LAYOUT},
{ "MODEL_ALIGN_TOP", KBSCOPE_LAYOUT},
{ "MODEL_ALIGN_BOTTOM", KBSCOPE_LAYOUT},
{ "MODEL_ALIGN_LEFT", KBSCOPE_LAYOUT},
{ "MODEL_ALIGN_RIGHT", KBSCOPE_LAYOUT},
{ "MODEL_ALIGN_CENTER_VERT", KBSCOPE_LAYOUT},
{ "MODEL_ALIGN_CENTER_HORIZ", KBSCOPE_LAYOUT},
{ "MODEL_DISTRIBUTE_HORIZ", KBSCOPE_LAYOUT},
{ "MODEL_DISTRIBUTE_VERT", KBSCOPE_LAYOUT}
};

class KeyBinding {
public:
    static wxString EncodeKey(wxKeyCode key, bool shift);
    static wxKeyCode DecodeKey(wxString key);
    static bool IsShiftedKey(wxKeyCode ch);

    KeyBinding(wxKeyCode k, bool disabled, std::string type, bool control = false, bool alt = false, bool shift = false) : 
    _key(k), _disabled(disabled), _type(type), _effectName(""), _effectString(""), _effectDataVersion(""), _control(control), 
    _alt(alt), _shift(shift) {
        _scope = KeyBindingTypes.at(type);
        if (IsShiftedKey(_key))
        {
            _shift = true;
        }
    }
    KeyBinding(wxString k, bool disabled, std::string type, bool control = false, bool alt = false, bool shift = false) : 
    _disabled(disabled), _type(type), _effectName(""), _effectString(""), _effectDataVersion(""), _control(control), 
    _alt(alt), _shift(shift) {
        _key = DecodeKey(k);
        if (_key == WXK_NONE) _disabled = true;
        _scope = KeyBindingTypes.at(type);
        if (IsShiftedKey(_key))
        {
            _shift = true;
        }
    }
    KeyBinding(wxKeyCode k, bool disabled, const wxString &name, const wxString &eff, const wxString &ver, bool control = false, bool alt = false, bool shift = false)
        : _key(k), _disabled(disabled), _type("EFFECT"), _effectName(name), _effectString(eff), _effectDataVersion(ver), _control(control), _alt(alt), _shift(shift)
    {
        _scope = KeyBindingTypes.at(_type);
        if (IsShiftedKey(_key))
        {
            _shift = true;
        }
    }
    KeyBinding(wxString k, bool disabled, const wxString &name, const wxString &eff, const wxString &ver, bool control = false, bool alt = false, bool shift = false)
        : _disabled(disabled), _type("EFFECT"), _effectName(name), _effectString(eff), _effectDataVersion(ver), _control(control), _alt(alt), _shift(shift)
    {
        _key = DecodeKey(k);
        if (_key == WXK_NONE) _disabled = true;
        _scope = KeyBindingTypes.at(_type);
        if (IsShiftedKey(_key))
        {
            _shift = true;
        }
    }
    
    wxString GetType() const { return _type; }
    wxKeyCode GetKey() const { return _key; }
    const std::string &GetEffectString() const { return _effectString;}
    const std::string &GetEffectName() const { return _effectName;}
    const std::string &GetEffectDataVersion() const { return _effectDataVersion;}
    bool RequiresControl() const { return _control; }
    bool RequiresAlt() const { return _alt; }
    bool RequiresShift() const { return _shift; }
    bool InScope(const KBSCOPE scope) const { return scope == _scope; }
    bool IsDisabled() const { return _disabled; }
    std::string Description() const;

private:
    wxKeyCode _key;
    std::string _type;
    std::string _effectName;
    std::string _effectString;
    std::string _effectDataVersion;
    bool _control;
    bool _alt;
    bool _shift;
    KBSCOPE _scope;
    bool _disabled;
};

class KeyBindingMap {
public:
    KeyBindingMap() {}
    
    void LoadDefaults();

    void Load(wxFileName &file);
    void Save(wxFileName &file);
    
    KeyBinding *Find(wxKeyEvent& event, KBSCOPE scope);

    std::string Dump();
    
private:
    std::vector<KeyBinding> bindings;
};

#endif /* defined(__xLights__KeyBindings__) */
