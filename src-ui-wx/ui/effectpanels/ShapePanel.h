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

#include "JsonEffectPanel.h"

#include <list>
#include <string>

class wxStaticText;
class wxFontPickerCtrl;
class wxSpinCtrl;
class wxChoice;
class wxContextMenuEvent;
class MediaPickerCtrl;

class ShapePanelEmoji {
public:
    ShapePanelEmoji(std::string name, std::string font, int c)
        : _name(std::move(name)), _font(std::move(font)), _c(c) {}
    const std::string& GetName() const { return _name; }
    const std::string& GetFont() const { return _font; }
    int GetChar() const { return _c; }
private:
    std::string _name;
    std::string _font;
    int _c;
};

class ShapePanel : public JsonEffectPanel {
public:
    ShapePanel(wxWindow* parent, const nlohmann::json& metadata);
    ~ShapePanel() override;
    void ValidateWindow() override;
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer, const nlohmann::json& prop, int cols) override;

private:
    void PopulateEmojiList();
    void OnEmojiContextMenu(wxContextMenuEvent& event);
    void OnEmojiMenuItem(wxCommandEvent& event);

    wxStaticText* _emojiDisplay = nullptr;
    wxFontPickerCtrl* _fontPicker = nullptr;
    wxSpinCtrl* _charSpin = nullptr;
    wxChoice* _skinToneChoice = nullptr;
    MediaPickerCtrl* _svgPicker = nullptr;
    std::list<ShapePanelEmoji*> _emojis;
};
