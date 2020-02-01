#include <wx/xml/xml.h>

#include "ControllerNull.h"
#include "OutputManager.h"
#include "Output.h"
#include "../UtilFunctions.h"
#include "../SpecialOptions.h"
#include "../OutputModelManager.h"
#include "NullOutput.h"

std::string ControllerNull::GetChannelMapping(int32_t ch) const
{
    return wxString::Format("Channel %ld maps to ...\nTYPE: NULL\nName: %s\nChannel: %ld", ch, GetName(), ch - GetStartChannel() + 1) + (IsActive() ? _("\n") : _(" INACTIVE\n"));
}

std::string ControllerNull::GetLongDescription() const
{
    std::string res = "";

    if (!IsActive()) res += "INACTIVE ";
    res += GetName() + "  NULL ";
    res += "(" + std::string(wxString::Format(wxT("%ld"), GetStartChannel())) + "-" + std::string(wxString::Format(wxT("%ld"), GetEndChannel())) + ") ";
    res += _description;

    return res;
}

void ControllerNull::SetId(int id)
{
    SetTheId(id);
    dynamic_cast<NullOutput*>(GetFirstOutput())->SetId(id);
}

wxXmlNode* ControllerNull::Save()
{
    wxXmlNode* um = Controller::Save();

    return um;
}

void ControllerNull ::AddProperties(wxPropertyGrid* propertyGrid)
{
    Controller::AddProperties(propertyGrid);

    if (_outputs.size() > 0) _outputs.front()->AddProperties(propertyGrid, true);
}

bool ControllerNull::HandlePropertyEvent(wxPropertyGridEvent& event, OutputModelManager* outputModelManager)
{
    if (Controller::HandlePropertyEvent(event, outputModelManager)) return true;

    wxString name = event.GetPropertyName();
    wxPropertyGrid* grid = dynamic_cast<wxPropertyGrid*>(event.GetEventObject());

    if (_outputs.size() > 0)
    {
        if (_outputs.front()->HandlePropertyEvent(event, outputModelManager)) return true;
    }

    return false;
}

ControllerNull::ControllerNull(OutputManager* om, wxXmlNode* node, const std::string& showDir) : Controller(om, node, showDir)
{
    _dirty = false;
    wxASSERT(_outputs.size() == 1);
}

ControllerNull::ControllerNull(OutputManager* om) : Controller(om)
{
    _name = om->UniqueName("Null_");
    _outputs.push_back(new NullOutput());
}

void ControllerNull::Convert(wxXmlNode* node, std::string showDir)
{
    Controller::Convert(node, showDir);

    _outputs.push_back(Output::Create(node, showDir));
    if (_name == "" || StartsWith(_name, "Null_"))
    {
        _id = dynamic_cast<NullOutput*>(_outputs.front())->GetId();
        if (_outputs.back()->GetDescription() != "")
        {
            _name = _outputManager->UniqueName(_outputs.back()->GetDescription());
        }
        else
        {
            _name = _outputManager->UniqueName("Unnamed");
        }
    }
    if (_outputs.back() == nullptr)
    {
        // this shouldnt happen unless we are loading a future file with an output type we dont recognise
        _outputs.pop_back();
    }
}