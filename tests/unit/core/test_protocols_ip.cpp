#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>
#include <catch2/catch_approx.hpp>

#include "../support/ProtocolAccess.h"
#include "../support/TempDir.h"

using namespace protocols_test;
using Bytes = std::vector<uint8_t>;

namespace {

template <class T>
void Prep(T& o, int universe, int channels, const char* ip = "127.0.0.1") {
    o.SetIP(ip, true);
    o.SetUniverse(universe);
    o.SetChannels(channels);
}

template <class T, class DG>
bool HasSocket(T& o, DG which) {
    return (o.*which) != nullptr;
}

// DDPOutput allocates its buffer with malloc but its destructor uses delete, so close every opened output first.
struct DdpGuard {
    DDPOutput& o;
    explicit DdpGuard(DDPOutput& out) : o(out) {}
    ~DdpGuard() { o.Close(); }
};

#define REQUIRE_SOCKET(expr) \
    do { if ((expr) == nullptr) SKIP("UDP sockets are not available here"); } while (0)

} // namespace

// ---------------------------------------------------------------------------------------------
// E1.31
// ---------------------------------------------------------------------------------------------

TEST_CASE("E131 packet header layout", "[protocols][ip][e131]") {
    const int channels = GENERATE(1, 2, 255, 256, 257, 510, 511, 512);
    const int universe = GENERATE(1, 2, 255, 256, 257, 4095, 63999, 65535);
    E131Output o;
    Prep(o, universe, channels);
    o.SetPriority(150);
    o.Open();
    auto& d = PT(o, e131_data);
    INFO("channels " << channels << " universe " << universe);

    CHECK(d[0] == 0x00);
    CHECK(d[1] == 0x10);
    CHECK(d[2] == 0x00);
    CHECK(d[3] == 0x00);
    CHECK(std::string((char*)&d[4], 9) == "ASC-E1.17");
    CHECK(d[13] == 0);
    CHECK(d[14] == 0);
    CHECK(d[15] == 0);
    // root layer PDU: flags 0x7 + 12 bit length of (packet - 16)
    const int rootLen = (126 + channels) - 16;
    CHECK(d[16] == (0x70 | (rootLen >> 8)));
    CHECK(d[17] == (rootLen & 0xFF));
    CHECK(d[21] == 0x04);
    // framing layer
    const int frameLen = (126 + channels) - 38;
    CHECK(d[38] == (0x70 | (frameLen >> 8)));
    CHECK(d[39] == (frameLen & 0xFF));
    CHECK(d[43] == 0x02);
    CHECK(d[108] == 150);
    CHECK(d[109] == 0);
    CHECK(d[110] == 0);
    CHECK(d[112] == 0);
    CHECK(d[113] == (universe >> 8));
    CHECK(d[114] == (universe & 0xFF));
    // DMP layer
    const int dmpLen = (126 + channels) - 115;
    CHECK(d[115] == (0x70 | (dmpLen >> 8)));
    CHECK(d[116] == (dmpLen & 0xFF));
    CHECK(d[117] == 0x02);
    CHECK(d[118] == 0xA1);
    CHECK(d[119] == 0);
    CHECK(d[120] == 0);
    CHECK(d[121] == 0);
    CHECK(d[122] == 0x01);
    CHECK(d[123] == ((channels + 1) >> 8));
    CHECK(d[124] == ((channels + 1) & 0xFF));
    CHECK(d[125] == 0);
}

TEST_CASE("E131 full universe lengths match the spec constants", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 512);
    o.Open();
    auto& d = PT(o, e131_data);
    CHECK(d[16] == 0x72);
    CHECK(d[17] == 0x6E);
    CHECK(d[38] == 0x72);
    CHECK(d[39] == 0x58);
    CHECK(d[115] == 0x72);
    CHECK(d[116] == 0x0B);
    CHECK(d[123] == 0x02);
    CHECK(d[124] == 0x01);
}

TEST_CASE("E131 CID is the xLights UUID", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    auto& d = PT(o, e131_data);
    const Bytes expect = { 0xc0, 0xde, 0x00, 0x80, 0xc6, 0x9b, 0x11, 0xe0, 0x95, 0x72, 0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66 };
    CHECK(Bytes(&d[22], &d[38]) == expect);
}

TEST_CASE("E131 source name is the per-process tag and terminated", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    auto& d = PT(o, e131_data);
    std::string name((char*)&d[44]);
    CHECK(name == E131Output::GetTag());
    CHECK(name.rfind("xLights ", 0) == 0);
    CHECK(d[107] == 0);
}

TEST_CASE("E131 Open requires an enabled output with a valid address", "[protocols][ip][e131]") {
    E131Output o;
    o.SetChannels(10);
    CHECK_FALSE(o.Open());
    o.SetIP("not an ip", true, false);
    CHECK_FALSE(o.Open());
    o.SetIP("127.0.0.1", true);
    o.Enable(false);
    CHECK(o.Open());
}

TEST_CASE("E131 disabled Open leaves the packet empty", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 3, 10);
    o.Enable(false);
    o.Open();
    auto& d = PT(o, e131_data);
    CHECK(d[1] == 0);
    CHECK(d[113] == 0);
}

TEST_CASE("E131 multicast addresses are derived from the universe", "[protocols][ip][e131]") {
    const int universe = GENERATE(1, 255, 256, 257, 65535);
    E131Output o;
    o.SetIP("MULTICAST", true);
    o.SetUniverse(universe);
    o.SetChannels(10);
    o.Open();
    CHECK(PT(o, e131_remote) == "239.255." + std::to_string(universe >> 8) + "." + std::to_string(universe & 0xFF));
}

TEST_CASE("E131 239.255.x.x addresses are also treated as multicast", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 0x0102, 10, "239.255.9.9");
    o.Open();
    CHECK(PT(o, e131_remote) == "239.255.1.2");
}

TEST_CASE("E131 unicast keeps the resolved address", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 7, 10, "10.20.30.40");
    o.Open();
    CHECK(PT(o, e131_remote) == "10.20.30.40");
}

TEST_CASE("E131 sequence numbers count up and wrap at 256", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    REQUIRE_SOCKET(PT(o, e131_dg));
    auto& d = PT(o, e131_data);
    for (int i = 0; i < 600; i++) {
        o.EndFrame(0);
        INFO("frame " << i);
        REQUIRE(d[111] == (i & 0xFF));
        REQUIRE(PT(o, e131_seq) == ((i + 1) & 0xFF));
    }
}

TEST_CASE("E131 sequence number restarts when reopened", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    REQUIRE_SOCKET(PT(o, e131_dg));
    for (int i = 0; i < 5; i++) o.EndFrame(0);
    CHECK(PT(o, e131_seq) == 5);
    o.Close();
    o.Open();
    CHECK(PT(o, e131_seq) == 0);
}

TEST_CASE("E131 suppressed duplicate frames do not advance the sequence", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.SetSuppressDuplicateFrames(true);
    o.Open();
    REQUIRE_SOCKET(PT(o, e131_dg));
    o.EndFrame(5);
    CHECK(PT(o, e131_seq) == 1);
    for (int i = 0; i < 5; i++) {
        o.EndFrame(5);
        CHECK(PT(o, e131_seq) == 1);
    }
    o.EndFrame(5);
    CHECK(PT(o, e131_seq) == 2);
    o.SetOneChannel(0, 1);
    o.EndFrame(5);
    CHECK(PT(o, e131_seq) == 3);
}

TEST_CASE("E131 EndFrame does nothing when disabled, suspended or temp disabled", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    REQUIRE_SOCKET(PT(o, e131_dg));
    o.Suspend(true);
    o.EndFrame(0);
    o.Suspend(false);
    o.TempDisable(true);
    o.EndFrame(0);
    o.TempDisable(false);
    o.Enable(false);
    o.EndFrame(0);
    CHECK(PT(o, e131_seq) == 0);
    o.Enable(true);
    o.EndFrame(0);
    CHECK(PT(o, e131_seq) == 1);
}

TEST_CASE("E131 EndFrame without a socket is harmless", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.EndFrame(0);
    CHECK(PT(o, e131_seq) == 0);
}

TEST_CASE("E131 channel data lands after the 126 byte header", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 512);
    o.Open();
    auto& d = PT(o, e131_data);
    o.SetOneChannel(0, 11);
    o.SetOneChannel(511, 22);
    CHECK(d[126] == 11);
    CHECK(d[637] == 22);
    CHECK(o.GetStartChannel() == -1);
}

TEST_CASE("E131 SetManyChannels boundaries", "[protocols][ip][e131]") {
    struct Row { int channel; int size; int expectedCopied; };
    const Row row = GENERATE(Row{ 0, 0, 0 }, Row{ 0, 1, 1 }, Row{ 0, 512, 512 }, Row{ 0, 600, 512 }, Row{ 100, 412, 412 },
                             Row{ 100, 500, 412 }, Row{ 511, 1, 1 }, Row{ 511, 5, 1 }, Row{ 512, 5, 0 }, Row{ -1, 5, 0 },
                             Row{ 1000, 5, 0 }, Row{ -100000, 1, 0 });
    E131Output o;
    Prep(o, 1, 512);
    o.Open();
    auto& d = PT(o, e131_data);
    std::vector<unsigned char> src(std::max(row.size, 1), 0xEE);
    o.SetManyChannels(row.channel, src.data(), row.size);
    int copied = 0;
    for (int i = 126; i < 638; i++) copied += d[i] == 0xEE;
    INFO("channel " << row.channel << " size " << row.size);
    CHECK(copied == row.expectedCopied);
    if (row.expectedCopied == 0) CHECK_FALSE(PT(o, out_changed));
    else CHECK(PT(o, out_changed));
    // the header is never touched
    CHECK(d[125] == 0);
    CHECK(d[113] == 0);
}

