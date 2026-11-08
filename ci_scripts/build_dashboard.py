#!/usr/bin/env python3
"""Build the nightly test dashboard published to GitHub Pages.

    build_dashboard.py RESULTS_JSON SITE HISTORY_JSON RUN_URL [--no-entry] [--drop-run ID]

RESULTS_JSON is the nightly_report.py output for this run. HISTORY_JSON is the
published site's history.json (a missing file means the first run). Writes
SITE/index.html and SITE/history.json with this run added to the history.
The page is static (inline CSS and SVG, no scripts) so it needs no build step
on the Pages side. When SITE/coverage/totals.json exists (the unit test
coverage report, put there by the workflow) the page links to it and lists the
least covered files. --no-entry re-renders the page from its history without
adding this run (after a layout change); --drop-run ID removes the entries a
run made that should not have been there.
"""
import html
import json
import os
import sys

MAX_HISTORY = 400
MAX_FAILED = 10
RECENT = 14
REPO = "xLightsSequencer/xLights"
SERIES_COLORS = ["var(--c1)", "var(--c2)", "var(--c3)", "var(--c4)", "var(--c5)", "var(--c6)", "var(--c7)"]
LABELS = {"unit": "Unit tests", "render": "Render regression"}
SUITE_ORDER = ["unit", "render"]
PLATFORMS = {"linux-x64": "Linux x64", "windows-x64": "Windows x64", "macos-arm64": "macOS arm64"}

CSS = """
:root { --bg:#fbfbfa; --fg:#1d1f21; --muted:#5f6670; --line:#e2e4e8; --card:#fff;
  --good:#1a7f37; --bad:#c62828; --c1:#2563eb; --c2:#d97706; --c3:#7c3aed;
  --c4:#0f766e; --c5:#be185d; --c6:#4d7c0f; --c7:#b45309; }
@media (prefers-color-scheme: dark) { :root { --bg:#111315; --fg:#e7e9ec; --muted:#9aa3ad;
  --line:#2b2f34; --card:#1a1d20; --good:#4ac26b; --bad:#ff6b6b; --c1:#6ea2ff; --c2:#f5a524;
  --c3:#b392ff; --c4:#2dd4bf; --c5:#f472b6; --c6:#a3e635; --c7:#fbbf24; } }
* { box-sizing:border-box; }
body { margin:0; background:var(--bg); color:var(--fg); font:15px/1.5 system-ui,-apple-system,"Segoe UI",sans-serif; }
main { max-width:1100px; margin:0 auto; padding:24px 16px 64px; }
h1 { font-size:1.6rem; margin:0 0 4px; } h2 { font-size:1.15rem; margin:0 0 10px; }
h3 { font-size:.8rem; text-transform:uppercase; letter-spacing:.06em; color:var(--muted); margin:0; }
a { color:var(--c1); } code { font-size:.88em; }
.sub, .note { color:var(--muted); margin:0 0 16px; }
.cards { display:grid; grid-template-columns:repeat(auto-fit,minmax(170px,1fr)); gap:12px; margin:20px 0; }
.card { background:var(--card); border:1px solid var(--line); border-radius:10px; padding:14px 16px; }
.card.good { border-top:3px solid var(--good); } .card.bad { border-top:3px solid var(--bad); }
.card p { margin:4px 0 0; color:var(--muted); font-size:.85rem; }
.big { font-size:1.8rem; font-weight:650; } .big small { font-size:.9rem; font-weight:400; color:var(--muted); }
section { background:var(--card); border:1px solid var(--line); border-radius:10px; padding:16px; margin:16px 0; }
.charts { display:grid; grid-template-columns:repeat(auto-fit,minmax(min(100%,420px),1fr)); gap:0 16px; }
.chart { width:100%; height:auto; } .grid { stroke:var(--line); }
.axis { fill:var(--muted); font-size:11px; }
.legend { display:flex; flex-wrap:wrap; gap:14px; font-size:.85rem; color:var(--muted); }
.legend i { display:inline-block; width:10px; height:10px; border-radius:2px; margin-right:5px; }
.scroll { overflow-x:auto; }
table { border-collapse:collapse; width:100%; font-size:.88rem; }
th, td { text-align:left; padding:6px 10px; border-bottom:1px solid var(--line); white-space:nowrap; }
th { color:var(--muted); font-weight:600; }
td.good { color:var(--good); } td.bad { color:var(--bad); } td.none { color:var(--muted); }
ul.fails { margin:6px 0 14px; padding-left:20px; } ul.fails li { margin:2px 0; }
.bar { display:inline-block; width:70px; height:8px; background:var(--line); border-radius:4px;
  margin-right:8px; vertical-align:middle; overflow:hidden; }
.bar span { display:block; height:100%; background:var(--c1); }
.up { color:var(--good); } .down { color:var(--bad); }
.links { display:flex; flex-wrap:wrap; gap:8px 16px; margin:10px 0 0; }
"""


