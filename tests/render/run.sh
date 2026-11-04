#!/usr/bin/env bash
# Render-regression harness: render every .xsq in a test show headlessly on the
# CPU-only path, hash the .fseq output with `xLights --fseqhash`, and compare to
# the committed manifest. Also renders twice and requires identical hashes, which
# is the run-to-run determinism check (no stored data needed for that one).
#
#   tests/render/run.sh --bin <xLights binary> [--data <xLights-test-data dir>]
#                       [--show basic] [--filter <substring>] [--update]
#                       [--save <dir>] [--keep]
#
# Scratch files are always deleted on exit (pass/fail/error) so nothing is left
# behind -- on macOS they live inside the app's sandbox container. --save <dir>
# first copies the hash reports, manifest diff and render logs out to <dir>;
# --keep skips the cleanup (then clean up by hand, see the notice printed).
#
# Exit 0 = all match; 1 = mismatch/non-determinism; 2 = setup error.
set -u
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BIN=""; DATA="${XL_TEST_DATA:-}"; SHOW="basic"; FILTER=""; UPDATE=0; SAVE=""; KEEP=0
while [ $# -gt 0 ]; do
    case "$1" in
        --bin) BIN="$2"; shift 2;;
        --data) DATA="$2"; shift 2;;
        --show) SHOW="$2"; shift 2;;
        --filter) FILTER="$2"; shift 2;;
        --update) UPDATE=1; shift;;
        --save) SAVE="$2"; shift 2;;
        --keep) KEEP=1; shift;;
        *) echo "unknown argument: $1" >&2; exit 2;;
    esac
done
[ -x "$BIN" ] || { echo "--bin <xLights executable> is required" >&2; exit 2; }
# A binary that predates --fseqhash would pop an "unrecognized option" dialog
# on every call instead of failing, so check the option table up front.
strings -a "$BIN" | grep -q -- "fseqhash" || { echo "$BIN does not support --fseqhash; rebuild it" >&2; exit 2; }
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
        "$WORK"/expected.txt "$WORK"/manifest.diff
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
render "$WORK/out1" || exit 2
render "$WORK/out2" || exit 2
hashes "$WORK/out1" >"$WORK/hashes1.txt"
hashes "$WORK/out2" >"$WORK/hashes2.txt"
[ -s "$WORK/hashes1.txt" ] || { echo "no hashes produced" >&2; exit 2; }

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
exit $rc
