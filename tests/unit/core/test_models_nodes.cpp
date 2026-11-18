#include <array>
#include <string>
#include <vector>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelManager.h"
#include "models/Node.h"
#include "models/PWMOutput.h"
#include "outputs/OutputManager.h"
#include "utils/Color.h"

// The RGBW handling modes, in the order Model::SetRGBWHandling names them.
namespace {
constexpr int kNormal = 0;   // "R=G=B -> W"
constexpr int kRGBOnly = 1;  // "RGB Only"
constexpr int kWhiteOnly = 2; // "White Only"
constexpr int kAdvanced = 3; // "Advanced"
constexpr int kAll = 4;      // "White On All"

using Bytes = std::array<unsigned char, 5>;

Bytes Out(const NodeBaseClass& n, const xlColor& c) {
    std::unique_ptr<NodeBaseClass> copy(n.clone());
    copy->SetColor(c);
    Bytes b{};
    copy->GetForChannels(b.data());
    return b;
}

// Several modes scale through a float, which can land one below the exact value.
bool Near(int a, int b) {
    return a >= b - 1 && a <= b + 1;
}

xlColor In(const NodeBaseClass& n, Bytes b) {
    std::unique_ptr<NodeBaseClass> copy(n.clone());
    copy->SetFromChannels(b.data());
    xlColor c;
    copy->GetColor(c);
    return c;
}

struct ShowFixture {
    OutputManager om;
    ModelManager mm{ &om };

    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        mm.RecalcStartChannels();
        return m;
    }

    Model* Line(const std::string& stringType, int nodes, const std::string& extra = "", const std::string& children = "", int start = 1) {
        return Add("<model name=\"L\" DisplayAs=\"Single Line\" StartChannel=\"" + std::to_string(start) + "\" parm1=\"1\" parm2=\"" + std::to_string(nodes) +
                   "\" parm3=\"1\" StringType=\"" + stringType + "\" " + extra + ">" + children + "</model>");
    }
};
} // namespace

// ---- RGB nodes ----

TEST_CASE("RGB node type names follow the colour order", "[models][nodes]") {
    for (const char* order : { "RGB", "RBG", "GRB", "GBR", "BRG", "BGR" }) {
        INFO(order);
        NodeBaseClass n(0, 1, order);
        CHECK(n.GetNodeType() == order);
        CHECK(n.GetChanCount() == 3);
    }
}

TEST_CASE("RGB node channels are written and read in the node's colour order", "[models][nodes]") {
    NodeBaseClass grb(0, 1, "GRB");
    Bytes b = Out(grb, xlColor(10, 20, 30));
    CHECK(b[0] == 20);
    CHECK(b[1] == 10);
    CHECK(b[2] == 30);
    CHECK(In(grb, { 20, 10, 30 }) == xlColor(10, 20, 30));

    NodeBaseClass bgr(0, 1, "BGR");
    b = Out(bgr, xlColor(10, 20, 30));
    CHECK(b[0] == 30);
    CHECK(b[1] == 20);
    CHECK(b[2] == 10);
}

// ---- single colour nodes ----

TEST_CASE("Single colour nodes use one channel holding their own component", "[models][nodes]") {
    NodeClassRed r(0, 1);
    NodeClassGreen g(0, 1);
    NodeClassBlue bl(0, 1);
    const xlColor c(10, 20, 30);
    CHECK(r.GetChanCount() == 1);
    CHECK(Out(r, c)[0] == 10);
    CHECK(Out(g, c)[0] == 20);
    CHECK(Out(bl, c)[0] == 30);
    CHECK(r.GetNodeType() == NodeBaseClass::RED);
    CHECK(g.GetNodeType() == NodeBaseClass::GREEN);
    CHECK(bl.GetNodeType() == NodeBaseClass::BLUE);
}

TEST_CASE("White node outputs the dimmest component and reads back as grey", "[models][nodes]") {
    NodeClassWhite w(0, 1);
    CHECK(w.GetNodeType() == NodeBaseClass::WHITE);
    CHECK(Out(w, xlColor(200, 90, 150))[0] == 90);
    CHECK(In(w, { 77 }) == xlColor(77, 77, 77));
}

