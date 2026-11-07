#include <map>
#include <memory>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "../support/TempDir.h"
#include "outputs/Controller.h"
#include "outputs/ControllerEthernet.h"
#include "outputs/ControllerNull.h"
#include "outputs/ControllerSerial.h"
#include "outputs/Output.h"
#include "outputs/OutputManager.h"
#include "outputs/TestPreset.h"
#include "test_roundtrip_common.h"

using namespace rt;

namespace {

std::string Print(pugi::xml_node n) {
    std::ostringstream ss;
    n.print(ss, "", pugi::format_raw);
    return ss.str();
}

std::string SaveCtrl(Controller& c) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("root");
    c.Save(root);
    return Print(root.first_child());
}

std::unique_ptr<Controller> Load(OutputManager& om, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    return std::unique_ptr<Controller>(Controller::Create(&om, doc.first_child(), ""));
}

std::string Net(const std::string& type, const std::string& extra) {
    return "<network NetworkType=\"" + type + "\" " + extra + "/>";
}

std::string Ctrl(const std::string& type, const Attrs& attrs, const std::string& children) {
    std::string x = "<Controller Type=\"" + type + "\"";
    for (auto& a : attrs) x += " " + a.first + "=\"" + Esc(a.second) + "\"";
    return x + ">" + children + "</Controller>";
}

const Attrs kCommon = { { "Id", "321" }, { "Name", "My Ctrl & <1>" }, { "Description", "a \"desc\" with 'q' & more" }, { "Vendor", "ZZVendor" }, { "Model", "ZZModel" }, { "Variant", "ZZVar" },
    { "AutoSize", "1" }, { "AutoLayout", "1" }, { "FromBase", "1" }, { "ActiveState", "Inactive" }, { "SuppressDuplicates", "1" }, { "Monitor", "0" },
    { "DefaultBrightnessUnderFullControl", "55" }, { "DefaultGammaUnderFullControl", "2.5" }, { "AutoUpload", "0" } };

Attrs With(Attrs a, const Attrs& b) {
    a.insert(a.end(), b.begin(), b.end());
    return a;
}

struct Case {
    std::string label;
    std::string xml;
    std::set<std::string> na; // network attributes that the owning controller legitimately overrides
};

std::string EthNets(const std::string& type, int n, const std::string& per) {
    std::string r;
    for (int i = 0; i < n; ++i) r += Net(type, "ComPort=\"10.9.8.7\" BaudRate=\"" + std::to_string(i + 5) + "\" MaxChannels=\"" + std::to_string(100 + i) + "\" " + per);
    return r;
}

