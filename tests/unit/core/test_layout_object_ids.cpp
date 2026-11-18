#include <cctype>
#include <functional>
#include <set>
#include <sstream>
#include <unordered_set>
#include <string>

#include <catch2/catch_test_macros.hpp>

#include <pugixml.hpp>

#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "models/SubModel.h"
#include "models/ViewObject.h"
#include "models/ViewObjectManager.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceViewManager.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"
#include "utils/LayoutObjectId.h"
#include "XmlSerializer/XmlSerializer.h"

// Permanent layout ids (utils/LayoutObjectId.h): every layout record carries a
// unique `xlId` that survives renames, rebuilds and saves, and that copies do
// not inherit.
namespace {

const char* kIdA = "11111111-1111-4111-8111-111111111111";
const char* kIdB = "22222222-2222-4222-8222-222222222222";
const char* kIdC = "33333333-3333-4333-8333-333333333333";
const char* kIdD = "44444444-4444-4444-8444-444444444444";

std::string IdAttr(const std::string& id) {
    return id.empty() ? std::string() : " xlId=\"" + id + "\"";
}

// `extra` may override NodesPerString; it is only defaulted when absent.
std::string LineXml(const std::string& name, const std::string& id = "", const std::string& children = "", const std::string& extra = "") {
    std::string nodes = extra.find("NodesPerString") == std::string::npos ? " NodesPerString=\"10\"" : "";
    return "<model name=\"" + name + "\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\"" + nodes + IdAttr(id) + " " + extra + ">" + children + "</model>";
}

std::string SubXml(const std::string& name, const std::string& id = "") {
    return "<subModel name=\"" + name + "\" layout=\"horizontal\" type=\"ranges\" line0=\"1-5\"" + IdAttr(id) + "/>";
}

std::string GroupXml(const std::string& name, const std::string& models, const std::string& id = "") {
    return "<modelGroup name=\"" + name + "\" models=\"" + models + "\" layout=\"minimalGrid\" GridSize=\"400\"" + IdAttr(id) + "/>";
}

std::string GridlinesXml(const std::string& name, const std::string& id = "") {
    return "<view_object name=\"" + name + "\" DisplayAs=\"Gridlines\" LayoutGroup=\"Default\" GridLineSpacing=\"50\" GridWidth=\"100\" GridHeight=\"100\"" + IdAttr(id) + "/>";
}

std::string ViewXml(const std::string& name, const std::string& models, const std::string& id = "") {
    return "<view name=\"" + name + "\" models=\"" + models + "\"" + IdAttr(id) + "/>";
}

std::string NullControllerXml(const std::string& name, const std::string& id = "") {
    return "<Controller Type=\"Null\" Id=\"64001\" Name=\"" + name + "\"" + IdAttr(id) + "><network NetworkType=\"NULL\" MaxChannels=\"10\"/></Controller>";
}

std::string ShowXml(const std::string& models, const std::string& groups = "", const std::string& objects = "",
                    const std::string& views = "", const std::string& extra = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<xrgb><models>" + models + "</models><modelGroups>" + groups +
           "</modelGroups><view_objects>" + objects + "</view_objects><views>" + views + "</views>" + extra + "</xrgb>";
}

std::string NetworksXml(const std::string& controllers = "") {
    return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks>" + controllers + "</Networks>";
}

struct FixFileReset {
    ~FixFileReset() {
        FileUtils::SetFixFileShowDir("");
        FileUtils::SetFixFileDirectories({});
        FileUtils::ClearNonExistentFiles();
    }
};

struct ShowFolder {
    TempDir dir;
    FixFileReset reset;
    HeadlessRenderContext ctx;