TEST_CASE("E131 SetManyChannels with identical data does not mark changed", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 100);
    o.Open();
    unsigned char zeros[10] = { 0 };
    o.SetManyChannels(0, zeros, 10);
    CHECK_FALSE(PT(o, out_changed));
    unsigned char one[10] = { 0, 0, 1 };
    o.SetManyChannels(0, one, 10);
    CHECK(PT(o, out_changed));
}

TEST_CASE("E131 AllOff clears only the configured channels", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    auto& d = PT(o, e131_data);
    std::vector<unsigned char> src(512, 0x77);
    o.SetManyChannels(0, src.data(), 512);
    o.AllOff();
    for (int i = 0; i < 10; i++) CHECK(d[126 + i] == 0);
    CHECK(d[136] == 0x77);
}

TEST_CASE("E131 disabled output ignores data", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 1, 10);
    o.Open();
    o.Enable(false);
    o.SetOneChannel(0, 5);
    unsigned char x[3] = { 1, 2, 3 };
    o.SetManyChannels(0, x, 3);
    o.AllOff();
    CHECK(PT(o, e131_data)[126] == 0);
    CHECK_FALSE(PT(o, out_changed));
}

TEST_CASE("E131 priority is stored in 8 bits", "[protocols][ip][e131]") {
    // _priority is a uint8_t, so SetPriority(300) and Priority="300" wrap modulo 256 instead of being rejected or
    // clamped to the E1.31 range of 0-200.
    E131Output o;
    o.SetPriority(500);
    CHECK(o.GetPriority() <= 200);
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="E131" ComPort="1.2.3.4" BaudRate="1" MaxChannels="10" Priority="-1"/>)");
    E131Output x(n, true);
    CHECK(x.GetPriority() >= 0);
    CHECK(x.GetPriority() <= 200);
}

TEST_CASE("E131 priority values 0 to 200 are written into the packet", "[protocols][ip][e131]") {
    const int prio = GENERATE(0, 1, 100, 199, 200);
    E131Output o;
    Prep(o, 1, 10);
    o.SetPriority(prio);
    o.Open();
    CHECK(PT(o, e131_data)[108] == prio);
}

TEST_CASE("E131 NumUniverses expands to consecutive sub outputs", "[protocols][ip][e131]") {
    const int n = GENERATE(2, 3, 10);
    pugi::xml_document doc;
    auto node = ParseNode(doc, "<network NetworkType=\"E131\" ComPort=\"10.0.0.1\" BaudRate=\"5\" MaxChannels=\"100\" NumUniverses=\"" + std::to_string(n) + "\"/>");
    E131Output o(node, true);
    CHECK(o.IsOutputCollection_CONVERT());
    CHECK(o.GetUniverses_CONVERT() == n);
    auto outs = o.GetOutputs_CONVERT();
    REQUIRE((int)outs.size() == n);
    int u = 5;
    for (auto* s : outs) {
        CHECK(s->GetUniverse() == u++);
        CHECK(s->GetChannels() == 100);
        CHECK(s->GetIP() == "10.0.0.1");
    }
}

TEST_CASE("E131 NumUniverses of 0, 1 or garbage is not a collection", "[protocols][ip][e131]") {
    const char* v = GENERATE("0", "1", "-4", "abc", "");
    pugi::xml_document doc;
    auto node = ParseNode(doc, std::string("<network NetworkType=\"E131\" ComPort=\"10.0.0.1\" BaudRate=\"5\" MaxChannels=\"100\" NumUniverses=\"") + v + "\"/>");
    E131Output o(node, true);
    INFO(v);
    CHECK_FALSE(o.IsOutputCollection_CONVERT());
    CHECK(o.GetOutputs_CONVERT().empty());
}

TEST_CASE("E131 SetTransientData advances by the channel count", "[protocols][ip][e131]") {
    E131Output o;
    o.SetChannels(300);
    int32_t start = 1000;
    o.SetTransientData(start, 0);
    CHECK(o.GetStartChannel() == 1000);
    CHECK(o.GetEndChannel() == 1299);
    CHECK(start == 1300);
}

TEST_CASE("E131 export row", "[protocols][ip][e131]") {
    E131Output o;
    Prep(o, 12, 34, "1.2.3.4");
    int32_t start = 5;
    o.SetTransientData(start, 0);
    CHECK(o.GetExport() == ",5,38,,E131,1.2.3.4,,,,12,34");
    CHECK(o.GetLongDescription() == "E1.31 {12} [1-34] (5-38)");
    o.Enable(false);
    CHECK(o.GetLongDescription().rfind("INACTIVE ", 0) == 0);
}

// ---------------------------------------------------------------------------------------------
// ArtNet
// ---------------------------------------------------------------------------------------------

TEST_CASE("ArtNet ArtDmx header layout", "[protocols][ip][artnet]") {
    const int channels = GENERATE(1, 2, 255, 256, 510, 511, 512);
    const int universe = GENERATE(0, 1, 15, 16, 255, 256, 0x1234, 0x7FFF);
    ArtNetOutput o;
    Prep(o, universe, channels);
    o.Open();
    auto& d = PT(o, art_data);
    INFO("channels " << channels << " universe " << universe);
    CHECK(std::string((char*)d, 7) == "Art-Net");
    CHECK(d[7] == 0);
    CHECK(d[8] == 0x00);
    CHECK(d[9] == 0x50);
    CHECK(d[10] == 0);
    CHECK(d[11] == 14);
    CHECK(d[12] == 0);
    CHECK(d[13] == 0);
    CHECK(d[14] == (universe & 0xFF));
    CHECK(d[15] == ((universe >> 8) & 0xFF));
    CHECK(d[16] == (channels >> 8));
    CHECK(d[17] == (channels & 0xFF));
}

TEST_CASE("ArtNet sequence numbers wrap at 256", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    Prep(o, 1, 10);
    o.Open();
    REQUIRE_SOCKET(PT(o, art_dg));
    auto& d = PT(o, art_data);
    for (int i = 0; i < 520; i++) {
        o.SetOneChannel(0, (unsigned char)(i + 1));
        o.EndFrame(0);
        REQUIRE(d[12] == (i & 0xFF));
        REQUIRE(PT(o, art_seq) == ((i + 1) & 0xFF));
    }
}

TEST_CASE("ArtNet EndFrame clears the changed flag", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    Prep(o, 1, 10);
    o.SetSuppressDuplicateFrames(true);
    o.Open();
    REQUIRE_SOCKET(PT(o, art_dg));
    o.SetOneChannel(1, 4);
    CHECK(PT(o, out_changed));
    o.EndFrame(100);
    CHECK_FALSE(PT(o, out_changed));
    o.EndFrame(100);
    CHECK(PT(o, art_seq) == 1);
}

TEST_CASE("ArtNet channel data starts at byte 18", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    Prep(o, 1, 512);
    o.Open();
    auto& d = PT(o, art_data);
    o.SetOneChannel(0, 1);
    o.SetOneChannel(511, 2);
    CHECK(d[18] == 1);
    CHECK(d[529] == 2);
}

TEST_CASE("ArtNet SetManyChannels clamps to the channel count and ignores bad channels", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    Prep(o, 1, 100);
    o.Open();
    auto& d = PT(o, art_data);
    unsigned char src[100];
    std::fill(src, src + 100, 0xAB);
    o.SetManyChannels(0, src, 100);
    CHECK(d[18] == 0xAB);
    CHECK(d[117] == 0xAB);
    CHECK(d[118] == 0);
    o.AllOff();
    CHECK(d[18] == 0);
    CHECK(d[117] == 0);
}

TEST_CASE("ArtNet Open needs a valid address", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    o.SetChannels(10);
    CHECK_FALSE(o.Open());
    o.SetIP("999.1.1.1", true, false);
    CHECK_FALSE(o.Open());
    o.SetIP("127.0.0.1", true);
    o.Enable(false);
    CHECK(o.Open());
}

TEST_CASE("ArtNet universe string shows net subnet and universe", "[protocols][ip][artnet]") {
    struct Row { int u; const char* s; };
    const Row row = GENERATE(Row{ 0, "0:0:0 or 0" }, Row{ 1, "0:0:1 or 1" }, Row{ 16, "0:1:0 or 16" }, Row{ 256, "1:0:0 or 256" },
                             Row{ 0x7FFF, "127:15:15 or 32767" }, Row{ 0x1234, "18:3:4 or 4660" });
    ArtNetOutput o;
    o.SetUniverse(row.u);
    CHECK(o.GetUniverseString() == row.s);
}

TEST_CASE("ArtNet SetArtNetUniverse masks each field", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    o.SetArtNetUniverse(0x1FF, 0x1F, 0x1F);
    CHECK(o.GetUniverse() == 0x7FFF);
    o.SetArtNetUniverse(0, 0, 0);
    CHECK(o.GetUniverse() == 0);
    o.SetArtNetUniverse(-1, -1, -1);
    CHECK(o.GetUniverse() == 0x7FFF);
}

TEST_CASE("ArtNet universe pieces round trip for every net subnet and universe", "[protocols][ip][artnet]") {
    int bad = 0;
    for (int net = 0; net < 128; net++)
        for (int sub = 0; sub < 16; sub++)
            for (int uni = 0; uni < 16; uni++) {
                int c = ArtNetOutput::GetArtNetCombinedUniverse(net, sub, uni);
                bad += !(ArtNetOutput::GetArtNetNet(c) == net && ArtNetOutput::GetArtNetSubnet(c) == sub && ArtNetOutput::GetArtNetUniverse(c) == uni);
            }
    CHECK(bad == 0);
}