std::vector<Case> Controllers() {
    std::vector<Case> c;
    auto eth = [&](const std::string& label, const std::string& proto, const std::string& netType, const Attrs& extra, const std::string& perNet, int n = 2) {
        Attrs a = With(kCommon, { { "Protocol", proto }, { "IP", "10.9.8.7" }, { "Priority", "77" }, { "FPPProxy", "10.1.1.1" }, { "Expanded", "TRUE" }, { "UPS", "TRUE" }, { "ForceLocalIP", "192.168.5.5" }, { "Version", "3" } });
        a.insert(a.end(), extra.begin(), extra.end());
        c.push_back({ label, Ctrl("Ethernet", a, EthNets(netType, n, perNet) + "<ExtraProperty name=\"k1\" value=\"v&amp;1\"/><ExtraProperty name=\"k2\" value=\"\"/>"), {} });
    };
    eth("E131", "E131", "E131", {}, "Priority=\"60\"");
    eth("ArtNet", "ArtNet", "ArtNet", {}, "ForceSourcePort=\"TRUE\"");
    eth("DDP", "DDP", "DDP", {}, "ChannelsPerPacket=\"1000\" KeepChannelNumbers=\"1\"");
    eth("KiNET", "KINET", "KINET", {}, "Version=\"1\"");
    eth("OPC", "OPC", "OPC", {}, "");
    eth("Twinkly", "Twinkly", "Twinkly", {}, "HTTPPort=\"8080\"");
    eth("ZCPP", "ZCPP", "ZCPP", {}, "Vendor=\"12\" Model=\"34\" Priority=\"90\" SupportsVirtualStrings=\"TRUE\" SupportsSmartRemotes=\"TRUE\" Multicast=\"TRUE\" DontConfigure=\"TRUE\" Protocols=\"ws2811,apa102\"", 1);
    eth("xxx Ethernet", "xxx Ethernet", "xxx Ethernet", {}, "Id=\"4\"");

    auto ser = [&](const std::string& label, const std::string& proto, const std::string& netType, const std::string& port, const std::string& speed, const std::string& perNet) {
        Attrs a = With(kCommon, { { "Protocol", proto }, { "Port", port }, { "Speed", speed }, { "Prefix", "AA55" }, { "Postfix", "FF" }, { "FPPProxy", "10.2.2.2" } });
        c.push_back({ label, Ctrl("Serial", a, Net(netType, "Id=\"" + std::string("7") + "\" ComPort=\"" + port + "\" BaudRate=\"" + speed + "\" MaxChannels=\"64\" " + perNet)), {} });
    };
    ser("DMX serial", "DMX", "DMX", "COM3", "250000", "");
    ser("OpenDMX serial", "OpenDMX", "OpenDMX", "COM4", "250000", "");
    ser("Pixelnet serial", "Pixelnet", "Pixelnet", "COM5", "115200", "");
    ser("Open Pixelnet serial", "Pixelnet-Open", "Pixelnet-Open", "COM5", "115200", "");
    ser("LOR serial", "LOR", "LOR", "COM6", "19200", "");
    ser("LOR optimised serial", "LOR Optimised", "LOR Optimised", "COM7", "57600", "");
    ser("D-Light serial", "D-Light", "D-Light", "COM8", "38400", "");
    ser("Renard serial", "Renard", "Renard", "COM9", "57600", "");
    ser("xxx serial", "xxx Serial", "xxx Serial", "COM10", "9600", "DeviceChannels=\"16\"");
    ser("Generic serial", "Generic Serial", "Generic Serial", "/dev/ttyUSB0", "9600", "");

    c.push_back({ "Null", Ctrl("Null", With(kCommon, {}), Net("NULL", "Id=\"64005\" MaxChannels=\"77\"")), {} });
    const std::map<std::string, std::set<std::string>> overridden = {
        { "E131", { "Priority" } }, { "KiNET", { "Version" } }, { "ZCPP", { "Priority" } }, { "xxx Ethernet", { "BaudRate" } },
        { "DMX serial", { "Id" } }, { "OpenDMX serial", { "Id" } }, { "Pixelnet serial", { "Id", "BaudRate" } }, { "Open Pixelnet serial", { "Id", "BaudRate" } },
        { "LOR serial", { "Id" } }, { "LOR optimised serial", { "Id", "MaxChannels" } }, { "D-Light serial", { "Id" } }, { "Renard serial", { "Id" } },
        { "xxx serial", { "Id", "MaxChannels" } }, { "Generic serial", { "Id" } } };
    for (auto& cs : c) {
        auto it = overridden.find(cs.label);
        if (it != overridden.end()) cs.na = it->second;
    }
    return c;
}

void CheckStable(const Case& cs) {
    OutputManager om1;
    auto c1 = Load(om1, cs.xml);
    REQUIRE(c1 != nullptr);
    std::string s1 = SaveCtrl(*c1);
    OutputManager om2;
    auto c2 = Load(om2, s1);
    REQUIRE(c2 != nullptr);
    std::string s2 = SaveCtrl(*c2);
    INFO("save1 vs save2:" << DiffAttrs(s1, s2));
    CHECK(s2 == s1);
    OutputManager om3;
    auto c3 = Load(om3, s2);
    REQUIRE(c3 != nullptr);
    CHECK(SaveCtrl(*c3) == s2);
    CHECK(c2->GetOutputCount() == c1->GetOutputCount());
    CHECK(c2->GetChannels() == c1->GetChannels());
    CHECK(c2->GetName() == c1->GetName());
    CHECK(c2->GetDescription() == c1->GetDescription());
}

