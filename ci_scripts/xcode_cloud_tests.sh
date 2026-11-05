#!/bin/bash
# Run the Catch2 unit tests and the render regression on an Xcode Cloud macOS
# builder, summarize them, and push the result to GitHub (repository_dispatch,
# handled by .github/workflows/Xcode_Cloud_Results.yml, which adds it to the
# dashboard). Called from ci_post_xcodebuild.sh after the archive action, and
# runnable by hand on a Mac for testing.
#
#   ci_scripts/xcode_cloud_tests.sh <summary-file>
#
# <summary-file> receives a one-line summary for the TestFlight "What to Test"
# text. Best effort: every step has a time limit and a failure here only shows
# up in the results, never as a failed build.
#
# Xcode Cloud workflow settings:
#   XL_NIGHTLY_TESTS=1            turn this on (ci_post_xcodebuild.sh checks it)
#   XL_DISPATCH_TOKEN (secret) fine-grained token, xLights repo, Contents: write
# Optional for local testing:
#   XL_APP_PATH      xLights.app to use instead of the archive's
#   XL_NIGHTLY_DRY_RUN=1  the GitHub workflow builds the page but does not publish it
#   XL_SKIP_UNIT=1 / XL_SKIP_RENDER=1
#   XL_RENDER_JUNIT_FILE  use this existing render JUnit report instead of running the regression
#   XL_ALLOW_LOCAL_RENDER=1  run the render regression outside Xcode Cloud (see the note in that step)
SUMMARY_FILE="${1:-/dev/null}"
REPO="${CI_PRIMARY_REPOSITORY_PATH:-$(cd "$(dirname "$0")/.." && pwd)}"
OUT="${TMPDIR:-/tmp}/xl-nightly-tests"
PLAT="macos-$(uname -m | sed 's/aarch64/arm64/')"
rm -rf "$OUT"
RES="$OUT/results"       # only the JUnit reports, one directory per suite, for nightly_report.py
mkdir -p "$RES/results-unit-$PLAT" "$RES/results-render-$PLAT"

# macOS has no timeout(1); perl's alarm does the job.
limit() { local secs=$1; shift; perl -e 'alarm shift; exec @ARGV' "$secs" "$@"; }
log() { echo "[xcode-cloud-tests] $*"; }

PROJECT="$REPO/macOS/xLights.xcodeproj"
DD="${CI_DERIVED_DATA_PATH:-$OUT/derived}"
# Where a scheme's product ends up, whatever the project's build-location settings are.
product() {  # scheme -> path of the built product
    xcodebuild -project "$PROJECT" -scheme "$1" -configuration Release -derivedDataPath "$DD" -showBuildSettings 2>/dev/null |
        awk -F' = ' '/^ *TARGET_BUILD_DIR = /{d=$2} /^ *FULL_PRODUCT_NAME = /{n=$2} END{if(d&&n) print d"/"n}'
}

# ---------------------------------------------------------------- unit tests
if [ -z "${XL_SKIP_UNIT:-}" ]; then
    git -C "$REPO" submodule update --init dependencies/Catch2 >"$OUT/submodule.log" 2>&1
    log "building xLights-unit-tests"
    limit 3000 xcodebuild -project "$PROJECT" -scheme xLights-unit-tests -configuration Release \
        -derivedDataPath "$DD" CODE_SIGNING_ALLOWED=NO build >"$OUT/unit-build.log" 2>&1
    UBIN="$(product xLights-unit-tests)"
    if [ -x "$UBIN" ]; then
        log "running $UBIN"
        (cd "$REPO" && limit 900 "$UBIN" -r junit -o "$RES/results-unit-$PLAT/unit-$PLAT.xml") >"$OUT/unit.log" 2>&1
        tail -3 "$OUT/unit.log"
    else
        log "no unit-test executable was built (see unit-build.log):"; tail -15 "$OUT/unit-build.log"
    fi
fi

# ------------------------------------------------------------ render regression
if [ -n "${XL_RENDER_JUNIT_FILE:-}" ]; then
    cp "$XL_RENDER_JUNIT_FILE" "$RES/results-render-$PLAT/render-$PLAT.xml"
elif [ -z "${XL_SKIP_RENDER:-}" ] && [ -z "${CI_BUILD_ID:-}" ] && [ -z "${XL_ALLOW_LOCAL_RENDER:-}" ]; then
    # A re-signed copy of the app has the same bundle id as the real one, so launching it on a developer Mac
    # registers it with Launch Services, and a launch killed by a timeout makes macOS ask to "Reopen" windows.
    log "skipping the render regression outside Xcode Cloud (set XL_ALLOW_LOCAL_RENDER=1 to run it anyway)"
