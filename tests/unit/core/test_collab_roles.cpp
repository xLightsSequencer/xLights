// Viewer / editor roles and row assignments in a live session: the host
// lists them in its roster, rejects proposals that break them and keeps them
// across leaves, deletes and re-syncs.

#include <catch2/catch_test_macros.hpp>

#include "test_collab_common.h"

#include "collab/CollabRoles.h"
#include "render/SequenceElementCommands.h"

using namespace Collab;
using namespace CollabTest;

namespace {

LoopbackOptions Lan() {
    LoopbackOptions o;
    o.seed = 1;
    o.minDelayMs = 10;
    o.maxDelayMs = 10;
    return o;
}

SessionOptions Opts(const std::string& name = "") {
    SessionOptions o;
    o.verifyHashes = true;
    o.displayName = name;
    return o;
}

// A host Hana and clients Ari, Bea, Cy.
Node& Host(Sim& sim) {
    return sim.AddHost("", Opts("Hana"));
}
Node& Client(Sim& sim) {
    static const char* names[] = { "Ari", "Bea", "Cy", "Dee" };
    return sim.AddClientOn("", Opts(names[(sim.nodes.size() - 1) % 4]));
}

// The test show plus two model groups: Front (Line, Arch and Grid's Top
// submodel) and All (Front and Star), and a sequence with rows for both.
struct GroupShow : Show {
    GroupShow() {
        dir.Write("xlights_rgbeffects.xml",
                  "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb>\n<models>\n"
                  "<model name=\"Grid\" DisplayAs=\"Matrix\" StartChannel=\"1\" NumStrings=\"3\" NodesPerString=\"4\" StrandsPerString=\"1\">"
                  "<subModel name=\"Top\" layout=\"horizontal\" type=\"ranges\" bufferstyle=\"Default\" line0=\"1-2\"/></model>\n"
                  "<model name=\"Line\" DisplayAs=\"Single Line\" StartChannel=\"100\" NumStrings=\"1\" NodesPerString=\"4\"/>\n"
                  "<model name=\"Arch\" DisplayAs=\"Arches\" StartChannel=\"200\" NumStrings=\"1\" NodesPerString=\"8\"/>\n"
                  "<model name=\"Star\" DisplayAs=\"Star\" StartChannel=\"300\" NumStrings=\"1\" NodesPerString=\"10\"/>\n"
                  "<model name=\"Pole\" DisplayAs=\"Single Line\" StartChannel=\"400\" NumStrings=\"2\" NodesPerString=\"3\"/>\n"
                  "</models>\n<modelGroups>\n"
                  "<modelGroup name=\"Front\" models=\"Line,Arch,Grid/Top\" layout=\"minimalGrid\" GridSize=\"400\"/>\n"
                  "<modelGroup name=\"All\" models=\"Front,Star\" layout=\"minimalGrid\" GridSize=\"400\"/>\n"
                  "</modelGroups>\n</xrgb>\n");
        HeadlessRenderContext ctx;
        REQUIRE(ctx.LoadShowFolder(path));
        SequenceElements& se = ctx.GetSequenceElements();
        se.SetViewsManager(&ctx._sequenceViewManager);
        SequenceFile sf(start, 50);
        SeqMutationGuard::Scope building;
        Element* beats = se.AddElement("Beats", "timing", true, false, true, false, false);
        beats->AddEffectLayer()->AddEffect(0, "", "", "", 0, 500, 0, false);
        for (const char* name : { "Grid", "Line", "Arch", "Star", "Front", "All" }) {
            Element* el = se.AddElement(name, "model", true, false, false, false, false);
            if (el->GetEffectLayerCount() == 0) {
                el->AddEffectLayer();
            }
            el->GetEffectLayer(0)->AddEffect(0, "On", "", "", 1000, 1500, 0, false);
        }
        // Rows for Grid's submodel and its first strand.
        auto* grid = dynamic_cast<ModelElement*>(se.GetElement("Grid"));
        REQUIRE(grid != nullptr);
        for (int i = 0; i < grid->GetSubModelAndStrandCount(); ++i) {
            SubModelElement* sub = grid->GetSubModel(i);
            if (sub->GetEffectLayerCount() == 0) {
                sub->AddEffectLayer();
            }
            sub->GetEffectLayer(0)->AddEffect(0, "On", "", "", 1000, 1500, 0, false);
        }
        REQUIRE(sf.Save(se));
    }
};

Element* Row(Node& n, const std::string& name) {
    Element* el = n.Seq().GetElement(name);
    if (el != nullptr) {
        return el;
    }
    // "Model/Sub"
    auto slash = name.find('/');
    REQUIRE(slash != std::string::npos);
    auto* me = dynamic_cast<ModelElement*>(n.Seq().GetElement(name.substr(0, slash)));
    REQUIRE(me != nullptr);
    for (int i = 0; i < me->GetSubModelAndStrandCount(); ++i) {
        if (me->GetSubModel(i)->GetFullName() == name) {
            return me->GetSubModel(i);
        }
    }
    FAIL("no row " << name);
    return nullptr;
}

SeqObjectId Uid(Node& n, const std::string& row) {
    return Row(n, row)->GetUid();
}

std::unique_ptr<SequenceCommand> AddOn(Node& n, const std::string& row, int start, int end) {
    EffectState s;
    s.name = "On";
    s.startTimeMS = start;
    s.endTimeMS = end;
    return std::make_unique<AddEffectCommand>(LocatorOf(Row(n, row)->GetEffectLayer(0)), s);
}

Effect* EffectAt(Node& n, const std::string& row, int startMS) {
    for (Effect* e : Row(n, row)->GetEffectLayer(0)->GetEffects()) {
        if (e->GetStartTimeMS() == startMS) {
            return e;
        }
    }
    return nullptr;
}

std::unique_ptr<SequenceCommand> SetKey(Node& n, const std::string& row, int startMS, const std::string& key, const std::string& value) {
    Effect* e = EffectAt(n, row, startMS);
    REQUIRE(e != nullptr);
    return std::make_unique<ModifyEffectCommand>(LocatorOf(e), std::nullopt, SettingsChange{ false, { { key, value } } }, SettingsChange{});
}

// Moves the effect at `startMS` on `from` to `toMS` on `to` (a delete plus an add).
std::unique_ptr<SequenceCommand> MoveBetweenRows(Node& n, const std::string& from, const std::string& to, int startMS, int toMS) {
    Effect* e = EffectAt(n, from, startMS);
    REQUIRE(e != nullptr);
    auto b = std::make_unique<BatchCommand>("Move");
    EffectState s;
    s.name = e->GetEffectName();
    s.startTimeMS = toMS;
    s.endTimeMS = toMS + 200;
    b->Add(std::make_unique<DeleteEffectCommand>(LocatorOf(e)));
    b->Add(std::make_unique<AddEffectCommand>(LocatorOf(Row(n, to)->GetEffectLayer(0)), s));
    return b;
}

bool Submit(Sim& sim, Node& n, std::unique_ptr<SequenceCommand> cmd) {
    return sim.As(n, [&] { return n.session->Submit(std::move(cmd)).ok; });
}

bool Claim(Sim& sim, Node& n, const std::string& row, ParticipantId owner, std::string* why = nullptr) {
    std::string reason;
    std::string text;
    bool ok = sim.As(n, [&] { return n.session->Claim(Uid(n, row), owner, reason, text); });
    if (why != nullptr) {
        *why = text;
    }
    return ok;
}

ParticipantId OwnerOf(Node& n, const std::string& row) {
    AssignmentIndex index;
    index.Build(n.Seq(), n.session->Assignments());
    return index.OwnerOf(n.Seq(), Uid(n, row));
}

void RequireConverged(Sim& sim) {
    REQUIRE(sim.Quiesce());
    Node& h = sim.host();
    std::string desc = DescribeSequence(h.Seq());
    for (size_t i = 1; i < sim.nodes.size(); ++i) {
        Node& c = *sim.nodes[i];
        if (!c.Client().IsActive()) {
            continue;
        }
        INFO("client " << i);
        CHECK(DescribeSequence(c.Seq()) == desc);
        CHECK(DocHasher::FullHash(c.Seq()) == DocHasher::FullHash(h.Seq()));
        CHECK(c.session->Assignments() == h.session->Assignments());
        CHECK(c.session->Roster() == h.session->Roster());
    }
}

size_t Rejects(Node& host, const char* reason) {
    auto it = host.Host().RejectCounts().find(reason);
    return it == host.Host().RejectCounts().end() ? 0 : it->second;
}

bool LogHas(Node& n, const std::string& text) {
    return std::any_of(n.log.begin(), n.log.end(), [&](const std::string& l) { return l.find(text) != std::string::npos; });
}

} // namespace

