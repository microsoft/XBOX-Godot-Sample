// fuzz_playfab_request_dictionary.cpp — Phase 3 libFuzzer target.
//
// Fuzzes playfab_api::get_request_value() — the function that resolves a
// field from a GDScript request Dictionary by trying the snake_case key first
// and falling back to the PascalCase key.
//
// This is interesting to fuzz because the production code path goes through
// StringName construction + Dictionary::has() lookup for every PlayFab API
// call that reads a request parameter.
//
// Input layout:
//   [0]          field_name length (1 byte, 0..63)
//   [1..N]       field_name bytes (PascalCase key)
//   [N+1]        snake_name length (1 byte, 0..63)
//   [N+2..M]     snake_name bytes (snake_case key)
//   remaining    Dictionary entries: repeated { key_len(1), key(key_len),
//                                               val_type(1), val_data(4) }

#include "godot_stub_init.h"

#include "playfab_request_value.h"

#include <cstdint>
#include <cstring>

namespace {

size_t decode_cstr(const uint8_t *data, size_t remaining,
                   char *out_buf, size_t out_max, size_t cap = 63) {
    if (remaining == 0) { out_buf[0] = '\0'; return 0; }
    size_t len = static_cast<size_t>(data[0]) & 0x3F; // 0..63
    if (cap < 63) len = len > cap ? cap : len;
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
    if (size < 4) return 0;

    size_t pos = 0;

    // Decode field names
    char field_name[65] = {};
    char snake_name[65] = {};
    pos += decode_cstr(data + pos, size - pos, field_name, sizeof(field_name));
    if (pos >= size) return 0;
    pos += decode_cstr(data + pos, size - pos, snake_name, sizeof(snake_name));

    // Build a Dictionary from the remaining fuzz bytes.
    // Each entry: key_len(1) + key_bytes + val_type(1) + val_data(4)
    godot::Dictionary dict;

    while (pos + 6 <= size) {
        // Key string
        char key_buf[65] = {};
        size_t consumed = decode_cstr(data + pos, size - pos, key_buf, sizeof(key_buf));
        pos += consumed;
        if (pos >= size) break;

        // Value: 1-byte type selector + 4-byte payload
        const uint8_t vtype = data[pos++];
        if (pos + 4 > size) break;
        uint32_t payload;
        ::memcpy(&payload, data + pos, 4);
        pos += 4;

        godot::Variant val;
        switch (vtype & 0x03) {
        case 0: /* nil */ break;
        case 1: val = godot::Variant(static_cast<int64_t>(payload)); break;
        case 2: {
            float f;
            ::memcpy(&f, &payload, 4);
            val = godot::Variant(static_cast<double>(f));
            break;
        }
        case 3: val = godot::Variant(static_cast<bool>(payload & 1)); break;
        }

        dict[godot::String(key_buf)] = val;
    }

    // Exercise the priority-lookup logic with both key orderings.
    godot::Variant out_val;
    bool found = godot::playfab_api::get_request_value(dict, field_name, snake_name, &out_val);
    (void)found;

    // Also test with swapped order to exercise both branches.
    godot::Variant out_val2;
    bool found2 = godot::playfab_api::get_request_value(dict, snake_name, field_name, &out_val2);
    (void)found2;

    return 0;
}
