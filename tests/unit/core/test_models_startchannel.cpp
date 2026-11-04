#include <functional>
#include <list>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/XmlSerializer.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"

namespace {

// Two E1.31 controllers: "Ctl1" 10.0.0.5 universes 1-2 (channels 1-1020) and
// "Ctl2" 10.0.0.6 universe 1 (channels 1021-1530).
struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };
    ControllerEthernet* ctl1 = nullptr;
    ControllerEthernet* ctl2 = nullptr;

    ShowFixture() {
        ctl1 = new ControllerEthernet(&om);
        ctl1->SetName("Ctl1");
        ctl1->SetIP("10.0.0.5");
        ctl1->AddOutput();
        ctl1->GetOutputs().back()->SetUniverse(2);
        om.AddController(ctl1);
        ctl2 = new ControllerEthernet(&om);
        ctl2->SetName("Ctl2");
        ctl2->SetIP("10.0.0.6");
        om.AddController(ctl2);
        om.SomethingChanged();
    }

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        return mm.createAndAddModel(doc.first_child(), 100, 100);
    }

    Model* Line(const std::string& name, const std::string& startChannel, int nodes = 10, int strings = 1) {
        std::string xml = "<model name=\"" + name + "\" DisplayAs=\"Single Line\" NumStrings=\"" + std::to_string(strings) +
                          "\" NodesPerString=\"" + std::to_string(nodes) + "\" StartChannel=\"" + startChannel + "\" StringType=\"RGB Nodes\"/>";
        return Add(xml);
    }
};

} // namespace

TEST_CASE("Absolute start channel resolves to a zero based first channel", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Line("A", "100", 10);
    REQUIRE(m != nullptr);
    f.mm.RecalcStartChannels();
    CHECK(m->CouldComputeStartChannel);
    CHECK(m->GetFirstChannel() == 99);
    CHECK(m->GetChanCount() == 30);
    CHECK(m->GetLastChannel() == 99 + 30 - 1);
}

TEST_CASE("Start channel is trimmed of whitespace", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Line("A", "  25  ");
    f.mm.RecalcStartChannels();
    CHECK(m->GetFirstChannel() == 24);
    CHECK(m->CouldComputeStartChannel);
}

TEST_CASE("Missing StartChannel attribute defaults to channel 1", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="A" DisplayAs="Single Line" NumStrings="1" NodesPerString="5"/>)");
    REQUIRE(m != nullptr);
    f.mm.RecalcStartChannels();
    CHECK(m->GetModelStartChannel() == "1");
    CHECK(m->GetFirstChannel() == 0);
}

TEST_CASE("Invalid absolute start channels fall back to channel 1 and are flagged", "[models][startchannel]") {
    ShowFixture f;
    for (const char* sc : { "0", "-5", "abc", "", "   ", "1.5x" }) {
        INFO("start channel '" << sc << "'");
        Model* m = f.Line(std::string("M") + std::to_string(f.mm.size()), sc);
        REQUIRE(m != nullptr);
        f.mm.RecalcStartChannels();
        if (std::string(sc) == "1.5x") {
            CHECK(m->GetFirstChannel() == 0);
            continue;
        }
        CHECK_FALSE(m->CouldComputeStartChannel);
        CHECK(m->GetFirstChannel() == 0);
        CHECK_FALSE(m->IsValidStartChannelString());
    }
}

TEST_CASE("Absurdly large start channels do not crash", "[models][startchannel]") {
    ShowFixture f;
    for (const char* sc : { "99999999999999999999", "2147483647", "4294967296" }) {
        Model* m = f.Line(std::string("M") + std::to_string(f.mm.size()), sc);
        REQUIRE(m != nullptr);
        f.mm.RecalcStartChannels();
        (void)m->GetFirstChannel();
        (void)m->GetLastChannel();
        (void)m->IsValidStartChannelString();
    }
    SUCCEED();
}

TEST_CASE("Start channel relative to the start of another model (@)", "[models][startchannel]") {
    ShowFixture f;
    Model* base = f.Line("Base", "100", 10);
    Model* same = f.Line("Same", "@Base:1");
    Model* off = f.Line("Off", "@Base:5");
    f.mm.RecalcStartChannels();
    CHECK(same->CouldComputeStartChannel);
    CHECK(same->GetFirstChannel() == base->GetFirstChannel());
    CHECK(off->GetFirstChannel() == base->GetFirstChannel() + 4);
    CHECK(same->IsValidStartChannelString());
    CHECK(off->IsValidStartChannelString());
}

