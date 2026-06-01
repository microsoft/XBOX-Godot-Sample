# Tutorial 5 — Create and join a lobby

## What you'll build

The bare-minimum host/join flow built on PlayFab Multiplayer
lobbies. By the end you will:

- Initialize PlayFab Multiplayer and listen for state-change events.
- **Host:** create a public lobby with a max member count and seed
  it with a few search properties.
- **Client:** join that same lobby using its `connection_string`.
- Update member properties and lobby properties from both sides,
  observing the corresponding `state_changed` events on the
  opposite peer.
- Handle the disconnect and member-removal events that fire when
  someone leaves.

Sample output (host side):

```
[Lobby] Multiplayer initialized
[Lobby] Lobby created: id=BBA9... max=4
[Lobby] connection string ready — copy to second client
[Lobby] member added: title_player_account:6F4B... (local=false)
[Lobby] member updated: title_player_account:6F4B...
[Lobby] member removed: title_player_account:6F4B...
```

## Prerequisites

- [Tutorial 1 — Sign in a user](01-sign-in-user.md) is complete.
- The title-side Lobby configuration is in place: PlayFab
  Multiplayer → Lobby is enabled in Game Manager. Recently created
  titles enable this by default; older titles require the feature to
  be enabled manually. See
  [PlayFab title prerequisites — §2 Lobby](../playfab/prerequisites.md#lobby-t5-t6-t7-t8).
- Two Godot processes (one host, one client). The simplest way to
  test on one PC is to run the host scene in the editor and a
  separate exported build for the client, each signed into a
  different XBOX test account. Two editors with two different
  PlayFab sessions also works as long as both PCs are in the same
  sandbox.
- `Auth.playfab_user` resolves to an XBOX-backed PlayFab session
  for the snippets below. Custom-ID sessions also work for lobby
  but invites from the XBOX shell will not.

> **Lobby vs. Matchmaking.** Lobbies are persistent rooms identified
> by a lobby id and a connection string. Matchmaking
> (`create_match_ticket_async`) returns a `PlayFabMatchTicket`;
> when the ticket completes, its
> `arranged_lobby_connection_string` is what you hand to
> `join_arranged_lobby_async`. This tutorial covers explicit lobby
> create / join; the matchmaking pipeline is a future tutorial.

## Relevant addon surfaces

- [`PlayFab.multiplayer`](../../addons/godot_playfab/doc_classes/PlayFabMultiplayer.xml) —
  `initialize_async`, `create_lobby_async`, `join_lobby_async`,
  `find_lobbies_async`, signals `state_changed`,
  `invite_received`, `multiplayer_error`.
- [`PlayFabLobbyConfig`](../../addons/godot_playfab/doc_classes/PlayFabLobbyConfig.xml) — the typed config
  for `create_lobby_async`. Constants `ACCESS_POLICY_*`,
  `OWNER_MIGRATION_*` live here.
- [`PlayFabLobbyJoinConfig`](../../addons/godot_playfab/doc_classes/PlayFabLobbyJoinConfig.xml) — the typed
  config for `join_lobby_async`.
- [`PlayFabLobby`](../../addons/godot_playfab/doc_classes/PlayFabLobby.xml) — the typed result that
  carries the live lobby. Read `lobby_id`, `connection_string`,
  `members`, `max_member_count`, `member_count`; call
  `set_member_properties_async`, `set_properties_async`,
  `leave_async`, `is_owner`. Constants `MEMBER_ADDED`,
  `MEMBER_REMOVED`, `MEMBER_UPDATED`, `PROPERTIES_UPDATED`,
  `OWNER_CHANGED`, `DISCONNECTED` live here.
- [`GDK.presence`](../../addons/godot_gdk/doc_classes/GDKPresence.xml)
  — `set_presence_async`, `clear_presence_async`, signal
  `local_presence_set`. Used in Step 8 to advertise the local
  user's lobby state to XBOX friends. The `state` parameter is a
  **rich-presence ID** registered on the title in Partner Center,
  not a freeform string; if your title has no presence IDs
  configured, skip Step 8 — the rest of the lobby flow still works.
- [`GDK.users`](../../addons/godot_gdk/doc_classes/GDKUsers.xml)
  — `check_privilege_async`, `resolve_privilege_with_ui_async`.
  Step 2 gates host / join on the **Multiplayer** privilege so
  parental controls and restricted accounts cannot blow past the
  cert bar for invitable sessions.
- [`GDK.privacy`](../../addons/godot_gdk/doc_classes/GDKPrivacy.xml)
  — `batch_check_permission_async`. Used in Step 2 alongside the
  privilege check to confirm the local user is allowed to play
  multiplayer with **the specific other party** (block lists,
  privacy settings, cross-network restrictions).
- One-page primer on the addons' async model:
  [Async patterns](../async-patterns.md).

> **Privileges vs. permissions vs. `XblPrivilege`.** Three terms
> show up here and they are *not* the same thing:
>
> - A **privilege** (XGameRuntime `XUserPrivilege` enum) is what
>   the local user is allowed to do at all — multiplayer,
>   voice chat, add friends. `check_privilege_async` answers
>   *"can this account use multiplayer?"*. The integer values
>   live in `<XUser.h>`; the tutorials below pin the ones we use
>   as named local constants so you do not paste raw integers.
> - A **permission** (XSAPI string token like `play_multiplayer`)
>   answers *"can this account multiplayer **with that other
>   account**?"*. Block lists, mute lists, privacy settings, and
>   cross-network friend rules layer on top of the privilege.
> - `XblPrivilege` (an XSAPI enum, distinct from `XUserPrivilege`)
>   shows up *inside* permission deny-reason details as
>   `restricted_privilege` — it is the XSAPI side's view of which
>   privilege blocked a target-user check. Don't confuse it with
>   the local privilege check; you almost never read it directly.

## Step 1 — Bring up the Lobby autoload

PlayFab Multiplayer is a separate runtime that sits on top of the
main PlayFab runtime — it needs its own `initialize_async` before
any lobby method works. Bring it up **lazily** the first time the
user actually hosts or joins, rather than eagerly in `_ready`. The
init has cost (allocates queues, opens a hub connection) and most
of your scenes don't need lobbies. A small `_ensure_initialized()`
helper keeps the call sites readable.

Pair the lazy init with a tracked `State` enum so panels and
sibling autoloads can drive UI from a single firehose
(`state_changed`) instead of holding their own bookkeeping. The
[Auth autoload](01-sign-in-user.md#step-4--track-the-sign-in-state-machine)
uses the same pattern; matching it across the autoloads keeps the
codebase consistent.

> **Note — single-slot design.** The autoload here owns exactly
> one `PlayFabLobby` at a time and the host/join methods reject
> re-entrant calls. That's a deliberate trade for the tutorial:
> panels render one lobby's state without juggling lobby IDs.
> The PlayFab addon itself supports multiple live lobbies per
> process, so if your shipping game needs concurrent lobbies
> (e.g. a persistent clan/social lobby alongside a match lobby),
> refactor `host_lobby` / `join_lobby` to return the new
> `PlayFabLobby` (instead of stashing it on `_lobby`) and have
> the caller hold the reference plus connect to it directly.
> Drop or partition the `_state` bookkeeping at the same time.

```gdscript
extends Node

enum State {
    UNINITIALIZED,    # autoload _ready has not finished sign-in
    READY,            # signed in, no lobby; host/join allowed
    HOSTING,          # host_lobby() in flight
    JOINING,          # join_lobby() in flight
    IN_LOBBY,         # active lobby; leave allowed
    LEAVING,          # leave_lobby() in flight
}

signal state_changed(state: State)
signal lobby_joined(lobby: PlayFabLobby)
signal lobby_left
signal lobby_disconnected   ## involuntary — PlayFab kicked us

var _state: State = State.UNINITIALIZED
var _lobby: PlayFabLobby = null
var _pf_multiplayer_signals_connected: bool = false

func _ready() -> void:
    await _ensure_ready()
    print("[Lobby] autoload ready (PlayFab Multiplayer init is lazy)")

# Guarded accessor — returns null unless we're actually in a lobby.
var current_lobby: PlayFabLobby:
    get:
        return _lobby if _state == State.IN_LOBBY else null

func is_in_lobby() -> bool: return _state == State.IN_LOBBY
func is_busy() -> bool:
    return _state == State.HOSTING or _state == State.JOINING or _state == State.LEAVING

func _set_state(next: State) -> void:
    if _state == next:
        return
    _state = next
    state_changed.emit(_state)

# Awaits sign-in (Auth.sign_in is idempotent for concurrent callers)
# and transitions UNINITIALIZED -> READY. Safe to call from _ready
# and from host_lobby / join_lobby entry points.
func _ensure_ready() -> bool:
    if _state == State.READY or _state == State.IN_LOBBY:
        return true
    if is_busy() or _state != State.UNINITIALIZED:
        return false
    if not await Auth.sign_in():
        return false
    _set_state(State.READY)
    return true

# PlayFab Multiplayer SDK lazy init. The `_pf_multiplayer_signals_connected`
# guard makes the helper safe to call from every entry point that needs
# the SDK up — `host_lobby`, `join_lobby`, `find_lobbies` — without
# duplicating signal connections on repeated calls.
func _ensure_initialized() -> bool:
    if not PlayFab.multiplayer.is_initialized():
        var init: PlayFabResult = await PlayFab.multiplayer.initialize_async()
        if not init.ok:
            push_error("[Lobby] Multiplayer init failed: %s" % init.message)
            return false
        print("[Lobby] Multiplayer initialized lazily")

    if not _pf_multiplayer_signals_connected:
        PlayFab.multiplayer.state_changed.connect(_on_state_changed)
        PlayFab.multiplayer.invite_received.connect(_on_invite_received)
        PlayFab.multiplayer.multiplayer_error.connect(_on_multiplayer_error)
        _pf_multiplayer_signals_connected = true
    return true

func _on_state_changed(change: PlayFabMultiplayerStateChange) -> void:
    pass # Per-lobby changes are routed to _on_lobby_state_changed in Step 5.

func _on_invite_received(invite: PlayFabLobbyInvite) -> void:
    print("[Lobby] invite from %s: %s" % [invite.sender_entity_key, invite.connection_string])

func _on_multiplayer_error(result: PlayFabResult) -> void:
    push_warning("[Lobby] multiplayer error: %s (%s)" % [result.message, result.code])
```

The three PlayFab signals are the **firehose** of every lobby +
matchmaking change. Connect them exactly once on first init; per-
call awaits handle the synchronous part of a create / join, but
member updates, disconnect, and ownership migration all arrive
here.

The state-machine pieces (`State` enum, `_set_state`,
`_ensure_ready`, the guarded `current_lobby` getter) cost a few
extra lines but pay back across the rest of the tutorial:

- `host_lobby` / `join_lobby` / `leave_lobby` return `bool` and
  reject re-entrant calls instead of racing against an in-flight
  PlayFab operation.
- Panels listen on `state_changed` to drive button enable state
  with no local bookkeeping (`HOSTING` / `JOINING` / `LEAVING`
  → disable everything; `IN_LOBBY` → enable Leave; `READY` →
  enable Host / Join).
- Involuntary disconnect (`lobby_disconnected`) is distinguishable
  from voluntary leave (`lobby_left`), so the UI can warn the
  user without conflating the two.

> The `current_lobby` getter guards against accessing a stale
> `PlayFabLobby` after a disconnect. Callers that need the lobby
> outside `IN_LOBBY` (for example to log its id during cleanup)
> should pull it from `lobby_joined` / `lobby_left` payloads
> rather than poking the getter.

## Step 2 — Gate host/join on the Multiplayer privilege

Lobbies are a multiplayer feature; certification (and parental
controls) require that you verify the local user is actually
allowed to use multiplayer before you create or join. The check
is two layers:

1. `GDK.users.check_privilege_async(user, XUSER_PRIVILEGE_MULTIPLAYER)`
   answers *"can this account do multiplayer at all?"* — parental
   controls, restricted accounts, missing subscription. Result
   data carries `has_privilege: bool` and a `deny_reason: String`.
2. When `has_privilege` comes back `false` and the runtime allows
   the user to fix it (sign in to XBOX Live, accept the
   subscription prompt, ask a parent), `resolve_privilege_with_ui_async`
   pops the system UI to walk the user through it. The same
   signal returns the post-resolution `has_privilege`.

Add this helper to your lobby autoload alongside `_ready`:

```gdscript
# XGameRuntime XUserPrivilege values from <XUser.h>. Naming the
# constants locally keeps the call sites readable without paste-
# blocking a raw integer into your code.
const XUSER_PRIVILEGE_MULTIPLAYER := 254

func can_use_multiplayer() -> bool:
    var user: GDKUser = Auth.xbox_user
    if user == null:
        return false

    var pf: GDKResult = await GDK.users.check_privilege_async(
            user, XUSER_PRIVILEGE_MULTIPLAYER)
    if pf.ok and bool(pf.data.get("has_privilege", false)):
        return true

    # If the runtime tells us the user can resolve the block
    # themselves, surface the system UI. The data dictionary keeps
    # the original deny_reason so you can log why we needed to ask.
    print("[Lobby] multiplayer denied (%s) — resolving with UI" % pf.data.get("deny_reason", ""))
    var resolved: GDKResult = await GDK.users.resolve_privilege_with_ui_async(
            user, XUSER_PRIVILEGE_MULTIPLAYER)
    if not resolved.ok:
        push_warning("[Lobby] resolve_privilege_with_ui failed: %s" % resolved.message)
        return false
    return bool(resolved.data.get("has_privilege", false))
```

Then guard `host_lobby` / `join_lobby` with it before doing any
PlayFab work:

```gdscript
func host_lobby() -> void:
    if not await can_use_multiplayer():
        push_warning("[Lobby] host blocked — multiplayer privilege denied")
        return
    # ... existing create_lobby_async flow ...
```

For the **target-user** layer (block lists, mute lists, privacy
settings), `GDK.privacy.batch_check_permission_async` filters a
list of XUIDs down to the ones the local user is actually allowed
to play multiplayer with. This is the function you call **before**
`send_invites_async` in T6, but the same shape is useful here if
you are joining a lobby seeded with a specific friend list:

```gdscript
func filter_invitable(xuids: PackedStringArray) -> PackedStringArray:
    var user: GDKUser = Auth.xbox_user
    if user == null or xuids.is_empty():
        return PackedStringArray()
    var pf: GDKResult = await GDK.privacy.batch_check_permission_async(
            user, "play_multiplayer", xuids)
    if not pf.ok:
        push_warning("[Lobby] permission batch failed: %s" % pf.message)
        return PackedStringArray()
    var allowed := PackedStringArray()
    for entry: Dictionary in pf.data:
        if bool(entry.get("allowed", false)):
            allowed.append(String(entry.get("target_xuid", "")))
    return allowed
```

Notes:

- Permission tokens are **snake_case** strings (`play_multiplayer`,
  `communicate_using_voice`, `view_target_profile`, …). The addon
  rejects PascalCase aliases — if you copy from XSAPI docs that
  use `PlayMultiplayer`, pre-translate the token.
- Each entry in the `batch_check_permission_async` result dict
  carries `allowed`, `target_xuid`, `permission`, and a `reasons`
  array. The `reasons` entries each carry a `reason` string
  (`block_list_restricts_target`, `missing_privilege`, …) and an
  optional `restricted_privilege` — that field is the XSAPI
  `XblPrivilege` mentioned in the surfaces note above, used by
  the cert pass to explain *why* the per-target check failed.

## Step 3 — Host a lobby

Build a `PlayFabLobbyConfig`, hand it to `create_lobby_async`, and
hold on to the returned `PlayFabLobby`. The lobby's
`connection_string` is what your client side will need:

```gdscript
func host_lobby() -> bool:
    if not await _ensure_ready():
        return false
    if _state != State.READY:
        push_warning("[Lobby] host_lobby rejected — busy or already in a lobby (state=%d)" % _state)
        return false

    _set_state(State.HOSTING)
    if not await _ensure_initialized():
        _set_state(State.READY)
        return false

    var user: PlayFabUser = Auth.playfab_user

    var config := PlayFabLobbyConfig.new()
    config.max_players = 4
    config.access_policy = PlayFabLobbyConfig.ACCESS_POLICY_PUBLIC
    config.owner_migration_policy = PlayFabLobbyConfig.OWNER_MIGRATION_AUTOMATIC
    config.search_properties = {
        "string_key1": "casual",
    }
    config.lobby_properties = {
        "map": "harbor",
        "mode": "deathmatch",
    }
    config.member_properties = {
        "loadout": "rifle",
        "xuid": Auth.xbox_user.xuid,
    }

    var result: PlayFabResult = await PlayFab.multiplayer.create_lobby_async(user, config)
    if not result.ok:
        push_warning("[Lobby] create_lobby failed: %s (%s)" % [result.message, result.code])
        _set_state(State.READY)
        return false

    _lobby = result.data
    _lobby.state_changed.connect(_on_lobby_state_changed)
    _set_state(State.IN_LOBBY)
    lobby_joined.emit(_lobby)

    print("[Lobby] Lobby created: id=%s max=%d" % [_lobby.lobby_id, _lobby.max_member_count])
    print("[Lobby] connection string ready — copy to second client")
    print("[Lobby] %s" % _lobby.connection_string)
    return true
```

Notes:

- **`search_properties` must use PlayFab's reserved key names**
  (`string_key1` .. `string_keyN`, `number_key1` ..). Plain names
  like `"playstyle"` won't be searchable through
  `find_lobbies_async`. Lobby-property dictionaries do not have
  that restriction.
- All property values are **strings**, since PlayFab Multiplayer
  itself models them as strings. Encode richer types with
  `JSON.stringify(...)` on the writer side and `JSON.parse_string`
  on the reader.
- `ACCESS_POLICY_PUBLIC` makes the lobby searchable. Use
  `ACCESS_POLICY_FRIENDS` for friends-only or
  `ACCESS_POLICY_PRIVATE` for invite-only.
- The `xuid` key in `member_properties` is title-defined. The
  PlayFab entity key on a lobby member is **not** an XUID, so
  T7 (PlayFab Party) needs another way to map a Party peer back
  to an XUID for per-peer permission checks. Writing the local
  user's `Auth.xbox_user.xuid` into `member_properties["xuid"]`
  on host **and** join puts that mapping in the lobby roster,
  where T7's `_xuid_for_peer(peer_id)` can read it.

### Property scopes — what's public vs private

Picking the right bucket matters. The three dictionaries on
`PlayFabLobbyConfig` (and the matching `set_*_properties_async`
calls on `PlayFabLobby` once you're in the lobby) advertise to
very different audiences:

| Property bucket | Mutator | Who sees the value | Searchable filter? |
|---|---|---|---|
| `search_properties` | Set on `PlayFabLobbyConfig` at create time. Cannot be edited after create. | Anyone running `find_lobbies_async` for the lobby's `access_policy` audience (everyone for `PUBLIC`, friends for `FRIENDS`, nobody for `PRIVATE`). Returned on `PlayFabLobbySummary.search_properties`. | **Yes** — only these keys can appear in `find_lobbies_async` filter expressions, and only with reserved names like `string_key1`. |
| `lobby_properties` | `PlayFabLobby.set_properties_async` (owner only). | Only **lobby members** (read via `PlayFabLobby.properties`). Hidden from `find_lobbies_async` results. | No. |
| `member_properties` | `PlayFabLobby.set_member_properties_async` (acts on the local member only). | Only **lobby members** (read via `PlayFabLobbyMember.properties`). Hidden from search results. | No. |

Practical rules of thumb when wiring properties for a new feature:

- **Anything a matchmaker or lobby browser must filter on goes in
  `search_properties`.** Region, mode, skill bracket, lobby
  visibility flags — these are server-indexed and let other
  clients narrow their search before joining.
- **Anything a player should read after joining (but no one outside
  the lobby needs) goes in `lobby_properties`.** Selected map,
  game mode display name, round number, host preferences. Free of
  the `string_keyN` naming constraint and never leaks via
  `find_lobbies_async`.
- **Anything per-player — loadout, character pick, ready state,
  the XUID bridge from the note above — goes in
  `member_properties`.** Only that member can write their own
  entry; everyone in the lobby reads the same snapshot.

Don't put secrets in `search_properties`. Anything in that bucket
is visible to every client that can discover the lobby — including
clients you have no relationship with on `ACCESS_POLICY_PUBLIC`.

The `connection_string` survives across restarts of the title only
as long as the lobby itself stays alive — once the last member
leaves, the lobby is torn down and the string becomes invalid.

## Step 4 — Join from a second client

The simplest test flow: print the connection string in the host's
Output, paste it into a hard-coded `JOIN_STRING` constant in the
client scene, and run that scene:

```gdscript
const JOIN_STRING := "<paste connection string here>"

func join_lobby() -> bool:
    if not await _ensure_ready():
        return false
    if _state != State.READY:
        push_warning("[Lobby] join_lobby rejected — busy or already in a lobby (state=%d)" % _state)
        return false

    _set_state(State.JOINING)
    if not await _ensure_initialized():
        _set_state(State.READY)
        return false

    var user: PlayFabUser = Auth.playfab_user

    var config := PlayFabLobbyJoinConfig.new()
    config.member_properties = {
        "loadout": "shotgun",
        "xuid": Auth.xbox_user.xuid,
    }

    var result: PlayFabResult = await PlayFab.multiplayer.join_lobby_async(user, JOIN_STRING, config)
    if not result.ok:
        push_warning("[Lobby] join_lobby failed: %s (%s)" % [result.message, result.code])
        _set_state(State.READY)
        return false

    _lobby = result.data
    _lobby.state_changed.connect(_on_lobby_state_changed)
    _set_state(State.IN_LOBBY)
    lobby_joined.emit(_lobby)
    print("[Lobby] Joined lobby id=%s with %d member(s)" % [_lobby.lobby_id, _lobby.member_count])
    return true
```

In production you would replace the hard-coded join string with
either:

- A `find_lobbies_async` call against the `search_properties` you
  set on the host. PlayFab filter syntax uses the reserved search
  keys, for example `string_key1 eq 'casual'`.
- An XBOX-shell invite — the host invites a friend, the client side
  receives `PlayFab.multiplayer.invite_received` with a
  `PlayFabLobbyInvite.connection_string`, and the client calls
  `join_lobby_async` with that string.

## Step 5 — React to lobby state changes

`PlayFabLobby.state_changed` fires for everything that happens in
that specific lobby — member adds, member updates, ownership
changes, disconnects. The `kind` field on the payload tells you
which constant fired:

```gdscript
func _on_lobby_state_changed(change: PlayFabLobbyStateChange) -> void:
    match change.kind:
        PlayFabLobby.MEMBER_ADDED:
            var m: PlayFabLobbyMember = change.member
            print("[Lobby] member added: %s (local=%s)" % [m.user_id, str(m.is_local)])
        PlayFabLobby.MEMBER_REMOVED:
            var m: PlayFabLobbyMember = change.member
            print("[Lobby] member removed: %s" % m.user_id)
        PlayFabLobby.MEMBER_UPDATED:
            var m: PlayFabLobbyMember = change.member
            print("[Lobby] member updated: %s" % m.user_id)
        PlayFabLobby.PROPERTIES_UPDATED:
            print("[Lobby] lobby properties: %s" % str(change.properties))
        PlayFabLobby.OWNER_CHANGED:
            print("[Lobby] owner changed: %s" % str(change.lobby.owner_entity_key))
        PlayFabLobby.DISCONNECTED:
            push_warning("[Lobby] disconnected from lobby")
```

The snapshot on the lobby (members, properties, owner key) is
already refreshed by the time the signal fires, so reading
`change.lobby.members` or `change.lobby.properties` from inside the
handler gives you the post-change state.

For the aggregate `PlayFab.multiplayer.state_changed` signal, the
payload is a `PlayFabMultiplayerStateChange` that may apply to a
lobby **or** a matchmaking ticket. Most titles connect to the per-
lobby signal as above and ignore the multiplayer-aggregate signal
unless they also use matchmaking.

## Step 6 — Update properties while joined

Both lobby-wide properties (visible to every member) and per-member
properties (visible to every member; each user writes their own)
live behind awaitable mutators:

```gdscript
func push_loadout_change(loadout: String) -> void:
    if _lobby == null:
        return
    var pf: PlayFabResult = await _lobby.set_member_properties_async({ "loadout": loadout })
    if not pf.ok:
        push_warning("[Lobby] member props failed: %s" % pf.message)
        return
    # The local member snapshot is updated before this await resumes.
    # Remote members receive the change via MEMBER_UPDATED.

func change_map(new_map: String) -> void:
    if _lobby == null or not _lobby.is_owner(Auth.playfab_user):
        return
    var pf: PlayFabResult = await _lobby.set_properties_async({ "map": new_map })
    if not pf.ok:
        push_warning("[Lobby] lobby props failed: %s" % pf.message)
```

`set_member_properties_async` updates **this user's** member
properties — you cannot edit another member's properties.
`set_properties_async` is lobby-wide and only the current owner can
push it; non-owners should guard with `is_owner` or expect the
awaited `PlayFabResult` to come back with an error from the
service.

The opposite peer sees the change as
`PlayFabLobby.MEMBER_UPDATED` / `PROPERTIES_UPDATED` events. Use
that to drive UI like "Player A switched to Shotgun".

## Step 7 — Leave cleanly

Have a "leave" UI button or scene-exit hook call
`leave_async`:

```gdscript
func leave_lobby() -> bool:
    if _state != State.IN_LOBBY:
        push_warning("[Lobby] leave_lobby rejected — not in a lobby (state=%d)" % _state)
        return false

    _set_state(State.LEAVING)
    var pf: PlayFabResult = await _lobby.leave_async()
    if pf.ok:
        print("[Lobby] left lobby")
    else:
        push_warning("[Lobby] leave failed: %s" % pf.message)

    if _lobby != null and _lobby.state_changed.is_connected(_on_lobby_state_changed):
        _lobby.state_changed.disconnect(_on_lobby_state_changed)
    _lobby = null
    _set_state(State.READY)
    lobby_left.emit()
    return pf.ok
```

> **Why `LEAVING` is its own state.** `leave_async` is a round-trip
> to PlayFab — during the await, the lobby is no longer usable
> (calls like `set_member_properties_async` will fail) but it isn't
> torn down yet either. Gating the UI on `is_busy()` (which
> includes `LEAVING`) prevents the user from re-clicking Host
> before the leave completes and racing two operations.

The opposite peer sees this as `MEMBER_REMOVED`. The lobby itself
sticks around as long as there is at least one member; when the
last member leaves it is destroyed and the connection string becomes
invalid.

Always call `leave_async` before shutting down the PlayFab runtime
on a graceful exit — otherwise the lobby holds the seat open for
the normal disconnect timeout (~30 seconds today).

## Step 8 — Advertise rich presence (optional)

This step requires one piece of **Partner Center title config** —
a rich-presence ID registered against your title's SCID. If your
title has none configured yet, skip the step; the rest of the
chain still works. Configuring a presence ID is a one-time per-
title step in Partner Center → XBOX Live → Rich Presence.

Assuming you registered a presence ID called `in_lobby`, extend
your `Lobby` autoload so successful host / join sets the local
user's XBOX presence, and `leave` clears it:

```gdscript
const PRESENCE_IN_LOBBY := "in_lobby"

# Append to host_lobby() / join_lobby(), after _lobby is set:
func _publish_lobby_presence() -> void:
    if Auth.xbox_user == null:
        return
    var pf: GDKResult = await GDK.presence.set_presence_async(
            Auth.xbox_user, PRESENCE_IN_LOBBY)
    if not pf.ok:
        # Presence is best-effort — a missing rich-presence ID, an
        # offline test sandbox, or a wrong-sandbox PC all fail here.
        # Do not fail the host / join on a presence failure.
        push_warning("[Lobby] presence write failed: %s" % pf.message)

# Replace the end of leave_lobby():
func _clear_lobby_presence() -> void:
    if Auth.xbox_user == null:
        return
    var pf: GDKResult = await GDK.presence.clear_presence_async(Auth.xbox_user)
    if not pf.ok:
        push_warning("[Lobby] presence clear failed: %s" % pf.message)
```

Hook them into the existing methods:

```gdscript
func host_lobby() -> bool:
    # ... existing create_lobby_async / _lobby = result.data wiring ...
    await _publish_lobby_presence()
    return true

func join_lobby() -> bool:
    # ... existing join_lobby_async / _lobby = result.data wiring ...
    await _publish_lobby_presence()
    return true

func leave_lobby() -> bool:
    # ... existing _lobby.leave_async wiring ...
    await _clear_lobby_presence()
    _lobby = null
    return true
```

Notes:

- `set_presence_async`'s `state` argument is the **presence ID
  string** from Partner Center, not a UI-display string. XBOX
  resolves it on the friend's shell to whatever localized text
  you authored against that ID.
- The optional `rich_presence` Dictionary lets you pass
  `{ "scid": "...", "token_ids": ["...", ...] }` for presence IDs
  that include token substitutions. The empty default uses the
  title's current SCID and no token substitutions.
- `clear_presence_async` is the matching teardown. Skip it and
  the title's previous presence record sticks until the user
  signs out.
- `GDK.presence.local_presence_set(user: GDKUser)` fires on
  every successful write — connect it once at startup if you
  need a UI confirmation, instead of awaiting in each call site.

## Verify

Host run, then client joins, then client leaves:

Host Output:

```
[Lobby] Multiplayer initialized
[Lobby] Lobby created: id=BBA9... max=4
[Lobby] connection string ready — copy to second client
[Lobby] member added: title_player_account:6F4B... (local=false)
[Lobby] member updated: title_player_account:6F4B...
[Lobby] member removed: title_player_account:6F4B...
```

Client Output:

```
[Lobby] Multiplayer initialized
[Lobby] Joined lobby id=BBA9... with 2 member(s)
[Lobby] lobby properties: { "map": "harbor", "mode": "deathmatch" }
[Lobby] member updated: title_player_account:9831... (local=true)
[Lobby] left lobby
```

Common failures:

| Output | Diagnosis | Fix |
|---|---|---|
| `Multiplayer init failed: title_not_configured` | PlayFab Multiplayer is not enabled for the title. | Enable it in PlayFab Game Manager → Multiplayer → Lobby. |
| `create_lobby failed: invalid_property_key` | A `search_properties` key was not `string_keyN` / `number_keyN`. | Rename to PlayFab's reserved search-key namespace. |
| `join_lobby failed: not_found` | Stale connection string — the lobby was torn down. | Re-create the lobby on the host side and grab a fresh string. |
| `join_lobby failed: full` | Tried to join a lobby that has reached `max_players`. | Either increase `max_players` on the host or pick another lobby. |
| `set_properties_async` succeeds but the opposite peer never sees a `PROPERTIES_UPDATED` event | The opposite peer's `_lobby.state_changed` signal is not connected. | Connect it in the same function that returned the lobby. |

## Reference implementation

The cumulative end-state lives in
[`sample/tutorial_app/`](../../sample/tutorial_app/README.md):

- Scene: [`sample/tutorial_app/t05_lobby.tscn`](../../sample/tutorial_app/t05_lobby.tscn)
- Script: [`sample/tutorial_app/t05_lobby.gd`](../../sample/tutorial_app/t05_lobby.gd)
- Autoload introduced here: [`sample/tutorial_app/autoload/lobby.gd`](../../sample/tutorial_app/autoload/lobby.gd)
  (extended in T6, consumed by T7 and T8).

> **Path note.** The tutorial places `lobby.gd` at
> `res://lobby/lobby.gd` (one folder per topic). The sample
> collapses every autoload under `res://autoload/` because all
> three (`Auth`, `Lobby`, `Party`) are registered from the first
> tutorial. Same code, different folder.

## What's next
