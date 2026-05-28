# Tutorial 8 — Integration tech demo

## What you'll build

A single Control scene that wires together every surface from
Tutorials 1–7 into one panel-per-surface dashboard. The capstone
is a deliberately plain tech demo — not a game, not styled UI —
whose job is to give the reader a place where every addon they
just learned **runs at the same time, against the same signed-in
identity**, without conflicting.

By the end you will have, in one scene:

- a **HUD strip** that reads `Auth.xbox_user` / `Auth.playfab_user`
  and renders the live identity badge, plus a runtime-error
  indicator that lights up when any addon fires a service-level
  error.
- an **Achievements** panel that exercises T2 (unlock the next
  progress step on a button press, observe `achievement_unlocked`).
- a **Leaderboards** panel that exercises T3 (live top-10 plus
  around-user paging, refresh on demand).
- a **Game Saves** panel that exercises T4 (write a timestamped
  blob, upload to the cloud, read back).
- a **Lobby** panel that exercises T5 (create / join / leave,
  member list).
- an **MPA** panel that exercises T6 (advertised state +
  invite-sent log + accepted-invite log).
- a **Party** panel that exercises T7 (peer list + text-chat box
  + voice mute toggle).

Sample boot output (one signed-in user, no friends in the lobby
yet):

```
[Auth] Sign-in complete.
[Hud] identity badge live for SteelGorilla
[Ach] cached 3 achievement(s) for the local user
[Lb]  top-10 cached (10 entries)
[Gs]  user folder resolved: C:\Users\…\Saves\…
[Lobby] panel ready
[Mpa] activity panel ready (idle — no lobby)
[Pty] party panel ready (idle — no network)
```

Sample mid-session output once the host has created a lobby,
unlocked an achievement, submitted a score, and saved progress:

```
[Ach] Unlocked id=1 (Welcome aboard) → score 10
[Lb]  Top-10 refresh: 1. SteelGorilla 1240
[Gs]  Wrote progress.dat (47 bytes), upload synced
[Lobby] Lobby created: id=BBA9... max=4
[Mpa] Activity advertised: max=4 current=1 cross_platform=false
[Pty] Network created — descriptor published on lobby
```

This is the end state of your cumulative project. Every prior
tutorial built one of the pieces; this tutorial just lays them
out side by side.

## Prerequisites

- Tutorials 1–7 are complete, in order. The capstone does **not**
  re-derive any of the per-surface code — it composes the
  autoloads and helper functions you already built.
- The cumulative `Auth` and `Lobby` autoloads from T1 and T5,
  extended with T6's MPA wiring, are registered in the project's
  **Autoload** list.
- `playfab/runtime/initialize_on_startup` and
  `gdk/runtime/initialize_on_startup` are both `true` so the two
  runtimes are up by the time the dashboard's panels query them.
- A signed-in Xbox test account with at least one **declared
  achievement** in Partner Center (any progress-style achievement
  works) and a **PlayFab statistic + a leaderboard that sources the
  statistic** configured in Game Manager (the snippets below use
  `"high_score"` for both, matching T3).
- For end-to-end Lobby / MPA / Party panels: a second client
  signed into a different test account in the same sandbox. The
  single-client run still walks through every panel; it just shows
  the empty-state UI for the multiplayer surfaces.
- One-page primer on async patterns: [Async patterns](../async-patterns.md).

## Relevant addon surfaces

- `Auth.xbox_user`, `Auth.playfab_user`, `Auth.state_changed`,
  `Auth.sign_in()`, `Auth.is_signed_in()` — your T1 state-machine
  autoload.
- `Lobby._lobby`, `Lobby.host_lobby`, `Lobby.join_lobby`,
  `Lobby.leave_lobby`, `Lobby.invite_friend`,
  `Lobby.open_invite_picker` — your T5+T6 autoload.
- [`GDK.achievements`](../../addons/godot_gdk/doc_classes/GDKAchievements.xml)
  — `query_player_achievements_async`, `update_achievement_async`,
  `achievement_unlocked` signal.
- [`PlayFab.statistics`](../../addons/godot_playfab/doc_classes/PlayFabStatistics.xml)
  — `update_statistics_async` (client-write entry point for the
  statistic that backs the leaderboard).
- [`PlayFab.leaderboards`](../../addons/godot_playfab/doc_classes/PlayFabLeaderboards.xml)
  — `get_leaderboard_async`,
  `get_leaderboard_around_user_async`. Response shape:
  `result.data = { entry_count, version, [next_reset], rankings: Array<Dictionary> }`,
  where each ranking carries `display_name`, `rank`, `entity: {id, type}`,
  and `scores: PackedStringArray` (decimal-encoded column values).
- [`PlayFab.game_saves`](../../addons/godot_playfab/doc_classes/PlayFabGameSaves.xml) —
  `add_user_with_ui_async`, `upload_with_ui_async`,
  `get_folder`.
