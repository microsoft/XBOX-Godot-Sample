#!/usr/bin/env bash
# Dry-run of the git sequence in .github/workflows/track-traffic.yml against a
# local bare repo standing in for origin. Exercises three consecutive "days":
#   run 1 -- no metrics branch yet (bootstrap path)
#   run 2 -- metrics branch exists, new data (shallow-fetch + push path)
#   run 3 -- metrics branch exists, identical data (no-op path)
#
# The run-2 push is the part most likely to break in production: the workflow
# fetches with --depth 1, and git has historically refused to push from a
# shallow repository. file:// is required below -- a bare local path triggers
# git's local-clone optimization and silently ignores --depth, which would make
# this test pass while production fails.
set -euo pipefail

REPO_ROOT=$(cd "$(dirname "$0")/../.." && pwd)
MERGE="$REPO_ROOT/.github/scripts/merge_traffic_csv.sh"

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

origin="$work/origin.git"
git init -q --bare "$origin"
REMOTE="file://$origin"

fail=0
note() { printf '\n=== %s ===\n' "$1"; }

# Mirrors the workflow's snapshot step.
snapshot() {
  local rundir=$1 rows=$2
  mkdir -p "$rundir"
  cd "$rundir"

  git init -q -b metrics
  git remote add origin "$REMOTE"
  if git fetch -q --depth 1 origin metrics 2>/dev/null; then
    git reset -q --hard FETCH_HEAD
    echo "  fetched existing metrics branch"
  else
    echo "  no metrics branch yet -- bootstrapping"
  fi

  printf '%s\n' "$rows" | bash "$MERGE" data/clones.csv 'date,count,uniques' 1 >/dev/null
  echo "placeholder readme" > README.md
  printf '* text eol=lf\n' > .gitattributes

  git add -A
  if git diff --cached --quiet; then
    echo "  RESULT: no changes to commit"
    return 0
  fi
  git -c user.name='github-actions[bot]' \
      -c user.email='41898282+github-actions[bot]@users.noreply.github.com' \
      commit -q -m "chore(metrics): traffic snapshot"
  if git push -q origin metrics 2>&1; then
    echo "  RESULT: pushed"
  else
    echo "  RESULT: PUSH FAILED"
    return 1
  fi
}

note "run 1 -- bootstrap onto empty origin"
snapshot "$work/run1" "$(printf '2026-08-01,9,4\n2026-08-02,12,8')" || fail=1

note "run 2 -- shallow fetch, new day, push back"
snapshot "$work/run2" "$(printf '2026-08-02,15,9\n2026-08-03,4,3')" || fail=1

note "run 3 -- identical data, expect no commit"
snapshot "$work/run3" "$(printf '2026-08-02,15,9\n2026-08-03,4,3')" || fail=1

note "final state on origin/metrics"
verify="$work/verify"
git clone -q "$REMOTE" -b metrics "$verify"
cd "$verify"
# Normalize line endings: a Windows checkout with core.autocrlf=true rewrites
# LF to CRLF on checkout, which would fail the comparison for reasons that have
# nothing to do with the merge logic. The runner (ubuntu) never does this.
actual=$(tr -d '\r' < data/clones.csv)
echo "$actual"

# 08-02 must show run 2's corrected value (15,9), not run 1's partial (12,8).
expected=$(printf 'date,count,uniques\n2026-08-01,9,4\n2026-08-02,15,9\n2026-08-03,4,3')
note "assertions"
if [ "$actual" = "$expected" ]; then
  echo "PASS  merged series is correct across runs (partial day self-corrected)"
else
  echo "FAIL  merged series mismatch"
  echo "  expected: $(printf '%s' "$expected" | tr '\n' '|')"
  fail=1
fi

commits=$(git rev-list --count metrics)
if [ "$commits" -eq 2 ]; then
  echo "PASS  exactly 2 commits (run 3 correctly made no commit)"
else
  echo "FAIL  expected 2 commits, found $commits"
  fail=1
fi

echo
if [ "$fail" -eq 0 ]; then echo "orphan-branch dry run PASSED"; else echo "orphan-branch dry run FAILED"; fi
exit "$fail"
