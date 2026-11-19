#include <catch2/catch_test_macros.hpp>

#include "test_collab_common.h"

#include "collab/CollabCommandInfo.h"
#include "collab/CollabDocHash.h"
#include "collab/CollabMessages.h"
#include "collab/CollabTransport.h"
#include "collab/CollabUidTable.h"
#include "render/SequenceCommands.h"

#include <random>

using namespace Collab;
using namespace CollabTest;

namespace {

std::vector<Message> SampleMessages() {
    std::vector<Message> out;
    Message m;
    m.type = MsgType::Hello;
    m.appVersion = "2026.16";
    m.name = "Desk \xE2\x80\x94 Mac";
    m.deviceKind = "mac";
    out.push_back(m);
    m = {};
    m.type = MsgType::Welcome;
    m.participant = 7;
    m.rev = 12;
    m.name = "Host";
    out.push_back(m);
    m = {};
    m.type = MsgType::Refuse;
    m.reason = Reason::Full;
    m.text = "full";
    out.push_back(m);
    m = {};
    m.type = MsgType::Snapshot;
    m.rev = 3;
    m.docHash = 0xFEDCBA9876543210ull;
    m.clientSeq = 4;
    m.blob = 77;
    m.uids = nlohmann::json::array({ 1, 2 });
    out.push_back(m);
    m = {};
    m.type = MsgType::Manifest;
    m.files = { { "xlights_rgbeffects.xml", FileRole::Layout, 1234, 0xFEDCBA9876543210ull },
                { "Audio/song \xE2\x80\x94 mix.mp3", FileRole::Audio, 99, 1 },
                { "/Users/someone/Pictures/a.png", FileRole::Media, 0, 0xFFFFFFFFFFFFFFFFull } };
    out.push_back(m);
    m = {};
    m.type = MsgType::Manifest; // an empty show
    out.push_back(m);
    m = {};
    m.type = MsgType::NeedFiles;
    m.paths = { "Images/a.png", "xlights_networks.xml" };
    out.push_back(m);
    m = {};
    m.type = MsgType::File;
    m.blob = 5;
    m.files = { { "Images/a.png", FileRole::Media, 10, 20 } };
    out.push_back(m);
    m = {};
    m.type = MsgType::File;
    m.files = { { "Images/b.png", FileRole::Media, 0, 0 } };
    m.reason = Reason::NotInManifest;
    out.push_back(m);
    m = {};
    m.type = MsgType::NeedSnapshot;
    m.reason = "drift";
    out.push_back(m);
    m = {};
    m.type = MsgType::Propose;
    m.clientSeq = 9;
    m.baseRev = 8;
    m.kind = EditKind::Undo;
    m.undoOf = 5;
    m.command = nlohmann::json::parse(MoveEffectCommand({ 1, 2, 3 }, 100, 200).ToJson());
    out.push_back(m);
    m = {};
    m.type = MsgType::Commit;
    m.rev = 10;
    m.participant = 3;
    m.clientSeq = 9;
    m.kind = EditKind::Redo;
    m.undoOf = 6;
    m.command = nlohmann::json::parse(MoveEffectCommand({ 1, 2, 3 }, 100, 200).ToJson());
    m.docHash = 0xFFFFFFFFFFFFFFFFull;
    out.push_back(m);
    m = {};
    m.type = MsgType::Reject;
    m.clientSeq = 9;
    m.rev = 11;
    m.kind = EditKind::Undo;
    m.reason = Reason::ChangedSince;
    m.participant = 2;
    out.push_back(m);
    m = {};
    m.type = MsgType::Resend;
    m.rev = 44;
    out.push_back(m);
    m = {};
    m.type = MsgType::Presence;
    m.participant = 4;
    m.presence.startMS = 1000;
    m.presence.endMS = 5000;
    m.presence.focusedElements = { 1, 2 };
    m.presence.selectedEffects = { 3 };
    m.presence.softLock = 3;
    out.push_back(m);
    m = {};
    m.type = MsgType::Ping;
    m.rev = 4;
    m.time = 99;
    out.push_back(m);
    m = {};
    m.type = MsgType::Pong;
    m.rev = 4;
    m.time = 99;
    out.push_back(m);
    m = {};
    m.type = MsgType::Bye;
    m.reason = Reason::Ended;
    out.push_back(m);
    return out;
}

} // namespace

