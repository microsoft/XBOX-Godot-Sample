#ifndef GDK_STORE_H
#define GDK_STORE_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <vector>

#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <XStore.h>

namespace godot {

class GDK;
class GDKPendingSignal;
class GDKResult;
class GDKRuntime;
class GDKUser;

class GDKStoreLicenseStatus : public RefCounted {
    GDCLASS(GDKStoreLicenseStatus, RefCounted);

    String m_store_id;
    String m_licensable_sku;
    int64_t m_status = 0;

protected:
    static void _bind_methods();

public:
    String get_store_id() const;
    String get_licensable_sku() const;
    int64_t get_status() const;

    void set_values(const String &p_store_id, const String &p_licensable_sku, int64_t p_status);
};

class GDKStore : public RefCounted {
    GDCLASS(GDKStore, RefCounted);

    struct CachedLicenseStatus {
        String store_id;
        Ref<GDKStoreLicenseStatus> status;
    };

    GDK *m_owner = nullptr;
    bool m_runtime_ready = false;
    XStoreContextHandle m_store_context = nullptr;
    std::vector<CachedLicenseStatus> m_cached_license_status;

    GDKRuntime *_get_runtime() const;
    void _close_store_context();
    XStoreContextHandle _get_or_create_store_context(HRESULT &r_hresult);
    Signal _make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data = Variant()) const;
    Signal _start_license_status_async(const Ref<GDKUser> &p_user, const String &p_store_id, bool p_is_refresh);
    static String _normalize_store_id(const String &p_store_id);

protected:
    static void _bind_methods();

public:
    ~GDKStore() override;

    void set_owner(GDK *p_owner);

    Ref<GDKResult> on_runtime_initialized();
    void shutdown();
    bool is_runtime_ready() const;
    void on_user_removed(const Ref<GDKUser> &p_user);

    Signal query_license_status_async(const Ref<GDKUser> &p_user, const String &p_store_id);
    Signal refresh_entitlements_async(const Ref<GDKUser> &p_user, const String &p_store_id);
    Signal show_purchase_ui_async(const Ref<GDKUser> &p_user, const String &p_store_id);

    Ref<GDKStoreLicenseStatus> get_cached_license_status(const String &p_store_id) const;
    Ref<GDKResult> check_cached_license_status(const String &p_store_id) const;
    Ref<GDKStoreLicenseStatus> _find_cached_license_status(const String &p_store_id) const;
    Ref<GDKStoreLicenseStatus> _cache_license_status(const Ref<GDKStoreLicenseStatus> &p_status);
};

} // namespace godot

#endif // GDK_STORE_H