TEST_CASE("ArtNet universe accessors on out of range values", "[protocols][ip][artnet]") {
    CHECK(ArtNetOutput::GetArtNetNet(0xFFFF) == 0x7F);
    CHECK(ArtNetOutput::GetArtNetNet(-1) == 0x7F);
    CHECK(ArtNetOutput::GetArtNetSubnet(-1) == 0xF);
    CHECK(ArtNetOutput::GetArtNetUniverse(-1) == 0xF);
    CHECK(ArtNetOutput::GetArtNetNet(0x8000) == 0);
}

TEST_CASE("ArtNet ForceSourcePort attribute is exactly TRUE", "[protocols][ip][artnet]") {
    struct Row { const char* v; bool expect; };
    const Row row = GENERATE(Row{ "TRUE", true }, Row{ "true", false }, Row{ "1", false }, Row{ "FALSE", false }, Row{ "", false }, Row{ "yes", false });
    pugi::xml_document doc;
    auto n = ParseNode(doc, std::string("<network NetworkType=\"ArtNet\" ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"10\" ForceSourcePort=\"") + row.v + "\"/>");
    ArtNetOutput o(n, true);
    CHECK(o.isForceSourcePort() == row.expect);
    auto s = SaveToString(o);
    CHECK((s.find("ForceSourcePort=\"TRUE\"") != std::string::npos) == row.expect);
}

TEST_CASE("ArtNet clamps oversized channel counts from xml", "[protocols][ip][artnet]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="ArtNet" ComPort="1.1.1.1" BaudRate="1" MaxChannels="100000"/>)");
    ArtNetOutput o(n, true);
    CHECK(o.GetChannels() == 512);
}

TEST_CASE("ArtNet copy keeps force source port and starts a fresh sequence", "[protocols][ip][artnet]") {
    ArtNetOutput o;
    Prep(o, 77, 33, "4.3.2.1");
    o.SetForceSourcePort(true);
    std::unique_ptr<Output> c(o.Copy());
    auto* a = dynamic_cast<ArtNetOutput*>(c.get());
    REQUIRE(a != nullptr);
    CHECK(a->isForceSourcePort());
    CHECK(a->GetUniverse() == 77);
    CHECK(a->GetChannels() == 33);
    CHECK(a->GetIP() == "4.3.2.1");
    CHECK(PT(*a, art_seq) == 0);
}

// ---------------------------------------------------------------------------------------------
// DDP
// ---------------------------------------------------------------------------------------------

TEST_CASE("DDP header after Open", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 300);
    o.Open();
    auto& d = PT(o, ddp_data);
    CHECK(d[2] == 0);
    CHECK(d[3] == DDP_ID_DISPLAY);
    CHECK(PT(o, ddp_seq) == 1);
    CHECK(PT(o, ddp_full) != nullptr);
}

TEST_CASE("DDP splits channels into packets of the configured size", "[protocols][ip][ddp]") {
    struct Row { int channels; int cpp; int lastLen; int lastOffset; int packets; };
    const Row row = GENERATE(Row{ 1, 1440, 1, 0, 1 }, Row{ 1440, 1440, 1440, 0, 1 }, Row{ 1441, 1440, 1, 1440, 2 },
                             Row{ 2880, 1440, 1440, 1440, 2 }, Row{ 3000, 1440, 120, 2880, 3 }, Row{ 512, 512, 512, 0, 1 },
                             Row{ 513, 512, 1, 512, 2 }, Row{ 511, 512, 511, 0, 1 }, Row{ 100, 7, 2, 98, 15 },
                             Row{ 10, 1, 1, 9, 10 });
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, row.channels);
    o.SetChannelsPerPacket(row.cpp);
    o.SetKeepChannelNumber(false);
    o.Open();
    REQUIRE_SOCKET(PT(o, ddp_dg));
    auto* full = PT(o, ddp_full);
    for (int i = 0; i < row.channels; i++) full[i] = (uint8_t)(i * 7 + 1);
    o.EndFrame(0);
    auto& d = PT(o, ddp_data);
    INFO("channels " << row.channels << " cpp " << row.cpp);
    CHECK(d[0] == (DDP_FLAGS1_VER1 | DDP_FLAGS1_PUSH));
    CHECK(((d[4] << 24) | (d[5] << 16) | (d[6] << 8) | d[7]) == row.lastOffset);
    CHECK(((d[8] << 8) | d[9]) == row.lastLen);
    CHECK((d[1] & 0xF0) == 0);
    for (int i = 0; i < row.lastLen; i++) {
        REQUIRE(d[10 + i] == (uint8_t)((row.lastOffset + i) * 7 + 1));
    }
    // sequence runs 1..15 then back to 1, never 0
    int expectedSeq = 1;
    for (int i = 0; i < row.packets; i++) expectedSeq = expectedSeq == 15 ? 1 : expectedSeq + 1;
    CHECK(PT(o, ddp_seq) == expectedSeq);
    CHECK(d[1] != 0x00);
}

TEST_CASE("DDP sequence never uses zero", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 10);
    o.Open();
    REQUIRE_SOCKET(PT(o, ddp_dg));
    for (int i = 0; i < 100; i++) {
        o.SetOneChannel(0, (unsigned char)(i + 1));
        o.EndFrame(0);
        auto seq = PT(o, ddp_data)[1] & 0x0F;
        REQUIRE(seq == (i % 15) + 1);
    }
}

TEST_CASE("DDP keep channel numbers makes offsets absolute", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 100);
    o.SetChannelsPerPacket(1440);
    o.SetKeepChannelNumber(true);
    PT(o, out_start) = 1001;
    o.Open();
    REQUIRE_SOCKET(PT(o, ddp_dg));
    o.EndFrame(0);
    auto& d = PT(o, ddp_data);
    CHECK(((d[4] << 24) | (d[5] << 16) | (d[6] << 8) | d[7]) == 1000);
}

TEST_CASE("DDP offset uses all four bytes", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 10);
    o.SetChannelsPerPacket(1440);
    o.SetKeepChannelNumber(true);
    const int32_t start = GENERATE(1, 256, 257, 65536, 65537, 16777216, 2000000, 2147483647);
    PT(o, out_start) = start;
    o.Open();
    REQUIRE_SOCKET(PT(o, ddp_dg));
    o.EndFrame(0);
    auto& d = PT(o, ddp_data);
    uint32_t off = ((uint32_t)d[4] << 24) | ((uint32_t)d[5] << 16) | ((uint32_t)d[6] << 8) | d[7];
    CHECK(off == (uint32_t)(start - 1));
}

TEST_CASE("DDP channel data boundaries", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 10);
    // before Open nothing is allocated, so writes are ignored
    o.SetOneChannel(0, 5);
    o.Open();
    auto* f = PT(o, ddp_full);
    REQUIRE(f != nullptr);
    CHECK(f[0] == 0);
    o.SetOneChannel(0, 5);
    o.SetOneChannel(9, 6);
    o.SetOneChannel(10, 7);
    CHECK(f[0] == 5);
    CHECK(f[9] == 6);
    unsigned char big[40];
    std::fill(big, big + 40, 0x33);
    o.SetManyChannels(5, big, 40);
    CHECK(f[4] == 0);
    CHECK(f[5] == 0x33);
    CHECK(f[9] == 0x33);
    o.SetManyChannels(-1, big, 5);
    o.SetManyChannels(10, big, 5);
    o.SetManyChannels(100, big, 5);
    CHECK(f[0] == 5);
    o.AllOff();
    for (int i = 0; i < 10; i++) CHECK(f[i] == 0);
}

TEST_CASE("DDP disabled output sends and stores nothing", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 10);
    o.Open();
    o.Enable(false);
    o.SetOneChannel(0, 5);
    CHECK(PT(o, ddp_full)[0] == 0);
    o.EndFrame(0);
    CHECK(PT(o, ddp_seq) == 1);
}

TEST_CASE("DDP Open fails for missing or bad addresses", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    CHECK_FALSE(o.Open());
    o.SetIP("hostname.invalid", true, false);
    CHECK_FALSE(o.Open());
}

TEST_CASE("DDP Close frees the full data buffer", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 10);
    o.Open();
    CHECK(PT(o, ddp_full) != nullptr);
    o.Close();
    CHECK(PT(o, ddp_full) == nullptr);
    o.SetOneChannel(0, 1);
    o.Close();
}

TEST_CASE("DDP xml without ChannelsPerPacket gets a usable packet size", "[protocols][ip][ddp]") {
    // ChannelsPerPacket defaults to 0 when missing, and EndFrame then loops forever because every pass sends 0 channels.
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="DDP" ComPort="1.1.1.1" BaudRate="64001" MaxChannels="100"/>)");
    DDPOutput o(n, true);
    CHECK(o.GetChannelsPerPacket() > 0);
}

TEST_CASE("DDP EndFrame with a zero packet size does not hang", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 100);
    o.SetChannelsPerPacket(0);
    o.Open();
    o.EndFrame(0);
    SUCCEED();
}

TEST_CASE("DDP EndFrame with a packet size over 1440 overruns the packet buffer", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64001, 3000);
    o.SetChannelsPerPacket(3000);
    o.Open();
    o.EndFrame(0);
    SUCCEED();
}

TEST_CASE("DDP Open then destroy frees the buffer with the right deallocator", "[protocols][ip][ddp]") {
    // _fulldata is malloc()ed but the destructor and the second Open() use delete: invalid free / mismatched
    // deallocation under valgrind or ASan.
    DDPOutput o;
    Prep(o, 64001, 10);
    o.Open();
    o.Open();
    SUCCEED();
}

