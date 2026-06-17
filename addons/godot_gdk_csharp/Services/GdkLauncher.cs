using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.launcher</c> — launch URIs (e.g. deep links) for a user.</summary>
public sealed class GdkLauncher : GdkServiceBase
{
    internal GdkLauncher(GodotObject o) : base(o) { }

    public GdkResult LaunchUri(string uri, GdkUser user) =>
        GdkResult.From(Call("launch_uri", uri, user?.Raw).AsGodotObject());
}
