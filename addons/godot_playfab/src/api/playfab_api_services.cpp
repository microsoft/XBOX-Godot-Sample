// PlayFab API binding file.
#include "api/playfab_api_services.h"
#include "api/playfab_api_models.h"
#include "playfab.h"

#include <memory>

#include <playfab/core/PFEvents.h>
#include <playfab/services/PFAccountManagement.h>
#include <playfab/services/PFCatalog.h>
#include <playfab/services/PFCloudScript.h>
#include <playfab/services/PFData.h>
#include <playfab/services/PFExperimentation.h>
#include <playfab/services/PFFriends.h>
#include <playfab/services/PFGroups.h>
#include <playfab/services/PFInventory.h>
#include <playfab/services/PFLocalization.h>
#include <playfab/services/PFPlayerDataManagement.h>
#include <playfab/services/PFProfiles.h>
#include <playfab/services/PFStatistics.h>
#include <playfab/services/PFTitleDataManagement.h>

namespace godot {
using namespace playfab_api;
namespace {
Signal make_api_call_error(PlayFabRuntime *runtime, HRESULT hr, const String &code, const String &message) { return playfab_api::make_error_signal(runtime, hr, code, message); }
bool validate_api_user(const Ref<PlayFabUser> &user) { return user.is_valid() && user->get_entity_handle() != nullptr; }
}

void PlayFabAccounts::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_or_update_contact_email_async", "user", "request"), &PlayFabAccounts::add_or_update_contact_email_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_account_info_async", "user", "request"), &PlayFabAccounts::get_account_info_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_player_combined_info_async", "user", "request"), &PlayFabAccounts::get_player_combined_info_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_player_profile_async", "user", "request"), &PlayFabAccounts::get_player_profile_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_play_fab_ids_from_battle_net_account_ids_async", "user", "request"), &PlayFabAccounts::get_play_fab_ids_from_battle_net_account_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_play_fab_ids_from_google_ids_async", "user", "request"), &PlayFabAccounts::get_play_fab_ids_from_google_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_play_fab_ids_from_kongregate_ids_async", "user", "request"), &PlayFabAccounts::get_play_fab_ids_from_kongregate_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_play_fab_ids_from_steam_ids_async", "user", "request"), &PlayFabAccounts::get_play_fab_ids_from_steam_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_play_fab_ids_from_steam_names_async", "user", "request"), &PlayFabAccounts::get_play_fab_ids_from_steam_names_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_play_fab_ids_from_xbox_live_ids_async", "user", "request"), &PlayFabAccounts::get_play_fab_ids_from_xbox_live_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("link_battle_net_account_async", "user", "request"), &PlayFabAccounts::link_battle_net_account_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("link_custom_id_async", "user", "request"), &PlayFabAccounts::link_custom_id_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("link_open_id_connect_async", "user", "request"), &PlayFabAccounts::link_open_id_connect_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("link_steam_account_async", "user", "request"), &PlayFabAccounts::link_steam_account_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("link_xbox_account_async", "user", "request"), &PlayFabAccounts::link_xbox_account_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("remove_contact_email_async", "user", "request"), &PlayFabAccounts::remove_contact_email_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("report_player_async", "user", "request"), &PlayFabAccounts::report_player_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("unlink_battle_net_account_async", "user", "request"), &PlayFabAccounts::unlink_battle_net_account_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("unlink_custom_id_async", "user", "request"), &PlayFabAccounts::unlink_custom_id_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("unlink_open_id_connect_async", "user", "request"), &PlayFabAccounts::unlink_open_id_connect_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("unlink_steam_account_async", "user", "request"), &PlayFabAccounts::unlink_steam_account_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("unlink_xbox_account_async", "user", "request"), &PlayFabAccounts::unlink_xbox_account_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_avatar_url_async", "user", "request"), &PlayFabAccounts::update_avatar_url_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_user_title_display_name_async", "user", "request"), &PlayFabAccounts::update_user_title_display_name_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_title_players_from_xbox_live_ids_async", "user", "request"), &PlayFabAccounts::get_title_players_from_xbox_live_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_display_name_async", "user", "request"), &PlayFabAccounts::set_display_name_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_profile_async", "user", "request"), &PlayFabAccounts::get_profile_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_profiles_async", "user", "request"), &PlayFabAccounts::get_profiles_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_title_players_from_master_player_account_ids_async", "user", "request"), &PlayFabAccounts::get_title_players_from_master_player_account_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_profile_language_async", "user", "request"), &PlayFabAccounts::set_profile_language_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_profile_policy_async", "user", "request"), &PlayFabAccounts::set_profile_policy_async, DEFVAL(Dictionary()));
}
void PlayFabAccounts::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabAccounts::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabAccounts::add_or_update_contact_email_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementAddOrUpdateContactEmailRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementAddOrUpdateContactEmailRequest, PFAccountManagementAddOrUpdateContactEmailRequest, PFAccountManagementClientAddOrUpdateContactEmailAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientAddOrUpdateContactEmail");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientAddOrUpdateContactEmail.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_account_info_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetAccountInfoRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetAccountInfoRequest, PFAccountManagementGetAccountInfoRequest, PFAccountManagementGetAccountInfoResult, PFAccountManagementClientGetAccountInfoAsync, PFAccountManagementClientGetAccountInfoGetResultSize, PFAccountManagementClientGetAccountInfoGetResult, to_variant_PFAccountManagementGetAccountInfoResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetAccountInfo");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetAccountInfo.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_player_combined_info_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayerCombinedInfoRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayerCombinedInfoRequest, PFAccountManagementGetPlayerCombinedInfoRequest, PFAccountManagementGetPlayerCombinedInfoResult, PFAccountManagementClientGetPlayerCombinedInfoAsync, PFAccountManagementClientGetPlayerCombinedInfoGetResultSize, PFAccountManagementClientGetPlayerCombinedInfoGetResult, to_variant_PFAccountManagementGetPlayerCombinedInfoResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayerCombinedInfo");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayerCombinedInfo.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_player_profile_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayerProfileRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayerProfileRequest, PFAccountManagementGetPlayerProfileRequest, PFAccountManagementGetPlayerProfileResult, PFAccountManagementClientGetPlayerProfileAsync, PFAccountManagementClientGetPlayerProfileGetResultSize, PFAccountManagementClientGetPlayerProfileGetResult, to_variant_PFAccountManagementGetPlayerProfileResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayerProfile");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayerProfile.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_play_fab_ids_from_battle_net_account_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsRequest, PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsRequest, PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsResult, PFAccountManagementClientGetPlayFabIDsFromBattleNetAccountIdsAsync, PFAccountManagementClientGetPlayFabIDsFromBattleNetAccountIdsGetResultSize, PFAccountManagementClientGetPlayFabIDsFromBattleNetAccountIdsGetResult, to_variant_PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayFabIDsFromBattleNetAccountIds");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayFabIDsFromBattleNetAccountIds.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_play_fab_ids_from_google_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayFabIDsFromGoogleIDsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayFabIDsFromGoogleIDsRequest, PFAccountManagementGetPlayFabIDsFromGoogleIDsRequest, PFAccountManagementGetPlayFabIDsFromGoogleIDsResult, PFAccountManagementClientGetPlayFabIDsFromGoogleIDsAsync, PFAccountManagementClientGetPlayFabIDsFromGoogleIDsGetResultSize, PFAccountManagementClientGetPlayFabIDsFromGoogleIDsGetResult, to_variant_PFAccountManagementGetPlayFabIDsFromGoogleIDsResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayFabIDsFromGoogleIDs");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayFabIDsFromGoogleIDs.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_play_fab_ids_from_kongregate_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayFabIDsFromKongregateIDsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayFabIDsFromKongregateIDsRequest, PFAccountManagementGetPlayFabIDsFromKongregateIDsRequest, PFAccountManagementGetPlayFabIDsFromKongregateIDsResult, PFAccountManagementClientGetPlayFabIDsFromKongregateIDsAsync, PFAccountManagementClientGetPlayFabIDsFromKongregateIDsGetResultSize, PFAccountManagementClientGetPlayFabIDsFromKongregateIDsGetResult, to_variant_PFAccountManagementGetPlayFabIDsFromKongregateIDsResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayFabIDsFromKongregateIDs");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayFabIDsFromKongregateIDs.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_play_fab_ids_from_steam_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayFabIDsFromSteamIDsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayFabIDsFromSteamIDsRequest, PFAccountManagementGetPlayFabIDsFromSteamIDsRequest, PFAccountManagementGetPlayFabIDsFromSteamIDsResult, PFAccountManagementClientGetPlayFabIDsFromSteamIDsAsync, PFAccountManagementClientGetPlayFabIDsFromSteamIDsGetResultSize, PFAccountManagementClientGetPlayFabIDsFromSteamIDsGetResult, to_variant_PFAccountManagementGetPlayFabIDsFromSteamIDsResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayFabIDsFromSteamIDs");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayFabIDsFromSteamIDs.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_play_fab_ids_from_steam_names_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayFabIDsFromSteamNamesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayFabIDsFromSteamNamesRequest, PFAccountManagementGetPlayFabIDsFromSteamNamesRequest, PFAccountManagementGetPlayFabIDsFromSteamNamesResult, PFAccountManagementClientGetPlayFabIDsFromSteamNamesAsync, PFAccountManagementClientGetPlayFabIDsFromSteamNamesGetResultSize, PFAccountManagementClientGetPlayFabIDsFromSteamNamesGetResult, to_variant_PFAccountManagementGetPlayFabIDsFromSteamNamesResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayFabIDsFromSteamNames");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayFabIDsFromSteamNames.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_play_fab_ids_from_xbox_live_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetPlayFabIDsFromXboxLiveIDsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetPlayFabIDsFromXboxLiveIDsRequest, PFAccountManagementGetPlayFabIDsFromXboxLiveIDsRequest, PFAccountManagementGetPlayFabIDsFromXboxLiveIDsResult, PFAccountManagementClientGetPlayFabIDsFromXboxLiveIDsAsync, PFAccountManagementClientGetPlayFabIDsFromXboxLiveIDsGetResultSize, PFAccountManagementClientGetPlayFabIDsFromXboxLiveIDsGetResult, to_variant_PFAccountManagementGetPlayFabIDsFromXboxLiveIDsResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientGetPlayFabIDsFromXboxLiveIDs");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientGetPlayFabIDsFromXboxLiveIDs.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::link_battle_net_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementClientLinkBattleNetAccountRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementClientLinkBattleNetAccountRequest, PFAccountManagementClientLinkBattleNetAccountRequest, PFAccountManagementClientLinkBattleNetAccountAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientLinkBattleNetAccount");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientLinkBattleNetAccount.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::link_custom_id_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementLinkCustomIDRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementLinkCustomIDRequest, PFAccountManagementLinkCustomIDRequest, PFAccountManagementClientLinkCustomIDAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientLinkCustomID");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientLinkCustomID.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::link_open_id_connect_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementLinkOpenIdConnectRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementLinkOpenIdConnectRequest, PFAccountManagementLinkOpenIdConnectRequest, PFAccountManagementClientLinkOpenIdConnectAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientLinkOpenIdConnect");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientLinkOpenIdConnect.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::link_steam_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementLinkSteamAccountRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementLinkSteamAccountRequest, PFAccountManagementLinkSteamAccountRequest, PFAccountManagementClientLinkSteamAccountAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientLinkSteamAccount");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientLinkSteamAccount.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::link_xbox_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementClientLinkXboxAccountRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementClientLinkXboxAccountRequest, PFAccountManagementClientLinkXboxAccountRequest, PFAccountManagementClientLinkXboxAccountAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientLinkXboxAccount");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientLinkXboxAccount.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::remove_contact_email_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementRemoveContactEmailRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementRemoveContactEmailRequest, PFAccountManagementRemoveContactEmailRequest, PFAccountManagementClientRemoveContactEmailAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientRemoveContactEmail");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientRemoveContactEmail.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::report_player_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementReportPlayerClientRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFAccountManagementReportPlayerClientRequest, PFAccountManagementReportPlayerClientRequest, PFAccountManagementReportPlayerClientResult, PFAccountManagementClientReportPlayerAsync, PFAccountManagementClientReportPlayerGetResult, to_variant_PFAccountManagementReportPlayerClientResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientReportPlayer");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientReportPlayer.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::unlink_battle_net_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementClientUnlinkBattleNetAccountRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementClientUnlinkBattleNetAccountRequest, PFAccountManagementClientUnlinkBattleNetAccountRequest, PFAccountManagementClientUnlinkBattleNetAccountAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUnlinkBattleNetAccount");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUnlinkBattleNetAccount.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::unlink_custom_id_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementUnlinkCustomIDRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementUnlinkCustomIDRequest, PFAccountManagementUnlinkCustomIDRequest, PFAccountManagementClientUnlinkCustomIDAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUnlinkCustomID");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUnlinkCustomID.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::unlink_open_id_connect_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementUnlinkOpenIdConnectRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementUnlinkOpenIdConnectRequest, PFAccountManagementUnlinkOpenIdConnectRequest, PFAccountManagementClientUnlinkOpenIdConnectAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUnlinkOpenIdConnect");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUnlinkOpenIdConnect.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::unlink_steam_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementUnlinkSteamAccountRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementUnlinkSteamAccountRequest, PFAccountManagementUnlinkSteamAccountRequest, PFAccountManagementClientUnlinkSteamAccountAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUnlinkSteamAccount");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUnlinkSteamAccount.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::unlink_xbox_account_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementClientUnlinkXboxAccountRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementClientUnlinkXboxAccountRequest, PFAccountManagementClientUnlinkXboxAccountRequest, PFAccountManagementClientUnlinkXboxAccountAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUnlinkXboxAccount");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUnlinkXboxAccount.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::update_avatar_url_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementClientUpdateAvatarUrlRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFAccountManagementClientUpdateAvatarUrlRequest, PFAccountManagementClientUpdateAvatarUrlRequest, PFAccountManagementClientUpdateAvatarUrlAsync>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUpdateAvatarUrl");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUpdateAvatarUrl.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::update_user_title_display_name_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementUpdateUserTitleDisplayNameRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementUpdateUserTitleDisplayNameRequest, PFAccountManagementUpdateUserTitleDisplayNameRequest, PFAccountManagementUpdateUserTitleDisplayNameResult, PFAccountManagementClientUpdateUserTitleDisplayNameAsync, PFAccountManagementClientUpdateUserTitleDisplayNameGetResultSize, PFAccountManagementClientUpdateUserTitleDisplayNameGetResult, to_variant_PFAccountManagementUpdateUserTitleDisplayNameResult>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementClientUpdateUserTitleDisplayName");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementClientUpdateUserTitleDisplayName.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_title_players_from_xbox_live_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementGetTitlePlayersFromXboxLiveIDsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementGetTitlePlayersFromXboxLiveIDsRequest, PFAccountManagementGetTitlePlayersFromXboxLiveIDsRequest, PFAccountManagementGetTitlePlayersFromProviderIDsResponse, PFAccountManagementGetTitlePlayersFromXboxLiveIDsAsync, PFAccountManagementGetTitlePlayersFromXboxLiveIDsGetResultSize, PFAccountManagementGetTitlePlayersFromXboxLiveIDsGetResult, to_variant_PFAccountManagementGetTitlePlayersFromProviderIDsResponse>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementGetTitlePlayersFromXboxLiveIDs");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementGetTitlePlayersFromXboxLiveIDs.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::set_display_name_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFAccountManagementSetDisplayNameRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFAccountManagementSetDisplayNameRequest, PFAccountManagementSetDisplayNameRequest, PFAccountManagementSetDisplayNameResponse, PFAccountManagementSetDisplayNameAsync, PFAccountManagementSetDisplayNameGetResultSize, PFAccountManagementSetDisplayNameGetResult, to_variant_PFAccountManagementSetDisplayNameResponse>(runtime, pending_signal, p_user, std::move(request), "PFAccountManagementSetDisplayName");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFAccountManagementSetDisplayName.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_profile_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFProfilesGetEntityProfileRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFProfilesGetEntityProfileRequest, PFProfilesGetEntityProfileRequest, PFProfilesGetEntityProfileResponse, PFProfilesGetProfileAsync, PFProfilesGetProfileGetResultSize, PFProfilesGetProfileGetResult, to_variant_PFProfilesGetEntityProfileResponse>(runtime, pending_signal, p_user, std::move(request), "PFProfilesGetProfile");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFProfilesGetProfile.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_profiles_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFProfilesGetEntityProfilesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFProfilesGetEntityProfilesRequest, PFProfilesGetEntityProfilesRequest, PFProfilesGetEntityProfilesResponse, PFProfilesGetProfilesAsync, PFProfilesGetProfilesGetResultSize, PFProfilesGetProfilesGetResult, to_variant_PFProfilesGetEntityProfilesResponse>(runtime, pending_signal, p_user, std::move(request), "PFProfilesGetProfiles");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFProfilesGetProfiles.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::get_title_players_from_master_player_account_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest, PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest, PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse, PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsAsync, PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsGetResultSize, PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsGetResult, to_variant_PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse>(runtime, pending_signal, p_user, std::move(request), "PFProfilesGetTitlePlayersFromMasterPlayerAccountIds");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFProfilesGetTitlePlayersFromMasterPlayerAccountIds.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::set_profile_language_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFProfilesSetProfileLanguageRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFProfilesSetProfileLanguageRequest, PFProfilesSetProfileLanguageRequest, PFProfilesSetProfileLanguageResponse, PFProfilesSetProfileLanguageAsync, PFProfilesSetProfileLanguageGetResultSize, PFProfilesSetProfileLanguageGetResult, to_variant_PFProfilesSetProfileLanguageResponse>(runtime, pending_signal, p_user, std::move(request), "PFProfilesSetProfileLanguage");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFProfilesSetProfileLanguage.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabAccounts::set_profile_policy_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFProfilesSetEntityProfilePolicyRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFProfilesSetEntityProfilePolicyRequest, PFProfilesSetEntityProfilePolicyRequest, PFProfilesSetEntityProfilePolicyResponse, PFProfilesSetProfilePolicyAsync, PFProfilesSetProfilePolicyGetResultSize, PFProfilesSetProfilePolicyGetResult, to_variant_PFProfilesSetEntityProfilePolicyResponse>(runtime, pending_signal, p_user, std::move(request), "PFProfilesSetProfilePolicy");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFProfilesSetProfilePolicy.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabCatalog::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_draft_item_async", "user", "request"), &PlayFabCatalog::create_draft_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("create_upload_urls_async", "user", "request"), &PlayFabCatalog::create_upload_urls_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_entity_item_reviews_async", "user", "request"), &PlayFabCatalog::delete_entity_item_reviews_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_item_async", "user", "request"), &PlayFabCatalog::delete_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_catalog_config_async", "user", "request"), &PlayFabCatalog::get_catalog_config_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_draft_item_async", "user", "request"), &PlayFabCatalog::get_draft_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_draft_items_async", "user", "request"), &PlayFabCatalog::get_draft_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_entity_draft_items_async", "user", "request"), &PlayFabCatalog::get_entity_draft_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_entity_item_review_async", "user", "request"), &PlayFabCatalog::get_entity_item_review_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_item_async", "user", "request"), &PlayFabCatalog::get_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_item_containers_async", "user", "request"), &PlayFabCatalog::get_item_containers_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_item_moderation_state_async", "user", "request"), &PlayFabCatalog::get_item_moderation_state_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_item_publish_status_async", "user", "request"), &PlayFabCatalog::get_item_publish_status_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_item_reviews_async", "user", "request"), &PlayFabCatalog::get_item_reviews_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_item_review_summary_async", "user", "request"), &PlayFabCatalog::get_item_review_summary_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_items_async", "user", "request"), &PlayFabCatalog::get_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("publish_draft_item_async", "user", "request"), &PlayFabCatalog::publish_draft_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("report_item_async", "user", "request"), &PlayFabCatalog::report_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("report_item_review_async", "user", "request"), &PlayFabCatalog::report_item_review_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("review_item_async", "user", "request"), &PlayFabCatalog::review_item_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("search_items_async", "user", "request"), &PlayFabCatalog::search_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_item_moderation_state_async", "user", "request"), &PlayFabCatalog::set_item_moderation_state_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("submit_item_review_vote_async", "user", "request"), &PlayFabCatalog::submit_item_review_vote_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("takedown_item_reviews_async", "user", "request"), &PlayFabCatalog::takedown_item_reviews_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_catalog_config_async", "user", "request"), &PlayFabCatalog::update_catalog_config_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_draft_item_async", "user", "request"), &PlayFabCatalog::update_draft_item_async, DEFVAL(Dictionary()));
}
void PlayFabCatalog::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabCatalog::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabCatalog::create_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogCreateDraftItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogCreateDraftItemRequest, PFCatalogCreateDraftItemRequest, PFCatalogCreateDraftItemResponse, PFCatalogCreateDraftItemAsync, PFCatalogCreateDraftItemGetResultSize, PFCatalogCreateDraftItemGetResult, to_variant_PFCatalogCreateDraftItemResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogCreateDraftItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogCreateDraftItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::create_upload_urls_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogCreateUploadUrlsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogCreateUploadUrlsRequest, PFCatalogCreateUploadUrlsRequest, PFCatalogCreateUploadUrlsResponse, PFCatalogCreateUploadUrlsAsync, PFCatalogCreateUploadUrlsGetResultSize, PFCatalogCreateUploadUrlsGetResult, to_variant_PFCatalogCreateUploadUrlsResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogCreateUploadUrls");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogCreateUploadUrls.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::delete_entity_item_reviews_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogDeleteEntityItemReviewsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogDeleteEntityItemReviewsRequest, PFCatalogDeleteEntityItemReviewsRequest, PFCatalogDeleteEntityItemReviewsAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogDeleteEntityItemReviews");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogDeleteEntityItemReviews.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::delete_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogDeleteItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogDeleteItemRequest, PFCatalogDeleteItemRequest, PFCatalogDeleteItemAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogDeleteItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogDeleteItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_catalog_config_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetCatalogConfigRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetCatalogConfigRequest, PFCatalogGetCatalogConfigRequest, PFCatalogGetCatalogConfigResponse, PFCatalogGetCatalogConfigAsync, PFCatalogGetCatalogConfigGetResultSize, PFCatalogGetCatalogConfigGetResult, to_variant_PFCatalogGetCatalogConfigResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetCatalogConfig");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetCatalogConfig.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetDraftItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetDraftItemRequest, PFCatalogGetDraftItemRequest, PFCatalogGetDraftItemResponse, PFCatalogGetDraftItemAsync, PFCatalogGetDraftItemGetResultSize, PFCatalogGetDraftItemGetResult, to_variant_PFCatalogGetDraftItemResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetDraftItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetDraftItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_draft_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetDraftItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetDraftItemsRequest, PFCatalogGetDraftItemsRequest, PFCatalogGetDraftItemsResponse, PFCatalogGetDraftItemsAsync, PFCatalogGetDraftItemsGetResultSize, PFCatalogGetDraftItemsGetResult, to_variant_PFCatalogGetDraftItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetDraftItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetDraftItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_entity_draft_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetEntityDraftItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetEntityDraftItemsRequest, PFCatalogGetEntityDraftItemsRequest, PFCatalogGetEntityDraftItemsResponse, PFCatalogGetEntityDraftItemsAsync, PFCatalogGetEntityDraftItemsGetResultSize, PFCatalogGetEntityDraftItemsGetResult, to_variant_PFCatalogGetEntityDraftItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetEntityDraftItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetEntityDraftItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_entity_item_review_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetEntityItemReviewRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetEntityItemReviewRequest, PFCatalogGetEntityItemReviewRequest, PFCatalogGetEntityItemReviewResponse, PFCatalogGetEntityItemReviewAsync, PFCatalogGetEntityItemReviewGetResultSize, PFCatalogGetEntityItemReviewGetResult, to_variant_PFCatalogGetEntityItemReviewResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetEntityItemReview");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetEntityItemReview.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemRequest, PFCatalogGetItemRequest, PFCatalogGetItemResponse, PFCatalogGetItemAsync, PFCatalogGetItemGetResultSize, PFCatalogGetItemGetResult, to_variant_PFCatalogGetItemResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_item_containers_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemContainersRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemContainersRequest, PFCatalogGetItemContainersRequest, PFCatalogGetItemContainersResponse, PFCatalogGetItemContainersAsync, PFCatalogGetItemContainersGetResultSize, PFCatalogGetItemContainersGetResult, to_variant_PFCatalogGetItemContainersResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItemContainers");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItemContainers.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_item_moderation_state_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemModerationStateRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemModerationStateRequest, PFCatalogGetItemModerationStateRequest, PFCatalogGetItemModerationStateResponse, PFCatalogGetItemModerationStateAsync, PFCatalogGetItemModerationStateGetResultSize, PFCatalogGetItemModerationStateGetResult, to_variant_PFCatalogGetItemModerationStateResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItemModerationState");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItemModerationState.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_item_publish_status_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemPublishStatusRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemPublishStatusRequest, PFCatalogGetItemPublishStatusRequest, PFCatalogGetItemPublishStatusResponse, PFCatalogGetItemPublishStatusAsync, PFCatalogGetItemPublishStatusGetResultSize, PFCatalogGetItemPublishStatusGetResult, to_variant_PFCatalogGetItemPublishStatusResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItemPublishStatus");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItemPublishStatus.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_item_reviews_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemReviewsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemReviewsRequest, PFCatalogGetItemReviewsRequest, PFCatalogGetItemReviewsResponse, PFCatalogGetItemReviewsAsync, PFCatalogGetItemReviewsGetResultSize, PFCatalogGetItemReviewsGetResult, to_variant_PFCatalogGetItemReviewsResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItemReviews");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItemReviews.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_item_review_summary_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemReviewSummaryRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemReviewSummaryRequest, PFCatalogGetItemReviewSummaryRequest, PFCatalogGetItemReviewSummaryResponse, PFCatalogGetItemReviewSummaryAsync, PFCatalogGetItemReviewSummaryGetResultSize, PFCatalogGetItemReviewSummaryGetResult, to_variant_PFCatalogGetItemReviewSummaryResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItemReviewSummary");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItemReviewSummary.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::get_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogGetItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogGetItemsRequest, PFCatalogGetItemsRequest, PFCatalogGetItemsResponse, PFCatalogGetItemsAsync, PFCatalogGetItemsGetResultSize, PFCatalogGetItemsGetResult, to_variant_PFCatalogGetItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogGetItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogGetItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::publish_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogPublishDraftItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogPublishDraftItemRequest, PFCatalogPublishDraftItemRequest, PFCatalogPublishDraftItemAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogPublishDraftItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogPublishDraftItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::report_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogReportItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogReportItemRequest, PFCatalogReportItemRequest, PFCatalogReportItemAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogReportItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogReportItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::report_item_review_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogReportItemReviewRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogReportItemReviewRequest, PFCatalogReportItemReviewRequest, PFCatalogReportItemReviewAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogReportItemReview");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogReportItemReview.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::review_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogReviewItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogReviewItemRequest, PFCatalogReviewItemRequest, PFCatalogReviewItemAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogReviewItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogReviewItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::search_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogSearchItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogSearchItemsRequest, PFCatalogSearchItemsRequest, PFCatalogSearchItemsResponse, PFCatalogSearchItemsAsync, PFCatalogSearchItemsGetResultSize, PFCatalogSearchItemsGetResult, to_variant_PFCatalogSearchItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogSearchItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogSearchItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::set_item_moderation_state_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogSetItemModerationStateRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogSetItemModerationStateRequest, PFCatalogSetItemModerationStateRequest, PFCatalogSetItemModerationStateAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogSetItemModerationState");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogSetItemModerationState.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::submit_item_review_vote_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogSubmitItemReviewVoteRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogSubmitItemReviewVoteRequest, PFCatalogSubmitItemReviewVoteRequest, PFCatalogSubmitItemReviewVoteAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogSubmitItemReviewVote");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogSubmitItemReviewVote.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::takedown_item_reviews_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogTakedownItemReviewsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogTakedownItemReviewsRequest, PFCatalogTakedownItemReviewsRequest, PFCatalogTakedownItemReviewsAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogTakedownItemReviews");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogTakedownItemReviews.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::update_catalog_config_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogUpdateCatalogConfigRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFCatalogUpdateCatalogConfigRequest, PFCatalogUpdateCatalogConfigRequest, PFCatalogUpdateCatalogConfigAsync>(runtime, pending_signal, p_user, std::move(request), "PFCatalogUpdateCatalogConfig");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogUpdateCatalogConfig.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCatalog::update_draft_item_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCatalogUpdateDraftItemRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCatalogUpdateDraftItemRequest, PFCatalogUpdateDraftItemRequest, PFCatalogUpdateDraftItemResponse, PFCatalogUpdateDraftItemAsync, PFCatalogUpdateDraftItemGetResultSize, PFCatalogUpdateDraftItemGetResult, to_variant_PFCatalogUpdateDraftItemResponse>(runtime, pending_signal, p_user, std::move(request), "PFCatalogUpdateDraftItem");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCatalogUpdateDraftItem.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabCloudScript::_bind_methods() {
    ClassDB::bind_method(D_METHOD("execute_cloud_script_async", "user", "request"), &PlayFabCloudScript::execute_cloud_script_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("execute_entity_cloud_script_async", "user", "request"), &PlayFabCloudScript::execute_entity_cloud_script_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("execute_function_async", "user", "request"), &PlayFabCloudScript::execute_function_async, DEFVAL(Dictionary()));
}
void PlayFabCloudScript::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabCloudScript::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabCloudScript::execute_cloud_script_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCloudScriptExecuteCloudScriptRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCloudScriptExecuteCloudScriptRequest, PFCloudScriptExecuteCloudScriptRequest, PFCloudScriptExecuteCloudScriptResult, PFCloudScriptClientExecuteCloudScriptAsync, PFCloudScriptClientExecuteCloudScriptGetResultSize, PFCloudScriptClientExecuteCloudScriptGetResult, to_variant_PFCloudScriptExecuteCloudScriptResult>(runtime, pending_signal, p_user, std::move(request), "PFCloudScriptClientExecuteCloudScript");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCloudScriptClientExecuteCloudScript.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCloudScript::execute_entity_cloud_script_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCloudScriptExecuteEntityCloudScriptRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCloudScriptExecuteEntityCloudScriptRequest, PFCloudScriptExecuteEntityCloudScriptRequest, PFCloudScriptExecuteCloudScriptResult, PFCloudScriptExecuteEntityCloudScriptAsync, PFCloudScriptExecuteEntityCloudScriptGetResultSize, PFCloudScriptExecuteEntityCloudScriptGetResult, to_variant_PFCloudScriptExecuteCloudScriptResult>(runtime, pending_signal, p_user, std::move(request), "PFCloudScriptExecuteEntityCloudScript");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCloudScriptExecuteEntityCloudScript.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabCloudScript::execute_function_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFCloudScriptExecuteFunctionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFCloudScriptExecuteFunctionRequest, PFCloudScriptExecuteFunctionRequest, PFCloudScriptExecuteFunctionResult, PFCloudScriptExecuteFunctionAsync, PFCloudScriptExecuteFunctionGetResultSize, PFCloudScriptExecuteFunctionGetResult, to_variant_PFCloudScriptExecuteFunctionResult>(runtime, pending_signal, p_user, std::move(request), "PFCloudScriptExecuteFunction");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFCloudScriptExecuteFunction.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabEntityData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("abort_file_uploads_async", "user", "request"), &PlayFabEntityData::abort_file_uploads_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_files_async", "user", "request"), &PlayFabEntityData::delete_files_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("finalize_file_uploads_async", "user", "request"), &PlayFabEntityData::finalize_file_uploads_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_files_async", "user", "request"), &PlayFabEntityData::get_files_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_objects_async", "user", "request"), &PlayFabEntityData::get_objects_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("initiate_file_uploads_async", "user", "request"), &PlayFabEntityData::initiate_file_uploads_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_objects_async", "user", "request"), &PlayFabEntityData::set_objects_async, DEFVAL(Dictionary()));
}
void PlayFabEntityData::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabEntityData::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabEntityData::abort_file_uploads_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataAbortFileUploadsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataAbortFileUploadsRequest, PFDataAbortFileUploadsRequest, PFDataAbortFileUploadsResponse, PFDataAbortFileUploadsAsync, PFDataAbortFileUploadsGetResultSize, PFDataAbortFileUploadsGetResult, to_variant_PFDataAbortFileUploadsResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataAbortFileUploads");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataAbortFileUploads.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabEntityData::delete_files_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataDeleteFilesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataDeleteFilesRequest, PFDataDeleteFilesRequest, PFDataDeleteFilesResponse, PFDataDeleteFilesAsync, PFDataDeleteFilesGetResultSize, PFDataDeleteFilesGetResult, to_variant_PFDataDeleteFilesResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataDeleteFiles");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataDeleteFiles.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabEntityData::finalize_file_uploads_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataFinalizeFileUploadsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataFinalizeFileUploadsRequest, PFDataFinalizeFileUploadsRequest, PFDataFinalizeFileUploadsResponse, PFDataFinalizeFileUploadsAsync, PFDataFinalizeFileUploadsGetResultSize, PFDataFinalizeFileUploadsGetResult, to_variant_PFDataFinalizeFileUploadsResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataFinalizeFileUploads");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataFinalizeFileUploads.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabEntityData::get_files_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataGetFilesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataGetFilesRequest, PFDataGetFilesRequest, PFDataGetFilesResponse, PFDataGetFilesAsync, PFDataGetFilesGetResultSize, PFDataGetFilesGetResult, to_variant_PFDataGetFilesResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataGetFiles");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataGetFiles.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabEntityData::get_objects_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataGetObjectsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataGetObjectsRequest, PFDataGetObjectsRequest, PFDataGetObjectsResponse, PFDataGetObjectsAsync, PFDataGetObjectsGetResultSize, PFDataGetObjectsGetResult, to_variant_PFDataGetObjectsResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataGetObjects");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataGetObjects.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabEntityData::initiate_file_uploads_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataInitiateFileUploadsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataInitiateFileUploadsRequest, PFDataInitiateFileUploadsRequest, PFDataInitiateFileUploadsResponse, PFDataInitiateFileUploadsAsync, PFDataInitiateFileUploadsGetResultSize, PFDataInitiateFileUploadsGetResult, to_variant_PFDataInitiateFileUploadsResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataInitiateFileUploads");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataInitiateFileUploads.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabEntityData::set_objects_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFDataSetObjectsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFDataSetObjectsRequest, PFDataSetObjectsRequest, PFDataSetObjectsResponse, PFDataSetObjectsAsync, PFDataSetObjectsGetResultSize, PFDataSetObjectsGetResult, to_variant_PFDataSetObjectsResponse>(runtime, pending_signal, p_user, std::move(request), "PFDataSetObjects");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFDataSetObjects.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabEvents::_bind_methods() {
}
void PlayFabEvents::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabEvents::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

