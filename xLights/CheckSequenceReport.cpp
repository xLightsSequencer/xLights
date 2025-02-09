#pragma execution_character_set("utf-8")

#include "CheckSequenceReport.h"
#include <wx/datetime.h>
#include <wx/file.h>
#include <UtilFunctions.h>

const std::vector<CheckSequenceReport::ReportSection> CheckSequenceReport::REPORT_SECTIONS = {
    { "network", "&#127760;", "Network Configuration", "Network connectivity and configuration issues" },
    { "preferences", "&#9881;", "Preference Settings", "Configuration and settings validation" },
    { "controllers", "&#127898;", "Controller Checks", "Controller configuration and connectivity" },
    { "models", "&#128230;", "Model Checks", "Model configuration and channel validation" },
    { "sequence", "&#127916;", "Sequence Issues", "Problems with sequence effects and media" },
    { "os", "&#128187;", "System Checks", "File system and performance issues" }
};

CheckSequenceReport::CheckSequenceReport() {
    // Get current timestamp for the report
    wxDateTime now = wxDateTime::Now();
    mGeneratedTime = now.FormatISODate().ToStdString() + " " + now.FormatISOTime().ToStdString();
}

void CheckSequenceReport::AddSection(const ReportSection& section) {
    mSections.push_back(section);
    mSectionIndex[section.id] = mSections.size() - 1;
    UpdateCounts();
}

void CheckSequenceReport::AddIssue(const std::string& sectionId, const ReportIssue& issue) {
    auto it = mSectionIndex.find(sectionId);
    if (it != mSectionIndex.end()) {
        mSections[it->second].issues.push_back(issue);
        if (issue.type == ReportIssue::Type::CRITICAL) {
            mSections[it->second].errorCount++;
        } else if (issue.type == ReportIssue::Type::WARNING) {
            mSections[it->second].warningCount++;
        }
        UpdateCounts();
    }
}

void CheckSequenceReport::UpdateCounts() {
    mTotalErrors = 0;
    mTotalWarnings = 0;
    for (const auto& section : mSections) {
        mTotalErrors += section.errorCount;
        mTotalWarnings += section.warningCount;
    }
}

std::string GetCssPath() {
    std::string resourcesPath = GetResourcesDirectory();

    // Convert backslashes to forward slashes for consistent URL format
    std::replace(resourcesPath.begin(), resourcesPath.end(), '\\', '/');

    // For Windows, add an extra forward slash after file:
    #ifdef __WXMSW__
        return "file:///" + resourcesPath + "/resources/tailwind.min.css";
    #else
        return "file://" + resourcesPath + "/resources/tailwind.min.css";
    #endif
}

std::string CheckSequenceReport::GenerateHTML() const {
    std::string darkMode = IsDarkMode() ? "true" : "false";
    std::string html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>xLights Show Status Report</title>)";
    html += "<link href=\"" + GetCssPath() + "\" rel=\"stylesheet\">\n";
    html += R"(    <style>
        :root {
            --bg-primary: #f3f4f6;
            --bg-secondary: #ffffff;
            --bg-tertiary: #f9fafb;
            --bg-header: #eff6ff;
            --bg-error: #fee2e2;
            --bg-warning: #fef3c7;
            --text-error: #991b1b;
            --text-warning: #92400e;
            --text-primary: #111827;
            --text-secondary: #374151;
            --border-color: #e5e7eb;
            --text-description: #4B5563;
        }

        [data-theme="dark"] {
            --bg-primary: #111827;
            --bg-secondary: #1f2937;
            --bg-tertiary: #374151;
            --bg-header: #1e3a8a;
            --bg-error: #472121;
            --bg-warning: #473a21;
            --text-error: #fca5a5;
            --text-warning: #fcd34d;
            --text-primary: #f3f4f6;
            --text-secondary: #d1d5db;
            --border-color: #374151;
            --text-description: #9CA3AF;
        }

        body {
            background-color: var(--bg-primary);
            color: var(--text-primary);
            transition: background-color 0.3s ease, color 0.3s ease;
        }
        
        .theme-card {
            background-color: var(--bg-secondary);
            border-color: var(--border-color);
        }

        .theme-card-subtle {
            background-color: var(--bg-tertiary);
            border-color: var(--border-color);
        }
        
        .theme-header {
            background-color: var(--bg-header);
            border-color: var(--border-color);
        }

        .theme-error {
            background-color: var(--bg-error);
            color: var(--text-error);
        }

        .theme-warning {
            background-color: var(--bg-warning);
            color: var(--text-warning);
        }

        .theme-toggle {
            position: fixed;
            top: 1rem;
            right: 1rem;
            z-index: 50;
            padding: 0.5rem;
            border-radius: 0.5rem;
            background-color: var(--bg-secondary);
            border: 1px solid var(--border-color);
            cursor: pointer;
            transition: all 0.3s ease;
        }
        
        .theme-toggle:hover {
            transform: scale(1.1);
        }
        
        .theme-toggle svg {
            width: 1.5rem;
            height: 1.5rem;
            stroke: var(--text-primary);
        }

        .theme-description {
            color: var(--text-description);
        }

        .section-content {
            transition: max-height 300ms ease-in-out, opacity 300ms ease-in-out;
            overflow: hidden;
            max-height: 0;
            opacity: 0;
        }
        .section-chevron {
            transition: transform 300ms ease-in-out;
        }
        .category-content {
            transition: max-height 300ms ease-in-out, opacity 300ms ease-in-out;
        }

        [data-collapsed="true"] {
            background-color: rgb(239 246 255);
        }

        [data-theme="dark"] [data-collapsed="true"] {
            background-color: rgba(30 58 138 / 0.2);
        }

    </style>
</head>
<body class="min-h-screen">
    <button class="theme-toggle")";
    html += "\" onclick=\"toggleTheme()";
    html += R"(" aria-label="Toggle theme">
        <svg class="sun-icon" fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                  d="M12 3v1m0 16v1m9-9h-1M4 12H3m15.364 6.364l-.707-.707M6.343 6.343l-.707-.707m12.728 0l-.707.707M6.343 17.657l-.707.707M16 12a4 4 0 11-8 0 4 4 0 018 0z">
            </path>
        </svg>
        <svg class="moon-icon hidden" fill="none" stroke="currentColor" viewBox="0 0 24 24" xmlns="http://www.w3.org/2000/svg">
            <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" 
                  d="M20.354 15.354A9 9 0 018.646 3.646 9.003 9.003 0 0012 21a9.003 9.003 0 008.354-5.646z">
            </path>
        </svg>
    </button>
    <div class="container mx-auto px-4 py-8">
)";

    // Add header section
    html += GenerateHeader();

    // Add filter buttons
    html += GenerateFilters();

    // Add each section
    for (const auto& section : mSections) {
        html += GenerateSection(section);
    }

    html += R"(
    </div>)";

    html += R"(
    <script>
        let activeFilters = {
           sections: new Set(['network', 'preferences', 'controllers', 'models', 'sequence', 'os']),
           showErrors: true,
           showWarnings: true,
           searchText: ''
        };

        function toggleTheme() {
            const html = document.documentElement;
            const sunIcon = document.querySelector('.sun-icon');
            const moonIcon = document.querySelector('.moon-icon');
            
            if (html.getAttribute('data-theme') === 'dark') {
                html.removeAttribute('data-theme');
                sunIcon.classList.remove('hidden');
                moonIcon.classList.add('hidden');
                localStorage.setItem('theme', 'light');
            } else {
                html.setAttribute('data-theme', 'dark');
                sunIcon.classList.add('hidden');
                moonIcon.classList.remove('hidden');
                localStorage.setItem('theme', 'dark');
            }
        }

        function toggleFilter(key) {
            const button = event.currentTarget;
            const checkbox = button.querySelector('.checkbox');
            const isActive = button.classList.contains('active');
            
            button.classList.toggle('active');
            checkbox.textContent = isActive ? '' : '✓';
            button.style.opacity = isActive ? '0.5' : '1';
            
            if (button.dataset.section) {
               if (isActive) {
                   activeFilters.sections.delete(button.dataset.section);
               } else {
                   activeFilters.sections.add(button.dataset.section);
               }
            } else if (button.dataset.type === 'error') {
                activeFilters.showErrors = !isActive;
            } else if (button.dataset.type === 'warning') {
                activeFilters.showWarnings = !isActive;
            }
   
            applyFilters();
        }

        function toggleSection(sectionId) {
            const content = document.getElementById('section-' + sectionId);
            const chevron = event.currentTarget.querySelector('.section-chevron');
            const isExpanded = content.style.display !== 'none';
            
            if (!isExpanded) {
                content.style.display = 'block';
                content.style.maxHeight = '0';
                content.style.opacity = '0';
                // Trigger reflow
                content.offsetHeight;
                content.style.maxHeight = content.scrollHeight + 'px';
                content.style.opacity = '1';
            } else {
                content.style.maxHeight = '0';
                content.style.opacity = '0';
                // Wait for transition before hiding
                setTimeout(() => {
                    content.style.display = 'none';
                }, 300);
            }
        
            chevron.style.transform = isExpanded ? 'rotate(-90deg)' : '';
        }

        function toggleCategory(element) {
            const content = element.parentElement.querySelector('.category-content');
            const chevron = element.querySelector('.category-chevron');
            const section = element.parentElement;
    
            if (content.style.display === 'none' || content.style.display === '') {
                content.style.display = 'block';
                content.style.opacity = '1';
                chevron.style.transform = '';
                section.dataset.collapsed = 'false';
            } else {
                content.style.opacity = '0';
                content.style.display = 'none';
                chevron.style.transform = 'rotate(-90deg)';
                section.dataset.collapsed = 'true';
            }
        }

        function applyFilters() {
            const sections = document.querySelectorAll('.report-section');
            sections.forEach(section => {
                const sectionId = section.dataset.section;

                // First check if section is toggled
                if (!activeFilters.sections.has(sectionId)) {
                    section.style.display = 'none';
                    return;
                }

                let sectionHasVisibleContent = false;
        
                // Handle categories
                const categories = section.querySelectorAll('.filter-category');
                categories.forEach(category => {
                    const errorWarningItems  = category.querySelectorAll('.error-item, .warning-item');
                    if (errorWarningItems.length > 0) {
                        const hasVisibleItems = Array.from(errorWarningItems ).some(item => {
                            const isError = item.classList.contains('error-item');
                            const isWarning = item.classList.contains('warning-item');
                            return (isError && activeFilters.showErrors) || (isWarning && activeFilters.showWarnings);
                        });
            
                        // If no visible items due to toggle, hide the category
                        if (!hasVisibleItems) {
                            category.style.display = 'none';
                            return;
                        }
                    }

                    const categoryTitle = category.querySelector('.category-heading')?.textContent.toLowerCase() || '';
                    const categoryMatches = activeFilters.searchText === '' || categoryTitle.includes(activeFilters.searchText);
            
                    if (categoryMatches) {
                        category.style.display = 'block';
                        category.querySelectorAll('.filterable-item').forEach(item => {
                            const isError = item.classList.contains('error-item');
                            const isWarning = item.classList.contains('warning-item');
                            const typeVisible = (isError && activeFilters.showErrors) || (isWarning && activeFilters.showWarnings) || (!isError && !isWarning);
                            item.style.display = typeVisible ? (item.dataset.defaultDisplay || 'block') : 'none';
                        });
                        sectionHasVisibleContent = true;
                    } else {
                        let categoryHasVisibleContent = false;
                        category.querySelectorAll('.filterable-item').forEach(item => {
                            // Skip filtering items in groups with matching headers
                            const parentGroup = item.closest('.error-item, .warning-item');
                            const groupHeading = parentGroup?.querySelector('.group-heading')?.textContent.toLowerCase() || '';
                            const headingMatches = groupHeading && activeFilters.searchText !== '' && groupHeading.includes(activeFilters.searchText);
                    
                            if (headingMatches) {
                                item.style.display = item.dataset.defaultDisplay || 'block';
                                categoryHasVisibleContent = true;
                                return;
                            }
                    
                            const isError = item.classList.contains('error-item');
                            const isWarning = item.classList.contains('warning-item');
                            const typeVisible = (isError && activeFilters.showErrors) || (isWarning && activeFilters.showWarnings) || (!isError && !isWarning);
                    
                            const text = item.textContent.toLowerCase();
                            const matchesSearch = activeFilters.searchText === '' || text.includes(activeFilters.searchText);
                    
                            const shouldShow = typeVisible && matchesSearch;
                            item.style.display = shouldShow ? (item.dataset.defaultDisplay || 'block') : 'none';
                            if (shouldShow) categoryHasVisibleContent = true;
                        });
                
                        category.style.display = categoryHasVisibleContent ? 'block' : 'none';
                        if (categoryHasVisibleContent) sectionHasVisibleContent = true;
                    }
                });

                // Handle standalone groups (not in categories)
                const standaloneGroups = Array.from(section.querySelectorAll('.error-item, .warning-item'))
                    .filter(group => !group.closest('.filter-category'));
            
                standaloneGroups.forEach(group => {
                    const isError = group.classList.contains('error-item');
                    const isWarning = group.classList.contains('warning-item');
                    const typeVisible = (isError && activeFilters.showErrors) || (isWarning && activeFilters.showWarnings);
            
                    if (!typeVisible) {
                        group.style.display = 'none';
                        return;
                    }
            
                    const groupHeading = group.querySelector('.group-heading')?.textContent.toLowerCase() || '';
                    const headingMatches = activeFilters.searchText === '' || groupHeading.includes(activeFilters.searchText);
            
                    if (headingMatches) {
                        // If heading matches, show the whole group
                        group.style.display = 'block';
                        group.querySelectorAll('.filterable-item').forEach(item => {
                            item.style.display = item.dataset.defaultDisplay || 'block';
                        });
                        sectionHasVisibleContent = true;
                    } else {
                        // Check list items
                        let hasVisibleItems = false;
                        const items = group.querySelectorAll('.filterable-item');
                
                        items.forEach(item => {
                            const text = item.textContent.toLowerCase();
                            const matches = activeFilters.searchText === '' || text.includes(activeFilters.searchText);
                            item.style.display = matches ? (item.dataset.defaultDisplay || 'list-item') : 'none';
                            if (matches) hasVisibleItems = true;
                        });
                
                        group.style.display = hasVisibleItems ? 'block' : 'none';
                        if (hasVisibleItems) sectionHasVisibleContent = true;
                    }
                });

                section.style.display = sectionHasVisibleContent ? 'block' : 'none';
            });
        }

        

        function filterIssues(searchText) {
            activeFilters.searchText = searchText.toLowerCase();
            applyFilters();
        }

        function resetFilters() {
            document.querySelectorAll('.filter-toggle').forEach(button => {
                button.classList.add('active');
                button.style.opacity = '1';
                button.querySelector('.checkbox').textContent = '✓';
            });
            
            document.querySelector('input[type="text"]').value = '';
            
            activeFilters.sections = new Set(['network', 'preferences', 'controllers', 'models', 'sequence', 'os']);
            activeFilters.showErrors = true;
            activeFilters.showWarnings = true;
            activeFilters.searchText = '';
            
            applyFilters();
        }

        // Initialize sections
        document.addEventListener('DOMContentLoaded', function() {
            const savedTheme = localStorage.getItem('theme');
            const sunIcon = document.querySelector('.sun-icon');
            const moonIcon = document.querySelector('.moon-icon');

            if (!savedTheme) {  // First visit
                const prefersDark = )" +
            darkMode + R"(;
                if (prefersDark) {
                    document.documentElement.setAttribute('data-theme', 'dark');
                    sunIcon.classList.add('hidden');
                    moonIcon.classList.remove('hidden');
                    localStorage.setItem('theme', 'dark');
                }
            }

            if (savedTheme === 'dark') {
                document.documentElement.setAttribute('data-theme', 'dark');
                sunIcon.classList.add('hidden');
                moonIcon.classList.remove('hidden');
            }

            document.querySelectorAll('.section-content').forEach(section => {
                section.style.display = 'block';
                section.style.maxHeight = section.scrollHeight + 'px';
                section.style.opacity = '1';
            });

            document.querySelectorAll('.category-content').forEach(content => {
                content.style.maxHeight = content.scrollHeight + 'px';
                content.style.opacity = '1';
            });
        });
    </script>
