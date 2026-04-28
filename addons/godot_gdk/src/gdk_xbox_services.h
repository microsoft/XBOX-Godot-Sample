#ifndef GDK_XBOX_SERVICES_H
#define GDK_XBOX_SERVICES_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XGame.h>
#include <XUser.h>
#include <xsapi-c/services_c.h>

namespace godot {

class GDKResult;
class GDKUser;

class GDKXboxServices {
    struct UserContextState {
        XUserLocalId local_id = {};
        uint64_t xbox_user_id = 0;
        XblContextHandle context = nullptr;
    };

    bool m_initialized = false;
    uint32_t m_title_id = 0;
    String m_scid;
    std::vector<UserContextState> m_user_contexts;

    static String _build_default_scid(uint32_t p_title_id);
    static String _extract_scid_override(const Variant &p_config);
    UserContextState *_find_user_context(XUserLocalId p_local_id);
    HRESULT _ensure_user_context(const Ref<GDKUser> &p_user, UserContextState **r_context_state);

public:
    GDKXboxServices() = default;
    ~GDKXboxServices();

    Ref<GDKResult> initialize(XTaskQueueHandle p_queue, const Variant &p_config);
    void shutdown();

    bool is_initialized() const;
    uint32_t get_title_id() const;
    String get_scid() const;

    HRESULT get_xbox_user_id(const Ref<GDKUser> &p_user, uint64_t *r_xbox_user_id);
    HRESULT duplicate_context_for_user(const Ref<GDKUser> &p_user, XblContextHandle *r_context, uint64_t *r_xbox_user_id = nullptr);
    void forget_user(XUserLocalId p_local_id);
};

} // namespace godot

#endif // GDK_XBOX_SERVICES_H
