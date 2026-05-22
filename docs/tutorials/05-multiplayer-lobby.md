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
- Your PlayFab title has Lobby enabled. PlayFab titles created in
  the last few years have it on by default; older titles need the
  **Multiplayer → Lobby** feature switched on in the PlayFab Game
  Manager.
- Two Godot processes (one host, one client). The simplest way to
  test on one PC is to run the host scene in the editor and a
  separate exported build for the client, each signed into a
  different Xbox test account. Two editors with two different
  PlayFab sessions also works as long as both PCs are in the same
  sandbox.
- `Auth.playfab_user` resolves to an Xbox-backed PlayFab session
  for the snippets below. Custom-ID sessions also work for lobby
  but invites from the Xbox shell will not.

> **Lobby vs. Matchmaking.** Lobbies are persistent rooms identified
> by a lobby id and a connection string. Matchmaking
> (`create_match_ticket_async`) returns a `PlayFabMatchTicket`;
> when the ticket completes, its
> `arranged_lobby_connection_string` is what you hand to
> `join_arranged_lobby_async`. This tutorial covers explicit lobby
> create / join; the matchmaking pipeline is a future tutorial.

## Step 1 — Initialize PlayFab Multiplayer

PlayFab Multiplayer is a separate runtime that sits on top of the
main PlayFab runtime — it needs its own `initialize_async` before
any lobby method works:

```gdscript
extends Node

var _lobby: PlayFabLobby = null

func _ready() -> void:
    if Auth.playfab_user == null:
        await Auth.sign_in_completed

    if not PlayFab.multiplayer.is_initialized():
        var init: PlayFabResult = await PlayFab.multiplayer.initialize_async()
        if not init.ok:
            push_error("[Lobby] Multiplayer init failed: %s" % init.message)
            return

    print("[Lobby] Multiplayer initialized")

    PlayFab.multiplayer.state_changed.connect(_on_state_changed)
    PlayFab.multiplayer.invite_received.connect(_on_invite_received)
    PlayFab.multiplayer.multiplayer_error.connect(_on_multiplayer_error)

func _on_state_changed(change: PlayFabMultiplayerStateChange) -> void:
    pass # Per-lobby changes are routed to _on_lobby_state_changed in Step 4.

func _on_invite_received(invite: PlayFabLobbyInvite) -> void:
    print("[Lobby] invite from %s: %s" % [invite.sender_entity_key, invite.connection_string])

func _on_multiplayer_error(result: PlayFabResult) -> void:
    push_warning("[Lobby] multiplayer error: %s (%s)" % [result.message, result.code])
```

The three signals are the **firehose** of every lobby + matchmaking
change. Connect them at startup; per-call awaits handle the
synchronous part of a create / join, but member updates,
disconnect, and ownership migration all arrive here.

## Step 2 — Host a lobby

Build a `PlayFabLobbyConfig`, hand it to `create_lobby_async`, and
hold on to the returned `PlayFabLobby`. The lobby's
`connection_string` is what your client side will need:

```gdscript
func host_lobby() -> void:
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
    }

    var result: PlayFabResult = await PlayFab.multiplayer.create_lobby_async(user, config)
    if not result.ok:
        push_warning("[Lobby] create_lobby failed: %s (%s)" % [result.message, result.code])
        return

    _lobby = result.data
    _lobby.state_changed.connect(_on_lobby_state_changed)
    print("[Lobby] Lobby created: id=%s max=%d" % [_lobby.lobby_id, _lobby.max_member_count])
    print("[Lobby] connection string ready — copy to second client")
    print("[Lobby] %s" % _lobby.connection_string)
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

The `connection_string` survives across restarts of the title only
as long as the lobby itself stays alive — once the last member
leaves, the lobby is torn down and the string becomes invalid.

## Step 3 — Join from a second client

The simplest test flow: print the connection string in the host's
Output, paste it into a hard-coded `JOIN_STRING` constant in the
client scene, and run that scene:

```gdscript
const JOIN_STRING := "<paste connection string here>"

func join_lobby() -> void:
    var user: PlayFabUser = Auth.playfab_user

    var config := PlayFabLobbyJoinConfig.new()
    config.member_properties = {
        "loadout": "shotgun",
    }

    var result: PlayFabResult = await PlayFab.multiplayer.join_lobby_async(user, JOIN_STRING, config)
    if not result.ok:
        push_warning("[Lobby] join_lobby failed: %s (%s)" % [result.message, result.code])
        return

    _lobby = result.data
    _lobby.state_changed.connect(_on_lobby_state_changed)
    print("[Lobby] Joined lobby id=%s with %d member(s)" % [_lobby.lobby_id, _lobby.member_count])
```

In production you would replace the hard-coded join string with
either:

- A `find_lobbies_async` call against the `search_properties` you
  set on the host. PlayFab filter syntax uses the reserved search
  keys, for example `string_key1 eq 'casual'`.
- An Xbox-shell invite — the host invites a friend, the client side
  receives `PlayFab.multiplayer.invite_received` with a
  `PlayFabLobbyInvite.connection_string`, and the client calls
  `join_lobby_async` with that string.

## Step 4 — React to lobby state changes

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

## Step 5 — Update properties while joined

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

## Step 6 — Leave cleanly

Have a "leave" UI button or scene-exit hook call
`leave_async`:

```gdscript
func leave_lobby() -> void:
    if _lobby == null:
        return
    var pf: PlayFabResult = await _lobby.leave_async()
    if pf.ok:
        print("[Lobby] left lobby")
    else:
        push_warning("[Lobby] leave failed: %s" % pf.message)
    _lobby = null
```

The opposite peer sees this as `MEMBER_REMOVED`. The lobby itself
sticks around as long as there is at least one member; when the
last member leaves it is destroyed and the connection string becomes
invalid.

Always call `leave_async` before shutting down the PlayFab runtime
on a graceful exit — otherwise the lobby holds the seat open for
the normal disconnect timeout (~30 seconds today).

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

## What's next

You can now stand up a lobby, get a second client into it, and
keep both sides in sync as members come and go. Tutorial 6 covers
the input side — wiring Microsoft GameInput into Godot's
`InputMap` so your gameplay code keeps using
`Input.is_action_pressed("jump")`:

- [**Tutorial 6 — Map a controller through GameInput**](06-gameinput-action-bridge.md)
- Reference: [PlayFabMultiplayer](../playfab/plugin.md),
  [PlayFabLobby](../playfab/plugin.md)
