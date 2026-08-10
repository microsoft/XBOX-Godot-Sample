#include "xbox_accessibility.h"

#include "xbox.h"
#include "xbox_result.h"
#include "xbox_runtime.h"

namespace godot {

namespace {

Color _xcolor_to_color(const XColor &p_color) {
    const uint8_t *channels = reinterpret_cast<const uint8_t *>(&p_color.Value);
    return Color(
            static_cast<float>(channels[1]) / 255.0f,
            static_cast<float>(channels[2]) / 255.0f,
            static_cast<float>(channels[3]) / 255.0f,
            static_cast<float>(channels[0]) / 255.0f);
}

XboxClosedCaptionProperties::FontEdgeAttribute _to_font_edge_attribute(XClosedCaptionFontEdgeAttribute p_value) {
    switch (p_value) {
        case XClosedCaptionFontEdgeAttribute::NoEdgeAttribute:
            return XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_NONE;
        case XClosedCaptionFontEdgeAttribute::RaisedEdges:
            return XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_RAISED;
        case XClosedCaptionFontEdgeAttribute::DepressedEdges:
            return XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_DEPRESSED;
        case XClosedCaptionFontEdgeAttribute::UniformedEdges:
            return XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_UNIFORM;
        case XClosedCaptionFontEdgeAttribute::DropShadowedEdges:
            return XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_DROP_SHADOW;
        case XClosedCaptionFontEdgeAttribute::Default:
        default:
            return XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_DEFAULT;
    }
}

XboxClosedCaptionProperties::FontStyle _to_font_style(XClosedCaptionFontStyle p_value) {
    switch (p_value) {
        case XClosedCaptionFontStyle::MonospacedWithSerifs:
            return XboxClosedCaptionProperties::FONT_STYLE_MONOSPACED_SERIF;
        case XClosedCaptionFontStyle::ProportionalWithSerifs:
            return XboxClosedCaptionProperties::FONT_STYLE_PROPORTIONAL_SERIF;
        case XClosedCaptionFontStyle::MonospacedWithoutSerifs:
            return XboxClosedCaptionProperties::FONT_STYLE_MONOSPACED_SANS_SERIF;
        case XClosedCaptionFontStyle::ProportionalWithoutSerifs:
            return XboxClosedCaptionProperties::FONT_STYLE_PROPORTIONAL_SANS_SERIF;
        case XClosedCaptionFontStyle::Casual:
            return XboxClosedCaptionProperties::FONT_STYLE_CASUAL;
        case XClosedCaptionFontStyle::Cursive:
            return XboxClosedCaptionProperties::FONT_STYLE_CURSIVE;
        case XClosedCaptionFontStyle::SmallCapitals:
            return XboxClosedCaptionProperties::FONT_STYLE_SMALL_CAPITALS;
        case XClosedCaptionFontStyle::Default:
        default:
            return XboxClosedCaptionProperties::FONT_STYLE_DEFAULT;
    }
}

String _font_edge_attribute_to_name(XboxClosedCaptionProperties::FontEdgeAttribute p_value) {
    switch (p_value) {
        case XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_NONE:
            return "none";
        case XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_RAISED:
            return "raised";
        case XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_DEPRESSED:
            return "depressed";
        case XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_UNIFORM:
            return "uniform";
        case XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_DROP_SHADOW:
            return "drop_shadow";
        case XboxClosedCaptionProperties::FONT_EDGE_ATTRIBUTE_DEFAULT:
        default:
            return "default";
    }
}

String _font_style_to_name(XboxClosedCaptionProperties::FontStyle p_value) {
    switch (p_value) {
        case XboxClosedCaptionProperties::FONT_STYLE_MONOSPACED_SERIF:
            return "monospaced_serif";
        case XboxClosedCaptionProperties::FONT_STYLE_PROPORTIONAL_SERIF:
            return "proportional_serif";
        case XboxClosedCaptionProperties::FONT_STYLE_MONOSPACED_SANS_SERIF:
            return "monospaced_sans_serif";
        case XboxClosedCaptionProperties::FONT_STYLE_PROPORTIONAL_SANS_SERIF:
            return "proportional_sans_serif";
        case XboxClosedCaptionProperties::FONT_STYLE_CASUAL:
            return "casual";
        case XboxClosedCaptionProperties::FONT_STYLE_CURSIVE:
            return "cursive";
        case XboxClosedCaptionProperties::FONT_STYLE_SMALL_CAPITALS:
            return "small_capitals";
        case XboxClosedCaptionProperties::FONT_STYLE_DEFAULT:
        default:
            return "default";
    }
}

} // namespace

void XboxClosedCaptionProperties::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_background_color"), &XboxClosedCaptionProperties::get_background_color);
    ClassDB::bind_method(D_METHOD("get_font_color"), &XboxClosedCaptionProperties::get_font_color);
    ClassDB::bind_method(D_METHOD("get_window_color"), &XboxClosedCaptionProperties::get_window_color);
    ClassDB::bind_method(D_METHOD("get_font_edge_attribute"), &XboxClosedCaptionProperties::get_font_edge_attribute);
    ClassDB::bind_method(D_METHOD("get_font_edge_attribute_name"), &XboxClosedCaptionProperties::get_font_edge_attribute_name);
    ClassDB::bind_method(D_METHOD("get_font_style"), &XboxClosedCaptionProperties::get_font_style);
    ClassDB::bind_method(D_METHOD("get_font_style_name"), &XboxClosedCaptionProperties::get_font_style_name);
    ClassDB::bind_method(D_METHOD("get_font_scale"), &XboxClosedCaptionProperties::get_font_scale);
    ClassDB::bind_method(D_METHOD("is_enabled"), &XboxClosedCaptionProperties::is_enabled);

