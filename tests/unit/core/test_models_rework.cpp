#include <string>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/spdlog.h>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"

// ModelManager::ReworkStartChannel: the auto layout pass that gives every model
// on a controller port a "!Controller:channel" start channel, following the
// model chains, and places "No Controller" models after everything else.
namespace {

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };
    ControllerEthernet* ctl = nullptr;

    // One E1.31 controller "Ctl" with one 510 channel universe.
    explicit ShowFixture(bool autoSize = false) {
        // The rework logs to the "work" logger, which the apps register at startup.
        if (!spdlog::get("work")) {
            spdlog::register_logger(std::make_shared<spdlog::logger>("work", std::make_shared<spdlog::sinks::null_sink_mt>()));
        }
        ctl = new ControllerEthernet(&om);
        ctl->SetName("Ctl");
        ctl->SetIP("10.0.0.5");
        if (!autoSize) {
            ctl->SetAutoLayout(false);
        }
        om.AddController(ctl);
        om.SomethingChanged();
    }

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }

    // A single line of `nodes` RGB nodes (3 channels each) on a controller port.
    Model* OnPort(const std::string& name, const std::string& protocol, int port, const std::string& chain = "", int nodes = 10,
                  const std::string& extraCC = "", const std::string& controller = "Ctl") {
        return Add("<model name=\"" + name + "\" DisplayAs=\"Single Line\" parm1=\"1\" parm2=\"" + std::to_string(nodes) +
                   "\" parm3=\"1\" StringType=\"RGB Nodes\" StartChannel=\"1\" Controller=\"" + controller + "\" ModelChain=\"" + chain +
                   "\"><ControllerConnection Protocol=\"" + protocol + "\" Port=\"" + std::to_string(port) + "\" " + extraCC + "/></model>");
    }

    Model* Dmx(const std::string& name, int port, int dmxChannel, const std::string& chain = "", int nodes = 2) {
        return OnPort(name, "DMX", port, chain, nodes, "channel=\"" + std::to_string(dmxChannel) + "\"");
    }

    bool Rework() {
        bool changed = mm.ReworkStartChannel();
        mm.RecalcStartChannels();
        return changed;
    }
};

} // namespace

TEST_CASE("Models chained on a pixel port follow one another", "[models][rework]") {
    ShowFixture f;
    Model* a = f.OnPort("A", "ws2811", 1);
    Model* b = f.OnPort("B", "ws2811", 1, ">A");
    Model* c = f.OnPort("C", "ws2811", 1, ">B");
    CHECK(f.Rework());
    CHECK(a->GetModelStartChannel() == "!Ctl:1");
    CHECK(b->GetModelStartChannel() == "!Ctl:31");
    CHECK(c->GetModelStartChannel() == "!Ctl:61");
    CHECK(a->GetFirstChannel() == 0);
    CHECK(c->GetFirstChannel() == 60);
}

TEST_CASE("Chain order wins over the order models were added", "[models][rework]") {
    ShowFixture f;
    Model* z = f.OnPort("Z", "ws2811", 1, ">Y");
    Model* y = f.OnPort("Y", "ws2811", 1, ">X");
    Model* x = f.OnPort("X", "ws2811", 1);
    f.Rework();
    CHECK(x->GetModelStartChannel() == "!Ctl:1");
    CHECK(y->GetModelStartChannel() == "!Ctl:31");
    CHECK(z->GetModelStartChannel() == "!Ctl:61");
}

TEST_CASE("Ports follow one another in port order", "[models][rework]") {
    ShowFixture f;
    Model* p2 = f.OnPort("P2", "ws2811", 2, "", 5);
    Model* p1 = f.OnPort("P1", "ws2811", 1, "", 10);
    Model* p10 = f.OnPort("P10", "ws2811", 10, "", 1);
    f.Rework();
    CHECK(p1->GetModelStartChannel() == "!Ctl:1");
    CHECK(p2->GetModelStartChannel() == "!Ctl:31");
    CHECK(p10->GetModelStartChannel() == "!Ctl:46");
}

TEST_CASE("A second pass over an already laid out controller changes nothing", "[models][rework]") {
    ShowFixture f;
    f.OnPort("A", "ws2811", 1);
    f.OnPort("B", "ws2811", 1, ">A");
    f.OnPort("C", "ws2811", 2);
    CHECK(f.Rework());
    CHECK_FALSE(f.Rework());
}

TEST_CASE("A chain to a model on another port is reset to the beginning", "[models][rework]") {
    ShowFixture f;
    Model* a = f.OnPort("A", "ws2811", 1);
    Model* b = f.OnPort("B", "ws2811", 2, ">A");
    f.Rework();
    CHECK(b->GetModelChain().empty());
    CHECK(a->GetModelStartChannel() == "!Ctl:1");
    CHECK(b->GetModelStartChannel() == "!Ctl:31");
}

TEST_CASE("A chain to a model that does not exist is reset to the beginning", "[models][rework]") {
    ShowFixture f;
    Model* a = f.OnPort("A", "ws2811", 1, ">Gone");
    CHECK(f.Rework());
    CHECK(a->GetModelChain().empty());
    CHECK(a->GetModelStartChannel() == "!Ctl:1");
}

TEST_CASE("A port where every model is chained gets its first model as the beginning", "[models][rework]") {
    ShowFixture f;
    Model* a = f.OnPort("A", "ws2811", 1, ">B");
    Model* b = f.OnPort("B", "ws2811", 1, ">A");
    f.Rework();
    CHECK(a->GetModelChain().empty());
    CHECK(b->GetModelChain() == ">A");
    CHECK(a->GetModelStartChannel() == "!Ctl:1");
    CHECK(b->GetModelStartChannel() == "!Ctl:31");
}

