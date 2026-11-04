#!/usr/bin/env python3
"""Build the nightly test dashboard published to GitHub Pages.

    build_dashboard.py RESULTS_JSON SITE HISTORY_JSON RUN_URL [--no-entry] [--drop-run ID]

RESULTS_JSON is the nightly_report.py output for this run. HISTORY_JSON is the
published site's history.json (a missing file means the first run). Writes
SITE/index.html and SITE/history.json with this run added to the history.
The page is static (inline CSS and SVG, no scripts) so it needs no build step
on the Pages side. --no-entry re-renders the page from its history without
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
        }
    if not str(run_url).startswith(("https://", "http://")):
        run_url = ""
    return {"date": results["date"][:16] + "Z", "run": run_url, "sha": results.get("sha", ""),
            "event": results.get("event", ""), "suites": suites}


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


def chart(title, history, series_fn, keys, unit=""):
    """Inline SVG line chart. series_fn(entry, key) -> number or None."""
    pts = {k: [(i, series_fn(h, k)) for i, h in enumerate(history)] for k in keys}
    pts = {k: [(i, v) for i, v in p if v is not None] for k, p in pts.items()}
    pts = {k: p for k, p in pts.items() if p}
    out = ["<div><h2>%s</h2>" % esc(title)]
    if len(history) < 2 or not pts:
        out.append('<p class="note">Not enough history yet.</p></div>')
        return "".join(out)
    W, H, L, R, T, B = 520, 200, 44, 12, 10, 28
    vmax = nice_max(max(v for p in pts.values() for _, v in p))
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
        s = history[-1]["suites"].get(k) or next((h["suites"][k] for h in reversed(history) if k in h["suites"]), None)
        legend.append('<span><i style="background:%s"></i>%s</span>' % (color, esc(label(s["suite"], s["platform"]) if s else k)))
    svg.append("</svg>")
    out.append("".join(svg))
    out.append('<div class="legend">%s</div></div>' % "".join(legend))
    return "".join(out)


def render(history):
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

    parts.append('<section><div class="charts">')
    parts.append(chart("Tests per run", history, lambda h, k: h["suites"].get(k, {}).get("tests"), all_keys))
    parts.append(chart("Failures per run", history, lambda h, k: h["suites"].get(k, {}).get("failed"), all_keys))
    render_keys = [k for k in all_keys if k.startswith("render-")]
    parts.append(chart("Render time (s, one run of the show)", history,
                       lambda h, k: h["suites"].get(k, {}).get("render_seconds"), render_keys))
    parts.append("</div></section>")

    parts.append('<section><h2>Recent runs</h2><div class="scroll"><table><tr><th>Date</th><th>Commit</th><th>Run</th>')
    for k in all_keys:
        s = next((h["suites"][k] for h in reversed(history) if k in h["suites"]), None)
        parts.append("<th>%s</th>" % esc(label(s["suite"], s["platform"]) if s else k))
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
    with open(os.path.join(site, "history.json"), "w") as f:
        json.dump(history, f, indent=1)
        f.write("\n")
    with open(os.path.join(site, "index.html"), "w") as f:
        f.write(render(history))
    print("wrote %s (%d runs)" % (os.path.join(site, "index.html"), len(history)))


if __name__ == "__main__":
    main(sys.argv[1:])
