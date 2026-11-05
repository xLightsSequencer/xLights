// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/AlphaPix.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

// These three classes are only defined inside AlphaPix.cpp. The data members are repeated here, in the same
// order, so the tests can read and build them; the inline Dump helpers are deliberately left out.
class AlphaPixOutput {
public:
    const int output;
    int universe{ 1 };
    int startChannel{ 1 };
    int pixels{ 1 };
    int nullPixel{ 0 };
    int colorOrder{ 0 };
    bool reverse{ false };
    int brightness{ 100 };
    int zigZag{ 0 };
    bool upload{ false };
    explicit AlphaPixOutput(int output_) : output(output_) {}
};

class AlphaPixSerial {
public:
    const int output;
    int universe{ 1 };
    bool enabled{ false };
    bool upload{ false };
    explicit AlphaPixSerial(int output_) : output(output_) {}
};

class AlphaPixData {
public:
    std::string name;
    int protocol{ 0 };
    int inputMode{ 0 };
    AlphaPixData() = default;
};

namespace {
std::unique_ptr<AlphaPix> MakeDisconnectedAlpha() {
    VendorsTestInit();
    return std::make_unique<AlphaPix>("127.0.0.1:1", "");
}

std::string TextInput(const std::string& name, const std::string& value) {
    return "<input style=\"width: 80px\" type=\"text\" value=\"" + value + "\" name=\"" + name + "\"/>\n";
}

std::string Checkbox(const std::string& name, bool checked) {
    return "<input type=\"checkbox\" name=\"" + name + "\" " + (checked ? "checked=\"checked\" " : "") + "value=\"1\"/>\n";
}
} // namespace

TEST_CASE("AlphaPix2 ExtractFromPage reads text inputs", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = TextInput("SU1", "12") + TextInput("SC1", "345") + TextInput("empty", "");
    CHECK(a->ExtractFromPage(page, "SU1", "input") == "12");
    CHECK(a->ExtractFromPage(page, "SC1", "input") == "345");
    CHECK(a->ExtractFromPage(page, "empty", "input").empty());
    CHECK(a->ExtractFromPage(page, "nope", "input").empty());
    CHECK(a->ExtractFromPage("", "SU1", "input").empty());
}

TEST_CASE("AlphaPix2 ExtractFromPage on a single line with two inputs picks the right one", "[vendors2][alphapix2]") {
    // AlphaPix.cpp ExtractFromPage: the regex uses greedy .* for style and value, so on a page without line breaks between inputs the
    // match starts at the first input and the capture spans several inputs
    auto a = MakeDisconnectedAlpha();
    std::string page = "<input style=\"w\" type=\"text\" value=\"1\" name=\"SU1\"/><input style=\"w\" type=\"text\" value=\"5\" name=\"SU2\"/>";
    CHECK(a->ExtractFromPage(page, "SU2", "input") == "5");
}

TEST_CASE("AlphaPix2 ExtractFromPage reads selects radios and checkboxes", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = "<select name=\"IC\"><option value=\"0\">a</option><option value=\"3\" selected=\"selected\">b</option></select>\n"
                       "<select name=\"RGBS\"><option value=\"5\" selected=\"selected\">c</option></select>\n"
                       "<input type=\"radio\" id=\"r1\" name=\"EP\" value=\"2\" checked=\"checked\"/>\n" +
                       Checkbox("Rever1", true) + Checkbox("Rever2", false);
    CHECK(a->ExtractFromPage(page, "IC", "select") == "3");
    CHECK(a->ExtractFromPage(page, "RGBS", "select") == "5");
    CHECK(a->ExtractFromPage(page, "EP", "radio") == "2");
    CHECK(a->ExtractFromPage(page, "Rever1", "checkbox") == "1");
    CHECK(a->ExtractFromPage(page, "Rever2", "checkbox") == "0");
    CHECK(a->ExtractFromPage(page, "Rever3", "checkbox").empty());
    CHECK(a->ExtractFromPage(page, "EP2", "radio").empty());
}

