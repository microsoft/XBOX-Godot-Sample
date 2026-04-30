#include "PlayFabServices.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <playfab/services/PFServices.h>

#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

PlayFabServices *PlayFabServices::singleton = nullptr;

PlayFabServices *PlayFabServices::get_singleton() {
    return singleton;
}

PlayFabServices::PlayFabServices() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

PlayFabServices::~PlayFabServices() {
    shutdown();
    singleton = nullptr;
}

void PlayFabServices::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "title_id"), &PlayFabServices::initialize);
    ClassDB::bind_method(D_METHOD("authentication_login_with_custom_id", "custom_id"), &PlayFabServices::AuthenticationLoginWithCustomIDAsync);
    ClassDB::bind_method(D_METHOD("shutdown"), &PlayFabServices::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFabServices::is_initialized);
    ClassDB::bind_method(D_METHOD("get_title_id"), &PlayFabServices::get_title_id);
    ClassDB::bind_method(D_METHOD("get_endpoint"), &PlayFabServices::get_endpoint);

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
}

int PlayFabServices::initialize(const String &p_title_id) {
    if (m_initialized) {
        UtilityFunctions::printerr("PlayFabServices: already initialized");
        return 0;
    }

    if (p_title_id.is_empty()) {
        UtilityFunctions::printerr("PlayFabServices: title_id must not be empty");
        return 0;
    }

    m_title_id = p_title_id;
    m_endpoint = "https://" + m_title_id + ".playfabapi.com";

    HRESULT hr = PFServicesInitialize(nullptr);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("PlayFabServices: PFServicesInitialize failed with HRESULT 0x", String::num_int64(hr, 16));
        return 0;
    }

    std::string endpoint_std = m_endpoint.utf8().get_data();
    std::string title_id_std = m_title_id.utf8().get_data();

    hr = PFServiceConfigCreateHandle(endpoint_std.c_str(), title_id_std.c_str(), &m_service_config_handle);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("PlayFabServices: PFServiceConfigCreateHandle failed with HRESULT 0x", String::num_int64(hr, 16));
        return 0;
    }

    m_initialized = true;
    emit_signal("initialized");
    UtilityFunctions::print("PlayFabServices: initialized for title ", m_title_id);
    return 1;
}

int PlayFabServices::AuthenticationLoginWithCustomIDAsync(const String& p_custom_id) {
    // Log Player with LoginWithCustomIDRequest
    PFAuthenticationLoginWithCustomIDRequest request{};
    request.createAccount = true;
    request.customId = p_custom_id.utf8().get_data();
    XAsyncBlock async{};
    HRESULT hr = PFAuthenticationLoginWithCustomIDAsync(m_service_config_handle, &request, &async);
    hr = XAsyncGetStatus(&async, true);

    // Prepare login result buffer 
    std::vector<char> loginResultBuffer;
    PFAuthenticationLoginResult const* loginResult;
    size_t bufferSize;
    hr = PFAuthenticationLoginWithCustomIDGetResultSize(&async, &bufferSize);
    loginResultBuffer.resize(bufferSize);
    PFEntityHandle entityHandle = nullptr;
    hr = PFAuthenticationLoginWithCustomIDGetResult(&async, &entityHandle, loginResultBuffer.size(), loginResultBuffer.data(), &loginResult, nullptr);
    EntityHandle::set_handle(entityHandle, true);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("PlayFabCore: PFAuthenticationLoginWithCustomIDAsync failed with HRESULT 0x", String::num_int64(hr, 16));
        return 0;
    }
    return 1;
}

void PlayFabServices::shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_service_config_handle != nullptr) {
        PFServiceConfigCloseHandle(m_service_config_handle);
        m_service_config_handle = nullptr;
    }

    XAsyncBlock async = {};
    HRESULT hr = PFServicesUninitializeAsync(&async);
    if (SUCCEEDED(hr)) {
        XAsyncGetStatus(&async, true);
    }

    m_initialized = false;
    emit_signal("shutdown_completed");
    UtilityFunctions::print("PlayFabServices: shut down");
}

bool PlayFabServices::is_initialized() const {
    return m_initialized;
}

String PlayFabServices::get_title_id() const {
    return m_title_id;
}

String PlayFabServices::get_endpoint() const {
    return m_endpoint;
}

} // namespace godot