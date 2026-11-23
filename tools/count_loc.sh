#!/usr/bin/env bash
#
# This MobilityDB code is provided under The PostgreSQL License.
# Copyright (c) 2016-2026, Université libre de Bruxelles and MobilityDB
# contributors
#
# MobilityDB includes portions of PostGIS version 3 source code released
# under the GNU General Public License (GPLv2 or later).
# Copyright (c) 2001-2026, PostGIS contributors
#
# Permission to use, copy, modify, and distribute this software and its
# documentation for any purpose, without fee, and without a written
# agreement is hereby granted, provided that the above copyright notice and
# this paragraph and the following two paragraphs appear in all copies.
#
# IN NO EVENT SHALL UNIVERSITE LIBRE DE BRUXELLES BE LIABLE TO ANY PARTY FOR
# DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING
# LOST PROFITS, ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
# EVEN IF UNIVERSITE LIBRE DE BRUXELLES HAS BEEN ADVISED OF THE POSSIBILITY
# OF SUCH DAMAGE.
#
# UNIVERSITE LIBRE DE BRUXELLES SPECIFICALLY DISCLAIMS ANY WARRANTIES,
# INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
# AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON
# AN "AS IS" BASIS, AND UNIVERSITE LIBRE DE BRUXELLES HAS NO OBLIGATIONS TO
# PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
#

# Count the lines of code of MobilityDB, MEOS and every binding with cloc, on a
# fresh shallow clone of the default branch of each github.com/MobilityDB/<repo>,
# and write them to doc/contributing/generation/LINES-OF-CODE.md, as
# tools/io_matrix.py writes the matrix of the input and output plan beside its
# notes. MobilityNebula is counted without the NebulaStream engine it carries:
# the lines its default branch adds or modifies over its merge base with
# nebulastream/nebulastream main (cloc --diff). The section "Notes" of the file
# is written by hand and kept from one run to the next.
#
# Usage: tools/count_loc.sh [output]; LOC_WORK names the directory of the clones
# (default: count-loc-<pid> under ${TMPDIR:-/tmp}).

set -euo pipefail
ROOT=$(cd "$(dirname "$0")/.." && pwd)
OUT=${1:-$ROOT/doc/contributing/generation/LINES-OF-CODE.md}
WORK=${LOC_WORK:-${TMPDIR:-/tmp}/count-loc-$$}
mkdir -p "$WORK"
DATE=$(date +%F)
NES=https://github.com/nebulastream/nebulastream.git

REPO_GROUPS=(
  "Core|MobilityDB"
  "Programming languages|PyMEOS PyMEOS-CFFI JMEOS MEOS.NET MEOS.js GoMEOS meos-rs"
  "Batch and stream processors|MobilityDuck MobilitySpark MobilityFlink MobilityKafka MobilityNebula"
  "MobilityAPI|MobilityAPI MobilityAPI-Python"
  "MEOS-API|MEOS-API"
  "Benchmark|MobilityDB-BerlinMOD"
  "Index extension|mest"
  "Workshop|MobilityDB-workshop"
)
# The groups whose total is that of the bindings
BINDING_GROUPS="Programming languages|Batch and stream processors|MobilityAPI|MEOS-API"

# The hand-written notes of the previous file, kept as they are
NOTES=""
if [ -f "$OUT" ]; then
  NOTES=$(awk '/^## Notes$/{f=1;next} /^## /{f=0} f' "$OUT")
fi

# cloc's report holds one row per language: "<language> <files> <blank> <comment> <code>";
# the main languages are the first and every other holding at least a quarter of its lines
LANGS='/^-+$/{n++;next} n==2 && !/^SUM/{l=$0; sub(/ +[0-9]+ +[0-9]+ +[0-9]+ +[0-9]+$/,"",l);
  if (top == "") top=$NF; if ($NF*4 >= top) {printf "%s%s (%s)", (k++ ? ", " : ""), l, $NF}}'

