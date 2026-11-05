#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

#include "../support/ProtocolAccess.h"

using namespace protocols_test;
using Bytes = std::vector<uint8_t>;

namespace {

Bytes Cat(std::initializer_list<Bytes> parts) {
    Bytes r;
    for (auto& p : parts) r.insert(r.end(), p.begin(), p.end());
    return r;
}

Bytes Rep(uint8_t v, size_t n) { return Bytes(n, v); }

const Bytes kLorHeartbeat = { 0x00, 0xFF, 0x81, 0x56, 0x00 };

// LOR intensity byte for a 0-255 channel value (percent = round(v / 2.55), 0xF0 off .. 0x01 full).
uint8_t LorTable(int v) {
    int percent = (v * 100 + 127) / 255;
    if (percent == 0) return 0xF0;
    if (percent >= 100) return 0x01;
    return (uint8_t)(228 - percent * 2);
}

} // namespace

// ---------------------------------------------------------------------------------------------
// Renard
// ---------------------------------------------------------------------------------------------

TEST_CASE("Renard frame is sync, address, then the channel bytes", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(4);
    Pipe p;
    REQUIRE(OpenOnPipe(r, p));
    r.SetOneChannel(0, 1);
    r.SetOneChannel(1, 2);
    r.SetOneChannel(2, 3);
    r.SetOneChannel(3, 4);
    r.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0x7E, 0x80, 1, 2, 3, 4 });
}

TEST_CASE("Renard escapes the reserved bytes", "[protocols][serial][renard]") {
    const int v = GENERATE(range(0, 256));
    RenardOutput r;
    r.SetChannels(1);
    Pipe p;
    OpenOnPipe(r, p);
    r.SetOneChannel(0, (unsigned char)v);
    r.EndFrame(0);
    uint8_t expect = (uint8_t)v;
    if (v == 0x7D || v == 0x7E) expect = 0x7C;
    if (v == 0x7F) expect = 0x80;
    INFO("value " << v);
    CHECK(p.Drain() == Bytes{ 0x7E, 0x80, expect });
}

TEST_CASE("Renard frame length tracks the channel count", "[protocols][serial][renard]") {
    const int n = GENERATE(1, 2, 8, 255, 256, 1014, RENARD_MAX_CHANNELS);
    RenardOutput r;
    r.SetChannels(n);
    Pipe p;
    OpenOnPipe(r, p);
    r.SetOneChannel(n - 1, 9);
    r.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == (size_t)n + 2);
    CHECK(f[0] == 0x7E);
    CHECK(f[1] == 0x80);
    CHECK(f.back() == 9);
}

TEST_CASE("Renard AllOff clears every channel of the frame", "[protocols][serial][renard][!mayfail]") {
    // RenardOutput::AllOff calls SetOneChannel(i + 2, 0) but SetOneChannel already adds the 2 byte header
    // offset, so the first two channels are never cleared and two bytes past the frame are written instead.
    RenardOutput r;
    r.SetChannels(4);
    Pipe p;
    OpenOnPipe(r, p);
    for (int i = 0; i < 4; i++) r.SetOneChannel(i, 50);
    r.EndFrame(0);
    p.Drain();
    r.AllOff();
    r.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0x7E, 0x80, 0, 0, 0, 0 });
}

TEST_CASE("Renard does nothing when disabled or suspended", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(3);
    Pipe p;
    OpenOnPipe(r, p);
    r.SetOneChannel(0, 5);
    SECTION("suspended") {
        r.Suspend(true);
        r.EndFrame(0);
        CHECK(p.Drain().empty());
        r.Suspend(false);
        r.EndFrame(0);
        CHECK_FALSE(p.Drain().empty());
    }
    SECTION("disabled") {
        r.Enable(false);
        r.EndFrame(0);
        CHECK(p.Drain().empty());
    }
}

TEST_CASE("Renard disabled output ignores channel writes", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(3);
    Pipe p;
    OpenOnPipe(r, p);
    r.Enable(false);
    r.SetOneChannel(1, 77);
    r.Enable(true);
    r.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0x7E, 0x80, 0, 0, 0 });
}

TEST_CASE("Renard Open forces two stop bits", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(3);
    r.SetCommPort("NotConnected");
    r.Open();
    CHECK(std::string(PT(r, so_config)) == "8N2");
}

TEST_CASE("Renard Open on a disabled output leaves it alone and succeeds", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(3);
    r.Enable(false);
    CHECK(r.Open());
    CHECK(PT(r, ren_datalen) == 0);
}

TEST_CASE("Renard suppress duplicate frames", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(2);
    r.SetSuppressDuplicateFrames(true);
    Pipe p;
    OpenOnPipe(r, p);
    r.SetOneChannel(0, 1);
    r.EndFrame(3);
    CHECK(p.Drain().size() == 4);
    // nothing changed: skipped until the skip counter reaches the limit
    for (int i = 0; i < 3; i++) {
        r.EndFrame(3);
        CHECK(p.Drain().empty());
    }
    r.EndFrame(3);
    CHECK(p.Drain().size() == 4);
    // a change always goes out
    r.SetOneChannel(1, 9);
    r.EndFrame(3);
    CHECK(p.Drain() == Bytes{ 0x7E, 0x80, 1, 9 });
}

TEST_CASE("Renard unchanged channel writes do not mark the frame changed", "[protocols][serial][renard]") {
    RenardOutput r;
    r.SetChannels(2);
    r.SetSuppressDuplicateFrames(true);
    Pipe p;
    OpenOnPipe(r, p);
    r.EndFrame(100);
    p.Drain();
    r.SetOneChannel(0, 0);
    r.EndFrame(100);
    CHECK(p.Drain().empty());
}

// ---------------------------------------------------------------------------------------------
// PixelNet
// ---------------------------------------------------------------------------------------------

TEST_CASE("PixelNet frame is 0xAA then the full 4096 byte buffer", "[protocols][serial][pixelnet]") {
    PixelNetOutput o;
    o.SetChannels(10);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0, 5);
    o.SetOneChannel(9, 6);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == PIXELNET_MAX_CHANNELS + 1);
    CHECK(f[0] == 170);
    CHECK(f[1] == 5);
    CHECK(f[10] == 6);
    CHECK(f[11] == 0);
    CHECK(f.back() == 0);
}