TEST_CASE("Roster roles and assignments survive the wire", "[collab][collab_roles]") {
    Message m;
    m.type = MsgType::Roster;
    m.roster = { { 1, "Hana", "mac", ParticipantRole::Editor }, { 2, "Ari", "ipad", ParticipantRole::Viewer } };
    m.assignments = { { 77, 2, "Line" }, { 78, 1, "Beats" } };
    auto d = DecodeMessage(EncodeMessage(m));
    REQUIRE(d.has_value());
    CHECK(d->roster == m.roster);
    CHECK(d->assignments == m.assignments);
    Message c;
    c.type = MsgType::Claim;
    c.assignments = { { 77, kNoParticipant, "Line" } };
    auto dc = DecodeMessage(EncodeMessage(c));
    REQUIRE(dc.has_value());
    CHECK(dc->type == MsgType::Claim);
    CHECK(dc->assignments == c.assignments);
    // Malformed: an unknown role, a missing row, an empty claim.
    CHECK_FALSE(DecodeMessage("{\"v\":3,\"t\":\"Roster\",\"people\":[{\"id\":2,\"name\":\"x\",\"role\":\"boss\"}]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":3,\"t\":\"Roster\",\"people\":[],\"assign\":[{\"o\":2,\"n\":\"x\"}]}"));
    CHECK_FALSE(DecodeMessage("{\"v\":3,\"t\":\"Claim\",\"assign\":[]}"));
    // A roster from before roles reads as all editors, no assignments.
    auto old = DecodeMessage("{\"v\":3,\"t\":\"Roster\",\"people\":[{\"id\":2,\"name\":\"x\",\"device\":\"mac\"}]}");
    REQUIRE(old.has_value());
    CHECK(old->roster[0].role == ParticipantRole::Editor);
    CHECK(old->assignments.empty());
}

