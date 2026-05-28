# Tutorial 6 — Advertise your lobby with Multiplayer Activity

## What you'll build

Hook your [Tutorial 5 lobby](05-multiplayer-lobby.md) into the
Xbox shell's **Multiplayer Activity** (MPA) surface so the user's
friends can see "Playing with you" cards, accept invites from the
Game Bar, and rejoin your session through the system UI. By the
end you will:

- Set the local user's multiplayer activity to the lobby's
  `connection_string` every time you create or join a lobby, and
  clear it on leave.
- Wire the `pending_invite_received` and `invite_accepted`
  signals on `GDK.multiplayer_activity` so the system invite UI
  drives `PlayFab.multiplayer.join_lobby_async(...)` end-to-end.
- Send an invite to a specific XUID with
  `send_invites_async(...)` and surface the system invite picker
  with `show_invite_ui_async(...)`.
- React to `activities_updated` so an in-game friend list can
  light up "joinable" badges for friends who are in a session.

Sample output (host side, after a friend accepts an invite):

```
[MPA] Activity advertised: max=4 current=1 cross_platform=false
[MPA] Sent invite to 2814635463725476
[MPA] Activity updated for friends: ["2814635463725476"]
[MPA] Friend 2814635463725476 is in session: ms-xbl-multiplayer://...
[Lobby] member added: title_player_account:6F4B... (local=false)
```

Sample output (client side, after the user accepted from the
Game Bar):

```
[MPA] Invite accepted from Game Bar: scheme=ms-xbl-multiplayer action=inviteHandleAccept
[Lobby] Joined lobby id=BBA9... with 2 member(s)
[MPA] Activity advertised: max=4 current=2 cross_platform=false
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete and
  `Auth.xbox_user` resolves to a signed-in `GDKUser`.
- [Tutorial 5 — Create and join a lobby](05-multiplayer-lobby.md) is
  complete. This tutorial extends T5's `Lobby` autoload with
  activity wiring; you need the existing `_lobby: PlayFabLobby`
  member and the `host_lobby` / `join_lobby` / `leave_lobby`
  methods from T5 in place.
- The PC is in the sandbox that owns your title's SCID — MPA
  resolves to the per-sandbox MPSD ("Multiplayer Session
  Directory"), and a wrong-sandbox PC silently writes activities
  into a graveyard SCID nobody can see.
- For end-to-end testing of invites: two Xbox **test accounts**
  that follow each other in the same sandbox, signed into two
  separate machines (or two Xbox app sessions on the same PC).
  Friends-of-friends invites also work but are flakier in
  pre-release sandboxes.
- One-page primer on the addons' async model: [Async patterns](../async-patterns.md).

> **MPA vs. the lobby.** A `PlayFabLobby` is the actual session
> roster — connection string, member list, properties, ownership.
> The **multiplayer activity** is an Xbox shell concept layered on
> top: a per-user advertisement that says "this XUID is in a
> session with this connection string, here's how to join". MPA
> does not move bytes. It exists so the Xbox / Game Bar / Game DVR
> / "Currently playing" surfaces know that a user is joinable and
> what string to pass back into your title when a friend taps
> **Join**.

## Relevant addon surfaces

- [`GDK.multiplayer_activity`](../../addons/godot_gdk/doc_classes/GDKMultiplayerActivity.xml)
  — `set_activity_async`, `delete_activity_async`,
  `send_invites_async`, `show_invite_ui_async`,
  `get_activities_async`, `get_cached_activity`,
  `accept_pending_invite`, signals
  `pending_invite_received(invite: Dictionary)`,
  `invite_accepted(invite: Dictionary)`, and
  `activities_updated(xuids: PackedStringArray)`.
- [`GDKMultiplayerActivityInfo`](../../addons/godot_gdk/doc_classes/GDKMultiplayerActivityInfo.xml)
  — read-only snapshot returned by `get_cached_activity(xuid)`.
- [`GDK.presence`](../../addons/godot_gdk/doc_classes/GDKPresence.xml)
  — `track_presence`, `stop_tracking_presence`,
  `get_presence_async`, `get_cached_presence`, signals
  `device_presence_changed(xuid: String)` and
  `title_presence_changed(xuid: String, title_id: int)`. Used in
  Step 7 to show friends' device + title state next to their
  joinable badges.
- [`GDKPresenceRecord`](../../addons/godot_gdk/doc_classes/GDKPresenceRecord.xml)
  — typed snapshot returned by `get_cached_presence(xuid)`.
- [`PlayFab.multiplayer`](../../addons/godot_playfab/doc_classes/PlayFabMultiplayer.xml) — `join_lobby_async`
  is the receiving end of the invite flow.
- [`GDK.privacy`](../../addons/godot_gdk/doc_classes/GDKPrivacy.xml)
  — `batch_check_permission_async`. Step 5 uses it with the
  `play_multiplayer` token to filter a list of friend XUIDs down
  to the ones the local user is allowed to invite (block lists,
  privacy settings, parental controls). Pair it with the
  Multiplayer-privilege gate from
  [T5 Step 2](05-multiplayer-lobby.md#step-2--gate-hostjoin-on-the-multiplayer-privilege)
  — the privilege is the *"can I multiplayer at all"* check, the
  permission is the *"can I multiplayer with **that** person"*
  check.
- [`GDK.social`](../../addons/godot_gdk/doc_classes/GDKSocial.xml)
  — `start_social_graph`, `stop_social_graph`,
  `get_friends_async`, `get_group_users`, `destroy_social_group`.
  Step 5 wraps these in a `Lobby.get_friends_async()` helper that
  feeds the targeted invite UI with a real friends list instead of
  hand-typed XUIDs.
- [`GDKSocialGroup`](../../addons/godot_gdk/doc_classes/GDKSocialGroup.xml),
  [`GDKSocialUser`](../../addons/godot_gdk/doc_classes/GDKSocialUser.xml)
  — typed snapshots returned by the social-graph helpers; each
  user exposes `xuid`, `gamertag`, `display_name`, and
  `presence`.

## Step 1 — Wire the activity signals at startup

Add the three signal connections to the `Lobby` autoload's
`_ready` (alongside the existing `Auth.sign_in()` await). The goal
is to install handlers **before** the GDK runtime can fire a
deferred `invite_accepted` from a launch-with-invite activation —
that activation can race `_ready` if the title is launched cold
straight from a friend's invite.

The PlayFab Multiplayer side stays inside the
`_ensure_initialized()` helper you added in
[T5 Step 1](05-multiplayer-lobby.md#step-1--bring-up-the-lobby-autoload)
so the SDK only spins up when the user actually hosts / joins. The
GDK side wires up unconditionally in `_ready` — `GDK.multiplayer_activity`
is part of the core GDK runtime that's already initialized by the
bootstrap.

```gdscript
func _ready() -> void:
    if not await Auth.sign_in():
        return

    GDK.multiplayer_activity.pending_invite_received.connect(_on_pending_invite_received)
    GDK.multiplayer_activity.invite_accepted.connect(_on_invite_accepted)
    GDK.multiplayer_activity.activities_updated.connect(_on_activities_updated)

    print("[Lobby] autoload ready (Lobby + MPA wired; PlayFab Multiplayer init is lazy)")
