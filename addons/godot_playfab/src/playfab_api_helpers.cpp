#include "playfab_api_helpers.h"

#include <godot_cpp/classes/json.hpp>

#include "playfab_request_key.h"
#include "playfab_request_value.h"

namespace godot {
namespace playfab_api {

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

String variant_to_json_string(const Variant &p_value) {
    return JSON::stringify(p_value);
}

} // namespace playfab_api
} // namespace godot

