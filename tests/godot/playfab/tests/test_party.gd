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
	"PlayFabPartyTextToSpeechProfile",
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
	"release_local_user_async",
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
	"close_with_reason",
]

const PARTY_PEER_SIGNALS := [
	"connection_state_changed",
	"network_error",
]

# Chat is no longer carried on the transport peer. It lives on the single
# meshed chat surface reached as PlayFab.party.chat (PlayFabPartyChat), keyed by
# PlayFab entity keys.
const PARTY_CHAT_METHODS := [
	"get_local_chat_control",
	"get_chat_controls",
	"get_remote_entity_keys",
	"get_chat_control",
	"send_text_async",
	"set_chat_permissions_async",
	"set_audio_muted_async",
	"set_text_muted_async",
	"create_local_chat_control_async",
	"destroy_local_chat_control_async",
	"get_local_chat_indicator",
	"get_chat_indicator",
	"get_chat_indicators",
	"is_audio_input_muted",
	"set_audio_input_muted_async",
	"get_audio_render_volume",
	"set_audio_render_volume_async",
	"get_chat_permissions",
	"is_audio_muted",
	"is_text_muted",
	"get_language",
	"set_language_async",
	"get_transcription_options",
	"set_transcription_options_async",
	"get_text_chat_options",
	"set_text_chat_options_async",
	"get_audio_encoder_bitrate",
	"set_audio_encoder_bitrate_async",
	"get_voice_audio_options",
	"set_voice_audio_options_async",
	"get_audio_input_state",
	"get_audio_output_state",
	"populate_text_to_speech_profiles_async",
	"get_text_to_speech_profiles",
	"get_text_to_speech_profile",
	"set_text_to_speech_profile_async",
	"synthesize_text_to_speech_async",
]

