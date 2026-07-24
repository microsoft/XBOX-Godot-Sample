using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabGameSaves : PlayFabServiceBase
{
    internal PlayFabGameSaves(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> AddUserWithUiAsync(PlayFabUser user, int options = 0) =>
        CallResultAsync("add_user_with_ui_async", user?.Raw, options);

    public Task<PlayFabResult> UploadWithUiAsync(PlayFabUser user, bool release_device_as_active = false) =>
        CallResultAsync("upload_with_ui_async", user?.Raw, release_device_as_active);

    public Task<PlayFabResult> SetSaveDescriptionAsync(PlayFabUser user, string short_save_description) =>
        CallResultAsync("set_save_description_async", user?.Raw, short_save_description);

    public Task<PlayFabResult> ResetCloudAsync(PlayFabUser user) =>
        CallResultAsync("reset_cloud_async", user?.Raw);

    public PlayFabResult GetFolder(PlayFabUser user) =>
        PlayFabResult.From(Call("get_folder", user?.Raw).AsGodotObject());

    public PlayFabResult GetFolderSize(PlayFabUser user) =>
        PlayFabResult.From(Call("get_folder_size", user?.Raw).AsGodotObject());

    public PlayFabResult GetRemainingQuota(PlayFabUser user) =>
        PlayFabResult.From(Call("get_remaining_quota", user?.Raw).AsGodotObject());

    public PlayFabResult IsConnectedToCloud(PlayFabUser user) =>
        PlayFabResult.From(Call("is_connected_to_cloud", user?.Raw).AsGodotObject());

    public const int ADDUSEROPTIONNONE = 0;

    public const int ADDUSEROPTIONROLLBACKTOLASTKNOWNGOOD = 1;

    public const int ADDUSEROPTIONROLLBACKTOLASTCONFLICT = 2;
}
