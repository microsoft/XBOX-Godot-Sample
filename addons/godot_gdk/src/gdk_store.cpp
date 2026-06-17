#include "gdk_store.h"

#include <string>

#include <godot_cpp/variant/dictionary.hpp>

#include "gdk.h"
#include "gdk_pending_signal.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_signal_xasync_context.h"
#include "gdk_user.h"

namespace godot {

namespace {

class StoreXAsyncContext : public GDKSignalXAsyncContext {
protected:
    Ref<GDKStore> m_service;
    String m_store_id;
    std::string m_store_id_utf8;

public:
    StoreXAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_store_id) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_service(p_service),
            m_store_id(p_store_id),
            m_store_id_utf8(p_store_id.utf8().get_data()) {}

    const char *get_store_id_utf8() const {
        return m_store_id_utf8.c_str();
    }

    String get_store_id() const {
        return m_store_id;
    }
};

class StoreLicenseStatusAsyncContext final : public StoreXAsyncContext {
    bool m_is_refresh = false;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Store license status request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        XStoreCanAcquireLicenseResult native_result = {};
        HRESULT result_hr = XStoreCanAcquireLicenseForStoreIdResult(p_async_block, &native_result);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Store license status request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    m_is_refresh ? "Failed to refresh store entitlements." : "Failed to query store license status.",
                    m_is_refresh ? "store_entitlements_refresh_result_failed" : "store_license_status_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        if (!m_service.is_valid() || !m_service->is_runtime_ready()) {
            result = GDKResult::cancelled("GDKStore is shutting down.");
            get_pending_signal()->complete(result);
            return;
        }

        Ref<GDKStoreLicenseStatus> status;
        status.instantiate();
        status->set_values(get_store_id(), String::utf8(native_result.licensableSku), static_cast<int64_t>(native_result.status));
        Ref<GDKStoreLicenseStatus> cached_status = m_service->_cache_license_status(status);
        get_pending_signal()->complete(GDKResult::ok_result(cached_status));
    }

public:
    StoreLicenseStatusAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_store_id,
            bool p_is_refresh) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, p_store_id),
            m_is_refresh(p_is_refresh) {}
};

class StorePurchaseUiAsyncContext final : public StoreXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Store purchase UI request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = XStoreShowPurchaseUIResult(p_async_block);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Store purchase UI was dismissed.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the store purchase UI flow.",
                    "store_purchase_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary data;
        data["store_id"] = get_store_id();
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    StorePurchaseUiAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_store_id) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, p_store_id) {}
};

class StoreProductPageUiAsyncContext final : public StoreXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Store product page UI request cancelled."));
            return;
        }

        HRESULT result_hr = XStoreShowProductPageUIResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Store product page UI was dismissed."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the store product page UI flow.",
                    "store_product_page_result_failed"));
            return;
        }

        Dictionary data;
        data["store_id"] = get_store_id();
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    StoreProductPageUiAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_store_id) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, p_store_id) {}
};

class StoreAssociatedProductsUiAsyncContext final : public StoreXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Store associated products UI request cancelled."));
            return;
        }

        HRESULT result_hr = XStoreShowAssociatedProductsUIResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Store associated products UI was dismissed."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the store associated products UI flow.",
                    "store_associated_products_result_failed"));
            return;
        }

        Dictionary data;
        data["store_id"] = get_store_id();
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    StoreAssociatedProductsUiAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_store_id) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, p_store_id) {}
};

class StoreRateAndReviewUiAsyncContext final : public StoreXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Store rate and review UI request cancelled."));
            return;
        }

        XStoreRateAndReviewResult native_result = {};
        HRESULT result_hr = XStoreShowRateAndReviewUIResult(p_async_block, &native_result);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Store rate and review UI was dismissed."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the store rate and review UI flow.",
                    "store_rate_and_review_result_failed"));
            return;
        }

        Dictionary data;
        data["was_updated"] = native_result.wasUpdated;
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    StoreRateAndReviewUiAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, String()) {}
};

class StoreRedeemTokenUiAsyncContext final : public StoreXAsyncContext {
    std::string m_token_utf8;
    std::vector<std::string> m_allowed_store_ids_utf8;
    std::vector<const char *> m_allowed_store_id_ptrs;
    bool m_disallow_csv_redemption = false;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Store redeem token UI request cancelled."));
            return;
        }

        HRESULT result_hr = XStoreShowRedeemTokenUIResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Store redeem token UI was dismissed."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the store redeem token UI flow.",
                    "store_redeem_token_result_failed"));
            return;
        }

        get_pending_signal()->complete(GDKResult::ok_result(Dictionary()));
    }