TEST_CASE("Every collab message round-trips through its frame", "[collab][collab_proto]") {
    for (const Message& m : SampleMessages()) {
        INFO(MsgTypeName(m.type));
        std::string frame = EncodeMessage(m);
        auto back = DecodeMessage(frame);
        REQUIRE(back.has_value());
        CHECK(EncodeMessage(*back) == frame);
        CHECK(back->type == m.type);
        CHECK(back->protocol == kProtocolVersion);
        CHECK(back->files == m.files);
        CHECK(back->paths == m.paths);
        CHECK(back->blob == m.blob);
    }
}

TEST_CASE("Malformed collab frames decode to nothing", "[collab][collab_proto]") {
    CHECK_FALSE(DecodeMessage(""));
    CHECK_FALSE(DecodeMessage("not json"));
    CHECK_FALSE(DecodeMessage("[]"));
    CHECK_FALSE(DecodeMessage("{\"v\":1}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Nope\"}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Commit\",\"rev\":\"x\",\"cmd\":{}}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Commit\",\"rev\":0,\"cmd\":{}}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Propose\",\"seq\":-1,\"cmd\":{}}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Propose\",\"seq\":1}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Welcome\",\"participant\":70000}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Presence\",\"participant\":1,\"presence\":{\"sel\":[\"a\"]}}"));
    // Too deeply nested, even inside an otherwise fine frame.
    std::string deep = "{\"v\":1,\"t\":\"Propose\",\"seq\":1,\"cmd\":{\"x\":" + std::string(200, '[') + std::string(200, ']') + "}}";
    CHECK_FALSE(DecodeMessage(deep));
    // Brackets inside strings don't count as nesting.
    std::string brackets = "{\"v\":1,\"t\":\"Bye\",\"reason\":\"" + std::string(200, '[') + "\"}";
    CHECK(DecodeMessage(brackets).has_value());
    // A different protocol version only gets through as a Hello / Refuse / Bye.
    CHECK(DecodeMessage("{\"v\":2,\"t\":\"Hello\",\"name\":\"x\"}").has_value());
    CHECK(DecodeMessage("{\"v\":2,\"t\":\"Hello\"}")->protocol == 2);
    CHECK_FALSE(DecodeMessage("{\"v\":2,\"t\":\"Commit\",\"rev\":1,\"cmd\":{}}"));
    // Snapshots come as blobs; one without a blob, or with the old inline
    // document, is malformed.
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Snapshot\",\"rev\":1,\"uids\":[]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Snapshot\",\"rev\":1,\"xsq\":\"<x/>\",\"uids\":[]}"));
    CHECK(DecodeMessage("{\"v\":1,\"t\":\"Snapshot\",\"rev\":1,\"blob\":3,\"uids\":[]}").has_value());
    // Manifest / NeedFiles / File.
    const std::string file = "{\"path\":\"a.png\",\"role\":\"media\",\"size\":1,\"hash\":2}";
    CHECK(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[" + file + "]}").has_value());
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\"}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":{}}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[1]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[{\"path\":\"a\",\"role\":\"video\",\"size\":1,\"hash\":2}]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[{\"path\":\"\",\"role\":\"media\",\"size\":1,\"hash\":2}]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[{\"path\":\"a\",\"role\":\"media\",\"size\":-1,\"hash\":2}]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[{\"path\":\"" + std::string(kMaxPathBytes + 1, 'a') + "\",\"role\":\"media\",\"size\":1,\"hash\":2}]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"Manifest\",\"files\":[{\"path\":\"a\\u0000b\",\"role\":\"media\",\"size\":1,\"hash\":2}]}"));
    CHECK(DecodeMessage("{\"v\":1,\"t\":\"NeedFiles\",\"paths\":[\"a\"]}").has_value());
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"NeedFiles\"}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"NeedFiles\",\"paths\":[3]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"NeedFiles\",\"paths\":[\"\"]}"));
    CHECK(DecodeMessage("{\"v\":1,\"t\":\"File\",\"blob\":4,\"files\":[" + file + "]}").has_value());
    CHECK(DecodeMessage("{\"v\":1,\"t\":\"File\",\"reason\":\"unreadable\",\"files\":[" + file + "]}").has_value());
    // A File names exactly one file, and has a blob or a reason, not both / neither.
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"File\",\"blob\":4,\"files\":[]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"File\",\"blob\":4,\"files\":[" + file + "," + file + "]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"File\",\"files\":[" + file + "]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":1,\"t\":\"File\",\"blob\":4,\"reason\":\"x\",\"files\":[" + file + "]}"));
    // Oversized lists.
    std::string many = "{\"v\":1,\"t\":\"NeedFiles\",\"paths\":[";
    for (size_t i = 0; i <= kMaxManifestFiles; ++i) {
        many += (i ? ",\"a\"" : "\"a\"");
    }
    CHECK_FALSE(DecodeMessage(many + "]}"));
}