    void Load(const std::string& rgbeffects, const std::string& networks = NetworksXml()) {
        dir.Write("xlights_networks.xml", networks);
        dir.Write("xlights_rgbeffects.xml", rgbeffects);
        REQUIRE(ctx.LoadShowFolder(dir.path.string()));
    }
    Model* M(const std::string& name) const {
        Model* m = ctx.AllModels.GetModel(name);
        REQUIRE(m != nullptr);
        return m;
    }
    std::string Id(const std::string& name) const { return M(name)->GetObjectId(); }
    std::string ObjId(const std::string& name) const {
        ViewObject* o = ctx.AllObjects.GetViewObject(name);
        REQUIRE(o != nullptr);
        return o->GetObjectId();
    }
    std::string ViewId(const std::string& name) {
        SequenceView* v = ctx._sequenceViewManager.GetView(name);
        REQUIRE(v != nullptr);
        return v->GetObjectId();
    }
};

// The rgbeffects document the desktop would write from memory (the parts that
// are rebuilt through the serializer).
std::string BuildDesktopXml(HeadlessRenderContext& ctx) {
    pugi::xml_document doc;
    pugi::xml_node root = doc.append_child("xrgb");
    XmlSerializingVisitor visitor{ root };
    XmlSerializer::SerializeAllModels(ctx.AllModels, visitor);
    XmlSerializer::SerializeAllObjects(ctx.AllObjects, visitor);
    ctx._sequenceViewManager.Save(visitor);
    std::ostringstream out;
    doc.save(out);
    return out.str();
}

Model* Deserialize(ModelManager& mm, pugi::xml_node node, bool importing) {
    XmlSerializer s;
    return s.DeserializeModel(node, mm, importing);
}

bool HasIdAnywhere(pugi::xml_node node) {
    if (!LayoutObjectId::Read(node).empty()) return true;
    for (pugi::xml_node c = node.first_child(); c; c = c.next_sibling()) {
        if (HasIdAnywhere(c)) return true;
    }
    return false;
}

} // namespace

TEST_CASE("Layout ids are lower-case RFC 4122 v4 UUIDs", "[layoutid]") {
    std::set<std::string> seen;
    for (int i = 0; i < 200; ++i) {
        std::string id = LayoutObjectId::Generate();
        REQUIRE(id.size() == 36);
        CHECK(id[8] == '-');
        CHECK(id[13] == '-');
        CHECK(id[18] == '-');
        CHECK(id[23] == '-');
        CHECK(id[14] == '4');
        CHECK(std::string("89ab").find(id[19]) != std::string::npos);
        for (char c : id) {
            CHECK((c == '-' || std::isdigit((unsigned char)c) || (c >= 'a' && c <= 'f')));
        }
        seen.insert(id);
    }
    CHECK(seen.size() == 200);
}

TEST_CASE("Loading keeps the ids in the file", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", kIdA, SubXml("Left", kIdB)), GroupXml("G", "A", kIdC),
                   GridlinesXml("Grid", kIdD), ViewXml("V", "A", kIdA)));
    CHECK(f.Id("A") == kIdA);
    CHECK(f.M("A")->GetSubModel("Left")->GetObjectId() == kIdB);
    CHECK(f.Id("G") == kIdC);
    CHECK(f.ObjId("Grid") == kIdD);
    // views have an id space of their own
    CHECK(f.ViewId("V") == kIdA);
    CHECK(f.ctx.AllModels.GetModelByObjectId(kIdA) == f.M("A"));
    CHECK(f.ctx.AllModels.GetModelByObjectId(kIdB) == f.M("A")->GetSubModel("Left"));
    CHECK(f.ctx.AllModels.GetModelByObjectId(kIdC) == f.M("G"));
    CHECK(f.ctx.AllModels.GetModelByObjectId("nope") == nullptr);
    CHECK(f.ctx.AllObjects.GetViewObjectByObjectId(kIdD) == f.ctx.AllObjects.GetViewObject("Grid"));
}

