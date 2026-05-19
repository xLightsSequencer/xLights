
/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#include "ControllerCaps.h"
#include "UtilFunctions.h"

#include "utils/FileUtils.h"
#include "utils/ExternalHooks.h"
#include "../outputs/Controller.h"

#include <log.h>

#include <cassert>
#include <filesystem>

#pragma region Static Functions
std::map<std::string, std::map<std::string, std::list<ControllerCaps*>>> ControllerCaps::__controllers;
// Owns the loaded .xcontroller xml_documents — ControllerCaps instances hold
// xml_node references into these, so the docs must outlive every cap.
std::vector<pugi::xml_document> ControllerCaps::__sourceDocs;

inline ControllerCaps *FindVariant(std::list<ControllerCaps*> &variants, const std::string &var) {

    if (variants.size() == 1 && var == "")
    {
        return variants.front();
    }

    for (auto it : variants) {
        if (it->GetVariantName() == var) {
            return it;
        }
    }
    return nullptr;
}

static void merge(std::map<std::string, pugi::xml_node> &abstracts, const std::string &base, pugi::xml_node t) {
    pugi::xml_node baseNode = abstracts[base];
    if (baseNode) {
        for (pugi::xml_node nn = baseNode.first_child(); nn; nn = nn.next_sibling()) {
            if (!DoesXmlNodeExist(t, nn.name())) {
                t.append_copy(nn);
            }
        }
        std::string newBase = baseNode.attribute("Base").as_string("");
        if (newBase != "") {
            merge(abstracts, newBase, t);
        }
    }
}