def esc(s):
    return html.escape(str(s), quote=True)


def label(suite, platform):
    return "%s (%s)" % (LABELS.get(suite, suite), PLATFORMS.get(platform, platform))


def key_of(s):
    return "%s-%s" % (s["suite"], s["platform"])


def load_history(path, drop_runs):
    try:
        with open(path) as f:
            hist = json.load(f)
    except (OSError, ValueError):
        hist = []
    return [h for h in hist if h.get("suites")
            and not any(h.get("run", "").rstrip("/").endswith("/" + r) for r in drop_runs)]


def make_entry(results, run_url):
    suites = {}
    for s in results["suites"]:
        tests, failed = s["tests"], s["failed_count"]
        rs = (s.get("properties") or {}).get("render_seconds_run1")
        try:
            rs = float(rs)
        except (TypeError, ValueError):
            rs = None
        suites[key_of(s)] = {
            "suite": s["suite"], "platform": s["platform"], "status": s["status"],
            "tests": tests, "passed": max(tests - failed, 0), "failed": failed, "skipped": s["skipped"],
            "seconds": s["time"], "render_seconds": rs,
            "failed_names": [{"name": f["name"], "message": f["message"]} for f in s["failed"][:MAX_FAILED]],
            "skipped_names": [{"name": t["name"], "message": t["message"]} for t in s.get("skipped_tests", [])[:MAX_FAILED]],
        }
    if not str(run_url).startswith(("https://", "http://")):
        run_url = ""
    entry = {"date": results["date"][:16] + "Z", "run": run_url, "sha": results.get("sha", ""),
             "event": results.get("event", ""), "suites": suites}
    cov = results.get("coverage")
    if cov and cov.get("lines"):
        entry["coverage"] = {k: cov[k] for k in COVERAGE_KEYS if k in cov}
    return entry


COVERAGE_KEYS = ("lines", "lines_hit", "functions", "functions_hit", "files", "files_unreached", "areas")


def pct(h, n):
    return 100.0 * h / n if n else 0.0


def delta(now, before):
    """A signed change in percentage points, or nothing when there is no earlier value or no change."""
    if before is None or abs(now - before) < 0.05:
        return ""
    return '<span class="%s">%+.1f</span>' % ("up" if now > before else "down", now - before)


def ok(s):
    return s["failed"] == 0 and s["status"] == "ok"


def nice_max(v):
    if v <= 0:
        return 1
    mag = 10 ** (len(str(int(v))) - 1)
    for m in (1, 2, 2.5, 5, 10):
        if v <= m * mag:
            return m * mag
    return 10 * mag


