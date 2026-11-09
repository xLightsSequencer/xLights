#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "import_export/ExportEffectsReport.h"
#include "import_export/ExportModels.h"
#include "models/Model.h"
#include "models/ModelGroup.h"
#include "models/ModelManager.h"
#include "outputs/Controller.h"
#include "outputs/OutputManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "test_impexp2_map_common.h"
#include "test_render_common.h"
#include "utils/ZipUtils.h"

#include <pugixml.hpp>

#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

using namespace renderhelp;

namespace {

std::string Slurp(const std::string& p) {
    std::ifstream f(p); // text mode: Windows writers emit CRLF
    std::stringstream ss;
    ss << f.rdbuf();
    return ss.str();
}

std::vector<std::string> Lines(const std::string& s) {
    std::vector<std::string> out;
    std::string cur;
    for (char c : s) {
        if (c == '\n') {
            out.push_back(cur);
            cur.clear();
        } else {
            cur += c;
        }
    }
    if (!cur.empty()) out.push_back(cur);
    return out;
}

// Minimal RFC4180 reader: quoted fields may hold commas, doubled quotes and newlines.
std::vector<std::vector<std::string>> ParseCsv(const std::string& s) {
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string field;
    bool inq = false;
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (inq) {
            if (c == '"') {
                if (i + 1 < s.size() && s[i + 1] == '"') {
                    field += '"';
                    ++i;
                } else {
                    inq = false;
                }
            } else {
                field += c;
            }
        } else if (c == '"') {
            inq = true;
        } else if (c == ',') {
            row.push_back(field);
            field.clear();
        } else if (c == '\n') {
            row.push_back(field);
            field.clear();
            rows.push_back(row);
            row.clear();
        } else {
            field += c;
        }
    }
    if (!field.empty() || !row.empty()) {
        row.push_back(field);
        rows.push_back(row);
    }
    return rows;
}

struct EffShow {
    std::unique_ptr<Show> show;
    TempDir out;
    explicit EffShow(const ShowSpec& s) : show(std::make_unique<Show>(s)) {}
    bool Export(const std::string& name = "o.csv") { return ExportEffects(out.File(name), show->ctx->GetSequenceElements(), show->ctx->AllModels); }
    std::string Text(const std::string& name = "o.csv") { return Slurp(out.File(name)); }
};

ShowSpec Spec(std::vector<EffectSpec> effects, const std::string& models = SingleLine("A", 1, 4) + SingleLine("B", 100, 4), std::vector<std::string> els = { "A", "B" }, const std::string& groups = "") {
    ShowSpec s;
    s.models = models;
    s.groups = groups;
    s.modelElements = std::move(els);
    s.effects = std::move(effects);
    s.durationMs = 200000;
    return s;
}

// ---- XLSX reading -------------------------------------------------------

using Sheet = std::map<std::string, std::string>;

