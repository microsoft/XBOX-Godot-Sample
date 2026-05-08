#ifndef GDK_ERROR_REPORTING_H
#define GDK_ERROR_REPORTING_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

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

class GDK;
class GDKResult;
class GDKRuntime;

class GDKErrorReporting : public RefCounted {
    GDCLASS(GDKErrorReporting, RefCounted);

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

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    bool m_callback_enabled = false;
    std::mutex m_pending_errors_mutex;
    std::vector<PendingError> m_pending_errors;

    static bool _error_callback(HRESULT p_hr, const char *p_message, void *p_context);
    static Ref<GDKResult> _parse_options(int64_t p_options_value, XErrorOptions *r_options);
    Ref<GDKResult> _set_callback(XErrorCallback *p_callback, void *p_context);

protected:
    static void _bind_methods();

public:
    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();
    int dispatch();

    Ref<GDKResult> configure_options(
            int64_t p_debugger_present_options = ERROR_OPTIONS_NONE,
            int64_t p_debugger_not_present_options = ERROR_OPTIONS_NONE);
    Ref<GDKResult> set_callback_enabled(bool p_enabled);
    bool is_callback_enabled() const;

    GDKRuntime *get_runtime_internal() const;
    void push_error_internal(HRESULT p_hr, const char *p_message);
};

} // namespace godot

VARIANT_ENUM_CAST(godot::GDKErrorReporting::ErrorOptions);

#endif // GDK_ERROR_REPORTING_H
