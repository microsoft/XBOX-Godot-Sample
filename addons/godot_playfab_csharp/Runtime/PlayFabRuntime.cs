using Godot;

namespace GodotPlayFab.Runtime;

/// <summary>
/// C# autoload that mirrors <c>addons/godot_playfab/runtime/playfab_bootstrap.gd</c>.
/// Register this as an autoload in a C# project when using the managed facade:
/// it initializes PlayFab on startup when <c>playfab/runtime/initialize_on_startup</c>
/// is set and a title id is configured. Like the native bootstrap it stays inert
/// under the GDScript parse gate (<c>--gd-script-check</c>) and the headless test
/// runner (<c>res://tests/run_tests.gd</c>).
/// </summary>
public partial class PlayFabRuntime : Node
{
    private const string SettingInitializeOnStartup = "playfab/runtime/initialize_on_startup";
    private const string SettingTitleId = "playfab/runtime/title_id";
    private const string SettingEndpoint = "playfab/runtime/endpoint";
    private const string SettingEmbedDispatch = "playfab/runtime/embed_dispatch";
    private const string GdScriptCheckFlag = "--gd-script-check";
    private const string TestScriptPath = "res://tests/run_tests.gd";

    public override void _Ready()
    {
        if (ShouldSkipBootstrap())
        {
            SetProcess(false);
            return;
        }

        if (!PlayFab.IsAvailable)
        {
            GD.PushWarning("[PlayFab] Bootstrap: 'PlayFab' singleton not registered. Is the godot_playfab GDExtension built and loaded?");
            return;
        }

        PlayFab.Initialized += OnInitialized;

        bool initOnStartup = ProjectSettings.GetSetting(SettingInitializeOnStartup, false).AsBool();
        string titleId = ProjectSettings.GetSetting(SettingTitleId, string.Empty).AsString();
        _ = ProjectSettings.GetSetting(SettingEndpoint, string.Empty).AsString();
        _ = ProjectSettings.GetSetting(SettingEmbedDispatch, true).AsBool();

        if (initOnStartup && !string.IsNullOrEmpty(titleId) && !PlayFab.IsInitialized)
        {
            PlayFabResult init = PlayFab.Initialize();
            if (init.Ok)
            {
                GD.Print("[PlayFab] Bootstrap: PlayFab.Initialize() succeeded.");
            }
            else
            {
                GD.PushWarning($"[PlayFab] Bootstrap: {init.Message}");
            }
        }
    }

    private void OnInitialized()
    {
        GD.Print("[PlayFab] Runtime initialized");
    }

    public override void _Process(double delta)
    {
        bool embedDispatch = ProjectSettings.GetSetting(SettingEmbedDispatch, true).AsBool();
        if (!embedDispatch && PlayFab.IsAvailable && PlayFab.IsInitialized)
        {
            PlayFab.Dispatch();
        }
    }

    private static bool ShouldSkipBootstrap()
    {
        // Mirrors playfab_bootstrap.gd::_should_skip_bootstrap(): stay inert under
        // the GDScript parse gate (--gd-script-check) and the headless test runner
        // (res://tests/run_tests.gd) so validation/test contexts don't initialize
        // or shut down PlayFab.
        string[] userArgs = OS.GetCmdlineUserArgs();
        if (System.Array.IndexOf(userArgs, GdScriptCheckFlag) >= 0)
        {
            return true;
        }

        string[] args = OS.GetCmdlineArgs();
        if (System.Array.IndexOf(args, "--script") >= 0 && System.Array.IndexOf(args, TestScriptPath) >= 0)
        {
            GD.Print("[PlayFab] Bootstrap skipped for headless tests");
            return true;
        }

        return false;
    }

    public override void _ExitTree()
    {
        if (!PlayFab.IsAvailable)
        {
            return;
        }
        PlayFab.Initialized -= OnInitialized;
        if (PlayFab.IsInitialized)
        {
            PlayFab.Shutdown();
        }
    }
}