struct Workbook {
    std::vector<std::string> shared;
    std::vector<Sheet> sheets;
    bool ok = false;
    explicit Workbook(const std::string& path) {
        auto ss = ZipUtils::ReadEntry(path, "xl/sharedStrings.xml");
        if (ss) {
            pugi::xml_document d;
            d.load_string(ss->c_str());
            for (auto si : d.child("sst").children("si")) {
                std::string t;
                for (auto tn : si.children("t")) t += tn.text().get();
                shared.push_back(t);
            }
        }
        for (int i = 1; i <= 4; ++i) {
            auto x = ZipUtils::ReadEntry(path, "xl/worksheets/sheet" + std::to_string(i) + ".xml");
            if (!x) return;
            pugi::xml_document d;
            d.load_string(x->c_str());
            Sheet sh;
            for (auto row : d.child("worksheet").child("sheetData").children("row")) {
                for (auto c : row.children("c")) {
                    std::string v = c.child("v").text().get();
                    if (std::string(c.attribute("t").value()) == "s") {
                        size_t idx = (size_t)std::strtoul(v.c_str(), nullptr, 10);
                        v = idx < shared.size() ? shared[idx] : "<bad>";
                    }
                    sh[c.attribute("r").value()] = v;
                }
            }
            sheets.push_back(sh);
        }
        ok = true;
    }
    const Sheet& Models() const { return sheets[0]; }
    const Sheet& Groups() const { return sheets[1]; }
    const Sheet& Controllers() const { return sheets[2]; }
    const Sheet& Totals() const { return sheets[3]; }
    // Value of a column named `header` on 1-based data row `row` (2 = first data row).
    static std::string Col(const Sheet& s, const std::string& header, int row) {
        for (char c = 'A'; c <= 'Z'; ++c) {
            auto it = s.find(std::string(1, c) + "1");
            if (it != s.end() && it->second == header) {
                auto v = s.find(std::string(1, c) + std::to_string(row));
                return v == s.end() ? "" : v->second;
            }
        }
        for (char c = 'A'; c <= 'A'; ++c) {
            for (char d = 'A'; d <= 'Z'; ++d) {
                std::string col = std::string(1, c) + d;
                auto it = s.find(col + "1");
                if (it != s.end() && it->second == header) {
                    auto v = s.find(col + std::to_string(row));
                    return v == s.end() ? "" : v->second;
                }
            }
        }
        return "<nocol>";
    }
    std::string Total(const std::string& label) const {
        for (int r = 1; r <= 12; ++r) {
            auto a = Totals().find("A" + std::to_string(r));
            if (a != Totals().end() && a->second == label) {
                auto b = Totals().find("B" + std::to_string(r));
                return b == Totals().end() ? "" : b->second;
            }
        }
        return "<nolabel>";
    }
};

struct ModelShow {
    OutputManager om;
    ModelManager mm{ &om };
    TempDir out;
    ModelShow() { EnsureLoggers(); }
    Model* Add(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Model* m = mm.createAndAddModel(doc.first_child(), 100, 100);
        REQUIRE(m != nullptr);
        return m;
    }
    Model* Line(const std::string& name, int nodes, const std::string& sc, const std::string& extra = "") {
        return Add("<model name=\"" + XmlEscape(name) + "\" DisplayAs=\"Single Line\" parm1=\"1\" parm2=\"" + std::to_string(nodes) + "\" parm3=\"1\" StringType=\"RGB Nodes\" StartChannel=\"" + sc + "\" " + extra + "/>");
    }
    Controller* Ctl(const std::string& xml) {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(xml.c_str()));
        Controller* c = Controller::Create(&om, doc.first_child(), "");
        REQUIRE(c != nullptr);
        om.AddController(c);
        om.SomethingChanged();
        return c;
    }
    void Recalc() {
        om.SomethingChanged();
        mm.RecalcStartChannels();
    }
    bool Export(const std::string& name = "m.xlsx") { return ExportModels(out.File(name), mm, om); }
};

} // namespace

// ==========================================================================
// ExportEffects (CSV)
// ==========================================================================

TEST_CASE("ExportEffects of an empty sequence writes only headers and totals", "[impexp2][export]") {
    ShowSpec s = Spec({}, SingleLine("A", 1, 4), {});
    EffShow e(s);
    REQUIRE(e.Export());
    CHECK(e.Text() ==
          "Effect Name,StartTime,EndTime,Duration,Description,Element,ElementType,Files\n"
          "\"Effect Count\",0\n"
          "\n"
          "Effect Usage Summary\n"
          "Effect Name,Occurences,TotalTime\n"
          "\n"
          "Summary of files used\n");
}

TEST_CASE("ExportEffects writes one row per effect with formatted times", "[impexp2][export]") {
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }));
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() >= 3);
    CHECK(rows[0] == std::vector<std::string>{ "Effect Name", "StartTime", "EndTime", "Duration", "Description", "Element", "ElementType", "Files" });
    CHECK(rows[1] == std::vector<std::string>{ "On", "00:00.000", "00:01.000", "00:01.000", "", "A", "Model", "" });
    CHECK(rows[2] == std::vector<std::string>{ "Effect Count", "1" });
}

