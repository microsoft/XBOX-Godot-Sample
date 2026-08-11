#include "gdk_user.h"

#include <algorithm>
#include <cstring>
#include <string>
#include <vector>

#include <godot_cpp/classes/image.hpp>

#include <XGameErr.h>

#include "gdk.h"
#include "gdk_pending_signal.h"
#include "gdk_request_parsing.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_signal_xasync_context.h"

namespace godot {

namespace {

Signal _make_users_error_signal(
        GDKRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message,
        const Variant &p_data = Variant()) {
    ERR_FAIL_NULL_V(p_runtime, Signal());
    return p_runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
}

GDKUser::SignInState _user_state_to_sign_in_state(XUserState p_user_state) {
    switch (p_user_state) {
        case XUserState::SignedIn:
            return GDKUser::SIGN_IN_STATE_SIGNED_IN;
        case XUserState::SigningOut:
            return GDKUser::SIGN_IN_STATE_SIGNING_OUT;
        case XUserState::SignedOut:
        default:
            return GDKUser::SIGN_IN_STATE_SIGNED_OUT;
    }
}

String _sign_in_state_to_name(GDKUser::SignInState p_sign_in_state) {
    switch (p_sign_in_state) {
        case GDKUser::SIGN_IN_STATE_SIGNED_IN:
            return "signed_in";
        case GDKUser::SIGN_IN_STATE_SIGNING_OUT:
            return "signing_out";
        case GDKUser::SIGN_IN_STATE_SIGNED_OUT:
        default:
            return "signed_out";
    }
}

String _user_change_event_to_name(XUserChangeEvent p_event) {
    switch (p_event) {
        case XUserChangeEvent::SignedInAgain:
            return "signed_in_again";
        case XUserChangeEvent::Gamertag:
            return "gamertag";
        case XUserChangeEvent::GamerPicture:
            return "gamer_picture";
        case XUserChangeEvent::Privileges:
            return "privileges";
        default:
            return "unknown";
    }
}

GDKUser::AgeGroup _age_group_to_enum(XUserAgeGroup p_age_group) {
    switch (p_age_group) {
        case XUserAgeGroup::Child:
            return GDKUser::AGE_GROUP_CHILD;
        case XUserAgeGroup::Teen:
            return GDKUser::AGE_GROUP_TEEN;
        case XUserAgeGroup::Adult:
            return GDKUser::AGE_GROUP_ADULT;
        case XUserAgeGroup::Unknown:
        default:
            return GDKUser::AGE_GROUP_UNKNOWN;
    }
}

String _age_group_to_name(GDKUser::AgeGroup p_age_group) {
    switch (p_age_group) {
        case GDKUser::AGE_GROUP_CHILD:
            return "child";
        case GDKUser::AGE_GROUP_TEEN:
            return "teen";
        case GDKUser::AGE_GROUP_ADULT:
            return "adult";
        case GDKUser::AGE_GROUP_UNKNOWN:
        default:
            return "unknown";
    }
}

// APP_LOCAL_DEVICE_ID is a 32-byte opaque platform identifier. GDScript gets it
// as a lowercase hex string so it can be compared, stored, and printed without
// binding a byte-blob type; _try_parse_device_id() is the inverse.
String _device_id_to_string(const APP_LOCAL_DEVICE_ID &p_device_id) {
    static const char *hex_digits = "0123456789abcdef";

    char buffer[(APP_LOCAL_DEVICE_ID_SIZE * 2) + 1] = {};
    for (size_t i = 0; i < APP_LOCAL_DEVICE_ID_SIZE; ++i) {
        buffer[i * 2] = hex_digits[(p_device_id.value[i] >> 4) & 0x0F];
        buffer[(i * 2) + 1] = hex_digits[p_device_id.value[i] & 0x0F];
    }

    return String::utf8(buffer);
}

bool _try_parse_device_id(const String &p_device_id, APP_LOCAL_DEVICE_ID *r_device_id) {
    if (r_device_id == nullptr) {
        return false;
    }

    const String normalized = p_device_id.strip_edges().to_lower();
    if (normalized.length() != static_cast<int64_t>(APP_LOCAL_DEVICE_ID_SIZE * 2)) {
        return false;
    }

    APP_LOCAL_DEVICE_ID parsed = {};
    for (size_t i = 0; i < APP_LOCAL_DEVICE_ID_SIZE; ++i) {
        uint8_t nibbles[2] = {};
        for (size_t n = 0; n < 2; ++n) {
            const char32_t digit = normalized[static_cast<int64_t>((i * 2) + n)];
            if (digit >= '0' && digit <= '9') {
                nibbles[n] = static_cast<uint8_t>(digit - '0');
            } else if (digit >= 'a' && digit <= 'f') {
                nibbles[n] = static_cast<uint8_t>(10 + (digit - 'a'));
            } else {
                return false;
            }
        }
        parsed.value[i] = static_cast<BYTE>((nibbles[0] << 4) | nibbles[1]);
    }

    *r_device_id = parsed;
    return true;
}

bool _device_ids_equal(const APP_LOCAL_DEVICE_ID &p_left, const APP_LOCAL_DEVICE_ID &p_right) {
    return std::memcmp(p_left.value, p_right.value, APP_LOCAL_DEVICE_ID_SIZE) == 0;
}

bool _is_null_device_id(const APP_LOCAL_DEVICE_ID &p_device_id) {
    return _device_ids_equal(p_device_id, XUserNullDeviceId);
}

String _read_gamertag_component(XUserHandle p_user_handle, XUserGamertagComponent p_component, size_t p_max_bytes, HRESULT *r_hresult) {
    std::vector<char> buffer(p_max_bytes, '\0');
    size_t used = 0;
    const HRESULT hr = XUserGetGamertag(p_user_handle, p_component, buffer.size(), buffer.data(), &used);
    if (r_hresult != nullptr) {
        *r_hresult = hr;
    }
    if (FAILED(hr)) {
        return String();
    }

    return String::utf8(buffer.data());
}

String _privilege_deny_reason_to_string(XUserPrivilegeDenyReason p_reason) {
    switch (p_reason) {
        case XUserPrivilegeDenyReason::None:
            return "none";
        case XUserPrivilegeDenyReason::PurchaseRequired:
            return "purchase_required";
        case XUserPrivilegeDenyReason::Restricted:
            return "restricted";
        case XUserPrivilegeDenyReason::Banned:
            return "banned";
        case XUserPrivilegeDenyReason::Unknown:
        default:
            return "unknown";
    }
}

Dictionary _make_privilege_result(int64_t p_privilege, bool p_has_privilege, XUserPrivilegeDenyReason p_reason) {
    Dictionary data;
    data["privilege"] = p_privilege;
    data["has_privilege"] = p_has_privilege;
    data["deny_reason"] = _privilege_deny_reason_to_string(p_reason);
    data["deny_reason_value"] = static_cast<int64_t>(static_cast<uint32_t>(p_reason));
    return data;
}

Dictionary _make_privilege_resolution_result(int64_t p_privilege) {
    Dictionary data;
    data["privilege"] = p_privilege;
    return data;
}

Dictionary _make_issue_resolution_result(const String &p_url) {
    Dictionary data;
    if (!p_url.is_empty()) {
        data["url"] = p_url;
    }
    return data;
}

bool _try_parse_gamer_picture_size(const String &p_size, XUserGamerPictureSize *r_size) {
    if (r_size == nullptr) {
        return false;
    }

    const String normalized = p_size.strip_edges().to_lower();
    if (normalized == "small") {
        *r_size = XUserGamerPictureSize::Small;
        return true;
    }
    if (normalized == "medium") {
        *r_size = XUserGamerPictureSize::Medium;
        return true;
    }
    if (normalized == "large") {
        *r_size = XUserGamerPictureSize::Large;
        return true;
    }
    if (normalized == "extra_large" || normalized == "extra-large" || normalized == "extralarge") {
        *r_size = XUserGamerPictureSize::ExtraLarge;
        return true;
    }

    return false;
}

class AddUserAsyncContext final : public GDKSignalXAsyncContext {
    GDKUsers *m_users = nullptr;
    String m_action;
    bool m_by_id = false;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("User add operation cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        XUserHandle user_handle = nullptr;
        HRESULT result_hr = m_by_id
                ? XUserAddByIdWithUiResult(p_async_block, &user_handle)
                : XUserAddResult(p_async_block, &user_handle);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("User add operation cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(result_hr, m_action, "user_add_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        m_users->complete_add_user(user_handle, get_pending_signal());
    }

public:
    AddUserAsyncContext(GDKUsers *p_users, GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal, const String &p_action, bool p_by_id = false) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_action(p_action),
            m_by_id(p_by_id) {}
};

class SignOutAsyncContext final : public GDKSignalXAsyncContext {
    GDKUsers *m_users = nullptr;
    Ref<GDKUser> m_user;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("User sign-out cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = XUserSignOutResult(p_async_block);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("User sign-out cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(result_hr, "Failed to sign the user out.", "user_sign_out_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        // The platform also raises XUserChangeEvent::SignedOut, but that
        // callback is not guaranteed to have been dispatched yet. Reconcile
        // here so get_users()/get_primary_user() are already correct when the
        // completion signal resolves; whichever path runs first emits
        // user_changed("removed") exactly once.
        if (m_users != nullptr) {
            m_users->reconcile_signed_out_user(m_user);
        }

        get_pending_signal()->complete(GDKResult::ok_result(m_user));
    }

public:
    SignOutAsyncContext(GDKUsers *p_users, const Ref<GDKUser> &p_user, GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_user(p_user) {}
};

class FindControllerForUserAsyncContext final : public GDKSignalXAsyncContext {
    Ref<GDKUser> m_user;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Controller selection cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        APP_LOCAL_DEVICE_ID device_id = {};
        HRESULT result_hr = XUserFindControllerForUserWithUiResult(p_async_block, &device_id);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Controller selection cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to establish a controller for the user with system UI.",
                    "find_controller_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary data;
        data["device_id"] = _device_id_to_string(device_id);
        data["has_device"] = !_is_null_device_id(device_id);

        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    FindControllerForUserAsyncContext(const Ref<GDKUser> &p_user, GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user) {}
};

class ResolvePrivilegeAsyncContext final : public GDKSignalXAsyncContext {
    GDKUsers *m_users = nullptr;
    Ref<GDKUser> m_user;
    int64_t m_privilege = 0;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Privilege resolution cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = XUserResolvePrivilegeWithUiResult(p_async_block);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Privilege resolution cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to resolve the requested privilege with UI.",
                    "privilege_resolve_result_failed",
                    _make_privilege_resolution_result(m_privilege));
            get_pending_signal()->complete(result);
            return;
        }

