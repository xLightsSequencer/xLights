#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "../support/ProtocolAccess.h"
#include "../support/TempDir.h"
#include "outputs/ControllerSerial.h"
#include "outputs/ControllerNull.h"

using namespace protocols_test;
using Bytes = std::vector<uint8_t>;

namespace {

struct TypeRow {
    const char* type;
    bool ip;
};

// every network type Output::Create can build without a controller (ZCPP needs one)
const TypeRow kTypes[] = {
    { "E131", true },          { "ArtNet", true },       { "DDP", true },          { "KINET", true },       { "OPC", true },
    { "Twinkly", true },       { "xxx Ethernet", true }, { "NULL", false },        { "DMX", false },        { "xxx Serial", false },
    { "Pixelnet", false },     { "LOR", false },         { "LOR Optimised", false }, { "D-Light", false }, { "Renard", false },
    { "OpenDMX", false },      { "Pixelnet-Open", false }, { "Generic Serial", false },
};

std::unique_ptr<Output> Make(const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    return std::unique_ptr<Output>(Output::Create(nullptr, doc.first_child(), ""));
}

std::string Net(const std::string& type, const std::string& attrs) {
    return "<network NetworkType=\"" + type + "\" " + attrs + "/>";
}

std::string E131Controller(const std::string& name, const std::string& ip, int firstUniverse, int universes, int channels = 510) {
    std::ostringstream x;
    x << "<Controller Id=\"" << 100 + firstUniverse << "\" Name=\"" << name << "\" Type=\"Ethernet\" Protocol=\"E131\" IP=\"" << ip << "\" ActiveState=\"Active\">";
    for (int i = 0; i < universes; ++i) {
        x << "<network NetworkType=\"E131\" ComPort=\"" << ip << "\" BaudRate=\"" << firstUniverse + i << "\" MaxChannels=\"" << channels << "\"/>";
    }
    x << "</Controller>";
    return x.str();
}

Controller* Add(OutputManager& om, const std::string& xml) {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(xml.c_str()));
    Controller* c = Controller::Create(&om, doc.first_child(), "");
    REQUIRE(c != nullptr);
    om.AddController(c, -1);
    om.SomethingChanged();
    return c;
}

} // namespace

// ---------------------------------------------------------------------------------------------
// XML loading of every output type
// ---------------------------------------------------------------------------------------------

TEST_CASE("Every output type loads from a node with no attributes", "[protocols][outputs][xml]") {
    const TypeRow row = GENERATE(from_range(std::begin(kTypes), std::end(kTypes)));
    INFO(row.type);
    auto o = Make(Net(row.type, ""));
    REQUIRE(o != nullptr);
    CHECK(o->GetType() == row.type);
    CHECK(o->IsEnabled());
    if (std::string(row.type) != "xxx Ethernet") CHECK_FALSE(o->IsDirty());
    CHECK(o->IsIpOutput() == row.ip);
    CHECK(o->IsSerialOutput() == (!row.ip && std::string(row.type) != "NULL"));
    CHECK(o->GetChannels() >= 0);
}

TEST_CASE("xxx Ethernet loaded without an Id attribute is not dirty", "[protocols][outputs][xml][!mayfail]") {
    // xxxEthernetOutput(node) calls SetId() after the base constructors cleared the dirty flag and never clears it again,
    // so an Id that differs from the BaudRate(universe) attribute leaves a freshly loaded output dirty.
    auto o = Make(Net("xxx Ethernet", "ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"30\""));
    REQUIRE(o != nullptr);
    CHECK_FALSE(o->IsDirty());
}

TEST_CASE("Every output type reads the Enabled attribute as exactly Yes", "[protocols][outputs][xml]") {
    const TypeRow row = GENERATE(from_range(std::begin(kTypes), std::end(kTypes)));
    struct Case { const char* v; bool enabled; };
    const Case c = GENERATE(Case{ "Yes", true }, Case{ "No", false }, Case{ "yes", false }, Case{ "YES", false }, Case{ "true", false },
                            Case{ "1", false }, Case{ "", false }, Case{ "Yes ", false });
    INFO(row.type << " Enabled='" << c.v << "'");
    auto o = Make(Net(row.type, std::string("Enabled=\"") + c.v + "\""));
    REQUIRE(o != nullptr);
    CHECK(o->IsEnabled() == c.enabled);
}

TEST_CASE("Every output type reads SuppressDuplicates as exactly Yes", "[protocols][outputs][xml]") {
    const TypeRow row = GENERATE(from_range(std::begin(kTypes), std::end(kTypes)));
    struct Case { const char* v; bool on; };
    const Case c = GENERATE(Case{ "Yes", true }, Case{ "No", false }, Case{ "yes", false }, Case{ "1", false }, Case{ "", false });
    INFO(row.type << " '" << c.v << "'");
    auto o = Make(Net(row.type, std::string("SuppressDuplicates=\"") + c.v + "\""));
    REQUIRE(o != nullptr);
    CHECK(o->IsSuppressDuplicateFrames() == c.on);
}

TEST_CASE("Every output type survives garbage numeric attributes", "[protocols][outputs][xml]") {
    const TypeRow row = GENERATE(from_range(std::begin(kTypes), std::end(kTypes)));
    const char* junk = GENERATE("abc", "", " ", "1e3", "0x10", "--5", "9999999999999999999", "1.5");
    INFO(row.type << " '" << junk << "'");
    auto o = Make(Net(row.type, std::string("MaxChannels=\"") + junk + "\" BaudRate=\"" + junk + "\" Id=\"" + junk + "\" HTTPPort=\"" + junk + "\" Version=\"" + junk + "\" Priority=\"" + junk + "\""));
    REQUIRE(o != nullptr);
    CHECK(o->GetType() == row.type);
}