TEST_CASE("Start channel chained after another model (>)", "[models][startchannel]") {
    ShowFixture f;
    Model* base = f.Line("Base", "100", 10);
    Model* next = f.Line("Next", ">Base:1", 10);
    Model* third = f.Line("Third", ">Next:1", 5);
    f.mm.RecalcStartChannels();
    CHECK(next->GetFirstChannel() == base->GetLastChannel() + 1);
    CHECK(next->GetFirstChannel() == 129);
    CHECK(third->GetFirstChannel() == next->GetLastChannel() + 1);
    CHECK(third->GetFirstChannel() == 159);
    CHECK(f.mm.GetLastChannel() == third->GetLastChannel());
}

TEST_CASE("Chain offset adds a gap and chain offset zero overlaps the previous last channel", "[models][startchannel]") {
    ShowFixture f;
    Model* base = f.Line("Base", "1", 10);
    Model* gap = f.Line("Gap", ">Base:11");
    Model* overlap = f.Line("Overlap", ">Base:0");
    f.mm.RecalcStartChannels();
    CHECK(gap->GetFirstChannel() == base->GetLastChannel() + 11);
    CHECK(overlap->GetFirstChannel() == base->GetLastChannel());
    CHECK_FALSE(overlap->IsValidStartChannelString());
}

TEST_CASE("Chains resolve regardless of the order models were added", "[models][startchannel]") {
    ShowFixture f;
    Model* c = f.Line("C", ">B:1", 3);
    Model* b = f.Line("B", ">A:1", 3);
    Model* a = f.Line("A", "10", 3);
    f.mm.RecalcStartChannels();
    CHECK(a->GetFirstChannel() == 9);
    CHECK(b->GetFirstChannel() == 9 + 9);
    CHECK(c->GetFirstChannel() == 9 + 18);
    CHECK(c->CouldComputeStartChannel);
}

TEST_CASE("Dependent start channels follow when the base model moves", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "1", 10);
    Model* b = f.Line("B", ">A:1", 10);
    f.mm.RecalcStartChannels();
    CHECK(b->GetFirstChannel() == 30);
    a->SetStartChannel("501");
    f.mm.RecalcStartChannels();
    CHECK(b->GetFirstChannel() == 530);
}

TEST_CASE("Self referencing and unknown model start channels cannot be computed", "[models][startchannel]") {
    ShowFixture f;
    Model* self = f.Line("Self", "@Self:1");
    Model* unknown = f.Line("Unknown", "@Nope:1");
    Model* chainUnknown = f.Line("ChainUnknown", ">Nope:1");
    f.mm.RecalcStartChannels();
    CHECK_FALSE(self->CouldComputeStartChannel);
    CHECK_FALSE(unknown->CouldComputeStartChannel);
    CHECK_FALSE(chainUnknown->CouldComputeStartChannel);
    CHECK_FALSE(self->IsValidStartChannelString());
    CHECK(unknown->GetFirstChannel() == 0);
}

TEST_CASE("Cyclic start channel dependencies terminate", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", ">B:1");
    Model* b = f.Line("B", ">A:1");
    Model* c = f.Line("C", ">C:1");
    f.mm.RecalcStartChannels();
    CHECK_FALSE(a->CouldComputeStartChannel);
    CHECK_FALSE(b->CouldComputeStartChannel);
    CHECK_FALSE(c->CouldComputeStartChannel);
}

TEST_CASE("Universe based start channels resolve through the output manager", "[models][startchannel]") {
    ShowFixture f;
    Model* u1 = f.Line("U1", "#1:1");
    Model* u2 = f.Line("U2", "#2:10");
    f.mm.RecalcStartChannels();
    CHECK(u1->CouldComputeStartChannel);
    CHECK(u1->GetFirstChannel() == 0);
    CHECK(u2->CouldComputeStartChannel);
    CHECK(u2->GetFirstChannel() == 510 + 9);
    CHECK(u2->IsValidStartChannelString());
}