TEST_CASE("DDP defaults and id accessors", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    CHECK(o.GetId() == 64001);
    CHECK(o.GetChannels() == 512);
    CHECK(o.GetChannelsPerPacket() == 1440);
    CHECK(o.IsKeepChannelNumbers());
}

TEST_CASE("DDP copy keeps packet settings", "[protocols][ip][ddp]") {
    DDPOutput o;
    DdpGuard guard_(o);
    Prep(o, 64005, 77);
    o.SetChannelsPerPacket(480);
    o.SetKeepChannelNumber(false);
    std::unique_ptr<Output> c(o.Copy());
    auto* x = dynamic_cast<DDPOutput*>(c.get());
    REQUIRE(x != nullptr);
    CHECK(x->GetChannelsPerPacket() == 480);
    CHECK_FALSE(x->IsKeepChannelNumbers());
    CHECK(x->GetId() == 64005);
    CHECK(PT(*x, ddp_full) == nullptr);
}

TEST_CASE("DDP KeepChannelNumbers attribute values", "[protocols][ip][ddp]") {
    struct Row { const char* v; bool expect; };
    const Row row = GENERATE(Row{ "1", true }, Row{ "0", false }, Row{ "2", true }, Row{ "true", false }, Row{ "", false }, Row{ "-1", true });
    pugi::xml_document doc;
    auto n = ParseNode(doc, std::string("<network NetworkType=\"DDP\" ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"10\" KeepChannelNumbers=\"") + row.v + "\"/>");
    DDPOutput o(n, true);
    INFO(row.v);
    CHECK(o.IsKeepChannelNumbers() == row.expect);
}

// ---------------------------------------------------------------------------------------------
// KiNet
// ---------------------------------------------------------------------------------------------

TEST_CASE("KiNet v2 header", "[protocols][ip][kinet]") {
    const int universe = GENERATE(1, 2, 255, 256, 257, 512, 0);
    KinetOutput o;
    Prep(o, universe, 100);
    o.SetVersion(2);
    o.Open();
    auto& d = PT(o, kin_data);
    INFO("universe " << universe);
    CHECK(Bytes(d, d + 4) == Bytes{ 0x04, 0x01, 0xDC, 0x4A });
    CHECK(d[4] == 0x01);
    CHECK(d[5] == 0x00);
    CHECK(d[6] == 0x08);
    CHECK(d[7] == 0x01);
    CHECK(Bytes(d + 12, d + 16) == Bytes{ 0xFF, 0xFF, 0xFF, 0xFF });
    CHECK(d[16] == (uint8_t)((universe - 1) & 0xFF));
    CHECK(d[21] == 0x02);
}

TEST_CASE("KiNet v1 header", "[protocols][ip][kinet]") {
    const int universe = GENERATE(1, 2, 256, 257);
    KinetOutput o;
    Prep(o, universe, 100);
    o.SetVersion(1);
    o.Open();
    auto& d = PT(o, kin_data);
    CHECK(Bytes(d, d + 4) == Bytes{ 0x04, 0x01, 0xDC, 0x4A });
    CHECK(d[6] == 0x01);
    CHECK(d[7] == 0x01);
    CHECK(d[12] == (uint8_t)((universe - 1) & 0xFF));
    CHECK(Bytes(d + 16, d + 20) == Bytes{ 0xFF, 0xFF, 0xFF, 0xFF });
    CHECK(d[20] == 0);
}

TEST_CASE("KiNet data offset depends on the version", "[protocols][ip][kinet]") {
    for (int version : { 1, 2 }) {
        KinetOutput o;
        Prep(o, 1, 20);
        o.SetVersion(version);
        o.Open();
        auto& d = PT(o, kin_data);
        o.SetOneChannel(0, 0x5A);
        o.SetOneChannel(19, 0x5B);
        const int hdr = version == 1 ? 21 : 24;
        INFO("version " << version);
        CHECK(d[hdr] == 0x5A);
        CHECK(d[hdr + 19] == 0x5B);
    }
}

TEST_CASE("KiNet sequence is a 32 bit little endian counter", "[protocols][ip][kinet]") {
    KinetOutput o;
    Prep(o, 1, 20);
    o.Open();
    REQUIRE_SOCKET(PT(o, kin_dg));
    auto& d = PT(o, kin_data);
    o.EndFrame(0);
    CHECK(Bytes(d + 8, d + 12) == Bytes{ 0, 0, 0, 0 });
    o.EndFrame(0);
    CHECK(Bytes(d + 8, d + 12) == Bytes{ 1, 0, 0, 0 });
    PT(o, kin_seq) = 0x01020304;
    o.EndFrame(0);
    CHECK(Bytes(d + 8, d + 12) == Bytes{ 0x04, 0x03, 0x02, 0x01 });
    PT(o, kin_seq) = 0xFFFFFFFF;
    o.EndFrame(0);
    CHECK(Bytes(d + 8, d + 12) == Bytes{ 0xFF, 0xFF, 0xFF, 0xFF });
    CHECK(PT(o, kin_seq) == 0);
}

TEST_CASE("KiNet copy keeps the protocol version", "[protocols][ip][kinet]") {
    // KinetOutput's copy constructor does not copy _version, so a version 1 output copies as version 2.
    KinetOutput o;
    Prep(o, 1, 20);
    o.SetVersion(1);
    std::unique_ptr<Output> c(o.Copy());
    auto* k = dynamic_cast<KinetOutput*>(c.get());
    REQUIRE(k != nullptr);
    CHECK(k->GetVersion() == 1);
}

TEST_CASE("KiNet version attribute garbage", "[protocols][ip][kinet]") {
    struct Row { const char* v; int expect; };
    const Row row = GENERATE(Row{ "1", 1 }, Row{ "2", 2 }, Row{ "abc", 0 }, Row{ "", 0 }, Row{ "-1", -1 }, Row{ "7", 7 });
    pugi::xml_document doc;
    auto n = ParseNode(doc, std::string("<network NetworkType=\"KINET\" ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"10\" Version=\"") + row.v + "\"/>");
    KinetOutput o(n, true);
    CHECK(o.GetVersion() == row.expect);
    // any version except 1 uses the version 2 header
    o.Open();
    CHECK(PT(o, kin_data)[6] == (row.expect == 1 ? 0x01 : 0x08));
}

TEST_CASE("KiNet clamps channel count from xml", "[protocols][ip][kinet]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="KINET" ComPort="1.1.1.1" BaudRate="1" MaxChannels="4000"/>)");
    KinetOutput o(n, true);
    CHECK(o.GetChannels() == 512);
}

TEST_CASE("KiNet disabled output does nothing", "[protocols][ip][kinet]") {
    KinetOutput o;
    Prep(o, 1, 20);
    o.Open();
    o.Enable(false);
    o.SetOneChannel(0, 5);
    CHECK(PT(o, kin_data)[24] == 0);
    CHECK_FALSE(PT(o, out_changed));
}

TEST_CASE("KiNet Open needs a valid address", "[protocols][ip][kinet]") {
    KinetOutput o;
    o.SetChannels(10);
    CHECK_FALSE(o.Open());
    o.SetIP("1.2.3", true, false);
    CHECK_FALSE(o.Open());
}

// ---------------------------------------------------------------------------------------------
// OPC
// ---------------------------------------------------------------------------------------------

TEST_CASE("OPC header", "[protocols][ip][opc]") {
    struct Row { int channel; int len; };
    const Row row = GENERATE(Row{ 0, 1 }, Row{ 1, 3 }, Row{ 255, 300 }, Row{ 256, 3 }, Row{ 1, 256 }, Row{ 2, 65535 }, Row{ 3, 255 });
    OPCOutput o;
    Prep(o, row.channel, row.len);
    o.Open();
    auto* d = PT(o, opc_data);
    REQUIRE(d != nullptr);
    CHECK(d[0] == (row.channel & 0xFF));
    CHECK(d[1] == 0);
    CHECK(d[2] == (row.len >> 8));
    CHECK(d[3] == (row.len & 0xFF));
}

TEST_CASE("OPC channel data starts after the 4 byte header", "[protocols][ip][opc]") {
    OPCOutput o;
    Prep(o, 1, 30);
    o.Open();
    auto* d = PT(o, opc_data);
    o.SetOneChannel(0, 1);
    o.SetOneChannel(29, 2);
    CHECK(d[4] == 1);
    CHECK(d[33] == 2);
    o.AllOff();
    CHECK(d[4] == 0);
    CHECK(d[33] == 0);
}

TEST_CASE("OPC SetChannels reallocates and clears the buffer", "[protocols][ip][opc]") {
    OPCOutput o;
    o.SetChannels(10);
    o.SetOneChannel(0, 9);
    CHECK(PT(o, opc_data)[4] == 9);
    o.SetChannels(20);
    CHECK(PT(o, opc_data)[4] == 0);
    o.SetOneChannel(19, 3);
    CHECK(PT(o, opc_data)[23] == 3);
}

TEST_CASE("OPC channel count is clamped at 65535 from xml", "[protocols][ip][opc]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="OPC" ComPort="1.1.1.1" BaudRate="1" MaxChannels="70000"/>)");
    OPCOutput o(n, true);
    CHECK(o.GetChannels() == 65535);
}

TEST_CASE("OPC SetManyChannels stays inside the buffer", "[protocols][ip][opc]") {
    // OPCOutput::SetManyChannels clamps to GetMaxChannels() (65535) rather than _channels, and _data is only
    // _channels + 4 bytes long: a size larger than the output overruns the heap buffer.
    OPCOutput o;
    o.SetChannels(10);
    std::vector<unsigned char> src(100, 0x11);
    o.SetManyChannels(0, src.data(), src.size());
    SUCCEED();
}

