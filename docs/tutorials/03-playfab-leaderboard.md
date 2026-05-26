# Tutorial 3 — Post and query a PlayFab leaderboard

## What you'll build

A scene that submits a player score into a **PlayFab leaderboard** and
then reads it back, top of the page first. By the end you will:

- Submit a score with `PlayFab.leaderboards.submit_score_async`.
- Query the global top with
  `PlayFab.leaderboards.get_leaderboard_async`.
- Page through the result with `start_position` + `page_size` and
  read the same leaderboard "around the player" with
  `get_leaderboard_around_user_async`.
- Pull an **Xbox-friend leaderboard** with
  `get_friend_leaderboard_async(..., include_xbox_friends=true)`.

Sample output:

```
[Lead] Submitted score 1234 for leaderboard "high_score"
[Lead] Global page 1: rank 1..10 of ~317 entries (version 4)
[Lead]   #1   SteelGorilla — 99800
[Lead]   #2   ThunderBison — 87420
...
[Lead] Around-user: 7 row(s) centered on you
[Lead]   #142 SteelGorilla — 1234 (you)
[Lead] Xbox-friend leaderboard: 3 row(s)
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete and
  `Auth.playfab_user` resolves to a signed-in `PlayFabUser`.
- One **leaderboard** is configured in the PlayFab Game Manager for
  this title. The snippets below use the name `"high_score"`;
  substitute your own. Leaderboards are not auto-created — submissions
  against an unknown leaderboard return a service error.
- The **column types** on your leaderboard match the numeric type
  you're submitting. The snippets below assume a single integer
  column. The addon serializes scores as decimal strings, so a
  leaderboard configured for a different stat type rejects the
  submission.
- For the friend leaderboard step: `Auth.playfab_user` was obtained
  via `PlayFab.users.sign_in_with_xuser_async(Auth.xbox_user)`, not
  `sign_in_with_custom_id_async`. The friend-leaderboard call needs
  the local Xbox token, which is only available when the PlayFab
  session is backed by an Xbox user. The signed-in test account must
  also have at least one Xbox friend who has submitted a score.

> **PlayFab leaderboards vs. GDK leaderboards.** PlayFab leaderboards
> are an explicitly **versioned** resource: a leaderboard is created
> in Game Manager with a reset cadence (or a manual reset endpoint),
> and every reset increments `version`. Submissions and queries
> default to the current version. The GDK leaderboard surface
> (`GDK.leaderboards` + `GDK.stats`) is a separate Xbox Live
> service with a different reset model; the two are not
> interchangeable. This tutorial uses PlayFab; the GDK surface
> remains available in the addon for titles that prefer it.

## Relevant addon surfaces

- [`PlayFab.leaderboards`](../../addons/godot_playfab/doc_classes/PlayFabLeaderboards.xml)
  — `submit_score_async`, `get_leaderboard_async`,
  `get_leaderboard_around_user_async`,
  `get_friend_leaderboard_async`.
- [`PlayFab.users`](../playfab/plugin.md) — provides the
  `PlayFabUser` every leaderboard call takes as its first parameter
  (typically `Auth.playfab_user` from T1).
- [`PlayFabResult`](../playfab/plugin.md) — the normalized result
  type. `result.ok`, `result.message`, `result.data` (a `Dictionary`
  for leaderboard responses; see the response shape in Step 2).
- One-page primer on the addons' async model:
  [Async patterns](../async-patterns.md).

## Step 1 — Submit a score

PlayFab leaderboard submissions are **single-shot** — no staging,
no flush; one call submits the row:

```gdscript
extends Node

const LEADERBOARD_NAME := "high_score"

func _ready() -> void:
    if Auth.playfab_user == null:
        await Auth.sign_in_completed

    await _submit_score(1234)

func _submit_score(score: int) -> void:
    var user: PlayFabUser = Auth.playfab_user

    var result: PlayFabResult = await PlayFab.leaderboards.submit_score_async(
            user, LEADERBOARD_NAME, score)
    if not result.ok:
        push_warning("[Lead] Submit failed: %s" % result.message)
        return

    print("[Lead] Submitted score %d for leaderboard \"%s\"" % [score, LEADERBOARD_NAME])
