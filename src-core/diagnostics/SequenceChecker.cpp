#ifdef _MSC_VER
#pragma execution_character_set("utf-8")
#endif

#include "diagnostics/SequenceChecker.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <map>
#include <set>
#include <sstream>
#include <vector>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "effects/EffectManager.h"
#include "effects/FacesEffect.h"
#include "effects/RenderableEffect.h"
#include "effects/StateEffect.h"
#include "models/DMX/DmxMovingHeadComm.h"
#include "models/DisplayAsType.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "models/TwoPointScreenLocation.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "render/DataLayer.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "utils/ExternalHooks.h"
#include "utils/FileUtils.h"
#include "utils/UtilClasses.h"
#include "utils/UtilFunctions.h"
#include "utils/ip_utils.h"
#include "utils/string_utils.h"

namespace {

void LogMsg(const std::string& msg) {
    spdlog::debug("CheckSequence: " + msg);
}

// Helper used only inside SequenceChecker file walks to sort models by start channel.
bool CompareModelStartChannel(const Model* first, const Model* second) {
    int firstStart = first->GetNumberFromChannelString(first->ModelStartChannel);
    int secondStart = second->GetNumberFromChannelString(second->ModelStartChannel);
    return firstStart < secondStart;
}

// Split a string by any of the provided delimiter characters. Mirrors the
// xLights desktop helper used in the show-folder repeated-name path check.
std::vector<std::string> SplitByAny(const std::string& in, const std::string& delims) {
    std::vector<std::string> out;
    std::string current;
    for (char c : in) {
        if (delims.find(c) != std::string::npos) {
            if (!current.empty()) {
                out.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    if (!current.empty()) {
        out.push_back(current);
    }
    return out;
}

} // namespace

SequenceChecker::SequenceChecker(SequenceElements& elements,
                                 ModelManager& models,
                                 OutputManager& outputs,
                                 SequenceFile* sequenceFile,
                                 std::string showFolder,
                                 SequenceCheckerCallbacks* callbacks) :
    _elements(elements),
    _models(models),
    _outputs(outputs),
    _sequenceFile(sequenceFile),
    _showFolder(std::move(showFolder)),
    _callbacks(callbacks) {
}

void SequenceChecker::RecordIssue(CheckSequenceReport& report,
                                  const std::string& sectionId,
                                  const CheckSequenceReport::ReportIssue& issue) {
    LogMsg(issue.message);
    report.AddIssue(sectionId, issue);
    if (issue.type == CheckSequenceReport::ReportIssue::CRITICAL) {
        ++_errors;
    } else if (issue.type == CheckSequenceReport::ReportIssue::WARNING) {
        ++_warnings;
    }
}

int SequenceChecker::RunFullCheck(CheckSequenceReport& report) {
    if (_callbacks) {
        _callbacks->OnProgress(0, "Controllers");
    }
    RunControllerChecks(report);
    if (_callbacks) {
        _callbacks->OnProgress(25, "Models");
    }
    RunModelChecks(report);
    if (_callbacks) {
        _callbacks->OnProgress(50, "Sequence");
    }
    RunSequenceChecks(report);
    if (_callbacks) {
        _callbacks->OnProgress(75, "File references");
    }
    RunFileReferenceChecks(report);
    if (_callbacks) {
        _callbacks->OnProgress(100, "Done");
    }
    return _errors;
}

bool SequenceChecker::CheckStart(CheckSequenceReport& report,
                                 const std::string& startModel,
                                 std::list<std::string>& seen,
                                 std::string& nextModel) {
    Model* m = _models.GetModel(nextModel);
    if (m == nullptr) {
        return true; // already reported elsewhere
    }
    std::string start = m->ModelStartChannel;

    if (!start.empty() && (start[0] == '>' || start[0] == '@')) {
        seen.push_back(nextModel);
        size_t colon = start.find(':', 1);
        std::string reference = start.substr(1, colon - 1);

        if (std::find(seen.begin(), seen.end(), reference) != seen.end()) {
            std::string msg = fmt::format("    ERR: Model '{}' start channel results in a reference loop.\n", startModel);
            for (const auto& it : seen) {
                msg += fmt::format("       '{}' ->\n", it);
            }
            msg += fmt::format("       '{}'", reference);
            RecordIssue(report, "models",
                        CheckSequenceReport::ReportIssue::ForModel(
                            CheckSequenceReport::ReportIssue::CRITICAL,
                            msg, "channels", startModel));
            return false;
        }
        return CheckStart(report, startModel, seen, reference);
    }
    return true;
}

int SequenceChecker::RunControllerChecks(CheckSequenceReport& report) {
    int startErrors = _errors;

    // Inactive controller check
    for (const auto& c : _outputs.GetControllers()) {
        if (!c->IsEnabled() && c->CanSendData() &&
            c->GetModel() != "FPP Player Only" &&
            c->GetModel() != "FPP Video Playing Remote Only") {
            std::string msg = fmt::format("    WARN: Inactive controller {} {}:{}.",
                                          c->GetName(), c->GetColumn1Label(), c->GetColumn2Label());
            RecordIssue(report, "controllers",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "inactive"));
        }
    }

    // Multiple outputs sending to the same destination
    {
        std::list<std::string> used;
        for (const auto& o : _outputs.GetAllOutputs()) {
            if (o->IsIpOutput() && (o->GetType() == OUTPUT_E131 || o->GetType() == OUTPUT_ARTNET || o->GetType() == OUTPUT_KINET)) {
                std::string usedval = o->GetIP() + "|" + o->GetUniverseString();
                if (std::find(used.begin(), used.end(), usedval) != used.end()) {
                    int32_t sc;
                    auto c = _outputs.GetController(o->GetStartChannel(), sc);
                    std::string cname = c ? c->GetName() : std::string();
                    std::string msg = fmt::format("    ERR: Multiple outputs being sent to the same controller '{}' ({}) and universe {}.",
                                                  cname, o->GetIP(), o->GetUniverseString());
                    RecordIssue(report, "controllers",
                                CheckSequenceReport::ReportIssue(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "duplicates"));
                } else {
                    used.push_back(usedval);
                }
            } else if (o->IsSerialOutput()) {
                if (o->GetCommPort() != "NotConnected") {
                    if (std::find(used.begin(), used.end(), o->GetCommPort()) != used.end()) {
                        std::string msg = fmt::format("    ERR: Multiple outputs being sent to the same comm port {} '{}'.",
                                                      o->GetType(), o->GetCommPort());
                        RecordIssue(report, "controllers",
                                    CheckSequenceReport::ReportIssue(
                                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "duplicates"));
                    } else {
                        used.push_back(o->GetCommPort());
                    }
                }
            }
        }
    }

    // Build list of unique managed/ZCPP/DDP controllers
    std::list<Controller*> uniqueControllers;
    for (const auto& it : _outputs.GetControllers()) {
        auto eth = dynamic_cast<ControllerEthernet*>(it);
        if (eth != nullptr && (eth->GetProtocol() == OUTPUT_ZCPP || eth->GetProtocol() == OUTPUT_DDP || eth->IsManaged())) {
            uniqueControllers.push_back(eth);
        }
    }

    if (!uniqueControllers.empty()) {
        // duplicate IP across controllers
        for (const auto& it : uniqueControllers) {
            for (const auto& itc : _outputs.GetControllers()) {
                auto eth = dynamic_cast<ControllerEthernet*>(itc);
                if (eth != nullptr) {
                    if (eth != it && it->GetIP() != "MULTICAST" &&
                        (it->GetIP() == eth->GetIP() || it->GetIP() == eth->GetResolvedIP(false))) {
                        std::string msg = fmt::format("    ERR: {} IP Address '{}' for controller '{}' used on another controller '{}'. This is not allowed.",
                                                      it->GetProtocol(), it->GetIP(), it->GetName(), eth->GetName());
                        RecordIssue(report, "controllers",
                                    CheckSequenceReport::ReportIssue(
                                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "overlap"));
                        break;
                    }
                }
            }
        }

        // models per controller per port — invalid controller connection / model chain duplicates
        std::map<std::string, std::map<std::string, std::list<Model*>>> modelsByPortByController;
        for (const auto& it : _models) {
            if (it.second->GetControllerName() != "") {
                auto c = _outputs.GetController(it.second->GetControllerName());
                if (c != nullptr) {
                    auto caps = c->GetControllerCaps();
                    if (!it.second->IsControllerConnectionValid() &&
                        (caps != nullptr && caps->GetMaxPixelPort() != 0 && caps->GetMaxSerialPort() != 0)) {
                        std::string msg = fmt::format("    ERR: Model {} on {} controller '{}:{}' has invalid controller connection '{}'.",
                                                      it.second->GetName(), c->GetProtocol(), c->GetName(),
                                                      c->GetIP(), it.second->GetControllerConnectionString());
                        RecordIssue(report, "controllers",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::CRITICAL,
                                        msg, "invalid", it.second->GetName()));
                    }

                    auto& portMap = modelsByPortByController[c->GetName()];
                    std::string key = fmt::format("{}:{}:{}",
                                                  it.second->IsPixelProtocol() ? "pixel" : "serial",
                                                  it.second->GetControllerPort(),
                                                  it.second->GetSmartRemote());
                    portMap[key].push_back(it.second);
                }
            }
        }

        // chain validation per controller/port
        for (auto& it : modelsByPortByController) {
            auto c = _outputs.GetController(it.first);
            for (auto& itp : it.second) {
                if (Contains(itp.first, "pixel")) {
                    std::string last = "";
                    while (!itp.second.empty()) {
                        bool pushed = false;
                        for (auto itms = begin(itp.second); itms != end(itp.second); ++itms) {
                            if (((*itms)->GetModelChain() == "Beginning" && last == "") ||
                                (*itms)->GetModelChain() == last ||
                                (*itms)->GetModelChain() == ">" + last) {
                                pushed = true;
                                last = (*itms)->GetName();
                                itp.second.erase(itms);
                                break;
                            }
                        }
                        if (!pushed && !itp.second.empty()) {
                            while (!itp.second.empty()) {
                                std::string ip = c ? c->GetIP() : std::string();
                                std::string cname = c ? c->GetName() : std::string();
                                std::string msg = fmt::format("    ERR: Model {} on ZCPP controller '{}:{}' on port '{}' has invalid Model Chain '{}'. It may be a duplicate or point to a non existent model on this controller port or there may be a loop.",
                                                              itp.second.front()->GetName(), ip, cname,
                                                              itp.second.front()->GetControllerConnectionString(),
                                                              itp.second.front()->GetModelChain());
                                RecordIssue(report, "controllers",
                                            CheckSequenceReport::ReportIssue::ForModel(
                                                CheckSequenceReport::ReportIssue::CRITICAL,
                                                msg, "duplicate", itp.second.front()->GetName()));
                                itp.second.pop_front();
                            }
                        }
                    }
                }
            }
        }

        // Vendor-specific UDController validation
        for (const auto& it : _outputs.GetControllers()) {
            std::string controllerId = fmt::format("{}:{}", it->GetName(), it->GetIP());
            std::string infoMsg = fmt::format("Applying controller rules for {}:{}:{}", it->GetName(), it->GetIP(), it->GetDescription());
            RecordIssue(report, "controllers",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::INFO, infoMsg, "vendor:" + controllerId));

            UDController edc(it, &_outputs, &_models, false);
            std::string check;
            auto fcr = ControllerCaps::GetControllerConfig(it->GetVendor(), it->GetModel(), it->GetVariant());
            if (fcr != nullptr) {
                if (!edc.Check(fcr, check)) {
                    std::istringstream stream(check);
                    std::string line;
                    while (std::getline(stream, line)) {
                        if (line.find("ERR:") != std::string::npos) {
                            RecordIssue(report, "controllers",
                                        CheckSequenceReport::ReportIssue(
                                            CheckSequenceReport::ReportIssue::CRITICAL,
                                            line, "vendor:" + controllerId));
                        } else if (line.find("WARN:") != std::string::npos) {
                            RecordIssue(report, "controllers",
                                        CheckSequenceReport::ReportIssue(
                                            CheckSequenceReport::ReportIssue::WARNING,
                                            line, "vendor:" + controllerId));
                        }
                    }
                }
            } else {
                RecordIssue(report, "controllers",
                            CheckSequenceReport::ReportIssue(
                                CheckSequenceReport::ReportIssue::INFO,
                                "Unknown controller vendor - vendor specific checks skipped.",
                                "vendor:" + controllerId));
            }
        }
    }