        if (m_user.is_valid()) {
            HRESULT refresh_hr = m_user->refresh();
            if (FAILED(refresh_hr)) {
                result = GDKResult::hresult_error(
                        refresh_hr,
                        "Resolved the privilege UI flow but failed to refresh the cached user state.",
                        "user_refresh_after_privilege_resolution_failed");
                get_pending_signal()->complete(result);
                return;
            }

            if (m_users != nullptr) {
                m_users->emit_signal("user_changed", m_user, "privileges");
            }
        }

        get_pending_signal()->complete(GDKResult::ok_result(_make_privilege_resolution_result(m_privilege)));
    }

public:
    ResolvePrivilegeAsyncContext(GDKUsers *p_users, const Ref<GDKUser> &p_user, GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal, int64_t p_privilege) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_user(p_user),
            m_privilege(p_privilege) {}
};

class ResolveIssueAsyncContext final : public GDKSignalXAsyncContext {
    GDKUsers *m_users = nullptr;
    Ref<GDKUser> m_user;
    String m_url;
    std::string m_url_utf8;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("User issue resolution cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = XUserResolveIssueWithUiResult(p_async_block);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("User issue resolution cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to resolve the user issue with system UI.",
                    "user_issue_resolve_result_failed",
                    _make_issue_resolution_result(m_url));
            get_pending_signal()->complete(result);
            return;
        }

        if (m_user.is_valid()) {
            HRESULT refresh_hr = m_user->refresh();
            if (FAILED(refresh_hr)) {
                result = GDKResult::hresult_error(
                        refresh_hr,
                        "Resolved the user issue but failed to refresh the cached user state.",
                        "user_refresh_after_issue_resolution_failed");
                get_pending_signal()->complete(result);
                return;
            }

            if (m_users != nullptr) {
                m_users->emit_signal("user_changed", m_user, "privileges");
            }
        }

        get_pending_signal()->complete(GDKResult::ok_result(_make_issue_resolution_result(m_url)));
    }

public:
    ResolveIssueAsyncContext(GDKUsers *p_users, const Ref<GDKUser> &p_user, GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal, const String &p_url) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_users(p_users),
            m_user(p_user),
            m_url(p_url) {
        const CharString url_utf8 = p_url.utf8();
        if (url_utf8.get_data() != nullptr) {
            m_url_utf8 = url_utf8.get_data();
        }
    }

    const char *get_url() const {
        return m_url_utf8.empty() ? nullptr : m_url_utf8.c_str();
    }
};

