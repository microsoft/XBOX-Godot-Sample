using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary>
/// <c>GDK.get_game_save()</c> — XGameSaveFiles-backed per-user save storage. The
/// resolved container folder is a normal filesystem path the title can read and
/// write directly.
/// </summary>
public sealed class XboxGameSave : XboxServiceBase
{
    internal XboxGameSave(GodotObject o) : base(o) { }

    public Task<XboxResult> GetFolderAsync(XboxUser user) =>
        CallResultAsync("get_folder_async", user?.Raw);

    public Task<XboxResult> GetRemainingQuotaAsync(XboxUser user) =>
        CallResultAsync("get_remaining_quota_async", user?.Raw);
}