TEST_CASE("Loading assigns an id to every record that has none", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", "", SubXml("Left") + SubXml("Right")) + LineXml("B"), GroupXml("G", "A,B"),
                   GridlinesXml("Grid"), ViewXml("V", "A")));
    std::set<std::string> ids{ f.Id("A"), f.Id("B"), f.Id("G"),
                               f.M("A")->GetSubModel("Left")->GetObjectId(),
                               f.M("A")->GetSubModel("Right")->GetObjectId() };
    CHECK(ids.size() == 5);
    CHECK(ids.count("") == 0);
    CHECK_FALSE(f.ObjId("Grid").empty());
    CHECK_FALSE(f.ViewId("V").empty());
}

TEST_CASE("Loading gives the later of two records sharing an id a new one", "[layoutid]") {
    ShowFolder f;
    // A, then B with A's id, a submodel repeating it, and a group repeating it.
    f.Load(ShowXml(LineXml("A", kIdA) + LineXml("B", kIdA, SubXml("Left", kIdA) + SubXml("Right", kIdB)),
                   GroupXml("G", "A,B", kIdB),
                   GridlinesXml("Grid1", kIdC) + GridlinesXml("Grid2", kIdC),
                   ViewXml("V1", "A", kIdD) + ViewXml("V2", "B", kIdD)));
    CHECK(f.Id("A") == kIdA);
    CHECK(f.Id("B") != kIdA);
    CHECK(f.M("B")->GetSubModel("Left")->GetObjectId() != kIdA);
    CHECK(f.M("B")->GetSubModel("Right")->GetObjectId() == kIdB);
    CHECK(f.Id("G") != kIdB);
    CHECK_FALSE(f.Id("G").empty());
    CHECK(f.ObjId("Grid1") == kIdC);
    CHECK(f.ObjId("Grid2") != kIdC);
    CHECK(f.ViewId("V1") == kIdD);
    CHECK(f.ViewId("V2") != kIdD);
}

TEST_CASE("Loading ignores attributes and elements it does not know", "[layoutid]") {
    // What an older version sees when a newer one wrote ids: the record loads
    // the same, without error. Here a newer attribute it cannot know stands in.
    auto load = [](const std::string& extraAttr, const std::string& extraChild) {
        ShowFolder f;
        f.Load(ShowXml(LineXml("A", kIdA, SubXml("Left", kIdB) + extraChild, extraAttr),
                       "<modelGroup name=\"G\" models=\"A\" layout=\"minimalGrid\" GridSize=\"400\" " + extraAttr + "/>",
                       "<view_object name=\"Grid\" DisplayAs=\"Gridlines\" " + extraAttr + "/>",
                       "<view name=\"V\" models=\"A\" " + extraAttr + "/>"));
        CHECK(f.M("A")->GetNodeCount() == 10);
        CHECK(f.M("A")->GetSubModel("Left") != nullptr);
        CHECK(f.ctx.AllObjects.GetViewObject("Grid") != nullptr);
        CHECK(f.ViewId("V") != "");
        std::string xml = BuildDesktopXml(f.ctx);
        // ids are random where the file had none; blank them for the comparison
        pugi::xml_document d;
        REQUIRE(d.load_string(xml.c_str()));
        std::function<void(pugi::xml_node)> blank = [&](pugi::xml_node n) {
            n.remove_attribute(LayoutObjectId::ATTRIBUTE);
            for (auto c = n.first_child(); c; c = c.next_sibling()) blank(c);
        };
        blank(d.document_element());
        std::ostringstream out;
        d.save(out);
        return out.str();
    };
    std::string plain = load("", "");
    std::string withUnknown = load("xlFutureThing=\"q\"", "<futureElement a=\"1\"/>");
    CHECK(withUnknown == plain);

    // controllers too
    TempDir dir;
    dir.Write("xlights_networks.xml",
              NetworksXml("<Controller Type=\"Null\" Id=\"64001\" Name=\"N\" xlFutureThing=\"q\"><network NetworkType=\"NULL\" MaxChannels=\"10\"/><futureElement/></Controller>"));
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    REQUIRE(om.GetController("N") != nullptr);
    CHECK(om.GetController("N")->GetChannels() == 10);
}