    BIND_ENUM_CONSTANT(FONT_EDGE_ATTRIBUTE_DEFAULT);
    BIND_ENUM_CONSTANT(FONT_EDGE_ATTRIBUTE_NONE);
    BIND_ENUM_CONSTANT(FONT_EDGE_ATTRIBUTE_RAISED);
    BIND_ENUM_CONSTANT(FONT_EDGE_ATTRIBUTE_DEPRESSED);
    BIND_ENUM_CONSTANT(FONT_EDGE_ATTRIBUTE_UNIFORM);
    BIND_ENUM_CONSTANT(FONT_EDGE_ATTRIBUTE_DROP_SHADOW);

    BIND_ENUM_CONSTANT(FONT_STYLE_DEFAULT);
    BIND_ENUM_CONSTANT(FONT_STYLE_MONOSPACED_SERIF);
    BIND_ENUM_CONSTANT(FONT_STYLE_PROPORTIONAL_SERIF);
    BIND_ENUM_CONSTANT(FONT_STYLE_MONOSPACED_SANS_SERIF);
    BIND_ENUM_CONSTANT(FONT_STYLE_PROPORTIONAL_SANS_SERIF);
    BIND_ENUM_CONSTANT(FONT_STYLE_CASUAL);
    BIND_ENUM_CONSTANT(FONT_STYLE_CURSIVE);
    BIND_ENUM_CONSTANT(FONT_STYLE_SMALL_CAPITALS);

    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "background_color"), "", "get_background_color");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "font_color"), "", "get_font_color");
    ADD_PROPERTY(PropertyInfo(Variant::COLOR, "window_color"), "", "get_window_color");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_edge_attribute", PROPERTY_HINT_ENUM, "Default,None,Raised,Depressed,Uniform,Drop Shadow"), "", "get_font_edge_attribute");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "font_style", PROPERTY_HINT_ENUM, "Default,Monospaced Serif,Proportional Serif,Monospaced Sans Serif,Proportional Sans Serif,Casual,Cursive,Small Capitals"), "", "get_font_style");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "font_scale"), "", "get_font_scale");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "", "is_enabled");
}

Color XboxClosedCaptionProperties::get_background_color() const {
    return m_background_color;
}

Color XboxClosedCaptionProperties::get_font_color() const {
    return m_font_color;
}

Color XboxClosedCaptionProperties::get_window_color() const {
    return m_window_color;
}

XboxClosedCaptionProperties::FontEdgeAttribute XboxClosedCaptionProperties::get_font_edge_attribute() const {
    return m_font_edge_attribute;
}

String XboxClosedCaptionProperties::get_font_edge_attribute_name() const {
    return _font_edge_attribute_to_name(m_font_edge_attribute);
}

XboxClosedCaptionProperties::FontStyle XboxClosedCaptionProperties::get_font_style() const {
    return m_font_style;
}

String XboxClosedCaptionProperties::get_font_style_name() const {
    return _font_style_to_name(m_font_style);
}

double XboxClosedCaptionProperties::get_font_scale() const {
    return m_font_scale;
}

bool XboxClosedCaptionProperties::is_enabled() const {
    return m_enabled;
}

void XboxClosedCaptionProperties::set_from_native(const XClosedCaptionProperties &p_properties) {
    m_background_color = _xcolor_to_color(p_properties.BackgroundColor);
    m_font_color = _xcolor_to_color(p_properties.FontColor);
    m_window_color = _xcolor_to_color(p_properties.WindowColor);
    m_font_edge_attribute = _to_font_edge_attribute(p_properties.FontEdgeAttribute);
    m_font_style = _to_font_style(p_properties.FontStyle);
    m_font_scale = static_cast<double>(p_properties.FontScale);
    m_enabled = p_properties.Enabled;
}