def chart(title, history, series_fn, keys, unit="", names=None):
    """Inline SVG line chart. series_fn(entry, key) -> number or None; names maps a key to its legend text."""
    pts = {k: [(i, series_fn(h, k)) for i, h in enumerate(history)] for k in keys}
    pts = {k: [(i, v) for i, v in p if v is not None] for k, p in pts.items()}
    pts = {k: p for k, p in pts.items() if p}
    out = ["<div><h2>%s</h2>" % esc(title)]
    if len(history) < 2 or not pts:
        out.append('<p class="note">Not enough history yet.</p></div>')
        return "".join(out)
    W, H, L, R, T, B = 520, 200, 44, 12, 10, 28
    vmax = nice_max(max(v for p in pts.values() for _, v in p))
    if unit == "%":
        vmax = min(vmax, 100)
    n = len(history) - 1
    x = lambda i: L + (W - L - R) * i / n
    y = lambda v: T + (H - T - B) * (1 - v / vmax)
    svg = ['<svg class="chart" viewBox="0 0 %d %d" role="img" aria-label="%s">' % (W, H, esc(title))]
    for t in range(5):
        v = vmax * t / 4
        svg.append('<line class="grid" x1="%d" x2="%d" y1="%.1f" y2="%.1f"/>' % (L, W - R, y(v), y(v)))
        svg.append('<text class="axis" x="%d" y="%.1f" text-anchor="end">%s</text>' % (L - 6, y(v) + 4, ("%g" % round(v, 2)) + unit))
    svg.append('<text class="axis" x="%d" y="%d">%s</text>' % (L, H - 8, esc(history[0]["date"][:10])))
    svg.append('<text class="axis" x="%d" y="%d" text-anchor="end">%s</text>' % (W - R, H - 8, esc(history[-1]["date"][:10])))
    legend = []
    for ci, (k, p) in enumerate(sorted(pts.items(), key=lambda kv: keys.index(kv[0]))):
        color = SERIES_COLORS[ci % len(SERIES_COLORS)]
        svg.append('<polyline fill="none" stroke="%s" stroke-width="2" points="%s"/>' % (
            color, " ".join("%.1f,%.1f" % (x(i), y(v)) for i, v in p)))
        li, lv = p[-1]
        svg.append('<circle cx="%.1f" cy="%.1f" r="3" fill="%s"/>' % (x(li), y(lv), color))
        if names:
            text = names.get(k, k)
        else:
            s = history[-1]["suites"].get(k) or next((h["suites"][k] for h in reversed(history) if k in h["suites"]), None)
            text = label(s["suite"], s["platform"]) if s else k
        legend.append('<span><i style="background:%s"></i>%s</span>' % (color, esc(text)))
    svg.append("</svg>")
    out.append("".join(svg))
    out.append('<div class="legend">%s</div></div>' % "".join(legend))
    return "".join(out)


def coverage_sections(history, report):
    """The coverage cards, chart and tables, from the history and the published report's totals.json."""
    covered = [h for h in history if h.get("coverage")]
    if not covered:
        return "", ""
    c = covered[-1]["coverage"]
    prev = covered[-2]["coverage"] if len(covered) > 1 else None
    lp, fp = pct(c["lines_hit"], c["lines"]), pct(c["functions_hit"], c["functions"])
    cards = ('<div class="card"><h3>src-core line coverage</h3><div class="big">%.1f%%</div>'
             "<p>%s of %s lines %s &middot; %s</p></div>" % (
                 lp, format(c["lines_hit"], ","), format(c["lines"], ","),
                 delta(lp, pct(prev["lines_hit"], prev["lines"]) if prev else None), esc(covered[-1]["date"][:10])))
    cards += ('<div class="card"><h3>src-core function coverage</h3><div class="big">%.1f%%</div>'
              "<p>%s of %s functions %s &middot; %d of %d files never reached</p></div>" % (
                  fp, format(c["functions_hit"], ","), format(c["functions"], ","),
                  delta(fp, pct(prev["functions_hit"], prev["functions"]) if prev else None),
                  c.get("files_unreached", 0), c.get("files", 0)))

    out = ['<section><h2>Unit test coverage of src-core</h2>'
           '<p class="note">A Linux <code>--coverage</code> build of the unit tests, measured with gcovr over '
           "<code>src-core</code> (not the submodules or the third-party code vendored into it). "
           "Code the unit tests cannot run on a CI runner (the GPU paths, video decode, network clients) "
           "keeps every directory short of 100%.</p>"]
    out.append('<div class="charts">')
    out.append(chart("Coverage over time", history,
                     lambda h, k: (pct(h["coverage"]["%s_hit" % k], h["coverage"][k]) if h.get("coverage") else None),
                     ["lines", "functions"], "%", {"lines": "Lines", "functions": "Functions"}))
    out.append("</div>")
    if report:
        out.append('<div class="links"><a href="coverage/html/index.html">Line-by-line coverage report</a>'
                   '<a href="coverage/functions.csv">Every function and its call count (CSV)</a>'
                   '<a href="coverage/summary.txt">Every file, least covered first</a></div>')
    out.append("</section>")

    before = (prev or {}).get("areas", {})
    tr = []
    for d, (n, h, fn, fh, files) in sorted(c.get("areas", {}).items(), key=lambda kv: kv[1][1] - kv[1][0]):
        p = pct(h, n)
        b = before.get(d)
        tr.append('<tr><td><code>%s</code></td><td>%d</td><td>%s</td>'
                  '<td><span class="bar"><span style="width:%.1f%%"></span></span>%.1f%% %s</td>'
                  "<td>%.1f%%</td><td>%s</td></tr>" % (
                      esc(d), files, format(n, ","), p, p, delta(p, pct(b[1], b[0]) if b else None),
                      pct(fh, fn), format(n - h, ",")))
    if tr:
        out.append('<section><h2>Coverage by source directory</h2><p class="note">Most unreached lines first: '
                   "the places a new test adds the most.</p>"
                   '<div class="scroll"><table><tr><th>Directory</th><th>Files</th><th>Lines</th>'
                   "<th>Line coverage</th><th>Functions</th><th>Lines not reached</th></tr>"
                   + "".join(tr) + "</table></div></section>")
    if report and report.get("by_file"):
        files = sorted(report["by_file"].items(), key=lambda kv: kv[1][1] - kv[1][0])[:40]
        tr = ['<tr><td><code>%s</code></td><td>%s</td>'
              '<td><span class="bar"><span style="width:%.1f%%"></span></span>%.1f%%</td>'
              "<td>%.1f%%</td><td>%s</td></tr>" % (
                  esc(name), format(n, ","), pct(h, n), pct(h, n), pct(fh, fn), format(n - h, ","))
              for name, (n, h, fn, fh) in files]
        out.append('<section><h2>The 40 files with the most unreached lines</h2>'
                   '<div class="scroll"><table><tr><th>File</th><th>Lines</th><th>Line coverage</th>'
                   "<th>Functions</th><th>Lines not reached</th></tr>" + "".join(tr) + "</table></div></section>")
    return cards, "".join(out)