class GamerPictureAsyncContext final : public GDKSignalXAsyncContext {
    Ref<GDKUser> m_user;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Gamer picture request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = XUserGetGamerPictureResultSize(p_async_block, &buffer_size);
        if (size_hr == E_ABORT) {
            result = GDKResult::cancelled("Gamer picture request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(size_hr)) {
            result = GDKResult::hresult_error(
                    size_hr,
                    "Failed to get the gamer picture buffer size.",
                    "gamer_picture_result_size_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<uint8_t> buffer(buffer_size);
        size_t buffer_used = 0;
        HRESULT result_hr = XUserGetGamerPictureResult(
                p_async_block,
                buffer.size(),
                buffer.empty() ? nullptr : buffer.data(),
                &buffer_used);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Gamer picture request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to retrieve the gamer picture bytes.",
                    "gamer_picture_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        PackedByteArray png_bytes;
        if (png_bytes.resize(static_cast<int64_t>(buffer_used)) != 0) {
            result = GDKResult::error_result(E_OUTOFMEMORY, "gamer_picture_buffer_alloc_failed", "Failed to allocate a buffer for the gamer picture.");
            get_pending_signal()->complete(result);
            return;
        }
        if (buffer_used > 0) {
            std::memcpy(png_bytes.ptrw(), buffer.data(), buffer_used);
        }

        Ref<Image> image;
        image.instantiate();
        if (image->load_png_from_buffer(png_bytes) != 0) {
            result = GDKResult::error_result(E_FAIL, "gamer_picture_decode_failed", "Failed to decode the gamer picture PNG into a Godot Image.");
            get_pending_signal()->complete(result);
            return;
        }

        get_pending_signal()->complete(GDKResult::ok_result(image));
    }

public:
    GamerPictureAsyncContext(const Ref<GDKUser> &p_user, GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user) {}
};

class TokenAndSignatureAsyncContext final : public GDKSignalXAsyncContext {
    Ref<GDKUser> m_user;
    PackedByteArray m_body;
    bool m_force_refresh = false;
    std::string m_method_utf8;
    std::string m_url_utf8;
    std::vector<std::string> m_header_names;
    std::vector<std::string> m_header_values;
    std::vector<XUserGetTokenAndSignatureHttpHeader> m_headers;

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Token and signature request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = XUserGetTokenAndSignatureResultSize(p_async_block, &buffer_size);
        if (size_hr == E_ABORT) {
            result = GDKResult::cancelled("Token and signature request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(size_hr)) {
            result = GDKResult::hresult_error(
                    size_hr,
                    "Failed to get the token/signature result size.",
                    "token_signature_result_size_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<uint8_t> buffer(buffer_size);
        XUserGetTokenAndSignatureData *token_data = nullptr;
        HRESULT result_hr = XUserGetTokenAndSignatureResult(
                p_async_block,
                buffer.size(),
                buffer.empty() ? nullptr : buffer.data(),
                &token_data,
                nullptr);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Token and signature request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to retrieve the token/signature payload.",
                    "token_signature_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary data;
        data["token"] = token_data != nullptr && token_data->token != nullptr ? String::utf8(token_data->token) : String();
        data["signature"] = token_data != nullptr && token_data->signature != nullptr ? String::utf8(token_data->signature) : String();

        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    TokenAndSignatureAsyncContext(
            const Ref<GDKUser> &p_user,
            GDKRuntime *p_runtime,
            const Ref<GDKPendingSignal> &p_pending_signal,
            const String &p_method,
            const String &p_url,
            const Dictionary &p_headers,
            const PackedByteArray &p_body,
            bool p_force_refresh) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_body(p_body),
            m_force_refresh(p_force_refresh) {
        const CharString method_utf8 = p_method.utf8();
        if (method_utf8.get_data() != nullptr) {
            m_method_utf8 = method_utf8.get_data();
        }

        const CharString url_utf8 = p_url.utf8();
        if (url_utf8.get_data() != nullptr) {
            m_url_utf8 = url_utf8.get_data();
        }

        const Array header_keys = p_headers.keys();
        m_header_names.reserve(static_cast<size_t>(header_keys.size()));
        m_header_values.reserve(static_cast<size_t>(header_keys.size()));
        for (int64_t i = 0; i < header_keys.size(); ++i) {
            const Variant key = header_keys[i];
            const String header_name = String(key);
            const String header_value = String(p_headers[key]);

            const CharString header_name_utf8 = header_name.utf8();
            const CharString header_value_utf8 = header_value.utf8();

            m_header_names.emplace_back(header_name_utf8.get_data() != nullptr ? header_name_utf8.get_data() : "");
            m_header_values.emplace_back(header_value_utf8.get_data() != nullptr ? header_value_utf8.get_data() : "");
        }

        m_headers.reserve(m_header_names.size());
        for (size_t i = 0; i < m_header_names.size(); ++i) {
            XUserGetTokenAndSignatureHttpHeader header = {};
            header.name = m_header_names[i].c_str();
            header.value = m_header_values[i].c_str();
            m_headers.push_back(header);
        }
    }

    XUserGetTokenAndSignatureOptions get_options() const {
        return m_force_refresh ? XUserGetTokenAndSignatureOptions::ForceRefresh : XUserGetTokenAndSignatureOptions::None;
    }

    const char *get_method() const {
        return m_method_utf8.c_str();
    }

    const char *get_url() const {
        return m_url_utf8.c_str();
    }

    size_t get_header_count() const {
        return m_headers.size();
    }

    const XUserGetTokenAndSignatureHttpHeader *get_headers() const {
        return m_headers.empty() ? nullptr : m_headers.data();
    }

    size_t get_body_size() const {
        return static_cast<size_t>(m_body.size());
    }

    const void *get_body_data() const {
        return m_body.is_empty() ? nullptr : static_cast<const void *>(m_body.ptr());
    }
};

} // namespace

void GDKUserSignOutDeferral::_bind_methods() {
    ClassDB::bind_method(D_METHOD("is_valid"), &GDKUserSignOutDeferral::is_valid);
    ClassDB::bind_method(D_METHOD("release"), &GDKUserSignOutDeferral::release);
}

GDKUserSignOutDeferral::~GDKUserSignOutDeferral() {
    release();
}

bool GDKUserSignOutDeferral::is_valid() const {
    return m_handle != nullptr;
}

void GDKUserSignOutDeferral::release() {
    if (m_handle != nullptr) {
        XUserCloseSignOutDeferralHandle(m_handle);
        m_handle = nullptr;
    }
}

void GDKUserSignOutDeferral::set_handle_internal(XUserSignOutDeferralHandle p_handle) {
    if (m_handle == p_handle) {
        return;
    }
    release();
    m_handle = p_handle;
}

void GDKUser::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_local_id"), &GDKUser::get_local_id);
    ClassDB::bind_method(D_METHOD("get_xuid"), &GDKUser::get_xuid);
    ClassDB::bind_method(D_METHOD("get_gamertag"), &GDKUser::get_gamertag);
    ClassDB::bind_method(D_METHOD("get_modern_gamertag"), &GDKUser::get_modern_gamertag);
    ClassDB::bind_method(D_METHOD("get_modern_gamertag_suffix"), &GDKUser::get_modern_gamertag_suffix);
    ClassDB::bind_method(D_METHOD("get_unique_modern_gamertag"), &GDKUser::get_unique_modern_gamertag);
    ClassDB::bind_method(D_METHOD("get_age_group"), &GDKUser::get_age_group);
    ClassDB::bind_method(D_METHOD("get_age_group_name"), &GDKUser::get_age_group_name);
    ClassDB::bind_method(D_METHOD("get_sign_in_state"), &GDKUser::get_sign_in_state);
    ClassDB::bind_method(D_METHOD("get_sign_in_state_name"), &GDKUser::get_sign_in_state_name);
    ClassDB::bind_method(D_METHOD("is_guest"), &GDKUser::is_guest);
    ClassDB::bind_method(D_METHOD("is_signed_in"), &GDKUser::is_signed_in);
    ClassDB::bind_method(D_METHOD("is_store_user"), &GDKUser::is_store_user);
    ClassDB::bind_method(D_METHOD("is_valid"), &GDKUser::is_valid);
    ClassDB::bind_method(D_METHOD("is_same_user", "other"), &GDKUser::is_same_user);
    ClassDB::bind_method(D_METHOD("duplicate_user"), &GDKUser::duplicate_user);

    BIND_ENUM_CONSTANT(AGE_GROUP_UNKNOWN);
    BIND_ENUM_CONSTANT(AGE_GROUP_CHILD);
    BIND_ENUM_CONSTANT(AGE_GROUP_TEEN);
    BIND_ENUM_CONSTANT(AGE_GROUP_ADULT);

    BIND_ENUM_CONSTANT(SIGN_IN_STATE_SIGNED_OUT);
    BIND_ENUM_CONSTANT(SIGN_IN_STATE_SIGNING_OUT);
    BIND_ENUM_CONSTANT(SIGN_IN_STATE_SIGNED_IN);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "local_id"), "", "get_local_id");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "xuid"), "", "get_xuid");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "gamertag"), "", "get_gamertag");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "modern_gamertag"), "", "get_modern_gamertag");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "modern_gamertag_suffix"), "", "get_modern_gamertag_suffix");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "unique_modern_gamertag"), "", "get_unique_modern_gamertag");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "age_group", PROPERTY_HINT_ENUM, "Unknown,Child,Teen,Adult"), "", "get_age_group");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "sign_in_state", PROPERTY_HINT_ENUM, "Signed Out,Signing Out,Signed In"), "", "get_sign_in_state");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "guest"), "", "is_guest");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "signed_in"), "", "is_signed_in");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "store_user"), "", "is_store_user");
}

