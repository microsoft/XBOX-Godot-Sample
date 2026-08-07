#!/usr/bin/env bash
# Local sanity checks for merge_traffic_csv.sh. Not wired into CI; run by hand:
#   bash .github/scripts/test_merge_traffic_csv.sh
set -euo pipefail

here=$(cd "$(dirname "$0")" && pwd)
merge="$here/merge_traffic_csv.sh"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

hdr='date,count,uniques'
csv="$work/clones.csv"
fail=0

check() {
  local name=$1 expected=$2 actual=$3
  if [ "$expected" = "$actual" ]; then
    echo "PASS  $name"
  else
    echo "FAIL  $name"
    echo "  expected: $(printf '%s' "$expected" | tr '\n' '|')"
    echo "  actual:   $(printf '%s' "$actual" | tr '\n' '|')"
    fail=1
  fi
}

# 1. Fresh file: header is written, rows are sorted chronologically.
printf '2026-08-03,5,3\n2026-08-01,9,4\n2026-08-02,7,7\n' | bash "$merge" "$csv" "$hdr" 1 >/dev/null
check "fresh file sorts by date and writes header" \
  "$(printf 'date,count,uniques\n2026-08-01,9,4\n2026-08-02,7,7\n2026-08-03,5,3')" \
  "$(cat "$csv")"

# 2. Overlapping snapshot: 08-03 was partial (5) and settles to 11; 08-04 is new.
#    The 13 days of overlap the API always resends must not duplicate.
printf '2026-08-03,11,6\n2026-08-04,2,2\n' | bash "$merge" "$csv" "$hdr" 1 >/dev/null
check "overlapping day upserts to the newer value" \
  "$(printf 'date,count,uniques\n2026-08-01,9,4\n2026-08-02,7,7\n2026-08-03,11,6\n2026-08-04,2,2')" \
  "$(cat "$csv")"

# 3. Idempotency: replaying the identical snapshot is a no-op.
before=$(cat "$csv")
printf '2026-08-03,11,6\n2026-08-04,2,2\n' | bash "$merge" "$csv" "$hdr" 1 >/dev/null
check "replaying the same snapshot changes nothing" "$before" "$(cat "$csv")"

# 4. Empty input (quiet repo / no traffic rows) must preserve history, not clobber it.
printf '' | bash "$merge" "$csv" "$hdr" 1 >/dev/null
check "empty stdin preserves existing rows" "$before" "$(cat "$csv")"

# 5. Composite key (snapshot_date + referrer): same date, different referrers coexist;
#    the same (date, referrer) pair upserts.
rhdr='snapshot_date,referrer,count,uniques'
rcsv="$work/referrers.csv"
printf '2026-08-04,github.com,10,5\n2026-08-04,Google,4,3\n' | bash "$merge" "$rcsv" "$rhdr" 2 >/dev/null
printf '2026-08-04,github.com,12,6\n2026-08-05,Google,1,1\n' | bash "$merge" "$rcsv" "$rhdr" 2 >/dev/null
check "composite key upserts per (date,referrer)" \
  "$(printf 'snapshot_date,referrer,count,uniques\n2026-08-04,Google,4,3\n2026-08-04,github.com,12,6\n2026-08-05,Google,1,1')" \
  "$(cat "$rcsv")"

# 6. Target in a not-yet-created directory (first ever run on the orphan branch).
deep="$work/nested/sub/views.csv"
printf '2026-08-01,3,2\n' | bash "$merge" "$deep" "$hdr" 1 >/dev/null
check "creates missing parent directories" \
  "$(printf 'date,count,uniques\n2026-08-01,3,2')" \
  "$(cat "$deep")"

echo
if [ "$fail" -eq 0 ]; then echo "all merge checks passed"; else echo "merge checks FAILED"; fi
exit "$fail"
