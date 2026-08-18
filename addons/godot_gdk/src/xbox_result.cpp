#include "xbox_result.h"

#include "xbox_result_codes_internal.h"

namespace godot {

void XboxResult::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_ok"), &XboxResult::is_ok);
    ClassDB::bind_method(D_METHOD("get_hresult"), &XboxResult::get_hresult);
    ClassDB::bind_method(D_METHOD("get_code"), &XboxResult::get_code);
    ClassDB::bind_method(D_METHOD("get_message"), &XboxResult::get_message);
    ClassDB::bind_method(D_METHOD("get_data"), &XboxResult::get_data);

    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "ok"), "", "is_ok");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "hresult"), "", "get_hresult");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "code"), "", "get_code");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "message"), "", "get_message");
    ADD_PROPERTY(PropertyInfo(Variant::NIL, "data"), "", "get_data");
}

bool XboxResult::is_ok() const {
    return m_ok;
}

int64_t XboxResult::get_hresult() const {
    return m_hresult;
}

String XboxResult::get_code() const {
    return m_code;
}

String XboxResult::get_message() const {
    return m_message;
}

Variant XboxResult::get_data() const {
    return m_data;
}

void XboxResult::set_values(bool p_ok, HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    m_ok = p_ok;
    m_hresult = static_cast<int64_t>(p_hresult);
    m_code = p_code;
    m_message = p_message;
    m_data = p_data;
}

Ref<XboxResult> XboxResult::ok_result(const Variant &p_data) {
    Ref<XboxResult> result;
    result.instantiate();
    result->set_values(true, S_OK, "ok", "", p_data);
    return result;
}

Ref<XboxResult> XboxResult::error_result(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    Ref<XboxResult> result;
    result.instantiate();
    result->set_values(false, p_hresult, p_code, p_message, p_data);
    return result;
}

Ref<XboxResult> XboxResult::hresult_error(HRESULT p_hresult, const String &p_action, const String &p_code, const Variant &p_data) {
    return error_result(
        p_hresult,
        xbox_internal::code_or_format_hresult(p_code, p_hresult),
        xbox_internal::format_hresult_message(p_action, p_hresult),
        p_data);
}

Ref<XboxResult> XboxResult::cancelled(const String &p_message) {
    return error_result(E_ABORT, "cancelled", p_message);
}

String XboxResult::format_hresult(HRESULT p_hresult) {
    return xbox_internal::format_hresult_string(p_hresult);
}

} // namespace godot