GDKUser::GDKUser() {}

GDKUser::~GDKUser() {
    clear();
}

int64_t GDKUser::get_local_id() const {
    return static_cast<int64_t>(m_local_id.value);
}

String GDKUser::get_xuid() const {
    return m_xuid;
}

String GDKUser::get_gamertag() const {
    return m_gamertag;
}

String GDKUser::get_modern_gamertag() const {
    return m_modern_gamertag;
}

String GDKUser::get_modern_gamertag_suffix() const {
    return m_modern_gamertag_suffix;
}

String GDKUser::get_unique_modern_gamertag() const {
    return m_unique_modern_gamertag;
}

GDKUser::AgeGroup GDKUser::get_age_group() const {
    return m_age_group;
}

String GDKUser::get_age_group_name() const {
    return _age_group_to_name(m_age_group);
}

GDKUser::SignInState GDKUser::get_sign_in_state() const {
    return m_sign_in_state;
}

String GDKUser::get_sign_in_state_name() const {
    return _sign_in_state_to_name(m_sign_in_state);
}

bool GDKUser::is_guest() const {
    return m_is_guest;
}

bool GDKUser::is_signed_in() const {
    return m_is_signed_in;
}

bool GDKUser::is_store_user() const {
    return m_is_store_user;
}

bool GDKUser::is_valid() const {
    return m_user_handle != nullptr;
}

bool GDKUser::is_same_user(const Ref<GDKUser> &p_other) const {
    if (!p_other.is_valid()) {
        return false;
    }

    // XUserCompare has no defined behavior for a null handle, and either side
    // can legitimately lack one (a default-constructed wrapper, or one whose
    // handle was released). Two handle-less wrappers are not "the same user"
    // either — there is no platform user to be the same as.
    if (m_user_handle == nullptr || p_other->get_handle() == nullptr) {
        return false;
    }

    // XUserCompare orders/compares two handles; equality (0) means both
    // handles refer to the same platform user, even when the wrappers are
    // distinct objects (for example a cached user vs. a fresh lookup).
    return XUserCompare(m_user_handle, p_other->get_handle()) == 0;
}

Ref<GDKUser> GDKUser::duplicate_user() const {
    if (m_user_handle == nullptr) {
        return Ref<GDKUser>();
    }

    XUserHandle duplicated_handle = nullptr;
    if (FAILED(XUserDuplicateHandle(m_user_handle, &duplicated_handle))) {
        return Ref<GDKUser>();
    }

    Ref<GDKUser> copy;
    copy.instantiate();
    if (FAILED(copy->adopt_handle(duplicated_handle))) {
        return Ref<GDKUser>();
    }

    return copy;
}

HRESULT GDKUser::_populate_from_handle(XUserHandle p_user_handle) {
    XUserLocalId local_id = {};
    HRESULT hr = XUserGetLocalId(p_user_handle, &local_id);
    if (FAILED(hr)) {
        return hr;
    }

    uint64_t xuid = 0;
    hr = XUserGetId(p_user_handle, &xuid);
    if (FAILED(hr)) {
        return hr;
    }

    char gamertag[XUserGamertagComponentClassicMaxBytes] = {};
    size_t gamertag_used = 0;
    hr = XUserGetGamertag(
            p_user_handle,
            XUserGamertagComponent::Classic,
            sizeof(gamertag),
            gamertag,
            &gamertag_used);
    if (FAILED(hr)) {
        return hr;
    }

    // The modern gamertag components are optional metadata: platforms and
    // accounts that predate modern gamertags return an empty suffix (or fail
    // the lookup outright). Never fail the whole populate over them -- the
    // classic gamertag above is the required identity string.
    HRESULT modern_hr = S_OK;
    const String modern_gamertag = _read_gamertag_component(
            p_user_handle,
            XUserGamertagComponent::Modern,
            XUserGamertagComponentModernMaxBytes,
            &modern_hr);
    const String modern_gamertag_suffix = _read_gamertag_component(
            p_user_handle,
            XUserGamertagComponent::ModernSuffix,
            XUserGamertagComponentModernSuffixMaxBytes,
            &modern_hr);
    const String unique_modern_gamertag = _read_gamertag_component(
            p_user_handle,
            XUserGamertagComponent::UniqueModern,
            XUserGamertagComponentUniqueModernMaxBytes,
            &modern_hr);

    bool is_guest = false;
    hr = XUserGetIsGuest(p_user_handle, &is_guest);
    if (FAILED(hr)) {
        return hr;
    }

    XUserState user_state = XUserState::SignedOut;
    hr = XUserGetState(p_user_handle, &user_state);
    if (FAILED(hr)) {
        return hr;
    }

    XUserAgeGroup age_group = XUserAgeGroup::Unknown;
    hr = XUserGetAgeGroup(p_user_handle, &age_group);
    if (FAILED(hr) && hr != E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED) {
        return hr;
    }

    m_local_id = local_id;
    m_xuid = String::num_uint64(xuid);
    m_gamertag = String::utf8(gamertag);
    m_modern_gamertag = modern_gamertag;
    m_modern_gamertag_suffix = modern_gamertag_suffix;
    m_unique_modern_gamertag = unique_modern_gamertag;
    m_age_group = hr == E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED ? AGE_GROUP_UNKNOWN : _age_group_to_enum(age_group);
    m_sign_in_state = _user_state_to_sign_in_state(user_state);
    m_is_guest = is_guest;
    m_is_signed_in = user_state == XUserState::SignedIn;
    m_is_store_user = XUserIsStoreUser(p_user_handle);

    return S_OK;
}

HRESULT GDKUser::adopt_handle(XUserHandle p_user_handle) {
    clear();

    if (p_user_handle == nullptr) {
        return E_INVALIDARG;
    }

    m_user_handle = p_user_handle;
    HRESULT hr = _populate_from_handle(m_user_handle);
    if (FAILED(hr)) {
        clear();
    }

    return hr;
}

