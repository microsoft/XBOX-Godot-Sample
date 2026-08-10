using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Services;

/// <summary><c>GDK.accessibility</c> — closed captions and high-contrast queries.</summary>
public sealed class XboxAccessibility : XboxServiceBase
{
    internal XboxAccessibility(GodotObject o) : base(o) { }

    public XboxResult QueryClosedCaptionProperties() =>
        XboxResult.From(Call("query_closed_caption_properties").AsGodotObject());

    public XboxResult SetClosedCaptionEnabled(bool enabled) =>
        XboxResult.From(Call("set_closed_caption_enabled", enabled).AsGodotObject());

    public XboxResult QueryHighContrastMode() =>
        XboxResult.From(Call("query_high_contrast_mode").AsGodotObject());

    public string GetHighContrastModeName(int mode) =>
        Call("get_high_contrast_mode_name", mode).AsString();
}
