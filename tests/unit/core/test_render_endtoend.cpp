#include <catch2/catch_test_macros.hpp>

#include <filesystem>

#include "test_render_common.h"

using namespace renderhelp;

namespace {

const Rendered::Rgb kBlack{ 0, 0, 0 };
const Rendered::Rgb kRed{ 255, 0, 0 };
const Rendered::Rgb kGreen{ 0, 255, 0 };
const Rendered::Rgb kBlue{ 0, 0, 255 };

bool AllNodes(const Rendered& r, unsigned frame, unsigned nodes, const Rendered::Rgb& c) {
    for (unsigned n = 0; n < nodes; ++n) {
        if (!(r.rgb(frame, n) == c)) return false;
    }
    return true;
}

} // namespace

TEST_CASE("On effect renders red on every node of every frame", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    Rendered r = Render(s);
    REQUIRE(r.loaded);
    REQUIRE(r.opened);
    REQUIRE(r.rendered);
    CHECK(r.frames == 20);
    CHECK(r.channels >= 12);
    for (unsigned f = 0; f < r.frames; ++f) {
        INFO("frame " << f);
        CHECK(AllNodes(r, f, 4, kRed));
    }
}

TEST_CASE("Frame count is duration divided by frame interval", "[render][endtoend]") {
    struct Case { int duration; int frameMs; unsigned expected; };
    for (Case c : { Case{ 1000, 50, 20 }, Case{ 1000, 25, 40 }, Case{ 1000, 100, 10 }, Case{ 500, 50, 10 }, Case{ 2000, 50, 40 } }) {
        INFO("duration " << c.duration << " frame " << c.frameMs);
        ShowSpec s;
        s.durationMs = c.duration;
        s.frameMs = c.frameMs;
        s.effects.push_back(Eff("M", 0, "On", 0, c.duration, Pal("#00FF00")));
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        CHECK(r.frames == c.expected);
        CHECK(r.frameMs == (unsigned)c.frameMs);
        CHECK(AllNodes(r, 0, 4, kGreen));
        CHECK(AllNodes(r, c.expected - 1, 4, kGreen));
    }
}

TEST_CASE("Two effects in sequence on one layer", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 0, "On", 500, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f < 10; ++f) CHECK(AllNodes(r, f, 4, kRed));
    for (unsigned f = 10; f < 20; ++f) CHECK(AllNodes(r, f, 4, kBlue));
}

