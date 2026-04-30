#include "PlayFabParty.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include <godot_cpp/core/print_string.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace Party;

namespace godot {

PlayFabParty *PlayFabParty::singleton = nullptr;

PlayFabParty *PlayFabParty::get_singleton() {
    return singleton;
}

PlayFabParty::PlayFabParty() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

PlayFabParty::~PlayFabParty() {
    shutdown();
    singleton = nullptr;
}

void PlayFabParty::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "title_id"), &PlayFabParty::initialize);
    ClassDB::bind_method(D_METHOD("create_local_user", "entity_handle"), &PlayFabParty::create_local_user);
    ClassDB::bind_method(D_METHOD("shutdown"), &PlayFabParty::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &PlayFabParty::is_initialized);

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
}

int PlayFabParty::initialize(const String &p_title_id) {
    if (m_initialized) {
        UtilityFunctions::printerr("PlayFabParty: already initialized");
        return 0;
    }

    if (p_title_id.is_empty()) {
        UtilityFunctions::printerr("PlayFabParty: title_id must not be empty");
        return 0;
    }

    m_title_id = p_title_id;

    std::string title_id_std = m_title_id.utf8().get_data();

    PartyInitializationConfiguration config = {};
    config.titleId = title_id_std.c_str();

    PartyError err = m_party_manager.Initialize(&config);
    if (PARTY_FAILED(err)) {
        PartyString error_message = nullptr;
        PartyGetErrorMessage(err, &error_message);
        UtilityFunctions::printerr("PlayFabParty: Initialize failed: ", error_message ? error_message : "unknown error");
        return 0;
    }

    m_initialized = true;
    emit_signal("initialized");
    UtilityFunctions::print("PlayFabParty: initialized for title ", m_title_id);
    return 1;
}

int PlayFabParty::create_local_user() {
    if (!m_initialized) {
        UtilityFunctions::printerr("PlayFabParty: not initialized");
        return 0;
    }

    PartyLocalUser* local_user = nullptr;
    PFEntityHandle entityHandle = EntityHandle::get_handle();
    PartyError err = m_party_manager.CreateLocalUser(entityHandle, &local_user);
    if (PARTY_FAILED(err)) {
        PartyString error_message = nullptr;
        PartyGetErrorMessage(err, &error_message);
        UtilityFunctions::printerr("PlayFabParty: CreateLocalUser failed: ", error_message ? error_message : "unknown error");
        return 0;
    }

    UtilityFunctions::print("PlayFabParty: local user created");
    return 1;
}

void PlayFabParty::shutdown() {
    if (!m_initialized) {
        return;
    }

    PartyError err = m_party_manager.Cleanup();
    if (PARTY_FAILED(err)) {
        PartyString error_message = nullptr;
        PartyGetErrorMessage(err, &error_message);
        UtilityFunctions::printerr("PlayFabParty: Shutdown failed: ", error_message ? error_message : "unknown error");
    }

    m_initialized = false;
    emit_signal("shutdown_completed");
    UtilityFunctions::print("PlayFabParty: shut down");
}

bool PlayFabParty::is_initialized() const {
    return m_initialized;
}

} // namespace godot
