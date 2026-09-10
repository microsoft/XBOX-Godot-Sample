# XBOX Godot NetRumble

**This folder is a pointer, not a project.** NetRumble lives in its own repository:
**https://github.com/microsoft/XBOX-Godot-NetRumble**

## What it is

NetRumble is a complete Godot 4 multiplayer game that integrates the Microsoft Game
Development Kit (GDK) and PlayFab. It is a 2D top-down space shooter: up to 8 players fly
ships around a wrapping asteroid field, collect weapon power-ups, and shoot each other for
points.

Where the tutorial samples in this repository demonstrate one surface at a time, NetRumble
wires every service into actual gameplay. It is the reference for what a full title looks
like once the addons are in use. It is a working sample, not a certified title.

## What it demonstrates

- GDK sign-in exchanged for a PlayFab identity, and a sign-in screen that cooperates with
  the system account picker
- Join-code matchmaking with PlayFab Lobby, and PlayFab Party as a drop-in Godot
  `MultiplayerPeer`
- Host-authoritative replication with client prediction
- Multiplayer and communications privilege checks, plus per-player mute, block and avoid
- Voice chat, and text verification before user-authored content is published
- One-shot and incremental achievements
- Cloud saves written to the Game Save folder rather than `user://`
- Suspend, resume and constrain handling
- Activity publishing, join-from-guide invites, and connectivity detection

Every platform call goes through a single services facade, so gameplay and UI never reach
into GDK or PlayFab directly.

## Relationship to this repository

NetRumble consumes the addons authored here. It pins this repository as a submodule at
`external/xbox-godot-sample` and builds it into its own `addons/` folder, so changes to the
GDK, PlayFab and GameInput addons flow into NetRumble when it moves that pin.

## Requirements

Godot 4.6 or later on Windows, with the addons built once from the submodule. Sign-in,
multiplayer and achievements need the PC in the **XDKS.1** sandbox and a signed-in test
account. Exporting to XBOX Series X|S additionally requires the W4 Games console fork of
Godot.

Full setup lives in the NetRumble repository's own README and `docs/`.

> [!NOTE]
> The NetRumble repository is currently private, so the link above will return a 404 unless
> you have been granted access. Request access from the XBOX Developer Middleware, Samples
> and Skills squad.

## See also

- [Tutorials index](../../docs/tutorials/README.md)
- [Getting started](../../docs/getting-started.md)
- Integrated Xbox + PlayFab sample: [`sample/tutorial_integrated/`](../tutorial_integrated/README.md)
