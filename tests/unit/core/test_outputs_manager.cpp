#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <fstream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerNull.h"
#include "outputs/ControllerSerial.h"
#include "outputs/E131Output.h"
#include "outputs/IPOutput.h"
#include "outputs/NullOutput.h"
#include "outputs/OutputManager.h"
#include "outputs/TestPreset.h"
#include "support/TempDir.h"

namespace {

// Records what the manager sends so channel routing can be checked without any socket.
class RecordingOutput : public IPOutput {
public:
    explicit RecordingOutput(int32_t channels, int universe = 1) {
        _channels = channels;
        _universe = universe;
        data.assign(channels, 0);
    }
    Output* Copy() override { return new RecordingOutput(_channels, _universe); }
    std::string GetType() const override { return OUTPUT_OPC; }
    int GetMaxChannels() const override { return 512; }
    bool IsValidChannelCount(int32_t c) const override { return c > 0 && c <= 512; }
    std::string GetLongDescription() const override { return "rec"; }
    void EndFrame(int) override { ++endFrames; }
    void StartFrame(long msec) override {
        Output::StartFrame(msec);
        ++startFrames;
    }
    void SetOneChannel(int32_t ch, unsigned char v) override {
        if (ch >= 0 && ch < (int32_t)data.size()) data[ch] = v;
        ++oneCalls;
    }
    void SetManyChannels(int32_t ch, unsigned char* d, size_t n) override {
        for (size_t i = 0; i < n; ++i) data[ch + i] = d[i];
        ++manyCalls;
    }
    void AllOff() override {
        std::fill(data.begin(), data.end(), 0);
        ++allOffs;
    }
    bool Open() override {
        ++opens;
        return true;
    }
    void Close() override { ++closes; }
    std::string GlobalFPPProxy() const { return _globalFPPProxy; }
    std::string GlobalForceLocalIP() const { return _globalForceLocalIP; }

    std::vector<uint8_t> data;
    std::atomic<int> endFrames{ 0 };
    int startFrames = 0;
    int oneCalls = 0;
    int manyCalls = 0;
    int allOffs = 0;
    int opens = 0;
    int closes = 0;
};

std::string E131Controller(const std::string& name, const std::string& ip, int firstUniverse, int universes, int channels = 510, const char* active = "Active") {
    std::ostringstream x;
    x << "<Controller Id=\"" << 100 + firstUniverse << "\" Name=\"" << name << "\" Type=\"Ethernet\" Protocol=\"E131\" IP=\"" << ip << "\" ActiveState=\"" << active << "\">";
    for (int i = 0; i < universes; ++i) {
        x << "<network NetworkType=\"E131\" ComPort=\"" << ip << "\" BaudRate=\"" << firstUniverse + i << "\" MaxChannels=\"" << channels << "\"/>";
    }
    x << "</Controller>";
    return x.str();
}

std::string NullController(const std::string& name, int id, int channels) {
    std::ostringstream x;
    x << "<Controller Id=\"" << id << "\" Name=\"" << name << "\" Type=\"Null\"><network NetworkType=\"NULL\" Id=\"" << id << "\" MaxChannels=\"" << channels << "\"/></Controller>";
    return x.str();
}

Controller* Add(OutputManager& om, const std::string& xml, int pos = -1) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    Controller* c = Controller::Create(&om, doc.first_child(), "");
    REQUIRE(c != nullptr);
    om.AddController(c, pos);
    om.SomethingChanged();
    return c;
}

Controller* At(OutputManager& om, int index) {
    return om.GetControllerIndex(index);
}

std::string ToString(OutputManager& om) {
    pugi::xml_document doc;
    om.SaveToXML(doc);
    std::ostringstream ss;
    doc.print(ss, "", pugi::format_raw);
    return ss.str();
}

// a 3 controller show: A has 3 universes (1530 ch), B is a Null of 20, C has one universe of 100
void BuildShow(OutputManager& om) {
    Add(om, E131Controller("A", "10.0.0.1", 1, 3));
    Add(om, NullController("B", 50, 20));
    Add(om, E131Controller("C", "10.0.0.2", 1, 1, 100));
}

} // namespace

TEST_CASE("Empty OutputManager", "[outputs]") {
    OutputManager om;
    CHECK(om.GetControllerCount() == 0);
    CHECK(om.GetOutputCount() == 0);
    CHECK(om.GetTotalChannels() == 0);
    CHECK_FALSE(om.IsDirty());
    CHECK(om.GetController("x") == nullptr);
    CHECK(At(om, 0) == nullptr);
    CHECK(om.GetAllOutputs().empty());
    CHECK(om.GetControllerNames().empty());
    CHECK(om.GetIps().empty());
    CHECK_FALSE(om.IsOutputting());
}