def render(history, report=None):
    latest = history[-1]
    order = lambda k: (SUITE_ORDER.index(k.split("-")[0]) if k.split("-")[0] in SUITE_ORDER else len(SUITE_ORDER), k)
    keys = sorted(latest["suites"], key=order)
    all_keys = sorted({k for h in history for k in h["suites"]}, key=order)
    # Each suite shows its own most recent result: the Xcode Cloud (macOS) and GitHub (Linux, Windows)
    # nightlies run at different times, so the newest entry does not always hold every suite.
    carry = {}
    for h in history:
        for k, s in h["suites"].items():
            carry[k] = (h, s)
    sha = latest.get("sha", "")
    parts = ['<!doctype html><html lang="en"><head><meta charset="utf-8">',
             '<meta name="viewport" content="width=device-width, initial-scale=1">',
             "<title>xLights tests</title><style>%s</style></head><body><main>" % CSS,
             "<h1>xLights tests</h1>",
             '<p class="sub">Nightly runs of the xLights unit tests (Catch2) and the render regression '
             "(a synthetic show rendered headlessly and compared to recorded hashes), on Linux and Windows "
             "from GitHub Actions and on macOS from the Xcode Cloud nightly. "
             "Last run %s" % esc(latest["date"].replace("T", " ")),
             (', commit <a href="https://github.com/%s/commit/%s">%s</a>' % (REPO, esc(sha), esc(sha[:10]))) if sha else "",
             (' (<a href="%s">run log</a>)' % esc(latest["run"])) if latest.get("run") else "", ".</p>"]

    parts.append('<div class="cards">')
    for k in all_keys:
        h, s = carry[k]
        parts.append('<div class="card %s"><h3>%s</h3><div class="big">%d <small>/ %d passed</small></div>'
                     "<p>%d failed &middot; %d skipped &middot; %.1f min &middot; %s</p></div>" % (
                         "good" if ok(s) else "bad", esc(label(s["suite"], s["platform"])), s["passed"], s["tests"],
                         s["failed"], s["skipped"], s["seconds"] / 60.0, esc(h["date"][:10])))
    cov_cards, cov_body = coverage_sections(history, report)
    parts.append(cov_cards)
    parts.append("</div>")

    failing = [(k, carry[k][1]) for k in all_keys if not ok(carry[k][1])]
    if failing:
        parts.append("<section><h2>Failing now</h2>")
        for k, s in failing:
            parts.append("<h3>%s</h3><ul class='fails'>" % esc(label(s["suite"], s["platform"])))
            for f in s["failed_names"]:
                parts.append("<li><code>%s</code>%s</li>" % (esc(f["name"]), (": " + esc(f["message"])) if f["message"] else ""))
            if s["failed"] > len(s["failed_names"]):
                parts.append("<li>...and %d more (see the run log)</li>" % (s["failed"] - len(s["failed_names"])))
            parts.append("</ul>")
        parts.append("</section>")

    skipping = [(k, carry[k][1]) for k in all_keys if carry[k][1].get("skipped_names")]
    if skipping:
        parts.append("<section><h2>Skipped</h2>")
        for k, s in skipping:
            parts.append("<h3>%s</h3><ul class='skips'>" % esc(label(s["suite"], s["platform"])))
            for t in s["skipped_names"]:
                parts.append("<li><code>%s</code>%s</li>" % (esc(t["name"]), (": " + esc(t["message"])) if t["message"] else ""))
            if s["skipped"] > len(s["skipped_names"]):
                parts.append("<li>...and %d more (see the run log)</li>" % (s["skipped"] - len(s["skipped_names"])))
            parts.append("</ul>")
        parts.append("</section>")

    parts.append('<section><div class="charts">')
    parts.append(chart("Tests per run", history, lambda h, k: h["suites"].get(k, {}).get("tests"), all_keys))
    parts.append(chart("Failures per run", history, lambda h, k: h["suites"].get(k, {}).get("failed"), all_keys))
    render_keys = [k for k in all_keys if k.startswith("render-")]
    parts.append(chart("Render time (s, one run of the show)", history,
                       lambda h, k: h["suites"].get(k, {}).get("render_seconds"), render_keys))
    parts.append("</div></section>")
    parts.append(cov_body)

    parts.append('<section><h2>Recent runs</h2><div class="scroll"><table><tr><th>Date</th><th>Commit</th><th>Run</th>')
    for k in all_keys:
        s = next((h["suites"][k] for h in reversed(history) if k in h["suites"]), None)
        parts.append("<th>%s</th>" % esc(label(s["suite"], s["platform"]) if s else k))
    has_cov = any(h.get("coverage") for h in history)
    if has_cov:
        parts.append("<th>Line coverage</th>")
    parts.append("</tr>")
    for h in reversed(history[-RECENT:]):
        parts.append("<tr><td>%s</td><td>%s</td><td>%s</td>" % (
            esc(h["date"].replace("T", " ")),
            ('<a href="https://github.com/%s/commit/%s">%s</a>' % (REPO, esc(h["sha"]), esc(h["sha"][:10]))) if h.get("sha") else "",
            ('<a href="%s">log</a>' % esc(h["run"])) if h.get("run") else ""))
        for k in all_keys:
            s = h["suites"].get(k)
            if not s:
                parts.append('<td class="none">-</td>')
            elif ok(s):
                parts.append('<td class="good">&#10003; %d</td>' % s["tests"])
            else:
                parts.append('<td class="bad">&#10007; %d / %d</td>' % (s["failed"], s["tests"]))
        if has_cov:
            c = h.get("coverage")
            parts.append("<td>%.1f%%</td>" % pct(c["lines_hit"], c["lines"]) if c else '<td class="none">-</td>')
        parts.append("</tr>")
    parts.append("</table></div></section>")

    parts.append('<p class="note">Produced by the <a href="https://github.com/%s/actions/workflows/Nightly_Tests.yml">'
                 'Nightly Tests</a> workflow. Raw history: <a href="history.json">history.json</a>.</p>' % REPO)
    parts.append("</main></body></html>")
    return "".join(parts)


def main(argv):
    args = [a for a in argv if not a.startswith("--")]
    results_file, site, history_file, run_url = args[:4]
    no_entry = "--no-entry" in argv
    drop = [a for i, a in enumerate(argv) if i and argv[i - 1] == "--drop-run"]
    history = load_history(history_file, drop)
    if not no_entry:
        with open(results_file) as f:
            entry = make_entry(json.load(f), run_url)
        history = [h for h in history if h.get("run") != run_url]
        history.append(entry)
    history.sort(key=lambda h: h["date"])
    history = history[-MAX_HISTORY:]
    if not history:
        sys.exit("no history to render")
    os.makedirs(site, exist_ok=True)
    report = None
    try:
        with open(os.path.join(site, "coverage", "totals.json")) as f:
            report = json.load(f)
    except (OSError, ValueError):
        report = None
    with open(os.path.join(site, "history.json"), "w") as f:
        json.dump(history, f, indent=1)
        f.write("\n")
    with open(os.path.join(site, "index.html"), "w") as f:
        f.write(render(history, report))
    print("wrote %s (%d runs)" % (os.path.join(site, "index.html"), len(history)))


if __name__ == "__main__":
    main(sys.argv[1:])