TEST_CASE("Custom colour node keeps only colours of its own hue", "[models][nodes]") {
    NodeClassCustom n(0, 1, xlColor(255, 0, 0));
    CHECK(n.GetChanCount() == 1);
    CHECK(Out(n, xlColor(128, 0, 0))[0] == 128);
    CHECK(Out(n, xlColor(0, 0, 255))[0] == 0);
    // White and grey count as the node's own colour at that brightness
    CHECK(Out(n, xlColor(200, 200, 200))[0] == 200);
    CHECK(n.GetMaskColor() == xlColor(255, 0, 0));
}

TEST_CASE("Custom colour node reads back as its colour at the channel level", "[models][nodes]") {
    NodeClassCustom n(0, 1, xlColor(0, 0, 255));
    xlColor c = In(n, { 128 });
    CHECK(c.red == 0);
    CHECK(c.green == 0);
    CHECK(c.blue == 128);
}

TEST_CASE("Intensity node outputs the brightness of any colour", "[models][nodes]") {
    NodeClassIntensity n(0, 1, xlColor(0, 255, 0));
    CHECK(Out(n, xlColor(0, 0, 255))[0] == 255);
    CHECK(Out(n, xlColor(64, 32, 0))[0] == 64);
    xlColor c = In(n, { 100 });
    CHECK(c.red == 0);
    CHECK(c.green == 100);
    CHECK(c.blue == 0);
}

// ---- RGBW nodes ----

TEST_CASE("RGBW node type names follow the colour order and the white position", "[models][nodes][rgbw]") {
    struct Case {
        const char* order;
        const char* whiteLast;
        const char* whiteFirst;
    };
    const Case cases[] = {
        { "RGB", "RGBW", "WRGB" }, { "RBG", "RBGW", "WRBG" }, { "GRB", "GRBW", "WGRB" },
        { "GBR", "GBRW", "WGBR" }, { "BRG", "BRGW", "WBRG" }, { "BGR", "BGRW", "WBGR" },
    };
    for (const auto& c : cases) {
        INFO(c.order);
        CHECK(NodeClassRGBW(0, 1, c.order, true, kNormal).GetNodeType() == c.whiteLast);
        CHECK(NodeClassRGBW(0, 1, c.order, false, kNormal).GetNodeType() == c.whiteFirst);
    }
}

TEST_CASE("RGBW normal mode sends grey to the white channel only", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "RGB", true, kNormal);
    CHECK(n.GetChanCount() == 4);
    CHECK(Out(n, xlColor(90, 90, 90)) == Bytes{ 0, 0, 0, 90, 0 });
    CHECK(Out(n, xlColor(90, 40, 10)) == Bytes{ 90, 40, 10, 0, 0 });
}

TEST_CASE("RGBW with white first puts white on the first channel", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "GRB", false, kNormal);
    CHECK(Out(n, xlColor(90, 90, 90)) == Bytes{ 90, 0, 0, 0, 0 });
    CHECK(Out(n, xlColor(10, 20, 30)) == Bytes{ 0, 20, 10, 30, 0 });
    CHECK(In(n, { 0, 20, 10, 30 }) == xlColor(10, 20, 30));
    CHECK(In(n, { 50, 20, 10, 30 }) == xlColor(50, 50, 50));
}

TEST_CASE("RGBW RGB-only mode never lights white", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "RGB", true, kRGBOnly);
    CHECK(Out(n, xlColor(90, 90, 90)) == Bytes{ 90, 90, 90, 0, 0 });
    CHECK(Out(n, xlColor(1, 2, 3)) == Bytes{ 1, 2, 3, 0, 0 });
    CHECK(In(n, { 1, 2, 3, 200 }) == xlColor(1, 2, 3));
}

TEST_CASE("RGBW white-only mode lights only white, and only for grey", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "RGB", true, kWhiteOnly);
    CHECK(Out(n, xlColor(90, 90, 90)) == Bytes{ 0, 0, 0, 90, 0 });
    CHECK(Out(n, xlColor(90, 40, 10)) == Bytes{ 0, 0, 0, 0, 0 });
    CHECK(In(n, { 1, 2, 3, 60 }) == xlColor(60, 60, 60));
}

TEST_CASE("RGBW white-on-all mode lights white alongside RGB for grey", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "RGB", true, kAll);
    CHECK(Out(n, xlColor(90, 90, 90)) == Bytes{ 90, 90, 90, 90, 0 });
    CHECK(Out(n, xlColor(90, 40, 10)) == Bytes{ 90, 40, 10, 0, 0 });
    CHECK(In(n, { 1, 2, 3, 60 }) == xlColor(1, 2, 3));
}

