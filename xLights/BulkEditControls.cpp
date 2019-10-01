#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/choicdlg.h>

#include "BulkEditControls.h"
#include "xLightsMain.h"
#include "xLightsApp.h"
#include "EffectsPanel.h"
#include "sequencer/MainSequencer.h"
#include "BulkEditSliderDialog.h"
#include "BulkEditFontPickerDialog.h"
#include "UtilFunctions.h"

#include <log4cpp/Category.hh>

// Here is how the sliders, labels, value curves and text boxes all work together.
//
// ID_ vs IDD_ ... whichever is ID_ is the value that will be stored as the setting. The IDD is ignored
// All sliders start with ID[D]_SLIDER
// All text boxes start with ID[D]_TEXTCTRL
// All labels start with ID_STATICTEXT
// All value curve buttons start with ID_VALUECURVE
// Whatever occurs after the above strings must match for the controls to be associated and work correctly
// Even for associated controls where you dont want bulk edit you can still use these classes

#pragma region Bulk Edit Control Constructors
BulkEditSlider::BulkEditSlider(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxSlider(parent, id, value, minValue, maxValue, pos, size, style, validator, name)
{
    _default = value;
    _supportsBulkEdit = true;
    _type = BESLIDERTYPE::BE_INT;
    ID_SLIDER_BULKEDIT = wxNewId();
    Connect(wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&BulkEditSlider::OnSlider_SliderUpdated);
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditSlider::OnRightDown, nullptr, this);
    Connect(wxEVT_RIGHT_DCLICK, (wxObjectEventFunction)& BulkEditSlider::OnDClick, nullptr, this);
}

BulkEditFontPicker::BulkEditFontPicker(wxWindow* parent, wxWindowID id, const wxFont& initial, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name)
    : wxFontPickerCtrl(parent, id, initial, pos, size, style, validator, name)
{
    _supportsBulkEdit = true;
    ID_FONTPICKER_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)& BulkEditFontPicker::OnRightDown, nullptr, this);
    this->GetPickerCtrl()->Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)& BulkEditFontPicker::OnRightDown, nullptr, this);
}

BulkEditTextCtrl::BulkEditTextCtrl(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
    _supportsBulkEdit = true;
    _type = BESLIDERTYPE::BE_INT;
    ID_TEXTCTRL_BULKEDIT = wxNewId();
    Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&BulkEditTextCtrl::OnTextCtrl_TextUpdated);
    Connect(wxEVT_KILL_FOCUS, (wxObjectEventFunction)&BulkEditTextCtrl::OnTextCtrl_TextLoseFocus);
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditTextCtrl::OnRightDown, nullptr, this);
}

BulkEditFilePickerCtrl::BulkEditFilePickerCtrl(wxWindow *parent, wxWindowID id, const wxString& path, const wxString& message, const wxString& wildcard, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : 
    wxFilePickerCtrl(parent, id, path, message, wildcard, pos, size, style, validator, name)
{
    _wildcard = wildcard;
    _supportsBulkEdit = true;
    ID_FILEPICKERCTRL_BULKEDIT_FN = wxNewId();
    ID_FILEPICKERCTRL_BULKEDIT_PN = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditFilePickerCtrl::OnRightDown, nullptr, this);
    this->GetTextCtrl()->Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditFilePickerCtrl::OnRightDown, nullptr, this);
}

BulkEditSpinCtrl::BulkEditSpinCtrl(wxWindow *parent, wxWindowID id, const wxString &value, const wxPoint &pos, const wxSize &size, long style, int min, int max, int initial, const wxString &name) : wxSpinCtrl(parent, id, value, pos, size, style, min, max, initial, name)
{
    _supportsBulkEdit = true;
    ID_SPINCTRL_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditSpinCtrl::OnRightDown, nullptr, this);
}

BulkEditValueCurveButton::BulkEditValueCurveButton(wxWindow *parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : ValueCurveButton(parent, id, bitmap, pos, size, style, validator, name)
{
    _supportsBulkEdit = true;
    ID_VALUECURVE_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditValueCurveButton::OnRightDown, nullptr, this);
}

