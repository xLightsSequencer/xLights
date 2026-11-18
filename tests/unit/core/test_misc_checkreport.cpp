#include <catch2/catch_test_macros.hpp>

#include <string>
#include <vector>

#include "diagnostics/CheckSequenceReport.h"

using Issue = CheckSequenceReport::ReportIssue;
using Section = CheckSequenceReport::ReportSection;

namespace {

CheckSequenceReport Standard() {
    CheckSequenceReport r;
    for (auto const& s : CheckSequenceReport::REPORT_SECTIONS) {
        r.AddSection(s);
    }
    return r;
}

size_t Count(const std::string& hay, const std::string& needle) {
    size_t n = 0;
    for (size_t p = hay.find(needle); p != std::string::npos; p = hay.find(needle, p + needle.size())) {
        ++n;
    }
    return n;
}

} // namespace

TEST_CASE("CheckSequenceReport defines the six standard sections", "[misc][diagnostics]") {
    auto const& s = CheckSequenceReport::REPORT_SECTIONS;
    REQUIRE(s.size() == 6);
    std::vector<std::string> ids;
    for (auto const& x : s) {
        ids.push_back(x.id);
        CHECK_FALSE(x.title.empty());
        CHECK_FALSE(x.icon.empty());
        CHECK_FALSE(x.description.empty());
    }
    std::vector<std::string> expect = { "network", "preferences", "controllers", "models", "sequence", "os" };
    CHECK(ids == expect);
}

TEST_CASE("CheckSequenceReport counts errors and warnings but not info", "[misc][diagnostics]") {
    auto r = Standard();
    CHECK(r.GetTotalErrors() == 0);
    CHECK(r.GetTotalWarnings() == 0);
    r.AddIssue("models", Issue(Issue::CRITICAL, "bad", "x"));
    r.AddIssue("models", Issue(Issue::CRITICAL, "bad2"));
    r.AddIssue("sequence", Issue(Issue::WARNING, "meh"));
    r.AddIssue("sequence", Issue(Issue::INFO, "fyi"));
    CHECK(r.GetTotalErrors() == 2);
    CHECK(r.GetTotalWarnings() == 1);
    auto const& secs = r.GetSections();
    REQUIRE(secs.size() == 6);
    CHECK(secs[3].errorCount == 2);
    CHECK(secs[3].issues.size() == 2);
    CHECK(secs[4].warningCount == 1);
    CHECK(secs[4].issues.size() == 2);
}

TEST_CASE("CheckSequenceReport drops issues for unknown sections", "[misc][diagnostics]") {
    auto r = Standard();
    r.AddIssue("nonexistent", Issue(Issue::CRITICAL, "lost"));
    r.AddIssue("", Issue(Issue::CRITICAL, "lost"));
    CHECK(r.GetTotalErrors() == 0);
    for (auto const& s : r.GetSections()) {
        CHECK(s.issues.empty());
    }
}

TEST_CASE("CheckSequenceReport with no sections ignores issues", "[misc][diagnostics]") {
    CheckSequenceReport r;
    r.AddIssue("models", Issue(Issue::CRITICAL, "lost"));
    CHECK(r.GetSections().empty());
    CHECK(r.GetTotalErrors() == 0);
}

TEST_CASE("CheckSequenceReport keeps counts for sections added with prefilled counters", "[misc][diagnostics]") {
    CheckSequenceReport r;
    Section s("custom", "i", "Custom", "d");
    s.errorCount = 3;
    s.warningCount = 4;
    r.AddSection(s);
    CHECK(r.GetTotalErrors() == 3);
    CHECK(r.GetTotalWarnings() == 4);
    r.AddIssue("custom", Issue(Issue::WARNING, "w"));
    CHECK(r.GetTotalWarnings() == 5);
}

TEST_CASE("CheckSequenceReport issue builders carry location data", "[misc][diagnostics]") {
    auto e = Issue::ForEffect(Issue::WARNING, "msg", "video", "Tree", "Bars", 1500, 2);
    CHECK(e.type == Issue::WARNING);
    CHECK(e.message == "msg");
    CHECK(e.category == "video");
    CHECK(e.modelName == "Tree");
    CHECK(e.effectName == "Bars");
    CHECK(e.startTimeMS == 1500);
    CHECK(e.layerIndex == 2);

    auto d = Issue::ForEffect(Issue::INFO, "m", "c", "M", "E", 0);
    CHECK(d.layerIndex == -1);

    auto m = Issue::ForModel(Issue::CRITICAL, "m", "c", "Arch");
    CHECK(m.modelName == "Arch");
    CHECK(m.effectName.empty());
    CHECK(m.startTimeMS == -1);
    CHECK(m.layerIndex == -1);

    Issue plain(Issue::INFO, "x");
    CHECK(plain.category.empty());
    CHECK(plain.startTimeMS == -1);
}