TEST_CASE("Two models at the beginning of a port share its first channel", "[models][rework]") {
    // Left for the user to sort out rather than guessing a chain.
    ShowFixture f;
    Model* a = f.OnPort("A", "ws2811", 1, "", 10);
    Model* b = f.OnPort("B", "ws2811", 1, "", 4);
    Model* c = f.OnPort("C", "ws2811", 2, "", 1);
    f.Rework();
    CHECK(a->GetModelStartChannel() == "!Ctl:1");
    CHECK(b->GetModelStartChannel() == "!Ctl:1");
    // The next port starts after the longer of the two.
    CHECK(c->GetModelStartChannel() == "!Ctl:31");
}

TEST_CASE("Smart remotes on a port are laid out in remote order", "[models][rework]") {
    ShowFixture f;
    Model* rb = f.OnPort("RB", "ws2811", 1, "", 10, "SmartRemote=\"2\"");
    Model* ra = f.OnPort("RA", "ws2811", 1, "", 10, "SmartRemote=\"1\"");
    f.Rework();
    CHECK(ra->GetModelStartChannel() == "!Ctl:1");
    CHECK(rb->GetModelStartChannel() == "!Ctl:31");
}

TEST_CASE("Models with no port or protocol are left where they are", "[models][rework]") {
    ShowFixture f;
    Model* a = f.OnPort("A", "ws2811", 1);
    Model* loose = f.Add(R"(<model name="Loose" DisplayAs="Single Line" parm1="1" parm2="10" parm3="1" StringType="RGB Nodes" StartChannel="400" Controller="Ctl"/>)");
    f.Rework();
    CHECK(a->GetModelStartChannel() == "!Ctl:1");
    CHECK(loose->GetModelStartChannel() == "400");
}

TEST_CASE("DMX models are placed at their DMX channel within the port", "[models][rework]") {
    ShowFixture f;
    Model* d1 = f.Dmx("D1", 1, 1);
    Model* d10 = f.Dmx("D10", 1, 10);
    f.Rework();
    CHECK(d1->GetModelStartChannel() == "!Ctl:1");
    CHECK(d10->GetModelStartChannel() == "!Ctl:10");
}

TEST_CASE("A chained DMX model is still placed at its own DMX channel", "[models][rework]") {
    // The controller dialog gives every model on a serial port an explicit DMX
    // channel rather than a chain; a model's DMX channel cannot be 0.
    ShowFixture f;
    Model* d1 = f.Dmx("D1", 1, 1, "", 2);
    Model* d2 = f.Dmx("D2", 1, 20, ">D1", 2);
    Model* zero = f.Dmx("Zero", 2, 0, "", 2);
    CHECK(zero->GetControllerDMXChannel() == 1);
    f.Rework();
    CHECK(d1->GetModelStartChannel() == "!Ctl:1");
    CHECK(d2->GetModelStartChannel() == "!Ctl:20");
}

TEST_CASE("Without controller rules serial ports sort ahead of pixel ports", "[models][rework]") {
    // The port keys sort by protocol name; only a controller whose rules put
    // DMX after the pixels moves the serial ports to the end.
    ShowFixture f;
    Model* pix = f.OnPort("Pix", "ws2811", 1, "", 10);
    Model* dmx = f.Dmx("Dmx", 1, 1, "", 2);
    f.Rework();
    CHECK(dmx->GetModelStartChannel() == "!Ctl:1");
    CHECK(pix->GetModelStartChannel() == "!Ctl:7");
}

TEST_CASE("No Controller models are placed after every controller, in name order", "[models][rework]") {
    ShowFixture f;
    f.OnPort("A", "ws2811", 1, "", 10);
    Model* nb = f.OnPort("NB", "ws2811", 0, "", 2, "", "No Controller");
    Model* na = f.OnPort("NA", "ws2811", 0, "", 2, "", "No Controller");
    f.Rework();
    // The controller ends at channel 510 (one universe).
    CHECK(na->GetModelStartChannel() == "511");
    CHECK(nb->GetModelStartChannel() == "517");
    CHECK(na->GetFirstChannel() == 510);
    CHECK(nb->GetFirstChannel() == 516);
}

TEST_CASE("An auto sized controller grows to hold its models", "[models][rework]") {
    ShowFixture f(true);
    REQUIRE(f.ctl->IsAutoSize());
    f.OnPort("A", "ws2811", 1, "", 170);
    f.OnPort("B", "ws2811", 2, "", 170);
    f.OnPort("C", "ws2811", 3, "", 10);
    f.Rework();
    // 510 + 510 + 30 channels in whole 510 channel universes
    CHECK(f.ctl->GetOutputCount() == 3);
    CHECK(f.ctl->GetChannels() == 1530);
    CHECK_FALSE(f.Rework());
}

TEST_CASE("An auto sized controller shrinks when its models do", "[models][rework]") {
    ShowFixture f(true);
    Model* a = f.OnPort("A", "ws2811", 1, "", 400);
    f.Rework();
    CHECK(f.ctl->GetOutputCount() == 3);
    f.mm.Delete(a->GetName());
    f.OnPort("Small", "ws2811", 1, "", 10);
    f.Rework();
    CHECK(f.ctl->GetOutputCount() == 1);
    CHECK(f.ctl->GetChannels() == 510);
}
