#include "PlayFabMultiplayer.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <PFMultiplayer.h>

#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

namespace godot {

PlayFabMultiplayer *PlayFabMultiplayer::singleton = nullptr;

PlayFabMultiplayer *PlayFabMultiplayer::get_singleton() {
    return singleton;
}

PlayFabMultiplayer::PlayFabMultiplayer() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

PlayFabMultiplayer::~PlayFabMultiplayer() {
    shutdown();
    singleton = nullptr;
}

void PlayFabMultiplayer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "title_id"), &PlayFabMultiplayer::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &PlayFabMultiplayer::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFabMultiplayer::is_initialized);

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
}

int PlayFabMultiplayer::initialize(const String &p_title_id) {
    if (m_initialized) {
        UtilityFunctions::printerr("PlayFabMultiplayer: already initialized");
        return 0;
    }

    if (p_title_id.is_empty()) {
        UtilityFunctions::printerr("PlayFabMultiplayer: title_id must not be empty");
        return 0;
    }

    m_title_id = p_title_id;

    std::string title_id_std = m_title_id.utf8().get_data();

    MultiplayerInitializationConfiguration config = {};
    config.titleId = title_id_std.c_str();
    config.multiplayerTaskQueue = nullptr;

    HRESULT hr = PFMultiplayerInitialize(&config, &m_multiplayer_handle);
    if (FAILED(hr)) {
        UtilityFunctions::printerr("PlayFabMultiplayer: PFMultiplayerInitialize failed with HRESULT 0x", String::num_int64(hr, 16));
        return 0;
    }

    m_initialized = true;
    emit_signal("initialized");
    UtilityFunctions::print("PlayFabMultiplayer: initialized for title ", m_title_id);
    return 1;
}

void PlayFabMultiplayer::shutdown() {
    if (!m_initialized) {
        return;
    }

    if (m_multiplayer_handle != nullptr) {
        PFMultiplayerUninitialize(m_multiplayer_handle);
        m_multiplayer_handle = nullptr;
    }

    m_initialized = false;
    emit_signal("shutdown_completed");
    UtilityFunctions::print("PlayFabMultiplayer: shut down");
}

bool PlayFabMultiplayer::is_initialized() const {
    return m_initialized;
}

PFMultiplayerHandle PlayFabMultiplayer::get_multiplayer_handle() const {
    return m_multiplayer_handle;
}

} // namespace godot