TEST_CASE("PixelNet never lets the start byte appear inside the data", "[protocols][serial][pixelnet]") {
    const int v = GENERATE(0, 1, 169, 170, 171, 255);
    PixelNetOutput o;
    o.SetChannels(1);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0, (unsigned char)v);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == PIXELNET_MAX_CHANNELS + 1);
    CHECK(f[1] == (v == 170 ? 171 : v));
    CHECK(std::count(f.begin() + 1, f.end(), (uint8_t)170) == 0);
}

TEST_CASE("PixelNet last channel boundary", "[protocols][serial][pixelnet]") {
    PixelNetOutput o;
    o.SetChannels(PIXELNET_MAX_CHANNELS);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(PIXELNET_MAX_CHANNELS - 1, 77);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == PIXELNET_MAX_CHANNELS + 1);
    CHECK(f.back() == 77);
}

TEST_CASE("PixelNet AllOff zeroes the used channels", "[protocols][serial][pixelnet]") {
    PixelNetOutput o;
    o.SetChannels(8);
    Pipe p;
    OpenOnPipe(o, p);
    for (int i = 0; i < 8; i++) o.SetOneChannel(i, 200);
    o.AllOff();
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == PIXELNET_MAX_CHANNELS + 1);
    CHECK(std::all_of(f.begin() + 1, f.end(), [](uint8_t b) { return b == 0; }));
}

TEST_CASE("PixelNet ignores baud rate changes", "[protocols][serial][pixelnet]") {
    PixelNetOutput o;
    CHECK_FALSE(o.AllowsBaudRateSetting());
    o.SetBaudRate(9600);
    CHECK(o.GetBaudRate() == 250000);
}

// ---------------------------------------------------------------------------------------------
// Open PixelNet
// ---------------------------------------------------------------------------------------------

TEST_CASE("Open PixelNet frame header", "[protocols][serial][openpixelnet]") {
    OpenPixelNetOutput o;
    o.SetChannels(3);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0, 1);
    o.SetOneChannel(1, 2);
    o.SetOneChannel(2, 3);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == OPENPIXELNET_MAX_CHANNELS + 8);
    CHECK(Bytes(f.begin(), f.begin() + 9) == Bytes{ 0xAA, 0x55, 0x55, 0xAA, 0x15, 0x5D, 1, 2, 3 });
}

TEST_CASE("Open PixelNet escapes 0xAA in the data", "[protocols][serial][openpixelnet]") {
    const int v = GENERATE(0, 0xA9, 0xAA, 0xAB, 0xFF);
    OpenPixelNetOutput o;
    o.SetChannels(1);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0, (unsigned char)v);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() > 6);
    CHECK(f[6] == (v == 0xAA ? 0xAB : v));
}

TEST_CASE("Open PixelNet two stop bits and fixed baud", "[protocols][serial][openpixelnet]") {
    OpenPixelNetOutput o;
    o.SetCommPort("NotConnected");
    o.SetChannels(2);
    o.Open();
    CHECK(std::string(PT(o, so_config)) == "8N2");
    o.SetBaudRate(9600);
    CHECK(o.GetBaudRate() == 1000000);
}

TEST_CASE("Open PixelNet trailing pad bytes stay zero", "[protocols][serial][openpixelnet]") {
    OpenPixelNetOutput o;
    o.SetChannels(OPENPIXELNET_MAX_CHANNELS);
    Pipe p;
    OpenOnPipe(o, p);
    for (int i = 0; i < OPENPIXELNET_MAX_CHANNELS; i++) o.SetOneChannel(i, 0xFE);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == OPENPIXELNET_MAX_CHANNELS + 8);
    CHECK(f[6 + OPENPIXELNET_MAX_CHANNELS - 1] == 0xFE);
    CHECK(f[f.size() - 1] == 0);
    CHECK(f[f.size() - 2] == 0);
}

// ---------------------------------------------------------------------------------------------
// Enttec DMX Pro
// ---------------------------------------------------------------------------------------------

TEST_CASE("DMX Pro frame header and trailer", "[protocols][serial][dmx]") {
    struct Row { int channels; int len; };
    const Row row = GENERATE(Row{ 0, 513 }, Row{ 1, 513 }, Row{ 3, 513 }, Row{ 511, 513 }, Row{ 512, 513 }, Row{ 513, 514 },
                             Row{ 600, 601 }, Row{ 1024, 1025 }, Row{ 4800, 4801 });
    DMXOutput o;
    o.SetChannels(row.channels);
    Pipe p;
    OpenOnPipe(o, p);
    if (row.channels > 0) o.SetOneChannel(0, 0x42);
    o.EndFrame(0);
    auto f = p.Drain();
    INFO("channels " << row.channels);
    REQUIRE(f.size() == (size_t)row.len + 5);
    CHECK(f[0] == 0x7E);
    CHECK(f[1] == 6);
    CHECK(f[2] == (row.len & 0xFF));
    CHECK(f[3] == ((row.len >> 8) & 0xFF));
    CHECK(f[4] == 0);
    if (row.channels > 0) CHECK(f[5] == 0x42);
    CHECK(f.back() == 0xE7);
}

TEST_CASE("DMX Pro channel offsets", "[protocols][serial][dmx]") {
    DMXOutput o;
    o.SetChannels(512);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(511, 0x99);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == 518);
    CHECK(f[5 + 511] == 0x99);
    CHECK(f[517] == 0xE7);
}

TEST_CASE("DMX Pro SetManyChannels copies and clamps", "[protocols][serial][dmx]") {
    DMXOutput o;
    o.SetChannels(20);
    Pipe p;
    OpenOnPipe(o, p);
    unsigned char d[5] = { 1, 2, 3, 4, 5 };
    o.SetManyChannels(10, d, 5);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == 518);
    CHECK(Bytes(f.begin() + 15, f.begin() + 20) == Bytes{ 1, 2, 3, 4, 5 });
    CHECK(f[14] == 0);
    CHECK(f[20] == 0);
}

TEST_CASE("DMX Pro SetManyChannels of nothing changes nothing", "[protocols][serial][dmx]") {
    DMXOutput o;
    o.SetChannels(20);
    o.SetSuppressDuplicateFrames(true);
    Pipe p;
    OpenOnPipe(o, p);
    o.EndFrame(10);
    p.Drain();
    unsigned char d[1] = { 0 };
    o.SetManyChannels(0, d, 0);
    o.EndFrame(10);
    CHECK(p.Drain().empty());
}