TEST_CASE("Ids survive a save and reload", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", "", SubXml("Left")) + LineXml("B"), GroupXml("G", "A,B"),
                   GridlinesXml("Grid"), ViewXml("V", "A")));
    std::string a = f.Id("A"), b = f.Id("B"), g = f.Id("G");
    std::string left = f.M("A")->GetSubModel("Left")->GetObjectId();
    std::string grid = f.ObjId("Grid"), view = f.ViewId("V");

    std::string saved = BuildDesktopXml(f.ctx);
    pugi::xml_document d;
    REQUIRE(d.load_string(saved.c_str()));
    CHECK(LayoutObjectId::Read(d.document_element().child("models").find_child_by_attribute("model", "name", "A")) == a);

    ShowFolder again;
    again.Load(saved);
    CHECK(again.Id("A") == a);
    CHECK(again.Id("B") == b);
    CHECK(again.Id("G") == g);
    CHECK(again.M("A")->GetSubModel("Left")->GetObjectId() == left);
    CHECK(again.ObjId("Grid") == grid);
    CHECK(again.ViewId("V") == view);
}

TEST_CASE("A renamed model keeps its id", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", kIdA, SubXml("Left", kIdB))));
    f.ctx.AllModels.Rename("A", "Renamed");
    CHECK(f.Id("Renamed") == kIdA);
    CHECK(f.M("Renamed")->GetSubModel("Left")->GetObjectId() == kIdB);
    CHECK(f.ctx.AllModels.GetModelByObjectId(kIdA) == f.M("Renamed"));
}

TEST_CASE("ReplaceModel keeps the id of the model it replaces", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", kIdA, SubXml("Left", kIdB)) + LineXml("B", kIdC)));
    ModelManager& mm = f.ctx.AllModels;

    SECTION("a rebuild from the model's own XML") {
        XmlSerializer s;
        pugi::xml_document doc = s.SerializeModel(f.M("A"));
        pugi::xml_node node = doc.document_element().first_child();
        node.attribute("NodesPerString").set_value("20");
        Model* nm = mm.CreateModel(node);
        mm.ReplaceModel("A", nm);
        CHECK(f.M("A") == nm);
        CHECK(f.M("A")->GetNodeCount() == 20);
        CHECK(f.Id("A") == kIdA);
        CHECK(f.M("A")->GetSubModel("Left")->GetObjectId() == kIdB);
    }
    SECTION("a replacement that brings no id") {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(LineXml("A", "", "", "NodesPerString=\"30\"").c_str()));
        Model* nm = mm.CreateModel(doc.first_child());
        CHECK(nm->GetObjectId().empty());
        mm.ReplaceModel("A", nm);
        CHECK(f.Id("A") == kIdA);
    }
    SECTION("a replacement under a new name") {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(LineXml("A2").c_str()));
        mm.ReplaceModel("A", mm.CreateModel(doc.first_child()));
        CHECK(mm.GetModel("A") == nullptr);
        CHECK(f.Id("A2") == kIdA);
    }
    SECTION("AddModel over a model of the same name") {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(LineXml("A").c_str()));
        mm.AddModel(mm.CreateModel(doc.first_child()));
        CHECK(f.Id("A") == kIdA);
    }
    CHECK(f.Id("B") == kIdC);
}

