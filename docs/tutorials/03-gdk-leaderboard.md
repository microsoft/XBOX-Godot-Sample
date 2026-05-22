# Tutorial 3 — Post and query a leaderboard

## What you'll build

A scene that submits a player score into a title-managed Xbox
statistic and then reads it back as a leaderboard, top of the page
first. By the end you will:

- Stage a numeric statistic update with
  `GDK.stats.set_stat_integer` and flush it with
  `flush_stats_async`.
- Query the global leaderboard for that stat through
  `GDK.leaderboards.get_leaderboard_async`.
- Page through the result with `get_next_page_async` and read
  the same leaderboard "around the player" with
  `get_leaderboard_around_user_async`.

Sample output:

```
[Lead] Submitted score 1234 for stat "high_score"
[Lead] Global page 1: 25 row(s) of ~317 total
[Lead]   #1   SteelGorilla — high_score=99800
[Lead]   #2   ThunderBison — high_score=87420
...
[Lead] Around-user page: 7 row(s)
[Lead]   #142 SteelGorilla — high_score=1234 (you)
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete.
- One **title-managed statistic** is declared in Partner Center for
  this title. The statistic name in the snippets below is
  `"high_score"`; substitute your own. Statistics are not
  automatically created — calls against an undeclared stat return a
  service error.
- The PC is in the same Xbox sandbox the stat was declared in.
- For around-user queries, at least one other test account in your
  sandbox has also submitted a score (otherwise the around-user
  query returns just your one row).

> **Stat declaration is a Partner Center step.** The
> [Sample project setup](../gdk/sample-setup.md) walks the UI
> path. The stat must be marked as **title-managed** (also called
> "client-writable" in some Partner Center revisions) for
> `set_stat_integer` and `flush_stats_async` to succeed from the
> client. Server-managed stats are read-only from GDScript.

## Step 1 — Submit a score

Title-managed stat updates are **staged then flushed**. Staging is
synchronous (no network), flushing is the round-trip:

```gdscript
extends Node

const STAT_NAME := "high_score"

func _ready() -> void:
    if Auth.xbox_user == null:
        await Auth.sign_in_completed

    await _submit_score(1234)

func _submit_score(score: int) -> void:
    var user: GDKUser = Auth.xbox_user

    var staged: GDKResult = GDK.stats.set_stat_integer(user, STAT_NAME, score)
    if not staged.ok:
        push_warning("[Lead] Staging failed: %s" % staged.message)
        return

    var flushed: GDKResult = await GDK.stats.flush_stats_async(user)
    if not flushed.ok:
        push_warning("[Lead] Flush failed: %s" % flushed.message)
        return

    print("[Lead] Submitted score %d for stat \"%s\"" % [score, STAT_NAME])
```

Two patterns worth knowing:

- **Batch your updates.** You can call `set_stat_integer` for several
  stats in a row and then call `flush_stats_async` **once** to ship
  them as a single submission. Submitting a score per frame would
  hammer the service for no good reason.
- **Use `set_stat_number` for float-valued stats.** The two staging
  helpers do the same thing for different numeric types; the choice
  is just whether the declared stat is integer or floating-point.

## Step 2 — Query the global top of the leaderboard

`get_leaderboard_async` returns a `GDKLeaderboard` snapshot — pages
of rows already copied out of the native handle, plus a column-type
descriptor and an opaque continuation handle for paging:

```gdscript
func _print_global_top() -> void:
    var user: GDKUser = Auth.xbox_user

    var result: GDKResult = await GDK.leaderboards.get_leaderboard_async(user, STAT_NAME, 25)
    if not result.ok:
        push_warning("[Lead] get_leaderboard failed: %s" % result.message)
        return

    var board: GDKLeaderboard = result.data
    print("[Lead] Global page 1: %d row(s) of ~%d total" % [board.rows.size(), board.total_row_count])
    for entry in board.rows:
        var row: GDKLeaderboardRow = entry
        var value: String = row.column_values[0] if row.column_values.size() > 0 else "?"
        var name: String = row.gamertag if not row.gamertag.is_empty() else row.modern_gamertag
        print("[Lead]   #%d  %s — %s=%s" % [row.rank, name, STAT_NAME, value])
