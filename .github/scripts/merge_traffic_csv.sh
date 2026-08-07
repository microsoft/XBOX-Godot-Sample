#!/usr/bin/env bash
#
# Merge CSV rows read from stdin into <target>, upserting on the first
# <key_fields> comma-separated columns. Used by .github/workflows/track-traffic.yml.
#
#   usage: merge_traffic_csv.sh <target.csv> <header-line> <key-field-count>
#
# Why upsert instead of append:
#
#   GitHub's traffic API reports a rolling 14-day window, so two consecutive
#   daily snapshots overlap by 13 days. Appending would duplicate those days
#   ~14 times over. Upserting on the date key keeps exactly one row per day.
#
#   Newer rows must WIN over older ones for the same key, because the current
#   UTC day is still in progress when we read it. Yesterday's snapshot of
#   "today" is a partial count; today's snapshot of that same date is the
#   settled one. Last-write-wins lets each day self-correct as it ages out of
#   the window.
#
#   This also makes the workflow idempotent: a manual re-run, a cron
#   double-fire, or a replayed snapshot rewrites rows in place rather than
#   corrupting the series.
#
# Rows are emitted sorted by their key columns (LC_ALL=C, so ISO-8601 dates
# sort chronologically), which keeps the committed diffs small and readable --
# a new day appends a line at the end instead of reshuffling the file.

set -euo pipefail

target=${1:?target csv path required}
header=${2:?header line required}
key_fields=${3:?key field count required}

case "$key_fields" in
  ''|*[!0-9]*) echo "key-field-count must be a positive integer, got '$key_fields'" >&2; exit 2 ;;
  0) echo "key-field-count must be greater than zero" >&2; exit 2 ;;
esac

body=$(mktemp)
incoming=$(mktemp)
trap 'rm -f "$body" "$incoming"' EXIT

cat > "$incoming"

mkdir -p "$(dirname "$target")"

{
  # Existing rows (header stripped) first, then the incoming rows. Order is
  # load-bearing: awk keeps the LAST row seen for a key, so incoming wins.
  if [ -f "$target" ]; then
    tail -n +2 "$target"
  fi
  cat "$incoming"
} | awk -F',' -v k="$key_fields" '
    # Drop blank lines so a trailing newline never becomes a phantom row.
    NF == 0 { next }
    {
      key = ""
      for (i = 1; i <= k; i++) key = key SUBSEP $i
      rows[key] = $0
    }
    END { for (key in rows) print rows[key] }
  ' | LC_ALL=C sort > "$body"

{ printf '%s\n' "$header"; cat "$body"; } > "$target"

echo "wrote $target ($(wc -l < "$body" | tr -d ' ') data row(s))"