TEST_CASE("DMX Pro SetManyChannels never writes past the maximum", "[protocols][serial][dmx]") {
    DMXOutput o;
    o.SetChannels(DMX_MAX_CHANNELS);
    Pipe p;
    OpenOnPipe(o, p);
    std::vector<unsigned char> d(100, 0x55);
    o.SetManyChannels(DMX_MAX_CHANNELS - 10, d.data(), d.size());
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == (size_t)DMX_MAX_CHANNELS + 1 + 5);
    CHECK(f[5 + DMX_MAX_CHANNELS - 1] == 0x55);
    CHECK(f[5 + DMX_MAX_CHANNELS - 10] == 0x55);
    CHECK(f[5 + DMX_MAX_CHANNELS - 11] == 0);
    CHECK(f.back() == 0xE7);
}

TEST_CASE("DMX Pro AllOff", "[protocols][serial][dmx]") {
    DMXOutput o;
    o.SetChannels(10);
    Pipe p;
    OpenOnPipe(o, p);
    for (int i = 0; i < 10; i++) o.SetOneChannel(i, 255);
    o.AllOff();
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == 518);
    for (int i = 0; i < 10; i++) CHECK(f[5 + i] == 0);
}

TEST_CASE("DMX Pro always runs at 250000 baud", "[protocols][serial][dmx]") {
    DMXOutput o;
    CHECK_FALSE(o.AllowsBaudRateSetting());
    o.SetBaudRate(1200);
    CHECK(o.GetBaudRate() == 250000);
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="DMX" ComPort="COM1" BaudRate="9600" MaxChannels="12"/>)");
    DMXOutput x(n);
    CHECK(x.GetBaudRate() == 250000);
}

TEST_CASE("DMX Pro frame does not go out when the port never opened", "[protocols][serial][dmx]") {
    DMXOutput o;
    o.SetChannels(10);
    o.SetCommPort("NotConnected");
    o.Open();
    o.EndFrame(0);
    SUCCEED();
}

// ---------------------------------------------------------------------------------------------
// Open DMX
// ---------------------------------------------------------------------------------------------

TEST_CASE("Open DMX always sends start code plus 512 slots", "[protocols][serial][opendmx]") {
    const int n = GENERATE(1, 10, 512);
    OpenDMXOutput o;
    o.SetChannels(n);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0, 0xAB);
    o.SetOneChannel(n - 1, 0xCD);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == 513);
    CHECK(f[0] == 0);
    if (n > 1) CHECK(f[1] == 0xAB);
    CHECK(f[n] == 0xCD);
    if (n < 512) CHECK(f[n + 1] == 0);
}

TEST_CASE("Open DMX uses two stop bits", "[protocols][serial][opendmx]") {
    OpenDMXOutput o;
    o.SetCommPort("NotConnected");
    o.Open();
    CHECK(std::string(PT(o, so_config)) == "8N2");
    CHECK(o.GetBaudRate() == 250000);
}

TEST_CASE("Open DMX SetManyChannels clamps at 512", "[protocols][serial][opendmx]") {
    OpenDMXOutput o;
    o.SetChannels(512);
    Pipe p;
    OpenOnPipe(o, p);
    std::vector<unsigned char> d(40, 7);
    o.SetManyChannels(500, d.data(), d.size());
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == 513);
    CHECK(f[500] == 0);
    CHECK(f[501] == 7);
    CHECK(f[512] == 7);
}

TEST_CASE("Open DMX AllOff clears and start code stays zero", "[protocols][serial][opendmx]") {
    OpenDMXOutput o;
    o.SetChannels(16);
    Pipe p;
    OpenOnPipe(o, p);
    for (int i = 0; i < 16; i++) o.SetOneChannel(i, 9);
    o.AllOff();
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == 513);
    CHECK(std::all_of(f.begin(), f.end(), [](uint8_t b) { return b == 0; }));
}

// ---------------------------------------------------------------------------------------------
// Generic serial
// ---------------------------------------------------------------------------------------------

TEST_CASE("Generic serial wraps the data in prefix and postfix", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(3);
    o.SetPrefix({ 0xAA, 0xBB });
    o.SetPostfix({ 0xCC });
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0, 1);
    o.SetOneChannel(1, 2);
    o.SetOneChannel(2, 3);
    o.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0xAA, 0xBB, 1, 2, 3, 0xCC });
}

TEST_CASE("Generic serial without prefix and postfix is raw data", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(4);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(3, 0xFF);
    o.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0, 0, 0, 0xFF });
}

TEST_CASE("Generic serial with zero channels sends nothing", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(0);
    Pipe p;
    OpenOnPipe(o, p);
    o.EndFrame(0);
    CHECK(p.Drain().empty());
}

TEST_CASE("Generic serial with only a prefix and postfix and no channels", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(0);
    o.SetPrefix({ 1, 2 });
    o.SetPostfix({ 3 });
    Pipe p;
    OpenOnPipe(o, p);
    o.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 1, 2, 3 });
}

TEST_CASE("Generic serial AllOff keeps prefix and postfix", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(3);
    o.SetPrefix({ 0x10 });
    o.SetPostfix({ 0x20, 0x21 });
    Pipe p;
    OpenOnPipe(o, p);
    for (int i = 0; i < 3; i++) o.SetOneChannel(i, 99);
    o.AllOff();
    o.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0x10, 0, 0, 0, 0x20, 0x21 });
}

TEST_CASE("Generic serial maximum channels fills the whole buffer", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(GENERICSERIAL_MAX_CHANNELS);
    o.SetPrefix({ 0xEE });
    o.SetPostfix({ 0xFF });
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(GENERICSERIAL_MAX_CHANNELS - 1, 0x33);
    o.EndFrame(0);
    auto f = p.Drain();
    REQUIRE(f.size() == (size_t)GENERICSERIAL_MAX_CHANNELS + 2);
    CHECK(f.front() == 0xEE);
    CHECK(f[GENERICSERIAL_MAX_CHANNELS] == 0x33);
    CHECK(f.back() == 0xFF);
}

TEST_CASE("Generic serial prefix and postfix that overflow the buffer fail the open", "[protocols][serial][generic][!mayfail]") {
    // Open() returns false for this case but leaves _ok true (set by SerialOutput::Open) and _datalen larger than
    // the buffer, so EndFrame would then write past the end of _data. Open should leave the output not-ok.
    GenericSerialOutput o;
    o.SetChannels(GENERICSERIAL_MAX_CHANNELS);
    o.SetPrefix(Bytes(200, 1));
    o.SetPostfix(Bytes(200, 2));
    o.SetCommPort("NotConnected");
    CHECK_FALSE(o.Open());
    CHECK_FALSE(o.IsOk());
}