TEST_CASE("ExportEffects time formatting", "[impexp2][export]") {
    auto [start, end, s1, s2, s3] = GENERATE(table<int, int, std::string, std::string, std::string>({
        { 0, 50, "00:00.000", "00:00.050", "00:00.050" },
        { 950, 1050, "00:00.950", "00:01.050", "00:00.100" },
        { 59900, 60000, "00:59.900", "01:00.000", "00:00.100" },
        { 60000, 125050, "01:00.000", "02:05.050", "01:05.050" },
        { 3600000, 3600050, "60:00.000", "60:00.050", "00:00.050" },
        { 5999950, 6000000, "99:59.950", "100:00.000", "00:00.050" },
    }));
    ShowSpec sp = Spec({ Eff("A", 0, "On", start, end, Pal("#FF0000")) });
    sp.durationMs = 7000000;
    EffShow e(sp);
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() > 1);
    CHECK(rows[1][1] == s1);
    CHECK(rows[1][2] == s2);
    CHECK(rows[1][3] == s3);
}

TEST_CASE("ExportEffects usage summary counts and sums by effect name sorted by name", "[impexp2][export]") {
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("A", 0, "Bars", 1000, 3000, Pal("#FF0000")), Eff("B", 0, "On", 0, 500, Pal("#FF0000")), Eff("B", 1, "Bars", 0, 250, Pal("#FF0000")), Eff("B", 1, "Fire", 500, 600, Pal("#FF0000")) }));
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    size_t i = 0;
    while (i < rows.size() && (rows[i].empty() || rows[i][0] != "Effect Name" || rows[i].size() != 3)) ++i;
    REQUIRE(i + 3 < rows.size());
    CHECK(rows[i + 1] == std::vector<std::string>{ "Bars", "2", "00:02.250" });
    CHECK(rows[i + 2] == std::vector<std::string>{ "Fire", "1", "00:00.100" });
    CHECK(rows[i + 3] == std::vector<std::string>{ "On", "2", "00:01.500" });
    bool found = false;
    for (auto& r : rows) {
        if (r.size() == 2 && r[0] == "Effect Count") {
            CHECK(r[1] == "5");
            found = true;
        }
    }
    CHECK(found);
}

TEST_CASE("ExportEffects classifies elements", "[impexp2][export]") {
    ShowSpec s = Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")), Eff("G", 0, "Bars", 0, 1000, Pal("#FF0000")) }, SingleLine("A", 1, 4) + SingleLine("B", 100, 4), { "A", "B", "G" },
                      R"(<modelGroup name="G" models="A,B" layout="minimalGrid" GridSize="400"/>)");
    EffShow e(s);
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    std::map<std::string, std::string> type;
    for (size_t i = 1; i < rows.size() && rows[i].size() == 8; ++i) type[rows[i][5]] = rows[i][6];
    CHECK(type["A"] == "Model");
    CHECK(type["G"] == "Model Group");
}

TEST_CASE("ExportEffects excludes timing tracks", "[impexp2][export]") {
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }));
    auto& se = e.show->ctx->GetSequenceElements();
    auto* t = se.AddElement("Beats", "timing", true, false, true, false, false);
    auto* l = t->AddEffectLayer();
    l->AddEffect(0, "Beat", "", "", 0, 500, false, false);
    REQUIRE(e.Export());
    auto text = e.Text();
    CHECK(text.find("Beats") == std::string::npos);
    CHECK(text.find("\"Beat\"") == std::string::npos);
}

TEST_CASE("ExportEffects reports submodel and strand elements", "[impexp2][export]") {
    ShowSpec s = Spec({ Eff("M", 0, "On", 0, 1000, Pal("#FF0000")) }, Matrix("M", 1, 3, 4), { "M" });
    EffShow e(s);
    auto* me = dynamic_cast<ModelElement*>(e.show->ctx->GetSequenceElements().GetElement("M"));
    REQUIRE(me != nullptr);
    auto* strand = me->GetStrand(1, true);
    REQUIRE(strand != nullptr);
    strand->GetEffectLayer(0)->AddEffect(0, "Bars", "", Pal("#FF0000"), 0, 2000, false, false);
    auto* nl = strand->GetNodeLayer(2, true);
    REQUIRE(nl != nullptr);
    nl->AddEffect(0, "Fire", "", Pal("#FF0000"), 1000, 1500, false, false);
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    std::map<std::string, std::string> byEffect;
    for (size_t i = 1; i < rows.size() && rows[i].size() == 8; ++i) byEffect[rows[i][0]] = rows[i][5] + "|" + rows[i][6];
    CHECK(byEffect["On"] == "M|Model");
    CHECK(byEffect["Bars"].substr(byEffect["Bars"].find('|')) == "|Strand");
    CHECK(byEffect["Fire"].substr(byEffect["Fire"].find('|')) == "|Node");
    CHECK(byEffect["Fire"].find("M/Strand 2/") == 0);
}