- [`GDK.multiplayer_activity`](../../addons/godot_gdk/doc_classes/GDKMultiplayerActivity.xml)
  — `set_activity_async`, `delete_activity_async`,
  `pending_invite_received`, `invite_accepted`,
  `activities_updated`.
- [`GDK.presence`](../../addons/godot_gdk/doc_classes/GDKPresence.xml)
  — `set_presence_async`, `clear_presence_async`,
  `track_presence`, `get_presence_async`, `get_cached_presence`,
  signals `device_presence_changed`, `title_presence_changed`,
  `presence_changed`. Inherited through the `Lobby` autoload
  from T5 Step 8 and T6 Step 7 — no panel code needed.
- [`GDK.users`](../../addons/godot_gdk/doc_classes/GDKUsers.xml)
  — `check_privilege_async`, `resolve_privilege_with_ui_async`.
  Inherited through the `Lobby` autoload (Multiplayer privilege
  from T5 Step 2) and the `Party` autoload (Communications +
  CommunicationVoiceIngame from T7 Step 6). See the
  **Cert readiness** sidebar after Step 7 for how the panels
  benefit without taking on their own gating.
- [`GDK.privacy`](../../addons/godot_gdk/doc_classes/GDKPrivacy.xml)
  — `check_permission_async`, `batch_check_permission_async`.
  Per-target permission filtering for invites (T6 Step 5) and
  per-peer chat permissions (T7 Step 6); inherited through the
  Lobby and Party autoloads.
- [`PlayFab.party`](../../addons/godot_playfab/doc_classes/PlayFabParty.xml) —
  `create_and_join_network_async`, `join_network_async`. The
  `PlayFabPartyNetwork.state_changed` signal carries network
  lifecycle changes; the `PlayFabPartyPeer.text_message_received`
  signal carries inbound chat.

## Step 1 — Scene skeleton

Create `res://t08_integration/t08_integration.tscn` with this
node hierarchy. The nodes are deliberately plain — no theming,
no custom containers, just enough Godot UI to hold every panel
in one frame:

```
TechDemo (Control)
├── Root (VBoxContainer)
│   ├── Hud (HBoxContainer)
│   │   ├── IdentityLabel (Label)
│   │   ├── ErrorLabel (Label)
│   │   └── SignInRetry (Button)
│   └── Tabs (TabContainer)
│       ├── Achievements (VBoxContainer)
│       ├── Leaderboard (VBoxContainer)
│       ├── GameSaves (VBoxContainer)
│       ├── Lobby (VBoxContainer)
│       ├── MPA (VBoxContainer)
│       └── Party (VBoxContainer)
```

Attach `res://t08_integration/t08_integration.gd` to the root
`TechDemo` node. The script is the wiring layer — every panel
gets its own script attached to its own `VBoxContainer`:

```gdscript
extends Control

@onready var _identity: Label = $Root/Hud/IdentityLabel
@onready var _error: Label = $Root/Hud/ErrorLabel
@onready var _retry: Button = $Root/Hud/SignInRetry

func _ready() -> void:
    _identity.text = "Signing in…"
    _error.text = ""
    _retry.pressed.connect(_on_retry_pressed)

    Auth.state_changed.connect(_on_auth_state_changed)
    _on_auth_state_changed(Auth.get_state())

    GDK.runtime_error.connect(_on_runtime_error.bind("gdk"))
    GDK.achievements.runtime_error.connect(_on_runtime_error.bind("achievements"))
    PlayFab.multiplayer.multiplayer_error.connect(_on_pf_runtime_error.bind("multiplayer"))
    PlayFab.party.party_error.connect(_on_pf_runtime_error.bind("party"))

    # Kick sign-in for cold T8 entry. Idempotent — joins the
    # autoload's in-flight attempt if one is already running.
    await Auth.sign_in()

func _on_auth_state_changed(_state: Auth.State) -> void:
    if Auth.is_signed_in():
        _identity.text = "%s ↔ PlayFab:%s" % [
            Auth.xbox_user.gamertag,
            str(Auth.playfab_user.entity_key.get("id", "")).left(8),
        ]
        _error.text = ""
        print("[Hud] identity badge live for %s" % Auth.xbox_user.gamertag)
    elif Auth.is_signing_in():
        _identity.text = "Signing in…"
    elif Auth.is_failed():
        _identity.text = "(not signed in)"
        _error.text = "Sign-in failed (%s): %s" % [
                Auth.get_last_error_stage(),
                Auth.get_last_error_message()]
        push_warning("[Hud] %s" % _error.text)
    else:
        _identity.text = "(not signed in)"

func _on_retry_pressed() -> void:
    _error.text = ""
    await Auth.sign_in()

func _on_runtime_error(result: GDKResult, source: String) -> void:
    _error.text = "[%s] %s" % [source, result.message]
    push_warning("[Hud] runtime error from %s: %s" % [source, result.message])

func _on_pf_runtime_error(result: PlayFabResult, source: String) -> void:
    _error.text = "[%s] %s" % [source, result.message]
    push_warning("[Hud] PlayFab runtime error from %s: %s" % [source, result.message])
```

