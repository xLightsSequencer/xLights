/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectPanelUtils.h"

#include <wx/scrolwin.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/filepicker.h>
#include <wx/statline.h>
#include <wx/radiobut.h>
#include <wx/slider.h>
#include <wx/panel.h>
#include <wx/choice.h>
#include <wx/bmpbuttn.h>
#include <wx/gbsizer.h>
#include <wx/button.h>
#include <wx/fontpicker.h>
#include <wx/choicebk.h>
#include <wx/valnum.h>

#include "../BitmapCache.h"
#include "../ValueCurveButton.h"
#include "../ValueCurveDialog.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

std::map<std::string, bool> EffectPanelUtils::buttonStates;
static std::map<wxControl *, wxControl*> LINKED_CONTROLS;
static std::map<wxControl *, ValueCurveButton*> VALUE_CURVE_BUTTONS;


void EffectPanelUtils::OnLockButtonClick(wxCommandEvent& event) {
    wxButton * button = (wxButton*)event.GetEventObject();
    SetLock(button);
}

void EffectPanelUtils::SetLock(wxButton *button) {
    wxString parent = button->GetName();
    if (parent.StartsWith("ID_BITMAPBUTTON_")) {
        parent = "ID_" + parent.substr(16);
    }
    bool islocked = buttonStates[std::string(parent)];
    if (islocked) {
        buttonStates[std::string(parent)] = false;
        button->SetBitmapLabel(BitmapCache::GetLockIcon(false));
    } else {
        buttonStates[std::string(parent)] = true;
        button->SetBitmapLabel(BitmapCache::GetLockIcon(true));
    }
}

bool EffectPanelUtils::IsLocked(std::string name) {
    return buttonStates[name];
}

bool EffectPanelUtils::IsLockable(wxControl* ctl) {
    wxString name = ctl->GetName();
    if (!name.StartsWith("ID_") && !name.StartsWith("IDD_")) {
        return false;
    }
    wxWindow* w;
    // check for the locks on effect setting
    if (name.StartsWith("ID_SLIDER_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_SLIDER_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("ID_CHOICE_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_CHOICE_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("ID_CHECKBOX_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_CHECKBOX_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("ID_TEXTCTRL_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_TEXTCTRL_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindowByName(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    return false;
}

void EffectPanelUtils::OnVCChanged(wxCommandEvent& event)
{
    ValueCurveButton * vcb = (ValueCurveButton*)event.GetEventObject();
    if (vcb != nullptr) {
        wxString name = vcb->GetName();
        wxString slidername = name;
        wxString slidername2 = name;
        slidername.Replace("ID_VALUECURVE_", "ID_SLIDER_");
        slidername2.Replace("ID_VALUECURVE_", "IDD_SLIDER_");
        wxString textctrlname = name;
        wxString textctrlname2 = name;
        textctrlname.Replace("ID_VALUECURVE_", "ID_TEXTCTRL_");
        textctrlname2.Replace("ID_VALUECURVE_", "IDD_TEXTCTRL_");
        wxSlider* slider = (wxSlider*)vcb->GetParent()->FindWindowByName(slidername);
        if (slider == nullptr || (void*)slider == (void*)vcb) {
            slider = (wxSlider*)vcb->GetParent()->FindWindowByName(slidername2);
        }
        wxTextCtrl* textctrl = (wxTextCtrl*)vcb->GetParent()->FindWindowByName(textctrlname);
        if (textctrl == nullptr || (void*)textctrl == (void*)vcb) {
            textctrl = (wxTextCtrl*)vcb->GetParent()->FindWindowByName(textctrlname2);
        }

        wxASSERT(slider != nullptr && (void*)slider != (void*)vcb);
        wxASSERT(textctrl != nullptr && (void*)textctrl != (void*)vcb);

        if (vcb->GetValue()->IsActive()) {
            if (slider != nullptr) {
                slider->Disable();
            }
            if (textctrl != nullptr) {
                textctrl->Disable();
            }
        }
        else {
            if (vcb->IsEnabled()) {
                if (slider != nullptr) {
                    slider->Enable();
                }
                if (textctrl != nullptr) {
                    textctrl->Enable();
                }
            }
        }
    }
}

void EffectPanelUtils::enableControlsByName(wxWindow *window, const wxString &name, bool enable) {
    wxWindow *w = window->FindWindowByName(name);
    if (w != nullptr) {
        w->Enable(enable);
    }
    wxString n2 = "IDD_" + name.SubString(3, name.size());
    w = window->FindWindowByName(name);
    if (w != nullptr) {
        w->Enable(enable);
    }
}

void EffectPanelUtils::OnVCButtonClick(wxCommandEvent& event)
{
    ValueCurveButton * vc = (ValueCurveButton*)event.GetEventObject();
    bool slideridd = false;
    wxString name = vc->GetName();
    name.Replace("IDD_VALUECURVE_", "ID_SLIDER_");
    name.Replace("ID_VALUECURVE_", "ID_SLIDER_");
    wxSlider *slider = (wxSlider*)vc->GetParent()->FindWindowByName(name);
    if (slider == nullptr || (void*)slider == (void*)vc)
    {
        name = vc->GetName();
        name.Replace("IDD_VALUECURVE_", "IDD_SLIDER_");
        name.Replace("ID_VALUECURVE_", "IDD_SLIDER_");
        slider = (wxSlider*)vc->GetParent()->FindWindowByName(name);
        if (slider != nullptr)
        {
            slideridd = true;
        }
    }

    name = vc->GetName();
    name.Replace("IDD_VALUECURVE_", "ID_TEXTCTRL_");
    name.Replace("ID_VALUECURVE_", "ID_TEXTCTRL_");
    wxTextCtrl *txt = (wxTextCtrl*)vc->GetParent()->FindWindowByName(name);
    if (txt == nullptr || (void*)txt == (void*)vc)
    {
        name = vc->GetName();
        name.Replace("IDD_VALUECURVE_", "IDD_TEXTCTRL_");
        name.Replace("ID_VALUECURVE_", "IDD_TEXTCTRL_");
        txt = (wxTextCtrl*)vc->GetParent()->FindWindowByName(name);
    }

    vc->ToggleActive();
    if (vc->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(vc->GetParent(), vc->GetValue(), slideridd);
        OptimiseDialogPosition(&vcd);
        if (vcd.ShowModal() == wxOK)
        {
            if (slider != nullptr)
            {
                slider->Disable();
            }
            if (txt != nullptr)
            {
                txt->Disable();
            }
        }
        else
        {
            if (slider != nullptr)
            {
                slider->Enable();
            }
            if (txt != nullptr)
            {
                txt->Enable();
            }
            vc->SetActive(false);
        }
        vc->UpdateState();
        if (vcd.DidExport())
        {
            wxCommandEvent e(EVT_VC_CHANGED);
            e.SetInt(-1);
            wxPostEvent(xLightsApp::GetFrame(), e);
        }
    }
    else
    {
        if (slider != nullptr)
        {
            slider->Enable();
        }
        if (txt != nullptr)
        {
            txt->Enable();
        }
    }
}
