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
#include "SketchCanvasPanel.h"
#include "SketchEffectDrawing.h"

class Model;

class MovingHeadPanel: public xlEffectPanel, public ISketchCanvasParent
{
public:
    
    MovingHeadPanel(wxWindow* parent);
    virtual ~MovingHeadPanel();
    virtual void ValidateWindow() override;

    //(*Declarations(MovingHeadPanel)
    BulkEditSlider* Slider_MHGroupings;
    BulkEditSliderF1* Slider_MHPan;
    BulkEditSliderF1* Slider_MHPanOffset;
    BulkEditSliderF1* Slider_MHTilt;
    BulkEditSliderF1* Slider_MHTiltOffset;
    BulkEditValueCurveButton* ValueCurve_MHGroupings;
    BulkEditValueCurveButton* ValueCurve_MHPan;
    BulkEditValueCurveButton* ValueCurve_MHPanOffset;
    BulkEditValueCurveButton* ValueCurve_MHTilt;
    BulkEditValueCurveButton* ValueCurve_MHTiltOffset;
    wxButton* Button_All;
    wxButton* Button_Evens;
    wxButton* Button_MHPathClear;
    wxButton* Button_MHPathContinue;
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
    wxCheckBox* CheckBox_PanPath;
    wxCheckBox* CheckBox_PanPosition;
    wxCheckBox* CheckBox_TiltPath;
    wxCheckBox* CheckBox_TiltPosition;
    wxFlexGridSizer* FlexGridSizer_Main;
    wxNotebook* Notebook1;
    wxPanel* PanelControl;
    wxPanel* PanelPathing;
    wxPanel* PanelPosition;
    wxStaticText* Label_Pan;
    wxStaticText* Label_Tilt;
    wxStaticText* Label_TiltOffset;
    wxStaticText* StaticText1;
    wxStaticText* StaticTextFixtures;
    wxStaticText* StaticText_Groupings;
    wxStaticText* StaticText_MH1;
    wxStaticText* StaticText_MH2;
    wxStaticText* StaticText_MH3;
    wxStaticText* StaticText_MH4;
    wxStaticText* StaticText_MH5;
    wxStaticText* StaticText_MH6;
    wxStaticText* StaticText_MH7;
    wxStaticText* StaticText_MH8;
    wxTextCtrl* TextCtrl_MH1_Settings;
    wxTextCtrl* TextCtrl_MH2_Settings;
    wxTextCtrl* TextCtrl_MH3_Settings;
    wxTextCtrl* TextCtrl_MH4_Settings;
    wxTextCtrl* TextCtrl_MH5_Settings;
    wxTextCtrl* TextCtrl_MH6_Settings;
    wxTextCtrl* TextCtrl_MH7_Settings;
    wxTextCtrl* TextCtrl_MH8_Settings;
    wxTextCtrl* TextCtrl_MHPathDef;
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
    static const long ID_SLIDER_MHPan;
    static const long ID_VALUECURVE_MHPan;
    static const long IDD_TEXTCTRL_MHPan;
    static const long ID_STATICTEXT_Tilt;
    static const long ID_SLIDER_MHTilt;
    static const long ID_VALUECURVE_MHTilt;
    static const long IDD_TEXTCTRL_MHTilt;
    static const long ID_STATICTEXT1;
    static const long ID_SLIDER_MHPanOffset;
    static const long ID_VALUECURVE_MHPanOffset;
    static const long IDD_TEXTCTRL_MHPanOffset;
    static const long ID_STATICTEXT_TiltOffset;
    static const long ID_SLIDER_MHTiltOffset;
    static const long ID_VALUECURVE_MHTiltOffset;
    static const long IDD_TEXTCTRL_MHTiltOffset;
    static const long ID_STATICTEXT_Groupings;
    static const long ID_SLIDER_MHGroupings;
    static const long ID_VALUECURVE_MHGroupings;
    static const long IDD_TEXTCTRL_MHGroupings;
    static const long ID_CHECKBOX_PanPosition;
    static const long ID_CHECKBOX_PanPath;
    static const long ID_CHECKBOX_TiltPosition;
    static const long ID_CHECKBOX_TiltPath;
    static const long ID_PANEL_Position;
    static const long ID_BUTTON_MHPathContinue;
    static const long ID_BUTTON_MHPathClear;
    static const long ID_TEXTCTRL_MHPathDef;
    static const long ID_PANEL_Pathing;
    static const long ID_PANEL_Control;
    static const long ID_NOTEBOOK1;
    static const long ID_STATICTEXT_MH1;
    static const long ID_TEXTCTRL_MH1;
    static const long ID_TEXTCTRL_MH1_Settings;
    static const long ID_STATICTEXT_MH2;
    static const long ID_TEXTCTRL_MH2;
    static const long ID_TEXTCTRL_MH2_Settings;
    static const long ID_STATICTEXT_MH3;
    static const long ID_TEXTCTRL_MH3;
    static const long ID_TEXTCTRL_MH3_Settings;
    static const long ID_STATICTEXT_MH4;
    static const long ID_TEXTCTRL_MH4;
    static const long ID_TEXTCTRL_MH4_Settings;
    static const long ID_STATICTEXT_MH5;
    static const long ID_TEXTCTRL_MH5;
    static const long ID_TEXTCTRL_MH5_Settings;
    static const long ID_STATICTEXT_MH6;
    static const long ID_TEXTCTRL_MH6;
    static const long ID_TEXTCTRL_MH6_Settings;
    static const long ID_STATICTEXT_MH7;
    static const long ID_TEXTCTRL_MH7;
    static const long ID_TEXTCTRL_MH7_Settings;
    static const long ID_STATICTEXT_MH8;
    static const long ID_TEXTCTRL_MH8;
    static const long ID_TEXTCTRL_MH8_Settings;
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
    void OnButton_MHLeftClick(wxCommandEvent& event);
    void OnButton_MHRightClick(wxCommandEvent& event);
    void OnCheckBox_PanPositionClick(wxCommandEvent& event);
    void OnCheckBox_PanPathClick(wxCommandEvent& event);
    void OnCheckBox_TiltPositionClick(wxCommandEvent& event);
    void OnCheckBox_TiltPathClick(wxCommandEvent& event);
    void OnButton_MHPathContinueClick(wxCommandEvent& event);
    void OnButton_MHPathClearClick(wxCommandEvent& event);
    //*)
    
