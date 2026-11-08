#include <catch2/catch_test_macros.hpp>

#include <list>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/FSEQFile.h"
#include "render/FSEQFileIO.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceData.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"

#include <pugixml.hpp>

#include <cstring>
#include <memory>
#include <string>
#include <vector>

namespace {
using Ranges = std::vector<std::pair<uint32_t, uint32_t>>;

struct IOFixture {
    HeadlessRenderContext ctx;
    SequenceElements& se;
    IOFixture() : se(ctx.GetSequenceElements()) {
        se.SetViewsManager(&ctx._sequenceViewManager);
    }
    Model* AddModel(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = ctx.AllModels.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }
    Model* Line(const std::string& name, int nodes, int startChannel) {
        return AddModel("<model name=\"" + name + "\" DisplayAs=\"Single Line\" NumStrings=\"1\" NodesPerString=\"" + std::to_string(nodes) +
                        "\" StringType=\"RGB Nodes\" StartChannel=\"" + std::to_string(startChannel) + "\"/>");
    }
    void Row(const std::string& name) {
        se.AddElement(name, "model", true, false, true, false, false)->AddEffectLayer();
    }
};

const FSEQFile::VariableHeader* Find(const FSEQFile& f, char a, char b) {
    for (const auto& h : f.getVariableHeaders()) {
        if (h.code[0] == a && h.code[1] == b) return &h;
    }
    return nullptr;
}
} // namespace

TEST_CASE("FSEQFileIO sparse ranges merge adjacent models and expand groups", "[seq2][fseqio]") {
    IOFixture f;
    f.Line("A", 10, 1);   // channels 0..29
    f.Line("C", 10, 31);  // channels 30..59, touching A
    f.Line("B", 5, 101);  // channels 100..114
    f.Line("D", 4, 201);  // channels 200..211, only reached through the group
    f.AddModel("<modelGroup name=\"G\" models=\"B,D\"/>");
    f.ctx.AllModels.RecalcStartChannels();
    f.Row("A");
    f.Row("C");
    f.Row("G");
    f.se.AddElement("Beats", "timing", true, false, true, false, false);
    f.Row("NotInLayout");

    auto r = FSEQFileIO::ComputeSparseRanges(f.se, f.ctx);
    CHECK(r == Ranges{ { 0, 60 }, { 100, 15 }, { 200, 12 } });
}

TEST_CASE("FSEQFileIO overlapping models collapse into one range", "[seq2][fseqio]") {
    IOFixture f;
    f.Line("Big", 20, 1);   // 0..59
    f.Line("Inner", 5, 10); // 9..23, inside Big
    f.Line("Same", 2, 1);   // same start as Big but shorter
    f.ctx.AllModels.RecalcStartChannels();
    f.Row("Inner");
    f.Row("Same");
    f.Row("Big");
    CHECK(FSEQFileIO::ComputeSparseRanges(f.se, f.ctx) == Ranges{ { 0, 60 } });

    IOFixture empty;
    CHECK(FSEQFileIO::ComputeSparseRanges(empty.se, empty.ctx).empty());
}

