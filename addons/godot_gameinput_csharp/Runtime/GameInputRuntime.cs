using Godot;

namespace GodotGameInput.Runtime;

/// <summary>
/// C# autoload that mirrors the native <c>GameInputBootstrap</c> GDScript
/// autoload. Register this (instead of the GDScript bootstrap) in a C# project:
/// it initializes the GameInput runtime on startup, polls every frame, and — when
/// <c>game_input/mapper/default_action_map</c> points at a
/// <c>GameInputActionMap</c> — spawns a default <c>GameInputMapper</c> child, all
/// driven by the same project settings the GDScript bootstrap reads. Like the
/// native bootstrap it stays inert under the GDScript parse gate
/// (<c>--gd-script-check</c>) and the headless test runner
/// (<c>res://tests/run_tests.gd</c>).
/// </summary>
public partial class GameInputRuntime : Node
{
    private const string SettingInitializeOnStartup = "game_input/runtime/initialize_on_startup";
    private const string SettingAutoPoll = "game_input/runtime/auto_poll";
    private const string SettingDefaultActionMap = "game_input/mapper/default_action_map";
    private const string GdScriptCheckFlag = "--gd-script-check";
    private const string TestScriptPath = "res://tests/run_tests.gd";

    private bool _autoPoll;
    private bool _initializedHere;
    private GameInputMapper _defaultMapper;

    public override void _Ready()
    {
        if (ShouldSkipBootstrap())
        {
            SetProcess(false);
            return;
        }

        if (!GameInput.IsAvailable)
        {
            GD.PushWarning("[GameInput] Bootstrap: 'GameInput' singleton not registered. Is the godot_gameinput GDExtension built and loaded?");
            SetProcess(false);
            return;
        }

        bool initOnStartup = ProjectSettings.GetSetting(SettingInitializeOnStartup, false).AsBool();
        _autoPoll = ProjectSettings.GetSetting(SettingAutoPoll, true).AsBool();

        if (initOnStartup && !GameInput.IsInitialized)
        {
            if (GameInput.Initialize())
            {
                _initializedHere = true;
            }
            else
            {
                GD.PushWarning("[GameInput] Bootstrap: GameInput.Initialize() failed; the runtime stays disabled on this host.");
            }
        }

        MaybeSpawnDefaultMapper();

        SetProcess(_autoPoll);
    }

    public override void _Process(double delta)
    {
        if (_autoPoll)
        {
            GameInput.Poll();
        }
    }

    private static bool ShouldSkipBootstrap()
    {
        // Mirrors gameinput_bootstrap.gd::_should_skip_bootstrap(): stay inert under
        // the GDScript parse gate (--gd-script-check) and the headless test runner
        // (res://tests/run_tests.gd) so validation/test contexts don't initialize
        // the runtime or spawn a mapper.
        string[] userArgs = OS.GetCmdlineUserArgs();
        if (System.Array.IndexOf(userArgs, GdScriptCheckFlag) >= 0)
        {
            return true;
        }

        string[] args = OS.GetCmdlineArgs();
        if (System.Array.IndexOf(args, "--script") >= 0 && System.Array.IndexOf(args, TestScriptPath) >= 0)
        {
            GD.Print("[GameInput] Bootstrap skipped for headless tests");
            return true;
        }

        return false;
    }

    private void MaybeSpawnDefaultMapper()
    {
        // Honour game_input/mapper/default_action_map: if the setting points at a
        // loadable GameInputActionMap, spawn a GameInputMapper child wired to it.
        // Soft-fails on every error path so a broken setting never breaks startup.
        // Mirrors gameinput_bootstrap.gd::_maybe_spawn_default_mapper().
        string path = ProjectSettings.GetSetting(SettingDefaultActionMap, string.Empty).AsString().Trim();
        if (string.IsNullOrEmpty(path))
        {
            return;
        }

        if (!ResourceLoader.Exists(path))
        {
            GD.PushWarning($"[GameInput] Bootstrap: default_action_map '{path}' does not exist.");
            return;
        }

        Resource resource = ResourceLoader.Load(path);
        if (resource == null)
        {
            GD.PushWarning($"[GameInput] Bootstrap: default_action_map '{path}' failed to load.");
            return;
        }

        if (!ClassDB.IsParentClass(resource.GetClass(), "GameInputActionMap"))
        {
            GD.PushWarning($"[GameInput] Bootstrap: default_action_map '{path}' is a {resource.GetClass()}, not a GameInputActionMap.");
            return;
        }

        if (!ClassDB.ClassExists("GameInputMapper"))
        {
            GD.PushWarning("[GameInput] Bootstrap: GameInputMapper class is not registered.");
            return;
        }

        _defaultMapper = GameInputMapper.Create();
        if (_defaultMapper?.Node == null)
        {
            _defaultMapper = null;
            GD.PushWarning("[GameInput] Bootstrap: failed to instantiate GameInputMapper.");
            return;
        }

        _defaultMapper.Node.Name = "DefaultMapper";
        _defaultMapper.ActionMap = GameInputActionMap.From(resource);
        AddChild(_defaultMapper.Node);
        GD.Print($"[GameInput] Bootstrap: default GameInputMapper spawned for '{path}'.");
    }

    public override void _ExitTree()
    {
        // Only shut down if THIS autoload initialized the runtime; otherwise leave
        // it to whoever brought it up (editor tooling, tests). Mirrors the native
        // GameInputBootstrap "_initialized_here" rule.
        if (_initializedHere && GameInput.IsAvailable && GameInput.IsInitialized)
        {
            GameInput.Shutdown();
            _initializedHere = false;
        }
    }
}
