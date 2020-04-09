
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#pragma once

// These classes have been updated to try to make them
// Const correct
// Take advantage of C++14 && std

#include "wx/wx.h"
#include "wx/filename.h"

#include <vector>
#include <memory>

enum class KBSCOPE
{
    All,
    Setup,
    Layout,
    Sequence,
    Invalid
};

class KeyBinding {
    static int __nextid;
public:
    static std::vector<wxKeyCode>& GetPossibleKeys();
    static std::string EncodeKey(wxKeyCode key, bool shift) noexcept;
    static wxKeyCode DecodeKey(std::string key) noexcept;
    static bool IsShiftedKey(wxKeyCode ch) noexcept;
    static std::string ParseKey(const std::string& key, bool& ctrl, bool& alt, bool& shift) noexcept;

    explicit KeyBinding(wxKeyCode k, bool disabled, const std::string& type, bool control = false, bool alt = false, bool shift = false);
    explicit KeyBinding(const std::string& k, bool disabled, const std::string& type, bool control = false, bool alt = false, bool shift = false);
    explicit KeyBinding(wxKeyCode k, bool disabled, const std::string& name, const std::string& eff, const std::string& ver, bool control = false, bool alt = false, bool shift = false)
        : _type(_("EFFECT")), _effectName(name), _effectString(eff), _effectDataVersion(ver), _control(control), _alt(alt), _shift(shift), _disabled(disabled), _key(k)
    {
        _id = __nextid++;
        _scope = KBSCOPE::Sequence;
        _shift |= IsShiftedKey(_key);
        _tip = "Insert an effect.";
    }
    explicit KeyBinding(const std::string& k, bool disabled, const std::string& name, const std::string& eff, const std::string& ver, bool control = false, bool alt = false, bool shift = false)
        : _type(_("EFFECT")), _effectName(name), _effectString(eff), _effectDataVersion(ver), _control(control), _alt(alt), _shift(shift), _disabled(disabled)
    {
        _id = __nextid++;
        _key = DecodeKey(k);
        if (_key == WXK_NONE) _disabled = true;
        _scope = KBSCOPE::Sequence;
        _shift |= IsShiftedKey(_key);
        _tip = "Insert an effect.";
    }
    explicit KeyBinding(bool disabled, wxKeyCode k, const std::string& presetName, bool control, bool alt, bool shift)
        : _type(_("PRESET")), _effectName(presetName), _control(control), _alt(alt), _shift(shift), _disabled(disabled), _key(k)
    {
        _id = __nextid++;
        _scope = KBSCOPE::Sequence;
        _shift |= IsShiftedKey(_key);
        _tip = "Insert a preset effect.";
    }
    explicit KeyBinding(bool disabled, const std::string& k, const std::string& presetName, bool control, bool alt, bool shift)
        : _type(_("PRESET")), _effectName(presetName), _control(control), _alt(alt), _shift(shift), _disabled(disabled)
    {
        _id = __nextid++;
        _key = DecodeKey(k);
        if (_key == WXK_NONE) _disabled = true;
        _scope = KBSCOPE::Sequence;
        _shift |= IsShiftedKey(_key);
        _tip = "Insert a preset effect.";
    }
	explicit KeyBinding(bool disabled, wxKeyCode k, const std::string& eff, const std::string& ver, bool control, bool alt, bool shift)
		: _type(_("APPLYSETTING")), _control(control), _effectString(eff), _effectDataVersion(ver), _alt(alt), _shift(shift), _disabled(disabled), _key(k)
	{
        _id = __nextid++;
        _scope = KBSCOPE::Sequence;
		_shift |= IsShiftedKey(_key);
        _tip = "Apply setting to selected effects.";
    }
	explicit KeyBinding(bool disabled, const std::string& k, const std::string& eff, const std::string& ver, bool control, bool alt, bool shift)
		: _type(_("APPLYSETTING")), _control(control), _effectString(eff), _effectDataVersion(ver), _alt(alt), _shift(shift), _disabled(disabled)
	{
        _id = __nextid++;
        _key = DecodeKey(k);
		if (_key == WXK_NONE) _disabled = true;
		_scope = KBSCOPE::Sequence;
		_shift |= IsShiftedKey(_key);
        _tip = "Apply setting to selected effects.";
	}
	
    const std::string& GetType() const noexcept { return _type; }
    wxKeyCode GetKey() const noexcept { return _key; }
    const std::string& GetEffectString() const noexcept { return _effectString; }
    const std::string& GetEffectName() const noexcept { return _effectName; }
    const std::string& GetEffectDataVersion() const noexcept { return _effectDataVersion; }
    bool RequiresControl() const noexcept { return _control; }
    bool RequiresAlt() const noexcept { return _alt; }
    bool RequiresShift() const noexcept { return _shift; }
    bool InScope(const KBSCOPE scope) const noexcept { return scope == _scope; }
    bool IsKey(wxKeyCode key) const noexcept { return (_key == key); }
    bool IsDisabled() const noexcept { return _disabled; }
    std::string Description() const noexcept;
    bool IsEquivalentKey(wxKeyCode key) const noexcept;
    int GetId() const noexcept { return _id; }
    void SetControl(bool ctrl) { _control = ctrl; }
    void SetShift(bool shift) { _shift = shift; }
    void SetAlt(bool alt) { _alt = alt; }
    void SetKey(const std::string& key);
    void SetKey(const wxKeyCode key);
    void SetEffectName(const std::string& effect) { _effectName = effect; }
    void SetEffectString(const std::string& effectString) { _effectString = effectString; }
    bool IsDuplicateKey(const KeyBinding& b) const;
    KBSCOPE GetScope() const { return _scope; }
    std::string GetTip() const { return _tip; }

private:
    std::string _type = _("");
    std::string _effectName = _("");
    std::string _effectString = _("");
    std::string _effectDataVersion = _("");
    std::string _tip = _("");
    bool _control = false;
    bool _alt = false;
    bool _shift = false;
    bool _disabled = true;
    KBSCOPE _scope = KBSCOPE::Invalid;
    wxKeyCode _key = WXK_NONE;
    int _id = -1;
};

class KeyBindingMap {
public:

    void LoadDefaults() noexcept;
    void Load(const wxFileName& file) noexcept;
    void Save(const wxFileName& file) const noexcept;
    void Save() const noexcept;
    int AddKey(const KeyBinding& kb);
    void DeleteKey(int id);

    std::shared_ptr<const KeyBinding> Find(const wxKeyEvent& event, KBSCOPE scope) const noexcept;

    std::string Dump() const noexcept;

    std::vector<KeyBinding>& GetBindings() { return _bindings; }
    KeyBinding& GetBinding(int id);
    bool IsDuplicateKey(const KeyBinding& b) const;

private:
    std::vector<KeyBinding> _bindings;
    wxFileName _openedFile;
};
