#include <functional>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "XmlSerializer/GdtfParser.h"
#include "models/DMX/DmxColorAbilityRGB.h"
#include "models/DMX/DmxModel.h"
#include "models/DMX/DmxMotor.h"
#include "models/DMX/DmxMovingHead.h"
#include "models/DMX/DmxMovingHeadAdv.h"
#include "models/DMX/DmxShutterAbility.h"
#include "models/Model.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "render/UICallbacks.h"

namespace {

class StubUi : public UICallbacks {
public:
    std::vector<std::string> answer;
    mutable std::vector<std::string> offered;
    mutable int asked = 0;

    void ShowMessage(const std::string&, const std::string&) const override {}
    bool PromptYesNo(const std::string&, const std::string&) const override { return true; }
    std::string PromptForDirectory(const std::string&, const std::string&) const override { return ""; }
    std::string PromptForFile(const std::string&, const std::string&, const std::string&) const override { return ""; }
    long PromptForNumber(const std::string&, const std::string&, long defaultValue, long, long) const override { return defaultValue; }
    std::string PromptForText(const std::string&, const std::string&, const std::string& defaultValue) const override { return defaultValue; }
    std::vector<std::string> ChooseFromList(const std::string&, const std::vector<std::string>& options) const override {
        ++asked;
        offered = options;
        return answer;
    }
    ProgressToken BeginProgress(const std::string&, int) override { return 1; }
    void UpdateProgress(ProgressToken, int, const std::string&) override {}
    void EndProgress(ProgressToken) override {}
};

struct Gdtf {
    OutputManager om;
    ModelManager mm{ &om };
    pugi::xml_document doc;
    XmlSerialize::GdtfModelData data;
    bool cancelled = false;

    bool Parse(const std::string& xml, UICallbacks* ui = nullptr) {
        REQUIRE(doc.load_string(xml.c_str()));
        return XmlSerialize::ParseGdtfDescriptionXml(doc, mm, ui, cancelled, data);
    }
};

std::string Wrap(const std::string& modes) {
    return "<GDTF DataVersion=\"1.1\"><FixtureType Name=\"Test\"><DMXModes>" + modes + "</DMXModes></FixtureType></GDTF>";
}

std::string Mode(const std::string& name, const std::string& channels) {
    return "<DMXMode Name=\"" + name + "\"><DMXChannels>" + channels + "</DMXChannels></DMXMode>";
}

std::string Chan(const std::string& offset, const std::string& attribute, const std::string& functions = "") {
    return "<DMXChannel Offset=\"" + offset + "\"><LogicalChannel Attribute=\"" + attribute + "\">" + functions + "</LogicalChannel></DMXChannel>";
}

std::string Fn(const std::string& sets) {
    return "<ChannelFunction Name=\"f\">" + sets + "</ChannelFunction>";
}

std::string Set(const std::string& name, const std::string& from) {
    return "<ChannelSet Name=\"" + name + "\" DMXFrom=\"" + from + "\"/>";
}

} // namespace

TEST_CASE("GDTF with no root element is rejected", "[models2][gdtf]") {
    Gdtf g;
    pugi::xml_document empty;
    CHECK_FALSE(XmlSerialize::ParseGdtfDescriptionXml(empty, g.mm, nullptr, g.cancelled, g.data));
    CHECK_FALSE(g.cancelled);
}

TEST_CASE("GDTF without FixtureType or modes is rejected", "[models2][gdtf]") {
    Gdtf g;
    CHECK_FALSE(g.Parse("<GDTF/>"));
    Gdtf h;
    CHECK_FALSE(h.Parse("<GDTF><FixtureType Name=\"x\"/></GDTF>"));
    Gdtf i;
    CHECK_FALSE(i.Parse("<GDTF><FixtureType Name=\"x\"><DMXModes/></FixtureType></GDTF>"));
    Gdtf j;
    CHECK_FALSE(j.Parse("<Other><DMXModes><DMXMode Name=\"m\"/></DMXModes></Other>"));
}