</body>
</html>
)";

    return html;
}

bool CheckSequenceReport::WriteToFile(wxFile& f) const {
    std::string html = GenerateHTML();
    return f.Write(html.c_str(), html.length()) == html.length();
}

std::string CheckSequenceReport::GenerateHeader() const {
    std::string header = R"(
        <!-- Header/Summary Section -->
        <div class="theme-card rounded-lg shadow-lg p-6 mb-8">
            <h1 class="text-3xl font-bold mb-6">xLights Show Status Report</h1>
            
            <div class="space-y-3 mb-6">
                <div class="grid grid-cols-1 gap-2">
                    <div>
                        <h2 class="text-sm uppercase tracking-wider text-gray-600">Show Folder</h2>
                        <p class="text-lg">)" +
                         EscapeHTML(mShowFolder) + R"(</p>
                    </div>
                    <div>
                        <h2 class="text-sm uppercase tracking-wider text-gray-600">Sequence</h2>
                        <p class="text-lg)" +
                         (mSequencePath.empty() ? " text-red-600 italic" : "") +
                         R"(">)" +
                         (mSequencePath.empty() ? "No sequence was loaded when the report ran" : EscapeHTML(mSequencePath)) + R"(</p>
                    </div>
                    <div class="border-t pt-3">
                        <h2 class="text-sm uppercase tracking-wider text-gray-600">Report Generated</h2>
                        <p class="text-lg">)" +
                         mGeneratedTime + R"(</p>
                    </div>
                </div>
            </div>

            <div class="theme-card-subtle rounded-lg p-6">
                <div class="grid grid-cols-2 md:grid-cols-4 gap-6">
                    <div>
                        <div class="text-4xl font-bold text-red-600">)" +
                         std::to_string(mTotalErrors) + R"(</div>
                        <div class="text-sm text-gray-600">Total Errors</div>
                    </div>
                    <div>
                        <div class="text-4xl font-bold text-yellow-600">)" +
                         std::to_string(mTotalWarnings) + R"(</div>
                        <div class="text-sm text-gray-600">Total Warnings</div>
                    </div>
                </div>
            </div>
        </div>
)";
    return header;
}

std::string CheckSequenceReport::GenerateFilters() const {
    std::string result = "        <!-- Search and Filter Controls -->\n"
                         "        <div class=\"theme-card rounded-lg shadow-lg p-4 mb-8\">\n"
                         "            <div class=\"flex flex-col gap-4\">\n"
                         "                <!-- Search Box - Full Width -->\n"
                         "                <div class=\"w-full\">\n"
                         "                    <input type=\"text\" \n"
                         "                           placeholder=\"Search report...\" \n"
                         "                           class=\"w-full px-4 py-3 text-lg border rounded-lg theme-card\" \n"
                         "                           onkeyup=\"filterIssues(this.value)\">\n"
                         "                </div>\n"
                         "                <!-- Filter Buttons -->\n"
                         "                <div class=\"flex flex-wrap gap-2 items-center\">\n"
                         "                    <div class=\"flex gap-2\">\n"
                         "                        <button onclick=\"toggleFilter('error')\" \n"
                         "                                class=\"filter-toggle active px-4 py-2 rounded-lg flex items-center gap-2 bg-red-100 text-red-800\" \n"
                         "                                data-type=\"error\">\n"
                         "                            <span class=\"inline-block w-4 h-4 border rounded flex items-center justify-center checkbox\">✓</span>\n"
                         "                            <span>Errors</span>\n"
                         "                        </button>\n"
                         "                        <button onclick=\"toggleFilter('warning')\" \n"
                         "                                class=\"filter-toggle active px-4 py-2 rounded-lg flex items-center gap-2 bg-yellow-100 text-yellow-800\" \n"
                         "                                data-type=\"warning\">\n"
                         "                            <span class=\"inline-block w-4 h-4 border rounded flex items-center justify-center checkbox\">✓</span>\n"
                         "                            <span>Warnings</span>\n"
                         "                        </button>\n"
                         "                    </div>\n"
                         "                    <!-- Section Toggles -->\n"
                         "                    <div class=\"flex flex-wrap gap-2\">";

    // Add section toggles
    for (const auto& section : REPORT_SECTIONS) {
        result += "                        <button onclick=\"toggleFilter('" + section.id +
                  "')\" class=\"filter-toggle active px-4 py-2 rounded-lg flex items-center gap-2 bg-blue-100 text-blue-800\" data-section=\"" +
                  section.id + "\">\n" +
                  "                            <span class=\"inline-block w-4 h-4 border rounded flex items-center justify-center checkbox\">✓</span>\n" +
                  "                            <span>" + section.title + "</span>\n" +
                  "                        </button>\n";
    }

    result += "                    </div>\n"
              "                    <button onclick=\"resetFilters()\" \n"
              "                            class=\"whitespace-nowrap px-4 py-2 bg-gray-100 text-gray-800 rounded-lg hover:bg-gray-200\">\n"
              "                        Reset All\n"
              "                    </button>\n"
              "                </div>\n"
              "            </div>\n"
              "        </div>\n";

    return result;
}

