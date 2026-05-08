#ifndef GDK_ACCESSIBILITY_H
#define GDK_ACCESSIBILITY_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/color.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XAccessibility.h>

namespace godot {

class GDK;
class GDKResult;
class GDKRuntime;

class GDKClosedCaptionProperties : public RefCounted {
    GDCLASS(GDKClosedCaptionProperties, RefCounted);

public:
    enum FontEdgeAttribute {
        FONT_EDGE_ATTRIBUTE_DEFAULT = 0,
        FONT_EDGE_ATTRIBUTE_NONE,
        FONT_EDGE_ATTRIBUTE_RAISED,
        FONT_EDGE_ATTRIBUTE_DEPRESSED,
        FONT_EDGE_ATTRIBUTE_UNIFORM,
        FONT_EDGE_ATTRIBUTE_DROP_SHADOW,
    };

    enum FontStyle {
        FONT_STYLE_DEFAULT = 0,
        FONT_STYLE_MONOSPACED_SERIF,
        FONT_STYLE_PROPORTIONAL_SERIF,
        FONT_STYLE_MONOSPACED_SANS_SERIF,
        FONT_STYLE_PROPORTIONAL_SANS_SERIF,
        FONT_STYLE_CASUAL,
        FONT_STYLE_CURSIVE,
        FONT_STYLE_SMALL_CAPITALS,
    };

private:
    Color m_background_color = Color(0, 0, 0, 1);
    Color m_font_color = Color(1, 1, 1, 1);
    Color m_window_color = Color(0, 0, 0, 1);
    FontEdgeAttribute m_font_edge_attribute = FONT_EDGE_ATTRIBUTE_DEFAULT;
    FontStyle m_font_style = FONT_STYLE_DEFAULT;
    double m_font_scale = 1.0;
    bool m_enabled = false;

protected:
    static void _bind_methods();

public:
    Color get_background_color() const;
    Color get_font_color() const;
    Color get_window_color() const;
    FontEdgeAttribute get_font_edge_attribute() const;
    String get_font_edge_attribute_name() const;
    FontStyle get_font_style() const;
    String get_font_style_name() const;
    double get_font_scale() const;
    bool is_enabled() const;

    void set_from_native(const XClosedCaptionProperties &p_properties);
};

class GDKAccessibility : public RefCounted {
    GDCLASS(GDKAccessibility, RefCounted);

public:
    enum HighContrastMode {
        HIGH_CONTRAST_MODE_OFF = 0,
        HIGH_CONTRAST_MODE_DARK,
        HIGH_CONTRAST_MODE_LIGHT,
        HIGH_CONTRAST_MODE_OTHER,
    };

private:
    GDK *m_owner = nullptr;

    GDKRuntime *_get_runtime() const;
    static HighContrastMode _to_high_contrast_mode(XHighContrastMode p_mode);
    static String _high_contrast_mode_to_name(HighContrastMode p_mode);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> query_closed_caption_properties() const;
    Ref<GDKResult> set_closed_caption_enabled(bool p_enabled) const;
    Ref<GDKResult> query_high_contrast_mode() const;
    String get_high_contrast_mode_name(HighContrastMode p_mode) const;
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKClosedCaptionProperties::FontEdgeAttribute);
VARIANT_ENUM_CAST(godot::GDKClosedCaptionProperties::FontStyle);
VARIANT_ENUM_CAST(godot::GDKAccessibility::HighContrastMode);

#endif // GDK_ACCESSIBILITY_H
