#include "ScanObject.h"

#include <wx/socket.h>
#include "../xLights/UtilFunctions.h"

void IPObject::CheckPort80()
{
    wxIPV4address addr;
    addr.Hostname(_ip);
    addr.Service(80);

    wxSocketClient* const client = new wxSocketClient(wxSOCKET_WAITALL | wxSOCKET_BLOCK);
    client->SetTimeout(5);

    if (client != nullptr) {
        if (client->Connect(addr)) {
            _port80 = true;
        }
        delete client;
    }
}

bool IPObject::InSameSubnet(const std::string& ip) const
{
    return IsInSameSubnet(_ip, ip);
}

bool IPObject::operator<(const IPObject& ip2)
{
    wxIPV4address i1;
    wxIPV4address i2;
    i1.Hostname(_ip);
    i2.Hostname(ip2._ip);

    return (((sockaddr_in*)i1.GetAddressData())->sin_addr.s_addr <
            ((sockaddr_in*)i2.GetAddressData())->sin_addr.s_addr);
}
