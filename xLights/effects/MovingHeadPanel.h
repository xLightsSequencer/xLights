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
#include "MovingHeadPanels/MovingHeadCanvasPanel.h"
#include "MovingHeadPanels/MHRgbPickerPanel.h"
#include "MovingHeadPanels/MHColorWheelPanel.h"
#include "SketchCanvasPanel.h"
#include "SketchEffectDrawing.h"

class Model;
class MHPresetBitmapButton;
class MHPathPresetBitmapButton;

class MovingHeadPanel: public xlEffectPanel, public IMovingHeadCanvasParent, public ISketchCanvasParent,
    public IMHRgbPickerPanelParent, public IMHColorWheelPanelParent
{
public:
    
    MovingHeadPanel(wxWindow* parent);
    virtual ~MovingHeadPanel();
    virtual void ValidateWindow() override;

    //(*Declarations(MovingHeadPanel)
    BulkEditCheckBox* CheckBox_MHIgnorePan;
    BulkEditCheckBox* CheckBox_MHIgnoreTilt;
    BulkEditSlider* Slider_MHGroupings;
    BulkEditSliderF1* Slider_MHCycles;
    BulkEditSliderF1* Slider_MHPan;
    BulkEditSliderF1* Slider_MHPanOffset;
    BulkEditSliderF1* Slider_MHPathScale;
    BulkEditSliderF1* Slider_MHTilt;
    BulkEditSliderF1* Slider_MHTiltOffset;
    BulkEditSliderF1* Slider_MHTimeOffset;
    BulkEditTextCtrlF1* TextCtrl_MHCycles;
    BulkEditTextCtrlF1* TextCtrl_MHPathScale;
    BulkEditTextCtrlF1* TextCtrl_MHTimeOffset;
    BulkEditValueCurveButton* ValueCurve_MHGroupings;
    BulkEditValueCurveButton* ValueCurve_MHPan;
    BulkEditValueCurveButton* ValueCurve_MHPanOffset;
    BulkEditValueCurveButton* ValueCurve_MHPathScale;
    BulkEditValueCurveButton* ValueCurve_MHTilt;
    BulkEditValueCurveButton* ValueCurve_MHTiltOffset;
    BulkEditValueCurveButton* ValueCurve_MHTimeOffset;
    wxButton* ButtonSavePathPreset;
    wxButton* ButtonSavePreset;
    wxButton* Button_All;
    wxButton* Button_Evens;
    wxButton* Button_MHPathClear;
    wxButton* Button_MHPathClose;
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
    wxFlexGridSizer* FlexGridSizerColor;
    wxFlexGridSizer* FlexGridSizerPathCanvas;
    wxFlexGridSizer* FlexGridSizerPathPresets;
    wxFlexGridSizer* FlexGridSizerPathing;
    wxFlexGridSizer* FlexGridSizerPosition;
    wxFlexGridSizer* FlexGridSizerPositionCanvas;
    wxFlexGridSizer* FlexGridSizerPresets;
    wxFlexGridSizer* FlexGridSizer_Main;
    wxNotebook* Notebook1;
    wxNotebook* Notebook2;
    wxPanel* PanelColor;
    wxPanel* PanelControl;
    wxPanel* PanelPathing;
    wxPanel* PanelPosition;
    wxStaticText* Label_Pan;
    wxStaticText* Label_PanOffset;
    wxStaticText* Label_PathScale;
    wxStaticText* Label_Tilt;
    wxStaticText* Label_TiltOffset;
    wxStaticText* Label_TimeOffset;
    wxStaticText* StaticTextFixtures;
    wxStaticText* StaticText_Groupings;
    wxStaticText* StaticText_MHCycles;
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
    static const long ID_STATICTEXT_PanOffset;
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
    static const long ID_STATICTEXT_MHCycles;
    static const long IDD_SLIDER_MHCycles;
    static const long ID_TEXTCTRL_MHCycles;
    static const long ID_BUTTON_SavePreset;
    static const long ID_PANEL_Position;
    static const long ID_BUTTON_MHPathContinue;
    static const long ID_BUTTON_MHPathClear;
    static const long ID_BUTTON_MHPathClose;
    static const long ID_TEXTCTRL_MHPathDef;
    static const long ID_STATICTEXT_PathScale;
    static const long ID_SLIDER_MHPathScale;
    static const long ID_VALUECURVE_MHPathScale;
    static const long IDD_TEXTCTRL_MHPathScale;
    static const long ID_STATICTEXT_TimeOffset;
    static const long ID_SLIDER_MHTimeOffset;
    static const long ID_VALUECURVE_MHTimeOffset;
    static const long IDD_TEXTCTRL_MHTimeOffset;
    static const long ID_CHECKBOX_MHIgnorePan;
    static const long ID_CHECKBOX_MHIgnoreTilt;
    static const long ID_BUTTON_SavePathPreset;
    static const long ID_PANEL_Pathing;
    static const long ID_PANEL_Color;
    static const long ID_NOTEBOOK2;
    static const long ID_PANEL_Control;
    static const long ID_NOTEBOOK1;
    static const long ID_TEXTCTRL_MH1_Settings;
    static const long ID_TEXTCTRL_MH2_Settings;
    static const long ID_TEXTCTRL_MH3_Settings;
    static const long ID_TEXTCTRL_MH4_Settings;
    static const long ID_TEXTCTRL_MH5_Settings;
    static const long ID_TEXTCTRL_MH6_Settings;
    static const long ID_TEXTCTRL_MH7_Settings;
    static const long ID_TEXTCTRL_MH8_Settings;
    //*)
    
private:
    
    //(*Handlers(MovingHeadPanel)
    void OnNotebook1PageChanged(wxNotebookEvent& event);
    void OnPaint(wxPaintEvent& event);
    void OnCheckBox_MHClick(wxCommandEvent& event);
    void OnButton_ApplyClick(wxCommandEvent& event);
    void OnButton_AllClick(wxCommandEvent& event);
    void OnButton_NoneClick(wxCommandEvent& event);
    void OnButton_EvensClick(wxCommandEvent& event);
    void OnButton_OddsClick(wxCommandEvent& event);
    void OnButton_MHLeftClick(wxCommandEvent& event);
    void OnButton_MHRightClick(wxCommandEvent& event);
    void OnButton_MHPathContinueClick(wxCommandEvent& event);
    void OnButton_MHPathClearClick(wxCommandEvent& event);
    void OnButton_MHPathCloseClick(wxCommandEvent& event);
    void OnCheckBox_MHIgnorePanClick(wxCommandEvent& event);
    void OnCheckBox_MHIgnoreTiltClick(wxCommandEvent& event);
    void OnButtonSavePresetClick(wxCommandEvent& event);
    void OnButtonSavePathPresetClick(wxCommandEvent& event);
    //*)
    
    DECLARE_EVENT_TABLE()

    std::list<Model*> GetActiveModels();
    void UncheckAllFixtures();
    void UpdateMHSettings();
    void UpdateColorSettings();
    void AddSetting(const std::string& name, const std::string& ctrl_name, std::string& mh_settings);
    void AddPath(std::string& mh_settings);
    void AddValueCurve(ValueCurve* vc, const std::string& name, std::string& mh_settings);
    void AddTextbox(const std::string& ctrl_id, const std::string& name, std::string& mh_settings);
    void OnSliderUpdated(wxCommandEvent& event);
    void OnTextCtrlUpdated(wxCommandEvent& event);
    void OnVCChanged(wxCommandEvent& event);
    void UpdateTextbox(const std::string& ctrl_name, float pos);
    void UpdateValueCurve(const std::string& ctrl_name, const std::string& curve_settings);
    void UpdateCheckbox(const std::string& ctrl_name, bool value);
    bool GetPosition(const std::string& ctrl_name, float& pos);
    void UpdatePositionCanvas(float pan, float tilt);
    void RecallSettings(const std::string mh_settings);
    
    // Preset Functions
    void PopulatePresets();
    void ProcessPresetDir(wxDir& directory, bool subdirs);
    std::string GetMHPresetFolder(const std::string& showFolder);
    void OnButtonPresetClick(wxCommandEvent& event);
    void OnButtonPathPresetClick(wxCommandEvent& event);
    void SavePreset(const wxArrayString& preset, bool is_path = false);
    void LoadMHPreset(const wxFileName& fn);
    void LoadMHPreset(const std::string& fn);
    void UpdateColorPanel();
    void OnResize(wxSizeEvent& event);

    // private variables
    bool recall {false};
    bool presets_loaded {false};
    std::vector<MHPresetBitmapButton*> presets;
    std::vector<MHPathPresetBitmapButton*> path_presets;

    MovingHeadCanvasPanel* m_movingHeadCanvasPanel = nullptr;
    MHRgbPickerPanel* m_rgbColorPanel = nullptr;
    MHColorWheelPanel* m_wheelColorPanel = nullptr;
    wxPanel* m_colorPanel = nullptr;

//***************************************************
// Pathing support
//***************************************************

public:
    // ISketchCanvasParent impl
    SketchEffectSketch& GetSketch() override;
    int GetSelectedPathIndex() override;
    void NotifySketchUpdated() override;
    void NotifySketchPathsUpdated() override;
    void NotifyPathStateUpdated(SketchCanvasPathState state) override;
    void SelectLastPath() override;
    void SetSketchDef(const std::string& sketchDef);
    
    void NotifyPositionUpdated() override;
    void NotifyColorUpdated() override;

private:
    bool canContinuePath() const;
    void OnCharHook(wxKeyEvent& event);

    SketchCanvasPanel* m_sketchCanvasPanel = nullptr;

    std::string m_sketchDef;
    SketchEffectSketch m_sketch;
    wxListBox* m_pathsListBox = nullptr;
    int selected_path = -1;

    wxString m_bgImagePath;
    wxImage m_bgImage;
    unsigned char m_bitmapAlpha = 0x30;
    int m_pathIndexToDelete = -1;
};

#endif
