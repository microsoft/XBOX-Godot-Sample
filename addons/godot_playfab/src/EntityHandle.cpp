#include "EntityHandle.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <playfab/core/PFEntity.h>

#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

PFEntityHandle EntityHandle::m_handle = nullptr;
bool EntityHandle::m_owns_handle = false;

EntityHandle::EntityHandle() {
}

EntityHandle::~EntityHandle() {
    close_handle();
}

void EntityHandle::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_entity_token"), &EntityHandle::get_entity_token);
    ClassDB::bind_method(D_METHOD("get_entity_key"), &EntityHandle::get_entity_key);
    ClassDB::bind_method(D_METHOD("is_title_player"), &EntityHandle::is_title_player);
    ClassDB::bind_method(D_METHOD("get_api_endpoint"), &EntityHandle::get_api_endpoint);
    ClassDB::bind_method(D_METHOD("get_title_id"), &EntityHandle::get_title_id);
    ClassDB::bind_method(D_METHOD("is_valid"), &EntityHandle::is_valid);
}

int EntityHandle::set_handle(PFEntityHandle p_handle, bool p_owns) {
    m_handle = p_handle;
    m_owns_handle = p_owns;
    return 1;
}

PFEntityHandle EntityHandle::get_handle() {
    return m_handle;
}

int EntityHandle::close_handle() {
    if (m_handle != nullptr && m_owns_handle) {
        PFEntityCloseHandle(m_handle);
    }
    m_handle = nullptr;
    m_owns_handle = false;
    return 1;
}

String EntityHandle::get_entity_token() {
    if (m_handle == nullptr) {
        UtilityFunctions::printerr("EntityHandle: handle is null");
        return String();
    }

    XAsyncBlock async = {};
    HRESULT hr = PFEntityGetEntityTokenAsync(m_handle, &async);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetEntityTokenAsync failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    hr = XAsyncGetStatus(&async, true);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: XAsyncGetStatus failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    size_t buffer_size = 0;
    hr = PFEntityGetEntityTokenResultSize(&async, &buffer_size);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetEntityTokenResultSize failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    std::vector<char> buffer(buffer_size);
    const PFEntityToken *token = nullptr;
    hr = PFEntityGetEntityTokenResult(&async, buffer_size, buffer.data(), &token, nullptr);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetEntityTokenResult failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    return String(token->token);
}

Dictionary EntityHandle::get_entity_key() {
    Dictionary result;

    if (m_handle == nullptr) {
        UtilityFunctions::printerr("EntityHandle: handle is null");
        return result;
    }

    size_t buffer_size = 0;
    HRESULT hr = PFEntityGetEntityKeySize(m_handle, &buffer_size);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetEntityKeySize failed with HRESULT 0x", String::num_int64(hr, 16));
        return result;
    }

    std::vector<char> buffer(buffer_size);
    const PFEntityKey *entity_key = nullptr;
    hr = PFEntityGetEntityKey(m_handle, buffer_size, buffer.data(), &entity_key, nullptr);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetEntityKey failed with HRESULT 0x", String::num_int64(hr, 16));
        return result;
    }

    result["id"] = String(entity_key->id);
    result["type"] = String(entity_key->type);
    return result;
}

bool EntityHandle::is_title_player() {
    if (m_handle == nullptr) {
        UtilityFunctions::printerr("EntityHandle: handle is null");
        return false;
    }

    bool is_tp = false;
    HRESULT hr = PFEntityIsTitlePlayer(m_handle, &is_tp);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityIsTitlePlayer failed with HRESULT 0x", String::num_int64(hr, 16));
        return false;
    }

    return is_tp;
}

String EntityHandle::get_api_endpoint() {
    if (m_handle == nullptr) {
        UtilityFunctions::printerr("EntityHandle: handle is null");
        return String();
    }

    size_t endpoint_size = 0;
    HRESULT hr = PFEntityGetAPIEndpointSize(m_handle, &endpoint_size);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetAPIEndpointSize failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    std::vector<char> buffer(endpoint_size);
    hr = PFEntityGetAPIEndpoint(m_handle, endpoint_size, buffer.data(), nullptr);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetAPIEndpoint failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    return String(buffer.data());
}

String EntityHandle::get_title_id() {
    if (m_handle == nullptr) {
        UtilityFunctions::printerr("EntityHandle: handle is null");
        return String();
    }

    size_t title_id_size = 0;
    HRESULT hr = PFEntityGetTitleIdSize(m_handle, &title_id_size);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetTitleIdSize failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    std::vector<char> buffer(title_id_size);
    hr = PFEntityGetTitleId(m_handle, title_id_size, buffer.data(), nullptr);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("EntityHandle: PFEntityGetTitleId failed with HRESULT 0x", String::num_int64(hr, 16));
        return String();
    }

    return String(buffer.data());
}

bool EntityHandle::is_valid() const {
    return m_handle != nullptr;
}

} // namespace godot