TEST_CASE("ExportEffects lists referenced files once and sorted", "[impexp2][export]") {
    ShowSpec s = Spec({ Eff("A", 0, "Pictures", 0, 1000, Pal("#FF0000"), "E_TEXTCTRL_Pictures_Filename=zeta.png"),
                        Eff("A", 0, "Pictures", 1000, 2000, Pal("#FF0000"), "E_TEXTCTRL_Pictures_Filename=alpha.png"),
                        Eff("B", 0, "Pictures", 0, 1000, Pal("#FF0000"), "E_TEXTCTRL_Pictures_Filename=zeta.png") });
    EffShow e(s);
    REQUIRE(e.Export());
    auto lines = Lines(e.Text());
    size_t idx = 0;
    while (idx < lines.size() && lines[idx] != "Summary of files used") ++idx;
    REQUIRE(idx < lines.size());
    std::vector<std::string> files(lines.begin() + idx + 1, lines.end());
    REQUIRE(files.size() == 2);
    CHECK(files[0].find("alpha.png") != std::string::npos);
    CHECK(files[1].find("zeta.png") != std::string::npos);
}

TEST_CASE("ExportEffects effect description column", "[impexp2][export]") {
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000"), "X_Effect_Description=Nice glow") }));
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    CHECK(rows[1][4] == "Nice glow");
}

TEST_CASE("ExportEffects description with a double quote stays one well formed CSV field", "[impexp2][export]") {
    // ExportEffectsReport.cpp WriteEffectRow: description/element name are written inside "%s" without doubling embedded quotes.
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }));
    e.show->ctx->GetSequenceElements().GetElement("A")->GetEffectLayer(0)->GetEffect(0)->GetSettings()["X_Effect_Description"] = "say \"hi\"";
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() > 1);
    CHECK(rows[1].size() == 8);
    CHECK(rows[1][4] == "say \"hi\"");
}

TEST_CASE("ExportEffects element name with a comma is quoted", "[impexp2][export]") {
    ShowSpec s = Spec({ Eff("Arch, left", 0, "On", 0, 1000, Pal("#FF0000")) }, SingleLine("Arch, left", 1, 4), { "Arch, left" });
    EffShow e(s);
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() > 1);
    CHECK(rows[1].size() == 8);
    CHECK(rows[1][5] == "Arch, left");
}

TEST_CASE("ExportEffects unicode element names", "[impexp2][export]") {
    ShowSpec s = Spec({ Eff("Árbol 日本", 0, "On", 0, 1000, Pal("#FF0000")) }, SingleLine("Árbol 日本", 1, 4), { "Árbol 日本" });
    EffShow e(s);
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() > 1);
    CHECK(rows[1][5] == "Árbol 日本");
}

TEST_CASE("ExportEffects file path containing a comma stays one field", "[impexp2][export]") {
    // Files column is written unquoted, so a path with a comma splits into extra columns.
    ShowSpec s = Spec({ Eff("A", 0, "Pictures", 0, 1000, Pal("#FF0000"), "E_TEXTCTRL_Pictures_Filename=a\\,b.png") });
    EffShow e(s);
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() > 1);
    CHECK(rows[1].size() == 8);
}

TEST_CASE("ExportEffects fails cleanly for unwritable paths", "[impexp2][export]") {
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }));
    auto bad = GENERATE(as<std::string>{}, "", "no_such_dir/sub/file.csv");
    CHECK_FALSE(ExportEffects(e.out.File(bad), e.show->ctx->GetSequenceElements(), e.show->ctx->AllModels));
    CHECK_FALSE(ExportEffects(e.out.path.string(), e.show->ctx->GetSequenceElements(), e.show->ctx->AllModels));
}

