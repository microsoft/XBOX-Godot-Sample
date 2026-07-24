using Godot;
using GodotGdk.Internal;

namespace GodotGdk.Services;

/// <summary><c>GDK.accessibility</c> — closed captions and high-contrast queries.</summary>
public sealed class GdkAccessibility : GdkServiceBase
{
    internal GdkAccessibility(GodotObject o) : base(o) { }

    public GdkResult QueryClosedCaptionProperties() =>
        GdkResult.From(Call("query_closed_caption_properties").AsGodotObject());

    public GdkResult SetClosedCaptionEnabled(bool enabled) =>
        GdkResult.From(Call("set_closed_caption_enabled", enabled).AsGodotObject());

    public GdkResult QueryHighContrastMode() =>
        GdkResult.From(Call("query_high_contrast_mode").AsGodotObject());

    public string GetHighContrastModeName(int mode) =>
        Call("get_high_contrast_mode_name", mode).AsString();
}
