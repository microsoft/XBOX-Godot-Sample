#include "PlayFabCore.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <XGameRuntimeInit.h>
#include <playfab/core/PFCore.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

PlayFabCore *PlayFabCore::singleton = nullptr;

PlayFabCore *PlayFabCore::get_singleton() {
    return singleton;
}

PlayFabCore::PlayFabCore() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

PlayFabCore::~PlayFabCore() {
    shutdown();
    singleton = nullptr;
}

void PlayFabCore::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &PlayFabCore::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &PlayFabCore::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFabCore::is_initialized);

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
}

int PlayFabCore::initialize() {
    if (m_initialized) {
        UtilityFunctions::printerr("PlayFabCore: already initialized");
        return 0;
    }
    HRESULT hr = XGameRuntimeInitialize();  
    if (FAILED(hr)) {
        UtilityFunctions::printerr("PlayFabCore: XGameRuntimeInitialize failed with HRESULT 0x", String::num_int64(hr, 16));
        return 0;
    }  

    hr = PFInitialize(nullptr);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("PlayFabCore: PFInitialize failed with HRESULT 0x", String::num_int64(hr, 16));
        return 0;
    }

    m_initialized = true;
    emit_signal("initialized");
    UtilityFunctions::print("PlayFabCore: initialized");
    return 1;
}

void PlayFabCore::shutdown() {
    if (!m_initialized) {
        return;
    }
    XAsyncBlock async = {};
    HRESULT hr = PFUninitializeAsync(&async);
    if (SUCCEEDED(hr)) {
        XAsyncGetStatus(&async, true);
    }

    m_initialized = false;

    XGameRuntimeUninitialize();
    emit_signal("shutdown_completed");
    UtilityFunctions::print("PlayFabCore: shut down");
}

bool PlayFabCore::is_initialized() const {
    return m_initialized;
}

} // namespace godot