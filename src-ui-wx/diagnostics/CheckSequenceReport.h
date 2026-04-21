#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

class wxFile;

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

        ReportIssue(Type t, const std::string& msg, const std::string& cat = "") :
            type(t), message(msg), category(cat) {
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

    // Generate and write report
    std::string GenerateHTML() const;
    bool WriteToFile(wxFile& f) const; // Uses wxFile like original code

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