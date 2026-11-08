#pragma once

// Shared fixtures for the [fpp3] tests: an FPP handler with its privates reachable
// (ExposeVendors.h), a loopback FPP stand-in that answers GETs from a route table and
// records every request, and a small show (one controller, models on its ports) built
// against hand written FPP capabilities.  POSIX/Itanium-ABI only, like the other
// handler tests.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// Everything the exposed headers pull in must be in the TU before the widening macros.
#include <chrono>
#include <future>
#include <thread>
#include "outputs/ControllerEthernet.h"
#include "outputs/SocketAbstraction.h"
#include "support/ExposeVendors.h"
#include "discovery/Discovery.h"
#include "controllers/FPP.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"
#include "support/VendorCapsFixture.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "controllers/ControllerCaps.h"
#include "controllers/ControllerUploadData.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerSerial.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "render/UICallbacks.h"
#include "utils/CurlManager.h"

namespace fpp3 {

inline void InitLoggers() {
    VendorsTestInit();
    for (const char* n : { "work", "curl" }) {
        if (!spdlog::get(n)) {
            spdlog::register_logger(std::make_shared<spdlog::logger>(n, std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
    }
}

// A loopback FPP.  GET (and any method on a path registered with Any) answers from the
// route table, everything else gets 200 with an empty body; an unknown GET is a 404.
class FppServer {
public:
    FppServer() :
        _srv([this](const FakeRequest& r) { return Handle(r); }) {}

    void Get(const std::string& path, const std::string& body, int status = 200) {
        std::lock_guard<std::mutex> l(_m);
        _get[path] = { status, body };
    }
    void GetJson(const std::string& path, const nlohmann::json& body) {
        Get(path, body.dump());
    }
    void Post(const std::string& path, int status, const std::string& body = "") {
        std::lock_guard<std::mutex> l(_m);
        _post[path] = { status, body };
    }

    std::string Address() const { return _srv.Address(); }
    bool ok() const { return _srv.ok(); }

    std::vector<FakeRequest> Requests() const { return _srv.Requests(); }
    std::vector<FakeRequest> Sent(const std::string& method, const std::string& path) const {
        std::vector<FakeRequest> out;
        for (auto& r : _srv.Requests()) {
            if (r.method == method && r.path == path) {
                out.push_back(r);
            }
        }
        return out;
    }
    int Count(const std::string& method, const std::string& path) const { return (int)Sent(method, path).size(); }
    // The body of the last request of that method to that path, parsed as JSON.
    nlohmann::json LastJson(const std::string& method, const std::string& path) const {
        auto s = Sent(method, path);
        REQUIRE_FALSE(s.empty());
        return nlohmann::json::parse(s.back().body);
    }
    bool Has(const std::string& method, const std::string& path) const { return Count(method, path) > 0; }
    // Every request received, one "METHOD path (body bytes)" per line, for failure messages.
    std::string All() const {
        std::string out;
        for (auto& r : _srv.Requests()) {
            out += r.method + " " + r.path + " (" + std::to_string(r.body.size()) + ")\n";
        }
        return out;
    }

private:
    FakeResponse Handle(const FakeRequest& r) {
        std::lock_guard<std::mutex> l(_m);
        FakeResponse resp;
        if (r.method == "GET") {
            auto it = _get.find(r.path);
            if (it == _get.end()) {
                resp.status = 404;
            } else {
                resp.status = it->second.first;
                resp.body = it->second.second;
            }
        } else {
            auto it = _post.find(r.path);
            if (it != _post.end()) {
                resp.status = it->second.first;
                resp.body = it->second.second;
            }
        }
        return resp;
    }

    std::mutex _m;
    std::map<std::string, std::pair<int, std::string>> _get;
    std::map<std::string, std::pair<int, std::string>> _post;
    FakeHttpServer _srv;
};

// Records what the handler tells or asks the user; answers every question with `answer`.
struct RecordingUi : UICallbacks {
    bool answer = true;
    mutable std::vector<std::string> messages;
    mutable std::vector<std::string> prompts;
    void ShowMessage(const std::string& m, const std::string&) const override { messages.push_back(m); }
    bool PromptYesNo(const std::string& m, const std::string&) const override {
        prompts.push_back(m);
        return answer;
    }
    std::string PromptForDirectory(const std::string&, const std::string&) const override { return ""; }
    std::string PromptForFile(const std::string&, const std::string&, const std::string&) const override { return ""; }
    long PromptForNumber(const std::string&, const std::string&, long d, long, long) const override { return d; }
    std::string PromptForText(const std::string&, const std::string&, const std::string& d) const override { return d; }
    ProgressToken BeginProgress(const std::string&, int) override { return 1; }
    void UpdateProgress(ProgressToken, int, const std::string&) override {}
    void EndProgress(ProgressToken) override {}
};

// Runs the shared CurlManager until every queued transfer, and whatever its callback
// queues in turn, has finished.
inline void DrainCurls() {
    for (int i = 0; i < 2000 && CurlManager::INSTANCE.processCurls(CurlManager::PUMP_WAIT_MS); i++) {
    }
}

// FPP capabilities for the tests.  The variants mirror the shapes of the real FPP file
// (a Beagle string cape, a shift-string cape with USB adapters, a Pi hat, a panel/virtual
// matrix only box) without depending on the shipped resources.
inline const char* kFppCaps = R"(<Vendor Name="FPP">
    <AbstractVariant Name="Base">
        <SupportsUpload/>
        <SupportsFullxLightsControl/>
        <SupportsDefaultBrightness/>
        <SupportsDefaultGamma/>
        <SupportsAutoLayout/>
        <MaxInputUniverses>1000</MaxInputUniverses>
        <InputProtocols><Protocol>e131</Protocol><Protocol>artnet</Protocol><Protocol>ddp</Protocol></InputProtocols>
        <SupportsMultipleSimultaneousInputProtocols/>
        <ConfigDriver>FPP</ConfigDriver>
    </AbstractVariant>
    <AbstractVariant Name="Strings" Base="FPP:Base">
        <MaxSerialPortChannels>512</MaxSerialPortChannels>
        <MaxPixelPortChannels>4800</MaxPixelPortChannels>
        <SupportsVirtualStrings/>
        <SupportsRemotes/>
        <SupportsSmartRemotes>6</SupportsSmartRemotes>
        <SupportsPixelPortEndNullPixels/>
        <SmartRemoteTypes><Type>falcon_v1</Type><Type>fpp_v2</Type><Type>falcon_v4</Type><Type>falcon_v5</Type></SmartRemoteTypes>
    </AbstractVariant>
    <Controller Name="TestCape">
        <Variant Name="" ID="TestCape" Base="FPP:Strings">
            <MaxPixelPort>8</MaxPixelPort>
            <MaxSerialPort>2</MaxSerialPort>
            <PixelProtocols><Protocol>ws2811</Protocol><Protocol>tm1814</Protocol><Protocol>ucs8904</Protocol><Protocol>apa102</Protocol></PixelProtocols>
            <SerialProtocols><Protocol>dmx</Protocol><Protocol>pixelnet</Protocol></SerialProtocols>
            <fppStringFileName>co-bbbStrings</fppStringFileName>
            <fpp>1</fpp>
            <fpp1>5,4</fpp1>
        </Variant>
    </Controller>
    <Controller Name="TestShift">
        <Variant Name="" ID="TestShift" Base="FPP:Strings">
            <MaxPixelPort>4</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <SupportsExtraSerialPorts/>
            <SupportsMultipleSimultaneousOutputProtocols/>
            <PixelProtocols><Protocol>ws2811</Protocol><Protocol>ws2811 slow</Protocol><Protocol>tm1814</Protocol><Protocol>ucs8904</Protocol></PixelProtocols>
            <SerialProtocols><Protocol>DMX-Open</Protocol><Protocol>LOR Enhanced</Protocol><Protocol>Renard</Protocol></SerialProtocols>
            <fppStringFileName>co-bbbStrings</fppStringFileName>
            <fppStringDriverType>BBShiftString</fppStringDriverType>
            <fpp>1</fpp>
            <fpp1>5,0</fpp1>
        </Variant>
    </Controller>
    <Controller Name="TestHat">
        <Variant Name="" ID="TestHat" Base="FPP:Strings">
            <MaxPixelPort>2</MaxPixelPort>
            <MaxSerialPort>1</MaxSerialPort>
            <MaxPWMPort>3</MaxPWMPort>
            <MaxPacing>40</MaxPacing>
            <SupportsVirtualMatrix/>
            <SupportsLEDPanelMatrix/>
            <PixelProtocols><Protocol>ws2811</Protocol></PixelProtocols>
            <SerialProtocols><Protocol>dmx</Protocol></SerialProtocols>
            <fppSerialPort1>ttyAMA0</fppSerialPort1>
            <fppStringFileName>co-pixelStrings</fppStringFileName>
            <fppStringDriverType>RPIWS281X</fppStringDriverType>
        </Variant>
    </Controller>
    <Controller Name="TestPanels">
        <Variant Name="" ID="TestPanels" Base="FPP:Base">
            <MaxPixelPort>0</MaxPixelPort>
            <MaxSerialPort>0</MaxSerialPort>
            <SupportsLEDPanelMatrix/>
            <SupportsVirtualMatrix/>
        </Variant>
    </Controller>
    <Controller Name="TestSerial">
        <Variant Name="" ID="TestSerial">
            <MaxPixelPort>0</MaxPixelPort>
            <MaxSerialPort>1</MaxSerialPort>
            <MaxSerialPortChannels>4096</MaxSerialPortChannels>
            <SerialProtocols><Protocol>DMX</Protocol><Protocol>Renard</Protocol><Protocol>LOR</Protocol><Protocol>Pixelnet</Protocol><Protocol>GenericSerial</Protocol></SerialProtocols>
            <ConfigDriver>FPP</ConfigDriver>
        </Variant>
    </Controller>
</Vendor>)";

// One controller "Ctl" on a big DDP output, with FPP capabilities loaded, and helpers
// to put models on its ports.
struct FppShow {
    VendorCapsFixture caps;
    OutputManager om;
    ModelManager mm{ &om };
    ControllerEthernet* ctl = nullptr;

    explicit FppShow(const std::string& model = "TestCape", const std::string& protocol = OUTPUT_DDP, int channels = 6000) :
        caps(kFppCaps) {
        InitLoggers();
        ctl = new ControllerEthernet(&om, false);
        ctl->SetName("Ctl");
        ctl->SetIP("10.0.0.5");
        ctl->SetProtocol(protocol);
        ctl->SetVendor("FPP");
        ctl->SetModel(model);
        ctl->SetVariant("");
        ctl->SetAutoLayout(false);
        // a new controller defaults to full control; most tests want the controller's
        // own config merged, and switch this on where they test the difference
        ctl->SetFullxLightsControl(false);
        if (protocol == OUTPUT_DDP) {
            ctl->GetOutputs().front()->SetChannels(channels);
        }
        om.AddController(ctl);
        om.SomethingChanged();
    }

    ControllerCaps* Caps() const { return ControllerCaps::GetControllerConfig(ctl); }

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }

    // `nodes` RGB nodes in `strings` strings on a controller port.  The start channel
    // is a controller-relative one when given, otherwise the model chains on `chain`.
    Model* OnPort(const std::string& name, const std::string& protocol, int port, int nodes = 10, const std::string& startChannel = "",
                  const std::string& extraCC = "", const std::string& chain = "", int strings = 1, const std::string& stringType = "RGB Nodes",
                  const std::string& extraModel = "") {
        std::string sc = startChannel.empty() ? "!Ctl:1" : startChannel;
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" parm1=\"" + std::to_string(strings) + "\" parm2=\"" + std::to_string(nodes) +
                   "\" parm3=\"1\" StringType=\"" + stringType + "\" StartChannel=\"" + sc + "\" Controller=\"Ctl\" ModelChain=\"" + chain + "\" " + extraModel +
                   "><ControllerConnection Protocol=\"" + protocol + "\" Port=\"" + std::to_string(port) + "\" " + extraCC + "/></model>");
    }

    void Recalc() {
        om.SomethingChanged();
        mm.RecalcStartChannels();
    }
    void Layout() {
        mm.ReworkStartChannel();
        Recalc();
    }
};

// An FPP handler aimed at a loopback server, with the version and pixel cape the tests
// usually want.
inline std::unique_ptr<FPP> MakeFpp(const FppServer& srv, uint32_t major = 9, uint32_t minor = 0) {
    InitLoggers();
    auto f = std::make_unique<FPP>(srv.Address(), "", "");
    f->ipAddress = srv.Address();
    f->majorVersion = major;
    f->minorVersion = minor;
    f->sysInfoLoaded = true;
    return f;
}

} // namespace fpp3

#endif