TEST_CASE("Frames outside any effect are black", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 250, 750, Pal("#FFFFFF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f = 0; f < 5; ++f) CHECK(AllNodes(r, f, 4, kBlack));
    for (unsigned f = 5; f < 15; ++f) CHECK(AllNodes(r, f, 4, Rendered::Rgb{ 255, 255, 255 }));
    for (unsigned f = 15; f < 20; ++f) CHECK(AllNodes(r, f, 4, kBlack));
}

TEST_CASE("Off effect between two On effects renders black", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 400, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 0, "Off", 400, 600, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 0, "On", 600, 1000, Pal("#FF0000")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(AllNodes(r, 7, 4, kRed));
    CHECK(AllNodes(r, 8, 4, kBlack));
    CHECK(AllNodes(r, 11, 4, kBlack));
    CHECK(AllNodes(r, 12, 4, kRed));
}

TEST_CASE("Empty sequence renders to all zeros", "[render][endtoend]") {
    ShowSpec s;
    Rendered r = Render(s);
    REQUIRE(r.loaded);
    REQUIRE(r.opened);
    REQUIRE(r.rendered);
    CHECK(r.frames == 20);
    for (unsigned f = 0; f < r.frames; ++f) {
        for (unsigned c = 0; c < r.channels; ++c) {
            REQUIRE(r.at(f, c) == 0);
        }
    }
}

TEST_CASE("Node colour channel order follows the string type", "[render][endtoend]") {
    struct Case { const char* type; int c0, c1, c2; };
    for (Case c : { Case{ "RGB Nodes", 255, 128, 0 }, Case{ "GRB Nodes", 128, 255, 0 }, Case{ "BRG Nodes", 0, 255, 128 }, Case{ "BGR Nodes", 0, 128, 255 }, Case{ "RBG Nodes", 255, 0, 128 }, Case{ "GBR Nodes", 128, 0, 255 } }) {
        INFO(c.type);
        ShowSpec s;
        s.models = SingleLine("M", 1, 2, c.type);
        s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF8000")));
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        for (unsigned n = 0; n < 2; ++n) {
            CHECK(std::abs(r.at(5, n * 3) - c.c0) <= 1);
            CHECK(std::abs(r.at(5, n * 3 + 1) - c.c1) <= 1);
            CHECK(std::abs(r.at(5, n * 3 + 2) - c.c2) <= 1);
        }
    }
}

TEST_CASE("Model start channel offsets the output", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("M", 10, 2);
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    REQUIRE(r.channels >= 15);
    for (unsigned c = 0; c < 9; ++c) CHECK(r.at(3, c) == 0);
    CHECK(r.at(3, 9) == 255);
    CHECK(r.at(3, 10) == 0);
    CHECK(r.at(3, 11) == 0);
    CHECK(r.at(3, 12) == 255);
    CHECK(r.at(3, 13) == 0);
    CHECK(r.at(3, 14) == 0);
}

TEST_CASE("Chained models write contiguous channels", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.modelElements = { "A", "B" };
    s.effects.push_back(Eff("A", 0, "On", 0, 1000, Pal("#FF0000")));
    s.effects.push_back(Eff("B", 0, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(4, 0) == kRed);
    CHECK(r.rgb(4, 1) == kRed);
    CHECK(r.rgb(4, 2) == kBlue);
    CHECK(r.rgb(4, 3) == kBlue);
}

TEST_CASE("Each model renders only its own effects", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.modelElements = { "A", "B" };
    s.effects.push_back(Eff("A", 0, "On", 0, 500, Pal("#FF0000")));
    s.effects.push_back(Eff("B", 0, "On", 500, 1000, Pal("#00FF00")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(2, 0) == kRed);
    CHECK(r.rgb(2, 2) == kBlack);
    CHECK(r.rgb(15, 0) == kBlack);
    CHECK(r.rgb(15, 2) == kGreen);
}

TEST_CASE("Effect on a model group lights every member", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 3);
    s.groups = R"(<modelGroup name="G" models="A,B"/>)";
    s.modelElements = { "A", "B", "G" };
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#00FFFF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    for (unsigned f : { 0u, 10u, 19u }) {
        CHECK(r.rgb(f, 0) == Rendered::Rgb{ 0, 255, 255 });
        CHECK(r.rgb(f, 1) == Rendered::Rgb{ 0, 255, 255 });
        CHECK(r.rgb(f, 2) == Rendered::Rgb{ 0, 255, 255 });
        CHECK(r.rgb(f, 3) == Rendered::Rgb{ 0, 255, 255 });
        CHECK(r.rgb(f, 4) == Rendered::Rgb{ 0, 255, 255 });
    }
}

TEST_CASE("Group effect is blended with member model effect by element order", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.groups = R"(<modelGroup name="G" models="A,B"/>)";
    s.modelElements = { "A", "B", "G" };
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#0000FF")));
    s.effects.push_back(Eff("A", 0, "On", 0, 1000, Pal("#FF0000"), "T_CHOICE_LayerMethod=Additive"));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 2) == kBlue);
    auto a = r.rgb(5, 0);
    CHECK((a == kRed || a == Rendered::Rgb{ 255, 0, 255 } || a == kBlue));
}

TEST_CASE("Matrix model sizes all render without crashing", "[render][endtoend]") {
    struct Case { int strands, nodes; };
    for (Case c : { Case{ 1, 1 }, Case{ 1, 7 }, Case{ 7, 1 }, Case{ 3, 5 }, Case{ 6, 2 }, Case{ 16, 16 } }) {
        INFO(c.strands << "x" << c.nodes);
        ShowSpec s;
        s.models = Matrix("M", 1, c.strands, c.nodes);
        s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000")));
        s.effects.push_back(Eff("M", 0, "Bars", 500, 1000, Pal("#00FF00", "#0000FF")));
        s.durationMs = 1000;
        Rendered r = Render(s);
        REQUIRE(r.rendered);
        const unsigned total = c.strands * c.nodes;
        CHECK(AllNodes(r, 2, total, kRed));
        bool anyLit = false;
        for (unsigned n = 0; n < total; ++n) {
            auto v = r.rgb(15, n);
            if (v.r || v.g || v.b) anyLit = true;
        }
        CHECK(anyLit);
    }
}

TEST_CASE("Single line model with one node", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("M", 1, 1);
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#123456")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(0, 0) == Rendered::Rgb{ 0x12, 0x34, 0x56 });
}