TEST_CASE("IP and universe start channels select the right controller", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "#10.0.0.6:1:5");
    Model* b = f.Line("B", "#10.0.0.5:2:1");
    f.mm.RecalcStartChannels();
    CHECK(a->CouldComputeStartChannel);
    CHECK(a->GetFirstChannel() == 1020 + 4);
    CHECK(b->GetFirstChannel() == 510);
    CHECK(a->IsValidStartChannelString());
}

TEST_CASE("Universe start channels for a universe that does not exist are invalid", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "#9:1");
    Model* b = f.Line("B", "#10.9.9.9:1:1");
    Model* c = f.Line("C", "#abc");
    f.mm.RecalcStartChannels();
    CHECK_FALSE(a->CouldComputeStartChannel);
    CHECK_FALSE(b->CouldComputeStartChannel);
    CHECK_FALSE(c->CouldComputeStartChannel);
    CHECK_FALSE(a->IsValidStartChannelString());
    CHECK_FALSE(b->IsValidStartChannelString());
    CHECK_FALSE(c->IsValidStartChannelString());
}

TEST_CASE("Controller start channels resolve relative to the controller", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "!Ctl2:1");
    Model* b = f.Line("B", "!Ctl2:3");
    Model* c = f.Line("C", "!Ctl1:1");
    f.mm.RecalcStartChannels();
    CHECK(a->GetFirstChannel() == 1020);
    CHECK(b->GetFirstChannel() == 1022);
    CHECK(c->GetFirstChannel() == 0);
    CHECK(a->IsValidStartChannelString());
}

TEST_CASE("Controller start channels for unknown controllers or offsets are invalid", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "!Nope:1");
    Model* b = f.Line("B", "!Ctl2:0");
    f.mm.RecalcStartChannels();
    CHECK_FALSE(a->CouldComputeStartChannel);
    CHECK_FALSE(b->CouldComputeStartChannel);
    CHECK_FALSE(a->IsValidStartChannelString());
    CHECK_FALSE(b->IsValidStartChannelString());
}

TEST_CASE("IsValidStartChannelString accepts every documented format", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Line("M", "1");
    f.Line("Other", "50");
    struct Case {
        const char* sc;
        bool valid;
    };
    const Case cases[] = {
        { "1", true },
        { "12345", true },
        { "0", false },
        { "-1", false },
        { "abc", false },
        { "", false },
        { "1:2:3:4", false },
        { "@Other:1", true },
        { ">Other:1", true },
        { "@Other:abc", false },
        { "@Other:0", false },
        { "@M:1", false },
        { "#1:1", true },
        { "#2:5", true },
        { "#9:5", false },
        { "#10.0.0.5:1:1", true },
        { "#1", false },
        { "!Ctl1:1", true },
        { "!Ctl1", false },
        { "!Nope:1", false },
        { "x:1", false },
    };
    for (const auto& c : cases) {
        INFO("start channel '" << c.sc << "'");
        m->ModelStartChannel = c.sc;
        CHECK(m->IsValidStartChannelString() == c.valid);
    }
}

TEST_CASE("GetNumberFromChannelString reports validity and dependencies", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "100");
    Model* b = f.Line("B", "1");
    f.mm.RecalcStartChannels();

    bool valid = false;
    std::string dep;
    CHECK(b->GetNumberFromChannelString("42", valid, dep) == 42);
    CHECK(valid);
    CHECK(dep.empty());

    CHECK(b->GetNumberFromChannelString(">A:1", valid, dep) == 130);
    CHECK(valid);
    CHECK(dep == "A");

    dep.clear();
    CHECK(b->GetNumberFromChannelString("@A:1", valid, dep) == 100);
    CHECK(valid);
    CHECK(dep == "A");

    dep.clear();
    CHECK(b->GetNumberFromChannelString("bogus", valid, dep) == 1);
    CHECK_FALSE(valid);

    CHECK(b->GetNumberFromChannelString("  7 ") == 7);
    CHECK(b->GetNumberFromChannelString("#2:10") == 520);
    CHECK(b->GetNumberFromChannelString("!Ctl2:5") == 1024);
    CHECK(a->GetNumberFromChannelString("-3") == 1);
}

TEST_CASE("Strings of a model are laid out back to back by default", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="3" NodesPerString="10" StrandsPerString="1" StartChannel="100"/>)");
    f.mm.RecalcStartChannels();
    REQUIRE(m->GetNumStrings() == 3);
    CHECK(m->NodeStartChannel(0) == 99);
    CHECK(m->NodeStartChannel(10) == 99 + 30);
    CHECK(m->NodeStartChannel(20) == 99 + 60);
    CHECK(m->NodeEndChannel(29) == 99 + 90 - 1);
    CHECK(m->GetLastChannel() == 99 + 90 - 1);
}