const PARTY_CHAT_SIGNALS := [
	"state_changed",
	"chat_control_added",
	"chat_control_removed",
	"text_message_received",
	"transcription_received",
	"chat_permissions_changed",
	"audio_muted_changed",
	"text_muted_changed",
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

	# Chat indicators are polled, not signalled. Values mirror the native
	# PartyLocalChatControlChatIndicator / PartyChatControlChatIndicator enums.
	assert_eq(get_class_constant("PlayFabParty", "LOCAL_CHAT_INDICATOR_SILENT"), 0, "LOCAL_CHAT_INDICATOR_SILENT == 0")
	assert_eq(get_class_constant("PlayFabParty", "LOCAL_CHAT_INDICATOR_TALKING"), 1, "LOCAL_CHAT_INDICATOR_TALKING == 1")
	assert_eq(get_class_constant("PlayFabParty", "LOCAL_CHAT_INDICATOR_AUDIO_INPUT_MUTED"), 2, "LOCAL_CHAT_INDICATOR_AUDIO_INPUT_MUTED == 2")
	assert_eq(get_class_constant("PlayFabParty", "LOCAL_CHAT_INDICATOR_NO_AUDIO_INPUT"), 3, "LOCAL_CHAT_INDICATOR_NO_AUDIO_INPUT == 3")

	assert_eq(get_class_constant("PlayFabParty", "CHAT_INDICATOR_SILENT"), 0, "CHAT_INDICATOR_SILENT == 0")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_INDICATOR_TALKING"), 1, "CHAT_INDICATOR_TALKING == 1")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_INDICATOR_INCOMING_VOICE_DISABLED"), 2, "CHAT_INDICATOR_INCOMING_VOICE_DISABLED == 2")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_INDICATOR_INCOMING_COMMUNICATIONS_MUTED"), 3, "CHAT_INDICATOR_INCOMING_COMMUNICATIONS_MUTED == 3")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_INDICATOR_NO_REMOTE_INPUT"), 4, "CHAT_INDICATOR_NO_REMOTE_INPUT == 4")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_INDICATOR_REMOTE_AUDIO_INPUT_MUTED"), 5, "CHAT_INDICATOR_REMOTE_AUDIO_INPUT_MUTED == 5")

	assert_eq(get_class_constant("PlayFabParty", "AUDIO_INPUT_STATE_NO_INPUT"), 0, "AUDIO_INPUT_STATE_NO_INPUT == 0")
	assert_eq(get_class_constant("PlayFabParty", "AUDIO_INPUT_STATE_INITIALIZED"), 1, "AUDIO_INPUT_STATE_INITIALIZED == 1")
	assert_eq(get_class_constant("PlayFabParty", "AUDIO_INPUT_STATE_USER_CONSENT_DENIED"), 3, "AUDIO_INPUT_STATE_USER_CONSENT_DENIED == 3")
	assert_eq(get_class_constant("PlayFabParty", "AUDIO_OUTPUT_STATE_NO_OUTPUT"), 0, "AUDIO_OUTPUT_STATE_NO_OUTPUT == 0")
	assert_eq(get_class_constant("PlayFabParty", "AUDIO_OUTPUT_STATE_INITIALIZED"), 1, "AUDIO_OUTPUT_STATE_INITIALIZED == 1")

	assert_eq(get_class_constant("PlayFabParty", "TRANSCRIPTION_OPTION_NONE"), 0, "TRANSCRIPTION_OPTION_NONE == 0")
	assert_eq(get_class_constant("PlayFabParty", "TRANSCRIPTION_OPTION_TRANSCRIBE_SELF"), 1, "TRANSCRIPTION_OPTION_TRANSCRIBE_SELF == 1")
	assert_eq(get_class_constant("PlayFabParty", "TRANSCRIPTION_OPTION_TRANSLATE_TO_LOCAL_LANGUAGE"), 16, "TRANSCRIPTION_OPTION_TRANSLATE_TO_LOCAL_LANGUAGE == 16")
	assert_eq(get_class_constant("PlayFabParty", "TEXT_CHAT_OPTION_NONE"), 0, "TEXT_CHAT_OPTION_NONE == 0")
	assert_eq(get_class_constant("PlayFabParty", "TEXT_CHAT_OPTION_TRANSLATE_TO_LOCAL_LANGUAGE"), 1, "TEXT_CHAT_OPTION_TRANSLATE_TO_LOCAL_LANGUAGE == 1")
	assert_eq(get_class_constant("PlayFabParty", "TEXT_CHAT_OPTION_FILTER_OFFENSIVE_TEXT"), 2, "TEXT_CHAT_OPTION_FILTER_OFFENSIVE_TEXT == 2")

	assert_eq(get_class_constant("PlayFabParty", "CHAT_MESSAGE_OPTION_NONE"), 0, "CHAT_MESSAGE_OPTION_NONE == 0")
	assert_eq(get_class_constant("PlayFabParty", "CHAT_MESSAGE_OPTION_FILTERED_ENTIRE_MESSAGE"), 2, "CHAT_MESSAGE_OPTION_FILTERED_ENTIRE_MESSAGE == 2")

	assert_eq(get_class_constant("PlayFabParty", "TEXT_TO_SPEECH_TYPE_NARRATION"), 0, "TEXT_TO_SPEECH_TYPE_NARRATION == 0")
	assert_eq(get_class_constant("PlayFabParty", "TEXT_TO_SPEECH_TYPE_VOICE_CHAT"), 1, "TEXT_TO_SPEECH_TYPE_VOICE_CHAT == 1")
	assert_eq(get_class_constant("PlayFabParty", "GENDER_NEUTRAL"), 0, "GENDER_NEUTRAL == 0")

	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATISTIC_AVERAGE_RELAY_SERVER_ROUND_TRIP_LATENCY_MS"), 0, "NETWORK_STATISTIC_AVERAGE_RELAY_SERVER_ROUND_TRIP_LATENCY_MS == 0")
	assert_eq(get_class_constant("PlayFabParty", "NETWORK_STATISTIC_CANCELED_SEND_MESSAGE_BYTES"), 15, "NETWORK_STATISTIC_CANCELED_SEND_MESSAGE_BYTES == 15")
	assert_eq(get_class_constant("PlayFabParty", "DEVICE_CONNECTION_TYPE_RELAY_SERVER"), 0, "DEVICE_CONNECTION_TYPE_RELAY_SERVER == 0")
	assert_eq(get_class_constant("PlayFabParty", "DEVICE_CONNECTION_TYPE_DIRECT_PEER_CONNECTION"), 1, "DEVICE_CONNECTION_TYPE_DIRECT_PEER_CONNECTION == 1")


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
	config.audio_input = "capture-device-1"
	config.audio_output = "render-device-1"
	config.language = "en-US"
	assert_eq(config.max_players, 4, "PlayFabPartyConfig.max_players setter")
	assert_eq(config.direct_peer_connectivity, get_class_constant("PlayFabParty", "DIRECT_PEER_CONNECTIVITY_SAME_PLATFORM_TYPE"), "PlayFabPartyConfig.direct_peer_connectivity setter")
	assert_eq(config.invitation_id, "invite-1", "PlayFabPartyConfig.invitation_id setter")
	assert_eq(config.audio_input, "capture-device-1", "PlayFabPartyConfig.audio_input setter")
	assert_eq(config.audio_output, "render-device-1", "PlayFabPartyConfig.audio_output setter")
	assert_eq(config.language, "en-US", "PlayFabPartyConfig.language setter")

	var text_config = instantiate_class("PlayFabPartyTextMessageConfig")
	assert_object_is(text_config, "PlayFabPartyTextMessageConfig", "PlayFabPartyTextMessageConfig can be instantiated")
	if text_config != null:
		text_config.metadata = {"id": 12}
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