TEST_CASE("AlphaPix2 ExtractFromPage select falls back to the whole page when the name is missing", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = "<option value=\"4\" selected=\"selected\">x</option>";
    CHECK(a->ExtractFromPage(page, "ZZ", "select") == "4");
}

TEST_CASE("AlphaPix2 ExtractFromPage guards the start offset", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = TextInput("SU1", "7");
    CHECK(a->ExtractFromPage(page, "SU1", "input", -1).empty());
    CHECK(a->ExtractFromPage(page, "SU1", "input", (int)page.size() + 1).empty());
    CHECK(a->ExtractFromPage(page, "SU1", "input", (int)page.size()).empty());
    CHECK(a->ExtractFromPage(page, "SU1", "input", 0) == "7");
    CHECK(a->ExtractFromPage(page, "SU1", "input", 1).empty());
}

TEST_CASE("AlphaPix2 ExtractFromPage with an unknown type asserts", "[vendors2][alphapix2]") {
    // AlphaPix.cpp ExtractFromPage: an unknown type hits assert(false) after logging
    auto a = MakeDisconnectedAlpha();
    CHECK(a->ExtractFromPage("x", "A", "bogus").empty());
}

TEST_CASE("AlphaPix2 ExtractIntFromPage and ExtractBoolFromPage defaults", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = TextInput("SU1", "42") + TextInput("SU2", "abc") + Checkbox("C1", true) + Checkbox("C2", false);
    CHECK(a->ExtractIntFromPage(page, "SU1", "input", 9) == 42);
    CHECK(a->ExtractIntFromPage(page, "SU2", "input", 9) == 0);
    CHECK(a->ExtractIntFromPage(page, "SU3", "input", 9) == 9);
    CHECK(a->ExtractBoolFromPage(page, "C1", "checkbox", false));
    CHECK_FALSE(a->ExtractBoolFromPage(page, "C2", "checkbox", true));
    CHECK(a->ExtractBoolFromPage(page, "C3", "checkbox", true));
    CHECK_FALSE(a->ExtractBoolFromPage(page, "C3", "checkbox", false));
}

TEST_CASE("AlphaPix2 ExtractIntFromPage with an overflowing number does not crash", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    CHECK_NOTHROW(a->ExtractIntFromPage(TextInput("SU1", "99999999999999999999"), "SU1", "input", 0));
}

TEST_CASE("AlphaPix2 ExtractOutputData V1 reads the numbered fields", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = TextInput("SU2", "7") + TextInput("SC2", "11") + TextInput("PC2", "50") + TextInput("NP2", "3") + TextInput("RA2", "4") + TextInput("LM2", "80") + Checkbox("RV2", true);
    std::unique_ptr<AlphaPixOutput> o(a->ExtractOutputData(page, 2));
    CHECK(o->output == 2);
    CHECK(o->universe == 7);
    CHECK(o->startChannel == 11);
    CHECK(o->pixels == 50);
    CHECK(o->nullPixel == 3);
    CHECK(o->zigZag == 4);
    CHECK(o->brightness == 80);
    CHECK(o->reverse);
}

TEST_CASE("AlphaPix2 ExtractOutputData V1 defaults when the fields are absent", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::unique_ptr<AlphaPixOutput> o(a->ExtractOutputData("", 5));
    CHECK(o->universe == 1);
    CHECK(o->startChannel == 1);
    CHECK(o->pixels == 1);
    CHECK(o->nullPixel == 0);
    CHECK(o->brightness == 100);
    CHECK_FALSE(o->reverse);
}

TEST_CASE("AlphaPix2 ExtractOutputData V2 uses two digit names", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = TextInput("U03", "9") + TextInput("C03", "2") + TextInput("P03", "100") + TextInput("N03", "1") + TextInput("R03", "6") + TextInput("L03", "55") + Checkbox("V03", true);
    std::unique_ptr<AlphaPixOutput> o(a->ExtractOutputDataV2(page, 3));
    CHECK(o->universe == 9);
    CHECK(o->startChannel == 2);
    CHECK(o->pixels == 100);
    CHECK(o->nullPixel == 1);
    CHECK(o->zigZag == 6);
    CHECK(o->brightness == 55);
    CHECK(o->reverse);
    std::unique_ptr<AlphaPixOutput> other(a->ExtractOutputDataV2(page, 13));
    CHECK(other->universe == 1);
}