TEST_CASE("FSEQFileIO Write stores sparse ranges and FPP command headers", "[seq2][fseqio]") {
    TempDir td;
    IOFixture f;
    f.Line("A", 4, 1);   // 0..11
    f.Line("B", 2, 21);  // 20..25
    f.ctx.AllModels.RecalcStartChannels();
    f.Row("A");
    f.Row("B");

    auto* cmds = dynamic_cast<TimingElement*>(f.se.AddElement("Cmds", "timing", true, false, true, false, false));
    cmds->SetSubType("FPP Commands");
    auto* cl = cmds->AddEffectLayer();
    cl->AddEffect(0, "Lights On", "", "", 100, 200, 0, false);
    cl->AddEffect(0, "Lights On", "", "", 400, 500, 0, false);
    cl->AddEffect(0, "Fog", "", "", 600, 700, 0, false);
    cl->AddEffect(0, "", "", "", 800, 900, 0, false);

    auto* fx = dynamic_cast<TimingElement*>(f.se.AddElement("Fx", "timing", true, false, true, false, false));
    fx->SetSubType("FPP Effects");
    fx->AddEffectLayer()->AddEffect(0, "Sparkle", "", "", 0, 50, 0, false);

    auto* blank = dynamic_cast<TimingElement*>(f.se.AddElement("Blank", "timing", true, false, true, false, false));
    blank->SetSubType("FPP Commands");
    blank->AddEffectLayer()->AddEffect(0, "", "", "", 0, 50, 0, false);

    SequenceData sd;
    sd.init(30, 20, 50);
    for (unsigned fr = 0; fr < 20; ++fr)
        for (unsigned c = 0; c < 30; ++c) sd[fr][c] = (uint8_t)(fr + c + 1);

    auto p = td.File("io.fseq");
    REQUIRE(FSEQFileIO::Write(p, sd, &f.se, &f.ctx));
    std::unique_ptr<FSEQFile> file(FSEQFile::openFSEQFile(p));
    REQUIRE(file);
    auto* v2 = dynamic_cast<V2FSEQFile*>(file.get());
    REQUIRE(v2);
    CHECK(v2->m_sparseRanges == Ranges{ { 0, 12 }, { 20, 6 } });

    // the data written covers only the model channels
    std::unique_ptr<FSEQFile::FrameData> fd(file->getFrame(3));
    REQUIRE(fd);
    std::vector<uint8_t> buf(32, 0);
    REQUIRE(fd->readFrame(buf.data(), 32));
    CHECK(buf[0] == 4);
    CHECK(buf[11] == 15);
    CHECK(buf[12] == 0);
    CHECK(buf[20] == 24);

    const auto* fc = Find(*file, 'F', 'C');
    REQUIRE(fc != nullptr);
    const auto& d = fc->data;
    REQUIRE(d.size() >= 6);
    CHECK(d[0] == 1);
    uint32_t count = 0;
    std::memcpy(&count, &d[1], 4);
    CHECK(count == 2); // "Fog" and "Lights On"; the unlabelled mark is dropped
    CHECK(d[5] == 0);  // empty instance list
    // first command (map order) is "Fog" with one range, frames 12..14
    CHECK(std::string(reinterpret_cast<const char*>(&d[6])) == "Fog");
    size_t pos = 6 + 4;
    uint32_t n = 0, s = 0, e = 0;
    std::memcpy(&n, &d[pos], 4);
    std::memcpy(&s, &d[pos + 4], 4);
    std::memcpy(&e, &d[pos + 8], 4);
    CHECK(n == 1);
    CHECK(s == 12);
    CHECK(e == 14);
    pos += 12;
    CHECK(std::string(reinterpret_cast<const char*>(&d[pos])) == "Lights On");
    pos += 10;
    std::memcpy(&n, &d[pos], 4);
    CHECK(n == 2);
    CHECK(d.size() == pos + 4 + 2 * 8);

    const auto* fe = Find(*file, 'F', 'E');
    REQUIRE(fe != nullptr);
    std::memcpy(&count, &fe->data[1], 4);
    CHECK(count == 1);

    int fcCount = 0;
    for (const auto& h : file->getVariableHeaders()) {
        if (h.code[0] == 'F' && h.code[1] == 'C') ++fcCount;
    }
    CHECK(fcCount == 1); // the all-blank command track writes nothing
}

TEST_CASE("FSEQFileIO Write without sparse or for v1 keeps every channel", "[seq2][fseqio]") {
    TempDir td;
    IOFixture f;
    f.Line("A", 2, 1);
    f.ctx.AllModels.RecalcStartChannels();
    f.Row("A");
    SequenceData sd;
    sd.init(16, 3, 50);
    sd[1][15] = 77;

    FSEQFileIO::WriteOptions o;
    o.sparse = false;
    auto p = td.File("dense.fseq");
    REQUIRE(FSEQFileIO::Write(p, sd, &f.se, &f.ctx, o));
    std::unique_ptr<FSEQFile> a(FSEQFile::openFSEQFile(p));
    REQUIRE(a);
    CHECK(dynamic_cast<V2FSEQFile*>(a.get())->m_sparseRanges.empty());
    std::unique_ptr<FSEQFile::FrameData> fd(a->getFrame(1));
    std::vector<uint8_t> buf(16, 0);
    REQUIRE(fd->readFrame(buf.data(), 16));
    CHECK(buf[15] == 77);

    o.sparse = true;
    o.version = 1;
    o.compression = FSEQFile::CompressionType::none;
    auto q = td.File("v1.fseq");
    REQUIRE(FSEQFileIO::Write(q, sd, &f.se, &f.ctx, o));
    std::unique_ptr<FSEQFile> b(FSEQFile::openFSEQFile(q));
    REQUIRE(b);
    CHECK(b->getVersionMajor() == 1);
    CHECK(b->getChannelCount() == 16);
}