elif [ -z "${XL_SKIP_RENDER:-}" ]; then
    # Never reopen windows from an earlier launch; on a one-shot builder this only avoids a stray prompt.
    [ -n "${CI_BUILD_ID:-}" ] && defaults write org.xlights ApplePersistenceIgnoreState -bool YES 2>/dev/null
    APP="${XL_APP_PATH:-}"
    [ -z "$APP" ] && [ -d "${CI_ARCHIVE_PATH:-}/Products/Applications/xLights.app" ] && APP="$CI_ARCHIVE_PATH/Products/Applications/xLights.app"
    [ -z "$APP" ] && [ -d "${CI_APP_STORE_SIGNED_APP_PATH:-}/xLights.app" ] && APP="$CI_APP_STORE_SIGNED_APP_PATH/xLights.app"
    if [ -n "$APP" ] && [ -d "$APP" ]; then
        log "render regression with $APP"
        # The shipped app is App Store signed and sandboxed, so it will not launch here and cannot reach
        # a plain directory. Test a copy re-signed ad hoc with only the entitlement it needs to load its libraries.
        cp -R "$APP" "$OUT/xLights.app"
        rm -f "$OUT/xLights.app/Contents/embedded.provisionprofile"
        cat >"$OUT/entitlements.plist" <<'PLIST'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0"><dict><key>com.apple.security.cs.disable-library-validation</key><true/></dict></plist>
PLIST
        codesign --force --deep --sign - --entitlements "$OUT/entitlements.plist" "$OUT/xLights.app" >"$OUT/codesign.log" 2>&1 ||
            { log "re-signing failed:"; cat "$OUT/codesign.log"; }
        DATA_REV="$(tr -d '[:space:]' <"$REPO/tests/render/data.version")"
        if limit 300 git clone --quiet https://github.com/xLightsSequencer/xLights-test-data.git "$OUT/test-data" &&
            git -C "$OUT/test-data" checkout --quiet "$DATA_REV"; then
            XL_TEST_WORK="$OUT/render-work" limit 900 bash "$REPO/tests/render/run.sh" \
                --bin "$OUT/xLights.app/Contents/MacOS/xLights" --data "$OUT/test-data" \
                --junit "$RES/results-render-$PLAT/render-$PLAT.xml" >"$OUT/render.log" 2>&1
            tail -6 "$OUT/render.log"
        else
            log "could not fetch the test data at $DATA_REV"
        fi
    else
        log "no xLights.app to test (set XL_APP_PATH, or run after the archive action)"
    fi
fi

# ------------------------------------------------------------------ aggregate
export GITHUB_SHA="${CI_COMMIT:-$(git -C "$REPO" rev-parse HEAD 2>/dev/null)}"
export GITHUB_RUN_ID="${CI_BUILD_NUMBER:-0}" GITHUB_EVENT_NAME="xcode-cloud" GITHUB_REPOSITORY="xLightsSequencer/xLights"
python3 "$REPO/ci_scripts/nightly_report.py" --results "$RES" \
    --expect "results-unit-$PLAT,results-render-$PLAT" --json "$OUT/results.json" >"$OUT/report.md"
cat "$OUT/report.md"

python3 - "$OUT/results.json" "$PLAT" >"$SUMMARY_FILE" <<'PY'
import json, sys
d = json.load(open(sys.argv[1])); parts = []
for s in d["suites"]:
    parts.append("%s NO REPORT" % s["suite"] if s["status"] == "no-report"
                 else "%s %d/%d" % (s["suite"], s["tests"] - s["failed_count"], s["tests"]))
bad = d["totals"]["failed"]
print("Tests (%s): %s%s" % (sys.argv[2], ", ".join(parts), "" if not bad else "  -- %d FAILED" % bad))
PY
log "summary: $(cat "$SUMMARY_FILE" 2>/dev/null)"

# ------------------------------------------------------------- push to GitHub
if [ -n "${XL_DISPATCH_TOKEN:-}" ]; then
    python3 - "$OUT/results.json" "$OUT/payload.json" <<'PY'
import json, os, sys
res = json.load(open(sys.argv[1]))
payload = {"event_type": "xcode-cloud-results",
           "client_payload": {"results": res, "dry_run": os.environ.get("XL_NIGHTLY_DRY_RUN") == "1",
                              "build_number": os.environ.get("CI_BUILD_NUMBER", ""), "workflow": os.environ.get("CI_WORKFLOW", "")}}
if len(json.dumps(payload)) > 60000:          # the API limit is 65535 characters: keep fewer failure details
    for s in res["suites"]:
        s["failed"] = s["failed"][:5]
        s["skipped_tests"] = s.get("skipped_tests", [])[:5]
with open(sys.argv[2], "w") as f:
    json.dump(payload, f)
PY
    CODE=$(curl -sS --max-time 60 -o "$OUT/dispatch.out" -w '%{http_code}' -X POST \
        -H "Authorization: Bearer $XL_DISPATCH_TOKEN" -H "Accept: application/vnd.github+json" \
        https://api.github.com/repos/xLightsSequencer/xLights/dispatches --data @"$OUT/payload.json")
    log "GitHub dispatch: HTTP $CODE"
    [ "$CODE" = "204" ] || head -c 300 "$OUT/dispatch.out"
else
    log "XL_DISPATCH_TOKEN is not set; not pushing results"
fi
exit 0