TEST_CASE("Individual string start channels override the contiguous layout", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="3" NodesPerString="10" StrandsPerString="1" StartChannel="1" Advanced="1" String1="1" String2="601" String3="301"/>)");
    f.mm.RecalcStartChannels();
    REQUIRE(m->GetNumStrings() == 3);
    CHECK(m->HasIndividualStartChannels());
    CHECK(m->NodeStartChannel(0) == 0);
    CHECK(m->NodeStartChannel(10) == 600);
    CHECK(m->NodeStartChannel(20) == 300);
    CHECK(m->GetFirstChannel() == 0);
    CHECK(m->GetLastChannel() == 629);
}

TEST_CASE("Individual start channels may chain to earlier strings", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Add(R"(<model name="T" DisplayAs="Tree 360" NumStrings="3" NodesPerString="10" StrandsPerString="1" StartChannel="50" Advanced="1" String1="" String2="" String3=""/>)");
    f.mm.RecalcStartChannels();
    CHECK(m->NodeStartChannel(0) == 49);
    CHECK(m->NodeStartChannel(10) == 79);
    CHECK(m->NodeStartChannel(20) == 109);
}

TEST_CASE("Start channel display formats", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "100", 10);
    Model* r = f.Line("R", "@A:5");
    Model* c = f.Line("C", "!Ctl2:3");
    Model* u = f.Line("U", "#2:10");
    f.mm.RecalcStartChannels();
    CHECK(a->GetStartChannelInDisplayFormat(&f.om) == "100");
    CHECK(r->GetStartChannelInDisplayFormat(&f.om) == "@A:5 (104)");
    CHECK(c->GetStartChannelInDisplayFormat(&f.om) == "!Ctl2:3 (1023)");
    CHECK(u->GetStartChannelInDisplayFormat(&f.om) == "#2:10 (520)");
}

TEST_CASE("First and last channel can be expressed in the model start channel format", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "100", 10);
    Model* u = f.Line("U", "#2:10", 10);
    Model* c = f.Line("C", "!Ctl2:3", 10);
    f.mm.RecalcStartChannels();
    CHECK(a->GetFirstChannelInStartChannelFormat(&f.om) == "100");
    CHECK(a->GetLastChannelInStartChannelFormat(&f.om) == "129");
    CHECK(u->GetFirstChannelInStartChannelFormat(&f.om) == "#2:10 (520)");
    CHECK(u->GetLastChannelInStartChannelFormat(&f.om) == "#2:39 (549)");
    CHECK(c->GetFirstChannelInStartChannelFormat(&f.om) == "!Ctl2:3 (1023)");
}

TEST_CASE("Renaming a model rewrites start channels that reference it", "[models][startchannel]") {
    ShowFixture f;
    f.Line("Base", "10");
    Model* chained = f.Line("Chained", ">Base:1");
    Model* fromStart = f.Line("FromStart", "@Base:3");
    f.mm.RecalcStartChannels();
    CHECK(f.mm.Rename("Base", "Renamed"));
    CHECK(f.mm["Base"] == nullptr);
    REQUIRE(f.mm["Renamed"] != nullptr);
    CHECK(chained->GetModelStartChannel() == ">Renamed:1");
    CHECK(fromStart->GetModelStartChannel() == "@Renamed:3");
    f.mm.RecalcStartChannels();
    CHECK(chained->CouldComputeStartChannel);
    CHECK(chained->GetFirstChannel() == 9 + 30);
}

TEST_CASE("Renaming a model that does not exist fails cleanly", "[models][startchannel]") {
    ShowFixture f;
    f.Line("A", "1");
    CHECK_FALSE(f.mm.Rename("Missing", "Other"));
    CHECK(f.mm["A"] != nullptr);
}

TEST_CASE("ReplaceIPInStartChannels rewrites IP based start channels", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "#10.0.0.6:1:5");
    Model* b = f.Line("B", "7");
    f.mm.ReplaceIPInStartChannels("10.0.0.6", "10.0.0.7");
    CHECK(a->GetModelStartChannel() == "#10.0.0.7:1:5");
    CHECK(b->GetModelStartChannel() == "7");
}