TEST_CASE("CheckSequenceReport stores paths and a generated timestamp", "[misc][diagnostics]") {
    CheckSequenceReport r;
    r.SetShowFolder("/shows/xmas");
    r.SetSequencePath("/shows/xmas/a.xsq");
    CHECK(r.GetShowFolder() == "/shows/xmas");
    CHECK(r.GetSequencePath() == "/shows/xmas/a.xsq");
    auto t = r.GetGeneratedTime();
    REQUIRE(t.size() == 19);
    CHECK(t[4] == '-');
    CHECK(t[7] == '-');
    CHECK(t[10] == ' ');
    CHECK(t[13] == ':');
    CHECK(t[16] == ':');
}

TEST_CASE("CheckSequenceReport HTML is a complete document", "[misc][diagnostics]") {
    auto r = Standard();
    auto html = r.GenerateHTML();
    CHECK(html.find("<!DOCTYPE html>") != std::string::npos);
    CHECK(html.find("</html>") != std::string::npos);
    CHECK(html.find("xLights Show Status Report") != std::string::npos);
    CHECK(html.find("checksequence_tailwind.min.css") != std::string::npos);
    CHECK(Count(html, "<script>") == Count(html, "</script>"));
    for (auto const& s : CheckSequenceReport::REPORT_SECTIONS) {
        CHECK(html.find("id=\"section-" + s.id + "\"") != std::string::npos);
        CHECK(html.find(s.title) != std::string::npos);
    }
}

TEST_CASE("CheckSequenceReport HTML header shows totals and paths", "[misc][diagnostics]") {
    auto r = Standard();
    r.SetShowFolder("/shows/xmas");
    r.SetSequencePath("/shows/xmas/a.xsq");
    r.AddIssue("models", Issue(Issue::CRITICAL, "ERR: broken"));
    r.AddIssue("sequence", Issue(Issue::WARNING, "WARN: odd"));
    r.AddIssue("sequence", Issue(Issue::WARNING, "WARN: odd2"));
    auto html = r.GenerateHTML();
    CHECK(html.find("/shows/xmas") != std::string::npos);
    CHECK(html.find("/shows/xmas/a.xsq") != std::string::npos);
    CHECK(html.find("<div class=\"text-4xl font-bold text-red-600\">1</div>") != std::string::npos);
    CHECK(html.find("<div class=\"text-4xl font-bold text-yellow-600\">2</div>") != std::string::npos);
    CHECK(html.find("1 Errors") != std::string::npos);
    CHECK(html.find("2 Warnings") != std::string::npos);
}

TEST_CASE("CheckSequenceReport HTML notes a missing sequence", "[misc][diagnostics]") {
    auto r = Standard();
    auto html = r.GenerateHTML();
    CHECK(html.find("No sequence was loaded when the report ran") != std::string::npos);
    r.SetSequencePath("x.xsq");
    CHECK(r.GenerateHTML().find("No sequence was loaded when the report ran") == std::string::npos);
}

TEST_CASE("CheckSequenceReport HTML escapes the folder and sequence paths", "[misc][diagnostics]") {
    auto r = Standard();
    r.SetShowFolder("C:\\Shows\\<b>&\"'");
    r.SetSequencePath("<script>alert(1)</script>.xsq");
    auto html = r.GenerateHTML();
    CHECK(html.find("<b>&\"'") == std::string::npos);
    CHECK(html.find("&lt;b&gt;&amp;&quot;&#39;") != std::string::npos);
    CHECK(html.find("<script>alert(1)</script>") == std::string::npos);
    CHECK(html.find("&lt;script&gt;alert(1)&lt;/script&gt;.xsq") != std::string::npos);
}

