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
class EffectManager;
class xLightsFrame;

class KeyBindingEditDialog : public wxDialog
{
    EffectManager* _effectManager = nullptr;
    KeyBindingMap* _keyBindings = nullptr;
    wxPropertyGrid* _propertyGrid = nullptr;
    xLightsFrame* _xLights = nullptr;

    void LoadList();
    void SetKeyBindingProperties();
    int GetSelectedKeyBindingIndex() const;
    void SelectKey(int id);

public:
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

    DECLARE_EVENT_TABLE()
};