TEST_CASE("AlphaPix2 serial data depends on the model", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = "Enabled:\n" + Checkbox("Rever5", true) + Checkbox("Rever2", true) + "DMX Universe:\n" + TextInput("DMX512", "21") + TextInput("DMX512_2", "22");
    a->_modelnum = 4;
    std::unique_ptr<AlphaPixSerial> s4(a->ExtractSerialData(page, 1));
    CHECK(s4->enabled);
    CHECK(s4->universe == 21);
    a->_modelnum = 16;
    std::unique_ptr<AlphaPixSerial> s16(a->ExtractSerialData(page, 2));
    CHECK(s16->enabled);
    CHECK(s16->universe == 22);
    CHECK(s16->output == 2);
}

TEST_CASE("AlphaPix2 serial data on a page without the section headings", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    a->_modelnum = 4;
    std::unique_ptr<AlphaPixSerial> s(a->ExtractSerialData("", 1));
    CHECK_FALSE(s->enabled);
    CHECK(s->universe == 0);
}

TEST_CASE("AlphaPix2 number of serial ports per model", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    auto row = GENERATE(table<int, int>({ { 4, 1 }, { 16, 3 }, { 48, 2 }, { 8, 0 }, { 0, 0 }, { -1, 0 } }));
    a->_modelnum = std::get<0>(row);
    CHECK(a->GetNumberOfSerial() == std::get<1>(row));
    CHECK(a->GetNumberOfOutputs() == std::get<0>(row));
}

TEST_CASE("AlphaPix2 header page parsers locate their headings", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = "Device name:\n" + TextInput("name", "Roof") + "Ethernet Protocol:\n<input type=\"radio\" id=\"x\" name=\"EP\" value=\"1\" checked=\"checked\"/>\n"
                       "Pixel IC Protocol:\n<select name=\"IC\"><option value=\"2\" selected=\"selected\">b</option></select>\n"
                       "Pixel Chip Color Output Order:\n<input type=\"radio\" id=\"y\" name=\"RGBORD\" value=\"0\" checked=\"checked\"/>\n<select name=\"RGBS\"><option value=\"4\" selected=\"selected\">c</option></select>\n";
    CHECK(a->ExtractName(page) == "Roof");
    CHECK(a->ExtractInputType(page) == 1);
    CHECK(a->ExtractProtocol(page) == 2);
    CHECK(a->ExtractColorType(page) == 0);
    CHECK(a->ExtractColor(page) == 4);
}

TEST_CASE("AlphaPix2 header page parsers survive a page without headings", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    CHECK(a->ExtractName("").empty());
    CHECK(a->ExtractInputType("") == 0);
    CHECK(a->ExtractProtocol("") == 0);
    CHECK(a->ExtractColorType("") == 0);
    CHECK(a->ExtractColor("") == 0);
    CHECK(a->ExtractSingleColor("", 3) == 0);
    CHECK_FALSE(a->ExtractDMXEnabled("", "Rever1"));
    CHECK(a->ExtractDMXUniverse("", "DMX512") == 0);
}

TEST_CASE("AlphaPix2 ExtractSingleColor reads the select for one output", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    std::string page = "Output 1:<select name=\"1_RGB\"><option value=\"2\" selected=\"selected\">x</option></select>\n"
                       "Output 2:<select name=\"2_RGB\"><option value=\"5\" selected=\"selected\">x</option></select>\n";
    CHECK(a->ExtractSingleColor(page, 1) == 2);
    CHECK(a->ExtractSingleColor(page, 2) == 5);
    CHECK(a->ExtractSingleColor(page, 3) == 0);
}