TEST_CASE("RGBW advanced mode moves the common part of the colour to white", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "RGB", true, kAdvanced);
    CHECK(Out(n, xlColor(0, 0, 0)) == Bytes{ 0, 0, 0, 0, 0 });
    CHECK(Out(n, xlColor(255, 255, 255)) == Bytes{ 0, 0, 0, 255, 0 });
    CHECK(Out(n, xlColor(255, 0, 0)) == Bytes{ 255, 0, 0, 0, 0 });

    // White takes (about) the smallest component and RGB keeps the rest, so the
    // sum of each colour channel and white is always the original component.
    for (const xlColor c : { xlColor(200, 100, 50), xlColor(10, 250, 120), xlColor(77, 77, 3), xlColor(1, 2, 3) }) {
        INFO((int)c.red << "," << (int)c.green << "," << (int)c.blue);
        Bytes b = Out(n, c);
        int minc = std::min<int>(c.red, std::min(c.green, c.blue));
        CHECK(b[3] <= minc);
        CHECK(b[3] + 1 >= minc);
        CHECK(b[0] + b[3] == c.red);
        CHECK(b[1] + b[3] == c.green);
        CHECK(b[2] + b[3] == c.blue);
    }
}

TEST_CASE("RGBW advanced mode reads back what it wrote", "[models][nodes][rgbw]") {
    NodeClassRGBW n(0, 1, "BRG", false, kAdvanced);
    for (const xlColor c : { xlColor(200, 100, 50), xlColor(255, 255, 255), xlColor(0, 0, 0), xlColor(30, 0, 220) }) {
        Bytes b = Out(n, c);
        CHECK(In(n, b) == c);
    }
}

TEST_CASE("RGBW advanced mode saturates rather than wrapping on channel data it did not write", "[models][nodes][rgbw]") {
    // Channel data from outside xLights' own RGBW split (an imported or edited
    // fseq) can carry more in colour plus white than one component holds.
    NodeClassRGBW n(0, 1, "RGB", true, kAdvanced);
    CHECK(In(n, { 200, 10, 0, 100 }) == xlColor(255, 110, 100));
    NodeClassRGBWW ww(0, 1, "RGB", kAdvanced);
    CHECK(In(ww, { 250, 0, 0, 250, 0 }) == xlColor(255, 250, 250));
}

// ---- RGBWW nodes ----

TEST_CASE("RGBWW nodes use five channels and never light the second white", "[models][nodes][rgbw]") {
    for (int mode : { kNormal, kRGBOnly, kWhiteOnly, kAdvanced, kAll }) {
        INFO("mode " << mode);
        NodeClassRGBWW n(0, 1, "RGB", mode);
        CHECK(n.GetChanCount() == 5);
        CHECK(n.GetNodeType() == NodeBaseClass::RGBWW);
        Bytes b{ 0, 0, 0, 0, 99 };
        n.SetColor(xlColor(255, 255, 255));
        n.GetForChannels(b.data());
        CHECK(b[4] == 0);
    }
}

TEST_CASE("RGBWW modes match RGBW with white last", "[models][nodes][rgbw]") {
    for (int mode : { kNormal, kRGBOnly, kWhiteOnly, kAdvanced, kAll }) {
        for (const xlColor c : { xlColor(90, 90, 90), xlColor(200, 100, 50), xlColor(0, 0, 0), xlColor(0, 255, 0) }) {
            INFO("mode " << mode << " colour " << (int)c.red << "," << (int)c.green << "," << (int)c.blue);
            CHECK(Out(NodeClassRGBWW(0, 1, "GRB", mode), c) == Out(NodeClassRGBW(0, 1, "GRB", true, mode), c));
        }
    }
}

// ---- SuperString nodes ----

namespace {
const std::vector<xlColor> kRGBW = { xlColor(255, 0, 0), xlColor(0, 255, 0), xlColor(0, 0, 255), xlColor(255, 255, 255) };
}

TEST_CASE("SuperString uses one channel per colour", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, kRGBW, kNormal);
    CHECK(n.GetChanCount() == 4);
    NodeClassSuperString two(0, 1, { xlColor(255, 0, 0), xlColor(255, 255, 255) }, kNormal);
    CHECK(two.GetChanCount() == 2);
}