public:
    StoreRedeemTokenUiAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_token,
            const PackedStringArray &p_allowed_store_ids,
            bool p_disallow_csv_redemption) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, String()),
            m_token_utf8(p_token.utf8().get_data()),
            m_disallow_csv_redemption(p_disallow_csv_redemption) {
        for (int64_t i = 0; i < p_allowed_store_ids.size(); ++i) {
            const String entry = p_allowed_store_ids[i].strip_edges();
            if (!entry.is_empty()) {
                m_allowed_store_ids_utf8.push_back(entry.utf8().get_data());
            }
        }
        m_allowed_store_id_ptrs.reserve(m_allowed_store_ids_utf8.size());
        for (const std::string &id : m_allowed_store_ids_utf8) {
            m_allowed_store_id_ptrs.push_back(id.c_str());
        }
    }

    const char *get_token_utf8() const {
        return m_token_utf8.c_str();
    }

    const char **get_allowed_store_ids() {
        return m_allowed_store_id_ptrs.empty() ? nullptr : m_allowed_store_id_ptrs.data();
    }

    size_t get_allowed_store_ids_count() const {
        return m_allowed_store_id_ptrs.size();
    }

    bool get_disallow_csv_redemption() const {
        return m_disallow_csv_redemption;
    }
};

class StoreGiftingUiAsyncContext final : public StoreXAsyncContext {
    std::string m_name_utf8;
    std::string m_extended_json_utf8;
    bool m_has_name = false;
    bool m_has_extended_json = false;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Store gifting UI request cancelled."));
            return;
        }

        HRESULT result_hr = XStoreShowGiftingUIResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Store gifting UI was dismissed."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the store gifting UI flow.",
                    "store_gifting_result_failed"));
            return;
        }

        Dictionary data;
        data["store_id"] = get_store_id();
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    StoreGiftingUiAsyncContext(
            GDKStore *p_service,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_store_id,
            const String &p_name,
            const String &p_extended_json) :
            StoreXAsyncContext(p_service, p_runtime, p_pending_signal, p_store_id) {
        const String name = p_name.strip_edges();
        if (!name.is_empty()) {
            m_name_utf8 = name.utf8().get_data();
            m_has_name = true;
        }
        const String extended_json = p_extended_json.strip_edges();
        if (!extended_json.is_empty()) {
            m_extended_json_utf8 = extended_json.utf8().get_data();
            m_has_extended_json = true;
        }
    }

    const char *get_name_utf8() const {
        return m_has_name ? m_name_utf8.c_str() : nullptr;
    }

    const char *get_extended_json_utf8() const {
        return m_has_extended_json ? m_extended_json_utf8.c_str() : nullptr;
    }
};

XStoreProductKind parse_product_kinds(const String &p_product_kinds) {
    const String normalized = p_product_kinds.strip_edges().to_lower();
    if (normalized.is_empty()) {
        return XStoreProductKind::Consumable | XStoreProductKind::Durable |
                XStoreProductKind::Game | XStoreProductKind::Pass |
                XStoreProductKind::UnmanagedConsumable;
    }

    XStoreProductKind kinds = XStoreProductKind::None;
    const PackedStringArray tokens = normalized.split(",", false);
    for (int64_t i = 0; i < tokens.size(); ++i) {
        const String token = tokens[i].strip_edges();
        if (token == "consumable") {
            kinds |= XStoreProductKind::Consumable;
        } else if (token == "durable") {
            kinds |= XStoreProductKind::Durable;
        } else if (token == "game") {
            kinds |= XStoreProductKind::Game;
        } else if (token == "pass") {
            kinds |= XStoreProductKind::Pass;
        } else if (token == "unmanaged_consumable" || token == "unmanagedconsumable") {
            kinds |= XStoreProductKind::UnmanagedConsumable;
        }
    }
    return kinds;
}

} // namespace

void GDKStoreLicenseStatus::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_store_id"), &GDKStoreLicenseStatus::get_store_id);
    ClassDB::bind_method(D_METHOD("get_licensable_sku"), &GDKStoreLicenseStatus::get_licensable_sku);
    ClassDB::bind_method(D_METHOD("get_status"), &GDKStoreLicenseStatus::get_status);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "store_id"), "", "get_store_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "licensable_sku"), "", "get_licensable_sku");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "status"), "", "get_status");
}

String GDKStoreLicenseStatus::get_store_id() const {
    return m_store_id;
}

