using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary>
/// <c>GDK.get_game_save()</c> — XGameSaveFiles-backed per-user save storage. The
/// resolved container folder is a normal filesystem path the title can read and
/// write directly.
/// </summary>
public sealed class GdkGameSave : GdkServiceBase
{
    internal GdkGameSave(GodotObject o) : base(o) { }

    public Task<GdkResult> GetFolderAsync(GdkUser user) =>
        CallResultAsync("get_folder_async", user?.Raw);

    public GdkResult GetRemainingQuota(GdkUser user) =>
        GdkResult.From(Call("get_remaining_quota", user?.Raw).AsGodotObject());
}