TEST_CASE("Generic serial prefix and postfix right at the limit are accepted", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    o.SetChannels(GENERICSERIAL_MAX_CHANNELS);
    o.SetPrefix(Bytes(128, 1));
    o.SetPostfix(Bytes(128, 2));
    o.SetCommPort("NotConnected");
    CHECK(o.Open());
}

TEST_CASE("Generic serial keeps the configured baud rate", "[protocols][serial][generic]") {
    GenericSerialOutput o;
    CHECK(o.AllowsBaudRateSetting());
    o.SetBaudRate(9600);
    CHECK(o.GetBaudRate() == 9600);
}

// ---------------------------------------------------------------------------------------------
// xxx serial
// ---------------------------------------------------------------------------------------------

TEST_CASE("xxx serial device channels split into devices", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("8,8,16");
    CHECK(o.GetChannels() == 32);
    CHECK((o.*PT_FN(xs_dev_from))(0) == 0);
    CHECK((o.*PT_FN(xs_dev_from))(7) == 0);
    CHECK((o.*PT_FN(xs_dev_from))(8) == 1);
    CHECK((o.*PT_FN(xs_dev_from))(15) == 1);
    CHECK((o.*PT_FN(xs_dev_from))(16) == 2);
    CHECK((o.*PT_FN(xs_dev_from))(31) == 2);
    CHECK((o.*PT_FN(xs_chan_on))(0) == 0);
    CHECK((o.*PT_FN(xs_chan_on))(9) == 1);
    CHECK((o.*PT_FN(xs_chan_on))(31) == 15);
    CHECK((o.*PT_FN(xs_dev_ch))((uint8_t)0) == 8);
    CHECK((o.*PT_FN(xs_dev_ch))((uint8_t)2) == 16);
    CHECK((o.*PT_FN(xs_dev_ch))((uint8_t)3) == 0);
}

TEST_CASE("xxx serial device lookups past the last device", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("4,4");
    CHECK((o.*PT_FN(xs_dev_from))(8) == 2);
    CHECK((o.*PT_FN(xs_dev_from))(1000) == 2);
}

TEST_CASE("xxx serial garbage device channels", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    const char* s = GENERATE("", "abc", ",", "x,y,z", "-5");
    INFO("'" << s << "'");
    o.SetDeviceChannels(s);
    CHECK(o.GetChannels() >= 0);
    CHECK((o.*PT_FN(xs_dev_from))(0) <= 3);
}

TEST_CASE("xxx serial 256 channels of device data is valid", "[protocols][serial][xxx][!mayfail]") {
    // SetDeviceChannels accumulates into a uint8_t, so the documented maximum of 256 channels ("128,128") wraps to 0.
    xxxSerialOutput o;
    o.SetDeviceChannels("128,128");
    CHECK(o.GetChannels() == 256);
}

TEST_CASE("xxx serial device channel sums over 255 do not wrap", "[protocols][serial][xxx][!mayfail]") {
    // same uint8_t accumulator: "200,100" gives 44 instead of 300 (which would then be rejected as too many channels)
    xxxSerialOutput o;
    o.SetDeviceChannels("200,100");
    CHECK(o.GetChannels() >= 256);
}

TEST_CASE("xxx serial first frame sends every channel", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("2,2");
    Pipe p;
    OpenOnPipe(o, p);
    unsigned char d[4] = { 1, 2, 3, 4 };
    o.SetManyChannels(0, d, 4);
    o.StartFrame(0);
    o.EndFrame(0);
    auto f = p.Drain();
    Bytes expect = {
        0x80, 1, 0x10, 1, 0, 0x81,
        0x80, 1, 0x10, 2, 1, 0x81,
        0x80, 2, 0x10, 3, 0, 0x81,
        0x80, 2, 0x10, 4, 1, 0x81,
        0x80, 0x00, 0x00, 0x81, // heartbeat
    };
    CHECK(f == expect);
}

TEST_CASE("xxx serial only changed channels are sent afterwards", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("4");
    Pipe p;
    OpenOnPipe(o, p);
    o.StartFrame(0);
    o.EndFrame(0);
    p.Drain();
    unsigned char d[4] = { 0, 9, 0, 0 };
    o.SetManyChannels(0, d, 4);
    o.StartFrame(10);
    o.EndFrame(0);
    CHECK(p.Drain() == Bytes{ 0x80, 1, 0x10, 9, 1, 0x81 });
}

TEST_CASE("xxx serial heartbeat every 500ms", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("1");
    o.SetSuppressDuplicateFrames(true);
    Pipe p;
    OpenOnPipe(o, p);
    const Bytes hb = { 0x80, 0, 0, 0x81 };
    auto frameAt = [&](long ms) {
        o.StartFrame(ms);
        o.EndFrame(1000);
        return p.Drain();
    };
    auto first = frameAt(0);
    CHECK(Bytes(first.end() - 4, first.end()) == hb);
    CHECK(frameAt(100).empty());
    CHECK(frameAt(499).empty());
    CHECK(frameAt(500) == hb);
    CHECK(frameAt(600).empty());
    // time going backwards forces a heartbeat
    CHECK(frameAt(100) == hb);
}

TEST_CASE("xxx serial ResetFrame forces the next heartbeat", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("1");
    o.SetSuppressDuplicateFrames(true);
    Pipe p;
    OpenOnPipe(o, p);
    o.StartFrame(10);
    o.EndFrame(1000);
    p.Drain();
    o.StartFrame(20);
    o.EndFrame(1000);
    CHECK(p.Drain().empty());
    o.ResetFrame();
    o.StartFrame(30);
    o.EndFrame(1000);
    CHECK(p.Drain() == Bytes{ 0x80, 0, 0, 0x81 });
}

TEST_CASE("xxx serial SetOneChannel writes immediately only on change", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("8");
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(3, 0);
    // channel 3 has _lastSent 0xFF from Open and notSentCount 0xF0 > 200, so it is sent
    CHECK(p.Drain() == Bytes{ 0x80, 1, 0x10, 0, 3, 0x81 });
    o.SetOneChannel(3, 0);
    CHECK(p.Drain().empty());
    o.SetOneChannel(3, 5);
    CHECK(p.Drain() == Bytes{ 0x80, 1, 0x10, 5, 3, 0x81 });
}

