#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <set>
#include <chrono>
#include <thread>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/IRenderProgressSink.h"
#include "render/RenderEngine.h"
#include "render/RenderProgressInfo.h"
#include "render/SequenceElements.h"

#include "test_render_common.h"


using namespace renderhelp;

namespace {

using Rgb = Rendered::Rgb;
const Rgb kBlack{ 0, 0, 0 };
const Rgb kRed{ 255, 0, 0 };
const Rgb kGreen{ 0, 255, 0 };
const Rgb kBlue{ 0, 0, 255 };

bool WaitDone(HeadlessRenderContext& ctx, int timeoutMs = 60000) {
    auto start = std::chrono::steady_clock::now();
    while (!ctx.IsRenderDone()) {
        if (std::chrono::steady_clock::now() - start > std::chrono::milliseconds(timeoutMs)) return false;
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    return true;
}

void FillAll(SequenceData& sd, uint8_t v) {
    for (unsigned f = 0; f < sd.NumFrames(); ++f) {
        for (unsigned c = 0; c < sd.NumChannels(); ++c) sd[f][c] = v;
    }
}

Effect* FirstEffect(HeadlessRenderContext& ctx, const std::string& model, int layer = 0) {
    Element* el = ctx._sequenceElements.GetElement(model);
    REQUIRE(el != nullptr);
    EffectLayer* l = el->GetEffectLayer(layer);
    REQUIRE(l != nullptr);
    REQUIRE(l->GetEffectCount() > 0);
    return l->GetEffect(0);
}

void ResetDirty(HeadlessRenderContext& ctx) {
    for (size_t i = 0; i < ctx._sequenceElements.GetElementCount(); ++i) {
        Element* el = ctx._sequenceElements.GetElement(i);
        int changes, st, ed;
        el->GetAndResetDirtyRange(changes, st, ed);
    }
}

ShowSpec TwoModels() {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.modelElements = { "A", "B" };
    s.effects.push_back(Eff("A", 0, "On", 0, 1000, Pal("#FF0000")));
    s.effects.push_back(Eff("B", 0, "On", 0, 1000, Pal("#00FF00")));
    return s;
}

struct CountingSink : IRenderProgressSink {
    std::atomic<int>* setups;
    std::atomic<int>* completes;
    CountingSink(std::atomic<int>* s, std::atomic<int>* c) : setups(s), completes(c) {}
    void SetupJobProgress(IRenderJobStatus*) override { ++*setups; }
    void OnRenderSetupComplete() override { ++*completes; }
};

} // namespace

TEST_CASE("Partial render with clear only rewrites its frame range", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto& sd = show.ctx->_seqData;
    FillAll(sd, 0x11);
    auto* eng = show.ctx->_renderEngine.get();
    REQUIRE(eng != nullptr);
    std::atomic<int> cbCount{ 0 };
    eng->Render(show.ctx->_sequenceElements, sd, eng->GetRenderTree().GetModels(), {}, 5, 9, nullptr, true,
                [&cbCount](bool) { ++cbCount; });
    REQUIRE(WaitDone(*show.ctx));
    CHECK(cbCount == 1);
    show.Capture();
    for (unsigned f = 0; f < 20; ++f) {
        INFO("frame " << f);
        if (f >= 5 && f <= 9) {
            CHECK(show.result.rgb(f, 0) == kRed);
            CHECK(show.result.rgb(f, 2) == kGreen);
        } else {
            CHECK(show.result.rgb(f, 0) == Rgb{ 0x11, 0x11, 0x11 });
            CHECK(show.result.rgb(f, 2) == Rgb{ 0x11, 0x11, 0x11 });
        }
    }
}

TEST_CASE("Partial render clamps an out of range frame range", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto& sd = show.ctx->_seqData;
    FillAll(sd, 0);
    auto* eng = show.ctx->_renderEngine.get();
    eng->Render(show.ctx->_sequenceElements, sd, eng->GetRenderTree().GetModels(), {}, -10, 500, nullptr, true, [](bool) {});
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    CHECK(show.result.rgb(0, 0) == kRed);
    CHECK(show.result.rgb(19, 2) == kGreen);
}

TEST_CASE("Restricted render only touches the restricted model's channels", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto& sd = show.ctx->_seqData;
    FillAll(sd, 0x11);
    auto* eng = show.ctx->_renderEngine.get();
    std::list<Model*> restrict{ show.ctx->AllModels["B"] };
    REQUIRE(restrict.front() != nullptr);
    eng->Render(show.ctx->_sequenceElements, sd, eng->GetRenderTree().GetModels(), restrict, 0, 19, nullptr, true, [](bool) {});
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    CHECK(show.result.rgb(4, 0) == Rgb{ 0x11, 0x11, 0x11 });
    CHECK(show.result.rgb(4, 1) == Rgb{ 0x11, 0x11, 0x11 });
    CHECK(show.result.rgb(4, 2) == kGreen);
    CHECK(show.result.rgb(4, 3) == kGreen);
}

TEST_CASE("Restricted clear render blanks a model with no effects", "[render2][engine]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.modelElements = { "A", "B" };
    s.effects.push_back(Eff("A", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s);
    REQUIRE(show.result.rendered);
    auto& sd = show.ctx->_seqData;
    auto* eng = show.ctx->_renderEngine.get();
    std::list<Model*> restrict{ show.ctx->AllModels["B"] };

    SECTION("clear") {
        FillAll(sd, 0x22);
        eng->Render(show.ctx->_sequenceElements, sd, eng->GetRenderTree().GetModels(), restrict, 0, 19, nullptr, true, [](bool) {});
        REQUIRE(WaitDone(*show.ctx));
        show.Capture();
        CHECK(show.result.rgb(3, 2) == kBlack);
        CHECK(show.result.rgb(3, 0) == Rgb{ 0x22, 0x22, 0x22 });
    }
    SECTION("no clear leaves it alone") {
        FillAll(sd, 0x22);
        eng->Render(show.ctx->_sequenceElements, sd, eng->GetRenderTree().GetModels(), restrict, 0, 19, nullptr, false, [](bool) {});
        REQUIRE(WaitDone(*show.ctx));
        show.Capture();
        CHECK(show.result.rgb(3, 2) == Rgb{ 0x22, 0x22, 0x22 });
    }
}

TEST_CASE("Render reports each job to the progress sink", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto* eng = show.ctx->_renderEngine.get();
    std::atomic<int> setups{ 0 }, completes{ 0 };
    std::atomic<int> timerStarts{ 0 };
    std::atomic<int> allDone{ 0 };
    eng->SetOnRenderStatusTimerStart([&] { ++timerStarts; });
    eng->SetOnAllRenderJobsComplete([&] { ++allDone; });
    eng->Render(show.ctx->_sequenceElements, show.ctx->_seqData, eng->GetRenderTree().GetModels(), {}, 0, 19,
                std::make_unique<CountingSink>(&setups, &completes), true, [](bool) {});
    REQUIRE(WaitDone(*show.ctx));
    CHECK(timerStarts == 1);
    CHECK(setups == 2);
    CHECK(completes == 1);
    eng->SetOnRenderStatusTimerStart(nullptr);
    eng->SetOnAllRenderJobsComplete(nullptr);
}

TEST_CASE("Render with nothing to do completes immediately", "[render2][engine]") {
    ShowSpec s;
    s.effects.clear();
    s.effects.push_back(Eff("M", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s);
    REQUIRE(show.result.rendered);
    auto* eng = show.ctx->_renderEngine.get();
    std::atomic<int> cb{ 0 };
    // no models at all
    eng->Render(show.ctx->_sequenceElements, show.ctx->_seqData, {}, {}, 0, 19, nullptr, true, [&cb](bool) { ++cb; });
    REQUIRE(WaitDone(*show.ctx));
    CHECK(cb == 1);
    show.Capture();
    CHECK(show.result.rgb(3, 0) == kRed);
}

TEST_CASE("RenderEffectForModel re-renders the edited time span", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    Effect* e = FirstEffect(*show.ctx, "A");
    e->SetPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
    // forget the dirty range the edit recorded so only the asked-for span renders
    ResetDirty(*show.ctx);
    show.ctx->RenderEffectForModel("A", 0, 400, false);
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    for (unsigned f = 0; f <= 8; ++f) CHECK(show.result.rgb(f, 0) == kBlue);
    for (unsigned f = 9; f < 20; ++f) CHECK(show.result.rgb(f, 0) == kRed);
    for (unsigned f = 0; f < 20; ++f) CHECK(show.result.rgb(f, 2) == kGreen);
}

TEST_CASE("RenderEffectForModel widens to the element's dirty range", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    ResetDirty(*show.ctx);
    // the edit marks the whole effect (0-1000ms) dirty on A
    FirstEffect(*show.ctx, "A")->SetPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
    show.ctx->RenderEffectForModel("A", 0, 100, false);
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    for (unsigned f = 0; f < 20; ++f) CHECK(show.result.rgb(f, 0) == kBlue);
    // the dirty range was consumed by the render
    int st, ed;
    show.ctx->_sequenceElements.GetElement("A")->GetDirtyRange(st, ed);
    CHECK(st == -1);
}

TEST_CASE("RenderEffectForModel ignores unknown models and suspended renders", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    Effect* e = FirstEffect(*show.ctx, "A");
    e->SetPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
    show.ctx->RenderEffectForModel("NoSuchModel", 0, 1000, false);
    auto* eng = show.ctx->_renderEngine.get();
    eng->RenderEffectForModel("A", 0, 1000, show.ctx->_sequenceElements, show.ctx->_seqData, true, show.ctx->modelsChangeCount);
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    CHECK(show.result.rgb(5, 0) == kRed);
}

TEST_CASE("RenderEffectForModel extends to a persistent effect at the edit end", "[render2][engine]") {
    ShowSpec s;
    s.effects.push_back(Eff("M", 0, "On", 0, 500, Pal("#FF0000")));
    s.effects.push_back(Eff("M", 0, "On", 500, 1000, Pal("#00FF00"), "B_CHECKBOX_OverlayBkg=1"));
    Show show(s);
    REQUIRE(show.result.rendered);
    CHECK(show.result.rgb(15, 0) == kGreen);
    FillAll(show.ctx->_seqData, 0);
    ResetDirty(*show.ctx);
    // the edit ends where the persistent effect starts, so its whole span is redone
    show.ctx->RenderEffectForModel("M", 0, 500, false);
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    CHECK(show.result.rgb(2, 0) == kRed);
    CHECK(show.result.rgb(19, 0) == kGreen);
}

TEST_CASE("RenderDirtyModels renders only the changed model and range", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto* eng = show.ctx->_renderEngine.get();
    ResetDirty(*show.ctx);

    SECTION("nothing dirty renders nothing") {
        FillAll(show.ctx->_seqData, 0x33);
        eng->RenderDirtyModels(show.ctx->_sequenceElements, show.ctx->_seqData, false, show.ctx->modelsChangeCount);
        REQUIRE(WaitDone(*show.ctx));
        show.Capture();
        CHECK(show.result.rgb(5, 0) == Rgb{ 0x33, 0x33, 0x33 });
    }
    SECTION("an edited effect is re-rendered") {
        FillAll(show.ctx->_seqData, 0x33);
        Effect* e = FirstEffect(*show.ctx, "A");
        e->SetPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
        int st, ed;
        show.ctx->_sequenceElements.GetElement("A")->GetDirtyRange(st, ed);
        CHECK(st == 0);
        CHECK(ed == 1000);
        eng->RenderDirtyModels(show.ctx->_sequenceElements, show.ctx->_seqData, false, show.ctx->modelsChangeCount);
        REQUIRE(WaitDone(*show.ctx));
        show.Capture();
        CHECK(show.result.rgb(5, 0) == kBlue);
        CHECK(show.result.rgb(5, 2) == Rgb{ 0x33, 0x33, 0x33 });
    }
    SECTION("suspended does nothing") {
        FillAll(show.ctx->_seqData, 0x33);
        FirstEffect(*show.ctx, "A")->SetPalette("C_BUTTON_Palette1=#0000FF,C_CHECKBOX_Palette1=1");
        eng->RenderDirtyModels(show.ctx->_sequenceElements, show.ctx->_seqData, true, show.ctx->modelsChangeCount);
        REQUIRE(WaitDone(*show.ctx));
        show.Capture();
        CHECK(show.result.rgb(5, 0) == Rgb{ 0x33, 0x33, 0x33 });
    }
}

TEST_CASE("ExportModelData copies one model's channels", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto* eng = show.ctx->_renderEngine.get();
    auto exp = eng->ExportModelData("B", show.ctx->_seqData);
    REQUIRE(exp.data != nullptr);
    CHECK(exp.chansPerNode == 3);
    CHECK(exp.data->NumFrames() == 20);
    CHECK(exp.data->NumChannels() == 6);
    for (unsigned f : { 0u, 19u }) {
        CHECK((*exp.data)[f][0] == 0);
        CHECK((*exp.data)[f][1] == 255);
        CHECK((*exp.data)[f][2] == 0);
        CHECK((*exp.data)[f][4] == 255);
    }
    CHECK(eng->ExportModelData("Missing", show.ctx->_seqData).data == nullptr);
}

TEST_CASE("Render tree lists every model and rebuilds on change", "[render2][engine]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.groups = R"(<modelGroup name="G" models="A,B"/>)";
    s.modelElements = { "A", "B", "G" };
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s);
    REQUIRE(show.result.rendered);
    auto* eng = show.ctx->_renderEngine.get();
    auto models = eng->GetRenderTree().GetModels();
    std::set<std::string> names;
    for (auto* m : models) names.insert(m->GetName());
    CHECK(names.count("A") == 1);
    CHECK(names.count("B") == 1);
    CHECK(names.count("G") == 1);
    unsigned before = eng->GetRenderTree().renderTreeChangeCount;
    eng->BuildRenderTree(show.ctx->_sequenceElements, show.ctx->modelsChangeCount + 1);
    CHECK(eng->GetRenderTree().renderTreeChangeCount == before + 1);
    CHECK(eng->GetRenderTree().renderTreeChangeCount != before);
    CHECK(eng->GetRenderTree().GetModels().size() == models.size());
}

TEST_CASE("Rendering a group's member alone gives the group's colour on its nodes", "[render2][engine]") {
    ShowSpec s;
    s.models = SingleLine("A", 1, 2) + SingleLine("B", 7, 2);
    s.groups = R"(<modelGroup name="G" models="A,B"/>)";
    s.modelElements = { "A", "B", "G" };
    s.effects.push_back(Eff("G", 0, "On", 0, 1000, Pal("#FF0000")));
    Show show(s);
    REQUIRE(show.result.rendered);
    FillAll(show.ctx->_seqData, 0);
    show.ctx->RenderEffectForModel("G", 0, 1000, false);
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    for (unsigned n = 0; n < 4; ++n) CHECK(show.result.rgb(10, n) == kRed);
}

TEST_CASE("Aborting a long render stops it and leaves the engine usable", "[render2][engine]") {
    ShowSpec small;
    small.models = Matrix("M", 1, 64, 64);
    small.durationMs = 100;
    small.effects.push_back(Eff("M", 0, "On", 0, 100, Pal("#FF0000")));
    Show show(small);
    REQUIRE(show.result.rendered);

    ShowSpec s = small;
    s.durationMs = 30000;
    s.effects.clear();
    s.effects.push_back(Eff("M", 0, "Plasma", 0, 30000, Pal("#FF0000", "#00FF00"), "B_SLIDER_Blur=8"));
    REQUIRE(show.ctx->OpenSequence(show.dir.Write("long.xsq", BuildXsq(s))));
    auto* eng = show.ctx->_renderEngine.get();
    REQUIRE(eng != nullptr);
    eng->BuildRenderTree(show.ctx->_sequenceElements, show.ctx->modelsChangeCount);
    eng->ResetAbortedRenderJobs();
    std::atomic<int> calls{ 0 };
    std::atomic<bool> aborted{ false };
    eng->Render(show.ctx->_sequenceElements, show.ctx->_seqData, eng->GetRenderTree().GetModels(), {}, 0,
                (int)show.ctx->_seqData.NumFrames() - 1, nullptr, true, [&](bool a) { aborted = a; ++calls; });
    // Wait for the pool to finish the setup and publish the jobs, so the abort
    // stops running jobs rather than a batch that has not started.
    int batches = 0, rows = 0;
    for (int i = 0; i < 2000 && rows == 0; ++i) {
        batches = 0;
        show.ctx->ForEachRenderProgress([&](RenderProgressInfo* rpi) {
            ++batches;
            rows = rpi->numRows.load();
        });
        if (rows == 0) std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
    CHECK(batches == 1);
    REQUIRE(rows == 1);
    CHECK_FALSE(eng->IsRenderDone());
    eng->LogUnfinishedRenderJobs("test");
    eng->CheckForStalledRender();
    CHECK(show.ctx->AbortRender(30000));
    CHECK(show.ctx->IsRenderDone());
    CHECK(calls == 1);
    CHECK(aborted);
    CHECK(eng->GetAbortedRenderJobs() > 0);
    // the last frame was cleared by the setup and never reached
    CHECK(show.ctx->_seqData[(int)show.ctx->_seqData.NumFrames() - 1][0] == 0);

    // still renders afterwards
    eng->Render(show.ctx->_sequenceElements, show.ctx->_seqData, eng->GetRenderTree().GetModels(), {}, 0, 1, nullptr, true, [](bool) {});
    REQUIRE(WaitDone(*show.ctx));
    show.Capture();
    bool lit = false;
    for (unsigned n = 0; n < 64 * 64; ++n) {
        auto c = show.result.rgb(0, n);
        if (c.r || c.g || c.b) lit = true;
    }
    CHECK(lit);
}

TEST_CASE("Show context resolves show and media relative paths", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.loaded);
    auto& ctx = *show.ctx;
    const std::string dir = ctx.showDirectory;
    const std::string sep = dir.substr(dir.size() - 1) == "/" ? "" : "/";
    CHECK(ctx.IsInShowFolder(dir + sep + "images/a.png"));
    CHECK_FALSE(ctx.IsInShowFolder("/elsewhere/a.png"));
    ctx.mediaDirectories = { "/media/one", "/media/two" };
    CHECK(ctx.IsInShowOrMediaFolder("/media/two/b.png"));
    CHECK(ctx.IsInShowOrMediaFolder(dir + sep + "c.png"));
    CHECK_FALSE(ctx.IsInShowOrMediaFolder("/media/three/b.png"));
    CHECK(ctx.MakeRelativePath(dir + "/sub/x.png") == "sub/x.png");
    CHECK(ctx.MakeRelativePath("/other/x.png") == "/other/x.png");
    CHECK(ctx.MakeRelativePath(dir) == dir);
    // nothing in flight
    int batches = 0;
    ctx.ForEachRenderProgress([&](RenderProgressInfo*) { ++batches; });
    CHECK(batches == 0);
    ctx.ForEachRenderProgress(nullptr);
}

TEST_CASE("SignalAbort on an idle engine is harmless", "[render2][engine]") {
    Show show(TwoModels());
    REQUIRE(show.result.rendered);
    auto* eng = show.ctx->_renderEngine.get();
    eng->SignalAbort();
    eng->CheckForStalledRender();
    eng->LogUnfinishedRenderJobs("test");
    CHECK(eng->IsRenderDone());
    CHECK(RenderEngine::RecommendedPoolSize() >= 1);
}
