# PlayFab Tutorial 2 — Post and query a PlayFab leaderboard

## What you'll build

A scene that records a player score into a **PlayFab statistic** and
then reads back the **leaderboard** that ranks the statistic, top of
the page first. By the end you will:

- Record a score with `PlayFab.statistics.update_statistics_async`
  against a statistic whose values feed the leaderboard.
- Query the global top of the leaderboard with
  `PlayFab.leaderboards.get_leaderboard_async`.
- Page through the result with `start_position` + `page_size` and
  read the same leaderboard "around the player" with
  `get_leaderboard_around_user_async`.
- Pull a **PlayFab-friend leaderboard** with
  `get_friend_leaderboard_async(..., include_xbox_friends=false)`.

Sample output:

```
[Lead] Recorded score 1234 to statistic "high_score"
[Lead] Global page 1: rank 1..10 of ~317 entries (version 4)
[Lead]   #1   SteelGorilla — 99800
[Lead]   #2   ThunderBison — 87420
...
[Lead] Around-user: 7 row(s) centered on you
[Lead]   #142 SteelGorilla — 1234 (you)
[Lead] PlayFab-friend leaderboard: 3 row(s)
```

## Prerequisites

- [PlayFab Tutorial 1 — Sign in a user](01-signin.md) is complete and
  `PlayFabAuth.playfab_user` resolves to a signed-in `PlayFabUser`.
