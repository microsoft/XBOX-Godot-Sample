#include "gdk_result.h"

#include <cstdio>

namespace godot {

void GDKResult::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_ok"), &GDKResult::is_ok);
    ClassDB::bind_method(D_METHOD("get_hresult"), &GDKResult::get_hresult);
    ClassDB::bind_method(D_METHOD("get_code"), &GDKResult::get_code);
    ClassDB::bind_method(D_METHOD("get_message"), &GDKResult::get_message);
    ClassDB::bind_method(D_METHOD("get_data"), &GDKResult::get_data);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ok"), "", "is_ok");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "hresult"), "", "get_hresult");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "code"), "", "get_code");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "message"), "", "get_message");
    ADD_PROPERTY(PropertyInfo(Variant::NIL, "data"), "", "get_data");
}

bool GDKResult::is_ok() const {
    return m_ok;
}

int64_t GDKResult::get_hresult() const {
    return m_hresult;
}

String GDKResult::get_code() const {
    return m_code;
}

String GDKResult::get_message() const {
    return m_message;
}

Variant GDKResult::get_data() const {
    return m_data;
}

void GDKResult::set_values(bool p_ok, HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    m_ok = p_ok;
    m_hresult = static_cast<int64_t>(p_hresult);
    m_code = p_code;
    m_message = p_message;
    m_data = p_data;
}

Ref<GDKResult> GDKResult::ok_result(const Variant &p_data) {
    Ref<GDKResult> result;
    result.instantiate();
    result->set_values(true, S_OK, "ok", "", p_data);
    return result;
}

Ref<GDKResult> GDKResult::error_result(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    Ref<GDKResult> result;
    result.instantiate();
    result->set_values(false, p_hresult, p_code, p_message, p_data);
    return result;
}

Ref<GDKResult> GDKResult::hresult_error(HRESULT p_hresult, const String &p_action, const String &p_code, const Variant &p_data) {
    String message = p_action;
    if (!message.is_empty()) {
        message += " ";
    }
    message += "(HRESULT " + format_hresult(p_hresult) + ")";

    return error_result(
        p_hresult,
        p_code.is_empty() ? format_hresult(p_hresult) : p_code,
        message,
        p_data);
}

Ref<GDKResult> GDKResult::cancelled(const String &p_message) {
    return error_result(E_ABORT, "cancelled", p_message);
}

String GDKResult::format_hresult(HRESULT p_hresult) {
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "0x%08X", static_cast<unsigned int>(p_hresult));
    return String(buffer);
}

} // namespace godot
