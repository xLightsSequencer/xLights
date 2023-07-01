/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/msgdlg.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/slider.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>

//(*InternalHeaders(EffectsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "EffectsPanel.h"
#include "RenderCommandEvent.h"
#include "UtilFunctions.h"
#include "effects/EffectPanelUtils.h"
#include "xLightsApp.h"
#include "xLightsMain.h"
#include "sequencer/MainSequencer.h"

#include <log4cpp/Category.hh>

//(*IdInit(EffectsPanel)
const long EffectsPanel::ID_CHOICEBOOK1 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectsPanel,wxPanel)
    //(*EventTable(EffectsPanel)
    //*)
END_EVENT_TABLE()


EffectsPanel::EffectsPanel(wxWindow *parent, EffectManager *manager, wxTimer *timer) : effectManager(manager), effectChangeTimer(timer)
{
    //(*Initialize(EffectsPanel)
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(1, 1, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    FlexGridSizer1->AddGrowableRow(0);
    EffectChoicebook = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
    FlexGridSizer1->Add(EffectChoicebook, 1, wxRIGHT|wxEXPAND, 2);
    SetSizer(FlexGridSizer1);

    Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&EffectsPanel::EffectSelected);
    //*)

    Connect(EffectChoicebook->GetChoiceCtrl()->GetId(), wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&EffectsPanel::OnRightDownChoice, nullptr, this);

    SetName("Effect");

    for (const auto& it : *effectManager) {
        RenderableEffect *p = it;
        wxScrolledWindow* sw = new wxScrolledWindow(EffectChoicebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_PANEL" + p->Name()));
        xlEffectPanel *panel = p->GetPanel(sw);
        panel->AddChangeListeners(timer);
        wxFlexGridSizer *fgs = new wxFlexGridSizer(1, 1, 0, 0);
        fgs->AddGrowableCol(0);
        fgs->AddGrowableRow(0);
        fgs->Add(panel, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 2);

        sw->SetSizer(fgs);
	sw->SetScrollRate(5, 5);
        fgs->Fit(sw);
        fgs->SetSizeHints(sw);

        EffectChoicebook->AddPage(sw, p->ToolTip(), false);
    }
    EffectChoicebook->SetSelection(0);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);
}

EffectsPanel::~EffectsPanel()
{
    //(*Destroy(EffectsPanel)
    //*)
}

void EffectsPanel::OnRightDownChoice(wxMouseEvent& event)
{
    if (xLightsApp::GetFrame()->GetMainSequencer() == nullptr) {
        return;
    }

    // i should only display the menu if at least one effect is selected
    int alleffects = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffectCount("");
    if (alleffects < 1)
    {
        return;
    }

    wxMenu mnu;
    mnu.Append(wxID_ANY, "Bulk Edit");
    mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&EffectsPanel::OnChoicePopup, nullptr, this);
    PopupMenu(&mnu);
}

void EffectsPanel::SetDefaultEffectValues(const wxString &name) {
    RenderableEffect *eff = effectManager->GetEffect(name.ToStdString());
    if (eff != nullptr) {
        eff->SetDefaultParameters();
	}
}

void EffectsPanel::SetSequenceElements(SequenceElements *els) {
    int x = 0;
    RenderableEffect *p = effectManager->GetEffect(x);
    while (p != nullptr) {
        p->SetSequenceElements(els);
        x++;
        p = effectManager->GetEffect(x);
    }
}

void EffectsPanel::SetEffectType(int effectId)
{
    _suppressChangeEvent = true;
    EffectChoicebook->SetSelection(effectId);
    _suppressChangeEvent = false;
}

void EffectsPanel::ValidateWindow()
{
    xlEffectPanel* panel = dynamic_cast<xlEffectPanel*>(GetSelectedPanel());
    if (panel != nullptr) {
        panel->ValidateWindow();
    }
}

void EffectsPanel::SetEffectPanelStatus(Model *cls, const wxString &name) {
    RenderableEffect *eff = effectManager->GetEffect(name.ToStdString());
    if (eff != nullptr) {
        eff->SetPanelStatus(cls);
	}
}

