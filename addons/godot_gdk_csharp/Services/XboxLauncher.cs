using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.launcher</c> — launch URIs (e.g. deep links) for a user.</summary>
public sealed class XboxLauncher : XboxServiceBase
{
    internal XboxLauncher(GodotObject o) : base(o) { }

    public XboxResult LaunchUri(string uri, XboxUser user) =>
        XboxResult.From(Call("launch_uri", uri, user?.Raw).AsGodotObject());
}
