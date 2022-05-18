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

#include <wx/artprov.h>
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
#include <wx/fontpicker.h>
#include <wx/notebook.h>
#include <wx/spinctrl.h>
#include <wx/clrpicker.h>

#include "../BitmapCache.h"
#include "../ValueCurveButton.h"
#include "../ValueCurveDialog.h"
#include "../xLightsApp.h"
#include "../xLightsMain.h"
#include "UtilFunctions.h"

std::map<std::string, bool> EffectPanelUtils::buttonStates;
static std::map<wxControl *, wxControl*> LINKED_CONTROLS;
static std::map<wxControl *, ValueCurveButton*> VALUE_CURVE_BUTTONS;

wxDEFINE_EVENT(EVT_VALIDATEWINDOW, wxCommandEvent);

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
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_SLIDER_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("ID_CHOICE_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_CHOICE_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("ID_CHECKBOX_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_CHECKBOX_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("ID_TEXTCTRL_")){
        name = "ID_BITMAPBUTTON_" + name.substr(3);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    if (name.StartsWith("IDD_TEXTCTRL_")){
        name = "ID_BITMAPBUTTON_" + name.substr(4);
        w = ctl->GetParent()->FindWindow(name);
        if (w == nullptr) {
            return false;
        }
        return true;
    }
    return false;
}