std::string CheckSequenceReport::GenerateSection(const ReportSection& section) const {
    std::string result = "        <div class=\"theme-card rounded-lg shadow-lg mb-8 report-section\" data-section=\"" +
                         section.id + "\">\n"
                                      "            <div class=\"p-4 theme-header border-b cursor-pointer\" onclick=\"toggleSection('" +
                         section.id + "')\">\n"
                                      "                <div class=\"flex justify-between items-center\">\n"
                                      "                    <div>\n"
                                      "                        <h2 class=\"text-xl font-bold flex items-center\">\n"
                                      "                            <span class=\"mr-2\">" +
                         section.icon + "</span>\n"
                                        "                            " +
                         section.title + "\n"
                                         "                        </h2>\n"
                                         "                        <p class=\"text-sm theme-description\">" +
                         section.description + "</p>\n"
                                               "                    </div>\n"
                                               "                    <div class=\"flex items-center gap-4\">\n"
                                               "                        <div class=\"text-sm font-semibold\">";

    if (section.errorCount > 0) {
        result += "<span class=\"text-red-600\">" + std::to_string(section.errorCount) + " Errors</span>";
    }
    if (section.errorCount > 0 && section.warningCount > 0) {
        result += "<span class=\"mx-2\">•</span>";
    }
    if (section.warningCount > 0) {
        result += "<span class=\"text-yellow-600\">" + std::to_string(section.warningCount) + " Warnings</span>";
    }

    result += "                        </div>\n"
              "                        <svg class=\"w-6 h-6 transform section-chevron\" fill=\"none\" stroke=\"currentColor\" viewBox=\"0 0 24 24\">\n"
              "                            <path stroke-linecap=\"round\" stroke-linejoin=\"round\" stroke-width=\"2\" d=\"M19 9l-7 7-7-7\"/>\n"
              "                        </svg>\n"
              "                    </div>\n"
              "                </div>\n"
              "            </div>\n"
              "            <div class=\"section-content\" id=\"section-" +
              section.id + "\">\n";

    if (section.id == "sequence") {
        result += RenderSequenceIssues(section.issues);
    } else if (section.id == "network") {
        result += RenderNetworkIssues(section.issues);
    } else if (section.id == "preferences") {
        result += RenderPreferenceIssues(section.issues);
    } else if (section.id == "controllers") {
        result += RenderControllerIssues(section.issues);
    } else if (section.id == "models") {
        result += RenderModelIssues(section.issues);
    } else if (section.id == "os") {
        result += RenderOSIssues(section.issues);
    }

    result += "            </div>\n"
              "        </div>\n";
    return result;
}

std::string CheckSequenceReport::RenderNetworkIssues(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
        <div class="p-6">)";

    html += RenderSectionStart("Network Configuration");

    std::string networkInfo;
    for (const auto& issue : issues) {
        if (issue.category == "network_info") {
            networkInfo += R"(
                    <p class="filterable-item">)" + CleanMessage(issue.message) +
                           R"(</p>)";
        }
    }

    if (!networkInfo.empty()) {
        html += R"(
                <div class="pl-4 space-y-1">)" +
                networkInfo + R"(
                </div>)";
    }

    html += RenderSectionEnd();

   html += RenderSectionStart("Socket Issues", " mt-4");

    std::vector<std::string> socketErrors;
    for (const auto& issue : issues) {
        if (issue.type == ReportIssue::CRITICAL && issue.category == "socket") {
            socketErrors.push_back(CleanMessage(issue.message));
        }
    }

    if (!socketErrors.empty()) {
        html += R"(
        <div class="theme-error rounded-lg p-4 mt-4 error-item filterable-item">
            <div class="flex flex-col">
                <h4 class="font-medium mb-2 group-heading">Socket Creation Failed</h4>
                <ul class="list-disc pl-8 space-y-1">)";
        for (const auto& message : socketErrors) {
            html += R"(
                    <li class="filterable-item" data-default-display="list-item">)" +
                    message + R"(</li>)";
        }
        html += R"(
                </ul>
            </div>
        </div>)";
    } else {
        html += RenderNoIssuesFound("socket issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Available IP Addresses", " mt-4");
    html += R"(
                    <ul class="list-disc list-inside space-y-1">)";

    bool hasIPs = false;
    std::string ipList;
    for (const auto& issue : issues) {
        if (issue.category == "network_ips") {
            if (!hasIPs) {
                html += R"(
                    <ul class="list-disc list-inside space-y-1">)";
            }
            hasIPs = true;
            html += R"(
                        <li class="filterable-item" data-default-display="list-item">)" +
                    CleanMessage(issue.message) + R"(</li>)";
        }
    }

    if (hasIPs) {
        html += R"(
                    </ul>)";
    } else {
        html += RenderNoIssuesFound("IP addresses");
    }
    html += RenderSectionEnd();

    html += R"(
        </div>)";

    return html;
}

std::string CheckSequenceReport::RenderPreferenceIssues(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
        <div class="p-6">)";

    html += RenderSectionStart("Backup Directory Check");

    bool hasBackupIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "backup") {
            hasBackupIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasBackupIssues) {
        html += RenderNoIssuesFound("backup directory issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Potentially Problematic Settings", " mt-4");

    bool hasSettingIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "rendering" || issue.category == "autosave" || issue.category == "layout") {
            hasSettingIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasSettingIssues) {
        html += RenderNoIssuesFound("problematic settings");
    }

    html += RenderSectionEnd();

    html += R"(
        </div>)";

    return html;
}

std::string CheckSequenceReport::RenderControllerIssues(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
        <div class="p-6">)";

    // Inactive Controllers
    html += RenderSectionStart("Inactive Controllers");
    bool hasInactiveIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "inactive") {
            hasInactiveIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasInactiveIssues) {
        html += RenderNoIssuesFound("inactive controller issues");
    }
    html += RenderSectionEnd();

    // Multiple Outputs
    html += RenderSectionStart("Multiple Outputs to Same Destination", " mt-4");
    bool hasMultipleIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "duplicates") {
            hasMultipleIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasMultipleIssues) {
        html += RenderNoIssuesFound("multiple output issues");
    }
    html += RenderSectionEnd();

    // Vendor-Specific Issues
    std::map<std::string, std::vector<const ReportIssue*>> controllerIssues;
    for (const auto& issue : issues) {
        if (issue.category.substr(0, 7) == "vendor:") {
            controllerIssues[issue.category.substr(7)].push_back(&issue);
        }
    }

    if (!controllerIssues.empty()) {
        html += RenderSectionStart("Controller-Specific Issues", " mt-4");

        for (const auto& controller : controllerIssues) {
            html += R"(
                <div class="theme-card rounded p-4 mt-4 shadow-sm filterable-item">
                    <h3 class="font-semibold mb-2 group-heading">)";

            // Find and display controller info
            std::string controllerName = "";
            for (const auto* issue : controller.second) {
                if (issue->type == ReportIssue::INFO) {
                    controllerName = CleanMessage(issue->message);
                    break;
                }
            }
            html += controllerName + "</h3>";

            // Show critical issues first
            bool hasCritical = false;
            for (const auto* issue : controller.second) {
                if (issue->type == ReportIssue::CRITICAL) {
                    if (!hasCritical) {
                        html += R"(
                    <div class="space-y-2 mb-3">)";
                        hasCritical = true;
                    }
                    html += RenderIssueBox(*issue, false);
                }
            }
            if (hasCritical) {
                html += R"(
                    </div>)";
            }

            // Then show warnings
            bool hasWarnings = false;
            for (const auto* issue : controller.second) {
                if (issue->type == ReportIssue::WARNING) {
                    if (!hasWarnings) {
                        html += R"(
                    <div class="space-y-2">)";
                        hasWarnings = true;
                    }
                    html += RenderIssueBox(*issue, false);
                }
            }
            if (hasWarnings) {
                html += R"(
                    </div>)";
            }

            if (!hasCritical && !hasWarnings) {
                html += RenderNoIssuesFound("issues");
            }

            html += R"(
                </div>)";
        }

        html += RenderSectionEnd();
    }

    // Other Controller Issues
    html += RenderSectionStart("Other Controller Issues", " mt-4");
    bool hasOtherIssues = false;
    for (const auto& issue : issues) {
        if (issue.category != "inactive" &&
            issue.category != "duplicates" &&
            issue.category.substr(0, 7) != "vendor:" &&
            issue.category != "checkdisabled" &&
            issue.type != ReportIssue::INFO) {
            hasOtherIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasOtherIssues) {
        html += RenderNoIssuesFound("other issues");
    }
    html += RenderSectionEnd();

    // Disabled Checks
    html += RenderDisabledChecks(issues);

    html += R"(
        </div>)";

    return html;
}