TEST_CASE("OutputManager lays out start channels across controllers", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetControllerCount() == 3);
    CHECK(om.GetOutputCount() == 5);
    CHECK(om.GetTotalChannels() == 1530 + 20 + 100);

    auto* a = om.GetController("A");
    auto* b = om.GetController("B");
    auto* c = om.GetController("C");
    REQUIRE(a != nullptr);
    REQUIRE(b != nullptr);
    REQUIRE(c != nullptr);
    CHECK(a->GetStartChannel() == 1);
    CHECK(a->GetEndChannel() == 1530);
    CHECK(b->GetStartChannel() == 1531);
    CHECK(b->GetEndChannel() == 1550);
    CHECK(c->GetStartChannel() == 1551);
    CHECK(c->GetEndChannel() == 1650);
    CHECK(om.GetController("A")->GetOutput(2)->GetStartChannel() == 1021);
}

TEST_CASE("OutputManager finds controllers and outputs by absolute channel", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    int32_t rel = 0;
    CHECK(om.GetController(1, rel)->GetName() == "A");
    CHECK(rel == 1);
    CHECK(om.GetController(1530, rel)->GetName() == "A");
    CHECK(rel == 1530);
    CHECK(om.GetController(1531, rel)->GetName() == "B");
    CHECK(rel == 1);
    CHECK(om.GetController(1650, rel)->GetName() == "C");
    CHECK(rel == 100);
    CHECK(om.GetController(1651, rel) == nullptr);
    CHECK(om.GetController(0, rel) == nullptr);

    Output* o = om.GetOutput(600, rel);
    REQUIRE(o != nullptr);
    CHECK(o->GetUniverse() == 2);
    CHECK(rel == 90);
    CHECK(om.GetOutput(1540, rel)->GetType() == OUTPUT_NULL);
    CHECK(om.GetOutput(5000, rel) == nullptr);
}

TEST_CASE("OutputManager GetControllersInRange", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetControllersInRange(1, 1).size() == 1);
    CHECK(om.GetControllersInRange(1530, 1531).size() == 2);
    CHECK(om.GetControllersInRange(1, 5000).size() == 3);
    CHECK(om.GetControllersInRange(2000, 3000).empty());
    auto r = om.GetControllersInRange(1540, 1560);
    REQUIRE(r.size() == 2);
    CHECK(r[0]->GetName() == "B");
    CHECK(r[1]->GetName() == "C");
}

TEST_CASE("OutputManager controller ordering", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    Controller* a = om.GetController("A");
    Controller* b = om.GetController("B");
    Controller* c = om.GetController("C");
    CHECK(om.GetControllerIndex(a) == 0);
    CHECK(om.GetControllerIndex(b) == 1);
    CHECK(om.GetControllerIndex(c) == 2);
    CHECK(At(om, 1) == b);
    CHECK(At(om, 3) == nullptr);

    Controller* d = Add(om, NullController("D", 60, 5), 0);
    CHECK(om.GetControllerIndex(d) == 0);
    CHECK(d->GetStartChannel() == 1);
    CHECK(a->GetStartChannel() == 6);
}

TEST_CASE("OutputManager MoveController recomputes channels", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    Controller* c = om.GetController("C");
    om.MoveController(c, 0);
    CHECK(om.GetControllerIndex(c) == 0);
    CHECK(c->GetStartChannel() == 1);
    CHECK(om.GetController("A")->GetStartChannel() == 101);
    CHECK(om.GetTotalChannels() == 1650);

    om.MoveController(c, 99);
    CHECK(om.GetControllerIndex(c) == 2);
    CHECK(om.GetController("A")->GetStartChannel() == 1);
}

TEST_CASE("OutputManager AddController with position out of range appends", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    Controller* d = Add(om, NullController("D", 60, 5), 99);
    CHECK(om.GetControllerIndex(d) == 3);
    Controller* e = Add(om, NullController("E", 61, 5), -1);
    CHECK(om.GetControllerIndex(e) == 4);
}

TEST_CASE("OutputManager DeleteController", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    om.DeleteController("B");
    CHECK(om.GetControllerCount() == 2);
    CHECK(om.GetController("B") == nullptr);
    om.SomethingChanged();
    CHECK(om.GetController("C")->GetStartChannel() == 1531);
    om.DeleteController("nonexistent");
    CHECK(om.GetControllerCount() == 2);
    om.DeleteAllControllers();
    CHECK(om.GetControllerCount() == 0);
    CHECK(om.GetOutputCount() == 0);
}

TEST_CASE("OutputManager name lookups and uniqueness", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetController("  A  ") == om.GetController("A"));
    CHECK(om.UniqueName("Fresh") == "Fresh");
    CHECK(om.UniqueName("A") == "A_1");
    Add(om, NullController("A_1", 70, 1));
    CHECK(om.UniqueName("A") == "A_2");
    CHECK(om.UniqueName("A_1") == "A_2");
    CHECK(om.IsControllerNameUnique("A"));
    CHECK(om.IsControllerNameUnique("not there"));
}

TEST_CASE("OutputManager duplicate names are reported", "[outputs]") {
    OutputManager om;
    Add(om, NullController("Same", 1, 1));
    Add(om, NullController("Same", 2, 1));
    CHECK_FALSE(om.IsControllerNameUnique("Same"));
}

