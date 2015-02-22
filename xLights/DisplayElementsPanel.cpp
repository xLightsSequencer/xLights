#include "DisplayElementsPanel.h"
#include "../include/eye-16.xpm"
#include "../include/eye-16_gray.xpm"

//(*InternalHeaders(DisplayElementsPanel)
#include <wx/intl.h>
#include <wx/string.h>
//*)

//(*IdInit(DisplayElementsPanel)
const long DisplayElementsPanel::ID_LISTCTRL_DISPLAY_ELEMENTS = wxNewId();
//*)

BEGIN_EVENT_TABLE(DisplayElementsPanel,wxPanel)
	//(*EventTable(DisplayElementsPanel)
	//*)
END_EVENT_TABLE()

DisplayElementsPanel::DisplayElementsPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(DisplayElementsPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, id, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("id"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 3, 0, 0);
	ListCtrlDisplayElements = new wxCheckedListCtrl(this, ID_LISTCTRL_DISPLAY_ELEMENTS, wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_LISTCTRL_DISPLAY_ELEMENTS"));
	FlexGridSizer1->Add(ListCtrlDisplayElements, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);
	//*)

	ListCtrlDisplayElements->SetImages((char**)eye_16,(char**)eye_16_gray);
}

DisplayElementsPanel::~DisplayElementsPanel()
{
	//(*Destroy(DisplayElementsPanel)
	//*)
}

void DisplayElementsPanel::SetSequenceElements(SequenceElements* elements)
{
    mSequenceElements = elements;
}

void DisplayElementsPanel::Initialize()
{
// Add first column
    ListCtrlDisplayElements->ClearAll();

	wxListItem col0;
	col0.SetId(0);
	col0.SetText( _("") );
	col0.SetWidth(30);
	ListCtrlDisplayElements->InsertColumn(0, col0);

	wxListItem col1;
	col1.SetId(1);
	col1.SetText( _("Element") );
	col1.SetWidth(130);
	ListCtrlDisplayElements->InsertColumn(1, col1);

	wxListItem col2;
	col2.SetId(2);
	col2.SetText( _("Type") );
	col2.SetWidth(50);
	ListCtrlDisplayElements->InsertColumn(2, col2);


    int j=0;
    for(int i=0;i<mSequenceElements->GetElementCount();i++)
    {
        if(mSequenceElements->GetElement(i)->GetType()!= "timing")
        {
            wxListItem li;
            li.SetId(j);
    //        li.SetText(mSequenceElements->GetElement(i)->GetName());
            ListCtrlDisplayElements->InsertItem(li);
            ListCtrlDisplayElements->SetItem(j,1,mSequenceElements->GetElement(i)->GetName());
            wxString type = mSequenceElements->GetElement(i)->GetType() == "model"?"Model":"View";
            ListCtrlDisplayElements->SetItem(j,1,mSequenceElements->GetElement(i)->GetName());
            ListCtrlDisplayElements->SetItem(j,2,type);
            ListCtrlDisplayElements->SetChecked(j,mSequenceElements->GetElement(i)->GetVisible());
            j++;
        }
    }
}