TEST_CASE("Render is deterministic across two independent runs", "[render][endtoend]") {
    ShowSpec s;
    s.models = Matrix("M", 1, 8, 8);
    s.effects.push_back(Eff("M", 0, "Twinkle", 0, 1000, Pal("#FF0000", "#00FF00")));
    s.effects.push_back(Eff("M", 1, "Snowstorm", 0, 1000, Pal("#FFFFFF", "#0000FF"), "T_CHOICE_LayerMethod=Additive"));
    Rendered a = Render(s);
    Rendered b = Render(s);
    REQUIRE(a.rendered);
    REQUIRE(b.rendered);
    CHECK(a.data == b.data);
}

TEST_CASE("WriteFseq produces a file that matches the rendered data", "[render][endtoend][fseq]") {
    ShowSpec s;
    s.durationMs = 600;
    s.effects.push_back(Eff("M", 0, "On", 0, 300, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 0, "On", 300, 600, Pal("#0000FF")));
    Show show(s);
    REQUIRE(show.result.rendered);
    const std::string path = show.dir.File("out.fseq");
    REQUIRE(show.ctx->WriteFseq(path));
    FseqContents f = ReadFseq(path);
    REQUIRE(f.ok);
    CHECK(f.frames == show.result.frames);
    CHECK(f.stepMs == 50);
    REQUIRE(f.frames == 12);
    for (unsigned fr = 0; fr < 12; ++fr) {
        INFO("frame " << fr);
        for (unsigned c = 0; c < 12; ++c) {
            REQUIRE(f.data[fr][c] == show.result.data[fr][c]);
        }
    }
    CHECK(f.data[0][0] == 255);
    CHECK(f.data[11][2] == 255);
}

TEST_CASE("Rendering the same show twice gives identical fseq contents", "[render][endtoend][fseq]") {
    ShowSpec s;
    s.models = Matrix("M", 1, 4, 4);
    s.effects.push_back(Eff("M", 0, "Bars", 0, 1000, Pal("#FF0000", "#0000FF")));
    Show a(s);
    Show b(s);
    REQUIRE(a.result.rendered);
    REQUIRE(b.result.rendered);
    REQUIRE(a.ctx->WriteFseq(a.dir.File("a.fseq")));
    REQUIRE(b.ctx->WriteFseq(b.dir.File("b.fseq")));
    FseqContents fa = ReadFseq(a.dir.File("a.fseq"));
    FseqContents fb = ReadFseq(b.dir.File("b.fseq"));
    REQUIRE(fa.ok);
    REQUIRE(fb.ok);
    CHECK(fa.frames == fb.frames);
    CHECK(fa.data == fb.data);
}

TEST_CASE("WriteFseq fails when no sequence is open", "[render][endtoend][fseq]") {
    TempDir dir;
    HeadlessRenderContext ctx;
    CHECK_FALSE(ctx.WriteFseq(dir.File("none.fseq")));
    CHECK_FALSE(std::filesystem::exists(dir.path / "none.fseq"));
}

TEST_CASE("RenderAndWait fails when no sequence is open", "[render][endtoend]") {
    EnsureLoggers();
    HeadlessRenderContext ctx;
    CHECK_FALSE(ctx.RenderAndWait(1000));
    CHECK_FALSE(ctx.IsSequenceLoaded());
}

TEST_CASE("OpenSequence fails for a missing or malformed file", "[render][endtoend]") {
    ShowSpec s;
    Show show(s);
    REQUIRE(show.result.opened);
    CHECK_FALSE(show.ctx->OpenSequence(show.dir.File("does_not_exist.xsq")));
    CHECK_FALSE(show.ctx->IsSequenceLoaded());
    std::string bad = show.dir.Write("bad.xsq", "<xsequence><head>");
    CHECK_FALSE(show.ctx->OpenSequence(bad));
}

TEST_CASE("LoadShowFolder fails for an empty folder", "[render][endtoend]") {
    EnsureLoggers();
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;
    CHECK_FALSE(ctx.LoadShowFolder(dir.path.string()));
}

TEST_CASE("A context can render a second sequence after the first", "[render][endtoend]") {
    ShowSpec s1;
    s1.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s1);
    REQUIRE(show.result.rendered);
    CHECK(show.result.rgb(3, 0) == kRed);

    ShowSpec s2;
    s2.durationMs = 500;
    s2.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#00FF00")));
    std::string p2 = show.dir.Write("second.xsq", BuildXsq(s2));
    REQUIRE(show.ctx->OpenSequence(p2));
    REQUIRE(show.ctx->RenderAndWait(60000));
    show.Capture();
    CHECK(show.result.frames == 10);
    CHECK(show.result.rgb(3, 0) == kGreen);
}