TEST_CASE("OutputManager unique ids skip used ids and output universes", "[outputs]") {
    OutputManager om;
    Add(om, NullController("one", 1, 1));
    Add(om, NullController("two", 2, 1));
    CHECK(om.IsIDUsed(1));
    CHECK(om.IsIDUsed(2));
    CHECK_FALSE(om.IsIDUsed(3));
    CHECK(om.IsIDUsed(1, nullptr));
    CHECK_FALSE(om.IsIDUsed(1, om.GetController("one")));
    CHECK(om.UniqueId() == 3);
    Add(om, E131Controller("E", "10.0.0.9", 3, 1));
    CHECK(om.UniqueId() == 4);
}

TEST_CASE("OutputManager name lists", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetControllerNames() == std::list<std::string>{ "A", "C" });
    CHECK(om.GetAutoLayoutControllerNames() == std::list<std::string>{ "A", "B", "C" });
    om.GetController("B")->SetAutoLayout(false);
    CHECK(om.GetAutoLayoutControllerNames() == std::list<std::string>{ "A", "C" });
}

TEST_CASE("OutputManager marks controllers sharing an ip as unmanaged", "[outputs]") {
    OutputManager om;
    auto* a = dynamic_cast<ControllerEthernet*>(Add(om, E131Controller("A", "10.0.0.1", 1, 1)));
    auto* b = dynamic_cast<ControllerEthernet*>(Add(om, E131Controller("B", "10.0.0.2", 1, 1)));
    CHECK(a->IsManaged());
    CHECK(b->IsManaged());
    auto* c = dynamic_cast<ControllerEthernet*>(Add(om, E131Controller("C", "10.0.0.1", 2, 1)));
    CHECK_FALSE(a->IsManaged());
    CHECK_FALSE(c->IsManaged());
    CHECK(b->IsManaged());
    om.DeleteController("C");
    CHECK(a->IsManaged());
}

TEST_CASE("OutputManager multicast controllers are never unmanaged", "[outputs]") {
    OutputManager om;
    auto* a = dynamic_cast<ControllerEthernet*>(Add(om, E131Controller("A", "MULTICAST", 1, 1)));
    auto* b = dynamic_cast<ControllerEthernet*>(Add(om, E131Controller("B", "MULTICAST", 2, 1)));
    CHECK(a->IsManaged());
    CHECK(b->IsManaged());
}

TEST_CASE("OutputManager lookups by ip", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetControllerWithIP("10.0.0.2")->GetName() == "C");
    CHECK(om.GetControllerWithIP("10.0.0.99") == nullptr);
    CHECK(om.GetControllers("10.0.0.1").size() == 1);
    CHECK(om.GetControllers("10.0.0.1", "").size() == 1);
    CHECK(om.GetControllers("10.9.9.9").empty());
    CHECK(om.GetIps() == std::list<std::string>{ "10.0.0.1", "10.0.0.2" });
    CHECK(om.GetControllerCount(CONTROLLER_ETHERNET, "10.0.0.1") == 1);
    CHECK(om.GetControllerCount(CONTROLLER_NULL, "") == 1);
    CHECK(om.GetControllerCount(CONTROLLER_ETHERNET, "10.0.0.99") == 0);
}

TEST_CASE("OutputManager GetAllOutputs filtering", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetAllOutputs().size() == 5);
    CHECK(om.GetOutputs().size() == 5);
    CHECK(om.GetAllOutputs("10.0.0.1").size() == 3);
    CHECK(om.GetAllOutputs("10.0.0.2").size() == 1);
    CHECK(om.GetAllOutputs("10.0.0.77").empty());
    CHECK(om.GetAllOutputs("").size() == 5);
    auto all = om.GetAllOutputs();
    CHECK(all.front()->GetUniverse() == 1);
    CHECK(all[2]->GetUniverse() == 3);
    CHECK(all[3]->GetType() == OUTPUT_NULL);
}

TEST_CASE("OutputManager GetOutput by universe and ip", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    Output* o = om.GetOutput(2, "10.0.0.1");
    REQUIRE(o != nullptr);
    CHECK(o->GetUniverse() == 2);
    CHECK(o->GetIP() == "10.0.0.1");
    CHECK(om.GetOutput(2, "10.0.0.2") == nullptr);
    CHECK(om.GetOutput(1, "10.0.0.2")->GetIP() == "10.0.0.2");
    CHECK(om.GetOutput(50, "")->GetType() == OUTPUT_NULL);
    CHECK(om.GetOutput(9999, "") == nullptr);
}