TEST_CASE("OPC Open needs a valid address", "[protocols][ip][opc]") {
    OPCOutput o;
    CHECK_FALSE(o.Open());
    o.SetIP("bad", true, false);
    CHECK_FALSE(o.Open());
}

TEST_CASE("OPC Open fills the header even when the connection fails", "[protocols][ip][opc]") {
    OPCOutput o;
    Prep(o, 5, 12);
    bool ok = o.Open();
    (void)ok;
    CHECK(PT(o, opc_data)[0] == 5);
    CHECK(PT(o, opc_data)[3] == 12);
}

TEST_CASE("OPC export row and description", "[protocols][ip][opc]") {
    OPCOutput o;
    Prep(o, 2, 30, "5.5.5.5");
    int32_t start = 1;
    o.SetTransientData(start, 0);
    CHECK(o.GetLongDescription() == "OPC {2} [1-30] (1-30)");
    CHECK(o.GetExport() == ",1,30,,OPC,5.5.5.5,,,,2,30");
    CHECK(start == 31);
}

// ---------------------------------------------------------------------------------------------
// xxx Ethernet
// ---------------------------------------------------------------------------------------------

TEST_CASE("xxx Ethernet splits into packets of 1200 channels", "[protocols][ip][xxxeth]") {
    struct Row { int channels; int lastCount; int lastStartPixel; };
    const Row row = GENERATE(Row{ 1, 1, 0 }, Row{ 3, 3, 0 }, Row{ 1199, 1199, 0 }, Row{ 1200, 1200, 0 }, Row{ 1201, 1, 400 },
                             Row{ 2400, 1200, 400 }, Row{ 3000, 600, 800 }, Row{ 12288, 288, 4000 });
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<network NetworkType=\"xxx Ethernet\" ComPort=\"127.0.0.1\" BaudRate=\"1\" Id=\"4\" MaxChannels=\"" + std::to_string(row.channels) + "\"/>");
    xxxEthernetOutput o(n, true);
    o.Open();
    REQUIRE_SOCKET(PT(o, xe_dg));
    o.EndFrame(0);
    auto& p = PT(o, xe_packet);
    INFO("channels " << row.channels);
    CHECK(p[0] == 0x80);
    CHECK(p[1] == 4);
    CHECK(p[2] == 0x1D);
    CHECK(((p[3] << 8) | p[4]) == row.lastStartPixel);
    CHECK(((p[5] << 8) | p[6]) == row.lastCount);
    CHECK(p[7 + row.lastCount] == 0x81);
}

TEST_CASE("xxx Ethernet uses the port as the universe byte", "[protocols][ip][xxxeth]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="xxx Ethernet" ComPort="127.0.0.1" BaudRate="1" Id="4" Port="9" MaxChannels="30"/>)");
    xxxEthernetOutput o(n, true);
    CHECK(o.GetUniverse() == 9);
    CHECK(o.GetId() == 9);
    o.Open();
    REQUIRE_SOCKET(PT(o, xe_dg));
    o.EndFrame(0);
    CHECK(PT(o, xe_packet)[1] == 9);
}

TEST_CASE("xxx Ethernet channel data", "[protocols][ip][xxxeth]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="xxx Ethernet" ComPort="127.0.0.1" BaudRate="1" MaxChannels="30"/>)");
    xxxEthernetOutput o(n, true);
    o.Open();
    REQUIRE_SOCKET(PT(o, xe_dg));
    o.SetOneChannel(0, 1);
    o.SetOneChannel(29, 2);
    o.EndFrame(0);
    auto& p = PT(o, xe_packet);
    CHECK(p[7] == 1);
    CHECK(p[36] == 2);
    CHECK(p[37] == 0x81);
}

TEST_CASE("xxx Ethernet buffer grows with SetChannels", "[protocols][ip][xxxeth]") {
    // The data buffer is allocated once in the constructor from the channel count at that time and there is no
    // SetChannels override, so growing a default-constructed output (0 channels) and writing a channel is a heap overflow.
    xxxEthernetOutput o;
    o.SetChannels(100);
    for (int i = 0; i < 100; i++) o.SetOneChannel(i, 1);
    SUCCEED();
}

TEST_CASE("xxx Ethernet id and port", "[protocols][ip][xxxeth]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="xxx Ethernet" ComPort="127.0.0.1" BaudRate="1" Id="12" MaxChannels="30"/>)");
    xxxEthernetOutput o(n, true);
    // Id is read first, then BaudRate is overwritten...  documented behaviour: universe comes from Port when present
    CHECK(o.GetId() == 12);
    auto s = SaveToString(o);
    CHECK(s.find("Id=\"12\"") != std::string::npos);
}

// ---------------------------------------------------------------------------------------------
// ZCPP
// ---------------------------------------------------------------------------------------------

TEST_CASE("ZCPP protocol encode and decode are inverse", "[protocols][zcpp]") {
    const int p = GENERATE(range(0, 20));
    std::string name = ZCPPOutput::DecodeProtocol(p);
    INFO(p << " " << name);
    CHECK(name != "unknown");
    CHECK(ZCPPOutput::EncodeProtocol(name) == p);
}

TEST_CASE("ZCPP unknown protocols", "[protocols][zcpp]") {
    CHECK(ZCPPOutput::DecodeProtocol(20) == "unknown");
    CHECK(ZCPPOutput::DecodeProtocol(-1) == "unknown");
    CHECK(ZCPPOutput::DecodeProtocol(255) == "unknown");
    CHECK(ZCPPOutput::EncodeProtocol("") == 0xFE);
    CHECK(ZCPPOutput::EncodeProtocol("nope") == 0xFE);
    CHECK(ZCPPOutput::EncodeProtocol("WS2811") == 0);
    CHECK(ZCPPOutput::EncodeProtocol("Ws2811") == 0);
    CHECK(ZCPPOutput::EncodeProtocol(" ws2811") == 0xFE);
}

TEST_CASE("ZCPP protocol numbers match the header constants", "[protocols][zcpp]") {
    CHECK(ZCPPOutput::EncodeProtocol("ws2811") == ZCPP_PROTOCOL_WS2811);
    CHECK(ZCPPOutput::EncodeProtocol("dmx") == ZCPP_PROTOCOL_DMX);
    CHECK(ZCPPOutput::EncodeProtocol("renard") == ZCPP_PROTOCOL_RENARD);
    CHECK(ZCPPOutput::EncodeProtocol("lor") == ZCPP_PROTOCOL_LOR);
    CHECK(ZCPPOutput::EncodeProtocol("p9813") == ZCPP_PROTOCOL_P9813);
}

TEST_CASE("ZCPP colour orders", "[protocols][zcpp]") {
    CHECK(ZCPPOutput::EncodeColourOrder("RGB") == ZCPP_COLOUR_ORDER_RGB);
    CHECK(ZCPPOutput::EncodeColourOrder("RBG") == ZCPP_COLOUR_ORDER_RBG);
    CHECK(ZCPPOutput::EncodeColourOrder("GRB") == ZCPP_COLOUR_ORDER_GRB);
    CHECK(ZCPPOutput::EncodeColourOrder("GBR") == ZCPP_COLOUR_ORDER_GBR);
    CHECK(ZCPPOutput::EncodeColourOrder("BRG") == ZCPP_COLOUR_ORDER_BRG);
    CHECK(ZCPPOutput::EncodeColourOrder("BGR") == ZCPP_COLOUR_ORDER_BGR);
    CHECK(ZCPPOutput::EncodeColourOrder("rgb") == 0);
    CHECK(ZCPPOutput::EncodeColourOrder("") == 0);
    CHECK(ZCPPOutput::EncodeColourOrder("RGBW") == 0);
}

TEST_CASE("ZCPP vendors", "[protocols][zcpp]") {
    CHECK(ZCPPOutput::EncodeVendor("Falcon") == ZCPP_VENDOR_FALCON);
    CHECK(ZCPPOutput::EncodeVendor("FPP") == ZCPP_VENDOR_FPP);
    CHECK(ZCPPOutput::EncodeVendor("ESP Pixel Stick") == ZCPP_VENDOR_ESPIXELSTICK);
    CHECK(ZCPPOutput::EncodeVendor("Unknown") == ZCPP_VENDOR_UNKNOWN);
    CHECK(ZCPPOutput::EncodeVendor("falcon") == ZCPP_VENDOR_UNKNOWN);
    CHECK(ZCPPOutput::EncodeVendor("") == ZCPP_VENDOR_UNKNOWN);
    CHECK(ZCPPOutput::DecodeVendor(ZCPP_VENDOR_FALCON) == "Falcon");
    CHECK(ZCPPOutput::DecodeVendor(ZCPP_VENDOR_FPP) == "FPP");
    CHECK(ZCPPOutput::DecodeVendor(ZCPP_VENDOR_ESPIXELSTICK) == "ESP Pixel Stick");
    CHECK(ZCPPOutput::DecodeVendor(12345) == "Unknown");
    CHECK(ZCPPOutput::DecodeVendor(-1) == "Unknown");
    CHECK(ZCPPOutput::GetVendors().size() == 3);
}

TEST_CASE("ZCPP vendors listed all encode back", "[protocols][zcpp]") {
    for (const auto& v : ZCPPOutput::GetVendors()) {
        INFO(v);
        CHECK(ZCPPOutput::DecodeVendor(ZCPPOutput::EncodeVendor(v)) == v);
    }
}

TEST_CASE("ZCPP discovery protocol bit conversions", "[protocols][zcpp]") {
    const int p = GENERATE(range(0, 20));
    uint32_t bit = ZCPP_ConvertProtocolToDiscoveryProtocol((uint8_t)p);
    CHECK(bit == (uint32_t)(1u << p));
    CHECK(ZCPP_ConvertDiscoveryProtocolToProtocol(bit) == p);
}

