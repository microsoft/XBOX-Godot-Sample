#include "gdk_error_reporting.h"

#include <cstdint>

#include <godot_cpp/variant/dictionary.hpp>

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_runtime.h"

namespace godot {

namespace {
constexpr const char *RUNTIME_NOT_INITIALIZED_ERROR_MESSAGE = "GDK runtime must be initialized before configuring error reporting.";
constexpr const char *ERROR_OPTIONS_OUT_OF_RANGE_MESSAGE = "Error reporting options value is out of valid range.";
constexpr const char *ERROR_OPTIONS_UNSUPPORTED_BITS_MESSAGE = "Error reporting options contain unsupported flag bits.";
}

void GDKErrorReporting::_bind_methods() {
    ClassDB::bind_method(D_METHOD("configure_options", "debugger_present_options", "debugger_not_present_options"), &GDKErrorReporting::configure_options, DEFVAL(static_cast<int64_t>(ERROR_OPTIONS_NONE)), DEFVAL(static_cast<int64_t>(ERROR_OPTIONS_NONE)));
    ClassDB::bind_method(D_METHOD("set_callback_enabled", "enabled"), &GDKErrorReporting::set_callback_enabled);
    ClassDB::bind_method(D_METHOD("is_callback_enabled"), &GDKErrorReporting::is_callback_enabled);

    BIND_ENUM_CONSTANT(ERROR_OPTIONS_NONE);
    BIND_ENUM_CONSTANT(ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR);
    BIND_ENUM_CONSTANT(ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR);
    BIND_ENUM_CONSTANT(ERROR_OPTIONS_FAIL_FAST_ON_ERROR);

    ADD_SIGNAL(MethodInfo("error_reported", PropertyInfo(Variant::OBJECT, "result")));
}

void GDKErrorReporting::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKErrorReporting::on_runtime_initialized() {
    XErrorSetCallback(nullptr, nullptr);
    _clear_callback_context();

    std::lock_guard<std::mutex> lock(m_pending_errors_mutex);
    m_pending_errors.clear();
    m_runtime_ready = true;
    m_callback_enabled = false;
    return GDKResult::ok_result();
}

void GDKErrorReporting::shutdown() {
    if (m_runtime_ready) {
        _set_callback(nullptr);
    }

    std::lock_guard<std::mutex> lock(m_pending_errors_mutex);
    m_pending_errors.clear();
    m_runtime_ready = false;
    m_callback_enabled = false;
}

int GDKErrorReporting::dispatch() {
    std::vector<PendingError> pending_errors;
    {
        std::lock_guard<std::mutex> lock(m_pending_errors_mutex);
        if (m_pending_errors.empty()) {
            return 0;
        }
        pending_errors.swap(m_pending_errors);
    }

    for (const PendingError &pending_error : pending_errors) {
        const char *native_message_chars = pending_error.message.empty() ? nullptr : pending_error.message.c_str();
        const String native_message = native_message_chars != nullptr ? String::utf8(native_message_chars) : String();
        Dictionary data;
        data["native_message"] = native_message;

        Ref<GDKResult> result = GDKResult::hresult_error(
                pending_error.hr,
                "GDK runtime error callback invoked.",
                "xerror_callback_error",
                data);

        if (m_owner != nullptr) {
            m_owner->emit_runtime_error(result);
        }
        emit_signal("error_reported", result);
    }

    return static_cast<int>(pending_errors.size());
}

Ref<GDKResult> GDKErrorReporting::configure_options(
        int64_t p_debugger_present_options,
        int64_t p_debugger_not_present_options) {
    XErrorOptions debugger_present_options = XErrorOptions::None;
    Ref<GDKResult> debugger_present_parse_result = _parse_options(p_debugger_present_options, &debugger_present_options);
    if (!debugger_present_parse_result->is_ok()) {
        return debugger_present_parse_result;
    }

    XErrorOptions debugger_not_present_options = XErrorOptions::None;
    Ref<GDKResult> debugger_not_present_parse_result = _parse_options(p_debugger_not_present_options, &debugger_not_present_options);
    if (!debugger_not_present_parse_result->is_ok()) {
        return debugger_not_present_parse_result;
    }

    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "runtime_not_initialized", RUNTIME_NOT_INITIALIZED_ERROR_MESSAGE);
    }

    XErrorSetOptions(debugger_present_options, debugger_not_present_options);
    GDKRuntime *runtime = get_runtime_internal();
    if (runtime != nullptr) {
    }

    return GDKResult::ok_result();
}

