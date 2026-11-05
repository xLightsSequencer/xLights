// The handlers are reached through ExposeVendors.h and a loopback server, both POSIX/Itanium-ABI only.
#ifndef _WIN32

#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "support/ExposeVendors.h"
#include "controllers/SanDevices.h"
#include "support/UnexposeVendors.h"
#include "support/FakeHttpServer.h"

namespace {
std::unique_ptr<SanDevices> MakeDisconnectedSan() {
    VendorsTestInit();
    return std::make_unique<SanDevices>("127.0.0.1:1", "");
}

std::string Input(const std::string& name, const std::string& value) {
    return "<input name='" + name + "' size='3' value='" + value + "'>";
}

std::string Select(const std::string& name, char selected) {
    return "<select name='" + name + "'><option value='A'>a<option value='" + std::string(1, selected) + "'selected>x</select>";
}

std::string Checkbox(const std::string& name, bool checked) {
    return "<input type='checkbox' name='" + name + "' value='1'" + (checked ? " checked" : "") + ">";
}
} // namespace

TEST_CASE("SanDevices2 protocol object only requests an upload when something changes", "[vendors2][sandevices2]") {
    SanDevicesProtocol p(2, 'A');
    CHECK(p.getGroup() == 2);
    CHECK(p.getProtocol() == 'A');
    CHECK(p.getTiming() == 'A');
    CHECK_FALSE(p.shouldUpload());
    p.setProtocol('A');
    p.setTiming('A');
    CHECK_FALSE(p.shouldUpload());
    p.setTiming('B');
    CHECK(p.shouldUpload());
    CHECK(p.getTiming() == 'B');
}

TEST_CASE("SanDevices2 protocol object remembers an upload after reverting", "[vendors2][sandevices2]") {
    SanDevicesProtocol p(1, 'C', 'D');
    p.setProtocol('X');
    p.setProtocol('C');
    CHECK(p.shouldUpload());
}

TEST_CASE("SanDevices2 output record defaults", "[vendors2][sandevices2]") {
    SanDevicesOutput o(2, 3, 7);
    CHECK(o.group == 2);
    CHECK(o.output == 3);
    CHECK(o.stringport == 7);
    CHECK(o.universe == 'A');
    CHECK(o.startChannel == 1);
    CHECK(o.pixels == 0);
    CHECK(o.brightness == 'A');
    CHECK_FALSE(o.upload);
    SanDevicesOutputV4 v(3);
    CHECK(v.group == 3);
    CHECK(v.protocol == 'D');
    CHECK(v.colorOrder == 'A');
    for (int i = 0; i < 4; ++i) {
        CHECK(v.nullPixel[i] == 0);
        CHECK_FALSE(v.reverse[i]);
    }
}