TEST_CASE("ZCPP discovery protocol conversion edge cases", "[protocols][zcpp]") {
    CHECK(ZCPP_ConvertDiscoveryProtocolToProtocol(0) == 0);
    CHECK(ZCPP_ConvertDiscoveryProtocolToProtocol(1) == 0);
    CHECK(ZCPP_ConvertDiscoveryProtocolToProtocol(0x80000000u) == 31);
    CHECK(ZCPP_ConvertProtocolToDiscoveryProtocol(31) == 0x80000000u);
    CHECK(ZCPP_ConvertProtocolToDiscoveryProtocol(32) == 0);
    CHECK(ZCPP_ConvertProtocolToDiscoveryProtocol(255) == 0);
    CHECK(ZCPP_DISCOVERY_PROTOCOL_WS2811 == ZCPP_ConvertProtocolToDiscoveryProtocol(ZCPP_PROTOCOL_WS2811));
    CHECK(ZCPP_DISCOVERY_PROTOCOL_LOR == ZCPP_ConvertProtocolToDiscoveryProtocol(ZCPP_PROTOCOL_LOR));
    CHECK(ZCPP_DISCOVERY_PROTOCOL_APA102 == ZCPP_ConvertProtocolToDiscoveryProtocol(ZCPP_PROTOCOL_APA102));
}

TEST_CASE("ZCPP port field helpers", "[protocols][zcpp]") {
    const int v = GENERATE(range(0, 256));
    CHECK(ZCPP_GetSmartRemote((uint8_t)v) == (v >> 6));
    CHECK(ZCPP_GetStringNumber((uint8_t)v) == (v & 0x3F));
    CHECK(ZCPP_IsReversed((uint8_t)v) == ((v & 0x80) != 0));
    CHECK(ZCPP_GetColourOrder((uint8_t)v) == (v & 7));
}

TEST_CASE("ZCPP gamma is tenths", "[protocols][zcpp]") {
    CHECK(ZCPP_GetGamma(0) == 0.0f);
    CHECK(ZCPP_GetGamma(10) == 1.0f);
    CHECK(ZCPP_GetGamma(22) == Catch::Approx(2.2f));
    CHECK(ZCPP_GetGamma(255) == Catch::Approx(25.5f));
}

TEST_CASE("ZCPP wire conversions are big endian", "[protocols][zcpp]") {
    const uint32_t v = GENERATE(0u, 1u, 255u, 256u, 0x01020304u, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu);
    uint32_t w = ZCPP_ToWire32(v);
    uint8_t* b = (uint8_t*)&w;
    CHECK(b[0] == (v >> 24));
    CHECK(b[1] == ((v >> 16) & 0xFF));
    CHECK(b[2] == ((v >> 8) & 0xFF));
    CHECK(b[3] == (v & 0xFF));
    CHECK(ZCPP_FromWire32(w) == v);
}

TEST_CASE("ZCPP 16 bit wire conversions", "[protocols][zcpp]") {
    const uint32_t v = GENERATE(0u, 1u, 255u, 256u, 0x0102u, 0x7FFFu, 0x8000u, 0xFFFFu, 0x10000u, 0x12345u);
    uint16_t w = ZCPP_ToWire16(v);
    uint8_t* b = (uint8_t*)&w;
    CHECK(b[0] == ((v >> 8) & 0xFF));
    CHECK(b[1] == (v & 0xFF));
    CHECK(ZCPP_FromWire16(w) == (uint16_t)v);
}

TEST_CASE("ZCPP packet sizes by type", "[protocols][zcpp]") {
    ZCPP_packet_t p;
    memset(&p, 0, sizeof(p));
    p.Discovery.Header.type = ZCPP_TYPE_DISCOVERY;
    CHECK(ZCPP_GetPacketActualSize(p) == sizeof(ZCPP_Discovery));
    CHECK(sizeof(ZCPP_Discovery) == 8);
    p.Discovery.Header.type = ZCPP_TYPE_DISCOVERY_RESPONSE;
    CHECK(ZCPP_GetPacketActualSize(p) == sizeof(ZCPP_DiscoveryResponse));
    p.Discovery.Header.type = ZCPP_TYPE_QUERY_CONFIG;
    CHECK(ZCPP_GetPacketActualSize(p) == 6);
    p.Discovery.Header.type = ZCPP_TYPE_SYNC;
    CHECK(ZCPP_GetPacketActualSize(p) == 7);
    p.Discovery.Header.type = 0xEE;
    CHECK(ZCPP_GetPacketActualSize(p) == 0);
}

TEST_CASE("ZCPP configuration packet size grows by 16 bytes per port", "[protocols][zcpp]") {
    CHECK(sizeof(ZCPP_PortConfig) == 16);
    const int ports = GENERATE(0, 1, 2, 50, 85);
    ZCPP_packet_t p;
    memset(&p, 0, sizeof(p));
    p.Configuration.Header.type = ZCPP_TYPE_CONFIG;
    p.Configuration.ports = (uint8_t)ports;
    CHECK(ZCPP_GetPacketActualSize(p) == ZCPP_CONFIGURATION_HEADER_SIZE + ports * 16);
    p.Configuration.Header.type = ZCPP_TYPE_QUERY_CONFIG_RESPONSE;
    p.QueryConfigurationResponse.ports = (uint8_t)ports;
    CHECK(ZCPP_GetPacketActualSize(p) == ZCPP_QUERYCONFIGURATIONRESPONSE_HEADER_SIZE + ports * 16);
}

TEST_CASE("ZCPP the largest legal configuration packet fits the packet union", "[protocols][zcpp]") {
    CHECK(ZCPP_CONFIGURATION_HEADER_SIZE + ZCPP_CONFIG_MAX_PORT_PER_PACKET * sizeof(ZCPP_PortConfig) <= sizeof(ZCPP_packet_t));
}

TEST_CASE("ZCPP data packet size follows the length field", "[protocols][zcpp]") {
    ZCPP_packet_t p;
    memset(&p, 0, sizeof(p));
    p.Data.Header.type = ZCPP_TYPE_DATA;
    const int len = GENERATE(0, 1, 100, 1443);
    p.Data.packetDataLength = ZCPP_ToWire16(len);
    CHECK(ZCPP_GetPacketActualSize(p) == ZCPP_DATA_HEADER_SIZE + len);
    CHECK(ZCPP_DATA_HEADER_SIZE == 15);
}

TEST_CASE("ZCPP extra data size walks the descriptions", "[protocols][zcpp]") {
    ZCPP_packet_t p;
    memset(&p, 0, sizeof(p));
    ZCPPOutput::InitialiseExtraConfigPacket(&p, 1, 100);
    CHECK(p.ExtraData.Header.type == ZCPP_TYPE_EXTRA_DATA);
    p.ExtraData.ports = 0;
    CHECK(ZCPP_GetPacketActualSize(p) == ZCPP_EXTRADATA_HEADER_SIZE);
    p.ExtraData.ports = 2;
    ZCPP_PortExtraData* a = (ZCPP_PortExtraData*)&p.raw[ZCPP_EXTRADATA_HEADER_SIZE];
    a->descriptionLength = 5;
    ZCPP_PortExtraData* b = (ZCPP_PortExtraData*)&p.raw[ZCPP_EXTRADATA_HEADER_SIZE + ZCPP_PORTEXTRADATA_HEADER_SIZE + 5];
    b->descriptionLength = 3;
    CHECK(ZCPP_GetPacketActualSize(p) == ZCPP_EXTRADATA_HEADER_SIZE + 2 * ZCPP_PORTEXTRADATA_HEADER_SIZE + 8);
}

TEST_CASE("ZCPP packet initialisers", "[protocols][zcpp]") {
    ZCPP_packet_t p;
    memset(&p, 0xCC, sizeof(p));
    ZCPPOutput::InitialiseModelDataPacket(&p, 0x0102, 77, "my controller");
    CHECK(Bytes(p.Configuration.Header.token, p.Configuration.Header.token + 4) == Bytes{ 'Z', 'C', 'P', 'P' });
    CHECK(p.Configuration.Header.type == ZCPP_TYPE_CONFIG);
    CHECK(p.Configuration.Header.protocolVersion == ZCPP_CURRENT_PROTOCOL_VERSION);
    CHECK(ZCPP_FromWire16(p.Configuration.sequenceNumber) == 0x0102);
    CHECK(p.Configuration.priority == 77);
    CHECK(std::string(p.Configuration.userControllerName) == "my controller");
    CHECK(p.Configuration.ports == 0);
    CHECK(p.Configuration.flags == 0);
}

TEST_CASE("ZCPP model data description is truncated and terminated", "[protocols][zcpp]") {
    ZCPP_packet_t p;
    ZCPPOutput::InitialiseModelDataPacket(&p, 1, 1, std::string(100, 'x'));
    CHECK(std::string(p.Configuration.userControllerName).size() == 31);
    CHECK(p.Configuration.userControllerName[31] == 0);
    ZCPPOutput::InitialiseModelDataPacket(&p, 1, 1, std::string(31, 'y'));
    CHECK(std::string(p.Configuration.userControllerName).size() == 31);
    ZCPPOutput::InitialiseModelDataPacket(&p, 1, 1, "");
    CHECK(std::string(p.Configuration.userControllerName).empty());
}

TEST_CASE("ZCPP sequence numbers are carried big endian", "[protocols][zcpp]") {
    const int seq = GENERATE(0, 1, 255, 256, 0xFFFF);
    ZCPP_packet_t p;
    ZCPPOutput::InitialiseExtraConfigPacket(&p, seq, 1);
    CHECK(ZCPP_FromWire16(p.ExtraData.sequenceNumber) == seq);
    CHECK(p.ExtraData.priority == 1);
}

