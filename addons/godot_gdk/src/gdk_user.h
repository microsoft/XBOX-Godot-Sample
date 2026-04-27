#ifndef GDK_USER_H
#define GDK_USER_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>

#include <XUser.h>
#include <XTaskQueue.h>

namespace godot {

// Opaque wrapper around XUserHandle — never exposes raw handle to GDScript
class GDKUserInfo : public RefCounted {
    GDCLASS(GDKUserInfo, RefCounted);

    XUserHandle m_user_handle = nullptr;
    String m_gamertag;
    uint64_t m_xuid = 0;
    bool m_handle_owned = false; // tracks if we own the handle (for duplication)

protected:
    static void _bind_methods();

public:
    GDKUserInfo();
    ~GDKUserInfo();

    // Internal: takes ownership of handle (will close on destruction)
    void set_from_handle(XUserHandle handle);
    // Internal: duplicate handle for safe sharing
    XUserHandle get_handle() const { return m_user_handle; }

    String get_gamertag() const;
    uint64_t get_xuid() const;
    bool is_valid() const;
    void invalidate();
};

// Singleton managing user sign-in lifecycle
class GDKUserManager : public Object {
    GDCLASS(GDKUserManager, Object);

    static GDKUserManager *singleton;
    Ref<GDKUserInfo> m_current_user;
    bool m_sign_in_pending = false;
    bool m_silent_attempt = false; // tracks if current attempt was silent (for fallback)
    bool m_picture_pending = false;
    XTaskQueueRegistrationToken m_change_token = {};
    bool m_change_registered = false;

    void _store_user(Ref<GDKUserInfo> user);
    void _clear_user();
    void _register_change_event();
    void _unregister_change_event();

protected:
    static void _bind_methods();

public:
    static GDKUserManager *get_singleton();

    GDKUserManager();
    ~GDKUserManager();

    void sign_in();
    void sign_in_silently();
    void sign_out();
    void get_gamer_picture();
    Ref<GDKUserInfo> get_current_user() const;
    bool is_signed_in() const;
    bool is_sign_in_pending() const;

    // Called from async callbacks (main thread via task queue)
    void _on_sign_in_complete(XUserHandle handle, HRESULT hr, bool was_silent);
    void _on_gamer_picture_complete(XAsyncBlock *async);
    void _on_user_change(XUserLocalId user_local_id, XUserChangeEvent event);
};

} // namespace godot

#endif // GDK_USER_H
