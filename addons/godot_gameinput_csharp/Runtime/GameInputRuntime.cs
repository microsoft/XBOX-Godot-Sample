using Godot;

namespace GodotGameInput.Runtime;

/// <summary>
/// C# autoload that mirrors the native <c>GameInputBootstrap</c> GDScript
/// autoload. Register this (instead of the GDScript bootstrap) in a C# project:
/// it initializes the GameInput runtime on startup and polls every frame, driven
/// by the same project settings the GDScript bootstrap reads.
/// </summary>
public partial class GameInputRuntime : Node
{
    private const string SettingInitializeOnStartup = "game_input/runtime/initialize_on_startup";
    private const string SettingAutoPoll = "game_input/runtime/auto_poll";

    private bool _autoPoll;

    public override void _Ready()
    {
        if (!GameInput.IsAvailable)
        {
            GD.PushWarning("[GameInput] Bootstrap: 'GameInput' singleton not registered. Is the godot_gameinput GDExtension built and loaded?");
            SetProcess(false);
            return;
        }

        bool initOnStartup = ProjectSettings.GetSetting(SettingInitializeOnStartup, true).AsBool();
        _autoPoll = ProjectSettings.GetSetting(SettingAutoPoll, true).AsBool();

        if (initOnStartup && !GameInput.IsInitialized)
        {
            if (!GameInput.Initialize())
            {
                GD.PushWarning("[GameInput] Bootstrap: GameInput.Initialize() failed; the runtime stays disabled on this host.");
            }
        }

        SetProcess(_autoPoll);
    }

    public override void _Process(double delta)
    {
        if (_autoPoll)
        {
            GameInput.Poll();
        }
    }

    public override void _ExitTree()
    {
        if (GameInput.IsAvailable && GameInput.IsInitialized)
        {
            GameInput.Shutdown();
        }
    }
}