TEST_CASE("CheckSequenceReport HTML filter bar badges reflect section counts", "[misc][diagnostics]") {
    auto r = Standard();
    CHECK(r.GenerateHTML().find("toc-badge toc-badge-error") == std::string::npos);
    r.AddIssue("controllers", Issue(Issue::CRITICAL, "ERR: c"));
    auto html = r.GenerateHTML();
    CHECK(html.find("<span class=\"toc-badge toc-badge-error\">1</span>") != std::string::npos);
}

TEST_CASE("CheckSequenceReport HTML shows issue messages without their severity prefix", "[misc][diagnostics]") {
    auto r = Standard();
    r.AddIssue("preferences", Issue(Issue::CRITICAL, "ERR: Backup is disabled", "backup"));
    r.AddIssue("preferences", Issue(Issue::WARNING, "WARN: Low disk space", "rendering"));
    r.AddIssue("preferences", Issue(Issue::WARNING, "   WARN: Indented warning", "autosave"));
    auto html = r.GenerateHTML();
    CHECK(html.find("Backup is disabled") != std::string::npos);
    CHECK(html.find("ERR: Backup is disabled") == std::string::npos);
    CHECK(html.find("Low disk space") != std::string::npos);
    CHECK(html.find("WARN: Low disk space") == std::string::npos);
    CHECK(html.find("Indented warning") != std::string::npos);
    CHECK(html.find("WARN: Indented warning") == std::string::npos);
}

TEST_CASE("CheckSequenceReport HTML renders every issue type in every section without trouble", "[misc][diagnostics]") {
    auto r = Standard();
    for (auto const& s : CheckSequenceReport::REPORT_SECTIONS) {
        r.AddIssue(s.id, Issue(Issue::CRITICAL, "ERR: something broke", "misc"));
        r.AddIssue(s.id, Issue(Issue::WARNING, "WARN: something odd", "misc"));
        r.AddIssue(s.id, Issue(Issue::INFO, "INFO: fyi", "checkdisabled"));
        r.AddIssue(s.id, Issue(Issue::WARNING, ""));
        r.AddIssue(s.id, Issue(Issue::CRITICAL, std::string(20000, 'x'), "weird"));
    }
    auto html = r.GenerateHTML(true);
    CHECK(html.size() > 10000);
    CHECK(r.GetTotalErrors() == 12);
    CHECK(r.GetTotalWarnings() == 12);
}

TEST_CASE("CheckSequenceReport HTML handles an empty report with no sections", "[misc][diagnostics]") {
    CheckSequenceReport r;
    auto html = r.GenerateHTML(false);
    CHECK(html.find("</html>") != std::string::npos);
    CHECK(html.find("id=\"section-") == std::string::npos);
}

TEST_CASE("CheckSequenceReport reports no issues for a clean section", "[misc][diagnostics]") {
    auto r = Standard();
    auto html = r.GenerateHTML();
    CHECK(html.find("No ") != std::string::npos);
    CHECK(html.find(" found") != std::string::npos);
}

TEST_CASE("CheckSequenceReport duplicate section ids point at the latest one", "[misc][diagnostics]") {
    CheckSequenceReport r;
    r.AddSection(Section("a", "i", "First", "d"));
    r.AddSection(Section("a", "i", "Second", "d"));
    r.AddIssue("a", Issue(Issue::CRITICAL, "x"));
    REQUIRE(r.GetSections().size() == 2);
    CHECK(r.GetSections()[0].errorCount == 0);
    CHECK(r.GetSections()[1].errorCount == 1);
    CHECK(r.GetTotalErrors() == 1);
}

// Issue messages go into the page through CleanMessage only, so markup in a message (a model named "<b>") is not escaped.
TEST_CASE("CheckSequenceReport HTML escapes markup inside issue messages", "[misc][diagnostics]") {
    auto r = Standard();
    r.AddIssue("preferences", Issue(Issue::CRITICAL, "ERR: path <img src=x onerror=alert(1)>", "backup"));
    auto html = r.GenerateHTML();
    CHECK(html.find("<img src=x onerror=alert(1)>") == std::string::npos);
}

TEST_CASE("CheckSequenceReport preference issues only show for their categories", "[misc][diagnostics]") {
    auto r = Standard();
    r.AddIssue("preferences", Issue(Issue::CRITICAL, "ERR: hidden message", "unknowncategory"));
    CHECK(r.GenerateHTML().find("hidden message") == std::string::npos);
    CHECK(r.GetTotalErrors() == 1);
}