TEST_CASE("Invalid UTF-8 in a message is replaced, not thrown", "[collab][collab_proto]") {
    Message m;
    m.type = MsgType::Hello;
    m.name = std::string("bad \xC3\x28 name");
    std::string frame;
    REQUIRE_NOTHROW(frame = EncodeMessage(m));
    auto back = DecodeMessage(frame);
    REQUIRE(back.has_value());
    CHECK(back->name != m.name);
}

TEST_CASE("The frame decoder survives random and mutated input", "[collab][collab_proto]") {
    std::mt19937 rng(1234);
    std::vector<std::string> frames;
    for (const Message& m : SampleMessages()) {
        frames.push_back(EncodeMessage(m));
    }
    size_t decoded = 0;
    for (int i = 0; i < 20000; ++i) {
        std::string f = frames[rng() % frames.size()];
        int edits = 1 + rng() % 6;
        for (int e = 0; e < edits && !f.empty(); ++e) {
            size_t pos = rng() % f.size();
            switch (rng() % 4) {
            case 0: f[pos] = (char)(rng() % 256); break;
            case 1: f.erase(pos, 1 + rng() % 8); break;
            case 2: f.insert(pos, 1, "{}[]\":,0-e\\"[rng() % 11]); break;
            default: f = f.substr(0, pos); break;
            }
        }
        std::optional<Message> m;
        REQUIRE_NOTHROW(m = DecodeMessage(f));
        if (m) {
            ++decoded;
            REQUIRE_NOTHROW(EncodeMessage(*m));
            REQUIRE_NOTHROW(DeserializeCommand(m->command));
        }
    }
    for (int i = 0; i < 5000; ++i) {
        std::string f(rng() % 64, ' ');
        for (char& c : f) {
            c = (char)(rng() % 256);
        }
        REQUIRE_NOTHROW(DecodeMessage(f));
    }
    // Some mutations still decode; that's what lets the host fuzz below reach
    // command validation.
    CHECK(decoded > 0);
}

TEST_CASE("Loopback transport delivers per link in order unless told not to", "[collab][collab_proto]") {
    for (bool fifo : { true, false }) {
        LoopbackOptions o;
        o.seed = 5;
        o.minDelayMs = 0;
        o.maxDelayMs = 50;
        o.fifoPerLink = fifo;
        LoopbackNetwork net(o);
        auto& a = net.AddPeer();
        auto& b = net.AddPeer();
        std::vector<uint64_t> got;
        b.onMessage = [&](PeerId from, const Message& m) {
            CHECK(from == a.LocalPeer());
            got.push_back(m.rev);
        };
        for (uint64_t i = 1; i <= 200; ++i) {
            Message m;
            m.type = MsgType::Ping;
            m.rev = i;
            a.Send(b.LocalPeer(), m);
        }
        REQUIRE(net.RunUntilIdle());
        REQUIRE(got.size() == 200);
        CHECK(std::is_sorted(got.begin(), got.end()) == fifo);
    }
}

TEST_CASE("Loopback transport drops, duplicates and reports lost peers", "[collab][collab_proto]") {
    LoopbackOptions o;
    o.seed = 9;
    o.dropRate = 0.3;
    o.duplicateRate = 0.3;
    LoopbackNetwork net(o);
    auto& a = net.AddPeer();
    auto& b = net.AddPeer();
    auto& c = net.AddPeer();
    size_t got = 0;
    std::vector<std::pair<PeerId, PeerEvent>> events;
    b.onMessage = [&](PeerId, const Message&) { ++got; };
    c.onPeer = [&](PeerId p, PeerEvent e) { events.push_back({ p, e }); };
    for (int i = 0; i < 1000; ++i) {
        Message m;
        m.type = MsgType::Ping;
        a.Send(b.LocalPeer(), m);
    }
    REQUIRE(net.RunUntilIdle());
    CHECK(net.Dropped() > 200);
    CHECK(got > 700);
    CHECK(got != 1000);
    events.clear();
    net.Disconnect(a.LocalPeer());
    Message m;
    m.type = MsgType::Ping;
    a.Broadcast(m);
    REQUIRE(net.RunUntilIdle());
    REQUIRE(events.size() == 1);
    CHECK(events[0].first == a.LocalPeer());
    CHECK(events[0].second == PeerEvent::Lost);
}

