# Using the GDK addon from C# (`godot_gdk_csharp`)

The C++ `godot_gdk` GDExtension already loads in a Godot **.NET** project — its
`GDK` singleton and `GDK*` classes are reachable through dynamic
`GodotObject` access. The `godot_gdk_csharp` addon is a thin, hand-written
**managed facade** over that singleton so C# code gets the same ergonomics
GDScript enjoys: typed members, `Task`-based async, and typed result objects.

There is **no second native runtime** — the facade forwards every call to the
unchanged native DLL. See [`../async-patterns.md`](../async-patterns.md) for the
underlying Signal/Result model that the C# bridge wraps.

## Setup

1. Build the native addon as usual (`cmake --build build --preset debug`) so
   `addons/godot_gdk/bin/*.dll` exists and the `GDK` singleton registers.
2. Reference the facade from your game's `.csproj`:

   ```xml
   <ItemGroup>
     <ProjectReference Include="addons/godot_gdk_csharp/GodotGdkCSharp.csproj" />
   </ItemGroup>
   ```

3. (Optional) Register the C# bootstrap autoload instead of the GDScript one.
   Create a project-local subclass so Godot can resolve it by script path:

   ```csharp
   // Autoload/GdkBootstrap.cs
   public partial class GdkBootstrap : GodotGdk.Runtime.GdkRuntime { }
   ```

   ```ini
   ; project.godot
   [autoload]
   GdkRuntime="*res://Autoload/GdkBootstrap.cs"
   ```

   It reads the same `gdk/runtime/*` project settings as the GDScript bootstrap
   (`initialize_on_startup`, `auto_add_primary_user`).

## The async bridge

Every native `*_async` method returns a one-shot `Signal`; the facade awaits it
and hands you a `Task<GdkResult>`:

```csharp
using GodotGdk;
using GodotGdk.Types;

GdkResult result = await Gdk.Users.AddDefaultUserAsync();
if (!result.Ok)
{
    GD.PushWarning($"sign-in failed: {result.Message} ({result.Code})");
    return;
}

GdkUser user = result.DataAs<GdkUser>();
GD.Print($"signed in as {user.Gamertag}");
```

`GdkResult` mirrors the native `GDKResult`: `Ok`, `Code`, `Message`, `Data`
(raw `Variant`), `DataObject` (`GodotObject`), and `DataAs<T>()` for typed
payloads.

## Services

`Gdk` is a static entry point exposing every native service namespace as a lazily
cached, typed accessor — `Gdk.Users`, `Gdk.Achievements`, `Gdk.Leaderboards`,
`Gdk.Stats`, `Gdk.Social`, `Gdk.Store`, `Gdk.Presence`, `Gdk.Package`,
`Gdk.TitleStorage`, and the rest (21 in total, matching the native members).

```csharp
// Lifecycle
if (!Gdk.IsAvailable) { return; }            // extension loaded?
GdkResult init = Gdk.Initialize();
bool ready = Gdk.IsInitialized;

// Achievements
GdkResult update = await Gdk.Achievements.UpdateAchievementAsync(user, "1", 100);

// Root signals → C# events
Gdk.Initialized += () => GD.Print("GDK ready");
Gdk.RuntimeError += r => GD.PushWarning(r.Message);
```

Synchronous native methods stay synchronous (`Gdk.IsInitialized`,
`Gdk.Presence.GetCachedPresence(xuid)`), and service-level `runtime_error`
signals are re-exposed as C# `event Action<...>` on the relevant service.

## Cross-addon sign-in

The GDK user object flows into PlayFab sign-in unchanged — see
[`../playfab/csharp.md`](../playfab/csharp.md).

## Parity guarantee

`tests/csharp/FacadeParity.Tests` asserts that **every** native `doc_classes`
method, member, and signal has a matching managed wrapper, so this facade cannot
silently drift from the native surface. Run it with
`tools/run_csharp_tests.ps1`.