TEST_CASE("OutputManager absolute channel helpers", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetOutputsAbsoluteChannel(0, 0) == 1);
    CHECK(om.GetOutputsAbsoluteChannel(1, 5) == 516);
    CHECK(om.GetOutputsAbsoluteChannel(4, 0) == 1551);
    CHECK(om.GetOutputsAbsoluteChannel(5, 0) == -1);
    CHECK(om.GetAbsoluteChannel(0, 0) == 1);
    CHECK(om.GetAbsoluteChannel(2, 9) == 1560);
    CHECK(om.GetAbsoluteChannel(3, 0) == -1);
    CHECK(om.GetAbsoluteChannel("10.0.0.1", 1, 10) == 521);
    CHECK(om.GetAbsoluteChannel("10.0.0.2", 0, 0) == 1551);
    CHECK(om.GetAbsoluteChannel("10.0.0.2", 5, 0) == -1);
}

TEST_CASE("OutputManager IP universe listing", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetIPUniverses("10.0.0.1") == std::list<int>{ 1, 2, 3 });
    CHECK(om.GetIPUniverses("10.0.0.2") == std::list<int>{ 1 });
    auto all = om.GetIPUniverses();
    CHECK(all == std::list<int>{ 1, 2, 3, 50 });
}

TEST_CASE("OutputManager DecodeStartChannel absolute and special forms", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.DecodeStartChannel("") == 0);
    CHECK(om.DecodeStartChannel("100") == 100);
    CHECK(om.DecodeStartChannel("abc") == 0);
    CHECK(om.DecodeStartChannel("#2:10") == 520);
    CHECK(om.DecodeStartChannel("#10.0.0.1:3:1") == 1021);
    CHECK(om.DecodeStartChannel("#10.0.0.2:1:5") == 1555);
    CHECK(om.DecodeStartChannel("!B:5") == 1535);
    CHECK(om.DecodeStartChannel("!C:1") == 1551);
}

TEST_CASE("OutputManager DecodeStartChannel rejects invalid forms", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.DecodeStartChannel("#0:10") == 0);
    CHECK(om.DecodeStartChannel("#2:0") == 0);
    CHECK(om.DecodeStartChannel("#999:1") == 0);
    CHECK(om.DecodeStartChannel("#10.0.0.1:99:1") == 0);
    CHECK(om.DecodeStartChannel("#:1:1") == 0);
    CHECK(om.DecodeStartChannel("#1:2:3:4") == 0);
    CHECK(om.DecodeStartChannel("!Nope:5") == 0);
    CHECK(om.DecodeStartChannel("!B:0") == 0);
    CHECK(om.DecodeStartChannel("!B") == 0);
    CHECK(om.DecodeStartChannel("1:2:3") == 0);
    CHECK(om.DecodeStartChannel("0:5") == 0);
}

TEST_CASE("OutputManager channel names", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.GetChannelName(0) == "Ch 1: Net 1 #1");
    CHECK(om.GetChannelName(1530) == "Ch 1531: Net 2 #1");
    CHECK(om.GetChannelName(1649) == "Ch 1650: Net 3 #100");
    CHECK(om.GetChannelName(1650) == "Ch 1651: invalid");
}

TEST_CASE("OutputManager routes channel data to outputs", "[outputs]") {
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->SetName("rec");
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    auto* o2 = new RecordingOutput(5, 2);
    c->AppendOutput(o1);
    c->AppendOutput(o2);
    om.AddController(c);
    om.SomethingChanged();
    REQUIRE(o1->GetStartChannel() == 1);
    REQUIRE(o2->GetStartChannel() == 11);

    om.SetOneChannel(0, 11);
    om.SetOneChannel(9, 22);
    om.SetOneChannel(10, 33);
    om.SetOneChannel(14, 44);
    om.SetOneChannel(15, 55);
    CHECK(o1->data[0] == 11);
    CHECK(o1->data[9] == 22);
    CHECK(o2->data[0] == 33);
    CHECK(o2->data[4] == 44);
    CHECK(o1->oneCalls == 2);
    CHECK(o2->oneCalls == 2);
}

TEST_CASE("OutputManager SetManyChannels spans outputs", "[outputs]") {
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    auto* o2 = new RecordingOutput(5, 2);
    c->AppendOutput(o1);
    c->AppendOutput(o2);
    om.AddController(c);
    om.SomethingChanged();

    unsigned char block[12];
    for (int i = 0; i < 12; ++i) block[i] = static_cast<unsigned char>(i + 1);
    om.SetManyChannels(6, block, 12);
    for (int i = 0; i < 4; ++i) CHECK(o1->data[6 + i] == i + 1);
    for (int i = 0; i < 5; ++i) CHECK(o2->data[i] == 5 + i);
    CHECK(o1->manyCalls == 1);
    CHECK(o2->manyCalls == 1);

    om.SetManyChannels(100, block, 3);
    om.SetManyChannels(0, block, 0);
    CHECK(o1->manyCalls == 1);
}

TEST_CASE("OutputManager skips disabled outputs when routing", "[outputs]") {
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    c->AppendOutput(o1);
    om.AddController(c);
    om.SomethingChanged();
    o1->Enable(false);
    om.SetOneChannel(2, 9);
    CHECK(o1->oneCalls == 0);
    c->SetActive("Active");
    o1->Enable(true);
    om.SetOneChannel(2, 9);
    CHECK(o1->data[2] == 9);
}

