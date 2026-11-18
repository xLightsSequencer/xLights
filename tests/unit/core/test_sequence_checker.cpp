#include <catch2/catch_test_macros.hpp>

#include <set>
#include <string>
#include <vector>

#include <pugixml.hpp>

#include "diagnostics/SequenceChecker.h"
#include "models/ModelManager.h"
#include "outputs/OutputManager.h"
#include "render/Effect.h"
#include "render/EffectLayer.h"
#include "render/Element.h"
#include "render/HeadlessRenderContext.h"
#include "render/SequenceElements.h"
#include "support/TempDir.h"
#include "utils/FileUtils.h"

namespace {

// An effect "extends beyond" the sequence when its end is after the end of
// the last frame the renderer draws.
bool ExtendsBeyond(int effectEndMS, int sequenceDurationMS, int frequency) {
    return effectEndMS > SequenceChecker::GetSequenceEndMS(sequenceDurationMS, frequency);
}

// Stub of the per-platform media compatibility probe: AVFoundation
// refuses Matroska containers ("File format not supported"), plays
// everything else here.
struct MediaCompatCallbacks : public SequenceCheckerCallbacks {
    std::set<std::string> probed;
    bool mediaCompatDisabled = false;

    bool IsCheckOptionDisabled(const std::string& option) const override {
        return mediaCompatDisabled && option == "MediaCompat";
    }

    std::string CheckVideoCompatibility(const std::string& path) override {
        probed.insert(path);
        if (path.size() >= 4 && path.compare(path.size() - 4, 4, ".mkv") == 0) {
            return "File format not supported";
        }
        return "";
    }

    std::string CheckAudioCompatibility(const std::string& path) override {
        probed.insert(path);
        return "";
    }
};

// One Single Line model with an element + effect layer, ready to host
// Video effects whose file references land in RunFileReferenceChecks.
struct CheckerFixture {
    HeadlessRenderContext ctx;
    Element* element = nullptr;
    EffectLayer* layer = nullptr;

    CheckerFixture() {
        pugi::xml_document doc;
        REQUIRE(doc.load_string(
            "<model name=\"L\" DisplayAs=\"Single Line\" StartChannel=\"1\" NumStrings=\"1\" "
            "NodesPerString=\"64\" StringType=\"RGB Nodes\"/>"));
        REQUIRE(ctx.AllModels.createAndAddModel(doc.first_child(), 100, 100) != nullptr);
        ctx.AllModels.RecalcStartChannels();
        element = ctx._sequenceElements.AddElement("L", "model", true, false, true, false, false);
        layer = element->AddEffectLayer();
    }

    Effect* VideoEffect(const std::string& filename) {
        Effect* e = layer->AddEffect(0, "Video", "", "", 0, 1000, 0, false);
        if (e && !filename.empty()) {
            e->GetSettings()["E_FILEPICKERCTRL_Video_Filename"] = filename;
        }
        return e;
    }

    // Runs the file-reference checks with all report sections installed
    // (RunFullCheck callers add REPORT_SECTIONS first; mirror that).
    CheckSequenceReport Run(SequenceCheckerCallbacks* callbacks) {
        CheckSequenceReport report;
        for (const auto& s : CheckSequenceReport::REPORT_SECTIONS) {
            report.AddSection(s);
        }
        SequenceChecker checker(ctx._sequenceElements, ctx.AllModels,
                                *ctx.GetOutputManager(),
                                nullptr, "", callbacks);
        checker.RunFileReferenceChecks(report);
        return report;
    }

    static int CountCategory(const CheckSequenceReport& report,
                             const std::string& sectionId,
                             const std::string& category) {
        int n = 0;
        for (const auto& s : report.GetSections()) {
            if (s.id != sectionId) continue;
            for (const auto& i : s.issues) {
                if (i.category == category) ++n;
            }
        }
        return n;
    }
};

} // namespace

TEST_CASE("GetSequenceEndMS rounds the duration to the nearest frame boundary", "[diagnostics][checksequence]") {
    // 25 ms frames (40 fps). A media length of 235.299s must be treated as
    // 235.300s, otherwise every effect ending on the final frame is flagged.
    CHECK(SequenceChecker::GetSequenceEndMS(235299, 40) == 235300);
    CHECK(SequenceChecker::GetSequenceEndMS(235300, 40) == 235300);
    CHECK(SequenceChecker::GetSequenceEndMS(230000, 40) == 230000);
    CHECK(SequenceChecker::GetSequenceEndMS(111438, 40) == 111450);
    CHECK(SequenceChecker::GetSequenceEndMS(227493, 40) == 227500);

    // The renderer uses the nearest frame boundary, not a strict ceil:
    // 235.311s is closer to 235.300s than to 235.325s.
    CHECK(SequenceChecker::GetSequenceEndMS(235311, 40) == 235300);

    // 50 ms frames (20 fps).
    CHECK(SequenceChecker::GetSequenceEndMS(227493, 20) == 227500);
    CHECK(SequenceChecker::GetSequenceEndMS(230000, 20) == 230000);
}

