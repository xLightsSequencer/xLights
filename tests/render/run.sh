#!/usr/bin/env bash
# Render-regression harness: render every .xsq in a test show headlessly on the
# CPU-only path, hash the .fseq output with `xLights --fseqhash`, and compare to
# the committed manifest. Also renders twice and requires identical hashes, which
# is the run-to-run determinism check (no stored data needed for that one).
#
#   tests/render/run.sh --bin <xLights binary> [--data <xLights-test-data dir>]
#                       [--show basic] [--filter <substring>] [--update]
#                       [--save <dir>] [--keep] [--junit <file>]
#
# Scratch files are always deleted on exit (pass/fail/error) so nothing is left
# behind -- on macOS they live inside the app's sandbox container. --save <dir>
# first copies the hash reports, manifest diff and render logs out to <dir>;
# --keep skips the cleanup (then clean up by hand, see the notice printed).
# --junit <file> writes a JUnit XML report (one testcase per sequence: pass,
# hash mismatch, non-deterministic, or blank) for CI and dashboards.
#
# Exit 0 = all match; 1 = mismatch/non-determinism; 2 = setup error.
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN=""; DATA="${XL_TEST_DATA:-}"; SHOW="basic"; FILTER=""; UPDATE=0; SAVE=""; KEEP=0; JUNIT=""
while [ $# -gt 0 ]; do
    case "$1" in
        --bin) BIN="$2"; shift 2;;
        --data) DATA="$2"; shift 2;;
        --show) SHOW="$2"; shift 2;;
        --filter) FILTER="$2"; shift 2;;
        --update) UPDATE=1; shift;;
        --save) SAVE="$2"; shift 2;;
        --keep) KEEP=1; shift;;
        --junit) JUNIT="$2"; shift 2;;
        *) echo "unknown argument: $1" >&2; exit 2;;
    esac
done
[ -x "$BIN" ] || { echo "--bin <xLights executable> is required" >&2; exit 2; }
# A binary that predates --fseqhash would pop an "unrecognized option" dialog
# on every call instead of failing, so check its own option list up front.
"$BIN" --help 2>&1 | grep -q -- "fseqhash" || { echo "$BIN does not support --fseqhash; rebuild it" >&2; exit 2; }
if [ -z "$DATA" ]; then
    for d in "$HERE/../../../xLights-test-data" "$HOME/working/xLights-test-data"; do
        [ -d "$d/shows/$SHOW" ] && DATA="$d" && break
    done
fi
SRC="$DATA/shows/$SHOW"
[ -d "$SRC" ] || { echo "test show not found: $SRC (use --data)" >&2; exit 2; }
MANIFEST="$HERE/manifest-$SHOW.txt"
# The manifest is only valid for the data revision it was generated from.
PIN="$(tr -d '[:space:]' <"$HERE/data.version" 2>/dev/null)"
HAVE="$(git -C "$DATA" rev-parse HEAD 2>/dev/null)"
if [ -n "$PIN" ] && [ -n "$HAVE" ] && [ "$PIN" != "$HAVE" ]; then
    echo "WARNING: test data is at ${HAVE:0:10}, manifest was generated from ${PIN:0:10}" >&2
fi

# The sandboxed macOS app can only touch directories it holds a bookmark for,
# plus its own container, which it can always use. So on macOS the work dir
# defaults to the container's tmp (XL_TEST_BUNDLE_ID overrides the bundle id).
# Elsewhere a normal temp dir is used.
if [ "$(uname)" = "Darwin" ]; then
    CONTAINER="$HOME/Library/Containers/${XL_TEST_BUNDLE_ID:-org.xlights}/Data"
    [ -d "$CONTAINER" ] || { echo "no sandbox container $CONTAINER (run the app once, or set XL_TEST_BUNDLE_ID/XL_TEST_WORK)" >&2; exit 2; }
    WORK="${XL_TEST_WORK:-$CONTAINER/tmp/xlights-render-tests}"