TEST_CASE("A copy of a model gets new ids", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", kIdA, SubXml("Left", kIdB)), GroupXml("G", "A", kIdC)));
    ModelManager& mm = f.ctx.AllModels;
    XmlSerializer s;

    SECTION("duplicated through its XML, as copy/paste and duplicate do") {
        pugi::xml_document doc = s.SerializeModel(f.M("A"));
        pugi::xml_node node = doc.document_element().first_child();
        node.attribute("name").set_value("Copy");
        REQUIRE(LayoutObjectId::Read(node) == kIdA);
        mm.AddModel(mm.CreateModel(node));
        CHECK(f.Id("Copy") != kIdA);
        CHECK_FALSE(f.Id("Copy").empty());
        CHECK(f.M("Copy")->GetSubModel("Left")->GetObjectId() != kIdB);
        CHECK(f.Id("A") == kIdA);
        CHECK(f.M("A")->GetSubModel("Left")->GetObjectId() == kIdB);
    }
    SECTION("imported, as an xModel or a model from another show") {
        pugi::xml_document doc = s.SerializeModel(f.M("A"));
        pugi::xml_node node = doc.document_element().first_child();
        Model* imported = Deserialize(mm, node, /*importing*/ true);
        REQUIRE(imported != nullptr);
        CHECK(imported->GetObjectId().empty());
        CHECK(imported->GetSubModel("Left")->GetObjectId() != kIdB);
        mm.AddModel(imported);
        CHECK_FALSE(imported->GetObjectId().empty());
        CHECK(imported->GetObjectId() != kIdA);
    }
    SECTION("a group cloned through its XML") {
        pugi::xml_document doc;
        pugi::xml_node root = doc.append_child("modelGroups");
        XmlSerializingVisitor visitor{ root };
        f.M("G")->Accept(visitor);
        pugi::xml_node node = root.first_child();
        node.attribute("name").set_value("G2");
        mm.AddModel(mm.CreateModel(node));
        CHECK(f.Id("G2") != kIdC);
        CHECK(f.Id("G") == kIdC);
    }
    SECTION("a submodel copied into another model") {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(LineXml("B").c_str()));
        Model* b = mm.createAndAddModel(doc.first_child(), 1000, 1000);
        auto* src = dynamic_cast<SubModel*>(f.M("A")->GetSubModel("Left"));
        REQUIRE(src != nullptr);
        auto* copy = new SubModel(b, src);
        b->AddSubmodel(copy);
        CHECK_FALSE(copy->GetObjectId().empty());
        CHECK(copy->GetObjectId() != kIdB);
    }
    SECTION("a view object duplicated through its XML") {
        ViewObjectManager& vom = f.ctx.AllObjects;
        pugi::xml_document vdoc;
        REQUIRE(vdoc.load_string(GridlinesXml("Grid", kIdD).c_str()));
        vom.AddViewObject(vom.CreateObject(vdoc.first_child()));
        REQUIRE(f.ObjId("Grid") == kIdD);
        vdoc.first_child().attribute("name").set_value("Grid2");
        vom.AddViewObject(vom.CreateObject(vdoc.first_child()));
        CHECK(f.ObjId("Grid2") != kIdD);
        CHECK(f.ObjId("Grid") == kIdD);
    }
}

TEST_CASE("Stripping ids from XML removes them at every level", "[layoutid]") {
    pugi::xml_document doc;
    REQUIRE(doc.load_string(LineXml("A", kIdA, SubXml("Left", kIdB)).c_str()));
    LayoutObjectId::StripFromXml(doc.first_child());
    CHECK_FALSE(HasIdAnywhere(doc.first_child()));
}

TEST_CASE("An exported model carries no id", "[layoutid]") {
    ShowFolder f;
    f.Load(ShowXml(LineXml("A", kIdA, SubXml("Left", kIdB)), GroupXml("G", "A", kIdC)));
    XmlSerializer s;
    pugi::xml_document exported = s.SerializeModel(f.M("A"), /*includeGroups*/ false, /*forExport*/ true);
    CHECK_FALSE(HasIdAnywhere(exported.document_element()));
    pugi::xml_document many = s.SerializeModels({ f.M("A"), f.M("G") }, false, true);
    CHECK_FALSE(HasIdAnywhere(many.document_element()));
    // the in-show form does carry them
    pugi::xml_document local = s.SerializeModel(f.M("A"));
    CHECK(HasIdAnywhere(local.document_element()));
}

TEST_CASE("Assigning ids at load does not mark controllers dirty", "[layoutid]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", NetworksXml(NullControllerXml("N1") + NullControllerXml("N2")));
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    CHECK_FALSE(om.GetController("N1")->GetObjectId().empty());
    CHECK_FALSE(om.IsDirty());
}

