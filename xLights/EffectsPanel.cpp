#include <wx/msgdlg.h>
#include <wx/fontpicker.h>
#include <wx/filepicker.h>
#include <wx/slider.h>
#include <wx/scrolwin.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/spinctrl.h>

//(*InternalHeaders(EffectsPanel)
#include <wx/bitmap.h>
#include <wx/intl.h>
#include <wx/image.h>
#include <wx/string.h>
//*)

#include <map>

#include "effects/EffectManager.h"
#include "effects/RenderableEffect.h"
#include "EffectsPanel.h"
#include "../include/padlock16x16-green.xpm" //-DJ
#include "../include/padlock16x16-red.xpm" //-DJ
#include "../include/padlock16x16-blue.xpm" //-DJ

#include "effects/EffectPanelUtils.h"

//(*IdInit(EffectsPanel)
const long EffectsPanel::ID_CHOICEBOOK1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON_CHOICEBOOK1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON87 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON1 = wxNewId();
const long EffectsPanel::ID_BITMAPBUTTON88 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectsPanel,wxPanel)
    //(*EventTable(EffectsPanel)
    //*)
END_EVENT_TABLE()


EffectsPanel::EffectsPanel(wxWindow *parent, EffectManager *manager) : effectManager(manager)
{
    //(*Initialize(EffectsPanel)
    wxFlexGridSizer* FlexGridSizer8;
    wxFlexGridSizer* FlexGridSizer6;
    wxFlexGridSizer* FlexGridSizer1;

    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    FlexGridSizer1 = new wxFlexGridSizer(0, 2, 0, 0);
    FlexGridSizer1->AddGrowableCol(0);
    EffectChoicebook = new wxChoicebook(this, ID_CHOICEBOOK1, wxDefaultPosition, wxDefaultSize, 0, _T("ID_CHOICEBOOK1"));
    FlexGridSizer1->Add(EffectChoicebook, 1, wxRIGHT|wxEXPAND, 2);
    FlexGridSizer8 = new wxFlexGridSizer(0, 1, 0, 0);
    FlexGridSizer6 = new wxFlexGridSizer(0, 2, 0, 0);
    BitmapButton_LayerEffect = new wxBitmapButton(this, ID_BITMAPBUTTON_CHOICEBOOK1, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON_CHOICEBOOK1"));
    BitmapButton_LayerEffect->SetDefault();
    FlexGridSizer6->Add(BitmapButton_LayerEffect, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_normal = new wxBitmapButton(this, ID_BITMAPBUTTON87, padlock16x16_green_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON87"));
    BitmapButton_normal->SetDefault();
    BitmapButton_normal->Hide();
    FlexGridSizer6->Add(BitmapButton_normal, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_locked = new wxBitmapButton(this, ID_BITMAPBUTTON1, padlock16x16_red_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
    BitmapButton_locked->SetDefault();
    BitmapButton_locked->Hide();
    FlexGridSizer6->Add(BitmapButton_locked, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    BitmapButton_random = new wxBitmapButton(this, ID_BITMAPBUTTON88, padlock16x16_blue_xpm, wxDefaultPosition, wxSize(13,13), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON88"));
    BitmapButton_random->SetDefault();
    BitmapButton_random->Hide();
    FlexGridSizer6->Add(BitmapButton_random, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 1);
    FlexGridSizer8->Add(FlexGridSizer6, 1, wxALL|wxEXPAND, 1);
    FlexGridSizer1->Add(FlexGridSizer8, 1, wxEXPAND, 2);
    SetSizer(FlexGridSizer1);
    FlexGridSizer1->Fit(this);
    FlexGridSizer1->SetSizeHints(this);

    Connect(ID_CHOICEBOOK1,wxEVT_COMMAND_CHOICEBOOK_PAGE_CHANGED,(wxObjectEventFunction)&EffectsPanel::EffectSelected);
    Connect(ID_BITMAPBUTTON_CHOICEBOOK1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&EffectsPanel::OnLockButtonClick);
    //*)

    this->SetName("Effect");


    for (auto it = effectManager->begin(); it != effectManager->end(); it++) {
        RenderableEffect *p = *it;
        wxScrolledWindow* sw = new wxScrolledWindow(EffectChoicebook, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxVSCROLL|wxHSCROLL, _T("ID_PANEL" + p->Name()));
        wxPanel *panel = p->GetPanel(sw);
        wxFlexGridSizer *fgs = new wxFlexGridSizer(1, 1, 0, 0);
        fgs->AddGrowableCol(0);
        fgs->AddGrowableRow(0);
        fgs->Add(panel, 1, wxALL|wxEXPAND|wxALIGN_LEFT|wxALIGN_TOP, 2);

        sw->SetSizer(fgs);
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

void EffectsPanel::SetDefaultEffectValues(Model *cls, AudioManager* audio, const wxString &name) {
    RenderableEffect *eff = effectManager->GetEffect(name.ToStdString());
    if (eff != nullptr) {
        eff->SetDefaultParameters(cls);
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


int EffectsPanel::GetRandomSliderValue(wxSlider* slider)
{
    if (isRandom(slider)) {
        return rand() % (slider->GetMax()-slider->GetMin()) + slider->GetMin();
    }
    return slider->GetValue();
}

wxString EffectsPanel::GetRandomEffectStringFromWindow(wxWindow *w, const wxString &prefix) {
    wxWindowList &ChildList = w->GetChildren();
    wxString s;
    for ( wxWindowList::iterator it = ChildList.begin(); it != ChildList.end(); ++it )
    {
        wxWindow *ChildWin = *it;
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
        }
    }
    return s;
}

// assumes effidx does not refer to Text effect
//modifed for partially random, allow random colors also -DJ
//void djdebug(const char* fmt, ...); //_DJ
wxString EffectsPanel::GetRandomEffectString(int effidx)
{
    wxString s,ChildName,AttrName;
    wxString prefix=",E_";

//    djdebug("GetRandomEffectString: %s rnd? %d", (const char*)Slider_Speed->GetName().c_str(), isRandom(Slider_Speed));

    // get speed
   //~ s = prefix + wxString::Format("SLIDER_Speed=%d", GetRandomSliderValue(Slider_Speed));

    // get effect controls
    wxWindow *window = EffectChoicebook->GetPage(effidx);
    s += GetRandomEffectStringFromWindow(window, prefix);

    return s;
}



//selectable clear canvas before render: -DJ
//this allows multiple effects to be overlayed for composite models
//also provides useful "bread crumbs" for some effects such as Spirograph
bool EffectsPanel::WantOverlayBkg(void)
{
    //~return CheckBox_OverlayBkg->GetValue();
    return false;
}



void EffectsPanel::EffectSelected(wxChoicebookEvent& event)
{
    EffectChanged=true;
    wxScrolledWindow* w = (wxScrolledWindow*)EffectChoicebook->GetPage(EffectChoicebook->GetSelection());

    wxString ef = EffectChoicebook->GetPageText(EffectChoicebook->GetSelection());
    SetDefaultEffectValues(nullptr, nullptr, ef);

    wxCommandEvent eventEffectChanged(EVT_SELECTED_EFFECT_CHANGED);
    // We do not have an actual effect in grid to send
    // Set Index of page.
    eventEffectChanged.SetInt(EffectChoicebook->GetSelection());
    eventEffectChanged.SetClientData(nullptr);
    wxPostEvent(GetParent(), eventEffectChanged);

    w->FitInside();
    w->SetScrollRate(5, 5);
    w->Refresh();
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
bool EffectsPanel::isRandom_(wxControl* ctl, const char*debug)
{
    return !EffectPanelUtils::IsLocked(std::string(ctl->GetName()));
}
bool EffectsPanel::isRandom_(void)
{
    return isRandom(EffectChoicebook);
}

void EffectsPanel::OnLockButtonClick(wxCommandEvent& event) {
    wxButton * button = (wxButton*)event.GetEventObject();
    setlock(button);
}