String GDKStoreLicenseStatus::get_licensable_sku() const {
    return m_licensable_sku;
}

int64_t GDKStoreLicenseStatus::get_status() const {
    return m_status;
}

void GDKStoreLicenseStatus::set_values(const String &p_store_id, const String &p_licensable_sku, int64_t p_status) {
    m_store_id = p_store_id;
    m_licensable_sku = p_licensable_sku;
    m_status = p_status;
}

void GDKStore::_bind_methods() {
    ClassDB::bind_method(D_METHOD("query_license_status_async", "user", "store_id"), &GDKStore::query_license_status_async);
    ClassDB::bind_method(D_METHOD("refresh_entitlements_async", "user", "store_id"), &GDKStore::refresh_entitlements_async);
    ClassDB::bind_method(D_METHOD("show_purchase_ui_async", "user", "store_id"), &GDKStore::show_purchase_ui_async);
    ClassDB::bind_method(D_METHOD("show_product_page_ui_async", "user", "store_id"), &GDKStore::show_product_page_ui_async);
    ClassDB::bind_method(D_METHOD("show_associated_products_ui_async", "user", "store_id", "product_kinds"), &GDKStore::show_associated_products_ui_async, DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("show_rate_and_review_ui_async", "user"), &GDKStore::show_rate_and_review_ui_async);
    ClassDB::bind_method(D_METHOD("show_redeem_token_ui_async", "user", "token", "allowed_store_ids", "disallow_csv_redemption"), &GDKStore::show_redeem_token_ui_async, DEFVAL(PackedStringArray()), DEFVAL(false));
    ClassDB::bind_method(D_METHOD("show_gifting_ui_async", "user", "store_id", "name", "extended_json"), &GDKStore::show_gifting_ui_async, DEFVAL(String()), DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("get_cached_license_status", "store_id"), &GDKStore::get_cached_license_status);
    ClassDB::bind_method(D_METHOD("check_cached_license_status", "store_id"), &GDKStore::check_cached_license_status);
}

GDKStore::~GDKStore() {
    shutdown();
}