BulkEditButton::BulkEditButton(wxWindow *parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxButton(parent, id, label, pos, size, style, validator, name)
{
    _supportsBulkEdit = true;
    ID_BUTTON_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditButton::OnRightDown, nullptr, this);
}

BulkEditChoice::BulkEditChoice(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int n, const wxString choices[], long style, const wxValidator &validator, const wxString &name) : wxChoice(parent, id, pos, size, n, choices, style, validator, name)
{
    _supportsBulkEdit = true;
    ID_CHOICE_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditChoice::OnRightDown, nullptr, this);
}

BulkEditFaceChoice::BulkEditFaceChoice(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int n, const wxString choices[], long style, const wxValidator &validator, const wxString &name) : BulkEditChoice(parent, id, pos, size, n, choices, style, validator, name)
{
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditFaceChoice::OnRightDown, nullptr, this);
}

BulkEditCheckBox::BulkEditCheckBox(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxCheckBox(parent, id, label, pos, size, style, validator, name)
{
    _supportsBulkEdit = true;
    ID_CHECKBOX_BULKEDIT_CHECKED = wxNewId();
    ID_CHECKBOX_BULKEDIT_UNCHECKED = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditCheckBox::OnRightDown, nullptr, this);
}

BulkEditSliderF1::BulkEditSliderF1(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditSlider(parent, id, value, minValue, maxValue, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT1;
}

BulkEditTextCtrlF1::BulkEditTextCtrlF1(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditTextCtrl(parent, id, value, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT1;
}

BulkEditSliderF2::BulkEditSliderF2(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditSlider(parent, id, value, minValue, maxValue, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT2;
}

BulkEditSliderF360::BulkEditSliderF360(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditSlider(parent, id, value, minValue, maxValue, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT360;
}

BulkEditTextCtrlF2::BulkEditTextCtrlF2(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditTextCtrl(parent, id, value, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT2;
}

BulkEditTextCtrlF360::BulkEditTextCtrlF360(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditTextCtrl(parent, id, value, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT360;
}
#pragma endregion

#pragma region Bulk Edit Right Click Handlers
void BulkEditSlider::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_SLIDER_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditSlider::OnSliderPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditFontPicker::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_FONTPICKER_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)& BulkEditFontPicker::OnFontPickerPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditValueCurveButton::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_VALUECURVE_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditValueCurveButton::OnValueCurvePopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditButton::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_BUTTON_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditButton::OnButtonPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditTextCtrl::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_TEXTCTRL_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditTextCtrl::OnTextCtrlPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditFilePickerCtrl::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_FILEPICKERCTRL_BULKEDIT_FN, "Bulk Edit Filename");
    mnu.Append(ID_FILEPICKERCTRL_BULKEDIT_PN, "Bulk Edit Path");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditFilePickerCtrl::OnFilePickerCtrlPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditSpinCtrl::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_SPINCTRL_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditSpinCtrl::OnSpinCtrlPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditChoice::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu mnu;
    mnu.Append(ID_CHOICE_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditChoice::OnChoicePopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditFaceChoice::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), true)) return;

    wxMenu mnu;
    mnu.Append(ID_CHOICE_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditChoice::OnChoicePopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditCheckBox::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (!IsBulkEditAvailable(GetParent(), false)) return;

    wxMenu* cc = new wxMenu();
    cc->Append(ID_CHECKBOX_BULKEDIT_CHECKED, "Checked");
    cc->Append(ID_CHECKBOX_BULKEDIT_UNCHECKED, "Unchecked");
    cc->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&BulkEditCheckBox::OnCheckBoxPopup, nullptr, this);
    wxMenu mnu;
    mnu.AppendSubMenu(cc, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditCheckBox::OnCheckBoxPopup, nullptr, this);
    PopupMenu(&mnu);
}
#pragma endregion

// Allows associated controls to reuse the slider function for bulk edit
void BulkEditSlider::BulkEdit()
{
    wxCommandEvent event;
    event.SetId(ID_SLIDER_BULKEDIT);
    OnSliderPopup(event);
}

#pragma region Do the bulk edit
void BulkEditSlider::OnSliderPopup(wxCommandEvent &event)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (event.GetId() == ID_SLIDER_BULKEDIT)
    {
        // Logging this because these dont happen often and I have seen crashes here and I would like to know which slider is crashing
        logger_base.debug("BulkEditSlider::OnSliderPopup %s", (const char *)GetName().c_str());

        // does it support a value curve - this function should be common
        ValueCurveButton* vcb = GetSettingValueCurveButton(GetParent(), GetName().ToStdString(), "SLIDER");

        // Get the label
        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "SLIDER");
        if (l != nullptr)
        {
            label = l->GetLabel();
        }

        BulkEditSliderDialog dlg(this, label, GetValue(), GetMin(), GetMax(), GetPageSize(), _type, vcb);
        OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK)
        {
            SetValue(dlg.Slider_BulkEdit->GetValue());
            wxTextCtrl* t = GetSettingTextControl(GetParent(), GetName().ToStdString(), "SLIDER");
            if (t != nullptr)
            {
                t->SetValue(dlg.TextCtrl_BulkEdit->GetValue());
            }
            else
            {
                logger_base.crit("BulkEditSlider::OnSliderPopup text control not found %s", (const char *)GetName().c_str());
                wxASSERT(false);
            }

            std::string vcid = "";
            if (vcb != nullptr)
            {
                if (vcb->GetValue() == nullptr)
                {
                    logger_base.crit("BulkEditSlider::OnSliderPopup value curve not present.");
                    wxASSERT(false);
                }

                vcid = vcb->GetValue()->GetId();
                dlg.BitmapButton_VC->GetValue()->SetId(vcid);
                vcid = FixIdForPanel(GetPanelName(GetParent()), vcid);

                if (dlg.BitmapButton_VC->GetValue()->IsActive())
                {
                    vcb->SetValue(dlg.BitmapButton_VC->GetValue()->Serialise());
                }
                else
                {
                    vcb->SetActive(false);
                }
            }

            std::string id = "";
            std::string value = "";
            if (GetName().StartsWith("ID_"))
            {
                id = GetName().ToStdString();
                value = wxString::Format("%d", GetValue()).ToStdString();
            }
            else
            {
                if (t != nullptr)
                {
                    id = t->GetName().ToStdString();
                    value = t->GetValue().ToStdString();
                }
            }

            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, dlg.BitmapButton_VC->GetValue(), vcid);
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, dlg.BitmapButton_VC->GetValue(), vcid);
            }
        }
    }
}

