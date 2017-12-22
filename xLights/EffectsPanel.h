#ifndef EFFECTSPANEL_H
#define EFFECTSPANEL_H

//(*Headers(EffectsPanel)
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/panel.h>
#include <wx/bmpbuttn.h>
#include <wx/choicebk.h>
//*)

#include <wx/filedlg.h>
#include <wx/fontdlg.h>
#include <unordered_map> //-DJ
#include <wx/colordlg.h>
#include "AudioManager.h"


class SequenceElements;
class Model;
class wxSlider;
class EffectManager;

class EffectsPanel: public wxPanel
{
public:

    EffectsPanel(wxWindow *parent, EffectManager *effects);
    virtual ~EffectsPanel();
    bool EffectChanged;
    void SetDefaultPalette();
    wxString* CurrentDir;
    void SetDefaultEffectValues(Model *cls, AudioManager* audio, const wxString &name);
    void SetEffectPanelStatus(Model *cls, const wxString &name);
    wxString GetRandomEffectString(int effidx);
    void SetButtonColor(wxButton* btn, const wxColour* c);

    void SetSequenceElements(SequenceElements *els);

    //(*Declarations(EffectsPanel)
    wxBitmapButton* BitmapButton_random;
    wxBitmapButton* BitmapButton_LayerEffect;
    wxBitmapButton* BitmapButton_normal;
    wxBitmapButton* BitmapButton_locked;
    wxChoicebook* EffectChoicebook;
    //*)

protected:
    EffectManager *effectManager;

    //(*Identifiers(EffectsPanel)
    static const long ID_CHOICEBOOK1;
    static const long ID_BITMAPBUTTON_CHOICEBOOK1;
    static const long ID_BITMAPBUTTON87;
    static const long ID_BITMAPBUTTON1;
    static const long ID_BITMAPBUTTON88;
    //*)



private:

    //(*Handlers(EffectsPanel)
    void EffectSelected(wxChoicebookEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    //*)

    int GetRandomSliderValue(wxSlider* slider);
    wxString GetRandomEffectStringFromWindow(wxWindow *ParentWin, const wxString &prefix);
    wxWindow* GetWindowPanel(wxWindow* w);

//add lock/unlock/random state flags -DJ
//these could be used to make fields read-only, but initially they are just used for partially random effects
    bool isRandom_(wxControl* ctl, const char*debug);
#define isRandom(ctl)  isRandom_(ctl, #ctl) //(buttonState[std::string(ctl->GetName())] == Random)
    void setlock(wxButton* button); //, EditState& islocked);

    DECLARE_EVENT_TABLE()
public:
    bool isRandom_(void);

    bool WantOverlayBkg(void); //selectable clear canvas before render -DJ
};

#endif
