#include "LorController.h"
#include "wx/wxchar.h"

LorController::LorController()
{
    _unit_id = 1;
    _num_channels = 16;
    _type = "AC Controller";
    _mode = LOR_ADDR_MODE_NORMAL;
    _description = "";
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

LorController::~LorController()
{
}

LorController::LorController(wxXmlNode* node)
{
    _unit_id = wxAtoi(node->GetAttribute("UnitId", "1"));
    _num_channels = wxAtoi(node->GetAttribute("NumChannels", "16"));;
    _type = node->GetAttribute("CntlrType", "AC Controller").ToStdString();
    _mode = AddressMode(wxAtoi(node->GetAttribute("AddrMode", "1")));
    _description = node->GetAttribute("CntlrDesc", "LOR Controller").ToStdString();
    _changeCount = 0;
    _lastSavedChangeCount = 0;
}

void LorController::Save(wxXmlNode* node)
{
    node->AddAttribute("UnitId", wxString::Format("%d", _unit_id));
    node->AddAttribute("NumChannels", wxString::Format("%d", _num_channels));
    node->AddAttribute("CntlrType", wxString::Format("%s", _type));
    node->AddAttribute("AddrMode", wxString::Format("%d", _mode));
    node->AddAttribute("CntlrDesc", wxString::Format("%s", _description));
}

std::string LorController::GetModeString() const
{
    std::string result = "";
    if( _mode == LOR_ADDR_MODE_NORMAL ) {
        result = "Normal";
    }
    else if( _mode == LOR_ADDR_MODE_LEGACY ) {
        result = "Legacy";
    }
    else if( _mode == LOR_ADDR_MODE_SPLIT ) {
        result = "Split";
    }
    return result;
}