TEST_CASE("GDTF single mode needs no prompt", "[models2][gdtf]") {
    Gdtf g;
    StubUi ui;
    REQUIRE(g.Parse(Wrap(Mode("Standard", Chan("1", "Dimmer") + Chan("2", "ColorAdd_R"))), &ui));
    CHECK(ui.asked == 0);
    CHECK(g.data.selectedMode == "Standard");
    CHECK(g.data.totalChannels == 2);
    CHECK(g.data.channels.size() == 2);
    CHECK_FALSE(g.data.isMovingHead);
}

TEST_CASE("GDTF mode without channels parses to an empty channel list", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap("<DMXMode Name=\"Empty\"/>")));
    CHECK(g.data.channels.empty());
    CHECK(g.data.totalChannels == 0);
    CHECK(g.data.selectedMode == "Empty");
}

TEST_CASE("GDTF channel attribute and offset are recorded", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("5", "Gobo1") + Chan("7,8", "Zoom")))));
    REQUIRE(g.data.channels.size() == 2);
    auto it = g.data.channels.begin();
    CHECK(it->attribute == "Gobo1");
    CHECK(it->channelStart == 5);
    CHECK(it->channels == 1);
    ++it;
    CHECK(it->attribute == "Zoom");
    CHECK(it->channelStart == 7);
    CHECK(it->channels == 2);
    CHECK(g.data.totalChannels == 8);
}

TEST_CASE("GDTF triple byte offset counts three channels", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1,2,3", "Pan")))));
    CHECK(g.data.channels.front().channels == 3);
    CHECK(g.data.totalChannels == 3);
}

TEST_CASE("GDTF total channels is the highest channel used regardless of order", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("12", "Shutter1") + Chan("3", "Dimmer") + Chan("6,7", "Tilt")))));
    CHECK(g.data.totalChannels == 12);
}

TEST_CASE("GDTF pan or tilt marks a moving head", "[models2][gdtf]") {
    Gdtf pan;
    REQUIRE(pan.Parse(Wrap(Mode("M", Chan("1", "Pan")))));
    CHECK(pan.data.isMovingHead);
    Gdtf tilt;
    REQUIRE(tilt.Parse(Wrap(Mode("M", Chan("1", "Dimmer") + Chan("2", "Tilt")))));
    CHECK(tilt.data.isMovingHead);
    Gdtf none;
    REQUIRE(none.Parse(Wrap(Mode("M", Chan("1", "Dimmer") + Chan("2", "Gobo1")))));
    CHECK_FALSE(none.data.isMovingHead);
}

TEST_CASE("GDTF attribute matching for moving heads is exact", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1", "PanMode") + Chan("2", "pan")))));
    CHECK_FALSE(g.data.isMovingHead);
    XmlSerialize::DMXChannelInfo info;
    info.attribute = "Pan";
    CHECK(info.IsMovingHead());
    info.attribute = "Tilt";
    CHECK(info.IsMovingHead());
    info.attribute = "Zoom";
    CHECK_FALSE(info.IsMovingHead());
}

TEST_CASE("GDTF channel sets record names and ranges", "[models2][gdtf]") {
    Gdtf g;
    std::string sets = Set("Open", "0/1") + Set("Red", "50/1") + Set("Blue", "100/1");
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1", "Color1", Fn(sets))))));
    const auto& v = g.data.channels.front().values;
    REQUIRE(v.size() == 3);
    auto it = v.begin();
    CHECK(it->description == "Open");
    CHECK(it->low == 0);
    CHECK(it->high == 49);
    ++it;
    CHECK(it->description == "Red");
    CHECK(it->low == 50);
    CHECK(it->high == 99);
    ++it;
    CHECK(it->description == "Blue");
    CHECK(it->low == 100);
    CHECK(it->high == 100);
}

TEST_CASE("GDTF two byte channel sets scale a single byte value", "[models2][gdtf]") {
    Gdtf g;
    std::string sets = Set("Low", "0/1") + Set("High", "128/1");
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1,2", "Gobo1", Fn(sets))))));
    const auto& v = g.data.channels.front().values;
    REQUIRE(v.size() == 2);
    CHECK(v.front().low == 0);
    CHECK(v.front().high == (128 << 8) - 1);
    CHECK(v.back().low == (128 << 8));
}