TEST_CASE("Controller ids survive a save and reload", "[layoutid]") {
    TempDir dir;
    dir.Write("xlights_networks.xml", NetworksXml(NullControllerXml("N1", kIdA) + NullControllerXml("N2") +
                                                  NullControllerXml("N3", kIdA)));
    OutputManager om;
    REQUIRE(om.Load(dir.path.string()));
    CHECK(om.GetController("N1")->GetObjectId() == kIdA);
    std::string n2 = om.GetController("N2")->GetObjectId();
    std::string n3 = om.GetController("N3")->GetObjectId();
    CHECK_FALSE(n2.empty());
    CHECK(n3 != kIdA); // the later repeat
    CHECK_FALSE(n3.empty());
    CHECK(om.GetControllerByObjectId(kIdA) == om.GetController("N1"));
    CHECK(om.GetControllerByObjectId("nope") == nullptr);

    REQUIRE(om.Save());
    OutputManager again;
    REQUIRE(again.Load(dir.path.string()));
    CHECK(again.GetController("N1")->GetObjectId() == kIdA);
    CHECK(again.GetController("N2")->GetObjectId() == n2);
    CHECK(again.GetController("N3")->GetObjectId() == n3);

    // a renamed controller keeps it
    again.GetController("N2")->SetName("Renamed");
    CHECK(again.GetController("Renamed")->GetObjectId() == n2);

    // a copy is a new record
    Controller* copy = again.GetController("N1")->Copy(&again);
    copy->SetName("Copy");
    again.AddController(copy);
    CHECK_FALSE(copy->GetObjectId().empty());
    CHECK(copy->GetObjectId() != kIdA);
}

TEST_CASE("Base show sync keeps the local controller id", "[layoutid]") {
    TempDir base;
    base.Write("xlights_networks.xml", NetworksXml(NullControllerXml("Shared", kIdB) + NullControllerXml("BaseOnly", kIdC)));
    TempDir local;
    local.Write("xlights_networks.xml",
                "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks BaseShowDir=\"" + base.path.string() + "\">" +
                    "<Controller Type=\"Null\" Id=\"64001\" Name=\"Shared\" FromBase=\"1\" xlId=\"" + kIdA +
                    "\"><network NetworkType=\"NULL\" MaxChannels=\"5\"/></Controller></Networks>");
    OutputManager om;
    REQUIRE(om.Load(local.path.string()));
    bool acceptAll = false, rejectAll = false, changed = false;
    REQUIRE(om.MergeFromBase(false, acceptAll, rejectAll, nullptr, &changed));
    CHECK(changed);
    CHECK(om.GetController("Shared")->GetChannels() == 10);
    CHECK(om.GetController("Shared")->GetObjectId() == kIdA);
    REQUIRE(om.GetController("BaseOnly") != nullptr);
    CHECK_FALSE(om.GetController("BaseOnly")->GetObjectId().empty());
    CHECK(om.GetController("BaseOnly")->GetObjectId() != kIdC);
}