TEST_CASE("Command helpers: optimistic edits, key diffs and redundant deletes", "[collab][collab_proto]") {
    Show show;
    Doc doc(show.path, show.dir.File("d"));
    doc.Open(show.start);
    SequenceElements& seq = doc.Sequence();
    EffectLayer* grid = seq.GetElement("Grid")->GetEffectLayer(0);
    Effect* on = grid->GetEffect(0);
    EffectLocator loc = LocatorOf(on);

    ModifyEffectCommand keys(loc, std::nullopt, { false, { { "E_X", std::string("1") } } }, {});
    ModifyEffectCommand type(loc, std::string("Bars"), SettingsChange::ReplaceWith(""), {});
    ModifyEffectCommand all(loc, std::nullopt, SettingsChange::ReplaceWith("E_TEXTCTRL_Eff_On_Start=10,E_NEW=2"), {});
    CHECK(IsOptimistic(keys));
    CHECK_FALSE(IsOptimistic(type));
    CHECK_FALSE(IsOptimistic(all));
    CHECK_FALSE(IsOptimistic(MoveEffectCommand(loc, 0, 10)));

    auto diff = AsKeyChanges(seq, all);
    REQUIRE(diff != nullptr);
    REQUIRE(IsOptimistic(*diff));
    const auto& d = static_cast<const ModifyEffectCommand&>(*diff);
    REQUIRE(d.GetSettings().keys.size() == 1);
    CHECK(d.GetSettings().keys.at("E_NEW") == std::optional<std::string>("2"));
    CHECK(AsKeyChanges(seq, type) == nullptr);

    // Deleting something that is gone is dropped; deleting something the
    // same batch creates is kept.
    EffectLocator gone{ loc.element, loc.layer, 0xABCDEF };
    auto pruned = WithoutRedundantDeletes(seq, DeleteEffectCommand(gone));
    REQUIRE(pruned != nullptr);
    CHECK(IsNoOp(*pruned));
    CHECK(WithoutRedundantDeletes(seq, DeleteEffectCommand(loc)) == nullptr);
    BatchCommand b("x");
    EffectState s;
    s.name = "On";
    s.startTimeMS = 5000;
    s.endTimeMS = 5100;
    auto add = std::make_unique<AddEffectCommand>(LayerLocator{ loc.element, loc.layer }, s);
    EffectLocator added = add->Target();
    b.Add(std::move(add));
    b.Add(std::make_unique<DeleteEffectCommand>(added));
    b.Add(std::make_unique<DeleteEffectCommand>(gone));
    auto pb = WithoutRedundantDeletes(seq, b);
    REQUIRE(pb != nullptr);
    CHECK(static_cast<const BatchCommand&>(*pb).GetCommands().size() == 2);

    // A real type change is a footprint "type" item; a same-type ModifyEffect isn't.
    auto inv = type.Clone()->Apply(seq, ApplyMode::Strict);
    REQUIRE(inv != nullptr);
    CHECK(TouchedItems(seq, type, *inv).count(TypeItem(loc.effect)) == 1);
    auto inv2 = keys.Clone()->Apply(seq, ApplyMode::Strict);
    REQUIRE(inv2 != nullptr);
    CHECK(TouchedItems(seq, keys, *inv2).count(TypeItem(loc.effect)) == 0);
}

