using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Types;

/// <summary>A single Xbox achievement and the player's progress toward it.</summary>
public sealed class GdkAchievement : GdkObject
{
    internal GdkAchievement(GodotObject o) : base(o) { }
    public static GdkAchievement From(GodotObject o) => o == null ? null : new GdkAchievement(o);

    public string Id => GetString("id");
    public string Name => GetString("name");
    public string ServiceConfigurationId => GetString("service_configuration_id");
    public string ProgressState => GetString("progress_state");
    public int ProgressPercent => GetInt32("progress_percent");
    public bool IsUnlocked => GetBool("unlocked");
    public bool IsSecret => GetBool("secret");
    public string LockedDescription => GetString("locked_description");
    public string UnlockedDescription => GetString("unlocked_description");
}
