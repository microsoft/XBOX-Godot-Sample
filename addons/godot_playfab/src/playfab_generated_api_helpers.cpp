#include "playfab_generated_api_helpers.h"

#include <godot_cpp/classes/json.hpp>

namespace godot {
namespace playfab_generated {

Signal make_error_signal(
        PlayFabRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message,
        const Variant &p_data) {
    if (p_runtime != nullptr) {
        return p_runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
    }

    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::error_result(p_hresult, p_code, p_message, p_data));
    return pending_signal->get_completed_signal();
}

bool get_request_value(const Dictionary &p_request, const char *p_field_name, const char *p_snake_name, Variant *r_value) {
    if (r_value == nullptr) {
        return false;
    }

    const StringName snake_name(p_snake_name);
    if (p_request.has(snake_name)) {
        *r_value = p_request[snake_name];
        return true;
    }

    const StringName field_name(p_field_name);
    if (p_request.has(field_name)) {
        *r_value = p_request[field_name];
        return true;
    }

    return false;
}

String variant_to_json_string(const Variant &p_value) {
    return JSON::stringify(p_value);
}

} // namespace playfab_generated
} // namespace godot