HRESULT GDKUser::refresh() {
    if (m_user_handle == nullptr) {
        return E_FAIL;
    }

    return _populate_from_handle(m_user_handle);
}

bool GDKUser::matches_local_id(XUserLocalId p_user_local_id) const {
    return m_local_id.value == p_user_local_id.value;
}

XUserLocalId GDKUser::get_native_local_id() const {
    return m_local_id;
}

XUserHandle GDKUser::get_handle() const {
    return m_user_handle;
}

void GDKUser::clear() {
    if (m_user_handle != nullptr) {
        XUserCloseHandle(m_user_handle);
        m_user_handle = nullptr;
    }

    m_local_id = {};
    m_xuid = "";
    m_gamertag = "";
    m_modern_gamertag = "";
    m_modern_gamertag_suffix = "";
    m_unique_modern_gamertag = "";
    m_age_group = AGE_GROUP_UNKNOWN;
    m_sign_in_state = SIGN_IN_STATE_SIGNED_OUT;
    m_is_guest = false;
    m_is_signed_in = false;
    m_is_store_user = false;
}

void GDKUsers::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_default_user_async"), &GDKUsers::add_default_user_async);
    ClassDB::bind_method(D_METHOD("add_user_with_ui_async", "allow_guests"), &GDKUsers::add_user_with_ui_async, DEFVAL(false));
    ClassDB::bind_method(D_METHOD("add_user_by_id_with_ui_async", "xuid"), &GDKUsers::add_user_by_id_with_ui_async);
    ClassDB::bind_method(D_METHOD("get_primary_user"), &GDKUsers::get_primary_user);
    ClassDB::bind_method(D_METHOD("get_users"), &GDKUsers::get_users);
    ClassDB::bind_method(D_METHOD("get_max_users"), &GDKUsers::get_max_users);
    ClassDB::bind_method(D_METHOD("is_sign_out_available"), &GDKUsers::is_sign_out_available);
    ClassDB::bind_method(D_METHOD("sign_out_async", "user"), &GDKUsers::sign_out_async);
    ClassDB::bind_method(D_METHOD("acquire_sign_out_deferral"), &GDKUsers::acquire_sign_out_deferral);
    ClassDB::bind_method(D_METHOD("find_user_by_xuid", "xuid"), &GDKUsers::find_user_by_xuid);
    ClassDB::bind_method(D_METHOD("find_user_by_local_id", "local_id"), &GDKUsers::find_user_by_local_id);
    ClassDB::bind_method(D_METHOD("find_user_for_device", "device_id"), &GDKUsers::find_user_for_device);
    ClassDB::bind_method(D_METHOD("find_controller_for_user_with_ui_async", "user"), &GDKUsers::find_controller_for_user_with_ui_async);
    ClassDB::bind_method(D_METHOD("get_device_associations"), &GDKUsers::get_device_associations);
    ClassDB::bind_method(D_METHOD("get_devices_for_user", "user"), &GDKUsers::get_devices_for_user);
    ClassDB::bind_method(
            D_METHOD("get_default_audio_endpoint", "user", "kind"),
            &GDKUsers::get_default_audio_endpoint,
            DEFVAL(static_cast<int64_t>(AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER)));
    ClassDB::bind_method(D_METHOD("check_privilege_async", "user", "privilege"), &GDKUsers::check_privilege_async);
    ClassDB::bind_method(D_METHOD("resolve_privilege_with_ui_async", "user", "privilege"), &GDKUsers::resolve_privilege_with_ui_async);
    ClassDB::bind_method(D_METHOD("resolve_issue_with_ui_async", "user", "url"), &GDKUsers::resolve_issue_with_ui_async, DEFVAL(String()));
    ClassDB::bind_method(D_METHOD("get_gamer_picture_async", "user", "size"), &GDKUsers::get_gamer_picture_async, DEFVAL(String("medium")));
    ClassDB::bind_method(
            D_METHOD("get_token_and_signature_async", "user", "method", "url", "headers", "body", "force_refresh"),
            &GDKUsers::get_token_and_signature_async,
            DEFVAL(Dictionary()),
            DEFVAL(PackedByteArray()),
            DEFVAL(false));

    BIND_ENUM_CONSTANT(AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER);
    BIND_ENUM_CONSTANT(AUDIO_ENDPOINT_KIND_COMMUNICATION_CAPTURE);

    ADD_SIGNAL(MethodInfo(
            "user_changed",
            PropertyInfo(Variant::OBJECT, "user", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_DEFAULT, "GDKUser"),
            PropertyInfo(Variant::STRING, "change_kind")));
    ADD_SIGNAL(MethodInfo(
            "device_association_changed",
            PropertyInfo(Variant::STRING, "device_id"),
            PropertyInfo(Variant::INT, "old_user_local_id"),
            PropertyInfo(Variant::INT, "new_user_local_id")));
    ADD_SIGNAL(MethodInfo(
            "default_audio_endpoint_changed",
            PropertyInfo(Variant::INT, "user_local_id"),
            PropertyInfo(Variant::INT, "kind"),
            PropertyInfo(Variant::STRING, "endpoint_id")));
}

void GDKUsers::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKUsers::on_runtime_initialized() {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(E_FAIL, "runtime_not_initialized", "Cannot initialize the users service before the GDK runtime.");
    }

    if (!m_change_event_registered) {
        HRESULT hr = XUserRegisterForChangeEvent(
                runtime->get_task_queue(),
                this,
                _user_change_callback,
                &m_change_token);
        if (FAILED(hr)) {
            return GDKResult::hresult_error(hr, "Failed to register the runtime-wide XUser change callback.", "user_change_event_register_failed");
        }

        m_change_event_registered = true;
    }

    // Device-association and default-audio-endpoint notifications are
    // best-effort: they are the XR-112 controller-pairing feed, but a platform
    // that does not surface them must not block GDK.initialize(). Report the
    // failure through GDK.runtime_error and keep going with an empty
    // association cache.
    if (!m_device_association_registered) {
        HRESULT hr = XUserRegisterForDeviceAssociationChanged(
                runtime->get_task_queue(),
                this,
                _device_association_changed_callback,
                &m_device_association_token);
        if (SUCCEEDED(hr)) {
            m_device_association_registered = true;
        } else if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::hresult_error(
                    hr,
                    "Failed to register for user/device association changes; controller pairing changes will not be reported.",
                    "device_association_register_failed"));
        }
    }

    if (!m_audio_endpoint_registered) {
        HRESULT hr = XUserRegisterForDefaultAudioEndpointUtf16Changed(
                runtime->get_task_queue(),
                this,
                _default_audio_endpoint_changed_callback,
                &m_audio_endpoint_token);
        if (SUCCEEDED(hr)) {
            m_audio_endpoint_registered = true;
        } else if (m_owner != nullptr) {
            m_owner->emit_runtime_error(GDKResult::hresult_error(
                    hr,
                    "Failed to register for default audio endpoint changes; audio endpoint changes will not be reported.",
                    "audio_endpoint_register_failed"));
        }
    }

    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKUsers::shutdown() {
    m_runtime_ready = false;

    if (m_change_event_registered) {
        // wait=true matches the pattern in gdk_activation.cpp and
        // gdk_multiplayer_activity.cpp: block until any in-flight change
        // callback has finished before we destroy member state. Defensive
        // even though the per-session GDK shutdown crash was actually
        // caused by cycling XGameRuntimeInitialize/Uninitialize (see
        // gdk_runtime.cpp); making the change-event unregister synchronous
        // keeps a worker-thread callback from ever dereferencing a GDKUsers
        // whose Refs have started unwinding.
        XUserUnregisterForChangeEvent(m_change_token, true);
        m_change_event_registered = false;
    }

    if (m_device_association_registered) {
        XUserUnregisterForDeviceAssociationChanged(m_device_association_token, true);
        m_device_association_registered = false;
    }

    if (m_audio_endpoint_registered) {
        XUserUnregisterForDefaultAudioEndpointUtf16Changed(m_audio_endpoint_token, true);
        m_audio_endpoint_registered = false;
    }

    m_primary_user.unref();
    m_users.clear();
    m_device_associations.clear();
}