```

The three GDK signals are stateless — connecting after a missed
`invite_accepted` simply means you miss that one. The launch-with-
invite path is handled by the activation service queueing the
event until the first signal listener attaches, so connecting in
`_ready` is the right place even when the title is cold-launched
from a join.

## Step 2 — Set the activity when you become a session

Add a helper that publishes the current lobby state to MPA, then
call it at the end of T5's `host_lobby` and `join_lobby`. Pull
the values straight off the live `PlayFabLobby`:

```gdscript
const MPA_JOIN_RESTRICTION_FOLLOWED := "followed"
const MPA_JOIN_RESTRICTION_PUBLIC := "public"
const MPA_JOIN_RESTRICTION_INVITE_ONLY := "invite_only"

func _publish_activity(allow_cross_platform_join: bool = false) -> void:
    if _lobby == null or Auth.xbox_user == null:
        return

    var current_players: int = _lobby.member_count
    var max_players: int = _lobby.max_member_count
    var connection_string: String = _lobby.connection_string

    var result: GDKResult = await GDK.multiplayer_activity.set_activity_async(
        Auth.xbox_user,
        connection_string,
        MPA_JOIN_RESTRICTION_FOLLOWED,
        max_players,
        current_players,
        "",
        allow_cross_platform_join)
    if not result.ok:
        push_warning("[MPA] set_activity failed: %s (%s)" % [result.message, result.code])
        return

    print("[MPA] Activity advertised: max=%d current=%d cross_platform=%s" % [
        max_players, current_players, str(allow_cross_platform_join)])
```

Then add this single line to the end of `host_lobby` (just after
the existing `print` lines that confirm the lobby was created):

```gdscript
    await _publish_activity()