std::string CheckSequenceReport::RenderModelIssues(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
        <div class="p-6">)";

    html += RenderSectionStart("Unconfigured Models");

    bool hasUnonfiguredIssues = false;
    std::vector<std::string> unconfiguredErrorModels;
    std::vector<std::string> unconfiguredWarningModels;

    for (const auto& issue : issues) {
        if (issue.category == "unconfigured") {
            hasUnonfiguredIssues = true;
            size_t firstQuote = issue.message.find('\'');
            size_t lastQuote = issue.message.find('\'', firstQuote + 1);
            if (firstQuote != std::string::npos && lastQuote != std::string::npos) {
                std::string modelName = issue.message.substr(firstQuote + 1, lastQuote - firstQuote - 1);

                if (issue.type == ReportIssue::CRITICAL) {
                    unconfiguredErrorModels.push_back(modelName);
                } else if (issue.type == ReportIssue::WARNING) {
                    unconfiguredWarningModels.push_back(modelName);
                }
            }
        }
    }

    if (hasUnonfiguredIssues) {
        // Show errors if any
        if (!unconfiguredErrorModels.empty()) {
            html += RenderIssueCount("models not configured for controller", unconfiguredErrorModels.size(), "error");
            html += RenderListGroup(true, "Models not configured for controller:", unconfiguredErrorModels);
        }

        // Show warnings if any
        if (!unconfiguredWarningModels.empty()) {
            if (!unconfiguredErrorModels.empty()) {
                // Add some spacing if we already showed errors
                html += R"(<div class="mt-4"></div>)";
            }
            html += RenderIssueCount("models have configuration warnings", unconfiguredWarningModels.size(), "warning");
            html += RenderListGroup(false, "Models with configuration warnings:", unconfiguredWarningModels);
        }
    } else {
        html += RenderNoIssuesFound("unconfigured models");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Configuration Issues", " mt-4");

    bool hasConfigIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "config") {
            hasConfigIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasConfigIssues) {
        html += RenderNoIssuesFound("model configuration issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Universe Configuration Issues", " mt-4");

    bool hasUniverseIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "universe") {
            hasUniverseIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasUniverseIssues) {
        html += RenderNoIssuesFound("universe configuration issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Empty Models and Groups", " mt-4");

    bool hasEmptyIssues = false;
    // Group empty issues by model/group
    std::map<std::string, std::vector<const ReportIssue*>> emptyIssuesByModel;
    for (const auto& issue : issues) {
        if (issue.category == "empty") {
            hasEmptyIssues = true;
            std::string msg = issue.message;
            size_t modelStart = msg.find("'") + 1;
            size_t modelEnd = msg.find("'", modelStart);
            if (modelStart != std::string::npos && modelEnd != std::string::npos) {
                std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
                emptyIssuesByModel[modelName].push_back(&issue);
            }
        }
    }

    if (!emptyIssuesByModel.empty()) {
        for (const auto& model : emptyIssuesByModel) {
            html += R"(
                    <div class="theme-card rounded p-4 shadow-sm filterable-item">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(</h4>
                        <div class="space-y-2">)";

            // Show critical issues first
            for (const auto* issue : model.second) {
                if (issue->type == ReportIssue::CRITICAL) {
                    html += RenderIssueBox(*issue);
                }
            }

            // Then show warnings
            for (const auto* issue : model.second) {
                if (issue->type == ReportIssue::WARNING) {
                    html += RenderIssueBox(*issue);
                }
            }

            html += RenderSectionEnd();
        }
    }

    if (!hasEmptyIssues) {
        html += RenderNoIssuesFound("empty model/group issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Channel Overlap Issues", " mt-4");

    bool hasChannelIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "channels") {
            hasChannelIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasChannelIssues) {
        html += RenderNoIssuesFound("channel configuration issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Channel Overlap Issues", " mt-4");

    bool hasOverlapIssues = false;
    std::map<std::string, std::pair<std::string, std::vector<std::pair<std::string, std::string>>>> modelOverlaps;

    for (const auto& issue : issues) {
        if (issue.category == "overlap") {
            hasOverlapIssues = true;
            std::string msg = issue.message;

            size_t model1Start = msg.find("'") + 1;
            size_t model1End = msg.find("'", model1Start);
            size_t range1Start = msg.find("(", model1End) + 1;
            size_t range1End = msg.find(")", range1Start);
            size_t model2Start = msg.find("'", range1End) + 1;
            size_t model2End = msg.find("'", model2Start);
            size_t range2Start = msg.find("(", model2End) + 1;
            size_t range2End = msg.find(")", range2Start);

            if (model1Start != std::string::npos && model1End != std::string::npos &&
                range1Start != std::string::npos && range1End != std::string::npos &&
                model2Start != std::string::npos && model2End != std::string::npos &&
                range2Start != std::string::npos && range2End != std::string::npos) {
                std::string model1 = msg.substr(model1Start, model1End - model1Start);
                std::string range1 = msg.substr(range1Start, range1End - range1Start);
                std::string model2 = msg.substr(model2Start, model2End - model2Start);
                std::string range2 = msg.substr(range2Start, range2End - range2Start);

                // Store range1 with model1 and all its overlaps
                if (modelOverlaps.find(model1) == modelOverlaps.end()) {
                    modelOverlaps[model1] = std::make_pair(range1, std::vector<std::pair<std::string, std::string>>());
                }
                modelOverlaps[model1].second.push_back(std::make_pair(model2, range2));
            }
        }
    }

    if (hasOverlapIssues) {
        html += RenderIssueCount("models have channel overlaps", modelOverlaps.size(), "warning");

        // Output each model's overlaps
        for (const auto& model : modelOverlaps) {
            html += R"(
                <div class="theme-warning rounded p-3 mt-4 warning-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + " (" + model.second.first + R"() overlaps with:</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& overlap : model.second.second) {
                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(overlap.first) + " (" + overlap.second + ")</li>";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("channel overlap issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Channel Gap Issues", " mt-4");

    bool hasGapIssues = false;
    std::map<std::pair<std::string, std::string>, std::vector<std::tuple<std::string, std::string, int>>> gapsByConnection;

    // First pass - collect all gaps by controller IP and connection
    for (const auto& issue : issues) {
        if (issue.category == "overlapgap") {
            hasGapIssues = true;
            std::string msg = issue.message;

            // Extract model names
            size_t model1Start = msg.find("Model '") + 7;
            size_t model1End = msg.find("'", model1Start);
            size_t model2Start = msg.find("Model '", model1End) + 7;
            size_t model2End = msg.find("'", model2Start);

            // Extract IP and connection
            size_t ipStart = msg.find("IP '") + 4;
            size_t ipEnd = msg.find("'", ipStart);
            size_t connStart = msg.find("Connection '") + 12;
            size_t connEnd = msg.find("'", connStart);

            // Extract gap size
            size_t gapStart = msg.find("gap of ") + 7;
            size_t gapEnd = msg.find(" ", gapStart);

            if (model1Start != std::string::npos && model1End != std::string::npos &&
                model2Start != std::string::npos && model2End != std::string::npos &&
                ipStart != std::string::npos && ipEnd != std::string::npos &&
                connStart != std::string::npos && connEnd != std::string::npos &&
                gapStart != std::string::npos && gapEnd != std::string::npos) {
                std::string model1 = msg.substr(model1Start, model1End - model1Start);
                std::string model2 = msg.substr(model2Start, model2End - model2Start);
                std::string ip = msg.substr(ipStart, ipEnd - ipStart);
                std::string conn = msg.substr(connStart, connEnd - connStart);
                int gapSize = std::stoi(msg.substr(gapStart, gapEnd - gapStart));

                gapsByConnection[std::make_pair(ip, conn)].push_back(
                    std::make_tuple(model1, model2, gapSize));
            }
        }
    }

    if (hasGapIssues) {
        html += RenderIssueCount("controller outputs have channel gaps", gapsByConnection.size(), "warning");

        // Output each controller's gaps
        for (const auto& connection : gapsByConnection) {
            html += R"(
                <div class="theme-warning rounded p-3 mt-4 warning-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">Controller IP ')" +
                    EscapeHTML(connection.first.first) + R"(', Output Connection ')" +
                    EscapeHTML(connection.first.second) + R"(':</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& gap : connection.second) {
                std::string gapDescription = std::get<2>(gap) <= 30 ? "Small gap" : "Gap";
                std::string nullPixels = std::get<2>(gap) <= 30 ? " (Possible NULL Pixels)" : "";

                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        gapDescription + " of " +
                        std::to_string(std::get<2>(gap)) + " channels between '" +
                        EscapeHTML(std::get<0>(gap)) + "' and '" +
                        EscapeHTML(std::get<1>(gap)) + "'" + nullPixels + R"(</li>)";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("channel gap issues");
    }

    html += RenderSectionEnd();
    html += RenderSectionStart("Model Layer Allocation Issues", " mt-4");

    bool hasModelLayerIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "overlapnodes") {
            hasModelLayerIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasModelLayerIssues) {
        html += RenderNoIssuesFound("model layer allocation issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Missing Face Images", " mt-4");

    bool hasFaceIssues = false;
    std::map<std::string, std::map<std::string, std::set<std::string>>> facesByModelAndType; // Changed vector to set

    // Group face issues by model, then by face name
    for (const auto& issue : issues) {
        if (issue.category == "faces") {
            hasFaceIssues = true;
            std::string msg = issue.message;
            size_t modelStart = msg.find("'") + 1;
            size_t modelEnd = msg.find("'", modelStart);
            size_t faceStart = msg.find("'", modelEnd + 1) + 1;
            size_t faceEnd = msg.find("'", faceStart);
            size_t pathStart = msg.find("missing ") + 8;
            if (modelStart != std::string::npos && modelEnd != std::string::npos &&
                faceStart != std::string::npos && faceEnd != std::string::npos) {
                std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
                std::string faceName = msg.substr(faceStart, faceEnd - faceStart);
                std::string path = msg.substr(pathStart);
                if (path.back() == '.') { // Remove trailing dot if present
                    path.pop_back();
                }
                facesByModelAndType[modelName][faceName].insert(path); // Using insert instead of push_back
            }
        }
    }

    if (hasFaceIssues) {
        html += RenderIssueCount("models have missing face images", facesByModelAndType.size(), "error");
        // List by model
        for (const auto& modelFaces : facesByModelAndType) {
            html += R"(
                <div class="theme-error rounded p-3 error-item filterable-item">
                    <h4 class="font-medium text-lg mb-2 group-heading">)" +
                    EscapeHTML(modelFaces.first) + R"(</h4>)";

            // Group by face type
            for (const auto& faceType : modelFaces.second) {
                html += R"(
                        <div class="theme-error p-3 rounded error-item filterable-item">
                            <div class="flex flex-col">
                                <h4 class="font-medium mb-2 group-heading">)" +
                        EscapeHTML(faceType.first) + R"(</h4>
                                <div class="pl-4 space-y-1 font-mono text-sm filterable-item">)";

                // List all missing locations for this face
                for (const auto& path : faceType.second) {
                    html += R"(
                                <p>)" +
                            EscapeHTML(path) + R"(</p>)";
                }

                html += R"(
                            </div>
                        </div>
                    </div>)";
            }

            html += R"(
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("missing face images");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Groups containing duplicate nodes", " mt-4");

    bool hasGroupIssues = false;
    std::map<std::string, std::map<std::string, std::vector<std::pair<std::string, std::string>>>> nestedOverlaps;

    // First pass - group the warnings by model group
    for (const auto& issue : issues) {
        if (issue.category == "groupoverlap") {
            hasGroupIssues = true;
            std::string msg = issue.message;

            // Use more specific delimiters
            const std::string GROUP_START = ": Model group '";
            const std::string GROUP_END = "' contains model '";
            const std::string MODEL2_START = "' and model '";
            const std::string MODEL2_END = "' which contain ";

            size_t groupStart = msg.find(GROUP_START);
            if (groupStart == std::string::npos)
                continue;
            groupStart += GROUP_START.length();

            size_t groupEnd = msg.find(GROUP_END, groupStart);
            if (groupEnd == std::string::npos)
                continue;

            size_t model1Start = groupEnd + GROUP_END.length();
            size_t model1End = msg.find(MODEL2_START, model1Start);
            if (model1End == std::string::npos)
                continue;

            size_t model2Start = model1End + MODEL2_START.length();
            size_t model2End = msg.find(MODEL2_END, model2Start);
            if (model2End == std::string::npos)
                continue;

            // Extract channel number (remains the same as it's well-defined)
            size_t chStart = msg.find("(ch ", model2End) + 4;
            size_t chEnd = msg.find(")", chStart);
            if (chStart == std::string::npos || chEnd == std::string::npos)
                continue;

            std::string groupName = msg.substr(groupStart, groupEnd - groupStart);
            std::string model1 = msg.substr(model1Start, model1End - model1Start);
            std::string model2 = msg.substr(model2Start, model2End - model2Start);
            std::string channel = msg.substr(chStart, chEnd - chStart);

            // Handle model names with '/' (remains the same)
            if (model2.find('/') != std::string::npos) {
                model2 = model2.substr(model2.find('/') + 1);
            }

            nestedOverlaps[groupName][model1].push_back(std::make_pair(model2, channel));
        }
    }

    if (hasGroupIssues) {
        // Summary count
        html += R"(
               <div class="theme-warning rounded p-3 warning-item filterable-item">
                   <div class="flex">
                       <div class="flex-shrink-0">
                           <span>⚠️</span>
                       </div>
                       <div class="ml-3">
                           <h4 class="font-medium group-heading">)" +
                std::to_string(nestedOverlaps.size()) +
                R"( model groups have overlapping nodes</h4>
                       </div>
                   </div>
               </div>)";

        // Output each group
        for (const auto& group : nestedOverlaps) {
            html += R"(
               <div class="theme-warning rounded p-3 mt-4 warning-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(group.first) + R"(:</h4>)";

            for (const auto& model : group.second) {
                // Always show the nested format
                html += R"(
                       <div class="ml-4 mb-2">
                           <p class="font-medium">)" +
                        EscapeHTML(model.first) + R"( overlaps with:</p>
                           <ul class="list-disc pl-8 space-y-1">)";

                for (const auto& overlap : model.second) {
                    html += R"(
                               <li class="filterable-item" data-default-display="list-item">)" +
                            EscapeHTML(overlap.first) + " (ch " + overlap.second + ")</li>";
                }

                html += R"(
                           </ul>
                       </div>)";
            }

            html += R"(
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("model group issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Group Reference Issues", " mt-4");

    bool hasNonExistentIssues = false;
    std::map<std::string, std::set<std::string>> uniqueIssuesByGroup;

    for (const auto& issue : issues) {
        if (issue.category == "groupnonexistent") {
            hasNonExistentIssues = true;

            size_t groupStart = issue.message.find("'") + 1;
            size_t groupEnd = issue.message.find("'", groupStart);

            if (groupStart != std::string::npos && groupEnd != std::string::npos) {
                std::string groupName = issue.message.substr(groupStart, groupEnd - groupStart);

                size_t descStart = issue.message.find("contains", groupEnd);
                if (descStart == std::string::npos) {
                    descStart = issue.message.find("refers", groupEnd);
                }

                if (descStart != std::string::npos) {
                    std::string description = issue.message.substr(descStart);
                    uniqueIssuesByGroup[groupName].insert(description);
                }
            }
        }
    }

    std::map<std::string, std::vector<std::string>> issuesByGroup;
    for (const auto& group : uniqueIssuesByGroup) {
        issuesByGroup[group.first] = std::vector<std::string>(group.second.begin(), group.second.end());
    }

    if (hasNonExistentIssues) {
        html += RenderIssueCount("model groups have reference issues", issuesByGroup.size(), "error");

        for (const auto& group : issuesByGroup) {
            html += R"(
                <div class="theme-error rounded p-3 mt-4 error-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(group.first) + R"(:</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& issue : group.second) {
                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(issue) + R"(</li>)";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("model group reference issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Group Preview Issues", " mt-4");

    bool hasPreviewIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::pair<std::string, std::string>>>> previewIssuesByGroup;

    for (const auto& issue : issues) {
        if (issue.category == "grouppreview") {
            hasPreviewIssues = true;

            size_t groupStart = issue.message.find("'") + 1;
            size_t groupEnd = issue.message.find("'", groupStart);
            size_t prev1Start = issue.message.find("'", groupEnd + 1) + 1;
            size_t prev1End = issue.message.find("'", prev1Start);
            size_t modelStart = issue.message.find("'", prev1End + 1) + 1;
            size_t modelEnd = issue.message.find("'", modelStart);
            size_t prev2Start = issue.message.find("'", modelEnd + 1) + 1;
            size_t prev2End = issue.message.find("'", prev2Start);

            if (groupStart != std::string::npos && groupEnd != std::string::npos &&
                prev1Start != std::string::npos && prev1End != std::string::npos &&
                modelStart != std::string::npos && modelEnd != std::string::npos &&
                prev2Start != std::string::npos && prev2End != std::string::npos) {
                std::string groupName = issue.message.substr(groupStart, groupEnd - groupStart);
                std::string modelName = issue.message.substr(modelStart, modelEnd - modelStart);
                std::string groupPreview = issue.message.substr(prev1Start, prev1End - prev1Start);
                std::string modelPreview = issue.message.substr(prev2Start, prev2End - prev2Start);

                previewIssuesByGroup[groupName].push_back(
                    std::make_pair(modelName, std::make_pair(modelPreview, groupPreview)));
            }
        }
    }

    if (hasPreviewIssues) {
        html += RenderIssueCount("model groups have preview mismatches", previewIssuesByGroup.size(), "warning");

        for (const auto& group : previewIssuesByGroup) {
            html += R"(
                <div class="theme-warning rounded p-3 mt-4 warning-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(group.first) + R"( (in preview ')" + group.second[0].second.second + R"(') contains models in different previews:</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& modelIssue : group.second) {
                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(modelIssue.first) + " (in preview '" +
                        EscapeHTML(modelIssue.second.first) + "')</li>";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("preview mismatch issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Group Name Uniqueness Issues", " mt-4");

    bool hasDistinctNameIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "groupdistinctnames") {
            hasDistinctNameIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasDistinctNameIssues) {
        html += RenderNoIssuesFound("duplicate name issues");
    }

    html += RenderSectionEnd();

    // Model Group Structure Errors
    html += RenderSectionStart("Model Group Structure Errors", " mt-4");

    bool hasStructureErrors = false;
    for (const auto& issue : issues) {
        if (issue.category == "grouperrors") {
            hasStructureErrors = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasStructureErrors) {
        html += RenderNoIssuesFound("structure errors");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Group DMX Issues", " mt-4");
    bool hasDMXIssues = false;
    std::map<std::string, std::vector<std::string>> dmxIssuesByGroup;
    for (const auto& issue : issues) {
        if (issue.category == "groupdmx") {
            hasDMXIssues = true;
            std::string msg = issue.message;

            // Extract model group name - always between first quotes
            size_t groupStart = msg.find("'") + 1;
            size_t groupEnd = msg.find("' contains", groupStart);

            if (groupStart != std::string::npos && groupEnd != std::string::npos) {
                std::string groupName = msg.substr(groupStart, groupEnd - groupStart);
                // Store the rest of the message after "contains"
                size_t containsPos = msg.find("contains", groupEnd);
                if (containsPos != std::string::npos) {
                    std::string issue = msg.substr(containsPos + 9); // +9 to skip "contains "
                    dmxIssuesByGroup[groupName].push_back(issue);
                }
            }
        }
    }

    if (hasDMXIssues) {
        html += RenderIssueCount("Model groups have DMX compatibility issues", dmxIssuesByGroup.size(), "warning");

        // Output each group's issues
        for (const auto& group : dmxIssuesByGroup) {
            html += R"(
                <div class="theme-warning rounded p-3 mt-4 warning-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(group.first) + R"( contains:</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& issue : group.second) {
                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(issue) + R"(</li>)";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("DMX compatibility issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("SubModels with no nodes", " mt-4");

    bool hasSubModelNoNodesIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> submodelsByModelNoNodes;

    for (const auto& issue : issues) {
        if (issue.category == "submodelsnodes") {
            hasSubModelNoNodesIssues = true;
            std::string msg = issue.message;

            // Extract the full model/submodel name
            size_t modelStart = msg.find("'") + 1;
            size_t modelEnd = msg.find("'", modelStart);
            if (modelStart != std::string::npos && modelEnd != std::string::npos) {
                std::string fullName = msg.substr(modelStart, modelEnd - modelStart);

                // Split into model and submodel
                size_t slashPos = fullName.find('/');
                if (slashPos != std::string::npos) {
                    std::string modelName = fullName.substr(0, slashPos);
                    std::string subModelName = fullName.substr(slashPos + 1);
                    // Get the rest of the message after the model name quote
                    std::string restOfMsg = msg.substr(modelEnd + 1);

                    submodelsByModelNoNodes[modelName].push_back(std::make_pair(subModelName, restOfMsg));
                }
            }
        }
    }

    if (hasSubModelNoNodesIssues) {
        for (const auto& model : submodelsByModelNoNodes) {
            html += R"(
                    <div class="theme-error rounded p-3 error-item filterable-item">
                        <div class="flex flex-col">
                            <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                            <div class="ml-4">)";

            for (const auto& subModel : model.second) {
                html += R"(
                                <div class="mb-2 filterable-item">
                                    <p class="font-medium">)" +
                        EscapeHTML(subModel.first) + R"(</p>
                                    <p class="pl-4">)" +
                        CleanMessage(subModel.second) + R"(</p>
                                </div>)";
            }

            html += R"(
                            </div>
                        </div>
                    </div>)";
        }
    } else {
        html += RenderNoIssuesFound("submodel with no issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("SubModels with duplicate nodes", " mt-4");

    bool hasSubModelDupNodesIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> submodelsByModelDups;

    for (const auto& issue : issues) {
        if (issue.category == "submodelsdups") {
            hasSubModelDupNodesIssues = true;
            std::string msg = issue.message;

            // Extract the full model/submodel name
            size_t modelStart = msg.find("'") + 1;
            size_t modelEnd = msg.find("'", modelStart);
            if (modelStart != std::string::npos && modelEnd != std::string::npos) {
                std::string fullName = msg.substr(modelStart, modelEnd - modelStart);

                // Split into model and submodel
                size_t slashPos = fullName.find('/');
                if (slashPos != std::string::npos) {
                    std::string modelName = fullName.substr(0, slashPos);
                    std::string subModelName = fullName.substr(slashPos + 1);
                    // Get the rest of the message after the model name quote
                    std::string restOfMsg = msg.substr(modelEnd + 1);

                    submodelsByModelDups[modelName].push_back(std::make_pair(subModelName, restOfMsg));
                }
            }
        }
    }

    if (hasSubModelDupNodesIssues) {
        for (const auto& model : submodelsByModelDups) {
            html += R"(
                    <div class="theme-warning rounded p-3 warning-item filterable-item">
                        <div class="flex flex-col">
                            <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                            <div class="ml-4">)";

            for (const auto& subModel : model.second) {
                html += R"(
                                <div class="mb-2 filterable-item">
                                    <p class="font-medium">)" +
                        EscapeHTML(subModel.first) + R"(</p>
                                    <p class="pl-4">)" +
                        CleanMessage(subModel.second) + R"(</p>
                                </div>)";
            }

            html += R"(
                            </div>
                        </div>
                    </div>)";
        }
    } else {
        html += RenderNoIssuesFound("submodel with duplicate nodes issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("SubModels with nodes not in parent model", " mt-4");

    bool hasSubModelRangeIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> submodelsByModelRange;

    for (const auto& issue : issues) {
        if (issue.category == "submodelsrange") {
            hasSubModelRangeIssues = true;
            std::string msg = issue.message;

            // Extract the full model/submodel name
            size_t modelStart = msg.find("'") + 1;
            size_t modelEnd = msg.find("'", modelStart);
            if (modelStart != std::string::npos && modelEnd != std::string::npos) {
                std::string fullName = msg.substr(modelStart, modelEnd - modelStart);

                // Split into model and submodel
                size_t slashPos = fullName.find('/');
                if (slashPos != std::string::npos) {
                    std::string modelName = fullName.substr(0, slashPos);
                    std::string subModelName = fullName.substr(slashPos + 1);
                    // Get the rest of the message after the model name quote
                    std::string restOfMsg = msg.substr(modelEnd + 1);

                    submodelsByModelRange[modelName].push_back(std::make_pair(subModelName, restOfMsg));
                }
            }
        }
    }

    if (hasSubModelRangeIssues) {
        for (const auto& model : submodelsByModelRange) {
            html += R"(
                    <div class="theme-error rounded p-3 error-item filterable-item">
                        <div class="flex flex-col">
                            <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                            <div class="ml-4">)";

            for (const auto& subModel : model.second) {
                html += R"(
                                <div class="mb-2 filterable-item">
                                    <p class="font-medium">)" +
                        EscapeHTML(subModel.first) + R"(</p>
                                    <p class="pl-4">)" +
                        CleanMessage(subModel.second) + R"(</p>
                                </div>)";
            }

            html += R"(
                            </div>
                        </div>
                    </div>)";
        }
    } else {
        html += RenderNoIssuesFound("submodel with nodes not in parent model issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Model Settings Issues", " mt-4");

    bool hasSettingIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "settings" || issue.category == "render") {
            hasSettingIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasSettingIssues) {
        html += RenderNoIssuesFound("model settings issues");
    }

    html += RenderSectionEnd();

    html += RenderDisabledChecks(issues);

    html += "        </div>\n";

    return html;
}

