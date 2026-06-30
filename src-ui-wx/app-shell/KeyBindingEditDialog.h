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

//(*Headers(KeyBindingEditDialog)
#include <wx/button.h>
#include <wx/choice.h>
#include <wx/dialog.h>
#include <wx/listctrl.h>
#include <wx/panel.h>
#include <wx/propgrid/propgrid.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
//*)

class KeyBindingMap;
class KeyBinding;
class EffectManager;
class xLightsFrame;
class wxSearchCtrl;

class KeyBindingEditDialog : public wxDialog
{
    EffectManager* _effectManager = nullptr;
    KeyBindingMap* _keyBindings = nullptr;
    wxPropertyGrid* _propertyGrid = nullptr;
    xLightsFrame* _xLights = nullptr;

    void LoadList();
    wxString BuildDetails(const KeyBinding& b) const;
    void RefreshDuplicateHighlights();
    void DoEditSelected();
    void RefreshRow(long index, const KeyBinding& b);
    void UpdateEditEnabled();
    void SetKeyBindingProperties();
    int GetSelectedKeyBindingIndex() const;
    void SelectKey(int id);

    // Display helpers for the bindings list.
    static wxString RenderShortcut(const KeyBinding& b);
    void OnListMouseMotion(wxMouseEvent& event);
    long _tooltipItem = -1;

    wxSearchCtrl* _filterCtrl = nullptr;
    wxString _filter; // lower-cased; whitespace-tokenised AND match in LoadList
    wxButton* _editButton = nullptr; // disabled when nothing is selected

public:
    // Stable window name used to find an already-open instance (type-based
    // lookup is unreliable here - wxDialog subclasses share RTTI in this build).
    static constexpr const char* WINDOW_NAME = "xlKeyBindingEditDialog";

    // Public so the popup editor can label a binding with its friendly name.
    static wxString FriendlyName(const std::string& type);

    KeyBindingEditDialog(xLightsFrame* parent, KeyBindingMap* keyBindings, EffectManager* effectManager, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~KeyBindingEditDialog();

    //(*Declarations(KeyBindingEditDialog)
    wxButton* ButtonAddApplySetting;
    wxButton* ButtonAddPreset;
    wxButton* ButtonSave;
    wxButton* Button_AddEffect;
    wxButton* Button_Close;
    wxChoice* Choice_Scope;
    wxFlexGridSizer* FlexGridSizer3;
    wxListCtrl* ListCtrl_Bindings;
    wxPanel* Panel_Properties;
    wxStaticText* StaticText1;
    //*)

protected:
    //(*Identifiers(KeyBindingEditDialog)
    static const long ID_STATICTEXT1;
    static const long ID_CHOICE1;
    static const long ID_LISTCTRL1;
    static const long ID_PANEL1;
    static const long ID_BUTTON1;
    static const long ID_BUTTON3;
    static const long ID_BUTTON2;
    static const long ID_BUTTON_SAVE;
    static const long ID_BUTTON_CANCEL;
    //*)

private:
    //(*Handlers(KeyBindingEditDialog)
    void OnButton_CancelClick(wxCommandEvent& event);
    void OnChoice_ScopeSelect(wxCommandEvent& event);
    void OnListCtrl_BindingsItemFocused(wxListEvent& event);
    void OnListCtrl_BindingsItemSelect(wxListEvent& event);
    void OnListCtrl_BindingsKeyDown(wxListEvent& event);
    void OnListCtrl_BindingsDeleteItem(wxListEvent& event);
    void OnButton_AddEffectClick(wxCommandEvent& event);
    void OnButtonAddApplySettingClick(wxCommandEvent& event);
    void OnButtonAddPresetClick(wxCommandEvent& event);
    void OnButtonSaveClick(wxCommandEvent& event);
    //*)

    void OnControllerPropertyGridChange(wxPropertyGridEvent& event);
    void OnClose(wxCloseEvent& event);

    DECLARE_EVENT_TABLE()
};
