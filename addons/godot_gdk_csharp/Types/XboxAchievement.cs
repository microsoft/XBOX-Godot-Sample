using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>A single Xbox achievement and the player's progress toward it.</summary>
public sealed class XboxAchievement : XboxObject
{
    internal XboxAchievement(GodotObject o) : base(o) { }
    public static XboxAchievement From(GodotObject o) => o == null ? null : new XboxAchievement(o);

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
