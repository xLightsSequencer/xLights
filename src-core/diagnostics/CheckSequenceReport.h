#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

// Structured report produced by `SequenceChecker` (and previously by
// the wx-bound `xLightsFrame::CheckSequence`). Both desktop and iPad
// consume this. Desktop renders to HTML for the in-editor viewer
// via `GenerateHTML()`; iPad walks the structured `mSections /
// .issues` tree directly so SwiftUI can offer per-issue jump-to.
//
// `ReportIssue` carries optional location data — `modelName`,
// `effectName`, `startTimeMS`, `layerIndex` — populated when an
// issue is anchored to a specific row / effect in the sequence.
// The desktop HTML renderer ignores them today, but a future
// in-app results panel can use them for navigation. The iPad
// uses them for the tap-to-jump rows on the Check Sequence sheet.
class CheckSequenceReport {
public:
    struct ReportIssue {
        enum Type {
            INFO,
            WARNING,
            CRITICAL
        };

        Type type;
        std::string message;  // The actual error/warning message
        std::string category; // e.g., "video", "singlestrand", "network", etc.

        // Optional location data. Empty / -1 when not applicable.
        std::string modelName;
        std::string effectName;
        int startTimeMS = -1;
        int layerIndex = -1;

        ReportIssue(Type t, const std::string& msg, const std::string& cat = "") :
            type(t), message(msg), category(cat) {
        }

        // Convenience builder for an effect-anchored issue. `layerIndex`
        // defaults to -1 since callers without ready access to the
        // layer can omit it; the iPad UI renders the tap-to-jump even
        // when only `startTimeMS` is set.
        static ReportIssue ForEffect(Type type,
                                      const std::string& message,
                                      const std::string& category,
                                      const std::string& modelName,
                                      const std::string& effectName,
                                      int startTimeMS,
                                      int layerIndex = -1) {
            ReportIssue r(type, message, category);
            r.modelName = modelName;
            r.effectName = effectName;
            r.startTimeMS = startTimeMS;
            r.layerIndex = layerIndex;
            return r;
        }

        static ReportIssue ForModel(Type type,
                                     const std::string& message,
                                     const std::string& category,
                                     const std::string& modelName) {
            ReportIssue r(type, message, category);
            r.modelName = modelName;
            return r;
        }
    };

    struct ReportSection {
        std::string id;          // Unique identifier like "network", "sequence"
        std::string icon;        // Icon identifier
        std::string title;       // Display title
        std::string description; // Short description
        std::vector<ReportIssue> issues;
        int errorCount = 0;
        int warningCount = 0;

        ReportSection(const std::string& sid, const std::string& sicon,
                      const std::string& stitle, const std::string& sdesc) :
            id(sid), icon(sicon), title(stitle), description(sdesc) {
        }
    };

    static const std::vector<ReportSection> REPORT_SECTIONS;

    CheckSequenceReport();
    ~CheckSequenceReport() = default;

    // Methods to build the report
    void SetShowFolder(const std::string& folder) {
        mShowFolder = folder;
    }
    void SetSequencePath(const std::string& path) {
        mSequencePath = path;
    }

    // Add sections and issues
    void AddSection(const ReportSection& section);
    void AddIssue(const std::string& sectionId, const ReportIssue& issue);

    // Get report statistics
    int GetTotalErrors() const {
        return mTotalErrors;
    }
    int GetTotalWarnings() const {
        return mTotalWarnings;
    }

    // Section accessors for structured consumers (iPad).
    const std::vector<ReportSection>& GetSections() const { return mSections; }
    const std::string& GetShowFolder() const { return mShowFolder; }
    const std::string& GetSequencePath() const { return mSequencePath; }
    const std::string& GetGeneratedTime() const { return mGeneratedTime; }

    // Generate the HTML report. `darkMode` tints the body background +
    // text for callers that want to match the host app's appearance.
    // The CSS file (`checksequence_tailwind.min.css`) is referenced
    // by the returned HTML; the caller is responsible for placing it
    // alongside the written HTML file (desktop does this via
    // `xLightsFrame::CheckSequence`).
    std::string GenerateHTML(bool darkMode = false) const;

private:
    // Report data
    std::vector<ReportSection> mSections;
    std::map<std::string, size_t> mSectionIndex; // maps section id to index in sections vector

    // Report metadata
    std::string mShowFolder;
    std::string mSequencePath;
    std::string mGeneratedTime;
    int mTotalErrors = 0;
    int mTotalWarnings = 0;

    // HTML generation helper methods
    std::string GenerateHeader() const;
    std::string GenerateFilters() const;
    std::string GenerateSection(const ReportSection& section) const;

    // Issue rendering methods for different types
    std::string RenderNetworkIssues(const std::vector<ReportIssue>& issues) const;
    std::string RenderPreferenceIssues(const std::vector<ReportIssue>& issues) const;
    std::string RenderControllerIssues(const std::vector<ReportIssue>& issues) const;
    std::string RenderModelIssues(const std::vector<ReportIssue>& issues) const;
    std::string RenderSequenceIssues(const std::vector<ReportIssue>& issues) const;
    std::string RenderOSIssues(const std::vector<ReportIssue>& issues) const;

    // Helper methods
    void UpdateCounts();
    static std::string EscapeHTML(const std::string& text);
    static std::string CleanMessage(const std::string& text);
    std::string RenderIssueBox(const ReportIssue& issue, bool includeItem = true) const;
    std::string RenderSectionStart(const std::string& title, const std::string& extraClasses = "") const;
    std::string RenderSectionEnd() const;
    std::string RenderNoIssuesFound(const std::string& message) const;
    std::string RenderDisabledChecks(const std::vector<ReportIssue>& issues) const;
    std::string RenderIssueCount(const std::string& message, size_t count, const std::string& type) const;
    std::string RenderListGroup(bool isError, const std::string& heading, const std::vector<std::string>& items) const;

    // String joining helper
    template<typename Map, typename Func>
    static std::string JoinMap(const Map& map, const std::string& delim, Func keyExtractor) {
        std::string result;
        bool first = true;
        for (const auto& item : map) {
            if (!first)
                result += delim;
            result += keyExtractor(item);
            first = false;
        }
        return result;
    }
};