std::string BulkEditFontPicker::GetValue() const
{
    wxFont f = GetSelectedFont();
    if (f.IsOk()) {
        wxString FontDesc = f.GetNativeFontInfoUserDesc();
        FontDesc.Replace(" unknown-90", "");
        return FontDesc.ToStdString();
    }
    return "";
}

void BulkEditFontPicker::OnFontPickerPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_FONTPICKER_BULKEDIT)
    {
        // Get the label
        std::string label = "Font";

        BulkEditFontPickerDialog dlg(this, label, GetValue());
        OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK)
        {
            wxFont oldfont;
            oldfont.SetNativeFontInfoUserDesc(dlg.GetValue());
            SetSelectedFont(oldfont);

            std::string id = GetName().ToStdString();
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, GetValue(), nullptr, "");
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, GetValue(), nullptr, "");
            }
        }
    }
}

void BulkEditValueCurveButton::OnValueCurvePopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_VALUECURVE_BULKEDIT)
    {
        if (IsSliderTextPair(GetParent(), GetName(), "VALUECURVE"))
        {
            BulkEditSlider* slider = GetSettingSliderControl(GetParent(), GetName().ToStdString(), "VALUECURVE");

            if (slider == nullptr)
            {
                wxASSERT(false);
            }
            else
            {
                slider->BulkEdit();
            }
        }
    }
}