void CheckAttrs(const Case& cs) {
    OutputManager om1;
    auto c1 = Load(om1, cs.xml);
    REQUIRE(c1 != nullptr);
    pugi::xml_document in, out;
    REQUIRE(in.load_string(cs.xml.c_str()));
    REQUIRE(out.load_string(SaveCtrl(*c1).c_str()));
    auto inA = AttrMap(in.first_child());
    auto outA = AttrMap(out.first_child());
    for (auto& kv : inA) {
        INFO("controller attribute " << kv.first << " = " << kv.second);
        if (kv.first == "Id" && c1->GetType() == "Null") continue;
        REQUIRE(outA.count(kv.first) == 1);
        CHECK(Same(outA[kv.first], kv.second));
    }
    auto inN = in.first_child().children("network");
    auto outN = out.first_child().children("network");
    auto io = inN.begin();
    auto oo = outN.begin();
    int idx = 0;
    for (; io != inN.end() && oo != outN.end(); ++io, ++oo, ++idx) {
        auto a = AttrMap(*io);
        auto b = AttrMap(*oo);
        for (auto& kv : a) {
            INFO("network " << idx << " attribute " << kv.first << " = " << kv.second);
            if (kv.first == "AutoSize" || cs.na.count(kv.first)) continue;
            REQUIRE(b.count(kv.first) == 1);
            CHECK(Same(b[kv.first], kv.second));
        }
    }
    CHECK((io == inN.end()) == (oo == outN.end()));
    int ep = 0;
    for (auto e : out.first_child().children("ExtraProperty")) {
        (void)e;
        ++ep;
    }
    int epIn = 0;
    for (auto e : in.first_child().children("ExtraProperty")) {
        (void)e;
        ++epIn;
    }
    CHECK(ep == epIn);
}

} // namespace

#define CTRL_CASES(IDX, LABEL)                                                                                   \
    TEST_CASE("Roundtrip controller is stable: " LABEL, "[roundtrip][outputs]") { CheckStable(Controllers()[IDX]); } \
    TEST_CASE("Roundtrip controller keeps attributes: " LABEL, "[roundtrip][outputs]") { CheckAttrs(Controllers()[IDX]); }

CTRL_CASES(0, "E131")
CTRL_CASES(1, "ArtNet")
CTRL_CASES(2, "DDP")
CTRL_CASES(3, "KiNET")
CTRL_CASES(4, "OPC")
CTRL_CASES(5, "Twinkly")
CTRL_CASES(6, "ZCPP")
CTRL_CASES(7, "xxx Ethernet")
CTRL_CASES(8, "DMX serial")
CTRL_CASES(9, "OpenDMX serial")
CTRL_CASES(10, "Pixelnet serial")
CTRL_CASES(11, "Open Pixelnet serial")
CTRL_CASES(12, "LOR serial")
CTRL_CASES(13, "LOR optimised serial")
CTRL_CASES(14, "D-Light serial")
CTRL_CASES(15, "Renard serial")
CTRL_CASES(16, "xxx serial")
CTRL_CASES(17, "Generic serial")
CTRL_CASES(18, "Null")

TEST_CASE("Roundtrip default controllers are stable", "[roundtrip][outputs]") {
    for (const char* t : { "Ethernet", "Serial", "Null" }) {
        INFO(t);
        OutputManager om;
        std::unique_ptr<Controller> c;
        if (std::string(t) == "Ethernet") c.reset(new ControllerEthernet(&om));
        else if (std::string(t) == "Serial") c.reset(new ControllerSerial(&om));
        else c.reset(new ControllerNull(&om));
        std::string s1 = SaveCtrl(*c);
        OutputManager om2;
        auto c2 = Load(om2, s1);
        REQUIRE(c2 != nullptr);
        std::string s2 = SaveCtrl(*c2);
        INFO("save1 vs save2:" << DiffAttrs(s1, s2));
        CHECK(s2 == s1);
    }
}