Signal GDKUsers::add_default_user_async() {
    return _start_add_user_async(XUserAddOptions::AddDefaultUserSilently, "Failed to add the default user.");
}

Signal GDKUsers::add_user_with_ui_async(bool p_allow_guests) {
    // Interactive add-with-UI is an advanced-user-model feature. Under the
    // simplified (PC-default) user model, XUserAddAsync rejects every UI/guest
    // option (None, AllowGuests, AddDefaultUserAllowingUI) with E_INVALIDARG --
    // only the silent add_default_user_async() path works there. Titles that
    // call this method must be running the advanced user model.
    //
    // p_allow_guests selects the interactive flow:
    //   false (default) -> AddDefaultUserAllowingUI: resolve the launching
    //                      default user, surfacing the system sign-in UI.
    //   true            -> AllowGuests: open the full account picker (without the
    //                      default/silent flags) so the player can choose any
    //                      account, including guests.
    //
    // When the GDK reports the player cancelled (E_ABORT), finalize() normalizes
    // it to a cancelled GDKResult. Whether dismissing the system UI fires the
    // completion callback at all is GDK platform behavior outside the addon's
    // control (see issue #115); the addon faithfully surfaces whatever the GDK
    // reports.
    const XUserAddOptions options = p_allow_guests
            ? XUserAddOptions::AllowGuests
            : XUserAddOptions::AddDefaultUserAllowingUI;
    return _start_add_user_async(options, "Failed to add a user with UI.");
}

Ref<GDKUser> GDKUsers::get_primary_user() const {
    return m_primary_user;
}

Array GDKUsers::get_users() const {
    Array users;
    for (const Ref<GDKUser> &user : m_users) {
        users.push_back(user);
    }
    return users;
}

Signal GDKUsers::add_user_by_id_with_ui_async(const String &p_xuid) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    // Parse through the shared request-parsing seam every other GDK service
    // wrapper uses: it rejects negatives, trailing garbage, and range errors,
    // none of which String::is_valid_int() / to_int() catch.
    uint64_t xuid = 0;
    if (!gdk_request_parsing::try_parse_xuid(p_xuid, &xuid, /*p_reject_zero=*/true)) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_xuid", "A decimal XUID string is required.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new AddUserAsyncContext(this, runtime, pending_signal, "Failed to add the requested user with UI.", true);
    context->bind_cancel_handler();

    HRESULT hr = XUserAddByIdWithUiAsync(xuid, context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to start the add-user-by-id UI flow.", "user_add_by_id_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<GDKResult> GDKUsers::get_max_users() const {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    uint32_t max_users = 0;
    HRESULT hr = XUserGetMaxUsers(&max_users);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to read the maximum simultaneous user count.", "get_max_users_failed");
    }

    return GDKResult::ok_result(static_cast<int64_t>(max_users));
}

bool GDKUsers::is_sign_out_available() const {
    return m_runtime_ready && XUserIsSignOutPresent();
}

Signal GDKUsers::sign_out_async(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }
    if (!XUserIsSignOutPresent()) {
        // XUserSignOutAsync is only valid on platforms that surface a
        // title-driven sign-out affordance; is_sign_out_available() is the
        // documented gate for it.
        return _make_users_error_signal(
                runtime,
                E_NOTIMPL,
                "sign_out_not_available",
                "This platform does not support title-initiated sign-out. Check is_sign_out_available() first.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new SignOutAsyncContext(this, p_user, runtime, pending_signal);
    context->bind_cancel_handler();

    HRESULT hr = XUserSignOutAsync(p_user->get_handle(), context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to start the user sign-out.", "user_sign_out_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<GDKResult> GDKUsers::acquire_sign_out_deferral() const {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    XUserSignOutDeferralHandle handle = nullptr;
    HRESULT hr = XUserGetSignOutDeferral(&handle);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to acquire a user sign-out deferral.", "acquire_sign_out_deferral_failed");
    }

    Ref<GDKUserSignOutDeferral> deferral;
    deferral.instantiate();
    deferral->set_handle_internal(handle);
    return GDKResult::ok_result(deferral);
}

Ref<GDKResult> GDKUsers::find_user_by_xuid(const String &p_xuid) {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    // Same shared parsing seam as add_user_by_id_with_ui_async(); see the note
    // there on why is_valid_int() / to_int() is not sufficient for an XUID.
    uint64_t xuid = 0;
    if (!gdk_request_parsing::try_parse_xuid(p_xuid, &xuid, /*p_reject_zero=*/true)) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_xuid", "A decimal XUID string is required.");
    }

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserFindUserById(xuid, &user_handle);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to find a signed-in user with the requested XUID.", "user_not_found");
    }

    return _wrap_found_handle(user_handle);
}

Ref<GDKResult> GDKUsers::find_user_by_local_id(int64_t p_local_id) {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (p_local_id == 0) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_local_id", "A non-zero user local id is required.");
    }

    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_local_id);

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserFindUserByLocalId(local_id, &user_handle);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to find a signed-in user with the requested local id.", "user_not_found");
    }

    return _wrap_found_handle(user_handle);
}

Ref<GDKResult> GDKUsers::find_user_for_device(const String &p_device_id) {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    APP_LOCAL_DEVICE_ID device_id = {};
    if (!_try_parse_device_id(p_device_id, &device_id)) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_device_id",
                "A device id must be the 64-character hex string reported by get_device_associations().");
    }

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserFindForDevice(&device_id, &user_handle);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to find a user associated with the requested device.", "user_not_found");
    }

    return _wrap_found_handle(user_handle);
}

Signal GDKUsers::find_controller_for_user_with_ui_async(const Ref<GDKUser> &p_user) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new FindControllerForUserAsyncContext(p_user, runtime, pending_signal);
    context->bind_cancel_handler();

    HRESULT hr = XUserFindControllerForUserWithUiAsync(p_user->get_handle(), context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the controller selection UI for the user.",
                "find_controller_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Array GDKUsers::get_device_associations() const {
    Array associations;
    for (const DeviceAssociation &association : m_device_associations) {
        Dictionary entry;
        entry["device_id"] = _device_id_to_string(association.device_id);
        entry["user_local_id"] = static_cast<int64_t>(association.user_local_id.value);
        associations.push_back(entry);
    }
    return associations;
}