TEST_CASE("ExportEffects overwrites an existing report", "[impexp2][export]") {
    EffShow e(Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }));
    e.out.Write("o.csv", std::string(5000, 'x'));
    REQUIRE(e.Export());
    CHECK(e.Text().find("xxxx") == std::string::npos);
}

TEST_CASE("ExportEffects handles many effects", "[impexp2][export]") {
    std::vector<EffectSpec> v;
    for (int i = 0; i < 500; ++i) v.push_back(Eff("A", i % 3, "On", i * 50, i * 50 + 50, Pal("#FF0000")));
    EffShow e(Spec(v));
    REQUIRE(e.Export());
    auto rows = ParseCsv(e.Text());
    int count = 0;
    for (auto& r : rows) {
        if (r.size() == 8 && r[0] == "On") ++count;
    }
    CHECK(count == 500);
}

TEST_CASE("ExportEffects effect on a model absent from the layout is still reported", "[impexp2][export]") {
    ShowSpec s = Spec({ Eff("A", 0, "On", 0, 1000, Pal("#FF0000")) }, SingleLine("A", 1, 4), { "A" });
    EffShow e(s);
    ModelManager empty(e.show->ctx->GetOutputManager());
    REQUIRE(ExportEffects(e.out.File("o.csv"), e.show->ctx->GetSequenceElements(), empty));
    auto rows = ParseCsv(e.Text());
    REQUIRE(rows.size() > 1);
    CHECK(rows[1][0] == "On");
    CHECK(rows[1][6] == "Model");
}

// ==========================================================================
// ExportModels (XLSX)
// ==========================================================================

TEST_CASE("ExportModels with nothing to export produces a four sheet workbook", "[impexp2][export][xlsx]") {
    ModelShow s;
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Models().at("A1") == "Model Name");
    CHECK(wb.Groups().at("A1") == "Group Name");
    CHECK(wb.Controllers().at("A1") == "Controller Name");
    CHECK(wb.Models().count("A2") == 0);
    CHECK(wb.Total("Model Count") == "0");
    CHECK(wb.Total("Group Count") == "0");
    CHECK(wb.Total("First Used Channel") == "0");
    CHECK(wb.Total("Last Used Channel") == "0");
    CHECK(wb.Total("Actual Used Channel") == "0");
    CHECK(wb.Total("Bulbs") == "0");
    CHECK(wb.Total("DMX Channels") == "0");
}

TEST_CASE("ExportModels model sheet fields", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("Mega Tree", 50, "1", "Description=\"A big one\"");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    const auto& m = wb.Models();
    CHECK(Workbook::Col(m, "Model Name", 2) == "Mega Tree");
    CHECK(Workbook::Col(m, "Display As", 2) == "Single Line");
    CHECK(Workbook::Col(m, "String Type", 2) == "RGB Nodes");
    CHECK(Workbook::Col(m, "Node Count", 2) == "50");
    CHECK(Workbook::Col(m, "Light Count", 2) == "50");
    CHECK(Workbook::Col(m, "Channels Per Node", 2) == "3");
    CHECK(Workbook::Col(m, "Channel Count", 2) == "150");
    CHECK(Workbook::Col(m, "Start Channel", 2) == "1");
    CHECK(Workbook::Col(m, "Start Channel No", 2) == "1");
    CHECK(Workbook::Col(m, "End Channel No", 2) == "150");
    CHECK(Workbook::Col(m, "String Count", 2) == "1");
    CHECK(Workbook::Col(m, "Controller Name", 2) == "");
    CHECK(Workbook::Col(m, "Controller Type", 2) == "Unknown");
    CHECK(Workbook::Col(m, "Protocol", 2) == "Unknown");
    CHECK(Workbook::Col(m, "Controller Channel", 2) == "-1");
}

TEST_CASE("ExportModels totals for several models", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("A", 10, "1");
    s.Line("B", 20, "101");
    s.Line("C", 5, "301");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Total("Model Count") == "3");
    CHECK(wb.Total("First Used Channel") == "1");
    CHECK(wb.Total("Last Used Channel") == "315");
    CHECK(wb.Total("Actual Used Channel") == std::to_string(30 + 60 + 15));
    CHECK(wb.Total("Model Channels (incl. overlap)") == std::to_string(30 + 60 + 15));
    CHECK(wb.Total("Bulbs") == "35");
}