    // Multiple outputs with the same universe/id number
    if (!(_callbacks && _callbacks->IsCheckOptionDisabled("DupUniv"))) {
        std::map<int, int> useduid;
        for (auto o : _outputs.GetAllOutputs()) {
            if (o->GetType() != OUTPUT_ZCPP) {
                useduid[o->GetUniverse()]++;
            }
        }
        for (auto u : useduid) {
            if (u.second > 1) {
                std::string msg = fmt::format("    WARN: Multiple outputs ({}) with same universe/id number {}. If using #universe:start_channel result may be incorrect.",
                                              u.second, u.first);
                RecordIssue(report, "controllers",
                            CheckSequenceReport::ReportIssue(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "multiple"));
            }
        }
    } else {
        RecordIssue(report, "controllers",
                    CheckSequenceReport::ReportIssue(
                        CheckSequenceReport::ReportIssue::INFO,
                        "Multiple outputs with same universe/id number - CHECK DISABLED",
                        "checkdisabled"));
    }

    // Controller universes out of order
    {
        std::map<std::string, int> lastuniverse;
        for (auto n : _outputs.GetAllOutputs()) {
            if (n->IsIpOutput() && (n->GetType() == OUTPUT_E131 || n->GetType() == OUTPUT_ARTNET)) {
                auto found = lastuniverse.find(n->GetIP());
                if (found == lastuniverse.end()) {
                    lastuniverse[n->GetIP()] = n->GetUniverse();
                } else {
                    if (found->second > n->GetUniverse()) {
                        std::string msg = fmt::format("    WARN: Controller {} Universe {} occurs after universe {}. Some controllers do not like out of order universes.",
                                                      n->GetIP(), n->GetUniverse(), found->second);
                        RecordIssue(report, "controllers",
                                    CheckSequenceReport::ReportIssue(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "order"));
                    } else {
                        found->second = n->GetUniverse();
                    }
                }
            }
        }
    }

    // Invalid controller IP addresses
    for (const auto& c : _outputs.GetControllers()) {
        auto eth = c;
        if (eth->GetIP() != "" && eth->GetIP() != "MULTICAST") {
            if (!ip_utils::IsIPValidOrHostname(eth->GetIP())) {
                std::string msg = fmt::format("    WARN: IP address '{}' on controller '{}' does not look valid.",
                                              eth->GetIP(), eth->GetName());
                RecordIssue(report, "controllers",
                            CheckSequenceReport::ReportIssue(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "ip_validation"));
            } else {
                auto ipElements = SplitByAny(eth->GetIP(), ".");
                if (ipElements.size() > 3) {
                    int ip1 = (int)std::strtol(ipElements[0].c_str(), nullptr, 10);
                    int ip2 = (int)std::strtol(ipElements[1].c_str(), nullptr, 10);
                    int ip3 = (int)std::strtol(ipElements[2].c_str(), nullptr, 10);
                    int ip4 = (int)std::strtol(ipElements[3].c_str(), nullptr, 10);

                    auto reportCritical = [&](const std::string& msg) {
                        RecordIssue(report, "controllers",
                                    CheckSequenceReport::ReportIssue(
                                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "ip_validation"));
                    };

                    if (ip1 == 10) {
                        if (ip2 == 255 && ip3 == 255 && ip4 == 255) {
                            reportCritical(fmt::format("    ERR: IP address '{}' on controller '{}' is a broadcast address.",
                                                       eth->GetIP(), eth->GetName()));
                        }
                    } else if (ip1 == 192 && ip2 == 168) {
                        if (ip3 == 255 && ip4 == 255) {
                            reportCritical(fmt::format("    ERR: IP address '{}' on controller '{}' is a broadcast address.",
                                                       eth->GetIP(), eth->GetName()));
                        }
                    } else if (ip1 == 172 && ip2 >= 16 && ip2 <= 31) {
                        // valid
                    } else if (ip1 == 255 && ip2 == 255 && ip3 == 255 && ip4 == 255) {
                        reportCritical(fmt::format("    ERR: IP address '{}' on controller '{}' is a broadcast address.",
                                                   eth->GetIP(), eth->GetName()));
                    } else if (ip1 == 0) {
                        reportCritical(fmt::format("    ERR: IP address '{}' on controller '{}' not valid.",
                                                   eth->GetIP(), eth->GetName()));
                    } else if (ip1 >= 224 && ip1 <= 239) {
                        reportCritical(fmt::format("    ERR: IP address '{}' on controller '{}' is a multicast address.",
                                                   eth->GetIP(), eth->GetName()));
                    } else {
                        std::string msg = fmt::format("    WARN: IP address '{}' on controller '{}' in internet routable ... are you sure you meant to do this.",
                                                      eth->GetIP(), eth->GetName());
                        RecordIssue(report, "controllers",
                                    CheckSequenceReport::ReportIssue(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "ip_validation"));
                    }
                }
            }
        }
    }

