# Tutorial 2 — Unlock an achievement

## What you'll build

A scene that watches the player rack up score, then unlocks an Xbox
achievement when they cross a threshold. By the end you will:

- Have one achievement declared in Partner Center for your title.
- Drive incremental progress on it (`0` → `50` → `100`) from
  GDScript using `GDK.achievements.update_achievement_async`.
- React to the unlock by listening to the `achievement_unlocked`
  signal so your HUD / toast popup fires at the right moment.
- Verify the unlock both in the Output panel and on the Xbox
  achievement viewer for the signed-in test account.

When it works, the editor Output ends with:

```
[Ach] Updated to 50% — result ok
[Ach] Updated to 100% — result ok
[Ach] Unlocked: First Score
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete and
  reaches `[Auth] Sign-in complete.`. The snippets below read
  `Auth.xbox_user`.
- Your title is set up in
  [Partner Center](https://partner.microsoft.com/dashboard) with at
  least one declared achievement. See
  [Microsoft GDK — Achievements](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/player-data/achievements/live-achievements-nav)
  for the Partner Center authoring flow. You need:
  - **Achievement ID** — a short integer string Partner Center hands
    you (e.g. `"1"` for the first achievement, `"2"` for the second).
    The snippets below use `FIRST_SCORE_ID = "1"`.
  - **Title ID** + **SCID** wired into `MicrosoftGame.config`
    (handled in the quickstart).
- The PC is in the same Xbox sandbox the achievement was
  declared in. New achievement metadata only propagates to the
  sandbox you authored it in until you promote it to retail.

> **Achievement declaration is a Partner Center step, not a code
> step.** If you have never set one up, see
> [Microsoft GDK — Achievements](https://learn.microsoft.com/en-us/gaming/gdk/docs/services/player-data/achievements/live-achievements-nav)
> for the Partner Center walkthrough and
> [Sample project setup — achievements](../gdk/sample-setup.md)
> for the addon-side counterpart. Code-side `update_achievement_async`
> calls against an undeclared id return a service error and do not
> create the achievement on the fly.

## Relevant addon surfaces

- [`GDK.achievements`](../../addons/godot_gdk/doc_classes/GDKAchievements.xml)
  — `query_player_achievements_async`,
  `update_achievement_async`, `get_cached_achievements`,
  signal `achievement_unlocked(user, achievement_id)`,
  `runtime_error` for service-level failures.
- [`GDKAchievement`](../../addons/godot_gdk/doc_classes/GDKAchievement.xml)
  — the cached snapshot wrapper. Read `id`, `name`,
  `progress_percent`, `is_secret`.
- One-page primer on the addons' async model:
  [Async patterns](../async-patterns.md).

## Step 1 — Confirm the achievement is reachable

Before you write any progress code, prove the round-trip works by
querying the cached achievement list for the signed-in user. Add this
to a fresh `res://achievements_demo.gd`:

```gdscript
extends Node

const FIRST_SCORE_ID := "1"

func _ready() -> void:
    if not await Auth.sign_in():
        return

    await _print_cached_achievements()

func _print_cached_achievements() -> void:
    var user: GDKUser = Auth.xbox_user

    var result: GDKResult = await GDK.achievements.query_player_achievements_async(user)
    if not result.ok:
        push_warning("[Ach] query failed: %s" % result.message)
        return

    var cache: Array = GDK.achievements.get_cached_achievements(user)
    print("[Ach] %d achievement(s) declared for this title" % cache.size())
    for entry in cache:
        var ach: GDKAchievement = entry
        print("[Ach]   %s (%s) — %d%%" % [ach.id, ach.name, ach.progress_percent])
```

Run the scene with this script attached. The Output should list every
achievement declared for the title, including `FIRST_SCORE_ID`. If
the list is empty you are either in the wrong sandbox, the
declaration has not propagated yet, or the test account does not have
access to the title — fix that before moving on. The PlayFab-side
sign-in is incidental to this query; only `GDK.users` and the Xbox
sandbox matter here.

## Step 2 — Stage incremental progress

Replace `_print_cached_achievements` with a routine that pushes
progress in two steps. Splitting the call lets you see that the
`achievement_unlocked` signal only fires on the final 100% update,
not on the 50% intermediate step:

```gdscript
func _ready() -> void:
    if not await Auth.sign_in():
        return

    GDK.achievements.achievement_unlocked.connect(_on_achievement_unlocked)

    await _push_progress(50)
    await _push_progress(100)

func _push_progress(percent: int) -> void:
    var user: GDKUser = Auth.xbox_user
    var result: GDKResult = await GDK.achievements.update_achievement_async(
        user, FIRST_SCORE_ID, percent)
    if result.ok:
        print("[Ach] Updated to %d%% — result ok" % percent)
    else:
        push_warning("[Ach] Update to %d%% failed: %s (%s)" % [percent, result.message, result.code])

func _on_achievement_unlocked(user: GDKUser, achievement_id: String) -> void:
    var cache: Array = GDK.achievements.get_cached_achievements(user)
    for entry in cache:
        var ach: GDKAchievement = entry
        if ach.id == achievement_id:
            print("[Ach] Unlocked: %s" % ach.name)
            return
    print("[Ach] Unlocked id=%s (not in cache yet)" % achievement_id)
```

