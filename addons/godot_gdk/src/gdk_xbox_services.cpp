#include "gdk_xbox_services.h"

#include <algorithm>
#include <cstdio>

#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/dictionary.hpp>

#include <XAsync.h>

#include "gdk_result.h"
#include "gdk_user.h"

namespace godot {

String GDKXboxServices::_build_default_scid(uint32_t p_title_id) {
    char buffer[37];
    std::snprintf(buffer, sizeof(buffer), "00000000-0000-0000-0000-0000%08x", p_title_id);
    return String(buffer);
}

String GDKXboxServices::_extract_scid_override(const Variant &p_config) {
    if (p_config.get_type() != Variant::DICTIONARY) {
        return String();
    }

    Dictionary config = p_config;
    if (config.has("scid")) {
        return String(config["scid"]);
    }
    if (config.has("service_configuration_id")) {
        return String(config["service_configuration_id"]);
    }
    if (config.has("xbox_live/scid")) {
        return String(config["xbox_live/scid"]);
    }
    if (config.has("xbox_live")) {
        Variant xbox_live_value = config["xbox_live"];
        if (xbox_live_value.get_type() == Variant::DICTIONARY) {
            Dictionary xbox_live = xbox_live_value;
            if (xbox_live.has("scid")) {
                return String(xbox_live["scid"]);
            }
        }
    }

    return String();
}

GDKXboxServices::UserContextState *GDKXboxServices::_find_user_context(XUserLocalId p_local_id) {
    for (UserContextState &state : m_user_contexts) {
        if (state.local_id.value == p_local_id.value) {
            return &state;
        }
    }

    return nullptr;
}

HRESULT GDKXboxServices::_ensure_user_context(const Ref<GDKUser> &p_user, UserContextState **r_context_state) {
    ERR_FAIL_COND_V(r_context_state == nullptr, E_POINTER);

    if (!m_initialized) {
        return E_FAIL;
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return E_INVALIDARG;
    }

    XUserLocalId local_id = {};
    local_id.value = static_cast<uint64_t>(p_user->get_local_id());

    UserContextState *existing_state = _find_user_context(local_id);
    if (existing_state != nullptr) {
        *r_context_state = existing_state;
        return S_OK;
    }

    XblContextHandle context = nullptr;
    HRESULT hr = XblContextCreateHandle(p_user->get_handle(), &context);
    if (FAILED(hr)) {
        return hr;
    }

    uint64_t xbox_user_id = 0;
    hr = XblContextGetXboxUserId(context, &xbox_user_id);
    if (FAILED(hr)) {
        XblContextCloseHandle(context);
        return hr;
    }

    UserContextState state;
    state.local_id = local_id;
    state.xbox_user_id = xbox_user_id;
    state.context = context;
    m_user_contexts.push_back(state);

    *r_context_state = &m_user_contexts.back();
    return S_OK;
}

GDKXboxServices::~GDKXboxServices() {
    shutdown();
}

Ref<GDKResult> GDKXboxServices::initialize(XTaskQueueHandle p_queue, const Variant &p_config) {
    shutdown();

    uint32_t title_id = 0;
    HRESULT title_hr = XGameGetXboxTitleId(&title_id);
    if (FAILED(title_hr)) {
        return GDKResult::hresult_error(
            title_hr,
            "Xbox title ID is unavailable; Xbox services were not initialized.",
            "xbox_title_id_unavailable");
    }

    String scid = _extract_scid_override(p_config);
    if (scid.is_empty()) {
        scid = _build_default_scid(title_id);
    }

    CharString scid_utf8 = scid.utf8();
    XblInitArgs init_args = {};
    init_args.queue = p_queue;
    init_args.scid = scid_utf8.get_data();

    HRESULT hr = XblInitialize(&init_args);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(hr, "Failed to initialize Xbox services.", "xbox_services_initialize_failed");
    }

    m_initialized = true;
    m_title_id = title_id;
    m_scid = scid;
    return GDKResult::ok_result();
}

void GDKXboxServices::shutdown() {
    for (UserContextState &state : m_user_contexts) {
        if (state.context != nullptr) {
            XblContextCloseHandle(state.context);
            state.context = nullptr;
        }
    }
    m_user_contexts.clear();

    if (m_initialized) {
        XAsyncBlock cleanup_async = {};

        HRESULT cleanup_hr = XblCleanupAsync(&cleanup_async);
        if (SUCCEEDED(cleanup_hr)) {
            XAsyncGetStatus(&cleanup_async, true);
        }
    }

    m_initialized = false;
    m_title_id = 0;
    m_scid = "";
}

bool GDKXboxServices::is_initialized() const {
    return m_initialized;
}

uint32_t GDKXboxServices::get_title_id() const {
    return m_title_id;
}

String GDKXboxServices::get_scid() const {
    return m_scid;
}

HRESULT GDKXboxServices::get_xbox_user_id(const Ref<GDKUser> &p_user, uint64_t *r_xbox_user_id) {
    ERR_FAIL_COND_V(r_xbox_user_id == nullptr, E_POINTER);

    UserContextState *state = nullptr;
    HRESULT hr = _ensure_user_context(p_user, &state);
    if (FAILED(hr)) {
        return hr;
    }

    *r_xbox_user_id = state->xbox_user_id;
    return S_OK;
}

HRESULT GDKXboxServices::duplicate_context_for_user(const Ref<GDKUser> &p_user, XblContextHandle *r_context, uint64_t *r_xbox_user_id) {
    ERR_FAIL_COND_V(r_context == nullptr, E_POINTER);

    UserContextState *state = nullptr;
    HRESULT hr = _ensure_user_context(p_user, &state);
    if (FAILED(hr)) {
        return hr;
    }

    hr = XblContextDuplicateHandle(state->context, r_context);
    if (FAILED(hr)) {
        return hr;
    }

    if (r_xbox_user_id != nullptr) {
        *r_xbox_user_id = state->xbox_user_id;
    }

    return S_OK;
}

void GDKXboxServices::forget_user(XUserLocalId p_local_id) {
    m_user_contexts.erase(
        std::remove_if(
            m_user_contexts.begin(),
            m_user_contexts.end(),
            [p_local_id](UserContextState &state) {
                if (state.local_id.value != p_local_id.value) {
                    return false;
                }

                if (state.context != nullptr) {
                    XblContextCloseHandle(state.context);
                    state.context = nullptr;
                }
                return true;
            }),
        m_user_contexts.end());
}

} // namespace godot
