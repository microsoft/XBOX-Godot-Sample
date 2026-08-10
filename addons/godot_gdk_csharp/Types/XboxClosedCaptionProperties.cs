using Godot;
using GodotXbox.Internal;

namespace GodotXbox.Types;

/// <summary>System closed-caption rendering properties.</summary>
public sealed class XboxClosedCaptionProperties : XboxObject
{
    internal XboxClosedCaptionProperties(GodotObject o) : base(o) { }
    public static XboxClosedCaptionProperties From(GodotObject o) => o == null ? null : new XboxClosedCaptionProperties(o);

    public Color BackgroundColor => GetColor("background_color");
    public Color FontColor => GetColor("font_color");
    public Color WindowColor => GetColor("window_color");
    public int FontEdgeAttribute => GetInt32("font_edge_attribute");
    public string FontEdgeAttributeName => Call("get_font_edge_attribute_name").AsString();
    public int FontStyle => GetInt32("font_style");
    public string FontStyleName => Call("get_font_style_name").AsString();
    public double FontScale => GetDouble("font_scale");
    public bool IsEnabled => GetBool("enabled");
}
