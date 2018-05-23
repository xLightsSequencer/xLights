#include "WarpPanel.h"
#include "xlGLCanvas.h"

//(*InternalHeaders(WarpPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(WarpPanel)
const long WarpPanel::ID_STATICTEXT1 = wxNewId();
const long WarpPanel::ID_CHOICE_Warp_Effect = wxNewId();
const long WarpPanel::ID_STATICTEXT2 = wxNewId();
const long WarpPanel::ID_CHOICE_Warp_Type = wxNewId();
//*)

const long WarpPanel::ID_CANVAS = wxNewId();

WarpPreview::WarpPreview( wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size, long style, const wxString &name, bool coreProfile)
   : xlGLCanvas( parent, id, pos, size, style, name, coreProfile )
{

}

WarpPreview::~WarpPreview()
{

}

void WarpPreview::InitializeGLCanvas()
{
   SetCurrentGLContext();

   mIsInitialized = true;
}

BEGIN_EVENT_TABLE(WarpPanel,wxPanel)
	//(*EventTable(WarpPanel)
	//*)
END_EVENT_TABLE()

WarpPanel::WarpPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(WarpPanel)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;
	wxFlexGridSizer* PreviewSizer;

	Create(parent, wxID_ANY, wxDefaultPosition, wxSize(230,145), wxTAB_TRAVERSAL, _T("wxID_ANY"));
	SetMinSize(wxDLG_UNIT(parent,wxSize(-1,-1)));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(3, 2, 0, 0);
	StaticText1 = new wxStaticText(this, ID_STATICTEXT1, _("Warp Effect"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT1"));
	FlexGridSizer2->Add(StaticText1, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Warp_Effect = new wxComboBox(this, ID_CHOICE_Warp_Effect, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_READONLY, wxDefaultValidator, _T("ID_CHOICE_Warp_Effect"));
	Choice_Warp_Effect->Append(_("ripple"));
	Choice_Warp_Effect->Append(_("dissolve"));
	FlexGridSizer2->Add(Choice_Warp_Effect, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	StaticText2 = new wxStaticText(this, ID_STATICTEXT2, _("Warp Type"), wxDefaultPosition, wxDefaultSize, 0, _T("ID_STATICTEXT2"));
	FlexGridSizer2->Add(StaticText2, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	Choice_Warp_Type = new wxComboBox(this, ID_CHOICE_Warp_Type, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0, 0, wxCB_READONLY, wxDefaultValidator, _T("ID_CHOICE_Warp_Type"));
	Choice_Warp_Type->Append(_("in"));
	Choice_Warp_Type->Append(_("out"));
	FlexGridSizer2->Add(Choice_Warp_Type, 1, wxALL|wxALIGN_LEFT|wxALIGN_CENTER_VERTICAL, 5);
	PreviewSizer = new wxFlexGridSizer(1, 1, 0, 0);
	FlexGridSizer2->Add(PreviewSizer, 1, wxALL|wxEXPAND|wxFIXED_MINSIZE, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	SetSizer(FlexGridSizer1);
	Layout();
	//*)

	_preview = new WarpPreview( this, ID_CANVAS );
	PreviewSizer->Add(_preview, 1, wxALL | wxEXPAND, 0);
}

WarpPanel::~WarpPanel()
{
	//(*Destroy(WarpPanel)
	//*)
}

