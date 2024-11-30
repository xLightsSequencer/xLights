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

//(*Headers(EffectsPanel)
#include <wx/choicebk.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
//*)

class AudioManager;
class SequenceElements;
class Model;
class wxSlider;
class EffectManager;
class xlLockButton;
class wxTimer;

class EffectsPanel: public wxPanel
{
public:
    bool EffectChanged;
    wxString* CurrentDir;
    bool _suppressChangeEvent = false;

    EffectsPanel(wxWindow *parent, EffectManager *effects, wxTimer *timer);
    virtual ~EffectsPanel();
    
    void SetDefaultEffectValues(const wxString &name);
    void SetEffectPanelStatus(Model *cls, const wxString &name, int startTimeMs, int endTimeMs);
    void SetSequenceElements(SequenceElements *els);
    void SetEffectType(int effectId);
    void ValidateWindow();
    
    wxString GetRandomEffectString(int effidx);
    bool isRandom_() const;
    bool WantOverlayBkg() const; //selectable clear canvas before render -DJ
    wxWindow* GetSelectedPanel() const;

    //(*Declarations(EffectsPanel)
    wxChoicebook* EffectChoicebook;
    //*)

protected:
    EffectManager *effectManager;

    //(*Identifiers(EffectsPanel)
    static const long ID_CHOICEBOOK1;
    //*)

private:
    wxTimer *effectChangeTimer;
    
    //(*Handlers(EffectsPanel)
    void EffectSelected(wxChoicebookEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    //*)

    void OnRightDownChoice(wxMouseEvent& event);
    void OnChoicePopup(wxCommandEvent& event);
    int GetRandomSliderValue(wxSlider* slider) const;
    wxString GetRandomEffectStringFromWindow(wxWindow *ParentWin, const wxString &prefix);
    wxWindow* GetWindowPanel(wxWindow* w) const;

    //add lock/unlock/random state flags -DJ
    //these could be used to make fields read-only, but initially they are just used for partially random effects
    bool isRandom_(wxControl* ctl, const char*debug) const;
#define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)
    void setlock(wxButton* button); //, EditState& islocked);

    DECLARE_EVENT_TABLE()
};
