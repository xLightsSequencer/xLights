#!/usr/bin/env bash
# Tally maintainer for the platform-parity scorecards.
#
# Counting rule (canonical): a scorecard table row contributes 1 to each DISTINCT
# status emoji it contains. Single-status tables therefore contribute one mark per
# row; the multi-status matrices (06 model-type: create/edit/renders, 07 output-type:
# configure/live) contribute one mark per distinct status appearing in the row.
# Status emoji are RESERVED for status cells — in prose/evidence cells write the
# status name in words ("marked missing", "iPad-only"), never the emoji, or the
# tallies drift. `lint` enforces this.
#
# Usage:
#   ./tally.sh          # recompute + diff against 00-overview.md; exit 1 on drift
#   ./tally.sh fix      # rewrite 00-overview.md's tally table + index numbers in place
#   ./tally.sh lint     # flag rows carrying more status emoji than their table allows
set -euo pipefail
cd "$(dirname "$0")"

MODE="${1:-check}"
OV=00-overview.md
S_OK='✅'; S_PART='🟡'; S_MISS='❌'; S_INF='🚫'; S_BLUE='🔵'; S_NA='➖'

FILES=(01-file-lifecycle.md 02-sequencer-grid.md 03-timing-audio.md
       04-effects-catalog.md 05-color-value-curves.md 06-layout-models.md
       07-controllers-setup-upload.md 08-import-export.md 09-render-playback.md
       10-presets-views-perspectives.md 11-preferences-settings.md
       12-ai-automation-scripting.md 13-tools-diagnostics-help.md)

count() { grep '^|' "$1" | grep -c "$2" || true; }