A couple of notes:

- Achievement progress is **monotonic** on the service side. Pushing
  `50` after `100` is silently treated as `100`, so guarding for
  re-progress is not required.
- The `achievement_unlocked` signal is driven by the Xbox
  Achievements Manager, which dispatches once `GDK.dispatch()` pumps
  it. On Godot 4.5+, the addon does this every process frame by
  default (via `gdk/runtime/embed_dispatch`), so connecting the
  signal in `_ready` is enough. On Godot 4.3 / 4.4, or when you
  disable `embed_dispatch`, call `GDK.dispatch()` yourself each
  frame (for example from a tiny autoload's `_process`).

## Step 3 — Hook unlocks into your real game

In a real game the score update lives at the gameplay layer, not in
a demo `_ready`. The bridge from "gameplay progress" to "Xbox
progress" is small enough that it usually fits in one helper on
your `Auth` autoload or a peer achievements singleton:

```gdscript
# In a new res://achievements/achievements_service.gd autoload.
extends Node

const FIRST_SCORE_ID := "1"

var _unlocked: Dictionary = {}

func _ready() -> void:
    GDK.achievements.achievement_unlocked.connect(_on_unlocked)

func report_score(score: int) -> void:
    var user: GDKUser = Auth.xbox_user
    if user == null:
        return

    # Map raw game score to 0..100 achievement progress.
    var percent: int = clamp(int(round(float(score) / 100.0 * 100.0)), 0, 100)
    if _unlocked.get(FIRST_SCORE_ID, false):
        return

    var result: GDKResult = await GDK.achievements.update_achievement_async(
        user, FIRST_SCORE_ID, percent)
    if not result.ok:
        push_warning("[Ach] update failed: %s" % result.message)

func _on_unlocked(user: GDKUser, achievement_id: String) -> void:
    _unlocked[achievement_id] = true
```

`_unlocked` is a cheap local guard so the call site does not have
to think about whether the achievement is already unlocked — the
service does the right thing either way, but skipping the network
call when you already know the result is good citizenship.

## Step 4 — React to runtime errors

Achievement updates can fail asynchronously when the Achievements
Manager surfaces an error between frames (network drop, sandbox
change, sign-out mid-call). The service exposes a dedicated
`runtime_error` signal for those:

```gdscript
func _ready() -> void:
    GDK.achievements.achievement_unlocked.connect(_on_unlocked)
    GDK.achievements.runtime_error.connect(_on_achievements_runtime_error)

func _on_achievements_runtime_error(result: GDKResult) -> void:
    push_warning("[Ach] Achievements subsystem error: %s (0x%08X)" % [result.message, result.hresult])
```

Unsolicited failures land here; the failures returned from your own
`*_async` calls land in the awaited result instead. Wiring both is
cheap and lets your HUD show "Achievements offline" when the user
loses connectivity mid-session.

## Verify

A successful run prints, in order:

```
[Ach] 3 achievement(s) declared for this title
[Ach]   1 (First Score) — 0%
[Ach]   2 (Double Down) — 0%
[Ach]   3 (Centurion) — 0%
[Ach] Updated to 50% — result ok
[Ach] Updated to 100% — result ok
[Ach] Unlocked: First Score
```

On the Xbox app for the signed-in test account, opening the title's
achievements view should show **First Score** with the unlock
timestamp.

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| Achievement count is `0` | Wrong sandbox or the achievement has not propagated to your sandbox yet. | Use **GDK → Change Sandbox…** to switch to the sandbox you authored in. |
| `Update to 50% failed: invalid_argument (...)` | The `achievement_id` does not exist in this title's declared list. | Check the id in Partner Center — it is a small integer like `"1"`, not a slug. |
| `Update to 50% failed: unauthorized (...)` | The signed-in user has no rights to the title (no test-account assignment). | Add the test account to the title's sandbox in Partner Center. |
| `[Ach] Updated to 100% — result ok` but no `Unlocked` signal | A second copy of the unlock listener already consumed it, or the achievement was previously unlocked for this account. | Check `progress_percent` and `unlocked` on the cached entry; once unlocked the signal stays silent on repeat 100% pushes. |

## Reference implementation

The cumulative end-state lives in
[`sample/tutorial_app/`](../../sample/tutorial_app/README.md):

- Scene: [`sample/tutorial_app/t02_achievement.tscn`](../../sample/tutorial_app/t02_achievement.tscn)
- Script: [`sample/tutorial_app/t02_achievement.gd`](../../sample/tutorial_app/t02_achievement.gd)
- Reuses the `Auth` autoload from T1
  ([`sample/tutorial_app/autoload/auth.gd`](../../sample/tutorial_app/autoload/auth.gd)).

## What's next
