#ifndef GDK_RESULT_H
#define GDK_RESULT_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

namespace godot {

class GDKResult : public RefCounted {
    GDCLASS(GDKResult, RefCounted);

    bool m_ok = false;
    int64_t m_hresult = 0;
    String m_code;
    String m_message;
    Variant m_data;

protected:
    static void _bind_methods();

public:
    bool is_ok() const;
    int64_t get_hresult() const;
    String get_code() const;
    String get_message() const;
    Variant get_data() const;

    void set_values(bool p_ok, HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());

    static Ref<GDKResult> ok_result(const Variant &p_data = Variant());
    static Ref<GDKResult> error_result(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant());
    static Ref<GDKResult> hresult_error(HRESULT p_hresult, const String &p_action, const String &p_code = String(), const Variant &p_data = Variant());
    static Ref<GDKResult> cancelled(const String &p_message = "Operation cancelled.");
    static String format_hresult(HRESULT p_hresult);
};

} // namespace godot

#endif // GDK_RESULT_H