TEST_CASE("Base show sync keeps local model ids and gives new models their own", "[layoutid]") {
    TempDir base;
    base.Write("xlights_rgbeffects.xml",
               ShowXml(LineXml("Shared", kIdB, SubXml("Left", kIdC) + SubXml("New", kIdD), "NodesPerString=\"20\"") + LineXml("BaseOnly", kIdC),
                       GroupXml("BG", "Shared", kIdD)));

    SECTION("in the file, before the models load") {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(ShowXml(LineXml("Shared", kIdA, SubXml("Left", "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa"), "FromBase=\"1\"")).c_str()));
        pugi::xml_node models = doc.document_element().child("models");
        pugi::xml_node groups = doc.document_element().child("modelGroups");
        bool changed = false;
        REQUIRE(ModelManager::MergeBaseXml(base.path.string(), models, groups, &changed));
        CHECK(changed);
        pugi::xml_node shared = models.find_child_by_attribute("model", "name", "Shared");
        CHECK(std::string(shared.attribute("NodesPerString").value()) == "20");
        CHECK(LayoutObjectId::Read(shared) == kIdA);
        CHECK(LayoutObjectId::Read(shared.find_child_by_attribute("subModel", "name", "Left")) == "aaaaaaaa-aaaa-4aaa-8aaa-aaaaaaaaaaaa");
        // a submodel the local copy never had is assigned one on load
        CHECK(LayoutObjectId::Read(shared.find_child_by_attribute("subModel", "name", "New")).empty());
        CHECK_FALSE(HasIdAnywhere(models.find_child_by_attribute("model", "name", "BaseOnly")));
        CHECK_FALSE(HasIdAnywhere(groups.find_child_by_attribute("modelGroup", "name", "BG")));
    }
    SECTION("an id alone is not a change") {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(ShowXml(LineXml("Shared", kIdA, SubXml("Left", kIdA) + SubXml("New"), "FromBase=\"1\" NodesPerString=\"20\"") +
                                        LineXml("BaseOnly", kIdD, "", "FromBase=\"1\""),
                                        GroupXml("BG", "Shared", kIdA) )
                                    .c_str()));
        pugi::xml_node models = doc.document_element().child("models");
        pugi::xml_node groups = doc.document_element().child("modelGroups");
        groups.first_child().append_attribute("FromBase") = "1";
        // the local copy wrote attributes in a different order; only ids differ from the base
        bool changed = false;
        REQUIRE(ModelManager::MergeBaseXml(base.path.string(), models, groups, &changed));
        CHECK_FALSE(changed);
    }
    SECTION("in memory, through MergeFromBase") {
        ShowFolder f;
        f.Load(ShowXml(LineXml("Shared", kIdA, SubXml("Left"), "FromBase=\"1\"")),
               "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<Networks BaseShowDir=\"" + base.path.string() + "\"/>");
        std::string left = f.M("Shared")->GetSubModel("Left")->GetObjectId();
        bool acceptAll = false, rejectAll = false, changed = false;
        REQUIRE(f.ctx.AllModels.MergeFromBase(base.path.string(), false, acceptAll, rejectAll, &changed));
        CHECK(changed);
        CHECK(f.M("Shared")->GetNodeCount() == 20);
        CHECK(f.Id("Shared") == kIdA);
        CHECK(f.M("Shared")->GetSubModel("Left")->GetObjectId() == left);
        CHECK_FALSE(f.M("Shared")->GetSubModel("New")->GetObjectId().empty());
        CHECK(f.M("Shared")->GetSubModel("New")->GetObjectId() != kIdD);
        CHECK_FALSE(f.Id("BaseOnly").empty());
        CHECK(f.Id("BaseOnly") != kIdC);
        CHECK(f.Id("BG") != kIdD);
    }
}

TEST_CASE("Base show sync keeps local view object ids", "[layoutid]") {
    TempDir base;
    base.Write("xlights_rgbeffects.xml",
               ShowXml("", "", "<view_object name=\"Grid\" DisplayAs=\"Gridlines\" GridLineSpacing=\"75\" xlId=\"" + std::string(kIdB) + "\"/>" +
                                   GridlinesXml("BaseGrid", kIdC)));
    pugi::xml_document doc;
    REQUIRE(doc.load_string(ShowXml("", "", "<view_object name=\"Grid\" DisplayAs=\"Gridlines\" GridLineSpacing=\"50\" FromBase=\"1\" xlId=\"" + std::string(kIdA) + "\"/>").c_str()));
    pugi::xml_node objects = doc.document_element().child("view_objects");
    bool changed = false;
    REQUIRE(ViewObjectManager::MergeBaseXml(base.path.string(), objects, &changed));
    CHECK(changed);
    pugi::xml_node grid = objects.find_child_by_attribute("view_object", "name", "Grid");
    CHECK(std::string(grid.attribute("GridLineSpacing").value()) == "75");
    CHECK(LayoutObjectId::Read(grid) == kIdA);
    CHECK(LayoutObjectId::Read(objects.find_child_by_attribute("view_object", "name", "BaseGrid")).empty());
}

