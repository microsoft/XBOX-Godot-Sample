#include "gdk_result_codes_internal.h"

#include <cstdio>

namespace gdk_internal {

char *format_hresult_hex(HRESULT p_hresult, char *out_buffer, std::size_t buffer_size) noexcept {
    if (out_buffer == nullptr || buffer_size == 0) {
        return out_buffer;
    }
    if (buffer_size < HRESULT_HEX_BUFFER_SIZE) {
        out_buffer[0] = '\0';
        return out_buffer;
    }
    std::snprintf(out_buffer, buffer_size, "0x%08X", static_cast<unsigned int>(p_hresult));
    return out_buffer;
}

godot::String format_hresult_string(HRESULT p_hresult) {
    char buffer[16];
    format_hresult_hex(p_hresult, buffer, sizeof(buffer));
    return godot::String(buffer);
}

godot::String format_hresult_message(const godot::String &p_action, HRESULT p_hresult) {
    godot::String message = p_action;
    if (!message.is_empty()) {
        message += " ";
    }
    message += "(HRESULT " + format_hresult_string(p_hresult) + ")";
    return message;
}

godot::String code_or_format_hresult(const godot::String &p_provided, HRESULT p_hresult) {
    return p_provided.is_empty() ? format_hresult_string(p_hresult) : p_provided;
}

} // namespace gdk_internal