The HUD does exactly two things:

- shows the live identity (so you can confirm the right account
  signed in)
- shows the most recent service-level runtime error (so the panel
  scripts below can stay focused on their happy path; if a
  surface goes wrong, the HUD lights up).

This is also the only place we listen to `Auth.state_changed` for
its full lifecycle — panel scripts just `await Auth.sign_in()` and
assume success. Re-entrant sign-in is driven by the retry button,
which calls the same `Auth.sign_in()` accessor that resets stale
failure state and starts a fresh attempt.

## Step 2 — Achievements panel

Attach `res://t08_integration/panel_achievements.gd` to the
`Achievements` VBoxContainer. The panel mirrors the T2 happy path
in a button-per-step form so you can click through progress
without rebuilding the project:

```gdscript
extends VBoxContainer

const ACHIEVEMENT_ID := "1"

@onready var _status: Label = $Status
@onready var _progress_25: Button = $Progress25
@onready var _progress_50: Button = $Progress50
@onready var _progress_75: Button = $Progress75
@onready var _unlock: Button = $Unlock

func _ready() -> void:
    if not await Auth.sign_in():
        return

    GDK.achievements.achievement_unlocked.connect(_on_achievement_unlocked)
    _progress_25.pressed.connect(_push_progress.bind(25))
    _progress_50.pressed.connect(_push_progress.bind(50))
    _progress_75.pressed.connect(_push_progress.bind(75))
    _unlock.pressed.connect(_push_progress.bind(100))

    var result: GDKResult = await GDK.achievements.query_player_achievements_async(Auth.xbox_user)
    if result.ok:
        print("[Ach] cached %d achievement(s) for the local user" % result.data.size())
        _refresh_status()
    else:
        push_warning("[Ach] query failed: %s" % result.message)

func _push_progress(percent: int) -> void:
    var result: GDKResult = await GDK.achievements.update_achievement_async(
        Auth.xbox_user, ACHIEVEMENT_ID, percent)
    if result.ok:
        _status.text = "Pushed %d%%" % percent
        print("[Ach] Updated to %d%%" % percent)
    else:
        _status.text = "Update failed: %s" % result.message
        push_warning("[Ach] %s" % _status.text)

func _on_achievement_unlocked(user: GDKUser, achievement_id: String) -> void:
    if achievement_id != ACHIEVEMENT_ID:
        return
    _status.text = "Unlocked %s for %s" % [achievement_id, user.gamertag]
    print("[Ach] Unlocked id=%s" % achievement_id)
    _refresh_status()

func _refresh_status() -> void:
    var cached: Array = GDK.achievements.get_cached_achievements(Auth.xbox_user)
    for ach in cached:
        if ach.id == ACHIEVEMENT_ID:
            _status.text = "%s: %d%% — %s" % [ach.id, ach.progress_percent,
                "Unlocked" if ach.progress_percent >= 100 else "In progress"]
            return
    _status.text = "Achievement %s not yet in cache" % ACHIEVEMENT_ID
```

The four buttons (25 / 50 / 75 / Unlock) write the canonical T2
progress curve. You should see each click push the next step,
the panel update its status label, and the `achievement_unlocked`
signal fire exactly once when the progress hits 100 — same as in
T2, just behind a button.

## Step 3 — Leaderboards panel

Attach `res://t08_integration/panel_leaderboard.gd` to the
`Leaderboard` VBoxContainer. The panel pulls the top-10 and
around-user windows side by side, with a refresh button so you
can re-run after a score submission:

```gdscript
extends VBoxContainer

const STATISTIC_NAME := "high_score"
const LEADERBOARD_NAME := "high_score"

@onready var _top10: Label = $Top10
@onready var _around: Label = $AroundUser
@onready var _submit: Button = $SubmitScore
@onready var _refresh: Button = $Refresh
@onready var _status: Label = $Status

var _scratch_score: int = 100

func _ready() -> void:
    if not await Auth.sign_in():
        return
    _submit.pressed.connect(_on_submit_pressed)
    _refresh.pressed.connect(_refresh_views)
    await _refresh_views()

func _on_submit_pressed() -> void:
    _scratch_score += 10
    var result: PlayFabResult = await PlayFab.statistics.update_statistics_async(
            Auth.playfab_user, {
                "statistics": [
                    {"name": STATISTIC_NAME, "scores": [str(_scratch_score)]},
                ],
            })
    if result.ok:
        _status.text = "Recorded %d to %s" % [_scratch_score, STATISTIC_NAME]
        print("[Lb] Recorded %d to %s" % [_scratch_score, STATISTIC_NAME])
    else:
        _status.text = "Record failed: %s" % result.message
        return
    await _refresh_views()

func _refresh_views() -> void:
    var top: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_async(
            Auth.playfab_user, LEADERBOARD_NAME, 1, 10)
    if top.ok:
        var rankings: Array = top.data.get("rankings", [])
        _top10.text = _render(rankings)
        if not rankings.is_empty():
            var first: Dictionary = rankings[0]
            print("[Lb] Top-10 refresh: 1. %s %d" % [
                _display_name(first), _primary_score(first)])
        else:
            print("[Lb] Top-10 refresh: (empty)")
    else:
        _top10.text = "Top-10 failed: %s" % top.message

    var around: PlayFabResult = await PlayFab.leaderboards.get_leaderboard_around_user_async(
            Auth.playfab_user, LEADERBOARD_NAME, 3)
    if around.ok:
        _around.text = _render(around.data.get("rankings", []))
    else:
        _around.text = "Around-user failed: %s" % around.message

func _render(rankings: Array) -> String:
    var lines := PackedStringArray()
    for entry in rankings:
        var row: Dictionary = entry
        lines.append("%d. %s — %d" % [row.get("rank", 0), _display_name(row), _primary_score(row)])
    if lines.is_empty():
        return "(no entries)"
    return "\n".join(lines)

func _display_name(row: Dictionary) -> String:
    var name: String = row.get("display_name", "")
    if not name.is_empty():
        return name
    var entity: Dictionary = row.get("entity", {})
    return entity.get("id", "?")

func _primary_score(row: Dictionary) -> int:
    var scores: PackedStringArray = row.get("scores", PackedStringArray())
    return scores[0].to_int() if not scores.is_empty() else 0
```

Two things worth calling out:

- The submit button drives the same `update_statistics_async` flow
  from T3, except the score increments every click instead of being
  read from a gameplay value. That lets you watch the refresh redraw
  the user's position in the around-user window after each record
  (statistic-to-leaderboard propagation is eventually consistent —
  a one-second delay between record and refresh is normal, and
  PlayFab can occasionally take longer).
- The top-10 and around-user calls run **sequentially** here for
  prose clarity. In production, fan them out by saving the
  signals first (`var top_signal := …`, `var around_signal := …`)
  and awaiting both — see the fan-in pattern in
  [Async patterns](../async-patterns.md).

## Step 4 — Game Saves panel

Attach `res://t08_integration/panel_game_saves.gd` to the
`GameSaves` VBoxContainer. The panel writes a timestamped blob,
uploads it, and reads it back:

```gdscript
extends VBoxContainer

const SAVE_FILE := "progress.dat"

@onready var _status: Label = $Status
@onready var _last_read: Label = $LastRead
@onready var _write: Button = $Write
@onready var _read: Button = $Read

var _save_folder: String = ""

func _ready() -> void:
    if not await Auth.sign_in():
        return

    var result: PlayFabResult = await PlayFab.game_saves.add_user_with_ui_async(Auth.playfab_user)
    if not result.ok:
        _status.text = "Add user failed: %s" % result.message
        push_warning("[Gs] add_user failed: %s" % result.message)
        return
    _save_folder = String(result.data.get("folder", ""))
    print("[Gs] user folder resolved: %s" % _save_folder)
    _status.text = "Folder: %s" % _save_folder

    _write.pressed.connect(_on_write_pressed)
    _read.pressed.connect(_on_read_pressed)

func _on_write_pressed() -> void:
    if _save_folder.is_empty():
        return
    var path: String = "%s/%s" % [_save_folder, SAVE_FILE]
    var payload: String = "saved=%s timestamp=%s" % [Auth.xbox_user.gamertag, Time.get_datetime_string_from_system()]
    var f := FileAccess.open(path, FileAccess.WRITE)
    if f == null:
        _status.text = "Write open failed: %s" % str(FileAccess.get_open_error())
        return
    f.store_string(payload)
    f.close()
    var bytes: int = payload.length()
    var upload: PlayFabResult = await PlayFab.game_saves.upload_with_ui_async(Auth.playfab_user, false)
    if upload.ok:
        _status.text = "Wrote %s (%d bytes), upload synced" % [SAVE_FILE, bytes]
        print("[Gs] Wrote %s (%d bytes), upload synced" % [SAVE_FILE, bytes])
    else:
        _status.text = "Wrote locally, upload failed: %s" % upload.message
        push_warning("[Gs] upload failed: %s" % upload.message)

func _on_read_pressed() -> void:
    if _save_folder.is_empty():
        return
    var path: String = "%s/%s" % [_save_folder, SAVE_FILE]
    var f := FileAccess.open(path, FileAccess.READ)
    if f == null:
        _last_read.text = "Open failed: %s" % str(FileAccess.get_open_error())
        return
    _last_read.text = f.get_as_text()
    f.close()
```