TEST_CASE("OutputManager output lifecycle and frames", "[outputs]") {
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->SetIP("10.0.0.5");
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    auto* o2 = new RecordingOutput(10, 2);
    c->AppendOutput(o1);
    c->AppendOutput(o2);
    om.AddController(c);
    om.SomethingChanged();

    CHECK(om.StartOutput());
    CHECK(om.IsOutputting());
    CHECK(o1->opens == 1);
    CHECK(o2->opens == 1);
    CHECK_FALSE(om.StartOutput());

    om.StartFrame(40);
    CHECK(o1->startFrames == 1);
    CHECK(o1->GetTimer() == 40);
    om.EndFrame();
    CHECK(o1->endFrames == 1);
    CHECK(o2->endFrames == 1);

    om.SetParallelTransmission(true);
    CHECK(om.GetParallelTransmission());
    om.EndFrame();
    CHECK(o1->endFrames == 2);
    CHECK(o2->endFrames == 2);

    om.AllOff(false);
    CHECK(o1->allOffs == 1);
    CHECK(o1->endFrames == 2);
    om.AllOff(true);
    CHECK(o1->allOffs == 2);
    CHECK(o1->endFrames == 3);

    om.StopOutput();
    CHECK_FALSE(om.IsOutputting());
    CHECK(o1->closes == 1);
    om.StartFrame(80);
    CHECK(o1->startFrames == 1);
}

TEST_CASE("OutputManager frame calls do nothing when not outputting", "[outputs]") {
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    c->AppendOutput(o1);
    om.AddController(c);
    om.StartFrame(1);
    om.EndFrame();
    om.ResetFrame();
    CHECK(o1->startFrames == 0);
    CHECK(o1->endFrames == 0);
    CHECK(om.GetPacketsPerSecond() == 0);
}

TEST_CASE("OutputManager StartOutput with no outputs does not start", "[outputs]") {
    OutputManager om;
    CHECK_FALSE(om.StartOutput());
    CHECK_FALSE(om.IsOutputting());
}

TEST_CASE("OutputManager settings track dirtiness", "[outputs]") {
    OutputManager om;
    CHECK_FALSE(om.IsDirty());
    om.SetSuppressFrames(0);
    CHECK_FALSE(om.IsDirty());
    om.SetSuppressFrames(3);
    CHECK(om.GetSuppressFrames() == 3);
    CHECK(om.IsDirty());
}

TEST_CASE("OutputManager sync settings", "[outputs]") {
    OutputManager om;
    CHECK_FALSE(om.IsSyncEnabled());
    om.SetSyncEnabled(true);
    om.SetSyncUniverse(7);
    CHECK(om.IsSyncEnabled());
    CHECK(OutputManager::IsSyncEnabled_());
    CHECK(om.GetSyncUniverse() == 7);
    om.SetSyncEnabled(false);
    CHECK(om.GetSyncUniverse() == 0);
    CHECK_FALSE(OutputManager::IsSyncEnabled_());
}

TEST_CASE("OutputManager global FPP proxy reaches outputs", "[outputs]") {
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    c->AppendOutput(o1);
    om.AddController(c);
    om.SetGlobalFPPProxy("10.0.0.99");
    CHECK(om.GetGlobalFPPProxy() == "10.0.0.99");
    CHECK(o1->GlobalFPPProxy() == "10.0.0.99");
    CHECK(om.IsDirty());
}

TEST_CASE("OutputManager global force local ip does not touch the FPP proxy", "[outputs]") {
    // OutputManager::SetGlobalForceLocalIP forwards the value to SetGlobalFPPProxy on every
    // controller (a copy and paste slip), so outputs end up with the local ip as their proxy.
    OutputManager om;
    auto* c = new ControllerEthernet(&om);
    c->ClearOutputs();
    auto* o1 = new RecordingOutput(10, 1);
    c->AppendOutput(o1);
    om.AddController(c);
    om.SetGlobalForceLocalIP("10.0.0.77");
    CHECK(om.GetGlobalForceLocalIP() == "10.0.0.77");
    CHECK(o1->GlobalFPPProxy().empty());
    CHECK(o1->GlobalForceLocalIP() == "10.0.0.77");
}

TEST_CASE("OutputManager suspend and protocol queries", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    CHECK(om.AtLeastOneOutputUsingProtocol(OUTPUT_E131));
    CHECK_FALSE(om.AtLeastOneOutputUsingProtocol(OUTPUT_DDP));
    CHECK(om.AtLeastOneOutputUsingProtocol(OUTPUT_NULL));
    CHECK(om.GetForceIPs(OUTPUT_E131) == std::list<std::string>{ "" });
    CHECK(om.GetForceIPs(OUTPUT_DDP).empty());
    om.SuspendAll(true);
    om.SuspendAll(false);
}