void ControllerCaps::LoadControllers() {

    if (__controllers.size() != 0) return;

    std::string d = FileUtils::GetResourcesDir() + "/controllers";

    // in debug look in the master folder
    std::error_code ec;
    if (!std::filesystem::exists(d, ec)) {
#ifdef _DEBUG
#ifdef _WIN32
        d = FileUtils::GetResourcesDir() + "/../../../controllers";
#endif
#endif
#ifdef LINUX
        d = FileUtils::GetResourcesDir() + "/../controllers";
#endif
    }

    if (std::filesystem::exists(d, ec)) {
        std::vector<std::string> files;
        for (const auto& entry : std::filesystem::directory_iterator(d, ec)) {
            if (entry.is_regular_file() && entry.path().extension() == ".xcontroller") {
                files.push_back(entry.path().string());
            }
        }
        __sourceDocs.clear();
        __sourceDocs.resize(files.size());
        int count = 0;
        for (const auto& filename : files) {
            if (FileExists(filename)) {
                pugi::xml_parse_result result = __sourceDocs[count].load_file(filename.c_str());
                if (!result) {
                    assert(false);
                    spdlog::error("Problem loading " + filename);
                }
                count++;
            }
        }
        std::map<std::string, pugi::xml_node> abstracts;
        for (auto &doc : __sourceDocs) {
            pugi::xml_node root = doc.document_element();
            if (root) {
                for (pugi::xml_node n = root; n; n = n.next_sibling()) {
                    if (std::string_view(n.name()) == "Vendor") {
                        std::string vendor = n.attribute("Name").as_string("");
                        for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                            if (std::string_view(nn.name()) == "AbstractVariant") {
                                std::string var = nn.attribute("Name").as_string("");
                                abstracts[vendor + ":" + var] = nn;
                            }
                        }
                    }
                }
            }
        }
        for (auto &doc : __sourceDocs) {
            pugi::xml_node root = doc.document_element();
            if (root) {
                for (pugi::xml_node n = root; n; n = n.next_sibling()) {
                    if (std::string_view(n.name()) == "Vendor") {
                        std::string vendor = n.attribute("Name").as_string("");

                        if (__controllers.find(vendor) == end(__controllers)) {
                            __controllers[vendor] = std::map<std::string, std::list<ControllerCaps*>>();
                        }

                        auto& v = __controllers[vendor];

                        for (pugi::xml_node nn = n.first_child(); nn; nn = nn.next_sibling()) {
                            if (std::string_view(nn.name()) == "Controller") {
                                std::string controller = nn.attribute("Name").as_string("");
                                if (v.find(controller) == v.end()) {
                                    v[controller] = std::list<ControllerCaps*>();
                                }

                                auto& c = v[controller];
                                for (pugi::xml_node nnn = nn.first_child(); nnn; nnn = nnn.next_sibling()) {
                                    if (std::string_view(nnn.name()) == "Variant") {
                                        if (!nnn.attribute("Base").empty()) {
                                            std::string base = nnn.attribute("Base").as_string("");
                                            merge(abstracts, base, nnn);
                                        }
                                        c.push_back(new ControllerCaps(vendor, controller, nnn));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    } else {
        spdlog::error("Controllers folder not found " + d);
    }
}

void ControllerCaps::UnloadControllers() {

    // delete all the ControllerCaps
    for (const auto& it : __controllers) {
        for (const auto& it2 : it.second) {
            for (auto it3 : it2.second) {
                delete it3;
            }
        }
    }
    __controllers.clear();
    __sourceDocs.clear();
}

std::list<std::string> ControllerCaps::GetVendors(const std::string& type) {

    LoadControllers();
    std::list<std::string> vendors;
    vendors.push_back("");
    for (const auto& it : __controllers) {
        bool done = false;
        for (const auto& it2 : it.second) {
            for (const auto& it3 : it2.second) {
                if (type == CONTROLLER_ETHERNET && it3->SupportsEthernetInputProtols()) {
                    vendors.push_back(it.first);
                    done = true;
                    break;
                }
                else if (type == CONTROLLER_SERIAL && it3->SupportsSerialInputProtols()) {
                    vendors.push_back(it.first);
                    done = true;
                    break;
                }
            }
            if (done) break;
        }
    }
    return vendors;
}

std::list<std::string> ControllerCaps::GetModels(const std::string& type, const std::string& vendor) {

    LoadControllers();
    std::list<std::string> models;
    models.push_back("");

    auto v = __controllers.find(vendor);
    if (v != __controllers.end()) {
        for (const auto& it : v->second) {
            for (const auto& it3 : it.second) {
                if (type == CONTROLLER_ETHERNET && it3->SupportsEthernetInputProtols()) {
                    models.push_back(it.first);
                    break;
                } else if (type == CONTROLLER_SERIAL && it3->SupportsSerialInputProtols()) {
                    models.push_back(it.first);
                    break;
                }
                else if (type == CONTROLLER_ETHERNET && it3->IsPlayerOnly()) {
                    models.push_back(it.first);
                    break;
                }
            }
        }
    }
    return models;
}

std::list<std::string> ControllerCaps::GetVariants(const std::string& type, const std::string& vendor, const std::string& model) {

    LoadControllers();
    std::list<std::string> versions;

    auto v = __controllers.find(vendor);
    if (v != __controllers.end()) {
        auto m = v->second.find(model);
        if (m != v->second.end()) {
            for (const auto& it : m->second) {
                if (it->GetVariantName() != "") {
                    if (type == CONTROLLER_ETHERNET && it->SupportsEthernetInputProtols()) {
                        versions.push_back(it->GetVariantName());
                    }
                    else if (type == CONTROLLER_SERIAL && it->SupportsSerialInputProtols()) {
                        versions.push_back(it->GetVariantName());
                    }
                }
            }
        }
    }
    if (versions.empty()) {
        versions.push_back("");
    }
    return versions;
}

ControllerCaps* ControllerCaps::GetControllerConfig(const Controller* const controller) {

    if (controller == nullptr) return nullptr;

    return GetControllerConfig(controller->GetVendor(), controller->GetModel(), controller->GetVariant());
}

ControllerCaps* ControllerCaps::GetControllerConfig(const std::string& vendor, const std::string& model, const std::string& variant) {
    LoadControllers();
    //std::list<std::string> versions;

    auto v = __controllers.find(vendor);
    if (v != __controllers.end()) {
        auto m = v->second.find(model);
        if (m != v->second.end()) {
            auto f = FindVariant(m->second, variant);
            if (f) return f;
        }
    }
    return nullptr;
}
ControllerCaps* ControllerCaps::GetControllerConfigByID(const std::string& ID) {
    if (ID == "") {
        return nullptr;
    }
    LoadControllers();
    for (auto &v : __controllers) {
        for (auto &m : v.second) {
            for (auto &vr : m.second) {
                if (ID == vr->GetID()) {
                    return vr;
                }
            }
        }
    }
    return nullptr;
}
ControllerCaps* ControllerCaps::GetControllerConfigByVendor(const std::string& vendor) {
    LoadControllers();
    auto v = __controllers.find(vendor);
    if (v != __controllers.end()) {
            return v->second.begin()->second.front();
    }
    return nullptr;
}
ControllerCaps* ControllerCaps::GetControllerConfigByModel( const std::string& model, const std::string& variant)
{
    LoadControllers();
    // look for controller in other "vendors" if branding changes
    for (auto [name, cap] : __controllers) {
        auto con = cap.find(model);
        if (con != cap.end()) {
            auto f = FindVariant(con->second, variant);
            if (f)
                return f;
        }
    }
    return nullptr;
}

ControllerCaps* ControllerCaps::GetControllerConfigByAlternateName(const std::string& vendor, const std::string& model, const std::string& variant) {
    LoadControllers();
    // look for controller if name changed

    auto v = __controllers.find(vendor);
    if (v != __controllers.end()) {
        for (auto const &[_, cap] : v->second) {
            for (auto const& vr : cap) {
                auto const& names = vr->GetAlternativeNames();
                if (std::find(names.begin(), names.end(), model) != names.end() && vr->GetVariantName() == variant) {
                    return vr;
                }
            }
        }
        // did not find new model/variant, see if the model itself has become a variant
        if (!model.empty()) {
            for (auto const &[_, cap] : v->second) {
                for (auto const& vr : cap) {
                    if (vr->GetVariantName() == model) {
                        return vr;
                    }
                }
            }
        }
    }
    return nullptr;
}
#pragma endregion

#pragma region Getters and Setters

bool ControllerCaps::SupportsUpload() const {

    return DoesXmlNodeExist(_config, "SupportsUpload") ||
           DoesXmlNodeExist(_config, "SupportsInputOnlyUpload");
}

bool ControllerCaps::SupportsFullxLightsControl() const
{
    return DoesXmlNodeExist(_config, "SupportsFullxLightsControl");
}

bool ControllerCaps::SupportsInputOnlyUpload() const {

    return DoesXmlNodeExist(_config, "SupportsInputOnlyUpload");
}

bool ControllerCaps::NeedsDDPInputUpload() const
{
    return DoesXmlNodeExist(_config, "NeedsDDPInputUpload");
}

bool ControllerCaps::SupportsLEDPanelMatrix() const {

    return DoesXmlNodeExist(_config, "SupportsLEDPanelMatrix");
}

bool ControllerCaps::SupportsPWM() const {

    return DoesXmlNodeExist(_config, "SupportsPWM");
}

bool ControllerCaps::SupportsVirtualMatrix() const {

    return DoesXmlNodeExist(_config, "SupportsVirtualMatrix");
}

bool ControllerCaps::SupportsVirtualStrings() const {

    return DoesXmlNodeExist(_config, "SupportsVirtualStrings");
}

bool ControllerCaps::SupportsSmartRemotes() const {

    return DoesXmlNodeExist(_config, "SupportsSmartRemotes");
}

bool ControllerCaps::SupportsRemotes() const
{
    return DoesXmlNodeExist(_config, "SupportsRemotes");
}

bool ControllerCaps::SupportsAutoLayout() const {
    return DoesXmlNodeExist(_config, "SupportsAutoLayout");
}

bool ControllerCaps::SupportsAutoUpload() const {
    return DoesXmlNodeExist(_config, "SupportsAutoUpload");
}

bool ControllerCaps::DDPStartsAtOne() const {
    return DoesXmlNodeExist(_config, "DDPStartsAtOne");
}

bool ControllerCaps::SupportsUniversePerString() const
{
    return DoesXmlNodeExist(_config, "SupportsUniversePerString");
}

bool ControllerCaps::DMXAfterPixels() const
{
    return DoesXmlNodeExist(_config, "DMXAfterPixels");
}

bool ControllerCaps::OpenSourceFirmware() const
{
    return DoesXmlNodeExist(_config, "OpenSourceFirmware");
}

bool ControllerCaps::SupportsMultipleSimultaneousOutputProtocols() const {

    return DoesXmlNodeExist(_config, "SupportsMultipleSimultaneousOutputProtocols");
}

bool ControllerCaps::SupportsMultipleSimultaneousInputProtocols() const {

    return DoesXmlNodeExist(_config, "SupportsMultipleSimultaneousInputProtocols");
}

bool ControllerCaps::MergeConsecutiveVirtualStrings() const {

    return DoesXmlNodeExist(_config, "MergeConsecutiveVirtualStrings");
}

bool ControllerCaps::AllInputUniversesMustBeSameSize() const {

    return DoesXmlNodeExist(_config, "AllInputUniversesMustBeSameSize");
}

bool ControllerCaps::AllInputUniversesMustBe510() const
{
    return DoesXmlNodeExist(_config, "AllInputUniversesMustBe510");
}

bool ControllerCaps::UniversesMustBeInNumericalOrder() const {

    return DoesXmlNodeExist(_config, "UniversesMustBeInNumericalOrder");
}

bool ControllerCaps::UniversesMustBeSequential() const {

    return DoesXmlNodeExist(_config, "UniversesMustBeSequential");
}

bool ControllerCaps::NoWebUI() const
{
    return DoesXmlNodeExist(_config, "NoWebUI");
}

bool ControllerCaps::SupportsPixelPortBrightness() const {

    return SupportsPixelPortCommonSettings() || DoesXmlNodeExist(_config, "SupportsPixelPortBrightness");
}

bool ControllerCaps::SupportsPixelPortGamma() const {

    return SupportsPixelPortCommonSettings() || DoesXmlNodeExist(_config, "SupportsPixelPortGamma");
}

bool ControllerCaps::SupportsDefaultGamma() const
{
    return DoesXmlNodeExist(_config, "SupportsDefaultGamma");
}

bool ControllerCaps::SupportsDefaultBrightness() const
{
    return DoesXmlNodeExist(_config, "SupportsDefaultBrightness");
}

bool ControllerCaps::SupportsPixelPortColourOrder() const {

    return SupportsPixelPortCommonSettings() || DoesXmlNodeExist(_config, "SupportsPixelPortColourOrder");
}

bool ControllerCaps::SupportsEthernetInputProtols() const
{
    for (const auto& it : GetInputProtocols()) {
        if (it == "e131" || it == "artnet" || it == "kinet" || it == "zcpp" || it == "ddp" || it == "opc" || it == "xxx ethernet" || it == "twinkly")
            return true;
    }
    return false;
}

bool ControllerCaps::IsPlayerOnly() const
{
    return DoesXmlNodeExist(_config, "PlayerOnly");
}

bool ControllerCaps::SupportsSerialInputProtols() const
{
    for (const auto& it : GetInputProtocols()) {
        if (it == "dmx" || it == "lor" || it == "renard" ||
            it == "opendmx" || it == "pixelnet" || it == "open pixelnet" ||
            it == "dlight" || it == "lor optimised" || it == "xxx serial" || it == "ddp-input") return true;
    }
    return false;
}

bool ControllerCaps::NeedsFullUniverseForDMX() const
{
    return DoesXmlNodeExist(_config, "NeedsFullUniverseForSerial");
}

bool ControllerCaps::SupportsPixelPortNullPixels() const {

    return SupportsPixelPortCommonSettings() || DoesXmlNodeExist(_config, "SupportsPixelPortNullPixels");
}

bool ControllerCaps::SupportsPixelPortEndNullPixels() const
{

    return DoesXmlNodeExist(_config, "SupportsPixelPortEndNullPixels");
}

bool ControllerCaps::SupportsPixelPortDirection() const {

    return SupportsPixelPortCommonSettings() || DoesXmlNodeExist(_config, "SupportsPixelPortDirection");
}

bool ControllerCaps::SupportsPixelPortGrouping() const {

    return SupportsPixelPortCommonSettings() || DoesXmlNodeExist(_config, "SupportsPixelPortGrouping");
}

bool ControllerCaps::SupportsPixelZigZag() const
{
    return DoesXmlNodeExist(_config, "SupportsPixelZigZag");
}

bool ControllerCaps::SupportsTs() const
{
    return DoesXmlNodeExist(_config, "SupportsTs");
}

bool ControllerCaps::SupportsPixelPortCommonSettings() const {

    return DoesXmlNodeExist(_config, "SupportsPixelPortCommonSettings");
}

int ControllerCaps::GetMaxInputE131Universes() const {

    return (int)strtol(GetXmlNodeContent(_config, "MaxInputUniverses").c_str(), nullptr, 10);
}

int ControllerCaps::GetSmartRemoteCount() const
{

    return (int)strtol(GetXmlNodeContent(_config, "SupportsSmartRemotes").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxPixelPort() const {

    return (int)strtol(GetXmlNodeContent(_config, "MaxPixelPort").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxSerialPort() const {
    return (int)strtol(GetXmlNodeContent(_config, "MaxSerialPort").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxPWMPort() const {
    return (int)strtol(GetXmlNodeContent(_config, "MaxPWMPort").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxVirtualMatrixPort() const {
    //for now, use 1 if supported.  Technially FPP supports unlimitted Virtual Matrices,
    //on one port, but has two HDMI ports on the Pi4 so some of this may need to
    //be adjusted at some point
    return SupportsVirtualMatrix() ? (int)strtol(GetXmlNodeContent(_config, "MaxVirtualMatrixPorts", "1").c_str(), nullptr, 10): 0;
}
int ControllerCaps::GetMaxLEDPanelMatrixPort() const {
    //FPP 9 supports up to 5 PanelMatrices defined
    return SupportsLEDPanelMatrix() ? 5 : 0;
}

int ControllerCaps::GetMaxPixelPortChannels() const {

    return (int)strtol(GetXmlNodeContent(_config, "MaxPixelPortChannels").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxSerialPortChannels() const {

    return (int)strtol(GetXmlNodeContent(_config, "MaxSerialPortChannels").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxInputUniverseChannels() const {

    return (int)strtol(GetXmlNodeContent(_config, "MaxInputUniverseChannels", "512").c_str(), nullptr, 10);
}

int ControllerCaps::GetMinInputUniverseChannels() const
{
    return (int)strtol(GetXmlNodeContent(_config, "MinInputUniverseChannels", "1").c_str(), nullptr, 10);
}

int ControllerCaps::GetNumberOfBanks() const
{
    return (int)strtol(GetXmlNodeContent(_config, "NumberOfBanks", "1").c_str(), nullptr, 10);
}

int ControllerCaps::GetBankSize() const
{
    return (int)strtol(GetXmlNodeContent(_config, "BankSize", "16").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxStartNullPixels() const
{
    return (int)strtol(GetXmlNodeContent(_config, "MaxStartNulls", "-1").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxEndNullPixels() const
{
    return (int)strtol(GetXmlNodeContent(_config, "MaxEndNulls", "-1").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxGroupPixels() const
{
    return (int)strtol(GetXmlNodeContent(_config, "MaxGroup", "-1").c_str(), nullptr, 10);
}

int ControllerCaps::GetMaxZigZagPixels() const
{
    return (int)strtol(GetXmlNodeContent(_config, "MaxZigZag", "-1").c_str(), nullptr, 10);
}

// Maximum pixels on a local port or dumb remotes and achieve 40 FPS
int ControllerCaps::GetMaxPixelsAt40FPS() const
{
    return (int)strtol(GetXmlNodeContent(_config, "FPS40Pixels", "-1").c_str(), nullptr, 10);
}

// Maximum pixels on a port using smart remotes and achieve 40 FPS
int ControllerCaps::GetMaxPixelsAt40FPS_SR() const
{
    int res = (int)strtol(GetXmlNodeContent(_config, "FPS40Pixels_SR", "-1").c_str(), nullptr, 10);

    if (res == -1)
        return GetMaxPixelsAt40FPS();
    return res;
}

int ControllerCaps::GetMinGroupPixels() const
{
    return (int)strtol(GetXmlNodeContent(_config, "MinGroup", "-1").c_str(), nullptr, 10);
}

bool ControllerCaps::IsValidPixelProtocol(const std::string& protocol) const {

    auto pp = GetPixelProtocols();
    auto p = Lower(protocol);

    for (const auto& it : pp) {
        if (Lower(it) == p) return true;
    }
    return false;
}

bool ControllerCaps::IsValidSerialProtocol(const std::string& protocol) const {

    auto pp = GetSerialProtocols();
    auto p = Lower(protocol);

    for (const auto& it : pp) {
        if (Lower(it) == p) return true;
    }
    return false;
}

bool ControllerCaps::IsValidInputProtocol(const std::string& protocol) const {

    auto pp = GetInputProtocols();
    auto p = Lower(protocol);

    for (const auto& it : pp) {
        if (Lower(it) == p) return true;
    }
    return false;
}

bool ControllerCaps::IsSerialController() const
{
    return (!IsValidInputProtocol("e131") && !IsValidInputProtocol("artnet") && !IsValidInputProtocol("ddp") && !IsValidInputProtocol("zcpp") && !IsValidInputProtocol("xxx ethernet") && !IsValidInputProtocol("opc") && !IsValidInputProtocol("kinet"));
}

std::vector<std::string> ControllerCaps::GetInputProtocols() const {

    return GetXmlNodeListContent(_config, "InputProtocols", "Protocol");
}

std::vector<std::string> ControllerCaps::GetPixelProtocols() const {

    return GetXmlNodeListContent(_config, "PixelProtocols", "Protocol");
}

std::vector<std::string> ControllerCaps::GetSerialProtocols() const {

    return GetXmlNodeListContent(_config, "SerialProtocols", "Protocol");
}

std::vector<std::string> ControllerCaps::GetAllProtocols() const
{
    std::vector<std::string> res = GetPixelProtocols();
    auto serial = GetSerialProtocols();
    res.insert(end(res), begin(serial), end(serial));
    if (SupportsVirtualMatrix()) {
        res.push_back("Virtual Matrix");
    }
    if (SupportsLEDPanelMatrix()) {
        res.push_back("LED Panel Matrix");
    }
    if (SupportsPWM()) {
        res.push_back("PWM");
    }
    return res;
}

std::string ControllerCaps::GetVariantName() const {
    return _config.attribute("Name").as_string();
}
std::string ControllerCaps::GetID() const {
    return _config.attribute("ID").as_string();
}

std::string ControllerCaps::GetPreferredInputProtocol() const
{
    return GetXmlNodeContent(_config, "PreferredInputProtocol", "");
}

std::string ControllerCaps::GetPreferredState() const {
    return GetXmlNodeContent(_config, "PreferredState", "");
}

std::string ControllerCaps::GetConfigDriver() const
{
    return GetXmlNodeContent(_config, "ConfigDriver", "");
}

bool ControllerCaps::DisableMonitoring() const {
    return DoesXmlNodeExist(_config, "DisableMonitoring");
}

std::vector<std::string> ControllerCaps::GetSmartRemoteTypes() const {
    if (!SupportsSmartRemotes()) {
        return { "" };
    }
    auto types = GetXmlNodeListContent(_config, "SmartRemoteTypes", "Type");
    if (types.empty()) {
        types.emplace_back("");
    }
    return types;
}

bool ControllerCaps::AllSmartRemoteTypesPerPortMustBeSame() const {

    return DoesXmlNodeExist(_config, "AllSmartRemoteTypesPerPortMustBeSame");
}

std::string ControllerCaps::GetCustomPropertyByPath(const std::string name, const std::string& def) const {

    return GetXmlNodeContent(_config, name, def);
}

std::vector<std::string> ControllerCaps::GetAlternativeNames() const {
    return GetXmlNodeListContent(_config, "AltNames", "AltName");
}

void ControllerCaps::Dump() const
{
    
    spdlog::debug("Controller Capabilities " + _vendor + ":" + _model + ":" + GetVariantName());

    if (OpenSourceFirmware()) spdlog::debug("   Open source firmware.");
    if (SupportsUpload()) spdlog::debug("   Supports upload.");
    if (SupportsInputOnlyUpload()) spdlog::debug("   Supports input only upload.");
    if (SupportsLEDPanelMatrix()) spdlog::debug("   Supports LED panel matrices.");
    if (SupportsVirtualStrings()) spdlog::debug("   Supports virtual strings.");
    if (SupportsSmartRemotes()) {
        spdlog::debug("   Supports smart remotes.");
        spdlog::debug("   Supported smart remotes types:");
        for (auto const& it : GetSmartRemoteTypes()) {
            spdlog::debug("      " + it);
        }
    }
    if (SupportsMultipleSimultaneousOutputProtocols()) spdlog::debug("   Supports multiple simultaneous output protocols.");
    if (AllInputUniversesMustBeSameSize()) spdlog::debug("   All input universes must be the same size.");
    if (UniversesMustBeInNumericalOrder()) spdlog::debug("   All input universes must be in numerical order.");
    spdlog::debug("   Inputs: maximum of {} universes.", GetMaxInputE131Universes());
    spdlog::debug("   Input protocols supported:");
    for (const auto& it : GetInputProtocols()) {
        spdlog::debug("      " + it);
    }
    spdlog::debug("   Pixel ports: {} ports with a maximum of {} channels per port.", GetMaxPixelPort(), GetMaxPixelPortChannels());
    spdlog::debug("   Pixel protocols supported:");
    for (const auto& it : GetPixelProtocols()) {
        spdlog::debug("      " + it);
    }
    spdlog::debug("   Serial ports: {} ports with a maximum of {} channels per port.", GetMaxSerialPort(), GetMaxSerialPortChannels());
    spdlog::debug("   Serial protocols supported:");
    for (const auto& it : GetSerialProtocols()) {
        spdlog::debug("      " + it);
    }
}


std::vector<ControllerCaps::ExtraPropertyDef> ControllerCaps::GetExtraPropertyDefs() const {
    std::vector<ExtraPropertyDef> result;
    for (pugi::xml_node n = _config.first_child(); n; n = n.next_sibling()) {
        if (std::string_view(n.name()) == "ExtraProperties") {
            for (pugi::xml_node pnode = n.first_child(); pnode; pnode = pnode.next_sibling()) {
                ExtraPropertyDef def;
                def.name = pnode.attribute("name").as_string("");
                def.label = pnode.attribute("label").as_string("");
                def.defaultValue = GetXmlNodeContent(pnode, "Default");
                def.type = GetXmlNodeContent(pnode, "Type", "String");
                if (def.type == "Enum") {
                    def.values = GetXmlNodeListContent(pnode, "Values", "Value");
                }
                result.push_back(std::move(def));
            }
        }
    }
    return result;
}

#pragma endregion