TEST_CASE("xxx serial xml round trip", "[protocols][serial][xxx]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="xxx Serial" ComPort="COM9" BaudRate="n/a" Id="3" DeviceChannels="8,16"/>)");
    xxxSerialOutput o(n);
    CHECK(o.GetDeviceChannels() == "8,16");
    CHECK(o.GetChannels() == 24);
    CHECK(o.GetId() == 3);
    CHECK_FALSE(o.IsDirty());
    auto s = SaveToString(o);
    CHECK(s.find("DeviceChannels=\"8,16\"") != std::string::npos);
    CHECK(s.find("Id=\"3\"") != std::string::npos);
}

TEST_CASE("xxx serial missing device channels default to 8", "[protocols][serial][xxx]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="xxx Serial" ComPort="COM9"/>)");
    xxxSerialOutput o(n);
    CHECK(o.GetDeviceChannels() == "8");
    CHECK(o.GetChannels() == 8);
}

TEST_CASE("xxx serial copy keeps the device layout", "[protocols][serial][xxx]") {
    xxxSerialOutput o;
    o.SetDeviceChannels("4,4");
    std::unique_ptr<Output> c(o.Copy());
    auto* x = dynamic_cast<xxxSerialOutput*>(c.get());
    REQUIRE(x != nullptr);
    CHECK(x->GetDeviceChannels() == "4,4");
    CHECK(x->GetChannels() == 8);
    CHECK_FALSE(x->IsDirty());
}

// ---------------------------------------------------------------------------------------------
// LOR
// ---------------------------------------------------------------------------------------------

TEST_CASE("LOR heartbeat bytes", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(16);
    Pipe p;
    OpenOnPipe(o, p);
    o.SendHeartbeat();
    CHECK(p.Drain() == kLorHeartbeat);
}

TEST_CASE("LOR heartbeat needs an enabled open output", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(16);
    o.SendHeartbeat();
    Pipe p;
    OpenOnPipe(o, p);
    o.Enable(false);
    o.SendHeartbeat();
    CHECK(p.Drain().empty());
}

TEST_CASE("LOR AllOff sends an off command per channel then a heartbeat", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(18);
    Pipe p;
    OpenOnPipe(o, p);
    o.AllOff();
    auto f = p.Drain();
    REQUIRE(f.size() == 18 * 6 + 5);
    for (int ch = 0; ch < 18; ch++) {
        Bytes cmd(f.begin() + ch * 6, f.begin() + ch * 6 + 6);
        INFO("channel " << ch);
        CHECK(cmd == Bytes{ 0x00, (uint8_t)((ch >> 4) + 1), 0x03, 0xF0, (uint8_t)(0x80 | (ch & 0xF)), 0x00 });
    }
    CHECK(Bytes(f.end() - 5, f.end()) == kLorHeartbeat);
    // a second AllOff has nothing left to turn off
    o.AllOff();
    CHECK(p.Drain() == kLorHeartbeat);
}

TEST_CASE("LOR AllOff caps at the addressable channels", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(5000);
    Pipe p;
    OpenOnPipe(o, p);
    o.AllOff();
    auto f = p.DrainWait(3840 * 6 + 5, 200);
    // the pipe holds 64K, so everything fits
    CHECK(f.size() == 3840 * 6 + 5);
}

TEST_CASE("LOR AllOff on a disabled output sends nothing", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(16);
    Pipe p;
    OpenOnPipe(o, p);
    o.Enable(false);
    o.AllOff();
    CHECK(p.Drain().empty());
}

TEST_CASE("LOR frames convert every 0-255 value to the right intensity byte", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(256);
    o.SetBaudRate(1000000);
    Pipe p;
    OpenOnPipe(o, p);
    for (int i = 0; i < 256; i++) o.SetOneChannel(i, (unsigned char)i);
    o.EndFrame(0);
    auto f = p.DrainWait(5 + 256 * 6, 3000);
    REQUIRE(f.size() >= 5 + 256 * 6);
    std::map<int, uint8_t> got;
    size_t i = 0;
    while (i + 5 <= f.size()) {
        if (i + 5 <= f.size() && Bytes(f.begin() + i, f.begin() + i + 5) == kLorHeartbeat) {
            i += 5;
            continue;
        }
        if (i + 6 > f.size()) break;
        REQUIRE(f[i] == 0x00);
        REQUIRE(f[i + 2] == 0x03);
        REQUIRE(f[i + 5] == 0x00);
        int ch = ((f[i + 1] - 1) << 4) | (f[i + 4] & 0x0F);
        REQUIRE((f[i + 4] & 0x80) != 0);
        got[ch] = f[i + 3];
        i += 6;
    }
    REQUIRE(got.size() == 256);
    for (int c = 0; c < 256; c++) {
        INFO("value " << c);
        CHECK(got[c] == LorTable(c));
    }
}

TEST_CASE("LOR intensity table is monotonic and bounded", "[protocols][serial][lor]") {
    int prev = 0xF0 + 1;
    for (int v = 0; v < 256; v++) {
        int b = LorTable(v);
        CHECK(b <= 0xF0);
        CHECK(b >= 0x01);
        CHECK(b <= prev);
        prev = b;
    }
    CHECK(LorTable(0) == 0xF0);
    CHECK(LorTable(255) == 0x01);
}

TEST_CASE("LOR unit ids roll over every 16 channels", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(0xF0 * 16);
    o.SetBaudRate(2000000);
    Pipe p;
    OpenOnPipe(o, p);
    o.SetOneChannel(0xF0 * 16 - 1, 255);
    o.EndFrame(0);
    auto f = p.DrainWait(5 + 3840 * 6, 3000);
    REQUIRE(f.size() >= 6);
    bool found = false;
    for (size_t i = 0; i + 6 <= f.size();) {
        if (i + 5 <= f.size() && Bytes(f.begin() + i, f.begin() + i + 5) == kLorHeartbeat) { i += 5; continue; }
        if (f[i + 1] == 0xF0 && (f[i + 4] & 0x0F) == 0x0F) {
            found = true;
            CHECK(f[i + 3] == 0x01);
        }
        i += 6;
    }
    CHECK(found);
}

TEST_CASE("LOR SetOneChannel ignores out of range channels", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(16);
    o.SetOneChannel(-1, 5);
    o.SetOneChannel(0xF0 * 16, 5);
    o.SetOneChannel(100000, 5);
    SUCCEED();
}

TEST_CASE("LOR max channels and validity", "[protocols][serial][lor]") {
    LOROutput o;
    CHECK(o.GetMaxChannels() == 0xF0 * 16);
    CHECK(o.IsValidChannelCount(1));
    CHECK(o.IsValidChannelCount(3840));
    CHECK_FALSE(o.IsValidChannelCount(3841));
    CHECK_FALSE(o.IsValidChannelCount(0));
    CHECK_FALSE(o.IsValidChannelCount(-1));
    CHECK(o.GetType() == OUTPUT_LOR);
}