TEST_CASE("ZCPP multicast data address", "[protocols][zcpp]") {
    struct Row { const char* in; const char* out; };
    const Row row = GENERATE(Row{ "10.10.10.10", "224.0.31.10" }, Row{ "  1.2.3.4  ", "224.0.31.4" }, Row{ "1.2.3.255", "224.0.31.255" },
                             Row{ "1.2.3.0", "224.0.31.0" }, Row{ "1.2.3", "" }, Row{ "1.2.3.4.5", "" }, Row{ "", "" }, Row{ "   ", "" },
                             Row{ "1.2.3.256", "" }, Row{ "a.b.c.d", "" }, Row{ "1.2.3.-4", "" },                              Row{ "1.2.3.4.", "224.0.31.4" }, Row{ "host", "" }, Row{ "1.2.3.4a", "" });
    INFO("'" << row.in << "'");
    CHECK(ZCPP_GetDataMulticastAddress(row.in) == row.out);
}

TEST_CASE("ZCPP multicast data address rejects an empty octet", "[protocols][zcpp]") {
    // the octet check only rejects non-digits, and an empty component passes isdigit-by-vacuity and atoi("") == 0
    CHECK(ZCPP_GetDataMulticastAddress("1..3.4") == "");
    CHECK(ZCPP_GetDataMulticastAddress("..3.4") == "");
    CHECK(ZCPP_GetDataMulticastAddress(".1.2.3.4") == "");
}

TEST_CASE("ZCPP header constants", "[protocols][zcpp]") {
    CHECK(sizeof(ZCPP_Header) == 6);
    CHECK(sizeof(ZCPP_packet_t) == 1458);
    CHECK(ZCPP_PORT == 30005);
    CHECK(ZCPP_token[0] == 'Z');
    CHECK(ZCPP_token[3] == 'P');
    CHECK(sizeof(ZCPP_DiscoveryResponse) == 88);
    CHECK(sizeof(ZCPP_Sync) == 7);
}

TEST_CASE("ZCPP output default state", "[protocols][zcpp]") {
    ZCPPOutput o;
    CHECK(o.GetChannels() == 1);
    CHECK(o.GetId() == -1);
    CHECK(o.GetMaxChannels() == 16 * 1024);
    CHECK(o.IsValidChannelCount(16384));
    CHECK_FALSE(o.IsValidChannelCount(16385));
    CHECK_FALSE(o.IsValidChannelCount(0));
    CHECK(o.GetUniverseString() == "");
    CHECK(o.GetPriority() == 100);
    CHECK(o.GetType() == OUTPUT_ZCPP);
}

TEST_CASE("ZCPP SetChannels reallocates and clamps the used channels", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetChannels(100);
    PT(o, zc_used) = 100;
    CHECK(o.GetChannels() == 100);
    CHECK(PT(o, zc_data)[99] == 0);
    o.SetChannels(40);
    CHECK(PT(o, zc_used) == 40);
    o.SetChannels(200);
    CHECK(PT(o, zc_used) == 40);
    o.SetChannels(200);
}

TEST_CASE("ZCPP channel data and AllOn", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetChannels(10);
    o.SetOneChannel(0, 3);
    o.SetOneChannel(9, 4);
    CHECK(PT(o, zc_data)[0] == 3);
    CHECK(PT(o, zc_data)[9] == 4);
    o.AllOn();
    for (int i = 0; i < 10; i++) CHECK(PT(o, zc_data)[i] == 0x20);
    o.AllOff();
    for (int i = 0; i < 10; i++) CHECK(PT(o, zc_data)[i] == 0);
    unsigned char big[50];
    std::fill(big, big + 50, 9);
    o.SetManyChannels(5, big, 50);
    CHECK(PT(o, zc_data)[4] == 0);
    CHECK(PT(o, zc_data)[9] == 9);
}

TEST_CASE("ZCPP protocols are stored lower case without duplicates", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.AddProtocol("WS2811");
    o.AddProtocol("ws2811");
    o.AddProtocol("DMX");
    CHECK(o.SupportsProtocol("ws2811"));
    CHECK(o.SupportsProtocol("Ws2811"));
    CHECK(o.SupportsProtocol("dmx"));
    CHECK_FALSE(o.SupportsProtocol("lor"));
    CHECK((o.*PT_FN(zc_serialise))() == "ws2811|dmx");
}

TEST_CASE("ZCPP data packets chunk at 1443 bytes", "[protocols][zcpp]") {
    struct Row { long used; long lastLen; long lastAddr; int packets; uint8_t lastFlags; };
    const Row row = GENERATE(Row{ 1, 1, 0, 1, ZCPP_DATA_FLAG_LAST | ZCPP_DATA_FLAG_FIRST }, Row{ 1443, 1443, 0, 1, ZCPP_DATA_FLAG_LAST | ZCPP_DATA_FLAG_FIRST },
                             Row{ 1444, 1, 1443, 2, ZCPP_DATA_FLAG_LAST }, Row{ 2886, 1443, 1443, 2, ZCPP_DATA_FLAG_LAST },
                             Row{ 3000, 114, 2886, 3, ZCPP_DATA_FLAG_LAST }, Row{ 16384, 16384 - 11 * 1443, 11 * 1443, 12, ZCPP_DATA_FLAG_LAST });
    ZCPPOutput o;
    o.SetIP("127.0.0.1", true);
    o.SetChannels(16384);
    o.SetDontConfigure(true);
    PT(o, zc_used) = row.used;
    o.Open();
    REQUIRE_SOCKET(PT(o, zc_dg));
    for (long i = 0; i < row.used; i++) PT(o, zc_data)[i] = (uint8_t)(i * 3);
    o.EndFrame(0);
    auto& pk = PT(o, zc_packet).Data;
    INFO("used " << row.used);
    CHECK(pk.Header.type == ZCPP_TYPE_DATA);
    CHECK(ZCPP_FromWire32(pk.frameAddress) == (uint32_t)row.lastAddr);
    CHECK(ZCPP_FromWire16(pk.packetDataLength) == row.lastLen);
    CHECK((pk.flags & (ZCPP_DATA_FLAG_LAST | ZCPP_DATA_FLAG_FIRST)) == row.lastFlags);
    CHECK(pk.sequenceNumber == 0);
    CHECK(PT(o, zc_data)[0] == 0);
    for (long i = 0; i < row.lastLen; i++) REQUIRE(pk.data[i] == (uint8_t)((row.lastAddr + i) * 3));
    CHECK(PT(o, zc_seq) == 1);
    CHECK(ZCPP_GetPacketActualSize(PT(o, zc_packet)) == ZCPP_DATA_HEADER_SIZE + row.lastLen);
}

TEST_CASE("ZCPP does not send with nothing used", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetIP("127.0.0.1", true);
    o.SetChannels(10);
    o.SetDontConfigure(true);
    PT(o, zc_used) = 0;
    o.Open();
    o.EndFrame(0);
    CHECK(PT(o, zc_seq) == 0);
}

TEST_CASE("ZCPP multicast remote address", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetIP("10.1.2.77", true);
    o.SetChannels(10);
    o.SetMulticast(true);
    o.Open();
    CHECK(PT(o, zc_remote) == "224.0.31.77");
    ZCPPOutput u;
    u.SetIP("10.1.2.77", true);
    u.SetChannels(10);
    u.Open();
    CHECK(PT(u, zc_remote) == "10.1.2.77");
}

TEST_CASE("ZCPP data packet header after Open", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetIP("127.0.0.1", true);
    o.SetChannels(10);
    o.SetPriority(55);
    o.Open();
    auto& pk = PT(o, zc_packet).Data;
    CHECK(Bytes(pk.Header.token, pk.Header.token + 4) == Bytes{ 'Z', 'C', 'P', 'P' });
    CHECK(pk.Header.type == ZCPP_TYPE_DATA);
    CHECK(pk.priority == 55);
}

TEST_CASE("ZCPP save and reload", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetIP("1.2.3.4", true, false);
    o.SetVendor(1);
    o.SetModel(9);
    o.SetPriority(33);
    o.SetSupportsSmartRemotes(true);
    o.SetMulticast(true);
    o.SetDontConfigure(true);
    o.AddProtocol("DMX");
    o.AddProtocol("LOR");
    auto s = SaveToString(o);
    CHECK(s.find("Vendor=\"1\"") != std::string::npos);
    CHECK(s.find("Model=\"9\"") != std::string::npos);
    CHECK(s.find("Priority=\"33\"") != std::string::npos);
    CHECK(s.find("SupportsVirtualStrings=\"FALSE\"") != std::string::npos);
    CHECK(s.find("SupportsSmartRemotes=\"TRUE\"") != std::string::npos);
    CHECK(s.find("DontConfigure=\"TRUE\"") != std::string::npos);
    CHECK(s.find("Multicast=\"TRUE\"") != std::string::npos);
    CHECK(s.find("Protocols=\"dmx|lor\"") != std::string::npos);
}

TEST_CASE("ZCPP copy keeps the configuration", "[protocols][zcpp]") {
    ZCPPOutput o;
    o.SetIP("1.2.3.4", true, false);
    o.SetChannels(20);
    o.SetVendor(1);
    o.SetModel(9);
    o.SetPriority(33);
    o.SetSupportsVirtualStrings(true);
    o.SetDontConfigure(true);
    o.AddProtocol("DMX");
    std::unique_ptr<Output> c(o.Copy());
    auto* z = dynamic_cast<ZCPPOutput*>(c.get());
    REQUIRE(z != nullptr);
    CHECK(z->GetVendor() == 1);
    CHECK(z->GetModel() == 9);
    CHECK(z->GetPriority() == 33);
    CHECK(z->IsSupportsVirtualStrings());
    CHECK(z->IsDontConfigure());
    CHECK(z->SupportsProtocol("dmx"));
    CHECK(z->GetChannels() == 20);
    CHECK(PT(*z, zc_data) != nullptr);
}