void PlayFabExperimentation::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_treatment_assignment_async", "user", "request"), &PlayFabExperimentation::get_treatment_assignment_async, DEFVAL(Dictionary()));
}
void PlayFabExperimentation::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabExperimentation::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabExperimentation::get_treatment_assignment_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFExperimentationGetTreatmentAssignmentRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFExperimentationGetTreatmentAssignmentRequest, PFExperimentationGetTreatmentAssignmentRequest, PFExperimentationGetTreatmentAssignmentResult, PFExperimentationGetTreatmentAssignmentAsync, PFExperimentationGetTreatmentAssignmentGetResultSize, PFExperimentationGetTreatmentAssignmentGetResult, to_variant_PFExperimentationGetTreatmentAssignmentResult>(runtime, pending_signal, p_user, std::move(request), "PFExperimentationGetTreatmentAssignment");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFExperimentationGetTreatmentAssignment.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabFriends::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_friend_async", "user", "request"), &PlayFabFriends::add_friend_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_friends_list_async", "user", "request"), &PlayFabFriends::get_friends_list_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("remove_friend_async", "user", "request"), &PlayFabFriends::remove_friend_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("set_friend_tags_async", "user", "request"), &PlayFabFriends::set_friend_tags_async, DEFVAL(Dictionary()));
}
void PlayFabFriends::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabFriends::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabFriends::add_friend_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFFriendsClientAddFriendRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFFriendsClientAddFriendRequest, PFFriendsClientAddFriendRequest, PFFriendsAddFriendResult, PFFriendsClientAddFriendAsync, PFFriendsClientAddFriendGetResult, to_variant_PFFriendsAddFriendResult>(runtime, pending_signal, p_user, std::move(request), "PFFriendsClientAddFriend");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFFriendsClientAddFriend.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabFriends::get_friends_list_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFFriendsClientGetFriendsListRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFFriendsClientGetFriendsListRequest, PFFriendsClientGetFriendsListRequest, PFFriendsGetFriendsListResult, PFFriendsClientGetFriendsListAsync, PFFriendsClientGetFriendsListGetResultSize, PFFriendsClientGetFriendsListGetResult, to_variant_PFFriendsGetFriendsListResult>(runtime, pending_signal, p_user, std::move(request), "PFFriendsClientGetFriendsList");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFFriendsClientGetFriendsList.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabFriends::remove_friend_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFFriendsClientRemoveFriendRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFFriendsClientRemoveFriendRequest, PFFriendsClientRemoveFriendRequest, PFFriendsClientRemoveFriendAsync>(runtime, pending_signal, p_user, std::move(request), "PFFriendsClientRemoveFriend");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFFriendsClientRemoveFriend.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabFriends::set_friend_tags_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFFriendsClientSetFriendTagsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFFriendsClientSetFriendTagsRequest, PFFriendsClientSetFriendTagsRequest, PFFriendsClientSetFriendTagsAsync>(runtime, pending_signal, p_user, std::move(request), "PFFriendsClientSetFriendTags");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFFriendsClientSetFriendTags.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabGroups::_bind_methods() {
    ClassDB::bind_method(D_METHOD("accept_group_application_async", "user", "request"), &PlayFabGroups::accept_group_application_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("accept_group_invitation_async", "user", "request"), &PlayFabGroups::accept_group_invitation_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("add_members_async", "user", "request"), &PlayFabGroups::add_members_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("apply_to_group_async", "user", "request"), &PlayFabGroups::apply_to_group_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("block_entity_async", "user", "request"), &PlayFabGroups::block_entity_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("change_member_role_async", "user", "request"), &PlayFabGroups::change_member_role_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("create_group_async", "user", "request"), &PlayFabGroups::create_group_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("create_role_async", "user", "request"), &PlayFabGroups::create_role_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_group_async", "user", "request"), &PlayFabGroups::delete_group_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_role_async", "user", "request"), &PlayFabGroups::delete_role_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_group_async", "user", "request"), &PlayFabGroups::get_group_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("invite_to_group_async", "user", "request"), &PlayFabGroups::invite_to_group_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("is_member_async", "user", "request"), &PlayFabGroups::is_member_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_group_applications_async", "user", "request"), &PlayFabGroups::list_group_applications_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_group_blocks_async", "user", "request"), &PlayFabGroups::list_group_blocks_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_group_invitations_async", "user", "request"), &PlayFabGroups::list_group_invitations_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_group_members_async", "user", "request"), &PlayFabGroups::list_group_members_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_membership_async", "user", "request"), &PlayFabGroups::list_membership_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_membership_opportunities_async", "user", "request"), &PlayFabGroups::list_membership_opportunities_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("remove_group_application_async", "user", "request"), &PlayFabGroups::remove_group_application_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("remove_group_invitation_async", "user", "request"), &PlayFabGroups::remove_group_invitation_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("remove_members_async", "user", "request"), &PlayFabGroups::remove_members_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("unblock_entity_async", "user", "request"), &PlayFabGroups::unblock_entity_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_group_async", "user", "request"), &PlayFabGroups::update_group_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_role_async", "user", "request"), &PlayFabGroups::update_role_async, DEFVAL(Dictionary()));
}
void PlayFabGroups::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabGroups::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabGroups::accept_group_application_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsAcceptGroupApplicationRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsAcceptGroupApplicationRequest, PFGroupsAcceptGroupApplicationRequest, PFGroupsAcceptGroupApplicationAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsAcceptGroupApplication");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsAcceptGroupApplication.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::accept_group_invitation_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsAcceptGroupInvitationRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsAcceptGroupInvitationRequest, PFGroupsAcceptGroupInvitationRequest, PFGroupsAcceptGroupInvitationAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsAcceptGroupInvitation");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsAcceptGroupInvitation.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::add_members_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsAddMembersRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsAddMembersRequest, PFGroupsAddMembersRequest, PFGroupsAddMembersAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsAddMembers");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsAddMembers.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::apply_to_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsApplyToGroupRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsApplyToGroupRequest, PFGroupsApplyToGroupRequest, PFGroupsApplyToGroupResponse, PFGroupsApplyToGroupAsync, PFGroupsApplyToGroupGetResultSize, PFGroupsApplyToGroupGetResult, to_variant_PFGroupsApplyToGroupResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsApplyToGroup");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsApplyToGroup.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::block_entity_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsBlockEntityRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsBlockEntityRequest, PFGroupsBlockEntityRequest, PFGroupsBlockEntityAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsBlockEntity");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsBlockEntity.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::change_member_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsChangeMemberRoleRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsChangeMemberRoleRequest, PFGroupsChangeMemberRoleRequest, PFGroupsChangeMemberRoleAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsChangeMemberRole");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsChangeMemberRole.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::create_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsCreateGroupRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsCreateGroupRequest, PFGroupsCreateGroupRequest, PFGroupsCreateGroupResponse, PFGroupsCreateGroupAsync, PFGroupsCreateGroupGetResultSize, PFGroupsCreateGroupGetResult, to_variant_PFGroupsCreateGroupResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsCreateGroup");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsCreateGroup.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::create_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsCreateGroupRoleRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsCreateGroupRoleRequest, PFGroupsCreateGroupRoleRequest, PFGroupsCreateGroupRoleResponse, PFGroupsCreateRoleAsync, PFGroupsCreateRoleGetResultSize, PFGroupsCreateRoleGetResult, to_variant_PFGroupsCreateGroupRoleResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsCreateRole");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsCreateRole.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::delete_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsDeleteGroupRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsDeleteGroupRequest, PFGroupsDeleteGroupRequest, PFGroupsDeleteGroupAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsDeleteGroup");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsDeleteGroup.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::delete_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsDeleteRoleRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsDeleteRoleRequest, PFGroupsDeleteRoleRequest, PFGroupsDeleteRoleAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsDeleteRole");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsDeleteRole.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::get_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsGetGroupRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsGetGroupRequest, PFGroupsGetGroupRequest, PFGroupsGetGroupResponse, PFGroupsGetGroupAsync, PFGroupsGetGroupGetResultSize, PFGroupsGetGroupGetResult, to_variant_PFGroupsGetGroupResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsGetGroup");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsGetGroup.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::invite_to_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsInviteToGroupRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsInviteToGroupRequest, PFGroupsInviteToGroupRequest, PFGroupsInviteToGroupResponse, PFGroupsInviteToGroupAsync, PFGroupsInviteToGroupGetResultSize, PFGroupsInviteToGroupGetResult, to_variant_PFGroupsInviteToGroupResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsInviteToGroup");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsInviteToGroup.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::is_member_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsIsMemberRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFGroupsIsMemberRequest, PFGroupsIsMemberRequest, PFGroupsIsMemberResponse, PFGroupsIsMemberAsync, PFGroupsIsMemberGetResult, to_variant_PFGroupsIsMemberResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsIsMember");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsIsMember.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::list_group_applications_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsListGroupApplicationsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsListGroupApplicationsRequest, PFGroupsListGroupApplicationsRequest, PFGroupsListGroupApplicationsResponse, PFGroupsListGroupApplicationsAsync, PFGroupsListGroupApplicationsGetResultSize, PFGroupsListGroupApplicationsGetResult, to_variant_PFGroupsListGroupApplicationsResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsListGroupApplications");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsListGroupApplications.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::list_group_blocks_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsListGroupBlocksRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsListGroupBlocksRequest, PFGroupsListGroupBlocksRequest, PFGroupsListGroupBlocksResponse, PFGroupsListGroupBlocksAsync, PFGroupsListGroupBlocksGetResultSize, PFGroupsListGroupBlocksGetResult, to_variant_PFGroupsListGroupBlocksResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsListGroupBlocks");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsListGroupBlocks.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::list_group_invitations_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsListGroupInvitationsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsListGroupInvitationsRequest, PFGroupsListGroupInvitationsRequest, PFGroupsListGroupInvitationsResponse, PFGroupsListGroupInvitationsAsync, PFGroupsListGroupInvitationsGetResultSize, PFGroupsListGroupInvitationsGetResult, to_variant_PFGroupsListGroupInvitationsResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsListGroupInvitations");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsListGroupInvitations.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::list_group_members_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsListGroupMembersRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsListGroupMembersRequest, PFGroupsListGroupMembersRequest, PFGroupsListGroupMembersResponse, PFGroupsListGroupMembersAsync, PFGroupsListGroupMembersGetResultSize, PFGroupsListGroupMembersGetResult, to_variant_PFGroupsListGroupMembersResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsListGroupMembers");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsListGroupMembers.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::list_membership_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsListMembershipRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsListMembershipRequest, PFGroupsListMembershipRequest, PFGroupsListMembershipResponse, PFGroupsListMembershipAsync, PFGroupsListMembershipGetResultSize, PFGroupsListMembershipGetResult, to_variant_PFGroupsListMembershipResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsListMembership");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsListMembership.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::list_membership_opportunities_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsListMembershipOpportunitiesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsListMembershipOpportunitiesRequest, PFGroupsListMembershipOpportunitiesRequest, PFGroupsListMembershipOpportunitiesResponse, PFGroupsListMembershipOpportunitiesAsync, PFGroupsListMembershipOpportunitiesGetResultSize, PFGroupsListMembershipOpportunitiesGetResult, to_variant_PFGroupsListMembershipOpportunitiesResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsListMembershipOpportunities");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsListMembershipOpportunities.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::remove_group_application_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsRemoveGroupApplicationRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsRemoveGroupApplicationRequest, PFGroupsRemoveGroupApplicationRequest, PFGroupsRemoveGroupApplicationAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsRemoveGroupApplication");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsRemoveGroupApplication.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::remove_group_invitation_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsRemoveGroupInvitationRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsRemoveGroupInvitationRequest, PFGroupsRemoveGroupInvitationRequest, PFGroupsRemoveGroupInvitationAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsRemoveGroupInvitation");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsRemoveGroupInvitation.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::remove_members_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsRemoveMembersRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsRemoveMembersRequest, PFGroupsRemoveMembersRequest, PFGroupsRemoveMembersAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsRemoveMembers");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsRemoveMembers.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::unblock_entity_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsUnblockEntityRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFGroupsUnblockEntityRequest, PFGroupsUnblockEntityRequest, PFGroupsUnblockEntityAsync>(runtime, pending_signal, p_user, std::move(request), "PFGroupsUnblockEntity");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsUnblockEntity.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::update_group_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsUpdateGroupRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsUpdateGroupRequest, PFGroupsUpdateGroupRequest, PFGroupsUpdateGroupResponse, PFGroupsUpdateGroupAsync, PFGroupsUpdateGroupGetResultSize, PFGroupsUpdateGroupGetResult, to_variant_PFGroupsUpdateGroupResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsUpdateGroup");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsUpdateGroup.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabGroups::update_role_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFGroupsUpdateGroupRoleRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFGroupsUpdateGroupRoleRequest, PFGroupsUpdateGroupRoleRequest, PFGroupsUpdateGroupRoleResponse, PFGroupsUpdateRoleAsync, PFGroupsUpdateRoleGetResultSize, PFGroupsUpdateRoleGetResult, to_variant_PFGroupsUpdateGroupRoleResponse>(runtime, pending_signal, p_user, std::move(request), "PFGroupsUpdateRole");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFGroupsUpdateRole.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabInventory::_bind_methods() {
    ClassDB::bind_method(D_METHOD("add_inventory_items_async", "user", "request"), &PlayFabInventory::add_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_inventory_collection_async", "user", "request"), &PlayFabInventory::delete_inventory_collection_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_inventory_items_async", "user", "request"), &PlayFabInventory::delete_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("execute_inventory_operations_async", "user", "request"), &PlayFabInventory::execute_inventory_operations_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("execute_transfer_operations_async", "user", "request"), &PlayFabInventory::execute_transfer_operations_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_inventory_collection_ids_async", "user", "request"), &PlayFabInventory::get_inventory_collection_ids_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_inventory_items_async", "user", "request"), &PlayFabInventory::get_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_inventory_operation_status_async", "user", "request"), &PlayFabInventory::get_inventory_operation_status_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_transaction_history_async", "user", "request"), &PlayFabInventory::get_transaction_history_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("purchase_inventory_items_async", "user", "request"), &PlayFabInventory::purchase_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("redeem_google_play_inventory_items_async", "user", "request"), &PlayFabInventory::redeem_google_play_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("redeem_microsoft_store_inventory_items_async", "user", "request"), &PlayFabInventory::redeem_microsoft_store_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("redeem_play_station_store_inventory_items_async", "user", "request"), &PlayFabInventory::redeem_play_station_store_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("redeem_steam_inventory_items_async", "user", "request"), &PlayFabInventory::redeem_steam_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("subtract_inventory_items_async", "user", "request"), &PlayFabInventory::subtract_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("transfer_inventory_items_async", "user", "request"), &PlayFabInventory::transfer_inventory_items_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_inventory_items_async", "user", "request"), &PlayFabInventory::update_inventory_items_async, DEFVAL(Dictionary()));
}
void PlayFabInventory::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabInventory::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabInventory::add_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryAddInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryAddInventoryItemsRequest, PFInventoryAddInventoryItemsRequest, PFInventoryAddInventoryItemsResponse, PFInventoryAddInventoryItemsAsync, PFInventoryAddInventoryItemsGetResultSize, PFInventoryAddInventoryItemsGetResult, to_variant_PFInventoryAddInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryAddInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryAddInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::delete_inventory_collection_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryDeleteInventoryCollectionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFInventoryDeleteInventoryCollectionRequest, PFInventoryDeleteInventoryCollectionRequest, PFInventoryDeleteInventoryCollectionAsync>(runtime, pending_signal, p_user, std::move(request), "PFInventoryDeleteInventoryCollection");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryDeleteInventoryCollection.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::delete_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryDeleteInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryDeleteInventoryItemsRequest, PFInventoryDeleteInventoryItemsRequest, PFInventoryDeleteInventoryItemsResponse, PFInventoryDeleteInventoryItemsAsync, PFInventoryDeleteInventoryItemsGetResultSize, PFInventoryDeleteInventoryItemsGetResult, to_variant_PFInventoryDeleteInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryDeleteInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryDeleteInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::execute_inventory_operations_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryExecuteInventoryOperationsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryExecuteInventoryOperationsRequest, PFInventoryExecuteInventoryOperationsRequest, PFInventoryExecuteInventoryOperationsResponse, PFInventoryExecuteInventoryOperationsAsync, PFInventoryExecuteInventoryOperationsGetResultSize, PFInventoryExecuteInventoryOperationsGetResult, to_variant_PFInventoryExecuteInventoryOperationsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryExecuteInventoryOperations");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryExecuteInventoryOperations.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::execute_transfer_operations_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryExecuteTransferOperationsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryExecuteTransferOperationsRequest, PFInventoryExecuteTransferOperationsRequest, PFInventoryExecuteTransferOperationsResponse, PFInventoryExecuteTransferOperationsAsync, PFInventoryExecuteTransferOperationsGetResultSize, PFInventoryExecuteTransferOperationsGetResult, to_variant_PFInventoryExecuteTransferOperationsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryExecuteTransferOperations");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryExecuteTransferOperations.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::get_inventory_collection_ids_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryGetInventoryCollectionIdsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryGetInventoryCollectionIdsRequest, PFInventoryGetInventoryCollectionIdsRequest, PFInventoryGetInventoryCollectionIdsResponse, PFInventoryGetInventoryCollectionIdsAsync, PFInventoryGetInventoryCollectionIdsGetResultSize, PFInventoryGetInventoryCollectionIdsGetResult, to_variant_PFInventoryGetInventoryCollectionIdsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryGetInventoryCollectionIds");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryGetInventoryCollectionIds.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::get_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryGetInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryGetInventoryItemsRequest, PFInventoryGetInventoryItemsRequest, PFInventoryGetInventoryItemsResponse, PFInventoryGetInventoryItemsAsync, PFInventoryGetInventoryItemsGetResultSize, PFInventoryGetInventoryItemsGetResult, to_variant_PFInventoryGetInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryGetInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryGetInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::get_inventory_operation_status_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryGetInventoryOperationStatusRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryGetInventoryOperationStatusRequest, PFInventoryGetInventoryOperationStatusRequest, PFInventoryGetInventoryOperationStatusResponse, PFInventoryGetInventoryOperationStatusAsync, PFInventoryGetInventoryOperationStatusGetResultSize, PFInventoryGetInventoryOperationStatusGetResult, to_variant_PFInventoryGetInventoryOperationStatusResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryGetInventoryOperationStatus");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryGetInventoryOperationStatus.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::get_transaction_history_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryGetTransactionHistoryRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryGetTransactionHistoryRequest, PFInventoryGetTransactionHistoryRequest, PFInventoryGetTransactionHistoryResponse, PFInventoryGetTransactionHistoryAsync, PFInventoryGetTransactionHistoryGetResultSize, PFInventoryGetTransactionHistoryGetResult, to_variant_PFInventoryGetTransactionHistoryResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryGetTransactionHistory");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryGetTransactionHistory.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::purchase_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryPurchaseInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryPurchaseInventoryItemsRequest, PFInventoryPurchaseInventoryItemsRequest, PFInventoryPurchaseInventoryItemsResponse, PFInventoryPurchaseInventoryItemsAsync, PFInventoryPurchaseInventoryItemsGetResultSize, PFInventoryPurchaseInventoryItemsGetResult, to_variant_PFInventoryPurchaseInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryPurchaseInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryPurchaseInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::redeem_google_play_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryRedeemGooglePlayInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryRedeemGooglePlayInventoryItemsRequest, PFInventoryRedeemGooglePlayInventoryItemsRequest, PFInventoryRedeemGooglePlayInventoryItemsResponse, PFInventoryRedeemGooglePlayInventoryItemsAsync, PFInventoryRedeemGooglePlayInventoryItemsGetResultSize, PFInventoryRedeemGooglePlayInventoryItemsGetResult, to_variant_PFInventoryRedeemGooglePlayInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryRedeemGooglePlayInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryRedeemGooglePlayInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::redeem_microsoft_store_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryRedeemMicrosoftStoreInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryRedeemMicrosoftStoreInventoryItemsRequest, PFInventoryRedeemMicrosoftStoreInventoryItemsRequest, PFInventoryRedeemMicrosoftStoreInventoryItemsResponse, PFInventoryRedeemMicrosoftStoreInventoryItemsAsync, PFInventoryRedeemMicrosoftStoreInventoryItemsGetResultSize, PFInventoryRedeemMicrosoftStoreInventoryItemsGetResult, to_variant_PFInventoryRedeemMicrosoftStoreInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryRedeemMicrosoftStoreInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryRedeemMicrosoftStoreInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::redeem_play_station_store_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryRedeemPlayStationStoreInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryRedeemPlayStationStoreInventoryItemsRequest, PFInventoryRedeemPlayStationStoreInventoryItemsRequest, PFInventoryRedeemPlayStationStoreInventoryItemsResponse, PFInventoryRedeemPlayStationStoreInventoryItemsAsync, PFInventoryRedeemPlayStationStoreInventoryItemsGetResultSize, PFInventoryRedeemPlayStationStoreInventoryItemsGetResult, to_variant_PFInventoryRedeemPlayStationStoreInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryRedeemPlayStationStoreInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryRedeemPlayStationStoreInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::redeem_steam_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryRedeemSteamInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryRedeemSteamInventoryItemsRequest, PFInventoryRedeemSteamInventoryItemsRequest, PFInventoryRedeemSteamInventoryItemsResponse, PFInventoryRedeemSteamInventoryItemsAsync, PFInventoryRedeemSteamInventoryItemsGetResultSize, PFInventoryRedeemSteamInventoryItemsGetResult, to_variant_PFInventoryRedeemSteamInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryRedeemSteamInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryRedeemSteamInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::subtract_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventorySubtractInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventorySubtractInventoryItemsRequest, PFInventorySubtractInventoryItemsRequest, PFInventorySubtractInventoryItemsResponse, PFInventorySubtractInventoryItemsAsync, PFInventorySubtractInventoryItemsGetResultSize, PFInventorySubtractInventoryItemsGetResult, to_variant_PFInventorySubtractInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventorySubtractInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventorySubtractInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::transfer_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryTransferInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryTransferInventoryItemsRequest, PFInventoryTransferInventoryItemsRequest, PFInventoryTransferInventoryItemsResponse, PFInventoryTransferInventoryItemsAsync, PFInventoryTransferInventoryItemsGetResultSize, PFInventoryTransferInventoryItemsGetResult, to_variant_PFInventoryTransferInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryTransferInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryTransferInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabInventory::update_inventory_items_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFInventoryUpdateInventoryItemsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFInventoryUpdateInventoryItemsRequest, PFInventoryUpdateInventoryItemsRequest, PFInventoryUpdateInventoryItemsResponse, PFInventoryUpdateInventoryItemsAsync, PFInventoryUpdateInventoryItemsGetResultSize, PFInventoryUpdateInventoryItemsGetResult, to_variant_PFInventoryUpdateInventoryItemsResponse>(runtime, pending_signal, p_user, std::move(request), "PFInventoryUpdateInventoryItems");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFInventoryUpdateInventoryItems.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabLocalization::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_language_list_async", "user", "request"), &PlayFabLocalization::get_language_list_async, DEFVAL(Dictionary()));
}
void PlayFabLocalization::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabLocalization::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabLocalization::get_language_list_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFLocalizationGetLanguageListRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFLocalizationGetLanguageListRequest, PFLocalizationGetLanguageListRequest, PFLocalizationGetLanguageListResponse, PFLocalizationGetLanguageListAsync, PFLocalizationGetLanguageListGetResultSize, PFLocalizationGetLanguageListGetResult, to_variant_PFLocalizationGetLanguageListResponse>(runtime, pending_signal, p_user, std::move(request), "PFLocalizationGetLanguageList");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFLocalizationGetLanguageList.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabPlayerData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("delete_player_custom_properties_async", "user", "request"), &PlayFabPlayerData::delete_player_custom_properties_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_player_custom_property_async", "user", "request"), &PlayFabPlayerData::get_player_custom_property_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_user_data_async", "user", "request"), &PlayFabPlayerData::get_user_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_user_publisher_data_async", "user", "request"), &PlayFabPlayerData::get_user_publisher_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_user_publisher_read_only_data_async", "user", "request"), &PlayFabPlayerData::get_user_publisher_read_only_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_user_read_only_data_async", "user", "request"), &PlayFabPlayerData::get_user_read_only_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_player_custom_properties_async", "user"), &PlayFabPlayerData::list_player_custom_properties_async);
    ClassDB::bind_method(D_METHOD("update_player_custom_properties_async", "user", "request"), &PlayFabPlayerData::update_player_custom_properties_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_user_data_async", "user", "request"), &PlayFabPlayerData::update_user_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_user_publisher_data_async", "user", "request"), &PlayFabPlayerData::update_user_publisher_data_async, DEFVAL(Dictionary()));
}
void PlayFabPlayerData::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabPlayerData::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabPlayerData::delete_player_custom_properties_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest, PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest, PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult, PFPlayerDataManagementClientDeletePlayerCustomPropertiesAsync, PFPlayerDataManagementClientDeletePlayerCustomPropertiesGetResultSize, PFPlayerDataManagementClientDeletePlayerCustomPropertiesGetResult, to_variant_PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientDeletePlayerCustomProperties");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientDeletePlayerCustomProperties.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::get_player_custom_property_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementClientGetPlayerCustomPropertyRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFPlayerDataManagementClientGetPlayerCustomPropertyRequest, PFPlayerDataManagementClientGetPlayerCustomPropertyRequest, PFPlayerDataManagementClientGetPlayerCustomPropertyResult, PFPlayerDataManagementClientGetPlayerCustomPropertyAsync, PFPlayerDataManagementClientGetPlayerCustomPropertyGetResultSize, PFPlayerDataManagementClientGetPlayerCustomPropertyGetResult, to_variant_PFPlayerDataManagementClientGetPlayerCustomPropertyResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientGetPlayerCustomProperty");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientGetPlayerCustomProperty.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::get_user_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementGetUserDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementClientGetUserDataResult, PFPlayerDataManagementClientGetUserDataAsync, PFPlayerDataManagementClientGetUserDataGetResultSize, PFPlayerDataManagementClientGetUserDataGetResult, to_variant_PFPlayerDataManagementClientGetUserDataResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientGetUserData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientGetUserData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::get_user_publisher_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementGetUserDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementClientGetUserDataResult, PFPlayerDataManagementClientGetUserPublisherDataAsync, PFPlayerDataManagementClientGetUserPublisherDataGetResultSize, PFPlayerDataManagementClientGetUserPublisherDataGetResult, to_variant_PFPlayerDataManagementClientGetUserDataResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientGetUserPublisherData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientGetUserPublisherData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::get_user_publisher_read_only_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementGetUserDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementClientGetUserDataResult, PFPlayerDataManagementClientGetUserPublisherReadOnlyDataAsync, PFPlayerDataManagementClientGetUserPublisherReadOnlyDataGetResultSize, PFPlayerDataManagementClientGetUserPublisherReadOnlyDataGetResult, to_variant_PFPlayerDataManagementClientGetUserDataResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientGetUserPublisherReadOnlyData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientGetUserPublisherReadOnlyData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::get_user_read_only_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementGetUserDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementGetUserDataRequest, PFPlayerDataManagementClientGetUserDataResult, PFPlayerDataManagementClientGetUserReadOnlyDataAsync, PFPlayerDataManagementClientGetUserReadOnlyDataGetResultSize, PFPlayerDataManagementClientGetUserReadOnlyDataGetResult, to_variant_PFPlayerDataManagementClientGetUserDataResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientGetUserReadOnlyData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientGetUserReadOnlyData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::list_player_custom_properties_async(const Ref<PlayFabUser> &p_user) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityNoRequestVariableResultCallContext<PFPlayerDataManagementClientListPlayerCustomPropertiesResult, PFPlayerDataManagementClientListPlayerCustomPropertiesAsync, PFPlayerDataManagementClientListPlayerCustomPropertiesGetResultSize, PFPlayerDataManagementClientListPlayerCustomPropertiesGetResult, to_variant_PFPlayerDataManagementClientListPlayerCustomPropertiesResult>(runtime, pending_signal, p_user, "PFPlayerDataManagementClientListPlayerCustomProperties");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientListPlayerCustomProperties.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::update_player_custom_properties_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest, PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest, PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult, PFPlayerDataManagementClientUpdatePlayerCustomPropertiesAsync, PFPlayerDataManagementClientUpdatePlayerCustomPropertiesGetResult, to_variant_PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientUpdatePlayerCustomProperties");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientUpdatePlayerCustomProperties.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::update_user_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementClientUpdateUserDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFPlayerDataManagementClientUpdateUserDataRequest, PFPlayerDataManagementClientUpdateUserDataRequest, PFPlayerDataManagementUpdateUserDataResult, PFPlayerDataManagementClientUpdateUserDataAsync, PFPlayerDataManagementClientUpdateUserDataGetResult, to_variant_PFPlayerDataManagementUpdateUserDataResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientUpdateUserData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientUpdateUserData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabPlayerData::update_user_publisher_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFPlayerDataManagementClientUpdateUserDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFPlayerDataManagementClientUpdateUserDataRequest, PFPlayerDataManagementClientUpdateUserDataRequest, PFPlayerDataManagementUpdateUserDataResult, PFPlayerDataManagementClientUpdateUserPublisherDataAsync, PFPlayerDataManagementClientUpdateUserPublisherDataGetResult, to_variant_PFPlayerDataManagementUpdateUserDataResult>(runtime, pending_signal, p_user, std::move(request), "PFPlayerDataManagementClientUpdateUserPublisherData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFPlayerDataManagementClientUpdateUserPublisherData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabStatistics::_bind_methods() {
    ClassDB::bind_method(D_METHOD("create_statistic_definition_async", "user", "request"), &PlayFabStatistics::create_statistic_definition_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_statistic_definition_async", "user", "request"), &PlayFabStatistics::delete_statistic_definition_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("delete_statistics_async", "user", "request"), &PlayFabStatistics::delete_statistics_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_statistic_definition_async", "user", "request"), &PlayFabStatistics::get_statistic_definition_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_statistics_async", "user", "request"), &PlayFabStatistics::get_statistics_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_statistics_for_entities_async", "user", "request"), &PlayFabStatistics::get_statistics_for_entities_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("increment_statistic_version_async", "user", "request"), &PlayFabStatistics::increment_statistic_version_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("list_statistic_definitions_async", "user", "request"), &PlayFabStatistics::list_statistic_definitions_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_statistic_definition_async", "user", "request"), &PlayFabStatistics::update_statistic_definition_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("update_statistics_async", "user", "request"), &PlayFabStatistics::update_statistics_async, DEFVAL(Dictionary()));
}
void PlayFabStatistics::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabStatistics::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabStatistics::create_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsCreateStatisticDefinitionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFStatisticsCreateStatisticDefinitionRequest, PFStatisticsCreateStatisticDefinitionRequest, PFStatisticsCreateStatisticDefinitionAsync>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsCreateStatisticDefinition");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsCreateStatisticDefinition.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::delete_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsDeleteStatisticDefinitionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFStatisticsDeleteStatisticDefinitionRequest, PFStatisticsDeleteStatisticDefinitionRequest, PFStatisticsDeleteStatisticDefinitionAsync>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsDeleteStatisticDefinition");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsDeleteStatisticDefinition.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::delete_statistics_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsDeleteStatisticsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFStatisticsDeleteStatisticsRequest, PFStatisticsDeleteStatisticsRequest, PFStatisticsDeleteStatisticsResponse, PFStatisticsDeleteStatisticsAsync, PFStatisticsDeleteStatisticsGetResultSize, PFStatisticsDeleteStatisticsGetResult, to_variant_PFStatisticsDeleteStatisticsResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsDeleteStatistics");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsDeleteStatistics.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::get_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsGetStatisticDefinitionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFStatisticsGetStatisticDefinitionRequest, PFStatisticsGetStatisticDefinitionRequest, PFStatisticsGetStatisticDefinitionResponse, PFStatisticsGetStatisticDefinitionAsync, PFStatisticsGetStatisticDefinitionGetResultSize, PFStatisticsGetStatisticDefinitionGetResult, to_variant_PFStatisticsGetStatisticDefinitionResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsGetStatisticDefinition");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsGetStatisticDefinition.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::get_statistics_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsGetStatisticsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFStatisticsGetStatisticsRequest, PFStatisticsGetStatisticsRequest, PFStatisticsGetStatisticsResponse, PFStatisticsGetStatisticsAsync, PFStatisticsGetStatisticsGetResultSize, PFStatisticsGetStatisticsGetResult, to_variant_PFStatisticsGetStatisticsResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsGetStatistics");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsGetStatistics.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::get_statistics_for_entities_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsGetStatisticsForEntitiesRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFStatisticsGetStatisticsForEntitiesRequest, PFStatisticsGetStatisticsForEntitiesRequest, PFStatisticsGetStatisticsForEntitiesResponse, PFStatisticsGetStatisticsForEntitiesAsync, PFStatisticsGetStatisticsForEntitiesGetResultSize, PFStatisticsGetStatisticsForEntitiesGetResult, to_variant_PFStatisticsGetStatisticsForEntitiesResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsGetStatisticsForEntities");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsGetStatisticsForEntities.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::increment_statistic_version_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsIncrementStatisticVersionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityFixedResultCallContext<OwnedPFStatisticsIncrementStatisticVersionRequest, PFStatisticsIncrementStatisticVersionRequest, PFStatisticsIncrementStatisticVersionResponse, PFStatisticsIncrementStatisticVersionAsync, PFStatisticsIncrementStatisticVersionGetResult, to_variant_PFStatisticsIncrementStatisticVersionResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsIncrementStatisticVersion");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsIncrementStatisticVersion.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::list_statistic_definitions_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsListStatisticDefinitionsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFStatisticsListStatisticDefinitionsRequest, PFStatisticsListStatisticDefinitionsRequest, PFStatisticsListStatisticDefinitionsResponse, PFStatisticsListStatisticDefinitionsAsync, PFStatisticsListStatisticDefinitionsGetResultSize, PFStatisticsListStatisticDefinitionsGetResult, to_variant_PFStatisticsListStatisticDefinitionsResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsListStatisticDefinitions");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsListStatisticDefinitions.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::update_statistic_definition_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsUpdateStatisticDefinitionRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVoidCallContext<OwnedPFStatisticsUpdateStatisticDefinitionRequest, PFStatisticsUpdateStatisticDefinitionRequest, PFStatisticsUpdateStatisticDefinitionAsync>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsUpdateStatisticDefinition");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsUpdateStatisticDefinition.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabStatistics::update_statistics_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFStatisticsUpdateStatisticsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFStatisticsUpdateStatisticsRequest, PFStatisticsUpdateStatisticsRequest, PFStatisticsUpdateStatisticsResponse, PFStatisticsUpdateStatisticsAsync, PFStatisticsUpdateStatisticsGetResultSize, PFStatisticsUpdateStatisticsGetResult, to_variant_PFStatisticsUpdateStatisticsResponse>(runtime, pending_signal, p_user, std::move(request), "PFStatisticsUpdateStatistics");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFStatisticsUpdateStatistics.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

