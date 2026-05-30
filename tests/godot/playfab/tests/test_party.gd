extends "res://addons/godot_gdk_tests/playfab_test_base.gd"
## PlayFab Party public contract coverage.
##
## These tests are intentionally non-live: they validate registration,
## object shape, constants, and immediate failure paths without bringing
## up any Party network or connecting to the PlayFab Party service.

const PARTY_REGISTERED_CLASSES := [
	"PlayFabParty",
	"PlayFabPartyConfig",
	"PlayFabPartyTextMessageConfig",
	"PlayFabPartyMember",
	"PlayFabPartyChatMessage",
	"PlayFabPartyChatStateChange",
	"PlayFabPartyChatControl",
	"PlayFabPartyChat",
	"PlayFabPartyNetworkStateChange",
	"PlayFabPartyNetwork",
	"PlayFabPartyPeer",
]

const PARTY_SERVICE_METHODS := [
	"is_initialized",
	"initialize_async",
	"shutdown_async",
	"create_and_join_network_async",
	"join_network_async",
	"leave_network_async",
	"get_chat",
	"get_networks",
]

const PARTY_PEER_METHODS := [
	"get_network",
	"get_local_user",
	"get_descriptor",
	"get_peer_entity_key",
	"get_peer_member",
	"get_peers",
	"get_local_chat_control",
	"get_peer_chat_control",
	"send_text_async",
	"set_peer_chat_permissions_async",
	"set_peer_muted_async",
	"close_with_reason",
]

const PARTY_PEER_SIGNALS := [
	"connection_state_changed",
	"network_error",
	"chat_control_added",
	"chat_control_removed",
	"text_message_received",
	"transcription_received",
	"chat_permissions_changed",
	"peer_muted_changed",
]


func test_party_class_registration() -> void:
	for registered_class in PARTY_REGISTERED_CLASSES:
		assert_true(ClassDB.class_exists(registered_class), "%s registered in ClassDB" % registered_class)

	assert_true(ClassDB.is_parent_class("PlayFabParty", "RefCounted"), "PlayFabParty extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyConfig", "RefCounted"), "PlayFabPartyConfig extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyTextMessageConfig", "RefCounted"), "PlayFabPartyTextMessageConfig extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyMember", "RefCounted"), "PlayFabPartyMember extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyChatMessage", "RefCounted"), "PlayFabPartyChatMessage extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyChatStateChange", "RefCounted"), "PlayFabPartyChatStateChange extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyChatControl", "RefCounted"), "PlayFabPartyChatControl extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyChat", "RefCounted"), "PlayFabPartyChat extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyNetworkStateChange", "RefCounted"), "PlayFabPartyNetworkStateChange extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyNetwork", "RefCounted"), "PlayFabPartyNetwork extends RefCounted")
	assert_true(ClassDB.is_parent_class("PlayFabPartyPeer", "MultiplayerPeerExtension"), "PlayFabPartyPeer extends MultiplayerPeerExtension")


func test_party_root_accessor() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()

	var party = playfab.get_party()
	assert_object_is(party, "PlayFabParty", "PlayFab.get_party() returns PlayFabParty")
	assert_object_is(playfab.party, "PlayFabParty", "PlayFab.party property returns PlayFabParty")

	if party != null:
		for method_name in PARTY_SERVICE_METHODS:
			assert_has_method_named(party, method_name)
		assert_has_signal_named(party, "party_error")
		assert_eq(party.is_initialized(), false, "PlayFab.party starts uninitialized")
		assert_eq(party.get_networks().size(), 0, "PlayFab.party starts with no tracked networks")

		var chat = party.get_chat()
		assert_object_is(chat, "PlayFabPartyChat", "PlayFab.party.get_chat() returns PlayFabPartyChat")


