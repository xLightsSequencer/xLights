/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "EffectIconPanel.h"
#include "sequencer/DragEffectBitmapButton.h"
#include "BitmapCache.h"
#include "UtilFunctions.h"
#include "effects/EffectManager.h"

//(*InternalHeaders(EffectIconPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectIconPanel)
//*)

BEGIN_EVENT_TABLE(EffectIconPanel, wxPanel)
	//(*EventTable(EffectIconPanel)
	//*)
END_EVENT_TABLE()

void AddEffectPanelButtons(const EffectManager& manager, EffectIconPanel* panel, wxGridSizer* sizer)
{
    int size = panel->FromDIP(16);
    for (int x = 0; x < manager.size(); x++) {
        DragEffectBitmapButton* BitmapButton = new DragEffectBitmapButton(panel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(size, size),
                                                                          wxBU_AUTODRAW | wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON38"));
        BitmapButton->SetMinSize(wxSize(size, size));
        BitmapButton->SetMaxSize(wxSize(size, size));
        BitmapButton->SetEffect(manager[x], 16);
        sizer->Add(BitmapButton, 1, wxALL | wxALIGN_CENTER_HORIZONTAL | wxALIGN_CENTER_VERTICAL, x);
    }
}

EffectIconPanel::EffectIconPanel(const EffectManager& m, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) :
    manager(m)
{
    //(*Initialize(EffectIconPanel)
    Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
    GridSizer1 = new wxGridSizer(0, 5, 0, 0);
    SetSizer(GridSizer1);

    Connect(wxEVT_SIZE, (wxObjectEventFunction)&EffectIconPanel::OnResize);
    //*)

    GridSizer1->SetCols(10);
    AddEffectPanelButtons(manager, this, GridSizer1);

    wxSizeEvent evt;
    OnResize(evt);

    GridSizer1->Fit(this);
    GridSizer1->SetSizeHints(this);
}

EffectIconPanel::~EffectIconPanel()
{
	//(*Destroy(EffectIconPanel)
	//*)
}

void EffectIconPanel::OnResize(wxSizeEvent& event)
{
    int cnt = manager.size();
    wxSize wsz = event.GetSize();
    if (wsz.GetWidth() <= 10) {
        return;
    }

    double cols = wsz.GetWidth() * cnt;
    cols /= std::max(wsz.GetHeight(), 1);
    cols = std::sqrt(cols);
    int i = std::round(cols);
    GridSizer1->SetCols(i);
    GridSizer1->SetDimension(0, 0, wsz.GetWidth(), wsz.GetHeight());
    GridSizer1->Layout();
}