```

Two patterns worth knowing:

- **Submit when the run ends, not every frame.** Each submission is
  a service round-trip. Posting a score per frame both rate-limits
  you and wastes bandwidth.
- **`submit_score_async` accepts additional scores and metadata.**
  Pass `additional_scores: Array` (each int) when the leaderboard
  has more than one column, and `metadata: String` for a small
  per-entry payload (e.g., JSON-encoded run summary). For a
  single-column integer leaderboard the two extras stay defaulted.

## Step 2 — Query the global top of the leaderboard

`get_leaderboard_async` returns a `Dictionary` describing the page —
the rows already copied out plus the leaderboard's `version` and a
total entry count:

```gdscript
func _print_global_top() -> void:
    var user: PlayFabUser = Auth.playfab_user

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
    var user: PlayFabUser = Auth.playfab_user
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
    var user: PlayFabUser = Auth.playfab_user

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

## Step 5 — Pull the Xbox-friend leaderboard

For social cards and "challenge a friend" UI, switch to
`get_friend_leaderboard_async` with `include_xbox_friends=true`:

```gdscript
func _print_xbox_friend_leaderboard() -> void:
    var user: PlayFabUser = Auth.playfab_user

    var result: PlayFabResult = await PlayFab.leaderboards.get_friend_leaderboard_async(
            user, LEADERBOARD_NAME, true)
    if not result.ok:
        push_warning("[Lead] friend leaderboard failed: %s" % result.message)
        return

    var page: Dictionary = result.data
    var rankings: Array = page.get("rankings", [])
    print("[Lead] Xbox-friend leaderboard: %d row(s)" % rankings.size())
    for entry in rankings:
        var row: Dictionary = entry
        print("[Lead]   #%d  %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])
```

`include_xbox_friends=true` is what makes this an *Xbox-friend*
leaderboard rather than a PlayFab-friend leaderboard. The addon
acquires an Xbox token for the local PlayFab API call, which only
works when the `PlayFabUser` was created with
`sign_in_with_xuser_async`. A custom-ID PlayFab session is allowed
to call this method but the **Xbox-friend half** of the query
returns an empty list — only mutual PlayFab friends appear.

For PlayFab-only friend graphs pass `false` and skip the Xbox
prereq; mutual PlayFab friends still appear either way.

## Verify

A clean run prints something like:

```
[Lead] Submitted score 1234 for leaderboard "high_score"
[Lead] Global page 1: rank 1..10 of ~317 entries (version 4)
[Lead]   #1   SteelGorilla — 99800
[Lead]   #2   ThunderBison — 87420
...
[Lead] Around-user: 7 row(s) centered on you
[Lead]   #141 SilverPanda — 1290
[Lead]   #142 SteelGorilla — 1234 (you)
[Lead]   #143 GoldenWolf — 1180
[Lead] Xbox-friend leaderboard: 3 row(s)
```

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| `Submit failed: leaderboard_not_found` | The leaderboard name is not configured in Game Manager. | Open **PlayFab Game Manager → Leaderboards** and create the leaderboard. |
| `Submit failed: invalid_score` or column-mismatch error | The submission shape doesn't match the configured column types. | Match the column count and types in `additional_scores`. A single-column integer leaderboard takes one int score, no extras. |
| `get_leaderboard failed: not_found` | The leaderboard exists but no one has submitted yet. | Submit at least one score first (your own counts). |
| Around-user returns only one row | You are the only player with a score in this leaderboard version. | Submit scores from a second test account, or use the global query for the demo. |
| `friend leaderboard failed: missing_xbox_token` | `Auth.playfab_user` was created with `sign_in_with_custom_id_async`, so no Xbox token is available. | Switch the session to `PlayFab.users.sign_in_with_xuser_async(Auth.xbox_user)`. |
| Xbox-friend page is empty for a friended account | The friend has never submitted to this leaderboard, or you and the friend are in different PlayFab titles in the same Xbox sandbox. | Have the friend submit a score; confirm the same PlayFab title id on both accounts. |

## What's next

You can now write and read PlayFab leaderboards. Tutorial 4 moves to
**PlayFab Game Saves**, which builds on the same PlayFab session:

- [**Tutorial 4 — Save the player's progress**](04-game-saves.md)
- Reference:
  [PlayFabLeaderboards](../../addons/godot_playfab/doc_classes/PlayFabLeaderboards.xml)
