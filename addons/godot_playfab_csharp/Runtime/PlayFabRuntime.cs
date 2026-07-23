using Godot;

namespace GodotPlayFab.Runtime;

/// <summary>
/// C# autoload that mirrors the PlayFab runtime bootstrap settings.
/// Register this as an autoload in a C# project when using the managed facade.
/// </summary>
public partial class PlayFabRuntime : Node
{
    private const string SettingInitializeOnStartup = "playfab/runtime/initialize_on_startup";
    private const string SettingTitleId = "playfab/runtime/title_id";
    private const string SettingEndpoint = "playfab/runtime/endpoint";
    private const string SettingEmbedDispatch = "playfab/runtime/embed_dispatch";

    public override void _Ready()
    {
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

    public override void _ExitTree()
    {
        if (PlayFab.IsAvailable && PlayFab.IsInitialized)
        {
            PlayFab.Shutdown();
        }
    }
}
