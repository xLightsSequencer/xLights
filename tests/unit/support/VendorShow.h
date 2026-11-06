#pragma once

// A tiny show (one Ethernet controller plus models wired to its ports) so a vendor
// handler's SetOutputs can be driven end to end. Include after the vendor handler headers.
#include <filesystem>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "controllers/ControllerCaps.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "render/UICallbacks.h"
#include "support/TempDir.h"
#include "support/VendorCapsXml.h"
#include "utils/FileUtils.h"

// Loads the vendor files from VendorCapsXml.h (plus a vendor with no config driver) so
// UDController runs the checks users get for those boards.
struct RealCapsFixture {
    TempDir dir;
    std::string prevResources;
    RealCapsFixture() {
        prevResources = FileUtils::GetResourcesDir();
        std::filesystem::create_directories(dir.path / "controllers");
        dir.Write("controllers/advatek.xcontroller", kCaps_advatek);
        dir.Write("controllers/falcon.xcontroller", kCaps_falcon);
        dir.Write("controllers/hinkspix.xcontroller", kCaps_hinkspix);
        dir.Write("controllers/ilightthat.xcontroller", kCaps_ilightthat);
        dir.Write("controllers/j1sys.xcontroller", kCaps_j1sys);
        dir.Write("controllers/experience.xcontroller", kCaps_experience);
        dir.Write("controllers/powerdmx.xcontroller", kCaps_powerdmx);
        dir.Write("controllers/wled.xcontroller", kCaps_wled);
        dir.Write("controllers/acme.xcontroller", R"(<Vendor Name="Acme"><Controller Name="Plain"><Variant Name=""><SupportsUpload/><MaxPixelPort>4</MaxPixelPort></Variant></Controller></Vendor>)");
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(dir.path.string());
        ControllerCaps::LoadControllers();
    }
    ~RealCapsFixture() {
        ControllerCaps::UnloadControllers();
        FileUtils::SetResourcesDir(prevResources);
    }
};

class RecordingUi : public UICallbacks {
public:
    mutable std::vector<std::string> messages;
    mutable std::vector<std::string> captions;
    bool yes = true;

    void ShowMessage(const std::string& m, const std::string& c) const override {
        messages.push_back(m);
        captions.push_back(c);
    }
    bool PromptYesNo(const std::string& m, const std::string&) const override {
        messages.push_back(m);
        return yes;
    }
    std::string PromptForDirectory(const std::string&, const std::string&) const override { return ""; }
    std::string PromptForFile(const std::string&, const std::string&, const std::string&) const override { return ""; }
    long PromptForNumber(const std::string&, const std::string&, long d, long, long) const override { return d; }
    std::string PromptForText(const std::string&, const std::string&, const std::string& d) const override { return d; }
    std::vector<std::string> ChooseFromList(const std::string&, const std::vector<std::string>&) const override { return {}; }
    ProgressToken BeginProgress(const std::string&, int) override { return 1; }
    void UpdateProgress(ProgressToken, int, const std::string&) override {}
    void EndProgress(ProgressToken) override {}

    bool Said(const std::string& needle) const {
        for (auto& m : messages) {
            if (m.find(needle) != std::string::npos) return true;
        }
        return false;
    }
    std::string All() const {
        std::string r;
        for (auto& m : messages) r += m + "\n";
        return r;
    }
};

struct VendorShow {
    RealCapsFixture caps;
    OutputManager om;
    ModelManager mm{ &om };
    ControllerEthernet* ctl = nullptr;
    RecordingUi ui;

    // universes outputs of `size` channels each, starting at universe `firstUniverse`
    VendorShow(const std::string& vendor, const std::string& model, const std::string& variant, int universes = 4,
               const std::string& protocol = OUTPUT_E131, int size = 510, int firstUniverse = 1, const std::string& ip = "127.0.0.1") {
        ctl = new ControllerEthernet(&om);
        ctl->SetName("Ctl");
        // a new controller defaults to full control; one loaded from a show file does not
        ctl->SetFullxLightsControl(false);
        ctl->SetAutoLayout(false);
        ctl->SetProtocol(protocol);
        ctl->SetIP(ip);
        for (int i = 1; i < universes; ++i) ctl->AddOutput();
        int u = firstUniverse;
        for (auto* o : ctl->GetOutputs()) {
            o->SetUniverse(u++);
            o->SetChannels(size);
        }
        ctl->SetVendor(vendor);
        ctl->SetModel(model);
        ctl->SetVariant(variant);
        om.AddController(ctl);
        om.SomethingChanged();
    }

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        if (!doc.load_string(xml.c_str())) return nullptr;
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        mm.RecalcStartChannels();
        return m;
    }

    // A single string pixel line on a pixel port. `cc` is extra ControllerConnection attributes.
    Model* Pixels(const std::string& name, int port, int nodes, int startChannel, const std::string& cc = "",
                  const std::string& protocol = "ws2811", int strings = 1, const std::string& extra = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" NumStrings=\"" + std::to_string(strings) + "\" NodesPerString=\"" +
                   std::to_string(nodes) + "\" StartChannel=\"" + std::to_string(startChannel) + "\" StringType=\"RGB Nodes\" Controller=\"Ctl\" " + extra +
                   "><ControllerConnection Port=\"" + std::to_string(port) + "\" Protocol=\"" + protocol + "\" " + cc + "/></model>");
    }

    // A channel block on a serial port.
    Model* Serial(const std::string& name, int port, int channels, int startChannel, const std::string& protocol = "DMX", const std::string& cc = "") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Channel Block\" parm1=\"" + std::to_string(channels) + "\" StartChannel=\"" +
                   std::to_string(startChannel) + "\" StringType=\"Single Color White\" Controller=\"Ctl\"><ControllerConnection Port=\"" +
                   std::to_string(port) + "\" Protocol=\"" + protocol + "\" " + cc + "/></model>");
    }
};

// REQUIRE an upload succeeded, reporting what the handler told the user when it did not.
#define REQUIRE_UPLOAD(show, expr)        \
    do {                                   \
        bool uploadOk_ = (expr);           \
        INFO("ui: " << (show).ui.All());   \
        REQUIRE(uploadOk_);                \
    } while (0)
