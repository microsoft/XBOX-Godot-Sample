# PlayFab Demo

`sample\playfab_demo\` is the canonical smoke-test sample for the
`godot_playfab` addon.

## Build the repo

From the repository root:

```powershell
cmake --preset default
cmake --build build --preset debug
```

This refreshes the synced addon payload under `sample\playfab_demo\addons\`.
Do **not** hand-edit the sample `.gdextension` file to point at a different DLL
path; the repo build already keeps the sample copy aligned with the built addon.

## Open the sample

Open this Godot project:

```text
sample\playfab_demo\project.godot
```

The sample expects:

- the synced `godot_gdk` addon to be present
- the synced `godot_playfab` addon to be present
- a GDK-capable environment where Xbox sign-in is available

## Configure PlayFab

Set these Project Settings values before running the sample:

- `playfab/titleid` — required
- `playfab/endpoint` — optional; leave blank to derive the default endpoint
- `playfab/runtime/embed_dispatch` — defaults to `true`

The sample also uses the `GDKBootstrap` autoload so Xbox runtime initialization
and user sign-in are available before PlayFab sign-in.

## Run the flow

1. Open the project in the Godot editor.
2. Make sure the GDK runtime can initialize in your environment.
3. Click **Start Test**.
4. The sample will:
   - check that `PlayFab` and `GDK` are loaded
   - ensure a signed-in Xbox user is available
   - initialize the PlayFab runtime
   - sign the Xbox user into PlayFab
   - show the resulting PlayFab entity key in the status label

## Headless tests

Run the PlayFab contract suite from `sample\playfab_demo\`:

```powershell
godot --headless --script res://tests/run_tests.gd
```

The suite checks:

- singleton and class registration
- Project Settings registration and default values
- deterministic `not_initialized` and invalid-config error surfaces
- optional live init plus PlayFab sign-in smoke coverage when `playfab/titleid`
  is configured and Xbox sign-in is available

## Notes

- The scene in this sample is still the manual PlayFab smoke test.
- The repo now also includes the headless contract suite under
  `sample\playfab_demo\tests\`.
- If you change synced PlayFab addon files under `addons\godot_playfab\`, rebuild
  the repo so the sample copy stays current.