TEST_CASE("ExportModels does not double count overlapping models", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("A", 10, "1");
    s.Line("Shadow", 10, "1");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Total("Model Count") == "2");
    CHECK(wb.Total("Actual Used Channel") == "30");
    CHECK(wb.Total("Model Channels (incl. overlap)") == "60");
    CHECK(wb.Total("Bulbs") == "10");
}

TEST_CASE("ExportModels DMX models contribute channels but not bulbs", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Add(R"(<model name="D" DisplayAs="DmxGeneral" StartChannel="1" StringType="Single Color White" DmxChannelCount="5"/>)");
    s.Line("A", 4, "10");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Total("DMX Channels") == "5");
    CHECK(wb.Total("Bulbs") == "4");
    CHECK(wb.Total("Model Count") == "2");
}

TEST_CASE("ExportModels groups sheet", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("A", 4, "1");
    s.Line("B", 4, "101");
    s.Add(R"(<modelGroup name="Both" models="A,B" layout="minimalGrid" GridSize="400"/>)");
    s.Add(R"(<modelGroup name="Empty" models="" layout="minimalGrid" GridSize="400"/>)");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Total("Model Count") == "2");
    CHECK(wb.Total("Group Count") == "2");
    std::map<std::string, std::pair<std::string, std::string>> groups;
    for (int r = 2; r <= 3; ++r) {
        groups[Workbook::Col(wb.Groups(), "Group Name", r)] = { Workbook::Col(wb.Groups(), "Models", r), Workbook::Col(wb.Groups(), "Models Count", r) };
    }
    REQUIRE(groups.count("Both") == 1);
    CHECK(groups["Both"].first == "A, B");
    CHECK(groups["Both"].second == "2");
    REQUIRE(groups.count("Empty") == 1);
    CHECK(groups["Empty"].first == "");
    CHECK(groups["Empty"].second == "0");
}

TEST_CASE("ExportModels keeps awkward model names intact", "[impexp2][export][xlsx]") {
    auto name = GENERATE(as<std::string>{}, "Plain", "With, comma", "With \"quotes\"", "Amp & <angle> 'apos'", "Árbol 日本語 ☃", "Multi word  double space", "slash/name", "=SUM(A1)");
    ModelShow s;
    s.Line(name, 3, "1");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CAPTURE(name);
    CHECK(Workbook::Col(wb.Models(), "Model Name", 2) == name);
}

TEST_CASE("ExportModels name with a newline survives", "[impexp2][export][xlsx]") {
    ModelShow s;
    auto* m = s.Line("Temp", 3, "1");
    m->SetName("Line1\nLine2");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(Workbook::Col(wb.Models(), "Model Name", 2) == "Line1\nLine2");
}

TEST_CASE("ExportModels aliases are joined with commas", "[impexp2][export][xlsx]") {
    ModelShow s;
    auto* m = s.Line("A", 3, "1");
    m->AddAlias("Alpha");
    m->AddAlias("Beta, with comma");
    s.Line("B", 3, "101");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    std::map<std::string, std::string> aliases;
    for (int r = 2; r <= 3; ++r) aliases[Workbook::Col(wb.Models(), "Model Name", r)] = Workbook::Col(wb.Models(), "Aliases", r);
    CHECK(aliases["A"] == "alpha, beta, with comma");
    CHECK(aliases["B"] == "");
}

TEST_CASE("ExportModels with controllers fills controller columns", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Ctl(R"(<Controller Id="1" Name="Ctl One" Type="Ethernet" Protocol="E131" IP="10.0.0.20" ActiveState="Active">
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="5" MaxChannels="510"/>
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="6" MaxChannels="510"/></Controller>)");
    s.Line("A", 10, "1");
    s.Line("B", 10, "600");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    std::map<std::string, int> rowOf;
    for (int r = 2; r <= 3; ++r) rowOf[Workbook::Col(wb.Models(), "Model Name", r)] = r;
    int ra = rowOf.at("A");
    int rb = rowOf.at("B");
    CHECK(Workbook::Col(wb.Models(), "Controller Name", ra) == "Ctl One");
    CHECK(Workbook::Col(wb.Models(), "IP", ra) == "10.0.0.20");
    CHECK(Workbook::Col(wb.Models(), "Universe/Id", ra) == "5");
    CHECK(Workbook::Col(wb.Models(), "Controller Channel", ra) == "1");
    CHECK(Workbook::Col(wb.Models(), "#Universe(or id):Start Channel", ra) == "#5:1");
    CHECK(Workbook::Col(wb.Models(), "Universe/Id", rb) == "6");
    CHECK(Workbook::Col(wb.Models(), "#Universe(or id):Start Channel", rb) == "#6:90");
    CHECK(Workbook::Col(wb.Controllers(), "Controller Name", 2) == "Ctl One");
    CHECK(Workbook::Col(wb.Controllers(), "IP", 2) == "10.0.0.20");
}

