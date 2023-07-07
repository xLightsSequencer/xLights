#ifndef MOVINGHEADPANEL_H
#define MOVINGHEADPANEL_H

//(*Headers(MovingHeadPanel)
#include <wx/bmpbuttn.h>
#include <wx/button.h>
#include <wx/checkbox.h>
#include <wx/notebook.h>
#include <wx/panel.h>
#include <wx/sizer.h>
#include <wx/slider.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
//*)

#include "../BulkEditControls.h"
#include "EffectPanelUtils.h"

class Model;

class MovingHeadPanel: public xlEffectPanel
{
public:
    
    MovingHeadPanel(wxWindow* parent);
    virtual ~MovingHeadPanel();
    virtual void ValidateWindow() override;
    
    //(*Declarations(MovingHeadPanel)
    BulkEditSliderF1* Slider_Pan;
    BulkEditSliderF1* Slider_Tilt;
    BulkEditValueCurveButton* ValueCurve_Pan;
    BulkEditValueCurveButton* ValueCurve_Tilt;
    wxButton* Button_All;
    wxButton* Button_Apply;
    wxButton* Button_Evens;
    wxButton* Button_None;
    wxButton* Button_Odds;
    wxCheckBox* CheckBox_MH1;
    wxCheckBox* CheckBox_MH2;
    wxCheckBox* CheckBox_MH3;
    wxCheckBox* CheckBox_MH4;
    wxCheckBox* CheckBox_MH5;
    wxCheckBox* CheckBox_MH6;
    wxCheckBox* CheckBox_MH7;
    wxCheckBox* CheckBox_MH8;
    wxFlexGridSizer* FlexGridSizer_Main;
    wxNotebook* Notebook1;
    wxPanel* PanelControl;
    wxPanel* PanelFan;
    wxPanel* PanelMovement;
    wxPanel* PanelPosition;
    wxStaticText* Label_Pan;
    wxStaticText* Label_Tilt;
    wxStaticText* StaticTextFixtures;
    wxStaticText* StaticText_MH1;
    wxStaticText* StaticText_MH2;
    wxStaticText* StaticText_MH3;
    wxStaticText* StaticText_MH4;
    wxStaticText* StaticText_MH5;
    wxStaticText* StaticText_MH6;
    wxStaticText* StaticText_MH7;
    wxStaticText* StaticText_MH8;
    //*)
    
protected:
    
    //(*Identifiers(MovingHeadPanel)
    static const long ID_STATICTEXT_Fixtures;
    static const long IDD_CHECKBOX_MH1;
    static const long IDD_CHECKBOX_MH2;
    static const long IDD_CHECKBOX_MH3;
    static const long IDD_CHECKBOX_MH4;
    static const long IDD_CHECKBOX_MH5;
    static const long IDD_CHECKBOX_MH6;
    static const long IDD_CHECKBOX_MH7;
    static const long IDD_CHECKBOX_MH8;
    static const long ID_BUTTON_All;
    static const long ID_BUTTON_None;
    static const long ID_BUTTON_Evens;
    static const long ID_BUTTON_Odds;
    static const long ID_STATICTEXT_Pan;
    static const long IDD_SLIDER_Pan;
    static const long ID_VALUECURVE_Pan;
    static const long ID_TEXTCTRL_Pan;
    static const long ID_STATICTEXT_Tilt;
    static const long IDD_SLIDER_Tilt;
    static const long ID_VALUECURVE_Tilt;
    static const long ID_TEXTCTRL_Tilt;
    static const long ID_PANEL_Position;
    static const long ID_PANEL_Fan;
    static const long ID_PANEL_Movement;
    static const long ID_PANEL_Control;
    static const long ID_NOTEBOOK1;
    static const long ID_BUTTON_Apply;
    static const long ID_STATICTEXT_MH1;
    static const long ID_TEXTCTRL_MH1;
    static const long ID_STATICTEXT_MH2;
    static const long ID_TEXTCTRL_MH2;
    static const long ID_STATICTEXT_MH3;
    static const long ID_TEXTCTRL_MH3;
    static const long ID_STATICTEXT_MH4;
    static const long ID_TEXTCTRL_MH4;
    static const long ID_STATICTEXT_MH5;
    static const long ID_TEXTCTRL_MH5;
    static const long ID_STATICTEXT_MH6;
    static const long ID_TEXTCTRL_MH6;
    static const long ID_STATICTEXT_MH7;
    static const long ID_TEXTCTRL_MH7;
    static const long ID_STATICTEXT_MH8;
    static const long ID_TEXTCTRL_MH8;
    //*)
    
private:
    
    //(*Handlers(MovingHeadPanel)
    void OnNotebook1PageChanged(wxNotebookEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnCheckBox_MH2Click(wxCommandEvent& event);
    void OnButton_ApplyClick(wxCommandEvent& event);
    void OnButton_AllClick(wxCommandEvent& event);
    void OnButton_NoneClick(wxCommandEvent& event);
    void OnButton_EvensClick(wxCommandEvent& event);
    void OnButton_OddsClick(wxCommandEvent& event);
    //*)
    
    DECLARE_EVENT_TABLE()

    std::list<Model*> GetActiveModels();
    void UncheckAllFixtures();
    void ProcessFirstFixture();
};

#endif
