#pragma once

#include "EffectPanelUtils.h"

class SketchPanel : public xlEffectPanel
{
public:
    SketchPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SketchPanel() = default;

    void ValidateWindow() override;

 private:
    DECLARE_EVENT_TABLE()

    static const long ID_TEXTCTRL_Sketch;

    void OnButton_DefineSketch(wxCommandEvent& event);
};