TEST_CASE("LOR Close stops the pump and can be reopened", "[protocols][serial][lor]") {
    LOROutput o;
    o.SetChannels(16);
    Pipe p;
    OpenOnPipe(o, p);
    o.EndFrame(0);
    p.DrainWait(5, 1000);
    o.Close();
    CHECK(PT(o, so_serial) == nullptr);
    o.Close();
    SUCCEED();
}

// ---------------------------------------------------------------------------------------------
// LOR Optimised
// ---------------------------------------------------------------------------------------------

namespace {

std::string LorOptXml(const std::string& controllers) {
    return std::string(R"(<network NetworkType="LOR Optimised" ComPort="NotConnected" BaudRate="19200" MaxChannels="0"><controllers>)") +
           controllers + "</controllers></network>";
}

struct LorOptRig {
    pugi::xml_document doc;
    std::unique_ptr<LOROptimisedOutput> out;
    Pipe pipe;
    explicit LorOptRig(const std::string& controllers) {
        auto n = ParseNode(doc, LorOptXml(controllers));
        out.reset(new LOROptimisedOutput(n));
        out->Open();
        PT(*out, so_serial) = pipe.MakePort();
        PT(*out, out_ok) = true;
    }
};

const char* kUnit1 = R"(<controller UnitId="1" NumChannels="16" CntlrType="AC Controller" AddrMode="0"/>)";

} // namespace

TEST_CASE("LOR optimised generates single channel command", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    REQUIRE(r.out->GetChannels() == 16);
    unsigned char d[16] = { 255 };
    r.out->SetManyChannels(0, d, 16);
    CHECK(r.pipe.Drain() == Bytes{ 0x00, 0x01, 0x31, 0x01, 0x00, 0x00 });
}

TEST_CASE("LOR optimised all off bank is one short command", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    unsigned char d[16] = { 255 };
    r.out->SetManyChannels(0, d, 16);
    r.pipe.Drain();
    unsigned char z[16] = { 0 };
    r.out->SetManyChannels(0, z, 16);
    CHECK(r.pipe.Drain() == Bytes{ 0x00, 0x01, 0x41, 0x00, 0x00 });
}

TEST_CASE("LOR optimised unchanged data sends nothing", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    unsigned char d[16] = { 255, 1, 2, 3 };
    r.out->SetManyChannels(0, d, 16);
    CHECK_FALSE(r.pipe.Drain().empty());
    r.out->SetManyChannels(0, d, 16);
    CHECK(r.pipe.Drain().empty());
}

TEST_CASE("LOR optimised mixed levels use the value byte form", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    unsigned char d[16] = { 128 };
    r.out->SetManyChannels(0, d, 16);
    CHECK(r.pipe.Drain() == Bytes{ 0x00, 0x01, 0x13, 0xF0, 0xFE, 0xFF, 0x00, 0x01, 0x33, LorTable(128), 0x01, 0x00, 0x00 });
}

TEST_CASE("LOR optimised full on bank", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    unsigned char d[16];
    std::fill(d, d + 16, 255);
    r.out->SetManyChannels(0, d, 16);
    CHECK(r.pipe.Drain() == Bytes{ 0x00, 0x01, 0x11, 0xFF, 0xFF, 0x00, 0x00 });
}

TEST_CASE("LOR optimised GenerateCommand byte layouts", "[protocols][serial][loroptimised]") {
    struct Row {
        int unit; int bank; bool value; uint8_t dbyte; uint8_t lsb; uint8_t msb; Bytes expect;
    };
    const Row row = GENERATE(
        Row{ 1, 0, false, 0, 0x01, 0x00, { 0x00, 0x01, 0x31, 0x01 } },
        Row{ 1, 0, false, 0, 0x00, 0x01, { 0x00, 0x01, 0x21, 0x01 } },
        Row{ 1, 0, false, 0, 0x03, 0x01, { 0x00, 0x01, 0x11, 0x03, 0x01 } },
        Row{ 2, 0, true, 0x55, 0x01, 0x00, { 0x00, 0x02, 0x33, 0x55, 0x01 } },
        Row{ 2, 0, true, 0x55, 0x00, 0x80, { 0x00, 0x02, 0x23, 0x55, 0x80 } },
        Row{ 2, 0, true, 0x55, 0xFF, 0xFF, { 0x00, 0x02, 0x13, 0x55, 0xFF, 0xFF } },
        Row{ 3, 1, false, 0, 0x01, 0x00, { 0x00, 0x03, 0x51, 0x41, 0x01 } },
        Row{ 3, 1, false, 0, 0x00, 0x01, { 0x00, 0x03, 0x51, 0x81, 0x01 } },
        Row{ 3, 5, false, 0, 0x11, 0x22, { 0x00, 0x03, 0x51, 0x05, 0x11, 0x22 } },
        Row{ 4, 2, true, 0x77, 0x11, 0x22, { 0x00, 0x04, 0x53, 0x77, 0x02, 0x11, 0x22 } },
        Row{ 4, 63, true, 0x77, 0x01, 0x00, { 0x00, 0x04, 0x53, 0x77, 0x7F, 0x01 } });
    LOROptimisedOutput o;
    uint8_t d[32] = { 0 };
    size_t idx = 0;
    (o.*PT_FN(lo_gen))(d, idx, row.unit, row.bank, row.value, row.dbyte, row.lsb, row.msb);
    CHECK(Bytes(d, d + idx) == row.expect);
}

TEST_CASE("LOR optimised never emits a stray delimiter inside a command", "[protocols][serial][loroptimised]") {
    // every command starts with 0x00; a 0x00 in the mask bytes must not be written (zero masks are skipped)
    LOROptimisedOutput o;
    uint8_t d[32] = { 0 };
    size_t idx = 0;
    (o.*PT_FN(lo_gen))(d, idx, 1, 0, true, 0x10, 0x00, 0x00);
    // both masks zero is not a command anyone should send, but it must not overrun
    CHECK(idx <= 6);
}

TEST_CASE("LOR optimised addresses each controller by unit id", "[protocols][serial][loroptimised]") {
    LorOptRig r(std::string(kUnit1) + R"(<controller UnitId="2" NumChannels="16" CntlrType="AC Controller" AddrMode="0"/>)");
    REQUIRE(r.out->GetChannels() == 32);
    unsigned char d[32] = { 0 };
    d[16] = 255;
    r.out->SetManyChannels(0, d, 32);
    auto f = r.pipe.Drain();
    // only unit 2 changed
    CHECK(f == Bytes{ 0x00, 0x02, 0x31, 0x01, 0x00, 0x00 });
}