void EffectPanelUtils::OnVCChanged(wxCommandEvent& event)
{
    ValueCurveButton* vcb = (ValueCurveButton*)event.GetEventObject();
    if (vcb != nullptr && vcb->GetParent() != nullptr) {
        wxString name = vcb->GetName();
        wxString slidername = name;
        wxString slidername2 = name;
        slidername.Replace("ID_VALUECURVE_", "ID_SLIDER_");
        slidername2.Replace("ID_VALUECURVE_", "IDD_SLIDER_");
        wxString textctrlname = name;
        wxString textctrlname2 = name;
        textctrlname.Replace("ID_VALUECURVE_", "ID_TEXTCTRL_");
        textctrlname2.Replace("ID_VALUECURVE_", "IDD_TEXTCTRL_");
        wxSlider* slider = (wxSlider*)vcb->GetParent()->FindWindow(slidername);
        if (slider == nullptr || (void*)slider == (void*)vcb) {
            slider = (wxSlider*)vcb->GetParent()->FindWindow(slidername2);
        }
        wxTextCtrl* textctrl = (wxTextCtrl*)vcb->GetParent()->FindWindow(textctrlname);
        if (textctrl == nullptr || (void*)textctrl == (void*)vcb) {
            textctrl = (wxTextCtrl*)vcb->GetParent()->FindWindow(textctrlname2);
        }

        wxASSERT(slider != nullptr && (void*)slider != (void*)vcb);
        wxASSERT(textctrl != nullptr && (void*)textctrl != (void*)vcb);
        wxASSERT(vcb->GetValue() != nullptr);

        if ((vcb->GetValue() != nullptr && vcb->GetValue()->IsActive()) || !vcb->IsEnabled()) {
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

        wxCommandEvent e(EVT_VALIDATEWINDOW);
        wxPostEvent(vcb->GetParent(), e);
    }
    else {
        wxASSERT(false);
    }
}

void EffectPanelUtils::enableControlsByName(wxWindow *window, const wxString &name, bool enable) {
    wxWindow *w = window->FindWindow(name);
    if (w != nullptr) {
        w->Enable(enable);
    }
    wxString n2 = "IDD_" + name.SubString(3, name.size());
    w = window->FindWindow(name);
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
    wxSlider *slider = (wxSlider*)vc->GetParent()->FindWindow(name);
    if (slider == nullptr || (void*)slider == (void*)vc)
    {
        name = vc->GetName();
        name.Replace("IDD_VALUECURVE_", "IDD_SLIDER_");
        name.Replace("ID_VALUECURVE_", "IDD_SLIDER_");
        slider = (wxSlider*)vc->GetParent()->FindWindow(name);
        if (slider != nullptr)
        {
            slideridd = true;
        }
    }

    name = vc->GetName();
    name.Replace("IDD_VALUECURVE_", "ID_TEXTCTRL_");
    name.Replace("ID_VALUECURVE_", "ID_TEXTCTRL_");
    wxTextCtrl *txt = (wxTextCtrl*)vc->GetParent()->FindWindow(name);
    if (txt == nullptr || (void*)txt == (void*)vc)
    {
        name = vc->GetName();
        name.Replace("IDD_VALUECURVE_", "IDD_TEXTCTRL_");
        name.Replace("ID_VALUECURVE_", "IDD_TEXTCTRL_");
        txt = (wxTextCtrl*)vc->GetParent()->FindWindow(name);
    }

    vc->ToggleActive();
    if (vc->GetValue()->IsActive())
    {
        ValueCurveDialog vcd(vc->GetParent(), vc->GetValue(), slideridd, ValueCurve::GetSequenceElements());
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



xlEffectPanel::xlEffectPanel(wxWindow* parent) : changeTimer(nullptr) {
}
xlEffectPanel::~xlEffectPanel() {
}


void xlEffectPanel::AddListeners(wxWindow *ParentWin)
{
    wxString s;
    for (const auto& it : ParentWin->GetChildren()) {
        wxWindow *ChildWin = it;
        wxString ChildName = ChildWin->GetName();
        if (ChildName.StartsWith("IDD_") || ChildName.StartsWith("IID_") ) {
            ChildName = "ID_" + ChildName.substr(4);
        }       
        if (ChildName.StartsWith("ID_SLIDER")) {
            Connect(ChildWin->GetId(),wxEVT_SLIDER,(wxObjectEventFunction)&xlEffectPanel::HandleCommandChange);
        } else if (ChildName.StartsWith("ID_VALUECURVE")) {
            //ValueCurveButton* ctrl = (ValueCurveButton*)ChildWin;
            // ValueCurves will automatically fire the change event
        } else if (ChildName.StartsWith("ID_TEXTCTRL")) {
            Connect(ChildWin->GetId(),wxEVT_TEXT,(wxObjectEventFunction)&xlEffectPanel::HandleCommandChange);
        } else if (ChildName.StartsWith("ID_SPINCTRL")) {
            Connect(ChildWin->GetId(),wxEVT_SPINCTRL,(wxObjectEventFunction)&xlEffectPanel::HandleSpinChange);
        } else if (ChildName.StartsWith("ID_CHOICE")) {
            Connect(ChildWin->GetId(),wxEVT_CHOICE,(wxObjectEventFunction)&xlEffectPanel::HandleCommandChange);
        } else if (ChildName.StartsWith("ID_CHECKBOX")) {
            Connect(ChildWin->GetId(),wxEVT_CHECKBOX,(wxObjectEventFunction)&xlEffectPanel::HandleCommandChange);
        } else if (ChildName.StartsWith("ID_RADIOBUTTON")) {
            Connect(ChildWin->GetId(),wxEVT_RADIOBUTTON,(wxObjectEventFunction)&xlEffectPanel::HandleCommandChange);
        } else if (ChildName.StartsWith("ID_FILEPICKER") || ChildName.StartsWith("ID_0FILEPICKER")) {
            Connect(ChildWin->GetId(),wxEVT_FILEPICKER_CHANGED,(wxObjectEventFunction)&xlEffectPanel::HandleFileDirChange);
        } else if (ChildName.StartsWith("ID_FONTPICKER")) {
            Connect(ChildWin->GetId(),wxEVT_FONTPICKER_CHANGED,(wxObjectEventFunction)&xlEffectPanel::HandleFontChange);
        } else if (ChildName.StartsWith("ID_COLOURPICKERCTR")) {
            Connect(ChildWin->GetId(),wxEVT_COLOURPICKER_CHANGED,(wxObjectEventFunction)&xlEffectPanel::HandleColorChange);
        } else if (ChildName.StartsWith("ID_NOTEBOOK")) {
            Connect(ChildWin->GetId(),wxEVT_NOTEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&xlEffectPanel::HandleNotebookChange);
            wxBookCtrlBase *nb = (wxBookCtrlBase*)ChildWin;
            for (int x = 0; x < nb->GetPageCount(); x++) {
                AddListeners(nb->GetPage(x));
            }
        } else if (ChildName.StartsWith("ID_PANEL")
                   || ChildName.StartsWith("ID_SCROLLEDWINDOW")
                   || ChildName.StartsWith("ID_SCROLLED_")
                   || ChildName.StartsWith("panel")
                   || ChildName.StartsWith("groupBox")  //wxStaticBoxSizer creates these
                   ) {
            AddListeners(ChildWin);
        } else if (ChildName.StartsWith("ID_STATICTEXT")
                   || ChildName.StartsWith("ID_STATICLINE")
                   || ChildName.StartsWith("ID_CUSTOM") // various custom controls, they need to fire events themselves
                   || ChildName.StartsWith("ID_BITMAPBUTTON_SLIDER") //locks
                   || ChildName.StartsWith("ID_BITMAPBUTTON_CHOICE")
                   || ChildName.StartsWith("ID_BITMAPBUTTON_BUTTON")
                   || ChildName.StartsWith("ID_BITMAPBUTTON_CHECKBOX")
                   || ChildName.StartsWith("ID_BITMAPBUTTON_FONTPICKER")
                   || ChildName.StartsWith("scrollBar")  //wxScrolledWindow creates these
                   || ChildName.StartsWith("wxID_ANY") //some static text entries
                   
                   || ChildName.StartsWith("ID_BITMAPBUTTON") // misc buttons, they will need to fire event themselves
                   || ChildName.StartsWith("ID_BUTTON")
                   ) {
            //nothing to do
        } else {
            //printf("Unknown ID: %s\n", ChildName.c_str());
        }
    }
}



void xlEffectPanel::AddChangeListeners(wxTimer *timer) {
    changeTimer = timer;
    AddListeners(this);
}
void xlEffectPanel::FireChangeEvent() {
    //static int cnt = 0;  printf("Change fired: %d\n", cnt++);
    if (changeTimer) {
        changeTimer->StartOnce(25);
    }
}


void xlEffectPanel::HandleFileDirChange(wxFileDirPickerEvent& event) { event.Skip(true); FireChangeEvent(); }
void xlEffectPanel::HandleFontChange(wxFontPickerEvent& event) { event.Skip(true); FireChangeEvent(); }
void xlEffectPanel::HandleSpinChange(wxSpinEvent& event) { event.Skip(true); FireChangeEvent(); }
void xlEffectPanel::HandleNotebookChange(wxBookCtrlEvent& event) { event.Skip(true); FireChangeEvent(); }
void xlEffectPanel::HandleScrollChange(wxScrollEvent& event) { event.Skip(true); FireChangeEvent(); }
void xlEffectPanel::HandleCommandChange(wxCommandEvent& event) { event.Skip(true); FireChangeEvent(); }
void xlEffectPanel::HandleColorChange(wxColourPickerEvent& event) { event.Skip(true); FireChangeEvent(); }


const wxBitmapBundle& xlEffectPanel::GetValueCurveNotSelectedBitmap() {
    static wxBitmapBundle bundle = wxArtProvider::GetBitmapBundle("xlART_valuecurve_notselected", wxART_BUTTON);
    return bundle;
}
