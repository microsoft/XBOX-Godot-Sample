using Godot;

namespace GodotGdk.Runtime;

/// <summary>
/// C# autoload that mirrors <c>addons/godot_gdk/runtime/gdk_bootstrap.gd</c>.
/// Register this as an autoload (instead of the GDScript bootstrap) in a C#
/// project: it initializes the GDK runtime on startup and optionally adds the
/// default user, both driven by the same project settings the GDScript
/// bootstrap reads. Like the native bootstrap it stays inert under the GDScript
/// parse gate (<c>--gd-script-check</c>) and the headless test runner
/// (<c>res://tests/run_tests.gd</c>).
/// </summary>
public partial class GdkRuntime : Node
{
    private const string SettingInitializeOnStartup = "gdk/runtime/initialize_on_startup";
    private const string SettingAutoAddPrimaryUser = "gdk/runtime/auto_add_primary_user";
    private const string GdScriptCheckFlag = "--gd-script-check";
    private const string TestScriptPath = "res://tests/run_tests.gd";

    private bool _startupUserInProgress;

    public override void _Ready()
    {
        if (ShouldSkipBootstrap())
        {
            return;
        }

        if (!Gdk.IsAvailable)
        {
            GD.PushWarning("[GDK] Bootstrap: 'GDK' singleton not registered. Is the godot_gdk GDExtension built and loaded?");
            return;
        }

        Gdk.Initialized += OnInitialized;
        Gdk.RuntimeError += OnRuntimeError;
        Gdk.Users.UserChanged += OnUserChanged;

        bool initOnStartup = ProjectSettings.GetSetting(SettingInitializeOnStartup, false).AsBool();
        if (initOnStartup && !Gdk.IsInitialized)
        {
            GdkResult init = Gdk.Initialize();
            if (init.Ok)
            {
                GD.Print("[GDK] Bootstrap: GDK.Initialize() succeeded.");
                _ = MaybeStartDefaultUser();
            }
            else
            {
                GD.PushWarning($"[GDK] Bootstrap: {init.Message}");
            }
        }
        else if (Gdk.IsInitialized)
        {
            _ = MaybeStartDefaultUser();
        }
    }

    private void OnInitialized()
    {
        GD.Print("[GDK] Runtime initialized");
        _ = MaybeStartDefaultUser();
    }

    private void OnRuntimeError(GdkResult result) => GD.PushWarning($"[GDK] {result.Message}");

    private void OnUserChanged(GodotGdk.Types.GdkUser user, string changeKind)
    {
        if (user == null)
        {
            return;
        }

        GD.Print($"[GDK] User {changeKind}: {user.Gamertag}");
    }

    private async System.Threading.Tasks.Task MaybeStartDefaultUser()
    {
        bool autoAdd = ProjectSettings.GetSetting(SettingAutoAddPrimaryUser, false).AsBool();
        if (!autoAdd || !Gdk.IsInitialized || _startupUserInProgress)
        {
            return;
        }

        if (Gdk.Users.GetPrimaryUser() != null)
        {
            return;
        }

        _startupUserInProgress = true;
        GdkResult result = await Gdk.Users.AddDefaultUserAsync();
        _startupUserInProgress = false;

        if (result != null && !result.Ok && result.Code != "cancelled")
        {
            GD.PushWarning($"[GDK] Bootstrap: silent sign-in did not complete successfully: {result.Message}");
        }
    }

    private static bool ShouldSkipBootstrap()
    {
        // Mirrors gdk_bootstrap.gd::_should_skip_bootstrap(): stay inert under the
        // GDScript parse gate (--gd-script-check) and the headless test runner
        // (res://tests/run_tests.gd) so validation/test contexts don't initialize
        // GDK or kick off silent sign-in.
        string[] userArgs = OS.GetCmdlineUserArgs();
        if (System.Array.IndexOf(userArgs, GdScriptCheckFlag) >= 0)
        {
            return true;
        }

        string[] args = OS.GetCmdlineArgs();
        bool runningScript = System.Array.IndexOf(args, "--script") >= 0 || System.Array.IndexOf(args, "-s") >= 0;
        if (runningScript && System.Array.IndexOf(args, TestScriptPath) >= 0)
        {
            GD.Print("[GDK] Bootstrap skipped for headless tests");
            return true;
        }

        return false;
    }

    public override void _ExitTree()
    {
        if (!Gdk.IsAvailable)
        {
            return;
        }
        Gdk.Initialized -= OnInitialized;
        Gdk.RuntimeError -= OnRuntimeError;
        Gdk.Users.UserChanged -= OnUserChanged;
        if (Gdk.IsInitialized)
        {
            Gdk.Shutdown();
        }
    }
}
