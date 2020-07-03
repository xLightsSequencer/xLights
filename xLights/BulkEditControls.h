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

#include <wx/slider.h>
#include <wx/textctrl.h>
#include <wx/spinctrl.h>
#include <wx/choice.h>
#include <wx/checkbox.h>
#include <wx/fontpicker.h>
#include "ValueCurveButton.h"
#include "xlLockButton.h"
#include <wx/filepicker.h>
#include <wx/clrpicker.h>

class wxStaticText;

typedef enum
{
    BE_INT,
    BE_FLOAT1,
    BE_FLOAT2,
    BE_FLOAT360
}
BESLIDERTYPE;

class BulkEditSlider : public wxSlider
{
protected:
    long ID_SLIDER_BULKEDIT;
    BESLIDERTYPE _type;
    bool _supportsBulkEdit;
    int _default;

    public:

    BulkEditSlider(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditSlider() {}
    void OnRightDown(wxMouseEvent& event);
    void OnDClick(wxMouseEvent& event);
    void OnSliderPopup(wxCommandEvent &event);
    void OnSlider_SliderUpdated(wxScrollEvent& event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
    void BulkEdit();
};

class BulkEditFontPicker : public wxFontPickerCtrl
{
protected:
    long ID_FONTPICKER_BULKEDIT;
    bool _supportsBulkEdit;

public:

    BulkEditFontPicker(wxWindow* parent,
        wxWindowID id,
        const wxFont& initial = wxNullFont,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFNTP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFontPickerCtrlNameStr);
        virtual ~BulkEditFontPicker() {}
    void OnRightDown(wxMouseEvent& event);
    void OnFontPickerPopup(wxCommandEvent& event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
    std::string GetValue() const;
};

class BulkEditColourPickerCtrl : public wxColourPickerCtrl
{
protected:
    long ID_COLOURPICKER_BULKEDIT;
    bool _supportsBulkEdit;

public:

    BulkEditColourPickerCtrl(wxWindow* parent,
        wxWindowID id,
        const wxColour& initial = *wxBLACK,
        const wxPoint& pos = wxDefaultPosition,
        const wxSize& size = wxDefaultSize,
        long style = wxFNTP_DEFAULT_STYLE,
        const wxValidator& validator = wxDefaultValidator,
        const wxString& name = wxFontPickerCtrlNameStr);
    virtual ~BulkEditColourPickerCtrl() {}
    void OnRightDown(wxMouseEvent& event);
    void OnColourPickerPopup(wxCommandEvent& event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
    wxColour GetValue() const;
    std::string GetStringValue() const;
};

class BulkEditSliderF1 : public BulkEditSlider
{
public:
    BulkEditSliderF1(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditSliderF1() {}
};

class BulkEditSliderF2 : public BulkEditSlider
{
public:
    BulkEditSliderF2(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditSliderF2() {}
};

class BulkEditSliderF360 : public BulkEditSlider
{
public:
    BulkEditSliderF360(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditSliderF360() {}
};

class BulkEditTextCtrl : public wxTextCtrl
{
protected:
    long ID_TEXTCTRL_BULKEDIT;
    BESLIDERTYPE _type;
    bool _supportsBulkEdit;

    public:

    BulkEditTextCtrl(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditTextCtrl() {}
    void OnRightDown(wxMouseEvent& event);
    void OnTextCtrlPopup(wxCommandEvent &event);
    void OnTextCtrl_TextUpdated(wxCommandEvent& event);
    void OnTextCtrl_TextLoseFocus(wxFocusEvent& event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
    void TextUpdate(bool force);
};

class BulkEditFilePickerCtrl : public wxFilePickerCtrl
{
protected:
    long ID_FILEPICKERCTRL_BULKEDIT_FN;
    long ID_FILEPICKERCTRL_BULKEDIT_PN;
    bool _supportsBulkEdit;
    wxString _wildcard;

    public:

    BulkEditFilePickerCtrl(wxWindow *parent, wxWindowID id, const wxString& path, const wxString& message, const wxString& wildcard, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditFilePickerCtrl() {}
    void OnRightDown(wxMouseEvent& event);
    void OnFilePickerCtrlPopup(wxCommandEvent &event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
};

class BulkEditTextCtrlF1 : public BulkEditTextCtrl
{
public:
    BulkEditTextCtrlF1(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditTextCtrlF1() {}
};

class BulkEditTextCtrlF2 : public BulkEditTextCtrl
{
public:
    BulkEditTextCtrlF2(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditTextCtrlF2() {}
};

class BulkEditTextCtrlF360 : public BulkEditTextCtrl
{
public:
    BulkEditTextCtrlF360(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSL_HORIZONTAL, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxSliderNameStr);
    virtual ~BulkEditTextCtrlF360() {}
};

class BulkEditSpinCtrl : public wxSpinCtrl
{
protected:
    long ID_SPINCTRL_BULKEDIT;
    bool _supportsBulkEdit;

public:

    BulkEditSpinCtrl(wxWindow *parent, wxWindowID id = wxID_ANY, const wxString &value = wxEmptyString, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = wxSP_ARROW_KEYS, int min = 0, int max = 100, int initial = 0, const wxString &name = "wxSpinCtrl");
    virtual ~BulkEditSpinCtrl() {}
    void OnRightDown(wxMouseEvent& event);
    void OnSpinCtrlPopup(wxCommandEvent &event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
};

class BulkEditChoice : public wxChoice
{
protected:
    long ID_CHOICE_BULKEDIT;
    bool _supportsBulkEdit;

public:

    BulkEditChoice(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxChoiceNameStr);
    virtual ~BulkEditChoice() {}
    void OnRightDown(wxMouseEvent& event);
    virtual void OnChoicePopup(wxCommandEvent &event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
};

class BulkEditFaceChoice : public BulkEditChoice
{
public:

    BulkEditFaceChoice(wxWindow *parent, wxWindowID id, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, int n = 0, const wxString choices[] = NULL, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxChoiceNameStr);
    virtual ~BulkEditFaceChoice() {}
    void OnRightDown(wxMouseEvent& event);
};

class BulkEditValueCurveButton : public ValueCurveButton
{
protected:
    long ID_VALUECURVE_BULKEDIT;
    bool _supportsBulkEdit;

public:

    BulkEditValueCurveButton(wxWindow *parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = wxBU_AUTODRAW, const wxValidator& validator = wxDefaultValidator, const wxString& name = wxButtonNameStr);
    virtual ~BulkEditValueCurveButton() {}
    void OnRightDown(wxMouseEvent& event);
    void OnValueCurvePopup(wxCommandEvent &event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
};

class BulkEditCheckBox : public wxCheckBox
{
protected:
    long ID_CHECKBOX_BULKEDIT_CHECKED;
    long ID_CHECKBOX_BULKEDIT_UNCHECKED;
    bool _supportsBulkEdit;

public:

    BulkEditCheckBox(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxCheckBoxNameStr);
    virtual ~BulkEditCheckBox() {}
    void OnRightDown(wxMouseEvent& event);
    void OnCheckBoxPopup(wxCommandEvent &event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
};

class BulkEditButton : public wxButton
{
protected:
    long ID_BUTTON_BULKEDIT;
    bool _supportsBulkEdit;

public:

    BulkEditButton(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos = wxDefaultPosition, const wxSize &size = wxDefaultSize, long style = 0, const wxValidator &validator = wxDefaultValidator, const wxString &name = wxCheckBoxNameStr);
    virtual ~BulkEditButton() {}
    void OnRightDown(wxMouseEvent& event);
    void OnButtonPopup(wxCommandEvent &event);
    void SetSupportsBulkEdit(bool supportsBulkEdit) { _supportsBulkEdit = supportsBulkEdit; }
    bool SupportsBulkEdit() const { return  _supportsBulkEdit; }
};


// Helper functions
wxWindow* GetPanel(wxWindow* w);
std::string GetPanelName(wxWindow* w);
std::string FixIdForPanel(std::string panel, std::string id);
bool IsAssociatedControl(wxWindow* source, wxWindow* target);
wxWindow* GetAssociatedWindow(wxWindow* w, wxString ourName, wxString ourType, wxString desiredType);
BulkEditValueCurveButton* GetSettingValueCurveButton(wxWindow* w, std::string ourName, std::string ourType);
wxStaticText* GetSettingLabelControl(wxWindow* w, std::string ourName, std::string ourType);
BulkEditTextCtrl* GetSettingTextControl(wxWindow* w, std::string ourName, std::string ourType);
BulkEditSlider* GetSettingSliderControl(wxWindow* w, std::string ourName, std::string ourType);
bool IsSliderTextPair(wxWindow* w, wxString ourName, wxString ourType);
bool IsBulkEditAvailable(wxWindow* w, bool requireOneElement = false);