    DECLARE_EVENT_TABLE()

    std::list<Model*> GetActiveModels();
    void UncheckAllFixtures();
    void UpdateMHSettings();
    void AddSetting(const std::string& name, const std::string& ctrl_name, std::string& ugly_settings, std::string& pretty_settings);
    void AddValueCurve(ValueCurve* vc, const std::string& name, std::string& ugly_settings, std::string& pretty_settings);
    void AddTextbox(const std::string& ctrl_id, const std::string& name, std::string& ugly_settings, std::string& pretty_settings);
    void OnSliderUpdated(wxCommandEvent& event);
    void OnTextCtrlUpdated(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);


//***************************************************
// Pathing support
//***************************************************

public:
    typedef std::function<void(const std::string&, const std::string&, unsigned char)> SketchUpdateCallback;

    // ISketchCanvasParent impl
    SketchEffectSketch& GetSketch() override;
    int GetSelectedPathIndex() override;
    void NotifySketchUpdated() override;
    void NotifySketchPathsUpdated() override;
    void NotifyPathStateUpdated(SketchCanvasPathState state) override;
    void SelectLastPath() override;
    void SetSketchDef(const std::string& sketchDef);

private:
    bool canContinuePath() const;
    void OnCharHook(wxKeyEvent& event);

    SketchCanvasPanel* m_sketchCanvasPanel = nullptr;
    std::string m_sketchDef;
    SketchEffectSketch m_sketch;
    SketchUpdateCallback m_sketchUpdateCB;
    wxListBox* m_pathsListBox = nullptr;
    int selected_path = -1;

    wxString m_bgImagePath;
    wxImage m_bgImage;
    unsigned char m_bitmapAlpha = 0x30;
    int m_pathIndexToDelete = -1;

};

#endif