declare -A GT GN
ROWS=""
NESBASE=""
for g in "${REPO_GROUPS[@]}"; do
  name=${g%%|*}; GT[$name]=0; GN[$name]=0; first=1
  for r in ${g#*|}; do
    [ -d "$WORK/$r" ] || git clone -q --depth 1 "https://github.com/MobilityDB/$r.git" "$WORK/$r"
    c=$(git -C "$WORK/$r" log -1 --abbrev=7 --format='%h` %cs')
    if [ "$r" = MobilityNebula ]; then
      # The lines MobilityNebula adds or modifies over its NebulaStream base
      git -C "$WORK/$r" fetch -q --unshallow || true
      git -C "$WORK/$r" fetch -q "$NES" main
      base=$(git -C "$WORK/$r" merge-base HEAD FETCH_HEAD)
      rm -rf "$WORK/nebulastream"
      git -C "$WORK/$r" worktree add -q --detach "$WORK/nebulastream" "$base"
      cloc --quiet --diff --exclude-dir=.git "$WORK/nebulastream" "$WORK/$r" > "$WORK/$r.diff.cloc"
      git -C "$WORK/$r" worktree remove --force "$WORK/nebulastream"
      read -r files code < <(awk '/^SUM:/{exit} /^ *(added|modified) /{f+=$2; c+=$5} END{print f, c}' "$WORK/$r.diff.cloc")
      nd=$(awk '/^SUM:/{exit} /^[A-Za-z]/{lang=$0} /^ *(added|modified) / && lang !~ /^(CSV|JSON)/{c+=$5} END{print c}' "$WORK/$r.diff.cloc")
      cpp=$(awk '/^SUM:/{exit} /^[A-Za-z]/{lang=$0} /^ *(added|modified) / && lang == "C++"{c+=$5} END{print c}' "$WORK/$r.diff.cloc")
      main="C++ ($cpp)"
      repo="$r (without NebulaStream)"
      NESBASE=$(git -C "$WORK/$r" log -1 --abbrev=7 --format='%h` %cs' "$base")
    else
      cloc --quiet --exclude-dir=.git "$WORK/$r" > "$WORK/$r.cloc"
      cloc --quiet --exclude-dir=.git --exclude-lang=CSV,JSON "$WORK/$r" > "$WORK/$r.nodata.cloc"
      read -r files code < <(awk '/^SUM:/{print $2, $5}' "$WORK/$r.cloc")
      nd=$(awk '/^SUM:/{print $5}' "$WORK/$r.nodata.cloc")
      main=$(awk "$LANGS" "$WORK/$r.nodata.cloc")
      repo=$r
    fi
    label=""; [ $first = 1 ] && label=$name; first=0
    ROWS+=$(printf '| %s | %s | `%s | %s | %s | %s | %s |' "$label" "$repo" "$c" "$files" "$code" "$nd" "$main")$'\n'
    GT[$name]=$((GT[$name]+code)); GN[$name]=$((GN[$name]+nd))
  done
done

{
echo "<!--"
echo "  MobilityDB — Generation of the bindings: lines of code"
echo "  Copyright(c) MobilityDB Contributors"
echo ""
echo "  This documentation is licensed under a Creative Commons Attribution-Share"
echo "  Alike 3.0 License: https://creativecommons.org/licenses/by-sa/3.0/"
echo "-->"
echo
echo "# Lines of code of MobilityDB, MEOS and the bindings, $DATE"
echo
echo "[Back to the index](00-INDEX.md)"
echo
echo "Counted by \`tools/count_loc.sh\` with cloc $(cloc --version) on a fresh \`git clone --depth 1\`"
echo "of the default branch of each \`github.com/MobilityDB/<repo>\` (submodules not fetched, \`.git\`"
echo "excluded); each pull request of the generation plan runs it again. *Code* is cloc's code"
echo "column. *Code w/o data* leaves out the CSV and JSON rows, which are mostly test fixtures, sample"
echo "data or generated catalogs. MobilityNebula counts the lines its default branch adds or modifies"
echo "over its NebulaStream base, \`nebulastream/nebulastream\` at \`$NESBASE)."
echo
echo "## Summary"
echo
echo "| Group | Repository | Commit | Files | Code | Code w/o data | Main language (code) |"
echo "|---|---|---|---:|---:|---:|---|"
printf '%s' "$ROWS"
echo
echo "| Group | Code | Code w/o data |"
echo "|---|---:|---:|"
T=0; N=0; B=0; BN=0
for g in "${REPO_GROUPS[@]}"; do
  name=${g%%|*}; echo "| $name | ${GT[$name]} | ${GN[$name]} |"
  T=$((T+GT[$name])); N=$((N+GN[$name]))
  if [[ "$name" =~ ^($BINDING_GROUPS)$ ]]; then B=$((B+GT[$name])); BN=$((BN+GN[$name])); fi
  if [ "$name" = MEOS-API ]; then echo "| **All bindings** | **$B** | **$BN** |"; fi
done
echo "| **Total** | **$T** | **$N** |"
echo
echo "## Notes"
printf '%s\n\n' "$NOTES"
echo "## Per-repository cloc output"
for g in "${REPO_GROUPS[@]}"; do
  for r in ${g#*|}; do
    echo
    if [ "$r" = MobilityNebula ]; then
      echo "### $r (\`cloc --diff\` over NebulaStream \`$NESBASE); counted: added + modified)"
      echo; echo '```'; sed -n '/^-----/,$p' "$WORK/$r.diff.cloc"; echo '```'
    else
      echo "### $r"
      echo; echo '```'; sed -n '/^-----/,$p' "$WORK/$r.cloc"; echo '```'
    fi
  done
done
} > "$OUT"
echo "$OUT"