# ---------- lint ----------
if [[ "$MODE" == "lint" ]]; then
  bad=0
  for f in "${FILES[@]}" 14-reverse-parity-ipad-only.md; do
    out=$(awk -v f="$f" '
      /^## /   { section=$0 }
      /^\| *[0-9]+ +\|/ {
        allowed=1
        if (f ~ /^04/ && section ~ /Per-effect matrix/) allowed=2
        if (f ~ /^06/ && section ~ /Model type matrix/) allowed=3
        if (f ~ /^07/ && section ~ /Output type matrix/) allowed=2
        n=0
        n+=gsub(/✅/,"✅"); n+=gsub(/🟡/,"🟡"); n+=gsub(/❌/,"❌")
        n+=gsub(/🚫/,"🚫"); n+=gsub(/🔵/,"🔵"); n+=gsub(/➖/,"➖")
        if (n>allowed) printf "%s:%d: %d status emoji (max %d): %.100s\n", f, NR, n, allowed, $0
      }' "$f")
    [[ -n "$out" ]] && { echo "$out"; bad=1; }
  done
  [[ $bad == 0 ]] && echo "lint clean — status emoji appear only in status cells"
  exit $bad
fi

# ---------- compute ----------
declare -a OKS PARTS MISSES INFS BLUES
tOK=0; tPART=0; tMISS=0; tINF=0; tBLUE=0
for i in "${!FILES[@]}"; do
  f="${FILES[$i]}"
  OKS[$i]=$(count "$f" "$S_OK");   PARTS[$i]=$(count "$f" "$S_PART")
  MISSES[$i]=$(count "$f" "$S_MISS"); INFS[$i]=$(count "$f" "$S_INF")
  BLUES[$i]=$(count "$f" "$S_BLUE")
  tOK=$((tOK+OKS[$i])); tPART=$((tPART+PARTS[$i])); tMISS=$((tMISS+MISSES[$i]))
  tINF=$((tINF+INFS[$i])); tBLUE=$((tBLUE+BLUES[$i]))
done
NA11=$(count 11-preferences-settings.md "$S_NA")
BLUE14=$(count 14-reverse-parity-ipad-only.md "$S_BLUE")
meaningful=$((tOK+tPART+tMISS))
pFull=$(( (tOK*100 + meaningful/2) / meaningful ))
pFP=$(( ((tOK+tPART)*100 + meaningful/2) / meaningful ))
pHalf=$(( ((tOK*2+tPART)*100 + meaningful*2/2) / (meaningful*2) ))

echo "theme                              ✅    🟡    ❌    🚫    🔵"
for i in "${!FILES[@]}"; do
  printf "%-34s %4d  %4d  %4d  %4d  %4d\n" "${FILES[$i]}" \
    "${OKS[$i]}" "${PARTS[$i]}" "${MISSES[$i]}" "${INFS[$i]}" "${BLUES[$i]}"
done
printf "%-34s %4d  %4d  %4d  %4d  %4d\n" "TOTAL(01-13)" $tOK $tPART $tMISS $tINF $tBLUE
echo "11 ➖=$NA11   14 🔵=$BLUE14   meaningful=$meaningful   full=${pFull}%   full+partial=${pFP}%   half-weight=${pHalf}%"

# ---------- check / fix against the overview ----------
drift=0
for i in "${!FILES[@]}"; do
  nn=$(printf "%02d" $((i+1)))
  line=$(grep -m1 "^| $nn " "$OV" || true)
  [[ -z "$line" ]] && { echo "MISSING overview row for theme $nn"; drift=1; continue; }
  read -r o p m inf b <<<"$(echo "$line" | awk -F'|' '{gsub(/ /,"");print $3,$4,$5,$6,$7}')"
  want="${OKS[$i]} ${PARTS[$i]} ${MISSES[$i]} ${INFS[$i]} ${BLUES[$i]}"
  have="$o $p $m $inf $b"
  [[ "$want" != "$have" ]] && { echo "DRIFT theme $nn: overview has [$have], computed [$want]"; drift=1; }
done
tot=$(grep -m1 '^| \*\*Total' "$OV" | awk -F'|' '{gsub(/[* ]/,"");print $3,$4,$5,$6,$7}')
[[ "$tot" != "$tOK $tPART $tMISS $tINF $tBLUE" ]] && { echo "DRIFT totals: overview [$tot], computed [$tOK $tPART $tMISS $tINF $tBLUE]"; drift=1; }

if [[ "$MODE" == "fix" ]]; then
  tmpmap=$(mktemp)
  for i in "${!FILES[@]}"; do
    printf "%02d %d %d %d %d %d\n" $((i+1)) "${OKS[$i]}" "${PARTS[$i]}" "${MISSES[$i]}" "${INFS[$i]}" "${BLUES[$i]}" >>"$tmpmap"
  done
  awk -v map="$tmpmap" -v tok=$tOK -v tpart=$tPART -v tmiss=$tMISS -v tinf=$tINF -v tblue=$tBLUE '
    BEGIN { while ((getline l < map) > 0) { split(l,a," "); for(j=2;j<=6;j++) M[a[1]"."j]=a[j] } }
    /^\| [0-9][0-9] / {
      nn=substr($0,3,2)
      if (M[nn".2"] != "") {
        n=split($0,c,"|")
        c[3]=" "M[nn".2"]" "; c[4]=" "M[nn".3"]" "; c[5]=" "M[nn".4"]" "
        c[6]=" "M[nn".5"]" "; c[7]=" "M[nn".6"]" "
        out=""; for(j=2;j<n;j++) out=out"|"c[j]; print out"|"; next
      }
    }
    /^\| \*\*Total \(01–13\)\*\*/ {
      printf "| **Total (01–13)** | **%d** | **%d** | **%d** | **%d** | **%d** | |\n", tok, tpart, tmiss, tinf, tblue; next
    }
    { print }
  ' "$OV" >"$OV.tmp" && mv "$OV.tmp" "$OV"
  rm -f "$tmpmap"
  meaningfulFmt=$(LC_NUMERIC=en_US.UTF-8 printf "%'d" "$meaningful" 2>/dev/null || echo "$meaningful")
  perl -0777 -i -pe "
    s/of the [0-9,]+ rows where an iPad status is meaningful/of the $meaningfulFmt rows where an iPad status is meaningful/;
    s/\*\*[0-9]+%\*\*(\s+)are(\s+)at\s+full parity/**${pFull}%**\$1are\$2at full parity/;
    s/\*\*[0-9]+%\*\*(\s+)at full-or-partial/**${pFP}%**\$1at full-or-partial/;
    s/\*\*≈[0-9]+% of desktop\*\*/**≈${pHalf}% of desktop**/;
    s/The 🚫 bucket\s*\([0-9]+\)/The 🚫 bucket (${tINF})/;
    s/Theme 11 additionally has [0-9]+ ➖ rows/Theme 11 additionally has ${NA11} ➖ rows/;
    s/Theme 14 \(reverse parity\) now has \*\*[0-9]+\*\* 🔵 rows/Theme 14 (reverse parity) now has **${BLUE14}** 🔵 rows/;
  " "$OV"
  echo "fixed $OV (table cells, totals, parity-index numbers)"
  exit 0
fi

if [[ $drift == 0 ]]; then echo "overview matches computed tallies"; fi
exit $drift
