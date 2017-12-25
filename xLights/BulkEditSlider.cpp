#include "BulkEditSlider.h"

#include "xLightsMain.h"
#include "xLightsApp.h"
#include "EffectsPanel.h"
#include "sequencer/MainSequencer.h"
#include "BulkEditSliderDialog.h"

#include <wx/textdlg.h>
#include <wx/numdlg.h>
#include <wx/choicdlg.h>

// Here is how the sliders, labels, value curves and text boxes all work together.
//
// ID_ vs IDD_ ... whichever is ID_ is the value that will be stored as the setting. The IDD is ignored
// All sliders start with ID[D]_SLIDER
// All text boxes start with ID[D]_TEXTCTRL
// All labels start with ID_STATICTEXT
// All value curve buttons start with ID_VALUECURVE
// Whatever occurs after the above strings must match for the controls to be associated and work correctly
// Even for associated controls where you dont want bulk edit you can still use these classes

BulkEditSlider::BulkEditSlider(wxWindow *parent, wxWindowID id, int value, int minValue, int maxValue, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxSlider(parent, id, value, minValue, maxValue, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_INT;
    ID_SLIDER_BULKEDIT = wxNewId();
    Connect(wxEVT_COMMAND_SLIDER_UPDATED, (wxObjectEventFunction)&BulkEditSlider::OnSlider_SliderUpdated);
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditSlider::OnRightDown, nullptr, this);
}

BulkEditTextCtrl::BulkEditTextCtrl(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxTextCtrl(parent, id, value, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_INT;
    ID_TEXTCTRL_BULKEDIT = wxNewId();
    Connect(wxEVT_COMMAND_TEXT_UPDATED, (wxObjectEventFunction)&BulkEditTextCtrl::OnTextCtrl_TextUpdated);
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditTextCtrl::OnRightDown, nullptr, this);
}

BulkEditSpinCtrl::BulkEditSpinCtrl(wxWindow *parent, wxWindowID id, const wxString &value, const wxPoint &pos, const wxSize &size, long style, int min, int max, int initial, const wxString &name) : wxSpinCtrl(parent, id, value, pos, size, style, min, max, initial, name)
{
    _type = BESLIDERTYPE::BE_INT;
    ID_SPINCTRL_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditSpinCtrl::OnRightDown, nullptr, this);
}

BulkEditValueCurveButton::BulkEditValueCurveButton(wxWindow *parent, wxWindowID id, const wxBitmap& bitmap, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : ValueCurveButton(parent, id, bitmap, pos, size, style, validator, name)
{
    //_type = BESLIDERTYPE::BE_INT;
    ID_VALUECURVE_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditValueCurveButton::OnRightDown, nullptr, this);
}

BulkEditChoice::BulkEditChoice(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, int n, const wxString choices[], long style, const wxValidator &validator, const wxString &name) : wxChoice(parent, id, pos, size, n, choices, style, validator, name)
{
    ID_CHOICE_BULKEDIT = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditChoice::OnRightDown, nullptr, this);
}

BulkEditCheckBox::BulkEditCheckBox(wxWindow *parent, wxWindowID id, const wxString &label, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : wxCheckBox(parent, id, label, pos, size, style, validator, name)
{
    ID_CHECKBOX_BULKEDIT_CHECKED = wxNewId();
    ID_CHECKBOX_BULKEDIT_UNCHECKED = wxNewId();
    Connect(wxEVT_RIGHT_DOWN, (wxObjectEventFunction)&BulkEditCheckBox::OnRightDown, nullptr, this);
}

void BulkEditSlider::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    // i should only display the menu if more than one effect is selected
    if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("") < 2) return;

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetParent()->GetName() == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect) < 2) return;
    }

    wxMenu mnu;
    mnu.Append(ID_SLIDER_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditSlider::OnSliderPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditValueCurveButton::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    // i should only display the menu if more than one effect is selected
    if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("") < 2) return;

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetParent()->GetName() == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect) < 2) return;
    }

    wxMenu mnu;
    mnu.Append(ID_VALUECURVE_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditValueCurveButton::OnValueCurvePopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditTextCtrl::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    // i should only display the menu if more than one effect is selected
    if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("") < 2) return;

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetParent()->GetName() == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect) < 2) return;
    }

    wxMenu mnu;
    mnu.Append(ID_TEXTCTRL_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditTextCtrl::OnTextCtrlPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditSpinCtrl::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    // i should only display the menu if more than one effect is selected
    if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("") < 2) return;

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetParent()->GetName() == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect) < 2) return;
    }

    wxMenu mnu;
    mnu.Append(ID_SPINCTRL_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditSpinCtrl::OnSpinCtrlPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditChoice::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    // i should only display the menu if more than one effect is selected
    if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("") < 2) return;

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetParent()->GetName() == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect) < 2) return;
    }

    wxMenu mnu;
    mnu.Append(ID_CHOICE_BULKEDIT, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditChoice::OnChoicePopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditCheckBox::OnRightDown(wxMouseEvent& event)
{
    if (!_supportsBulkEdit) return;
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) return;

    // i should only display the menu if more than one effect is selected
    if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("") < 2) return;

    // if it is an effect setting maybe i should check more than 1 of that effect are selected
    if (GetParent()->GetName() == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        if (xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount(effect) < 2) return;
    }

    wxMenu* cc = new wxMenu();
    cc->Append(ID_CHECKBOX_BULKEDIT_CHECKED, "Checked");
    cc->Append(ID_CHECKBOX_BULKEDIT_UNCHECKED, "Unchecked");
    cc->Connect(wxEVT_COMMAND_MENU_SELECTED, (wxObjectEventFunction)&BulkEditCheckBox::OnCheckBoxPopup, nullptr, this);
    wxMenu mnu;
    mnu.AppendSubMenu(cc, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&BulkEditCheckBox::OnCheckBoxPopup, nullptr, this);
    PopupMenu(&mnu);
}

