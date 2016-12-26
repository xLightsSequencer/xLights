#include "IPOutput.h"

#include <wx/xml/xml.h>
#include <wx/regex.h>
#include <log4cpp/Category.hh>

#pragma region Constructors and Destructors
IPOutput::IPOutput(wxXmlNode* node) : Output(node)
{
    _ip = node->GetAttribute("ComPort", "").ToStdString();
    _universe = wxAtoi(node->GetAttribute("BaudRate", "1"));
}

IPOutput::IPOutput() : Output()
{
    _universe = 0;
    _ip = "";
}
#pragma endregion Constructors and Destructors

#pragma region Static Functions
bool IPOutput::IsIPValid(std::string ip)
{
    wxString ips = wxString(ip).Trim(false).Trim(true);
    if (ips == "")
    {
        return false;
    }
    else
    {
        wxRegEx regxIPAddr("^(([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]{1}|[0-9]{2}|[0-1][0-9]{2}|2[0-4][0-9]|25[0-5])$");

        if (regxIPAddr.Matches(ips))
        {
            return true;
        }
    }

    return false;
}

std::string IPOutput::CleanupIP(std::string ip)
{
    wxString IpAddr(ip.c_str());
    static wxRegEx leadingzero1("(^0+)(?:[1-9]|0\\.)", wxRE_ADVANCED);
    if (leadingzero1.Matches(IpAddr))
    {
        wxString s0 = leadingzero1.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero1.GetMatch(IpAddr, 1);
        leadingzero1.ReplaceFirst(&IpAddr, "" + s0.Right(s0.size() - s1.size()));
    }
    static wxRegEx leadingzero2("(\\.0+)(?:[1-9]|0\\.|0$)", wxRE_ADVANCED);
    while (leadingzero2.Matches(IpAddr)) // need to do it several times because the results overlap
    {
        wxString s0 = leadingzero2.GetMatch(IpAddr, 0);
        wxString s1 = leadingzero2.GetMatch(IpAddr, 1);
        leadingzero2.ReplaceFirst(&IpAddr, "." + s0.Right(s0.size() - s1.size()));
    }

    return IpAddr.ToStdString();
}
#pragma endregion Static Functions

wxXmlNode* IPOutput::Save()
{
    wxXmlNode* node = new wxXmlNode(wxXML_ELEMENT_NODE, "network");
    Save(node);

    return node;
}

void IPOutput::Save(wxXmlNode* node)
{
    if (_ip != "")
    {
        node->AddAttribute("ComPort", _ip);
    }

    if (_universe != 0)
    {
        node->AddAttribute("BaudRate", wxString::Format("%d", _universe));
    }

    Output::Save(node);
}

#pragma region Operators
bool IPOutput::operator==(const IPOutput& output) const
{
    if (GetType() != output.GetType()) return false;

    return _universe == output.GetUniverse() && _ip == output.GetIP();
}
#pragma endregion Operators