Ref<GDKResult> GDKErrorReporting::set_callback_enabled(bool p_enabled) {
    XErrorCallback *callback = p_enabled ? _error_callback : nullptr;
    Ref<GDKResult> result = _set_callback(callback);
    if (result->is_ok()) {
        m_callback_enabled = p_enabled;
    }
    return result;
}

void GDKErrorReporting::_clear_callback_context() {
    if (m_callback_context) {
        std::lock_guard<std::mutex> lock(m_callback_context->mutex);
        m_callback_context->active.store(false, std::memory_order_release);
        m_callback_context->service = nullptr;
    }

    if (m_callback_token) {
        constexpr size_t MAX_RETIRED_CALLBACK_TOKENS = 16;
        m_retired_callback_tokens.push_back(m_callback_token);
        if (m_retired_callback_tokens.size() > MAX_RETIRED_CALLBACK_TOKENS) {
            m_retired_callback_tokens.erase(m_retired_callback_tokens.begin());
        }
        m_callback_token.reset();
    }
    m_callback_context.reset();
}

Ref<GDKResult> GDKErrorReporting::_set_callback(XErrorCallback *p_callback) {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "runtime_not_initialized", RUNTIME_NOT_INITIALIZED_ERROR_MESSAGE);
    }

    if (p_callback == nullptr) {
        _clear_callback_context();
        XErrorSetCallback(nullptr, nullptr);
        return GDKResult::ok_result();
    }

    _clear_callback_context();
    m_callback_context = std::make_shared<CallbackContext>();
    m_callback_context->service = this;
    m_callback_token = std::make_shared<CallbackToken>();
    m_callback_token->context = m_callback_context;
    XErrorSetCallback(p_callback, m_callback_token.get());

    return GDKResult::ok_result();
}

bool GDKErrorReporting::is_callback_enabled() const {
    return m_callback_enabled;
}

GDKRuntime *GDKErrorReporting::get_runtime_internal() const {
    if (m_owner == nullptr) {
        return nullptr;
    }
    return m_owner->get_runtime();
}

void GDKErrorReporting::push_error_internal(HRESULT p_hr, const char *p_message) {
    std::lock_guard<std::mutex> lock(m_pending_errors_mutex);
    m_pending_errors.push_back({ p_hr, p_message != nullptr ? p_message : "" });
}

bool GDKErrorReporting::_error_callback(HRESULT p_hr, const char *p_message, void *p_context) {
    CallbackToken *token = static_cast<CallbackToken *>(p_context);
    std::shared_ptr<CallbackContext> callback_context = token != nullptr ? token->context.lock() : nullptr;
    if (!callback_context || !callback_context->active.load(std::memory_order_acquire)) {
        return true;
    }

    std::lock_guard<std::mutex> context_lock(callback_context->mutex);
    GDKErrorReporting *service = callback_context->service;
    if (!callback_context->active.load(std::memory_order_acquire) || service == nullptr) {
        return true;
    }

    service->push_error_internal(p_hr, p_message);
    return true;
}

Ref<GDKResult> GDKErrorReporting::_parse_options(int64_t p_options_value, XErrorOptions *r_options) {
    ERR_FAIL_COND_V(r_options == nullptr, GDKResult::error_result(E_POINTER, "internal_error", "Missing XError options output buffer."));

    if (p_options_value < 0 || p_options_value > static_cast<int64_t>(INT32_MAX)) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_error_reporting_options", ERROR_OPTIONS_OUT_OF_RANGE_MESSAGE);
    }

    const int32_t options_bits = static_cast<int32_t>(p_options_value);
    const int32_t supported_mask =
            static_cast<int32_t>(ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR) |
            static_cast<int32_t>(ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR) |
            static_cast<int32_t>(ERROR_OPTIONS_FAIL_FAST_ON_ERROR);
    if ((options_bits & ~supported_mask) != 0) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_error_reporting_options", ERROR_OPTIONS_UNSUPPORTED_BITS_MESSAGE);
    }

    *r_options = static_cast<XErrorOptions>(options_bits);
    return GDKResult::ok_result();
}

} // namespace godot