else
    WORK="${XL_TEST_WORK:-${TMPDIR:-/tmp}/xlights-render-tests}"
fi
STAGE="$WORK/show-$SHOW"

# Remove only what this script creates (never the whole of a user-supplied
# XL_TEST_WORK), leaving the container exactly as found.
cleanup() {
    if [ $KEEP -eq 1 ]; then
        echo "NOTICE: --keep given; scratch left in $WORK -- delete it when done: rm -rf '$WORK'" >&2
        return
    fi
    if [ -n "$SAVE" ]; then
        mkdir -p "$SAVE" && cp -R "$WORK"/hashes*.txt "$WORK"/manifest.diff "$WORK"/expected.txt "$SAVE"/ 2>/dev/null
        local o; for o in out1 out2; do
            [ -d "$WORK/$o" ] && mkdir -p "$SAVE/$o" && cp -R "$WORK/$o"/. "$SAVE/$o"/
        done
    fi
    rm -rf "$STAGE" "$WORK/out1" "$WORK/out2" "$WORK"/hashes1.txt "$WORK"/hashes2.txt \
        "$WORK"/expected.txt "$WORK"/manifest.diff "$WORK"/tmp.*
    rmdir "$WORK" 2>/dev/null
}
trap cleanup EXIT
rm -rf "$STAGE" "$WORK/out1" "$WORK/out2"
mkdir -p "$STAGE" "$WORK/out1" "$WORK/out2" || exit 2
cp -R "$SRC/." "$STAGE/"

# Everything that can legitimately differ between hosts or runs is switched off:
# GPU compute, per-effect Metal, GPU shader context, threaded scheduling.
export XL_SERIAL=1 XL_NO_GPU_COMPUTE=1 XL_NO_METAL_FX=ALL XL_HEADLESS_NO_GL=1