PackedStringArray GDKUsers::get_devices_for_user(const Ref<GDKUser> &p_user) const {
    PackedStringArray devices;
    if (!p_user.is_valid()) {
        return devices;
    }

    const XUserLocalId local_id = p_user->get_native_local_id();
    if (local_id.value == 0) {
        return devices;
    }

    for (const DeviceAssociation &association : m_device_associations) {
        if (association.user_local_id.value == local_id.value) {
            devices.push_back(_device_id_to_string(association.device_id));
        }
    }

    return devices;
}

Ref<GDKResult> GDKUsers::get_default_audio_endpoint(const Ref<GDKUser> &p_user, int64_t p_kind) const {
    if (!m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return GDKResult::error_result(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }
    if (p_kind != AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER && p_kind != AUDIO_ENDPOINT_KIND_COMMUNICATION_CAPTURE) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_audio_endpoint_kind",
                "Audio endpoint kind must be AUDIO_ENDPOINT_KIND_COMMUNICATION_RENDER or AUDIO_ENDPOINT_KIND_COMMUNICATION_CAPTURE.");
    }

    wchar_t endpoint_id[XUserAudioEndpointMaxUtf16Count] = {};
    size_t endpoint_id_used = 0;
    HRESULT hr = XUserGetDefaultAudioEndpointUtf16(
            p_user->get_native_local_id(),
            static_cast<XUserDefaultAudioEndpointKind>(static_cast<uint32_t>(p_kind)),
            XUserAudioEndpointMaxUtf16Count,
            endpoint_id,
            &endpoint_id_used);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to read the default audio endpoint for the user.", "get_default_audio_endpoint_failed");
    }

    Dictionary data;
    data["kind"] = p_kind;
    data["endpoint_id"] = String(endpoint_id);
    return GDKResult::ok_result(data);
}

Signal GDKUsers::check_privilege_async(const Ref<GDKUser> &p_user, int64_t p_privilege) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    bool has_privilege = false;
    XUserPrivilegeDenyReason deny_reason = XUserPrivilegeDenyReason::None;
    HRESULT hr = XUserCheckPrivilege(
            p_user->get_handle(),
            XUserPrivilegeOptions::None,
            static_cast<XUserPrivilege>(static_cast<uint32_t>(p_privilege)),
            &has_privilege,
            &deny_reason);
    if (hr == E_GAMEUSER_RESOLVE_USER_ISSUE_REQUIRED) {
        Dictionary data = _make_privilege_result(p_privilege, false, deny_reason);
        data["needs_user_issue_resolution"] = true;
        return _make_users_error_signal(
                runtime,
                hr,
                "user_issue_resolution_required",
                "The user must resolve an account issue with system UI before the privilege can be checked.",
                data);
    }
    if (FAILED(hr)) {
        return _make_users_error_signal(
                runtime,
                hr,
                "privilege_check_failed",
                "Failed to check the requested user privilege.",
                _make_privilege_result(p_privilege, false, deny_reason));
    }

    Dictionary data = _make_privilege_result(p_privilege, has_privilege, deny_reason);
    data["needs_user_issue_resolution"] = false;
    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    pending_signal->complete_deferred(GDKResult::ok_result(data));
    return pending_signal->get_completed_signal();
}