void PlayFabTitleData::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_publisher_data_async", "user", "request"), &PlayFabTitleData::get_publisher_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_time_async", "user"), &PlayFabTitleData::get_time_async);
    ClassDB::bind_method(D_METHOD("get_title_data_async", "user", "request"), &PlayFabTitleData::get_title_data_async, DEFVAL(Dictionary()));
    ClassDB::bind_method(D_METHOD("get_title_news_async", "user", "request"), &PlayFabTitleData::get_title_news_async, DEFVAL(Dictionary()));
}
void PlayFabTitleData::set_owner(PlayFab *p_owner) { m_owner = p_owner; }
PlayFabRuntime *PlayFabTitleData::_get_runtime() const { return m_owner != nullptr ? m_owner->get_runtime() : nullptr; }

Signal PlayFabTitleData::get_publisher_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFTitleDataManagementGetPublisherDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFTitleDataManagementGetPublisherDataRequest, PFTitleDataManagementGetPublisherDataRequest, PFTitleDataManagementGetPublisherDataResult, PFTitleDataManagementClientGetPublisherDataAsync, PFTitleDataManagementClientGetPublisherDataGetResultSize, PFTitleDataManagementClientGetPublisherDataGetResult, to_variant_PFTitleDataManagementGetPublisherDataResult>(runtime, pending_signal, p_user, std::move(request), "PFTitleDataManagementClientGetPublisherData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFTitleDataManagementClientGetPublisherData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabTitleData::get_time_async(const Ref<PlayFabUser> &p_user) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityNoRequestFixedResultCallContext<PFTitleDataManagementGetTimeResult, PFTitleDataManagementClientGetTimeAsync, PFTitleDataManagementClientGetTimeGetResult, to_variant_PFTitleDataManagementGetTimeResult>(runtime, pending_signal, p_user, "PFTitleDataManagementClientGetTime");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFTitleDataManagementClientGetTime.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabTitleData::get_title_data_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFTitleDataManagementGetTitleDataRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFTitleDataManagementGetTitleDataRequest, PFTitleDataManagementGetTitleDataRequest, PFTitleDataManagementGetTitleDataResult, PFTitleDataManagementClientGetTitleDataAsync, PFTitleDataManagementClientGetTitleDataGetResultSize, PFTitleDataManagementClientGetTitleDataGetResult, to_variant_PFTitleDataManagementGetTitleDataResult>(runtime, pending_signal, p_user, std::move(request), "PFTitleDataManagementClientGetTitleData");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFTitleDataManagementClientGetTitleData.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

Signal PlayFabTitleData::get_title_news_async(const Ref<PlayFabUser> &p_user, const Dictionary &p_request) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) { return make_api_call_error(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first."); }
    if (!validate_api_user(p_user)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_user", "This PlayFab API requires a signed-in PlayFabUser."); }
    auto request = std::make_unique<OwnedPFTitleDataManagementGetTitleNewsRequest>();
    String request_error;
    if (!request->from_dictionary(p_request, &request_error)) { return make_api_call_error(runtime, E_INVALIDARG, "invalid_request", request_error.is_empty() ? "Invalid PlayFab API request." : request_error); }
    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new EntityVariableResultCallContext<OwnedPFTitleDataManagementGetTitleNewsRequest, PFTitleDataManagementGetTitleNewsRequest, PFTitleDataManagementGetTitleNewsResult, PFTitleDataManagementClientGetTitleNewsAsync, PFTitleDataManagementClientGetTitleNewsGetResultSize, PFTitleDataManagementClientGetTitleNewsGetResult, to_variant_PFTitleDataManagementGetTitleNewsResult>(runtime, pending_signal, p_user, std::move(request), "PFTitleDataManagementClientGetTitleNews");
    context->bind_cancel_handler();
    const HRESULT start_hr = context->start();
    if (FAILED(start_hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(start_hr, "Failed to start PlayFab API call: PFTitleDataManagementClientGetTitleNews.", "playfab_api_start_failed");
        runtime->set_last_error(result);
        pending_signal->complete_deferred(result);
    }
    return pending_signal->get_completed_signal();
}

}