```

And the same line at the end of `join_lobby` so the joining
client also broadcasts that they are now in this session:

```gdscript
    await _publish_activity()
```

A few notes:

- The `join_restriction` parameter takes one of three string
  constants. `"followed"` is the most common — only people who
  follow the local user can join. `"public"` allows anyone (use
  for matchmaking lobbies). `"invite_only"` hides the session
  from "join from friend's profile" but still allows explicit
  invites.
- `max_players` and `current_players` are integers, not strings.
  Pass `0` for either to mean "don't advertise that field" — for
  most titles you want to advertise both so the Game Bar can show
  "2 / 4".
- `group_id` is for grouping friends who are about to play
  together (e.g. a pre-game party). Most titles pass `""`.
- `allow_cross_platform_join` controls whether the activity is
  visible to non-Xbox platforms (PlayFab Multiplayer's
  cross-platform discovery). Defaults to `false` because most
  titles want explicit opt-in.

## Step 3 — Refresh the activity when the member count changes

The `current_players` field on MPA is a snapshot, not a live
count — if it drifts from reality, the Game Bar will show stale
data ("3 / 4" when really 2 are in the session). Republish from
inside the existing `_on_lobby_state_changed` handler from T5
whenever a member is added or removed:

```gdscript
func _on_lobby_state_changed(change: PlayFabLobbyStateChange) -> void:
    match change.kind:
        PlayFabLobby.MEMBER_ADDED:
            var m: PlayFabLobbyMember = change.member
            print("[Lobby] member added: %s (local=%s)" % [m.user_id, str(m.is_local)])
            await _publish_activity()
        PlayFabLobby.MEMBER_REMOVED:
            var m: PlayFabLobbyMember = change.member
            print("[Lobby] member removed: %s" % m.user_id)
            await _publish_activity()
        PlayFabLobby.MEMBER_UPDATED:
            var m: PlayFabLobbyMember = change.member
            print("[Lobby] member updated: %s" % m.user_id)
        PlayFabLobby.PROPERTIES_UPDATED:
            print("[Lobby] lobby properties: %s" % str(change.properties))
        PlayFabLobby.OWNER_CHANGED:
            print("[Lobby] owner changed: %s" % str(change.lobby.owner_entity_key))
        PlayFabLobby.DISCONNECTED:
            push_warning("[Lobby] disconnected from lobby")
            await _clear_activity()
```

`_clear_activity` is the cleanup path — call it when you leave
or get disconnected, so the user's "Currently playing" card
doesn't keep showing a dead session:

```gdscript
func _clear_activity() -> void:
    if Auth.xbox_user == null:
        return
    var result: GDKResult = await GDK.multiplayer_activity.delete_activity_async(Auth.xbox_user)
    if result.ok:
        print("[MPA] Activity cleared")
    else:
        push_warning("[MPA] delete_activity failed: %s" % result.message)
```

Update T5's `leave_lobby` to clear the activity before tearing
down the lobby reference. Replace the existing `leave_lobby` body
with:

```gdscript
func leave_lobby() -> void:
    if _lobby == null:
        return
    await _clear_activity()
    var pf: PlayFabResult = await _lobby.leave_async()
    if pf.ok:
        print("[Lobby] left lobby")
    else:
        push_warning("[Lobby] leave failed: %s" % pf.message)
    _lobby = null
```

Order matters here: clear the activity **before** the lobby
disconnect so the Game Bar surface goes dark immediately. If you
clear afterwards the friend's UI shows "in session" for a few
seconds while the leave round-trips.

## Step 4 — Accept an invite that arrives mid-game

When a friend taps **Join** on your activity, the Xbox shell
queues an activation event. The addon parses the activation URI
into a `Dictionary` and fires `invite_accepted`. The handler
should pull the `connectionstring` out of that dict and decide
whether the user can be auto-joined right now or whether the UI
should prompt them first:

```gdscript
func _on_invite_accepted(invite: Dictionary) -> void:
    print("[MPA] Invite accepted from Game Bar: scheme=%s action=%s" % [
        invite.get("scheme", ""), invite.get("action", "")])

    var connection_string: String = invite.get("connectionstring", "")
    if connection_string.is_empty():
        push_warning("[MPA] Invite did not carry a connection string: %s" % invite.get("raw_uri", ""))
        return

    # The Lobby autoload's state machine (T5 Step 1) rejects re-entrant
    # host/join/leave calls. Drop the invite if we're mid-flight rather
    # than racing the existing operation — the user can re-tap once it
    # settles.
    if is_busy():
        push_warning("[MPA] Invite ignored — lobby autoload is busy (state=%d)" % _state)
        return

    # Not in a lobby — nothing to leave. Direct-join.
    if _state != State.IN_LOBBY:
        await join_lobby(connection_string)
        return

    # IN_LOBBY — stash the invite and ask the UI to confirm.
    # confirm_pending_invite / reject_pending_invite drive the
    # leave+join from here. (See "Confirming the destructive accept"
    # below.)
    if _pending_invite_confirming:
        push_warning("[MPA] Invite arrived while confirming another — dropping new invite")
        return
    _pending_invite_id += 1
    _pending_invite_cs = connection_string
    invite_pending_confirmation.emit(_pending_invite_id, connection_string)
