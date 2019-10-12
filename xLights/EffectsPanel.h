#ifndef EFFECTSPANEL_H
#define EFFECTSPANEL_H

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

class EffectsPanel: public wxPanel
{
public:
    bool EffectChanged;
    wxString* CurrentDir;
    bool _suppressChangeEvent = false;

    EffectsPanel(wxWindow *parent, EffectManager *effects);
    virtual ~EffectsPanel();

    void SetDefaultEffectValues(const wxString &name);
    void SetEffectPanelStatus(Model *cls, const wxString &name);
    void SetSequenceElements(SequenceElements *els);
    void SetEffectType(int effectId);
    
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

#endif
