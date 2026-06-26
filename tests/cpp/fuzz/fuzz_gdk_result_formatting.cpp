// fuzz_gdk_result_formatting.cpp — Phase 3 libFuzzer target.
//
// Fuzzes the HRESULT-formatting helpers from gdk_result_codes_internal and
// playfab_result_codes_internal.  These functions return godot::String values
// so they require the Phase 3 stub harness.
//
// Call pattern:
//   LLVMFuzzerInitialize — call godot_stub::init() exactly once.
//   LLVMFuzzerTestOneInput — decode fuzz bytes as an HRESULT + optional
//   action string, then exercise the three formatting entry points.

#include "godot_stub_init.h"

#include "gdk_result_codes_internal.h"
#include "playfab_result_codes_internal.h"

#include <cstdint>
#include <cstring>

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
    godot_stub::init();
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 4) return 0;

    // First 4 bytes → HRESULT value
    HRESULT hr;
    ::memcpy(&hr, data, 4);

    // Remaining bytes → null-terminated action string (limit to 256 chars)
    const size_t action_len = (size - 4) < 256 ? (size - 4) : 256;
    char action_buf[257] = {};
    if (action_len > 0) {
        ::memcpy(action_buf, data + 4, action_len);
        // Ensure null termination (may already be embedded in fuzz bytes)
        action_buf[action_len] = '\0';
    }

    // Build a godot::String from the action buffer (stub harness handles conversion)
    godot::String action = godot::String(action_buf);

    // Exercise gdk_internal formatting
    godot::String hex_str = gdk_internal::format_hresult_string(hr);
    godot::String msg_str = gdk_internal::format_hresult_message(action, hr);
    godot::String code_or = gdk_internal::code_or_format_hresult(action, hr);

    // Exercise playfab_internal formatting
    godot::String pf_hex = playfab_internal::format_hresult_string(hr);
    godot::String pf_msg = playfab_internal::format_hresult_message(action, hr);
    godot::String pf_code_or = playfab_internal::code_or_format_hresult(action, hr);

    // Prevent the compiler from optimizing away the results
    (void)hex_str; (void)msg_str; (void)code_or;
    (void)pf_hex;  (void)pf_msg;  (void)pf_code_or;

    return 0;
}