```

A few notes:

- The dictionary keys are **lowercased** (the addon normalizes them
  for stable indexing across activations). The Xbox URI uses
  `connectionString` (camel case); the parsed dict exposes it as
  `"connectionstring"`. Use `invite.get("raw_uri", "")` for the
  full URI when you need to log unparsed cases.
- The `is_busy()` guard avoids a known race: tapping Join in the
  Game Bar while `host_lobby` is mid-flight would otherwise leak a
  half-created lobby. Dropping the invite is the conservative
  choice — the friend can re-tap, or the local user can retry
  once UI shows them as `READY`.
- `pending_invite_received` fires on the receiver side **before**
  the user actually accepts (the Game Bar is showing a toast).
  Most titles only need `invite_accepted`. Connect
  `pending_invite_received` if you want to render an in-game
  toast yourself instead of letting the system UI handle it:

  ```gdscript
  func _on_pending_invite_received(invite: Dictionary) -> void:
      print("[MPA] Pending invite (not yet accepted): %s" % invite.get("raw_uri", ""))
  ```

### Confirming the destructive accept

Auto-leaving the user's current lobby to honor an invite is a
**destructive UX**. Users who already started a match with one
friend don't expect a Game Bar tap from a different friend to
silently bump them. The pattern: only auto-join when there's
nothing to leave. When `IN_LOBBY`, fire a signal that the UI
binds to a confirmation dialog, and gate the actual leave+join
behind the dialog's Accept callback.

The autoload owns the pending-invite slot (so it survives scene
switches and gets cleared on disconnect / manual leave); the UI
owns the dialog rendering. An `invite_id` token guards against
stale dialogs: if a second invite arrives while the first is
still on-screen, the autoload increments the token and re-emits;
a tap on the stale dialog with the old token is a no-op.

```gdscript
signal invite_pending_confirmation(invite_id: int, connection_string: String)
signal invite_pending_cleared(invite_id: int)

var _pending_invite_id: int = 0
var _pending_invite_cs: String = ""
var _pending_invite_confirming: bool = false

func confirm_pending_invite(invite_id: int) -> bool:
    if _pending_invite_cs.is_empty() or invite_id != _pending_invite_id:
        return false                           # stale token
    if _pending_invite_confirming:
        return false                           # double-tap
    _pending_invite_confirming = true
    var cs := _pending_invite_cs
    var id := _pending_invite_id
    _pending_invite_cs = ""                    # snapshot-then-clear

    if _state == State.IN_LOBBY:
        if not await leave_lobby():
            _pending_invite_confirming = false
            invite_pending_cleared.emit(id)
            return false

    var ok := await join_lobby(cs)
    _pending_invite_confirming = false
    invite_pending_cleared.emit(id)
    return ok

func reject_pending_invite(invite_id: int) -> void:
    if invite_id != _pending_invite_id:
        return
    _clear_pending_invite()

func _clear_pending_invite() -> void:
    if _pending_invite_cs.is_empty():
        return
    var id := _pending_invite_id
    _pending_invite_cs = ""
    invite_pending_cleared.emit(id)
```

Wire `_clear_pending_invite()` into your state transitions so a
stale invite can't outlive the lobby it would tear down:

```gdscript
func _set_state(next: State) -> void:
    if _state == next:
        return
    var was_in_lobby := _state == State.IN_LOBBY
    _state = next
    state_changed.emit(_state)
    # Manual leave, disconnect, etc. — drop the pending invite so a
    # user who taps Accept after the underlying lobby is gone doesn't
    # join the invited lobby out of context.
    if was_in_lobby and _state != State.IN_LOBBY:
        _clear_pending_invite()