TEST_CASE("Roundtrip controller states", "[roundtrip][outputs]") {
    for (const char* st : { "Active", "Inactive", "xLights Only" }) {
        INFO(st);
        OutputManager om;
        auto c = Load(om, Ctrl("Null", { { "Name", "N" }, { "ActiveState", st } }, Net("NULL", "Id=\"1\" MaxChannels=\"5\"")));
        REQUIRE(c != nullptr);
        std::string s1 = SaveCtrl(*c);
        pugi::xml_document d;
        REQUIRE(d.load_string(s1.c_str()));
        CHECK(std::string(d.first_child().attribute("ActiveState").value()) == st);
        OutputManager om2;
        CHECK(SaveCtrl(*Load(om2, s1)) == s1);
    }
}
TEST_CASE("Roundtrip controller text fields with specials", "[roundtrip][outputs]") {
    for (const char* t : { "a & b", "<x>", "q\"uote'", "tab\there", "multi\nline", "uni\xC3\xA9\xE2\x9D\x84", "100%", "" }) {
        INFO(t);
        OutputManager om;
        auto c = Load(om, Ctrl("Ethernet", { { "Name", std::string("N ") + t }, { "Description", t }, { "Protocol", "E131" }, { "IP", "1.2.3.4" }, { "Vendor", t }, { "FPPProxy", t } }, Net("E131", "ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"10\"")));
        REQUIRE(c != nullptr);
        std::string s1 = SaveCtrl(*c);
        OutputManager om2;
        auto c2 = Load(om2, s1);
        REQUIRE(c2 != nullptr);
        CHECK(c2->GetName() == c->GetName());
        CHECK(c2->GetDescription() == c->GetDescription());
        CHECK(SaveCtrl(*c2) == s1);
    }
}
TEST_CASE("Roundtrip controller extra properties with specials", "[roundtrip][outputs]") {
    OutputManager om;
    auto c = Load(om, Ctrl("Null", { { "Name", "N" } }, Net("NULL", "Id=\"1\" MaxChannels=\"5\"") + "<ExtraProperty name=\"a&amp;b\" value=\"&lt;v&gt;&#10;x\"/><ExtraProperty name=\"\xC3\xA9\" value=\"\"/>"));
    REQUIRE(c != nullptr);
    std::string s1 = SaveCtrl(*c);
    OutputManager om2;
    auto c2 = Load(om2, s1);
    REQUIRE(c2 != nullptr);
    CHECK(c2->GetExtraProperty("a&b") == "<v>\nx");
    CHECK(SaveCtrl(*c2) == s1);
}
TEST_CASE("Roundtrip controller extreme numbers settle", "[roundtrip][outputs]") {
    for (const char* v : { "0", "-1", "2147483647", "-2147483648", "99999999999", "abc", "", "1e30" }) {
        for (const char* attr : { "Id", "Priority", "Version", "DefaultBrightnessUnderFullControl", "DefaultGammaUnderFullControl" }) {
            INFO(attr << " = " << v);
            OutputManager om;
            auto c = Load(om, Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" }, { attr, v } }, Net("E131", "ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"10\"")));
            if (!c) continue;
            std::string s1 = SaveCtrl(*c);
            OutputManager om2;
            auto c2 = Load(om2, s1);
            REQUIRE(c2 != nullptr);
            std::string s2 = SaveCtrl(*c2);
            OutputManager om3;
            auto c3 = Load(om3, s2);
            REQUIRE(c3 != nullptr);
            INFO("save1 vs save2:" << DiffAttrs(s1, s2));
            CHECK(s2 == s1);
            CHECK(SaveCtrl(*c3) == s2);
        }
    }
}
TEST_CASE("Roundtrip output extreme universe and channel counts settle", "[roundtrip][outputs]") {
    for (const char* v : { "0", "-1", "1", "63999", "64000", "65535", "65536", "2147483647", "abc", "" }) {
        for (const char* attr : { "BaudRate", "MaxChannels" }) {
            INFO(attr << " = " << v);
            std::string per = std::string("ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"10\"");
            std::string netAttrs = attr == std::string("BaudRate") ? std::string("ComPort=\"1.2.3.4\" BaudRate=\"") + v + "\" MaxChannels=\"10\"" : std::string("ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"") + v + "\"";
            OutputManager om;
            auto c = Load(om, Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" } }, Net("E131", netAttrs)));
            if (!c) continue;
            std::string s1 = SaveCtrl(*c);
            OutputManager om2;
            auto c2 = Load(om2, s1);
            REQUIRE(c2 != nullptr);
            std::string s2 = SaveCtrl(*c2);
            INFO("save1 vs save2:" << DiffAttrs(s1, s2));
            CHECK(s2 == s1);
        }
    }
}
TEST_CASE("Roundtrip controller with no outputs is stable", "[roundtrip][outputs]") {
    OutputManager om;
    auto c = Load(om, Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" } }, ""));
    REQUIRE(c != nullptr);
    std::string s1 = SaveCtrl(*c);
    OutputManager om2;
    auto c2 = Load(om2, s1);
    REQUIRE(c2 != nullptr);
    CHECK(SaveCtrl(*c2) == s1);
}
TEST_CASE("Roundtrip controller with many outputs keeps order", "[roundtrip][outputs]") {
    std::string nets;
    for (int i = 0; i < 40; ++i) nets += Net("E131", "ComPort=\"1.2.3.4\" BaudRate=\"" + std::to_string(100 - i) + "\" MaxChannels=\"" + std::to_string(10 + i) + "\"");
    OutputManager om;
    auto c = Load(om, Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" } }, nets));
    REQUIRE(c != nullptr);
    std::string s1 = SaveCtrl(*c);
    pugi::xml_document d;
    REQUIRE(d.load_string(s1.c_str()));
    int i = 0;
    for (auto n : d.first_child().children("network")) {
        CHECK(n.attribute("BaudRate").as_int() == 100 - i);
        CHECK(n.attribute("MaxChannels").as_int() == 10 + i);
        ++i;
    }
    CHECK(i == 40);
    OutputManager om2;
    CHECK(SaveCtrl(*Load(om2, s1)) == s1);
}
TEST_CASE("Roundtrip output enabled and suppress flags", "[roundtrip][outputs]") {
    // the output's Enabled flag is derived from the controller's ActiveState, whatever the file says
    for (const char* active : { "Active", "Inactive" }) {
        for (const char* en : { "Yes", "No" }) {
            INFO(active << " / " << en);
            OutputManager om;
            auto c = Load(om, Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" }, { "ActiveState", active } }, Net("E131", std::string("ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"10\" Enabled=\"") + en + "\" SuppressDuplicates=\"Yes\"")));
            REQUIRE(c != nullptr);
            std::string s1 = SaveCtrl(*c);
            pugi::xml_document d;
            REQUIRE(d.load_string(s1.c_str()));
            CHECK(std::string(d.first_child().child("network").attribute("Enabled").value()) == (std::string(active) == "Active" ? "Yes" : "No"));
            OutputManager om2;
            CHECK(SaveCtrl(*Load(om2, s1)) == s1);
        }
    }
}
TEST_CASE("Roundtrip output FPP proxy per output", "[roundtrip][outputs]") {
    OutputManager om;
    auto c = Load(om, Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" } }, Net("E131", "ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"10\" FPPProxy=\"fpp.local\"")));
    REQUIRE(c != nullptr);
    std::string s1 = SaveCtrl(*c);
    pugi::xml_document d;
    REQUIRE(d.load_string(s1.c_str()));
    CHECK(std::string(d.first_child().child("network").attribute("FPPProxy").value()) == "fpp.local");
    OutputManager om2;
    CHECK(SaveCtrl(*Load(om2, s1)) == s1);
}

