#ifndef GODOT_PLAYFAB_OWNED_UTF8_H
#define GODOT_PLAYFAB_OWNED_UTF8_H

#include <string>

namespace playfab_internal {

class OwnedUtf8String final {
    std::string m_value;

public:
    OwnedUtf8String() = default;

    explicit OwnedUtf8String(const char *p_value) {
        set(p_value);
    }

    void set(const char *p_value) {
        m_value = p_value != nullptr ? p_value : "";
    }

    const char *c_str() const noexcept {
        return m_value.c_str();
    }

    const char *c_str_or_null() const noexcept {
        return m_value.empty() ? nullptr : m_value.c_str();
    }

    const std::string &str() const noexcept {
        return m_value;
    }
};

} // namespace playfab_internal

#endif // GODOT_PLAYFAB_OWNED_UTF8_H