void BulkEditSlider::BulkEdit()
{
    wxCommandEvent event;
    event.SetId(ID_SLIDER_BULKEDIT);
    OnSliderPopup(event);
}

void BulkEditSlider::OnSliderPopup(wxCommandEvent &event)
{
    if (event.GetId() == ID_SLIDER_BULKEDIT)
    {
        // does it support a value curve - this function should be common
        ValueCurveButton* vcb = GetSettingValueCurveButton(GetParent(), GetName().ToStdString(), "SLIDER");

        // Get the label
        std::string label = "Bulk Edit";
        wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "SLIDER");
        if (l != nullptr)
        {
            label = l->GetLabel();
        }

        BulkEditSliderDialog dlg(this, label, GetValue(), GetMin(), GetMax(), _type, vcb);

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
                wxASSERT(false);
            }

            std::string vcid = "";
            if (vcb != nullptr)
            {
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
                std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, dlg.BitmapButton_VC->GetValue(), vcid);
            }
            else
            {
                xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, dlg.BitmapButton_VC->GetValue(), vcid);
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

void BulkEditTextCtrl::OnTextCtrlPopup(wxCommandEvent& event)
{
    if (event.GetId() == ID_TEXTCTRL_BULKEDIT)
    {
        if (IsSliderTextPair(GetParent(), GetName().ToStdString(), "TEXTCTRL"))
        {
            // does it support a value curve - this function should be common
            ValueCurveButton* vcb = GetSettingValueCurveButton(GetParent(), GetName().ToStdString(), "TEXTCTRL");

            // Get the label
            std::string label = "Bulk Edit";
            wxStaticText* l = GetSettingLabelControl(GetParent(), GetName().ToStdString(), "TEXTCTRL");
            if (l != nullptr)
            {
                label = l->GetLabel();
            }

            wxSlider* s = GetSettingSliderControl(GetParent(), GetName().ToStdString(), "TEXTCTRL");

            BulkEditSliderDialog dlg(s, label, s->GetValue(), s->GetMin(), s->GetMax(), _type, vcb);

            if (dlg.ShowModal() == wxID_OK)
            {
                s->SetValue(dlg.Slider_BulkEdit->GetValue());
                SetValue(dlg.TextCtrl_BulkEdit->GetValue());

                std::string vcid = "";
                if (vcb != nullptr)
                {
                    vcid = vcb->GetValue()->GetId();
                    dlg.BitmapButton_VC->GetValue()->SetId(vcid);
                    vcid = FixIdForPanel(GetPanelName(GetParent()), vcid);

                    if (dlg.BitmapButton_VC->GetValue()->IsActive())
                    {
                        vcb->SetValue(dlg.BitmapButton_VC->GetValue()->Serialise());
                    }
                }

                std::string id = "";
                std::string value = "";
                if (GetName().StartsWith("ID_"))
                {
                    id = GetName().ToStdString();
                    value = GetValue().ToStdString();
                }
                else
                {
                    id = s->GetName().ToStdString();
                    value = wxString::Format("%d", s->GetValue()).ToStdString();
                }

                id = FixIdForPanel(GetPanelName(GetParent()), id);

                if (GetPanelName(GetParent()) == "Effect")
                {
                    std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
                    xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, dlg.BitmapButton_VC->GetValue(), vcid);
                }
                else
                {
                    xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, dlg.BitmapButton_VC->GetValue(), vcid);
                }
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

            if (dlg.ShowModal() == wxID_OK)
            {
                SetValue(dlg.GetValue());

                std::string id = "";
                std::string value = "";
                id = GetName().ToStdString();
                value = GetValue().ToStdString();
                id = FixIdForPanel(GetPanelName(GetParent()), id);

                if (GetPanelName(GetParent()) == "Effect")
                {
                    std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
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

        if (dlg.ShowModal() == wxID_OK)
        {
            SetValue(dlg.GetValue());

            std::string id = "";
            std::string value = "";
            id = GetName().ToStdString();
            value = wxString::Format("%d", GetValue()).ToStdString();
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
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

        if (dlg.ShowModal() == wxID_OK)
        {
            SetSelection(dlg.GetSelection());

            std::string id = "";
            std::string value = "";
            id = GetName().ToStdString();
            value = GetString(dlg.GetSelection());
            id = FixIdForPanel(GetPanelName(GetParent()), id);

            if (GetPanelName(GetParent()) == "Effect")
            {
                std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
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

    std::string id = "";
    std::string value = "";
    id = GetName().ToStdString();
    value = checked ? "1" : "0";
    id = FixIdForPanel(GetPanelName(GetParent()), id);

    if (GetPanelName(GetParent()) == "Effect")
    {
        std::string effect = ((EffectsPanel*)GetParent())->EffectChoicebook->GetChoiceCtrl()->GetStringSelection().ToStdString();
        xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected(effect, id, value, nullptr, "");
    }
    else
    {
        xLightsApp::GetFrame()->GetMainSequencer()->ApplyEffectSettingToSelected("", id, value, nullptr, "");
    }
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
                t->SetValue(s);
            }
        }
        break;
        case BESLIDERTYPE::BE_FLOAT1:
        {
            auto s = wxString::Format("%.1f", (float)GetValue() / 10.0);
            if (t->GetValue() != s)
            {
                t->SetValue(s);
            }
        }
        break;
        case BESLIDERTYPE::BE_FLOAT2:
        {
            auto s = wxString::Format("%.2f", (float)GetValue() / 100.0);
            if (t->GetValue() != s)
            {
                t->SetValue(s);
            }
        }
        break;
        }
    }
}

void BulkEditTextCtrl::OnTextCtrl_TextUpdated(wxCommandEvent& event)
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
                    s->SetValue(t);
                }
            }
            break;
            case BESLIDERTYPE::BE_FLOAT1:
            {
                auto t = wxAtof(GetValue()) * 10;
                if (s->GetValue() != t)
                {
                    s->SetValue(t);
                }
            }
            break;
            case BESLIDERTYPE::BE_FLOAT2:
            {
                auto t = wxAtof(GetValue()) * 100.0;
                if (s->GetValue() != t)
                {
                    s->SetValue(t);
                }
            }
            break;
            }
        }
    }
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