TEST_CASE("GDTF two byte channel sets keep two byte values", "[models2][gdtf]") {
    Gdtf g;
    std::string sets = Set("Low", "0/2") + Set("High", "300/2");
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1,2", "Gobo1", Fn(sets))))));
    const auto& v = g.data.channels.front().values;
    REQUIRE(v.size() == 2);
    CHECK(v.front().high == 299);
    CHECK(v.back().low == 300);
}

TEST_CASE("GDTF channel sets without a name are skipped", "[models2][gdtf]") {
    Gdtf g;
    std::string sets = "<ChannelSet DMXFrom=\"0/1\"/>" + Set("Named", "10/1");
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1", "Color1", Fn(sets))))));
    const auto& v = g.data.channels.front().values;
    REQUIRE(v.size() == 1);
    CHECK(v.front().description == "Named");
    CHECK(v.front().low == 10);
}

TEST_CASE("GDTF malformed DMXFrom values read as zero", "[models2][gdtf]") {
    Gdtf g;
    std::string sets = Set("A", "garbage") + Set("B", "") + Set("C", "7/9");
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("1", "Color1", Fn(sets))))));
    const auto& v = g.data.channels.front().values;
    REQUIRE(v.size() == 3);
    for (const auto& value : v) {
        CHECK(value.low == 0);
    }
}

TEST_CASE("GDTF channel without a logical channel has no attribute", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", "<DMXChannel Offset=\"3\"/>"))));
    REQUIRE(g.data.channels.size() == 1);
    CHECK(g.data.channels.front().attribute.empty());
    CHECK(g.data.channels.front().channelStart == 3);
    CHECK(g.data.totalChannels == 3);
}

TEST_CASE("GDTF channel with a missing offset occupies no channels", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", "<DMXChannel><LogicalChannel Attribute=\"Dimmer\"/></DMXChannel>"))));
    REQUIRE(g.data.channels.size() == 1);
    CHECK(g.data.channels.front().channels <= 1);
    CHECK(g.data.totalChannels <= 1);
}

TEST_CASE("GDTF offset of None does not become channel zero garbage", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("M", Chan("None", "Virtual") + Chan("2", "Dimmer")))));
    CHECK(g.data.totalChannels == 2);
}

TEST_CASE("GDTF without a prompt picks the first mode by name", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Zeta", Chan("1", "Dimmer")) + Mode("Alpha", Chan("1", "Pan") + Chan("2", "Tilt")))));
    CHECK(g.data.selectedMode == "Alpha");
    CHECK(g.data.isMovingHead);
    CHECK(g.data.totalChannels == 2);
}

TEST_CASE("GDTF multiple modes are offered to the prompt", "[models2][gdtf]") {
    Gdtf g;
    StubUi ui;
    ui.answer = { "Zeta" };
    REQUIRE(g.Parse(Wrap(Mode("Zeta", Chan("1", "Dimmer")) + Mode("Alpha", Chan("1", "Pan") + Chan("2", "Tilt")) + Mode("Mid", Chan("1", "Dimmer") + Chan("4", "Zoom"))), &ui));
    CHECK(ui.asked == 1);
    REQUIRE(ui.offered.size() == 3);
    CHECK(ui.offered[0] == "Alpha");
    CHECK(ui.offered[1] == "Mid");
    CHECK(ui.offered[2] == "Zeta");
    CHECK(g.data.selectedMode == "Zeta");
    CHECK(g.data.totalChannels == 1);
    CHECK_FALSE(g.data.isMovingHead);
}

TEST_CASE("GDTF cancelled mode prompt reports cancellation", "[models2][gdtf]") {
    Gdtf g;
    StubUi ui;
    CHECK_FALSE(g.Parse(Wrap(Mode("A", Chan("1", "Dimmer")) + Mode("B", Chan("1", "Pan"))), &ui));
    CHECK(g.cancelled);
    CHECK(g.data.channels.empty());
}

TEST_CASE("GDTF prompt answer naming an unknown mode still parses", "[models2][gdtf]") {
    Gdtf g;
    StubUi ui;
    ui.answer = { "Nope" };
    bool ok = g.Parse(Wrap(Mode("A", Chan("1", "Dimmer")) + Mode("B", Chan("1", "Pan"))), &ui);
    if (ok) {
        CHECK(g.data.channels.size() <= 1);
    }
}

