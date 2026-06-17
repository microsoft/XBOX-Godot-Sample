using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>An Xbox Live user profile (display names, gamerscore, picture URIs).</summary>
public sealed class GdkUserProfile : GdkObject
{
    internal GdkUserProfile(GodotObject o) : base(o) { }
    public static GdkUserProfile From(GodotObject o) => o == null ? null : new GdkUserProfile(o);

    public string Xuid => GetString("xuid");
    public string AppDisplayName => GetString("app_display_name");
    public string AppDisplayPictureResizeUri => GetString("app_display_picture_resize_uri");
    public string GameDisplayName => GetString("game_display_name");
    public string GameDisplayPictureResizeUri => GetString("game_display_picture_resize_uri");
    public string Gamerscore => GetString("gamerscore");
    public string Gamertag => GetString("gamertag");
    public string ModernGamertag => GetString("modern_gamertag");
    public string ModernGamertagSuffix => GetString("modern_gamertag_suffix");
    public string UniqueModernGamertag => GetString("unique_modern_gamertag");
}