BulkEditTextCtrlF2::BulkEditTextCtrlF2(wxWindow *parent, wxWindowID id, wxString value, const wxPoint &pos, const wxSize &size, long style, const wxValidator &validator, const wxString &name) : BulkEditTextCtrl(parent, id, value, pos, size, style, validator, name)
{
    _type = BESLIDERTYPE::BE_FLOAT1;
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

    for (auto it = names.begin(); it != names.end(); ++it)
    {
        if (sourceName.StartsWith(*it))
        {
            tail = sourceName.SubString(it->size(), sourceName.size() - it->size());
            break;
        }
    }

    if (tail == "")
    {
        return false;
    }

    return target->GetName().EndsWith(tail);
}

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
    else if (panel == "Layer")
    {
        i.Replace("ID_", "L_");
    }
    else
    {
        wxASSERT(false);
    }

    return i.ToStdString();
}

std::string GetPanelName(wxWindow* w)
{
    while (w != nullptr)
    {
        if (w->GetName() == "Effect")
        {
            return "Effect";
        }
        else if (w->GetName() == "Color")
        {
            return "Color";
        }
        else if (w->GetName() == "Layer")
        {
            return "Layer";
        }
        else if (w->GetName() == "Timing")
        {
            return "Timing";
        }

        w = w->GetParent();
    }

    return "";
}

wxWindow* GetAssociatedWindow(wxWindow* w, wxString ourName, wxString ourType, wxString desiredType)
{
    wxString name1 = ourName;
    wxString name2 = ourName;
    name1.Replace(ourType, desiredType);
    name2.Replace(ourType, desiredType);
    if (name2.StartsWith("IDD"))
    {
        name2.Replace("IDD", "ID");
    }
    else
    {
        name2.Replace("ID", "IDD");
    }

    wxWindow* res = w->FindWindowByName(name1);
    if (res == nullptr)
    {
        res = w->FindWindowByName(name2);
    }

    return res;
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
