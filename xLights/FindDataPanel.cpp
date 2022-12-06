/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/smeighan/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/smeighan/xLights/blob/master/License.txt
 **************************************************************/

#include "FindDataPanel.h"
#include "sequencer/DragEffectBitmapButton.h"
#include "BitmapCache.h"
#include "UtilFunctions.h"
#include "sequencer/SequenceElements.h"
#include "xLightsMain.h"
#include "sequencer/MainSequencer.h"

//(*InternalHeaders(FindDataPanel)
#include <wx/intl.h>
#include <wx/listctrl.h>
#include <wx/sizer.h>
#include <wx/string.h>
//*)

//(*IdInit(FindDataPanel)
const long FindDataPanel::ID_LISTVIEW1 = wxNewId();
//*)

#include "effects/EffectManager.h"

BEGIN_EVENT_TABLE(FindDataPanel,wxPanel)
	//(*EventTable(FindDataPanel)
	//*)
END_EVENT_TABLE()

FindDataPanel::FindDataPanel(wxWindow* parent,wxWindowID id,const wxPoint& pos,const wxSize& size)
{
	//(*Initialize(FindDataPanel)
	wxFlexGridSizer* FlexGridSizer1;

	Create(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTAB_TRAVERSAL, _T("wxID_ANY"));
	FlexGridSizer1 = new wxFlexGridSizer(0, 1, 0, 0);
	FlexGridSizer1->AddGrowableCol(0);
	FlexGridSizer1->AddGrowableRow(0);
	ListView_FoundEffects = new wxListView(this, ID_LISTVIEW1, wxDefaultPosition, wxDefaultSize, wxLC_REPORT, wxDefaultValidator, _T("ID_LISTVIEW1"));
	FlexGridSizer1->Add(ListView_FoundEffects, 1, wxALL|wxEXPAND, 5);
	SetSizer(FlexGridSizer1);

	Connect(ID_LISTVIEW1,wxEVT_COMMAND_LIST_ITEM_ACTIVATED,(wxObjectEventFunction)&FindDataPanel::OnListView_FoundEffectsItemActivated);
	//*)

	std::vector<findDataEffect> effects;
    UpdateEffects(effects, nullptr);
}

FindDataPanel::~FindDataPanel()
{
	//(*Destroy(FindDataPanel)
	//*)
}

#define COL_MODEL 0
#define COL_STRANDSUBMODEL 1
#define COL_LAYERNODE 2
#define COL_EFFECT 3
#define COL_TIME 4

void FindDataPanel::UpdateEffects(const std::vector<findDataEffect>& effects, const EffectsGrid* eg)
{
    _effectsGrid = eg;

    ListView_FoundEffects->ClearAll();

	ListView_FoundEffects->AppendColumn("Element");
    ListView_FoundEffects->AppendColumn("Strand/Submodel");
    ListView_FoundEffects->AppendColumn("Layer/Node");
    ListView_FoundEffects->AppendColumn("Effect");
    ListView_FoundEffects->AppendColumn("Start");

	for (const auto& it : effects) {
        long index = ListView_FoundEffects->InsertItem(ListView_FoundEffects->GetItemCount(), it.GetName());
        if (it.e != nullptr) {
            ListView_FoundEffects->SetItem(index, COL_STRANDSUBMODEL, it.GetStrandSubmodel());
        }
        if (it.el != nullptr) {
            ListView_FoundEffects->SetItem(index, COL_LAYERNODE, wxString::Format("Layer %d", it.el->GetLayerNumber()));
        } else if (it.nl != nullptr) {
            ListView_FoundEffects->SetItem(index, COL_LAYERNODE, wxString::Format("Node %d", it.GetNode() + 1));
        }
        if (it.ef != nullptr) {
            ListView_FoundEffects->SetItem(index, COL_EFFECT, it.ef->GetEffectName());
            ListView_FoundEffects->SetItem(index, COL_TIME, wxString::Format("%0.2f", (float)it.ef->GetStartTimeMS() / 1000.0));
        }

        if (it.dl != nullptr) {
            ListView_FoundEffects->SetItemTextColour(index, *wxBLUE);
        }
    }

    if (effects.size() == 0) {
        long index = ListView_FoundEffects->InsertItem(ListView_FoundEffects->GetItemCount(), "No effects found ... try render all");
        ListView_FoundEffects->SetItemTextColour(index, *wxRED);
    }

    ListView_FoundEffects->SetColumnWidth(COL_MODEL, wxLIST_AUTOSIZE);
    if (ListView_FoundEffects->GetColumnWidth(COL_MODEL) < 60) {
        ListView_FoundEffects->SetColumnWidth(COL_MODEL, 60);
    }
    ListView_FoundEffects->SetColumnWidth(COL_STRANDSUBMODEL, wxLIST_AUTOSIZE);
    if (ListView_FoundEffects->GetColumnWidth(COL_STRANDSUBMODEL) < 140) {
        ListView_FoundEffects->SetColumnWidth(COL_STRANDSUBMODEL, 140);
    }
    ListView_FoundEffects->SetColumnWidth(COL_LAYERNODE, wxLIST_AUTOSIZE);
    if (ListView_FoundEffects->GetColumnWidth(COL_LAYERNODE) < 140) {
        ListView_FoundEffects->SetColumnWidth(COL_LAYERNODE, 140);
    }
    ListView_FoundEffects->SetColumnWidth(COL_EFFECT, wxLIST_AUTOSIZE);
    if (ListView_FoundEffects->GetColumnWidth(COL_EFFECT) < 50) {
        ListView_FoundEffects->SetColumnWidth(COL_EFFECT, 50);
    }
    ListView_FoundEffects->SetColumnWidth(COL_TIME, wxLIST_AUTOSIZE);
    if (ListView_FoundEffects->GetColumnWidth(COL_TIME) < 50) {
        ListView_FoundEffects->SetColumnWidth(COL_TIME, 50);
    }
}

void FindDataPanel::OnListView_FoundEffectsItemActivated(wxListEvent& event)
{
    if (_effectsGrid != nullptr && ListView_FoundEffects->GetItemText(event.GetIndex(), COL_TIME) != "") {
        auto se = _effectsGrid->GetSequenceElements();
        TimeLine* tl = se->GetXLightsFrame()->GetMainSequencer()->PanelTimeLine;

        // scroll time
        tl->SetTimelinePosition(wxAtof(ListView_FoundEffects->GetItemText(event.GetIndex(), COL_TIME)) * 1000);

        // scroll model
        // this currently will only go to the model ... not the submodel/strand/node
        Element* elem = se->GetElement(ListView_FoundEffects->GetItemText(event.GetIndex(), COL_MODEL));
        if (elem != nullptr) {
            for (int row = 0; row < se->GetRowInformationSize(); row++) {
                EffectLayer* el = se->GetEffectLayer(row);
                if (el->GetParentElement()->GetModelName() == elem->GetName()) {
                    se->GetXLightsFrame()->GetMainSequencer()->ScrollToRow(row - se->GetNumberOfTimingRows());
                    break;
                }
            }
        }
    }
}