The panel deliberately uses `FileAccess` directly on the folder
that Game Saves resolves, mirroring T4. Game Saves is the cloud
sync layer — once the folder path is in hand, everything else is
ordinary Godot file IO.

## Step 5 — Lobby panel

Attach `res://t08_integration/panel_lobby.gd` to the `Lobby`
VBoxContainer. The panel is the smallest possible Lobby UI: three
buttons (host / join from clipboard / leave) and a member list:

```gdscript
extends VBoxContainer

@onready var _host: Button = $Host
@onready var _join: Button = $Join
@onready var _leave: Button = $Leave
@onready var _status: Label = $Status
@onready var _members: Label = $Members
@onready var _connection_string: LineEdit = $ConnectionString

func _ready() -> void:
    if not await Auth.sign_in():
        return
    _host.pressed.connect(_on_host_pressed)
    _join.pressed.connect(_on_join_pressed)
    _leave.pressed.connect(_on_leave_pressed)
    PlayFab.multiplayer.state_changed.connect(_refresh)
    print("[Lobby] panel ready")
    _refresh()

func _on_host_pressed() -> void:
    await Lobby.host_lobby()
    if Lobby._lobby != null:
        _connection_string.text = Lobby._lobby.connection_string
    _refresh()

func _on_join_pressed() -> void:
    var text: String = _connection_string.text.strip_edges()
    if text.is_empty():
        _status.text = "Paste a connection string into the field first"
        return
    await Lobby.join_lobby_with_string(text)
    _refresh()

func _on_leave_pressed() -> void:
    await Lobby.leave_lobby()
    _refresh()

func _refresh(_change = null) -> void:
    if Lobby._lobby == null:
        _status.text = "Not in a lobby"
        _members.text = ""
        return
    _status.text = "Lobby %s (%d / %d)" % [
        Lobby._lobby.lobby_id.left(8),
        Lobby._lobby.member_count,
        Lobby._lobby.max_member_count]
    var lines := PackedStringArray()
    for member: PlayFabLobbyMember in Lobby._lobby.members:
        lines.append("- %s%s" % [member.user_id, " (you)" if member.is_local else ""])
    _members.text = "\n".join(lines)
```

> **Note:** the snippet above calls `Lobby.join_lobby_with_string(text)`.
> Tutorial 5 used a hard-coded `JOIN_STRING` constant; the
> capstone swaps that for a `join_lobby_with_string(text: String)`
> helper on the `Lobby` autoload so the panel can pass a string
> from a `LineEdit`. Add this helper to the autoload alongside
> the existing `join_lobby`:
>
> ```gdscript
> func join_lobby_with_string(connection_string: String) -> void:
>     var user: PlayFabUser = Auth.playfab_user
>     var config := PlayFabLobbyJoinConfig.new()
>     var result: PlayFabResult = await PlayFab.multiplayer.join_lobby_async(user, connection_string, config)
>     if not result.ok:
>         push_warning("[Lobby] join_lobby failed: %s (%s)" % [result.message, result.code])
>         return
>     _lobby = result.data
>     _lobby.state_changed.connect(_on_lobby_state_changed)
>     print("[Lobby] Joined lobby id=%s with %d member(s)" % [_lobby.lobby_id, _lobby.member_count])
>     await _publish_activity()
>     await _publish_lobby_presence()
> ```