TEST_CASE("AlphaPix2 string protocol encoding depends on the model", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    a->_modelnum = 4;
    CHECK(a->EncodeStringPortProtocol("ws2811") == 0);
    CHECK(a->EncodeStringPortProtocol("WS2801") == 1);
    CHECK(a->EncodeStringPortProtocol("lpd6803") == 2);
    CHECK(a->EncodeStringPortProtocol("tls3001") == 4);
    CHECK(a->EncodeStringPortProtocol("tm18xx") == 6);
    a->_modelnum = 48;
    CHECK(a->EncodeStringPortProtocol("tm18xx") == 4);
    CHECK(a->EncodeStringPortProtocol("ws2811") == 0);
}

TEST_CASE("AlphaPix2 tls3001 on the flex asserts", "[vendors2][alphapix2]") {
    // AlphaPix.cpp EncodeStringPortProtocol: tls3001 on a 48 port (Flex) hits assert(false)
    auto a = MakeDisconnectedAlpha();
    a->_modelnum = 48;
    CHECK(a->EncodeStringPortProtocol("tls3001") == 0);
}

TEST_CASE("AlphaPix2 colour order and direction encoding", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    const char* names[] = { "rgb", "rbg", "grb", "gbr", "brg", "bgr" };
    for (int i = 0; i < 6; ++i) {
        CHECK(a->EncodeColorOrder(names[i]) == i);
    }
    CHECK(a->EncodeColorOrder("RGB") == 0);
    CHECK(a->EncodeDirection("Reverse"));
    CHECK_FALSE(a->EncodeDirection("reverse"));
    CHECK_FALSE(a->EncodeDirection("Forward"));
}

TEST_CASE("AlphaPix2 unknown colour order asserts", "[vendors2][alphapix2]") {
    // AlphaPix.cpp EncodeColorOrder: assert(false) for an order such as rgbw
    auto a = MakeDisconnectedAlpha();
    CHECK(a->EncodeColorOrder("rgbw") == 0);
}

TEST_CASE("AlphaPix2 SafeDescription truncates to sixteen characters", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    CHECK(a->SafeDescription("") == "");
    CHECK(a->SafeDescription("short") == "short");
    CHECK(a->SafeDescription(std::string(16, 'x')) == std::string(16, 'x'));
    CHECK(a->SafeDescription(std::string(17, 'x')) == std::string(16, 'x'));
    CHECK(a->SafeDescription(std::string(10000, 'x')).size() == 16);
}

TEST_CASE("AlphaPix2 BuildStringPortRequest formats the form post", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    AlphaPixOutput o(3);
    o.universe = 5;
    o.startChannel = 7;
    o.pixels = 20;
    o.nullPixel = 1;
    o.zigZag = 2;
    o.brightness = 90;
    CHECK(a->BuildStringPortRequest(&o) == "SU3=5&SC3=7&PC3=20&NP3=1&RA3=2&LM3=90");
    o.reverse = true;
    CHECK(a->BuildStringPortRequest(&o) == "SU3=5&SC3=7&PC3=20&NP3=1&RA3=2&LM3=90&RV3=1");
}

TEST_CASE("AlphaPix2 BuildStringPortRequestV2 pads output numbers", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    AlphaPixOutput o(3);
    CHECK(a->BuildStringPortRequestV2(&o) == "U03=1&C03=1&P03=1&N03=0&R03=0&L03=100");
    o.reverse = true;
    AlphaPixOutput big(12);
    big.reverse = true;
    CHECK(a->BuildStringPortRequestV2(&o).substr(a->BuildStringPortRequestV2(&o).size() - 6) == "&V03=1");
    CHECK(a->BuildStringPortRequestV2(&big).find("U12=1") == 0);
}

TEST_CASE("AlphaPix2 FindPortData and FindSerialData look up by output number", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    a->_pixelOutputs.push_back(new AlphaPixOutput(1));
    a->_pixelOutputs.push_back(new AlphaPixOutput(2));
    a->_serialOutputs.push_back(new AlphaPixSerial(1));
    CHECK(a->FindPortData(2) == a->_pixelOutputs[1]);
    CHECK(a->FindSerialData(1) == a->_serialOutputs[0]);
}