TEST_CASE("Snapshot ids put on a fresh load reproduce the document hash", "[collab][collab_proto]") {
    Show show;
    Doc a(show.path, show.dir.File("a"));
    Doc b(show.path, show.dir.File("b"));
    a.Open(show.start);
    b.Open(show.start);
    // Ids differ between two loads; the hash includes them.
    CHECK(DocHasher::FullHash(a.Sequence()) != DocHasher::FullHash(b.Sequence()));
    REQUIRE(ApplyUids(b.Sequence(), CaptureUids(a.Sequence())));
    CHECK(DocHasher::FullHash(a.Sequence()) == DocHasher::FullHash(b.Sequence()));
    CHECK(DescribeSequence(a.Sequence()) == DescribeSequence(b.Sequence()));

    // Effect state a reload wouldn't reproduce (here a timing mark's
    // settings, which the .xsq doesn't keep) comes back from the table.
    Effect* mark = a.Sequence().GetElement("Beats")->GetEffectLayer(0)->GetEffect(2);
    REQUIRE(ModifyEffectCommand(LocatorOf(mark), std::nullopt, { false, { { "E_X", std::string("1") } } }, {}).Apply(a.Sequence()) != nullptr);
    std::string xsq;
    REQUIRE(a.SaveXsq(xsq));
    Doc c(show.path, show.dir.File("c"));
    REQUIRE(c.LoadXsq(xsq));
    CHECK(DescribeSequence(c.Sequence(), false) != DescribeSequence(a.Sequence(), false));
    REQUIRE(ApplyUids(c.Sequence(), CaptureUids(a.Sequence())));
    CHECK(DescribeSequence(c.Sequence()) == DescribeSequence(a.Sequence()));
    CHECK(DocHasher::FullHash(c.Sequence()) == DocHasher::FullHash(a.Sequence()));

    // Moves don't re-sort a layer and the .xsq is written in the layer's
    // order (with position ids); the table puts a fresh load in the same
    // order, so both save the same bytes.
    EffectLayer* gl = a.Sequence().GetElement("Grid")->GetEffectLayer(0);
    REQUIRE(MoveEffectCommand(LocatorOf(gl->GetEffect(0)), 3000, 3200).Apply(a.Sequence()) != nullptr);
    REQUIRE(a.SaveXsq(xsq));
    Doc d(show.path, show.dir.File("d"));
    REQUIRE(d.LoadXsq(xsq));
    CHECK(d.SaveAs(show.dir.File("d1.xsq")) != a.SaveAs(show.dir.File("a1.xsq")));
    REQUIRE(ApplyUids(d.Sequence(), CaptureUids(a.Sequence())));
    CHECK(d.SaveAs(show.dir.File("d2.xsq")) == a.SaveAs(show.dir.File("a2.xsq")));

    // A table from a document with different rows / layers is refused.
    REQUIRE(AddLayerCommand(a.Sequence().GetElement("Line")->GetUid(), kNoSeqObjectId, kNoSeqObjectId).Apply(a.Sequence()) != nullptr);
    std::string problem;
    CHECK_FALSE(ApplyUids(b.Sequence(), CaptureUids(a.Sequence()), &problem));
    CHECK_FALSE(problem.empty());
}

TEST_CASE("The incremental document hash tracks every command it is told about", "[collab][collab_proto]") {
    Show show;
    Doc doc(show.path, show.dir.File("d"));
    doc.Open(show.start);
    SequenceElements& seq = doc.Sequence();
    DocHasher h;
    uint64_t before = h.Hash(seq);
    EffectLayer* layer = seq.GetElement("Grid")->GetEffectLayer(0);
    SetLayerNameCommand rename(LocatorOf(layer), "Renamed");
    REQUIRE(rename.Clone()->Apply(seq) != nullptr);

    // Control: a hasher that isn't told about the rename keeps the stale
    // value (a layer name change doesn't move the row's change count).
    DocHasher stale;
    stale.Hash(seq);
    SetLayerNameCommand back(LocatorOf(layer), "Again");
    REQUIRE(back.Clone()->Apply(seq) != nullptr);
    stale.invalidateEnabled = false;
    stale.Invalidate(seq, back);
    CHECK(stale.Hash(seq) != DocHasher::FullHash(seq));

    h.Invalidate(seq, rename);
    h.Invalidate(seq, back);
    CHECK(h.Hash(seq) == DocHasher::FullHash(seq));
    CHECK(h.Hash(seq) != before);

    // Local state isn't hashed.
    uint64_t now = h.Hash(seq);
    layer->GetEffect(0)->SetSelected(1);
    seq.GetElement("Grid")->SetCollapsed(true);
    seq.GetElement("Grid")->SetVisible(false);
    CHECK(DocHasher::FullHash(seq) == now);
}

TEST_CASE("EffectLayer::ReorderEffects takes only a permutation of the layer's effects", "[collab][collab_proto]") {
    Show show;
    Doc doc(show.path, show.dir.File("d"));
    doc.Open(show.start);
    EffectLayer* l = doc.Sequence().GetElement("Grid")->GetEffectLayer(0);
    REQUIRE(l->GetEffectCount() == 2);
    Effect* a = l->GetEffect(0);
    Effect* b = l->GetEffect(1);
    SeqMutationGuard::Scope scope;
    CHECK_FALSE(l->ReorderEffects({ a }));
    CHECK_FALSE(l->ReorderEffects({ a, a }));
    Effect* other = doc.Sequence().GetElement("Line")->GetEffectLayer(0)->GetEffect(0);
    CHECK_FALSE(l->ReorderEffects({ a, other }));
    CHECK(l->GetEffect(0) == a);
    REQUIRE(l->ReorderEffects({ b, a }));
    CHECK(l->GetEffect(0) == b);
    CHECK(l->GetEffect(1) == a);
    // Position ids are kept, and lookups by id still work.
    CHECK(b->GetID() == 1);
    CHECK(l->GetEffectFromUid(a->GetUid()) == a);
}
