#!/usr/bin/env python3
"""Turn the .gcda counts of a --coverage build of the unit tests into reports.

    coverage_report.py REPO_ROOT BUILD_DIR OUT

Runs gcovr over BUILD_DIR, keeping only src-core (not the submodules or the
third-party code vendored into src-core), and writes to OUT:
  coverage.json   gcovr's JSON, the raw counts
  html/           browsable per-line detail
  functions.csv   every function with its call count, for finding the ones no test reaches
  summary.txt     per-file line and function coverage, least covered first
  summary.md      totals, each directory and the biggest gaps, in Markdown
  totals.json     the same numbers as data, for the nightly dashboard

A file whose object was never linked into the test executable still has its
.gcno, so it is counted with every line unreached rather than left out.
"""
import json
import os
import subprocess
import sys

EXCLUDE = [
    r"src-core/graphics/mapbox/",
    r"src-core/media/kiss_fft/",
    r"src-core/graphics/tiny_obj_loader\.h",
]


def pct(h, n):
    return "%.1f%%" % (100.0 * h / n) if n else "-"


def main(argv):
    if len(argv) != 3:
        sys.exit(__doc__)
    root, build, out = (os.path.abspath(a) for a in argv)
    os.makedirs(os.path.join(out, "html"), exist_ok=True)
    # Only the build's own targets: the Catch2 build under _deps compiles with
    # relative paths gcovr cannot resolve, and none of it is src-core anyway.
    cmd = ["gcovr", "-r", root, os.path.join(build, "CMakeFiles"), "--filter", os.path.join(root, "src-core") + "/",
           "-j", str(os.cpu_count() or 2),
           "--json", os.path.join(out, "coverage.json"),
           "--html-details", os.path.join(out, "html", "index.html"),
           "--html-title", "xLights src-core unit test coverage",
           "--print-summary",
           # The build uses -fprofile-update=atomic so threaded tests count correctly;
           # should gcov still report a negative count (gcc bug 68080), warn rather than fail.
           "--gcov-ignore-parse-errors=negative_hits.warn_once_per_file"]
    for e in EXCLUDE:
        cmd += ["--exclude", e]
    print(" ".join(cmd), flush=True)
    subprocess.run(cmd, check=True)

    with open(os.path.join(out, "coverage.json")) as f:
        data = json.load(f)
    rows, funcs = [], []
    for fl in data["files"]:
        name = fl["file"]
        lines = [l for l in fl["lines"] if not l.get("gcovr/noncode")]
        hit = sum(1 for l in lines if l["count"] > 0)
        fn = fl.get("functions", [])
        fhit = sum(1 for x in fn if x.get("execution_count", 0) > 0)
        rows.append((name, len(lines), hit, len(fn), fhit))
        for x in fn:
            funcs.append((name, x.get("lineno", 0), x.get("demangled_name") or x["name"], x.get("execution_count", 0)))
    rows = [r for r in rows if r[1]]
    rows.sort(key=lambda r: (r[2] / r[1], -r[1]))
    tl, th = sum(r[1] for r in rows), sum(r[2] for r in rows)
    tf, tfh = sum(r[3] for r in rows), sum(r[4] for r in rows)
    unreached = sum(1 for r in rows if not r[2])

    with open(os.path.join(out, "summary.txt"), "w") as s:
        s.write("%6s %6s %6s %6s %5s  file\n" % ("lines", "hit", "line%", "funcs", "hit"))
        for name, n, h, fn, fh in rows:
            s.write("%6d %6d %5.1f%% %6d %5d  %s\n" % (n, h, 100.0 * h / n, fn, fh, name))
        s.write("%6d %6d %5.1f%%  TOTAL\n" % (tl, th, 100.0 * th / tl if tl else 0))

    # [lines, hit, functions, hit, files] per directory, named without the src-core/ prefix.
    areas = {}
    for name, n, h, fn, fh in rows:
        d = os.path.dirname(name)
        d = d[len("src-core/"):] if d.startswith("src-core/") else ("(top level)" if d == "src-core" else d)
        a = areas.setdefault(d, [0, 0, 0, 0, 0])
        a[0] += n; a[1] += h; a[2] += fn; a[3] += fh; a[4] += 1
    with open(os.path.join(out, "totals.json"), "w") as t:
        json.dump({"lines": tl, "lines_hit": th, "functions": tf, "functions_hit": tfh,
                   "files": len(rows), "files_unreached": unreached, "areas": areas,
                   # [lines, hit, functions, hit] per file
                   "by_file": {r[0]: list(r[1:]) for r in rows}}, t)

    with open(os.path.join(out, "summary.md"), "w") as m:
        m.write("### Unit test coverage of `src-core`\n\n")
        m.write("**Lines %s** (%s of %s) · **functions %s** (%s of %s) · %d of %d files never reached\n\n" % (
            pct(th, tl), format(th, ","), format(tl, ","), pct(tfh, tf), format(tfh, ","), format(tf, ","),
            unreached, len(rows)))
        m.write("<details><summary>By source directory, most unreached lines first</summary>\n\n")
        m.write("| directory | files | lines | line % | function % | lines not reached |\n|---|--:|--:|--:|--:|--:|\n")
        for d, (n, h, fn, fh, files) in sorted(areas.items(), key=lambda kv: kv[1][1] - kv[1][0]):
            m.write("| `%s` | %d | %s | %s | %s | %s |\n" % (d, files, format(n, ","), pct(h, n), pct(fh, fn), format(n - h, ",")))
        m.write("\n</details>\n\n<details><summary>The 30 files with the most unreached lines</summary>\n\n")
        m.write("| file | lines | line % | function % | lines not reached |\n|---|--:|--:|--:|--:|\n")
        for name, n, h, fn, fh in sorted(rows, key=lambda r: r[2] - r[1])[:30]:
            m.write("| `%s` | %s | %s | %s | %s |\n" % (name, format(n, ","), pct(h, n), pct(fh, fn), format(n - h, ",")))
        m.write("\n</details>\n")

    with open(os.path.join(out, "functions.csv"), "w") as c:
        c.write("file,line,calls,function\n")
        for name, line, fn, n in sorted(funcs):
            c.write('%s,%d,%d,"%s"\n' % (name, line, n, fn.replace('"', "'")))
    print("src-core: lines %s, functions %s, %d of %d files never reached" % (pct(th, tl), pct(tfh, tf), unreached, len(rows)))


if __name__ == "__main__":
    main(sys.argv[1:])