render() { # outdir
    local files=()
    while IFS= read -r f; do files+=("$f"); done < <(ls "$STAGE"/*.xsq | grep -F -- "$FILTER" | sort)
    [ ${#files[@]} -gt 0 ] || { echo "no sequences match '$FILTER'" >&2; return 2; }
    "$BIN" --headless -s "$STAGE" --outputdir "$1" "${files[@]}" >"$1/render.log" 2>&1 || {
        echo "render failed (see $1/render.log)" >&2; return 1; }
    local f n=0
    for f in "${files[@]}"; do
        [ -s "$1/$(basename "${f%.xsq}").fseq" ] || { echo "no output for $(basename "$f")" >&2; n=1; }
    done
    return $n
}
hashes() { # outdir -> sorted hash report on stdout
    local files=()
    while IFS= read -r f; do files+=("$f"); done < <(ls "$1"/*.fseq | sort)
    "$BIN" --fseqhash -s "$STAGE" "${files[@]}" 2>/dev/null | grep -E '^(  model |[^ ].*  frames=)'
}

echo "rendering $(ls "$STAGE"/*.xsq | grep -c -F -- "$FILTER") sequence(s) from $SRC (x2 for determinism)"
T0=$(date +%s); render "$WORK/out1" || exit 2; T1=$(date +%s)
render "$WORK/out2" || exit 2; T2=$(date +%s)
hashes "$WORK/out1" >"$WORK/hashes1.txt"
hashes "$WORK/out2" >"$WORK/hashes2.txt"
[ -s "$WORK/hashes1.txt" ] || { echo "no hashes produced" >&2; exit 2; }
cp "$WORK/hashes1.txt" "$WORK/tmp.all1"

rc=0
if ! diff -q "$WORK/hashes1.txt" "$WORK/hashes2.txt" >/dev/null; then
    echo "NON-DETERMINISTIC: two identical renders hash differently:"
    diff "$WORK/hashes1.txt" "$WORK/hashes2.txt" | grep '^[<>]  *[^ ]' | head -20
    rc=1
fi

if [ $UPDATE -eq 1 ]; then
    if [ -n "$FILTER" ]; then echo "--update refuses to run with --filter" >&2; exit 2; fi
    cp "$WORK/hashes1.txt" "$MANIFEST"
    [ -n "$HAVE" ] && echo "$HAVE" >"$HERE/data.version"
    echo "wrote $MANIFEST ($(grep -c 'frames=' "$MANIFEST") sequences)"
    # Sequences that render identically are suspect: an effect that silently
    # does nothing hashes the same as "Off" and then guards nothing.
    grep 'frames=' "$MANIFEST" | awk '{h[$NF]=h[$NF]" "$1; c[$NF]++} END{for(k in c) if(c[k]>1) print "NOTE: identical output ("c[k]"):" h[k]}'
    exit $rc
fi

[ -f "$MANIFEST" ] || { echo "no manifest $MANIFEST (run with --update)" >&2; exit 2; }
if [ -n "$FILTER" ]; then
    # compare only the sequences we rendered
    awk -v pat="$FILTER" '/^[^ ]/{keep=index($1,pat)>0} keep' "$MANIFEST" >"$WORK/expected.txt"
else
    cp "$MANIFEST" "$WORK/expected.txt"
fi
# Determinism-only sequences (see determinism-only-<show>.txt): must not render blank, and are left out
# of the comparison against the recorded hashes.
DETONLY=""
if [ -f "$HERE/determinism-only-$SHOW.txt" ]; then
    DETONLY="$(grep -v '^[[:space:]]*#' "$HERE/determinism-only-$SHOW.txt" | tr -s '[:space:]' ' ')"
fi
if [ -n "$DETONLY" ]; then
    BLACK="$(awk '$1=="effect_off.fseq"{print $NF}' "$MANIFEST")"
    for n in $DETONLY; do
        got="$(awk -v n="$n.fseq" '$1==n{print $NF}' "$WORK/hashes1.txt")"
        [ -z "$got" ] && continue
        if [ -n "$BLACK" ] && [ "$got" = "$BLACK" ]; then
            echo "FAIL: $n renders blank (same hash as effect_off)"; rc=1
            echo "$n.fseq" >>"$WORK/tmp.blank"
        fi
    done
    dropdet() { awk -v skip="$DETONLY" 'BEGIN{k=split(skip,a," "); for(i=1;i<=k;i++) s[a[i]".fseq"]=1} /^[^ ]/{drop=($1 in s)} !drop' "$1" >"$1.tmp" && mv "$1.tmp" "$1"; }
    dropdet "$WORK/expected.txt"; dropdet "$WORK/hashes1.txt"
    echo "NOTE: not compared to the manifest (determinism-only): $DETONLY"
fi
if diff "$WORK/expected.txt" "$WORK/hashes1.txt" >"$WORK/manifest.diff"; then
    [ $rc -eq 0 ] && echo "PASS: $(grep -c 'frames=' "$WORK/hashes1.txt") sequences match the manifest"
else
    echo "FAIL: render output differs from the manifest. Changed sequences / models:"
    awk '/^[<>] [^ ]/{seq=$2} /^[<>]   model/{print "  " seq " " $3 " (" ($1=="<"?"expected":"got") ")"}' "$WORK/manifest.diff" | sort -u | head -40
    grep -E '^[<>] [^ ]' "$WORK/manifest.diff" | awk '{print "  " $2}' | sort -u | head -40
    head -60 "$WORK/manifest.diff"
    [ -n "$SAVE" ] && echo "full diff and outputs saved to $SAVE" || echo "(use --save <dir> to keep the full diff and rendered fseqs)"
    rc=1
fi

# JUnit report: one testcase per rendered sequence.
if [ -n "$JUNIT" ]; then
    xesc() { sed -e 's/&/\&amp;/g' -e 's/</\&lt;/g' -e 's/>/\&gt;/g' -e 's/"/\&quot;/g'; }
    blockmap() { awk '/^[^ ]/{if(n)print n"\t"b; n=$1; b=$0; next} {b=b"|"$0} END{if(n)print n"\t"b}' "$1" | sort; }
    blockmap "$WORK/tmp.all1" >"$WORK/tmp.b1"; blockmap "$WORK/hashes2.txt" >"$WORK/tmp.b2"
    blockmap "$WORK/expected.txt" >"$WORK/tmp.be"; blockmap "$WORK/hashes1.txt" >"$WORK/tmp.bg"
    awk -F'\t' 'NR==FNR{a[$1]=$2;next} ($1 in a)&&a[$1]!=$2{print $1}' "$WORK/tmp.b1" "$WORK/tmp.b2" >"$WORK/tmp.nondet"
    awk -F'\t' 'NR==FNR{e[$1]=$2;next} !($1 in e)||e[$1]!=$2{print $1}' "$WORK/tmp.be" "$WORK/tmp.bg" >"$WORK/tmp.mismatch"
    touch "$WORK/tmp.blank"
    : >"$WORK/tmp.cases"; NTEST=0; NFAIL=0
    for n in $(awk -F'\t' '{print $1}' "$WORK/tmp.b1"); do
        base="${n%.fseq}"; NTEST=$((NTEST+1))
        if grep -qxF "$n" "$WORK/tmp.blank"; then msg="renders blank (same hash as effect_off)"
        elif grep -qxF "$n" "$WORK/tmp.nondet"; then msg="non-deterministic: two identical renders hash differently"
        elif grep -qxF "$n" "$WORK/tmp.mismatch"; then msg="hash differs from the manifest"
        else msg=""; fi
        if [ -n "$msg" ]; then
            NFAIL=$((NFAIL+1))
            printf '    <testcase classname="render.%s" name="%s" time="0"><failure message="%s"/></testcase>\n' "$SHOW" "$(printf %s "$base" | xesc)" "$(printf %s "$msg" | xesc)" >>"$WORK/tmp.cases"
        elif printf ' %s ' "$DETONLY" | grep -qF " $base "; then
            printf '    <testcase classname="render.%s" name="%s" time="0"><system-out>determinism-only: not compared to the manifest</system-out></testcase>\n' "$SHOW" "$(printf %s "$base" | xesc)" >>"$WORK/tmp.cases"
        else
            printf '    <testcase classname="render.%s" name="%s" time="0"/>\n' "$SHOW" "$(printf %s "$base" | xesc)" >>"$WORK/tmp.cases"
        fi
    done
    mkdir -p "$(dirname "$JUNIT")"
    {
        echo '<?xml version="1.0" encoding="UTF-8"?>'
        printf '<testsuites>\n  <testsuite name="render.%s" tests="%s" failures="%s" errors="0" skipped="0" time="%s">\n' "$SHOW" "$NTEST" "$NFAIL" "$((T2-T0))"
        echo '    <properties>'
        printf '      <property name="platform" value="%s"/>\n' "$(uname -sm | xesc)"
        printf '      <property name="data_revision" value="%s"/>\n' "${HAVE:-unknown}"
        printf '      <property name="manifest_data_revision" value="%s"/>\n' "${PIN:-unknown}"
        printf '      <property name="render_seconds_run1" value="%s"/>\n' "$((T1-T0))"
        printf '      <property name="render_seconds_run2" value="%s"/>\n' "$((T2-T1))"
        printf '      <property name="determinism_only" value="%s"/>\n' "$(printf %s "$DETONLY" | xesc)"
        echo '    </properties>'
        cat "$WORK/tmp.cases"
        printf '  </testsuite>\n</testsuites>\n'
    } >"$JUNIT"
    echo "wrote $JUNIT ($NTEST testcases, $NFAIL failed)"
fi
exit $rc