```

The UI side is a `ConfirmationDialog` in the scene, bound to the
autoload via the pair of signals:

```gdscript
var _dialog_invite_id: int = 0

func _ready() -> void:
    # ...other wiring...
    Lobby.invite_pending_confirmation.connect(_on_invite_pending_confirmation)
    Lobby.invite_pending_cleared.connect(_on_invite_pending_cleared)
    $InviteDialog.confirmed.connect(_on_invite_dialog_confirmed)
    $InviteDialog.canceled.connect(_on_invite_dialog_canceled)

func _on_invite_pending_confirmation(invite_id: int, _cs: String) -> void:
    _dialog_invite_id = invite_id
    $InviteDialog.popup_centered()

func _on_invite_pending_cleared(invite_id: int) -> void:
    if invite_id == _dialog_invite_id and $InviteDialog.visible:
        $InviteDialog.hide()

func _on_invite_dialog_confirmed() -> void:
    await Lobby.confirm_pending_invite(_dialog_invite_id)

func _on_invite_dialog_canceled() -> void:
    Lobby.reject_pending_invite(_dialog_invite_id)
```

The dialog text is intentionally generic
("You're already in a lobby. Leave it and join the invited lobby?")
— don't render the raw connection string, it's not human-readable
and it changes between invites for the same lobby.

## Step 5 — Send an invite from inside the game

Two flavors:

- **Targeted invite** — you know the friend's XUID and want to
  push the invite without surfacing UI. Use
  `Lobby.get_friends_async()` (Step 5a) to pull a real friends
  list from the Xbox Social Manager instead of asking the player
  to paste a XUID by hand.
- **Picker invite** — surface the system **People Picker** so the
  user chooses who to invite.

> **Cert callout — filter invitable XUIDs before sending.** When
> you build the targeted-invite list yourself (no system picker),
> certification expects you to drop XUIDs the local user is not
> permitted to play multiplayer with. The friends list from
> `Lobby.get_friends_async()` is unfiltered — it includes friends
> who have you blocked, friends whose privacy setting denies
> multiplayer, and friends a parental control restriction hides
> from your account. Pre-filter through
> `GDK.privacy.batch_check_permission_async(user, "play_multiplayer", xuids)`
> and keep only the entries with `entry.allowed == true`:
>
> ```gdscript
> func _filter_invitable(xuids: PackedStringArray) -> PackedStringArray:
>     if xuids.is_empty():
>         return PackedStringArray()
>     var pf: GDKResult = await GDK.privacy.batch_check_permission_async(
>             Auth.xbox_user, "play_multiplayer", xuids)
>     if not pf.ok:
>         push_warning("[MPA] permission batch failed: %s" % pf.message)
>         return PackedStringArray()
>     var allowed := PackedStringArray()
>     for entry: Dictionary in pf.data:
>         if bool(entry.get("allowed", false)):
>             allowed.append(String(entry.get("target_xuid", "")))
>     return allowed
> ```
>
> The `show_invite_ui_async` picker handles this filtering for
> you internally — only the targeted `send_invites_async` path
> needs the explicit guard. Apply this same filter to the
> friend-presence panel from Step 7 so denied XUIDs do not get
> a "Joinable" badge they cannot act on.

### Step 5a — Pull a real friends list from Social Manager

The friends list lives in the **Xbox Social Manager**, which
tracks the local user's social graph in the background and exposes
it as a snapshot of `GDKSocialUser` records (XUID + gamertag +
presence). Bring it up lazily on first call, then reuse the cached
group on subsequent refreshes. Add this helper to the `Lobby`
autoload:

```gdscript
var _social_graph_started: bool = false
var _friends_group: GDKSocialGroup = null

func get_friends_async() -> Array:
    var user: GDKUser = Auth.xbox_user
    if user == null:
        return []
    if not _social_graph_started:
        var sg: GDKResult = GDK.social.start_social_graph(user)
        if not sg.ok:
            push_warning("[Lobby] start_social_graph failed: %s" % sg.message)
            return []
        _social_graph_started = true
    if _friends_group == null:
        var f: GDKResult = await GDK.social.get_friends_async(user)
        if not f.ok:
            push_warning("[Lobby] get_friends failed: %s" % f.message)
            return []
        _friends_group = f.data
    var users: GDKResult = GDK.social.get_group_users(_friends_group)
    if not users.ok:
        push_warning("[Lobby] get_group_users failed: %s" % users.message)
        return []
    return users.data

