// fuzz_gdk_result_factories.cpp — Phase 3 libFuzzer target.
//
// Fuzzes the GDKResult and PlayFabResult factory methods (ok_result,
// error_result, hresult_error, cancelled) and their field accessors.
// These classes use Ref<RefCounted> internally, which requires the Phase 3
// stub harness's object lifecycle stubs (stub_object.cpp) to work without
// a live Godot engine.
//
// Input layout (all fuzz bytes consumed):
//   [0..3]   HRESULT value (4 bytes, little-endian)
//   [4]      variant data type byte (0=nil, 1=int, 2=float, 3=bool)
//   [5..8]   int/float payload for variant (4 bytes)
//   [9]      action string length  (1 byte, 0..127)
//   [10..]   action string bytes
//   [10+action_len]      code string length (1 byte, 0..127)
//   [10+action_len+1..]  code string bytes

#include "godot_stub_init.h"

#include "gdk_result.h"
#include "playfab_result.h"

#include <cstdint>
#include <cstring>

namespace {

// Decode a length-prefixed string from the fuzz buffer.
// Returns number of bytes consumed (1 + actual string length).
size_t decode_string(const uint8_t *data, size_t remaining,
                     char *out_buf, size_t out_max) {
    if (remaining == 0) {
        out_buf[0] = '\0';
        return 0;
    }
    size_t len = static_cast<size_t>(data[0] & 0x7F); // cap at 127
    if (len > remaining - 1) len = remaining - 1;
    if (len >= out_max) len = out_max - 1;
    ::memcpy(out_buf, data + 1, len);
    out_buf[len] = '\0';
    return 1 + len;
}

} // namespace

extern "C" int LLVMFuzzerInitialize(int *, char ***) {
    godot_stub::init();
    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 10) return 0;

    // Decode HRESULT
    HRESULT hr;
    ::memcpy(&hr, data, 4);

    // Decode simple Variant payload (nil, int, float, bool)
    const uint8_t vtype = data[4];
    uint32_t vpayload;
    ::memcpy(&vpayload, data + 5, 4);

    godot::Variant v;
    switch (vtype & 0x03) {
    case 0: /* nil */ break;
    case 1: v = godot::Variant(static_cast<int64_t>(vpayload)); break;
    case 2: {
        float f;
        ::memcpy(&f, &vpayload, 4);
        v = godot::Variant(static_cast<double>(f));
        break;
    }
    case 3: v = godot::Variant(static_cast<bool>(vpayload & 1)); break;
    }

    // Decode action and code strings
    char action_buf[128] = {};
    char code_buf[128]   = {};
    size_t pos = 9;
    pos += decode_string(data + pos, size - pos, action_buf, sizeof(action_buf));
    if (pos < size) {
        decode_string(data + pos, size - pos, code_buf, sizeof(code_buf));
    }

    const godot::String action(action_buf);
    const godot::String code(code_buf);

    // ── GDKResult factory + accessor round-trips ──────────────────────────
    {
        godot::Ref<godot::GDKResult> ok = godot::GDKResult::ok_result(v);
        if (ok.is_valid()) {
            (void)ok->is_ok();
            (void)ok->get_hresult();
            (void)ok->get_code();
            (void)ok->get_message();
            (void)ok->get_data();
        }
    }
    {
        godot::Ref<godot::GDKResult> err =
            godot::GDKResult::error_result(hr, code, action, v);
        if (err.is_valid()) {
            (void)err->is_ok();
            (void)err->get_hresult();
            (void)err->get_code();
            (void)err->get_message();
        }
    }
    {
        godot::Ref<godot::GDKResult> herr =
            godot::GDKResult::hresult_error(hr, action, code, v);
        if (herr.is_valid()) {
            (void)herr->get_code();
            (void)herr->get_message();
        }
    }
    {
        godot::Ref<godot::GDKResult> canc =
            godot::GDKResult::cancelled(action);
        if (canc.is_valid()) {
            (void)canc->is_ok();
            (void)canc->get_code();
        }
    }

    // ── PlayFabResult factory + accessor round-trips ──────────────────────
    {
        godot::Ref<godot::PlayFabResult> ok = godot::PlayFabResult::ok_result(v);
        if (ok.is_valid()) {
            (void)ok->is_ok();
            (void)ok->get_hresult();
            (void)ok->get_code();
            (void)ok->get_message();
            (void)ok->get_data();
        }
    }
    {
        godot::Ref<godot::PlayFabResult> err =
            godot::PlayFabResult::error_result(hr, code, action, v);
        if (err.is_valid()) {
            (void)err->is_ok();
            (void)err->get_hresult();
            (void)err->get_code();
        }
    }
    {
        godot::Ref<godot::PlayFabResult> herr =
            godot::PlayFabResult::hresult_error(hr, action, code, v);
        if (herr.is_valid()) {
            (void)herr->get_code();
            (void)herr->get_message();
        }
    }
    {
        godot::Ref<godot::PlayFabResult> canc =
            godot::PlayFabResult::cancelled(action);
        if (canc.is_valid()) {
            (void)canc->is_ok();
            (void)canc->get_code();
        }
    }

    return 0;
}