std::string CheckSequenceReport::RenderSequenceIssues(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
        <div class="p-6">)";

    html += RenderSectionStart("Uncommon and often undesirable Settings");

    bool hasUncommonIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "general") {
            hasUncommonIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasUncommonIssues) {
        html += RenderNoIssuesFound("uncommon or undesirable settings");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Autosave", " mt-4");

    bool hasAutosaveIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "autosave") {
            hasAutosaveIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasAutosaveIssues) {
        html += RenderNoIssuesFound("autosave issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Models Not Found in Layout", " mt-4");

    bool hasNonexistentModels = false;
    std::vector<std::string> missingModels;

    for (const auto& issue : issues) {
        if (issue.category == "modelnotinlayout") {
            hasNonexistentModels = true;
            size_t modelStart = issue.message.find("Model ") + 6;
            size_t modelEnd = issue.message.find(" in your sequence");
            if (modelStart != std::string::npos && modelEnd != std::string::npos) {
                missingModels.push_back(issue.message.substr(modelStart, modelEnd - modelStart));
            }
        }
    }

    if (hasNonexistentModels) {
        html += RenderListGroup(true, "Models in your sequence do not exist in the layout. These will need to be deleted or remapped to another model next time you load this sequence.", missingModels);
    } else {
        html += RenderNoIssuesFound("missing model issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Hidden Model Effects", " mt-4");

    bool hasHiddenEffects = false;
    std::map<std::string, std::vector<std::string>> hiddenByGroup;

    for (const auto& issue : issues) {
        if (issue.category == "modeleffectshidden") {
            hasHiddenEffects = true;
            std::string msg = issue.message;
            size_t groupStart = msg.find("'") + 1;
            size_t groupEnd = msg.find("'", groupStart);
            size_t modelStart = msg.find("'", groupEnd + 1) + 1;
            size_t modelEnd = msg.find("'", modelStart);

            if (groupStart != std::string::npos && groupEnd != std::string::npos &&
                modelStart != std::string::npos && modelEnd != std::string::npos) {
                std::string groupName = msg.substr(groupStart, groupEnd - groupStart);
                std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
                hiddenByGroup[groupName].push_back(modelName);
            }
        }
    }

    if (hasHiddenEffects) {
        for (const auto& group : hiddenByGroup) {
            html += R"(
                <div class="theme-warning rounded p-3 warning-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(group.first) + R"( will hide effects on:</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& model : group.second) {
                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(model) + R"(</li>)";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("hidden effect issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Random Effects", " mt-4");

    bool hasRandomEffects = false;
    std::map<std::string, std::vector<std::tuple<std::string, std::string, int>>> effectsByModel;

    for (const auto& issue : issues) {
        if (issue.category == "unexpected") {
            hasRandomEffects = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect ") + 7;
            size_t effectEnd = msg.find(" (", effectStart);
            size_t timeStart = msg.find("(", effectEnd) + 1;
            size_t timeEnd = msg.find(")", timeStart);
            size_t modelStart = msg.find("Model '") + 7;
            size_t modelEnd = msg.find("'", modelStart);
            size_t layerStart = msg.find("layer ") + 6;
            size_t layerEnd = msg.find(" ", layerStart);

            if (modelStart != std::string::npos && modelEnd != std::string::npos) {
                std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
                std::string effectName = msg.substr(effectStart, effectEnd - effectStart);
                std::string timeRange = msg.substr(timeStart, timeEnd - timeStart);
                int layer = std::stoi(msg.substr(layerStart, layerEnd - layerStart));

                effectsByModel[modelName].push_back(std::make_tuple(effectName, timeRange, layer));
            }
        }
    }

    if (hasRandomEffects) {
        html += R"(
               <div class="theme-error rounded p-3 error-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">The following effects are marked as random. This should never happen and may cause issues.</h4>)";

        for (const auto& model : effectsByModel) {
            html += R"(
                           <div class="theme-error p-3 rounded error-item filterable-item">
                                <div class="flex flex-col">
                                    <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(</h4>
                               <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& effect : model.second) {
                html += R"(
                                   <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(std::get<0>(effect)) + " (" +
                        EscapeHTML(std::get<1>(effect)) + ") on layer " +
                        std::to_string(std::get<2>(effect)) + R"(</li>)";
            }

            html += R"(
                               </ul>
                           </div>
                    </div>)";
        }

        html += R"(
                       </div>
               </div>)";
    } else {
        html += RenderNoIssuesFound("random effect issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Effects With No Nodes", " mt-4");

    bool hasNoNodeEffects = false;
    std::map<std::string, std::vector<std::tuple<std::string, std::string, int>>> effectsNoNodesByModel;

    for (const auto& issue : issues) {
        if (issue.category == "nonodestorender") {
            hasNoNodeEffects = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect ") + 7;
            size_t effectEnd = msg.find(" (", effectStart);
            size_t timeStart = msg.find("(", effectEnd) + 1;
            size_t timeEnd = msg.find(")", timeStart);
            size_t modelStart = msg.find("Model '") + 7;
            size_t modelEnd = msg.find("'", modelStart);
            size_t layerStart = msg.find("layer ") + 6;
            int layer = std::stoi(msg.substr(layerStart));

            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
            std::string effectName = msg.substr(effectStart, effectEnd - effectStart);
            std::string timeRange = msg.substr(timeStart, timeEnd - timeStart);

            effectsNoNodesByModel[modelName].push_back(std::make_tuple(effectName, timeRange, layer));
        }
    }

    if (hasNoNodeEffects) {
        html += R"(
                <div class="theme-error rounded p-3 error-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">The following effects have no nodes and won't do anything:</h4>)";

        for (const auto& model : effectsNoNodesByModel) {
            html += R"(
                            <div class="theme-error p-3 rounded error-item filterable-item">
                                <div class="flex flex-col">
                                    <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(</h4>
                                    <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& effect : model.second) {
                html += R"(
                                        <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(std::get<0>(effect)) + " (" +
                        EscapeHTML(std::get<1>(effect)) + ") on layer " +
                        std::to_string(std::get<2>(effect)) + R"(</li>)";
            }

            html += R"(
                                    </ul>
                                </div>
                            </div>)";
        }

        html += R"(
                        </div>
                </div>)";
    } else {
        html += RenderNoIssuesFound("effects with no nodes");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Impossible Effect Overlaps", " mt-4");

    bool hasOverlaps = false;
    std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string, std::string, int>>> overlapsByModel;

    for (const auto& issue : issues) {
        if (issue.category == "impossibleoverlap") {
            hasOverlaps = true;
            std::string msg = issue.message;

            size_t effect1Start = msg.find("Effect ") + 7;
            size_t effect1End = msg.find(" (", effect1Start);
            size_t time1Start = msg.find("(", effect1End) + 1;
            size_t time1End = msg.find(")", time1Start);

            size_t effect2Start = msg.find("Effect ", time1End) + 7;
            size_t effect2End = msg.find(" (", effect2Start);
            size_t time2Start = msg.find("(", effect2End) + 1;
            size_t time2End = msg.find(")", time2Start);

            size_t modelStart = msg.find("Model '") + 7;
            size_t modelEnd = msg.find("'", modelStart);
            size_t layerStart = msg.find("layer ") + 6;
            int layer = std::stoi(msg.substr(layerStart));

            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
            std::string effect1Name = msg.substr(effect1Start, effect1End - effect1Start);
            std::string time1Range = msg.substr(time1Start, time1End - time1Start);
            std::string effect2Name = msg.substr(effect2Start, effect2End - effect2Start);
            std::string time2Range = msg.substr(time2Start, time2End - time2Start);

            overlapsByModel[modelName].push_back(std::make_tuple(effect1Name, time1Range, effect2Name, time2Range, layer));
        }
    }

    if (hasOverlaps) {
        html += R"(
               <div class="theme-error rounded p-3 error-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">The following effect overlaps should not be possible:</h4>)";

        for (const auto& model : overlapsByModel) {
            html += R"(
                           <div class="theme-error p-3 rounded error-item filterable-item">
                                <div class="flex flex-col">
                                    <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(</h4>
                                    <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& overlap : model.second) {
                html += R"(
                                        <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(std::get<0>(overlap)) + " (" +
                        EscapeHTML(std::get<1>(overlap)) + ") overlaps with " +
                        EscapeHTML(std::get<2>(overlap)) + " (" +
                        EscapeHTML(std::get<3>(overlap)) + ") on layer " +
                        std::to_string(std::get<4>(overlap)) + R"(</li>)";
            }

            html += R"(
                                    </ul>
                                </div>
                            </div>)";
        }
        html += R"(
                       </div>
               </div>)";
    } else {
        html += RenderNoIssuesFound("impossible effect overlaps");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Uncached Video Effects", " mt-4");

    bool hasUncachedVideos = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> uncachedByModel;

    for (const auto& issue : issues) {
        if (issue.category == "videocache") {
            hasUncachedVideos = true;
            std::string msg = issue.message;

            size_t effectStart = msg.rfind("Effect: ") + 8;
            size_t modelStart = msg.rfind("Model: ") + 7;
            size_t startTimeStart = msg.rfind("Start ") + 6;

            std::string effectName = msg.substr(effectStart, msg.find(", Model:") - effectStart);
            std::string modelName = msg.substr(modelStart, msg.find(", Start") - modelStart);
            std::string startTime = msg.substr(startTimeStart);

            uncachedByModel[modelName].push_back(std::make_pair(effectName, startTime));
        }
    }

    if (hasUncachedVideos) {
        for (const auto& model : uncachedByModel) {
            html += R"(
                <div class="theme-warning rounded p-3 warning-item filterable-item">
                    <div class="flex flex-col">
                        <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                        <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& effect : model.second) {
                html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(effect.first) +
                        " starting at " + EscapeHTML(effect.second) + R"(</li>)";
            }

            html += R"(
                        </ul>
                    </div>
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("uncached video effects");
    }

    html += RenderSectionEnd();


    html += RenderSectionStart("Per Model Buffer Issues", " mt-4");

    bool hasBufferIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> buffersByModel;

    for (const auto& issue : issues) {
        if (issue.category == "buffer") {
            hasBufferIssues = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect: ") + 8;
            size_t modelStart = msg.find(", Model: ") + 8;
            size_t startTimeStart = msg.find(", Start ") + 8;

            if (effectStart != std::string::npos && modelStart != std::string::npos && startTimeStart != std::string::npos) {
                size_t effectLength = msg.find(", Model:") - effectStart;
                size_t modelLength = msg.find(", Start") - modelStart;

                std::string effectName = msg.substr(effectStart, effectLength);
                std::string modelName = msg.substr(modelStart, modelLength);
                std::string startTime = msg.substr(startTimeStart);

                buffersByModel[modelName].push_back(std::make_pair(effectName, startTime));
            }
        }
    }

    if (hasBufferIssues) {
        for (const auto& model : buffersByModel) {
            html += R"(
               <div class="theme-error rounded p-3 error-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                       <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& effect : model.second) {
                html += R"(
                           <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(effect.first) +
                        " (Start " + EscapeHTML(effect.second) + R"()</li>)";
            }

            html += R"(
                       </ul>
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("per model buffer issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Old Value Curves", " mt-4");

    bool hasOldCurves = false;
    std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> curvesByModel;

    for (const auto& issue : issues) {
        if (issue.category == "oldcurves") {
            hasOldCurves = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect: ") + 8;
            size_t modelStart = msg.find("Model: ") + 7;
            size_t startTimeStart = msg.find("Start ") + 6;
            size_t propertyStart = msg.rfind(" (") + 2;
            size_t propertyEnd = msg.rfind(")");

            std::string effectName = msg.substr(effectStart, msg.find(", Model:") - effectStart);
            std::string modelName = msg.substr(modelStart, msg.find(", Start") - modelStart);
            std::string startTime = msg.substr(startTimeStart, msg.rfind(" (") - startTimeStart);
            std::string property = msg.substr(propertyStart, propertyEnd - propertyStart);

            curvesByModel[modelName].push_back(std::make_tuple(effectName, startTime, property));
        }
    }

    if (hasOldCurves) {
        for (const auto& model : curvesByModel) {
            html += R"(
               <div class="theme-error rounded p-3 error-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                       <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& curve : model.second) {
                html += R"(
                           <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(std::get<0>(curve)) +
                        " (Start " + EscapeHTML(std::get<1>(curve)) +
                        ") - " + EscapeHTML(std::get<2>(curve)) + R"(</li>)";
            }

            html += R"(
                       </ul>
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("old value curve issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Canvas Mode Issues", " mt-4");

    bool hasCanvasIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> canvasByModel;

    for (const auto& issue : issues) {
        if (issue.category == "canvas") {
            hasCanvasIssues = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect: ") + 8;
            size_t modelStart = msg.find("Model: ") + 7;
            size_t startTimeStart = msg.find("Start ") + 6;

            std::string effectName = msg.substr(effectStart, msg.find(", Model:") - effectStart);
            std::string modelName = msg.substr(modelStart, msg.find(", Start") - modelStart);
            std::string startTime = msg.substr(startTimeStart);

            canvasByModel[modelName].push_back(std::make_pair(effectName, startTime));
        }
    }

    if (hasCanvasIssues) {
        for (const auto& model : canvasByModel) {
            html += R"(
               <div class="theme-warning rounded p-3 warning-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                       <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& effect : model.second) {
                html += R"(
                           <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(effect.first) +
                        " (Start " + EscapeHTML(effect.second) + R"()</li>)";
            }

            html += R"(
                       </ul>
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("canvas mode issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Transition Issues", " mt-4");

    bool hasTransitionIssues = false;
    std::map<std::string, std::vector<std::string>> transitionsByModel;

    for (const auto& issue : issues) {
        if (issue.category == "transitions") {
            hasTransitionIssues = true;
            std::string msg = issue.message;

            size_t modelStart = msg.find("Model '") + 7;
            size_t modelEnd = msg.find("'", modelStart);
            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);

            size_t effectStart = msg.find("effect ") + 7;
            size_t effectEnd = msg.find(" at start", effectStart);
            std::string effectName = msg.substr(effectStart, effectEnd - effectStart);

            size_t startTimeStart = msg.find("start time ") + 11;
            size_t startTimeEnd = msg.find(" on Model", startTimeStart);
            std::string startTime = msg.substr(startTimeStart, startTimeEnd - startTimeStart);

            // Extract transition details
            std::string transitionInfo;
            if (msg.find("Transition in time") != std::string::npos && msg.find("transition out time") != std::string::npos) {
                size_t inTimeStart = msg.find("time ") + 5;
                double inTime = std::stod(msg.substr(inTimeStart));
                size_t outTimeStart = msg.find("out time ") + 9;
                double outTime = std::stod(msg.substr(outTimeStart));
                transitionInfo = "In " + std::to_string(inTime) + "s + Out " + std::to_string(outTime) + "s";
            } else if (msg.find("Transition in time") != std::string::npos) {
                size_t timeStart = msg.find("time ") + 5;
                double time = std::stod(msg.substr(timeStart));
                transitionInfo = "In " + std::to_string(time) + "s";
            } else {
                size_t timeStart = msg.find("time ") + 5;
                double time = std::stod(msg.substr(timeStart));
                transitionInfo = "Out " + std::to_string(time) + "s";
            }

            std::string formattedMsg = effectName + " (Start " + startTime + ") - " + transitionInfo;
            transitionsByModel[modelName].push_back(formattedMsg);
        }
    }

    if (hasTransitionIssues) {
        for (const auto& model : transitionsByModel) {
            html += R"(
               <div class="theme-warning rounded p-3 warning-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                       <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& transition : model.second) {
                html += R"(
                           <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(transition) + R"(</li>)";
            }

            html += R"(
                       </ul>
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("transition issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Effects Extending Beyond Sequence End", " mt-4");

    bool hasTimingIssues = false;
    std::map<std::string, std::vector<std::tuple<std::string, std::string, std::string>>> timingByModel;

    for (const auto& issue : issues) {
        if (issue.category == "timing") {
            hasTimingIssues = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect ") + 7;
            size_t effectEnd = msg.find(" ends", effectStart);
            size_t modelStart = msg.find("Model: '") + 8;
            size_t modelEnd = msg.find("'", modelStart);
            size_t startTimeStart = msg.find("Start: ") + 7;
            size_t endTimeStart = msg.find("ends at ") + 8;
            size_t endTimeEnd = msg.find(" after", endTimeStart);

            std::string effectName = msg.substr(effectStart, effectEnd - effectStart);
            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
            std::string startTime = msg.substr(startTimeStart);
            std::string endTime = msg.substr(endTimeStart, endTimeEnd - endTimeStart);

            timingByModel[modelName].push_back(std::make_tuple(effectName, startTime, endTime));
        }
    }

    if (hasTimingIssues) {
        for (const auto& model : timingByModel) {
            html += R"(
               <div class="theme-warning rounded p-3 warning-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                       <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& timing : model.second) {
                html += R"(
                           <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(std::get<0>(timing)) +
                        " (Start " + EscapeHTML(std::get<1>(timing)) +
                        ", Ends " + EscapeHTML(std::get<2>(timing)) + R"()</li>)";
            }

            html += R"(
                       </ul>
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("effects extending beyond sequence end");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Node Level Effect Issues", " mt-4");

    bool hasNodeIssues = false;
    std::map<std::string, std::vector<std::pair<std::string, std::string>>> nodesByModel;

    for (const auto& issue : issues) {
        if (issue.category == "nodes") {
            hasNodeIssues = true;
            std::string msg = issue.message;

            size_t effectStart = msg.find("Effect ") + 7;
            size_t effectEnd = msg.find(" at start", effectStart);
            size_t modelStart = msg.find("Model '") + 7;
            size_t modelEnd = msg.find("'", modelStart);
            size_t startTimeStart = msg.find("start time ") + 11;
            size_t startTimeEnd = msg.find(" on Model", startTimeStart);

            std::string effectName = msg.substr(effectStart, effectEnd - effectStart);
            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
            std::string startTime = msg.substr(startTimeStart, startTimeEnd - startTimeStart);

            nodesByModel[modelName].push_back(std::make_pair(effectName, startTime));
        }
    }

    if (hasNodeIssues) {
        for (const auto& model : nodesByModel) {
            html += R"(
               <div class="theme-warning rounded p-3 warning-item filterable-item">
                   <div class="flex flex-col">
                       <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(model.first) + R"(:</h4>
                       <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& node : model.second) {
                html += R"(
                           <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(node.first) +
                        " (Start " + EscapeHTML(node.second) + R"()</li>)";
            }

            html += R"(
                       </ul>
                   </div>
               </div>)";
        }
    } else {
        html += RenderNoIssuesFound("node level effect issues");
    }

    html += RenderSectionEnd();

    html += RenderSectionStart("Effect Settings Issues", " mt-4");

    bool hasSettingIssues = false;
    std::map<std::string, std::vector<std::tuple<std::string, std::string, bool>>> issuesByEffect;

    for (const auto& issue : issues) {
        if (issue.category == "effectsettings") {
            hasSettingIssues = true;
            std::string msg = issue.message;

            size_t effectTagPos = msg.rfind("--Effect:");
            std::string effectName;
            if (effectTagPos != std::string::npos) {
                effectName = msg.substr(effectTagPos + 9);
                msg = msg.substr(0, effectTagPos);
            }

            bool isError = msg.find("ERR:") != std::string::npos;
            issuesByEffect[effectName].push_back(std::make_tuple(msg, effectName, isError));
        }
    }

    if (hasSettingIssues) {
        for (const auto& effect : issuesByEffect) {
            html += R"(
                <div class="theme-card-subtle p-3 rounded filterable-item">
                    <h4 class="font-medium mb-2 group-heading">)" +
                    EscapeHTML(effect.first) + R"(</h4>)";

            // First show errors
            bool hasErrors = false;
            for (const auto& issue : effect.second) {
                if (std::get<2>(issue)) {
                    if (!hasErrors) {
                        html += R"(
                    <div class="theme-error rounded p-3 error-item filterable-item">
                        <ul class="list-disc pl-8 space-y-1">)";
                        hasErrors = true;
                    }
                    html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                            CleanMessage(std::get<0>(issue)) + R"(</li>)";
                }
            }
            if (hasErrors) {
                html += R"(
                        </ul>
                    </div>)";
            }

            // Then show warnings
            bool hasWarnings = false;
            for (const auto& issue : effect.second) {
                if (!std::get<2>(issue)) {
                    if (!hasWarnings) {
                        html += R"(
                    <div class="theme-warning rounded p-3 warning-item mt-2 filterable-item">
                        <ul class="list-disc pl-8 space-y-1">)";
                        hasWarnings = true;
                    }
                    html += R"(
                            <li class="filterable-item" data-default-display="list-item">)" +
                            CleanMessage(std::get<0>(issue)) + R"(</li>)";
                }
            }
            if (hasWarnings) {
                html += R"(
                        </ul>
                    </div>)";
            }

            html += R"(
                </div>)";
        }
    } else {
        html += RenderNoIssuesFound("effect settings issues");
    }

    html += RenderSectionEnd();


    bool hasModelFaces = false;
    std::map<std::string, std::vector<std::string>> facesByModel;

    for (const auto& issue : issues) {
        if (issue.category == "usedfaces") {
            hasModelFaces = true;
            std::string msg = issue.message;

            size_t modelStart = msg.find("Model: ") + 7;
            size_t modelEnd = msg.find(",", modelStart);
            size_t faceStart = msg.find("Face: ") + 6;
            size_t faceEnd = msg.find(".", faceStart);

            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
            std::string faceName = msg.substr(faceStart, faceEnd - faceStart);

            facesByModel[modelName].push_back(faceName);
        }
    }

    bool hasModelStates = false;
    std::map<std::string, std::vector<std::string>> statesByModel;

    for (const auto& issue : issues) {
        if (issue.category == "usedstates") {
            hasModelStates = true;
            std::string msg = issue.message;

            size_t modelStart = msg.find("Model: ") + 7;
            size_t modelEnd = msg.find(",", modelStart);
            size_t stateStart = msg.find("State: ") + 6;
            size_t stateEnd = msg.find(".", stateStart);

            std::string modelName = msg.substr(modelStart, modelEnd - modelStart);
            std::string stateName = msg.substr(stateStart, stateEnd - stateStart);

            statesByModel[modelName].push_back(stateName);
        }
    }

    bool hasViewpoints = false;
    std::vector<std::string> viewpoints;

    for (const auto& issue : issues) {
        if (issue.category == "usedviewpoints") {
            hasViewpoints = true;
            std::string msg = issue.message;

            size_t viewpointStart = msg.find("Viewpoint: ") + 11;
            size_t viewpointEnd = msg.find(".", viewpointStart);
            std::string viewpoint = msg.substr(viewpointStart, viewpointEnd - viewpointStart);

            viewpoints.push_back(viewpoint);
        }
    }

    if (hasModelFaces || hasModelStates || hasViewpoints)
    {
        html += RenderSectionStart("If you are planning on importing this sequence be aware the sequence relies on the following items that will not be imported.", " mt-4");
        html += R"(<div class="theme-card rounded p-4 mt-4 shadow-sm filterable-item">)";
        if (hasModelFaces)
        {
            html += R"(<h3 class="font-semibold mb-2 group-heading">Model Faces Used</h3>)";
            for (const auto& model : facesByModel) {
                html += R"(
                               <div class="theme-card-subtle p-3 mb-2 rounded filterable-item">
                                   <div class="flex flex-col">
                                       <h4 class="font-medium mb-2 group-heading">)" +
                        EscapeHTML(model.first) + R"(:</h4>
                                       <ul class="list-disc pl-8 space-y-1">)";

                for (const auto& face : model.second) {
                    html += R"(
                                           <li class="filterable-item" data-default-display="list-item">)" +
                            EscapeHTML(face) + R"(</li>)";
                }

                html += R"(
                                       </ul>
                                   </div>
                               </div>)";
            }
        }

        if (hasModelStates)
        {
            html += R"(<h3 class="font-semibold mb-2 group-heading">Model States Used</h3>)";
            for (const auto& model : statesByModel) {
                html += R"(
                    <div class="theme-card-subtle p-3 rounded mb-2 filterable-item">
                        <div class="flex flex-col">
                            <h4 class="font-medium mb-2 group-heading">)" +
                                EscapeHTML(model.first) + R"(:</h4>
                            <ul class="list-disc pl-8 space-y-1">)";

                        for (const auto& state : model.second) {
                            html += R"(
                                <li class="filterable-item" data-default-display="list-item">)" +
                                    EscapeHTML(state) + R"(</li>)";
                        }

                        html += R"(
                            </ul>
                        </div>
                    </div>)";
                }
        }

        if (hasViewpoints) {
            html += R"(<h3 class="font-semibold mb-2 group-heading">Viewpoints Used</h3>)";
            html += R"(
            <div class="theme-card-subtle p-3 rounded mb-2 filterable-item">
                <ul class="list-disc pl-8 space-y-1">)";

            for (const auto& viewpoint : viewpoints) {
                html += R"(
                    <li class="filterable-item" data-default-display="list-item">)" +
                        EscapeHTML(viewpoint) + R"(</li>)";
            }

            html += R"(
                </ul>
            </div>)";
        }

        html += R"(</div>)";
        html += RenderSectionEnd();

    }

    html += RenderSectionStart("General Notes", " mt-4");
    bool hasGeneralNotes = false;
    for (const auto& issue : issues) {
        if (issue.category == "sequencegeneral" && issue.type == ReportIssue::INFO) {
            hasGeneralNotes = true;
            html += R"(
                    <p>)" +
                    CleanMessage(issue.message) + R"(</p>)";
        }
    }

    if (!hasGeneralNotes)
    {
        html += RenderNoIssuesFound("general notes");
    }

    html += RenderSectionEnd();

    // Disabled Checks
    html += RenderDisabledChecks(issues);

    html += R"(
        </div>)";

    return html;
}

