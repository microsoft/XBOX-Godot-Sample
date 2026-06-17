using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>Presence/relationship filter used to create a social group.</summary>
public sealed class GdkSocialFilter : GdkObject
{
    internal GdkSocialFilter(GodotObject o) : base(o) { }
    public static GdkSocialFilter From(GodotObject o) => o == null ? null : new GdkSocialFilter(o);

    public int PresenceFilter
    {
        get => GetInt32("presence_filter");
        set => _o.Set("presence_filter", value);
    }

    public int RelationshipFilter
    {
        get => GetInt32("relationship_filter");
        set => _o.Set("relationship_filter", value);
    }
}

/// <summary>A live Social Manager group of tracked users.</summary>
public sealed class GdkSocialGroup : GdkObject
{
    internal GdkSocialGroup(GodotObject o) : base(o) { }
    public static GdkSocialGroup From(GodotObject o) => o == null ? null : new GdkSocialGroup(o);

    public GdkUser LocalUser => GdkUser.From(GetObject("local_user"));
    public bool IsLoaded => GetBool("loaded");
    public int GroupType => GetInt32("group_type");
    public string GroupTypeName => Call("get_group_type_name").AsString();
    public int PresenceFilter => Call("get_presence_filter").AsInt32();
    public int RelationshipFilter => Call("get_relationship_filter").AsInt32();
    public string[] TrackedXuids => Get("tracked_xuids").AsStringArray();
}

/// <summary>A user within a social graph/group, with rich profile/presence data.</summary>
public sealed class GdkSocialUser : GdkObject
{
    internal GdkSocialUser(GodotObject o) : base(o) { }
    public static GdkSocialUser From(GodotObject o) => o == null ? null : new GdkSocialUser(o);

    public string Xuid => GetString("xuid");
    public bool IsFavorite => GetBool("favorite");
    public bool IsFriend => GetBool("friend");
    public bool IsFollowingUser => Call("is_following_user").AsBool();
    public bool IsFollowedByCaller => Call("is_followed_by_caller").AsBool();
    public string DisplayName => GetString("display_name");
    public string RealName => GetString("real_name");
    public string DisplayPictureUrl => GetString("display_picture_url");
    public bool UsesAvatar => Call("uses_avatar").AsBool();
    public string Gamerscore => GetString("gamerscore");
    public string Gamertag => GetString("gamertag");
    public string ModernGamertag => Call("get_modern_gamertag").AsString();
    public string ModernGamertagSuffix => Call("get_modern_gamertag_suffix").AsString();
    public string UniqueModernGamertag => Call("get_unique_modern_gamertag").AsString();
    public GdkPresenceRecord Presence => GdkPresenceRecord.From(GetObject("presence"));
    public Godot.Collections.Dictionary TitleHistory => GetDict("title_history");
    public Godot.Collections.Dictionary PreferredColor => GetDict("preferred_color");
}