TEST_CASE("OutputManager sorting", "[outputs]") {
    OutputManager om;
    Add(om, E131Controller("Zed", "10.0.0.30", 1, 1));
    Add(om, E131Controller("Alpha", "10.0.0.20", 1, 1));
    Add(om, E131Controller("Mid", "10.0.0.10", 1, 1));

    om.SortControllersbyName();
    CHECK(om.GetControllerNames() == std::list<std::string>{ "Alpha", "Mid", "Zed" });
    CHECK(om.GetController("Alpha")->GetStartChannel() == 1);

    om.SortControllersbyIP();
    CHECK(om.GetControllerNames() == std::list<std::string>{ "Mid", "Alpha", "Zed" });
    CHECK(om.GetController("Mid")->GetStartChannel() == 1);

    om.SortControllersbyProtocal();
    CHECK(om.GetControllerCount() == 3);
}

TEST_CASE("OutputManager sorts by id and model", "[outputs]") {
    OutputManager om;
    Add(om, NullController("n3", 30, 1));
    Add(om, NullController("n1", 10, 1));
    Add(om, NullController("n2", 20, 1));
    om.SortControllersbyID();
    CHECK(At(om, 0)->GetName() == "n1");
    CHECK(At(om, 2)->GetName() == "n3");
    om.SortControllersbyModel();
    CHECK(At(om, 0)->GetName() == "n1");
}

TEST_CASE("OutputManager test presets", "[outputs]") {
    OutputManager om;
    CHECK(om.GetTestPresets().empty());
    TestPreset* p = om.CreateTestPreset("first");
    p->AddChannel(5);
    om.CreateTestPreset("second");
    CHECK(om.GetTestPresets() == std::list<std::string>{ "first", "second" });
    CHECK(om.GetTestPreset("first") == p);
    CHECK(om.GetTestPreset("missing") == nullptr);

    TestPreset* replaced = om.CreateTestPreset("first");
    CHECK(replaced->GetChannels().empty());
    CHECK(om.GetTestPresets().size() == 2);
    om.DeleteTestPreset();
    CHECK(om.GetTestPresets().empty());
}

TEST_CASE("TestPreset compresses consecutive channels when saved", "[outputs]") {
    TestPreset p("p");
    p.AddChannel(10);
    p.AddChannel(1);
    p.AddChannelRange(2, 3);
    p.AddChannel(7);
    p.AddChannel(11);
    pugi::xml_document doc;
    pugi::xml_node n = p.Save(doc);
    CHECK(std::string(n.attribute("name").as_string()) == "p");
    std::vector<std::string> parts;
    for (auto c : n.children()) {
        if (std::string(c.name()) == "channel") parts.push_back("c" + std::to_string(c.attribute("id").as_int()));
        else parts.push_back("r" + std::to_string(c.attribute("start").as_int()) + "-" + std::to_string(c.attribute("end").as_int()));
    }
    CHECK(parts == std::vector<std::string>{ "r1-3", "c7", "r10-11" });

    TestPreset back(n);
    CHECK(back.GetName() == "p");
    CHECK(back.GetChannels().size() == 6);
}

TEST_CASE("OutputManager SaveToXML records global settings", "[outputs]") {
    OutputManager om;
    BuildShow(om);
    om.SetGlobalFPPProxy("10.0.0.88");
    om.SetSuppressFrames(4);
    om.SetSyncEnabled(true);
    om.SetSyncUniverse(9);
    om.SetBaseShowDir("/some/base");
    om.SetAutoUpdateFromBaseShowDir(true);
    om.CreateTestPreset("tp")->AddChannel(3);

    pugi::xml_document doc;
    om.SaveToXML(doc);
    pugi::xml_node root = doc.child("Networks");
    REQUIRE(root);
    CHECK(std::string(root.attribute("GlobalFPPProxy").as_string()) == "10.0.0.88");
    CHECK(std::string(root.attribute("BaseShowDir").as_string()) == "/some/base");
    CHECK(std::string(root.attribute("AutoUpdateFromBase").as_string()) == "1");
    CHECK(root.child("e131sync").attribute("universe").as_int() == 9);
    CHECK(root.child("suppressframes").attribute("frames").as_int() == 4);
    int controllers = 0;
    for (auto c : root.children("Controller")) {
        (void)c;
        ++controllers;
    }
    CHECK(controllers == 3);
    CHECK(root.child("testpreset"));
    om.SetSyncEnabled(false);
}

TEST_CASE("OutputManager Save and Load round trip", "[outputs]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    Add(om, E131Controller("A", "10.0.0.1", 1, 3, 510, "Inactive"));
    Add(om, NullController("B", 50, 20));
    Add(om, E131Controller("C", "10.0.0.2", 1, 1, 100, "Inactive"));
    om.SetSuppressFrames(2);
    om.SetGlobalFPPProxy("10.0.0.88");
    om.CreateTestPreset("tp")->AddChannelRange(1, 5);
    REQUIRE(om.Save());
    CHECK_FALSE(om.IsDirty());
    CHECK(std::filesystem::exists(dir.path / "xlights_networks.xml"));

    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    CHECK_FALSE(loaded.DidConvert());
    CHECK(loaded.GetControllerCount() == 3);
    CHECK(loaded.GetOutputCount() == 5);
    CHECK(loaded.GetTotalChannels() == 1650);
    CHECK(loaded.GetSuppressFrames() == 2);
    CHECK(loaded.GetGlobalFPPProxy() == "10.0.0.88");
    CHECK(loaded.GetTestPresets() == std::list<std::string>{ "tp" });
    CHECK(loaded.GetController("C")->GetStartChannel() == 1551);
    CHECK_FALSE(loaded.GetController("A")->IsActive());
    CHECK(ToString(loaded) == ToString(om));
}