void GDKStore::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKStore::on_runtime_initialized() {
    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKStore::shutdown() {
    m_runtime_ready = false;
    _close_store_context();
    m_cached_license_status.clear();
}

bool GDKStore::is_runtime_ready() const {
    return m_runtime_ready;
}

void GDKStore::on_user_removed(const Ref<GDKUser> &p_user) {
    (void)p_user;
    m_cached_license_status.clear();
}

Signal GDKStore::query_license_status_async(const Ref<GDKUser> &p_user, const String &p_store_id) {
    return _start_license_status_async(p_user, p_store_id, false);
}

Signal GDKStore::refresh_entitlements_async(const Ref<GDKUser> &p_user, const String &p_store_id) {
    return _start_license_status_async(p_user, p_store_id, true);
}

Signal GDKStore::show_purchase_ui_async(const Ref<GDKUser> &p_user, const String &p_store_id) {
    const String store_id = _normalize_store_id(p_store_id);
    if (store_id.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_product_id", "A non-empty Store product ID is required.");
    }

    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_available()) {
        return _make_error_signal(E_FAIL, "runtime_unavailable", "GDK runtime is unavailable in the current process.");
    }
    if (!m_runtime_ready || !runtime->is_initialized()) {
        return _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    // PC GDK uses the Microsoft Store signed-in account for XStore context and
    // expects a nullptr user handle here. We still validate p_user above so the
    // public Godot API keeps a consistent "signed-in local user required"
    // contract across store calls.
    HRESULT context_hr = S_OK;
    XStoreContextHandle store_context = _get_or_create_store_context(context_hr);
    if (FAILED(context_hr) || store_context == nullptr) {
        return _make_error_signal(context_hr, "store_context_create_failed", "Failed to create an XStore context for the user.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StorePurchaseUiAsyncContext(this, runtime, pending_signal, store_id);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreShowPurchaseUIAsync(store_context, context->get_store_id_utf8(), nullptr, nullptr, context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                start_hr,
                "Failed to start the store purchase UI flow.",
                "store_purchase_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

bool GDKStore::_prepare_store_ui_call(const Ref<GDKUser> &p_user, GDKRuntime *&r_runtime, XStoreContextHandle &r_store_context, Signal &r_error_signal) {
    r_runtime = _get_runtime();
    if (r_runtime == nullptr || !r_runtime->is_available()) {
        r_error_signal = _make_error_signal(E_FAIL, "runtime_unavailable", "GDK runtime is unavailable in the current process.");
        return false;
    }
    if (!m_runtime_ready || !r_runtime->is_initialized()) {
        r_error_signal = _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
        return false;
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        r_error_signal = _make_error_signal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
        return false;
    }

    // PC GDK uses the Microsoft Store signed-in account for XStore context and
    // expects a nullptr user handle here. We still validate p_user above so the
    // public Godot API keeps a consistent "signed-in local user required"
    // contract across store calls.
    HRESULT context_hr = S_OK;
    r_store_context = _get_or_create_store_context(context_hr);
    if (FAILED(context_hr) || r_store_context == nullptr) {
        r_error_signal = _make_error_signal(context_hr, "store_context_create_failed", "Failed to create an XStore context for the user.");
        return false;
    }

    return true;
}

Signal GDKStore::show_product_page_ui_async(const Ref<GDKUser> &p_user, const String &p_store_id) {
    const String store_id = _normalize_store_id(p_store_id);
    if (store_id.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_product_id", "A non-empty Store product ID is required.");
    }

    GDKRuntime *runtime = nullptr;
    XStoreContextHandle store_context = nullptr;
    Signal error_signal;
    if (!_prepare_store_ui_call(p_user, runtime, store_context, error_signal)) {
        return error_signal;
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StoreProductPageUiAsyncContext(this, runtime, pending_signal, store_id);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreShowProductPageUIAsync(store_context, context->get_store_id_utf8(), context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                start_hr,
                "Failed to start the store product page UI flow.",
                "store_product_page_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKStore::show_associated_products_ui_async(const Ref<GDKUser> &p_user, const String &p_store_id, const String &p_product_kinds) {
    const String store_id = _normalize_store_id(p_store_id);
    if (store_id.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_product_id", "A non-empty Store product ID is required.");
    }

    GDKRuntime *runtime = nullptr;
    XStoreContextHandle store_context = nullptr;
    Signal error_signal;
    if (!_prepare_store_ui_call(p_user, runtime, store_context, error_signal)) {
        return error_signal;
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StoreAssociatedProductsUiAsyncContext(this, runtime, pending_signal, store_id);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreShowAssociatedProductsUIAsync(store_context, context->get_store_id_utf8(), parse_product_kinds(p_product_kinds), context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                start_hr,
                "Failed to start the store associated products UI flow.",
                "store_associated_products_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKStore::show_rate_and_review_ui_async(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = nullptr;
    XStoreContextHandle store_context = nullptr;
    Signal error_signal;
    if (!_prepare_store_ui_call(p_user, runtime, store_context, error_signal)) {
        return error_signal;
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StoreRateAndReviewUiAsyncContext(this, runtime, pending_signal);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreShowRateAndReviewUIAsync(store_context, context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                start_hr,
                "Failed to start the store rate and review UI flow.",
                "store_rate_and_review_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKStore::show_redeem_token_ui_async(const Ref<GDKUser> &p_user, const String &p_token, const PackedStringArray &p_allowed_store_ids, bool p_disallow_csv_redemption) {
    const String token = p_token.strip_edges();
    if (token.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_token", "A non-empty redemption token is required.");
    }

    GDKRuntime *runtime = nullptr;
    XStoreContextHandle store_context = nullptr;
    Signal error_signal;
    if (!_prepare_store_ui_call(p_user, runtime, store_context, error_signal)) {
        return error_signal;
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StoreRedeemTokenUiAsyncContext(this, runtime, pending_signal, token, p_allowed_store_ids, p_disallow_csv_redemption);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreShowRedeemTokenUIAsync(
            store_context,
            context->get_token_utf8(),
            context->get_allowed_store_ids(),
            context->get_allowed_store_ids_count(),
            context->get_disallow_csv_redemption(),
            context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                start_hr,
                "Failed to start the store redeem token UI flow.",
                "store_redeem_token_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKStore::show_gifting_ui_async(const Ref<GDKUser> &p_user, const String &p_store_id, const String &p_name, const String &p_extended_json) {
    const String store_id = _normalize_store_id(p_store_id);
    if (store_id.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_product_id", "A non-empty Store product ID is required.");
    }

    GDKRuntime *runtime = nullptr;
    XStoreContextHandle store_context = nullptr;
    Signal error_signal;
    if (!_prepare_store_ui_call(p_user, runtime, store_context, error_signal)) {
        return error_signal;
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StoreGiftingUiAsyncContext(this, runtime, pending_signal, store_id, p_name, p_extended_json);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreShowGiftingUIAsync(
            store_context,
            context->get_store_id_utf8(),
            context->get_name_utf8(),
            context->get_extended_json_utf8(),
            context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                start_hr,
                "Failed to start the store gifting UI flow.",
                "store_gifting_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Ref<GDKStoreLicenseStatus> GDKStore::get_cached_license_status(const String &p_store_id) const {
    return _find_cached_license_status(_normalize_store_id(p_store_id));
}

Ref<GDKResult> GDKStore::check_cached_license_status(const String &p_store_id) const {
    const String store_id = _normalize_store_id(p_store_id);
    if (store_id.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_product_id", "A non-empty Store product ID is required.");
    }

    Ref<GDKStoreLicenseStatus> status = _find_cached_license_status(store_id);
    if (!status.is_valid()) {
        return GDKResult::error_result(E_FAIL, "license_status_not_cached", "No cached license status is available for the requested Store product ID.");
    }

    return GDKResult::ok_result(status);
}

GDKRuntime *GDKStore::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

void GDKStore::_close_store_context() {
    if (m_store_context != nullptr) {
        XStoreCloseContextHandle(m_store_context);
        m_store_context = nullptr;
    }
}

XStoreContextHandle GDKStore::_get_or_create_store_context(HRESULT &r_hresult) {
    if (m_store_context != nullptr) {
        r_hresult = S_OK;
        return m_store_context;
    }

    XStoreContextHandle store_context = nullptr;
    r_hresult = XStoreCreateContext(nullptr, &store_context);
    if (FAILED(r_hresult) || store_context == nullptr) {
        if (SUCCEEDED(r_hresult)) {
            r_hresult = E_UNEXPECTED;
        }
        return nullptr;
    }

    m_store_context = store_context;
    return m_store_context;
}

Signal GDKStore::_make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) const {
    GDKRuntime *runtime = _get_runtime();
    if (runtime != nullptr) {
        return runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
    }

    Ref<GDKPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(GDKResult::error_result(p_hresult, p_code, p_message, p_data));
    return pending_signal->get_completed_signal();
}

Signal GDKStore::_start_license_status_async(const Ref<GDKUser> &p_user, const String &p_store_id, bool p_is_refresh) {
    const String store_id = _normalize_store_id(p_store_id);
    if (store_id.is_empty()) {
        return _make_error_signal(E_INVALIDARG, "invalid_product_id", "A non-empty Store product ID is required.");
    }

    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_available()) {
        return _make_error_signal(E_FAIL, "runtime_unavailable", "GDK runtime is unavailable in the current process.");
    }
    if (!m_runtime_ready || !runtime->is_initialized()) {
        return _make_error_signal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_error_signal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    // PC GDK uses the Microsoft Store signed-in account for XStore context and
    // expects a nullptr user handle here. We still validate p_user above so the
    // public Godot API keeps a consistent "signed-in local user required"
    // contract across store calls.
    HRESULT context_hr = S_OK;
    XStoreContextHandle store_context = _get_or_create_store_context(context_hr);
    if (FAILED(context_hr) || store_context == nullptr) {
        return _make_error_signal(context_hr, "store_context_create_failed", "Failed to create an XStore context for the user.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new StoreLicenseStatusAsyncContext(this, runtime, pending_signal, store_id, p_is_refresh);
    context->bind_cancel_handler();

    HRESULT start_hr = XStoreCanAcquireLicenseForStoreIdAsync(store_context, context->get_store_id_utf8(), context->get_async_block());
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                start_hr,
                p_is_refresh ? "Failed to start entitlements refresh." : "Failed to start store license status query.",
                p_is_refresh ? "store_entitlements_refresh_start_failed" : "store_license_status_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<GDKStoreLicenseStatus> GDKStore::_find_cached_license_status(const String &p_store_id) const {
    for (const CachedLicenseStatus &entry : m_cached_license_status) {
        if (entry.store_id == p_store_id) {
            return entry.status;
        }
    }
    return Ref<GDKStoreLicenseStatus>();
}

Ref<GDKStoreLicenseStatus> GDKStore::_cache_license_status(const Ref<GDKStoreLicenseStatus> &p_status) {
    if (!p_status.is_valid()) {
        return Ref<GDKStoreLicenseStatus>();
    }

    const String store_id = p_status->get_store_id();
    for (CachedLicenseStatus &entry : m_cached_license_status) {
        if (entry.store_id == store_id) {
            entry.status = p_status;
            return p_status;
        }
    }

    m_cached_license_status.push_back({ store_id, p_status });
    return p_status;
}

String GDKStore::_normalize_store_id(const String &p_store_id) {
    return p_store_id.strip_edges();
}

} // namespace godot