    // Models spanning controllers
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup &&
            it.second->GetDisplayAs() != DisplayAsType::Label) {
            int32_t start = it.second->GetFirstChannel() + 1;
            int32_t end = it.second->GetLastChannel() + 1;

            int32_t sc;
            Controller* ostart = _outputs.GetController(start, sc);
            Controller* oend = _outputs.GetController(end, sc);

            auto eth_ostart = dynamic_cast<ControllerEthernet*>(ostart);
            auto eth_oend = dynamic_cast<ControllerEthernet*>(oend);
            auto ser_ostart = dynamic_cast<ControllerSerial*>(ostart);
            auto ser_oend = dynamic_cast<ControllerSerial*>(oend);

            if (ostart != nullptr && oend == nullptr) {
                std::string msg = fmt::format("    ERR: Model '{}' starts on controller '{}' but ends at channel {} which is not on a controller.",
                                              it.first, ostart->GetName(), end);
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
            } else if (ostart == nullptr || oend == nullptr) {
                std::string msg;
                CheckSequenceReport::ReportIssue::Type type;
                if (!it.second->IsActive()) {
                    msg = fmt::format("    WARN: Model '{}' is not configured for a controller.", it.first);
                    type = CheckSequenceReport::ReportIssue::WARNING;
                } else {
                    msg = fmt::format("    ERR: Model '{}' is not configured for a controller.", it.first);
                    type = CheckSequenceReport::ReportIssue::CRITICAL;
                }
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(type, msg, "unconfigured", it.first));
            } else if (ostart->GetType() != oend->GetType()) {
                std::string msg = fmt::format("    WARN: Model '{}' starts on controller '{}' of type '{}' but ends on a controller '{}' of type '{}'.",
                                              it.first, ostart->GetName(), ostart->GetType(),
                                              oend->GetDescription(), oend->GetType());
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "config", it.first));
            } else if (eth_ostart != nullptr && eth_oend != nullptr &&
                       eth_ostart->GetIP() == "MULTICAST" && eth_oend->GetIP() == "MULTICAST") {
                // ignore
            } else if (eth_ostart != nullptr && eth_oend != nullptr &&
                       eth_ostart->GetIP() + eth_oend->GetIP() != "") {
                if (eth_ostart->GetIP() != eth_oend->GetIP()) {
                    std::string msg = fmt::format("    WARN: Model '{}' starts on controller '{}' with IP '{}' but ends on a controller '{}' with IP '{}'.",
                                                  it.first, ostart->GetName(), eth_ostart->GetIP(),
                                                  oend->GetName(), eth_oend->GetIP());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::WARNING, msg, "config", it.first));
                }
            } else if (ser_ostart != nullptr && ser_oend != nullptr &&
                       ser_ostart->GetPort() + ser_oend->GetPort() != "") {
                if (ser_ostart->GetPort() != ser_oend->GetPort()) {
                    std::string msg = fmt::format("    WARN: Model '{}' starts on controller '{}' with CommPort '{}' but ends on a controller '{}' with CommPort '{}'.",
                                                  it.first, ostart->GetName(), ser_ostart->GetPort(),
                                                  ser_oend->GetName(), ser_oend->GetPort());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::WARNING, msg, "config", it.first));
                }
            }
        }
    }

    return _errors - startErrors;
}