void BulkEditButton::OnButtonPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_BUTTON_BULKEDIT)
    {
        std::string id = GetName().ToStdString();
        id = FixIdForPanel(GetPanelName(GetParent()), id);

        if (GetPanelName(GetParent()) == "Effect")
        {
            std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
            xLightsApp::GetFrame()->GetMainSequencer()->ApplyButtonPressToSelected(effect, id);
        }
        else
        {
            xLightsApp::GetFrame()->GetMainSequencer()->ApplyButtonPressToSelected("", id);
        }

        // unselect and select effect to update the panel
        auto effect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
        if (effect != nullptr)
        {
            xLightsApp::GetFrame()->GetMainSequencer()->PanelEffectGrid->RaiseSelectedEffectChanged(effect, true, true);
        }
    }
}

void BulkEditTextCtrl::OnTextCtrlPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_TEXTCTRL_BULKEDIT)
    {
        if (IsSliderTextPair(GetParent(), GetName().ToStdString(), "TEXTCTRL"))
        {
            BulkEditSlider* slider = GetSettingSliderControl(GetParent(), GetName().ToStdString(), "TEXTCTRL");
            if (slider == nullptr)
            {
                wxASSERT(false);
            }
            else
            {
                slider->BulkEdit();
            }
        }
        else
        {
            // Get the label
            std::string label = "Bulk Edit";
            wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "TEXTCTRL");
            if (l != nullptr)
            {
                label = l->GetLabel();
            }

            wxTextEntryDialog dlg(this, "", label, GetValue());
            OptimiseDialogPosition(&dlg);

            if (dlg.ShowModal() == wxID_OK)
            {
                SetValue(dlg.GetValue());

                std::string id = GetName().ToStdString();
                std::string value = GetValue().ToStdString();
                id = FixIdForPanel(GetPanelName(GetParent()), id);

                if (GetPanelName(GetParent()) == "Effect")
                {
                    std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                    xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, nullptr, "");
                }
                else
                {
                    xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, nullptr, "");
                }
            }
        }
    }
}

void BulkEditFilePickerCtrl::OnFilePickerCtrlPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_FILEPICKERCTRL_BULKEDIT_FN)
    {
        // Get the label
        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "FILEPICKERCTRL");
        if (l != nullptr)
        {
            label = l->GetLabel();
        }

        wxFileName fn(GetFileName());

        wxString filename = wxFileSelector(label, GetPath(), GetFileName().GetName(), "*" + fn.GetExt(), _wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

        if (filename.empty())
        {
            // editing cancelled
        }
        else
        {
            SetFileName(filename);

            std::string id = GetName().ToStdString();
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id+"_FN", filename, nullptr, "");
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id+"_FN", filename, nullptr, "");
            }
        }
    }
    else if (event.GetId() == ID_FILEPICKERCTRL_BULKEDIT_PN)
    {
        // Get the label
        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "FILEPICKERCTRL");
        if (l != nullptr)
        {
            label = l->GetLabel();
        }

        wxFileName fn(GetFileName());

        wxDirDialog dlg(this, label, fn.GetPath(), wxDD_DEFAULT_STYLE, wxDefaultPosition, wxDefaultSize, _T("wxDirDialog"));
        OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK)
        {
            fn.SetPath(dlg.GetPath());
            SetFileName(fn.GetFullPath());

            std::string id = GetName().ToStdString();
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id + "_PN", dlg.GetPath(), nullptr, "");
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id+"_PN", dlg.GetPath(), nullptr, "");
            }
        }
    }
}

void BulkEditSpinCtrl::OnSpinCtrlPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_SPINCTRL_BULKEDIT)
    {
        // Get the label
        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "SPINCTRL");
        if (l != nullptr)
        {
            label = l->GetLabel();
        }

        wxNumberEntryDialog dlg(this, "", label, label, GetValue(), GetMin(), GetMax());
        OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK)
        {
            SetValue(dlg.GetValue());

            std::string id = GetName().ToStdString();
            std::string value = wxString::Format("%d", GetValue()).ToStdString();
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, nullptr, "");
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, nullptr, "");
            }
        }
    }
}

