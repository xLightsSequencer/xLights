import SwiftUI

/// Tools → Check Sequence. Surfaces authoring issues from the
/// loaded sequence (the iPad subset of desktop's Tools → Check
/// Sequence — see `XLSequenceDocument runSequenceCheckWithProgress`
/// for the list of checks). Sheet groups issues by severity with a
/// summary count at top; tapping a row that names an effect jumps
/// the grid + playhead to that effect (switching to Master View
/// first if the effect's model isn't visible in the current view).
struct CheckSequenceSheet: View {
    @Environment(SequencerViewModel.self) var viewModel
    @Environment(\.dismiss) private var dismiss

    /// `nil` until the async run completes.
    @State private var issues: [XLCheckSequenceIssue]? = nil
    @State private var progressPercent: Int = 0
    @State private var progressStep: String = ""
    @State private var didStart: Bool = false

    /// Filter state. Empty = show everything; defaults match the
    /// desktop HTML report which boots with all chips on.
    @State private var hiddenSeverities: Set<Int> = []
    @State private var hiddenSections: Set<String> = []

    var body: some View {
        NavigationStack {
            content
                .navigationTitle("Check Sequence")
                .navigationBarTitleDisplayMode(.inline)
                .toolbar {
                    ToolbarItem(placement: .topBarTrailing) {
                        Button("Done") { dismiss() }
                    }
                }
        }
        .onAppear {
            if !didStart {
                didStart = true
                Task { await runCheck() }
            }
        }
    }

    private func runCheck() async {
        progressPercent = 0
        progressStep = "Starting…"
        let result = await viewModel.runSequenceCheckAsync { pct, step in
            progressPercent = pct
            progressStep = step
        }
        issues = result
    }

    @ViewBuilder
    private var content: some View {
        if let issues {
            if issues.isEmpty {
                emptyState
            } else {
                issuesList(issues)
            }
        } else {
            progressView
        }
    }