int SequenceChecker::RunModelChecks(CheckSequenceReport& report) {
    int startErrors = _errors;

    // Model channel checks — start channel references
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup &&
            it.second->GetDisplayAs() != DisplayAsType::Label) {
            std::string start = it.second->ModelStartChannel;

            if (!start.empty() && (start[0] == '>' || start[0] == '@')) {
                size_t colon = start.find(':', 1);
                std::string reference = start.substr(1, colon - 1);
                if (reference == it.first) {
                    std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' refers to itself.", it.first, start);
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                } else {
                    Model* m = _models.GetModel(reference);
                    if (m == nullptr) {
                        std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' refers to non existent model '{}'.",
                                                      it.first, start, reference);
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                    }
                }
            } else if (!start.empty() && start[0] == '!') {
                auto comp = SplitByAny(start.substr(1), ":");
                if (!comp.empty() && _outputs.GetController(comp[0]) == nullptr) {
                    std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' refers to non existent controller '{}'.",
                                                  it.first, start, comp[0]);
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                }
            }
            if (it.second->GetLastChannel() == (unsigned int)-1) {
                std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' evaluates to an illegal channel number.",
                                              it.first, start);
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
            }
        }
    }

    // Recursive start channel chain check
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup &&
            it.second->GetDisplayAs() != DisplayAsType::Label) {
            std::string start = it.second->ModelStartChannel;

            if (!start.empty() && (start[0] == '>' || start[0] == '@')) {
                std::list<std::string> seen;
                seen.push_back(it.first);
                size_t colon = start.find(':', 1);
                if (colon != std::string::npos) {
                    std::string reference = start.substr(1, colon - 1);
                    if (reference != it.first) {
                        if (!CheckStart(report, it.first, seen, reference)) {
                            // already counted
                        }
                    }
                } else {
                    std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' invalid.", it.first, start);
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                }
            } else if (!start.empty() && start[0] == '#') {
                size_t colon = start.find(':', 1);
                if (colon != std::string::npos) {
                    size_t colon2 = start.find(':', colon + 1);
                    if (colon2 == std::string::npos) {
                        colon2 = colon;
                        colon = 0;
                    }
                    int universe = (int)std::strtol(start.substr(colon + 1, colon2 - 1).c_str(), nullptr, 10);
                    Output* o = _outputs.GetOutput(universe, "");
                    if (o == nullptr) {
                        std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' refers to undefined universe {}.",
                                                      it.first, start, universe);
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                    }
                } else {
                    std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' invalid.", it.first, start);
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                }
            } else if (!start.empty() && start[0] == '!') {
                // nothing to check
            } else if (start.find(':') != std::string::npos) {
                size_t colon = start.find(':');
                int output = (int)std::strtol(start.substr(0, colon).c_str(), nullptr, 10);
                int cnt = _outputs.GetOutputCount();
                if (output < 1 || output > cnt) {
                    std::string msg = fmt::format("    ERR: Model '{}' start channel '{}' refers to undefined output {}. Only {} outputs are defined.",
                                                  it.first, start, output, cnt);
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "channels", it.first));
                }
            }
        }
    }

    // Overlapping model channels
    for (auto it = std::begin(_models); it != std::end(_models); ++it) {
        if (it->second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            if (StartsWith(it->second->GetModelStartChannel(), "@") &&
                it->second->GetDisplayAs() == DisplayAsType::SingleLine &&
                it->second->GetNumStrings() > 1) {
                spdlog::debug("Skipping Overlap Checking for {} [{}]",
                              it->second->GetFullName(), it->second->GetModelStartChannel());
                continue;
            }

            auto m1start = it->second->GetFirstChannel() + 1;
            auto m1end = it->second->GetLastChannel() + 1;

            for (auto it2 = std::next(it); it2 != std::end(_models); ++it2) {
                if (it2->second->GetDisplayAs() != DisplayAsType::ModelGroup &&
                    it2->second->GetShadowModelFor() != it->first &&
                    it->second->GetShadowModelFor() != it2->first) {
                    auto m2start = it2->second->GetFirstChannel() + 1;
                    auto m2end = it2->second->GetLastChannel() + 1;
                    if (m2start <= m1end && m2end >= m1start) {
                        std::string msg = fmt::format("    WARN: Probable model overlap '{}' ({}-{}) and '{}' ({}-{}).",
                                                      it->first, m1start, m1end, it2->first, m2start, m2end);
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "overlap", it->first));
                    }
                }
            }
        }
    }

    // Non contiguous channels on controller ports
    if (!(_callbacks && _callbacks->IsCheckOptionDisabled("NonContigChOnPort"))) {
        std::map<std::string, std::list<Model*>*> modelsByPort;
        for (const auto& it : _models) {
            if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
                std::string cc;
                if (it.second->IsControllerConnectionValid()) {
                    cc = fmt::format("{}:{}:{}:{}",
                                     it.second->IsPixelProtocol() ? "pixel" : "serial",
                                     it.second->GetControllerProtocol(),
                                     it.second->GetControllerPort(),
                                     it.second->GetSmartRemote());
                }
                if (!cc.empty()) {
                    int32_t start = it.second->GetFirstChannel() + 1;
                    int32_t sc;
                    Output* o = _outputs.GetOutput(start, sc);
                    if (o != nullptr && o->IsIpOutput() && o->GetIP() != "MULTICAST") {
                        std::string key = o->GetIP() + cc;
                        if (modelsByPort.find(key) == modelsByPort.end()) {
                            modelsByPort[key] = new std::list<Model*>();
                        }
                        modelsByPort[key]->push_back(it.second);
                    }
                }
            }
        }
        for (auto& it : modelsByPort) {
            if (it.second->size() > 1 && !Contains(it.first, "serial")) {
                it.second->sort(CompareModelStartChannel);
                auto it2 = it.second->begin();
                auto it3 = it2;
                ++it3;
                while (it3 != it.second->end()) {
                    int32_t m1start = (*it2)->GetNumberFromChannelString((*it2)->ModelStartChannel);
                    int32_t m1end = m1start + (*it2)->GetChanCount() - 1;
                    int32_t m2start = (*it3)->GetNumberFromChannelString((*it3)->ModelStartChannel);

                    if (m1end + 1 != m2start && m2start - m1end - 1 > 0) {
                        int32_t sc;
                        Output* o = _outputs.GetOutput(m1start, sc);
                        std::string ip = o ? o->GetIP() : std::string();
                        std::string msg;
                        if (m2start - m1end - 1 <= 30) {
                            msg = fmt::format("    WARN: Model '{}' and Model '{}' are on controller IP '{}' Output Connection '{}' but there is a small gap of {} channels between them. Maybe these are NULL Pixels?",
                                              (*it2)->GetName(), (*it3)->GetName(), ip,
                                              (*it2)->GetControllerConnectionString(),
                                              m2start - m1end - 1);
                        } else {
                            msg = fmt::format("    WARN: Model '{}' and Model '{}' are on controller IP '{}' Output Connection '{}' but there is a gap of {} channels between them.",
                                              (*it2)->GetName(), (*it3)->GetName(), ip,
                                              (*it2)->GetControllerConnectionString(),
                                              m2start - m1end - 1);
                        }
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "overlapgap", (*it2)->GetName()));
                    }
                    ++it2;
                    ++it3;
                }
            }
            delete it.second;
            it.second = nullptr;
        }
    } else {
        RecordIssue(report, "models",
                    CheckSequenceReport::ReportIssue(
                        CheckSequenceReport::ReportIssue::INFO,
                        "Non contiguous channels on controller ports - CHECK DISABLED",
                        "checkdisabled"));
    }

    // Model nodes not allocated to layers correctly
    for (auto it = _models.begin(); it != _models.end(); ++it) {
        if (it->second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            const std::string& st = it->second->GetStringType();
            if (st.size() >= 5 && st.compare(st.size() - 5, 5, "Nodes") == 0 &&
                !it->second->AllNodesAllocated()) {
                std::string msg = fmt::format("    WARN: {} model '{}' Node Count and Layer Size allocations dont match.",
                                              DisplayAsTypeToString(it->second->GetDisplayAs()), it->first);
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "overlapnodes", it->first));
            }
        }
    }

    // Models with issues (CheckModelSettings)
    for (const auto& it : _models) {
        std::list<std::string> warnings = it.second->CheckModelSettings();
        for (const auto& w : warnings) {
            auto issueType = (w.find("WARN:") != std::string::npos)
                                 ? CheckSequenceReport::ReportIssue::WARNING
                                 : CheckSequenceReport::ReportIssue::CRITICAL;
            RecordIssue(report, "models",
                        CheckSequenceReport::ReportIssue::ForModel(issueType, w, "settings", it.first));
        }
        if ((it.second->GetPixelStyle() == Model::PIXEL_STYLE::PIXEL_STYLE_SOLID_CIRCLE ||
             it.second->GetPixelStyle() == Model::PIXEL_STYLE::PIXEL_STYLE_BLENDED_CIRCLE) &&
            it.second->GetNodeCount() > 100) {
            std::string msg = fmt::format("    WARN: model '{}' uses pixel style '{}' which is known to render really slowly. Consider using a different pixel style.",
                                          it.first, Model::GetPixelStyleDescription(it.second->GetPixelStyle()));
            RecordIssue(report, "models",
                        CheckSequenceReport::ReportIssue::ForModel(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "render", it.first));
        }
    }

    // Model Groups containing models from different previews
    if (!(_callbacks && _callbacks->IsCheckOptionDisabled("PreviewGroup"))) {
        for (const auto& it : _models) {
            if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
                std::string mgp = it.second->GetLayoutGroup();
                ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
                if (mg == nullptr) {
                    std::string msg = fmt::format("Model {} says it is a model group but it doesn't cast as one.",
                                                  it.second->GetName());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "grouperrors", it.second->GetName()));
                } else {
                    auto models = mg->ModelNames();
                    for (auto it2 : models) {
                        Model* m = _models.GetModel(it2);
                        if (m == nullptr) {
                            std::string msg = fmt::format("Model Group {} contains non existent model {}.",
                                                          mg->GetName(), it2);
                            RecordIssue(report, "models",
                                        CheckSequenceReport::ReportIssue::ForModel(
                                            CheckSequenceReport::ReportIssue::CRITICAL, msg, "grouperrors", mg->GetName()));
                        } else if (m->GetDisplayAs() != DisplayAsType::ModelGroup) {
                            if (m->GetLayoutGroup() != "All Previews" &&
                                mg->GetLayoutGroup() != "All Previews" &&
                                mgp != m->GetLayoutGroup()) {
                                std::string msg = fmt::format("    WARN: Model Group '{}' in preview '{}' contains model '{}' which is in preview '{}'. This will cause the '{}' model to also appear in the '{}' preview.",
                                                              mg->GetName(), mg->GetLayoutGroup(),
                                                              m->GetName(), m->GetLayoutGroup(),
                                                              m->GetName(), mg->GetLayoutGroup());
                                RecordIssue(report, "models",
                                            CheckSequenceReport::ReportIssue::ForModel(
                                                CheckSequenceReport::ReportIssue::WARNING, msg, "grouppreview", mg->GetName()));
                            }
                        }
                    }
                }
            }
        }
    } else {
        RecordIssue(report, "models",
                    CheckSequenceReport::ReportIssue(
                        CheckSequenceReport::ReportIssue::INFO,
                        "Model Groups containing models from different previews - CHECK DISABLED",
                        "checkdisabled"));
    }

    // Duplicate model/group names
    for (auto it = std::begin(_models); it != std::end(_models); ++it) {
        for (auto it2 = std::next(it); it2 != std::end(_models); ++it2) {
            if (it->second->GetName() == it2->second->GetName()) {
                std::string msg = fmt::format("    ERR: Duplicate Model/Model Group Name '{}'.", it->second->GetName());
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "groupdistinctnames", it->second->GetName()));
            }
        }
    }

    // Model Groups containing non existent models / itself
    std::list<std::string> emptyModelGroups;
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) {
                auto models = mg->ModelNames();
                int modelCount = 0;
                for (const auto& m : models) {
                    Model* model = _models.GetModel(m);
                    if (model == nullptr) {
                        std::string msg = fmt::format("    ERR: Model group '{}' refers to non existent model '{}'.",
                                                      mg->GetName(), m);
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "groupnonexistent", mg->GetName()));
                    } else {
                        modelCount++;
                        if (model->GetName() == mg->GetName()) {
                            std::string msg = fmt::format("    ERR: Model group '{}' contains reference to itself.", mg->GetName());
                            RecordIssue(report, "models",
                                        CheckSequenceReport::ReportIssue::ForModel(
                                            CheckSequenceReport::ReportIssue::CRITICAL, msg, "groupnonexistent", mg->GetName()));
                        }
                    }
                }
                if (modelCount == 0) {
                    emptyModelGroups.push_back(it.first);
                }
            }
        }
    }
    for (const auto& it : emptyModelGroups) {
        std::string msg = fmt::format("    ERR: Model group '{}' contains no models.", it);
        RecordIssue(report, "models",
                    CheckSequenceReport::ReportIssue::ForModel(
                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "groupnonexistent", it));
    }

    // Model Groups containing duplicate nodes
    if (!(_callbacks && _callbacks->IsCheckOptionDisabled("DupNodeMG"))) {
        for (const auto& it : _models) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) {
                std::map<long, Model*> usedch;
                std::map<std::string, bool> warned;
                for (const auto& m : mg->Models()) {
                    std::vector<NodeBaseClassPtr> nodes;
                    int bufwi;
                    int bufhi;
                    m->InitRenderBufferNodes("Default", "2D", "None", nodes, bufwi, bufhi, 0);
                    for (const auto& n : nodes) {
                        auto e = usedch.find(n->ActChan);
                        if (e != end(usedch)) {
                            if (m->GetFullName() != e->second->GetFullName()) {
                                std::string warn = mg->Name() + m->Name() + e->second->Name();
                                if (warned.find(warn) == end(warned)) {
                                    warned[warn] = true;
                                    std::string msg = fmt::format("    WARN: Model group '{}' contains model '{}' and model '{}' which contain at least one overlapping node (ch {}). This may not render as expected.",
                                                                  mg->Name(), m->GetFullName(), e->second->GetFullName(), n->ActChan);
                                    RecordIssue(report, "models",
                                                CheckSequenceReport::ReportIssue::ForModel(
                                                    CheckSequenceReport::ReportIssue::WARNING, msg, "groupoverlap", mg->Name()));
                                }
                            }
                        } else {
                            usedch[n->ActChan] = m;
                        }
                    }
                }
            }
        }
    }

    // Model Groups with DMX models likely to cause issues
    std::list<ModelGroup*> modelGroupsWithDMXModels;
    for (const auto& it : _models) {
        ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
        if (mg != nullptr) {
            for (const auto& it2 : mg->ModelNames()) {
                auto m = _models[it2];
                if (m != nullptr && m->IsDMXModel()) {
                    modelGroupsWithDMXModels.push_back(mg);
                    break;
                }
            }
        }
    }
    for (const auto& it : _models) {
        ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
        if (mg != nullptr) {
            for (const auto& it2 : modelGroupsWithDMXModels) {
                if (mg->DirectlyContainsModel(it2)) {
                    std::string msg = fmt::format("    WARN: Model group '{}' contains model group '{}' which contains one or more DMX models. This is not likely to work as expected.",
                                                  mg->Name(), it2->Name());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::WARNING, msg, "groupdmx", mg->Name()));
                }
            }
        }
    }
    for (const auto& it : modelGroupsWithDMXModels) {
        int numchannels = -1;
        for (const auto& it2 : it->ModelNames()) {
            auto m = _models[it2];
            if (m == nullptr || !m->IsDMXModel()) {
                std::string msg = fmt::format("    WARN: Model group '{}' contains a mix of DMX and non DMX models. This is not likely to work as expected.",
                                              it->Name());
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "groupdmx", it->Name()));
                break;
            } else {
                if (numchannels == -1) {
                    numchannels = m->GetChanCount();
                } else if ((uint32_t)numchannels != m->GetChanCount()) {
                    std::string msg = fmt::format("    WARN: Model group '{}' contains DMX models with varying numbers of channels. This is not likely to work as expected.",
                                                  it->Name());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::WARNING, msg, "groupdmx", it->Name()));
                    break;
                }
            }
        }
    }

    // Model Groups containing moving heads which have not been numbered
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() == DisplayAsType::ModelGroup) {
            ModelGroup* mg = dynamic_cast<ModelGroup*>(it.second);
            if (mg != nullptr) {
                auto models = mg->ModelNames();
                bool allMovingHeads = true;
                uint32_t count = 0;
                for (const auto& m : models) {
                    Model* model = _models.GetModel(m);
                    if (model != nullptr) {
                        if (model->GetDisplayAs() != DisplayAsType::DmxMovingHeadAdv &&
                            model->GetDisplayAs() != DisplayAsType::DmxMovingHead) {
                            allMovingHeads = false;
                            break;
                        }
                        ++count;
                    }
                }
                if (count > 1 && allMovingHeads) {
                    bool numberOK = false;
                    for (const auto& m : models) {
                        Model* model = _models.GetModel(m);
                        if (model != nullptr) {
                            auto mh = dynamic_cast<DmxMovingHeadComm*>(model);
                            if (mh != nullptr && mh->GetFixture() != "MH1") {
                                numberOK = true;
                                break;
                            }
                        }
                    }
                    if (!numberOK) {
                        std::string msg = fmt::format("    WARN: Model group '{}' contains multiple moving heads but they are all numbered MH1. This may not work as expected with the moving head effect if you want to do fans.",
                                                      mg->GetName());
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "groupmovinghead", mg->GetName()));
                    }
                }
            }
        }
    }

    // SubModels with no nodes
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            for (int i = 0; i < it.second->GetNumSubModels(); ++i) {
                Model* sm = it.second->GetSubModel(i);
                if (sm->GetNodeCount() == 0) {
                    std::string msg = fmt::format("    ERR: SubModel '{}' contains no nodes.", sm->GetFullName());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "submodelsnodes", it.first));
                }
            }
        }
    }

    // SubModels with duplicate nodes
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            for (int i = 0; i < it.second->GetNumSubModels(); ++i) {
                SubModel* sm = dynamic_cast<SubModel*>(it.second->GetSubModel(i));
                if (sm != nullptr) {
                    std::string sameDups = sm->GetSameLineDuplicates();
                    std::string crossDups = sm->GetCrossLineDuplicates();
                    if (!sameDups.empty()) {
                        std::string msg = fmt::format("    WARN: SubModel '{}' contains same line duplicate nodes: {}.",
                                                      sm->GetFullName(), sameDups);
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "submodelsdups", it.first));
                    }
                    if (!crossDups.empty()) {
                        std::string msg = fmt::format("    WARN: SubModel '{}' contains cross line duplicate nodes: {}.",
                                                      sm->GetFullName(), crossDups);
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "submodelsdups", it.first));
                    }
                }
            }
        }
    }

    // SubModels referencing nodes outside parent model range
    for (const auto& it : _models) {
        if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
            for (int i = 0; i < it.second->GetNumSubModels(); ++i) {
                SubModel* sm = dynamic_cast<SubModel*>(it.second->GetSubModel(i));
                if (sm != nullptr && !sm->IsNodesAllValid()) {
                    std::string msg = fmt::format("    ERR: SubModel '{}' has invalid nodes outside the range of the parent model.",
                                                  sm->GetFullName());
                    RecordIssue(report, "models",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "submodelsrange", it.first));
                }
            }
        }
    }

    if (_callbacks && _callbacks->IsCheckOptionDisabled("CustomSizeCheck")) {
        RecordIssue(report, "models",
                    CheckSequenceReport::ReportIssue(
                        CheckSequenceReport::ReportIssue::INFO,
                        "Custom models with excessive blank cells - CHECK DISABLED",
                        "checkdisabled"));
    }

    // Missing matrix face images
    for (const auto& it : _models) {
        auto facefiles = it.second->GetFaceFiles(std::list<std::string>(), true, true);
        for (const auto& fit : facefiles) {
            auto ff = SplitByAny(fit, "|");
            if (ff.size() < 2) {
                std::string msg = fmt::format("    ERR: Model '{}' has a malformed face entry '{}'.",
                                              it.second->GetFullName(), fit);
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "faces", it.first));
                continue;
            }
            if (!FileExists(ff[1])) {
                std::string msg = fmt::format("    ERR: Model '{}' face '{}' image missing {}.",
                                              it.second->GetFullName(), ff[0], ff[1]);
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "faces", it.first));
            }
        }
    }

    // Large blocks of unused channels + single line orientation
    {
        std::list<Model*> modelssorted;
        for (const auto& it : _models) {
            if (it.second->GetDisplayAs() != DisplayAsType::ModelGroup) {
                modelssorted.push_back(it.second);
            }
        }
        modelssorted.sort(CompareModelStartChannel);

        int32_t last = 0;
        Model* lastm = nullptr;
        for (const auto& m : modelssorted) {
            int32_t start = m->GetNumberFromChannelString(m->ModelStartChannel);
            int32_t gap = start - last - 1;
            if (gap > 511) {
                auto issueType = (gap > 49999) ? CheckSequenceReport::ReportIssue::CRITICAL
                                               : CheckSequenceReport::ReportIssue::WARNING;
                const char* level = (issueType == CheckSequenceReport::ReportIssue::CRITICAL) ? "ERR" : "WARN";
                std::string msg;
                if (lastm == nullptr) {
                    msg = fmt::format("    {}: First Model '{}' starts at channel {} leaving a block of {} of unused channels.",
                                      level, m->GetName(), start, start - 1);
                } else {
                    msg = fmt::format("    {}: Model '{}' starts at channel {} leaving a block of {} of unused channels between this and the prior model '{}'.",
                                      level, m->GetName(), start, gap, lastm->GetName());
                }
                RecordIssue(report, "models",
                            CheckSequenceReport::ReportIssue::ForModel(issueType, msg, "universe", m->GetName()));
            }
            long newlast = start + m->GetChanCount() - 1;
            if (newlast > last) {
                last = newlast;
                lastm = m;
            }
            if (m->GetDisplayAs() == DisplayAsType::SingleLine) {
                size_t nodeCount = m->GetNodeCount();
                if (nodeCount < 2)
                    continue;
                TwoPointScreenLocation& screenLoc = dynamic_cast<TwoPointScreenLocation&>(m->GetBaseObjectScreenLocation());
                float dx = screenLoc.GetX2();
                float dy = screenLoc.GetY2();
                float dz = screenLoc.GetZ2();
                float deltaX = std::fabs(dx);
                float deltaY = std::fabs(dy);
                float deltaZ = std::fabs(dz);
                if (deltaX > deltaY && deltaX > deltaZ) {
                    if (dx < 0) {
                        std::string msg = fmt::format("    WARN: Model '{}' should have the green square on the left of the blue square for best render results.",
                                                      m->GetName());
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "config", m->GetName()));
                    }
                } else if (deltaY > deltaX && deltaY > deltaZ) {
                    if (dy < 0) {
                        std::string msg = fmt::format("    WARN: Model '{}' should have the green square on the bottom of the blue square for best render results.",
                                                      m->GetName());
                        RecordIssue(report, "models",
                                    CheckSequenceReport::ReportIssue::ForModel(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "config", m->GetName()));
                    }
                }
            }
        }
    }

    return _errors - startErrors;
}

