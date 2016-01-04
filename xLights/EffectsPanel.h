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

wxDECLARE_EVENT(EVT_SELECTED_EFFECT_CHANGED, wxCommandEvent);

class SequenceElements;
class ModelClass;
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
    void SetDefaultEffectValues(ModelClass *cls, const wxString &name);
    wxString GetRandomEffectString(int effidx);
    void SetButtonColor(wxButton* btn, const wxColour* c);
    wxString GetEffectStringFromWindow(wxWindow *ParentWin);

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
    void enableControlsByName(const wxString &name, bool enable);

    //(*Handlers(EffectsPanel)
    void OnCheckBox_PaletteClick(wxCommandEvent& event);
    void OnButton_PaletteNumberClick(wxCommandEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnTextCtrl2Text(wxCommandEvent& event);
    void OnTextCtrl1Text(wxCommandEvent& event);
    void OnTextCtrl_Eff_On_StartText(wxCommandEvent& event);
    void OnTextCtrl_Eff_On_EndText(wxCommandEvent& event);
    void OnResize(wxSizeEvent& event);
    void OnPaint1(wxPaintEvent& event);
    void OnLockButtonClick(wxCommandEvent& event);
    void UpdateLinkedSlider(wxCommandEvent& event);
    void UpdateLinkedTextCtrl(wxScrollEvent& event);
    void UpdateLinkedSlider360(wxCommandEvent& event);
    void UpdateLinkedTextCtrl360(wxScrollEvent& event);
    void EffectSelected(wxChoicebookEvent& event);
    void UpdateLinkedTextCtrlFloat(wxScrollEvent& event);
    void UpdateLinkedSliderFloat(wxCommandEvent& event);
    //*)

    int GetRandomSliderValue(wxSlider* slider);
    wxString GetRandomEffectStringFromWindow(wxWindow *ParentWin, const wxString &prefix);

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