TEST_CASE("OutputManager Load of a missing file fails", "[outputs]") {
    TempDir dir;
    OutputManager om;
    Add(om, NullController("x", 1, 1));
    CHECK_FALSE(om.Load(dir.path.string()));
    CHECK(om.GetControllerCount() == 0);
}

TEST_CASE("OutputManager Load of a malformed file fails", "[outputs]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", "<Networks><Controller");
    OutputManager om;
    CHECK_FALSE(om.Load(dir.path.string()));
}

TEST_CASE("OutputManager Load skips unknown controller and network types", "[outputs]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", "<Networks>" + NullController("keep", 1, 4) + "<Controller Name=\"alien\" Type=\"Teleporter\"/></Networks>");
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    CHECK(om.GetControllerCount() == 1);
    CHECK(om.GetController("keep") != nullptr);
}

TEST_CASE("OutputManager Load does not carry over state from the previous show", "[outputs]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", "<Networks>" + NullController("only", 1, 4) + "</Networks>");
    OutputManager om;
    Add(om, NullController("old", 9, 1));
    om.SetSuppressFrames(8);
    REQUIRE(om.Load(dir.path.string()));
    CHECK(om.GetControllerCount() == 1);
    CHECK(om.GetController("old") == nullptr);
    CHECK_FALSE(om.IsDirty());
}

TEST_CASE("OutputManager Load reads the legacy flat networks format", "[outputs]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", R"(<Networks>
        <network NetworkType="E131" ComPort="10.0.0.5" BaudRate="1" MaxChannels="510" Enabled="No" Description="Front"/>
        <network NetworkType="E131" ComPort="10.0.0.5" BaudRate="2" MaxChannels="510" Enabled="No" Description="Front"/>
        <network NetworkType="E131" ComPort="10.0.0.6" BaudRate="1" MaxChannels="100" Enabled="No" Description="Side"/>
        <network NetworkType="NULL" Id="3" MaxChannels="10" Enabled="No" Description="Gap"/>
        <e131sync universe="5"/>
        <suppressframes frames="3"/>
    </Networks>)");
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    CHECK(om.DidConvert());
    CHECK(om.GetSyncUniverse() == 5);
    CHECK(om.GetSuppressFrames() == 3);
    REQUIRE(om.GetControllerCount() == 3);
    auto* first = At(om, 0);
    CHECK(first->GetOutputCount() == 2);
    CHECK(first->GetChannels() == 1020);
    CHECK(first->GetIP() == "10.0.0.5");
    CHECK(first->GetName() == "Front");
    CHECK(At(om, 1)->GetIP() == "10.0.0.6");
    CHECK(At(om, 2)->GetType() == CONTROLLER_NULL);
    CHECK(om.GetTotalChannels() == 1020 + 100 + 10);
    CHECK_FALSE(first->IsActive());
}

TEST_CASE("OutputManager converts old output number start channels", "[outputs]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", R"(<Networks>
        <network NetworkType="E131" ComPort="10.0.0.5" BaudRate="1" MaxChannels="510" Enabled="No" Description="Front"/>
        <network NetworkType="E131" ComPort="MULTICAST" BaudRate="9" MaxChannels="510" Enabled="No" Description="Mc"/>
        <network NetworkType="NULL" Id="3" MaxChannels="10" Enabled="No" Description="Gap"/>
    </Networks>)");
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));

    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<models>
        <model name="m1" StartChannel="1:7"/>
        <model name="m2" StartChannel="2:3"/>
        <model name="m3" StartChannel="3:2"/>
        <model name="m4" StartChannel="500"/>
        <model name="m5" StartChannel="#10.0.0.5:1:1"/>
        <model name="m6" StartChannel="!Front:4"/>
    </models>)"));
    CHECK(om.ConvertModelStartChannels(doc.child("models")));
    auto sc = [&](const char* name) {
        return std::string(doc.select_node((std::string("//model[@name='") + name + "']").c_str()).node().attribute("StartChannel").as_string());
    };
    CHECK(sc("m1") == "#10.0.0.5:1:7");
    CHECK(sc("m2") == "#9:3");
    CHECK(sc("m3") == "!Gap:2");
    CHECK(sc("m4") == "500");
    CHECK(sc("m5") == "#10.0.0.5:1:1");
    CHECK(sc("m6") == "!Front:4");
}