TEST_CASE("Re-rendering the same open sequence gives the same output", "[render][endtoend]") {
    ShowSpec s;
    s.models = Matrix("M", 1, 4, 4);
    s.effects.push_back(Eff("M", 0, "Bars", 0, 1000, Pal("#FF0000", "#0000FF")));
    Show show(s);
    REQUIRE(show.result.rendered);
    auto first = show.result.data;
    REQUIRE(show.ctx->RenderAndWait(60000));
    show.Capture();
    CHECK(show.result.data == first);
}

TEST_CASE("Render disabled element stays dark", "[render][endtoend]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.modelElements = { "A", "B" };
    s.disabledElements = { "A" };
    s.effects.push_back(Eff("A", 0, "On", 0, 1000, Pal("#FF0000")));
    s.effects.push_back(Eff("B", 0, "On", 0, 1000, Pal("#FF0000")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 0) == kBlack);
    CHECK(r.rgb(5, 2) == kRed);
}

TEST_CASE("AbortRender after a completed render succeeds and keeps data", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s);
    REQUIRE(show.result.rendered);
    CHECK(show.ctx->IsRenderDone());
    CHECK(show.ctx->AbortRender(5000));
    show.Capture();
    CHECK(show.result.rgb(3, 0) == kRed);
}

TEST_CASE("CloseSequence releases the sequence", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s);
    REQUIRE(show.result.rendered);
    CHECK(show.ctx->IsSequenceLoaded());
    CHECK(show.ctx->CloseSequence());
    CHECK_FALSE(show.ctx->IsSequenceLoaded());
    CHECK_FALSE(show.ctx->RenderAndWait(1000));
}

TEST_CASE("Effect end is exclusive of the following frame", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 100, 300, Pal("#FF0000")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(AllNodes(r, 1, 4, kBlack));
    CHECK(AllNodes(r, 2, 4, kRed));
    CHECK(AllNodes(r, 5, 4, kRed));
    CHECK(AllNodes(r, 6, 4, kBlack));
}

TEST_CASE("Colour Wash blends between its two palette colours over time", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "Color Wash", 0, 1000, Pal("#FF0000", "#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    auto first = r.rgb(0, 0);
    auto last = r.rgb(19, 0);
    CHECK(first.r > first.b);
    CHECK(last.b > last.r);
}

TEST_CASE("Timing tracks in the sequence do not disturb rendering", "[render][endtoend]") {
    ShowSpec s;
    s.modelElements = { "M" };
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    std::string xsq = BuildXsq(s);
    const std::string de = "<DisplayElements>";
    xsq.insert(xsq.find(de) + de.size(), "<Element collapsed=\"0\" type=\"timing\" name=\"T1\" visible=\"1\" active=\"1\" views=\"\"/>");
    const std::string ee = "<ElementEffects>";
    xsq.insert(xsq.find(ee) + ee.size(), "<Element type=\"timing\" name=\"T1\"><EffectLayer><Effect label=\"a\" startTime=\"0\" endTime=\"500\"/><Effect label=\"b\" startTime=\"500\" endTime=\"1000\"/></EffectLayer></Element>");
    Show show(s, xsq);
    REQUIRE(show.result.opened);
    REQUIRE(show.result.rendered);
    CHECK(show.result.rgb(10, 0) == kRed);
    CHECK(show.ctx->_sequenceElements.GetElement("T1") != nullptr);
}

TEST_CASE("Unknown model element in the sequence is ignored", "[render][endtoend]") {
    ShowSpec s;
    s.modelElements = { "M", "Ghost" };
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    s.effects.push_back(Eff("Ghost", 0, "On", 0, 1000, Pal("#00FF00")));
    Rendered r = Render(s);
    REQUIRE(r.opened);
    REQUIRE(r.rendered);
    CHECK(r.rgb(5, 0) == kRed);
}

TEST_CASE("Zero length sequence cannot be rendered", "[render][endtoend]") {
    ShowSpec s;
    s.durationMs = 0;
    Show show(s);
    CHECK_FALSE(show.result.rendered);
}

TEST_CASE("Higher layer index is the background of a lower one", "[render][endtoend]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 1, "On", 0, 1000, Pal("#0000FF")));
    Rendered r = Render(s);
    REQUIRE(r.rendered);
    CHECK(AllNodes(r, 3, 4, kRed));
    CHECK(AllNodes(r, 15, 4, kBlue));
}