std::string CheckSequenceReport::RenderOSIssues(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
        <div class="p-6">)";

    // File Access Performance
    html += RenderSectionStart("File Access Performance");

    bool hasPerformanceIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "file_access") {
            hasPerformanceIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                if (issue.type == ReportIssue::WARNING) {
                    // Special handling for performance warnings with additional file info
                    html += R"(
                    <div class="theme-warning rounded p-3">
                        <div class="flex">
                            <div class="flex-shrink-0">
                                <span>⚠️</span>
                            </div>
                            <div class="ml-3">
                                <p>)" +
                            CleanMessage(issue.message) + R"(</p>)";

                    // Check for associated info entries
                    bool hasFileInfo = false;
                    for (const auto& infoIssue : issues) {
                        if (infoIssue.type == ReportIssue::INFO && infoIssue.category == "file_access") {
                            if (!hasFileInfo) {
                                html += R"(
                                <div class="mt-2 space-y-1 text-sm font-mono p-2 rounded">)";
                                hasFileInfo = true;
                            }
                            html += R"(
                                    <p>)" +
                                    CleanMessage(infoIssue.message) + R"(</p>)";
                        }
                    }
                    if (hasFileInfo) {
                        html += R"(
                                </div>)";
                    }

                    html += R"(
                            </div>
                        </div>
                    </div>)";
                } else {
                    html += RenderIssueBox(issue);
                }
            }
        }
    }

    if (!hasPerformanceIssues) {
        html += RenderNoIssuesFound("file access performance issues");
    }
    html += RenderSectionEnd();

    // File Path Issues
    html += RenderSectionStart("File Path Issues", " mt-4");

    bool hasPathIssues = false;
    for (const auto& issue : issues) {
        if (issue.category == "paths") {
            hasPathIssues = true;
            if (issue.type == ReportIssue::CRITICAL || issue.type == ReportIssue::WARNING) {
                html += RenderIssueBox(issue);
            }
        }
    }

    if (!hasPathIssues) {
        html += RenderNoIssuesFound("file path issues");
    }
    html += RenderSectionEnd();

    // Disabled Checks
    html += RenderDisabledChecks(issues);

    html += R"(
        </div>)";

    return html;
}