TEST_CASE("AlphaPix2 FindPortData of a missing port asserts", "[vendors2][alphapix2]") {
    // AlphaPix.cpp FindPortData / FindSerialData: assert(false) before returning nullptr
    auto a = MakeDisconnectedAlpha();
    CHECK(a->FindPortData(9) == nullptr);
}

TEST_CASE("AlphaPix2 static urls", "[vendors2][alphapix2]") {
    CHECK(AlphaPix::GetNameURL() == "/ABOUT");
    CHECK(AlphaPix::GetInputTypeURL() == "/InputSignal");
    CHECK(AlphaPix::GetOutputURL() == "/SetSPI");
    CHECK(AlphaPix::GetOutputURL(3) == "/SetSPI_3");
    CHECK(AlphaPix::GetColorOrderURL() == "/PixelColor");
    CHECK(AlphaPix::GetDMXURL() == "/DMX512");
    CHECK(AlphaPix::GetDMXURL(2) == "/DMX512_2");
    CHECK(AlphaPix::GetProtocolURL() == "/PixelIC");
    CHECK(AlphaPix::GetIndvColorOrderURL() == "/RGBOrder");
}

TEST_CASE("AlphaPix2 writeFunction appends to the string", "[vendors2][alphapix2]") {
    std::string s = "ab";
    CHECK(AlphaPix::writeFunction((void*)"cdef", 2, 2, &s) == 4);
    CHECK(s == "abcdef");
    CHECK(AlphaPix::writeFunction((void*)"x", 1, 0, &s) == 0);
    CHECK(s == "abcdef");
    CHECK(AlphaPix::writeFunction((void*)"x", 1, 1, nullptr) == 0);
}

TEST_CASE("AlphaPix2 constructor identifies models and firmware", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, std::vector<std::string>>(
        { { "<h1>16 Port Ethernet to SPI Controller</h1> Currently Installed Firmware Version:  2.08", { "16", "AlphaPix 16 v1", "2.08" } },
          { "<title>AlphaPix 4 V2</title> Currently Installed Firmware Version:  2.18 name=\"U01\"", { "4", "AlphaPix 4 v2", "2.18" } },
          { "AlphaPix Flex Lighting Controller", { "48", "AlphaPix Flex v1", "" } },
          { "AlphaPix Evolution Lighting Controller name=\"U01\"", { "48", "AlphaPix Flex v2", "" } } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.contentType = "text/html";
        r.body = std::get<0>(row);
        return r;
    });
    AlphaPix a(srv.Address(), "");
    CHECK(a.IsConnected());
    CHECK(a._modelnum == std::stoi(std::get<1>(row)[0]));
    CHECK(a.GetModel() == std::get<1>(row)[1]);
    CHECK(a.GetVersion() == std::get<1>(row)[2]);
}

TEST_CASE("AlphaPix2 constructor with an unrecognised page is not connected", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<html>router login</html>";
        return r;
    });
    AlphaPix a(srv.Address(), "");
    CHECK_FALSE(a.IsConnected());
}

TEST_CASE("AlphaPix2 constructor with a locked out page logs but keeps going", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "Existing user login 4 Port Ethernet to SPI Controller";
        return r;
    });
    AlphaPix a(srv.Address(), "");
    CHECK(a.IsConnected());
    CHECK(a._modelnum == 4);
}

TEST_CASE("AlphaPix2 constructor with an empty page is not connected", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    AlphaPix a(srv.Address(), "");
    CHECK_FALSE(a.IsConnected());
}

TEST_CASE("AlphaPix2 constructor with an unreachable controller is not connected", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    CHECK_FALSE(a->IsConnected());
    CHECK(a->UsesHTTP());
    CHECK(a->needsHTTP_0_9());
    CHECK(a->_revision == 1);
}