Signal GDKUsers::resolve_privilege_with_ui_async(const Ref<GDKUser> &p_user, int64_t p_privilege) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new ResolvePrivilegeAsyncContext(this, p_user, runtime, pending_signal, p_privilege);
    context->bind_cancel_handler();

    HRESULT hr = XUserResolvePrivilegeWithUiAsync(
            p_user->get_handle(),
            XUserPrivilegeOptions::None,
            static_cast<XUserPrivilege>(static_cast<uint32_t>(p_privilege)),
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the privilege resolution UI.",
                "privilege_resolve_start_failed",
                _make_privilege_resolution_result(p_privilege));
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal GDKUsers::resolve_issue_with_ui_async(const Ref<GDKUser> &p_user, const String &p_url) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new ResolveIssueAsyncContext(this, p_user, runtime, pending_signal, p_url.strip_edges());
    context->bind_cancel_handler();

    HRESULT hr = XUserResolveIssueWithUiAsync(
            p_user->get_handle(),
            context->get_url(),
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the user issue resolution UI.",
                "user_issue_resolve_start_failed",
                _make_issue_resolution_result(p_url.strip_edges()));
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal GDKUsers::get_gamer_picture_async(const Ref<GDKUser> &p_user, const String &p_size) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    XUserGamerPictureSize native_size = XUserGamerPictureSize::Medium;
    if (!_try_parse_gamer_picture_size(p_size, &native_size)) {
        return _make_users_error_signal(
                runtime,
                E_INVALIDARG,
                "invalid_gamer_picture_size",
                "Gamer picture size must be one of: small, medium, large, extra_large.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new GamerPictureAsyncContext(p_user, runtime, pending_signal);
    context->bind_cancel_handler();

    HRESULT hr = XUserGetGamerPictureAsync(
            p_user->get_handle(),
            native_size,
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the gamer picture request.",
                "gamer_picture_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal GDKUsers::get_token_and_signature_async(
        const Ref<GDKUser> &p_user,
        const String &p_method,
        const String &p_url,
        const Dictionary &p_headers,
        const PackedByteArray &p_body,
    bool p_force_refresh) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required.");
    }

    const String method = p_method.strip_edges();
    const String url = p_url.strip_edges();
    if (method.is_empty()) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_http_method", "Token/signature requests require a non-empty HTTP method.");
    }
    if (url.is_empty()) {
        return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_request_url", "Token/signature requests require a non-empty URL.");
    }

    const Array header_keys = p_headers.keys();
    for (int64_t i = 0; i < header_keys.size(); ++i) {
        const String header_name = String(header_keys[i]).strip_edges();
        if (header_name.is_empty()) {
            return _make_users_error_signal(runtime, E_INVALIDARG, "invalid_request_headers", "Token/signature request headers require non-empty string keys.");
        }
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new TokenAndSignatureAsyncContext(p_user, runtime, pending_signal, method, url, p_headers, p_body, p_force_refresh);
    context->bind_cancel_handler();

    HRESULT hr = XUserGetTokenAndSignatureAsync(
            p_user->get_handle(),
            context->get_options(),
            context->get_method(),
            context->get_url(),
            context->get_header_count(),
            context->get_headers(),
            context->get_body_size(),
            context->get_body_data(),
            context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the token/signature request.",
                "token_signature_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

void GDKUsers::on_user_change(XUserLocalId p_user_local_id, XUserChangeEvent p_event) {
    GDKRuntime *runtime = _get_runtime();
    if (!m_runtime_ready || runtime == nullptr || runtime->is_shutting_down()) {
        return;
    }

    Ref<GDKUser> user = _find_user_by_local_id(p_user_local_id);
    if (!user.is_valid()) {
        return;
    }

    switch (p_event) {
        case XUserChangeEvent::SignedOut: {
            const bool was_primary = m_primary_user.is_valid() && m_primary_user->get_local_id() == user->get_local_id();

            if (m_owner != nullptr) {
                m_owner->notify_user_removed(user);
            }

            _remove_user_by_local_id(p_user_local_id);
            if (was_primary) {
                m_primary_user.unref();
            }

            emit_signal("user_changed", user, "removed");
        } break;
        case XUserChangeEvent::SignedInAgain:
        case XUserChangeEvent::Gamertag:
        case XUserChangeEvent::GamerPicture:
        case XUserChangeEvent::Privileges: {
            if (SUCCEEDED(user->refresh())) {
                emit_signal("user_changed", user, _user_change_event_to_name(p_event));
            }
        } break;
        case XUserChangeEvent::SigningOut:
        default:
            break;
    }
}

void GDKUsers::on_device_association_changed(const XUserDeviceAssociationChange &p_change) {
    GDKRuntime *runtime = _get_runtime();
    if (!m_runtime_ready || runtime == nullptr || runtime->is_shutting_down()) {
        return;
    }

    _update_device_association(p_change.deviceId, p_change.newUser);

    emit_signal(
            "device_association_changed",
            _device_id_to_string(p_change.deviceId),
            static_cast<int64_t>(p_change.oldUser.value),
            static_cast<int64_t>(p_change.newUser.value));
}

void GDKUsers::on_default_audio_endpoint_changed(XUserLocalId p_user_local_id, XUserDefaultAudioEndpointKind p_kind, const String &p_endpoint_id) {
    GDKRuntime *runtime = _get_runtime();
    if (!m_runtime_ready || runtime == nullptr || runtime->is_shutting_down()) {
        return;
    }

    emit_signal(
            "default_audio_endpoint_changed",
            static_cast<int64_t>(p_user_local_id.value),
            static_cast<int64_t>(static_cast<uint32_t>(p_kind)),
            p_endpoint_id);
}

void GDKUsers::reconcile_signed_out_user(const Ref<GDKUser> &p_user) {
    if (!p_user.is_valid()) {
        return;
    }

    on_user_change(p_user->get_native_local_id(), XUserChangeEvent::SignedOut);
}

void GDKUsers::complete_add_user(XUserHandle p_user_handle, const Ref<GDKPendingSignal> &p_pending_signal) {
    Ref<GDKUser> user;
    user.instantiate();

    HRESULT hr = user->adopt_handle(p_user_handle);
    if (FAILED(hr)) {
        // adopt_handle() closes the handle itself when population fails, and only
        // rejects without taking ownership when the handle is already null.
        Ref<GDKResult> result = GDKResult::hresult_error(hr, "Failed to translate the native XUser into a Godot wrapper.", "user_wrapper_create_failed");
        p_pending_signal->complete(result);
        return;
    }

    const bool is_new_user = _add_or_update_user(user);
    const bool establish_primary_user = !m_primary_user.is_valid();
    if (establish_primary_user) {
        m_primary_user = user;
    }

    emit_signal("user_changed", user, is_new_user ? String("added") : String("signed_in_again"));

    p_pending_signal->complete(GDKResult::ok_result(user));
}

void CALLBACK GDKUsers::_user_change_callback(void *p_context, XUserLocalId p_user_local_id, XUserChangeEvent p_event) {
    auto *users = static_cast<GDKUsers *>(p_context);
    users->on_user_change(p_user_local_id, p_event);
}

void CALLBACK GDKUsers::_device_association_changed_callback(void *p_context, const XUserDeviceAssociationChange *p_change) {
    if (p_change == nullptr) {
        return;
    }

    auto *users = static_cast<GDKUsers *>(p_context);
    users->on_device_association_changed(*p_change);
}

void CALLBACK GDKUsers::_default_audio_endpoint_changed_callback(
        void *p_context,
        XUserLocalId p_user_local_id,
        XUserDefaultAudioEndpointKind p_kind,
        const wchar_t *p_endpoint_id_utf16) {
    auto *users = static_cast<GDKUsers *>(p_context);
    users->on_default_audio_endpoint_changed(
            p_user_local_id,
            p_kind,
            p_endpoint_id_utf16 != nullptr ? String(p_endpoint_id_utf16) : String());
}

GDKRuntime *GDKUsers::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Signal GDKUsers::_start_add_user_async(XUserAddOptions p_options, const String &p_action) {
    GDKRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return _make_users_error_signal(runtime, E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new AddUserAsyncContext(this, runtime, pending_signal, p_action);
    context->bind_cancel_handler();

    HRESULT hr = XUserAddAsync(p_options, context->get_async_block());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(hr, p_action, "user_add_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

bool GDKUsers::_add_or_update_user(const Ref<GDKUser> &p_user) {
    for (Ref<GDKUser> &existing : m_users) {
        if (existing.is_valid() && existing->get_local_id() == p_user->get_local_id()) {
            existing = p_user;
            return false;
        }
    }

    m_users.push_back(p_user);
    return true;
}

Ref<GDKUser> GDKUsers::_find_user_by_local_id(XUserLocalId p_user_local_id) const {
    for (const Ref<GDKUser> &user : m_users) {
        if (user.is_valid() && user->matches_local_id(p_user_local_id)) {
            return user;
        }
    }

    return Ref<GDKUser>();
}

void GDKUsers::_remove_user_by_local_id(XUserLocalId p_user_local_id) {
    m_users.erase(
            std::remove_if(
                    m_users.begin(),
                    m_users.end(),
                    [p_user_local_id](const Ref<GDKUser> &user) {
                        return user.is_null() || user->matches_local_id(p_user_local_id);
                    }),
            m_users.end());
}

Ref<GDKResult> GDKUsers::_wrap_found_handle(XUserHandle p_user_handle) {
    if (p_user_handle == nullptr) {
        return GDKResult::error_result(E_FAIL, "user_not_found", "The platform returned no user handle for the request.");
    }

    Ref<GDKUser> found;
    found.instantiate();

    HRESULT hr = found->adopt_handle(p_user_handle);
    if (FAILED(hr)) {
        // adopt_handle() closes the handle itself when population fails.
        return GDKResult::hresult_error(hr, "Failed to translate the native XUser into a Godot wrapper.", "user_wrapper_create_failed");
    }

    // Prefer the cached wrapper so callers get the same object identity that
    // get_users() / get_primary_user() hand out. The freshly opened handle is
    // released with the temporary wrapper above.
    Ref<GDKUser> cached = _find_user_by_local_id(found->get_native_local_id());
    if (cached.is_valid()) {
        return GDKResult::ok_result(cached);
    }

    return GDKResult::ok_result(found);
}

void GDKUsers::_update_device_association(const APP_LOCAL_DEVICE_ID &p_device_id, XUserLocalId p_user_local_id) {
    for (size_t i = 0; i < m_device_associations.size(); ++i) {
        if (!_device_ids_equal(m_device_associations[i].device_id, p_device_id)) {
            continue;
        }

        if (p_user_local_id.value == 0) {
            m_device_associations.erase(m_device_associations.begin() + static_cast<ptrdiff_t>(i));
        } else {
            m_device_associations[i].user_local_id = p_user_local_id;
        }
        return;
    }

    // A change to "no user" for a device we never tracked is a no-op.
    if (p_user_local_id.value == 0) {
        return;
    }

    DeviceAssociation association;
    association.device_id = p_device_id;
    association.user_local_id = p_user_local_id;
    m_device_associations.push_back(association);
}

} // namespace godot