func test_party_chat_methods_detached() -> void:
	var chat = instantiate_class("PlayFabPartyChat")
	if chat == null:
		return

	for method_name in PARTY_CHAT_METHODS:
		assert_has_method_named(chat, method_name)
	for signal_name in PARTY_CHAT_SIGNALS:
		assert_has_signal_named(chat, signal_name)

	# A chat surface with no connected network (no owner / no local chat
	# control) must surface deferred error results keyed by entity key.
	await _assert_signal_error(chat.send_text_async("hi", [{ "id": "guest", "type": "title_player_account" }]), "party_peer_not_connected", "Detached PlayFabPartyChat.send_text_async() reports party_peer_not_connected")
	await _assert_signal_error(chat.set_chat_permissions_async({ "id": "guest", "type": "title_player_account" }, get_class_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_TEXT")), "party_peer_not_connected", "Detached PlayFabPartyChat.set_chat_permissions_async() reports party_peer_not_connected")
	await _assert_signal_error(chat.set_audio_muted_async({ "id": "guest", "type": "title_player_account" }, true), "party_peer_not_connected", "Detached PlayFabPartyChat.set_audio_muted_async() reports party_peer_not_connected")
	await _assert_signal_error(chat.set_text_muted_async({ "id": "guest", "type": "title_player_account" }, true), "party_peer_not_connected", "Detached PlayFabPartyChat.set_text_muted_async() reports party_peer_not_connected")


func test_party_chat_polling_surface_detached() -> void:
	# Chat indicators and voice-chat state are polled, never signalled. A
	# detached chat surface must answer with safe defaults instead of erroring,
	# so a title can poll every frame without null-checking.
	var chat = instantiate_class("PlayFabPartyChat")
	if chat == null:
		return

	var guest := { "id": "guest", "type": "title_player_account" }
	assert_eq(chat.get_local_chat_indicator(), get_class_constant("PlayFabParty", "LOCAL_CHAT_INDICATOR_NO_AUDIO_INPUT"), "Detached PlayFabPartyChat.get_local_chat_indicator() reports no audio input")
	assert_eq(chat.get_chat_indicator(guest), get_class_constant("PlayFabParty", "CHAT_INDICATOR_SILENT"), "Detached PlayFabPartyChat.get_chat_indicator() reports silent")
	assert_eq(chat.get_chat_indicators().size(), 0, "Detached PlayFabPartyChat.get_chat_indicators() is empty")
	assert_false(chat.is_audio_input_muted(), "Detached PlayFabPartyChat.is_audio_input_muted() is false")
	assert_eq(chat.get_audio_render_volume(guest), 0.0, "Detached PlayFabPartyChat.get_audio_render_volume() is 0")
	assert_eq(chat.get_language(), "", "Detached PlayFabPartyChat.get_language() is empty")
	assert_eq(chat.get_transcription_options(), get_class_constant("PlayFabParty", "TRANSCRIPTION_OPTION_NONE"), "Detached PlayFabPartyChat.get_transcription_options() is none")
	assert_eq(chat.get_text_chat_options(), get_class_constant("PlayFabParty", "TEXT_CHAT_OPTION_NONE"), "Detached PlayFabPartyChat.get_text_chat_options() is none")
	assert_eq(chat.get_audio_encoder_bitrate(), 0, "Detached PlayFabPartyChat.get_audio_encoder_bitrate() is 0")
	assert_eq(chat.get_audio_input_state(), get_class_constant("PlayFabParty", "AUDIO_INPUT_STATE_NO_INPUT"), "Detached PlayFabPartyChat.get_audio_input_state() is no input")
	assert_eq(chat.get_audio_output_state(), get_class_constant("PlayFabParty", "AUDIO_OUTPUT_STATE_NO_OUTPUT"), "Detached PlayFabPartyChat.get_audio_output_state() is no output")
	assert_eq(chat.get_text_to_speech_profiles().size(), 0, "Detached PlayFabPartyChat.get_text_to_speech_profiles() is empty")
	assert_null(chat.get_text_to_speech_profile(get_class_constant("PlayFabParty", "TEXT_TO_SPEECH_TYPE_NARRATION")), "Detached PlayFabPartyChat.get_text_to_speech_profile() is null")

	# The async setters require a local chat control and must fail deferred.
	await _assert_signal_error(chat.set_audio_input_muted_async(true), "party_resource_not_ready", "Detached PlayFabPartyChat.set_audio_input_muted_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.set_language_async("en-US"), "party_resource_not_ready", "Detached PlayFabPartyChat.set_language_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.set_transcription_options_async(get_class_constant("PlayFabParty", "TRANSCRIPTION_OPTION_TRANSCRIBE_SELF")), "party_resource_not_ready", "Detached PlayFabPartyChat.set_transcription_options_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.set_text_chat_options_async(get_class_constant("PlayFabParty", "TEXT_CHAT_OPTION_TRANSLATE_TO_LOCAL_LANGUAGE")), "party_resource_not_ready", "Detached PlayFabPartyChat.set_text_chat_options_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.set_audio_encoder_bitrate_async(24000), "party_resource_not_ready", "Detached PlayFabPartyChat.set_audio_encoder_bitrate_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.set_voice_audio_options_async(get_class_constant("PlayFabParty", "VOICE_AUDIO_OPTION_NOISE_SUPPRESSION")), "party_resource_not_ready", "Detached PlayFabPartyChat.set_voice_audio_options_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.populate_text_to_speech_profiles_async(), "party_resource_not_ready", "Detached PlayFabPartyChat.populate_text_to_speech_profiles_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.set_text_to_speech_profile_async(get_class_constant("PlayFabParty", "TEXT_TO_SPEECH_TYPE_VOICE_CHAT"), "profile"), "party_resource_not_ready", "Detached PlayFabPartyChat.set_text_to_speech_profile_async() reports party_resource_not_ready")
	await _assert_signal_error(chat.synthesize_text_to_speech_async(get_class_constant("PlayFabParty", "TEXT_TO_SPEECH_TYPE_NARRATION"), "hello"), "party_resource_not_ready", "Detached PlayFabPartyChat.synthesize_text_to_speech_async() reports party_resource_not_ready")


func test_party_text_to_speech_profile_contract() -> void:
	var profile = instantiate_class("PlayFabPartyTextToSpeechProfile")
	assert_object_is(profile, "PlayFabPartyTextToSpeechProfile", "PlayFabPartyTextToSpeechProfile can be instantiated")
	if profile == null:
		return
	for method_name in ["get_identifier", "get_name", "get_language_code", "get_gender"]:
		assert_has_method_named(profile, method_name)
	assert_eq(profile.identifier, "", "PlayFabPartyTextToSpeechProfile.identifier default")
	assert_eq(profile.language_code, "", "PlayFabPartyTextToSpeechProfile.language_code default")
	assert_eq(profile.gender, get_class_constant("PlayFabParty", "GENDER_NEUTRAL"), "PlayFabPartyTextToSpeechProfile.gender default")


func test_party_network_statistics_detached() -> void:
	var network = instantiate_class("PlayFabPartyNetwork")
	if network == null:
		return
	for method_name in ["get_statistics", "get_device_connection_type"]:
		assert_has_method_named(network, method_name)
	# No native network handle => empty statistics and the conservative
	# relay-server answer rather than a crash.
	assert_eq(network.get_statistics().size(), 0, "Detached PlayFabPartyNetwork.get_statistics() is empty")
	assert_eq(network.get_device_connection_type(1), get_class_constant("PlayFabParty", "DEVICE_CONNECTION_TYPE_RELAY_SERVER"), "Detached PlayFabPartyNetwork.get_device_connection_type() reports relay server")


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
		"set_audio_muted_async",
		"set_text_muted_async",
		"destroy_async",
		"get_local_chat_indicator",
		"get_chat_indicator",
		"get_audio_input_state",
		"get_audio_output_state",
		"is_audio_input_muted",
		"set_audio_input_muted_async",
		"get_audio_render_volume",
		"set_audio_render_volume_async",
		"get_language",
		"set_language_async",
		"get_transcription_options",
		"set_transcription_options_async",
		"get_text_chat_options",
		"set_text_chat_options_async",
		"get_audio_encoder_bitrate",
		"set_audio_encoder_bitrate_async",
		"get_voice_audio_options",
		"set_voice_audio_options_async",
		"populate_text_to_speech_profiles_async",
		"get_text_to_speech_profiles",
		"get_text_to_speech_profile",
		"set_text_to_speech_profile_async",
		"synthesize_text_to_speech_async",
	]:
		assert_has_method_named(control, method_name)
	for signal_name in ["state_changed", "message_received", "transcription_received"]:
		assert_has_signal_named(control, signal_name)

	await _assert_signal_error(control.send_text_async([], "hello"), "party_resource_not_ready", "Detached PlayFabPartyChatControl.send_text_async() reports party_resource_not_ready")
	await _assert_signal_error(control.set_permissions_async(null, get_class_constant("PlayFabParty", "CHAT_PERMISSION_RECEIVE_TEXT")), "party_chat_permission_failed", "Detached PlayFabPartyChatControl.set_permissions_async() reports party_chat_permission_failed")
	await _assert_signal_error(control.set_audio_muted_async(null, true), "party_chat_permission_failed", "Detached PlayFabPartyChatControl.set_audio_muted_async() reports party_chat_permission_failed")
	await _assert_signal_error(control.set_text_muted_async(null, true), "party_chat_permission_failed", "Detached PlayFabPartyChatControl.set_text_muted_async() reports party_chat_permission_failed")
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


func test_party_leave_network_completed_classification_uses_state_result() -> void:
	if pending_unless_playfab_available():
		return

	var playfab = get_playfab()
	reset_playfab_runtime()
	var party = playfab.get_party()
	if party == null:
		return
	if not party.has_method("_test_classify_leave_network_completed"):
		pending("PlayFab Party leave classification test requires debug test hooks.")
		return

	# Party SDK enum values from PartyStateChangeResult in Party.h.
	const PARTY_STATE_CHANGE_SUCCEEDED := 0
	const PARTY_STATE_CHANGE_CANCELED_BY_TITLE := 2
	const PARTY_STATE_CHANGE_LEAVE_NETWORK_CALLED := 14

	var succeeded_with_detail: Dictionary = party._test_classify_leave_network_completed(PARTY_STATE_CHANGE_SUCCEEDED, 1)
	assert_true(bool(succeeded_with_detail.get("ok", false)), "PartyLeaveNetwork treats Succeeded as success even with diagnostic detail")

	var graceful_leave: Dictionary = party._test_classify_leave_network_completed(PARTY_STATE_CHANGE_LEAVE_NETWORK_CALLED, 0)
	assert_true(bool(graceful_leave.get("ok", false)), "PartyLeaveNetwork treats LeaveNetworkCalled as a successful local leave")

	var canceled_zero_detail: Dictionary = party._test_classify_leave_network_completed(PARTY_STATE_CHANGE_CANCELED_BY_TITLE, 0)
	assert_false(bool(canceled_zero_detail.get("ok", true)), "PartyLeaveNetwork does not treat every zero errorDetail as success")
	assert_eq(String(canceled_zero_detail.get("code", "")), "party_resource_not_ready", "Failed PartyLeaveNetwork keeps the expected error code")
	assert_true(String(canceled_zero_detail.get("message", "")).contains("CanceledByTitle"), "Zero-detail PartyLeaveNetwork failure names the SDK state result")
	assert_false(String(canceled_zero_detail.get("message", "")).contains("operation succeeded"), "Zero-detail PartyLeaveNetwork failure does not report operation succeeded")


func _assert_signal_error(async_signal, expected_code: String, name: String) -> void:
	assert_eq(typeof(async_signal), TYPE_SIGNAL, "%s returns completion Signal" % name)
	if typeof(async_signal) != TYPE_SIGNAL:
		return
	assert_playfab_result_error(await await_completion(async_signal), expected_code, name)
