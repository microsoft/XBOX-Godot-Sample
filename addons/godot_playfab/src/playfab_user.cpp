#include "playfab_user.h"

#include <vector>

#include <playfab/core/PFLocalUser_Xbox.h>

namespace godot {

void PlayFabUser::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_local_id"), &PlayFabUser::get_local_id);
    ClassDB::bind_method(D_METHOD("get_entity_key"), &PlayFabUser::get_entity_key);

    ADD_PROPERTY(PropertyInfo(Variant::INT, "local_id"), "", "get_local_id");
    ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "entity_key"), "", "get_entity_key");
}

PlayFabUser::PlayFabUser() {}

PlayFabUser::~PlayFabUser() {
    clear();
}

uint64_t PlayFabUser::get_local_id() const {
    return m_local_id.value;
}

Dictionary PlayFabUser::get_entity_key() const {
    Dictionary key;
    key["id"] = m_entity_id;
    key["type"] = m_entity_type;
    return key;
}

HRESULT PlayFabUser::populate_from_user_handle(XUserHandle p_user_handle) {
    XUserLocalId local_id = {};
    HRESULT hr = XUserGetLocalId(p_user_handle, &local_id);
    if (FAILED(hr)) {
        return hr;
    }

    m_local_id = local_id;
    return S_OK;
}

HRESULT PlayFabUser::populate_from_entity_handle(PFEntityHandle p_entity_handle) {
    size_t entity_key_size = 0;
    HRESULT hr = PFEntityGetEntityKeySize(p_entity_handle, &entity_key_size);
    if (FAILED(hr)) {
        return hr;
    }

    std::vector<char> entity_key_buffer(entity_key_size);
    const PFEntityKey *entity_key = nullptr;
    hr = PFEntityGetEntityKey(
            p_entity_handle,
            entity_key_buffer.size(),
            entity_key_buffer.data(),
            &entity_key,
            nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    m_entity_id = entity_key != nullptr && entity_key->id != nullptr ? String::utf8(entity_key->id) : String();
    m_entity_type = entity_key != nullptr && entity_key->type != nullptr ? String::utf8(entity_key->type) : String();
    return S_OK;
}

HRESULT PlayFabUser::adopt_session(XUserHandle p_user_handle, PFEntityHandle p_entity_handle, PFServiceConfigHandle p_service_config_handle) {
    clear();

    if (p_user_handle == nullptr || p_entity_handle == nullptr || p_service_config_handle == nullptr) {
        return E_INVALIDARG;
    }

    m_entity_handle = p_entity_handle;

    HRESULT hr = PFLocalUserCreateHandleWithXboxUser(
            p_service_config_handle,
            p_user_handle,
            nullptr,
            &m_local_user_handle);
    if (FAILED(hr)) {
        clear();
        return hr;
    }

    hr = populate_from_user_handle(p_user_handle);
    if (FAILED(hr)) {
        clear();
        return hr;
    }

    hr = populate_from_entity_handle(m_entity_handle);
    if (FAILED(hr)) {
        clear();
        return hr;
    }

    return S_OK;
}

bool PlayFabUser::matches_local_id(XUserLocalId p_local_id) const {
    return m_local_id.value == p_local_id.value;
}

PFEntityHandle PlayFabUser::get_entity_handle() const {
    return m_entity_handle;
}

HRESULT PlayFabUser::duplicate_local_user_handle(PFLocalUserHandle *r_local_user_handle) const {
    if (r_local_user_handle == nullptr) {
        return E_INVALIDARG;
    }

    *r_local_user_handle = nullptr;
    if (m_local_user_handle == nullptr) {
        return E_HANDLE;
    }

    return PFLocalUserDuplicateHandle(m_local_user_handle, r_local_user_handle);
}

void PlayFabUser::clear() {
    if (m_local_user_handle != nullptr) {
        PFLocalUserCloseHandle(m_local_user_handle);
        m_local_user_handle = nullptr;
    }

    if (m_entity_handle != nullptr) {
        PFEntityCloseHandle(m_entity_handle);
        m_entity_handle = nullptr;
    }

    m_local_id = {};
    m_entity_id = "";
    m_entity_type = "";
}

} // namespace godot
