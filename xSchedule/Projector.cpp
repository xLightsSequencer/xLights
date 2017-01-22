#include "Projector.h"
#include <wx/xml/xml.h>

Projector::Projector(wxXmlNode* node)
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
    Load(node);
}

void Projector::Load(wxXmlNode* node)
{
    _name = node->GetAttribute("Projector", "").ToStdString();
    _ip = node->GetAttribute("IP", "").ToStdString();
    _password = node->GetAttribute("Password", "").ToStdString();
}

Projector::Projector()
{
    _changeCount = 0;
    _lastSavedChangeCount = 0;
	_name = "";
	_IP = "";
	_password = "";
}

wxXmlNode* Projector::Save()
{
    wxXmlNode* res = new wxXmlNode(nullptr, wxXML_ELEMENT_NODE, "Projector");

    res->AddAttribute("Projector", _name);
    res->AddAttribute("IP", _ip);
    res->AddAttribute("Password", _password);

    return res;
}