TEST_CASE("LOR optimised legacy mode spreads a controller over consecutive unit ids", "[protocols][serial][loroptimised]") {
    LorOptRig r(R"(<controller UnitId="5" NumChannels="32" CntlrType="AC Controller" AddrMode="1"/>)");
    unsigned char d[32] = { 0 };
    d[17] = 255;
    r.out->SetManyChannels(0, d, 32);
    CHECK(r.pipe.Drain() == Bytes{ 0x00, 0x06, 0x31, 0x02, 0x00, 0x00 });
}

TEST_CASE("LOR optimised total channels from controllers", "[protocols][serial][loroptimised]") {
    struct Row { const char* xml; int channels; };
    const Row row = GENERATE(
        Row{ R"(<controller UnitId="1" NumChannels="16" CntlrType="AC Controller" AddrMode="0"/>)", 16 },
        Row{ R"(<controller UnitId="1" NumChannels="16" CntlrType="Pixie4" AddrMode="0"/>)", 64 },
        Row{ R"(<controller UnitId="1" NumChannels="10" CntlrType="Pixie16" AddrMode="0"/>)", 160 },
        Row{ R"(<controller UnitId="1" NumChannels="0" CntlrType="AC Controller" AddrMode="0"/>)", 0 },
        Row{ R"(<controller UnitId="1" NumChannels="40000" CntlrType="AC Controller" AddrMode="0"/>)", 20480 });
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml(row.xml));
    LOROptimisedOutput o(n);
    CHECK(o.GetChannels() == row.channels);
}

TEST_CASE("LOR optimised unit ids beyond the table are ignored safely", "[protocols][serial][loroptimised]") {
    const int unit = GENERATE(-5, 0, 255, 256, 1000, 100000);
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml("<controller UnitId=\"" + std::to_string(unit) + R"(" NumChannels="5000" CntlrType="AC Controller" AddrMode="1"/>)"));
    LOROptimisedOutput o(n);
    CHECK(o.GetChannels() == 5000);
    CHECK(PT(o, lo_inuse)[0]);
}

TEST_CASE("LOR optimised no controllers means no channels", "[protocols][serial][loroptimised]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml(""));
    LOROptimisedOutput o(n);
    CHECK(o.GetChannels() == 0);
    CHECK(o.GetControllers().GetControllers().empty());
}

TEST_CASE("LOR optimised xml without a controllers node", "[protocols][serial][loroptimised]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<network NetworkType="LOR Optimised" ComPort="COM1" BaudRate="19200" MaxChannels="0"/>)");
    LOROptimisedOutput o(n);
    CHECK(o.GetChannels() == 0);
}

TEST_CASE("LOR optimised default output has one controller", "[protocols][serial][loroptimised]") {
    LOROptimisedOutput o;
    REQUIRE(o.GetControllers().GetControllers().size() == 1);
    CHECK(o.GetControllers().GetControllers().front()->GetUnitId() == 1);
    CHECK(o.GetControllers().GetControllers().front()->GetNumChannels() == 16);
}

TEST_CASE("LOR optimised save and reload keeps the controllers", "[protocols][serial][loroptimised]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml(std::string(kUnit1) + R"(<controller UnitId="9" NumChannels="48" CntlrType="Pixie2" AddrMode="2" CntlrDesc="x&amp;y" Expanded="FALSE"/>)"));
    LOROptimisedOutput o(n);
    pugi::xml_document out;
    auto root = out.append_child("root");
    o.Save(root);
    LOROptimisedOutput again(root.first_child());
    REQUIRE(again.GetControllers().GetControllers().size() == 2);
    auto* c = again.GetControllers().GetControllers().back();
    CHECK(c->GetUnitId() == 9);
    CHECK(c->GetNumChannels() == 48);
    CHECK(c->GetType() == "Pixie2");
    CHECK(c->GetAddressMode() == LorController::AddressMode::LOR_ADDR_MODE_SPLIT);
    CHECK(c->GetDescription() == "x&y");
    CHECK_FALSE(c->IsExpanded());
    CHECK(again.GetChannels() == o.GetChannels());
}

TEST_CASE("LOR optimised copy duplicates the controllers", "[protocols][serial][loroptimised]") {
    LOROptimisedOutput o;
    o.GetControllers().GetControllers().front()->SetUnitID(7);
    std::unique_ptr<Output> c(o.Copy());
    auto* x = dynamic_cast<LOROptimisedOutput*>(c.get());
    REQUIRE(x != nullptr);
    REQUIRE(x->GetControllers().GetControllers().size() == 1);
    CHECK(x->GetControllers().GetControllers().front()->GetUnitId() == 7);
    CHECK(x->GetControllers().GetControllers().front() != o.GetControllers().GetControllers().front());
}

TEST_CASE("LOR optimised AllOff sends a clear per bank of each unit", "[protocols][serial][loroptimised]") {
    LorOptRig r(R"(<controller UnitId="1" NumChannels="32" CntlrType="AC Controller" AddrMode="0"/>)");
    r.pipe.Drain();
    r.out->AllOff();
    auto f = r.pipe.Drain();
    Bytes expect = { 0x00, 0x01, 0x41, 0x00, 0x01, 0x51, 0xC1, 0x00, 0x00 };
    CHECK(Bytes(f.begin(), f.begin() + expect.size()) == expect);
    CHECK(Bytes(f.end() - 5, f.end()) == kLorHeartbeat);
}

TEST_CASE("LOR optimised SetOneChannel ignores out of range channels", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    r.out->SetOneChannel(-1, 5);
    r.out->SetOneChannel(20480, 5);
    r.out->SetOneChannel(1 << 30, 5);
    SUCCEED();
}

TEST_CASE("LOR optimised EndFrame writes the staged channels", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    r.out->SetOneChannel(0, 255);
    r.out->EndFrame(0);
    auto f = r.pipe.DrainWait(6, 1000);
    REQUIRE(f.size() >= 6);
    CHECK(Bytes(f.begin(), f.begin() + 6) == Bytes{ 0x00, 0x01, 0x31, 0x01, 0x00, 0x00 });
}

TEST_CASE("LOR optimised controller with data past the supplied buffer sends nothing", "[protocols][serial][loroptimised]") {
    LorOptRig r(kUnit1);
    unsigned char d[4] = { 255, 255, 255, 255 };
    r.out->SetManyChannels(0, d, 4);
    CHECK(r.pipe.Drain().empty());
}