TEST_CASE("Stamping writes in-memory ids into a patched layout file", "[layoutid]") {
    ShowFolder f;
    std::string file = ShowXml(LineXml("A", "", SubXml("Left")) + LineXml("B", kIdA), GroupXml("G", "A,B"),
                               GridlinesXml("Grid"), ViewXml("V", "A"),
                               "<layoutGroups><layoutGroup name=\"Front\"/><layoutGroup name=\"Back\" xlId=\"" + std::string(kIdB) +
                                   "\"/><layoutGroup name=\"Side\" xlId=\"" + std::string(kIdB) + "\"/></layoutGroups>"
                                   "<perspectives><perspective name=\"P1\"/><perspective name=\"P2\"/></perspectives>");
    f.Load(file);

    // What a load-modify-write host does: re-read the file and patch it. Here
    // nothing was edited at all, which is the case that must still get ids.
    pugi::xml_document doc;
    REQUIRE(doc.load_string(file.c_str()));
    pugi::xml_node root = doc.document_element();
    f.ctx.StampLayoutObjectIds(root, [](const std::string& name) {
        return name == "Front" ? std::string(kIdC) : std::string();
    });

    auto model = [&](const std::string& n) { return root.child("models").find_child_by_attribute("model", "name", n.c_str()); };
    CHECK(LayoutObjectId::Read(model("A")) == f.Id("A"));
    CHECK(LayoutObjectId::Read(model("B")) == kIdA);
    CHECK(LayoutObjectId::Read(model("A").child("subModel")) == f.M("A")->GetSubModel("Left")->GetObjectId());
    CHECK(LayoutObjectId::Read(root.child("modelGroups").child("modelGroup")) == f.Id("G"));
    CHECK(LayoutObjectId::Read(root.child("view_objects").child("view_object")) == f.ObjId("Grid"));
    CHECK(LayoutObjectId::Read(root.child("views").child("view")) == f.ViewId("V"));

    pugi::xml_node lgs = root.child("layoutGroups");
    CHECK(LayoutObjectId::Read(lgs.find_child_by_attribute("layoutGroup", "name", "Front")) == kIdC);
    CHECK(LayoutObjectId::Read(lgs.find_child_by_attribute("layoutGroup", "name", "Back")) == kIdB);
    std::string side = LayoutObjectId::Read(lgs.find_child_by_attribute("layoutGroup", "name", "Side"));
    CHECK_FALSE(side.empty());
    CHECK(side != kIdB);
    pugi::xml_node ps = root.child("perspectives");
    std::string p1 = LayoutObjectId::Read(ps.find_child_by_attribute("perspective", "name", "P1"));
    std::string p2 = LayoutObjectId::Read(ps.find_child_by_attribute("perspective", "name", "P2"));
    CHECK_FALSE(p1.empty());
    CHECK_FALSE(p2.empty());
    CHECK(p1 != p2);

    // A second stamp of a document that already has them changes nothing.
    std::ostringstream first;
    doc.save(first);
    f.ctx.StampLayoutObjectIds(root, [](const std::string& name) {
        return name == "Front" ? std::string(kIdC) : std::string();
    });
    std::ostringstream second;
    doc.save(second);
    CHECK(first.str() == second.str());
}

TEST_CASE("Claim keeps a usable id and replaces a missing or repeated one", "[layoutid]") {
    std::unordered_set<std::string> used;
    std::string a = kIdA;
    CHECK_FALSE(LayoutObjectId::Claim(a, used));
    CHECK(a == kIdA);
    std::string again = kIdA;
    CHECK(LayoutObjectId::Claim(again, used));
    CHECK(again != kIdA);
    std::string empty;
    CHECK(LayoutObjectId::Claim(empty, used));
    CHECK(empty.size() == 36);
    CHECK(used.size() == 3);
}
