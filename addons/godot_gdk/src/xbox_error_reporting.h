#ifndef XBOX_ERROR_REPORTING_H
#define XBOX_ERROR_REPORTING_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/error_macros.hpp>

#include <XError.h>

namespace godot {

class Xbox;
class XboxResult;
class XboxRuntime;

class XboxErrorReporting : public RefCounted {
    GDCLASS(XboxErrorReporting, RefCounted);

public:
    enum ErrorOptions {
        ERROR_OPTIONS_NONE = 0,
        ERROR_OPTIONS_OUTPUT_DEBUG_STRING_ON_ERROR = static_cast<uint32_t>(XErrorOptions::OutputDebugStringOnError),
        ERROR_OPTIONS_DEBUG_BREAK_ON_ERROR = static_cast<uint32_t>(XErrorOptions::DebugBreakOnError),
        ERROR_OPTIONS_FAIL_FAST_ON_ERROR = static_cast<uint32_t>(XErrorOptions::FailFastOnError),
    };

private:
    struct PendingError {
        HRESULT hr = S_OK;
        std::string message;
    };

    struct CallbackContext {
        XboxErrorReporting *service = nullptr;
        std::atomic_bool active = true;
        std::mutex mutex;
    };

    struct CallbackToken {
        std::weak_ptr<CallbackContext> context;
    };

    Xbox *m_owner = nullptr;
    bool m_runtime_ready = false;
    bool m_callback_enabled = false;
    std::mutex m_pending_errors_mutex;
    std::vector<PendingError> m_pending_errors;
    std::shared_ptr<CallbackContext> m_callback_context;
    std::shared_ptr<CallbackToken> m_callback_token;
    std::vector<std::shared_ptr<CallbackToken>> m_retired_callback_tokens;

    static bool _error_callback(HRESULT p_hr, const char *p_message, void *p_context);
    static Ref<XboxResult> _parse_options(int64_t p_options_value, XErrorOptions *r_options);
    void _clear_callback_context();
    Ref<XboxResult> _set_callback(XErrorCallback *p_callback);

protected:
    static void _bind_methods();

public:
    void set_owner(Xbox *p_owner);

    Ref<XboxResult> on_runtime_initialized();
    void shutdown();
    int dispatch();

    Ref<XboxResult> configure_options(
            int64_t p_debugger_present_options = ERROR_OPTIONS_NONE,
            int64_t p_debugger_not_present_options = ERROR_OPTIONS_NONE);
    Ref<XboxResult> set_callback_enabled(bool p_enabled);
    bool is_callback_enabled() const;

    XboxRuntime *get_runtime_internal() const;
    void push_error_internal(HRESULT p_hr, const char *p_message);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::XboxErrorReporting::ErrorOptions);

#endif // XBOX_ERROR_REPORTING_H