void BulkEditChoice::OnChoicePopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_CHOICE_BULKEDIT)
    {
        // Get the label
        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "CHOICE");
        if (l != nullptr)
        {
            label = l->GetLabel();
        }

        wxArrayString choices;
        for (size_t i = 0; i < GetCount(); i++)
        {
            choices.push_back(GetString(i));
        }

        wxSingleChoiceDialog dlg(GetParent(), "", label, choices);
        dlg.SetSelection(GetSelection());
        OptimiseDialogPosition(&dlg);

        if (dlg.ShowModal() == wxID_OK)
        {
            SetSelection(dlg.GetSelection());

            std::string id = GetName().ToStdString();
            std::string value = GetString(dlg.GetSelection());
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, nullptr, "");
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, nullptr, "");
            }
        }
    }
}

void BulkEditCheckBox::OnCheckBoxPopup(wxCommandEvent& event)
{
    bool checked = false;

    if (event.GetId() == ID_CHECKBOX_BULKEDIT_CHECKED)
    {
        checked = true;
    }
    else if (event.GetId() == ID_CHECKBOX_BULKEDIT_UNCHECKED)
    {
    }
    else
    {
        return;
    }

    SetValue(checked);

    std::string id = GetName().ToStdString();
    std::string value = checked ? "1" : "0";
    id = FixIdForPanel(GetPanelName(GetParent()), id);

    if (GetPanelName(GetParent()) == "Effect")
    {
        std::string effect = static_cast<EffectsPanel*>(GetPanel(GetParent()))->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, nullptr, "");
    }
    else
    {
        xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, nullptr, "");
    }
}
#pragma endregion

#pragma region Keep associated controls in sync
void BulkEditSlider::OnDClick(wxMouseEvent& event)
{
    SetValue(_default);
    wxScrollEvent e;
    OnSlider_SliderUpdated(e);
}

void BulkEditSlider::OnSlider_SliderUpdated(wxScrollEvent& event)
{
    BulkEditValueCurveButton* vc = GetSettingValueCurveButton(GetParent(), GetName().ToStdString(), "SLIDER");
    if (vc != nullptr)
    {
        if (vc->GetValue()->GetType() == "Flat")
        {
            vc->GetValue()->SetParameter1((float)GetValue());
        }
    }

    wxTextCtrl* t = GetSettingTextControl(GetParent(), GetName().ToStdString(), "SLIDER");
    if (t == nullptr)
    {
        wxASSERT(false);
    }
    else
    {
        switch (_type)
        {
        case BESLIDERTYPE::BE_INT:
        {
            auto s = wxString::Format("%d", GetValue());
            if (t->GetValue() != s)
            {
                t->ChangeValue(s);
            }
        }
        break;
        case BESLIDERTYPE::BE_FLOAT1:
        {
            auto s = wxString::Format("%.1f", (float)GetValue() / 10.0);
            if (t->GetValue() != s)
            {
                t->ChangeValue(s);
            }
        }
        break;
        case BESLIDERTYPE::BE_FLOAT2:
        {
            auto s = wxString::Format("%.2f", (float)GetValue() / 100.0);
            if (t->GetValue() != s)
            {
                t->ChangeValue(s);
            }
        }
        break;
        case BESLIDERTYPE::BE_FLOAT360:
        {
            auto s = wxString::Format("%.2f", (float)GetValue() / 360.0);
            if (t->GetValue() != s)
            {
                t->ChangeValue(s);
            }
        }
        break;
        }
    }
    GetParent()->HandleWindowEvent(event);
}

void BulkEditTextCtrl::OnTextCtrl_TextUpdated(wxCommandEvent& event)
{
    TextUpdate(false);
    GetParent()->HandleWindowEvent(event);
}

void BulkEditTextCtrl::OnTextCtrl_TextLoseFocus(wxFocusEvent& event)
{
    TextUpdate(true);
    event.Skip();
}