TEST_CASE("SuperString reads back the brightest contribution of each colour", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, kRGBW, kNormal);
    CHECK(In(n, { 255, 0, 0, 0 }) == xlColor(255, 0, 0));
    CHECK(In(n, { 0, 0, 0, 128 }) == xlColor(128, 128, 128));
    CHECK(In(n, { 255, 0, 0, 128 }) == xlColor(255, 128, 128));
    CHECK(In(n, { 0, 0, 0, 0 }) == xlColor(0, 0, 0));
}

TEST_CASE("SuperString normal mode lights only the white string for grey", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, kRGBW, kNormal);
    Bytes b = Out(n, xlColor(100, 100, 100));
    CHECK(b[0] == 0);
    CHECK(b[1] == 0);
    CHECK(b[2] == 0);
    CHECK(Near(b[3], 100));
}

TEST_CASE("SuperString normal mode lights the matching string for a primary colour", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, kRGBW, kNormal);
    CHECK(Out(n, xlColor(0, 180, 0)) == Bytes{ 0, 180, 0, 0, 0 });
    CHECK(Out(n, xlColor(0, 0, 40)) == Bytes{ 0, 0, 40, 0, 0 });
}

TEST_CASE("SuperString normal mode scales each colour string for a mixed colour", "[models][nodes][superstring]") {
    // Yellow-ish has no matching string: each coloured string gets the
    // fraction of its own colour the pixel can show.
    NodeClassSuperString n(0, 1, { xlColor(255, 0, 0), xlColor(0, 255, 0), xlColor(255, 255, 255) }, kNormal);
    Bytes b = Out(n, xlColor(255, 128, 0));
    CHECK(Near(b[0], 255));
    CHECK(Near(b[1], 128));
    CHECK(b[2] == 0);
}

TEST_CASE("SuperString white-only mode lights white with the common level", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, kRGBW, kWhiteOnly);
    CHECK(Out(n, xlColor(200, 100, 50))[3] == 50);
    CHECK(Out(n, xlColor(100, 100, 100))[3] == 100);
}

TEST_CASE("SuperString advanced mode splits the common part to white", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, kRGBW, kAdvanced);
    Bytes b = Out(n, xlColor(200, 50, 50));
    CHECK(b[3] == 50);
    CHECK(Near(b[0], 150));
    CHECK(b[1] == 0);
    CHECK(b[2] == 0);
}

TEST_CASE("SuperString of a single colour lights that colour for a matching pixel", "[models][nodes][superstring]") {
    NodeClassSuperString n(0, 1, { xlColor(0, 0, 255) }, kAll);
    CHECK(Out(n, xlColor(0, 0, 77))[0] == 77);
    CHECK(Out(n, xlColor(0, 0, 0))[0] == 0);
}

// ---- through a model ----

TEST_CASE("Model string types create nodes with the matching channel layout", "[models][nodes]") {
    struct Case {
        const char* stringType;
        int chans;
        const char* nodeType;
    };
    const Case cases[] = {
        { "RGB Nodes", 3, "RGB" }, { "GRB Nodes", 3, "GRB" }, { "BRG Nodes", 3, "BRG" },
        { "RGBW Nodes", 4, "RGBW" }, { "GRBW Nodes", 4, "GRBW" }, { "BRGW Nodes", 4, "BRGW" },
        { "RBGW Nodes", 4, "RBGW" }, { "WRGB Nodes", 4, "WRGB" }, { "WBRG Nodes", 4, "WBRG" },
        { "RGBWW Nodes", 5, "RGBWW" },
    };
    for (const auto& c : cases) {
        INFO(c.stringType);
        ShowFixture f;
        Model* m = f.Line(c.stringType, 3);
        REQUIRE(m->GetNodeCount() == 3);
        CHECK(m->GetChanCountPerNode() == c.chans);
        CHECK(m->GetChanCount() == 3 * c.chans);
        CHECK(m->NodeType(0) == c.nodeType);
        CHECK(m->NodeStartChannel(1) == (uint32_t)c.chans);
    }
}

