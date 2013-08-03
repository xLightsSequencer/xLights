#include "EffectTreeDialog.h"

//(*InternalHeaders(EffectTreeDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(EffectTreeDialog)
const long EffectTreeDialog::ID_TREECTRL1 = wxNewId();
const long EffectTreeDialog::ID_BUTTON1 = wxNewId();
const long EffectTreeDialog::ID_BUTTON2 = wxNewId();
const long EffectTreeDialog::ID_BUTTON3 = wxNewId();
const long EffectTreeDialog::ID_BUTTON4 = wxNewId();
const long EffectTreeDialog::ID_BUTTON5 = wxNewId();
//*)

BEGIN_EVENT_TABLE(EffectTreeDialog,wxDialog)
	//(*EventTable(EffectTreeDialog)
	//*)
END_EVENT_TABLE()

EffectTreeDialog::EffectTreeDialog(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(EffectTreeDialog)
	wxFlexGridSizer* FlexGridSizer2;
	wxBoxSizer* BoxSizer1;
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer2 = new wxFlexGridSizer(0, 2, 0, 0);
	TreeCtrl1 = new wxTreeCtrl(this, ID_TREECTRL1, wxDefaultPosition, wxSize(200,300), wxTR_DEFAULT_STYLE, wxDefaultValidator, _T("ID_TREECTRL1"));
	FlexGridSizer2->Add(TreeCtrl1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	BoxSizer1 = new wxBoxSizer(wxVERTICAL);
	Button1 = new wxButton(this, ID_BUTTON1, _("Label"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	BoxSizer1->Add(Button1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button2 = new wxButton(this, ID_BUTTON2, _("Label"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	BoxSizer1->Add(Button2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button3 = new wxButton(this, ID_BUTTON3, _("Label"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON3"));
	BoxSizer1->Add(Button3, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button4 = new wxButton(this, ID_BUTTON4, _("Label"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON4"));
	BoxSizer1->Add(Button4, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer2->Add(BoxSizer1, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button5 = new wxButton(this, ID_BUTTON5, _("&Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON5"));
	FlexGridSizer1->Add(Button5, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)
	treeRootID = TreeCtrl1->AddRoot(wxT("Effect Presets"));
}

EffectTreeDialog::~EffectTreeDialog()
{
	//(*Destroy(EffectTreeDialog)
	//*)
}

void EffectTreeDialog::InitItems(wxXmlNode *e)
{
    treeFavoritesGroupID = TreeCtrl1->AppendItem(treeRootID, "Favorites", -1,-1, NULL);
    treeUserGroupID = TreeCtrl1->AppendItem(treeRootID, "User Group", -1,-1, NULL);
    treeNCcomGroupID = TreeCtrl1->AppendItem(treeRootID, "Nutcraker.com Effects", -1,-1, NULL);
    TreeCtrl1->AppendItem(treeNCcomGroupID, "is a call", -1,-1, NULL);
    TreeCtrl1->AppendItem(treeUserGroupID, "This", -1,-1, NULL);
    TreeCtrl1->AppendItem(treeNCcomGroupID, "Foo Fighters", -1,-1, NULL);
    for (int i=0; i < 20; i++)
    {
        TreeCtrl1->AppendItem(treeFavoritesGroupID, "Foo Fighters", -1,-1, NULL);
    }
}