TEST_CASE("SanDevices2 V4 string protocol encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    auto row = GENERATE(table<std::string, char>({ { "ws2811", 'D' }, { "WS2811", 'D' }, { "tm18xx", 'D' }, { "ws2801", 'B' }, { "tls3001", 'I' }, { "lpd6803", 'A' },
                                                   { "gece", 'C' }, { "dmx", 'K' }, { "renard", 'L' } }));
    CHECK(s->EncodeStringPortProtocolV4(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("SanDevices2 V5 string protocol encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    auto row = GENERATE(table<std::string, char>({ { "ws2811", 'A' }, { "tm18xx", 'D' }, { "ws2801", 'C' }, { "lpd6803", 'B' }, { "apa102", 'G' }, { "lpd8806", 'E' },
                                                   { "gece", 'O' }, { "dmx", 'J' }, { "renard", 'M' }, { "Renard", 'M' } }));
    CHECK(s->EncodeStringPortProtocolV5(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("SanDevices2 unsupported protocols fall back to a default", "[vendors2][sandevices2]") {
    // a user can pick e.g. APA102 on a firmware 4 port; the encoder returns its default code instead of failing
    auto s = MakeDisconnectedSan();
    CHECK(s->EncodeStringPortProtocolV4("apa102") == 'D');
}

TEST_CASE("SanDevices2 serial protocol encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK(s->EncodeSerialPortProtocolV5("dmx") == 'J');
    CHECK(s->EncodeSerialPortProtocolV5("DMX") == 'J');
    CHECK(s->EncodeSerialPortProtocolV5("renard") == 'M');
}

TEST_CASE("SanDevices2 universe size encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK(s->EncodeUniverseSize(510) == 'A');
    CHECK(s->EncodeUniverseSize(512) == 'B');
}

TEST_CASE("SanDevices2 colour order encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    const char* names[] = { "rgb", "rbg", "grb", "gbr", "brg", "bgr" };
    for (int i = 0; i < 6; ++i) {
        std::string upper = names[i];
        for (auto& c : upper) c = (char)toupper(c);
        CHECK(s->EncodeColorOrderV4(names[i]) == i);
        CHECK(s->EncodeColorOrderV4(upper) == i);
        CHECK(s->EncodeColorOrderV5(names[i]) == 'A' + i);
        CHECK(s->EncodeColorOrderV5(upper) == 'A' + i);
    }
}

TEST_CASE("SanDevices2 brightness encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    auto row = GENERATE(table<int, char>({ { 0, 'D' }, { 18, 'D' }, { 19, 'C' }, { 37, 'C' }, { 38, 'B' }, { 74, 'B' }, { 75, 'A' }, { 100, 'A' }, { 10000, 'A' }, { -5, 'D' } }));
    INFO(std::get<0>(row));
    CHECK(s->EncodeBrightness(std::get<0>(row)) == std::get<1>(row));
}

TEST_CASE("SanDevices2 renard speed encoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK(s->EncoderRenardSpeed(57600) == 'A');
    CHECK(s->EncoderRenardSpeed(115200) == 'B');
    CHECK(s->EncoderRenardSpeed(230400) == 'C');
    CHECK(s->EncoderRenardSpeed(9600) == 'A');
    CHECK(s->EncoderRenardSpeed(0) == 'A');
    CHECK(s->EncoderRenardSpeed(-1) == 'A');
}

TEST_CASE("SanDevices2 direction encoding is case sensitive", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK(s->EncodeDirection("Reverse"));
    CHECK_FALSE(s->EncodeDirection("reverse"));
    CHECK_FALSE(s->EncodeDirection("Forward"));
    CHECK_FALSE(s->EncodeDirection(""));
}

TEST_CASE("SanDevices2 universe encoding uses the position in the universe list", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_universes = { 10, 11, 12, 20 };
    CHECK(s->EncodeUniverse(10) == 'A');
    CHECK(s->EncodeUniverse(12) == 'C');
    CHECK(s->EncodeUniverse(20) == 'D');
    CHECK(s->EncodeUniverse(99) == 'E'); // unknown universes land one past the end
    s->_universes.clear();
    CHECK(s->EncodeUniverse(1) == 'A');
}

TEST_CASE("SanDevices2 universe encoding with duplicate universes picks the first", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_universes = { 5, 5, 5 };
    CHECK(s->EncodeUniverse(5) == 'A');
}

TEST_CASE("SanDevices2 controller type decoding", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK(s->DecodeControllerType("E680") == SanDevices::SanDeviceModel::E680);
    CHECK(s->DecodeControllerType("E681") == SanDevices::SanDeviceModel::E681);
    CHECK(s->DecodeControllerType("E682") == SanDevices::SanDeviceModel::E682);
    CHECK(s->DecodeControllerType("E6804") == SanDevices::SanDeviceModel::E6804);
}

TEST_CASE("SanDevices2 unknown controller type decodes to Unknown", "[vendors2][sandevices2]") {
    // a model the regex matches but the table lacks (e.g. "E683")
    auto s = MakeDisconnectedSan();
    CHECK(s->DecodeControllerType("E683") == SanDevices::SanDeviceModel::Unknown);
}

TEST_CASE("SanDevices2 outputs per group depend on the model", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::Unknown;
    CHECK(s->GetOutputsPerGroup() == 1);
    CHECK(s->GetMaxStringOutputs() == 4);
    for (auto m : { SanDevices::SanDeviceModel::E680, SanDevices::SanDeviceModel::E681, SanDevices::SanDeviceModel::E682 }) {
        s->_sdmodel = m;
        CHECK(s->IsE682());
        CHECK(s->GetOutputsPerGroup() == 4);
        CHECK(s->GetMaxStringOutputs() == 16);
    }
    s->_sdmodel = SanDevices::SanDeviceModel::E6804;
    CHECK(s->IsE6804());
    CHECK_FALSE(s->IsE682());
    CHECK(s->GetOutputsPerGroup() == 1);
    CHECK(SanDevices::GetNumberOfOutputGroups() == 4);
}

TEST_CASE("SanDevices2 output numbering helpers", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::E682;
    CHECK(s->EncodeXlightsOutput(1, 1) == 1);
    CHECK(s->EncodeXlightsOutput(1, 4) == 4);
    CHECK(s->EncodeXlightsOutput(2, 1) == 5);
    CHECK(s->EncodeXlightsOutput(4, 4) == 16);
    CHECK(s->EncodeControllerPortV5(1, 1) == 1);
    CHECK(s->EncodeControllerPortV5(2, 3) == 7);
    s->_sdmodel = SanDevices::SanDeviceModel::E6804;
    CHECK(s->EncodeXlightsOutput(3, 1) == 3);
}

TEST_CASE("SanDevices2 firmware flags", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK_FALSE(s->IsFirmware4());
    CHECK_FALSE(s->IsFirmware5());
    s->_firmware = SanDevices::FirmwareVersion::Four;
    CHECK(s->IsFirmware4());
    s->_firmware = SanDevices::FirmwareVersion::Five;
    CHECK(s->IsFirmware5());
    CHECK(s->UsesHTTP());
    CHECK(s->needsHTTP_0_9());
}

TEST_CASE("SanDevices2 ExtractFromPage reads each field type", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    std::string page = Input("A", "12") + Input("B", "3.5") + "<input name='C' size='3' value='abc  '>" + Select("D", 'C') + Select("E", '7') + Checkbox("N", true) +
                       Checkbox("O", false);
    CHECK(s->ExtractFromPage(page, "A", "input") == "12");
    CHECK(s->ExtractFromPage(page, "B", "input") == "3.5");
    CHECK(s->ExtractFromPage(page, "C", "inputText") == "abc");
    CHECK(s->ExtractFromPage(page, "D", "select") == "C");
    CHECK(s->ExtractFromPage(page, "E", "select") == "7");
    CHECK(s->ExtractFromPage(page, "E", "selectLetter").empty());
    CHECK(s->ExtractFromPage(page, "N", "checkbox") == "1");
    CHECK(s->ExtractFromPage(page, "O", "checkbox") == "0");
    CHECK(s->ExtractFromPage(page, "Z", "input").empty());
    CHECK(s->ExtractFromPage(page, "A", "bogus").empty());
    CHECK(s->ExtractFromPage("", "A", "input").empty());
}

TEST_CASE("SanDevices2 ExtractFromPage respects the start offset", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    std::string page = Input("A", "1") + "SPLIT" + Input("A", "2");
    CHECK(s->ExtractFromPage(page, "A", "input", 0) == "1");
    CHECK(s->ExtractFromPage(page, "A", "input", (int)page.find("SPLIT")) == "2");
    CHECK(s->ExtractFromPage(page, "A", "input", (int)page.size()).empty());
}

TEST_CASE("SanDevices2 ExtractFromPage with a start beyond the page should not throw", "[vendors2][sandevices2]") {
    // a failed p.find(...) (npos) arrives as start == -1, and a truncated page can put start past the end;
    // both must return empty rather than let page.substr(start) throw out of the parse
    auto s = MakeDisconnectedSan();
    CHECK_NOTHROW(s->ExtractFromPage("short", "A", "input", 100));
    CHECK_NOTHROW(s->ExtractFromPage("short", "A", "input", -1));
}

TEST_CASE("SanDevices2 typed extractors fall back to defaults", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    std::string page = Input("A", "42") + Select("B", 'Q') + Checkbox("C", true) + Checkbox("D", false) + Input("E", "");
    CHECK(s->ExtractIntFromPage(page, "A", "input", 7) == 42);
    CHECK(s->ExtractIntFromPage(page, "Z", "input", 7) == 7);
    CHECK(s->ExtractIntFromPage(page, "E", "input", 9) == 9);
    CHECK(s->ExtractCharFromPage(page, "B", "select", 'x') == 'Q');
    CHECK(s->ExtractCharFromPage(page, "Z", "select", 'x') == 'x');
    CHECK(s->ExtractBoolFromPage(page, "C", "checkbox", false));
    CHECK_FALSE(s->ExtractBoolFromPage(page, "D", "checkbox", true));
    CHECK(s->ExtractBoolFromPage(page, "Z", "checkbox", true));
    CHECK_FALSE(s->ExtractBoolFromPage(page, "Z", "checkbox", false));
}

TEST_CASE("SanDevices2 ExtractIntFromPage with an absurd number does not crash", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    std::string page = Input("A", "99999999999999999999999");
    CHECK_NOTHROW(s->ExtractIntFromPage(page, "A", "input", 0));
}

TEST_CASE("SanDevices2 GenerateOutputURLV5 builds the request", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    SanDevicesOutput o(1, 1, 1);
    o.pixels = 50;
    o.universe = 'B';
    o.startChannel = 7;
    o.groupCount = 2;
    o.colorOrder = 'C';
    o.brightness = 'D';
    CHECK(s->GenerateOutputURLV5(&o) == "/K?A=50&E=C&Z=B&G=7&B=2&I=0&J=0&D=D");
    o.reverse = true;
    o.nullPixel = 3;
    o.chase = true;
    o.firstZig = 4;
    o.thenEvery = 5;
    CHECK(s->GenerateOutputURLV5(&o) == "/K?A=50&E=C&Z=B&G=7&N=1&H=3&B=2&F=1&I=4&J=5&D=D");
    o.serial = true;
    CHECK(s->GenerateOutputURLV5(&o).find("&E=") == std::string::npos);
}

TEST_CASE("SanDevices2 GenerateOutputURLV5 letter follows the group and output", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    SanDevicesOutput first(1, 1, 1);
    SanDevicesOutput last(4, 4, 16);
    SanDevicesOutput mid(2, 3, 7);
    CHECK(s->GenerateOutputURLV5(&first)[1] == 'K');
    CHECK(s->GenerateOutputURLV5(&mid)[1] == 'J' + 7);
    CHECK(s->GenerateOutputURLV5(&last)[1] == 'J' + 16);
}

TEST_CASE("SanDevices2 GenerateProtocolURLV5", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    SanDevicesProtocol p(1, 'A', 'B');
    CHECK(s->GenerateProtocolURLV5(&p) == "/K?E=A&K=B");
    SanDevicesProtocol q(4, 'G');
    CHECK(s->GenerateProtocolURLV5(&q) == "/N?E=G&K=A");
}

TEST_CASE("SanDevices2 GenerateOutputURLV4 for a single output model", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::E6804;
    SanDevicesOutputV4 o(1);
    o.protocol = 'D';
    o.pixels = 100;
    o.groupCount = 1;
    o.colorOrder = 2;
    o.universe = 'B';
    o.startChannel = 5;
    o.zigzag = 0;
    CHECK(s->GenerateOutputURLV4(&o) == "/4?B=D&C=100&D=1&E=2&F=B&G=5&L=0&M=0");
    o.reverse[0] = true;
    o.nullPixel[0] = 2;
    CHECK(s->GenerateOutputURLV4(&o) == "/4?B=D&C=100&D=1&E=2&F=B&G=5&L=0&H=1&M=1");
    o.outputSize = 3;
    CHECK(s->GenerateOutputURLV4(&o).find("?A=3&B=D") != std::string::npos);
}

TEST_CASE("SanDevices2 GenerateOutputURLV4 for a four output model", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::E682;
    SanDevicesOutputV4 o(4);
    o.outputSize = 2;
    o.reverse[1] = true;
    o.reverse[3] = true;
    o.nullPixel[2] = 1;
    auto url = s->GenerateOutputURLV4(&o);
    CHECK(url.substr(0, 9) == "/7?A=2&B=");
    CHECK(url.find("&I=1") != std::string::npos);
    CHECK(url.find("&K=1") != std::string::npos);
    CHECK(url.find("&H=1") == std::string::npos);
    CHECK(url.find("&M=0&N=0&O=1&P=0") != std::string::npos);
}

TEST_CASE("SanDevices2 ExtractOutputDataV5 reads one output block", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::E682;
    std::string page = "<table><tr><td>1-1</td>" + Input("A", "30") + Select("E", 'C') + Select("Z", 'B') + Input("G", "11") + Checkbox("N", true) + Input("H", "2") +
                       Input("B", "3") + Checkbox("F", true) + Input("I", "4") + Input("J", "5") + Select("D", 'B') + "</tr><tr><td>1-2</td>" + Input("A", "99") + "</tr></table>";
    std::unique_ptr<SanDevicesOutput> o(s->ExtractOutputDataV5(page, 1, 1));
    CHECK(o->group == 1);
    CHECK(o->output == 1);
    CHECK(o->stringport == 1);
    CHECK(o->pixels == 30);
    CHECK(o->colorOrder == 'C');
    CHECK(o->universe == 'B');
    CHECK(o->startChannel == 11);
    CHECK(o->reverse);
    CHECK(o->nullPixel == 2);
    CHECK(o->groupCount == 3);
    CHECK(o->chase);
    CHECK(o->firstZig == 4);
    CHECK(o->thenEvery == 5);
    CHECK(o->brightness == 'B');
    std::unique_ptr<SanDevicesOutput> second(s->ExtractOutputDataV5(page, 1, 2));
    CHECK(second->pixels == 99);
    CHECK(second->output == 2);
}

TEST_CASE("SanDevices2 ExtractOutputDataV5 on a page without the block should not throw", "[vendors2][sandevices2]") {
    // p.find(tofind) is npos, which reaches ExtractFromPage as start == -1
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::E682;
    std::unique_ptr<SanDevicesOutput> o;
    CHECK_NOTHROW(o.reset(s->ExtractOutputDataV5("<html></html>", 1, 1)));
}

TEST_CASE("SanDevices2 ParseV5OutputWebpage on a page that is not the output page should not throw", "[vendors2][sandevices2]") {
    // the same start == -1 path, reached through the upload flow when the controller returns the wrong page
    auto s = MakeDisconnectedSan();
    s->_sdmodel = SanDevices::SanDeviceModel::E682;
    CHECK_NOTHROW(s->ParseV5OutputWebpage("<html>wrong page</html>"));
}

TEST_CASE("SanDevices2 ParseV4Webpage on an empty page should not throw", "[vendors2][sandevices2]") {
    // " Universe" is not found, so ExtractFromPage gets start == -1
    auto s = MakeDisconnectedSan();
    CHECK_NOTHROW(s->ParseV4Webpage(""));
}

TEST_CASE("SanDevices2 ParseV5MainWebpage reads universes and protocol groups", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    std::string page = "<h1>Output Group Configuration:</h1> Universe ";
    for (char c = 'A'; c < 'A' + 12; ++c) {
        page += Input(std::string(1, c), std::to_string(100 + (c - 'A')));
    }
    for (int g = 1; g <= 4; ++g) {
        page += "<tr><td>" + std::to_string(g) + "-1 thru " + std::to_string(g) + "-4</td>" + Select("E", (char)('A' + g)) + Select("K", 'B') + "</tr>";
    }
    CHECK(s->ParseV5MainWebpage(page));
    REQUIRE(s->_universes.size() == 12);
    CHECK(s->_universes[0] == 100);
    CHECK(s->_universes[11] == 111);
    REQUIRE(s->_protocolData.size() == 4);
    CHECK(s->_protocolData[0]->getProtocol() == 'B');
    CHECK(s->_protocolData[3]->getProtocol() == 'E');
    CHECK(s->_protocolData[2]->getTiming() == 'B');
    // the parse allocates the protocol objects with new and the destructor frees them
}

TEST_CASE("SanDevices2 ParseV5MainWebpage again replaces the stored data", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    std::string page = "Output Group Configuration: Universe ";
    for (int g = 1; g <= 4; ++g) {
        page += "<td>" + std::to_string(g) + "-1 thru " + std::to_string(g) + "-4</td>" + Select("E", 'B') + Select("K", 'A');
    }
    CHECK(s->ParseV5MainWebpage(page));
    for (auto* p : s->_protocolData) delete p; // ParseV5MainWebpage clears without freeing, so release them first
    s->_protocolData.clear();
    CHECK(s->ParseV5MainWebpage(page));
    CHECK(s->_protocolData.size() == 4);
}

TEST_CASE("SanDevices2 constructor recognises firmware 4", "[vendors2][sandevices2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.contentType = "text/html";
        r.body = "<html>Controller Model E682<table><th>Firmware Version:</th></td><td></td><td>4.12</td></table></html>";
        return r;
    });
    SanDevices s(srv.Address(), "");
    CHECK(s.IsConnected());
    CHECK(s.IsFirmware4());
    CHECK(s.GetVersion() == "4.12");
    CHECK(s.GetModel() == "E682");
}

