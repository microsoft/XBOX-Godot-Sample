using Godot;
using GodotPlayFab.Internal;

namespace GodotPlayFab.Types;

public sealed class PlayFabPartyTextMessageConfig : PlayFabObject
{
    internal PlayFabPartyTextMessageConfig(GodotObject o) : base(o)
    {
    }

    public static PlayFabPartyTextMessageConfig From(GodotObject o) => o == null ? null : new PlayFabPartyTextMessageConfig(o);

    public Godot.Collections.Dictionary Metadata => GetDict("metadata");
}