// ---- whole OutputManager --------------------------------------------------------------------------------

namespace {

std::string ManagerXml(OutputManager& om) {
    pugi::xml_document doc;
    om.SaveToXML(doc);
    std::ostringstream ss;
    doc.print(ss, "", pugi::format_raw);
    std::string s = ss.str();
    // the host name is machine specific
    auto p = s.find("computer=\"");
    if (p != std::string::npos) {
        auto e = s.find('"', p + 10);
        s.erase(p, e - p + 1);
    }
    return s;
}

void FillManager(OutputManager& om, size_t from, size_t to) {
    auto cs = Controllers();
    for (size_t i = from; i < to && i < cs.size(); ++i) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(cs[i].xml.c_str()));
        Controller* c = Controller::Create(&om, doc.first_child(), "");
        REQUIRE(c != nullptr);
        c->SetName(c->GetName() + " #" + std::to_string(i));
        c->SetId(1000 + (int)i);
        om.AddController(c);
    }
    om.SomethingChanged();
}

} // namespace

TEST_CASE("Roundtrip output manager with every controller type", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 19);
    REQUIRE(om.GetControllerCount() == 19);
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    CHECK(loaded.GetControllerCount() == 19);
    INFO("saved vs loaded:" << DiffXml(ManagerXml(om), ManagerXml(loaded)));
    CHECK(ManagerXml(loaded) == ManagerXml(om));
    loaded.SetShowDir(dir.path.string());
    REQUIRE(loaded.Save());
    OutputManager again;
    REQUIRE(again.Load(dir.path.string()));
    CHECK(ManagerXml(again) == ManagerXml(om));
}
TEST_CASE("Roundtrip output manager keeps controller order", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 8);
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    REQUIRE(loaded.GetControllerCount() == om.GetControllerCount());
    for (int i = 0; i < om.GetControllerCount(); ++i) {
        CHECK(loaded.GetControllerIndex(i)->GetName() == om.GetControllerIndex(i)->GetName());
        CHECK(loaded.GetControllerIndex(i)->GetId() == om.GetControllerIndex(i)->GetId());
    }
}
TEST_CASE("Roundtrip output manager global settings", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 2);
    om.SetGlobalFPPProxy("10.0.0.88");
    om.SetGlobalForceLocalIP("192.168.9.9");
    om.SetSuppressFrames(7);
    om.SetSyncEnabled(true);
    om.SetSyncUniverse(33);
    om.SetAutoUpdateFromBaseShowDir(true);
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    CHECK(loaded.GetGlobalFPPProxy() == "10.0.0.88");
    CHECK(loaded.GetSuppressFrames() == 7);
    CHECK(loaded.GetSyncUniverse() == 33);
    CHECK(loaded.IsAutoUpdateFromBaseShowDir() == true);
    CHECK(ManagerXml(loaded) == ManagerXml(om));
    om.SetSyncEnabled(false);
    loaded.SetSyncEnabled(false);
}
// KNOWN BUG candidate: GlobalForceLocalIP is written by SaveToXML; check it is read back.
TEST_CASE("Roundtrip output manager force local ip", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 1);
    om.SetGlobalForceLocalIP("192.168.9.9");
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    CHECK(loaded.GetGlobalForceLocalIP() == "192.168.9.9");
}
TEST_CASE("Roundtrip output manager test presets", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 1);
    om.CreateTestPreset("tp one")->AddChannelRange(1, 5);
    om.CreateTestPreset("tp & two")->AddChannel(9);
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    CHECK(loaded.GetTestPresets() == om.GetTestPresets());
    CHECK(ManagerXml(loaded) == ManagerXml(om));
}
TEST_CASE("Roundtrip output manager sync universe zero and suppress zero are not written", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 1);
    std::string x = ManagerXml(om);
    CHECK(x.find("e131sync") == std::string::npos);
    CHECK(x.find("suppressframes") == std::string::npos);
}
TEST_CASE("Roundtrip output manager empty show", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    CHECK(loaded.GetControllerCount() == 0);
    CHECK(ManagerXml(loaded) == ManagerXml(om));
}
TEST_CASE("Roundtrip output manager start channels are stable", "[roundtrip][outputs][manager]") {
    TempDir dir;
    OutputManager om;
    om.SetShowDir(dir.path.string());
    FillManager(om, 0, 8);
    om.SomethingChanged();
    REQUIRE(om.Save());
    OutputManager loaded;
    REQUIRE(loaded.Load(dir.path.string()));
    loaded.SomethingChanged();
    CHECK(loaded.GetTotalChannels() == om.GetTotalChannels());
    for (int i = 0; i < om.GetControllerCount(); ++i) {
        CHECK(loaded.GetControllerIndex(i)->GetStartChannel() == om.GetControllerIndex(i)->GetStartChannel());
        CHECK(loaded.GetControllerIndex(i)->GetChannels() == om.GetControllerIndex(i)->GetChannels());
    }
}