void SequenceChecker::CheckEffect(Effect* ef, CheckSequenceReport& report,
                                  const std::string& elementName,
                                  const std::string& modelName,
                                  int layerIndex,
                                  bool node,
                                  bool& videoCacheWarning,
                                  bool& disabledEffects,
                                  std::list<std::pair<std::string, std::string>>& faces,
                                  std::list<std::pair<std::string, std::string>>& states,
                                  std::list<std::string>& viewPoints,
                                  std::list<std::string>& allFiles) {
    EffectManager& em = _elements.GetEffectManager();
    SettingsMap& sm = ef->GetSettings();
    std::string renderCacheMode = _callbacks ? _callbacks->GetRenderCacheMode() : std::string("Enabled");

    if (ef->GetEffectName() == "Video") {
        if (renderCacheMode == "Disabled") {
            videoCacheWarning = true;
        } else if (!ef->IsLocked() && renderCacheMode == "Locked Only") {
            videoCacheWarning = true;
            std::string msg = fmt::format("    WARN: Video effect unlocked but only locked video effects are being render cached. Effect: {}, Model: {}, Start {}",
                                          ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue::ForEffect(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "videocache",
                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
        }
    }

    if (ef->IsRenderDisabled())
        disabledEffects = true;

    bool isPerModel = false;
    bool isSubBuffer = false;
    for (const auto& it : sm) {
        isPerModel |= (it.first == "B_CHOICE_BufferStyle" && StartsWith(it.second, "Per Model"));
        isSubBuffer |= (it.first == "B_CUSTOM_SubBuffer" && it.second != "");
    }
    if (isPerModel && isSubBuffer) {
        std::string msg = fmt::format("    ERR: Effect on a model group using a 'Per Model' render buffer is also using a subbuffer. This will not work as you might expect. Effect: {}, Model: {}, Start {}",
                                      ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
        RecordIssue(report, "sequence",
                    CheckSequenceReport::ReportIssue::ForEffect(
                        CheckSequenceReport::ReportIssue::CRITICAL, msg, "buffer",
                        modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
    }

    // very old value curves not yet upgraded
    for (auto it = sm.begin(); it != sm.end(); ++it) {
        const std::string& value = it->second;
        if (value.find("|Type=") != std::string::npos &&
            value.find("RV=TRUE") == std::string::npos) {
            size_t startPos = value.find("|Id=");
            std::string property;
            if (startPos != std::string::npos) {
                startPos += 4;
                size_t endPos = value.find('|', startPos);
                if (endPos == std::string::npos)
                    endPos = value.size();
                property = value.substr(startPos, endPos - startPos);
            }
            std::string msg = fmt::format("    ERR: Effect contains very old value curve. Click on this effect and then save the sequence to convert it. Effect: {}, Model: {}, Start {} ({})",
                                          ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()), property);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue::ForEffect(
                            CheckSequenceReport::ReportIssue::CRITICAL, msg, "oldcurves",
                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
        }
    }

    float fadein = sm.GetFloat("T_TEXTCTRL_Fadein", 0.0);
    float fadeout = sm.GetFloat("T_TEXTCTRL_Fadeout", 0.0);
    float efdur = (ef->GetEndTimeMS() - ef->GetStartTimeMS()) / 1000.0;

    if (sm.Get("T_CHECKBOX_Canvas", "0") == "1") {
        if (ef->GetEffectName() != "Off" && ef->GetEffectName() != "Warp" &&
            ef->GetEffectName() != "Kaleidoscope" && ef->GetEffectName() != "Shader") {
            std::string msg = fmt::format("    WARN: Canvas mode enabled on an effect it is not normally used on. This will slow down rendering. Effect: {}, Model: {}, Start {}",
                                          ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue::ForEffect(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "canvas",
                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
        }
    }

    if (!(_callbacks && _callbacks->IsCheckOptionDisabled("TransTime"))) {
        if (fadein > efdur) {
            std::string msg = fmt::format("    WARN: Transition in time {:.2f} on effect {} at start time {}  on Model '{}' is greater than effect duration {:.2f}.",
                                          fadein, ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), elementName, efdur);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue::ForEffect(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "transitions",
                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
        }
        if (fadeout > efdur) {
            std::string msg = fmt::format("    WARN: Transition out time {:.2f} on effect {} at start time {}  on Model '{}' is greater than effect duration {:.2f}.",
                                          fadeout, ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), elementName, efdur);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue::ForEffect(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "transitions",
                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
        }
        if (fadein <= efdur && fadeout <= efdur && fadein + fadeout > efdur) {
            std::string msg = fmt::format("    WARN: Transition in time {:.2f} + transition out time {:.2f} = {:.2f} on effect {} at start time {}  on Model '{}' is greater than effect duration {:.2f}.",
                                          fadein, fadeout, fadein + fadeout, ef->GetEffectName(),
                                          FORMATTIME(ef->GetStartTimeMS()), elementName, efdur);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue::ForEffect(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "transitions",
                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
        }
    }

    if (_sequenceFile != nullptr && ef->GetEndTimeMS() > _sequenceFile->GetSequenceDurationMS()) {
        std::string msg = fmt::format("    WARN: Effect {} ends at {} after the sequence end {}. Model: '{}' Start: {}",
                                      ef->GetEffectName(), FORMATTIME(ef->GetEndTimeMS()),
                                      FORMATTIME(_sequenceFile->GetSequenceDurationMS()), elementName,
                                      FORMATTIME(ef->GetStartTimeMS()));
        RecordIssue(report, "sequence",
                    CheckSequenceReport::ReportIssue::ForEffect(
                        CheckSequenceReport::ReportIssue::WARNING, msg, "timing",
                        modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
    }

    auto looksNumericEnough = [](const std::string& v) -> bool {
        bool hasNumericChar = false;
        for (char c : v) {
            unsigned char uc = static_cast<unsigned char>(c);
            if (std::isdigit(uc) || c == '.' || c == '-' || c == '+' || c == '`') {
                hasNumericChar = true;
            } else {
                return false;
            }
        }
        return hasNumericChar;
    };

    auto isValidNum = [](const std::string& v) -> bool {
        if (v.empty()) return true;
        size_t i = 0;
        if (v[i] == '+' || v[i] == '-') ++i;
        if (i == v.size()) return false;
        bool hasDigit = false, hasDot = false;
        while (i < v.size()) {
            unsigned char c = static_cast<unsigned char>(v[i]);
            if (std::isdigit(c)) {
                hasDigit = true;
            } else if (v[i] == '.' && !hasDot) {
                hasDot = true;
            } else if ((v[i] == 'e' || v[i] == 'E') && hasDigit) {
                ++i;
                if (i < v.size() && (v[i] == '+' || v[i] == '-')) ++i;
                if (i == v.size() || !std::isdigit(static_cast<unsigned char>(v[i]))) return false;
                while (i < v.size() && std::isdigit(static_cast<unsigned char>(v[i]))) ++i;
                return i == v.size();
            } else {
                return false;
            }
            ++i;
        }
        return hasDigit;
    };

    auto checkMapForCorruptNumbers = [&](const SettingsMap& map) {
        for (const auto& kv : map) {
            const std::string& key = kv.first;
            const std::string& val = kv.second;
            if (val.empty()) continue;
            if (key.find("_VALUECURVE_") != std::string::npos) continue;

            bool checkAsNumeric = false;
            if (key.find("_SLIDER_") != std::string::npos ||
                key.find("_SPINCTRL_") != std::string::npos) {
                checkAsNumeric = true;
            } else if (key == "T_TEXTCTRL_Fadein" || key == "T_TEXTCTRL_Fadeout") {
                checkAsNumeric = true;
            } else if (key.find("_TEXTCTRL_") != std::string::npos) {
                // Validate only when the stored value looks like it was meant
                // to be a number (no letters, slashes, '#', etc.)
                checkAsNumeric = looksNumericEnough(val);
            }
            if (!checkAsNumeric) continue;

            if (!isValidNum(val)) {
                std::string msg = fmt::format(
                    "    ERR: Effect has invalid numeric value '{}' for setting '{}'. Effect: {}, Model: {}, Start {}",
                    val, key, ef->GetEffectName(), modelName, FORMATTIME(ef->GetStartTimeMS()));
                RecordIssue(report, "sequence",
                            CheckSequenceReport::ReportIssue::ForEffect(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "corruptsettings",
                                modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
            }
        }
    };
    checkMapForCorruptNumbers(ef->GetSettings());
    checkMapForCorruptNumbers(ef->GetPaletteMap());

    if (ef->GetEffectIndex() >= 0) {
        RenderableEffect* re = em.GetEffect(ef->GetEffectIndex());
        if (re != nullptr) {
            if (node && !re->AppropriateOnNodes()) {
                std::string msg = fmt::format("    WARN: Effect {} at start time {}  on Model '{}' really shouldnt be used at the node level.",
                                              ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()), elementName);
                RecordIssue(report, "sequence",
                            CheckSequenceReport::ReportIssue::ForEffect(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "nodes",
                                modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
            }

            bool renderCache = renderCacheMode == "Enabled" ||
                               (renderCacheMode == "Locked Only" && ef->IsLocked());
            Model* m = _models.GetModel(elementName);
            if (m == nullptr) {
                m = _models.GetModel(modelName);
            }
            AudioManager* media = _sequenceFile ? _sequenceFile->GetMedia() : nullptr;

            // `RenderableEffect::GetTiming()` (used by VUMeter, Servo,
            // Faces, State, etc. inside CheckEffectSettings to validate
            // timing-track references) walks `mSequenceElements`. The
            // EffectManager singletons never have it set (no propagation
            // path on either platform), which made every "Timing Event"
            // VU Meter etc. report "unknown timing track" even when the
            // track exists. Set it for the call and restore so we don't
            // leave a dangling pointer once this checker goes away.
            SequenceElements* prevSeqElements = re->GetSequenceElements();
            re->SetSequenceElements(&_elements);
            std::list<std::string> warnings = re->CheckEffectSettings(sm, media, m, ef, renderCache);
            re->SetSequenceElements(prevSeqElements);
            for (const auto& s : warnings) {
                auto issueType = (s.find("WARN:") != std::string::npos)
                                     ? CheckSequenceReport::ReportIssue::WARNING
                                     : CheckSequenceReport::ReportIssue::CRITICAL;
                RecordIssue(report, "sequence",
                            CheckSequenceReport::ReportIssue::ForEffect(
                                issueType, s + "--Effect:" + ef->GetEffectName(), "effectsettings",
                                modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layerIndex));
            }

            // Accumulate referenced files for this effect
            if (m != nullptr) {
                auto refs = re->GetFileReferences(m, sm);
                allFiles.splice(allFiles.end(), refs);
            }

            if (ef->GetEffectName() == "Faces") {
                for (const auto& it : static_cast<FacesEffect*>(re)->GetFacesUsed(sm)) {
                    bool found = false;
                    for (auto it2 : faces) {
                        if (it2.first == modelName && it2.second == it) {
                            found = true;
                        }
                    }
                    if (!found) {
                        faces.push_back({ modelName, it });
                    }
                }
            } else if (ef->GetEffectName() == "State") {
                for (const auto& it : static_cast<StateEffect*>(re)->GetStatesUsed(sm)) {
                    bool found = false;
                    for (auto it2 : states) {
                        if (it2.first == modelName && it2.second == it) {
                            found = true;
                        }
                    }
                    if (!found) {
                        states.push_back({ modelName, it });
                    }
                }
            }

            for (const auto& it : sm) {
                if (it.first == "B_CHOICE_PerPreviewCamera") {
                    bool found = false;
                    for (auto it2 : viewPoints) {
                        if (it2 == it.second) {
                            found = true;
                        }
                    }
                    if (!found) {
                        viewPoints.push_back(it.second);
                    }
                }
            }
        }
    }
}

void SequenceChecker::CheckElement(Element* e, CheckSequenceReport& report,
                                   const std::string& name,
                                   const std::string& modelName,
                                   bool& videoCacheWarning,
                                   bool& disabledEffects,
                                   std::list<std::pair<std::string, std::string>>& faces,
                                   std::list<std::pair<std::string, std::string>>& states,
                                   std::list<std::string>& viewPoints,
                                   bool& usesShader,
                                   std::list<std::string>& allFiles) {
    Model* m = _models[modelName];
    EffectManager& em = _elements.GetEffectManager();

    int layer = 0;
    for (const auto& el : e->GetEffectLayers()) {
        layer++;
        for (const auto& ef : el->GetEffects()) {
            if (ef->GetEffectName() == "Random") {
                std::string msg = fmt::format("    ERR: Effect {} ({}-{}) on Model '{}' layer {} is a random effect. This should never happen and may cause other issues.",
                                              ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()),
                                              FORMATTIME(ef->GetEndTimeMS()), name, layer);
                RecordIssue(report, "sequence",
                            CheckSequenceReport::ReportIssue::ForEffect(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "unexpected",
                                modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layer));
            } else {
                RenderableEffect* eff = em.GetEffect(ef->GetEffectIndex());
                if (eff != nullptr) {
                    auto refs = eff->GetFileReferences(m, ef->GetSettings());
                    allFiles.splice(allFiles.end(), refs);
                }

                if (m != nullptr) {
                    if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                        if (m->GetNodeCount() == 0) {
                            std::string msg = fmt::format("    ERR: Effect {} ({}-{}) on Model '{}' layer {} Has no nodes and wont do anything.",
                                                          ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()),
                                                          FORMATTIME(ef->GetEndTimeMS()), name, layer);
                            RecordIssue(report, "sequence",
                                        CheckSequenceReport::ReportIssue::ForEffect(
                                            CheckSequenceReport::ReportIssue::CRITICAL, msg, "nonodestorender",
                                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layer));
                        }
                    } else if (e->GetType() == ElementType::ELEMENT_TYPE_STRAND) {
                        StrandElement* se = (StrandElement*)e;
                        if (m->GetStrandLength(se->GetStrand()) == 0) {
                            std::string msg = fmt::format("    ERR: Effect {} ({}-{}) on Model '{}' layer {} Has no nodes and wont do anything.",
                                                          ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()),
                                                          FORMATTIME(ef->GetEndTimeMS()), name, layer);
                            RecordIssue(report, "sequence",
                                        CheckSequenceReport::ReportIssue::ForEffect(
                                            CheckSequenceReport::ReportIssue::CRITICAL, msg, "nonodestorender",
                                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layer));
                        }
                    } else if (e->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                        Model* se = _models[name];
                        if (se != nullptr && se->GetNodeCount() == 0) {
                            std::string msg = fmt::format("    ERR: Effect {} ({}-{}) on Model '{}' layer {} Has no nodes and wont do anything.",
                                                          ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()),
                                                          FORMATTIME(ef->GetEndTimeMS()), name, layer);
                            RecordIssue(report, "sequence",
                                        CheckSequenceReport::ReportIssue::ForEffect(
                                            CheckSequenceReport::ReportIssue::CRITICAL, msg, "nonodestorender",
                                            modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layer));
                        }
                    }
                }

                CheckEffect(ef, report, name, modelName, layer, false,
                            videoCacheWarning, disabledEffects, faces, states, viewPoints, allFiles);
                if (ef->GetEffectName() == "Shader") {
                    usesShader = true;
                }
            }
        }

        Effect* lastEffect = nullptr;
        for (const auto& ef : el->GetEffects()) {
            if (lastEffect != nullptr) {
                if (ef->GetStartTimeMS() < lastEffect->GetEndTimeMS()) {
                    std::string msg = fmt::format("    ERR: Effect {} ({}-{}) overlaps with Effect {} ({}-{}) on Model '{}' layer {}. This shouldn't be possible.",
                                                  ef->GetEffectName(), FORMATTIME(ef->GetStartTimeMS()),
                                                  FORMATTIME(ef->GetEndTimeMS()),
                                                  lastEffect->GetEffectName(),
                                                  FORMATTIME(lastEffect->GetStartTimeMS()),
                                                  FORMATTIME(lastEffect->GetEndTimeMS()),
                                                  name, layer);
                    RecordIssue(report, "sequence",
                                CheckSequenceReport::ReportIssue::ForEffect(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "impossibleoverlap",
                                    modelName, ef->GetEffectName(), ef->GetStartTimeMS(), layer));
                }
            }
            lastEffect = ef;
        }
    }
}

int SequenceChecker::RunSequenceChecks(CheckSequenceReport& report) {
    int startErrors = _errors;

    if (_sequenceFile != nullptr) {
        if (_sequenceFile->GetRenderMode() == SequenceFile::CANVAS_MODE) {
            std::string msg = "    WARN: Render mode set to canvas mode. Unless you specifically know you need this it is not recommended.";
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "general"));
        }

        if (_callbacks && _callbacks->IsCheckOptionDisabled("TransTime")) {
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::INFO,
                            "Effect transition times - CHECK DISABLED.",
                            "checkdisabled"));
        }

        bool dataLayer = false;
        DataLayerSet& data_layers = _sequenceFile->GetDataLayers();
        for (int j = 0; j < data_layers.GetNumLayers(); ++j) {
            DataLayer* dl = data_layers.GetDataLayer(j);
            if (dl->GetName() != "Nutcracker") {
                dataLayer = true;
                break;
            }
        }
        if (dataLayer) {
            std::string msg = "    WARN: Sequence includes a data layer. There is nothing wrong with this but it is uncommon and not always intended.";
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::WARNING, msg, "general"));
        }

        if (_sequenceFile->GetSequenceType() == "Media") {
            if (!FileExists(_sequenceFile->GetMediaFile())) {
                std::string msg = fmt::format("    ERR: media file {} does not exist.",
                                              _sequenceFile->GetMediaFile());
                RecordIssue(report, "sequence",
                            CheckSequenceReport::ReportIssue(
                                CheckSequenceReport::ReportIssue::CRITICAL, msg, "general"));
            }
        }

        // Models hidden by effects on groups (only when blending is off)
        if (!_sequenceFile->supportsModelBlending()) {
            std::string viewModels = _elements.GetViewModels(_elements.GetViewName(0));
            auto modelnames = SplitByAny(viewModels, ",");

            std::list<std::string> seenmodels;
            for (const auto& it : modelnames) {
                Model* m = _models.GetModel(it);
                if (m == nullptr) {
                    std::string msg = fmt::format("    ERR: Model {} in your sequence does not seem to exist in the layout. This will need to be deleted or remapped to another model next time you load this sequence.",
                                                  it);
                    RecordIssue(report, "sequence",
                                CheckSequenceReport::ReportIssue::ForModel(
                                    CheckSequenceReport::ReportIssue::CRITICAL, msg, "modelnotinlayout", it));
                } else {
                    if (m->GetDisplayAs() == DisplayAsType::ModelGroup) {
                        ModelGroup* mg = dynamic_cast<ModelGroup*>(m);
                        if (mg != nullptr) {
                            for (auto it2 : mg->Models()) {
                                if (std::find(seenmodels.begin(), seenmodels.end(), it2->GetName()) != seenmodels.end()) {
                                    std::string msg = fmt::format("    WARN: Model Group '{}' will hide effects on model '{}'.",
                                                                  mg->GetName(), it2->GetName());
                                    RecordIssue(report, "sequence",
                                                CheckSequenceReport::ReportIssue::ForModel(
                                                    CheckSequenceReport::ReportIssue::WARNING, msg, "modeleffectshidden", mg->GetName()));
                                }
                            }
                        }
                    } else {
                        seenmodels.push_back(m->GetName());
                    }
                }
            }
        }

        // Walk every element / effect
        bool disabledEffects = false;
        bool videoCacheWarning = false;
        std::list<std::pair<std::string, std::string>> faces;
        std::list<std::pair<std::string, std::string>> states;
        std::list<std::string> viewPoints;
        bool usesShader = false;
        std::list<std::string> allfiles;

        for (const auto& it : _models) {
            allfiles.splice(allfiles.end(), it.second->GetFileReferences());
        }

        for (size_t i = 0; i < _elements.GetElementCount(MASTER_VIEW); i++) {
            Element* e = _elements.GetElement(i);
            if (e->GetType() != ElementType::ELEMENT_TYPE_TIMING) {
                CheckElement(e, report, e->GetFullName(), e->GetName(),
                             videoCacheWarning, disabledEffects, faces, states,
                             viewPoints, usesShader, allfiles);

                if (e->GetType() == ElementType::ELEMENT_TYPE_MODEL) {
                    ModelElement* me = dynamic_cast<ModelElement*>(e);
                    if (me != nullptr) {
                        Model* model = _models[me->GetModelName()];
                        for (int j = 0; j < me->GetStrandCount(); ++j) {
                            StrandElement* se = me->GetStrand(j);
                            CheckElement(se, report, se->GetFullName(), e->GetName(),
                                         videoCacheWarning, disabledEffects, faces, states,
                                         viewPoints, usesShader, allfiles);

                            for (int k = 0; k < se->GetNodeLayerCount(); ++k) {
                                NodeLayer* nl = se->GetNodeLayer(k);
                                for (int l = 0; l < nl->GetEffectCount(); l++) {
                                    Effect* ef = nl->GetEffect(l);
                                    std::string nodeName = fmt::format("{} Strand {}/Node {}",
                                                                        se->GetFullName(), j + 1, l + 1);
                                    CheckEffect(ef, report, nodeName, e->GetName(),
                                                k, true, videoCacheWarning, disabledEffects,
                                                faces, states, viewPoints, allfiles);
                                    EffectManager& em = _elements.GetEffectManager();
                                    RenderableEffect* eff = em.GetEffect(ef->GetEffectIndex());
                                    if (eff != nullptr && model != nullptr) {
                                        auto refs = eff->GetFileReferences(model, ef->GetSettings());
                                        allfiles.splice(allfiles.end(), refs);
                                    }
                                }
                            }
                        }
                        for (int j = 0; j < me->GetSubModelAndStrandCount(); ++j) {
                            Element* sme = me->GetSubModel(j);
                            if (sme != nullptr && sme->GetType() == ElementType::ELEMENT_TYPE_SUBMODEL) {
                                CheckElement(sme, report, sme->GetFullName(), e->GetName(),
                                             videoCacheWarning, disabledEffects, faces, states,
                                             viewPoints, usesShader, allfiles);
                            }
                        }
                    }
                }
            }
        }

        if (videoCacheWarning) {
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::WARNING,
                            "    WARN: Sequence has one or more video effects where render caching is turned off. This will render slowly.",
                            "sequencegeneral"));
        }
        if (disabledEffects) {
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::WARNING,
                            "    WARN: Sequence has one or more effects which are disabled. They are being ignored.",
                            "sequencegeneral"));
        }

        // Summary lists (faces / states / viewpoints used)
        for (const auto& it : faces) {
            std::string msg = fmt::format("        Model: {}, Face: {}.", it.first, it.second);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::INFO, msg, "usedfaces"));
        }
        for (const auto& it : states) {
            std::string msg = fmt::format("        Model: {}, State: {}.", it.first, it.second);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::INFO, msg, "usedstates"));
        }
        for (const auto& it : viewPoints) {
            std::string msg = fmt::format("        Viewpoint: {}.", it);
            RecordIssue(report, "sequence",
                        CheckSequenceReport::ReportIssue(
                            CheckSequenceReport::ReportIssue::INFO, msg, "usedviewpoints"));
        }
    }

    return _errors - startErrors;
}