std::string CheckSequenceReport::EscapeHTML(const std::string& text) {
    std::string escaped;
    escaped.reserve(text.length());
    for (char c : text) {
        switch (c) {
        case '&':
            escaped += "&amp;";
            break;
        case '<':
            escaped += "&lt;";
            break;
        case '>':
            escaped += "&gt;";
            break;
        case '"':
            escaped += "&quot;";
            break;
        case '\'':
            escaped += "&#39;";
            break;
        default:
            escaped += c;
            break;
        }
    }
    return escaped;
}

std::string CheckSequenceReport::CleanMessage(const std::string& message) {
    // Removes leading spaces and INFO/WARN/ERR prefixes
    std::string msg = message;

    // Remove leading spaces
    while (msg.length() > 0 && msg[0] == ' ') {
        msg = msg.substr(1);
    }

    // Remove common prefixes
    const std::vector<std::string> prefixes = {
        "INFO: ",
        "WARN: ",
        "ERR: "
    };

    for (const auto& prefix : prefixes) {
        if (msg.substr(0, prefix.length()) == prefix) {
            msg = msg.substr(prefix.length());
            break;
        }
    }

    return msg;
}

std::string CheckSequenceReport::RenderIssueBox(const CheckSequenceReport::ReportIssue& issue, bool includeItem) const {
    const bool isCritical = issue.type == ReportIssue::CRITICAL;
    std::string themeClass = isCritical ? "theme-error" : "theme-warning";
    std::string itemClass = includeItem ? (isCritical ? " error-item filterable-item" : " warning-item filterable-item") : "";
    std::string icon = isCritical ? std::string("❌") : std::string("⚠️");

    return R"(
                    <div class=")" +
           themeClass + R"( rounded p-3)" + itemClass + R"(">
                        <div class="flex">
                            <div class="flex-shrink-0">
                                <span>)" +
           icon + R"(</span>
                            </div>
                            <p class="ml-3">)" +
           CleanMessage(issue.message) +
           R"(</p>
                        </div>
                    </div>)";
}

