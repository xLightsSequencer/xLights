/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "xLightsMain.h"

#include <wx/config.h>
#include <wx/timer.h>

#include "controllers/FPP.h"
#include "discovery/Discovery.h"
#include "outputs/ArtNetOutput.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/DDPOutput.h"
#include "outputs/TwinklyOutput.h"
#include "controllers/Pixlite16.h"
#include "utils/CurlManager.h"
#include "utils/ip_utils.h"

#include <log.h>

std::list<std::string> xLightsFrame::GetDiscoveryAddresses(std::list<std::string>* forcedOut) const {
    std::list<std::string> addresses;

    // Read forced IPs from config
    wxConfigBase* config = wxConfigBase::Get();
    wxString force;
    config->Read("FPPConnectForcedIPs", &force, "");
    if (!force.empty()) {
        auto const ips = Split(force.ToStdString(), '|');
        for (const auto& a : ips) {
            if (!a.empty()) {
                addresses.push_back(a);
                if (forcedOut) {
                    forcedOut->push_back(a);
                }
            }
        }
    }

    // Add active controller IPs from OutputManager
    for (auto& it : _outputManager.GetControllers()) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && !eth->GetIP().empty() && eth->GetIP() != "MULTICAST") {
            std::string resolvedIP = eth->GetResolvedIP(true);
            if (resolvedIP.empty()) {
                addresses.push_back(::Lower(eth->GetIP()));
            } else if (eth->IsActive() && (ip_utils::IsIPValid(resolvedIP) || resolvedIP != eth->GetIP())) {
                addresses.push_back(::Lower(resolvedIP));
            }
            if (!eth->GetFPPProxy().empty()) {
                addresses.push_back(::Lower(ip_utils::ResolveIP(eth->GetFPPProxy())));
            }
        }
    }

    addresses.sort();
    addresses.unique();
    return addresses;
}

void xLightsFrame::PrepareAllControllerDiscovery(Discovery& discovery) {
    auto addresses = GetDiscoveryAddresses();
    FPP::PrepareDiscovery(discovery, addresses);
    ArtNetOutput::PrepareDiscovery(discovery);
    TwinklyOutput::PrepareDiscovery(discovery);
    Pixlite16::PrepareDiscovery(discovery);
    DDPOutput::PrepareDiscovery(discovery);
}

// Timer for cleaning up Discovery objects after outstanding curl requests complete
namespace {
class DiscoveryCleanupTimer : public wxTimer {
public:
    std::list<Discovery*> discs;
    void Notify() override {
        if (!CurlManager::INSTANCE.processCurls()) {
            for (auto d : discs) delete d;
            discs.clear();
            Stop();
        }
    }
};
static DiscoveryCleanupTimer s_cleanupTimer;
}

std::list<FPP*> xLightsFrame::DiscoverFPPInstances(DiscoveryDelegate* delegate) {
    std::list<FPP*> instances;
    std::list<std::string> forcedAddresses;
    auto addresses = GetDiscoveryAddresses(&forcedAddresses);

    Discovery* discovery = new Discovery(&_outputManager, delegate);
    FPP::PrepareDiscovery(*discovery, addresses);
    discovery->Discover();
    FPP::MapToFPPInstances(*discovery, instances, &_outputManager);
    for (auto* fpp : instances) {
        fpp->_authDelegate = delegate;
    }
    instances.sort(sortByIP);

    // Update forced IPs list: remove any that were not discovered
    std::string newForce;
    for (const auto& a : forcedAddresses) {
        for (const auto& fpp : instances) {
            if (case_insensitive_match(a, fpp->hostName) || case_insensitive_match(a, fpp->ipAddress)) {
                if (!newForce.empty()) {
                    newForce.append(",");
                }
                newForce.append(a);
            }
        }
    }
    wxConfigBase* config = wxConfigBase::Get();
    wxString currentForce;
    config->Read("FPPConnectForcedIPs", &currentForce, "");
    if (newForce != currentForce.ToStdString()) {
        config->Write("FPPConnectForcedIPs", wxString(newForce));
        config->Flush();
    }

    // Clean up discovery - use timer for outstanding curl requests
    discovery->Close(false);
    if (CurlManager::INSTANCE.processCurls()) {
        s_cleanupTimer.discs.push_back(discovery);
        s_cleanupTimer.Start(500);
    } else {
        delete discovery;
    }

    spdlog::info("FPP Discovery Complete. Found {} instances.", instances.size());
    return instances;
}