TEST_CASE("Model RGBW handling reaches the nodes", "[models][nodes]") {
    ShowFixture f;
    Model* m = f.Line("RGBW Nodes", 2, "RGBWHandling=\"RGB Only\"");
    CHECK(m->GetRGBWHandling() == "RGB Only");
    m->SetNodeColor(0, xlColor(80, 80, 80));
    Bytes b{};
    m->GetNodeChannelValues(0, b.data());
    CHECK(b == Bytes{ 80, 80, 80, 0, 0 });

    m->SetRGBWHandling("White Only");
    m->Setup();
    m->SetNodeColor(0, xlColor(80, 80, 80));
    b = {};
    m->GetNodeChannelValues(0, b.data());
    CHECK(b == Bytes{ 0, 0, 0, 80, 0 });
}

TEST_CASE("Superstring models of up to three colours drive only the coloured strings", "[models][nodes][superstring]") {
    ShowFixture f;
    Model* m = f.Line("Superstring", 1, "SuperStringColour0=\"#FF0000\" SuperStringColour1=\"#FFFFFF\"");
    CHECK(m->GetChanCountPerNode() == 2);
    m->SetNodeColor(0, xlColor(255, 0, 0));
    Bytes b{};
    m->GetNodeChannelValues(0, b.data());
    CHECK(b[0] == 255);
    CHECK(b[1] == 0);
}

// ---- PWM outputs ----

TEST_CASE("PWM outputs give each channel of a node its own port", "[models][nodes][pwm]") {
    ShowFixture f;
    Model* m = f.Line("RGB Nodes", 2, "", "<ControllerConnection Protocol=\"PWM\" Port=\"1\"/>", 11);
    REQUIRE(m->IsPWMProtocol());
    REQUIRE(m->NodeStartChannel(0) == 10);
    std::vector<PWMOutput> out = m->GetPWMOutputs();
    REQUIRE(out.size() == 6);
    // FPP's PWM outputs index the channel data with this directly: 0 based,
    // like the DMX models' PWM outputs.
    for (size_t i = 0; i < out.size(); ++i) {
        INFO("output " << i);
        CHECK(out[i].startChannel == 10 + i);
        CHECK(out[i].channels == 1);
        CHECK(out[i].type == PWMOutput::Type::LED);
    }
    CHECK(out[0].label == "LED-1-R");
    CHECK(out[1].label == "LED-1-G");
    CHECK(out[2].label == "LED-1-B");
    CHECK(out[3].label == "LED-2-R");
}

TEST_CASE("PWM output labels follow the node colour order", "[models][nodes][pwm]") {
    struct Case {
        const char* stringType;
        std::vector<std::string> labels;
    };
    const Case cases[] = {
        { "GRB Nodes", { "LED-1-G", "LED-1-R", "LED-1-B" } },
        { "BRGW Nodes", { "LED-1-B", "LED-1-R", "LED-1-G", "LED-1-W" } },
        { "RBGW Nodes", { "LED-1-R", "LED-1-B", "LED-1-G", "LED-1-W" } },
        { "WRGB Nodes", { "LED-1-W", "LED-1-R", "LED-1-G", "LED-1-B" } },
    };
    for (const auto& c : cases) {
        INFO(c.stringType);
        ShowFixture f;
        Model* m = f.Line(c.stringType, 1, "", "<ControllerConnection Protocol=\"PWM\" Port=\"1\"/>");
        std::vector<PWMOutput> out = m->GetPWMOutputs();
        REQUIRE(out.size() == c.labels.size());
        for (size_t i = 0; i < out.size(); ++i) {
            CHECK(out[i].label == c.labels[i]);
        }
    }
}

TEST_CASE("PWM outputs use node names when the model has them", "[models][nodes][pwm]") {
    ShowFixture f;
    Model* m = f.Line("Node Single Color", 2, "NodeNames=\"Left,Right\"", "<ControllerConnection Protocol=\"PWM\" Port=\"1\"/>");
    std::vector<PWMOutput> out = m->GetPWMOutputs();
    REQUIRE(out.size() == 2);
    CHECK(out[0].label == "Left");
    CHECK(out[1].label == "Right");
    CHECK(out[0].startChannel == 0);
    CHECK(out[1].startChannel == 1);
}

TEST_CASE("Models not on a PWM port have no PWM outputs", "[models][nodes][pwm]") {
    ShowFixture f;
    Model* m = f.Line("RGB Nodes", 2, "", "<ControllerConnection Protocol=\"ws2811\" Port=\"1\"/>");
    CHECK(m->GetPWMOutputs().empty());
}