namespace {

struct ZcppFile {
    std::string bytes = "ZCPP";
    void AddConfig(uint32_t start, uint32_t len, int ports = 1) {
        ZCPP_packet_t p;
        ZCPPOutput::InitialiseModelDataPacket(&p, 1, 100, "ctl");
        p.Configuration.ports = (uint8_t)ports;
        for (int i = 0; i < ports; i++) {
            p.Configuration.PortConfig[i].startChannel = ZCPP_ToWire32(start + i * len);
            p.Configuration.PortConfig[i].channels = ZCPP_ToWire32(len);
        }
        bytes.push_back(0x00);
        bytes.push_back((char)(sizeof(p) >> 8));
        bytes.push_back((char)(sizeof(p) & 0xFF));
        bytes.append((char*)&p, sizeof(p));
    }
};

// ZCPPOutput malloc()s the packets it reads but its destructor delete()s them; free them here so that
// a valgrind run of the normal tests stays clean (the mismatch has its own hidden test).
struct ZcppDeleter {
    void operator()(ZCPPOutput* o) const {
        for (auto* p : PT(*o, zc_model)) free(p);
        PT(*o, zc_model).clear();
        for (auto* p : PT(*o, zc_extra)) free(p);
        PT(*o, zc_extra).clear();
        delete o;
    }
};
using ZcppPtr = std::unique_ptr<ZCPPOutput, ZcppDeleter>;

ZCPPOutput* LoadZcppRaw(OutputManager& om, const TempDir& td, const std::string& fileBytes, bool autosize, int channels, const std::string& ip = "10.1.2.3") {
    std::string name = ip;
    std::replace(name.begin(), name.end(), '.', '_');
    td.Write(name + ".zcpp", fileBytes);
    ControllerEthernet c(&om, false);
    c.SetAutoLayout(true);
    c.SetAutoSize(autosize, nullptr);
    std::string xml = "<network NetworkType=\"ZCPP\" ComPort=\"" + ip + "\" BaudRate=\"1\" MaxChannels=\"" + std::to_string(channels) + "\"/>";
    pugi::xml_document doc;
    auto n = ParseNode(doc, xml);
    return new ZCPPOutput(&c, n, td.path.string());
}

ZcppPtr LoadZcpp(OutputManager& om, const TempDir& td, const std::string& fileBytes, bool autosize, int channels, const std::string& ip = "10.1.2.3") {
    return ZcppPtr(LoadZcppRaw(om, td, fileBytes, autosize, channels, ip));
}

} // namespace

TEST_CASE("ZCPP loads model data and works out the used channels", "[protocols][zcpp]") {
    struct Row { uint32_t start; uint32_t len; long used; };
    const Row row = GENERATE(Row{ 0, 3, 3 }, Row{ 0, 100, 100 }, Row{ 50, 10, 60 }, Row{ 0, 1, 1 });
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(row.start, row.len);
    auto o = LoadZcpp(om, td, f.bytes, true, 500);
    INFO("start " << row.start << " len " << row.len);
    CHECK(PT(*o, zc_model).size() == 1);
    CHECK(PT(*o, zc_used) == row.used);
    CHECK(o->GetChannels() == row.used);
}

TEST_CASE("ZCPP used channels count a two channel string", "[protocols][zcpp]") {
    // ExtractUsedChannelsFromModelData tests `start + len - 1 > used` against an initial value of 1, so a string of
    // exactly 2 channels at 0 (or 1 channel at 1) leaves the used count at 1, one channel short.
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(0, 2);
    auto o = LoadZcpp(om, td, f.bytes, true, 500);
    CHECK(PT(*o, zc_used) == 2);
}

TEST_CASE("ZCPP used channels are capped by the channel count without autosize", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(0, 100);
    auto o = LoadZcpp(om, td, f.bytes, false, 40);
    CHECK(PT(*o, zc_used) == 40);
    CHECK(o->GetChannels() == 40);
}

TEST_CASE("ZCPP model data file with a bad tag is ignored", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    auto o = LoadZcpp(om, td, "XCPP\x00\x05\xB2", true, 10);
    CHECK(PT(*o, zc_model).empty());
}

TEST_CASE("ZCPP model data file with a wrong packet size stops reading", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    std::string bytes = "ZCPP";
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    bytes.push_back(0x10);
    bytes.append(100, 'x');
    auto o = LoadZcpp(om, td, bytes, true, 10);
    CHECK(PT(*o, zc_model).empty());
}

TEST_CASE("ZCPP model data file with only the tag", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    auto o = LoadZcpp(om, td, "ZCPP", false, 10);
    CHECK(PT(*o, zc_model).empty());
    CHECK(PT(*o, zc_used) == 1);
    CHECK(o->GetChannels() == 10);
}

TEST_CASE("ZCPP model data file with an unknown record type", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    std::string bytes = "ZCPP";
    bytes.push_back((char)0x77);
    auto o = LoadZcpp(om, td, bytes, true, 10);
    CHECK(PT(*o, zc_model).empty());
}

TEST_CASE("ZCPP model data file end marker stops reading", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(0, 4);
    f.bytes.push_back((char)0xFF);
    f.AddConfig(0, 99);
    auto o = LoadZcpp(om, td, f.bytes, true, 10);
    CHECK(PT(*o, zc_model).size() == 1);
    CHECK(PT(*o, zc_used) == 4);
}

TEST_CASE("ZCPP model data with an impossible port count is rejected", "[protocols][zcpp]") {
    // the debug assert in ExtractUsedChannelsFromModelData aborts before the corrupt-file handling that follows it can run
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(0, 4, 1);
    // patch the port count to 200 (> 85)
    f.bytes[4 + 3 + 6 + 2 + 32 + 3] = (char)200;
    auto o = LoadZcpp(om, td, f.bytes, true, 10);
    CHECK(PT(*o, zc_used) == 1);
}

TEST_CASE("ZCPP does not load a model file when DontConfigure is set", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(0, 4);
    td.Write("10_1_2_3.zcpp", f.bytes);
    ControllerEthernet c(&om, false);
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="ZCPP" ComPort="10.1.2.3" BaudRate="1" MaxChannels="10" DontConfigure="TRUE"/>)");
    ZCPPOutput o(&c, n, td.path.string());
    CHECK(PT(o, zc_model).empty());
    CHECK(o.IsDontConfigure());
}

TEST_CASE("ZCPP xml attribute defaults", "[protocols][zcpp]") {
    OutputManager om;
    TempDir td;
    ControllerEthernet c(&om, false);
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="ZCPP" ComPort="10.1.2.3" BaudRate="1" MaxChannels="0"/>)");
    ZCPPOutput o(&c, n, td.path.string());
    CHECK(o.GetVendor() == 65535);
    CHECK(o.GetModel() == 65535);
    CHECK(o.GetPriority() == 100);
    CHECK_FALSE(o.IsSupportsVirtualStrings());
    CHECK_FALSE(o.IsSupportsSmartRemotes());
    CHECK_FALSE(o.IsMulticast());
    CHECK_FALSE(o.IsDontConfigure());
    CHECK(PT(o, zc_data) == nullptr);
}

TEST_CASE("ZCPP created with no controller does not crash", "[protocols][zcpp]") {
    // Output::Create passes the controller straight to the ZCPPOutput constructor, which calls c->IsActive() without
    // a null check (every other network type guards with `c && c->IsActive()`).
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="ZCPP" ComPort="10.1.2.3" BaudRate="1" MaxChannels="10"/>)");
    std::unique_ptr<Output> o(Output::Create(nullptr, n, ""));
    CHECK(o != nullptr);
}

TEST_CASE("ZCPP model packets freed with the matching deallocator", "[protocols][zcpp]") {
    // The constructor malloc()s each packet read from the .zcpp file but the destructor delete()s them:
    // mismatched deallocation under valgrind / ASan.
    OutputManager om;
    TempDir td;
    ZcppFile f;
    f.AddConfig(0, 4);
    std::unique_ptr<ZCPPOutput> o(LoadZcppRaw(om, td, f.bytes, true, 10));
    o.reset();
    SUCCEED();
}

// ---------------------------------------------------------------------------------------------
// Twinkly
// ---------------------------------------------------------------------------------------------

TEST_CASE("Twinkly http port defaults and garbage", "[protocols][ip][twinkly]") {
    struct Row { const char* v; int expect; };
    const Row row = GENERATE(Row{ "8080", 8080 }, Row{ "", 0 }, Row{ "abc", 0 }, Row{ "-1", 65535 }, Row{ "65536", 0 }, Row{ "70000", 4464 });
    pugi::xml_document doc;
    auto n = ParseNode(doc, std::string("<network NetworkType=\"Twinkly\" ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"30\" HTTPPort=\"") + row.v + "\"/>");
    TwinklyOutput o(n, true);
    auto s = SaveToString(o);
    CHECK(s.find("HTTPPort=\"" + std::to_string(row.expect) + "\"") != std::string::npos);
    pugi::xml_document d2;
    auto n2 = ParseNode(d2, "<network NetworkType=\"Twinkly\" ComPort=\"1.1.1.1\" BaudRate=\"1\" MaxChannels=\"30\"/>");
    TwinklyOutput o2(n2, true);
    CHECK(SaveToString(o2).find("HTTPPort=\"80\"") != std::string::npos);
}