TEST_CASE("The host rejects a viewer's edits, undo and redo until it is an editor again", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    // a makes an edit as an editor, then becomes a viewer.
    REQUIRE(Submit(sim, a, AddOn(a, "Line", 3000, 3100)));
    REQUIRE(sim.Quiesce());
    REQUIRE(a.session->CanUndo());
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(aid, ParticipantRole::Viewer); }));
    CHECK_FALSE(sim.As(h, [&] { return h.Host().SetRole(kHostParticipant, ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    CHECK(a.session->LocalRole() == ParticipantRole::Viewer);
    CHECK(b.session->RoleOf(aid) == ParticipantRole::Viewer);
    CHECK(h.session->RoleOf(aid) == ParticipantRole::Viewer);
    CHECK(h.session->LocalRole() == ParticipantRole::Editor);
    CHECK(LogHas(a, "The host made you a viewer"));

    Rev head = h.Host().HeadRev();
    std::string before = DescribeSequence(h.Seq());
    // A structural edit, an optimistic key change, an undo.
    CHECK(Submit(sim, a, AddOn(a, "Grid", 4000, 4100)));
    CHECK(Submit(sim, a, SetKey(a, "Grid", 1000, "E_TEXTCTRL_Eff_On_Start", "55")));
    CHECK(sim.As(a, [&] { return a.session->Undo().ok; }));
    RequireConverged(sim);
    CHECK(h.Host().HeadRev() == head);
    CHECK(DescribeSequence(h.Seq()) == before);
    CHECK(Rejects(h, Reason::Viewer) == 3);
    CHECK(a.rejects[Reason::Viewer] == 3);
    CHECK(EffectAt(a, "Grid", 1000)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "10");

    // Editor again: edits commit.
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(aid, ParticipantRole::Editor); }));
    REQUIRE(sim.Quiesce());
    CHECK(a.session->LocalRole() == ParticipantRole::Editor);
    CHECK(Submit(sim, a, AddOn(a, "Grid", 4000, 4100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Grid", 4000) != nullptr);
    CHECK(Rejects(h, Reason::Viewer) == 3);
}

TEST_CASE("The host can admit new joiners as viewers", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    REQUIRE(sim.Quiesce());
    CHECK(h.Host().JoinRole() == ParticipantRole::Editor);
    sim.As(h, [&] { h.Host().SetJoinRole(ParticipantRole::Viewer); });
    Node& b = Client(sim);
    REQUIRE(sim.Quiesce());
    CHECK(a.session->LocalRole() == ParticipantRole::Editor);
    CHECK(b.session->LocalRole() == ParticipantRole::Viewer);
    CHECK(Submit(sim, b, AddOn(b, "Grid", 4000, 4100)));
    CHECK(Submit(sim, a, AddOn(a, "Grid", 5000, 5100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Grid", 4000) == nullptr);
    CHECK(EffectAt(h, "Grid", 5000) != nullptr);
    CHECK(b.rejects[Reason::Viewer] == 1);
}

TEST_CASE("Only the owner may edit an assigned row: direct edits, moves between rows, undo", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    // b edits Line before it is assigned (to undo later).
    REQUIRE(Submit(sim, b, AddOn(b, "Line", 3000, 3100)));
    REQUIRE(sim.Quiesce());
    // a claims Line for itself.
    REQUIRE(Claim(sim, a, "Line", aid));
    REQUIRE(sim.Quiesce());
    REQUIRE(h.session->Assignments().size() == 1);
    CHECK(h.session->Assignments()[0].owner == aid);
    CHECK(h.session->Assignments()[0].name == "Line");
    CHECK(OwnerOf(b, "Line") == aid);
    CHECK(OwnerOf(b, "Grid") == kNoParticipant);

    std::string before = DescribeSequence(h.Seq());
    CHECK(Submit(sim, b, AddOn(b, "Line", 4000, 4100)));                           // direct add
    CHECK(Submit(sim, b, SetKey(b, "Line", 0, "E_TEXTCTRL_Eff_On_Start", "77")));   // optimistic key change
    CHECK(Submit(sim, b, MoveBetweenRows(b, "Grid", "Line", 1000, 6000)));          // moved onto it
    CHECK(Submit(sim, b, MoveBetweenRows(b, "Line", "Grid", 0, 6000)));             // moved off it
    CHECK(Submit(sim, b, std::make_unique<DeleteElementCommand>(Uid(b, "Line")))); // the row itself
    CHECK(sim.As(b, [&] { return b.session->Undo().ok; }));                         // undo of b's add
    CHECK(Submit(sim, h, AddOn(h, "Line", 5000, 5100)) == false);                   // the host too
    RequireConverged(sim);
    CHECK(DescribeSequence(h.Seq()) == before);
    CHECK(Rejects(h, Reason::Assigned) == 7);
    CHECK(b.rejects[Reason::Assigned] == 6);
    CHECK(h.rejects[Reason::Assigned] == 1);
    CHECK(EffectAt(b, "Line", 0)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "<none>") == "<none>");
    CHECK(EffectAt(b, "Grid", 1000) != nullptr);
    for (const auto& [reason, text] : h.Host().RejectLog()) {
        if (reason == Reason::Assigned) {
            CHECK(text == "Line is assigned to Ari");
        }
    }
    // The owner's edits commit; others' rows stay open to everyone.
    CHECK(Submit(sim, a, AddOn(a, "Line", 4000, 4100)));
    CHECK(Submit(sim, a, MoveBetweenRows(a, "Grid", "Line", 2000, 6000)));
    CHECK(Submit(sim, b, AddOn(b, "Grid", 6000, 6100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Line", 4000) != nullptr);
    CHECK(EffectAt(h, "Line", 3000) != nullptr);
    CHECK(EffectAt(h, "Grid", 6000) != nullptr);
    CHECK(EffectAt(h, "Grid", 2000) == nullptr);
    CHECK(EffectAt(h, "Line", 6000) != nullptr);
}

TEST_CASE("A claim on a group covers its members, their submodels and nested groups", "[collab][collab_roles]") {
    GroupShow show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    ParticipantId bid = b.session->Participant();
    // The host gives Front to a.
    std::string why;
    REQUIRE(Claim(sim, h, "Front", aid, &why));
    REQUIRE(sim.Quiesce());
    for (const char* row : { "Front", "Line", "Arch", "Grid/Top" }) {
        INFO(row);
        CHECK(OwnerOf(b, row) == aid);
    }
    for (const char* row : { "Grid", "Star", "All", "Beats" }) {
        INFO(row);
        CHECK(OwnerOf(b, row) == kNoParticipant);
    }
    // b can't edit any of Front's rows; Grid itself and Star are free.
    std::string before = DescribeSequence(h.Seq());
    for (const char* row : { "Front", "Line", "Arch", "Grid/Top" }) {
        CHECK(Submit(sim, b, AddOn(b, row, 4000, 4100)));
    }
    RequireConverged(sim);
    CHECK(DescribeSequence(h.Seq()) == before);
    CHECK(Rejects(h, Reason::Assigned) == 4);
    CHECK(Submit(sim, b, AddOn(b, "Grid", 4000, 4100)));
    CHECK(Submit(sim, a, AddOn(a, "Grid/Top", 4000, 4100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Grid", 4000) != nullptr);
    CHECK(EffectAt(h, "Grid/Top", 4000) != nullptr);

    // Claims that overlap someone else's are refused: a member of Front, a
    // group holding Front.
    CHECK_FALSE(Claim(sim, b, "Line", bid, &why));
    CHECK(why == "Line overlaps Front, which is assigned to Ari");
    CHECK_FALSE(Claim(sim, b, "All", bid, &why));
    CHECK(why.find("overlaps Front") != std::string::npos);
    CHECK(Claim(sim, b, "Star", bid));
    // Only the host may assign to someone else; a claim on a submodel row
    // claims its model.
    CHECK_FALSE(Claim(sim, b, "Grid", aid, &why));
    CHECK(why == "Only the host can assign rows to someone else");
    // Grid's Top submodel is in a's Front.
    CHECK_FALSE(Claim(sim, b, "Grid/Strand 1", bid, &why));
    CHECK(why == "Grid overlaps Front, which is assigned to Ari");
    REQUIRE(sim.Quiesce());
    REQUIRE(Claim(sim, h, "Front", kNoParticipant));
    REQUIRE(sim.Quiesce());
    CHECK(Claim(sim, b, "Grid/Strand 1", bid));
    REQUIRE(sim.Quiesce());
    CHECK(OwnerOf(a, "Grid") == bid);
    CHECK(OwnerOf(a, "Grid/Strand 1") == bid);
    CHECK(OwnerOf(a, "Grid/Top") == bid);
    CHECK(OwnerOf(a, "Star") == bid);
    CHECK(OwnerOf(a, "Line") == kNoParticipant);
    // Now Front would overlap b's Grid.
    CHECK_FALSE(Claim(sim, h, "Front", aid, &why));
    CHECK(why == "Front overlaps Grid, which is assigned to Bea");
    // Strand rows go with their model.
    CHECK(Submit(sim, a, AddOn(a, "Grid/Strand 1", 4000, 4100)));
    CHECK(Submit(sim, a, AddOn(a, "Line", 4000, 4100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Grid/Strand 1", 4000) == nullptr);
    CHECK(EffectAt(h, "Line", 4000) != nullptr);
    CHECK(h.session->Assignments().size() == 2);
}

TEST_CASE("Claims go when the owner or the host releases them, the owner leaves or becomes a viewer, or the row is deleted",
          "[collab][collab_roles]") {
    GroupShow show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    ParticipantId bid = b.session->Participant();
    REQUIRE(Claim(sim, a, "Line", aid));
    REQUIRE(Claim(sim, a, "Grid", aid));
    REQUIRE(Claim(sim, b, "Arch", bid));
    REQUIRE(sim.Quiesce());
    REQUIRE(h.session->Assignments().size() == 3);

    // Someone else can't release it; the owner can.
    std::string why;
    CHECK_FALSE(Claim(sim, b, "Line", kNoParticipant, &why));
    CHECK(why == "Line is assigned to Ari; only they or the host can release it");
    CHECK(Claim(sim, a, "Line", kNoParticipant));
    REQUIRE(sim.Quiesce());
    CHECK(OwnerOf(h, "Line") == kNoParticipant);
    // The host can release anyone's.
    CHECK(Claim(sim, h, "Arch", kNoParticipant));
    REQUIRE(sim.Quiesce());
    CHECK(OwnerOf(a, "Arch") == kNoParticipant);
    // Becoming a viewer releases them.
    REQUIRE(Claim(sim, b, "Arch", bid));
    REQUIRE(sim.Quiesce());
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(bid, ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    CHECK(OwnerOf(a, "Arch") == kNoParticipant);
    // A viewer can't claim, and rows can't be given to one.
    CHECK_FALSE(Claim(sim, b, "Arch", bid, &why));
    CHECK(why == "Viewers can't assign rows");
    CHECK_FALSE(Claim(sim, h, "Arch", bid, &why));
    CHECK(why.find("is a viewer") != std::string::npos);

    // Deleting the row (by its owner) drops the claim.
    REQUIRE(Claim(sim, a, "Line", aid));
    REQUIRE(sim.Quiesce());
    REQUIRE(h.session->Assignments().size() == 2);
    SeqObjectId line = Uid(h, "Line");
    CHECK(Submit(sim, a, std::make_unique<DeleteElementCommand>(line)));
    RequireConverged(sim);
    CHECK(h.Seq().GetElementFromUid(line) == nullptr);
    REQUIRE(h.session->Assignments().size() == 1);
    CHECK(OwnerOf(b, "Grid") == aid);

    // Leaving drops the rest.
    sim.As(a, [&] { a.Client().Leave(); });
    REQUIRE(sim.Quiesce());
    CHECK(h.session->Assignments().empty());
    CHECK(b.session->Assignments().empty());
    CHECK(Submit(sim, h, AddOn(h, "Grid", 4000, 4100)));
    RequireConverged(sim);
    CHECK(EffectAt(b, "Grid", 4000) != nullptr);
}

TEST_CASE("Roles and assignments survive a re-sync, even one whose roster was lost", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    ParticipantId bid = b.session->Participant();
    // b misses the roster that makes it a viewer and assigns Line to a.
    PeerId bPeer = b.transport->LocalPeer();
    sim.net.tamper = [&](PeerId, PeerId to, std::string& f) {
        auto m = DecodeMessage(f);
        if (to == bPeer && m && m->type == MsgType::Roster) {
            f.clear();
        }
    };
    REQUIRE(Claim(sim, h, "Line", aid));
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(bid, ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    sim.net.tamper = nullptr;
    CHECK(b.session->Assignments().empty());
    CHECK(b.session->LocalRole() == ParticipantRole::Editor);
    // The host still refuses b.
    CHECK(Submit(sim, b, AddOn(b, "Grid", 4000, 4100)));
    REQUIRE(sim.Quiesce());
    CHECK(EffectAt(h, "Grid", 4000) == nullptr);
    // A re-sync brings b the roster with the snapshot.
    sim.As(b, [&] { b.Client().RequestResync("test"); });
    RequireConverged(sim);
    CHECK(b.Client().ResyncCount() == 1);
    CHECK(b.session->LocalRole() == ParticipantRole::Viewer);
    CHECK(OwnerOf(b, "Line") == aid);
    // a re-syncs too: its claim still holds by the same row id.
    sim.As(a, [&] { a.Client().RequestResync("test"); });
    REQUIRE(sim.Quiesce());
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(bid, ParticipantRole::Editor); }));
    REQUIRE(sim.Quiesce());
    CHECK(Submit(sim, b, AddOn(b, "Line", 4000, 4100)));
    CHECK(Submit(sim, a, AddOn(a, "Line", 5000, 5100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Line", 4000) == nullptr);
    CHECK(EffectAt(h, "Line", 5000) != nullptr);
    // A late joiner gets them too.
    Node& c = Client(sim);
    REQUIRE(sim.Quiesce());
    CHECK(OwnerOf(c, "Line") == aid);
    CHECK(c.session->RoleOf(bid) == ParticipantRole::Editor);
}

TEST_CASE("A local edit the host rejects is rolled back through the render path", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(aid, ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    SeqObjectId uid = EffectAt(a, "Grid", 1000)->GetUid();
    // Applied at once on the client, then rejected (a viewer's edit).
    SubmitResult r = sim.As(a, [&] { return a.session->Submit(SetKey(a, "Grid", 1000, "E_TEXTCTRL_Eff_On_Start", "55")); });
    REQUIRE(r.ok);
    REQUIRE(r.optimistic);
    CHECK(EffectAt(a, "Grid", 1000)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "55");
    RequireConverged(sim);
    CHECK(a.rejects[Reason::Viewer] == 1);
    CHECK(EffectAt(a, "Grid", 1000)->GetSettings().Get("E_TEXTCTRL_Eff_On_Start", "") == "10");
    // The app was told what the rollback changed, as for a commit, so the
    // row renders again.
    REQUIRE(a.rolledBack.size() == 1);
    const auto& [applied, inverse] = a.rolledBack[0];
    std::unique_ptr<SequenceCommand> cmd = DeserializeCommand(applied);
    std::unique_ptr<SequenceCommand> inv = DeserializeCommand(inverse);
    REQUIRE(cmd != nullptr);
    REQUIRE(inv != nullptr);
    std::vector<const SequenceCommand*> members;
    for (const auto& m : static_cast<const BatchCommand&>(*cmd).GetCommands()) {
        members.push_back(m.get());
    }
    REQUIRE(members.size() == 1);
    REQUIRE(members[0]->Type() == CommandType::ModifyEffect);
    CHECK(static_cast<const ModifyEffectCommand&>(*members[0]).Target().effect == uid);
    CHECK(h.rolledBack.empty());
    // A structural edit was never applied locally: nothing to roll back.
    CHECK(Submit(sim, a, AddOn(a, "Grid", 4000, 4100)));
    RequireConverged(sim);
    CHECK(a.rolledBack.size() == 1);
}

TEST_CASE("A rejoin for someone still connected is refused and leaves them alone", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    Node& x = sim.AddClient(false); // connected, never joins: sends forged hellos
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    ParticipantId bid = b.session->Participant();
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(bid, ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    auto roster = h.session->Roster();
    for (ParticipantId who : { aid, bid }) {
        Message hello;
        hello.type = MsgType::Hello;
        hello.rejoin = true;
        hello.name = "Mallory";
        hello.participant = who;
        hello.rev = h.Host().HeadRev();
        hello.clientSeq = 1;
        x.transport->Send(h.transport->LocalPeer(), hello);
    }
    sim.RunFor(2000);
    CHECK(LogHas(h, "refused a rejoin as participant " + std::to_string(aid)));
    CHECK(LogHas(h, "refused a rejoin as participant " + std::to_string(bid)));
    // Nobody took their places: same roster, roles and connections.
    CHECK(h.session->Roster() == roster);
    CHECK(h.session->RoleOf(bid) == ParticipantRole::Viewer);
    CHECK(h.Host().ClientCount() == 2);
    REQUIRE(a.Client().IsActive());
    REQUIRE(b.Client().IsActive());
    CHECK(Submit(sim, a, AddOn(a, "Grid", 4000, 4100)));
    RequireConverged(sim);
    CHECK(EffectAt(h, "Grid", 4000) != nullptr);
    CHECK(b.session->LocalRole() == ParticipantRole::Viewer);
}

TEST_CASE("A viewer can't come back as an editor by rejoining outside a host change", "[collab][collab_roles]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& x = sim.AddClient(false);
    REQUIRE(sim.Quiesce());
    ParticipantId aid = a.session->Participant();
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(aid, ParticipantRole::Viewer); }));
    REQUIRE(sim.Quiesce());
    Rev head = h.Host().HeadRev();
    sim.As(a, [&] { a.Client().Leave(); });
    sim.RunFor(200);
    REQUIRE(h.Host().ClientCount() == 0);
    Message hello;
    hello.type = MsgType::Hello;
    hello.rejoin = true;
    hello.name = "Ari";
    hello.participant = aid;
    hello.rev = head;
    hello.clientSeq = 1;
    x.transport->Send(h.transport->LocalPeer(), hello);
    sim.RunFor(2000);
    CHECK(h.Host().ClientCount() == 0);
    CHECK(h.Host().Clients().empty());
    const auto& roster = h.session->Roster();
    CHECK(std::none_of(roster.begin(), roster.end(), [&](const RosterEntry& r) { return r.participant == aid; }));
    CHECK(LogHas(h, "refused a rejoin as participant " + std::to_string(aid)));
}

TEST_CASE("A rejoiner whose first connection went silent reconnects and keeps its role", "[collab][collab_roles][migrate]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    Node& x = sim.AddClient(false); // b's first rejoin connection, which then goes silent
    REQUIRE(sim.Quiesce());
    ParticipantId bid = b.session->Participant();
    REQUIRE(sim.As(h, [&] { return h.Host().SetRole(bid, ParticipantRole::Viewer); }));
    REQUIRE(Submit(sim, a, AddOn(a, "Grid", 3000, 3100)));
    REQUIRE(sim.Quiesce());
    sim.RunFor(1500); // a ping round: everyone knows a takes over
    a.migrates = b.migrates = true;
    sim.net.Disconnect(h.transport->LocalPeer());
    sim.RunFor(50);
    REQUIRE(a.Client().GetState() == CollabClient::State::Migrating);
    REQUIRE(b.Client().GetState() == CollabClient::State::Migrating);
    sim.As(a, [&] { a.Client().BeginSuccession(); });
    sim.TakeOver(a);
    PeerId ap = a.transport->LocalPeer();
    // b's first connection rejoins, then nothing more comes over it, and
    // the new host hasn't noticed by the time b reconnects.
    Message hello;
    hello.type = MsgType::Hello;
    hello.rejoin = true;
    hello.name = "Bea";
    hello.participant = bid;
    hello.rev = b.Client().AppliedRev();
    hello.clientSeq = 1;
    hello.secret = b.Client().RejoinSecret();
    x.transport->Send(ap, hello);
    sim.RunFor(100);
    REQUIRE(a.Host().ClientCount() == 1);
    sim.As(b, [&] { b.Client().Rejoin(ap); });
    sim.RunFor(2000);
    // The new connection replaced the silent one: b carries on, still a viewer.
    REQUIRE(b.Client().GetState() == CollabClient::State::Active);
    CHECK(a.Host().ClientCount() == 1);
    CHECK(a.session->RoleOf(bid) == ParticipantRole::Viewer);
    CHECK(b.session->LocalRole() == ParticipantRole::Viewer);
    CHECK(LogHas(a, "reconnected; its old connection is dropped"));
    // Edits flow to it through the new connection.
    REQUIRE(Submit(sim, a, AddOn(a, "Grid", 5000, 5100)));
    sim.RunFor(1000);
    CHECK(b.Client().AppliedRev() == a.Host().HeadRev());
    CHECK(DescribeSequence(b.Seq()) == DescribeSequence(a.Seq()));
    CHECK(DocHasher::FullHash(b.Seq()) == DocHasher::FullHash(a.Seq()));
    // Outside the takeover window the same is still refused.
    sim.RunFor(16000);
    x.transport->Send(ap, hello);
    sim.RunFor(200);
    CHECK(LogHas(a, "refused a rejoin as participant " + std::to_string(bid)));
    CHECK(b.Client().GetState() == CollabClient::State::Active);
}

namespace {
// A host, Ari and Bea (a viewer); the host is lost and Ari takes over.
struct TakenOver {
    Show show;
    Sim sim{ show, Lan(), Opts() };
    Node& h = Host(sim);
    Node& a = Client(sim);
    Node& b = Client(sim);
    Node& x = sim.AddClient(false); // a connection that never joined
    ParticipantId bid = kNoParticipant;
    PeerId ap = kNoPeer;
    TakenOver() {
        REQUIRE(sim.Quiesce());
        bid = b.session->Participant();
        REQUIRE(sim.As(h, [&] { return h.Host().SetRole(bid, ParticipantRole::Viewer); }));
        REQUIRE(sim.Quiesce());
        sim.RunFor(1500);
        a.migrates = b.migrates = true;
        sim.net.Disconnect(h.transport->LocalPeer());
        sim.RunFor(50);
        REQUIRE(b.Client().GetState() == CollabClient::State::Migrating);
        sim.As(a, [&] { a.Client().BeginSuccession(); });
        sim.TakeOver(a);
        ap = a.transport->LocalPeer();
    }
    Message Hello(const std::string& secret) {
        Message m;
        m.type = MsgType::Hello;
        m.rejoin = true;
        m.name = "Mallory";
        m.participant = bid;
        m.rev = a.Host().HeadRev();
        m.clientSeq = 1;
        m.secret = secret;
        return m;
    }
};
} // namespace

TEST_CASE("The roster carries a digest of each rejoin secret, never the secret", "[collab][collab_roles][migrate]") {
    Show show;
    Sim sim(show, Lan(), Opts());
    Node& h = Host(sim);
    Node& a = Client(sim);
    REQUIRE(sim.Quiesce());
    const std::string& secret = a.Client().RejoinSecret();
    REQUIRE(secret.size() == 64);
    ParticipantId aid = a.session->Participant();
    for (Node* n : { &h, &a }) {
        auto& roster = n->session->Roster();
        auto it = std::find_if(roster.begin(), roster.end(), [&](const RosterEntry& r) { return r.participant == aid; });
        REQUIRE(it != roster.end());
        CHECK(it->secretHash == SecretDigest(secret));
        CHECK(it->secretHash != secret);
    }
    Message r;
    r.type = MsgType::Roster;
    r.roster = h.session->Roster();
    std::string wire = EncodeMessage(r);
    CHECK(wire.find(secret) == std::string::npos);
    CHECK(DecodeMessage(wire)->roster == r.roster);
    // Known digest (SHA-256 of "abc").
    CHECK(SecretDigest("abc") == "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
}

TEST_CASE("A rejoin without the participant's secret is refused, even during a host change", "[collab][collab_roles][migrate]") {
    TakenOver t;
    // Missing and wrong secrets, inside the takeover window.
    t.x.transport->Send(t.ap, t.Hello(""));
    t.x.transport->Send(t.ap, t.Hello(std::string(32, '0')));
    t.sim.RunFor(200);
    CHECK(t.a.Host().ClientCount() == 0);
    CHECK(LogHas(t.a, "refused a rejoin as participant " + std::to_string(t.bid) + ": wrong rejoin secret"));
    // Bea's own rejoin carries her secret.
    t.sim.As(t.b, [&] { t.b.Client().Rejoin(t.ap); });
    t.sim.RunFor(1000);
    REQUIRE(t.b.Client().GetState() == CollabClient::State::Active);
    CHECK(t.a.session->RoleOf(t.bid) == ParticipantRole::Viewer);
    // A forger can't evict her now either.
    t.x.transport->Send(t.ap, t.Hello(std::string(32, 'f')));
    t.sim.RunFor(1000);
    CHECK(t.a.Host().ClientCount() == 1);
    CHECK(t.b.Client().GetState() == CollabClient::State::Active);
    CHECK_FALSE(LogHas(t.a, "reconnected; its old connection is dropped"));
}

TEST_CASE("A rejoin with the secret replaces a stale connection and keeps the role", "[collab][collab_roles][migrate]") {
    TakenOver t;
    t.sim.As(t.b, [&] { t.b.Client().Rejoin(t.ap); });
    t.sim.RunFor(1000);
    REQUIRE(t.b.Client().GetState() == CollabClient::State::Active);
    // The same participant, from another connection, with the secret: the
    // old connection goes, the new one is Bea with her role.
    t.x.transport->Send(t.ap, t.Hello(t.b.Client().RejoinSecret()));
    t.sim.RunFor(500);
    CHECK(LogHas(t.a, "reconnected; its old connection is dropped"));
    CHECK(t.a.Host().ClientCount() == 1);
    CHECK(t.a.session->RoleOf(t.bid) == ParticipantRole::Viewer);
    // And the digest the new host lists is still hers.
    auto& roster = t.a.session->Roster();
    auto it = std::find_if(roster.begin(), roster.end(), [&](const RosterEntry& r) { return r.participant == t.bid; });
    REQUIRE(it != roster.end());
    CHECK(it->secretHash == SecretDigest(t.b.Client().RejoinSecret()));
}