func test_party_stable_constants() -> void:
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_NONE"), 0, "DIRECT_PEER_CONNECTIVITY_NONE == 0")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE"), 1, "DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE == 1")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_DIFFERENT_PLATFORM_TYPE"), 2, "DIRECT_PEER_CONNECTIVITY_DIFFERENT_PLATFORM_TYPE == 2")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE"), 3, "DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE == 3")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_ENTITY_LOGIN_PROVIDER"), 4, "DIRECT_PEER_CONNECTIVITY_SAME_ENTITY_LOGIN_PROVIDER == 4")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_DIFFERENT_ENTITY_LOGIN_PROVIDER"), 8, "DIRECT_PEER_CONNECTIVITY_DIFFERENT_ENTITY_LOGIN_PROVIDER == 8")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY_ENTITY_LOGIN_PROVIDER"), 12, "DIRECT_PEER_CONNECTIVITY_ANY_ENTITY_LOGIN_PROVIDER == 12")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY"), 15, "DIRECT_PEER_CONNECTIVITY_ANY == 15")
	assert_eq(get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS"), 16, "DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS == 16")

	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_CREATING"), 0, "NETWORK_STATE_CREATING == 0")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_CONNECTING"), 1, "NETWORK_STATE_CONNECTING == 1")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_AUTHENTICATING"), 2, "NETWORK_STATE_AUTHENTICATING == 2")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_CONNECTED"), 3, "NETWORK_STATE_CONNECTED == 3")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_DISCONNECTING"), 4, "NETWORK_STATE_DISCONNECTING == 4")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_DISCONNECTED"), 5, "NETWORK_STATE_DISCONNECTED == 5")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATE_FAILED"), 6, "NETWORK_STATE_FAILED == 6")

	assert_eq(get_class_constant("PlayFabParty", "CHAT_PERMISSION_NONE"), 0, "CHAT_PERMISSION_NONE == 0")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_PERMISSION_SEND_AUDIO"), 1, "CHAT_PERMISSION_SEND_AUDIO == 1")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_AUDIO"), 2, "CHAT_PERMISSION_RECEIVE_AUDIO == 2")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_TEXT"), 4, "CHAT_PERMISSION_RECEIVE_TEXT == 4")


