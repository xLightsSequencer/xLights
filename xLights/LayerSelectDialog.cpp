/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "LayerSelectDialog.h"

//(*InternalHeaders(LayerSelectDialog)
#include <wx/intl.h>
#include <wx/string.h>
//*)

#include <wx/menu.h>

//(*IdInit(LayerSelectDialog)
const long LayerSelectDialog::ID_CHECKLISTBOX1 = wxNewId();
const long LayerSelectDialog::ID_BUTTON1 = wxNewId();
const long LayerSelectDialog::ID_BUTTON2 = wxNewId();
//*)

const long LayerSelectDialog::ID_MCU_SELECTALL = wxNewId();
const long LayerSelectDialog::ID_MCU_SELECTNONE = wxNewId();
const long LayerSelectDialog::ID_MCU_SELECTPOPULATED = wxNewId();

BEGIN_EVENT_TABLE(LayerSelectDialog,wxDialog)
	//(*EventTable(LayerSelectDialog)
	//*)
END_EVENT_TABLE()

LayerSelectDialog::LayerSelectDialog(wxWindow* parent, int startLayer, int endLayer, std::string layersSelected, std::vector<int> layerWithEffect, wxWindowID id,const wxPoint& pos,const wxSize& size):
	_layerWithEffect(layerWithEffect)
{
	//(*Initialize(LayerSelectDialog)
	wxFlexGridSizer* FlexGridSizer1;
	wxFlexGridSizer* FlexGridSizer2;

	Create(parent, id, _("Choose canvas layers ..."), wxDefaultPosition, wxDefaultSize, wxCAPTION|wxRESIZE_BORDER|wxMAXIMIZE_BOX, _T("id"));
	SetClientSize(wxDefaultSize);
	Move(wxDefaultPosition);
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	CheckListBox_Layers = new wxCheckListBox(this, ID_CHECKLISTBOX1, wxDefaultPosition, wxSize(-1,300), 0, 0, 0, wxDefaultValidator, _T("ID_CHECKLISTBOX1"));
	FlexGridSizer1->Add(CheckListBox_Layers, 1, wxALL|wxEXPAND, 5);
	FlexGridSizer2 = new wxFlexGridSizer(0, 3, 0, 0);
	Button_Ok = new wxButton(this, ID_BUTTON1, _("Ok"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON1"));
	Button_Ok->SetDefault();
	FlexGridSizer2->Add(Button_Ok, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	Button_Cancel = new wxButton(this, ID_BUTTON2, _("Cancel"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator, _T("ID_BUTTON2"));
	FlexGridSizer2->Add(Button_Cancel, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	FlexGridSizer1->Add(FlexGridSizer2, 1, wxALL|wxALIGN_CENTER_HORIZONTAL|wxALIGN_CENTER_VERTICAL, 5);
	SetSizer(FlexGridSizer1);
	FlexGridSizer1->Fit(this);
	FlexGridSizer1->SetSizeHints(this);

	Connect(ID_CHECKLISTBOX1,wxEVT_COMMAND_CHECKLISTBOX_TOGGLED,(wxObjectEventFunction)&LayerSelectDialog::OnCheckListBox_LayersToggled);
	Connect(ID_BUTTON1,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayerSelectDialog::OnButton_OkClick);
	Connect(ID_BUTTON2,wxEVT_COMMAND_BUTTON_CLICKED,(wxObjectEventFunction)&LayerSelectDialog::OnButton_CancelClick);
	//*)

	Connect(ID_CHECKLISTBOX1, wxEVT_CONTEXT_MENU, (wxObjectEventFunction)&LayerSelectDialog::OnListRClick);

    _start = startLayer;

    for (int i = startLayer; i <= endLayer; i++)
    {
        CheckListBox_Layers->Append(wxString::Format("Layer %d", i));
    }

    if (layersSelected == "")
    {
        SelectAllLayers();
    }
    else
    {
        wxArrayString layers = wxSplit(layersSelected, '|');
        for (const auto& it : layers)
        {
            int l = wxAtoi(it);
            if (l < (int)CheckListBox_Layers->GetCount())
            {
                CheckListBox_Layers->Check(l);
            }
        }
    }

    SetEscapeId(Button_Cancel->GetId());
    
    ValidateWindow();
}

LayerSelectDialog::~LayerSelectDialog()
{
	//(*Destroy(LayerSelectDialog)
	//*)
}

std::string LayerSelectDialog::GetSelectedLayers() const
{
    std::string res;
    wxArrayInt items;
    CheckListBox_Layers->GetCheckedItems(items);
    for (auto it = items.begin(); it != items.end(); ++it)
    {
        if (res != "") res += "|";
        res = res + wxString::Format("%d", *it).ToStdString();
    }

    return res;
}

void LayerSelectDialog::SelectAllLayers()
{
    for (size_t i = 0; i < CheckListBox_Layers->GetCount(); i++)
    {
        CheckListBox_Layers->Check(i);
    }
}


void LayerSelectDialog::OnButton_CancelClick(wxCommandEvent& event)
{
    EndDialog(wxID_CANCEL);
}

void LayerSelectDialog::OnButton_OkClick(wxCommandEvent& event)
{
    EndDialog(wxID_OK);
}

void LayerSelectDialog::OnCheckListBox_LayersToggled(wxCommandEvent& event)
{
    ValidateWindow();
}

void LayerSelectDialog::ValidateWindow()
{
    wxArrayInt items;
    CheckListBox_Layers->GetCheckedItems(items);
    if (items.Count() == 0)
    {
        Button_Ok->Enable(false);
    }
    else
    {
        Button_Ok->Enable(true);
    }
}

void LayerSelectDialog::OnListRClick(wxContextMenuEvent& event)
{
	wxMenu mnu;
	mnu.Append(ID_MCU_SELECTALL, "Select All");
	mnu.Append(ID_MCU_SELECTNONE, "Deselect All");
	mnu.Append(ID_MCU_SELECTPOPULATED, "Select Layers With Effects");

	mnu.Connect(wxEVT_MENU, (wxObjectEventFunction)&LayerSelectDialog::OnPopup, nullptr, this);
	PopupMenu(&mnu);
}

void LayerSelectDialog::OnPopup(wxCommandEvent& event)
{
	if (event.GetId() == ID_MCU_SELECTALL)
	{
		SelectAllLayers();
	}
	else if (event.GetId() == ID_MCU_SELECTNONE)
	{
		DeselectAllLayers();
	}
	else if (event.GetId() == ID_MCU_SELECTPOPULATED)
	{
		SelectLayersWithEffects();
	}
}

void LayerSelectDialog::DeselectAllLayers()
{
	for (size_t i = 0; i < CheckListBox_Layers->GetCount(); i++)
	{
		CheckListBox_Layers->Check(i, false);
	}
}

void LayerSelectDialog::SelectLayersWithEffects()
{
	for (size_t i = 0; i < CheckListBox_Layers->GetCount(); i++)
	{
		int layer = _start + i - 1;//zero based index
		if (std::find(_layerWithEffect.begin(), _layerWithEffect.end(), layer) != _layerWithEffect.end())
		{
			CheckListBox_Layers->Check(i);
		}
		else
		{
			CheckListBox_Layers->Check(i, false);
		}
	}
}