TEST_CASE("GetSequenceEndMS keeps the duration when the frequency is unusable", "[diagnostics][checksequence]") {
    CHECK(SequenceChecker::GetSequenceEndMS(235299, 0) == 235299);
    CHECK(SequenceChecker::GetSequenceEndMS(235299, -40) == 235299);
}

TEST_CASE("Effects ending on the last frame are not past the sequence end", "[diagnostics][checksequence]") {
    // The exact cases that produced false positives: duration 235.299s and
    // effects snapped to the 235.300s frame boundary.
    CHECK_FALSE(ExtendsBeyond(235300, 235299, 40));

    // 225.085s rounds to 225.075s at 25 ms, so an effect at 225.100s is one
    // frame past and must still be reported.
    CHECK(ExtendsBeyond(225100, 225085, 40));

    // Genuine overruns of one or more frames stay reported.
    CHECK(ExtendsBeyond(235325, 235299, 40));
    CHECK(ExtendsBeyond(230025, 230000, 40));
    CHECK(ExtendsBeyond(230050, 230000, 40));
    CHECK_FALSE(ExtendsBeyond(230000, 230000, 40));
}

TEST_CASE("File reference checks probe video files for media compatibility", "[diagnostics][checksequence][mediacompat]") {
    TempDir d;
    CheckerFixture f;

    SECTION("An AVFoundation-incompatible file is reported once") {
        std::string mkv = d.Write("matrix.mkv", "not really a matroska file");
        f.VideoEffect(mkv);
        // Same file referenced again by a second effect: the probe must
        // still fire only once for it.
        Effect* b = f.layer->AddEffect(0, "Video", "", "", 1000, 2000, 0, false);
        if (b) {
            b->GetSettings()["E_FILEPICKERCTRL_Video_Filename"] = mkv;
        }

        MediaCompatCallbacks cb;
        CheckSequenceReport report = f.Run(&cb);

        CHECK(cb.probed.size() == 1);
        CHECK(CheckerFixture::CountCategory(report, "sequence", "mediacompat") == 1);
        CHECK(report.GetTotalWarnings() == 1);
        bool found = false;
        for (const auto& s : report.GetSections()) {
            if (s.id != "sequence") continue;
            for (const auto& i : s.issues) {
                if (i.category == "mediacompat") {
                    found = i.type == CheckSequenceReport::ReportIssue::WARNING &&
                            i.message.find("matrix.mkv") != std::string::npos &&
                            i.message.find("File format not supported") != std::string::npos;
                }
            }
        }
        CHECK(found);
    }

    SECTION("A compatible file produces no warning") {
        std::string mp4 = d.Write("matrix.mp4", "not really an mp4 file");
        f.VideoEffect(mp4);

        MediaCompatCallbacks cb;
        CheckSequenceReport report = f.Run(&cb);

        CHECK(cb.probed.size() == 1);
        CHECK(CheckerFixture::CountCategory(report, "sequence", "mediacompat") == 0);
        CHECK(report.GetTotalWarnings() == 0);
    }

    SECTION("Missing files are not probed (the path checks already report them)") {
        f.VideoEffect(d.File("does-not-exist.mkv"));

        MediaCompatCallbacks cb;
        CheckSequenceReport report = f.Run(&cb);

        CHECK(cb.probed.empty());
        CHECK(CheckerFixture::CountCategory(report, "sequence", "mediacompat") == 0);
    }

    SECTION("Non-video files are not probed") {
        std::string txt = d.Write("notes.txt", "hello");
        f.VideoEffect(txt);

        MediaCompatCallbacks cb;
        CheckSequenceReport report = f.Run(&cb);

        CHECK(cb.probed.empty());
        CHECK(CheckerFixture::CountCategory(report, "sequence", "mediacompat") == 0);
    }

    SECTION("Disabling the check records the CHECK DISABLED info line and skips the probe") {
        std::string mkv = d.Write("matrix.mkv", "not really a matroska file");
        f.VideoEffect(mkv);

        MediaCompatCallbacks cb;
        cb.mediaCompatDisabled = true;
        CheckSequenceReport report = f.Run(&cb);

        CHECK(cb.probed.empty());
        CHECK(CheckerFixture::CountCategory(report, "sequence", "mediacompat") == 0);
        bool found = false;
        for (const auto& s : report.GetSections()) {
            if (s.id != "sequence") continue;
            for (const auto& i : s.issues) {
                if (i.category == "checkdisabled") {
                    found = i.message.find("Media compatibility - CHECK DISABLED") != std::string::npos;
                }
            }
        }
        CHECK(found);
    }
}