TEST_CASE("OutputManager ConvertModelStartChannels does nothing without a conversion", "[outputs]") {
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<models><model name="m" StartChannel="1:7"/></models>)"));
    CHECK_FALSE(om.ConvertModelStartChannels(doc.child("models")));
    CHECK(std::string(doc.child("models").child("model").attribute("StartChannel").as_string()) == "1:7");
}

TEST_CASE("OutputManager merge from a base show folder adds new controllers", "[outputs]") {
    TempDir base;
    base.Write("xlights_networks.xml", "<Networks>" + NullController("fromBase", 5, 12) + NullController("shared", 6, 3) + "</Networks>");

    OutputManager om;
    Add(om, NullController("shared", 6, 3));
    om.SetBaseShowDir(base.path.string());
    bool acceptAll = false;
    bool rejectAll = false;
    bool changed = false;
    REQUIRE(om.MergeFromBase(false, acceptAll, rejectAll, nullptr, &changed));
    CHECK(changed);
    CHECK(om.GetControllerCount() == 2);
    auto* added = om.GetController("fromBase");
    REQUIRE(added != nullptr);
    CHECK(added->IsFromBase());
    CHECK(added->GetChannels() == 12);
    CHECK_FALSE(om.GetController("shared")->IsFromBase());
}

TEST_CASE("OutputManager merge only refreshes controllers that came from the base", "[outputs]") {
    TempDir base;
    base.Write("xlights_networks.xml", "<Networks><Controller Id=\"5\" Name=\"c\" Description=\"new text\" Type=\"Null\"><network NetworkType=\"NULL\" Id=\"5\" MaxChannels=\"12\"/></Controller></Networks>");

    OutputManager local;
    Add(local, "<Controller Id=\"5\" Name=\"c\" Description=\"old text\" Type=\"Null\"><network NetworkType=\"NULL\" Id=\"5\" MaxChannels=\"12\"/></Controller>");
    local.SetBaseShowDir(base.path.string());
    bool a = false, r = false, changed = true;
    REQUIRE(local.MergeFromBase(false, a, r, nullptr, &changed));
    CHECK_FALSE(changed);
    CHECK(local.GetController("c")->GetDescription() == "old text");

    local.GetController("c")->SetFromBase(true);
    REQUIRE(local.MergeFromBase(false, a, r, nullptr, &changed));
    CHECK(changed);
    CHECK(local.GetController("c")->GetDescription() == "new text");
}

TEST_CASE("OutputManager merge fails when the base networks file is missing", "[outputs]") {
    TempDir base;
    OutputManager om;
    om.SetBaseShowDir(base.path.string());
    bool a = false, r = false;
    CHECK_FALSE(om.MergeFromBase(false, a, r));
}

TEST_CASE("OutputManager base controller sync tracking", "[outputs]") {
    OutputManager om;
    CHECK_FALSE(om.NeedsBaseControllersUpdate());
    TempDir base;
    base.Write("xlights_networks.xml", "<Networks/>");
    om.SetBaseShowDir(base.path.string());
    CHECK(om.NeedsBaseControllersUpdate());
    om.MarkBaseControllersSynced();
    CHECK_FALSE(om.NeedsBaseControllersUpdate());
}

TEST_CASE("OutputManager export headers", "[outputs]") {
    auto h = OutputManager::GetExportHeaders();
    REQUIRE(h.size() == 18);
    CHECK(h.front() == "Controller Name");
    CHECK(h[1] == "Start Absolute");
    CHECK(h.back() == "FPP Proxy");
}

TEST_CASE("OutputManager confirm callback is only used when interactive", "[outputs]") {
    int calls = 0;
    OutputManager::SetConfirmCallback([&](const std::string&, const std::string&) {
        ++calls;
        return true;
    });
    const bool prev = OutputManager::IsInteractive();
    OutputManager::SetInteractive(false);
    CHECK_FALSE(OutputManager::Confirm("q", "t"));
    CHECK(calls == 0);
    OutputManager::SetInteractive(true);
    CHECK(OutputManager::Confirm("q", "t"));
    CHECK(calls == 1);
    OutputManager::SetConfirmCallback(nullptr);
    CHECK_FALSE(OutputManager::Confirm("q", "t"));
    OutputManager::SetInteractive(prev);
}

TEST_CASE("OutputManager retry open flag", "[outputs]") {
    const bool prev = OutputManager::IsRetryOpen();
    OutputManager::SetRetryOpen(true);
    CHECK(OutputManager::IsRetryOpen());
    OutputManager::SetRetryOpen(false);
    CHECK_FALSE(OutputManager::IsRetryOpen());
    OutputManager::SetRetryOpen(prev);
}

TEST_CASE("OutputManager serial controllers resolve by id", "[outputs]") {
    OutputManager om;
    Add(om, R"(<Controller Id="7" Name="ser" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="7" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)");
    CHECK(om.GetOutput(7, "") != nullptr);
    CHECK(om.GetController("ser")->GetStartChannel() == 1);
    CHECK(om.GetController("ser")->GetEndChannel() == 512);
    CHECK(om.GetTotalChannels() == 512);
}