std::string CheckSequenceReport::RenderSectionStart(const std::string& title, const std::string& extraClasses) const {
    return R"(
            <div class="theme-card-subtle rounded-lg p-4 mb-2 filter-category" +
           extraClasses + R"( ">
                <div onclick='toggleCategory(this)' class="flex justify-between items-center cursor-pointer"><h3 class="font-semibold category-heading">)" +
           title + R"(</h3>
                <svg class="w-4 h-4 transform category-chevron text-gray-400" fill="none" stroke="currentColor" viewBox="0 0 24 24">
                    <path stroke-linecap="round" stroke-linejoin="round" stroke-width="2" d="M19 9l-7 7-7-7"/>
                </svg>
                </div>
                <div class="category-content space-y-2 mt-2" style="display: block">)";
}

std::string CheckSequenceReport::RenderSectionEnd() const {
    return R"(
                </div>
            </div>)";
}

std::string CheckSequenceReport::RenderNoIssuesFound(const std::string& message) const {
    return R"(
                    <p class="text-green-600">No )" +
           message + R"( found</p>)";
}

std::string CheckSequenceReport::RenderDisabledChecks(const std::vector<ReportIssue>& issues) const {
    std::string html = R"(
            <!-- Disabled Checks -->
            <div class="mt-4">)";

    for (const auto& issue : issues) {
        if (issue.category == "checkdisabled") {
            html += R"(
                <div class="text-gray-500 italic text-sm">)" +
                    CleanMessage(issue.message) + R"(</div>)";
        }
    }

    html += R"(
            </div>)";
    return html;
}

std::string CheckSequenceReport::RenderIssueCount(const std::string& message, size_t count, const std::string& type) const {
    std::string themeClass = "theme-" + type;
    std::string itemClass = type + "-item";
    std::string icon = (type == "error") ? "❌" : "⚠️";

    return R"(
                <div class=")" +
           themeClass + R"( rounded p-3 filterable-item )" + itemClass + R"(">
                    <div class="flex">
                        <div class="flex-shrink-0">
                            <span>)" +
           icon + R"(</span>
                        </div>
                        <div class="ml-3">
                            <h4 class="font-medium">)" +
           std::to_string(count) + " " + message + R"(</h4>
                        </div>
                    </div>
                </div>)";
}

std::string CheckSequenceReport::RenderListGroup(bool isError, const std::string& heading, const std::vector<std::string>& items) const {
    std::string themeClass = isError ? "error" : "warning";
    std::string html = R"(
    <div class="theme-)" +
                       themeClass +
                       R"( rounded p-3 mt-4 )" + themeClass + R"(-item filterable-item">
        <div class="flex flex-col">
            <h4 class="font-medium mb-2 group-heading">)" +
                       heading + R"(</h4>
            <ul class="list-disc pl-8 space-y-1">)";

    for (const auto& item : items) {
        html += R"(
                <li class="filterable-item" data-default-display="list-item">)" +
                EscapeHTML(item) + R"(</li>)";
    }

    html += R"(
            </ul>
        </div>
    </div>)";
    return html;
}