func _exit_tree() -> void:
    if Engine.has_singleton("GDK"):
        if _friends_group != null:
            GDK.social.destroy_social_group(_friends_group)
            _friends_group = null
        if _social_graph_started:
            GDK.social.stop_social_graph(Auth.xbox_user)
            _social_graph_started = false
```

The UI side feeds these into an `ItemList` and binds each row's
metadata to the XUID, so the invite button just walks the
selection:

```gdscript
func _on_refresh_pressed() -> void:
    _friends_list.clear()
    var friends: Array = await Lobby.get_friends_async()
    if friends.is_empty():
        _friends_list.add_item("(no friends found)")
        return
    for friend: GDKSocialUser in friends:
        var label := "%s  —  %s" % [friend.gamertag, friend.xuid]
        var idx := _friends_list.add_item(label)
        _friends_list.set_item_metadata(idx, friend.xuid)

func _selected_xuids() -> PackedStringArray:
    var out := PackedStringArray()
    for idx in _friends_list.get_selected_items():
        var meta = _friends_list.get_item_metadata(idx)
        if typeof(meta) == TYPE_STRING and not (meta as String).is_empty():
            out.append(meta)
    return out
```

Notes:

- `start_social_graph(user)` is required **once per local user**
  before any `get_friends_async` or `create_social_group` call.
  Idempotent guards (`_social_graph_started`) keep repeated UI
  refreshes from re-starting the graph.
- `get_friends_async` returns a `GDKSocialGroup`; cache it. The
  group keeps tracking friends in the background and the Social
  Manager fires
  [`social_graph_changed(user)`](../../addons/godot_gdk/doc_classes/GDKSocial.xml)
  when entries appear/disappear. Connect to that signal and
  re-call `get_group_users(group)` for live updates instead of
  re-running `get_friends_async`.
- Always destroy the group in `_exit_tree`. Leaving it alive after
  the autoload is gone keeps the Social Manager doing background
  work for nothing.
- For testing the targeted invite path without two real Xbox
  accounts, fall back to the system picker
  (`show_invite_ui_async`) which uses the shell's own friends
  view and works against test sandbox accounts immediately.

### Step 5b — Send the invite

Both call into `GDK.multiplayer_activity`:

```gdscript
func invite_friend(xuid: String) -> bool:
    if _lobby == null:
        push_warning("[MPA] Cannot invite — not in a lobby")
        return false

    var xuids: PackedStringArray = [xuid]
    var result: GDKResult = await GDK.multiplayer_activity.send_invites_async(
        Auth.xbox_user,
        xuids,
        false,
        _lobby.connection_string)
    if result.ok:
        print("[MPA] Sent invite to %s" % xuid)
        return true
    push_warning("[MPA] send_invites failed: %s (%s)" % [result.message, result.code])
    return false

func open_invite_picker() -> void:
    if _lobby == null:
        push_warning("[MPA] Cannot open picker — not in a lobby")
        return

    var result: GDKResult = await GDK.multiplayer_activity.show_invite_ui_async(Auth.xbox_user)
    if not result.ok:
        push_warning("[MPA] show_invite_ui failed: %s" % result.message)
```

Notes:

- `send_invites_async`'s `connection_string` parameter overrides
  what is already in the activity record. If you pass `""` the
  recipient gets the activity's current connection string, which
  is usually what you want. The explicit parameter exists for
  flows where the inviter is advertising a different "lobby" than
  the one they want the invitee to join (rare; mostly relevant for
  staged invite flows where the inviter has not joined yet).
- `show_invite_ui_async` is a thin wrapper over the system Xbox
  People Picker. The UI surfaces the user's friends list with
  multi-select; the actual invites go out automatically once the
  user confirms. The awaited result fires when the picker closes,
  not when the invites are accepted.

## Step 6 — Light up "joinable" badges for friends

`activities_updated` fires when an MPA record for one of the
**friends you have already queried** changes. That is the live
"is this friend joinable right now?" signal — perfect for an
in-game friend list that wants to render a "Join" button when a
friend's activity goes from empty to populated:

```gdscript
var _watched_xuids: PackedStringArray = PackedStringArray()

func track_friend_activities(xuids: PackedStringArray) -> void:
    _watched_xuids = xuids
    var result: GDKResult = await GDK.multiplayer_activity.get_activities_async(
        Auth.xbox_user, xuids)
    if not result.ok:
        push_warning("[MPA] get_activities failed: %s" % result.message)
        return

    for xuid in xuids:
        _print_activity(xuid)