TEST_CASE("SanDevices2 constructor recognises firmware 5 including the W5200 suffix", "[vendors2][sandevices2][http]") {
    VendorsTestInit();
    auto row = GENERATE(table<std::string, std::string>({ { "5.038", "5.038" }, { " 5.051-W5200", "5.051-W5200" } }));
    FakeHttpServer srv([&](const FakeRequest&) {
        FakeResponse r;
        r.body = "Controller Model E6804 Firmware Version:</th></td><td>" + std::get<0>(row) + "</td>";
        return r;
    });
    SanDevices s(srv.Address(), "");
    CHECK(s.IsConnected());
    CHECK(s.IsFirmware5());
    CHECK(s.GetVersion() == std::get<1>(row));
    CHECK(s.GetModel() == "E6804");
}

TEST_CASE("SanDevices2 constructor retries once for an unrecognised page", "[vendors2][sandevices2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "<html>nothing useful</html>";
        return r;
    });
    SanDevices s(srv.Address(), "");
    CHECK(s.IsConnected());
    CHECK_FALSE(s.IsFirmware4());
    CHECK_FALSE(s.IsFirmware5());
    CHECK(srv.Count("/H?") == 2);
    CHECK(srv.Count("/") == 2);
}

TEST_CASE("SanDevices2 constructor with an unreachable controller", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK_FALSE(s->IsConnected());
    CHECK(s->GetModel().empty());
}