int EffectsPanel::GetRandomSliderValue(wxSlider* slider) const
{
    if (isRandom(slider)) {
        return rand() % (slider->GetMax()-slider->GetMin()) + slider->GetMin();
    }
    return slider->GetValue();
}

wxWindow* EffectsPanel::GetWindowPanel(wxWindow* w) const {
    wxWindowList &ChildList = w->GetChildren();
    for (wxWindowList::iterator it = ChildList.begin(); it != ChildList.end(); ++it)
    {
        wxWindow *ChildWin = *it;
        wxString ChildName = ChildWin->GetName();
        if (ChildName.StartsWith("ID_PANEL")) {
            return ChildWin;
        }
    }
    return w;
}

wxString EffectsPanel::GetRandomEffectStringFromWindow(wxWindow *w, const wxString &prefix) {
    wxWindowList &ChildList = w->GetChildren();
    wxString s;

    for ( auto it : ChildList)
    {
        wxWindow *ChildWin = it;
        wxString ChildName = ChildWin->GetName();
        wxString AttrName = prefix + ChildName.Mid(3) + "=";

        if (ChildName.StartsWith("ID_SLIDER")) {
            wxSlider* ctrl=(wxSlider*)ChildWin;
            if (ChildName.Contains("Spirograph_r")) {
                // always set little radius, r, to its minimum value
                s += AttrName + wxString::Format("%d", 0);
            } else {
                s += AttrName + wxString::Format("%d", GetRandomSliderValue(ctrl));
            }
		}
		else if (ChildName.StartsWith("ID_TEXTCTRL")) {

			wxSlider * slider = (wxSlider*)w->FindWindowByName("IDD_SLIDER_" + ChildName.SubString(12, ChildName.size()));
			if (slider != nullptr) {
				int i = GetRandomSliderValue(slider);
				slider->SetValue(i);
				wxScrollEvent event(wxEVT_SLIDER, slider->GetId());
				event.SetEventObject(slider);
				event.SetInt(i);
				slider->ProcessWindowEvent(event);
			}
			wxTextCtrl* ctrl = (wxTextCtrl*)ChildWin;
			wxString v = ctrl->GetValue();
			v.Replace("&", "&amp;", true);
			v.Replace(",", "&comma;", true);
			s += AttrName + v;
		} else if (ChildName.StartsWith("ID_SPINCTRL")) {
			wxSpinCtrl* ctrl = (wxSpinCtrl*)ChildWin;
			int v = ctrl->GetValue();
			s += AttrName + wxString::Format(wxT("%i"), v);
		} else if (ChildName.StartsWith("ID_CHOICE")) {
            wxChoice* ctrl=(wxChoice*)ChildWin;
            if (ctrl->GetCount() <= 0) {
                continue;
            }
            s += AttrName + ctrl->GetString(isRandom(ctrl)? rand()%ctrl->GetCount(): ctrl->GetSelection()); //-DJ
        } else if (ChildName.StartsWith("ID_CHECKBOX")) {
            if(ChildName.Contains("Spirograph_Animate")) {
                // always animate spirograph
                s+=AttrName+wxString::Format("%d", 1 );
            } else {
                wxCheckBox* ctrl = (wxCheckBox*)ChildWin;
                int i = isRandom(ctrl)? (rand()%2): ctrl->GetValue();
                wxString v = i ? "1" : "0"; //want random? -DJ
                s += AttrName + v;
            }
        } else if (ChildName.StartsWith("ID_NOTEBOOK")) {
            wxNotebook *notebook = (wxNotebook*)ChildWin;
            int i = rand() % notebook->GetPageCount();
            s += AttrName+notebook->GetPageText(i);
            s += GetRandomEffectStringFromWindow(notebook->GetPage(i), prefix);
        } else if (ChildName.StartsWith("ID_VALUECURVE")) {
//            if (rand()%2 == 1) { // choose to do a valuecurve or not
//                wxWindow *valuveCurveWin = GetWindowPanel(ChildWin);
//                // TODO(craig) need to build the value curve settings
//                s += GetRandomValueCurveFromWindow(valuveCurveWin, prefix);
//            }
        }
    }
    return s;
}

