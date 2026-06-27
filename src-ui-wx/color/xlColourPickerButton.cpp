/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xlColourPickerButton.h"
#include "xlColourData.h"
#include "shared/utils/wxUtilities.h"

#include <wx/dc.h>
#include <wx/dcmemory.h>
#include <wx/sizer.h>

BEGIN_EVENT_TABLE(xlColourPickerButton, wxPanel)
    EVT_BUTTON(wxID_ANY, xlColourPickerButton::OnClick)
END_EVENT_TABLE()

xlColourPickerButton::xlColourPickerButton(wxWindow* parent, wxWindowID id, const wxColour& initial,
                                           const wxPoint& pos, const wxSize& size, long /*style*/,
                                           const wxValidator& /*validator*/, const wxString& name)
    : wxPanel(parent, id, pos, size, wxBORDER_NONE, name),
      m_colour(initial)
{
    wxSize btnSize = (size == wxDefaultSize) ? wxSize(32, 24) : size;
    m_button = new wxBitmapButton(this, wxID_ANY, MakeColorBitmap(m_colour, btnSize),
                                  wxDefaultPosition, btnSize, wxBU_AUTODRAW | wxBORDER_DEFAULT);
    auto* sizer = new wxBoxSizer(wxHORIZONTAL);
    sizer->Add(m_button, 1, wxEXPAND);
    SetSizer(sizer);
    SetMinSize(btnSize);
}

// static
wxBitmap xlColourPickerButton::MakeColorBitmap(const wxColour& c, const wxSize& sz)
{
    wxSize bmpSz = sz;
    if (bmpSz.GetWidth() <= 0)  bmpSz.SetWidth(32);
    if (bmpSz.GetHeight() <= 0) bmpSz.SetHeight(24);

    wxBitmap bmp(bmpSz.GetWidth(), bmpSz.GetHeight());
    wxMemoryDC dc(bmp);
    dc.SetBackground(wxBrush(c));
    dc.Clear();
    // draw a thin border
    dc.SetPen(wxPen(wxColour(80, 80, 80)));
    dc.SetBrush(*wxTRANSPARENT_BRUSH);
    dc.DrawRectangle(0, 0, bmpSz.GetWidth(), bmpSz.GetHeight());
    return bmp;
}

void xlColourPickerButton::SetColour(const wxColour& c)
{
    m_colour = c;
    if (m_button) {
        wxSize sz = m_button->GetSize();
        if (sz.GetWidth() <= 0) sz = wxSize(32, 24);
        m_button->SetBitmap(MakeColorBitmap(m_colour, sz));
        m_button->Refresh();
    }
}

void xlColourPickerButton::OnClick(wxCommandEvent& /*event*/)
{
    auto [res, newcolor] = xlColourData::INSTANCE.ShowColorDialog(this, m_colour);
    if (res == wxID_OK) {
        SetColour(newcolor);
        wxColourPickerEvent evt(this, GetId(), m_colour);
        GetEventHandler()->ProcessEvent(evt);
    }
}