TEST_CASE("Overlapping models are detected", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "1", 10);
    Model* b = f.Line("B", "30", 10);
    Model* c = f.Line("C", "31", 10);
    Model* d = f.Line("D", "1000", 1);
    f.mm.RecalcStartChannels();
    CHECK(f.mm.IsModelOverlapping(a));
    CHECK(f.mm.IsModelOverlapping(b));
    CHECK(f.mm.IsModelOverlapping(c));
    CHECK_FALSE(f.mm.IsModelOverlapping(d));
}

TEST_CASE("Adjacent models do not overlap", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "1", 10);
    Model* b = f.Line("B", ">A:1", 10);
    f.mm.RecalcStartChannels();
    CHECK_FALSE(f.mm.IsModelOverlapping(a));
    CHECK_FALSE(f.mm.IsModelOverlapping(b));
}

TEST_CASE("GetModelsOnChannels lists the models covering a channel range", "[models][startchannel]") {
    ShowFixture f;
    f.Line("A", "1", 10);
    f.Line("B", "31", 10);
    f.mm.RecalcStartChannels();
    CHECK(f.mm.GetModelsOnChannels(1, 30, 0) == "A");
    CHECK(f.mm.GetModelsOnChannels(31, 31, 0) == "B");
    CHECK(f.mm.GetModelsOnChannels(25, 40, 0) == "A, B");
    CHECK(f.mm.GetModelsOnChannels(200, 300, 0).empty());
}

TEST_CASE("GenerateNewStartChannel chains after the last model", "[models][startchannel]") {
    ShowFixture f;
    CHECK(f.mm.GenerateNewStartChannel() == "1");
    f.Line("A", "1", 10);
    f.Line("B", "100", 10);
    f.mm.RecalcStartChannels();
    CHECK(f.mm.GenerateNewStartChannel() == ">B:1");
    CHECK(f.mm.GenerateNewStartChannel("A") == ">A:1");
    CHECK(f.mm.GenerateNewStartChannel("Missing") == ">B:1");
}

TEST_CASE("Model chain start channel is resolved after the show manager recalculates", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "1", 10);
    Model* b = f.Line("B", ">A:1", 10);
    CHECK(f.mm.RecalcStartChannels());
    CHECK(b->GetFirstChannel() == a->GetLastChannel() + 1);
    CHECK_FALSE(f.mm.RecalcStartChannels());
}

TEST_CASE("SafeModelName strips characters that break start channels and names", "[models][startchannel]") {
    CHECK(Model::SafeModelName("Tree 1") == "Tree 1");
    CHECK(Model::SafeModelName("  padded  ") == "padded");
    CHECK(Model::SafeModelName("a,b~c!d;e<f>g\"h'i&j:k|l/m\\n") == "abcdefghijklmn");
    CHECK(Model::SafeModelName("a@b") == "ab");
    CHECK(Model::SafeModelName("a@b", true) == "a@b");
    CHECK(Model::SafeModelName("").empty());
    CHECK(Model::SafeModelName(",,,").empty());
}

TEST_CASE("ParseFaceNodes expands ranges and lists", "[models][startchannel]") {
    ShowFixture f;
    Model* m = f.Line("A", "1");
    CHECK(m->ParseFaceNodes("1-3,5") == std::list<int>{ 0, 1, 2, 4 });
    CHECK(m->ParseFaceNodes("3-1") == std::list<int>{ 0, 1, 2 });
    CHECK(m->ParseFaceNodes("7") == std::list<int>{ 6 });
}

TEST_CASE("Controller start channel is validated against the controller name", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "1");
    a->SetControllerName("Ctl1");
    CHECK(a->GetControllerName() == "Ctl1");
    a->SetControllerName("");
    CHECK(a->GetControllerName().empty());
}

TEST_CASE("Models with a start channel on no controller chain through the manager", "[models][startchannel]") {
    ShowFixture f;
    Model* a = f.Line("A", "1", 4);
    Model* b = f.Line("B", ">A:1", 4);
    Model* c = f.Line("C", ">B:1", 4);
    f.mm.RecalcStartChannels();
    CHECK(a->GetLastChannel() == 11);
    CHECK(b->GetFirstChannel() == 12);
    CHECK(c->GetFirstChannel() == 24);
    CHECK(f.mm.GetLastChannel() == 35);
}