TEST_CASE("Channel counts above the protocol maximum are clamped on load", "[protocols][outputs][xml]") {
    struct Case { const char* type; int max; };
    const Case c = GENERATE(Case{ "E131", 512 }, Case{ "ArtNet", 512 }, Case{ "OPC", 65535 });
    const char* v = GENERATE("513", "1000", "65536", "100000", "2147483647");
    auto o = Make(Net(c.type, std::string("ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"") + v + "\""));
    REQUIRE(o != nullptr);
    INFO(c.type << " " << v);
    CHECK(o->GetChannels() <= c.max);
    CHECK(o->GetChannels() >= std::min(atoi(v), c.max) );
}

TEST_CASE("Negative channel counts are not carried into the layout", "[protocols][outputs][xml][!mayfail]") {
    // MaxChannels="-1" (or an overflowing value) is stored as given, so GetChannels() is negative and
    // SetTransientData then moves the following outputs' start channel backwards.
    const char* type = GENERATE("E131", "ArtNet", "KINET", "DDP", "DMX", "Renard", "NULL", "Twinkly");
    const char* v = GENERATE("-1", "-100", "2147483648");
    auto o = Make(Net(type, std::string("ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"") + v + "\""));
    REQUIRE(o != nullptr);
    INFO(type << " " << v);
    CHECK(o->GetChannels() >= 0);
}

TEST_CASE("Unknown network types are not created", "[protocols][outputs][xml][.][known-crash]") {
    // Output::Create does assert(false) for an unknown NetworkType before returning nullptr, so a debug build aborts
    // on a hand edited / future-version file instead of skipping the output.
    auto o = Make(Net("Bogus", ""));
    CHECK(o == nullptr);
}

TEST_CASE("Output::Create maps the legacy Sy... Ethernet name", "[protocols][outputs][xml]") {
    for (const char* n : { "Sy Ethernet", "Syx Ethernet", "Sy-anything Ethernet" }) {
        auto o = Make(Net(n, "ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"30\""));
        INFO(n);
        REQUIRE(o != nullptr);
        CHECK(o->GetType() == OUTPUT_xxxETHERNET);
    }
}

// ---------------------------------------------------------------------------------------------
// Save / reload / copy
// ---------------------------------------------------------------------------------------------

namespace {
const std::map<std::string, std::string> kRealXml = {
    { "E131", "ComPort=\"10.1.2.3\" BaudRate=\"7\" MaxChannels=\"170\" Enabled=\"No\" Priority=\"42\"" },
    { "ArtNet", "ComPort=\"10.1.2.3\" BaudRate=\"4660\" MaxChannels=\"300\" ForceSourcePort=\"TRUE\"" },
    { "DDP", "ComPort=\"10.1.2.3\" BaudRate=\"64002\" MaxChannels=\"1500\" ChannelsPerPacket=\"480\" KeepChannelNumbers=\"0\"" },
    { "KINET", "ComPort=\"10.1.2.3\" BaudRate=\"3\" MaxChannels=\"200\" Version=\"2\"" },
    { "OPC", "ComPort=\"10.1.2.3\" BaudRate=\"2\" MaxChannels=\"900\"" },
    { "Twinkly", "ComPort=\"10.1.2.3\" BaudRate=\"1\" MaxChannels=\"900\" HTTPPort=\"8080\"" },
    { "xxx Ethernet", "ComPort=\"10.1.2.3\" BaudRate=\"5\" Id=\"5\" MaxChannels=\"90\"" },
    { "NULL", "Id=\"64005\" MaxChannels=\"77\"" },
    { "DMX", "ComPort=\"COM3\" BaudRate=\"250000\" Id=\"1\" MaxChannels=\"512\"" },
    { "xxx Serial", "ComPort=\"COM3\" BaudRate=\"n/a\" Id=\"2\" MaxChannels=\"16\" DeviceChannels=\"8,8\"" },
    { "Pixelnet", "ComPort=\"COM3\" BaudRate=\"n/a\" Id=\"3\" MaxChannels=\"200\"" },
    { "LOR", "ComPort=\"COM3\" BaudRate=\"19200\" Id=\"4\" MaxChannels=\"160\"" },
    { "D-Light", "ComPort=\"COM3\" BaudRate=\"19200\" Id=\"4\" MaxChannels=\"160\"" },
    { "Renard", "ComPort=\"COM3\" BaudRate=\"57600\" Id=\"5\" MaxChannels=\"96\"" },
    { "OpenDMX", "ComPort=\"COM3\" BaudRate=\"250000\" Id=\"6\" MaxChannels=\"512\"" },
    { "Pixelnet-Open", "ComPort=\"COM3\" BaudRate=\"1000000\" Id=\"7\" MaxChannels=\"300\"" },
    { "Generic Serial", "ComPort=\"COM3\" BaudRate=\"9600\" Id=\"8\" MaxChannels=\"100\"" },
};
} // namespace

TEST_CASE("Saved output xml reloads to the same xml", "[protocols][outputs][xml]") {
    const auto& kv = GENERATE_REF(from_range(kRealXml.begin(), kRealXml.end()));
    INFO(kv.first);
    auto a = Make(Net(kv.first, kv.second));
    REQUIRE(a != nullptr);
    const std::string once = SaveToString(*a);
    auto b = Make(once);
    REQUIRE(b != nullptr);
    CHECK(SaveToString(*b) == once);
    CHECK(b->GetChannels() == a->GetChannels());
    CHECK(b->IsEnabled() == a->IsEnabled());
    CHECK(b->GetType() == a->GetType());
}

TEST_CASE("Output Save clears the dirty flag and Create does not set it", "[protocols][outputs][xml]") {
    const auto& kv = GENERATE_REF(from_range(kRealXml.begin(), kRealXml.end()));
    auto a = Make(Net(kv.first, kv.second));
    REQUIRE(a != nullptr);
    CHECK_FALSE(a->IsDirty());
    a->SetUniverse(a->GetUniverse() + 1);
    CHECK(a->IsDirty());
    SaveToString(*a);
    CHECK_FALSE(a->IsDirty());
}

TEST_CASE("Copy of an output saves the same xml for network protocols", "[protocols][outputs][clone]") {
    const char* type = GENERATE("E131", "ArtNet", "DDP", "OPC", "Twinkly", "xxx Ethernet");
    auto a = Make(Net(type, kRealXml.at(type)));
    REQUIRE(a != nullptr);
    std::unique_ptr<Output> c(a->Copy());
    REQUIRE(c != nullptr);
    INFO(type);
    CHECK(SaveToString(*c) == SaveToString(*a));
    CHECK(c.get() != a.get());
    CHECK(*c == *a);
}

TEST_CASE("Copy of a serial output keeps the port, baud rate, size and enabled state", "[protocols][outputs][clone]") {
    const char* type = GENERATE("DMX", "xxx Serial", "Pixelnet", "LOR", "D-Light", "Renard", "OpenDMX", "Pixelnet-Open", "Generic Serial", "LOR Optimised");
    auto a = Make(Net(type, type == std::string("LOR Optimised") ? "ComPort=\"COM3\" BaudRate=\"19200\" Id=\"4\"" : kRealXml.at(type)));
    REQUIRE(a != nullptr);
    a->Enable(false);
    std::unique_ptr<Output> c(a->Copy());
    REQUIRE(c != nullptr);
    INFO(type);
    CHECK(c->GetCommPort() == "COM3");
    CHECK(c->GetBaudRate() == a->GetBaudRate());
    CHECK(c->GetChannels() == a->GetChannels());
    CHECK(c->IsEnabled() == false);
    CHECK(c->GetType() == a->GetType());
}

TEST_CASE("Copy of a serial or null output keeps its id", "[protocols][outputs][clone][!mayfail]") {
    // Output's copy constructor does not copy _universe, and SerialOutput / NullOutput do not add it back, so the Id
    // of a copied serial / NULL output reverts to 0.
    const char* type = GENERATE("DMX", "xxx Serial", "Pixelnet", "LOR", "Renard", "OpenDMX", "Pixelnet-Open", "Generic Serial", "NULL");
    auto a = Make(Net(type, kRealXml.at(type)));
    REQUIRE(a != nullptr);
    std::unique_ptr<Output> c(a->Copy());
    INFO(type);
    CHECK(c->GetUniverse() == a->GetUniverse());
}

TEST_CASE("Copy keeps proxy, suppress and enabled flags", "[protocols][outputs][clone]") {
    E131Output a;
    a.SetIP("1.2.3.4", false, false);
    a.SetFPPProxyIP("9.9.9.9");
    a.SetSuppressDuplicateFrames(true);
    a.Enable(false);
    a.SetForceLocalIP("5.5.5.5");
    std::unique_ptr<Output> c(a.Copy());
    CHECK(c->GetFPPProxyIP() == "9.9.9.9");
    CHECK(c->IsSuppressDuplicateFrames());
    CHECK_FALSE(c->IsEnabled());
    CHECK(c->GetForceLocalIP() == "5.5.5.5");
    CHECK(c->IsOk());
}

TEST_CASE("Copy is independent of the original", "[protocols][outputs][clone]") {
    E131Output a;
    a.SetIP("1.2.3.4", false, false);
    a.SetUniverse(5);
    a.SetChannels(100);
    std::unique_ptr<Output> c(a.Copy());
    c->SetUniverse(6);
    c->SetChannels(10);
    c->SetIP("4.3.2.1", false, false);
    CHECK(a.GetUniverse() == 5);
    CHECK(a.GetChannels() == 100);
    CHECK(a.GetIP() == "1.2.3.4");
}

TEST_CASE("Copy of a dirty output is dirty and a clean one is clean", "[protocols][outputs][clone]") {
    E131Output a;
    a.ClearDirty();
    std::unique_ptr<Output> c(a.Copy());
    CHECK_FALSE(c->IsDirty());
    a.SetUniverse(77);
    std::unique_ptr<Output> d(a.Copy());
    CHECK(d->IsDirty());
}

// ---------------------------------------------------------------------------------------------
// Comparison
// ---------------------------------------------------------------------------------------------

TEST_CASE("IP outputs compare by type, universe and address", "[protocols][outputs][compare]") {
    auto mk = [](const char* type, const char* ip, int uni) {
        return Make(Net(type, std::string("ComPort=\"") + ip + "\" BaudRate=\"" + std::to_string(uni) + "\" MaxChannels=\"10\""));
    };
    auto a = mk("E131", "10.0.0.1", 1);
    CHECK(*a == *mk("E131", "10.0.0.1", 1));
    CHECK_FALSE(*a == *mk("E131", "10.0.0.1", 2));
    CHECK_FALSE(*a == *mk("E131", "10.0.0.2", 1));
    CHECK_FALSE(*a == *mk("ArtNet", "10.0.0.1", 1));
    CHECK_FALSE(*a == *mk("DDP", "10.0.0.1", 1));
    CHECK_FALSE(*a == *mk("KINET", "10.0.0.1", 1));
    CHECK(*mk("ArtNet", "10.0.0.1", 1) == *mk("ArtNet", "10.0.0.1", 1));
}

TEST_CASE("Output comparison is symmetric", "[protocols][outputs][compare]") {
    E131Output a, b;
    a.SetIP("10.0.0.1", true);
    b.SetIP("10.0.0.1", true);
    a.SetUniverse(3);
    b.SetUniverse(3);
    CHECK((a == b) == (b == a));
    b.SetUniverse(4);
    CHECK((a == b) == (b == a));
    CHECK_FALSE(a == b);
}

TEST_CASE("Output comparison ignores channel counts and enabled state", "[protocols][outputs][compare]") {
    E131Output a, b;
    a.SetIP("10.0.0.1", true);
    b.SetIP("10.0.0.1", true);
    a.SetUniverse(3);
    b.SetUniverse(3);
    a.SetChannels(10);
    b.SetChannels(500);
    b.Enable(false);
    CHECK(a == b);
}

TEST_CASE("Outputs with a resolved address match the unresolved name", "[protocols][outputs][compare]") {
    E131Output a, b;
    a.SetIP("host.example", false, false);
    a.SetResolvedIP("10.9.9.9");
    b.SetIP("10.9.9.9", false, false);
    a.SetUniverse(1);
    b.SetUniverse(1);
    CHECK(a == b);
}

TEST_CASE("Serial outputs compare by port only", "[protocols][outputs][compare]") {
    auto mk = [](const char* type, const char* port, int id, int baud) {
        return Make(Net(type, std::string("ComPort=\"") + port + "\" BaudRate=\"" + std::to_string(baud) + "\" Id=\"" + std::to_string(id) + "\" MaxChannels=\"10\""));
    };
    CHECK(*mk("DMX", "COM1", 1, 250000) == *mk("DMX", "COM1", 2, 9600));
    CHECK_FALSE(*mk("DMX", "COM1", 1, 250000) == *mk("DMX", "COM2", 1, 250000));
    CHECK_FALSE(*mk("DMX", "COM1", 1, 250000) == *mk("Renard", "COM1", 1, 250000));
    CHECK_FALSE(*mk("LOR", "COM1", 1, 19200) == *mk("LOR Optimised", "COM1", 1, 19200));
}

TEST_CASE("NULL outputs with different ids do not compare equal", "[protocols][outputs][compare][!mayfail]") {
    // NullOutput is neither IP nor serial, so Output::operator== falls into the serial branch and compares the (empty)
    // comm ports: every NULL output equals every other NULL output.
    auto a = Make(Net("NULL", "Id=\"64001\" MaxChannels=\"10\""));
    auto b = Make(Net("NULL", "Id=\"64002\" MaxChannels=\"10\""));
    CHECK_FALSE(*a == *b);
}

TEST_CASE("Serial subclass comparison operator matches the base", "[protocols][outputs][compare]") {
    RenardOutput a, b;
    a.SetCommPort("COM1");
    b.SetCommPort("COM1");
    CHECK(a == b);
    b.SetCommPort("COM2");
    CHECK_FALSE(a == b);
}

// ---------------------------------------------------------------------------------------------
// Transient data / start channel arithmetic
// ---------------------------------------------------------------------------------------------

TEST_CASE("SetTransientData chains start channels across many outputs", "[protocols][outputs][layout]") {
    const int count = GENERATE(1, 2, 10, 255, 256, 257, 1000);
    std::vector<std::unique_ptr<E131Output>> outs;
    for (int i = 0; i < count; i++) {
        outs.emplace_back(new E131Output());
        outs.back()->SetChannels(1 + (i % 512));
    }
    int32_t start = 1;
    int64_t expected = 1;
    for (auto& o : outs) {
        o->SetTransientData(start, 0);
        REQUIRE(o->GetStartChannel() == expected);
        REQUIRE(o->GetEndChannel() == expected + o->GetChannels() - 1);
        expected += o->GetChannels();
        REQUIRE(start == expected);
    }
}

TEST_CASE("SetTransientData handles zero channel outputs", "[protocols][outputs][layout]") {
    E131Output a, b, c;
    a.SetChannels(10);
    b.SetChannels(0);
    c.SetChannels(5);
    int32_t start = 1;
    a.SetTransientData(start, 0);
    b.SetTransientData(start, 0);
    c.SetTransientData(start, 0);
    CHECK(b.GetStartChannel() == 11);
    CHECK(b.GetEndChannel() == 10);
    CHECK(c.GetStartChannel() == 11);
    CHECK(start == 16);
}

TEST_CASE("SetTransientData near the 32 bit limit", "[protocols][outputs][layout]") {
    E131Output a;
    a.SetChannels(512);
    int32_t start = 2147483647 - 512;
    a.SetTransientData(start, 0);
    CHECK(a.GetStartChannel() == 2147483647 - 512);
    CHECK(start == 2147483647);
}

TEST_CASE("Unplaced outputs report start channel -1", "[protocols][outputs][layout]") {
    E131Output a;
    a.SetChannels(10);
    CHECK(a.GetStartChannel() == -1);
    CHECK(a.GetEndChannel() == 8);
    NullOutput n;
    CHECK(n.GetStartChannel() == -1);
}

TEST_CASE("Output base SetTransientData numbers NULL outputs", "[protocols][outputs][layout]") {
    NullOutput n1, n2;
    int32_t start = 1;
    n1.SetTransientData(start, 3);
    n2.SetTransientData(start, 4);
    CHECK(n1.GetSortName() == "NULL03");
    CHECK(n2.GetSortName() == "NULL04");
    NullOutput unplaced;
    CHECK(unplaced.GetSortName() == "NULL-1");
}

TEST_CASE("NULL output ignores channel writes and always opens", "[protocols][outputs][null]") {
    NullOutput n;
    CHECK(n.Open());
    CHECK_FALSE(n.IsOutputable());
    n.SetOneChannel(0, 1);
    n.SetOneChannel(-5, 1);
    n.SetOneChannel(100000000, 1);
    unsigned char d[3] = { 1, 2, 3 };
    n.SetManyChannels(-1, d, 3);
    n.AllOff();
    n.StartFrame(5);
    n.EndFrame(0);
    n.Close();
    CHECK(n.GetMaxChannels() == 9999999);
    CHECK(n.IsValidChannelCount(1));
    CHECK_FALSE(n.IsValidChannelCount(0));
    CHECK(n.IsValidChannelCount(9999999));
}

TEST_CASE("NULL output description", "[protocols][outputs][null]") {
    NullOutput n;
    n.SetChannels(10);
    int32_t start = 21;
    n.SetTransientData(start, 0);
    CHECK(n.GetLongDescription() == "NULL (21-30)");
    n.Enable(false);
    CHECK(n.GetLongDescription() == "INACTIVE NULL (21-30)");
}

TEST_CASE("NULL output default id", "[protocols][outputs][null]") {
    NullOutput n;
    CHECK(n.GetId() == 64001);
    CHECK(n.GetChannels() == 512);
    auto x = Make(Net("NULL", ""));
    CHECK(x->GetUniverse() == 64001);
    n.SetId(5);
    CHECK(n.GetUniverse() == 5);
}

// ---------------------------------------------------------------------------------------------
// Ethernet controller sizing and universe numbering
// ---------------------------------------------------------------------------------------------

TEST_CASE("Ethernet SetChannelSize allocates ceil(channels / 510) universes", "[protocols][outputs][controller]") {
    const int channels = GENERATE(1, 509, 510, 511, 1019, 1020, 1021, 1530, 1531, 5100, 5101, 51000);
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    REQUIRE(c.SetChannelSize(channels));
    const int expected = (channels + 509) / 510;
    INFO("channels " << channels);
    CHECK(c.GetOutputCount() == expected);
    CHECK(c.GetChannels() == expected * 510);
    CHECK(c.GetChannels() >= channels);
    CHECK(c.GetChannels() - channels < 510);
}

TEST_CASE("Ethernet SetChannelSize of zero or negative keeps one universe", "[protocols][outputs][controller]") {
    const int channels = GENERATE(0, -1, -510, -100000);
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    REQUIRE(c.SetChannelSize(channels));
    CHECK(c.GetOutputCount() == 1);
}

TEST_CASE("Ethernet new universes continue the numbering of the last one", "[protocols][outputs][controller]") {
    const int first = GENERATE(1, 100, 255, 256, 32766, 63990);
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    c.GetFirstOutput()->SetUniverse(first);
    REQUIRE(c.SetChannelSize(510 * 5));
    REQUIRE(c.GetOutputCount() == 5);
    int u = first;
    for (const auto& o : c.GetOutputs()) {
        CHECK(o->GetUniverse() == u);
        u++;
    }
}

TEST_CASE("Ethernet SetChannelSize uses the first universe size for new ones", "[protocols][outputs][controller]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    c.GetFirstOutput()->SetChannels(170);
    REQUIRE(c.SetChannelSize(1000));
    CHECK(c.GetOutputCount() == 6);
    for (const auto& o : c.GetOutputs()) CHECK(o->GetChannels() == 170);
}

TEST_CASE("Ethernet SetChannelSize copies the settings to new universes", "[protocols][outputs][controller]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    c.SetFPPProxy("10.9.9.9");
    c.SetSuppressDuplicateFrames(true);
    REQUIRE(c.SetChannelSize(2000));
    for (const auto& o : c.GetOutputs()) {
        CHECK(o->GetIP() == "10.0.0.5");
        CHECK(o->GetFPPProxyIP() == "10.9.9.9");
        CHECK(o->IsSuppressDuplicateFrames());
        CHECK(o->IsEnabled());
    }
}

TEST_CASE("Ethernet SetChannelSize keeps the placed start channel", "[protocols][outputs][controller]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    int32_t sc = 1000;
    int nulls = 0;
    c.SetTransientData(sc, nulls);
    REQUIRE(c.SetChannelSize(1530));
    CHECK(c.GetStartChannel() == 1000);
    CHECK(c.GetOutput(1)->GetStartChannel() == 1510);
    CHECK(c.GetOutput(2)->GetStartChannel() == 2020);
    CHECK(c.GetEndChannel() == 1000 + 1530 - 1);
}

TEST_CASE("Ethernet SetChannelSize with a zero channel first universe", "[protocols][outputs][controller][.][known-crash]") {
    // channels_per_universe is taken from the first output; when that is 0 the universe count calculation divides by zero.
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    c.GetFirstOutput()->SetChannels(0);
    c.SetChannelSize(1000);
    SUCCEED();
}

TEST_CASE("Ethernet AddOutput numbers universes after the first", "[protocols][outputs][controller]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.SetIP("10.0.0.5");
    c.GetFirstOutput()->SetUniverse(10);
    c.GetFirstOutput()->SetChannels(300);
    for (int i = 0; i < 20; i++) c.AddOutput();
    REQUIRE(c.GetOutputCount() == 21);
    int u = 10;
    for (const auto& o : c.GetOutputs()) {
        CHECK(o->GetUniverse() == u++);
        CHECK(o->GetChannels() == 300);
        CHECK(o->GetIP() == "10.0.0.5");
    }
}

TEST_CASE("Ethernet controller with many universes lays out contiguous channels", "[protocols][outputs][controller]") {
    OutputManager om;
    auto* c = Add(om, E131Controller("Big", "10.0.0.9", 1, 200, 170));
    REQUIRE(c->GetOutputCount() == 200);
    CHECK(c->GetChannels() == 34000);
    CHECK(c->GetStartChannel() == 1);
    CHECK(c->GetEndChannel() == 34000);
    int i = 0;
    for (const auto& o : c->GetOutputs()) {
        REQUIRE(o->GetStartChannel() == 1 + i * 170);
        REQUIRE(o->GetEndChannel() == (i + 1) * 170);
        i++;
    }
}

TEST_CASE("Controller GetOutput by absolute channel at every universe boundary", "[protocols][outputs][controller]") {
    OutputManager om;
    auto* c = Add(om, E131Controller("Big", "10.0.0.9", 1, 50, 170));
    for (int u = 0; u < 50; u++) {
        int32_t rel = 0;
        const int first = u * 170 + 1;
        const int last = (u + 1) * 170;
        Output* o = c->GetOutput(first, rel);
        REQUIRE(o != nullptr);
        REQUIRE(o->GetUniverse() == u + 1);
        REQUIRE(rel == 1);
        o = c->GetOutput(last, rel);
        REQUIRE(o != nullptr);
        REQUIRE(o->GetUniverse() == u + 1);
        REQUIRE(rel == 170);
    }
    int32_t rel = 77;
    CHECK(c->GetOutput(0, rel) == nullptr);
    CHECK(c->GetOutput(-1, rel) == nullptr);
    CHECK(c->GetOutput(50 * 170 + 1, rel) == nullptr);
    CHECK(c->GetOutput(2147483647, rel) == nullptr);
    CHECK(c->GetOutput(-2147483647 - 1, rel) == nullptr);
    CHECK(rel == 77);
}

TEST_CASE("Controller ContainsChannels with many outputs", "[protocols][outputs][controller]") {
    OutputManager om;
    Add(om, E131Controller("A", "10.0.0.1", 1, 3));
    auto* b = Add(om, E131Controller("B", "10.0.0.2", 1, 4));
    REQUIRE(b->GetStartChannel() == 1531);
    REQUIRE(b->GetEndChannel() == 1530 + 2040);
    struct Row { uint32_t s, e; bool in; };
    const Row row = GENERATE(Row{ 1, 1530, false }, Row{ 1, 1531, true }, Row{ 1530, 1530, false }, Row{ 1531, 1531, true },
                             Row{ 3570, 3570, true }, Row{ 3571, 3571, false }, Row{ 3570, 5000, true }, Row{ 3571, 5000, false },
                             Row{ 0, 4000000000u, true }, Row{ 5, 4, false }, Row{ 0, 0, false }, Row{ 2000, 2100, true });
    INFO(row.s << "-" << row.e);
    CHECK(b->ContainsChannels(row.s, row.e) == row.in);
}

TEST_CASE("Empty controllers contain nothing and report zero channels", "[protocols][outputs][controller]") {
    OutputManager om;
    ControllerEthernet c(&om);
    c.DeleteAllOutputs();
    CHECK(c.GetOutputCount() == 0);
    CHECK(c.GetChannels() == 0);
    CHECK(c.GetStartChannel() == 0);
    CHECK(c.GetEndChannel() == 0);
    CHECK_FALSE(c.ContainsChannels(0, 100000));
    CHECK(c.GetOutput(0) == nullptr);
    CHECK(c.GetFirstOutput() == nullptr);
    int32_t rel;
    CHECK(c.GetOutput(5, rel) == nullptr);
    int32_t start = 5;
    int nulls = 0;
    c.SetTransientData(start, nulls);
    CHECK(start == 5);
}

TEST_CASE("Controller channel total sums outputs including empty ones", "[protocols][outputs][controller]") {
    OutputManager om;
    auto* c = Add(om, E131Controller("A", "10.0.0.1", 1, 3, 0));
    CHECK(c->GetChannels() == 0);
    CHECK(c->GetStartChannel() == 1);
    CHECK(c->GetEndChannel() == 0);
    // a controller covering no channels
    CHECK_FALSE(c->ContainsChannels(1, 1));
}

TEST_CASE("Controller SetTransientData applies active state and suppression to every output", "[protocols][outputs][controller]") {
    OutputManager om;
    auto* c = Add(om, E131Controller("A", "10.0.0.1", 1, 30));
    c->SetSuppressDuplicateFrames(true);
    c->SetActive("Inactive");
    int32_t start = 1;
    int nulls = 0;
    c->SetTransientData(start, nulls);
    for (const auto& o : c->GetOutputs()) {
        REQUIRE_FALSE(o->IsEnabled());
        REQUIRE(o->IsSuppressDuplicateFrames());
    }
    c->SetActive("Active");
    start = 1;
    c->SetTransientData(start, nulls);
    for (const auto& o : c->GetOutputs()) REQUIRE(o->IsEnabled());
}

// ---------------------------------------------------------------------------------------------
// OutputManager layout and routing
// ---------------------------------------------------------------------------------------------

TEST_CASE("OutputManager routes SetManyChannels across universe boundaries", "[protocols][outputs][manager]") {
    struct Row { int offset; int size; };
    const Row row = GENERATE(Row{ 0, 1 }, Row{ 0, 510 }, Row{ 0, 511 }, Row{ 505, 20 }, Row{ 509, 2 }, Row{ 510, 510 }, Row{ 509, 1022 },
                             Row{ 0, 1530 }, Row{ 1529, 1 }, Row{ 1529, 10 }, Row{ 1000, 530 });
    OutputManager om;
    auto* c = Add(om, E131Controller("A", "10.0.0.1", 1, 3));
    std::vector<unsigned char> data(row.size);
    for (int i = 0; i < row.size; i++) data[i] = (unsigned char)(1 + (row.offset + i) % 250);
    om.SetManyChannels(row.offset, data.data(), data.size());
    INFO("offset " << row.offset << " size " << row.size);
    for (int ch = 0; ch < 1530; ch++) {
        auto* e = dynamic_cast<E131Output*>(c->GetOutput(ch / 510));
        uint8_t got = PT(*e, e131_data)[126 + ch % 510];
        uint8_t want = (ch >= row.offset && ch < row.offset + row.size) ? (uint8_t)(1 + ch % 250) : 0;
        REQUIRE(got == want);
    }
}

TEST_CASE("OutputManager ignores writes outside every output", "[protocols][outputs][manager]") {
    OutputManager om;
    auto* c = Add(om, E131Controller("A", "10.0.0.1", 1, 2));
    unsigned char d[10];
    std::fill(d, d + 10, 9);
    om.SetManyChannels(-100, d, 10);
    om.SetManyChannels(1020, d, 10);
    om.SetManyChannels(100000, d, 10);
    om.SetManyChannels(5, d, 0);
    om.SetOneChannel(-1, 9);
    om.SetOneChannel(1020, 9);
    for (const auto& o : c->GetOutputs()) {
        auto* e = dynamic_cast<E131Output*>(o);
        for (int i = 0; i < 510; i++) REQUIRE(PT(*e, e131_data)[126 + i] == 0);
    }
}

TEST_CASE("OutputManager SetManyChannels running off the end is clipped", "[protocols][outputs][manager]") {
    OutputManager om;
    auto* c = Add(om, E131Controller("A", "10.0.0.1", 1, 2));
    std::vector<unsigned char> d(100, 7);
    om.SetManyChannels(1000, d.data(), d.size());
    auto* e = dynamic_cast<E131Output*>(c->GetOutput(1));
    CHECK(PT(*e, e131_data)[126 + 489] == 0);
    CHECK(PT(*e, e131_data)[126 + 490] == 7);
    CHECK(PT(*e, e131_data)[126 + 509] == 7);
}

TEST_CASE("OutputManager start channels are contiguous with mixed controllers", "[protocols][outputs][manager]") {
    OutputManager om;
    for (int i = 0; i < 30; i++) {
        Add(om, E131Controller("C" + std::to_string(i), "10.0." + std::to_string(i / 250) + "." + std::to_string(i % 250 + 1), 1, 1 + i % 4, 100 + i));
    }
    int32_t expected = 1;
    for (auto* c : om.GetControllers()) {
        REQUIRE(c->GetStartChannel() == expected);
        for (const auto& o : c->GetOutputs()) {
            REQUIRE(o->GetStartChannel() == expected);
            expected += o->GetChannels();
        }
        REQUIRE(c->GetEndChannel() == expected - 1);
    }
    CHECK(om.GetTotalChannels() == expected - 1);
}

TEST_CASE("OutputManager GetOutput by absolute channel around controller edges", "[protocols][outputs][manager]") {
    OutputManager om;
    Add(om, E131Controller("A", "10.0.0.1", 1, 2, 100));
    Add(om, E131Controller("B", "10.0.0.2", 1, 2, 50));
    struct Row { int32_t ch; int uni; const char* ip; int32_t rel; };
    const Row row = GENERATE(Row{ 1, 1, "10.0.0.1", 1 }, Row{ 100, 1, "10.0.0.1", 100 }, Row{ 101, 2, "10.0.0.1", 1 }, Row{ 200, 2, "10.0.0.1", 100 },
                             Row{ 201, 1, "10.0.0.2", 1 }, Row{ 250, 1, "10.0.0.2", 50 }, Row{ 251, 2, "10.0.0.2", 1 }, Row{ 300, 2, "10.0.0.2", 50 });
    int32_t rel = 0;
    Output* o = om.GetOutput(row.ch, rel);
    INFO("channel " << row.ch);
    REQUIRE(o != nullptr);
    CHECK(o->GetUniverse() == row.uni);
    CHECK(o->GetIP() == row.ip);
    CHECK(rel == row.rel);
    CHECK(om.GetOutput(301, rel) == nullptr);
    CHECK(om.GetOutput(0, rel) == nullptr);
    CHECK(om.GetOutput(-5, rel) == nullptr);
}

TEST_CASE("OutputManager DecodeStartChannel numeric forms", "[protocols][outputs][manager]") {
    OutputManager om;
    Add(om, E131Controller("A", "10.0.0.1", 1, 3));
    struct Row { const char* s; int32_t v; };
    const Row row = GENERATE(Row{ "1", 1 }, Row{ "0", 0 }, Row{ "00012", 12 }, Row{ " 5", 5 }, Row{ "5 ", 5 }, Row{ "12abc", 12 }, Row{ "#1:1", 1 },
                             Row{ "#3:510", 1530 }, Row{ "#3:511", 1531 }, Row{ "#2:1", 511 }, Row{ "#10.0.0.1:2:1", 511 }, Row{ "#10.0.0.9:2:1", 0 },
                             Row{ "!A:1", 1 }, Row{ "!A:1530", 1530 }, Row{ "!A:1531", 1531 }, Row{ "!a:1", 0 }, Row{ "!A:x", 0 }, Row{ "!A:1:2", 0 },
                             Row{ "#1:x", 0 }, Row{ "#x:1", 0 }, Row{ "#4:1", 0 }, Row{ "1:1", 0 }, Row{ "2:1", 0 }, Row{ ":5", 0 }, Row{ "#", 0 });
    INFO("'" << row.s << "'");
    CHECK(om.DecodeStartChannel(row.s) == row.v);
}

TEST_CASE("OutputManager DecodeStartChannel rejects values that do not fit 32 bits or are negative", "[protocols][outputs][manager][!mayfail]") {
    // the absolute form returns strtol() truncated to int32_t: "2147483648" becomes a large negative number and
    // "-5" is returned as a negative start channel instead of the documented 0 for an error.
    OutputManager om;
    const char* s = GENERATE("2147483648", "4294967297", "99999999999", "-5", "-1");
    INFO(s);
    CHECK(om.DecodeStartChannel(s) >= 0);
    CHECK(om.DecodeStartChannel(s) != -2147483647 - 1);
}

TEST_CASE("OutputManager universe lookups with several controllers sharing universe numbers", "[protocols][outputs][manager]") {
    OutputManager om;
    Add(om, E131Controller("A", "10.0.0.1", 1, 2));
    Add(om, E131Controller("B", "10.0.0.2", 1, 2));
    auto* byIp = om.GetOutput(2, "10.0.0.2");
    REQUIRE(byIp != nullptr);
    CHECK(byIp->GetIP() == "10.0.0.2");
    auto* any = om.GetOutput(2, "");
    REQUIRE(any != nullptr);
    CHECK(any->GetIP() == "10.0.0.1");
    CHECK(om.GetOutput(3, "") == nullptr);
    CHECK(om.GetOutput(0, "") == nullptr);
    CHECK(om.GetOutput(-1, "") == nullptr);
    CHECK(om.GetOutput(2, "9.9.9.9") == nullptr);
}

TEST_CASE("OutputManager disabled controllers leave a hole in the data routing", "[protocols][outputs][manager]") {
    OutputManager om;
    auto* a = Add(om, E131Controller("A", "10.0.0.1", 1, 1));
    Add(om, E131Controller("B", "10.0.0.2", 1, 1));
    a->SetActive("Inactive");
    om.SomethingChanged();
    std::vector<unsigned char> d(1020, 3);
    om.SetManyChannels(0, d.data(), d.size());
    auto* e0 = dynamic_cast<E131Output*>(a->GetOutput(0));
    auto* e1 = dynamic_cast<E131Output*>(om.GetController("B")->GetOutput(0));
    CHECK(PT(*e0, e131_data)[126] == 0);
    CHECK(PT(*e1, e131_data)[126] == 3);
    CHECK(PT(*e1, e131_data)[126 + 509] == 3);
}

// ---------------------------------------------------------------------------------------------
// Serial prefix / postfix text escapes
// ---------------------------------------------------------------------------------------------

TEST_CASE("Serial controller Encode table", "[protocols][outputs][serial]") {
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)"));
    std::unique_ptr<Controller> c(Controller::Create(&om, doc.first_child(), ""));
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    struct Row { const char* in; Bytes out; };
    const Row row = GENERATE(Row{ "\\x41\\x42", { 0x41, 0x42 } }, Row{ "\\X41", { 0x41 } }, Row{ "\\x0a", { 0x0A } }, Row{ "\\x00", { 0x00 } },
                             Row{ "\\xff", { 0xFF } }, Row{ "\\xFF", { 0xFF } }, Row{ "\\x", {} }, Row{ "\\", {} }, Row{ "a\\", { 'a' } },
                             Row{ "\\x4", {} }, Row{ "\\x4G", {} }, Row{ "\\xG4", { '4' } }, Row{ "\\x414", { 0x41, '4' } }, Row{ "a\\nb", { 'a', '\n', 'b' } },
                             Row{ "\\\\n", { '\\', 'n' } }, Row{ "\\\\\\\\", { '\\', '\\' } }, Row{ "\\t\\t", { '\t', '\t' } },
                             Row{ "\\x01\\x02", { 1, 2 } }, Row{ "\\x01 \\x02", { 1, ' ', 2 } }, Row{ "\xC3\xA9", { 0xC3, 0xA9 } }, Row{ "\\q\\x41", { 0x41 } });
    INFO("'" << row.in << "'");
    CHECK(s->Encode(row.in) == row.out);
}

TEST_CASE("Serial controller Encode keeps the backslash after a malformed hex escape", "[protocols][outputs][serial][!mayfail]") {
    // "\\x4\\n": the one digit \\x4 is malformed, and the state machine drops the *next* character (the backslash that starts
    // \\n) instead of re-reading it, so the newline escape turns into a literal 'n'.
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)"));
    std::unique_ptr<Controller> c(Controller::Create(&om, doc.first_child(), ""));
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    CHECK(s->Encode("\\x4\\n") == Bytes{ '\n' });
}

TEST_CASE("Serial controller Encode of long input", "[protocols][outputs][serial]") {
    OutputManager om;
    pugi::xml_document doc;
    REQUIRE(doc.load_string(R"(<Controller Id="1" Name="s" Type="Serial" Protocol="DMX" Port="COM3" Speed="250000" ActiveState="Inactive"><network NetworkType="DMX" Id="1" ComPort="COM3" BaudRate="250000" MaxChannels="512"/></Controller>)"));
    std::unique_ptr<Controller> c(Controller::Create(&om, doc.first_child(), ""));
    auto* s = dynamic_cast<ControllerSerial*>(c.get());
    REQUIRE(s != nullptr);
    std::string in;
    for (int i = 0; i < 5000; i++) in += "\\x7e";
    CHECK(s->Encode(in).size() == 5000);
    CHECK(s->Encode(std::string(100000, 'x')).size() == 100000);
}
