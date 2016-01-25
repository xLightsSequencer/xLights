#include "EffectIconPanel.h"
#include "sequencer/DragEffectBitmapButton.h"
#include "BitmapCache.h"

//(*InternalHeaders(EffectIconPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectIconPanel)
//*)


#include "effects/EffectManager.h"

BEGIN_EVENT_TABLE(EffectIconPanel,wxPanel)
	//(*EventTable(EffectIconPanel)
	//*)
END_EVENT_TABLE()



void AddEffectPanelButtons(const EffectManager &manager, EffectIconPanel *panel, wxGridSizer *sizer) {
    
    for (int x = 0; x < manager.size(); x++) {
        DragEffectBitmapButton *BitmapButton = new DragEffectBitmapButton(panel, wxID_ANY, wxNullBitmap, wxDefaultPosition, wxSize(16,16),
                                                                            wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON38"));
        BitmapButton->SetMinSize(wxSize(16,16));
        BitmapButton->SetMaxSize(wxSize(16,16));
        BitmapButton->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
        BitmapButton->SetEffect(manager[x]);
        sizer->Add(BitmapButton, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, x);
        
    }
}


EffectIconPanel::EffectIconPanel(const EffectManager &manager, wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EffectIconPanel)
	wxGridSizer* GridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	GridSizer1 = new wxGridSizer(0, 5, 0, 0);
	SetSizer(GridSizer1);
	GridSizer1->Fit(this);
	GridSizer1->SetSizeHints(this);
	//*)
    
    AddEffectPanelButtons(manager, this, GridSizer1);
    GridSizer1->Fit(this);
    GridSizer1->SetSizeHints(this);
}

EffectIconPanel::~EffectIconPanel()
{
	//(*Destroy(EffectIconPanel)
	//*)
}