void BulkEditTextCtrl::TextUpdate(bool force)
{
    if (IsSliderTextPair(GetParent(), GetName().ToStdString(), "TEXTCTRL"))
    {
        wxSlider* s = GetSettingSliderControl(GetParent(), GetName().ToStdString(), "TEXTCTRL");

        if (s == nullptr)
        {
            wxASSERT(false);
        }
        else
        {
            switch (_type)
            {
            case BESLIDERTYPE::BE_INT:
            {
                auto t = wxAtoi(GetValue());
                if (s->GetValue() != t)
                {
                    if (force || (s->GetMin() <= 0 && s->GetMax() >= 0) || t >= s->GetMin())
                    {
                        s->SetValue(t);
                        if (s->GetValue() != t)
                        {
                            ChangeValue(wxString::Format("%d", s->GetValue()));
                        }
                    }
                }
            }
            break;
            case BESLIDERTYPE::BE_FLOAT1:
            {
                auto t = wxAtof(GetValue()) * 10;
                if (s->GetValue() != t)
                {
                    if (force || (s->GetMin() <= 0 && s->GetMax() >= 0) || t >= s->GetMin())
                    {
                        s->SetValue(t);
                        if (s->GetValue() != t)
                        {
                            ChangeValue(wxString::Format("%.1f", (float)s->GetValue() / 10.0));
                        }
                    }
                }
            }
            break;
            case BESLIDERTYPE::BE_FLOAT2:
            {
                auto t = wxAtof(GetValue()) * 100.0;
                if (s->GetValue() != t)
                {
                    if (force || (s->GetMin() <= 0 && s->GetMax() >= 0) || t >= s->GetMin())
                    {
                        s->SetValue(t);
                        if (s->GetValue() != t)
                        {
                            ChangeValue(wxString::Format("%.2f", (float)s->GetValue() / 100.0));
                        }
                    }
                }
            }
            break;
            case BESLIDERTYPE::BE_FLOAT360:
            {
                auto t = wxAtof(GetValue()) * 360.0;
                if (s->GetValue() != t)
                {
                    if (force || (s->GetMin() <= 0 && s->GetMax() >= 0) || t >= s->GetMin())
                    {
                        s->SetValue(t);
                        if (s->GetValue() != t)
                        {
                            ChangeValue(wxString::Format("%.2f", (float)s->GetValue() / 360.0));
                        }
                    }
                }
            }
            break;
            }
        }
    }
}
#pragma endregion

#pragma region Bulk edit helper functions
// Convert a control ID into the appropriate setting id
std::string FixIdForPanel(std::string panel, std::string id)
{
    wxString i = id;
    if (panel == "Effect")
    {
        i.Replace("ID_", "E_");
    }
    else if (panel == "Color")
    {
        i.Replace("ID_", "C_");
    }
    else if (panel == "Timing")
    {
        i.Replace("ID_", "T_");
    }
    else if (panel == "Buffer")
    {
        i.Replace("ID_", "B_");
    }
    else
    {
        wxASSERT(false);
    }

    return i.ToStdString();
}

// Walk up the window tree until we find one of the settings panels
wxWindow* GetPanel(wxWindow* w)
{
    while (w != nullptr)
    {
        wxString name = w->GetName();
        if (name == "Effect" ||
            name == "Color" ||
            name == "Buffer" ||
            name == "Timing")
        {
            return w;
        }
        w = w->GetParent();
    }

    return nullptr;
}

// Find the name of the owning settings panel
std::string GetPanelName(wxWindow* w)
{
    w = GetPanel(w);
    if (w != nullptr) return w->GetName().ToStdString();
    return "";
}

// Find an associated control from a SLIDER/VALUECURVE/TEXTCTRL set
wxWindow* GetAssociatedWindow(wxWindow* w, wxString ourName, wxString ourType, wxString desiredType)
{
    wxString name1 = ourName;
    wxString name2 = ourName;
    name1.Replace(ourType, desiredType);
    name2.Replace(ourType, desiredType);
    if (name2.StartsWith("IDD"))
    {
        name2.Replace("IDD", "ID", false);
    }
    else if (name2.StartsWith("ID"))
    {
        name2.Replace("ID", "IDD", false);
    }

    wxWindow* res = wxWindow::FindWindowByName(name1, w);
    if (res == nullptr)
    {
        res = wxWindow::FindWindowByName(name2, w);
    }

    return res;
}