TEST_CASE("SanDevices2 constructor with an unsupported model name still connects", "[vendors2][sandevices2][http]") {
    // the model regex accepts any E\d+; models outside the table decode to Unknown
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) {
        FakeResponse r;
        r.body = "Controller Model E999 Firmware Version:</th></td><td></td><td>4.1</td>";
        return r;
    });
    SanDevices s(srv.Address(), "");
    CHECK(s.IsConnected());
}

TEST_CASE("SanDevices2 SetOutputs with unknown firmware fails", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    REQUIRE_FALSE(s->IsFirmware4());
    REQUIRE_FALSE(s->IsFirmware5());
    CHECK_FALSE(s->SetOutputs(nullptr, nullptr, nullptr, nullptr));
}

TEST_CASE("SanDevices2 SDGetURL returns the body or empty", "[vendors2][sandevices2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest& r) {
        FakeResponse resp;
        if (r.path == "/x") resp.body = "hello";
        return resp;
    });
    auto s = MakeDisconnectedSan();
    s->_ip = srv.Address();
    CHECK(s->SDGetURL("/x") == "hello");
    CHECK(s->SDGetURL("/y").empty());
}

TEST_CASE("SanDevices2 ResetStringOutputs hits the four group urls", "[vendors2][sandevices2][http]") {
    VendorsTestInit();
    FakeHttpServer srv([](const FakeRequest&) { return FakeResponse(); });
    auto s = MakeDisconnectedSan();
    s->_ip = srv.Address();
    s->ResetStringOutputs();
    CHECK(srv.Count("/4?A=0") == 1);
    CHECK(srv.Count("/5?A=0") == 1);
    CHECK(srv.Count("/6?A=0") == 1);
    CHECK(srv.Count("/7?A=0") == 1);
}

TEST_CASE("SanDevices2 SetInputUniverses without a stored page fails", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    CHECK_FALSE(s->SetInputUniverses(nullptr, nullptr));
}

TEST_CASE("SanDevices2 SetInputUniverses on firmware 5 needs the system page", "[vendors2][sandevices2]") {
    auto s = MakeDisconnectedSan();
    s->_firmware = SanDevices::FirmwareVersion::Five;
    s->_page = "<html>other page</html>";
    CHECK_FALSE(s->SetInputUniverses(nullptr, nullptr));
}

#endif