void XboxAccessibility::_bind_methods() {
    ClassDB::bind_method(D_METHOD("query_closed_caption_properties"), &XboxAccessibility::query_closed_caption_properties);
    ClassDB::bind_method(D_METHOD("set_closed_caption_enabled", "enabled"), &XboxAccessibility::set_closed_caption_enabled);
    ClassDB::bind_method(D_METHOD("query_high_contrast_mode"), &XboxAccessibility::query_high_contrast_mode);
    ClassDB::bind_method(D_METHOD("get_high_contrast_mode_name", "mode"), &XboxAccessibility::get_high_contrast_mode_name);

    BIND_ENUM_CONSTANT(HIGH_CONTRAST_MODE_OFF);
    BIND_ENUM_CONSTANT(HIGH_CONTRAST_MODE_DARK);
    BIND_ENUM_CONSTANT(HIGH_CONTRAST_MODE_LIGHT);
    BIND_ENUM_CONSTANT(HIGH_CONTRAST_MODE_OTHER);
}

void XboxAccessibility::set_owner(Xbox *p_owner) {
    m_owner = p_owner;
}

XboxRuntime *XboxAccessibility::_get_runtime() const {
    if (m_owner == nullptr) {
        return nullptr;
    }

    return m_owner->get_runtime();
}

XboxAccessibility::HighContrastMode XboxAccessibility::_to_high_contrast_mode(XHighContrastMode p_mode) {
    switch (p_mode) {
        case XHighContrastMode::Dark:
            return HIGH_CONTRAST_MODE_DARK;
        case XHighContrastMode::Light:
            return HIGH_CONTRAST_MODE_LIGHT;
        case XHighContrastMode::Other:
            return HIGH_CONTRAST_MODE_OTHER;
        case XHighContrastMode::Off:
        default:
            return HIGH_CONTRAST_MODE_OFF;
    }
}

String XboxAccessibility::_high_contrast_mode_to_name(HighContrastMode p_mode) {
    switch (p_mode) {
        case HIGH_CONTRAST_MODE_DARK:
            return "dark";
        case HIGH_CONTRAST_MODE_LIGHT:
            return "light";
        case HIGH_CONTRAST_MODE_OTHER:
            return "other";
        case HIGH_CONTRAST_MODE_OFF:
        default:
            return "off";
    }
}

Ref<XboxResult> XboxAccessibility::query_closed_caption_properties() const {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(E_FAIL, "runtime_unavailable", "GDK runtime is not initialized.");
    }

    XClosedCaptionProperties native_properties = {};
    HRESULT hr = XClosedCaptionGetProperties(&native_properties);
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(hr, "Failed to query closed caption properties.", "closed_caption_get_failed");
        return result;
    }

    Ref<XboxClosedCaptionProperties> properties;
    properties.instantiate();
    properties->set_from_native(native_properties);

    return XboxResult::ok_result(properties);
}

Ref<XboxResult> XboxAccessibility::set_closed_caption_enabled(bool p_enabled) const {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(E_FAIL, "runtime_unavailable", "GDK runtime is not initialized.");
    }

    HRESULT hr = XClosedCaptionSetEnabled(p_enabled);
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(hr, "Failed to update closed caption enabled state.", "closed_caption_set_failed");
        return result;
    }

    Dictionary data;
    data["enabled"] = p_enabled;
    return XboxResult::ok_result(data);
}

Ref<XboxResult> XboxAccessibility::query_high_contrast_mode() const {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return XboxResult::error_result(E_FAIL, "runtime_unavailable", "GDK runtime is not initialized.");
    }

    XHighContrastMode native_mode = XHighContrastMode::Off;
    HRESULT hr = XHighContrastGetMode(&native_mode);
    if (FAILED(hr)) {
        Ref<XboxResult> result = XboxResult::hresult_error(hr, "Failed to query high contrast mode.", "high_contrast_get_failed");
        return result;
    }

    const HighContrastMode mode = _to_high_contrast_mode(native_mode);
    Dictionary data;
    data["mode"] = static_cast<int64_t>(mode);
    data["mode_name"] = _high_contrast_mode_to_name(mode);

    return XboxResult::ok_result(data);
}

String XboxAccessibility::get_high_contrast_mode_name(HighContrastMode p_mode) const {
    return _high_contrast_mode_to_name(p_mode);
}

} // namespace godot
