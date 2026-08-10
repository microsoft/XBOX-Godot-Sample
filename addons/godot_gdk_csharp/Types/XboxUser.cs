using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>An Xbox user (local player) signed into the title via the GDK.</summary>
public sealed class XboxUser : XboxObject
{
    internal XboxUser(GodotObject o) : base(o)
    {
    }

    public static XboxUser From(GodotObject o) => o == null ? null : new XboxUser(o);

    public long LocalId => GetInt("local_id");
    public string Xuid => GetString("xuid");
    public string Gamertag => GetString("gamertag");
    public int AgeGroup => GetInt32("age_group");
    public string AgeGroupName => Call("get_age_group_name").AsString();
    public int SignInState => GetInt32("sign_in_state");
    public string SignInStateName => Call("get_sign_in_state_name").AsString();
    public bool IsGuest => GetBool("guest");
    public bool IsSignedIn => GetBool("signed_in");
    public bool IsStoreUser => GetBool("store_user");
}