> **Inherited from T5 + T6 (no extra panel code).** The Lobby
> autoload's `host_lobby` / `join_lobby` / `leave_lobby` already
> drive `GDK.presence.set_presence_async` / `clear_presence_async`
> from [T5 Step 8](05-multiplayer-lobby.md#step-8--advertise-rich-presence-optional),
> so creating or joining from this panel automatically publishes
> the local user's rich presence to friends' shells. The MPA
> panel (Step 6 below) inherits the friend-presence read flow
> from [T6 Step 7](06-multiplayer-activity.md#step-7--show-friend-presence-next-to-the-joinable-badge)
> the same way — no panel-level changes needed in the capstone.

## Step 6 — MPA panel

Attach `res://t08_integration/panel_mpa.gd` to the `MPA`
VBoxContainer. The panel reads the activity state from the
`Lobby` autoload and surfaces the invite send / picker buttons
from T6:

```gdscript
extends VBoxContainer

@onready var _state: Label = $State
@onready var _xuid_input: LineEdit = $XuidInput
@onready var _send: Button = $Send
@onready var _picker: Button = $Picker
@onready var _log: RichTextLabel = $Log

func _ready() -> void:
    if not await Auth.sign_in():
        return
    _send.pressed.connect(_on_send_pressed)
    _picker.pressed.connect(_on_picker_pressed)
    GDK.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)
    GDK.multiplayer_activity.pending_invite_received.connect(_on_pending_invite)
    PlayFab.multiplayer.state_changed.connect(_refresh)
    print("[Mpa] activity panel ready (idle — no lobby)")
    _refresh()

func _refresh(_change = null) -> void:
    if Lobby._lobby == null:
        _state.text = "No lobby — activity not advertised"
        return
    _state.text = "Advertising %s (%d / %d, cross=%s)" % [
        Lobby._lobby.lobby_id.left(8),
        Lobby._lobby.member_count,
        Lobby._lobby.max_member_count,
        str(false)]

func _on_send_pressed() -> void:
    var xuid: String = _xuid_input.text.strip_edges()
    if xuid.is_empty():
        _log.append_text("[i]Enter a XUID first[/i]\n")
        return
    await Lobby.invite_friend(xuid)
    _log.append_text("Sent invite to %s\n" % xuid)
    # invite_friend returns a bool — see T6 Step 5 — so this panel can
    # surface a suppressed-invite hint by checking `await`'s value if
    # the title wants to differentiate "sent" from "blocked / failed".

func _on_picker_pressed() -> void:
    await Lobby.open_invite_picker()
    _log.append_text("Closed system invite picker\n")

func _on_invite_accepted(invite: Dictionary) -> void:
    _log.append_text("Accepted: %s\n" % invite.get("raw_uri", ""))

func _on_pending_invite(invite: Dictionary) -> void:
    _log.append_text("Pending: %s\n" % invite.get("raw_uri", ""))
```

The panel is entirely reactive — the actual MPA state lives on
the `Lobby` autoload from T6. The MPA panel exists to make the
state legible at a glance (advertised vs. idle) and to expose the
two outbound invite flows behind buttons.

## Step 7 — Party panel

Attach `res://t08_integration/panel_party.gd` to the `Party`
VBoxContainer. The panel adds Party-network create / join and a
text-chat surface on top of the lobby panel:

```gdscript
extends VBoxContainer

@onready var _peer_list: Label = $PeerList
@onready var _chat_log: RichTextLabel = $ChatLog
@onready var _chat_input: LineEdit = $ChatInput
@onready var _send: Button = $Send
@onready var _mute_remotes: CheckButton = $MuteRemotes

var _network: PlayFabPartyNetwork = null
var _peer: PlayFabPartyPeer = null

func _ready() -> void:
    if not await Auth.sign_in():
        return
    # PlayFab.party init happens lazily inside the Party autoload when
    # host_party() / _join_party_network() runs. This panel observes via
    # Party.network_joined / network_left and does not bring the SDK up
    # itself.
    _send.pressed.connect(_on_send_pressed)
    _mute_remotes.toggled.connect(_on_mute_remotes_toggled)
    _attach_network(Party.network)
    print("[Pty] party panel ready (%s)" % ["connected" if _network else "idle — no network"])

func _attach_network(network: PlayFabPartyNetwork) -> void:
    if network == _network:
        return
    # Disconnect signals from the previous network/peer before rebinding so
    # tearing down and recreating the Party network doesn't multi-fire callbacks.
    if _network != null and _network.state_changed.is_connected(_on_network_state_changed):
        _network.state_changed.disconnect(_on_network_state_changed)
    if _peer != null:
        if _peer.text_message_received.is_connected(_on_text_received):
            _peer.text_message_received.disconnect(_on_text_received)
        if _peer.chat_control_added.is_connected(_on_chat_control_added):
            _peer.chat_control_added.disconnect(_on_chat_control_added)
        if _peer.chat_control_removed.is_connected(_on_chat_control_removed):
            _peer.chat_control_removed.disconnect(_on_chat_control_removed)
    _network = network
    if _network == null:
        _peer = null
        _refresh_peers()
        return
    _peer = _network.local_peer
    _network.state_changed.connect(_on_network_state_changed)
    if _peer != null:
        _peer.text_message_received.connect(_on_text_received)
        _peer.chat_control_added.connect(_on_chat_control_added)
        _peer.chat_control_removed.connect(_on_chat_control_removed)
    _refresh_peers()

func _on_send_pressed() -> void:
    var text: String = _chat_input.text.strip_edges()
    if text.is_empty() or _peer == null:
        return
    var result: PlayFabResult = await _peer.send_text_async(text)
    if result.ok:
        _chat_log.append_text("[me] %s\n" % text)
        _chat_input.text = ""
    else:
        _chat_log.append_text("[i]send_text_async failed: %s[/i]\n" % result.message)

func _on_mute_remotes_toggled(button_pressed: bool) -> void:
    # The addon exposes per-peer mute via PlayFabPartyPeer.set_peer_muted_async.
    # The capstone wires the toggle to mute/unmute every known remote peer at
    # once; production titles will typically expose this per row in a roster.
    if _peer == null:
        return
    for peer_id in _peer.get_peers():
        _peer.set_peer_muted_async(peer_id, button_pressed)

func _on_network_state_changed(_change: PlayFabPartyNetworkStateChange) -> void:
    _refresh_peers()

func _on_chat_control_added(_peer_id: int, _control: PlayFabPartyChatControl) -> void:
    _refresh_peers()

func _on_chat_control_removed(_peer_id: int) -> void:
    _refresh_peers()

func _on_text_received(peer_id: int, message: PlayFabPartyChatMessage) -> void:
    var label: String = "?"
    if _peer != null:
        var entity: Dictionary = _peer.get_peer_entity_key(peer_id)
        label = String(entity.get("id", "?")).left(8)
    _chat_log.append_text("[%s] %s\n" % [label, message.text])

func _refresh_peers() -> void:
    if _peer == null:
        _peer_list.text = "Not connected"
        return
    var lines := PackedStringArray()
    # PlayFabPartyPeer inherits MultiplayerPeer; get_peers() returns the
    # remote peer ids (the local peer is not listed).
    for peer_id in _peer.get_peers():
        var entity: Dictionary = _peer.get_peer_entity_key(peer_id)
        var id_label: String = String(entity.get("id", "?")).left(8)
        lines.append("- %s (peer %d)" % [id_label, peer_id])
    if lines.is_empty():
        lines.append("- (waiting for remote peers)")
    _peer_list.text = "\n".join(lines)
```

This panel assumes the existence of a `Party` autoload that holds
the active `PlayFabPartyNetwork` (introduced in T7). Wire the
autoload to call `_attach_network` whenever its network changes
(e.g., emit a `network_ready(network: PlayFabPartyNetwork)` signal
from the autoload after `create_and_join_network_async` resolves
and connect to it here, or poll `Party.network` from this panel's
`_process`).

The `PlayFabPartyPeer.text_message_received` signal arrives on
the **local peer** with the sending peer's id as its first
parameter — that's why the chat log resolves a label by looking
up `peer.get_peer_entity_key(peer_id)`. `send_text_async` with no
explicit `target_peer_ids` broadcasts to every chat control the
local peer has currently mapped; new peers that join later won't
retroactively see the message.

The **mute remotes** toggle uses `set_peer_muted_async(peer_id,
muted)` against every known remote peer. The addon does not
expose a local-microphone mute API; if you want "mute my mic"
behavior, gate input capture at the OS level (Windows audio
endpoint mute) instead. See T7 for the canonical create / join
flow that produces the `PlayFabPartyNetwork` this panel renders.

## Cert readiness — privileges and permissions inherited

The capstone deliberately does **not** spread privilege /
permission gating across each panel. The autoloads do that work
once, and every panel that depends on them benefits:

- The **Lobby** autoload (T5 Step 2) gates `host_lobby` /
  `join_lobby` on the local user's **Multiplayer** privilege via
  `GDK.users.check_privilege_async(user, XUSER_PRIVILEGE_MULTIPLAYER)`
  with `resolve_privilege_with_ui_async` as the fallback. The
  capstone Lobby panel's `_on_host_pressed` / `_on_join_pressed`
  call into the autoload, so the gate runs without any panel
  code.
- The **MPA** panel's targeted invite (Step 6) inherits the
  [T6 Step 5 callout](06-multiplayer-activity.md#step-5--send-an-invite-from-inside-the-game)'s
  `batch_check_permission_async("play_multiplayer", xuids)`
  filter through the same `Lobby.invite_friend` autoload helper.
  The picker invite path (`show_invite_ui_async`) does its own
  permission filtering inside the Xbox shell.
- The **Party** autoload (T7 Step 6) decides whether to enable
  voice / text on the `PlayFabPartyConfig` based on the local
  user's `Communications` + `CommunicationVoiceIngame`
  privileges, then walks each `chat_control_added` peer through
  `check_permission_async` for `communicate_using_voice` /
  `communicate_using_text` and calls
  `set_peer_chat_permissions_async` with the resulting mask. The
  capstone Party panel's chat surface is the post-gate UI; if
  the privilege check denies voice, the mic stays off
  regardless of what the panel's `mute_remotes` toggle says.

If you ship this capstone unchanged into a cert build, the
gating story is already correct. The thing to verify in a cert
pass is that the *autoloads* surface a denial cleanly (the
push_warning lines), not that the panels add their own checks on
top.

## Verify

A clean host-side first run prints, in order:

```
[Auth] Sign-in complete.
[Hud] identity badge live for SteelGorilla
[Ach] cached 3 achievement(s) for the local user
[Gs]  user folder resolved: C:\Users\…\Saves\…
[Lobby] panel ready
[Mpa] activity panel ready (idle — no lobby)
[Pty] party panel ready (idle — no network)
```

After clicking through the tabs:

```
[Ach] Updated to 25%
[Ach] Updated to 50%
[Ach] Updated to 75%
[Ach] Unlocked id=1
[Lb]  Recorded 110 to high_score
[Lb]  Top-10 refresh: 1. SteelGorilla 110
[Gs]  Wrote progress.dat (47 bytes), upload synced
[Lobby] Lobby created: id=BBA9... max=4
[Mpa] Activity advertised: max=4 current=1 cross_platform=false
[Pty] Network created — descriptor published on lobby
```

The HUD's `IdentityLabel` stays live across all tabs; the
`ErrorLabel` should stay empty unless a service-level runtime
error fires.

## Common failures

| Symptom | Diagnosis | Fix |
|---|---|---|
| HUD says "Signing in…" forever | `Auth` is stuck in `SIGNING_IN_*`. | Check the Output panel for a `[Auth] sign-in failed at <stage>: <message>` line and run the matching T1 fix. The HUD retry button re-runs `Auth.sign_in()` without restarting the scene; `sign_in()` resets stale failure state on each retry. |
| Achievements panel `Status` is "Achievement 1 not yet in cache" indefinitely | The achievement id passed in `ACHIEVEMENT_ID` does not match a declared achievement in Partner Center. | Update `ACHIEVEMENT_ID` to one of your declared ids (a numeric string like `"2"`, not a slug). |
| Leaderboard panel renders `(no entries)` even after a successful record | Statistic-to-leaderboard propagation is eventually consistent, or the leaderboard's source statistic does not match `STATISTIC_NAME`. | Wait 1–10 seconds and click **Refresh**. If still empty, confirm in Game Manager that `LEADERBOARD_NAME` matches a leaderboard sourced from the statistic named `STATISTIC_NAME`, and that the entity type seeded by your sign-in (`title_player_account`) matches the statistic's expected entity. See T3's common-failures table for the same diagnoses. |
| Game Saves panel `_save_folder` stays empty | `add_user_with_ui_async` failed. | Most common cause: the PlayFab session is custom-id rather than Xbox-backed. See T4's common failures table. |
| MPA panel shows "Advertising …" but the second client never sees a join card | Sandbox mismatch between PC and friend's PC. | See [Troubleshooting → Sandbox mismatch](../troubleshooting.md). |
| Party panel chat send returns ok but no remote ever receives it | The remote peer's chat control had not been mapped at send time (zero broadcast targets). | Wait for `PlayFabPartyPeer.chat_control_added(peer_id, control)` for at least one remote peer before exposing the chat send UI — same diagnosis as T7. |
| `_on_runtime_error` fires every frame for the same service | Stale connection that the addon keeps retrying. | Restart the runtime: `PlayFab.shutdown()` then `PlayFab.initialize()` from the HUD's retry button, or recreate the Party network from the Party panel. |

## Reference implementation

The cumulative end-state lives in
[`sample/tutorial_app/`](../../sample/tutorial_app/README.md):

- Scene: [`sample/tutorial_app/t08_integration/t08_integration.tscn`](../../sample/tutorial_app/t08_integration/t08_integration.tscn)
- Root script: [`sample/tutorial_app/t08_integration/t08_integration.gd`](../../sample/tutorial_app/t08_integration/t08_integration.gd)
- Per-surface panels (one per Step):
  - [`panel_achievements.gd`](../../sample/tutorial_app/t08_integration/panel_achievements.gd) (Step 2)
  - [`panel_leaderboard.gd`](../../sample/tutorial_app/t08_integration/panel_leaderboard.gd) (Step 3)
  - [`panel_game_saves.gd`](../../sample/tutorial_app/t08_integration/panel_game_saves.gd) (Step 4)
  - [`panel_lobby.gd`](../../sample/tutorial_app/t08_integration/panel_lobby.gd) (Step 5)
  - [`panel_mpa.gd`](../../sample/tutorial_app/t08_integration/panel_mpa.gd) (Step 6)
  - [`panel_party.gd`](../../sample/tutorial_app/t08_integration/panel_party.gd) (Step 7)
- Consumes the `Auth`, `Lobby`, and `Party` autoloads in
  [`sample/tutorial_app/autoload/`](../../sample/tutorial_app/autoload/).

Tutorial paths in this Step (`res://t08_integration/...`) match
the sample exactly — no per-folder divergence here.

## What's next

If you skipped the parallel GameInput track, that is the next
recommended read — it adds Microsoft GameInput on top of Godot's
`InputMap` and is independent of everything you built here:

- [**GameInput action bridge**](gameinput-action-bridge.md)

When you ship your own title using these addons, the only piece
that needs to grow from this tech demo is the **panel surfacing**
— your menu, your in-game HUD, your invite flow. The underlying
service calls in each panel stay almost identical to what you
have here.