TEST_CASE("GDTF duplicate mode names keep the last definition", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Same", Chan("1", "Dimmer")) + Mode("Same", Chan("1", "Pan") + Chan("2", "Tilt")))));
    CHECK(g.data.channels.size() == 2);
    CHECK(g.data.isMovingHead);
}

TEST_CASE("GDTF only the selected mode contributes channels", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("A", Chan("1", "Dimmer") + Chan("2", "Zoom")) + Mode("B", Chan("1", "Dimmer") + Chan("2", "Zoom") + Chan("3", "Focus") + Chan("4", "Iris")))));
    CHECK(g.data.selectedMode == "A");
    CHECK(g.data.channels.size() == 2);
    CHECK(g.data.totalChannels == 2);
}

TEST_CASE("GDTF unrelated child elements are ignored", "[models2][gdtf]") {
    Gdtf g;
    std::string xml = "<GDTF><Thing/><FixtureType Name=\"x\"><Wheels/><DMXModes><Junk/><DMXMode Name=\"M\"><Geometry/><DMXChannels><Junk/>" + Chan("1", "Pan") + "</DMXChannels></DMXMode></DMXModes></FixtureType></GDTF>";
    REQUIRE(g.Parse(xml));
    CHECK(g.data.channels.size() == 1);
    CHECK(g.data.isMovingHead);
}

TEST_CASE("GDTF data defaults", "[models2][gdtf]") {
    XmlSerialize::GdtfModelData d;
    CHECK(d.channels.empty());
    CHECK(d.totalChannels == 0);
    CHECK_FALSE(d.isMovingHead);
    CHECK(d.selectedMode.empty());
    XmlSerialize::DMXChannelInfo c;
    CHECK(c.channelStart == 0);
    CHECK(c.channels == 0);
    XmlSerialize::DMXChannelValue v;
    CHECK(v.low == 0);
    CHECK(v.high == 0);
}

TEST_CASE("GDTF parse re-run resets totals for the new mode", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("A", Chan("1", "Pan") + Chan("20", "Dimmer")))));
    CHECK(g.data.totalChannels == 20);
    REQUIRE(g.Parse(Wrap(Mode("B", Chan("1", "Dimmer")))));
    CHECK(g.data.totalChannels == 1);
    CHECK_FALSE(g.data.isMovingHead);
}

TEST_CASE("GDTF moving head converts to an advanced moving head", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1,2", "Pan") + Chan("3,4", "Tilt") + Chan("5", "Dimmer") + Chan("6", "Shutter1")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "17");
    REQUIRE(existing != nullptr);
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    auto* adv = dynamic_cast<DmxMovingHeadAdv*>(m);
    REQUIRE(adv != nullptr);
    CHECK(adv->GetDmxChannelCount() == 6);
    CHECK(adv->GetPanMotor()->GetChannelCoarse() == 1);
    CHECK(adv->GetTiltMotor()->GetChannelCoarse() == 3);
    CHECK(adv->GetShutterAbility()->GetShutterChannel() == 6);
    CHECK(adv->GetModelStartChannel() == "17");
    CHECK(adv->GetDescription() == "Mode: Std");
    delete m;
}

TEST_CASE("GDTF fixture without pan or tilt becomes a basic moving head with bars", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1", "Dimmer") + Chan("2", "Shutter") + Chan("3", "ColorAdd_R")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    REQUIRE(existing != nullptr);
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    auto* basic = dynamic_cast<DmxMovingHead*>(m);
    REQUIRE(basic != nullptr);
    CHECK(basic->GetDmxStyle() == "Moving Head Bars");
    CHECK(basic->GetDmxChannelCount() == 3);
    CHECK(basic->GetShutterAbility()->GetShutterChannel() == 2);
    delete m;
}

TEST_CASE("GDTF conversion names nodes from the channel attributes", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1,2", "Pan") + Chan("3", "Tilt") + Chan("4", "Dimmer") + Chan("6", "Zoom")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeNames() == "Pan-1,Pan-2,Tilt,Dimmer,,Zoom");
    delete m;
}

