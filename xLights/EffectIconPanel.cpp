#include "EffectIconPanel.h"
#include "DragEffectBitmapButton.h"

//(*InternalHeaders(EffectIconPanel)
#include <wx/settings.h>
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectIconPanel)
const long EffectIconPanel::ID_BITMAPBUTTON1 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON2 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON3 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON4 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON5 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON6 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON7 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON8 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON9 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON10 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON11 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON12 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON13 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON14 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON15 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON16 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON17 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON18 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON19 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON20 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON21 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON22 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON23 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON24 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON25 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON26 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON27 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON28 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON29 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON30 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON31 = wxNewId();
const long EffectIconPanel::ID_BITMAPBUTTON32 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectIconPanel,wxPanel)
	//(*EventTable(EffectIconPanel)
	//*)
END_EVENT_TABLE()

EffectIconPanel::EffectIconPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EffectIconPanel)
	wxGridSizer* GridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	GridSizer1 = new wxGridSizer(7, 5, 0, 0);
	BitmapButton1 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON1, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON1"));
	BitmapButton1->SetMinSize(wxSize(16,16));
	BitmapButton1->SetMaxSize(wxSize(16,16));
	BitmapButton1->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton1->SetEffectIndex(0);
	GridSizer1->Add(BitmapButton1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BitmapButton2 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON2, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON2"));
	BitmapButton2->SetMinSize(wxSize(16,16));
	BitmapButton2->SetMaxSize(wxSize(16,16));
	BitmapButton2->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton2->SetEffectIndex(1);
	GridSizer1->Add(BitmapButton2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 3);
	BitmapButton3 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON3, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON3"));
	BitmapButton3->SetMinSize(wxSize(16,16));
	BitmapButton3->SetMaxSize(wxSize(16,16));
	BitmapButton3->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton3->SetEffectIndex(2);
	GridSizer1->Add(BitmapButton3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton4 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON4, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON4"));
	BitmapButton4->SetMinSize(wxSize(16,16));
	BitmapButton4->SetMaxSize(wxSize(16,16));
	BitmapButton4->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton4->SetEffectIndex(3);
	GridSizer1->Add(BitmapButton4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton5 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON5, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON5"));
	BitmapButton5->SetMinSize(wxSize(16,16));
	BitmapButton5->SetMaxSize(wxSize(16,16));
	BitmapButton5->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton5->SetEffectIndex(4);
	GridSizer1->Add(BitmapButton5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton6 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON6, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON6"));
	BitmapButton6->SetMinSize(wxSize(16,16));
	BitmapButton6->SetMaxSize(wxSize(16,16));
	BitmapButton6->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton6->SetEffectIndex(5);
	GridSizer1->Add(BitmapButton6, 1, wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton7 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON7, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON7"));
	BitmapButton7->SetMinSize(wxSize(16,16));
	BitmapButton7->SetMaxSize(wxSize(16,16));
	BitmapButton7->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton7->SetEffectIndex(6);
	GridSizer1->Add(BitmapButton7, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton8 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON8, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON8"));
	BitmapButton8->SetMinSize(wxSize(16,16));
	BitmapButton8->SetMaxSize(wxSize(16,16));
	BitmapButton8->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton8->SetEffectIndex(7);
	GridSizer1->Add(BitmapButton8, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton9 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON9, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON9"));
	BitmapButton9->SetMinSize(wxSize(16,16));
	BitmapButton9->SetMaxSize(wxSize(16,16));
	BitmapButton9->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton9->SetEffectIndex(8);
	GridSizer1->Add(BitmapButton9, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton10 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON10, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON10"));
	BitmapButton10->SetMinSize(wxSize(16,16));
	BitmapButton10->SetMaxSize(wxSize(16,16));
	BitmapButton10->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton10->SetEffectIndex(9);
	GridSizer1->Add(BitmapButton10, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton11 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON11, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON11"));
	BitmapButton11->SetMinSize(wxSize(16,16));
	BitmapButton11->SetMaxSize(wxSize(16,16));
	BitmapButton11->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton11->SetEffectIndex(10);
	GridSizer1->Add(BitmapButton11, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton12 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON12, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON12"));
	BitmapButton12->SetMinSize(wxSize(16,16));
	BitmapButton12->SetMaxSize(wxSize(16,16));
	BitmapButton12->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton12->SetEffectIndex(11);
	GridSizer1->Add(BitmapButton12, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton13 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON13, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON13"));
	BitmapButton13->SetMinSize(wxSize(16,16));
	BitmapButton13->SetMaxSize(wxSize(16,16));
	BitmapButton13->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton13->SetEffectIndex(12);
	GridSizer1->Add(BitmapButton13, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton14 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON14, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON14"));
	BitmapButton14->SetMinSize(wxSize(16,16));
	BitmapButton14->SetMaxSize(wxSize(16,16));
	BitmapButton14->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton14->SetEffectIndex(13);
	GridSizer1->Add(BitmapButton14, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton15 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON15, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON15"));
	BitmapButton15->SetMinSize(wxSize(16,16));
	BitmapButton15->SetMaxSize(wxSize(16,16));
	BitmapButton15->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton15->SetEffectIndex(14);
	GridSizer1->Add(BitmapButton15, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton16 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON16, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON16"));
	BitmapButton16->SetMinSize(wxSize(16,16));
	BitmapButton16->SetMaxSize(wxSize(16,16));
	BitmapButton16->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton16->SetEffectIndex(15);
	GridSizer1->Add(BitmapButton16, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton17 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON17, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON17"));
	BitmapButton17->SetMinSize(wxSize(16,16));
	BitmapButton17->SetMaxSize(wxSize(16,16));
	BitmapButton17->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton17->SetEffectIndex(16);
	GridSizer1->Add(BitmapButton17, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton18 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON18, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON18"));
	BitmapButton18->SetMinSize(wxSize(16,16));
	BitmapButton18->SetMaxSize(wxSize(16,16));
	BitmapButton18->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton18->SetEffectIndex(17);
	GridSizer1->Add(BitmapButton18, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton19 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON19, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON19"));
	BitmapButton19->SetMinSize(wxSize(16,16));
	BitmapButton19->SetMaxSize(wxSize(16,16));
	BitmapButton19->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton19->SetEffectIndex(18);
	GridSizer1->Add(BitmapButton19, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton20 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON20, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON20"));
	BitmapButton20->SetMinSize(wxSize(16,16));
	BitmapButton20->SetMaxSize(wxSize(16,16));
	BitmapButton20->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton20->SetEffectIndex(19);
	GridSizer1->Add(BitmapButton20, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton21 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON21, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON21"));
	BitmapButton21->SetMinSize(wxSize(16,16));
	BitmapButton21->SetMaxSize(wxSize(16,16));
	BitmapButton21->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton21->SetEffectIndex(20);
	GridSizer1->Add(BitmapButton21, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton22 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON22, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON22"));
	BitmapButton22->SetMinSize(wxSize(16,16));
	BitmapButton22->SetMaxSize(wxSize(16,16));
	BitmapButton22->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton22->SetEffectIndex(21);
	GridSizer1->Add(BitmapButton22, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton23 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON23, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON23"));
	BitmapButton23->SetMinSize(wxSize(16,16));
	BitmapButton23->SetMaxSize(wxSize(16,16));
	BitmapButton23->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton23->SetEffectIndex(22);
	GridSizer1->Add(BitmapButton23, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton24 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON24, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON24"));
	BitmapButton24->SetMinSize(wxSize(16,16));
	BitmapButton24->SetMaxSize(wxSize(16,16));
	BitmapButton24->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton24->SetEffectIndex(23);
	GridSizer1->Add(BitmapButton24, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton25 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON25, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON25"));
	BitmapButton25->SetMinSize(wxSize(16,16));
	BitmapButton25->SetMaxSize(wxSize(16,16));
	BitmapButton25->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton25->SetEffectIndex(24);
	GridSizer1->Add(BitmapButton25, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton26 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON26, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON26"));
	BitmapButton26->SetMinSize(wxSize(16,16));
	BitmapButton26->SetMaxSize(wxSize(16,16));
	BitmapButton26->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton26->SetEffectIndex(25);
	GridSizer1->Add(BitmapButton26, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton27 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON27, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON27"));
	BitmapButton27->SetMinSize(wxSize(16,16));
	BitmapButton27->SetMaxSize(wxSize(16,16));
	BitmapButton27->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton27->SetEffectIndex(26);
	GridSizer1->Add(BitmapButton27, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton28 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON28, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON28"));
	BitmapButton28->SetMinSize(wxSize(16,16));
	BitmapButton28->SetMaxSize(wxSize(16,16));
	BitmapButton28->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton28->SetEffectIndex(27);
	GridSizer1->Add(BitmapButton28, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton29 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON29, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON29"));
	BitmapButton29->SetMinSize(wxSize(16,16));
	BitmapButton29->SetMaxSize(wxSize(16,16));
	BitmapButton29->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton29->SetEffectIndex(28);
	GridSizer1->Add(BitmapButton29, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton30 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON30, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON30"));
	BitmapButton30->SetMinSize(wxSize(16,16));
	BitmapButton30->SetMaxSize(wxSize(16,16));
	BitmapButton30->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton30->SetEffectIndex(29);
	GridSizer1->Add(BitmapButton30, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton31 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON31, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON31"));
	BitmapButton31->SetMinSize(wxSize(16,16));
	BitmapButton31->SetMaxSize(wxSize(16,16));
	BitmapButton31->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton31->SetEffectIndex(30);
	GridSizer1->Add(BitmapButton31, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BitmapButton32 = new DragEffectBitmapButton(this, ID_BITMAPBUTTON32, wxNullBitmap, wxDefaultPosition, wxSize(16,16), wxBU_AUTODRAW|wxNO_BORDER, wxDefaultValidator, _T("ID_BITMAPBUTTON32"));
	BitmapButton32->SetMinSize(wxSize(16,16));
	BitmapButton32->SetMaxSize(wxSize(16,16));
	BitmapButton32->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND));
	BitmapButton32->SetEffectIndex(31);
	GridSizer1->Add(BitmapButton32, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(GridSizer1);
	GridSizer1->Fit(this);
	GridSizer1->SetSizeHints(this);
	//*)
}

EffectIconPanel::~EffectIconPanel()
{
	//(*Destroy(EffectIconPanel)
	//*)
}