```

The third argument (`max_items`) controls page size, default 25.
The service caps it implicitly at around 100 — asking for `1000`
will quietly return at most ~100.

Column values come back JSON-encoded in `row.column_values`, ordered
by the columns reported in `board.columns`. For a single-stat
leaderboard like this one there is exactly one column whose
`stat_type` is `"uint64"`; the value string parses straight into an
integer with `int(value)`.

## Step 3 — Page through the rest

A leaderboard with more rows than your `max_items` exposes a
follow-up handle through `has_next` and `get_next_page_async`:

```gdscript
func _print_all_pages() -> void:
    var user: GDKUser = Auth.xbox_user

    var result: GDKResult = await GDK.leaderboards.get_leaderboard_async(user, STAT_NAME, 25)
    if not result.ok:
        push_warning("[Lead] first page failed: %s" % result.message)
        return

    var board: GDKLeaderboard = result.data
    var page_index := 1
    while board != null:
        print("[Lead] Page %d: %d row(s)" % [page_index, board.rows.size()])
        for entry in board.rows:
            var row: GDKLeaderboardRow = entry
            print("[Lead]   #%d  %s" % [row.rank, row.gamertag])

        if not board.has_next:
            break

        var next_result: GDKResult = await GDK.leaderboards.get_next_page_async(board)
        if not next_result.ok:
            push_warning("[Lead] next page failed: %s" % next_result.message)
            return
        board = next_result.data
        page_index += 1
```

`get_next_page_async` takes the previous `GDKLeaderboard` directly
because the native continuation state lives inside it; you never need
to track an opaque cursor yourself.

## Step 4 — Query "around the signed-in user"

For HUD ribbons and end-of-match summaries the more useful query
is the page centered on the local player:

```gdscript
func _print_around_user() -> void:
    var user: GDKUser = Auth.xbox_user

    var result: GDKResult = await GDK.leaderboards.get_leaderboard_around_user_async(user, STAT_NAME, 7)
    if not result.ok:
        push_warning("[Lead] around_user failed: %s" % result.message)
        return

    var board: GDKLeaderboard = result.data
    print("[Lead] Around-user page: %d row(s)" % board.rows.size())
    for entry in board.rows:
        var row: GDKLeaderboardRow = entry
        var marker := " (you)" if row.xuid == user.xuid else ""
        print("[Lead]   #%d  %s%s" % [row.rank, row.gamertag, marker])
```

Pass an odd `max_items` so the signed-in user appears in the middle
of the page. The same paging mechanic works on this result, but the
common UI pattern is one centered page, no further pages.

## Step 5 — Listen for cache refreshes

Both `get_leaderboard_async` and `get_next_page_async` update an
in-memory cache and fire `leaderboard_updated`. If multiple UI
panels need to react to a refresh, connect once at startup instead
of awaiting in each panel:

```gdscript
func _ready() -> void:
    if Auth.xbox_user == null:
        await Auth.sign_in_completed

    GDK.leaderboards.leaderboard_updated.connect(_on_leaderboard_updated)

func _on_leaderboard_updated(stat_name: String, board: GDKLeaderboard) -> void:
    if stat_name != STAT_NAME:
        return
    # Refresh the panel using board.rows / board.columns.
    print("[Lead] Cache refreshed: %d row(s)" % board.rows.size())
```

`GDK.leaderboards.get_cached_leaderboard(STAT_NAME)` returns the most
recent snapshot without a network call, useful for redrawing a panel
on focus.

## Verify

A clean run (one score submission followed by global query and
around-user query) prints something like:

```
[Lead] Submitted score 1234 for stat "high_score"
[Lead] Global page 1: 25 row(s) of ~317 total
[Lead]   #1   SteelGorilla — high_score=99800
[Lead]   #2   ThunderBison — high_score=87420
...
[Lead] Around-user page: 7 row(s)
[Lead]   #139 IronOwl — high_score=1421
[Lead]   #140 BraveTiger — high_score=1380
[Lead]   #141 SilverPanda — high_score=1290
[Lead]   #142 SteelGorilla — high_score=1234 (you)
[Lead]   #143 GoldenWolf — high_score=1180
...
```

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| `Staging failed: invalid_argument` | The stat name is not declared, or the wrong staging helper for the declared type. | Confirm Partner Center stat name and type. Use `set_stat_number` for floating-point stats. |
| `Flush failed: unauthorized` | Test account does not have the right entitlement, or the PC is in the wrong sandbox. | Switch the sandbox; reassign the test account. |
| `get_leaderboard failed: not_found` | The stat is declared but no one has submitted a score yet. | Submit at least one score (your own counts). |
| Around-user query returns only one row | You are the only test account with a score. | Submit scores from a second test account or use the global query for the demo. |
| `column_values[0] == ""` | Your stat is declared as a string or boolean. | Use a numeric stat type for leaderboards. |

## What's next

You can now write and read Xbox leaderboards. Tutorial 4 moves to
PlayFab Game Saves, which builds on the PlayFab session from
tutorial 1:

- [**Tutorial 4 — Save the player's progress**](04-game-saves.md)
- Reference: [GDKStats](../gdk/api-reference.md),
  [GDKLeaderboards](../gdk/api-reference.md)