func _on_activities_updated(xuids: PackedStringArray) -> void:
    print("[MPA] Activity updated for friends: %s" % str(xuids))
    for xuid in xuids:
        _print_activity(xuid)

func _print_activity(xuid: String) -> void:
    var info: GDKMultiplayerActivityInfo = GDK.multiplayer_activity.get_cached_activity(xuid)
    if info == null:
        print("[MPA] Friend %s is offline / not in a session" % xuid)
        return
    var conn: String = info.get_connection_string()
    if conn.is_empty():
        print("[MPA] Friend %s cleared their session" % xuid)
        return
    print("[MPA] Friend %s is in session: %s" % [xuid, conn])
```

The cache is owned by the addon — you read from
`get_cached_activity(xuid)` to fetch the current snapshot, and
re-query with `get_activities_async(...)` when you want to refresh
a stale entry. Most titles call `track_friend_activities` once
when the friends panel opens, then live-update through
`activities_updated` for the rest of the session.

## Step 7 — Show friend presence next to the joinable badge

`activities_updated` tells you when a friend's **session
advertisement** changes, but plenty of titles also want to render
the friend's online state itself — "Online (Console)", "Away",
"Offline" — so the friends panel reads correctly even when no
session is being advertised. That is what `GDK.presence` covers.

Extend `track_friend_activities` from Step 6 to also subscribe to
presence updates for the same XUIDs:

```gdscript
func track_friend_activities(xuids: PackedStringArray) -> void:
    _watched_xuids = xuids

    var activities: GDKResult = await GDK.multiplayer_activity.get_activities_async(
        Auth.xbox_user, xuids)
    if not activities.ok:
        push_warning("[MPA] get_activities failed: %s" % activities.message)

    GDK.presence.track_presence(Auth.xbox_user, xuids)

    var presence: GDKResult = await GDK.presence.get_presence_async(xuids)
    if not presence.ok:
        push_warning("[Pres] get_presence failed: %s" % presence.message)

    for xuid in xuids:
        _print_activity(xuid)
        _print_presence(xuid)
```

Then add the two handlers — they fire **without** a payload that
carries the new state, so the recommended pattern is to re-query
the changed XUID and render from the cache:

```gdscript
func _ready() -> void:
    # alongside the activity signal hookups from Step 1:
    GDK.presence.device_presence_changed.connect(_on_device_presence_changed)
    GDK.presence.title_presence_changed.connect(_on_title_presence_changed)
    GDK.presence.presence_changed.connect(_on_presence_changed)

func _on_device_presence_changed(xuid: String) -> void:
    if xuid in _watched_xuids:
        await GDK.presence.get_presence_async([xuid])

func _on_title_presence_changed(xuid: String, _title_id: int) -> void:
    if xuid in _watched_xuids:
        await GDK.presence.get_presence_async([xuid])

func _on_presence_changed(xuid: String, _record) -> void:
    _print_presence(xuid)

func _print_presence(xuid: String) -> void:
    var record: GDKPresenceRecord = GDK.presence.get_cached_presence(xuid)
    if record == null:
        print("[Pres] %s: (unknown)" % xuid)
        return
    var title_records: Array = record.get_title_records()
    var rich: String = ""
    if not title_records.is_empty():
        var first: Dictionary = title_records[0]
        rich = first.get("rich_presence_string", "")
    print("[Pres] %s: state=%s rich=%s" % [
        xuid, record.get_user_state_name(), rich])

func stop_tracking_friends() -> void:
    GDK.presence.stop_tracking_presence(Auth.xbox_user, _watched_xuids)
    _watched_xuids = PackedStringArray()