TEST_CASE("Roundtrip inactive controller output enabled flag does not depend on layout recalculation", "[roundtrip][outputs]") {
    const std::string serialNet = Net("DMX", "ComPort=\"COM3\" BaudRate=\"250000\" MaxChannels=\"10\"");
    const std::vector<std::pair<std::string, std::string>> cases = {
        { "Ethernet", Ctrl("Ethernet", { { "Name", "N" }, { "Protocol", "E131" }, { "IP", "1.2.3.4" }, { "ActiveState", "Inactive" } }, Net("E131", "ComPort=\"1.2.3.4\" BaudRate=\"1\" MaxChannels=\"10\"")) },
        { "Serial", Ctrl("Serial", { { "Name", "N" }, { "Protocol", "DMX" }, { "Port", "COM3" }, { "Speed", "250000" }, { "ActiveState", "Inactive" } }, serialNet) },
        // the FPP model swaps the serial output for a DDP output after the base constructor has run
        { "Serial FPP", Ctrl("Serial", { { "Name", "N" }, { "Protocol", "DMX" }, { "Port", "1.2.3.4:ttyS0" }, { "Speed", "250000" }, { "Vendor", "FPP" }, { "Model", "FPP" }, { "ActiveState", "Inactive" } }, serialNet) },
    };
    for (const auto& [label, xml] : cases) {
        INFO(label);
        OutputManager om;
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Controller* c = Controller::Create(&om, doc.first_child(), "");
        REQUIRE(c != nullptr);
        om.AddController(c);
        for (auto* o : c->GetOutputs()) CHECK_FALSE(o->IsEnabled());
        std::string before = ManagerXml(om);
        om.SomethingChanged();
        CHECK(ManagerXml(om) == before);
    }
}

TEST_CASE("Inactive serial controller keeps its outputs disabled across a protocol change", "[outputs]") {
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(Ctrl("Serial", { { "Name", "N" }, { "Protocol", "DMX" }, { "Port", "COM3" }, { "Speed", "250000" }, { "ActiveState", "Inactive" } }, Net("DMX", "ComPort=\"COM3\" BaudRate=\"250000\" MaxChannels=\"10\"")).c_str()));
    std::unique_ptr<Controller> c(Controller::Create(&om, doc.first_child(), ""));
    auto* sc = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(sc != nullptr);
    sc->SetProtocol("Renard");
    REQUIRE(sc->GetFirstOutput() != nullptr);
    CHECK(sc->GetFirstOutput()->GetType() == "Renard");
    CHECK_FALSE(sc->GetFirstOutput()->IsEnabled());
}
