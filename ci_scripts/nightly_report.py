#!/usr/bin/env python3
"""Aggregate the JUnit reports a nightly run produced into a markdown summary
and a results.json (the dashboard's source of truth).

Each report lives in a directory named after its artifact, e.g.
  <results>/results-render-linux-x64/render-linux-x64.xml
and the directory name gives the suite ("render") and platform ("linux-x64").

  nightly_report.py --results <dir> [--expect results-unit-linux-x64,...]
                    [--summary <file.md>] [--json <file.json>] [--fail-on-failures]

A name in --expect with no report counts as a failure ("no report"): a suite
that crashed before writing its XML must not look like a pass. Only the
standard library is used so it runs on any CI runner.
"""
import argparse
import datetime
import json
import os
import sys
import xml.etree.ElementTree as ET

MAX_LISTED = 20


def parse_report(path):
    """Return (tests, failed[(name, message)], skipped, seconds, properties) for one XML file."""
    root = ET.parse(path).getroot()
    suites = [root] if root.tag == "testsuite" else list(root.iter("testsuite"))
    tests = skipped = 0
    seconds = 0.0
    failed = []
    props = {}
    for suite in suites:
        try:
            seconds += float(suite.get("time", "0") or 0)
        except ValueError:
            pass
        for p in suite.findall("properties/property"):
            props[p.get("name")] = p.get("value")
        for case in suite.findall("testcase"):
            tests += 1
            bad = case.find("failure")
            if bad is None:
                bad = case.find("error")
            if bad is not None:
                msg = (bad.get("message") or (bad.text or "")).strip().splitlines()
                failed.append((case.get("name", "?"), msg[0][:200] if msg else ""))
            elif case.find("skipped") is not None:
                skipped += 1
    return tests, failed, skipped, seconds, props


def split_id(dirname):
    name = dirname[len("results-"):] if dirname.startswith("results-") else dirname
    suite, _, platform = name.partition("-")
    return suite or name, platform or "-"


def collect(results_dir):
    found = {}
    for entry in sorted(os.listdir(results_dir)):
        d = os.path.join(results_dir, entry)
        if not os.path.isdir(d):
            continue
        xmls = []
        for dp, _, files in os.walk(d):
            xmls += [os.path.join(dp, f) for f in sorted(files) if f.endswith(".xml")]
        if not xmls:
            continue
        suite, platform = split_id(entry)
        agg = {"artifact": entry, "suite": suite, "platform": platform, "tests": 0, "skipped": 0,
               "time": 0.0, "failed": [], "properties": {}, "status": "ok"}
        for x in xmls:
            try:
                tests, failed, skipped, seconds, props = parse_report(x)
            except ET.ParseError as e:
                agg["failed"].append((os.path.basename(x), "unreadable report: %s" % e))
                continue
            agg["tests"] += tests
            agg["skipped"] += skipped
            agg["time"] += seconds
            agg["failed"] += failed
            agg["properties"].update(props)
        if agg["failed"]:
            agg["status"] = "failed"
        found[entry] = agg
    return found


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results", required=True)
    ap.add_argument("--expect", default="")
    ap.add_argument("--summary")
    ap.add_argument("--json")
    ap.add_argument("--fail-on-failures", action="store_true")
    args = ap.parse_args()

    found = collect(args.results) if os.path.isdir(args.results) else {}
    for name in [n for n in args.expect.split(",") if n]:
        if name not in found:
            suite, platform = split_id(name)
            found[name] = {"artifact": name, "suite": suite, "platform": platform, "tests": 0,
                           "skipped": 0, "time": 0.0, "failed": [("(whole suite)", "no report was produced")],
                           "properties": {}, "status": "no-report"}
    suites = sorted(found.values(), key=lambda s: (s["suite"], s["platform"]))

    env = os.environ
    meta = {
        "date": datetime.datetime.now(datetime.timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "sha": env.get("GITHUB_SHA", ""),
        "run_id": env.get("GITHUB_RUN_ID", ""),
        "run_attempt": env.get("GITHUB_RUN_ATTEMPT", ""),
        "event": env.get("GITHUB_EVENT_NAME", ""),
        "repository": env.get("GITHUB_REPOSITORY", ""),
    }
    total_tests = sum(s["tests"] for s in suites)
    total_failed = sum(len(s["failed"]) for s in suites)

    lines = ["### Nightly tests - %s %s" % (meta["date"][:10], ("(" + meta["sha"][:10] + ")") if meta["sha"] else ""),
             "", "| Suite | Platform | Tests | Failed | Skipped | Time (s) |", "|---|---|---:|---:|---:|---:|"]
    for s in suites:
        mark = "" if s["status"] == "ok" else (" **no report**" if s["status"] == "no-report" else "")
        lines.append("| %s | %s | %d | %d%s | %d | %.0f |" % (
            s["suite"], s["platform"], s["tests"], len(s["failed"]), mark, s["skipped"], s["time"]))
    lines.append("")
    lines.append("**%d tests, %d failed.**" % (total_tests, total_failed))
    for s in suites:
        if not s["failed"]:
            continue
        lines += ["", "#### %s (%s)" % (s["suite"], s["platform"])]
        for name, msg in s["failed"][:MAX_LISTED]:
            lines.append("- `%s`%s" % (name, (": " + msg) if msg else ""))
        if len(s["failed"]) > MAX_LISTED:
            lines.append("- ...and %d more" % (len(s["failed"]) - MAX_LISTED))
    md = "\n".join(lines) + "\n"

    out = dict(meta)
    out["totals"] = {"tests": total_tests, "failed": total_failed, "suites": len(suites)}
    out["suites"] = [{"suite": s["suite"], "platform": s["platform"], "artifact": s["artifact"],
                      "status": s["status"], "tests": s["tests"], "failed_count": len(s["failed"]),
                      "skipped": s["skipped"], "time": round(s["time"], 2),
                      "failed": [{"name": n, "message": m} for n, m in s["failed"]],
                      "properties": s["properties"]} for s in suites]

    if args.summary:
        with open(args.summary, "a") as f:
            f.write(md)
    if args.json:
        with open(args.json, "w") as f:
            json.dump(out, f, indent=2)
            f.write("\n")
    sys.stdout.write(md)
    return 1 if (args.fail_on_failures and total_failed) else 0


if __name__ == "__main__":
    sys.exit(main())