func test_party_config_defaults() -> void:
	var config = instantiate_class("PlayFabPartyConfig")
	assert_object_is(config, "PlayFabPartyConfig", "PlayFabPartyConfig can be instantiated")
	if config == null:
		return

	assert_eq(config.max_players, 8, "PlayFabPartyConfig.max_players default")
	assert_eq(config.direct_peer_connectivity, get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_NONE"), "PlayFabPartyConfig.direct_peer_connectivity default")
	assert_eq(config.invitation_id, "", "PlayFabPartyConfig.invitation_id default")
	assert_eq(config.enable_voice_chat, true, "PlayFabPartyConfig.enable_voice_chat default")
	assert_eq(config.enable_text_chat, true, "PlayFabPartyConfig.enable_text_chat default")
	assert_eq(config.enable_transcription, false, "PlayFabPartyConfig.enable_transcription default")
	assert_eq(config.enable_translation, false, "PlayFabPartyConfig.enable_translation default")
	assert_eq(config.audio_input, "", "PlayFabPartyConfig.audio_input default")
	assert_eq(config.audio_output, "", "PlayFabPartyConfig.audio_output default")

	config.max_players = 4
	config.direct_peer_connectivity = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE")
	config.invitation_id = "invite-1"
	config.enable_voice_chat = false
	config.enable_text_chat = false
	config.metadata = {"map": "arena"}
	assert_eq(config.max_players, 4, "PlayFabPartyConfig.max_players setter")
	assert_eq(config.direct_peer_connectivity, get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE"), "PlayFabPartyConfig.direct_peer_connectivity setter")
	assert_eq(config.invitation_id, "invite-1", "PlayFabPartyConfig.invitation_id setter")
	assert_eq(config.metadata.get("map"), "arena", "PlayFabPartyConfig.metadata setter")

	var text_config = instantiate_class("PlayFabPartyTextMessageConfig")
	assert_object_is(text_config, "PlayFabPartyTextMessageConfig", "PlayFabPartyTextMessageConfig can be instantiated")
	if text_config != null:
		text_config.language_code = "en-US"
		text_config.translate_to_languages = PackedStringArray(["es-MX", "fr-FR"])
		text_config.metadata = {"id": 12}
		assert_eq(text_config.language_code, "en-US", "PlayFabPartyTextMessageConfig.language_code setter")
		assert_eq(text_config.translate_to_languages.size(), 2, "PlayFabPartyTextMessageConfig.translate_to_languages setter")
		assert_eq(int(text_config.metadata.get("id", 0)), 12, "PlayFabPartyTextMessageConfig.metadata setter")


func test_party_wrapper_classes_instantiable() -> void:
	for wrapper_class in [
		"PlayFabPartyMember",
		"PlayFabPartyChatMessage",
		"PlayFabPartyChatStateChange",
		"PlayFabPartyChatControl",
		"PlayFabPartyChat",
		"PlayFabPartyNetworkStateChange",
		"PlayFabPartyNetwork",
		"PlayFabPartyPeer",
	]:
		assert_object_is(instantiate_class(wrapper_class), wrapper_class, "%s can be instantiated" % wrapper_class)


func test_party_peer_contract() -> void:
	var peer = instantiate_class("PlayFabPartyPeer")
	assert_object_is(peer, "PlayFabPartyPeer", "PlayFabPartyPeer can be instantiated")
	if peer == null:
		return

	for method_name in PARTY_PEER_METHODS:
		assert_has_method_named(peer, method_name)
	for signal_name in PARTY_PEER_SIGNALS:
		assert_has_signal_named(peer, signal_name)

	# A freshly constructed peer is detached: no network, disconnected, host id 0.
	assert_eq(peer.get_network(), null, "Detached PlayFabPartyPeer.get_network() returns null")
	assert_eq(peer.get_descriptor(), "", "Detached PlayFabPartyPeer.get_descriptor() empty")
	assert_eq(peer.get_peers().size(), 0, "Detached PlayFabPartyPeer.get_peers() empty")
	assert_eq(peer.get_unique_id(), 0, "Detached PlayFabPartyPeer.get_unique_id() == 0")
	assert_eq(peer.get_connection_status(), MultiplayerPeer.CONNECTION_DISCONNECTED, "Detached PlayFabPartyPeer.get_connection_status() == DISCONNECTED")
	assert_eq(peer.get_available_packet_count(), 0, "Detached PlayFabPartyPeer.get_available_packet_count() == 0")
	peer.close_with_reason("audit-detached-close")
	assert_eq(peer.get_connection_status(), MultiplayerPeer.CONNECTION_DISCONNECTED, "Detached PlayFabPartyPeer.close_with_reason() keeps peer disconnected")
	assert_eq(peer.get_peers().size(), 0, "Detached PlayFabPartyPeer.close_with_reason() leaves peer list empty")


func test_party_network_detached_helpers() -> void:
	var network = instantiate_class("PlayFabPartyNetwork")
	assert_object_is(network, "PlayFabPartyNetwork", "PlayFabPartyNetwork can be instantiated")
	if network == null:
		return

	assert_has_method_named(network, "get_descriptor")
	assert_has_method_named(network, "leave_async")
	assert_has_signal_named(network, "state_changed")
	assert_eq(network.get_descriptor(), "", "Detached PlayFabPartyNetwork.get_descriptor() empty")
	assert_eq(network.is_host_network(), false, "Detached PlayFabPartyNetwork.is_host_network() == false")

	# Detached network has no owning service; leave_async must surface a deferred error result.
	await _assert_signal_error(network.leave_async(), "party_resource_not_ready", "Detached PlayFabPartyNetwork.leave_async() reports party_resource_not_ready")


func test_party_invalid_user_failures() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var party = playfab.get_party()
	if party == null:
		return

	var blank_user = instantiate_class("PlayFabUser")
	var config = instantiate_class("PlayFabPartyConfig")

	await _assert_signal_error(party.create_and_join_network_async(blank_user, config), "party_invalid_user", "PlayFab.party.create_and_join_network_async() with blank user")
	await _assert_signal_error(party.join_network_async(blank_user, "descriptor", config), "party_invalid_user", "PlayFab.party.join_network_async() with blank user")
	await _assert_signal_error(party.leave_network_async(null), "party_invalid_options", "PlayFab.party.leave_network_async(null) reports party_invalid_options")


# Regression: PartyManager::CreateNewNetwork rejects the network
# configuration struct if platform-type flags are not combined with at
# least one entity-login-provider flag (or vice versa), or if
# OnlyServers is mixed with anything else. The addon validates the
# bitmask before reaching the SDK so authors get a clear actionable
# error instead of a generic "invalid network configuration struct".
func test_party_invalid_direct_peer_connectivity_rejected() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var party = playfab.get_party()
	if party == null:
		return

	var blank_user = instantiate_class("PlayFabUser")
	var same_platform_only = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE")
	var any_platform_only = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY_PLATFORM_TYPE")
	var login_provider_only = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_ENTITY_LOGIN_PROVIDER")
	var only_servers = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ONLY_SERVERS")
	var any_preset = get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_ANY")

	# Platform-type flag without login-provider flag is invalid.
	var bad_platform_only = instantiate_class("PlayFabPartyConfig")
	bad_platform_only.direct_peer_connectivity = same_platform_only
	await _assert_signal_error(party.create_and_join_network_async(blank_user, bad_platform_only), "party_invalid_options", "create_and_join_network_async(SAME_PLATFORM_TYPE alone) rejected")

	var bad_any_platform = instantiate_class("PlayFabPartyConfig")
	bad_any_platform.direct_peer_connectivity = any_platform_only
	await _assert_signal_error(party.create_and_join_network_async(blank_user, bad_any_platform), "party_invalid_options", "create_and_join_network_async(ANY_PLATFORM_TYPE alone) rejected")

	# Login-provider flag without platform-type flag is invalid.
	var bad_login_only = instantiate_class("PlayFabPartyConfig")
	bad_login_only.direct_peer_connectivity = login_provider_only
	await _assert_signal_error(party.create_and_join_network_async(blank_user, bad_login_only), "party_invalid_options", "create_and_join_network_async(SAME_ENTITY_LOGIN_PROVIDER alone) rejected")

	# OnlyServers mixed with anything else is invalid.
	var bad_only_servers_mix = instantiate_class("PlayFabPartyConfig")
	bad_only_servers_mix.direct_peer_connectivity = only_servers | any_preset
	await _assert_signal_error(party.create_and_join_network_async(blank_user, bad_only_servers_mix), "party_invalid_options", "create_and_join_network_async(ONLY_SERVERS | ANY) rejected")

	# Bits outside the known mask are rejected.
	var bad_unknown_bits = instantiate_class("PlayFabPartyConfig")
	bad_unknown_bits.direct_peer_connectivity = 0x40
	await _assert_signal_error(party.create_and_join_network_async(blank_user, bad_unknown_bits), "party_invalid_options", "create_and_join_network_async(unknown bits 0x40) rejected")

	# Valid shapes pass the connectivity check and then trip the next
	# guard (blank user -> party_invalid_user). This confirms the
	# validator doesn't false-positive on the recommended combinations.
	for valid_value in [0, any_preset, only_servers, same_platform_only | login_provider_only]:
		var ok_config = instantiate_class("PlayFabPartyConfig")
		ok_config.direct_peer_connectivity = valid_value
		await _assert_signal_error(party.create_and_join_network_async(blank_user, ok_config), "party_invalid_user", "create_and_join_network_async(valid connectivity=%d) passes connectivity check" % valid_value)


func test_party_initialize_requires_playfab_runtime() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var party = playfab.get_party()
	if party == null:
		return

	# Without PlayFab.initialize(), Party initialization must fail with party_not_initialized.
	await _assert_signal_error(party.initialize_async(), "party_not_initialized", "PlayFab.party.initialize_async() before PlayFab.initialize()")


func test_party_peer_methods_route_to_chat() -> void:
	var peer = instantiate_class("PlayFabPartyPeer")
	if peer == null:
		return

	# Detached peer has no chat transport, so the peer-id helpers must surface deferred error results.
	await _assert_signal_error(peer.send_text_async("hi", PackedInt32Array([2])), "party_peer_not_connected", "Detached PlayFabPartyPeer.send_text_async() reports party_peer_not_connected")
	await _assert_signal_error(peer.set_peer_chat_permissions_async(2, get_class_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_TEXT")), "party_peer_not_connected", "Detached PlayFabPartyPeer.set_peer_chat_permissions_async() reports party_peer_not_connected")
	await _assert_signal_error(peer.set_peer_muted_async(2, true), "party_peer_not_connected", "Detached PlayFabPartyPeer.set_peer_muted_async() reports party_peer_not_connected")


func test_party_chat_control_helpers() -> void:
	var control = instantiate_class("PlayFabPartyChatControl")
	if control == null:
		return

	for method_name in [
		"get_id",
		"get_user",
		"is_voice_enabled",
		"is_text_enabled",
		"is_transcription_enabled",
		"is_local",
		"send_text_async",
		"set_permissions_async",
		"set_muted_async",
		"destroy_async",
	]:
		assert_has_method_named(control, method_name)
	for signal_name in ["state_changed", "message_received", "transcription_received"]:
		assert_has_signal_named(control, signal_name)

	await _assert_signal_error(control.send_text_async([], "hello"), "party_resource_not_ready", "Detached PlayFabPartyChatControl.send_text_async() reports party_resource_not_ready")
	await _assert_signal_error(control.set_permissions_async(null, get_class_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_TEXT")), "party_chat_permission_failed", "Detached PlayFabPartyChatControl.set_permissions_async() reports party_chat_permission_failed")
	await _assert_signal_error(control.set_muted_async(null, true), "party_chat_permission_failed", "Detached PlayFabPartyChatControl.set_muted_async() reports party_chat_permission_failed")
	var destroy_signal = control.destroy_async()
	assert_eq(typeof(destroy_signal), TYPE_SIGNAL, "Detached PlayFabPartyChatControl.destroy_async() returns completion Signal")
	if typeof(destroy_signal) == TYPE_SIGNAL:
		assert_playfab_result_ok(await await_completion(destroy_signal), "Detached PlayFabPartyChatControl.destroy_async()")


func test_party_shutdown_async_explicit_await_uninitialized() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var party = playfab.get_party()
	if party == null:
		return

	assert_playfab_result_ok(await await_completion(party.shutdown_async()), "await PlayFab.party.shutdown_async() while uninitialized")
	assert_false(party.is_initialized(), "PlayFab.party remains uninitialized after explicit awaited shutdown")


func test_party_shutdown_cancels_reentrant_pending_operations() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var party = playfab.get_party()
	if party == null:
		return
	if not party.has_method("_test_enqueue_shutdown_pending"):
		pending("PlayFab Party shutdown re-entry test requires debug test hooks.")
		return

	var completion_state := {
		"first": false,
		"reentrant": false,
	}
	var first_signal = party._test_enqueue_shutdown_pending()
	first_signal.connect(func(_result):
		completion_state["first"] = true
		var reentrant_signal = party._test_enqueue_shutdown_pending()
		reentrant_signal.connect(func(_reentrant_result):
			completion_state["reentrant"] = true
		)
	)

	assert_playfab_result_ok(await await_completion(party.shutdown_async()), "PlayFab.party.shutdown_async() with re-entrant pending completion")
	assert_true(completion_state["first"], "Initial pending operation completed during shutdown")
	assert_true(completion_state["reentrant"], "Re-entrant pending operation completed during the same shutdown")
	assert_eq(party._test_pending_operation_count(), 0, "Shutdown drains all PlayFab Party pending operations")


func _assert_signal_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