int SequenceChecker::RunFileReferenceChecks(CheckSequenceReport& report) {
    int startErrors = _errors;

    // Build the union of all referenced files (model file refs + effect refs)
    std::list<std::string> allfiles;
    for (const auto& it : _models) {
        allfiles.splice(allfiles.end(), it.second->GetFileReferences());
    }
    if (_sequenceFile != nullptr) {
        EffectManager& em = _elements.GetEffectManager();
        for (size_t i = 0; i < _elements.GetElementCount(MASTER_VIEW); ++i) {
            Element* e = _elements.GetElement(i);
            if (e == nullptr || e->GetType() == ElementType::ELEMENT_TYPE_TIMING)
                continue;
            ModelElement* me = dynamic_cast<ModelElement*>(e);
            Model* model = me ? _models[me->GetModelName()] : nullptr;
            for (const auto& el : e->GetEffectLayers()) {
                for (const auto& ef : el->GetEffects()) {
                    RenderableEffect* eff = em.GetEffect(ef->GetEffectIndex());
                    if (eff != nullptr && model != nullptr) {
                        auto refs = eff->GetFileReferences(model, ef->GetSettings());
                        allfiles.splice(allfiles.end(), refs);
                    }
                }
            }
        }
    }

    if (_showFolder.empty())
        return _errors - startErrors;

    // Show-folder name repeated in path warning
    std::string showdir = _showFolder;
    {
        auto bs = showdir.find_last_of('\\');
        auto fs = showdir.find_last_of('/');
        std::string sd2 = (bs != std::string::npos) ? showdir.substr(bs + 1) : std::string();
        std::string sd3 = (fs != std::string::npos) ? showdir.substr(fs + 1) : std::string();
        if (!sd2.empty() && sd2.size() < showdir.size())
            showdir = sd2;
        if (!sd3.empty() && sd3.size() < showdir.size())
            showdir = sd3;
    }

    for (const auto& it : allfiles) {
        std::string ff = FileUtils::FixFile(_showFolder, it);
        if (StartsWith(ff, _showFolder)) {
            if (FileExists(ff)) {
                std::string rel = ff.substr(_showFolder.size());
                auto folders = SplitByAny(rel, "\\/");
                for (const auto& it2 : folders) {
                    if (it2 == showdir) {
                        std::string msg = fmt::format("    WARN: path to file {} contains the show folder name '{}' more than once. This will make it hard to move sequence to other computers as it won't be able to fix paths automatically.",
                                                      it, showdir);
                        RecordIssue(report, "os",
                                    CheckSequenceReport::ReportIssue(
                                        CheckSequenceReport::ReportIssue::WARNING, msg, "paths"));
                    }
                }
            } else {
                std::string msg = fmt::format("    WARN: Unable to check file {} because it was not found. If this location is on another computer please run check sequence there to check this condition properly.",
                                              it);
                RecordIssue(report, "os",
                            CheckSequenceReport::ReportIssue(
                                CheckSequenceReport::ReportIssue::WARNING, msg, "paths"));
            }
        }
    }

    return _errors - startErrors;
}
