#include <catch2/catch_test_macros.hpp>

#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "render/SequenceFile.h"
#include "support/TempDir.h"
#include "utils/ZipUtils.h"

#include <minizip/zip.h>

#include <string>
#include <tuple>
#include <vector>

namespace {

struct LspCtx : HeadlessRenderContext {
    LspCtx() { GetSequenceElements().SetViewsManager(&_sequenceViewManager); }
    TimingElement* AddTimingElement(const std::string& name, const std::string& = "") override {
        auto* e = dynamic_cast<TimingElement*>(GetSequenceElements().AddElement(name, "timing", true, false, true, false, false));
        if (e != nullptr && e->GetEffectLayerCount() == 0) {
            e->AddEffectLayer();
        }
        return e;
    }
};

bool BuildZip(const std::string& path, const std::string& entry, const std::string& body) {
    zipFile zf = zipOpen(path.c_str(), APPEND_STATUS_CREATE);
    if (zf == nullptr) {
        return false;
    }
    zip_fileinfo zi{};
    zipOpenNewFileInZip(zf, entry.c_str(), &zi, nullptr, 0, nullptr, 0, nullptr, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    zipWriteInFileInZip(zf, body.data(), static_cast<unsigned>(body.size()));
    zipCloseFileInZip(zf);
    zipClose(zf, nullptr);
    return true;
}

std::vector<std::tuple<int, int>> Marks(LspCtx& c, const std::string& name) {
    std::vector<std::tuple<int, int>> out;
    Element* e = c.GetSequenceElements().GetElement(name);
    if (e == nullptr) {
        return out;
    }
    auto* l = e->GetEffectLayer(0);
    for (int i = 0; i < l->GetEffectCount(); ++i) {
        out.emplace_back(l->GetEffect(i)->GetStartTimeMS(), l->GetEffect(i)->GetEndTimeMS());
    }
    return out;
}

std::string Interval(int eff, int att, int pos) {
    return "<TimeInterval eff=\"" + std::to_string(eff) + "\" att=\"" + std::to_string(att) + "\" pos=\"" + std::to_string(pos) + "\"/>";
}

}

TEST_CASE("LSP timing import reads colour coded intervals from the Sequence entry", "[impexp][lsp]") {
    TempDir d;
    LspCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto body = "<MusicalSequence><TimingTracks><Track><Intervals>" + Interval(7, 1, 44100) + Interval(7, 1, 88200) + Interval(7, 2, 44100) + "</Intervals></Track></TimingTracks></MusicalSequence>";
    REQUIRE(BuildZip(d.File("song.lsp"), "Sequence", body));
    sf.ProcessLSPTiming({ d.File("song.lsp") }, &c);
    CHECK(Marks(c, "x-song") == std::vector<std::tuple<int, int>>{ { 0, 500 }, { 500, 1000 } });
    CHECK(Marks(c, "R-song") == std::vector<std::tuple<int, int>>{ { 0, 500 } });
}

TEST_CASE("LSP timing import ignores archives without usable content", "[impexp][lsp]") {
    TempDir d;
    LspCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    REQUIRE(BuildZip(d.File("a.lsp"), "Other", "x"));
    REQUIRE(BuildZip(d.File("b.lsp"), "Sequence", "<NotMusical/>"));
    REQUIRE(BuildZip(d.File("c.lsp"), "Sequence", "<MusicalSequence><TimingTracks><Track/></TimingTracks></MusicalSequence>"));
    REQUIRE(BuildZip(d.File("d.lsp"), "Sequence", "<<<< broken"));
    sf.ProcessLSPTiming({ d.File("a.lsp"), d.File("b.lsp"), d.File("c.lsp"), d.File("d.lsp") }, &c);
    CHECK(c.GetSequenceElements().GetElementCount() == 0);
}

TEST_CASE("LSP timing import with huge positions does not crash", "[impexp][lsp]") {
    TempDir d;
    LspCtx c;
    SequenceFile sf("x.xsq");
    sf.SetSequenceLoaded(true);
    auto body = "<MusicalSequence><TimingTracks><Track><Intervals>" + Interval(7, 1, 2147483647) + Interval(7, 1, -5) + Interval(7, 99999, 10) + "</Intervals></Track></TimingTracks></MusicalSequence>";
    REQUIRE(BuildZip(d.File("big.lsp"), "Sequence", body));
    sf.ProcessLSPTiming({ d.File("big.lsp") }, &c);
    SUCCEED();
}

TEST_CASE("ZipUtils lists the entry written by a deflating writer", "[impexp][lsp]") {
    TempDir d;
    REQUIRE(BuildZip(d.File("z.zip"), "Sequence", std::string(5000, 'q')));
    CHECK(ZipUtils::ListEntries(d.File("z.zip")) == std::vector<std::string>{ "Sequence" });
    auto e = ZipUtils::ReadEntry(d.File("z.zip"), "Sequence");
    REQUIRE(e.has_value());
    CHECK(*e == std::string(5000, 'q'));
}