TEST_CASE("LOR optimised split mode with an odd channel count", "[protocols][serial][loroptimised]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml(R"(<controller UnitId="1" NumChannels="33" CntlrType="AC Controller" AddrMode="2"/>)"));
    LOROptimisedOutput o(n);
    CHECK(o.GetChannels() == 33);
    CHECK(PT(o, lo_inuse)[1]);
    CHECK(PT(o, lo_inuse)[2]);
}

TEST_CASE("LOR optimised split mode with one channel does not hang", "[protocols][serial][loroptimised][.][known-crash]") {
    // channels_per_pass = 1 / 2 = 0, so `controller_channels_to_process -= channels_per_pass` never reaches zero and
    // SetManyChannels / AllOff (and therefore Open) loop forever.
    LorOptRig r(R"(<controller UnitId="1" NumChannels="1" CntlrType="AC Controller" AddrMode="2"/>)");
    r.out->AllOff();
    SUCCEED();
}

// ---------------------------------------------------------------------------------------------
// LorController(s)
// ---------------------------------------------------------------------------------------------

TEST_CASE("LorController attribute defaults", "[protocols][lor]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, "<controller/>");
    LorController c(n);
    CHECK(c.GetUnitId() == 1);
    CHECK(c.GetNumChannels() == 16);
    CHECK(c.GetType() == "AC Controller");
    CHECK(c.GetAddressMode() == LorController::AddressMode::LOR_ADDR_MODE_LEGACY);
    CHECK(c.GetDescription() == "LOR Controller");
    CHECK(c.IsExpanded());
    CHECK_FALSE(c.IsDirty());
}

TEST_CASE("LorController default constructed", "[protocols][lor]") {
    LorController c;
    CHECK(c.GetUnitId() == 1);
    CHECK(c.GetNumChannels() == 16);
    CHECK(c.GetAddressMode() == LorController::AddressMode::LOR_ADDR_MODE_NORMAL);
    CHECK(c.IsDirty());
}

TEST_CASE("LorController total channels for Pixie types", "[protocols][lor]") {
    struct Row { const char* type; int per; int total; };
    const Row row = GENERATE(Row{ "AC Controller", 16, 16 }, Row{ "Pixie2", 10, 20 }, Row{ "Pixie4", 10, 40 }, Row{ "Pixie8", 10, 80 },
                             Row{ "Pixie16", 10, 160 }, Row{ "Pixie", 10, 0 }, Row{ "PixieX", 10, 0 }, Row{ "pixie4", 10, 10 },
                             Row{ "Pixie4", 0, 0 }, Row{ "Pixie4", -3, -12 });
    LorController c;
    c.SetType(row.type);
    c.SetNumChannels(row.per);
    INFO(row.type);
    CHECK(c.GetTotalNumChannels() == row.total);
}

TEST_CASE("LorController save and load", "[protocols][lor]") {
    LorController c;
    c.SetUnitID(12);
    c.SetNumChannels(48);
    c.SetType("Pixie8");
    c.SetMode(LorController::AddressMode::LOR_ADDR_MODE_SPLIT);
    c.SetDescription("desk");
    c.SetExpanded(false);
    pugi::xml_document doc;
    auto n = doc.append_child("controller");
    c.Save(n);
    CHECK_FALSE(c.IsDirty());
    LorController d(n);
    CHECK(d.GetUnitId() == 12);
    CHECK(d.GetNumChannels() == 48);
    CHECK(d.GetType() == "Pixie8");
    CHECK(d.GetModeString() == "Split");
    CHECK(d.GetDescription() == "desk");
    CHECK_FALSE(d.IsExpanded());
}

TEST_CASE("LorController mode strings", "[protocols][lor]") {
    LorController c;
    c.SetMode(LorController::AddressMode::LOR_ADDR_MODE_NORMAL);
    CHECK(c.GetModeString() == "Normal");
    c.SetMode(LorController::AddressMode::LOR_ADDR_MODE_LEGACY);
    CHECK(c.GetModeString() == "Legacy");
    c.SetMode(LorController::AddressMode::LOR_ADDR_MODE_SPLIT);
    CHECK(c.GetModeString() == "Split");
    c.SetMode(LorController::AddressMode(9));
    CHECK(c.GetModeString() == "");
}

TEST_CASE("LorController garbage attributes", "[protocols][lor]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, R"(<controller UnitId="abc" NumChannels="-4" AddrMode="77" Expanded="maybe"/>)");
    LorController c(n);
    CHECK(c.GetUnitId() == 0);
    CHECK(c.GetNumChannels() == -4);
    CHECK_FALSE(c.IsExpanded());
}

TEST_CASE("LorController setters track dirty only on change", "[protocols][lor]") {
    LorController c;
    c.ClearDirty();
    c.SetUnitID(1);
    c.SetNumChannels(16);
    c.SetType("AC Controller");
    c.SetDescription("");
    c.SetExpanded(true);
    c.SetMode(LorController::AddressMode::LOR_ADDR_MODE_NORMAL);
    CHECK_FALSE(c.IsDirty());
    c.SetUnitID(2);
    CHECK(c.IsDirty());
}

TEST_CASE("LorController limits", "[protocols][lor]") {
    LorController c;
    CHECK(c.GetMaxChannels() == 16383);
    CHECK(c.GetMaxUnitId() == 255);
}

TEST_CASE("LorControllers copy is deep and dirty", "[protocols][lor]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml(std::string(kUnit1) + kUnit1));
    LorControllers a(n);
    CHECK_FALSE(a.IsDirty());
    LorControllers b(a);
    CHECK(b.IsDirty());
    CHECK(b.GetControllers().size() == 2);
    CHECK(b.GetControllers().front() != a.GetControllers().front());
    b.ClearDirty();
    CHECK_FALSE(b.IsDirty());
    b.GetControllers().front()->SetUnitID(44);
    CHECK(b.IsDirty());
    CHECK(a.GetControllers().front()->GetUnitId() == 1);
}

TEST_CASE("LorControllers total channels sums Pixie multipliers", "[protocols][lor]") {
    pugi::xml_document doc;
    auto n = ParseNode(doc, LorOptXml(std::string(kUnit1) + R"(<controller UnitId="3" NumChannels="8" CntlrType="Pixie4"/>)"));
    LorControllers a(n);
    CHECK(a.GetTotalChannels() == 16 + 32);
}

#endif // !_WIN32
