#pragma once

#include "EffectPanelUtils.h"

class BulkEditTextCtrl;

class SketchPanel : public xlEffectPanel
{
public:
    SketchPanel(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    virtual ~SketchPanel() = default;

    void ValidateWindow() override;

    // controls are public to allow SketchEffect access
    BulkEditTextCtrl* TextCtrl_SketchDef;

protected:
    static const long ID_TEXTCTRL_SketchDef;

 private:
    DECLARE_EVENT_TABLE()

    void OnButton_DefineSketch(wxCommandEvent& event);
};