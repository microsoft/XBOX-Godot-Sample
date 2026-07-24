using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabEntityData : PlayFabServiceBase
{
    internal PlayFabEntityData(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> AbortFileUploadsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("abort_file_uploads_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteFilesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_files_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> FinalizeFileUploadsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("finalize_file_uploads_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetFilesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_files_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetObjectsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_objects_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> InitiateFileUploadsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("initiate_file_uploads_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetObjectsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("set_objects_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