bool IsAssociatedControl(wxWindow* source, wxWindow* target)
{
    wxArrayString names;
    names.push_back("ID_SLIDER");
    names.push_back("IDD_SLIDER");
    names.push_back("ID_TEXTCTRL");
    names.push_back("IDD_TEXTCTRL");
    names.push_back("ID_VALUECURVE");
    names.push_back("ID_CHOICE");
    names.push_back("ID_CHECKBOX");

    wxString tail = "";
    wxString sourceName = source->GetName();

    for (const auto& it : names)
    {
        if (sourceName.StartsWith(it))
        {
            tail = sourceName.SubString(it.size(), sourceName.size() - it.size());
            break;
        }
    }

    if (tail == "")
    {
        return false;
    }

    return target->GetName().EndsWith(tail);
}

BulkEditValueCurveButton* GetSettingValueCurveButton(wxWindow* w, std::string ourName, std::string ourType)
{
    return (BulkEditValueCurveButton*)GetAssociatedWindow(w, ourName, ourType, "VALUECURVE");
}

wxStaticText* GetSettingLabelControl(wxWindow* w, std::string ourName, std::string ourType)
{
    return (wxStaticText*)GetAssociatedWindow(w, ourName, ourType, "STATICTEXT");
}

BulkEditTextCtrl* GetSettingTextControl(wxWindow* w, std::string ourName, std::string ourType)
{
    return (BulkEditTextCtrl*)GetAssociatedWindow(w, ourName, ourType, "TEXTCTRL");
}

BulkEditSlider* GetSettingSliderControl(wxWindow* w, std::string ourName, std::string ourType)
{
    return (BulkEditSlider*)GetAssociatedWindow(w, ourName, ourType, "SLIDER");
}

bool IsSliderTextPair(wxWindow* w, wxString ourName, wxString ourType)
{
    if (ourType == "SLIDER")
    {
        return (GetSettingTextControl(w, ourName.ToStdString(), ourType.ToStdString()) != nullptr);
    }
    else if (ourType == "TEXTCTRL")
    {
        return (GetSettingSliderControl(w, ourName.ToStdString(), ourType.ToStdString()) != nullptr);
    }
    else if (ourType == "VALUECURVE")
    {
        return (GetSettingTextControl(w, ourName.ToStdString(), ourType.ToStdString()) != nullptr &&
            GetSettingSliderControl(w, ourName.ToStdString(), ourType.ToStdString()) != nullptr);
    }

    return false;
}

// Check if bulk edit is appropriate ... ie sequence is open and suitable effects are selected
bool IsBulkEditAvailable(wxWindow* w, bool requireOneElement)
{
    static log4cpp::Category &logger_base = log4cpp::Category::getInstance(std::string("log_base"));

    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) {
        logger_base.debug("Bulk edit refused ... no sequencer.");
        return false;
    }

    // i should only display the menu if more than one effect is selected
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects < 2)
    {
        logger_base.debug("Bulk edit refused ... insufficent effects selected.");
        return false;
    }

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetPanelName(w) == "Effect")
    {
        std::string effect = xLightsApp::GetFrame()->GetEffectManager().GetEffectName(((EffectsPanel*)GetPanel(w))->EffectChoicebook->GetSelection());

        int thiseffect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect);
        if (thiseffect < 2)
        {
            logger_base.debug("Bulk edit refused ... insufficient effects of type %s selected.", (const char *)effect.c_str());
            return false;
        }

        if (requireOneElement)
        {
            if (!xLightsApp::GetFrame()->GetMainSequencer()->AreAllSelectedEffectsOnTheSameElement())
            {
                return false;
            }
        }

        // mixed effect types is ok ... we will just skip those that dont match the current effect panel
    }

    return true;
}
#pragma endregion
