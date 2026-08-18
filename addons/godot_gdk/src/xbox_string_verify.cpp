#include "xbox_string_verify.h"

#include <utility>
#include <vector>

#include "xbox.h"
#include "xbox_pending_signal.h"
#include "xbox_result.h"
#include "xbox_runtime.h"
#include "xbox_signal_xasync_context.h"
#include "xbox_user.h"
#include "xbox_services.h"

namespace godot {

namespace {

String _utf8_or_empty(const char *p_value) {
    if (p_value == nullptr || p_value[0] == '\0') {
        return String();
    }
    return String::utf8(p_value);
}

String _verify_result_code_to_string(XblVerifyStringResultCode p_result_code) {
    switch (p_result_code) {
        case XblVerifyStringResultCode::Success:
            return "success";
        case XblVerifyStringResultCode::Offensive:
            return "offensive";
        case XblVerifyStringResultCode::TooLong:
            return "too_long";
        case XblVerifyStringResultCode::UnknownError:
        default:
            return "unknown_error";
    }
}

Dictionary _make_verify_result_dictionary(const XblVerifyStringResult &p_result) {
    Dictionary result;
    result["result_code"] = _verify_result_code_to_string(p_result.resultCode);
    result["acceptable"] = p_result.resultCode == XblVerifyStringResultCode::Success;
    result["first_offending_substring"] = _utf8_or_empty(p_result.firstOffendingSubstring);
    return result;
}

Array _make_verify_result_array(const XblVerifyStringResult *p_results, size_t p_count) {
    Array result;
    if (p_results == nullptr) {
        return result;
    }
    for (size_t i = 0; i < p_count; ++i) {
        result.push_back(_make_verify_result_dictionary(p_results[i]));
    }
    return result;
}

class StringVerifyAsyncContext final : public XboxSignalXAsyncContext {
    XblContextHandle m_context = nullptr;
    bool m_batch = false;
    String m_text;
    CharString m_text_utf8;
    PackedStringArray m_strings;
    std::vector<CharString> m_string_utf8_values;
    std::vector<const char *> m_string_ptrs;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<XboxResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = XboxResult::cancelled("String verification cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        size_t result_size = 0;
        HRESULT result_hr = m_batch ?
                XblStringVerifyStringsResultSize(p_async_block, &result_size) :
                XblStringVerifyStringResultSize(p_async_block, &result_size);
        if (result_hr == E_ABORT) {
            result = XboxResult::cancelled("String verification cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(result_hr)) {
            result = XboxResult::hresult_error(result_hr, "Failed to retrieve string verification result size.", "string_verify_result_size_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<uint8_t> buffer(result_size);
        size_t buffer_used = 0;
        if (m_batch) {
            XblVerifyStringResult *native_results = nullptr;
            size_t result_count = 0;
            result_hr = XblStringVerifyStringsResult(
                    p_async_block,
                    buffer.size(),
                    buffer.empty() ? nullptr : buffer.data(),
                    &native_results,
                    &result_count,
                    &buffer_used);
            if (FAILED(result_hr)) {
                result = result_hr == E_ABORT ? XboxResult::cancelled("String verification cancelled.") : XboxResult::hresult_error(result_hr, "Failed to retrieve string verification results.", "string_verify_results_failed");
                get_pending_signal()->complete(result);
                return;
            }

            get_pending_signal()->complete(XboxResult::ok_result(_make_verify_result_array(native_results, result_count)));
            return;
        }

        XblVerifyStringResult *native_result = nullptr;
        result_hr = XblStringVerifyStringResult(
                p_async_block,
                buffer.size(),
                buffer.empty() ? nullptr : buffer.data(),
                &native_result,
                &buffer_used);
        if (FAILED(result_hr)) {
            result = result_hr == E_ABORT ? XboxResult::cancelled("String verification cancelled.") : XboxResult::hresult_error(result_hr, "Failed to retrieve string verification result.", "string_verify_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary payload = native_result == nullptr ? Dictionary() : _make_verify_result_dictionary(*native_result);
        get_pending_signal()->complete(XboxResult::ok_result(payload));
    }

public:
    StringVerifyAsyncContext(XboxRuntime *p_runtime, const Ref<XboxPendingSignal> &p_pending_signal, XblContextHandle p_context, const String &p_text) :
            XboxSignalXAsyncContext(p_runtime, p_pending_signal),
            m_context(p_context),
            m_text(p_text) {
        m_text_utf8 = m_text.utf8();
    }

    StringVerifyAsyncContext(XboxRuntime *p_runtime, const Ref<XboxPendingSignal> &p_pending_signal, XblContextHandle p_context, const PackedStringArray &p_strings) :
            XboxSignalXAsyncContext(p_runtime, p_pending_signal),
            m_context(p_context),
            m_batch(true),
            m_strings(p_strings) {
        m_string_utf8_values.reserve(static_cast<size_t>(m_strings.size()));
        m_string_ptrs.reserve(static_cast<size_t>(m_strings.size()));
        for (int64_t i = 0; i < m_strings.size(); ++i) {
            m_string_utf8_values.push_back(m_strings[i].utf8());
        }
        for (const CharString &utf8 : m_string_utf8_values) {
            m_string_ptrs.push_back(utf8.get_data());
        }
    }

    ~StringVerifyAsyncContext() override {
        if (m_context != nullptr) {
            XblContextCloseHandle(m_context);
            m_context = nullptr;
        }
    }

    XblContextHandle get_context() const {
        return m_context;
    }

    const char *get_text() const {
        return m_text_utf8.get_data();
    }

    const char **get_strings_data() {
        return m_string_ptrs.data();
    }

    uint64_t get_strings_count() const {
        return static_cast<uint64_t>(m_string_ptrs.size());
    }
};

} // namespace

void XboxStringVerify::_bind_methods() {
    ClassDB::bind_method(D_METHOD("verify_string_async", "user", "text"), &XboxStringVerify::verify_string_async);
    ClassDB::bind_method(D_METHOD("verify_strings_async", "user", "strings"), &XboxStringVerify::verify_strings_async);
}

void XboxStringVerify::set_owner(Xbox *p_owner) {
    m_owner = p_owner;
}

XboxRuntime *XboxStringVerify::_get_runtime() const {
    return m_owner == nullptr ? nullptr : m_owner->get_runtime();
}

XboxServices *XboxStringVerify::_get_xbox_services() const {
    return m_owner == nullptr ? nullptr : m_owner->get_xbox_services();
}

Signal XboxStringVerify::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) const {
    XboxRuntime *runtime = _get_runtime();
    ERR_FAIL_NULL_V(runtime, Signal());
    return runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
}

Ref<XboxResult> XboxStringVerify::_ensure_ready_user(const Ref<XboxUser> &p_user) const {
    if (!m_runtime_ready) {
        return XboxResult::error_result(E_FAIL, "runtime_unavailable", "GDK runtime is not initialized.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr || !p_user->is_signed_in()) {
        return XboxResult::error_result(E_INVALIDARG, "invalid_user", "A signed-in XboxUser is required.");
    }
    return XboxResult::ok_result();
}

Ref<XboxResult> XboxStringVerify::on_runtime_initialized() {
    m_runtime_ready = true;
    return XboxResult::ok_result();
}

void XboxStringVerify::shutdown() {
    m_runtime_ready = false;
}

Signal XboxStringVerify::verify_string_async(const Ref<XboxUser> &p_user, const String &p_text) {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr) {
        return Signal();
    }

    Ref<XboxResult> validation = _ensure_ready_user(p_user);
    if (!validation->is_ok()) {
        return _make_error_signal(static_cast<HRESULT>(validation->get_hresult()), validation->get_code(), validation->get_message());
    }

    XboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return _make_error_signal(E_FAIL, "xbox_services_not_initialized", "Xbox services are unavailable. Ensure the title has a TitleId before using string verification.");
    }

    XblContextHandle context = nullptr;
    HRESULT hr = xbox_services->duplicate_context_for_user(p_user, &context);
    if (FAILED(hr)) {
        return _make_error_signal(hr, "xbox_context_unavailable", "Failed to create an Xbox services context for the user.");
    }

    Ref<XboxPendingSignal> pending_signal = runtime->make_pending_signal();
    StringVerifyAsyncContext *async_context = new StringVerifyAsyncContext(runtime, pending_signal, context, p_text);
    async_context->bind_cancel_handler();
    hr = XblStringVerifyStringAsync(async_context->get_context(), async_context->get_text(), async_context->get_async_block());
    if (FAILED(hr)) {
        async_context->clear_cancel_handler();
        delete async_context;
        return _make_error_signal(hr, "string_verify_start_failed", "Failed to start string verification.");
    }
    return pending_signal->get_completed_signal();
}

Signal XboxStringVerify::verify_strings_async(const Ref<XboxUser> &p_user, const PackedStringArray &p_strings) {
    XboxRuntime *runtime = _get_runtime();
    if (runtime == nullptr) {
        return Signal();
    }

    Ref<XboxResult> validation = _ensure_ready_user(p_user);
    if (!validation->is_ok()) {
        return _make_error_signal(static_cast<HRESULT>(validation->get_hresult()), validation->get_code(), validation->get_message());
    }

    if (p_strings.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_strings", "At least one string is required.");
    }

    XboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return _make_error_signal(E_FAIL, "xbox_services_not_initialized", "Xbox services are unavailable. Ensure the title has a TitleId before using string verification.");
    }

    XblContextHandle context = nullptr;
    HRESULT hr = xbox_services->duplicate_context_for_user(p_user, &context);
    if (FAILED(hr)) {
        return _make_error_signal(hr, "xbox_context_unavailable", "Failed to create an Xbox services context for the user.");
    }

    Ref<XboxPendingSignal> pending_signal = runtime->make_pending_signal();
    StringVerifyAsyncContext *async_context = new StringVerifyAsyncContext(runtime, pending_signal, context, p_strings);
    async_context->bind_cancel_handler();
    hr = XblStringVerifyStringsAsync(async_context->get_context(), async_context->get_strings_data(), async_context->get_strings_count(), async_context->get_async_block());
    if (FAILED(hr)) {
        async_context->clear_cancel_handler();
        delete async_context;
        return _make_error_signal(hr, "string_verify_start_failed", "Failed to start batch string verification.");
    }
    return pending_signal->get_completed_signal();
}

void XboxStringVerify::on_user_removed(const Ref<XboxUser> &p_user) {
    (void)p_user;
}

} // namespace godot
