#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include <wx/wx.h>

#include "xlCustomControl.h"

wxDECLARE_EVENT(SUBBUFFER_RANGE_CHANGED, wxCommandEvent);

class SubBufferPanel : public wxWindow, public xlCustomControl
{
public:
    SubBufferPanel(wxPanel* parent, bool usevc, wxWindowID id, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, long style = 0);
    virtual ~SubBufferPanel();

    virtual void SetValue(const std::string& val) override;
    std::string GetValue() const;
    void SetDefaults();
    bool ShouldInheritColours() const override { return true; }
protected:
    DECLARE_EVENT_TABLE()


    void mouseDblClick(wxMouseEvent& event);
    void mouseLeftDown(wxMouseEvent& event);
    void mouseLeftUp(wxMouseEvent& event);
    void mouseMoved(wxMouseEvent& event);
    void Paint(wxPaintEvent& event);
    void ContextMenu(wxContextMenuEvent& event);
    void MenuItemSelected(wxCommandEvent& event);
private:
    void SendChangeEvent();
    int OverMouseHandle(wxMouseEvent& event);
    void Convert(float& xx, float& yy, wxMouseEvent& event);
    bool HasVC() const;

    float x1 = 0.0;
    float y1 = 0.0;
    float x2 = 100.0;
    float y2 = 100.0;
    float x = 0.0;
    float y = 0.0;
    std::string x1vc, y1vc, x2vc, y2vc, xvc, yvc;
    bool _usevc = false;
    wxPoint _startMovePos = { 0,0 };
    wxPoint _start1 = { 0,0 };
    wxPoint _start2 = { 0,0 };

    int draggingHandle = 0;
};