```

Notes:

- `device_presence_changed` / `title_presence_changed` are
  **notifications only**; the payload is just the XUID (plus
  `title_id` for the title variant). The addon does not call
  `get_presence_async` on your behalf — you re-query to refresh
  the cache. Once `get_presence_async` resolves the addon fires
  `presence_changed(xuid, record)`, which is the right spot to
  drive your UI.
- `track_presence` registers the XSAPI handlers on the title side
  but does not seed the cache; pair it with an initial
  `get_presence_async(...)` call (as above) before reading.
- `stop_tracking_presence` is the matching teardown. Call it when
  the friends panel closes so XSAPI is not holding subscriptions
  for XUIDs your UI no longer cares about.
- Per-friend `get_user_state_name()` returns
  `"Unknown" / "Online" / "Away" / "Offline"`; the
  `get_title_records()` array carries one dictionary per title
  the friend is currently signed in to, each with a
  `rich_presence_string` field — the localized string the
  friend's title is currently publishing (the same string T5
  Step 7 wrote with `set_presence_async`).

> **MPA invite-list filter (forward-pointer).** When you reach
> the cert-readiness work in T8, this same `_watched_xuids` list
> is also what you filter through
> `GDK.privacy.batch_check_permission_async("play_multiplayer", xuids)`
> before calling `send_invites_async`. That keeps the system
> invite UI from offering "Invite to game" against friends the
> local user is not permitted to multiplayer with (parental
> controls, restricted accounts). Wired in the privileges /
> permissions step.

## Verify

A successful host-side first run prints, in order:

```
[Lobby] Multiplayer initialized (Lobby + MPA wired)
[Lobby] Lobby created: id=BBA9... max=4
[Lobby] connection string ready — copy to second client
[MPA] Activity advertised: max=4 current=1 cross_platform=false
[MPA] Sent invite to 2814635463725476
```

When the invited friend accepts from the Game Bar on the client
side:

```
[MPA] Invite accepted from Game Bar: scheme=ms-xbl-multiplayer action=inviteHandleAccept
[Lobby] Joined lobby id=BBA9... with 2 member(s)
[MPA] Activity advertised: max=4 current=2 cross_platform=false
```

And on the host side, the `MEMBER_ADDED` from T5 plus the
republished activity:

```
[Lobby] member added: title_player_account:6F4B... (local=false)
[MPA] Activity advertised: max=4 current=2 cross_platform=false
```

## Common failures

| Output | Diagnosis | Fix |
|---|---|---|
| `set_activity failed: invalid_connection_string` | `_lobby.connection_string` was empty when `_publish_activity` was called. | Make sure you call `_publish_activity` **after** `create_lobby_async` / `join_lobby_async` resolves — the connection string is populated as part of that completion. |
| `set_activity failed: auth_invalid_scid` | The PC's sandbox SCID does not match Partner Center. | See [Troubleshooting → SCID mismatch](../troubleshooting.md). |
| `invite_accepted` never fires when a friend taps Join | The PC and the friend are in different sandboxes, or the friend's title's SCID does not match yours. | Both sides must be in the same sandbox, with the same SCID published. |
| `invite_accepted` fires with `invite.get("connectionstring", "") == ""` | The invite was issued against a non-PlayFab activity (e.g. a stale Xbox-only MPSD session from an earlier build), or you passed an empty `connection_string` to `send_invites_async`. | Always advertise a real lobby connection string in `set_activity_async`, and let `send_invites_async` default to it. |
| `send_invites failed: invalid_xuids` | One or more entries in the `xuids` PackedStringArray was not a valid base-10 XUID. | XUIDs are decimal strings (e.g. `"2814635463725476"`) — not gamertags. Pull them from `Lobby.get_friends_async()` (Step 5a) — each `GDKSocialUser.xuid` is in the correct format. |
| `delete_activity failed: not_found` | No activity was ever set, or the runtime restarted between set and delete. | Safe to ignore — the user is already not advertising. |

## Reference implementation

The cumulative end-state lives in
[`sample/tutorial_app/`](../../sample/tutorial_app/README.md):

- Scene: [`sample/tutorial_app/t06_mpa.tscn`](../../sample/tutorial_app/t06_mpa.tscn)
- Script: [`sample/tutorial_app/t06_mpa.gd`](../../sample/tutorial_app/t06_mpa.gd)
- Extends the `Lobby` autoload introduced in T5
  ([`sample/tutorial_app/autoload/lobby.gd`](../../sample/tutorial_app/autoload/lobby.gd))
  with the MPA `set_activity_async` / `delete_activity_async`
  wiring + the `invite_accepted` listener.

## Next

- [**Tutorial 7 — Stand up a PlayFab Party network**](07-playfab-party.md)
- Reference:
  [`GDKMultiplayerActivity`](../../addons/godot_gdk/doc_classes/GDKMultiplayerActivity.xml),
  [`GDKMultiplayerActivityInfo`](../../addons/godot_gdk/doc_classes/GDKMultiplayerActivityInfo.xml),
  [`PlayFabLobby`](../../addons/godot_playfab/doc_classes/PlayFabLobby.xml),
  [`PlayFabMultiplayer`](../../addons/godot_playfab/doc_classes/PlayFabMultiplayer.xml)