TEST_CASE("AlphaPix2 constructor with a model number too large for int", "[vendors2][alphapix2][http]") {
    // AlphaPix.cpp constructor: (int)strtol saturates and truncates to -1, the "unknown model" sentinel, yet _connected is set true
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "99999999999999999999 Port Ethernet to SPI Controller";
        return r;
    });
    AlphaPix a(srv.Address(), "");
    CHECK(a.IsConnected());
    CHECK(a._modelnum > 0); // strtol saturates and the int cast makes it -1
}

TEST_CASE("AlphaPix2 ParseWebpage for a single colour order controller", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto a = MakeDisconnectedAlpha();
    a->_ip = srv.Address();
    a->_modelnum = 4;
    a->_revision = 1;
    std::string page = "Device name:\n" + TextInput("name", "Test") + "Pixel IC Protocol:\n<select name=\"IC\"><option value=\"1\" selected=\"selected\">b</option></select>\n"
                       "Pixel Chip Color Output Order:\n<input type=\"radio\" id=\"y\" name=\"RGBORD\" value=\"0\" checked=\"checked\"/>\n<select name=\"RGBS\"><option value=\"3\" selected=\"selected\">c</option></select>\n";
    for (int i = 1; i <= 4; ++i) page += TextInput("PC" + std::to_string(i), std::to_string(i * 10));
    AlphaPixData data;
    CHECK(a->ParseWebpage(page, data));
    CHECK(data.name == "Test");
    CHECK(data.protocol == 1);
    REQUIRE(a->_pixelOutputs.size() == 4);
    CHECK(a->_pixelOutputs[2]->pixels == 30);
    for (auto* o : a->_pixelOutputs) CHECK(o->colorOrder == 3);
    REQUIRE(a->_serialOutputs.size() == 1);
    CHECK(srv.Requests().empty());
}

TEST_CASE("AlphaPix2 ParseWebpage for an advanced colour order posts for the colour page", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/PixelColor") {
            resp.body = "Output 1:<select name=\"1_RGB\"><option value=\"4\" selected=\"selected\">x</option></select>\n"
                        "Output 2:<select name=\"2_RGB\"><option value=\"1\" selected=\"selected\">x</option></select>\n";
        }
        return resp;
    });
    auto a = MakeDisconnectedAlpha();
    a->_ip = srv.Address();
    a->_modelnum = 4;
    std::string page = "Pixel Chip Color Output Order:\n<input type=\"radio\" id=\"y\" name=\"RGBORD\" value=\"1\" checked=\"checked\"/>\n";
    AlphaPixData data;
    CHECK(a->ParseWebpage(page, data));
    REQUIRE(a->_pixelOutputs.size() == 4);
    CHECK(a->_pixelOutputs[0]->colorOrder == 4);
    CHECK(a->_pixelOutputs[1]->colorOrder == 1);
    CHECK(a->_pixelOutputs[2]->colorOrder == 0);
    REQUIRE(srv.Requests().size() == 1);
    CHECK(srv.Requests()[0].body == "RGBORD=1");
}

TEST_CASE("AlphaPix2 ParseWebpage with an unknown model creates no outputs", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    a->_modelnum = -1;
    AlphaPixData data;
    CHECK(a->ParseWebpage("", data));
    CHECK(a->_pixelOutputs.empty());
    CHECK(a->_serialOutputs.empty());
}

TEST_CASE("AlphaPix2 APGetURL and APPutURL talk to the controller", "[vendors2][alphapix2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        resp.body = r.method == "POST" ? "posted:" + r.body : "got:" + r.path;
        return resp;
    });
    auto a = MakeDisconnectedAlpha();
    a->_ip = srv.Address();
    CHECK(a->APGetURL("/ABOUT") == "got:/ABOUT");
    CHECK(a->APPutURL("/SetSPI", "SU1=2") == "posted:SU1=2");
    CHECK(a->APGetURL("/").find("got") == 0);
}

TEST_CASE("AlphaPix2 APPutURL and APGetURL fail quietly on a closed port", "[vendors2][alphapix2]") {
    auto a = MakeDisconnectedAlpha();
    CHECK(a->APGetURL("/").empty());
    CHECK(a->APPutURL("/", "x").empty());
}

#endif