// assumes effidx does not refer to Text effect
// modifed for partially random, allow random colors also -DJ
wxString EffectsPanel::GetRandomEffectString(int effidx)
{
    wxString s,ChildName,AttrName;
    wxString prefix=",E_";

    // get effect controls
    wxWindow *window = EffectChoicebook->GetPage(effidx);
    wxWindow *wPanel = GetWindowPanel(window);
    s += GetRandomEffectStringFromWindow(wPanel, prefix);

    return s;
}

//selectable clear canvas before render: -DJ
//this allows multiple effects to be overlayed for composite models
//also provides useful "bread crumbs" for some effects such as Spirograph
bool EffectsPanel::WantOverlayBkg(void) const
{
    //~return CheckBox_OverlayBkg->GetValue();
    return false;
}

wxWindow* EffectsPanel::GetSelectedPanel() const
{
    return GetWindowPanel(EffectChoicebook->GetPage(EffectChoicebook->GetSelection()));
}

void EffectsPanel::EffectSelected(wxChoicebookEvent& event)
{
    EffectChanged=true;
    wxScrolledWindow* w = (wxScrolledWindow*)EffectChoicebook->GetPage(EffectChoicebook->GetSelection());

    wxString ef = EffectChoicebook->GetPageText(EffectChoicebook->GetSelection());
    SetDefaultEffectValues(ef);

    if (!_suppressChangeEvent)
    {
        SelectedEffectChangedEvent eventEffectChanged(nullptr, false, true, true);
        // We do not have an actual effect in grid to send
        // Set Index of page.
        eventEffectChanged.SetInt(EffectChoicebook->GetSelection());
        wxPostEvent(GetParent(), eventEffectChanged);
    }

    w->FitInside();
    w->SetScrollRate(5, 5);
    w->Refresh();
    effectChangeTimer->StartOnce(25);
}

void EffectsPanel::OnChoicePopup(wxCommandEvent& event)
{
    std::string label = "Effect";

    wxArrayString choices;
    for (size_t i = 0; i < EffectChoicebook->GetChoiceCtrl()->GetCount(); i++)
    {
        choices.push_back(EffectChoicebook->GetChoiceCtrl()->GetString(i));
    }

    wxSingleChoiceDialog dlg(GetParent(), "", label, choices);
    dlg.SetSelection(EffectChoicebook->GetChoiceCtrl()->GetSelection());
    OptimiseDialogPosition(&dlg);

    if (dlg.ShowModal() == wxID_OK)
    {
        SetEffectType(dlg.GetSelection());

        SetDefaultEffectValues(dlg.GetStringSelection());

        xLightsApp::GetFrame()->GetMainSequencer()->ConvertSelectedEffectsTo(dlg.GetStringSelection());

        auto effect = xLightsApp::GetFrame()->GetMainSequencer()->GetSelectedEffect();
        if (effect != nullptr)
        {
            xLightsApp::GetFrame()->GetMainSequencer()->PanelEffectGrid->RaiseSelectedEffectChanged(effect, true, true);
        }
    }
}

//add lock/unlock/random state flags -DJ
//these could be used to make fields read-only, but initially they are just used for partially random effects
//void djdebug(const char* fmt, ...); //_DJ
//typedef enum { Normal, Locked, Random } EditState;
void EffectsPanel::setlock(wxButton* button) //, EditState& islocked)
{
    EffectPanelUtils::SetLock(button);
}

//#define isRandom(ctl)  (buttonState[std::string(ctl->GetName())] == Random)
bool EffectsPanel::isRandom_(wxControl* ctl, const char*debug) const
{
    if (!EffectPanelUtils::IsLockable(ctl)) {
        return false;
    }
    if (!EffectPanelUtils::IsLocked(std::string(ctl->GetName()))) {
        return true;
    }
    return false;
}

bool EffectsPanel::isRandom_(void) const
{
    return isRandom(EffectChoicebook);
}

void EffectsPanel::OnLockButtonClick(wxCommandEvent& event) {
    wxButton * button = (wxButton*)event.GetEventObject();
    setlock(button);
}
