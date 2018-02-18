#include "SequenceVideoPanel.h"

//(*InternalHeaders(SequenceVideoPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(SequenceVideoPanel)
//*)

BEGIN_EVENT_TABLE(SequenceVideoPanel,wxPanel)
	//(*EventTable(SequenceVideoPanel)
	//*)
END_EVENT_TABLE()

SequenceVideoPanel::SequenceVideoPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(SequenceVideoPanel)
	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	//*)
}

SequenceVideoPanel::~SequenceVideoPanel()
{
	//(*Destroy(SequenceVideoPanel)
	//*)
}

