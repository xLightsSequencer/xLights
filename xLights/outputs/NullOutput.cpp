#include "NullOutput.h"
#include "../OutputModelManager.h"

#include <wx/xml/xml.h>
#include <wx/propgrid/propgrid.h>
#include <wx/propgrid/advprops.h>

//#ifndef EXCLUDENETWORKUI
//#include "NullOutputDialog.h"
//#endif

#pragma region Save
void NullOutput::Save(wxXmlNode* node)
{
    // Dont need these any more
    //node->AddAttribute("Id", wxString::Format(wxT("%i"), GetId()));

    Output::Save(node);
}

wxXmlNode* NullOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}
#pragma endregion

#pragma region Getters and Setters
NullOutput::NullOutput(wxXmlNode* node) : Output(node)
{
    SetId(wxAtoi(node->GetAttribute("Id", "0")));
}

std::string NullOutput::GetLongDescription() const
{
    std::string res = "";

    if (!_enabled) res += "INACTIVE ";
    res += "NULL ";
    res += "(" + std::string(wxString::Format(wxT("%d"), _startChannel)) + "-" + std::string(wxString::Format(wxT("%d"), GetEndChannel())) + ")";

    return res;
}

std::string NullOutput::GetSortName() const
{
    return wxString::Format("NULL%02d", _nullNumber).ToStdString();
}

std::string NullOutput::GetExport() const
{
    // "Output Number,Start Absolute,End Absolute,Type,IP,Multicast,Universe/Id,Comm Port,Baud Rate,Description,Channels,Active,Suppress Duplicates,Auto Size,
    // FPP Proxy,Keep Channel Numbers,Channels Per Packet,Port,Dont Configure,Priority,Vendor,Model,Supports Virtual Strings,Supports Smart Remotes";
    return wxString::Format("%d,%ld,%ld,%s,,,%d,,,%s,%ld,,,,,,,,,,,,,",
        _outputNumber, GetStartChannel(), GetEndChannel(), GetType(), GetId(), _description, _channels).ToStdString();
}

void NullOutput::AddProperties(wxPropertyGrid* propertyGrid, bool allSameSize)
{
    wxPGProperty* p = propertyGrid->Append(new wxUIntProperty("Channels", "Channels", GetChannels()));
    p->SetAttribute("Min", 1);
    p->SetAttribute("Max", 10000000);
    p->SetEditor("SpinCtrl");
}

bool NullOutput::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager)
{
    wxString name = event.GetPropertyName();
    wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());

    if (name == "Channels")
    {
        SetChannels(event.GetValue().GetLong());
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANGE, "NullOutput::HandlePropertyEvent::Channels");
        outputModelManager->AddASAPWork(OutputModelManager::WORK_NETWORK_CHANNELSCHANGE, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddASAPWork(OutputModelManager::WORK_UPDATE_NETWORK_LIST, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        outputModelManager->AddLayoutTabWork(OutputModelManager::WORK_CALCULATE_START_CHANNELS, "NullOutput::HandlePropertyEvent::Channels", nullptr);
        return true;
    }

    if (Output::HandlePropertyEvent(event, outputModelManager)) return true;

    return false;
}

#pragma endregion Getters and Setters

//#pragma region UI
//#ifndef EXCLUDENETWORKUI
//Output* NullOutput::Configure(wxWindow* parent, OutputManager* outputManager, ModelManager* modelManager)
//{
//    NullOutputDialog dlg(parent, this, outputManager);
//
//    int res = dlg.ShowModal();
//
//    if (res == wxID_CANCEL)
//    {
//        return nullptr;
//    }

//    return this;
//}
//#endif
//#pragma endregion UI