    /// Shown while the SequenceChecker walks. Section + percent come
    /// from `SequenceCheckerCallbacks::OnProgress`; the bar updates
    /// per section transition (`Controller Checks`, `Model Checks`,
    /// `Sequence Checks`, `File Reference Checks`). On a small
    /// sequence the entire run finishes inside a single frame and
    /// this is barely visible; on a large sequence it's the
    /// difference between "is the app frozen?" and "okay it's
    /// working."
    private var progressView: some View {
        VStack(spacing: 18) {
            ProgressView(
                value: Double(min(max(progressPercent, 0), 100)),
                total: 100
            ) {
                Text(progressStep.isEmpty ? "Checking sequence…" : progressStep)
                    .font(.headline)
            }
            .progressViewStyle(.linear)
            .frame(maxWidth: 360)
            Text("\(progressPercent)%")
                .font(.caption.monospacedDigit())
                .foregroundStyle(.secondary)
        }
        .padding(40)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private var emptyState: some View {
        VStack(spacing: 12) {
            Image(systemName: "checkmark.circle.fill")
                .font(.system(size: 56))
                .foregroundStyle(.green)
            Text("No issues found")
                .font(.title3.weight(.semibold))
            Text("This sequence looks clean.")
                .font(.body)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    private func issuesList(_ all: [XLCheckSequenceIssue]) -> some View {
        // Counts come from the unfiltered set so the summary row +
        // chips always show "what's in the report" rather than
        // "what's currently visible". Filtering only affects the
        // section / row body below.
        let critical = all.filter { $0.severity == .critical }
        let warnings = all.filter { $0.severity == .warning }
        let info = all.filter { $0.severity == .info }
        let visible = all.filter { issueIsVisible($0) }
        let visibleCritical = visible.filter { $0.severity == .critical }
        let visibleWarnings = visible.filter { $0.severity == .warning }
        let visibleInfo = visible.filter { $0.severity == .info }
        return VStack(spacing: 0) {
            filterChipsRow(all: all,
                           criticalCount: critical.count,
                           warningCount: warnings.count,
                           infoCount: info.count)
            List {
                Section {
                    summaryRow(criticalCount: critical.count,
                                warningCount: warnings.count,
                                infoCount: info.count)
                }
                if !visibleCritical.isEmpty {
                    Section("Errors") {
                        ForEach(visibleCritical.indices, id: \.self) { i in
                            issueRow(visibleCritical[i])
                        }
                    }
                }
                if !visibleWarnings.isEmpty {
                    Section("Warnings") {
                        ForEach(visibleWarnings.indices, id: \.self) { i in
                            issueRow(visibleWarnings[i])
                        }
                    }
                }
                if !visibleInfo.isEmpty {
                    Section("Info") {
                        ForEach(visibleInfo.indices, id: \.self) { i in
                            issueRow(visibleInfo[i])
                        }
                    }
                }
                if visible.isEmpty {
                    Section {
                        Text("All issues are filtered out.")
                            .font(.subheadline)
                            .foregroundStyle(.secondary)
                            .frame(maxWidth: .infinity, alignment: .center)
                            .padding(.vertical, 24)
                    }
                }
            }
        }
    }

    private func issueIsVisible(_ issue: XLCheckSequenceIssue) -> Bool {
        if hiddenSeverities.contains(severityKey(for: issue.severity)) {
            return false
        }
        if hiddenSections.contains(issue.sectionID as String) {
            return false
        }
        return true
    }

    private func severityKey(for severity: XLCheckSequenceSeverity) -> Int {
        switch severity {
        case .critical: return 0
        case .warning: return 1
        case .info: return 2
        @unknown default: return -1
        }
    }

    /// Horizontal scroll of toggle chips at the top of the list.
    /// Mirrors the desktop HTML report header — Errors / Warnings /
    /// Info severity chips plus one per populated section. Counts in
    /// the chip labels reflect the full report so users can see at a
    /// glance how much they're hiding.
    private func filterChipsRow(all: [XLCheckSequenceIssue],
                                  criticalCount: Int,
                                  warningCount: Int,
                                  infoCount: Int) -> some View {
        // Section list ordered by first-occurrence in the report
        // (which itself follows REPORT_SECTIONS in the core), then
        // labelled with its display title.
        var sections: [(id: String, title: String, count: Int)] = []
        for iss in all {
            let sid = iss.sectionID as String
            if let idx = sections.firstIndex(where: { $0.id == sid }) {
                sections[idx].count += 1
            } else {
                sections.append((id: sid,
                                  title: iss.sectionTitle as String,
                                  count: 1))
            }
        }
        return ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 8) {
                if criticalCount > 0 {
                    severityChip(severity: .critical,
                                  label: "Errors",
                                  count: criticalCount,
                                  tint: .red)
                }
                if warningCount > 0 {
                    severityChip(severity: .warning,
                                  label: "Warnings",
                                  count: warningCount,
                                  tint: .orange)
                }
                if infoCount > 0 {
                    severityChip(severity: .info,
                                  label: "Info",
                                  count: infoCount,
                                  tint: .blue)
                }
                if !sections.isEmpty
                    && (criticalCount + warningCount + infoCount) > 0 {
                    Divider()
                        .frame(height: 22)
                        .padding(.horizontal, 4)
                }
                ForEach(sections, id: \.id) { sec in
                    sectionChip(id: sec.id,
                                 title: sec.title,
                                 count: sec.count)
                }
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 10)
        }
        .background(.thinMaterial)
    }

    private func severityChip(severity: XLCheckSequenceSeverity,
                                label: String,
                                count: Int,
                                tint: Color) -> some View {
        let key = severityKey(for: severity)
        let on = !hiddenSeverities.contains(key)
        return Button {
            if on { hiddenSeverities.insert(key) }
            else { hiddenSeverities.remove(key) }
        } label: {
            chipLabel(text: "\(label) (\(count))", on: on, tint: tint)
        }
        .buttonStyle(.plain)
    }

    private func sectionChip(id: String,
                              title: String,
                              count: Int) -> some View {
        let on = !hiddenSections.contains(id)
        return Button {
            if on { hiddenSections.insert(id) }
            else { hiddenSections.remove(id) }
        } label: {
            chipLabel(text: "\(title) (\(count))", on: on, tint: .accentColor)
        }
        .buttonStyle(.plain)
    }

    @ViewBuilder
    private func chipLabel(text: String, on: Bool, tint: Color) -> some View {
        HStack(spacing: 6) {
            Image(systemName: on ? "checkmark" : "circle")
                .font(.caption.weight(.semibold))
                .foregroundStyle(on ? tint : .secondary)
            Text(text)
                .font(.caption.weight(.medium))
                .foregroundStyle(on ? tint : .secondary)
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
        .background(
            Capsule()
                .fill(on ? tint.opacity(0.18) : Color.gray.opacity(0.12))
        )
        .overlay(
            Capsule()
                .stroke(on ? tint.opacity(0.45) : Color.gray.opacity(0.35),
                        lineWidth: 0.5)
        )
    }

    private func summaryRow(criticalCount: Int,
                             warningCount: Int,
                             infoCount: Int) -> some View {
        HStack(spacing: 16) {
            countBadge(systemImage: "xmark.octagon.fill",
                       tint: .red,
                       count: criticalCount,
                       label: "Errors")
            countBadge(systemImage: "exclamationmark.triangle.fill",
                       tint: .orange,
                       count: warningCount,
                       label: "Warnings")
            countBadge(systemImage: "info.circle.fill",
                       tint: .blue,
                       count: infoCount,
                       label: "Info")
            Spacer()
        }
        .padding(.vertical, 4)
    }

    private func countBadge(systemImage: String,
                             tint: Color,
                             count: Int,
                             label: String) -> some View {
        HStack(spacing: 6) {
            Image(systemName: systemImage)
                .foregroundStyle(tint)
            VStack(alignment: .leading) {
                Text("\(count)").font(.headline.monospacedDigit())
                Text(label).font(.caption2).foregroundStyle(.secondary)
            }
        }
    }

    @ViewBuilder
    private func issueRow(_ issue: XLCheckSequenceIssue) -> some View {
        if issue.startTimeMS >= 0 && (issue.modelName?.isEmpty == false) {
            Button {
                viewModel.jumpToEffect(
                    modelName: issue.modelName,
                    effectName: issue.effectName,
                    startTimeMS: Int(issue.startTimeMS),
                    layerIndex: Int(issue.layerIndex))
                dismiss()
            } label: {
                rowBody(issue, jumpable: true)
                    .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
        } else if issue.startTimeMS >= 0 {
            // No model anchor — fall back to seek-only.
            Button {
                viewModel.seekTo(ms: Int(issue.startTimeMS))
                dismiss()
            } label: {
                rowBody(issue, jumpable: true)
                    .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
        } else {
            rowBody(issue, jumpable: false)
        }
    }

    private func rowBody(_ issue: XLCheckSequenceIssue,
                          jumpable: Bool) -> some View {
        HStack(alignment: .top, spacing: 10) {
            severityIcon(for: issue.severity)
                .frame(width: 18)
                .padding(.top, 2)
            VStack(alignment: .leading, spacing: 2) {
                Text(issue.message)
                    .font(.subheadline)
                    .foregroundStyle(.primary)
                    .fixedSize(horizontal: false, vertical: true)
                HStack(spacing: 8) {
                    Text(issue.sectionTitle)
                        .font(.caption2.weight(.medium))
                        .foregroundStyle(.secondary)
                    if jumpable {
                        Text("·")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        let hasModel = (issue.modelName?.isEmpty == false)
                        Text(hasModel ? "Tap to select effect" : "Tap to jump to playhead")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
            }
        }
    }

    @ViewBuilder
    private func severityIcon(for severity: XLCheckSequenceSeverity) -> some View {
        switch severity {
        case .critical:
            Image(systemName: "xmark.octagon.fill")
                .foregroundStyle(.red)
        case .warning:
            Image(systemName: "exclamationmark.triangle.fill")
                .foregroundStyle(.orange)
        case .info:
            Image(systemName: "info.circle.fill")
                .foregroundStyle(.blue)
        @unknown default:
            Image(systemName: "questionmark.circle")
        }
    }
}