TEST_CASE("ExportModels marks models on inactive controllers", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Ctl(R"(<Controller Id="1" Name="Off" Type="Ethernet" Protocol="E131" IP="10.0.0.20" ActiveState="Inactive">
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="5" MaxChannels="510"/></Controller>)");
    s.Line("A", 10, "1");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(Workbook::Col(wb.Models(), "Active", 2) == "DISABLED");
}

TEST_CASE("ExportModels with controller name that needs escaping", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Ctl(R"(<Controller Id="1" Name="A&amp;B &lt;1&gt;" Type="Ethernet" Protocol="E131" IP="10.0.0.20" ActiveState="Active">
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="5" MaxChannels="510"/></Controller>)");
    s.Line("A", 10, "1");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(Workbook::Col(wb.Models(), "Controller Name", 2) == "A&B <1>");
}

TEST_CASE("ExportModels with controllers but no models", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Ctl(R"(<Controller Id="1" Name="Only" Type="Ethernet" Protocol="E131" IP="10.0.0.20" ActiveState="Active">
        <network NetworkType="E131" ComPort="10.0.0.20" BaudRate="5" MaxChannels="510"/></Controller>)");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(Workbook::Col(wb.Controllers(), "Controller Name", 2) == "Only");
    CHECK(wb.Total("Model Count") == "0");
}

TEST_CASE("ExportModels with many models", "[impexp2][export][xlsx]") {
    ModelShow s;
    for (int i = 0; i < 150; ++i) s.Line("M" + std::to_string(i), 5, std::to_string(i * 20 + 1));
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Total("Model Count") == "150");
    CHECK(wb.Total("Bulbs") == "750");
    CHECK(wb.Total("Actual Used Channel") == "2250");
    CHECK(wb.Total("Last Used Channel") == std::to_string(149 * 20 + 15));
}

TEST_CASE("ExportModels very long text does not corrupt the workbook", "[impexp2][export][xlsx]") {
    ModelShow s;
    auto* m = s.Line("A", 3, "1");
    m->description = std::string(40000, 'd');
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(Workbook::Col(wb.Models(), "Model Name", 2) == "A");
}

TEST_CASE("ExportModels fails for an unwritable destination", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("A", 3, "1");
    s.Recalc();
    CHECK_FALSE(ExportModels(s.out.File("no_such_dir/sub/m.xlsx"), s.mm, s.om));
    CHECK_FALSE(ExportModels(s.out.path.string(), s.mm, s.om));
}

TEST_CASE("ExportModels overwrites an existing file", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("A", 3, "1");
    s.Recalc();
    s.out.Write("m.xlsx", "garbage");
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    CHECK(wb.ok);
}

TEST_CASE("ExportModels model starting at a very high channel", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("Far", 4, "1000000");
    s.Line("Near", 4, "1");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(wb.Total("First Used Channel") == "1");
    CHECK(wb.Total("Last Used Channel") == "1000011");
    CHECK(wb.Total("Actual Used Channel") == "24");
}

TEST_CASE("ExportModels dimensions and buffer size columns", "[impexp2][export][xlsx]") {
    ModelShow s;
    s.Line("A", 12, "1");
    s.Recalc();
    REQUIRE(s.Export());
    Workbook wb(s.out.File("m.xlsx"));
    REQUIRE(wb.ok);
    CHECK(Workbook::Col(wb.Models(), "Default Buffer W x H", 2) == "12 x 1");
}