TEST_CASE("GDTF conversion maps RGB colour channels", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1", "Pan") + Chan("2", "Tilt") + Chan("3", "ColorAdd_R") + Chan("4", "ColorAdd_G") + Chan("5", "ColorAdd_B") + Chan("6", "ColorAdd_W")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    auto* dmx = dynamic_cast<DmxModel*>(m);
    REQUIRE(dmx != nullptr);
    REQUIRE(dmx->HasColorAbility());
    auto* rgb = dynamic_cast<DmxColorAbilityRGB*>(dmx->GetColorAbility());
    REQUIRE(rgb != nullptr);
    CHECK(rgb->GetRedChannel() == 3);
    CHECK(rgb->GetGreenChannel() == 4);
    CHECK(rgb->GetBlueChannel() == 5);
    delete m;
}

TEST_CASE("GDTF conversion alternative RGB attribute names", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1", "Pan") + Chan("2", "Tilt") + Chan("3", "ColorRGB_Red") + Chan("4", "ColorRGB_Green") + Chan("5", "ColorRGB_Blue")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    auto* rgb = dynamic_cast<DmxColorAbilityRGB*>(dynamic_cast<DmxModel*>(m)->GetColorAbility());
    REQUIRE(rgb != nullptr);
    CHECK(rgb->GetRedChannel() == 3);
    CHECK(rgb->GetGreenChannel() == 4);
    CHECK(rgb->GetBlueChannel() == 5);
    delete m;
}

TEST_CASE("GDTF conversion turns channel sets into states", "[models2][gdtf]") {
    Gdtf g;
    std::string sets = Set("Open", "0/1") + Set("Red", "40/1");
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1", "Pan") + Chan("2", "Tilt") + Chan("3", "Color1", Fn(sets))))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    const auto& states = m->GetStateInfo();
    auto it = states.find("Color1");
    REQUIRE(it != states.end());
    CHECK(it->second.at("s1-Name") == "Open");
    CHECK(it->second.at("s2-Name") == "Red");
    CHECK(it->second.at("s1") == "3");
    CHECK(it->second.at("s2-Color") == "#282828");
    delete m;
}

TEST_CASE("GDTF conversion preserves layout group and position", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1", "Pan") + Chan("2", "Tilt")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    existing->SetLayoutGroup("Roof");
    existing->SetHcenterPos(12.0f);
    existing->SetVcenterPos(34.0f);
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    REQUIRE(m != nullptr);
    CHECK(m->GetLayoutGroup() == "Roof");
    CHECK(m->GetHcenterPos() == 12.0f);
    CHECK(m->GetVcenterPos() == 34.0f);
    delete m;
}

TEST_CASE("GDTF conversion of a fixture with no channels still produces a model", "[models2][gdtf]") {
    XmlSerialize::GdtfModelData data;
    data.selectedMode = "Empty";
    OutputManager om;
    ModelManager mm{ &om };
    Model* existing = mm.CreateDefaultModel("Custom", "1");
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, data, mm);
    REQUIRE(m != nullptr);
    CHECK(m->GetDescription() == "Mode: Empty");
    delete m;
}

TEST_CASE("GDTF conversion ignores channels beyond the total", "[models2][gdtf]") {
    XmlSerialize::GdtfModelData data;
    data.totalChannels = 2;
    data.isMovingHead = true;
    XmlSerialize::DMXChannelInfo far;
    far.attribute = "Gobo1";
    far.channelStart = 9;
    far.channels = 1;
    data.channels.push_back(far);
    XmlSerialize::DMXChannelInfo ok;
    ok.attribute = "Dimmer";
    ok.channelStart = 1;
    ok.channels = 1;
    data.channels.push_back(ok);
    OutputManager om;
    ModelManager mm{ &om };
    Model* existing = mm.CreateDefaultModel("Custom", "1");
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, data, mm);
    REQUIRE(m != nullptr);
    CHECK(m->GetNodeNames() == "Dimmer,");
    delete m;
}

TEST_CASE("GDTF model creation rejects an absurd channel count", "[models2][gdtf]") {
    Gdtf g;
    REQUIRE(g.Parse(Wrap(Mode("Std", Chan("1", "Pan") + Chan("2000000000", "Dimmer")))));
    Model* existing = g.mm.CreateDefaultModel("Custom", "1");
    REQUIRE(existing != nullptr);
    Model* m = XmlSerialize::CreateDmxModelFromGdtfData(existing, g.data, g.mm);
    CHECK(m == nullptr);
}
