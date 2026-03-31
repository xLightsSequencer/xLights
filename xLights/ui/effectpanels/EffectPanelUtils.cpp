/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectPanelUtils.h"
#include "assist/AssistPanel.h"
#include "assist/xlGridCanvasEmpty.h"

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

#include "../../BitmapCache.h"
#include "ui/ValueCurveButton.h"
#include "ui/ValueCurveDialog.h"
#include "../../xLightsApp.h"
#include "../../xLightsMain.h"
#include "UtilFunctions.h"
#include "../wxUtilities.h"
#include "../../ExternalHooks.h"

#include <wx/tglbtn.h>

#include <format>

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
    w = window->FindWindow(n2);
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



// Two-phase construction: subclasses call Create(parent, ...) in their own constructors.
xlEffectPanel::xlEffectPanel() : wxPanel(), changeTimer(nullptr) {
}
xlEffectPanel::~xlEffectPanel() {
}


void xlEffectPanel::AddListeners(wxWindow *ParentWin)
{
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
        } else if (ChildName.StartsWith("ID_TOGGLEBUTTON")) {
            Connect(ChildWin->GetId(),wxEVT_TOGGLEBUTTON,(wxObjectEventFunction)&xlEffectPanel::HandleCommandChange);
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
            for (int x = 0; x < (int)nb->GetPageCount(); x++) {
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


void xlEffectPanel::SetSliderValue(wxSlider *slider, int value) {
    slider->SetValue(value);
    wxScrollEvent event(wxEVT_SLIDER, slider->GetId());
    event.SetEventObject(slider);
    event.SetInt(value);
    slider->ProcessWindowEvent(event);
}

void xlEffectPanel::SetSpinValue(wxSpinCtrl *spin, int value) {
    spin->SetValue(value);
    wxCommandEvent event(wxEVT_SPIN, spin->GetId());
    event.SetEventObject(spin);
    event.SetInt(value);
    spin->ProcessWindowEvent(event);
}

void xlEffectPanel::SetChoiceValue(wxChoice *choice, std::string value) {
    choice->SetStringSelection(wxString(value.c_str()));
    wxCommandEvent event(wxEVT_CHOICE, choice->GetId());
    event.SetEventObject(choice);
    event.SetString(wxString(value.c_str()));
    choice->ProcessWindowEvent(event);
}

void xlEffectPanel::SetTextValue(wxTextCtrl *text, std::string value) {
    text->SetValue(wxString(value.c_str()));
    wxCommandEvent event(wxEVT_TEXT, text->GetId());
    event.SetEventObject(text);
    event.SetString(wxString(value.c_str()));
    text->ProcessWindowEvent(event);
}

void xlEffectPanel::SetCheckBoxValue(wxCheckBox *c, bool b) {
    c->SetValue(b);
    wxCommandEvent evt(wxEVT_COMMAND_CHECKBOX_CLICKED, c->GetId());
    evt.SetEventObject(c);
    evt.SetInt(b);
    c->ProcessWindowEvent(evt);
}

void xlEffectPanel::SetRadioValue(wxRadioButton *r) {
    r->SetValue(true);
    wxCommandEvent evt(wxEVT_RADIOBUTTON, r->GetId());
    evt.SetEventObject(r);
    evt.SetInt(true);
    r->ProcessWindowEvent(evt);
}

// this is recursive
static wxString GetEffectStringFromWindow(wxWindow *ParentWin) {
    wxString s;
    for (const auto& it : ParentWin->GetChildren()) {
        wxWindow *ChildWin = it;
        if (!ChildWin->IsEnabled()) {
            continue;
        }
        wxString ChildName = ChildWin->GetName();
        wxString AttrName = "E_" + ChildName.Mid(3);
        if (ChildName.StartsWith("ID_SLIDER")) {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            s += AttrName+ "=" + wxString(std::format("{}", ctrl->GetValue())) + ",";
        } else if (ChildName.StartsWith("ID_VALUECURVE")) {
            ValueCurveButton* ctrl = (ValueCurveButton*)ChildWin;
            if (ctrl->GetValue()->IsActive()) {
                s += AttrName + "=" + ctrl->GetValue()->Serialise() + ",";
            }
        } else if (ChildName.StartsWith("ID_TEXTCTRL")) {
            wxTextCtrl* ctrl=(wxTextCtrl*)ChildWin;
            wxString v = ctrl->GetValue();
            v.Replace("&", "&amp;", true);
            v.Replace(",", "&comma;", true);
            s += AttrName + "=" + v + ",";
        } else if (ChildName.StartsWith("ID_SPINCTRL")) {
            wxSpinCtrl* ctrl = (wxSpinCtrl*)ChildWin;
            int i = ctrl->GetValue();
            s += AttrName + "=" + wxString(std::format("{}", i)) + ",";
        } else if (ChildName.StartsWith("ID_CHOICE")) {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            s += AttrName + "=" + ctrl->GetStringSelection() + ",";
        } else if (ChildName.StartsWith("ID_CHECKBOX")) {
            wxCheckBox* ctrl=(wxCheckBox*)ChildWin;
            wxString v=(ctrl->IsChecked()) ? "1" : "0";
            s += AttrName + "=" + v + ",";
        } else if (ChildName.StartsWith("ID_RADIOBUTTON")) {
            wxRadioButton* ctrl = (wxRadioButton*)ChildWin;
            wxString v = (ctrl->GetValue()) ? "1" : "0";
            s += AttrName + "=" + v + ",";
        } else if (ChildName.StartsWith("ID_TOGGLEBUTTON")) {
            wxToggleButton* ctrl = (wxToggleButton*)ChildWin;
            wxString checkedVal = ctrl->GetValue() ? "1" : "0";
            s += AttrName + "=" + checkedVal + ",";
        } else if (ChildName.StartsWith("ID_FONTPICKER")) {
            wxFontPickerCtrl *fontctrl = (wxFontPickerCtrl *)ChildWin;
            wxFont f = fontctrl->GetSelectedFont();
            if (f.IsOk()) {
                wxString FontDesc = f.GetNativeFontInfoUserDesc();
                FontDesc.Replace(" unknown-90", "");
                s += AttrName + "=" + FontDesc + ",";
            } else {
                s += AttrName + "=,";
            }
        } else if (ChildName.StartsWith("ID_FILEPICKER") || ChildName.StartsWith("ID_0FILEPICKER")) {
            wxFilePickerCtrl* ctrl = (wxFilePickerCtrl*)ChildWin;
            ObtainAccessToURL(ctrl->GetFileName().GetFullPath());
            s += AttrName + "=" + ctrl->GetFileName().GetFullPath() + ",";
        } else if (ChildName.StartsWith("ID_NOTEBOOK") || ChildName.StartsWith("IDD_NOTEBOOK")) {
            wxNotebook* ctrl = (wxNotebook*)ChildWin;
            //for IDD_ stuff, don't record the value of the actual page selected
            if (ChildName.StartsWith("ID_NOTEBOOK")) {
                s += AttrName + "=";
                s += ctrl->GetPageText(ctrl->GetSelection());
                s += ",";
            }
            for(int i = 0; i<(int)ctrl->GetPageCount(); i++) {
                wxString pageString = GetEffectStringFromWindow(ctrl->GetPage(i));
                if (pageString.size() > 0) {
                    s += pageString;
                    if (!s.EndsWith(",")) {
                        s += ",";
                    }
                }
            }
        } else if (ChildName.StartsWith("ID_PANEL_")) {
            wxString pageString = GetEffectStringFromWindow(ChildWin);
            if (pageString.size() > 0) {
                s += pageString;
                if (!s.EndsWith(",")) {
                    s += ",";
                }
            }
        }
    }
    return s.Mid(0,s.size()-1);
}

wxString xlEffectPanel::GetEffectString() {
    return GetEffectStringFromWindow(this);
}

AssistPanel* xlEffectPanel::GetAssistPanel(wxWindow* parent, xLightsFrame* xl_frame) {
    AssistPanel* assist_panel = new AssistPanel(parent);
    xlGridCanvas* grid = new xlGridCanvasEmpty(assist_panel->GetCanvasParent(), wxNewId(), wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL|wxFULL_REPAINT_ON_RESIZE, _T("EmptyGrid"));
    assist_panel->SetGridCanvas(grid);
    return assist_panel;
}

LinkedToggleButton::LinkedToggleButton(wxWindow* parent, wxWindowID id, const wxString& label, const wxPoint& pos, const wxSize& size, long style, const wxValidator& validator, const wxString& name) : wxBitmapToggleButton(parent, id, wxArtProvider::GetBitmapBundle("xlART_UNLINKED"), pos, size, style, validator, name) {
    this->SetBitmapPressed(wxArtProvider::GetBitmapBundle("xlART_LINKED"));
}