- The title-side leaderboards configuration is in place. PlayFab
  client-driven leaderboards are sourced from a **statistic**: the
  client writes a value to the statistic, and the leaderboard ranks
  the statistic across all entities. Configure both resources in
  PlayFab Game Manager:
  - **A statistic with a known name** (the snippets below use
    `"high_score"`; substitute an alternative and update the
    matching constant). Entity type `title_player_account`. A
    single column with `AggregationMethod = Last` matches the
    snippets below; `Max` is also a common choice for high-score
    boards.
  - **A leaderboard that sources its rankings from the statistic.**
    In Game Manager, create the leaderboard, then configure its
    source to be the statistic above. The leaderboard name and the
    statistic name may differ; the snippets below assume the same
    name (`"high_score"`) for both.
  - The full walkthrough is in
    [PlayFab title prerequisites — §2 Leaderboards](../../playfab/prerequisites.md#leaderboards-t3-t8).
- For the friend leaderboard step, this PlayFab-only track intentionally passes `include_xbox_friends=false`. Mutual PlayFab friends appear; Xbox friends are not queried because `PlayFabAuth` signs in with a custom id.

> **Why statistic-backed instead of direct writes?** PlayFab's
> `LeaderboardsV2/UpdateLeaderboardEntries` is, by default,
> server-only — a client call returns
> `E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS` (HRESULT
> `0x89235472`). The supported client-side pattern for
> player-driven leaderboards is to write a statistic and let the
> leaderboard surface those values. See
> [Troubleshooting → PlayFab leaderboard submit returns 0x89235472](../../troubleshooting.md#playfab-leaderboard-submit-fails-with-e_pf_api_not_enabled_for_game_client_access-0x89235472)
> for the diagnostic context.

> **PlayFab leaderboards vs. Microsoft GDK leaderboards.** PlayFab leaderboards
> are an explicitly **versioned** resource: a leaderboard is created
> in Game Manager with a reset cadence (or a manual reset endpoint),
> and every reset increments `version`. Statistics carry their own
> version that the leaderboard tracks. The Microsoft GDK leaderboard surface
> (`GDK.leaderboards` + `GDK.stats`) is a separate XBOX Live service
> with a different reset model; the two are not interchangeable.
> This tutorial uses PlayFab; the Microsoft GDK surface remains available in
> the addon for titles that prefer it.

## Relevant addon surfaces

- [`PlayFab.statistics`](../../../addons/godot_playfab/doc_classes/PlayFabStatistics.xml)
  — `update_statistics_async` (the client-write entry point for
  statistic-backed leaderboards), `get_statistics_async`,
  `get_statistics_for_entities_async`.
- [`PlayFab.leaderboards`](../../../addons/godot_playfab/doc_classes/PlayFabLeaderboards.xml)
  — `get_leaderboard_async`,
  `get_leaderboard_around_user_async`,
  `get_friend_leaderboard_async`.
- [`PlayFab.users`](../../../addons/godot_playfab/doc_classes/PlayFabUsers.xml) — provides the
  `PlayFabUser` every statistics and leaderboards call takes as its
  first parameter (typically `PlayFabAuth.playfab_user` from PlayFab 1).
- [`PlayFabResult`](../../../addons/godot_playfab/doc_classes/PlayFabResult.xml) — the normalized result
  type. `result.ok`, `result.message`, `result.data` (a `Dictionary`
  for leaderboard responses; see the response shape in Step 2).
- One-page primer on the addons' async model:
  [Async patterns](../../async-patterns.md).

## Step 1 — Record a score to the statistic

`PlayFab.statistics.update_statistics_async` takes the signed-in
user and a request dictionary that lists the statistics to update.
Each entry carries the statistic `name` and a `scores` array of
decimal-encoded strings, one per statistic column. A single-column
statistic takes a single-element `scores` array:

```gdscript
extends Node

const STATISTIC_NAME := "high_score"

func _ready() -> void:
    if not await PlayFabAuth.sign_in():
        return

    await _record_score(1234)

func _record_score(score: int) -> void:
    var user: PlayFabUser = PlayFabAuth.playfab_user

    var result: PlayFabResult = await PlayFab.statistics.update_statistics_async(user, {
        "statistics": [
            {"name": STATISTIC_NAME, "scores": [str(score)]},
        ],
    })
    if not result.ok:
        push_warning("[Lead] Record failed: %s" % result.message)
        return

    print("[Lead] Recorded score %d to statistic \"%s\"" % [score, STATISTIC_NAME])
```

Two patterns worth knowing:

- **Record when the run ends, not every frame.** Each statistic
  write is a service round-trip. Recording a score per frame both
  rate-limits you and wastes bandwidth.
- **`update_statistics_async` accepts multi-column scores, metadata,
  and an optional version.** Each entry in `statistics` may carry a
  multi-element `scores` array (one decimal string per statistic
  column), a `metadata` string (returned by every leaderboard
  query for the entry), and a `version` int to pin the write to a
  specific statistic version. A single-column statistic takes a
  single-element `scores` array and otherwise leaves the optional
  fields out.

## Step 2 — Query the global top of the leaderboard

The leaderboard surfaces the statistic values as ranked entries.
`get_leaderboard_async` returns a `Dictionary` describing the page —
the rows already copied out plus the leaderboard's `version` and a
total entry count. The leaderboard name passed here is the
**leaderboard** name configured in Game Manager (which may differ
from the statistic name; the snippets here use `"high_score"` for
both):

```gdscript
const LEADERBOARD_NAME := "high_score"

func _print_global_top() -> void:
    var user: PlayFabUser = PlayFabAuth.playfab_user

    var result: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
            user, LEADERBOARD_NAME, 1, 10)
    if not result.ok:
        push_warning("[Lead] get_leaderboard failed: %s" % result.message)
        return

    var page: Dictionary = result.data
    var rankings: Array = page.get("rankings", [])
    print("[Lead] Global page 1: rank 1..%d of ~%d entries (version %d)" % [
            rankings.size(),
            page.get("entry_count", 0),
            page.get("version", -1)])
    for entry in rankings:
        var row: Dictionary = entry
        print("[Lead]   #%d  %s — %d" % [
                row.get("rank", 0),
                _display_name(row),
                _primary_score(row)])
```

Two helpers your snippets will reuse — keep them on the same script
or move them to a shared util:

```gdscript
func _display_name(row: Dictionary) -> String:
    var name: String = row.get("display_name", "")
    if not name.is_empty():
        return name
    # PlayFab returns an empty display_name when the account has not set
    # one. Fall back to the entity id so the row still renders.
    var entity: Dictionary = row.get("entity", {})
    return entity.get("id", "?")

func _primary_score(row: Dictionary) -> int:
    # rankings[].scores is a PackedStringArray of decimal-encoded column
    # values, ordered by the leaderboard's column definitions.
    var scores: PackedStringArray = row.get("scores", PackedStringArray())
    return scores[0].to_int() if not scores.is_empty() else 0
```

`page_size` is clamped to `1..100` by the addon. Asking for `1000`
quietly returns at most 100. `start_position` is **1-based** and
clamps to at least 1; pass `1` for the first row.

## Step 3 — Page through the rest

PlayFab leaderboards page with explicit `start_position` and
`page_size`. There's no opaque continuation handle to track — you
just walk the indexes yourself, optionally pinning the `version`
captured from the first page so a mid-walk reset doesn't shift the
rows under you:

```gdscript
func _print_all_pages() -> void:
    var user: PlayFabUser = PlayFabAuth.playfab_user
    const PAGE_SIZE := 10

    var first: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
            user, LEADERBOARD_NAME, 1, PAGE_SIZE)
    if not first.ok:
        push_warning("[Lead] first page failed: %s" % first.message)
        return

    var page: Dictionary = first.data
    var total: int = page.get("entry_count", 0)
    var version: int = page.get("version", -1)
    var position := 1
    var page_index := 1

    while page != null:
        var rankings: Array = page.get("rankings", [])
        print("[Lead] Page %d: %d row(s)" % [page_index, rankings.size()])
        for entry in rankings:
            var row: Dictionary = entry
            print("[Lead]   #%d  %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])

        position += rankings.size()
        if rankings.is_empty() or position > total:
            break

        var next: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
                user, LEADERBOARD_NAME, position, PAGE_SIZE, version)
        if not next.ok:
            push_warning("[Lead] page %d failed: %s" % [page_index + 1, next.message])
            return
        page = next.data
        page_index += 1
```

Passing `version` on every subsequent call locks paging to a
consistent snapshot. Omit (or pass `-1`) when you don't care about
reset boundaries.

## Step 4 — Query "around the signed-in user"

For HUD ribbons and end-of-match summaries the more useful query
is the page centered on the local player:

```gdscript
func _print_around_user() -> void:
    var user: PlayFabUser = PlayFabAuth.playfab_user

    var result: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_around_user_async(
            user, LEADERBOARD_NAME, 3)
    if not result.ok:
        push_warning("[Lead] around_user failed: %s" % result.message)
        return

    var page: Dictionary = result.data
    var rankings: Array = page.get("rankings", [])
    print("[Lead] Around-user: %d row(s) centered on you" % rankings.size())

    var my_id: String = user.entity_key.get("id", "")
    for entry in rankings:
        var row: Dictionary = entry
        var entity: Dictionary = row.get("entity", {})
        var marker := " (you)" if entity.get("id", "") == my_id else ""
        print("[Lead]   #%d  %s — %d%s" % [
                row.get("rank", 0),
                _display_name(row),
                _primary_score(row),
                marker])
```

The `max_surrounding_entries` argument is the count **above and
below** the player, so the page size you see is roughly `2N + 1`
when there are enough rows on either side. Pass `3` for a 7-row
ribbon, `10` for a 21-row panel, etc.

## Step 5 — Pull the PlayFab-friend leaderboard

For social cards and "challenge a friend" UI, switch to
`get_friend_leaderboard_async` with `include_xbox_friends=false`:

```gdscript
func _print_xbox_friend_leaderboard() -> void:
    var user: PlayFabUser = PlayFabAuth.playfab_user

    var result: PlayFabResult = await PlayFab.leaderboards.get_friend_leaderboard_async(
            user, LEADERBOARD_NAME, false)
    if not result.ok:
        push_warning("[Lead] friend leaderboard failed: %s" % result.message)
        return

    var page: Dictionary = result.data
    var rankings: Array = page.get("rankings", [])
    print("[Lead] PlayFab-friend leaderboard: %d row(s)" % rankings.size())
    for entry in rankings:
        var row: Dictionary = entry
        print("[Lead]   #%d  %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])
```

`include_xbox_friends=false` keeps this PlayFab-only track decoupled from Xbox. The result is scoped to mutual PlayFab friends; an Xbox-linked title can pass `true` in the integrated track when it also has an Xbox-backed PlayFab session.

## Verify

A clean run prints something like:

```
[Lead] Recorded score 1234 to statistic "high_score"
[Lead] Global page 1: rank 1..10 of ~317 entries (version 4)
[Lead]   #1   SteelGorilla — 99800
[Lead]   #2   ThunderBison — 87420
...
[Lead] Around-user: 7 row(s) centered on you
[Lead]   #141 SilverPanda — 1290
[Lead]   #142 SteelGorilla — 1234 (you)
[Lead]   #143 GoldenWolf — 1180
[Lead] PlayFab-friend leaderboard: 3 row(s)
```

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| `Record failed: statistic_not_found` (or `errorCode 1310`, `StatisticDefinitionNotFound`) | The statistic name is not configured in Game Manager, or the entity type does not match. | Open **PlayFab Game Manager → Statistics** and create the statistic with entity type `title_player_account`. |
| `Record failed: invalid_request` or column-mismatch error | The `scores` array length does not match the statistic's configured column count, or a value is not a decimal-formatted string. | Match the column count and use `str(value)` for each integer score. A single-column statistic takes a single-element `scores` array. |
| `Record failed: ...APINotEnabledForGameClientAccess...` (HRESULT `0x89235472`) on `update_statistics_async` | The title's **Allow client to post player stats** setting is disabled. | Open **PlayFab Game Manager → your title → Title settings → API Features**, enable **Allow client to post player stats**, save, and re-run. See [PlayFab prerequisites — §2 Leaderboards step 3](../../playfab/prerequisites.md#leaderboards-t3-t8). |
| `Record failed: ...APINotEnabledForGameClientAccess...` (HRESULT `0x89235472`) on `submit_score_async` | The code is calling `PlayFab.leaderboards.submit_score_async` instead of `PlayFab.statistics.update_statistics_async`. The direct-leaderboard-write endpoint is server-only and has no Game Manager toggle. | Switch the write path to `update_statistics_async`; see [Troubleshooting → PlayFab leaderboard submit returns 0x89235472](../../troubleshooting.md#playfab-leaderboard-submit-fails-with-e_pf_api_not_enabled_for_game_client_access-0x89235472). |
| `get_leaderboard failed: not_found` | The leaderboard name does not match a leaderboard configured in Game Manager, or no entity has a statistic value yet. | Confirm the leaderboard exists and is sourced from the statistic. Then record at least one score (your own counts). |
| Leaderboard renders `(no entries)` even after a successful record | The leaderboard is sourced from a different statistic than the one being written, or the rankings have not refreshed yet. | Confirm the Game Manager leaderboard's source statistic matches `STATISTIC_NAME`. Statistic-to-leaderboard propagation is typically a few seconds; wait and re-query. |
| Around-user returns only one row | You are the only entity with a value in this statistic version. | Record values from a second test account, or use the global query for the demo. |
| `friend leaderboard is empty` | There are no mutual PlayFab friends with submitted scores. | Add mutual PlayFab friends or treat the empty page as valid for this track. |
| PlayFab-friend page is empty for a friended account | The friend has never recorded the statistic or is not a mutual PlayFab friend in this title. | Have the friend record a score and confirm both clients use the same PlayFab title id. |

## Reference implementation

The PlayFab-track reference implementation lives in
[`sample/tutorial_playfab/`](../../../sample/tutorial_playfab/README.md):

- Scene: [`sample/tutorial_playfab/p02_leaderboard.tscn`](../../../sample/tutorial_playfab/p02_leaderboard.tscn)
- Script: [`sample/tutorial_playfab/p02_leaderboard.gd`](../../../sample/tutorial_playfab/p02_leaderboard.gd)
- Reuses the `PlayFabAuth` autoload from PlayFab 1.

## What's next

Continue to [PlayFab Tutorial 3 — Lobby](03-lobby.md).



