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

#include "ui/effectpanels/JsonEffectPanel.h"
#include "ui/shared/controls/BulkEditControls.h"
#include "ui/shared/utils/xlLockButton.h"
#include "ui/effectpanels/EffectPanelUtils.h"

#include <wx/dir.h>
#include <wx/scrolwin.h>
#include <list>
#include <vector>
#include <map>

#if __has_include("osxUtils/TouchBars.h")
#include "osxUtils/TouchBars.h"
#endif

#include "render/ValueCurveConsts.h"
#include "utils/Color.h"

class ColourList;
class ColorCurveButton;
class wxBitmapButton;
class wxFlexGridSizer;
class wxSizer;

class ColorPanel : public JsonEffectPanel {
public:
    ColorPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
               const wxPoint& pos = wxDefaultPosition,
               const wxSize& size = wxDefaultSize);
    ~ColorPanel() override = default;

    void RefreshPaletteSize();
    void SetColorCount(int count);
    void SetDefaultSettings(bool optionbased = false);
    wxString GetColorString(bool colourOnly = false);
    wxString GetRandomColorString();
    void SetSupports(bool linear, bool radial);
    void ValidateWindow() override;

    bool PaletteChanged = false;
    bool EffectChanged = false;

    void SetDefaultPalette();

    static double GetSettingVCMin(const std::string& name) { return ValueCurveConsts::GetColorSettingVCMin(name); }
    static double GetSettingVCMax(const std::string& name) { return ValueCurveConsts::GetColorSettingVCMax(name); }
    static int GetSettingVCDivisor(const std::string& name) { return ValueCurveConsts::GetColorSettingVCDivisor(name); }

    void SetButtonColor(ColorCurveButton* btn, const std::string& v, bool notify = true);
    void SetButtonColor(int btn, const xlColor& v, bool notify = true);

#ifdef __XLIGHTS_HAS_TOUCHBARS__
    ColorPanelTouchBar* SetupTouchBar(xlTouchBarSupport& tbs);
    ColorPanelTouchBar* GetTouchBar() const { return touchBar.get(); }
    std::unique_ptr<ColorPanelTouchBar> touchBar;
#endif

protected:
    wxWindow* CreateCustomControl(wxWindow* parentWin, wxSizer* sizer,
                                   const nlohmann::json& prop, int cols) override;

private:
    // Custom row builders
    wxWindow* BuildPaletteHeaderRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildResetPanelRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildChromaKeyRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildSparklesRow(wxWindow* parentWin, wxSizer* sizer);
    wxWindow* BuildBrightnessLevelRow(wxWindow* parentWin, wxSizer* sizer);

    // Handlers
    void OnCheckBox_PaletteClick(wxCommandEvent& event);
    void OnCheckBox_MusicSparklesClick(wxCommandEvent& event);
    void OnBitmapButton_ReverseColoursClick(wxCommandEvent& event);
    void OnCheckBox_ResetColorPanelClick(wxCommandEvent& event);
    void OnCheckBox_EnableChromakeyClick(wxCommandEvent& event);
    void OnBitmapButton_ShiftColoursLeftClick(wxCommandEvent& event);
    void OnBitmapButton_ShiftColoursRightClick(wxCommandEvent& event);
    void OnBitmapButton_MenuPaletteClick(wxCommandEvent& event);
    void OnCCChanged(wxCommandEvent& event);
    void OnCCButtonClick(wxCommandEvent& event);
    void OnColourChoiceDropDown(wxCommandEvent& event);
    void OnColourChoiceSelect(wxCommandEvent& event);
    void OnListPopup(wxCommandEvent& event);
#ifdef __XLIGHTS_HAS_TOUCHBARS__
    void UpdateTouchBarSlider(wxScrollEvent& event);
#endif

    // Palette helpers
    std::string GetCurrentPalette() const;
    wxString FindPaletteFile(const wxString& filename, const wxString& palette) const;
    wxColour GetPaletteColor(int idx) const;
    wxCheckBox* GetPaletteCheckbox(int idx);
    wxButton* GetPaletteButton(int idx);
    void LoadPalettes(wxDir& directory, bool subdirs);
    void LoadAllPalettes();
    bool ValidateAndFormatPaletteString(wxString& input, wxString& errorMsg);
    void LoadColorsToButtons(const wxString& colorString);
    wxString RemoveNonAlphanumeric(wxString const& str) const;
    void ImportPalette();
    void GeneratePalette();
    void SavePalette(bool saveAs);
    void DeletePalette();
    void UpdateColor();

    // Menu action IDs
    static const wxWindowID ID_MNU_UPDATE;
    static const wxWindowID ID_MNU_SAVE;
    static const wxWindowID ID_MNU_SAVE_AS;
    static const wxWindowID ID_MNU_DELETE;
    static const wxWindowID ID_MNU_IMPORT;
    static const wxWindowID ID_MNU_GENERATE;

    // Cached pointers to the compound controls built by the custom rows
    ColourList* _colourList = nullptr;
    wxButton* _menuPaletteButton = nullptr;
    xlSizedBitmapButton* _reverseColoursButton = nullptr;
    xlSizedBitmapButton* _leftShiftColoursButton = nullptr;
    xlSizedBitmapButton* _rightShiftColoursButton = nullptr;
    wxFlexGridSizer* _paletteGridSizer = nullptr;
    wxScrolledWindow* _scrollWindow = nullptr;
    wxCheckBox* _resetColorPanelCheck = nullptr;

    BulkEditSlider* _chromaSensitivity = nullptr;
    BulkEditColourPickerCtrl* _chromaColour = nullptr;
    BulkEditCheckBox* _enableChromaCheck = nullptr;

    BulkEditSlider* _sparkleFrequency = nullptr;
    BulkEditValueCurveButton* _sparkleFrequencyVC = nullptr;
    BulkEditTextCtrl* _sparkleFrequencyText = nullptr;
    xlLockButton* _sparkleFrequencyLock = nullptr;
    BulkEditCheckBox* _musicSparklesCheck = nullptr;
    BulkEditColourPickerCtrl* _sparklesColour = nullptr;
    xlLockButton* _musicSparklesLock = nullptr;

    BulkEditCheckBox* _brightnessLevelCheck = nullptr;

    std::vector<ColorCurveButton*> buttons;
    std::vector<wxCheckBox*> checkBoxes;
    std::map<int, std::string> lastColors;

    std::list<std::string> _loadedPalettes;
    wxString _lastShowDir;
    bool _supportslinear = false;
    bool _supportsradial = false;
};
