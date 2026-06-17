# Using the GameInput addon from C# (`godot_gameinput_csharp`)

`godot_gameinput_csharp` is the managed facade over the native `godot_gameinput`
GDExtension. Unlike the GDK and PlayFab facades, the GameInput API is **fully
synchronous** (poll-based) and is shaped to integrate with Godot's
`Input`/`InputMap` flow rather than an async result model — so there is no
`Task`/`Result` bridge here, just typed wrappers, events, and an action-map
bridge.

## Setup

1. Build the native addon (`cmake --build build --preset debug`).
2. Reference the facade:

   ```xml
   <ItemGroup>
     <ProjectReference Include="addons/godot_gameinput_csharp/GodotGameInputCSharp.csproj" />
   </ItemGroup>
   ```

3. (Optional) C# bootstrap autoload — initializes on startup and polls each
   frame, driven by the `game_input/runtime/*` project settings:

   ```csharp
   // Autoload/GameInputBootstrap.cs
   public partial class GameInputBootstrap : GodotGameInput.Runtime.GameInputRuntime { }
   ```

   ```ini
   [autoload]
   GameInputRuntime="*res://Autoload/GameInputBootstrap.cs"
   ```

## Devices, readings, and haptics

```csharp
using GodotGameInput;

if (!GameInput.IsAvailable) { return; }

GameInputDevice pad = GameInput.GetPrimaryDevice(GameInput.DeviceKind.Gamepad);
if (pad != null)
{
    GameInputReading reading = GameInput.GetCurrentReading(pad);
    if (reading.WasButtonPressed(GameInputDevice.Button.A))
    {
        GameInput.SetVibration(pad, lowFreq: 0.6f, highFreq: 0.6f);
    }
    float moveX = reading.GetAxis(GameInputDevice.Axis.LeftX);
}

// Hot-plug events (always raised on the main thread)
GameInput.DeviceConnected += d => GD.Print($"connected: {d.DisplayName}");
GameInput.DeviceDisconnected += id => GD.Print($"disconnected: {id}");
```

Enums are nested for discoverability: `GameInputDevice.Button`,
`GameInputDevice.Axis`, `GameInputDevice.Source`, and `GameInput.DeviceKind`.

## The action-map bridge

`GameInputActionMap` + `GameInputBinding` + `GameInputMapper` wrap the native
authoring types. Build a map in code and let the mapper drive Godot's
`InputMap` every frame so the rest of your game keeps using
`Input.IsActionPressed("jump")`:

```csharp
GameInputActionMap map = GameInputActionMap.Create();

GameInputBinding jump = GameInputBinding.Create();
jump.Action = "jump";
jump.Source = GameInputDevice.Source.BtnA;
map.AddBinding(jump);

GameInputBinding left = GameInputBinding.Create();
left.Action = "move_left";
left.Source = GameInputDevice.Source.AxisLeftX;
left.IsAxis = true;
left.AxisInvert = true;
map.AddBinding(left);

GameInputMapper mapper = GameInputMapper.Create();
mapper.ActionMap = map;
AddChild(mapper.Node);   // add the underlying node to the scene tree
```

The actions you bind to (`"jump"`, `"move_left"`, …) must already exist in the
project's `InputMap`; the mapper additively refreshes both polled state and
`InputEventAction` delivery. A runnable example lives in
[`sample/tutorial_gameinput_csharp`](../../sample/tutorial_gameinput_csharp).

## Parity guarantee

`tests/csharp/FacadeParity.Tests` asserts the singleton method/signal surface and
the device/reading/mapping types are fully wrapped. Run via
`tools/run_csharp_tests.ps1`.
