#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif // end if not defined __cplusplus

#include <PartyTypes.h>

#include <playfab/core/PFCore.h>

#pragma push_macro("DEFINE_ENUM_FLAG_OPERATORS")
#ifndef DEFINE_ENUM_FLAG_OPERATORS
#define DEFINE_ENUM_FLAG_OPERATORS(ENUMTYPE) \
inline ENUMTYPE operator | (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE((static_cast<int>(a)) | (static_cast<int>(b))); } \
inline ENUMTYPE &operator |= (ENUMTYPE &a, ENUMTYPE b) { return (a = a | b); } \
inline ENUMTYPE operator & (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE((static_cast<int>(a)) & (static_cast<int>(b))); } \
inline ENUMTYPE &operator &= (ENUMTYPE &a, ENUMTYPE b) { return (a = a & b); } \
inline ENUMTYPE operator ~ (ENUMTYPE a) { return ENUMTYPE(~(static_cast<int>(a))); } \
inline ENUMTYPE operator ^ (ENUMTYPE a, ENUMTYPE b) { return ENUMTYPE((static_cast<int>(a)) ^ (static_cast<int>(b))); } \
inline ENUMTYPE &operator ^= (ENUMTYPE &a, ENUMTYPE b) { return (a = a ^ b); }
#endif

/// <summary>
/// Contains classes and enumerations for the Party library.
/// </summary>
namespace Party
{

class PartyLocalUser;
class PartyEndpoint;
class PartyLocalEndpoint;
class PartyDevice;
class PartyLocalDevice;
class PartyInvitation;
class PartyNetwork;
class PartyChatControl;
class PartyLocalChatControl;
class PartyTextToSpeechProfile;
class PartyAudioManipulationSourceStream;
class PartyAudioManipulationSinkStream;

/// <summary>
/// Convenience type for a constant array of PartyEndpoint pointers.
/// </summary>
/// <seealso cref="PartyEndpoint" />
typedef class PartyEndpoint * const * PartyEndpointArray;

/// <summary>
/// Convenience type for a constant array of PartyEndpoint pointers.
/// </summary>
/// <seealso cref="PartyEndpoint" />
typedef class PartyLocalEndpoint * const * PartyLocalEndpointArray;

/// <summary>
/// Convenience type for a constant array of PartyDevice pointers.
/// </summary>
/// <seealso cref="PartyDevice" />
typedef class PartyDevice * const * PartyDeviceArray;

/// <summary>
/// Convenience type for a constant array of PartyInvitation pointers.
/// </summary>
/// <seealso cref="PartyInvitation" />
typedef class PartyInvitation * const * PartyInvitationArray;

/// <summary>
/// Convenience type for a constant array of PartyNetwork pointers.
/// </summary>
/// <seealso cref="PartyNetwork" />
typedef class PartyNetwork * const * PartyNetworkArray;

/// <summary>
/// Convenience type for a constant array of PartyChatControl pointers.
/// </summary>
/// <seealso cref="PartyChatControl" />
typedef class PartyChatControl * const * PartyChatControlArray;

/// <summary>
/// Convenience type for a constant array of PartyChatControl pointers.
/// </summary>
/// <seealso cref="PartyLocalChatControl" />
typedef class PartyLocalChatControl * const * PartyLocalChatControlArray;

/// <summary>
/// Convenience type for a constant array of PartyTextToSpeechProfile pointers.
/// </summary>
/// <seealso cref="PartyTextToSpeechProfile" />
typedef class PartyTextToSpeechProfile * const * PartyTextToSpeechProfileArray;

/// <summary>
/// Convenience type for a constant array of constant PartyStateChange pointers.
/// </summary>
/// <seealso cref="PartyStateChange" />
typedef const struct PartyStateChange * const * PartyStateChangeArray;

/// <summary>
/// Convenience type for a constant array of PartyLocalUser pointers.
/// </summary>
/// <seealso cref="PartyLocalUser" />
typedef class PartyLocalUser * const * PartyLocalUserArray;

/// <summary>
/// The maximum allowed value for <see cref="PartyNetworkConfiguration::maxDeviceCount" />.
/// </summary>
/// <seealso cref="PartyNetworkConfiguration" />
constexpr uint32_t c_maxNetworkConfigurationMaxDeviceCount = 128;

/// <summary>
/// The default maximum number of direct peer connections that individual player devices attempt to establish.
/// </summary>
/// <remarks>
/// When successfully authenticating an initial local user into a network with a
/// <see cref="PartyNetworkConfiguration::directPeerConnectivityOptions" /> field set to a value other than
/// <see cref="PartyDirectPeerConnectivityOptions::None" />, devices may attempt direct peer connectivity to each other.
/// To avoid excessive resource consumption, the Party library will internally prevent any given device from attempting
/// to establish more direct peer connections than this maximum across all networks in which it's currently
/// participating. This doesn't affect the device's ability to participate in large or multiple networks with additional
/// remote devices, it simply defines the number of opportunities for endpoint messages and chat data between the
/// devices to be transmitted using those direct connections instead of via transparent cloud relay servers.
/// <para>
/// This is the default maximum value for devices that are authenticating a local user representing a
/// `title_player_account` PlayFab entity type. Devices authenticating a local user representing a `game_server` PlayFab
/// entity type do not have a direct peer connection limit when using default settings.
/// </para>
/// <para>
/// To override this value regardless of PlayFab entity type, call <see cref="PartyManager::SetOption" /> passing null
/// for the object parameter, <see cref="PartyOption::LocalDeviceMaxDirectPeerConnections" /> value for the option
/// parameter, and an optional pointer to a uint32_t variable for the value.
/// </para>
/// <para>
/// You can determine whether the local device actually established a direct peer-to-peer connection to a specific
/// remote device in a network by calling <see cref="PartyNetwork::GetDeviceConnectionType()" />.
/// </para>
/// </remarks>
/// <seealso cref="PartyDirectPeerConnectivityOptions" />
/// <seealso cref="PartyNetworkConfiguration" />
/// <seealso cref="PartyNetwork::GetDeviceConnectionType" />
constexpr uint32_t c_maxDirectPeerConnectionsPerDevice = 7;

/// <summary>
/// The maximum allowed value for <see cref="PartyNetworkConfiguration::maxEndpointsPerDeviceCount" />.
/// </summary>
/// <seealso cref="PartyNetworkConfiguration" />
constexpr uint32_t c_maxNetworkConfigurationMaxEndpointsPerDeviceCount = 32;

/// <summary>
/// The maximum number of local users that can be created with <see cref="PartyManager::CreateLocalUser()" /> or
/// <see cref="PartyManager::CreateLocalUserWithEntityType()" />.
/// </summary>
/// <seealso cref="PartyManager::CreateLocalUser" />
/// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
constexpr uint32_t c_maxLocalUsersPerDeviceCount = 8;

/// <summary>
/// Size of an opaque connection information binary blob.
/// </summary>
constexpr uint32_t c_opaqueConnectionInformationByteCount = 300;

/// <summary>
/// Maximum length in characters of a PartyInvitation identifier, not including the null terminator.
/// </summary>
/// <seealso cref="PartyInvitationConfiguration" />
constexpr uint32_t c_maxInvitationIdentifierStringLength = 127;

/// <summary>
/// Maximum number of PlayFab Entity IDs on a single invitation.
/// </summary>
/// <seealso cref="PartyInvitationConfiguration" />
constexpr uint32_t c_maxInvitationEntityIdCount = 1024;

/// <summary>
/// Maximum length in characters of an Entity ID, not including the null terminator
/// </summary>
constexpr uint32_t c_maxEntityIdStringLength = 20;

/// <summary>
/// Length in characters of a network identifier, not including the null terminator.
/// </summary>
constexpr uint32_t c_networkIdentifierStringLength = 36;

/// <summary>
/// Maximum length in characters of a region name, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxRegionNameStringLength = 19;

/// <summary>
/// Maximum length in characters of a serialized network descriptor, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxSerializedNetworkDescriptorStringLength = 448;

/// <summary>
/// Maximum length in characters of an audio device identifier, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxAudioDeviceIdentifierStringLength = 999;

/// <summary>
/// Maximum length in characters of a language code, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxLanguageCodeStringLength = 84;

/// <summary>
/// Maximum length in characters of a chat text message, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxChatTextMessageStringLength = 1023;

/// <summary>
/// Maximum length in characters of a chat transcription message, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxChatTranscriptionMessageStringLength = 1023;

/// <summary>
/// Maximum length in characters of a text-to-speech profile identifier, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxTextToSpeechProfileIdentifierStringLength = 255;

/// <summary>
/// Maximum length in characters of a text-to-speech profile name, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxTextToSpeechProfileNameStringLength = 127;

/// <summary>
/// Maximum length in characters of a text-to-speech synthesis input string, not including the null terminator.
/// </summary>
constexpr uint32_t c_maxTextToSpeechInputStringLength = 1023;

/// <summary>
/// A processor mask that represents all processors available to the title.
/// </summary>
constexpr uint64_t c_anyProcessor = 0xffffffffffffffff;

/// <summary>
/// The lowest allowed priority value for <see cref="PartySendMessageQueuingConfiguration::priority" />.
/// </summary>
constexpr int8_t c_minSendMessageQueuingPriority = -5;

/// <summary>
/// The effective send message queuing configuration priority internally used to transmit chat data.
/// </summary>
/// <remarks>
/// Assign messages a priority value smaller than this to let chat take precedence over them when connection quality is
/// unable to support both. Assign messages a priority value larger than this for caller game data to take precedence.
/// </remarks>
constexpr int8_t c_chatSendMessageQueuingPriority = -3;

/// <summary>
/// The default send message priority when no PartySendMessageQueuingConfiguration structure has been provided.
/// </summary>
constexpr int8_t c_defaultSendMessageQueuingPriority = 0;

/// <summary>
/// The highest allowed priority value for <see cref="PartySendMessageQueuingConfiguration::priority" />.
/// </summary>
constexpr int8_t c_maxSendMessageQueuingPriority = 5;

#pragma pack(push, 8)

/// <summary>
/// The types of state changes that can occur in the Party library.
/// </summary>
enum class PartyStateChangeType : uint32_t
{
    /// <summary>
    /// The list of regions in which the title may create networks has changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRegionsChangedStateChange" /> object for more
    /// information.
    /// </remarks>
    RegionsChanged = 0,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyManager::DestroyLocalUser()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyDestroyLocalUserCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    DestroyLocalUserCompleted = 1,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyManager::CreateNewNetwork()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyCreateNewNetworkCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    CreateNewNetworkCompleted = 2,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyManager::ConnectToNetwork()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyConnectToNetworkCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    ConnectToNetworkCompleted = 3,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::AuthenticateLocalUser()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyAuthenticateLocalUserCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    AuthenticateLocalUserCompleted = 4,

    /// <summary>
    /// The network configuration associated with the PartyNetwork is now available.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyNetworkConfigurationMadeAvailableStateChange" />
    /// object for more information.
    /// </remarks>
    NetworkConfigurationMadeAvailable = 5,

    /// <summary>
    /// The network descriptor associated with the PartyNetwork changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyNetworkDescriptorChangedStateChange" /> object
    /// for more information.
    /// </remarks>
    NetworkDescriptorChanged = 6,

    /// <summary>
    /// A local user has been removed from the network by a previous call to
    /// <see cref="PartyNetwork::RemoveLocalUser()" /> or <see cref="PartyManager::DestroyLocalUser()" />.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyLocalUserRemovedStateChange" /> object for more
    /// information.
    /// </remarks>
    LocalUserRemoved = 7,

    /// <summary>
    /// The operation to remove a local user started by a previous call to
    /// <see cref="PartyNetwork::RemoveLocalUser()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRemoveLocalUserCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    RemoveLocalUserCompleted = 8,

    /// <summary>
    /// A local user was kicked via <see cref="PartyNetwork::KickUser()" />.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyLocalUserKickedStateChange" /> object for more
    /// information.
    /// </remarks>
    /// <nyi />
    LocalUserKicked = 9,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::CreateEndpoint()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyCreateEndpointCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    CreateEndpointCompleted = 10,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::DestroyEndpoint()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyDestroyEndpointCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    DestroyEndpointCompleted = 11,

    /// <summary>
    /// A new endpoint was created.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyEndpointCreatedStateChange" /> object for more
    /// information.
    /// </remarks>
    EndpointCreated = 12,

    /// <summary>
    /// An endpoint was destroyed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyEndpointDestroyedStateChange" /> object for more
    /// information.
    /// </remarks>
    EndpointDestroyed = 13,

    /// <summary>
    /// A new remote device was created.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRemoteDeviceCreatedStateChange" /> object for
    /// more information.
    /// </remarks>
    RemoteDeviceCreated = 14,

    /// <summary>
    /// A remote device was destroyed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRemoteDeviceDestroyedStateChange" /> object for
    /// more information.
    /// </remarks>
    RemoteDeviceDestroyed = 15,

    /// <summary>
    /// A remote device joined a network.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRemoteDeviceJoinedNetworkStateChange" /> object
    /// for more information.
    /// </remarks>
    RemoteDeviceJoinedNetwork = 16,

    /// <summary>
    /// A remote device left a network.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRemoteDeviceLeftNetworkStateChange" /> object
    /// for more information.
    /// </remarks>
    RemoteDeviceLeftNetwork = 17,

    /// <summary>
    /// The properties associated with the device changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyDevicePropertiesChangedStateChange" /> object
    /// for more information.
    /// </remarks>
    /// <nyi />
    DevicePropertiesChanged = 18,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::LeaveNetwork()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyLeaveNetworkCompletedStateChange" /> object for
    /// more information.
    /// </remarks>
    LeaveNetworkCompleted = 19,

    /// <summary>
    /// A network was destroyed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyNetworkDestroyedStateChange" /> object for more
    /// information.
    /// </remarks>
    NetworkDestroyed = 20,

    /// <summary>
    /// A data send was received from an endpoint.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyEndpointMessageReceivedStateChange" /> object
    /// for more information.
    /// </remarks>
    EndpointMessageReceived = 21,

    /// <summary>
    /// The PartyDataBuffer set provided to SendMessage is no longer in use by the library.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyDataBuffersReturnedStateChange" /> object for
    /// more information.
    /// </remarks>
    DataBuffersReturned = 22,

    /// <summary>
    /// The properties associated with an endpoint changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyEndpointPropertiesChangedStateChange" /> object
    /// for more information.
    /// </remarks>
    /// <nyi />
    EndpointPropertiesChanged = 23,

    /// <summary>
    /// An operation started by a call to PartyManager::SynchronizeMessagesBetweenEndpoints has completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a
    /// <see cref="PartySynchronizeMessagesBetweenEndpointsCompletedStateChange" /> object for more information.
    /// </remarks>
    /// <nyi />
    SynchronizeMessagesBetweenEndpointsCompleted = 26,

    /// <summary>
    /// The properties associated with the PartyNetwork changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyNetworkPropertiesChangedStateChange" /> object
    /// for more information.
    /// </remarks>
    /// <nyi />
    NetworkPropertiesChanged = 27,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::KickDevice()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyKickDeviceCompletedStateChange" /> object for
    /// more information.
    /// </remarks>
    /// <nyi />
    KickDeviceCompleted = 28,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::KickUser()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyKickUserCompletedStateChange" /> object for more
    /// information.
    /// </remarks>
    /// <nyi />
    KickUserCompleted = 29,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalDevice::CreateChatControl()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyCreateChatControlCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    CreateChatControlCompleted = 31,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalDevice::DestroyChatControl()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyDestroyChatControlCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    DestroyChatControlCompleted = 32,

    /// <summary>
    /// A new PartyChatControl was created.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyChatControlCreatedStateChange" /> object for
    /// more information.
    /// </remarks>
    ChatControlCreated = 33,

    /// <summary>
    /// A PartyChatControl was destroyed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyChatControlDestroyedStateChange" /> object for
    /// more information.
    /// </remarks>
    ChatControlDestroyed = 34,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetAudioEncoderBitrate()" />
    /// completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a
    /// <see cref="PartySetChatAudioEncoderBitrateCompletedStateChange" /> object for more information.
    /// </remarks>
    /// <nyi />
    SetChatAudioEncoderBitrateCompleted = 35,

    /// <summary>
    /// Chat text has been received.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyChatTextReceivedStateChange" /> object for more
    /// information.
    /// </remarks>
    ChatTextReceived = 36,

    /// <summary>
    /// A voice chat transcription has been received.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyVoiceChatTranscriptionReceivedStateChange" />
    /// object for more information.
    /// </remarks>
    VoiceChatTranscriptionReceived = 37,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetAudioInput()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySetChatAudioInputCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    SetChatAudioInputCompleted = 38,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetAudioOutput()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySetChatAudioOutputCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    SetChatAudioOutputCompleted = 39,

    /// <summary>
    /// The state of an audio input associated with a local chat control changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyLocalChatAudioInputChangedStateChange" /> object
    /// for more information.
    /// </remarks>
    LocalChatAudioInputChanged = 40,

    /// <summary>
    /// The state of an audio output associated with a local chat control changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyLocalChatAudioOutputChangedStateChange" />
    /// object for more information.
    /// </remarks>
    LocalChatAudioOutputChanged = 41,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetTextToSpeechProfile()" />
    /// completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySetTextToSpeechProfileCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    SetTextToSpeechProfileCompleted = 42,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SynthesizeTextToSpeech()" />
    /// completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySynthesizeTextToSpeechCompletedStateChange" />
    /// object for more information. Receiving a state change of this type indicates that the audio has been
    /// synthesized, not that the audio has been transmitted to anyone.
    /// </remarks>
    SynthesizeTextToSpeechCompleted = 43,

    /// <summary>
    /// The properties associated with the chat control changed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyChatControlPropertiesChangedStateChange" />
    /// object for more information.
    /// </remarks>
    /// <nyi />
    ChatControlPropertiesChanged = 45,

    /// <summary>
    /// The chat control joined a network.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyChatControlJoinedNetworkStateChange" /> object
    /// for more information.
    /// </remarks>
    ChatControlJoinedNetwork = 46,

    /// <summary>
    /// The chat control left a network.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyChatControlLeftNetworkStateChange" /> object for
    /// more information.
    /// </remarks>
    ChatControlLeftNetwork = 47,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::ConnectChatControl()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyConnectChatControlCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    ConnectChatControlCompleted = 48,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::DisconnectChatControl()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyDisconnectChatControlCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    DisconnectChatControlCompleted = 49,

    /// <summary>
    /// The operation started by a previous call to
    /// <see cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a
    /// <see cref="PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange" /> object for more information.
    /// </remarks>
    PopulateAvailableTextToSpeechProfilesCompleted = 50,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::CreateInvitation()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyCreateInvitationCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    CreateInvitationCompleted = 51,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyNetwork::RevokeInvitation()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyRevokeInvitationCompletedStateChange" /> object
    /// for more information.
    /// </remarks>
    RevokeInvitationCompleted = 52,

    /// <summary>
    /// A new invitation was created.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyInvitationCreatedStateChange" /> object for more
    /// information.
    /// </remarks>
    InvitationCreated = 53,

    /// <summary>
    /// An invitation was destroyed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartyInvitationDestroyedStateChange" /> object for
    /// more information.
    /// </remarks>
    InvitationDestroyed = 54,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetLanguage()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySetLanguageCompletedStateChange" /> object for
    /// more information.
    /// </remarks>
    /// <nyi />
    SetLanguageCompleted = 55,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetTranscriptionOptions()" />
    /// completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySetTranscriptionOptionsCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    SetTranscriptionOptionsCompleted = 56,

    /// <summary>
    /// The operation started by a previous call to <see cref="PartyLocalChatControl::SetTextChatOptions()" />
    /// completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a <see cref="PartySetTextChatOptionsCompletedStateChange" />
    /// object for more information.
    /// </remarks>
    SetTextChatOptionsCompleted = 57,

    /// <summary>
    /// The operation started by a previous call to
    /// <see cref="PartyChatControl::ConfigureAudioManipulationVoiceStream()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a
    /// <see cref="PartyConfigureAudioManipulationVoiceStreamCompletedStateChange" /> object for more information.
    /// </remarks>
    ConfigureAudioManipulationVoiceStreamCompleted = 58,

    /// <summary>
    /// The operation started by a previous call to
    /// <see cref="PartyLocalChatControl::ConfigureAudioManipulationCaptureStream()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a
    /// <see cref="PartyConfigureAudioManipulationCaptureStreamCompletedStateChange" /> object for more information.
    /// </remarks>
    ConfigureAudioManipulationCaptureStreamCompleted = 59,

    /// <summary>
    /// The operation started by a previous call to
    /// <see cref="PartyLocalChatControl::ConfigureAudioManipulationRenderStream()" /> completed.
    /// </summary>
    /// <remarks>
    /// The PartyStateChange object should be cast to a
    /// <see cref="PartyConfigureAudioManipulationRenderStreamCompletedStateChange" /> object for more information.
    /// </remarks>
    ConfigureAudioManipulationRenderStreamCompleted = 60,
};

/// <summary>
/// Results for Party library operations that generate state changes.
/// </summary>
enum class PartyStateChangeResult
{
    /// <summary>
    /// The operation succeeded.
    /// </summary>
    Succeeded = 0,

    /// <summary>
    /// An unknown error occurred.
    /// </summary>
    UnknownError = 1,

    /// <summary>
    /// Another title operation canceled this operation.
    /// </summary>
    CanceledByTitle = 2,

    /// <summary>
    /// The local device has internet connectivity issues which caused the operation to fail.
    /// </summary>
    InternetConnectivityError = 3,

    /// <summary>
    /// The operation failed because of an unexpected error in the Party service.
    /// </summary>
    PartyServiceError = 4,

    /// <summary>
    /// There are no available transparent cloud relay servers in the regions specified by the call to
    /// <see cref="PartyManager::CreateNewNetwork()" />.
    /// </summary>
    NoServersAvailable = 5,

    /// <summary>
    /// The PartyLocalUser specified in the call to <see cref="PartyManager::CreateNewNetwork()" /> or
    /// <see cref="PartyNetwork::AuthenticateLocalUser()" /> is not authorized to complete the operation.
    /// </summary>
    UserNotAuthorized = 6,

    /// <summary>
    /// The PartyLocalUser specified in the call to <see cref="PartyManager::CreateNewNetwork()" /> has created too many
    /// networks and cannot create new networks at this time.
    /// </summary>
    UserCreateNetworkThrottled = 7,

    /// <summary>
    /// The title has not been enabled to use PlayFab Party. PlayFab Party must be enabled in the PlayFab Game Manager.
    /// </summary>
    TitleNotEnabledForParty = 8,

    /// <summary>
    /// The network rejected this operation because it would violate a limit in the network's configuration. See
    /// <see cref="PartyNetworkConfiguration" />.
    /// </summary>
    NetworkLimitReached = 10,

    /// <summary>
    /// The network no longer exists.
    /// </summary>
    NetworkNoLongerExists = 11,

    /// <summary>
    /// The network is not currently allowing new devices or users to join.
    /// </summary>
    /// <remarks>
    /// Note that there are currently no operations which can return this result.
    /// </remarks>
    /// <nyi />
    NetworkNotJoinable = 12,

    /// <summary>
    /// The operation failed because this version of the Party library was incompatible with either the Party service or
    /// the Party network.
    /// </summary>
    VersionMismatch = 13,

    /// <summary>
    /// The network was gracefully exited by the local device.
    /// </summary>
    LeaveNetworkCalled = 14,

    /// <summary>
    /// The operation failed because the Party library was unable to bind to the socket specified in the
    /// <see cref="PartyOption::LocalUdpSocketBindAddress" /> option.
    /// </summary>
    FailedToBindToLocalUdpSocket = 15,
};

/// <summary>
/// Reasons for removing a local user from a network.
/// </summary>
/// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
/// <seealso cref="PartyNetwork::RemoveLocalUser" />
/// <seealso cref="PartyManager::DestroyLocalUser" />
enum class PartyLocalUserRemovedReason
{
    /// <summary>
    /// The local user is being removed because <see cref="PartyNetwork::AuthenticateLocalUser()" /> asynchronously
    /// failed.
    /// </summary>
    AuthenticationFailed = 0,

    /// <summary>
    /// The local user is being removed because the title called <see cref="PartyNetwork::RemoveLocalUser()" />.
    /// </summary>
    RemoveLocalUser = 1,

    /// <summary>
    /// The local user is being removed because the title called <see cref="PartyManager::DestroyLocalUser()" />.
    /// </summary>
    DestroyLocalUser = 2,

    /// <summary>
    /// The local user is being removed because the network is being destroyed.
    /// </summary>
    DestroyNetwork = 3,
};

/// <summary>
/// Reasons an object might be destroyed.
/// </summary>
enum class PartyDestroyedReason
{
    /// <summary>
    /// The invitation, endpoint, device, network, or chat control is being destroyed gracefully by request.
    /// </summary>
    Requested = 0,

    /// <summary>
    /// The invitation, endpoint, device, network, or chat control is being destroyed because connectivity to the
    /// transparent cloud relay server was lost.
    /// </summary>
    Disconnected = 1,

    /// <summary>
    /// The device or user that the invitation, endpoint, or chat control depended upon was kicked.
    /// </summary>
    Kicked = 2,

    /// <summary>
    /// No local users on the local device were authenticated on the network, so the local device lost visibility to the
    /// remotely managed invitation, endpoint, device, or chat control.
    /// </summary>
    /// <remarks>
    /// Even though the local object referring to the remotely managed object has been destroyed, the remotely managed
    /// object may still exist in the network.
    /// </remarks>
    DeviceLostAuthentication = 3,

    /// <summary>
    /// The invitation or endpoint creation failed asynchronously, so the local object has been destroyed.
    /// </summary>
    CreationFailed = 4,
};

/// <summary>
/// The level of filtering that applies to incoming text chat when text moderation is enabled with
/// <see cref="PartyLocalChatControl::SetTextChatOptions" />.
/// </summary>
/// <see cref="PartyOption::TextChatFilterLevel" />
/// <seealso cref="PartyManager::SetOption" />
/// <seealso cref="PartyLocalChatControl::SetTextChatOptions" />
enum class PartyTextChatFilterLevel : uint32_t
{
    /// <summary>
    /// Text chat is filtered at a family-friendly level.
    /// </summary>
    /// <remarks>
    /// This value is the default when <see cref="PartyOption::TextChatFilterLevel" /> hasn't been configured.
    /// </remarks>
    FamilyFriendly = 0,

    /// <summary>
    /// Text chat is filtered at a medium level.
    /// </summary>
    Medium = 1,

    /// <summary>
    /// Text chat is filtered at a mature level.
    /// </summary>
    Mature = 2,
};

/// <summary>
/// Additional options that can be set to fine-tune Party library functionality.
/// </summary>
/// <seealso cref="PartyManager::SetOption" />
/// <seealso cref="PartyManager::GetOption" />
/// <seealso cref="PartyLocalUdpSocketBindAddressConfiguration" />
/// <seealso cref="PartyNetworkConfiguration" />
/// <seealso cref="PartyDirectPeerConnectivityOptions" />
/// <seealso cref="PartyRegionUpdateConfiguration" />
/// <seealso cref="PartyRegionQualityMeasurementConfiguration" />
enum class PartyOption : uint32_t
{
    /// <summary>
    /// An option used to configure how the Party library binds to a UDP socket.
    /// </summary>
    /// <remarks>
    /// To override this option, call <see cref="PartyManager::SetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and an optional pointer to a
    /// <see cref="PartyLocalUdpSocketBindAddressConfiguration" /> structure for the value. If a null
    /// PartyLocalUdpSocketBindAddressConfiguration pointer is provided, the default configuration is restored.
    /// <para>
    /// To query this option, call <see cref="PartyManager::GetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and a pointer to an output PartyLocalUdpSocketBindAddressConfiguration structure
    /// for the value.
    /// </para>
    /// <para>
    /// It's safe and recommended to override or query for this option prior to initializing the Party library.
    /// Overriding the local UDP socket bind address configuration will take effect the next time the Party library is
    /// initialized. It doesn't modify any Party library socket binding that has already been initialized. Similarly,
    /// querying retrieves the configuration that will be used with the next Party library initialization, not the value
    /// actively in use if already initialized.
    /// </para>
    /// </remarks>
    LocalUdpSocketBindAddress = 0,

    /// <summary>
    /// An option for constraining the permitted direct peer connectivity between the local device and other devices in
    /// networks that allow attempting such connections.
    /// </summary>
    /// <remarks>
    /// When successfully authenticating an initial local user into a network with a
    /// <see cref="PartyNetworkConfiguration::directPeerConnectivityOptions" /> field set to a value other than
    /// <see cref="PartyDirectPeerConnectivityOptions::None" />, or when a remote device authenticates into such a
    /// network, direct peer connectivity between this local device and the remote device(s) in the network may be
    /// attempted. The flags configured by this option can be used to further constrain the specific types of remote
    /// devices that are permitted to involve the local device in these attempts. All flags are evaluated using a
    /// bitwise AND operation. That is, a particular flag is actually only in effect for a given network's pair of
    /// devices if it's enabled in three places: the network's <see cref="PartyNetworkConfiguration" /> structure, and
    /// <em>both</em> devices' respective local mask options. Even if the Party network permits direct peer connectivity
    /// of the relevant form, either device can unilaterally opt out of the IP address disclosure and direct connection
    /// attempts between them by not enabling the flag when overriding this option.
    /// <para>
    /// Direct peer connectivity is supported for the Windows, Microsoft Game Core, PlayStationÂ®, Switch, Linux, and
    /// macOS versions of the library. On those platforms, the default value for the local device mask when not yet set
    /// is <c>PartyDirectPeerConnectivityOptions::AnyPlatformType |
    /// PartyDirectPeerConnectivityOptions::AnyEntityLoginProvider |
    /// PartyDirectPeerConnectivityOptions::OnlyServers</c>. This means that such devices don't restrict any direct peer
    /// connectivity permitted by a network's <see cref="PartyNetworkConfiguration" /> until explicitly overridden to be
    /// less permissive. For all other versions of the library, the default value for the local device mask is
    /// <c>PartyDirectPeerConnectivityOptions::None</c> and cannot be changed; attempting to change it via
    /// <see cref="PartyManager::SetOption" /> will fail. This means that such devices will never attempt direct peer
    /// connectivity.
    /// </para>
    /// <para>
    /// To override this option, call <see cref="PartyManager::SetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and an optional pointer to a <see cref="PartyDirectPeerConnectivityOptions" />
    /// variable containing all desired option flags. If a null PartyDirectPeerConnectivityOptions pointer is provided,
    /// the default options are restored.
    /// </para>
    /// <para>
    /// To query this option, call <see cref="PartyManager::GetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and a pointer to a <see cref="PartyDirectPeerConnectivityOptions" /> variable
    /// into which the currently configured option flags should be written.
    /// </para>
    /// <para>
    /// It's recommended to override or query this option when not connected to any networks. The configured value takes
    /// effect the next time this device authenticates an initial user into a new network. It doesn't alter the settings
    /// already being used to participate in any existing networks. Similarly, querying retrieves the currently
    /// configured value that will be used with future networks, and not the value(s) used with existing networks.
    /// </para>
    /// <para>
    /// It's also safe to override or query for this option prior to initializing the Party library.
    /// </para>
    /// </remarks>
    LocalDeviceDirectPeerConnectivityOptionsMask = 1,

    /// <summary>
    /// An option for fine-tuning the level at which chat text will be filtered.
    /// </summary>
    /// <remarks>
    /// This feature only applies to incoming chat text detected as English. The filter level cannot be changed for
    /// other languages.
    /// <para>
    /// The filter level will apply to incoming chat text for all local chat controls on the client. Filtering must be
    /// enabled with <see cref="PartyLocalChatControl::SetTextChatOptions" /> for at least one local chat control for
    /// this to have an effect on the chat text.
    /// </para>
    /// <para>
    /// To override this option, call <see cref="PartyManager::SetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and an optional pointer to a <see cref="PartyTextChatFilterLevel" /> variable
    /// containing the desired filter level. If a null PartyTextChatFilterLevel pointer is provided, the default level
    /// is restored.
    /// </para>
    /// <para>
    /// To query this option, call <see cref="PartyManager::GetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and a pointer to a <see cref="PartyTextChatFilterLevel" /> variable into which
    /// the currently configured filter level should be written.
    /// </para>
    /// <para>
    /// It's safe to override or query for this option at any time, including prior to initializing the Party library.
    /// Setting a new value will take effect immediately.
    /// </para>
    /// </remarks>
    TextChatFilterLevel = 2,

    /// <summary>
    /// An option for configuring the max number of direct peer connections this device attempts to establish to other
    /// devices.
    /// </summary>
    /// <remarks>
    /// When successfully authenticating an initial local user into a network with a
    /// <see cref="PartyNetworkConfiguration::directPeerConnectivityOptions" /> field set to a value other than
    /// <see cref="PartyDirectPeerConnectivityOptions::None" />, devices may attempt direct peer connectivity to each
    /// other. To avoid excessive resource consumption, the Party library will internally prevent any given device from
    /// attempting to establish more direct peer connections than this maximum across all networks in which it's
    /// currently participating. This doesn't affect the device's ability to participate in large or multiple networks
    /// with additional remote devices, it simply defines the number of opportunities for endpoint messages and chat
    /// data between the devices to be transmitted using those direct connections instead of via transparent cloud relay
    /// servers.
    /// <para>
    /// You can determine whether the local device actually established a direct peer-to-peer connection to a specific
    /// remote device in a network by calling <see cref="PartyNetwork::GetDeviceConnectionType()" />.
    /// </para>
    /// <para>
    /// The default value for this option is <c>c_maxDirectPeerConnectionsPerDevice</c>, though when using the default
    /// settings, the value only applies when authenticating a local user with a PlayFab entity type of
    /// `title_player_account`. When instead authenticating a local user with a PlayFab entity type of `game_server`
    /// with the default option configuration, there is no maximum number of direct peer connections.
    /// </para>
    /// <para>
    /// To override this value regardless of PlayFab entity type, call <see cref="PartyManager::SetOption" /> passing
    /// null for the object parameter, this value for the option parameter, and an optional pointer to a uint32_t
    /// variable for the value. If a null uint32_t pointer is provided, the default value and behavior is restored.
    /// </para>
    /// <para>
    /// To query this option, call <see cref="PartyManager::GetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and a pointer to a uint32_t variable into which the currently configured value
    /// should be written.
    /// </para>
    /// <para>
    /// It's safe to override or query for this option at any time, including prior to initializing the Party library.
    /// Setting a new value will take effect for all networks joined after the value is changed.
    /// </para>
    /// </remarks>
    LocalDeviceMaxDirectPeerConnections = 3,

    /// <summary>
    /// An option used to configure when the Party library performs automatic region discovery and connection quality
    /// measurement updates.
    /// </summary>
    /// <remarks>
    /// To override this option, call <see cref="PartyManager::SetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and an optional pointer to a <see cref="PartyRegionUpdateConfiguration" />
    /// structure for the value. If a null PartyRegionUpdateConfiguration pointer is provided, the default configuration
    /// is restored.
    /// <para>
    /// To query this option, call <see cref="PartyManager::GetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and a pointer to an output <see cref="PartyRegionUpdateConfiguration" />
    /// structure for the value.
    /// </para>
    /// <para>
    /// It's safe to override or query for this option at any time, including prior to initializing the Party library.
    /// Overriding the region update configuration will take effect immediately.
    /// </para>
    /// </remarks>
    RegionUpdateConfiguration = 4,

    /// <summary>
    /// An option used for fine-grained control over Party library region quality measurement behavior.
    /// </summary>
    /// <remarks>
    /// To override this option, call <see cref="PartyManager::SetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and an optional pointer to a
    /// <see cref="PartyRegionQualityMeasurementConfiguration" /> structure for the value. If a null
    /// PartyRegionQualityMeasurementConfiguration pointer is provided, the default configuration is restored.
    /// <para>
    /// To query this option, call <see cref="PartyManager::GetOption" /> passing null for the object parameter, this
    /// value for the option parameter, and a pointer to an output
    /// <see cref="PartyRegionQualityMeasurementConfiguration" /> structure for the value.
    /// </para>
    /// <para>
    /// It's safe to override or query for this option at any time, including prior to initializing the Party library.
    /// Overriding the region quality measurement configuration will take effect with the next region update.
    /// </para>
    /// </remarks>
    RegionQualityMeasurementConfiguration = 5,
};

/// <summary>
/// High-level categorization of internal processing tasks.
/// </summary>
/// <remarks>
/// When used with <see cref="PartyManager::GetWorkMode()" /> and <see cref="PartyManager::SetWorkMode()" />, allows the
/// title to read and write (respectively) the work mode for the associated internal processing task. Title interaction
/// with the internal processing task differs greatly depending on the currently configured work mode.
/// <para>
/// When the PartyThreadId::Audio work mode is set to <see cref="PartyWorkMode::Automatic" />, the Party library
/// performs the task using internally managed, high priority, frequently running threads with real-time requirements.
/// On Windows and Xbox consoles, these audio threads interact directly with XAudio2 every 40 milliseconds. Party
/// library instances of XAudio2 are initialized with a processor affinity that corresponds to the processor affinity
/// configured for PartyThreadId::Audio via <see cref="PartyManager::SetThreadAffinityMask()" />. If no processor
/// affinity is specified for the audio thread type, the instance(s) of XAudio2 is initialized with a processor affinity
/// of XAUDIO2_DEFAULT_PROCESSOR.
/// </para>
/// <para>
/// When the PartyThreadId::Networking work mode is set to <see cref="PartyWorkMode::Automatic" />, networking threads
/// are created and managed internally. These threads are driven from both network I/O and polling mechanisms, waking
/// every 16 milliseconds to 100 milliseconds or whenever network traffic is received. The processor affinity for these
/// PartyThreadId::Networking threads is optionally configured via <see cref="PartyManager::SetThreadAffinityMask()" />.
/// </para>
/// <para>
/// For any PartyThreadId type work mode set to <see cref="PartyWorkMode::Manual" />, the Party library doesn't create
/// internal worker threads and instead it's the title's responsibility to perform the required processing via periodic
/// calls to <see cref="PartyManager::DoWork()" />. The periodicity of those title calls should match or exceed the
/// minimum polling frequencies used by the internal threads when the work mode is
/// <see cref="PartyWorkMode::Automatic" />.
/// </para>
/// </remarks>
/// <seealso cref="PartyManager::GetThreadAffinityMask" />
/// <seealso cref="PartyManager::SetThreadAffinityMask" />
/// <seealso cref="PartyManager::SetWorkMode" />
/// <seealso cref="PartyManager::GetWorkMode" />
/// <seealso cref="PartyManager::DoWork" />
enum class PartyThreadId
{
    /// <summary>
    /// Represents the internal processing task related to audio.
    /// </summary>
    Audio = 0,

    /// <summary>
    /// Represents the internal processing task related to networking.
    /// </summary>
    Networking = 1,
};

/// <summary>
/// Configuration modes representing how the Party library will manage an internal processing task.
/// </summary>
/// <remarks>
/// For an overview of the processing tasks and their frequencies, see <see cref="PartyThreadId" />.
/// </remarks>
/// <seealso cref="PartyManager::SetWorkMode" />
/// <seealso cref="PartyManager::GetWorkMode" />
/// <seealso cref="PartyManager::DoWork" />
enum class PartyWorkMode
{
    /// <summary>
    /// The Party library will create internal threads to automatically handle the associated processing task.
    /// </summary>
    /// <remarks>
    /// This is the default, recommended option which encodes the Party library's best practices for processing its
    /// internal task.
    /// </remarks>
    Automatic = 0,

    /// <summary>
    /// The Party library will not create internal threads to handle the associated processing task, instead relying on
    /// the title to perform the task through calls to <see cref="PartyManager::DoWork()" />.
    /// </summary>
    Manual = 1,
};

/// <summary>
/// Options for controlling how a message is sent.
/// </summary>
/// <seealso cref="PartySendMessageQueuingConfiguration" />
/// <seealso cref="PartyDataBuffersReturnedStateChange" />
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
/// <seealso cref="PartyLocalEndpoint::FlushMessages" />
enum class PartySendMessageOptions : int32_t
{
    /// <summary>
    /// Use the default PartySendMessageOptions.
    /// </summary>
    /// <remarks>
    /// The default options are <em>BestEffortDelivery</em>, <em>NonsequentialDelivery</em>, <em>CopyDataBuffers</em>,
    /// <em>CoalesceOpportunistically</em>, and <em>RequireTimelyAcknowledgement</em>.
    /// </remarks>
    Default = 0x0000,

    /// <summary>
    /// Ensure the message is delivered to all targets, retransmitting if necessary.
    /// </summary>
    /// <remarks>
    /// This option flag guarantees the message will arrive at each target endpoint, regardless of environmental packet
    /// loss, unless the target endpoint is destroyed. Packet transmissions will be retried as needed.
    /// <para>
    /// This option flag works well when sending important state information that must always reach the destination or
    /// else the target should be removed from the network. Use it with message content that does not have redundancy or
    /// the ability to be interpolated/extrapolated if it's lost, and is worth the potential increased bandwidth usage
    /// should packet retransmissions be needed.
    /// </para>
    /// <para>
    /// Guaranteeing delivery by itself doesn't imply a guarantee of a particular delivery order; use the
    /// <em>SequentialDelivery</em> option flag to enforce ordering.
    /// </para>
    /// </remarks>
    GuaranteedDelivery = 0x0001,

    /// <summary>
    /// Transmit the message best-effort and ignore any packet loss.
    /// </summary>
    /// <remarks>
    /// This option flag requests only a single attempt to transmit the message. If environmental packet loss occurs,
    /// the transmission will not be retried and the application should be prepared to handle the message's absence.
    /// <para>
    /// This option flag works well for information that is constantly being updated and doesn't require every update to
    /// arrive. Use it with message content that already has redundancy or the ability to be interpolated/extrapolated
    /// if it's lost and isn't worth extra bandwidth to retransmit.
    /// </para>
    /// <para>
    /// This is the default if the <em>GuaranteedDelivery</em> option flag is not specified.
    /// </para>
    /// </remarks>
    BestEffortDelivery = 0x0000,

    /// <summary>
    /// Deliver the message in order relative to other messages sent from this local endpoint to the target endpoint
    /// that were also sent sequentially.
    /// </summary>
    /// <remarks>
    /// <em>SequentialDelivery</em> doesn't provide any guarantees about the ordering of messages sent from different
    /// local endpoints and/or to different target endpoints. Each endpoint pairing should be considered a separate
    /// sequence space.
    /// <para>
    /// No guarantees are made about the order of sequential messages in relation to non-sequential messages.
    /// </para>
    /// <para>
    /// This option flag works well for state information that should reach the destination in a particular sequence,
    /// even if that means slightly less network efficiency and possibly waiting a bit longer to receive it if there is
    /// packet loss or reordering by the environment.
    /// </para>
    /// <para>
    /// Using <em>SequentialDelivery</em> with <em>GuaranteedDelivery</em> may result in messages being queued on the
    /// target endpoint while waiting for previously sent sequential messages to arrive. This may result in a perceived
    /// increase in latency when experiencing environmental packet loss or reordering, but the target endpoint will
    /// always see every message, in the same order in which they were sent.
    /// </para>
    /// <para>
    /// Using <em>SequentialDelivery</em> with <em>BestEffortDelivery</em> may result in messages being dropped if one
    /// arrives at the target endpoint out of order and a later sequential message had already been delivered. The
    /// target endpoint will always see the sequence moving forward, but there may be gaps in that sequence. An older
    /// message will never be delivered after a newer one.
    /// </para>
    /// </remarks>
    SequentialDelivery = 0x0002,

    /// <summary>
    /// Deliver the message to targets as soon as it arrives.
    /// </summary>
    /// <remarks>
    /// Messages sent with the non-sequential delivery option do not provide any guarantees about the order in which
    /// they are delivered with respect to any other messages, sequential or non-sequential. They will be delivered to
    /// the targets as soon as they arrive, which may not be the same order in which they were sent if there is packet
    /// loss or reordering by the environment.
    /// <para>
    /// This option flag works well for messages that are safe to process in any order or have their own inherent
    /// ordering information already, and where you'd like maximum network efficiency and lowest perceived latency.
    /// </para>
    /// <para>
    /// This is the default if the <em>SequentialDelivery</em> option flag is not specified.
    /// </para>
    /// </remarks>
    NonsequentialDelivery = 0x0000,

    /// <summary>
    /// Instructs the Party library to make a copy of the supplied data buffers for subsequent transmission.
    /// </summary>
    /// <remarks>
    /// The memory contents in the provided PartyDataBuffer structures will be copied so the caller does not need to
    /// keep the buffers around after <see cref="PartyLocalEndpoint::SendMessage()" /> returns. This is more convenient
    /// but slightly less efficient than using the <em>DontCopyDataBuffers</em> option flag.
    /// <para>
    /// This is the default if the <em>DontCopyDataBuffers</em> option flag is not specified.
    /// </para>
    /// </remarks>
    CopyDataBuffers = 0x0000,

    /// <summary>
    /// Informs the Party library to use the supplied data buffers directly and that the caller will keep the memory
    /// valid until the library no longer needs them.
    /// </summary>
    /// <remarks>
    /// The memory referenced by the provided PartyDataBuffer structures will not be copied but instead ownership will
    /// temporarily be transferred to the Party library so the memory can be accessed directly without additional
    /// copying overhead during the transmission process. It's the caller's responsibility to ensure the memory buffers
    /// remain valid and unmodified until the library no longer needs them and ownership is transferred back via a
    /// <see cref="PartyDataBuffersReturnedStateChange" />. This is more efficient but can be less convenient than using
    /// the <em>CopyDataBuffers</em> option.
    /// <para>
    /// The PartyDataBuffer structures themselves do not need to remain valid after the
    /// <see cref="PartyLocalEndpoint::SendMessage()" /> call returns, only the memory they reference.
    /// </para>
    /// </remarks>
    DontCopyDataBuffers = 0x0004,

    /// <summary>
    /// Specifies that this message should be coalesced with any other queued messages but should not delay transmission
    /// if there aren't any waiting.
    /// </summary>
    /// <remarks>
    /// Coalescing multiple messages together in a single packet allows maximizing bandwidth efficiency (reducing per-
    /// packet overhead) at the potential expense of perceived latency for a message if you delay its transmission in
    /// order to coalesce. Sending with this flag causes the Party library to coalesce the message if there are other
    /// queued messages available, but not to wait for more messages if none exist and this message can be transmitted
    /// immediately.
    /// <para>
    /// Use this flag if you typically batch your network updates into single, periodic messages that are not likely to
    /// be queued around the same time as other messages and would not gain bandwidth efficiency if delayed.
    /// </para>
    /// <para>
    /// This flag does not guarantee that the message begins transmitting immediately. If the connection quality or
    /// receiver responsiveness do not appear to currently support sending additional data yet, then the message may be
    /// queued to await the next transmission opportunity.
    /// </para>
    /// <para>
    /// This is the default if the <em>AlwaysCoalesceUntilFlushed</em> option flag is not specified.
    /// </para>
    /// </remarks>
    CoalesceOpportunistically = 0x0000,

    /// <summary>
    /// Specifies that this message should always try to be coalesced with other messages and expect a
    /// <see cref="PartyLocalEndpoint::FlushMessages()" /> call to begin transmitting.
    /// </summary>
    /// <remarks>
    /// Coalescing multiple messages together in a single packet allows maximizing bandwidth efficiency (reducing per-
    /// packet overhead) at the potential expense of perceived latency for a message if you delay its transmission in
    /// order to coalesce. Sending with this flag causes the Party library to always prefer coalescing the message and
    /// delay transmission until PartyLocalEndpoint::FlushMessages() is called.
    /// <para>
    /// Consider using this flag if you typically send a lot of small messages to the same targets in the same update
    /// loop and want to explicitly inform the Party library when the full update loop iteration is complete.
    /// </para>
    /// <para>
    /// Even with this flag there are scenarios where the message might begin transmitting without requiring an explicit
    /// PartyLocalEndpoint::FlushMessages() call. This can occur when there are other queued messages that are already
    /// being transmitted for other reasons and there is room in the packet to include this message. Similarly, when
    /// enough message data bytes exist to send a full packet and no more coalescence is possible, the packet will be
    /// sent. Calling PartyLocalEndpoint::FlushMessages() when all messages have already begun transmitting is benign.
    /// </para>
    /// </remarks>
    AlwaysCoalesceUntilFlushed = 0x0008,

    /// <summary>
    /// Indicates that this message should be acknowledged (if needed) in a timely manner.
    /// </summary>
    /// <remarks>
    /// Receivers acknowledge reception of messages to inform the sender of delivery success or failure so it can do
    /// things such as retry dropped packets that contained messages sent with the <em>GuaranteedDelivery</em> option
    /// flag. Acknowledgements are often piggybacked on packets as part of typical bi-directional communication, but if
    /// no packets are flowing in the return direction, this flag instructs the target endpoints to only wait a small,
    /// internally managed timeout for piggybacking opportunities before forcing acknowledgement packets to inform the
    /// sender. That dedicated packet consumes some extra overhead but ensures the sender gets timely status for issuing
    /// any necessary retries.
    /// <para>
    /// Using this flag for most guaranteed delivery messages is recommended. It works well with messages that are
    /// latency sensitive. It also works well when bi-directional guaranteed delivery send patterns are infrequent or
    /// unpredictable.
    /// </para>
    /// <para>
    /// This is the default if the <em>AllowLazyAcknowledgement</em> option flag is not specified.
    /// </para>
    /// <para>
    /// This flag is ignored if the <em>GuaranteedDelivery</em> option flag is not specified.
    /// </para>
    /// </remarks>
    RequireTimelyAcknowledgement = 0x0000,

    /// <summary>
    /// Indicates that this message can be acknowledged when convenient rather than urgently.
    /// </summary>
    /// <remarks>
    /// Receivers acknowledge reception of messages to inform the sender of delivery success or failure so it can do
    /// things such as retry dropped packets that contained messages sent with the <em>GuaranteedDelivery</em> option
    /// flag. Acknowledgements are often piggybacked on packets as part of typical bi-directional communication, but if
    /// no packets are flowing in the return direction, this flag instructs the target endpoints to wait for
    /// piggybacking opportunities without forcing acknowledgement packets to inform the sender. This delays the sender
    /// finding out status for any necessary retries, but avoids extra overhead consumed by a dedicated packet.
    /// <para>
    /// Consider using this flag for guaranteed delivery messages that are "fire-and-forget" and are not latency
    /// sensitive. It may also reduce overhead when bi-directional send patterns are frequent.
    /// </para>
    /// <para>
    /// This flag is ignored if the <em>GuaranteedDelivery</em> option flag is not also specified.
    /// </para>
    /// </remarks>
    AllowLazyAcknowledgement = 0x0010,
};

DEFINE_ENUM_FLAG_OPERATORS(PartySendMessageOptions);

/// <summary>
/// Flags describing how a message was received.
/// </summary>
/// <seealso cref="PartyEndpointMessageReceivedStateChange" />
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
enum class PartyMessageReceivedOptions : int32_t
{
    /// <summary>
    /// There is no additional message reception information.
    /// </summary>
    None = 0x0000,

    /// <summary>
    /// The message's delivery was guaranteed.
    /// </summary>
    /// <remarks>
    /// This flag's presence or absence represents the actual delivery mode. The Party library may occasionally adjust
    /// messages internally to use guaranteed delivery even if
    /// <see cref="PartySendMessageOptions::GuaranteedDelivery" /> was not originally specified to
    /// <see cref="PartyLocalEndpoint::SendMessage()" />. It will never disable guaranteed delivery if it had originally
    /// been requested, however.
    /// </remarks>
    GuaranteedDelivery = 0x0001,

    /// <summary>
    /// The message was delivered using sequencing information.
    /// </summary>
    /// <remarks>
    /// This flag's presence or absence represents the actual delivery mode. The Party library may occasionally adjust
    /// messages internally to sequential delivery even if <see cref="PartySendMessageOptions::SequentialDelivery" />
    /// was not originally specified to <see cref="PartyLocalEndpoint::SendMessage()" />. It will never disable
    /// sequential delivery if it had originally been requested, however.
    /// </remarks>
    SequentialDelivery = 0x0002,

    /// <summary>
    /// The message was larger than could fit in available space in a single packet and needed to be split across
    /// multiple packets for delivery.
    /// </summary>
    /// <remarks>
    /// The Party library automatically fragments and reassembles large messages that exceed the maximum size supported
    /// by the environment so that callers are not required to manage this. However, there is a small amount of overhead
    /// associated with fragmentation. Callers that are able to send smaller messages or otherwise naturally break up
    /// large state payloads efficiently themselves may wish to do so.
    /// </remarks>
    RequiredFragmentation = 0x0004,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyMessageReceivedOptions);

/// <summary>
/// Type of expression <see cref="PartyLocalEndpoint::CancelMessages()" /> should evaluate in order to match queued
/// messages.
/// </summary>
/// <seealso cref="PartySendMessageQueuingConfiguration" />
/// <seealso cref="PartyLocalEndpoint::CancelMessages" />
/// <nyi />
enum class PartyCancelMessagesFilterExpression
{
    /// <summary>
    /// Cancels all messages regardless of identity.
    /// </summary>
    /// <remarks>
    /// The <see cref="PartyLocalEndpoint::CancelMessages()" /> <em>messageIdentityFilterMask</em> and
    /// <em>filteredMessageIdentitiesToMatch</em> parameters and messages' PartySendMessageQueuingConfiguration
    /// structure <em>identityForCancelFilters</em> field are ignored for this expression type.
    /// </remarks>
    None = 0,

    /// <summary>
    /// Cancels messages where the filter expression <c>(<em>identityForCancelFilters</em> &
    /// <em>messageIdentityFilterMask</em>) == <em>filteredMessageIdentitiesToMatch</em></c> evaluates to true.
    /// </summary>
    /// <remarks>
    /// To filter to messages that have PartySendMessageQueuingConfiguration structure <em>identityForCancelFilters</em>
    /// field values with a particular bit (or bits) set a particular way, specify a
    /// <see cref="PartyLocalEndpoint::CancelMessages()" /> <em>messageIdentityFilterMask</em> parameter with the bits
    /// to check enabled and the desired way those bits must be set for a match in the
    /// <em>filteredMessageIdentitiesToMatch</em> parameter value.
    /// <para>
    /// For example, to require matching specific <em>identityForCancelFilters</em> field values exactly, specify a
    /// <em>messageIdentityFilterMask</em> parameter with all bits set (the maximum uint32_t value) and the required
    /// value in the <em>filteredMessageIdentitiesToMatch</em> parameter.
    /// </para>
    /// <para>
    /// Alternatively, to filter to messages that have <em>identityForCancelFilters</em> values where no bits from a
    /// particular group are enabled, specify a <em>messageIdentityFilterMask</em> parameter with the relevant bits to
    /// check and a value of zero for the <em>filteredMessageIdentitiesToMatch</em> parameter.
    /// </para>
    /// </remarks>
    IdentityAndMaskEqualsMatchValue = 1,

    /// <summary>
    /// Cancels messages where the filter expression <c>(<em>identityForCancelFilters</em> &
    /// <em>messageIdentityFilterMask</em>) != <em>filteredMessageIdentitiesToMatch</em></c> evaluates to true.
    /// </summary>
    /// <remarks>
    /// To filter to messages that do not have PartySendMessageQueuingConfiguration structure
    /// <em>identityForCancelFilters</em> field values with a particular bit (or bits) set a specific way, provide a
    /// <see cref="PartyLocalEndpoint::CancelMessages()" /> <em>messageIdentityFilterMask</em> parameter with the bits
    /// to check enabled and the desired way those bits must not be set for a match in the
    /// <em>filteredMessageIdentitiesToMatch</em> parameter value.
    /// <para>
    /// For example, to filter to messages that have <em>identityForCancelFilters</em> values with a particular bit (or
    /// at least one bit from a particular group) enabled, specify a <em>messageIdentityFilterMask</em> parameter with
    /// the relevant bits to check and a value of zero for the <em>filteredMessageIdentitiesToMatch</em> parameter.
    /// </para>
    /// <para>
    /// Alternatively, to exclude specific <em>identityForCancelFilters</em> field values exactly, specify a
    /// <em>messageIdentityFilterMask</em> parameter with all bits set (the maximum uint32_t value) and the excluded
    /// value in the <em>filteredMessageIdentitiesToMatch</em> parameter.
    /// </para>
    /// </remarks>
    IdentityAndMaskDoesNotEqualMatchValue = 2,
};

/// <summary>
/// Options for synchronizing messages between endpoints.
/// </summary>
/// <nyi />
enum class PartySynchronizeMessagesBetweenEndpointsOptions : int32_t
{
    /// <summary>
    /// No additional options.
    /// </summary>
    None = 0x0000,

    /// <summary>
    /// Synchronize messages with the specified endpoints and all shared properties.
    /// </summary>
    SynchronizeMessagesWithSharedProperties = 0x0001,
};

DEFINE_ENUM_FLAG_OPERATORS(PartySynchronizeMessagesBetweenEndpointsOptions);

/// <summary>
/// Types of statistics that can be retrieved for a network.
/// </summary>
/// <seealso cref="PartyEndpointStatistic" />
/// <seealso cref="PartySendMessageOptions" />
/// <seealso cref="PartyNetwork::GetNetworkStatistics" />
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
/// <seealso_nyi cref="PartyLocalEndpoint::CancelMessages" />
/// <seealso cref="PartyLocalEndpoint::GetEndpointStatistics" />
enum class PartyNetworkStatistic
{
    /// <summary>
    /// The current moving average round trip latency ("ping time") in milliseconds to the network's cloud relay server.
    /// </summary>
    /// <remarks>
    /// This latency represents a moving average of the time it currently takes for this local device to send a message
    /// and receive a response from the transparent cloud relay server.
    /// <para>
    /// You can also determine a particular local endpoint's average round trip latency to another endpoint in the
    /// network by using <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> to retrieve the
    /// <see cref="PartyEndpointStatistic::AverageDeviceRoundTripLatencyInMilliseconds" /> statistic.
    /// </para>
    /// </remarks>
    AverageRelayServerRoundTripLatencyInMilliseconds = 0,

    /// <summary>
    /// The total number of internal protocol packets ever transmitted to remote devices as part of this network.
    /// </summary>
    /// <remarks>
    /// This statistic represents the total number of internal protocol packets transmitted by the local device for any
    /// network reason. It includes packets generated as a result of <see cref="PartyLocalEndpoint::SendMessage()" />
    /// calls but doesn't necessarily have a one-to-one correspondence with them because multiple small messages may be
    /// coalesced together into a single packet, or a large message may be fragmented into multiple packets depending on
    /// configuration and environmental factors. Further, packets may be used to carry chat or other internal library
    /// functionality (for example, API operation support messages, protocol acknowledgments or retries) on behalf of
    /// the application without an explicit PartyLocalEndpoint::SendMessage() call.
    /// <para>
    /// This statistic doesn't include any packets generated for HTTP web client operations that are used internally by
    /// the Party library for some aspects of authentication, management transactions, speech-to-text transcription, and
    /// text-to-speech synthesis.
    /// </para>
    /// </remarks>
    SentProtocolPackets = 1,

    /// <summary>
    /// The total number of internal protocol bytes ever transmitted to remote devices as part of this network.
    /// </summary>
    /// <remarks>
    /// This statistic represents the total number of bytes in internal protocol packets transmitted by the local device
    /// for any network reason. This size includes <see cref="PartyLocalEndpoint::SendMessage()" /> payloads but also
    /// their internal protocol overhead, as well as any chat or other internal library functionality needed (for
    /// example, API operation support messages, protocol acknowledgments or retries) on behalf of the application
    /// without an explicit PartyLocalEndpoint::SendMessage() call.
    /// <para>
    /// The reported value doesn't include the packet overhead for Internet protocols (for example, UDP, IP) or that of
    /// lower level media over which the Party library internal protocol operates.
    /// </para>
    /// <para>
    /// This statistic doesn't include the sizes of any packets generated for HTTP web client operations that are used
    /// internally by the Party library for some aspects of authentication, management transactions, speech-to-text
    /// transcription, and text-to-speech synthesis.
    /// </para>
    /// </remarks>
    SentProtocolBytes = 2,

    /// <summary>
    /// The total number of internal protocol packets ever retransmitted to remote devices as part of this network.
    /// </summary>
    /// <remarks>
    /// This statistic represents the total number of internal protocol packet retransmissions by the local device due
    /// to apparent environmental loss of an earlier attempt. The Party library will only retry lost packets that
    /// contain application messages sent with <see cref="PartySendMessageOptions::GuaranteedDelivery" /> or internal
    /// library messages with similar delivery requirements.
    /// <para>
    /// This statistic doesn't include any retries of packets generated for HTTP web client operations that are used
    /// internally by the Party library for some aspects of authentication, management transactions, speech-to-text
    /// transcription, and text-to-speech synthesis.
    /// </para>
    /// </remarks>
    RetriedProtocolPackets = 3,

    /// <summary>
    /// The total number of internal protocol bytes ever retransmitted to remote devices as part of this network.
    /// </summary>
    /// <remarks>
    /// This statistic represents the total number of bytes in internal protocol packet retransmissions by the local
    /// device due to apparent environmental loss of an earlier attempt. This size includes payloads of application
    /// messages sent with <see cref="PartySendMessageOptions::GuaranteedDelivery" /> or internal library messages with
    /// similar delivery requirements.
    /// <para>
    /// The reported value doesn't include the retried packet overhead for Internet protocols (for example, UDP, IP) or
    /// that of lower level media over which the Party library internal protocol operates.
    /// </para>
    /// <para>
    /// This statistic doesn't include the sizes of any retried packets generated for HTTP web client operations that
    /// are used internally by the Party library for some aspects of authentication, management transactions,
    /// speech-to-text transcription, and text-to-speech synthesis.
    /// </para>
    /// </remarks>
    RetriedProtocolBytes = 4,

    /// <summary>
    /// The total number of internal protocol packets to remote devices in this network that are believed to have been
    /// dropped.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This statistic represents the total number of internal protocol packet transmissions by the local device that
    /// were attempted but appeared to suffer environmental loss. They may or may not have been retried (see
    /// <em>RetriedProtocolPackets</em>).
    /// </para>
    /// <para>
    /// This statistic doesn't include any dropped packets that were generated for HTTP web client operations used
    /// internally by the Party library for some aspects of authentication, management transactions, speech-to-text
    /// transcription, and text-to-speech synthesis.
    /// </para>
    /// </remarks>
    DroppedProtocolPackets = 5,

    /// <summary>
    /// The total number of internal protocol packets ever received from remote devices as part of this network.
    /// </summary>
    /// <remarks>
    /// This statistic represents the total number of internal protocol packets received by the local device for any
    /// network reason, which doesn't have a one-to-one correspondence with
    /// <see cref="PartyEndpointMessageReceivedStateChange" />s. Multiple small messages may be coalesced together into
    /// a single packet, or a large message may be fragmented into multiple packets depending on configuration and
    /// environmental factors. Further, packets may be used to carry chat or other internal library functionality (for
    /// example, API operation support messages, protocol acknowledgments or retries) on behalf of the application
    /// without an explicit <see cref="PartyLocalEndpoint::SendMessage()" /> call.
    /// <para>
    /// This statistic doesn't include any packets received as part of HTTP web client operations that are used
    /// internally by the Party library for some aspects of authentication, management transactions, speech-to-text
    /// transcription, and text-to-speech synthesis.
    /// </para>
    /// </remarks>
    ReceivedProtocolPackets = 6,

    /// <summary>
    /// The total number of internal protocol bytes ever received from remote devices as part of this network.
    /// </summary>
    /// <remarks>
    /// This statistic represents the total number of bytes in internal protocol packets received by the local device
    /// for any network reason. This size includes <see cref="PartyEndpointMessageReceivedStateChange" /> payloads but
    /// also their internal protocol overhead, as well as any chat or other internal library functionality needed (for
    /// example, API operation support messages, protocol acknowledgments or retries).
    /// <para>
    /// The reported value does not include the packet overhead for Internet protocols (for example, UDP, IP) or that of
    /// lower level media over which the Party library internal protocol operates.
    /// </para>
    /// <para>
    /// This statistic does not include the sizes of any packets received as part of HTTP web client operations that are
    /// used internally by the Party library for some aspects of authentication, management transactions, speech-to-text
    /// transcription, and text-to-speech synthesis.
    /// </para>
    /// </remarks>
    ReceivedProtocolBytes = 7,

    /// <summary>
    /// The number of messages currently queued by <see cref="PartyLocalEndpoint::SendMessage()" /> but not yet
    /// transmitted from all local endpoints to all remote endpoints in the network.
    /// </summary>
    /// <remarks>
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages will only be counted a single
    /// time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CurrentlyQueuedSendMessages = 8,

    /// <summary>
    /// The number of bytes of data in messages currently queued by <see cref="PartyLocalEndpoint::SendMessage()" /> but
    /// not yet transmitted from all local endpoints to all remote endpoints in the network.
    /// </summary>
    /// <remarks>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages' bytes will only be counted a
    /// single time rather than multiplied per target endpoint.
    /// </remarks>
    CurrentlyQueuedSendMessageBytes = 9,

    /// <summary>
    /// The number of <see cref="PartyLocalEndpoint::SendMessage()" /> messages from all local endpoints to all remote
    /// endpoints in the network that are currently being transmitted or still have more local state changes to be
    /// processed.
    /// </summary>
    /// <remarks>
    /// A message is considered "active" as soon as its first byte has been placed in a packet that is being
    /// transmitted. It remains active until transmission completes and the local
    /// <see cref="PartyDataBuffersReturnedStateChange" /> for it (if applicable) has been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages will only be counted a single
    /// time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CurrentlyActiveSendMessages = 10,

    /// <summary>
    /// The number of bytes of data in <see cref="PartyLocalEndpoint::SendMessage()" /> messages from all local
    /// endpoints to all remote endpoints in the network that are currently being transmitted or still have more local
    /// state changes to be processed.
    /// </summary>
    /// <remarks>
    /// A message is considered "active" as soon as its first byte has been placed in a packet that is being
    /// transmitted. It remains active until transmission completes and the local
    /// <see cref="PartyDataBuffersReturnedStateChange" /> for it (if applicable) has been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages' bytes will only be counted a
    /// single time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CurrentlyActiveSendMessageBytes = 11,

    /// <summary>
    /// The total number of <see cref="PartyLocalEndpoint::SendMessage()" /> messages from all local endpoints to all
    /// remote endpoints in the network that were ever discarded for exceeding their send queue timeouts without being
    /// transmitted.
    /// </summary>
    /// <remarks>
    /// A message's send queue timeout is specified using
    /// <see cref="PartySendMessageQueuingConfiguration::timeoutInMillseconds" /> when calling
    /// PartyLocalEndpoint::SendMessage().
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages will only be counted a single
    /// time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    TimedOutSendMessages = 12,

    /// <summary>
    /// The total number of bytes of data in <see cref="PartyLocalEndpoint::SendMessage()" /> messages from all local
    /// endpoints to all remote endpoints in the network that were ever discarded for exceeding their send queue
    /// timeouts without being transmitted.
    /// </summary>
    /// <remarks>
    /// A message's send queue timeout is specified using
    /// <see cref="PartySendMessageQueuingConfiguration::timeoutInMillseconds" /> when calling
    /// PartyLocalEndpoint::SendMessage().
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages' bytes will only be counted a
    /// single time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    TimedOutSendMessageBytes = 13,

    /// <summary>
    /// The total number of <see cref="PartyLocalEndpoint::SendMessage()" /> messages from all local endpoints to all
    /// remote endpoints in the network that were ever canceled.
    /// </summary>
    /// <remarks>
    /// Messages are canceled before being transmitted by the application calling PartyLocalEndpoint::CancelMessages()
    /// with a matching filter expression.
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages will only be counted a single
    /// time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CanceledSendMessages = 14,

    /// <summary>
    /// The total number of bytes of data in <see cref="PartyLocalEndpoint::SendMessage()" /> messages from all local
    /// endpoints to all remote endpoints in the network that were ever canceled.
    /// </summary>
    /// <remarks>
    /// Messages are canceled before being transmitted by the application calling PartyLocalEndpoint::CancelMessages()
    /// with a matching filter expression.
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. Such messages' bytes will only be counted a
    /// single time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CanceledSendMessageBytes = 15,
};

/// <summary>
/// Types of statistics that can be retrieved for an endpoint.
/// </summary>
/// <seealso cref="PartyNetworkStatistic" />
/// <seealso cref="PartySendMessageQueuingConfiguration" />
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
/// <seealso_nyi cref="PartyLocalEndpoint::CancelMessages" />
/// <seealso cref="PartyLocalEndpoint::GetEndpointStatistics" />
/// <seealso cref="PartyNetwork::GetNetworkStatistics" />
enum class PartyEndpointStatistic
{
    /// <summary>
    /// The number of messages currently queued by <see cref="PartyLocalEndpoint::SendMessage()" /> but not yet
    /// transmitted from the local endpoint to the specified target endpoints.
    /// </summary>
    /// <remarks>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. If two or more of those endpoints are specified
    /// to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic, the message will
    /// only be counted a single time rather than multiplied per target endpoint.
    /// </remarks>
    CurrentlyQueuedSendMessages = 0,

    /// <summary>
    /// The number of bytes of data in messages currently queued by <see cref="PartyLocalEndpoint::SendMessage()" /> but
    /// not yet transmitted from the local endpoint to the specified target endpoints.
    /// </summary>
    /// <remarks>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. If two or more of those endpoints are specified
    /// to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic, the message
    /// bytes will only be counted a single time rather than multiplied per target endpoint.
    /// </remarks>
    CurrentlyQueuedSendMessageBytes = 1,

    /// <summary>
    /// The number of <see cref="PartyLocalEndpoint::SendMessage()" /> messages from the local endpoint to the specified
    /// target endpoints that are currently being transmitted or still have more local state changes to be processed.
    /// </summary>
    /// <remarks>
    /// A message is considered "active" as soon as its first byte has been placed in a packet that is being
    /// transmitted. It remains active until transmission completes and the local
    /// <see cref="PartyDataBuffersReturnedStateChange" /> for it (if applicable) has been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// For efficiency, only a single message is tracked per PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. If two or more of those endpoints are specified
    /// to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic, the message will
    /// only be counted a single time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CurrentlyActiveSendMessages = 2,

    /// <summary>
    /// The number of bytes of data in <see cref="PartyLocalEndpoint::SendMessage()" /> messages from the local endpoint
    /// to the specified target endpoints that are currently being transmitted or still have more local state changes to
    /// be processed.
    /// </summary>
    /// <remarks>
    /// A message is considered "active" as soon as its first byte has been placed in a packet that is being
    /// transmitted. It remains active until transmission completes and the local
    /// <see cref="PartyDataBuffersReturnedStateChange" /> for it (if applicable) has been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// For efficiency, only a single message is tracked per PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. If two or more of those endpoints are specified
    /// to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic, the message
    /// bytes will only be counted a single time rather than multiplied per target endpoint.
    /// </para>
    /// </remarks>
    CurrentlyActiveSendMessageBytes = 3,

    /// <summary>
    /// The total number of <see cref="PartyLocalEndpoint::SendMessage()" /> messages from the local endpoint to the
    /// specified target endpoints that were ever discarded for exceeding their send queue timeouts without being
    /// transmitted.
    /// </summary>
    /// <remarks>
    /// A message's send queue timeout is specified using
    /// <see cref="PartySendMessageQueuingConfiguration::timeoutInMillseconds" /> when calling
    /// PartyLocalEndpoint::SendMessage().
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. If two or more of those endpoints are specified
    /// to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic, the message will
    /// only be counted a single time rather than multiplied per target endpoint.
    /// </para>
    /// <para>
    /// When querying this statistic for all target endpoints by providing a zero-entry array to
    /// PartyLocalEndpoint::GetEndpointStatistics(), note that only endpoints that currently exist will be included. If
    /// an endpoint that had timed out messages gets destroyed between queries, this count might be perceived as "going
    /// backward".
    /// </para>
    /// </remarks>
    TimedOutSendMessages = 4,

    /// <summary>
    /// The total number of bytes of data in <see cref="PartyLocalEndpoint::SendMessage()" /> messages from the local
    /// endpoint to the specified target endpoints that were ever discarded for exceeding their send queue timeouts
    /// without being transmitted.
    /// </summary>
    /// <remarks>
    /// A message's send queue timeout is specified using
    /// <see cref="PartySendMessageQueuingConfiguration::timeoutInMillseconds" /> when calling
    /// PartyLocalEndpoint::SendMessage().
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the transparent cloud relay server. If two or more of those endpoints are specified
    /// to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic, the message
    /// bytes will only be counted a single time rather than multiplied per target endpoint.
    /// </para>
    /// <para>
    /// When querying this statistic for all target endpoints by providing a zero-entry array to
    /// PartyLocalEndpoint::GetEndpointStatistics(), note that only endpoints that currently exist will be included. If
    /// an endpoint that had timed out messages gets destroyed between queries, this count might be perceived as "going
    /// backward".
    /// </para>
    /// </remarks>
    TimedOutSendMessageBytes = 5,

    /// <summary>
    /// The total number of <see cref="PartyLocalEndpoint::SendMessage()" /> messages from the local endpoint to the
    /// specified target endpoints that were ever canceled.
    /// </summary>
    /// <remarks>
    /// Messages are canceled before being transmitted by the application calling PartyLocalEndpoint::CancelMessages()
    /// with a matching filter expression.
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the PartyNetwork transparent cloud relay server. If two or more of those endpoints
    /// are specified to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic,
    /// the message will only be counted a single time rather than multiplied per target endpoint.
    /// </para>
    /// <para>
    /// When querying this statistic for all target endpoints by providing a zero-entry array to
    /// PartyLocalEndpoint::GetEndpointStatistics(), note that only endpoints that currently exist will be included. If
    /// an endpoint that had canceled messages gets destroyed between queries, this count might be perceived as "going
    /// backward".
    /// </para>
    /// </remarks>
    CanceledSendMessages = 6,

    /// <summary>
    /// The total number of bytes of data in <see cref="PartyLocalEndpoint::SendMessage()" /> messages from the local
    /// endpoint to the specified target endpoints that were ever canceled.
    /// </summary>
    /// <remarks>
    /// Messages are canceled before being transmitted by the application calling PartyLocalEndpoint::CancelMessages()
    /// with a matching filter expression.
    /// <para>
    /// For efficiency, only a single message is queued by PartyLocalEndpoint::SendMessage() when sending to multiple
    /// target endpoints at once via the PartyNetwork transparent cloud relay server. If two or more of those endpoints
    /// are specified to <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> when querying for this statistic,
    /// the message bytes will only be counted a single time rather than multiplied per target endpoint.
    /// </para>
    /// <para>
    /// When querying this statistic for all target endpoints by providing a zero-entry array to
    /// PartyLocalEndpoint::GetEndpointStatistics(), note that only endpoints that currently exist will be included. If
    /// an endpoint that had canceled messages gets destroyed between queries, this count might be perceived as "going
    /// backward".
    /// </para>
    /// </remarks>
    CanceledSendMessageBytes = 7,

    /// <summary>
    /// The current moving average round trip latency ("ping time") in milliseconds to the endpoint's owning device.
    /// </summary>
    /// <remarks>
    /// This latency represents a moving average of the time it currently takes for this local device to send a message
    /// and receive a response from the target endpoint's device.
    /// <para>
    /// This statistic can only be queried for exactly one target endpoint at a time. The
    /// <see cref="PartyLocalEndpoint::GetEndpointStatistics()" /> method will fail if multiple targets are provided, or
    /// if a zero-entry array is provided to attempt to query all current endpoints.
    /// </para>
    /// <para>
    /// You can also determine the local device's average round trip latency to the network's transparent cloud relay
    /// server by using <see cref="PartyNetwork::GetNetworkStatistics()" /> to retrieve the
    /// <see cref="PartyNetworkStatistic::AverageRelayServerRoundTripLatencyInMilliseconds" /> statistic.
    /// </para>
    /// </remarks>
    AverageDeviceRoundTripLatencyInMilliseconds = 8,
};

/// <summary>
/// Options for who may revoke an invitation.
/// </summary>
/// <seealso cref="PartyNetwork::RevokeInvitation" />
/// <seealso cref="PartyInvitation" />
/// <seealso cref="PartyLocalUser" />
enum class PartyInvitationRevocability
{
    /// <summary>
    /// The invitation may only be revoked by the PartyLocalUser that created the invitation.
    /// </summary>
    Creator = 0,

    /// <summary>
    /// The invitation may be revoked by any authenticated user in the network.
    /// </summary>
    Anyone = 1,
};

/// <summary>
/// Options for defining the communication relationship between two chat controls.
/// </summary>
enum class PartyChatPermissionOptions
{
    /// <summary>
    /// No chat communication between the local chat control and the target chat control is allowed.
    /// </summary>
    None = 0x0,

    /// <summary>
    /// Microphone audio communication from the local chat control to the target chat control is allowed.
    /// </summary>
    /// <remarks>
    /// For most game scenarios, either all or no audio communication should be sent and <c>SendAudio</c> should be used
    /// instead. This flag can be used for game scenarios where microphone audio should be sent to the target chat
    /// control, but other types of audio should not.
    /// <para>
    /// Audio optionally submitted to a capture sink via <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" />
    /// is treated as microphone audio when applying chat permissions and determining which chat controls should receive
    /// the audio.
    /// </para>
    /// </remarks>
    SendMicrophoneAudio = 0x1,

    /// <summary>
    /// Text-to-speech audio communication from the local chat control to the target chat control is allowed.
    /// </summary>
    /// <remarks>
    /// For most game scenarios, either all or no audio communication should be sent and <c>SendAudio</c> should be used
    /// instead. This flag can be used for game scenarios where text-to-speech audio should be sent to the target chat
    /// control, but other types of audio should not.
    /// </remarks>
    SendTextToSpeechAudio = 0x2,

    /// <summary>
    /// All audio communication from the local chat control to the target chat control is allowed.
    /// </summary>
    /// <remarks>
    /// This flag is equivalent to <c>SendMicrophoneAudio | SendTextToSpeechAudio</c>.
    /// </remarks>
    SendAudio = SendMicrophoneAudio | SendTextToSpeechAudio,

    /// <summary>
    /// Microphone audio communication from the target chat control to the local chat control is allowed.
    /// </summary>
    /// <remarks>
    /// For most game scenarios, either all or no audio communication should be received and <c>ReceiveAudio</c> should
    /// be used instead. This flag can be used for game scenarios where microphone audio should be received from the
    /// target chat control, but other types of audio should not.
    /// <para>
    /// Audio optionally submitted to a capture sink via <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" />
    /// is treated as microphone audio when applying chat permissions and determining which chat controls should receive
    /// the audio.
    /// </para>
    /// </remarks>
    ReceiveMicrophoneAudio = 0x4,

    /// <summary>
    /// Text-to-speech audio communication from the target chat control to the local chat control is allowed.
    /// </summary>
    /// <remarks>
    /// For most game scenarios, either all or no audio communication should be received and <c>ReceiveAudio</c> should
    /// be used instead. This flag can be used for game scenarios where text-to-speech audio should be received from the
    /// target chat control, but other types of audio should not.
    /// </remarks>
    ReceiveTextToSpeechAudio = 0x8,

    /// <summary>
    /// Audio communication from the target chat control to the local chat control is allowed.
    /// </summary>
    /// <remarks>
    /// This flag is equivalent to <c>ReceiveMicrophoneAudio | ReceiveTextToSpeechAudio</c>.
    /// </remarks>
    ReceiveAudio = ReceiveMicrophoneAudio | ReceiveTextToSpeechAudio,

    /// <summary>
    /// Text communication from the target chat control to the local chat control is allowed.
    /// </summary>
    /// <para>
    /// There is no permission associated with sending chat text because each call to <see cref="SendText()" /> requires
    /// an explicit list of target chat controls. Including or omitting a target is equivalent to granting or denying
    /// send permission for that text message. The target still must have set <c>ReceiveText</c> in their instance of
    /// the library for it to actually be delivered.
    /// </para>
    ReceiveText = 0x10,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyChatPermissionOptions);

/// <summary>
/// Options for selecting audio devices
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetAudioInput" />
/// <seealso cref="PartyLocalChatControl::SetAudioOutput" />
enum class PartyAudioDeviceSelectionType
{
    /// <summary>
    /// Indicate that no audio device should be used.
    /// </summary>
    None = 0,

    /// <summary>
    /// Select audio device based on the platform settings.
    /// </summary>
    /// <remarks>
    /// If this selection type is used for <see cref="PartyLocalChatControl::SetAudioInput()" /> or
    /// <see cref="PartyLocalChatControl::SetAudioOutput()" />, then the audioDeviceSelectionContext should be nullptr.
    /// </remarks>
    SystemDefault = 1,

    /// <summary>
    /// Select audio device based on the platform and user settings.
    /// </summary>
    /// <remarks>
    /// If this selection type is used for <see cref="PartyLocalChatControl::SetAudioInput()" /> or
    /// <see cref="PartyLocalChatControl::SetAudioOutput()" />, then the audioDeviceSelectionContext must be a platform
    /// specific user context.
    /// </remarks>
    PlatformUserDefault = 2,

    /// <summary>
    /// Audio device specified by the game.
    /// </summary>
    /// <remarks>
    /// If this selection type is used for <see cref="PartyLocalChatControl::SetAudioInput()" /> or
    /// <see cref="PartyLocalChatControl::SetAudioOutput()" />, then the audioDeviceSelectionContext must be the
    /// identifier of the audio device to use.
    /// <para>
    /// On Android, iOS, and macOS, setting a manual input device is an illegal operation. Using manual device selection
    /// with <see cref="PartyLocalChatControl::SetAudioInput()" /> on Android, iOS, and macOS will result in an error
    /// indicating that the device selection type is not supported.
    /// </para>
    /// </remarks>
    Manual = 3,
};

/// <summary>
/// States of an audio input associated with a local chat control.
/// </summary>
enum class PartyAudioInputState
{
    /// <summary>
    /// No audio input was specified.
    /// </summary>
    NoInput = 0,

    /// <summary>
    /// The audio input was successfully initialized.
    /// </summary>
    Initialized = 1,

    /// <summary>
    /// The audio input wasn't found.
    /// </summary>
    /// <remarks>
    /// This can occur if the input was removed while in use. When in this state, the chat control will subscribe to
    /// audio device changes and use the specified input if it can be found at a later time.
    /// </remarks>
    NotFound = 2,

    /// <summary>
    /// The user didn't give permission to access the input.
    /// </summary>
    UserConsentDenied = 3,

    /// <summary>
    /// The input doesn't provide a supported format.
    /// </summary>
    UnsupportedFormat = 4,

    /// <summary>
    /// The audio input device is already in use by another chat control.
    /// </summary>
    AlreadyInUse = 5,

    /// <summary>
    /// Initializing the input resulted in an unknown error.
    /// </summary>
    UnknownError = 6,
};

/// <summary>
/// States of an audio output associated with a local chat control.
/// </summary>
enum class PartyAudioOutputState
{
    /// <summary>
    /// No audio output was specified.
    /// </summary>
    NoOutput = 0,

    /// <summary>
    /// The audio output was successfully initialized.
    /// </summary>
    Initialized = 1,

    /// <summary>
    /// The audio output wasn't found.
    /// </summary>
    /// <remarks>
    /// This can occur if the output was removed while in use. When in this state, the chat control will subscribe to
    /// audio device changes and use the specified output if it can be found at a later time.
    /// </remarks>
    NotFound = 2,

    /// <summary>
    /// The output doesn't provide a supported sample rate.
    /// </summary>
    UnsupportedFormat = 3,

    /// <summary>
    /// The audio output device is already in use by another chat control.
    /// </summary>
    AlreadyInUse = 4,

    /// <summary>
    /// Initializing the output resulted in an unknown error.
    /// </summary>
    UnknownError = 5,
};

/// <summary>
/// Audio states for a local chat control.
/// </summary>
enum class PartyLocalChatControlChatIndicator
{
    /// <summary>
    /// The local chat control is not generating voice audio.
    /// </summary>
    Silent = 0,

    /// <summary>
    /// The local chat control is generating voice audio.
    /// </summary>
    Talking = 1,

    /// <summary>
    /// The local chat control's audio input is muted.
    /// </summary>
    AudioInputMuted = 2,

    /// <summary>
    /// Either no audio input has been specified for the local chat control, or initializing the specified input failed.
    /// </summary>
    /// <remarks>
    /// This may be due to the user removing a physical audio device, or an error during audio device initialization. A
    /// <see cref="PartyLocalChatAudioInputChangedStateChange" /> or
    /// <see cref="PartyLocalChatAudioOutputChangedStateChange" /> with more granular information will have been
    /// provided prior to the chat indicator changing to this value.
    /// </remarks>
    NoAudioInput = 3,
};

/// <summary>
/// Audio states for a target chat control in relation to a local chat control.
/// </summary>
enum class PartyChatControlChatIndicator
{
    /// <summary>
    /// The target chat control isn't generating voice audio.
    /// </summary>
    Silent = 0,

    /// <summary>
    /// The target chat control is generating voice audio.
    /// </summary>
    Talking = 1,

    /// <summary>
    /// The local chat control is not configured to receive audio from the target chat control.
    /// </summary>
    IncomingVoiceDisabled = 2,

    /// <summary>
    /// The target chat control has been muted by the local chat control.
    /// </summary>
    IncomingCommunicationsMuted = 3,

    /// <summary>
    /// The target chat control does not have an audio input.
    /// </summary>
    NoRemoteInput = 4,

    /// <summary>
    /// The target chat control has an audio input but has muted it.
    /// </summary>
    RemoteAudioInputMuted = 5,
};

/// <summary>
/// Genders for text-to-speech profiles.
/// </summary>
/// <seealso cref="PartyTextToSpeechProfile" />
enum class PartyGender
{
    /// <summary>
    /// The text-to-speech profile represents a gender-neutral voice.
    /// </summary>
    Neutral = 0,

    /// <summary>
    /// The text-to-speech profile represents a female voice.
    /// </summary>
    Female = 1,

    /// <summary>
    /// The text-to-speech profile represents a male voice.
    /// </summary>
    Male = 2,
};

/// <summary>
/// Types of Party audio sources.
/// </summary>
enum class PartyAudioSourceType
{
    /// <summary>
    /// A microphone audio source.
    /// </summary>
    /// <remarks>
    /// Audio captured from a microphone or recording device, configured via
    /// <see cref="PartyLocalChatControl::SetAudioInput()" />, or optionally submitted to a capture sink, via
    /// <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" />, is treated as microphone audio.
    /// </remarks>
    Microphone = 0,

    /// <summary>
    /// A text-to-speech audio source.
    /// </summary>
    /// <remarks>
    /// Audio that is generated by <see cref="PartyLocalChatControl::SynthesizeTextToSpeech()"/> is treated as
    /// text-to-speech audio.
    /// </remarks>
    TextToSpeech = 1,
};

/// <summary>
/// Types of transcription phrases.
/// </summary>
/// <remarks>
/// A <em>Hypothesis</em> phrase represents a snapshot into the transcription process and does not represent a stable
/// accumulation of transcription. For example, a first speech hypothesis may contain the words "fine fun" and the
/// second hypothesis may contain the words "find funny". Hypothesis messages are frequent and there can be many
/// hypothesis messages associated with a particular phrase. Once a transcription of type Final is received, no more
/// hypothesis messages associated with that phrase will be provided; new hypothesis phrases will represent a new
/// logical phrase. Because each phrase is tied to a speaker, hypothesis messages for a phrase can be correlated by
/// inspecting the <em>speaker</em> field of the <see cref="PartyVoiceChatTranscriptionReceivedStateChange" />.
/// <para>
/// Hypothesis phrases can optionally be used to improve the user experience by improving perceived responsiveness. An
/// example implementation might have a special text box for hypothesis messages that is frequently updated with the
/// latest hypothesis transcription from each chat control. Once a non-hypothesis transcription is received, the
/// transcription for the chat control would be removed from the hypothesis text box and rendered in a long-lived text
/// box.
/// </para>
/// <para>
/// A <em>Final</em> phrase represents the end of the transcription process after the speaker has completed a sentence
/// or phrase. Capitalization, punctuation, inverse text normalization, and profanity masking will have been applied to
/// this transcription. For example, if a user speaks a phrase represented by the words "my flight to seattle leaves at
/// six", the transcription will read "My flight to Seattle leaves at 6." Inverse text normalization is the process that
/// converts the word "six" to the number "6". Profane words will be replaced by asterisks.
/// </para>
/// <para>
/// If the associated transcription represents the audio generated by a call to
/// <see cref="PartyLocalChatControl::SynthesizeTextToSpeech()" />, the transcription will match the text string used to
/// generate the audio. No post-processing, such as capitalization and punctuation, will be applied to the text.
/// </para>
/// </remarks>
/// <seealso cref="PartyLocalChatControl::SynthesizeTextToSpeech" />
/// <seealso cref="PartyVoiceChatTranscriptionReceivedStateChange" />
enum class PartyVoiceChatTranscriptionPhraseType
{
    /// <summary>
    /// The transcription is a hypothesis phrase.
    /// </summary>
    Hypothesis = 0,

    /// <summary>
    /// The transcription is a final phrase.
    /// </summary>
    Final = 1,
};

/// <summary>
/// Voice chat transcription options.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
/// <seealso cref="PartyVoiceChatTranscriptionReceivedStateChange" />
/// <seealso cref="PartyVoiceChatTranscriptionPhraseType" />
enum class PartyVoiceChatTranscriptionOptions
{
    /// <summary>
    /// No chat controls will be transcribed.
    /// </summary>
    None = 0x0,

    /// <summary>
    /// Transcriptions of the local chat control will be generated and provided to the same local chat control via
    /// <see cref="PartyVoiceChatTranscriptionReceivedStateChange" /> events while in a network with at least one other
    /// chat control.
    /// </summary>
    TranscribeSelf = 0x1,

    /// <summary>
    /// Transcriptions of other chat controls with the same language as the local chat control will be generated and
    /// provided to the local chat control via <see cref="PartyVoiceChatTranscriptionReceivedStateChange" /> events.
    /// </summary>
    TranscribeOtherChatControlsWithMatchingLanguages = 0x2,

    /// <summary>
    /// Transcriptions of other chat controls with languages that are different from the local chat control's language
    /// will be generated and provided to the local chat control via
    /// <see cref="PartyVoiceChatTranscriptionReceivedStateChange" /> events.
    /// </summary>
    /// <remarks>
    /// The transcriptions are not translated by default. Translation to the local chat control's language can also be
    /// enabled by adding <em>TranslateToLocalLanguage</em>.
    /// </remarks>
    TranscribeOtherChatControlsWithNonMatchingLanguages = 0x4,

    /// <summary>
    /// Hypothesis phrases will be disabled.
    /// </summary>
    /// <remarks>
    /// This option is currently unimplemented. Using it with
    /// <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> will fail.
    /// <para>
    /// Use of hypothesis phrases is encouraged as a best practice to minimize the perceived latency of the
    /// transcription. If the phrases will not be used, however, they can be disabled in order to reduce the network
    /// bandwidth used to send the phrases to the local chat control.
    /// </para>
    /// <para>
    /// This option will have no effect unless also combined with <em>TranscribeSelf</em>,
    /// <em>TranscribeOtherChatControlsWithMatchingLanguages</em>, and/or
    /// <em>TranscribeOtherChatControlsWithNonMatchingLanguages</em>.
    /// </para>
    /// </remarks>
    /// <nyi />
    DisableHypothesisPhrases = 0x8,

    /// <summary>
    /// Transcriptions will be translated to the local chat control's language.
    /// </summary>
    /// <remarks>
    /// Transcriptions generated as specified via other PartyVoiceChatTranscriptionOptions values will be translated
    /// into the local chat control's language, which is specified by
    /// <see cref="PartyLocalChatControl::GetLanguage()" />. The translations will be provided in addition to the
    /// original transcription via <see cref="PartyVoiceChatTranscriptionReceivedStateChange" /> events.
    /// <para>
    /// If translation is enabled, a translation corresponding to the local chat control's language will always be
    /// provided in each resulting PartyVoiceChatTranscriptionReceivedStateChange. If the speaking chat control's
    /// language is the same as the local chat control's language, the transcription and translation strings will be
    /// identical.
    /// </para>
    /// <para>
    /// This option will have no effect unless also combined with one or more of <em>TranscribeSelf</em>,
    /// <em>TranscribeOtherChatControlsWithMatchingLanguages</em>, and
    /// <em>TranscribeOtherChatControlsWithNonMatchingLanguages</em>.
    /// </para>
    /// </remarks>
    TranslateToLocalLanguage = 0x10,

    /// <summary>
    /// Transcriptions will be provided without masking profanity.
    /// </summary>
    /// <remarks>
    /// By default, profanity is masked by replacing each character with an asterisk. For instance, a 4-letter profanity
    /// is replaced with "****". When this option is enabled, no masking will be applied to profanity; the raw text will
    /// be provided.
    /// </remarks>
    DisableProfanityMasking = 0x20,

    /// <summary>
    /// Transcriptions of the local chat control will be generated and provided to the same local chat control via
    /// <see cref="PartyVoiceChatTranscriptionReceivedStateChange" /> regardless of whether the chat control is in a
    /// network with any other chat controls.
    /// </summary>
    /// <remarks>
    /// This option overrides <em>TranscribeSelf</em>. This option is provided for backwards compatibility and typically
    /// should not be used.
    /// </remarks>
    TranscribeSelfRegardlessOfNetworkState = 0x40,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyVoiceChatTranscriptionOptions);

/// <summary>
/// Text chat options.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetTextChatOptions" />
/// <seealso cref="PartyLocalChatControl::GetLanguage" />
/// <seealso cref="PartyChatTextReceivedStateChange" />
enum class PartyTextChatOptions
{
    /// <summary>
    /// No options are selected.
    /// </summary>
    None = 0x0,

    /// <summary>
    /// Incoming text chat will be translated to the local chat control's language.
    /// </summary>
    /// <remarks>
    /// The language specified via <see cref="PartyLocalChatControl::GetLanguage()" /> is the language to which the
    /// incoming text chat will be translated. The translations will be provided in addition to the original text chat
    /// via a <see cref="PartyChatTextReceivedStateChange" />.
    /// <para>
    /// If translation is enabled, a translation corresponding to the local chat control's language will always be
    /// provided in each resulting PartyChatTextReceivedStateChange. If the source chat control's language is the same
    /// as the local chat control's language, the chat text and translation strings will be identical.
    /// </para>
    /// </remarks>
    TranslateToLocalLanguage = 0x1,

    /// <summary>
    /// Offensive terms will be filtered out of incoming text chat.
    /// </summary>
    /// <remarks>
    /// For incoming chat text detected as English, the filtering level may be adjusted using the
    /// <see cref="PartyOption::TextChatFilterLevel" /> option in <see cref="PartyManager::SetOption()" />. The default
    /// level is family-friendly.
    /// </remarks>
    FilterOffensiveText = 0x2,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyTextChatOptions);

/// <summary>
/// Flags describing how a translation was received.
/// </summary>
enum class PartyTranslationReceivedOptions
{
    /// <summary>
    /// There is no additional translation information.
    /// </summary>
    None = 0x0,

    /// <summary>
    /// The translation was truncated.
    /// </summary>
    /// <remarks>
    /// A translation string may be up to <c>c_maxChatTextMessageLength</c> characters long, not including the null
    /// terminator. Truncation occurs if the translated string length would exceed that limit, which can happen due to
    /// language differences even though the original string length is less than or equal to
    /// <c>c_maxChatTextMessageLength</c>. Truncation may occur at an arbitrary point in the UTF-8 byte sequence and may
    /// not result in a complete, valid character or word. Strings are always null terminated, even when truncated.
    /// </remarks>
    Truncated = 0x1,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyTranslationReceivedOptions);

/// <summary>
/// Types of text-to-speech operations.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
/// <seealso cref="PartyLocalChatControl::GetTextToSpeechProfile" />
/// <seealso cref="PartyLocalChatControl::SynthesizeTextToSpeech" />
enum class PartySynthesizeTextToSpeechType
{
    /// <summary>
    /// The audio should be rendered to the local chat control's audio output.
    /// </summary>
    /// <remarks>
    /// This type is typically used for the "narration" scenario where text-to-speech audio is rendered to the local
    /// chat control for arbitrary game-defined scenarios, such as the narration of received text. It may also be used
    /// for the "voice auditioning" scenario where a user compares audio between text-to-speech profiles to select the
    /// profile they prefer to use in text-to-speech voice chat scenarios. The chat control does not need to be in a
    /// network for this functionality.
    /// <para>
    /// The audio will never be transcribed, even if transcription is enabled.
    /// </para>
    /// </remarks>
    Narration = 0,

    /// <summary>
    /// The audio should be rendered to the chat controls to which the local chat control is configured to send audio.
    /// </summary>
    /// <remarks>
    /// This type is typically used for the "text-to-speech voice chat" scenario where a user elects not to participate
    /// in voice chat with their microphone, but still wishes to participate in the voice chat via text-to-speech audio.
    /// The audio will be rendered to the chat controls to which the local chat control has been configured to send
    /// audio via <see cref="PartyLocalChatControl::SetPermissions()" />. The chat control does not need to be in a
    /// network for a text-to-speech operation of this type to succeed, but it will have no apparent effect because
    /// there will be no other chat controls to which the local chat control could be configured to send audio.
    /// <para>
    /// Because the audio represents the user's voice, it will be transcribed according to the options specified via
    /// <see cref="PartyLocalChatControl::SetTranscriptionOptions()" />.
    /// </para>
    /// </remarks>
    VoiceChat = 1,
};

/// <summary>
/// Types of Party audio samples.
/// </summary>
enum class PartyAudioSampleType
{
    /// <summary>
    /// Integer PCM format.
    /// </summary>
    Integer = 0,

    /// <summary>
    /// IEEE floating-point PCM format.
    /// </summary>
    Float = 1,
};

/// <summary>
/// Voice audio options.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetVoiceAudioOptions" />
/// <seealso cref="PartyLocalChatControl::GetVoiceAudioOptions" />
enum class PartyVoiceAudioOptions : uint32_t
{
    /// <summary>
    /// No voice audio options are selected.
    /// </summary>
    None = 0x0,

    /// <summary>
    /// Enable noise suppression on audio captured from a local chat control.
    /// </summary>
    /// <remarks>
    /// This option is only supported on iOS and Android. Using this option on any other platform will fail.
    /// <para>
    /// Enabling noise suppression eliminates noise captured from the local chat control to be transmitted to remote
    /// chat controls.
    /// </para>
    /// <para>
    /// Using this option requires packaging inside your application the model file named
    /// PartyDeepNoiseSuppressionModel.fpie that is shipped with PlayFab Party.
    /// </para>
    /// </remarks>
    NoiseSuppression = 0x1,
};

/// <summary>
/// Additional options to control how the Party library binds to the UDP socket specified by the
/// <see cref="PartyLocalUdpSocketBindAddressConfiguration" /> structure.
/// </summary>
/// <seealso cref="PartyLocalUdpSocketBindAddressConfiguration" />
enum class PartyLocalUdpSocketBindAddressOptions
{
    /// <summary>
    /// No flags are specified.
    /// </summary>
    /// <remarks>
    /// This value is the default if a <see cref="PartyLocalUdpSocketBindAddressConfiguration" /> structure has not been
    /// explicitly set.
    /// </remarks>
    None = 0x0,

    /// <summary>
    /// Don't default to using the Microsoft Game Core preferred UDP multiplayer port when the port is unspecified.
    /// </summary>
    /// <remarks>
    /// In the Microsoft Game Core version of the Party library, when the <em>port</em> field of the
    /// <see cref="PartyLocalUdpSocketBindAddressConfiguration" /> structure is 0, this flag informs the Party library
    /// to not use the Game Core preferred UDP multiplayer port. In other versions of the Party library or if the
    /// <em>port</em> port is non-zero, this flag must not be set.
    /// </remarks>
    ExcludeGameCorePreferredUdpMultiplayerPort = 0x1,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyLocalUdpSocketBindAddressOptions);

/// <summary>
/// Flags controlling the attempted use of direct peer-to-peer connectivity among devices in a network.
/// </summary>
/// <remarks>
/// As part of successfully authenticating an initial user into a network, a device may attempt to establish direct
/// peer-to-peer connections with other devices already participating in the network when permitted by these flags
/// declared in the network's <see cref="PartyNetworkConfiguration" /> structure. For attempts that are successful,
/// endpoint messages and chat data between the devices will be transmitted using those direct connections. For attempts
/// that fail due to environmental incompatibilities between the devices, all communication between those devices will
/// be transmitted via transparent cloud relay servers instead. If the devices aren't permitted to attempt direct peer
/// connections by these flags, then they never exchange IP address information and will always transmit endpoint
/// messages and chat data via transparent cloud relay servers.
/// <para>
/// You can determine whether the local device actually established a direct peer-to-peer connection to a specific
/// remote device by calling <see cref="PartyNetwork::GetDeviceConnectionType()" />.
/// </para>
/// <para>
/// Successful direct peer connectivity may provide lower latency between some devices, though attempting to establish
/// it also requires users to disclose their devices' IP addresses to others, which may be a concern for privacy or for
/// enabling malicious users to potentially attack peers' devices and Internet connections outside of the title. It also
/// may not be permitted on certain platforms for policy reasons. Be sure to use the appropriate flags for your
/// performance and security goals.
/// </para>
/// <para>
/// Besides a specific network's <see cref="PartyNetworkConfiguration::directPeerConnectivityOptions" /> value, the
/// flags may optionally be further constrained by a device for all networks into which it authenticates by using
/// <see cref="PartyManager::SetOption()" /> to set
/// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" />. All flags are evaluated using a bitwise
/// AND operation. That is, a particular flag is only in effect for a given network's pair of devices if it's enabled in
/// three places: the network's <see cref="PartyNetworkConfiguration" /> structure, and <em>both</em> devices'
/// respective local mask options. Even if the Party network permits direct peer connectivity of the relevant form,
/// either device can unilaterally opt out of the IP address disclosure and direct connection attempts between them by
/// not enabling the flag. The <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value defaults
/// to permitting all direct peer connections enabled by networks, so you only need to configure it if you have
/// device-specific requirements to prevent some or all direct peer connectivity involving it.
/// </para>
/// <para>
/// To avoid excessive resource consumption, the Party library will also internally prevent any given device from
/// attempting to establish more than a configured maximum number of direct peer connections across all networks in
/// which it's currently participating, even if permitted by these flags. This doesn't affect the device's ability to
/// participate in large or multiple networks with more remote devices. Communication with extra devices will simply be
/// transmitted via transparent cloud relay servers. To modify that maximum number of direct peer connections, use
/// <see cref="PartyManager::SetOption()" /> to set <see cref="PartyOption::LocalDeviceMaxDirectPeerConnections" />.
/// </para>
/// <para>
/// It's recommended that you don't actively enforce the availability of a direct peer-to-peer connection for any given
/// pair of devices (that is, don't call <see cref="PartyNetwork::LeaveNetwork()" /> if
/// <see cref="PartyNetwork::GetDeviceConnectionType()" /> reports a value other than
/// <see cref="PartyDeviceConnectionType::DirectPeerConnection" />) since the specific underlying transmission method in
/// use doesn't alter the overall logical ability to communicate. If your game design has stringent requirements for
/// maximum message latency that encourage direct peer connectivity, it's better to take action on the current concrete
/// observations of that latency as reported by the
/// <see cref="PartyEndpointStatistic::AverageDeviceRoundTripLatencyInMilliseconds" /> statistic rather than make
/// abstract assumptions based on transmission mechanism. Otherwise you might continually hinder users trying to play
/// with the same set of friends who always need to use nearby transparent cloud relay servers due to environmental
/// factors beyond their control.
/// </para>
/// </remarks>
/// <seealso cref="PartyNetworkConfiguration" />
/// <seealso cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" />
/// <seealso cref="PartyManager::SetOption" />
/// <seealso cref="PartyEndpointStatistic::AverageDeviceRoundTripLatencyInMilliseconds" />
/// <seealso cref="PartyLocalEndpoint::GetEndpointStatistics" />
enum class PartyDirectPeerConnectivityOptions : int32_t
{
    /// <summary>
    /// No flags are specified.
    /// </summary>
    /// <remarks>
    /// The absence of flags means that no direct peer connectivity attempts are permitted. All endpoint messages and
    /// chat data between devices will always be transmitted via transparent cloud relay servers, and no IP address
    /// information will ever be exchanged.
    /// </remarks>
    None = 0x0,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices of the same platform type.
    /// </summary>
    /// <remarks>
    /// A pair of devices will attempt to establish direct peer-to-peer connections if they're identified as having the
    /// same type of hardware and/or OS platform (for example, Windows PCs, Xbox gaming consoles, iOS-based mobile
    /// devices) associated with the specific Party library they use.
    /// <para>
    /// Note that this flag doesn't permit any direct peer-to-peer connectivity attempts by itself. It must be combined
    /// with one or both of the <c>SameEntityLoginProvider</c> and <c>DifferentEntityLoginProvider</c> flags.
    /// </para>
    /// <para>
    /// This flag can't be combined with the <c>OnlyServers</c> flag in the <see cref="PartyNetworkConfiguration" />
    /// structure. It may be combined with <c>OnlyServers</c> as part of the
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value.
    /// </para>
    /// </remarks>
    SamePlatformType = 0x1,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices with differing platform types.
    /// </summary>
    /// <remarks>
    /// A pair of devices will attempt to establish direct peer-to-peer connections if they're identified as having
    /// different types of hardware and/or OS platforms (for example, Windows PCs, Xbox gaming consoles, iOS-based
    /// mobile devices) associated with the specific Party libraries each device uses.
    /// <para>
    /// Note that this flag doesn't permit any direct peer-to-peer connectivity attempts by itself. It must be combined
    /// with one or both of the <c>SameEntityLoginProvider</c> and <c>DifferentEntityLoginProvider</c> flags.
    /// </para>
    /// <para>
    /// This flag can't be combined with the <c>OnlyServers</c> flag in the <see cref="PartyNetworkConfiguration" />
    /// structure. It may be combined with <c>OnlyServers</c> as part of the
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value.
    /// </para>
    /// </remarks>
    DifferentPlatformType = 0x2,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices regardless of their platform types.
    /// </summary>
    /// <remarks>
    /// This flag is equivalent to <c>SamePlatformType | DifferentPlatformType</c>.
    /// </remarks>
    AnyPlatformType = SamePlatformType | DifferentPlatformType,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices that have authenticated user PlayFab Entity IDs
    /// that were logged in using the same provider.
    /// </summary>
    /// <remarks>
    /// A device that's initially authenticating a local user into the network will attempt to establish direct peer-to-
    /// peer connections with remote devices that have at least one authenticated user PlayFab Entity ID that was logged
    /// in using the same provider (for example, Xbox Live, Facebook, iOS, Google) as the newly authenticating user.
    /// <para>
    /// Note that this flag doesn't permit any direct peer-to-peer connectivity attempts by itself. It must be combined
    /// with one or both of the <c>SamePlatformType</c> and <c>DifferentPlatformType</c> flags.
    /// </para>
    /// <para>
    /// This flag can't be combined with the <c>OnlyServers</c> flag in the <see cref="PartyNetworkConfiguration" />
    /// structure. It may be combined with <c>OnlyServers</c> as part of the
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value.
    /// <para>
    /// </remarks>
    SameEntityLoginProvider = 0x4,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices with authenticated user PlayFab Entity IDs that
    /// that were logged in using different providers.
    /// </summary>
    /// <remarks>
    /// A device that's initially authenticating a local user into the network will attempt to establish direct peer-to-
    /// peer connections with remote devices that don't have any authenticated user PlayFab Entity IDs that were logged
    /// in using the same provider (for example, Xbox Live, Facebook, iOS, Google) as the newly authenticating user.
    /// <para>
    /// Note that this flag doesn't permit any direct peer-to-peer connectivity attempts by itself. It must be combined
    /// with one or both of the <c>SamePlatformType</c> and <c>DifferentPlatformType</c> flags.
    /// </para>
    /// <para>
    /// This flag can't be combined with the <c>OnlyServers</c> flag in the <see cref="PartyNetworkConfiguration" />
    /// structure. It may be combined with <c>OnlyServers</c> as part of the
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value.
    /// </para>
    /// </remarks>
    DifferentEntityLoginProvider = 0x8,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices regardless of the provider used to login
    /// authenticated user PlayFab Entity IDs.
    /// </summary>
    /// <remarks>
    /// This flag is equivalent to <c>SameEntityLoginProvider | DifferentEntityLoginProvider</c>.
    /// </remarks>
    AnyEntityLoginProvider = SameEntityLoginProvider | DifferentEntityLoginProvider,

    /// <summary>
    /// Direct peer-to-peer connections may be attempted between devices if at least one of the devices has an
    /// authenticated user with a PlayFab Entity type of `game_server`.
    /// </summary>
    /// <remarks>
    /// A device that's initially authenticating a local user into the network will attempt to establish direct peer-to-
    /// peer connections with remote devices if either the local user or a user on the remote device is a `game_server`
    /// PlayFab Entity type, as specified to <see cref="PartyManager::CreateLocalUserWithEntityType()" />. In other
    /// words, players' devices will never connect directly to one another, only to server devices. Server devices will
    /// attempt to connect directly to all player and other server devices.
    /// <para>
    /// This flag can't be combined with any of the <c>SamePlatformType</c>, <c>DifferentPlatformType</c>,
    /// <c>SameEntityLoginProvider</c>, or <c>DifferentEntityLoginProvider</c> flags in the
    /// <see cref="PartyNetworkConfiguration" /> structure. It may be combined with those flags as part of the
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value.
    /// </para>
    /// </remarks>
    OnlyServers = 0x10,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyDirectPeerConnectivityOptions);

/// <summary>
/// The type of connection used for transmitting endpoint message or chat data to a device.
/// </summary>
/// <seealso cref="PartyNetwork::GetDeviceConnectionType" />
enum class PartyDeviceConnectionType
{
    /// <summary>
    /// All communication is transmitted via a transparent cloud relay server.
    /// </summary>
    RelayServer = 0,

    /// <summary>
    /// All communication is transmitted using a direct peer-to-peer connection.
    /// </summary>
    DirectPeerConnection = 1,
};

/// <summary>
/// Flags describing the received text message in PartyChatTextReceivedStateChange.
/// </summary>
/// <seealso cref="PartyChatTextReceivedStateChange" />
enum class PartyChatTextReceivedOptions
{
    /// <summary>
    /// No flags are specified.
    /// </summary>
    None = 0x0,

    /// <summary>
    /// Offensive terms were found and replaced with asterisks.
    /// </summary>
    /// <remarks>
    /// This value is exclusive to FilteredEntireMessage and FilteredDueToError.
    /// </remarks>
    FilteredOffensiveTerms = 0x1,

    /// <summary>
    /// The incoming text chat was unable to filter specific terms, and the entire text has been replaced by asterisks.
    /// </summary>
    /// <remarks>
    /// This option occurs when the moderation service was not able to identify specific terms to filter, but has still
    /// identified the text as being offensive.
    /// <para>
    /// This value is exclusive to FilteredOffensiveTerms and FilteredDueToError.
    /// </para>
    /// </remarks>
    FilteredEntireMessage = 0x2,

    /// <summary>
    /// The incoming text was unable to be filtered, and the entire text has been replaced by asterisks.
    /// </summary>
    /// <remarks>
    /// This value is exclusive to FilteredOffensiveTerms and FilteredEntireMessage.
    /// </remarks>
    FilteredDueToError = 0x4,
};

DEFINE_ENUM_FLAG_OPERATORS(PartyChatTextReceivedOptions);

/// <summary>
/// Configuration modes representing how the Party library performs automatic region discovery and connection quality
/// measurement as part of the <see cref="PartyRegionUpdateConfiguration" /> structure.
/// </summary>
/// <seealso cref="PartyOption::RegionUpdateConfiguration" />
/// <seealso cref="PartyRegionUpdateConfiguration" />
/// <seealso cref="PartyRegionsChangedStateChange" />
/// <seealso cref="PartyCreateNewNetworkCompletedStateChange" />
/// <seealso cref="PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange" />
/// <seealso cref="PartySetTextToSpeechProfileCompletedStateChange" />
/// <seealso cref="PartySetTranscriptionOptionsCompletedStateChange" />
/// <seealso cref="PartyManager::Initialize" />
/// <seealso cref="PartyManager::CreateNewNetwork" />
/// <seealso cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles" />
/// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
/// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
enum class PartyRegionUpdateMode
{
    /// <summary>
    /// Automatically begin retrieving the list of available regions and measuring connection quality to them as soon as
    /// possible.
    /// </summary>
    /// <remarks>
    /// The Party library automatically discovers the available regions and measures connection quality to them when
    /// PartyManager::Initialize() is first invoked.
    /// <para>
    /// The Party library also periodically refreshes the list and quality measurements until PartyManager::Cleanup() is
    /// called. If already connected or in the process of connecting to an existing Party network, then refreshing is
    /// delayed until disconnected from all Party networks. The periodic refresh interval can be configured using the
    /// <em>refreshIntervalInSeconds</em> field in the containing <see cref="PartyRegionUpdateConfiguration" />
    /// structure.
    /// </para>
    /// <para>
    /// The application is automatically provided a <see cref="PartyRegionsChangedStateChange" /> indicating available
    /// regions or the failure <see cref="PartyStateChangeResult" /> whenever a measurement operation completes.
    /// </para>
    /// <para>
    /// This mode is the default value when <see cref="PartyOption::RegionUpdateConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// </remarks>
    Immediate = 0,

    /// <summary>
    /// Delay retrieving the list of available regions and measuring connection quality to them until creating a new
    /// Party network or using certain text-to-speech or speech-to-text functionality.
    /// </summary>
    /// <remarks>
    /// The Party library doesn't automatically discover the available regions or measure connection quality to them
    /// unless the application calls <see cref="PartyManager::CreateNewNetwork()" /> with a region list array with zero
    /// entries, or isn't connected to an existing network but calls
    /// <see cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles()" />,
    /// <see cref="PartyLocalChatControl::SetTextToSpeechProfile()" />, or
    /// <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> with option flags that include
    /// <see cref="PartyVoiceChatTranscriptionOptions::TranscribeSelfRegardlessOfNetworkState" />. Once deferred region
    /// discovery and quality measurement start, a <see cref="PartyRegionsChangedStateChange" /> indicating available
    /// regions or the failure <see cref="PartyStateChangeResult" /> is provided prior to the triggering operation's
    /// <see cref="PartyCreateNewNetworkCompletedStateChange" />,
    /// <see cref="PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange" />,
    /// <see cref="PartySetTextToSpeechProfileCompletedStateChange" />, or
    /// <see cref="PartySetTranscriptionOptionsCompletedStateChange" /> completion state change.
    /// <para>
    /// On subsequent calls to the above functions with the described parameters, the region list is retrieved and
    /// measured again if the refresh interval has elapsed since the last measurements were successfully performed. If
    /// already connected or in the process of connecting to an existing Party network, then refreshing is delayed until
    /// disconnected from all Party networks. The refresh interval can be configured using the
    /// <em>refreshIntervalInSeconds</em> field in the containing <see cref="PartyRegionUpdateConfiguration" />
    /// structure.
    /// </para>
    /// <para>
    /// Chat controls internally use local device region latency estimates to optimize service usage for text-to-speech
    /// synthesis, speech-to-text transcription, and translation. Using PartyRegionUpdateMode::Deferred may result in
    /// these features experiencing higher latency.
    /// </para>
    /// </remarks>
    Deferred = 1,
};

/// <summary>
/// Categories of users associated with endpoints that can be used to filter the results returned by
/// PartyNetwork::GetEndpointsByUserType().
/// </summary>
/// <seealso cref="PartyNetwork::GetEndpointsByUserType" />
enum class PartyEndpointUserTypeFilter
{
    /// <summary>
    /// Retrieve all endpoints, regardless of associated user.
    /// </summary>
    NoUserFilter = 0,

    /// <summary>
    /// Retrieve only endpoints created by "title_player_account" entity type users.
    /// </summary>
    /// <remarks>
    /// Endpoints created by users of other entity types and endpoints with no associated user are not included in the
    /// retrieved array.
    /// </remarks>
    Players = 1,

    /// <summary>
    /// Retrieve only endpoints created by "game_server" entity type users.
    /// </summary>
    /// <remarks>
    /// Endpoints created by users of other entity types and endpoints with no associated user are not included in the
    /// retrieved array.
    /// </remarks>
    Servers = 2,
};

/// <summary>
/// Categories of owning device locations of endpoints that can be used to filter the results returned by
/// PartyNetwork::GetEndpointsByUserType().
/// </summary>
/// <seealso cref="PartyNetwork::GetEndpointsByUserType" />
enum class PartyEndpointLocationFilter
{
    /// <summary>
    /// Retrieve matching endpoints created by any device, whether local or remote.
    /// </summary>
    NoLocationFilter = 0,

    /// <summary>
    /// Retrieve only matching endpoints created by the local device.
    /// </summary>
    LocalOnly = 1,

    /// <summary>
    /// Retrieve only matching endpoints created by remote devices.
    /// </summary>
    RemoteOnly = 2,
};

/// <summary>
/// Initialization configuration for the Party library.
/// </summary>
/// <seealso cref="XTaskQueueCreate" />
/// <seealso cref="XTaskQueueDispatch()" />
/// <nyi />
struct PartyInitializationConfiguration
{
    /// <summary>
    /// The Title ID provided must be the same Title ID used to acquire the PlayFab Entity Handle that will be passed to
    /// <see cref="CreateLocalUser()" />.
    /// </summary>
    PartyString titleId;

    /// <summary>
    /// XTaskQueueHandle to be used for processing Party library audio tasks.
    /// </summary>
    /// <remarks>
    /// By default, the Party library will create and manage its own threads to process audio tasks. If the title
    /// prefers to create and manage its own threads, it must provide an XTaskQueueHandle. The Party library will rely
    /// on the the title calling <see cref="XTaskQueueDispatch()" /> on the provided XTaskQueueHandle to process the
    /// associated tasks. To create an XTaskQueueHandle, call <see cref="XTaskQueueCreate" />. When creating the
    /// XTaskQueue, the <see cref="XTaskQueueDispatchMode" /> should be set to
    /// <see cref="XTaskQueueDispatchMode::Manual" />. <see cref="XTaskQueueDispatchMode::ThreadPool" />,
    /// <see cref="XTaskQueueDispatchMode::SerializedThreadPool />, and <see cref="XTaskQueueDispatchMode::Immediate" /> dispatch modes are not supported and will result in undefined behavior. </remarks> 
    XTaskQueueHandle audioTaskQueue;

    /// <summary>
    /// XTaskQueueHandle to be used for processing Party library networking tasks.
    /// </summary>
    /// <remarks>
    /// By default, the Party library will create and manage its own threads to process networking tasks. If the title
    /// prefers to create and manage its own threads, it must provide an XTaskQueueHandle. The Party library will rely
    /// on the the title calling <see cref="XTaskQueueDispatch()" /> on the provided XTaskQueueHandle to process the
    /// associated tasks. To create an XTaskQueueHandle, call <see cref="XTaskQueueCreate" />. When creating the
    /// XTaskQueue, the <see cref="XTaskQueueDispatchMode" /> should be set to
    /// <see cref="XTaskQueueDispatchMode::Manual" />. <see cref="XTaskQueueDispatchMode::ThreadPool" />,
    /// <see cref="XTaskQueueDispatchMode::SerializedThreadPool />, and <see cref="XTaskQueueDispatchMode::Immediate" /> dispatch modes are not supported and will result in undefined behavior. </remarks> 
    XTaskQueueHandle networkingTaskQueue;
};

/// <summary>
/// The configuration used by the Party library to bind to a UDP socket.
/// </summary>
/// <remarks>
/// This structure can be used together with <see cref="PartyOption::LocalUdpSocketBindAddress" /> to either override or
/// query the Party library's current configuration via <see cref="PartyManager::SetOption()" /> or
/// <see cref="PartyManager::GetOption()" /> respectively.
/// </remarks>
/// <seealso cref="PartyOption::LocalUdpSocketBindAddress" />
/// <seealso cref="PartyLocalUdpSocketBindAddressOptions" />
/// <seealso cref="PartyManager::SetOption" />
/// <seealso cref="PartyManager::GetOption" />
struct PartyLocalUdpSocketBindAddressConfiguration
{
    /// <summary>
    /// Optional flags describing how to interpret this UDP socket configuration.
    /// </summary>
    PartyLocalUdpSocketBindAddressOptions options;

    /// <summary>
    /// The specific port number to which the local UDP socket will be bound the next time Party is initialized.
    /// </summary>
    /// <remarks>
    /// In the Microsoft Game Core version of the Party library, a port value of 0 means that the Party library selects
    /// the Game Core preferred local UDP multiplayer port unless the
    /// PartyLocalUdpSocketBindAddressOptions::ExcludeGameCorePreferredUdpMultiplayerPort option flag is specified in
    /// the <em>options</em> field. On all other versions of the Party library, a port value of 0 means the Party
    /// library lets the system dynamically select a port that's available on all local IP address interfaces.
    /// <para>
    /// If this port value can't be bound when the Party library is initialized,
    /// <see cref="PartyManager::Initialize()" /> synchronously returns an error. The human-readable form of the error
    /// code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </para>
    /// <para>
    /// The port should be specified in native host byte order. If your application also directly uses or is porting
    /// from its own socket API calls, be aware that this natural byte ordering may therefore differ from the network
    /// byte order used by socket address port numbers.
    /// </para>
    /// <para>
    /// The default value is 0 when <see cref="PartyOption::LocalUdpSocketBindAddress" /> hasn't been configured.
    /// </para>
    /// </remarks>
    uint16_t port;
};

/// <summary>
/// The configuration used by the Party library to control how it performs automatic region discovery and connection
/// quality measurement updates.
/// </summary>
/// <remarks>
/// This structure can be used together with <see cref="PartyOption::RegionUpdateConfiguration" /> to either override or
/// query the Party library's current configuration via <see cref="PartyManager::SetOption()" /> or
/// <see cref="PartyManager::GetOption()" /> respectively. It configures the behavior of the region discovery and
/// connection quality measurement process that produces <see cref="PartyRegionsChangedStateChange" /> state changes.
/// </remarks>
/// <seealso cref="PartyOption::RegionUpdateConfiguration" />
/// <seealso cref="PartyRegionUpdateMode" />
/// <seealso cref="PartyRegionsChangedStateChange" />
/// <seealso cref="PartyManager::SetOption" />
/// <seealso cref="PartyManager::GetOption" />
struct PartyRegionUpdateConfiguration
{
    /// <summary>
    /// The mode controlling when region discovery and connection quality measurement updates are performed.
    /// </summary>
    /// <remarks>
    /// The default value is <see cref="PartyRegionUpdateMode::Immediate" /> when
    /// <see cref="PartyOption::RegionUpdateConfiguration" /> hasn't been configured.
    /// </remarks>
    PartyRegionUpdateMode mode;

    /// <summary>
    /// The number of seconds allowed to elapse after region quality measurements complete before a new region discovery
    /// and connection quality measurement process may begin.
    /// </summary>
    /// <remarks>
    /// This refresh interval is used whenever a <see cref="PartyRegionsChangedStateChange" /> is provided to determine
    /// when the next region quality measurement is eligible to start under the update mode defined by the <em>mode</em>
    /// field.
    /// <para>
    /// Becoming eligible for a refresh doesn't guarantee that the refresh begins at that exact time. If the
    /// <em>mode</em> field is set to <see cref="PartyRegionUpdateMode::Deferred" />, the next refresh doesn't happen
    /// until one of the applicable methods is called to trigger a region update. The library also waits until the local
    /// device is no longer in any Party networks and applies a small amount of randomized delay to try to avoid PlayFab
    /// service load from multiple, unintentionally synchronized devices.
    /// </para>
    /// <para>
    /// The default value is 28800 (8 hours) when <see cref="PartyOption::RegionUpdateConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// <para>
    /// A value of zero causes the Party library to never automatically refresh measurements in the future. Otherwise,
    /// the minimum allowed value is 30 seconds.
    /// </para>
    /// </remarks>
    uint32_t refreshIntervalInSeconds;
};

/// <summary>
/// The configuration used by the Party library to control region quality measurement behavior.
/// </summary>
/// <remarks>
/// This structure can be used together with <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> to either
/// override or query the Party library's current configuration via <see cref="PartyManager::SetOption()" /> or
/// <see cref="PartyManager::GetOption()" /> respectively. It configures the low-level behavior of the region quality
/// measurement process associated with <see cref="PartyRegionsChangedStateChange" /> state changes.
/// </remarks>
/// <seealso cref="PartyOption::RegionQualityMeasurementConfiguration" />
/// <seealso cref="PartyRegionsChangedStateChange" />
/// <seealso cref="PartyManager::SetOption" />
/// <seealso cref="PartyManager::GetOption" />
struct PartyRegionQualityMeasurementConfiguration
{
    /// <summary>
    /// The maximum number of milliseconds to allow for resolving hostnames and attempting to measure networking
    /// conditions between the local device and PlayFab Quality of Service beacon servers.
    /// </summary>
    /// <remarks>
    /// This value controls the maximum allowed time, in milliseconds, for the Party library to resolve the PlayFab
    /// Quality of Service beacon server hostnames for all available regions and to send lightweight messages to measure
    /// connection quality to them. If environmental issues cause slow name resolution or high latency/packet loss to
    /// some regions, then only those regions that were successfully contacted at least once within this timeout are
    /// reported in the <see cref="PartyRegionsChangedStateChange" /> result.
    /// <para>
    /// This value does not include or affect the time it takes for the Party library to retrieve the list of available
    /// regions from the PlayFab service.
    /// </para>
    /// <para>
    /// Applications should typically define this timeout in terms of user experience goals/worst cases rather than
    /// attempting to derive some number based on technical assumptions. The exact amount of time required to measure
    /// any given region or all regions can vary greatly depending on factors like the number of available regions, the
    /// current networking environment conditions, the Party library's dynamically tuned timeouts based on observed
    /// latency, and more.
    /// </para>
    /// <para>
    /// This value is effectively a worst case timeout for devices that experience complete or partial failures. This
    /// may be useful to contrast with real-world observations of typical successes. Using the default
    /// PartyRegionQualityMeasurementConfiguration values, region availability, and player usage patterns, the Party
    /// library currently observes that most devices worldwide complete the entire measurement process successfully in
    /// 2.3 seconds or less, and 97% of devices successfully complete in less than 6 seconds. This duration is expected
    /// to increase slightly over time as more regions are introduced.
    /// </para>
    /// <para>
    /// The default value is 15000 (15 seconds) when <see cref="PartyOption::RegionQualityMeasurementConfiguration" />
    /// hasn't been configured.
    /// </para>
    /// <para>
    /// A value of zero means there's no timeout, and all measurements are therefore allowed to complete regardless of
    /// how long any regions take to succeed or fail. Otherwise, the minimum value is 50 milliseconds.
    /// </para>
    /// </remarks>
    uint32_t totalMeasurementTimeoutInMilliseconds;

    /// <summary>
    /// A hint indicating the highest latency threshold, in milliseconds, above which the application deems a region to
    /// be too latent to spend additional time performing detailed measurement.
    /// </summary>
    /// <remarks>
    /// This target value represents a soft upper limit for region latency. It doesn't directly prevent including
    /// regions measured to have higher latency, but it does cause the Party library to choose timeout and retry values
    /// optimized for regions with less latency than this number. It's also inspected for each region after the minimum
    /// number of responses (configured in the <em>minRequiredSuccessfulResponses</em> field) is received. If the median
    /// measured latency for a region is greater than this target, then the ideal number of responses (configured in the
    /// <em>idealNumberOfSuccessfulResponses</em> field) isn't used and no more measurement attempts are sent.
    /// <para>
    /// This value can't be zero.
    /// </para>
    /// <para>
    /// The default value is 200 when <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// </remarks>
    uint16_t highLatencyHintInMilliseconds;

    /// <summary>
    /// The minimum number of successful measurement message responses needed for each region for basic evaluation.
    /// </summary>
    /// <remarks>
    /// Each potential region to be measured is expected to respond successfully at least this many times during the
    /// quality measurement lightweight message exchange before results are considered accurate enough to evaluate
    /// against the <em>highLatencyHintInMilliseconds</em> field target latency. Setting larger minimum requirements
    /// results in more measurement samples, and therefore more consistency and accuracy in the face of any outlier
    /// packet fluctuations, at the expense of taking more time to complete.
    /// <para>
    /// Even without sufficient successful responses, measuring a particular region is considered complete if any of the
    /// following occur: no successful responses arrive at all and the number of attempts exceeds the value configured
    /// in the <em>maxRetriesWithNoResponse</em> field, at least one response arrived and the number of attempts exceeds
    /// the value configured in the <em>maxTimeoutsAfterResponse</em> field, or the overall timeout for all regions
    /// configured in the <em>totalMeasurementTimeoutInMilliseconds</em> field elapses.
    /// </para>
    /// <para>
    /// This value can't be zero.
    /// </para>
    /// <para>
    /// The default value is 3 when <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// <para>
    /// </remarks>
    uint16_t minRequiredSuccessfulResponses;

    /// <summary>
    /// The number of successful measurement message responses desired for each region in order to achieve thorough
    /// evaluation.
    /// </summary>
    /// <remarks>
    /// This value represents the total preferred number of successful responses to achieve the desired accuracy in
    /// latency measurements for regions of interest. This value must be the same or higher than the minimum value in
    /// the <em>minRequiredSuccessfulResponses</em> field. The Party library only attempts to obtain the additional
    /// response count difference between the minimum and this ideal value for a region if that region was already
    /// measured to have a median latency less than or equal to the threshold configured in the
    /// <em>highLatencyHintInMilliseconds</em> field. This field's ideal number of responses allows continuing to refine
    /// measurement accuracy for regions likely to be of more value to the local device without also spending that
    /// additional time for the higher latency regions.
    /// <para>
    /// If the minimum number of responses for a region wasn't obtained, then this additional ideal target isn't
    /// attempted. Otherwise, if the <em>maxTimeoutsAfterResponse</em> limit for the region is reached or the overall
    /// <em>totalMeasurementTimeoutInMilliseconds</em> field timeout for all regions elapses, then measuring the region
    /// is considered complete even without receiving this ideal number of successful responses.
    /// </para>
    /// <para>
    /// The default value is 4 when <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// </remarks>
    uint16_t idealNumberOfSuccessfulResponses;

    /// <summary>
    /// The maximum number of additional times to resend an initial measurement message to a region without any
    /// responses before considering the region unreachable.
    /// </summary>
    /// <remarks>
    /// This value configures the total number of additional attempts beyond the first that the Party library makes when
    /// retrying the first quality measurement lightweight message to a region before considering that region
    /// unavailable. If at least one successful response is received from the region's Quality of Service beacon server,
    /// then this maximum is no longer used and instead the <em>maxTimeoutsAfterResponse</em> field governs the number
    /// of attempts allowed to time out.
    /// <para>
    /// Larger maximum retry values are more tolerant of temporary environmental issues such as packet loss, at the
    /// expense of taking longer to abandon unreachable regions. The absolute maximum time permitted for measuring all
    /// regions is capped by the separate <em>totalMeasurementTimeoutInMilliseconds</em> field value.
    /// </para>
    /// <para>
    /// The default value is 3 when <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// </remarks>
    uint16_t maxRetriesWithNoResponse;

    /// <summary>
    /// The maximum number of measurement message attempts allowed to time out for a region that has previously
    /// responded before considering the networking path too unreliable to continue measuring.
    /// </summary>
    /// <remarks>
    /// If at least one successful response has been received from a region's Quality of Service beacon server, this
    /// value determines the total number of timed-out attempts the Party library will permit while trying to obtain
    /// <em>minRequiredSuccessfulResponses</em> and <em>idealNumberOfSuccessfulResponses</em> measurement results for
    /// the region.
    /// <para>
    /// Larger maximum timeout values are more tolerant of temporary environmental issues such as packet loss, at the
    /// expense of potentially taking longer to finish measuring regions with those poor quality characteristics. The
    /// absolute maximum time permitted for measuring all regions is capped by the separate
    /// <em>totalMeasurementTimeoutInMilliseconds</em> field value.
    /// </para>
    /// <para>
    /// The default value is 3 when <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> hasn't been
    /// configured.
    /// </para>
    /// </remarks>
    uint16_t maxTimeoutsAfterResponse;
};

/// <summary>
/// A descriptor containing the data required for a device to connect to a network.
/// </summary>
struct PartyNetworkDescriptor
{
    /// <summary>
    /// A unique identifier for the network.
    /// </summary>
    /// <remarks>
    /// This identifier can be used to correlate locally observed PartyNetwork behavior with remote telemetry gathered
    /// by the Party service and transparent cloud relay. It is recommended that this identifier be recorded alongside
    /// any other information the title records to diagnose network behavior.
    /// </remarks>
    char networkIdentifier[c_networkIdentifierStringLength + 1];

    /// <summary>
    /// The Azure region in which the network was created.
    /// </summary>
    char regionName[c_maxRegionNameStringLength + 1];

    /// <summary>
    /// Connection information for the network used internally by the Party library.
    /// </summary>
    uint8_t opaqueConnectionInformation[c_opaqueConnectionInformationByteCount];
};

/// <summary>
/// Network configuration data.
/// </summary>
/// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
/// <seealso cref="PartyAuthenticateLocalUserCompletedStateChange" />
/// <seealso cref="PartyManager::ConnectToNetwork" />
/// <seealso cref="PartyConnectToNetworkCompletedStateChange" />
/// <seealso cref="PartyNetwork::CreateEndpoint" />
/// <seealso cref="PartyNetworkDestroyedStateChange" />
/// <seealso cref="PartyStateChangeResult" />
/// <seealso cref="PartyDirectPeerConnectivityOptions" />
struct PartyNetworkConfiguration
{
    /// <summary>
    /// The maximum number of unique users allowed in the network.
    /// </summary>
    /// <remarks>
    /// This value must be greater than 0.
    /// <para>
    /// If a client would violate this limit by calling <see cref="PartyNetwork::AuthenticateLocalUser()" />, the
    /// operation will fail asynchronously and <see cref="PartyAuthenticateLocalUserCompletedStateChange::result" />
    /// will be set to <see cref="PartyStateChangeResult::NetworkLimitReached" />.
    /// </para>
    /// </remarks>
    uint32_t maxUserCount;

    /// <summary>
    /// The maximum number of devices allowed to connect to the network.
    /// </summary>
    /// <remarks>
    /// PlayFab Party networks can scale to support anywhere between 2 and 128 devices. The service will choose a
    /// network relay configuration optimized for your scenario, so it's important to configure the
    /// <c>maxDeviceCount</c> in <c>PartyNetworkConfiguration</c> to match the max expected number of devices in your
    /// network.
    /// <para>
    /// If a client would violate this limit by calling <see cref="PartyManager::ConnectToNetwork()" />, the operation
    /// will fail asynchronously and <see cref="PartyConnectToNetworkCompletedStateChange::result" /> will be set to
    /// <see cref="PartyStateChangeResult::NetworkLimitReached" />.
    /// </para>
    /// </remarks>
    _Field_range_(1, c_maxNetworkConfigurationMaxDeviceCount) uint32_t maxDeviceCount;

    /// <summary>
    /// The maximum number of users allowed per device.
    /// </summary>
    /// <remarks>
    /// This value must be between 1 and <c>c_maxLocalUsersPerDeviceCount</c> inclusive.
    /// <para>
    /// If a client would violate this limit by calling <see cref="PartyNetwork::AuthenticateLocalUser()" />, the
    /// operation will fail asynchronously and <see cref="PartyAuthenticateLocalUserCompletedStateChange::result" />
    /// will be set to <see cref="PartyStateChangeResult::NetworkLimitReached" />.
    /// </para>
    /// </remarks>
    _Field_range_(1, c_maxLocalUsersPerDeviceCount) uint32_t maxUsersPerDeviceCount;

    /// <summary>
    /// The maximum number of devices allowed per user.
    /// </summary>
    /// <remarks>
    /// This value must be greater than 0.
    /// <para>
    /// If a client would violate this limit by calling <see cref="PartyNetwork::AuthenticateLocalUser()" />, the
    /// operation will fail asynchronously and <see cref="PartyAuthenticateLocalUserCompletedStateChange::result" />
    /// will be set to <see cref="PartyStateChangeResult::NetworkLimitReached" />.
    /// </para>
    /// </remarks>
    uint32_t maxDevicesPerUserCount;

    /// <summary>
    /// The maximum number of endpoints allowed per device.
    /// </summary>
    /// <remarks>
    /// This value must be between 0 and <c>c_maxNetworkConfigurationMaxEndpointsPerDeviceCount</c> inclusive.
    /// <para>
    /// If a client would violate this limit by calling <see cref="PartyNetwork::CreateEndpoint()" /> after the network
    /// configuration was made available, the call to PartyNetwork::CreateEndpoint() will fail synchronously. If the
    /// client had already queued a violating number of endpoint creations when the network configuration became
    /// available, the client will be kicked from the network and a <see cref="PartyNetworkDestroyedStateChange" /> will
    /// be generated.
    /// </para>
    /// </remarks>
    _Field_range_(0, c_maxNetworkConfigurationMaxEndpointsPerDeviceCount) uint32_t maxEndpointsPerDeviceCount;

    /// <summary>
    /// Whether and how to support direct peer-to-peer connection attempts among devices in the network.
    /// </summary>
    /// <remarks>
    /// As part of successfully authenticating an initial user into a network, a device may attempt to establish direct
    /// peer-to-peer connections with other devices already participating in the network when permitted by these flags.
    /// For attempts that are successful, endpoint messages and chat data between the devices will be transmitted using
    /// those direct connections. For attempts that fail due to environmental incompatibilities between the devices, all
    /// communication between those devices will be transmitted via transparent cloud relay servers instead. If the
    /// devices aren't permitted to attempt direct peer connections by these flags, then they never exchange IP address
    /// information and will always transmit endpoint messages and chat data via transparent cloud relay servers.
    /// <para>
    /// You can determine whether the local device actually established a direct peer-to-peer connection to a specific
    /// remote device by calling <see cref="PartyNetwork::GetDeviceConnectionType()" />.
    /// </para>
    /// <para>
    /// Successful direct peer connectivity may provide lower latency between some devices, though attempting to
    /// establish it also requires users to disclose their devices' IP addresses to others, which may be a concern for
    /// privacy or for enabling malicious users to potentially attack peers' devices and Internet connections outside of
    /// the title. It also may not be permitted on certain platforms for policy reasons. Be sure to use the appropriate
    /// flags for your performance and security goals.
    /// </para>
    /// <para>
    /// Besides the specific network's value configured here, the flags may optionally be further constrained by a
    /// device for all networks into which it authenticates by using <see cref="PartyManager::SetOption()" /> to set
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" />. All flags are evaluated using a
    /// bitwise AND operation. That is, a particular flag is actually only in effect for a given network's pair of
    /// devices if it's enabled in three places: this field for the network, and <em>both</em> devices' respective local
    /// mask options. Even if this field permits direct peer connectivity of the relevant form, either device can
    /// unilaterally opt out of the IP address disclosure and direct connection attempts between them by not enabling
    /// the flag in its local device mask option. The
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> value defaults to permitting all direct
    /// peer connections enabled by networks, so you only need to configure it if you have device-specific requirements
    /// to prevent some or all direct peer connectivity involving it.
    /// </para>
    /// <para>
    /// To avoid excessive resource consumption, the Party library will also internally prevent any given device from
    /// attempting to establish more than a configured maximum number of direct peer connections across all networks in
    /// which it's currently participating, even if permitted by these flags. This doesn't affect the device's ability
    /// to participate in large or multiple networks with additional remote devices. Communication with additional
    /// devices will simply be transmitted via transparent cloud relay servers. To modify that maximum number of direct
    /// peer connections, use <see cref="PartyManager::SetOption()" /> to set
    /// <see cref="PartyOption::LocalDeviceMaxDirectPeerConnections" />.
    /// </para>
    /// <para>
    /// It's recommended that you don't actively enforce the availability of a direct peer-to-peer connection for any
    /// given pair of devices (i.e., don't call <see cref="PartyNetwork::LeaveNetwork()" /> if
    /// <see cref="PartyNetwork::GetDeviceConnectionType()" /> reports a value other than
    /// <see cref="PartyDeviceConnectionType::DirectPeerConnection" />) since the specific underlying transmission
    /// method in use doesn't alter the overall logical ability to communicate. If your game design has stringent
    /// requirements for maximum message latency that encourage direct peer connectivity, it's better to take action on
    /// the current concrete observations of that latency as reported by the
    /// <see cref="PartyEndpointStatistic::AverageDeviceRoundTripLatencyInMilliseconds" /> statistic rather than make
    /// abstract assumptions based on transmission mechanism. Otherwise you might continually hinder users trying to
    /// play with the same set of friends who always need to use nearby transparent cloud relay servers due to
    /// environmental factors beyond their control.
    /// </para>
    /// </remarks>
    PartyDirectPeerConnectivityOptions directPeerConnectivityOptions;
};

/// <summary>
/// Represents a Party library region.
/// </summary>
struct PartyRegion
{
    /// <summary>
    /// The name of the Azure region, such as "eastus2".
    /// </summary>
    /// <remarks>
    /// This name is not localized to the current user's language, and showing the string directly in UI is not
    /// recommended outside of troubleshooting.
    /// </remarks>
    char regionName[c_maxRegionNameStringLength + 1];

    /// <summary>
    /// The round trip latency between the local device and the region's datacenter.
    /// </summary>
    uint32_t roundTripLatencyInMilliseconds;
};

/// <summary>
/// Invitation configuration data.
/// </summary>
/// <seealso cref="PartyInvitation" />
/// <seealso cref="PartyInvitationRevocability" />
/// <seealso cref="PartyManager::CreateNewNetwork" />
/// <seealso cref="PartyNetwork::CreateInvitation" />
/// <seealso cref="PartyInvitation::GetInvitationConfiguration" />
struct PartyInvitationConfiguration
{
    /// <summary>
    /// The identifier of the invitation.
    /// </summary>
    /// <remarks>
    /// This value may only be nullptr when used as input to <see cref="PartyManager::CreateNewNetwork()" /> or
    /// <see cref="PartyNetwork::CreateInvitation()" />.
    /// <para>
    /// The length of this string must be less than or equal to <c>c_maxInvitationIdentifierStringLength</c>.
    /// </para>
    /// </remarks>
    _Maybenull_ PartyString identifier;

    /// <summary>
    /// The revocability of the invitation.
    /// </summary>
    PartyInvitationRevocability revocability;

    /// <summary>
    /// The number of PlayFab Entity IDs that the invitation allows to authenticate into the network.
    /// </summary>
    /// <remarks>
    /// If this value is 0, all users are allowed to authenticate using the invitation's identifier.
    /// <para>
    /// This value must be less than or equal to <c>c_maxInvitationEntityIdCount</c>.
    /// </para>
    /// </remarks>
    _Field_range_(0, c_maxInvitationEntityIdCount) uint32_t entityIdCount;

    /// <summary>
    /// The list of PlayFab Entity IDs that the invitation allows to authenticate into the network.
    /// </summary>
    /// <remarks>
    /// If this list is empty, all users are allowed to authenticate using the invitation's identifier.
    /// </remarks>
    _Field_size_(entityIdCount) const PartyString * entityIds;
};

/// <summary>
/// Optional configuration parameters for modifying local queuing behavior when sending a message.
/// </summary>
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
/// <seealso_nyi cref="PartyLocalEndpoint::CancelMessages" />
struct PartySendMessageQueuingConfiguration
{
    /// <summary>
    /// The priority of the locally queued message in relation to chat data or messages sent from other local endpoints.
    /// </summary>
    /// <remarks>
    /// <c>priority</c> must be a value between <c>c_minSendMessageQueuingPriority</c> and
    /// <c>c_maxSendMessageQueuingPriority</c>, inclusive. Higher numbers represent higher relative priority (will be
    /// transmitted first) over lower numbers. The default when no PartySendMessageQueuingConfiguration structure is
    /// provided to <see cref="PartyLocalEndpoint::SendMessage()" /> is <c>c_defaultSendMessageQueuingPriority</c>,
    /// which lies exactly in the middle of the signed integer range (zero).
    /// <para>
    /// Send queue priority does not modify the order in which messages sent from a given local endpoint are transmitted
    /// or delivered. It only affects the ordering of messages from different local endpoints, or between an endpoint
    /// and internally transmitted chat data, which uses priority <c>c_chatSendMessageQueuingPriority</c>. The local
    /// endpoint send queue that has the messages with the highest priority value will have its messages eligible for
    /// transmission first. If all remaining messages queued by local endpoints are of equal priority, then the
    /// individual messages will be eligible for transmission in the same order in which their
    /// PartyLocalEndpoint::SendMessage() calls occurred.
    /// </para>
    /// <para>
    /// One way to think of the behavior is that high priority messages automatically elevate the priority of all
    /// previous messages from the same local endpoint to the same value to ensure timely delivery of the high priority
    /// message but without breaking the sequential delivery expectations of the messages previously transmitted by the
    /// local endpoint. Other local endpoints have their own send sequence, and thus have no such ordering guarantees
    /// (though the Party library makes a best-effort attempt to preserve the ordering in which they were originally
    /// sent where possible).
    /// </para>
    /// <para>
    /// A message's send queue priority matters when there are more bytes to be sent than the connection can support due
    /// to connection quality or receiver responsiveness. Sending more important messages as higher priority ensures
    /// that they get the first opportunity to use the limited network resources. If you continue to send messages
    /// faster than can successfully be transmitted, lower priority messages may get "starved" and the associated local
    /// endpoint send queue will continue to grow. Be sure to manage such potential growth by canceling extraneous
    /// messages with PartyLocalEndpoint::CancelMessages(), using the <c>timeoutInMilliseconds</c> field to
    /// automatically time out messages that have been queued for too long, or simply reducing the size and/or frequency
    /// of the PartyLocalEndpoint::SendMessage() calls.
    /// </para>
    /// </remarks>
    _Field_range_(c_minSendMessageQueuingPriority, c_maxSendMessageQueuingPriority) int8_t priority;

    /// <summary>
    /// A caller-defined value to use when evaluating the message for applicability with cancel filter expressions.
    /// </summary>
    /// <remarks>
    /// This message identity value can have any caller-specific meaning and is not interpreted by the Party library
    /// other than for use by PartyLocalEndpoint::CancelMessages() when evaluating whether the message matches the
    /// optionally provided cancel filter expression.
    /// <para>
    /// A default value of zero is used when no PartySendMessageQueuingConfiguration structure is provided to
    /// <see cref="PartyLocalEndpoint::SendMessage()" />.
    /// </para>
    /// <para>
    /// Canceling messages can help prevent the local send queue from growing excessively when experiencing poor network
    /// conditions. Canceling message identities that match certain filter expressions works well when you have
    /// categories of messages that are regularly sent with the latest information and should replace any previously
    /// queued out-of-date message that might still be awaiting a transmission opportunity. Canceling can also be useful
    /// when you have an instance or category of opportunistic messages whose loss would not be fatal and that aren't
    /// worth the bandwidth to try transmitting instead of more valuable queued messages.
    /// </para>
    /// <para>
    /// This local identity value is not part of the transmitted data payload. It has no applicability once the message
    /// begins transmitting and can no longer be removed from the local send queue by
    /// PartyLocalEndpoint::CancelMessages().
    /// </para>
    /// </remarks>
    uint32_t identityForCancelFilters;

    /// <summary>
    /// The maximum time, in milliseconds, that the message is permitted to remain in a Party-managed send queue
    /// awaiting a transmission opportunity.
    /// </summary>
    /// <remarks>
    /// If the message has not started transmitting when this timeout elapses due to connection quality or receiver
    /// responsiveness, the message will be aborted and removed from the queue without being sent.
    /// <para>
    /// A <c>timeoutInMilliseconds</c> value of zero indicates that there should be no timeout and that the message
    /// should remain queued until it is either successfully transmitted, is explicitly canceled, or encounters some
    /// transmission failure such as remote disconnection. Zero is the default when no
    /// PartySendMessageQueuingConfiguration structure is provided to <see cref="PartyLocalEndpoint::SendMessage()" />.
    /// </para>
    /// <para>
    /// Message send queue timeouts can help prevent send queues from growing excessively when experiencing poor network
    /// conditions. They work well with messages that contain time-sensitive, periodic data where it would be a waste of
    /// bandwidth to transmit ones that are stale because a newer complete replacement message is sent regularly, and
    /// the loss of any individual one is not fatal.
    /// </para>
    /// <para>
    /// This timeout value only affects Party-managed send queuing. It does not affect the time it takes to actually
    /// transmit a message (environmental latency) nor alter how long to wait for the receiver to acknowledge the
    /// transmission if applicable.
    /// </para>
    /// <para>
    /// This timeout value is evaluated twice when sending to targets without direct peer connections: once for the
    /// sending client's local send queues to the transparent cloud relay, affected by local environmental conditions
    /// and transmission rates to the relay, and a second time on the relay itself, which may be forced to queue
    /// messages before forwarding based on differing network conditions, transmission rates, or responsiveness of the
    /// remote targets.
    /// </para>
    /// </remarks>
    uint32_t timeoutInMilliseconds;
};

/// <summary>
/// A data buffer.
/// </summary>
struct PartyDataBuffer
{
    /// <summary>
    /// The data buffer.
    /// </summary>
    _Field_size_bytes_(bufferByteCount) const void * buffer;

    /// <summary>
    /// The size of the buffer in bytes.
    /// </summary>
    uint32_t bufferByteCount;
};

/// <summary>
/// A data buffer that can be modified by the app.
/// </summary>
struct PartyMutableDataBuffer
{
    /// <summary>
    /// The data buffer.
    /// </summary>
    _Field_size_bytes_(bufferByteCount) void * buffer;

    /// <summary>
    /// The size of the buffer in bytes.
    /// </summary>
    uint32_t bufferByteCount;
};

/// <summary>
/// A translation.
/// </summary>
struct PartyTranslation
{
    /// <summary>
    /// Indicates whether the translation operation succeeded.
    /// </summary>
    /// <remarks>
    /// On success, <c>translation</c> is a string of nonzero length containing the translated text. On failure, the
    /// string is empty.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The language code of the translation.
    /// </summary>
    /// <remarks>
    /// The language code is in BCP 47 format, such as en-US for English (United States). Supported language codes are
    /// enumerated in
    /// <see xref="https://learn.microsoft.com/azure/ai-services/speech-service/language-support" name="Language support" />.
    /// </remarks>
    PartyString languageCode;

    /// <summary>
    /// Additional options describing the translation.
    /// </summary>
    PartyTranslationReceivedOptions options;

    /// <summary>
    /// The translation string.
    /// </summary>
    /// <remarks>
    /// By default, profanity masking is enabled and replaces each character of a profane word with an asterisk.
    /// Profanity masking is applied after translating the unmasked source transcription. Depending on context, it's
    /// possible for the transcription to contain masked profanity but not the translation. Similarly, it's possible for
    /// the translation to contain masked profanity but not the original transcription. Profanity masking can be
    /// disabled via <see cref="PartyVoiceChatTranscriptionOptions::DisableProfanityMasking" />.
    /// <para>
    /// The translation string may be up to <c>c_maxChatTextMessageLength</c> characters long, not including the null
    /// terminator. Truncation occurs if the translated string length would exceed that limit, which can happen due to
    /// language differences even though the original string length is less than or equal to
    /// <c>c_maxChatTextMessageLength</c>. In such a case, <c>options</c> contains
    /// <see cref="PartyTranslationReceivedOptions::Truncated" />. Truncation may occur at an arbitrary point in the
    /// UTF-8 byte sequence and may not result in a complete, valid character or word. Strings are always null
    /// terminated, even when truncated.
    /// </para>
    /// </remarks>
    PartyString translation;
};

/// <summary>
/// The format information needed to interpret Party audio data.
/// </summary>
struct PartyAudioFormat
{
    /// <summary>
    /// Specifies the sample frequency at which each channel should be played or recorded.
    /// </summary>
    uint32_t samplesPerSecond;

    /// <summary>
    /// Overrides the assignment of channels in a multichannel audio stream to speaker positions.
    /// </summary>
    uint32_t channelMask;

    /// <summary>
    /// Specifies the number of channels of audio data.
    /// </summary>
    uint16_t channelCount;

    /// <summary>
    /// Specifies the number of bits per sample. If this value is not byte-divisible, it will be assumed that the
    /// containing sample type is padded with bits to make it byte-divisble.
    /// </summary>
    uint16_t bitsPerSample;

    /// <summary>
    /// Specifies the sample type.
    /// </summary>
    PartyAudioSampleType sampleType;

    /// <summary>
    /// A flag representing whether the multichannel audio data is interleaved for multi-channel formats.
    /// </summary>
    PartyBool interleaved;
};

/// <summary>
/// The configuration information needed to set up an audio source stream.
/// </summary>
/// <seealso cref="PartyChatControl::ConfigureAudioManipulationVoiceStream" />
struct PartyAudioManipulationSourceStreamConfiguration
{
    /// <summary>
    /// Optionally specifies the format of the audio that should be produced by the source stream.
    /// </summary>
    /// <remarks>
    /// If this value is nullptr, the source stream will produce audio in the format most efficient for the library. The
    /// format can be queried via <see cref="PartyAudioManipulationSourceStream::GetFormat()" />.
    /// </remarks>
    _Maybenull_ PartyAudioFormat * format;

    /// <summary>
    /// The maximum total size of audio buffers that can concurrently exist for this queue, in milliseconds.
    /// </summary>
    /// <remarks>
    /// This defines the limit for the total amount of audio internally queued by the source stream, but not yet
    /// retrieved via <see cref="PartyAudioManipulationSourceStream::GetNextBuffer" />, plus the total amount of audio
    /// retrieved by the app, but not yet returned to the library via
    /// <see cref="PartyAudioManipulationSourceStream::ReturnBuffer" />. When this total is reached, the source stream
    /// will stop producing additional buffers.
    /// <para>
    /// Because the library processes audio in 40 millisecond intervals, the effective maximum is the nearest multiple
    /// of 40 less than the specified maximum. The minimum is 40 milliseconds.
    /// </para>
    /// </remarks>
    uint32_t maxTotalAudioBufferSizeInMilliseconds;
};

/// <summary>
/// The configuration information needed to set up an audio sink stream.
/// </summary>
/// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationCaptureStream" />
/// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationRenderStream" />
struct PartyAudioManipulationSinkStreamConfiguration
{
    /// <summary>
    /// Optionally specifies the format of the audio that will be submitted to the sink stream.
    /// </summary>
    /// <remarks>
    /// If this value is nullptr, the sink stream will be configured to use the format most efficient for the library.
    /// The format can be queried via <see cref="PartyAudioManipulationSinkStream::GetFormat()" />.
    /// </remarks>
    _Maybenull_ PartyAudioFormat * format;

    /// <summary>
    /// The maximum total size of audio buffers that can concurrently exist for this queue, in milliseconds.
    /// </summary>
    /// <remarks>
    /// This defines the limit for the total amount of audio internally queued by the sink stream, but not yet processed
    /// by the library. When this limit is reached, the sink stream will stop accepting additional buffers.
    /// <para>
    /// This value should be chosen based on how frequently the game will submit buffers to the sink stream and how
    /// large those buffers will be. Because the library processes audio in 40 milliseconds intervals, the minimum is 40
    /// milliseconds. Most games will find 200 milliseconds to be a reasonable value.
    /// </para>
    /// </remarks>
    uint32_t maxTotalAudioBufferSizeInMilliseconds;
};

/// <summary>
/// A generic, base structure representation of an event or change in state in the Party library.
/// </summary>
/// <remarks>
/// PartyStateChange structures are reported by <see cref="PartyManager::StartProcessingStateChanges()" /> for the title
/// to handle and then promptly pass back via the <see cref="PartyManager::FinishProcessingStateChanges()" /> method.
/// <para>
/// The <c>stateChangeType</c> field indicates which kind of state change occurred, and this base structure should then
/// be cast to a more specific derived structure to retrieve additional event-specific information.
/// </para>
/// </remarks>
/// <seealso cref="PartyManager::StartProcessingStateChanges" />
/// <seealso cref="PartyManager::FinishProcessingStateChanges" />
struct PartyStateChange
{
    /// <summary>
    /// The specific type of the state change represented.
    /// </summary>
    /// <remarks>
    /// Use this field to determine which corresponding derived structure is represented by this PartyStateChange
    /// structure header.
    /// </remarks>
    PartyStateChangeType stateChangeType;
};

/// <summary>
/// Information specific to the <em>RegionsChanged</em> type of state change.
/// </summary>
/// <remarks>
/// Retrieving the list of available regions and measuring connection quality to them occurs automatically in the
/// background. This operation is first initiated when <see cref="PartyManager::Initialize()" /> is called unless the
/// <see cref="PartyOption::RegionUpdateConfiguration" /> option was used to configure an update mode of
/// <see cref="PartyRegionUpdateMode::Deferred" />, in which case region updates may not happen at all. Deferred region
/// updates only occur if the application calls <see cref="PartyManager::CreateNewNetwork()" /> with a zero entry region
/// array, or isn't connected to an existing network but calls
/// <see cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles()" />,
/// <see cref="PartyLocalChatControl::SetTextToSpeechProfile()" />, or
/// <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> with option flags that include
/// <see cref="PartyVoiceChatTranscriptionOptions::TranscribeSelfRegardlessOfNetworkState" />. The
/// <see cref="PartyOption::RegionUpdateConfiguration" /> option can also be used to configure the interval after which
/// the regions are refreshed.
/// <para>
/// The amount of time it takes to retrieve the list of available regions and measure connection quality to them is
/// dependent on many environmental or dynamic factors, and whether the
/// <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> option was used to modify low-level measurement
/// settings. The <em>totalMeasurementTimeoutInMilliseconds</em> field in the
/// <see cref="PartyRegionQualityMeasurementConfiguration" /> structure is used to configure the worst case maximum
/// timeout before considering all measurements complete and generating this state change.
/// </para>
/// </remarks>
/// <seealso cref="PartyManager::GetRegions" />
struct PartyRegionsChangedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates whether a background operation to query the list of supported regions and the latency to each region
    /// succeeded, or provides the reason that it failed.
    /// </summary>
    /// <remarks>
    /// When the result is <see cref="PartyStateChangeResult::Succeeded" />, the region list provided by
    /// <see cref="PartyManager::GetRegions()" /> is populated with the results of the operation. On failure, the region
    /// list provided by PartyManager::GetRegions() is empty.
    /// <para>
    /// If the result is <see cref="PartyStateChangeResult::FailedToBindToLocalUdpSocket" />, the library couldn't bind
    /// to the local UDP socket specified in the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option. The title
    /// must clean up its instance of the library, update the <see cref="PartyOption::LocalUdpSocketBindAddress" />
    /// option to a valid, available bind address, and reinitialize the library.
    /// </para>
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing more troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>CreateNewNetworkCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyManager::CreateNewNetwork" />
/// <seealso cref="PartyManager::SerializeNetworkDescriptor" />
struct PartyCreateNewNetworkCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the create new network operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The local user provided to the call associated with this state change.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The network configuration provided to the call associated with this state change.
    /// </summary>
    PartyNetworkConfiguration networkConfiguration;

    /// <summary>
    /// The number of regions provided to the call associated with this state change.
    /// </summary>
    uint32_t regionCount;

    /// <summary>
    /// The regions provided to the call associated with this state change.
    /// </summary>
    const PartyRegion * regions;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;

    /// <summary>
    /// The network descriptor of the network that was created.
    /// </summary>
    /// <remarks>
    /// The regionName and opaqueConnectionInformation fields are only populated if the result field is
    /// <see cref="PartyStateChangeResult::Succeeded" />. The networkIdentifier field should always be populated. If the
    /// result field is PartyStateChangeResult::Succeeded, this network descriptor is serializable via
    /// <see cref="PartyManager::SerializeNetworkDescriptor()" />.
    /// </remarks>
    PartyNetworkDescriptor networkDescriptor;

    /// <summary>
    /// The identifier for the network's initial invitation.
    /// </summary>
    PartyString appliedInitialInvitationIdentifier;
};

/// <summary>
/// Information specific to the <em>ConnectToNetworkCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyManager::ConnectToNetwork" />
struct PartyConnectToNetworkCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the connect to network operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network descriptor provided to the call associated with this state change.
    /// </summary>
    PartyNetworkDescriptor networkDescriptor;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;

    /// <summary>
    /// The network that was connected to.
    /// </summary>
    PartyNetwork * network;
};

/// <summary>
/// Information specific to the <em>AuthenticateLocalUserCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
struct PartyAuthenticateLocalUserCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the authenticate local user operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user provided to the call associated with this state change.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The identifier of the invitation used to authenticate into the network.
    /// </summary>
    PartyString invitationIdentifier;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>NetworkConfigurationMadeAvailable</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::GetNetworkConfiguration" />
struct PartyNetworkConfigurationMadeAvailableStateChange : PartyStateChange
{
    /// <summary>
    /// The network where the network configuration was made available.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The newly available network configuration.
    /// </summary>
    PartyNetworkConfiguration * networkConfiguration;
};

/// <summary>
/// Information specific to the <em>NetworkDescriptorChanged</em> type of state change.
/// </summary>
/// <remarks>
/// This state change is generated when the network descriptor of a network changes due to migration from one server to
/// another. Use <see cref="PartyNetwork::GetNetworkDescriptor()" /> to get the new network descriptor. The old network
/// descriptor is no longer guaranteed to be usable for connecting to the network, so any outstanding external
/// references to the old network descriptor (such as invitations sent over a social network) should be updated.
/// </remarks>
/// <seealso cref="PartyNetwork::GetNetworkDescriptor" />
struct PartyNetworkDescriptorChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The network where the network descriptor changed.
    /// </summary>
    PartyNetwork * network;
};

/// <summary>
/// Information specific to the <em>LocalUserRemoved</em> type of state change.
/// </summary>
/// <remarks>
/// All PartyEndpoints depending on this user will have been destroyed prior to this state change being generated. All
/// PartyChatControls depending on this user will have left the network prior to this state change being generated.
/// </remarks>
/// <seealso cref="PartyNetwork::RemoveLocalUser" />
/// <seealso cref="PartyManager::DestroyLocalUser" />
struct PartyLocalUserRemovedStateChange : PartyStateChange
{
    /// <summary>
    /// The network that the local user was removed from.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user provided to the call associated with this state change.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The reason the user was removed from the network.
    /// </summary>
    PartyLocalUserRemovedReason removedReason;
};

/// <summary>
/// Information specific to the <em>RemoveLocalUserCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// All PartyEndpoints and PartyChatControls depending upon this user will have been destroyed with reason
/// <see cref="PartyDestroyedReason::UserRemoved" /> before this state change is generated. If the user referenced by
/// this state change was the last authenticated user in the network, then all remaining PartyEndpoints on this device,
/// which at this point inherently consists of only endpoints associated with the last user or no user, will also have
/// been destroyed before this state change is generated. An associated <see cref="PartyLocalUserRemovedStateChange" />
/// will have been generated before this state change is generated.
/// </remarks>
/// <seealso cref="PartyChatControlDestroyedStateChange" />
/// <seealso cref="PartyChatControlLeftNetworkStateChange" />
/// <seealso cref="PartyEndpointDestroyedStateChange" />
/// <seealso cref="PartyNetwork::RemoveLocalUser" />
struct PartyRemoveLocalUserCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the remove local user operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user provided to the call associated with this state change.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>DestroyLocalUserCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// This PartyLocalUser will be removed from all dependent networks prior to this state change. All dependent
/// PartyEndpoint and PartyChatControl objects will be destroyed with reason
/// <see cref="PartyDestroyedReason::UserRemoved" /> prior to the PartyLocalUser being removed from its dependent
/// networks. Once this state change is returned to <see cref="PartyManager::FinishProcessingStateChanges()" />, the
/// PartyLocalUser object memory will become invalid.
/// </remarks>
/// <seealso cref="PartyManager::DestroyLocalUser" />
struct PartyDestroyLocalUserCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the destroy local user operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The local user provided to the call associated with this state change.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>LocalUserKicked</em> type of state change.
/// </summary>
/// <remarks>
/// All PartyEndpoints and PartyChatControls depending upon this user will have been destroyed with reason
/// <see cref="PartyDestroyedReason::Kicked" /> before this state change is generated.
/// </remarks>
/// <seealso cref="PartyNetwork::KickUser" />
/// <nyi />
struct PartyLocalUserKickedStateChange : PartyStateChange
{
    /// <summary>
    /// The network from which the local user was kicked.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user that was kicked.
    /// </summary>
    PartyLocalUser * localUser;
};

/// <summary>
/// Information specific to the <em>CreateEndpointCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// An associated <see cref="PartyEndpointCreatedStateChange" /> will be generated before this state change is
/// generated.
/// </remarks>
/// <seealso cref="PartyNetwork::CreateEndpoint" />
struct PartyCreateEndpointCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the create endpoint Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user provided to the call associated with this state change if one was provided.
    /// </summary>
    _Maybenull_ PartyLocalUser * localUser;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;

    /// <summary>
    /// The endpoint that was created.
    /// </summary>
    PartyLocalEndpoint * localEndpoint;
};

/// <summary>
/// Information specific to the <em>DestroyEndpointCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// An associated <see cref="PartyEndpointDestroyedStateChange" /> will be generated before this state change is
/// generated.
/// </remarks>
/// <seealso cref="PartyNetwork::DestroyEndpoint" />
struct PartyDestroyEndpointCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the destroy endpoint operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The endpoint provided to the call associated with this state change.
    /// </summary>
    PartyLocalEndpoint * localEndpoint;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>EndpointCreated</em> type of state change.
/// </summary>
struct PartyEndpointCreatedStateChange : PartyStateChange
{
    /// <summary>
    /// The network of the endpoint that was created.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The endpoint that was created.
    /// </summary>
    PartyEndpoint * endpoint;
};

/// <summary>
/// Information specific to the <em>EndpointDestroyed</em> type of state change.
/// </summary>
struct PartyEndpointDestroyedStateChange : PartyStateChange
{
    /// <summary>
    /// The network of the endpoint that was destroyed.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The endpoint that was destroyed.
    /// </summary>
    PartyEndpoint * endpoint;

    /// <summary>
    /// The reason the endpoint was destroyed.
    /// </summary>
    PartyDestroyedReason reason;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>RemoteDeviceCreated</em> type of state change.
/// </summary>
/// <remarks>
/// This state change indicates that the device was just created.
/// </remarks>
struct PartyRemoteDeviceCreatedStateChange : PartyStateChange
{
    /// <summary>
    /// The device that was created.
    /// </summary>
    PartyDevice * device;
};

/// <summary>
/// Information specific to the <em>RemoteDeviceDestroyed</em> type of state change.
/// </summary>
/// <remarks>
/// This state change is generated when device has left all networks.
/// </remarks>
struct PartyRemoteDeviceDestroyedStateChange : PartyStateChange
{
    /// <summary>
    /// The device that was destroyed.
    /// </summary>
    PartyDevice * device;
};

/// <summary>
/// Information specific to the <em>RemoteDeviceJoinedNetwork</em> type of state change.
/// </summary>
/// <remarks>
/// This state change indicates that the device has just joined the network.
/// </remarks>
struct PartyRemoteDeviceJoinedNetworkStateChange : PartyStateChange
{
    /// <summary>
    /// The device that joined.
    /// </summary>
    PartyDevice * device;

    /// <summary>
    /// The network that was joined.
    /// </summary>
    PartyNetwork * network;
};

/// <summary>
/// Information specific to the <em>RemoteDeviceLeftNetwork</em> type of state change.
/// </summary>
/// <remarks>
/// This state change indicates that the device just left the network.
/// </remarks>
struct PartyRemoteDeviceLeftNetworkStateChange : PartyStateChange
{
    /// <summary>
    /// The reason the device left the network.
    /// </summary>
    PartyDestroyedReason reason;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The device that left.
    /// </summary>
    PartyDevice * device;

    /// <summary>
    /// The network that was left.
    /// </summary>
    PartyNetwork * network;
};

/// <summary>
/// Information specific to the <em>DevicePropertiesChanged</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::SetSharedProperties" />
/// <seealso cref="PartyDevice::GetSharedProperty" />
/// <nyi />
struct PartyDevicePropertiesChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The device where the properties changed.
    /// </summary>
    PartyDevice * device;

    /// <summary>
    /// The number of changed properties.
    /// </summary>
    uint32_t propertyCount;

    /// <summary>
    /// The keys of the properties that have changed.
    /// </summary>
    /// <remarks>
    /// Each key can represent an add, update, or delete. If the key was deleted,
    /// <see cref="PartyDevice::GetSharedProperty()" /> will return nullptr for the value.
    /// </remarks>
    _Field_size_(propertyCount) const PartyString * keys;
};

/// <summary>
/// Information specific to the <em>LeaveNetworkCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// An associated <see cref="PartyNetworkDestroyedStateChange" /> will be generated before this state change is
/// generated. The network object <c>network</c> is only valid until this state change is returned to
/// <see cref="PartyManager::FinishProcessingStateChanges()" />.
/// </remarks>
/// <seealso cref="PartyNetwork::LeaveNetwork" />
struct PartyLeaveNetworkCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the leave network operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>NetworkDestroyed</em> type of state change.
/// </summary>
struct PartyNetworkDestroyedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates why the network was destroyed.
    /// </summary>
    PartyDestroyedReason reason;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network that was destroyed.
    /// </summary>
    PartyNetwork * network;
};

/// <summary>
/// Information specific to the <em>EndpointMessageReceived</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
struct PartyEndpointMessageReceivedStateChange : PartyStateChange
{
    /// <summary>
    /// The network containing the endpoint on which the message was received.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The endpoint which sent the message.
    /// </summary>
    PartyEndpoint * senderEndpoint;

    /// <summary>
    /// The number of local endpoints to which the message was sent. This value will always be greater than zero. If the
    /// sender specified an empty target endpoints array in the <see cref="PartyLocalEndpoint::SendMessage()" /> call
    /// that resulted in this state change, <c>receiverEndpointCount</c> and <c>receiverEndpoints</c> will contain all
    /// local endpoints.
    /// </summary>
    uint32_t receiverEndpointCount;

    /// <summary>
    /// The local endpoints to which the message was sent. This array will never be empty. If the sender specified an
    /// empty target endpoints array in the <see cref="PartyLocalEndpoint::SendMessage()" /> call that resulted in this
    /// state change, <c>receiverEndpointCount</c> and <c>receiverEndpoints</c> will contain all local endpoints.
    /// </summary>
    _Field_size_(receiverEndpointCount) PartyLocalEndpointArray receiverEndpoints;

    /// <summary>
    /// The options used to send the message.
    /// </summary>
    PartyMessageReceivedOptions options;

    /// <summary>
    /// The size of the message in bytes.
    /// </summary>
    uint32_t messageSize;

    /// <summary>
    /// The message buffer.
    /// </summary>
    _Field_size_bytes_(messageSize) const void * messageBuffer;
};

/// <summary>
/// Information specific to the <em>DataBuffersReturned</em> type of state change.
/// </summary>
/// <remarks>
/// This state change is only returned if the corresponding call to <see cref="PartyLocalEndpoint::SendMessage()" />
/// included the <see cref="PartySendMessageOptions::DontCopyDataBuffers" /> option. This state change is returned once
/// the data buffers passed with this call are no longer in use by the library.
/// </remarks>
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
struct PartyDataBuffersReturnedStateChange : PartyStateChange
{
    /// <summary>
    /// The network on which the message was sent.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local endpoint used to send the message.
    /// </summary>
    PartyLocalEndpoint * localSenderEndpoint;

    /// <summary>
    /// The number of data buffers.
    /// </summary>
    uint32_t dataBufferCount;

    /// <summary>
    /// The data buffers.
    /// </summary>
    _Field_size_(dataBufferCount) const PartyDataBuffer * dataBuffers;

    /// <summary>
    /// The message identifier provided to the call associated with this state change.
    /// </summary>
    void * messageIdentifier;
};

/// <summary>
/// Information specific to the <em>EndpointPropertiesChanged</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalEndpoint::SetSharedProperties" />
/// <seealso cref="PartyEndpoint::GetSharedProperty" />
/// <nyi />
struct PartyEndpointPropertiesChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The endpoint where the properties changed.
    /// </summary>
    PartyEndpoint * endpoint;

    /// <summary>
    /// The number of changed properties.
    /// </summary>
    uint32_t propertyCount;

    /// <summary>
    /// The keys of the properties that have changed.
    /// </summary>
    /// <remarks>
    /// Each key can represent an add, update, or delete. If the key was deleted,
    /// <see cref="PartyEndpoint::GetSharedProperty()" /> will return nullptr for the value.
    /// </remarks>
    _Field_size_(propertyCount) const PartyString * keys;
};

/// <summary>
/// Information specific to the <em>SynchronizeMessagesBetweenEndpointsCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// This state change signifies that the synchronize operation has completed. This means that all data sends sent to or
/// from endpoints specified in the call to PartyManager::SynchronizeMessagesBetweenEndpoints before the call to
/// PartyManager::SynchronizeMessagesBetweenEndpoints have completed. Data sends queued after the call to
/// PartyManager::SynchronizeMessagesBetweenEndpoints will now start being sent and received.
/// </remarks>
/// <seealso cref="PartyLocalEndpoint::SendMessage" />
/// <seealso cref="PartyManager::SynchronizeMessagesBetweenEndpoints" />
/// <nyi />
struct PartySynchronizeMessagesBetweenEndpointsCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// The number of endpoints that were synchronized.
    /// </summary>
    uint32_t endpointCount;

    /// <summary>
    /// The endpoints that were synchronized.
    /// </summary>
    /// <remarks>
    /// The list of endpoints will not include any endpoints that were specified but were destroyed before the
    /// synchronize operation completed.
    /// </remarks>
    _Field_size_(endpointCount) PartyEndpointArray endpoints;

    /// <summary>
    /// The options provided to the call associated with this state change.
    /// </summary>
    PartySynchronizeMessagesBetweenEndpointsOptions options;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>CreateInvitationCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::CreateInvitation" />
struct PartyCreateInvitationCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the <see cref="PartyNetwork::CreateInvitation()" /> operation succeeded or provides the reason
    /// that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user that created the invitation.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;

    /// <summary>
    /// The invitation created for the network.
    /// </summary>
    PartyInvitation * invitation;
};

/// <summary>
/// Information specific to the <em>RevokeInvitationCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::RevokeInvitation" />
struct PartyRevokeInvitationCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the <see cref="PartyNetwork::RevokeInvitation()" /> operation succeeded or provides the reason
    /// that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The local user used to revoke the invitation.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The revoked invitation.
    /// </summary>
    PartyInvitation * invitation;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>InvitationCreated</em> type of state change.
/// </summary>
struct PartyInvitationCreatedStateChange : PartyStateChange
{
    /// <summary>
    /// The network of the invitation that was created.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The invitation that was created.
    /// </summary>
    /// <remarks>
    /// Upon receiving this state change, the invitation object will be queryable via
    /// <see cref="PartyNetwork::GetInvitations()" />.
    /// </remarks>
    PartyInvitation * invitation;
};

/// <summary>
/// Information specific to the <em>InvitationDestroyed</em> type of state change.
/// </summary>
struct PartyInvitationDestroyedStateChange : PartyStateChange
{
    /// <summary>
    /// The network of the invitation that was destroyed.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The invitation that was destroyed.
    /// </summary>
    /// <remarks>
    /// Upon receiving this state change, the invitation object will no longer be queryable via
    /// <see cref="PartyNetwork::GetInvitations()" />.
    /// </remarks>
    PartyInvitation * invitation;

    /// <summary>
    /// The reason the invitation was destroyed.
    /// </summary>
    /// <remarks>
    /// If the invitation object was destroyed because it was explicitly revoked via
    /// <see cref="PartyNetwork::RevokeInvitation()" /> or automatically revoked when the creating local user left the
    /// network, this value will be <see cref="PartyDestroyedReason::Requested" />. If the invitation object was
    /// destroyed because the local client is no longer authenticated in the network, this value will be
    /// <see cref="PartyDestroyedReason::DeviceLostAuthentication" />.
    /// </remarks>
    PartyDestroyedReason reason;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>NetworkPropertiesChanged</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::GetSharedProperty" />
/// <seealso cref="PartyNetwork::SetSharedProperties" />
/// <nyi />
struct PartyNetworkPropertiesChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The network where the properties changed.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The number of changed properties.
    /// </summary>
    uint32_t propertyCount;

    /// <summary>
    /// The keys of the properties that have changed.
    /// </summary>
    /// <remarks>
    /// Each key can represent an add, update, or delete. If the key was deleted,
    /// <see cref="PartyNetwork::GetSharedProperty()" /> will return nullptr for the value.
    /// </remarks>
    _Field_size_(propertyCount) const PartyString * keys;
};

/// <summary>
/// Information specific to the <em>KickDeviceCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// All associated <see cref="PartyRemoteDeviceDestroyedStateChange" /> and
/// <see cref="PartyEndpointDestroyedStateChange" /> state changes will be generated before this state change is
/// generated.
/// </remarks>
/// <seealso cref="PartyNetwork::KickDevice" />
/// <nyi />
struct PartyKickDeviceCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the kick device operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The device provided to the call associated with this state change.
    /// </summary>
    PartyDevice * kickedDevice;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>KickUserCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// All associated <see cref="PartyRemoteDeviceDestroyedStateChange" />,
/// <see cref="PartyEndpointDestroyedStateChange" />, and <see cref="PartyLocalUserKickedStateChange" /> state changes
/// will be generated before this state change is generated.
/// </remarks>
/// <seealso cref="PartyNetwork::KickUser" />
/// <nyi />
struct PartyKickUserCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the kick user operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The Entity ID provided to the call associated with this state change.
    /// </summary>
    PartyString kickedEntityId;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>CreateChatControlCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::CreateChatControl" />
struct PartyCreateChatControlCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the chat control creation operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The local device used in the call associated with this state change.
    /// </summary>
    PartyLocalDevice * localDevice;

    /// <summary>
    /// The local user provided to the call associated with this state change.
    /// </summary>
    PartyLocalUser * localUser;

    /// <summary>
    /// The language code provided to the call associated with this state change.
    /// </summary>
    PartyString languageCode;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;

    /// <summary>
    /// The chat control that was created.
    /// </summary>
    PartyLocalChatControl * localChatControl;
};

/// <summary>
/// Information specific to the <em>DestroyChatControlCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::DestroyChatControl" />
struct PartyDestroyChatControlCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the chat control destruction operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The local device used in the call associated with this state change.
    /// </summary>
    PartyLocalDevice * localDevice;

    /// <summary>
    /// The chat control that was destroyed.
    /// </summary>
    /// <remarks>
    /// The memory remains valid until this state change is returned.
    /// </remarks>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>ChatControlCreated</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::CreateChatControl" />
struct PartyChatControlCreatedStateChange : PartyStateChange
{
    /// <summary>
    /// The chat control that was created.
    /// </summary>
    PartyChatControl * chatControl;
};

/// <summary>
/// Information specific to the <em>ChatControlDestroyed</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::DestroyChatControl" />
struct PartyChatControlDestroyedStateChange : PartyStateChange
{
    /// <summary>
    /// The chat control that was destroyed.
    /// </summary>
    /// <remarks>
    /// The memory remains valid until this state change is returned.
    /// </remarks>
    PartyChatControl * chatControl;

    /// <summary>
    /// The reason the chat control was destroyed.
    /// </summary>
    PartyDestroyedReason reason;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>SetChatAudioEncoderBitrateCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetAudioEncoderBitrate" />
/// <nyi />
struct PartySetChatAudioEncoderBitrateCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the encoder bitrate configuration operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The local chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The bitrate provided to the call associated with this state change.
    /// </summary>
    uint32_t bitrate;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>ChatTextReceived</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::CreateChatControl" />
/// <seealso cref="PartyLocalChatControl::SendText" />
/// <seealso cref="PartyLocalChatControl::SetTextChatOptions" />
/// <seealso_nyi cref="PartyLocalChatControl::SetLanguage" />
struct PartyChatTextReceivedStateChange : PartyStateChange
{
    /// <summary>
    /// The chat control object that originated the text message.
    /// </summary>
    PartyChatControl * senderChatControl;

    /// <summary>
    /// The number of local receiver chat controls to which the text message is addressed.
    /// </summary>
    uint32_t receiverChatControlCount;

    /// <summary>
    /// The local receiver chat controls to which the text message is addressed.
    /// </summary>
    _Field_size_(receiverChatControlCount) PartyLocalChatControlArray receiverChatControls;

    /// <summary>
    /// The language of the chat text.
    /// </summary>
    /// <remarks>
    /// The language is only provided when translation to the local language is enabled. If translation isn't enabled,
    /// or failure is encountered during translation, the language code is an empty string.
    /// <para>
    /// The language code is in BCP 47 format, such as en-US for English (United States). Supported language codes are
    /// enumerated in
    /// <see xref="https://learn.microsoft.com/azure/ai-services/speech-service/language-support" name="Language support" />.
    /// </para>
    /// </remarks>
    PartyString languageCode;

    /// <summary>
    /// The received chat text.
    /// </summary>
    /// <remarks>
    /// The string may be up to <c>c_maxChatTextMessageLength</c> characters long, not including the null terminator.
    /// <para>
    /// When filtering is enabled, this text may not be the exact text sent by the remote user. The exact source text
    /// can be retrieved from the <c>originalChatText</c> field.
    /// </para>
    /// </remarks>
    PartyString chatText;

    /// <summary>
    /// The size of the data associated with this text message.
    /// </summary>
    uint32_t dataSize;

    /// <summary>
    /// The data associated with this text message.
    /// </summary>
    _Field_size_bytes_(dataSize) const void * data;

    /// <summary>
    /// The number of translations associated with the chat text.
    /// </summary>
    /// <remarks>
    /// Translations are provided if <see cref="PartyTextChatOptions::TranslateToLocalLanguage" /> has previously been
    /// specified via <see cref="PartyLocalChatControl::SetTextChatOptions()" /> on a chat control local to this device.
    /// There may be more than one translation if multiple local chat controls have enabled translation and the local
    /// chat controls have specified different languages via <see cref="PartyLocalDevice::CreateChatControl()" />. In
    /// that case, the app can compare the <c>languageCode</c> field of each PartyTranslation in <c>translations</c>
    /// against the language code, obtained via <see cref="PartyLocalChatControl::GetLanguage()" />, for each local chat
    /// control in <c>receiverChatControls</c> to determine the target local chat control for each translation.
    /// </remarks>
    uint32_t translationCount;

    /// <summary>
    /// An array containing the translations of the chat text string.
    /// </summary>
    /// <remarks>
    /// Translations are provided if <see cref="PartyTextChatOptions::TranslateToLocalLanguage" /> has previously been
    /// specified via <see cref="PartyLocalChatControl::SetTextChatOptions()" /> on a chat control local to this device.
    /// There may be more than one translation if multiple local chat controls have enabled translation and the local
    /// chat controls have specified different languages via <see cref="PartyLocalDevice::CreateChatControl()" />. In
    /// that case, the app can compare the <c>languageCode</c> field of each PartyTranslation in <c>translations</c>
    /// against the language code, obtained via <see cref="PartyLocalChatControl::GetLanguage()" />, for each local chat
    /// control in <c>receiverChatControls</c> to determine the target local chat control for each translation.
    /// </remarks>
    /// <para>
    /// A translation corresponding to the language for each chat control in <c>receiverChatControls</c> that has
    /// enabled translation is provided, even if the source chat control's language is the same as the local chat
    /// control's language. In such a case, the chat text and translation strings are identical.
    /// </para>
    _Field_size_(translationCount) PartyTranslation * translations;

    /// <summary>
    /// Flags describing additional information about the chat text.
    /// </summary>
    PartyChatTextReceivedOptions options;

    /// <summary>
    /// The original version of the incoming text chat.
    /// </summary>
    /// <remarks>
    /// This field is always the unfiltered source text sent by the remote user. If filtering isn't enabled, it's a
    /// pointer to <c>chatText</c>. If filtering is enabled but filtering was not needed, as indicated by
    /// PartyChatTextReceivedOptions::None in the <c>options</c> field, then this is a pointer to <c>chatText</c>.
    /// </remarks>
    PartyString originalChatText;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// This error is intended for diagnostic purposes only. If an error occurs, the
    /// <see cref="PartyChatTextReceivedOptions::FilteredDueToError" /> flag will be present in the <c>options</c>
    /// field.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>VoiceChatTranscriptionReceived</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalDevice::CreateChatControl" />
/// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
/// <seealso_nyi cref="PartyLocalChatControl::SetLanguage" />
struct PartyVoiceChatTranscriptionReceivedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the transcription operation Succeeded or provides the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, the <c>transcription</c> field is a string of non-zero length. On failure, the string is empty.
    /// Failure indicates that a transcription operation was attempted for the speaker but could not be completed. If
    /// transcription is enabled at the request of the user associated with the chat control, and the transcription
    /// messages are shown via UI, it is recommended that failures also be indicated to the user in order to provide
    /// feedback as to whether transcriptions are pending or have failed.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control object that originated the transcription message.
    /// </summary>
    PartyChatControl * senderChatControl;

    /// <summary>
    /// The number of local receiver chat controls to which the transcription is addressed.
    /// </summary>
    uint32_t receiverChatControlCount;

    /// <summary>
    /// The local receiver chat controls to which the transcription is addressed.
    /// </summary>
    _Field_size_(receiverChatControlCount) PartyLocalChatControlArray receiverChatControls;

    /// <summary>
    /// The type of audio source this transcription represents.
    /// </summary>
    /// <remarks>
    /// The audio source type can optionally be used for game scenarios in which transcriptions from different source
    /// types should be treated differently. For instance, different iconography may be shown depending on whether the
    /// transcription is associated with microphone or text-to-speech audio.
    /// </remarks>
    PartyAudioSourceType sourceType;

    /// <summary>
    /// The language code of the transcription.
    /// </summary>
    /// <remarks>
    /// The language code is always provided when the <c>result</c> field indicates success. Otherwise, the language
    /// code is an empty string.
    /// <para>
    /// The language code is in BCP 47 format, such as en-US for English (United States). Supported language codes are
    /// enumerated in
    /// <see xref="https://learn.microsoft.com/azure/ai-services/speech-service/language-support" name="Language support" />.
    /// </para>
    /// </remarks>
    PartyString languageCode;

    /// <summary>
    /// The transcribed voice chat text.
    /// </summary>
    /// <remarks>
    /// The string may be up to <c>c_maxChatTextMessageLength</c> characters long, not including the null terminator.
    /// The string is always empty when the <c>result</c> field indicates failures.
    /// </remarks>
    PartyString transcription;

    /// <summary>
    /// Indicates the phrase type of the text provided in the <c>transcription</c> field.
    /// </summary>
    /// <remarks>
    /// The type is always <see cref="PartyVoiceChatTranscriptionPhraseType::Final" /> when the <c>result</c> field
    /// indicates failure.
    /// </remarks>
    PartyVoiceChatTranscriptionPhraseType type;

    /// <summary>
    /// The number of translations associated with the transcribed voice chat text.
    /// </summary>
    /// <remarks>
    /// Translations are provided if <see cref="PartyVoiceChatTranscriptionOptions::TranslateToLocalLanguage" /> has
    /// previously been specified via <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> on a chat control
    /// local to this device. There may be more than one translation if multiple local chat controls have enabled
    /// translation and the local chat controls have specified different languages via
    /// <see cref="PartyLocalDevice::CreateChatControl()" />. In that case, the app can compare the <c>languageCode</c>
    /// field of each PartyTranslation in <c>translations</c> against the language code, obtained via
    /// <see cref="PartyLocalChatControl::GetLanguage()" />, for each local chat control in <c>receiverChatControls</c>
    /// to determine the target local chat control for each translation.
    /// </remarks>
    uint32_t translationCount;

    /// <summary>
    /// An array containing the translations of the voice chat transcription string.
    /// </summary>
    /// <remarks>
    /// Translations are provided if <see cref="PartyVoiceChatTranscriptionOptions::TranslateToLocalLanguage" /> has
    /// previously been specified via <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> on a chat control
    /// local to this device. There may be more than one translation if multiple local chat controls have enabled
    /// translation and the local chat controls have specified different languages via
    /// <see cref="PartyLocalDevice::CreateChatControl()" />. In that case, the app can compare the <c>languageCode</c>
    /// field of each PartyTranslation in <c>translations</c> against the language code, obtained via
    /// <see cref="PartyLocalChatControl::GetLanguage()" />, for each local chat control in <c>receiverChatControls</c>
    /// to determine the target local chat control for each translation.
    /// <para>
    /// A translation corresponding to the language for each chat control in <c>receiverChatControls</c> that has
    /// enabled translation is provided, even if the speaking chat control's language is the same as the local chat
    /// control's language. In such a case, the transcription and translation strings are identical.
    /// </para>
    /// </remarks>
    _Field_size_(translationCount) PartyTranslation * translations;
};

/// <summary>
/// Information specific to the <em>SetChatAudioInputCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetAudioInput" />
struct PartySetChatAudioInputCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the audio input configuration operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The audio device selection type provided to the call associated with this state change.
    /// </summary>
    PartyAudioDeviceSelectionType audioDeviceSelectionType;

    /// <summary>
    /// The device context provided to the call associated with this state change.
    /// </summary>
    PartyString audioDeviceSelectionContext;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>SetChatAudioOutputCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetAudioOutput" />
struct PartySetChatAudioOutputCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the audio output configuration operation Succeeded or provides the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The audio device selection type provided to the call associated with this state change.
    /// </summary>
    PartyAudioDeviceSelectionType audioDeviceSelectionType;

    /// <summary>
    /// The device context provided to the call associated with this state change.
    /// </summary>
    PartyString audioDeviceSelectionContext;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>LocalChatAudioInputChanged</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetAudioInput" />
struct PartyLocalChatAudioInputChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The local chat control which had an audio input state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// Indicates the new state of the audio input associated with the chat control.
    /// </summary>
    PartyAudioInputState state;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>LocalChatAudioOutputChanged</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetAudioOutput" />
struct PartyLocalChatAudioOutputChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The local chat control which had an audio output state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// Indicates the new state of the audio output associated with the chat control.
    /// </summary>
    PartyAudioOutputState state;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;
};

/// <summary>
/// Information specific to the <em>SetTextToSpeechProfileCompleted</em> type of state change.
/// </summary>
/// <remarks>
/// At most, one PartySetTextToSpeechProfileCompletedStateChange object will be returned in any state change batch from
/// <see cref="PartyManager::StartProcessingStateChanges()" />.
/// </remarks>
/// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
struct PartySetTextToSpeechProfileCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, <see cref="PartyLocalChatControl::GetTextToSpeechProfile()" /> will return the full profile object
    /// corresponding to the profile identifier provided to the
    /// <see cref="PartyLocalChatControl::SetTextToSpeechProfile()" /> call associated with this state change. On
    /// failure, PartyLocalChatControl::GetTextToSpeechProfile() will return null.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The type of text-to-speech operation provided to the call associated with this state change.
    /// </summary>
    PartySynthesizeTextToSpeechType type;

    /// <summary>
    /// The profile identifier provided to the call associated with this state change.
    /// </summary>
    PartyString profileIdentifier;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>SynthesizeTextToSpeechCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SynthesizeTextToSpeech" />
struct PartySynthesizeTextToSpeechCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The type of text-to-speech operation provided to the call associated with this state change.
    /// </summary>
    PartySynthesizeTextToSpeechType type;

    /// <summary>
    /// The synthesized text provided to the call associated with this state change.
    /// </summary>
    /// <remarks>
    /// If the accompanying result was a success, the audio synthesized for this text has been queued in the audio
    /// stream but may not have been transmitted yet.
    /// </remarks>
    PartyString textToSynthesize;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>SetLanguageCompleted</em> type of state change.
/// </summary>
/// <seealso_nyi cref="PartyLocalChatControl::SetLanguage" />
/// <nyi />
struct PartySetLanguageCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The language code provided to the call associated with this state change.
    /// </summary>
    PartyString languageCode;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>SetTranscriptionOptionsCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
struct PartySetTranscriptionOptionsCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, internal state has updated to reflect whether transcription operations should be attempted on behalf
    /// of <c>localChatControl</c>. Success does not guarantee that transcription operations will complete successfully.
    /// Failures to complete transcription operations will be exposed through the <c>result</c> field of the
    /// <see cref="PartyStateChangeType::VoiceChatTranscriptionReceived" /> state change.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The transcription options provided to the call associated with this state change.
    /// </summary>
    PartyVoiceChatTranscriptionOptions options;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>PartySetTextChatOptionsCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
struct PartySetTextChatOptionsCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The text chat options provided to the call associated with this state change.
    /// </summary>
    PartyTextChatOptions options;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>ChatControlPropertiesChanged</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::SetSharedProperties" />
/// <seealso cref="PartyChatControl::GetSharedProperty" />
/// <nyi />
struct PartyChatControlPropertiesChangedStateChange : PartyStateChange
{
    /// <summary>
    /// The chat control where the properties changed.
    /// </summary>
    PartyChatControl * chatControl;

    /// <summary>
    /// The number of changed properties.
    /// </summary>
    uint32_t propertyCount;

    /// <summary>
    /// The keys of the properties that have changed.
    /// </summary>
    /// <remarks>
    /// Each key can represent an add, update, or delete. If the key was deleted,
    /// <see cref="PartyChatControl::GetSharedProperty()" /> will return nullptr for the value.
    /// </remarks>
    _Field_size_(propertyCount) const PartyString * keys;
};

/// <summary>
/// Information specific to the <em>ChatControlJoinedNetwork</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::ConnectChatControl" />
struct PartyChatControlJoinedNetworkStateChange : PartyStateChange
{
    /// <summary>
    /// The network joined.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The chat control that joined the network.
    /// </summary>
    PartyChatControl * chatControl;
};

/// <summary>
/// Information specific to the <em>ChatControlLeftNetwork</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::DisconnectChatControl" />
struct PartyChatControlLeftNetworkStateChange : PartyStateChange
{
    /// <summary>
    /// The reason the chat control left the network.
    /// </summary>
    PartyDestroyedReason reason;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network left.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The chat control that left the network.
    /// </summary>
    PartyChatControl * chatControl;
};

/// <summary>
/// Information specific to the <em>ConnectChatControlCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::ConnectChatControl" />
struct PartyConnectChatControlCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The chat control provided to the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>DisconnectChatControlCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyNetwork::DisconnectChatControl" />
struct PartyDisconnectChatControlCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The network used in the call associated with this state change.
    /// </summary>
    PartyNetwork * network;

    /// <summary>
    /// The chat control provided to the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>PopulateAvailableTextToSpeechProfilesCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles" />
struct PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, the profile list provided by
    /// <see cref="PartyLocalChatControl::GetAvailableTextToSpeechProfiles()" /> will be populated with the results of
    /// the operation. On failure, the profile list will remain unchanged.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control provided to the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>ConfigureAudioManipulationVoiceStreamCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyChatControl::ConfigureAudioManipulationVoiceStream" />
struct PartyConfigureAudioManipulationVoiceStreamCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, the source stream provided by <see cref="PartyChatControl::GetAudioManipulationVoiceStream()" />
    /// will be updated.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyChatControl * chatControl;

    /// <summary>
    /// The configuration provided to the call associated with this state change.
    /// </summary>
    PartyAudioManipulationSourceStreamConfiguration * configuration;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>ConfigureAudioManipulationCaptureStreamCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationCaptureStream" />
struct PartyConfigureAudioManipulationCaptureStreamCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, the sink stream provided by
    /// <see cref="PartyLocalChatControl::GetAudioManipulationCaptureStream()" /> will be updated.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in to the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The configuration provided to the call associated with this state change.
    /// </summary>
    PartyAudioManipulationSinkStreamConfiguration * configuration;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>ConfigureAudioManipulationRenderStreamCompleted</em> type of state change.
/// </summary>
/// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationRenderStream" />
struct PartyConfigureAudioManipulationRenderStreamCompletedStateChange : PartyStateChange
{
    /// <summary>
    /// Indicates that the operation Succeeded or the reason that it failed.
    /// </summary>
    /// <remarks>
    /// On success, the sink stream provided by <see cref="PartyLocalChatControl::GetAudioManipulationRenderStream()" />
    /// will be updated.
    /// </remarks>
    PartyStateChangeResult result;

    /// <summary>
    /// A diagnostic value providing additional troubleshooting information regarding any potential error condition.
    /// </summary>
    /// <remarks>
    /// The human-readable form of this error detail can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </remarks>
    PartyError errorDetail;

    /// <summary>
    /// The chat control used in the call associated with this state change.
    /// </summary>
    PartyLocalChatControl * localChatControl;

    /// <summary>
    /// The configuration provided to the call associated with this state change.
    /// </summary>
    PartyAudioManipulationSinkStreamConfiguration * configuration;

    /// <summary>
    /// The async identifier provided to the call associated with this state change.
    /// </summary>
    void * asyncIdentifier;
};

/// <summary>
/// Information specific to the <em>MethodEntrance</em> type of profiling event.
/// </summary>
/// <seealso cref="PartyProfilingMethodEntranceCallback" />
struct PartyProfilingMethodEntranceEventData
{
    /// <summary>
    /// A string containing the fully qualified name of the method responsible for the callback.
    /// </summary>
    PartyString methodName;
};

/// <summary>
/// Information specific to the <em>MethodExit</em> type of profiling event.
/// </summary>
/// <seealso cref="PartyProfilingMethodExitCallback" />
struct PartyProfilingMethodExitEventData
{
    /// <summary>
    /// A string containing the fully qualified name of the method responsible for the callback.
    /// </summary>
    PartyString methodName;
};

#pragma pack(pop)

/// <summary>
/// A callback invoked every time the Party Library dynamically allocates a new memory buffer.
/// </summary>
/// <remarks>
/// This callback is optionally installed using the <see cref="PartyManager::SetMemoryCallbacks()" /> method.
/// <para>
/// The callback must allocate and return a pointer to a contiguous block of memory of the specified size that remains
/// valid until the title's corresponding <see cref="PartyFreeMemoryCallback" /> is invoked to release it. If this isn't
/// possible, the callback must return nullptr to fail the allocation. Memory allocation failures are sometimes
/// considered benign but usually cause current Party library operations to fail.
/// </para>
/// <para>
/// Every non-nullptr returned by this method is then passed to the corresponding <see cref="PartyFreeMemoryCallback" />
/// once the memory is no longer needed.
/// </para>
/// </remarks>
/// <param name="size">
/// The size of the allocation to be made. This value will never be zero.
/// </param>
/// <param name="memoryTypeId">
/// An opaque identifier representing the Party library internal category of memory being allocated. This value should
/// be ignored.
/// </param>
/// <returns>
/// A pointer to an allocated block of memory of the specified size, or nullptr if the allocation failed.
/// </returns>
/// <seealso cref="PartyFreeMemoryCallback" />
/// <seealso cref="PartyManager::SetMemoryCallbacks" />
/// <seealso cref="PartyManager::GetMemoryCallbacks" />
typedef
_Ret_maybenull_
_Post_writable_byte_size_(size) void *
(PARTY_CALLBACK * PartyAllocateMemoryCallback)(
    size_t size,
    uint32_t memoryTypeId
    );

/// <summary>
/// A callback invoked every time a previously allocated memory buffer is no longer needed by the Party library and can
/// be freed.
/// </summary>
/// <remarks>
/// This callback is optionally installed using the <see cref="PartyManager::SetMemoryCallbacks()" /> method.
/// <para>
/// The callback is invoked whenever the Party library has finished using a memory buffer previously returned by the
/// title's corresponding <see cref="PartyAllocateMemoryCallback" />, so that the title can free the memory buffer.
/// </para>
/// </remarks>
/// <param name="pointer">
/// A pointer to a memory buffer previously allocated. This value will never be nullptr.
/// </param>
/// <param name="memoryTypeId">
/// An opaque identifier representing the Party library internal category of memory being freed. This value should be
/// ignored.
/// </param>
/// <seealso cref="PartyAllocateMemoryCallback" />
/// <seealso cref="PartyManager::SetMemoryCallbacks" />
/// <seealso cref="PartyManager::GetMemoryCallbacks" />
typedef
void
(PARTY_CALLBACK * PartyFreeMemoryCallback)(
    _Post_invalid_ void * pointer,
    uint32_t memoryTypeId
    );

/// <summary>
/// A callback invoked every time the Party library enters an instrumented method.
/// </summary>
/// <remarks>
/// This callback is optionally installed using the
/// <see cref="PartyManager::SetProfilingCallbacksForMethodEntryExit()" /> method, which also details the types of
/// profiled events available to a caller.
/// <para>
/// In order to minimize the impact of profiling on title performance, callbacks should be kept as lightweight as
/// possible as they are expected to fire hundreds or thousands of times per second.
/// </para>
/// </remarks>
/// <param name="eventData">
/// A constant pointer to a structure containing additional information which may be of use when profiling this event.
/// The data referenced by this pointer is guaranteed to be valid only for the duration the callback.
/// </param>
/// <seealso cref="PartyProfilingMethodEntranceEventData" />
/// <seealso cref="PartyManager::SetProfilingCallbacksForMethodEntryExit" />
/// <seealso cref="PartyManager::GetProfilingCallbacksForMethodEntryExit" />
typedef
void
(PARTY_CALLBACK * PartyProfilingMethodEntranceCallback)(
    const PartyProfilingMethodEntranceEventData * eventData
    );

/// <summary>
/// A callback invoked every time the Party library is about to exit an instrumented method.
/// </summary>
/// <remarks>
/// This callback is optionally installed using the
/// <see cref="PartyManager::SetProfilingCallbacksForMethodEntryExit()" /> method, which also details the types of
/// profiled events available to a caller.
/// <para>
/// In order to minimize the impact of profiling on title performance, callbacks should be kept as lightweight as
/// possible as they are expected to fire hundreds or thousands of times per second.
/// </para>
/// </remarks>
/// <param name="eventData">
/// A constant pointer to a structure containing additional information which may be of use when profiling this event.
/// The data referenced by this pointer is guaranteed to be valid only for the duration the callback.
/// </param>
/// <seealso cref="PartyProfilingMethodExitEventData" />
/// <seealso cref="PartyManager::SetProfilingCallbacksForMethodEntryExit" />
/// <seealso cref="PartyManager::GetProfilingCallbacksForMethodEntryExit" />
typedef
void
(PARTY_CALLBACK * PartyProfilingMethodExitCallback)(
    const PartyProfilingMethodExitEventData * eventData
    );

/// <summary>
/// Represents a local user.
/// </summary>
/// <seealso cref="PartyManager::CreateLocalUser" />
/// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
/// <seealso cref="PartyManager::GetLocalUsers" />
/// <seealso cref="PartyManager::DestroyLocalUser" />
/// <seealso cref="PartyManager::CreateNewNetwork" />
/// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
/// <seealso cref="PartyNetwork::RemoveLocalUser" />
/// <seealso cref="PartyNetwork::CreateInvitation" />
/// <seealso cref="PartyNetwork::RevokeInvitation" />
/// <seealso cref="PartyNetwork::CreateEndpoint" />
/// <seealso cref="PartyLocalDevice::CreateChatControl" />
class PartyLocalUser
{
public:
    /// <summary>
    /// Gets the PlayFab Entity ID associated with this local user.
    /// </summary>
    /// <remarks>
    /// The retrieved PlayFab Entity ID is identical to the Entity ID provided by the title via
    /// <see cref="PartyManager::CreateLocalUser()" /> or <see cref="PartyManager::CreateLocalUserWithEntityType()" />.
    /// This is also the same value as the one found in the <em>id</em> field of a <c>PFEntityKey</c> structure
    /// representing the entity. The value returned from <see cref="GetEntityType()" /> would represent the other half
    /// of a <c>PFEntityKey</c>.
    /// <para>
    /// The memory for the Entity ID string remains valid for the life of the local user, which is until its
    /// <see cref="PartyDestroyLocalUserCompletedStateChange" /> has been provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> and all state changes referencing the local user have
    /// been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityId">
    /// The output Entity ID of the user.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalUser::GetEntityType" />
    /// <seealso cref="PartyManager::CreateLocalUser" />
    /// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
    PartyError GetEntityId(
        _Outptr_ PartyString * entityId
        ) const party_no_throw;

    /// <summary>
    /// Gets the PlayFab entity type associated with this local user.
    /// </summary>
    /// <remarks>
    /// The retrieved PlayFab entity type string will be "title_player_account" for users created via
    /// <see cref="PartyManager::CreateLocalUser()" />, or the entity type provided by the caller for users created via
    /// <see cref="PartyManager::CreateLocalUserWithEntityType()" />. This is also the same value as the one found in
    /// the <em>type</em> field of a <c>PFEntityKey</c> structure representing the entity. The value returned from
    /// <see cref="GetEntityId()" /> would represent the other half of a <c>PFEntityKey</c>.
    /// <para>
    /// The memory for the entity type string remains valid for the life of the local user, which is until its
    /// <see cref="PartyDestroyLocalUserCompletedStateChange" /> has been provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> and all state changes referencing the local user have
    /// been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityType">
    /// The output entity type name string of the user.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalUser::GetEntityId" />
    /// <seealso cref="PartyManager::CreateLocalUser" />
    /// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
    PartyError GetEntityType(
        _Outptr_ PartyString * entityType
        ) const party_no_throw;

    /// <summary>
    /// Updates the PlayFab Entity Token associated with this local user, for use in future authenticated operations.
    /// </summary>
    /// <remarks>
    /// This method takes a PlayFab Entity Token as <paramref name="entityToken" />. No synchronous validation is
    /// performed on this value. When the library performs operations that require user authentication or authorization,
    /// such as creating a network, authenticating into a network, or performing speech-to-text transcription, the Party
    /// service will validate that the token is valid, is not expired, is associated with the same Entity ID that was
    /// provided to the <see cref="PartyManager::CreateLocalUser()" /> or
    /// <see cref="PartyManager::CreateLocalUserWithEntityType()" /> call, and is authorized to perform the operation.
    /// If these conditions aren't met, the operation will fail.
    /// <para>
    /// A PlayFab Entity Token can be obtained from the output of a PlayFab login operation and then provided as input
    /// to this method. The token must be associated with the same PlayFab Entity ID and type as originally specified to
    /// <see cref="PartyManager::CreateLocalUser()" /> or <see cref="PartyManager::CreateLocalUserWithEntityType()" />.
    /// </para>
    /// <para>
    /// The provided <paramref name="entityToken" /> must have been acquired using the same Title ID that was passed to
    /// <see cref="PartyManager::Initialize()" />.
    /// </para>
    /// <para>
    /// The Party library makes a copy of the supplied PlayFab Entity Token for use in subsequent operations that
    /// require authentication or authorization of the local user, such as
    /// <see cref="PartyManager::CreateNewNetwork()" /> or <see cref="PartyNetwork::AuthenticateLocalUser()" />. If the
    /// token provided to this call is expired or otherwise invalid, operations that require a valid token will fail. A
    /// new, valid token can be provided to the Party library by another call to this method.
    /// </para>
    /// <para>
    /// The caller is responsible for monitoring the expiration of the entity token provided to this method and
    /// <see cref="PartyManager::CreateLocalUser()" /> or <see cref="PartyManager::CreateLocalUserWithEntityType()" />.
    /// When the token is nearing or past the expiration time a new token should be obtained by performing a PlayFab
    /// login operation and provided to the Party library by calling this method. We recommend acquiring a new token
    /// when the previously supplied token is halfway through its validity period. On platforms that may enter a low
    /// power state or otherwise cause the application to pause execution for a long time, preventing the token from
    /// being refreshed before it expires, the token should be checked for expiration once execution resumes.
    /// </para>
    /// </remarks>
    /// <param name="entityToken">
    /// The new PlayFab Entity Token to associate with the local user.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::GetErrorMessage" />
    /// <seealso cref="PartyManager::CreateLocalUser" />
    /// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    PartyError UpdateEntityToken(
        PartyString entityToken
        ) party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this local user
    /// object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalUser::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this local user object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the local user without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the player object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalUser::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyLocalUser() = delete;
    PartyLocalUser(const PartyLocalUser&) = delete;
    PartyLocalUser(PartyLocalUser&&) = delete;
    PartyLocalUser& operator=(const PartyLocalUser&) = delete;
    PartyLocalUser& operator=(PartyLocalUser&&) = delete;
    ~PartyLocalUser() = delete;
};

/// <summary>
/// Represents an endpoint in a network.
/// </summary>
/// <seealso cref="PartyLocalEndpoint" />
class PartyEndpoint
{
public:
    /// <summary>
    /// Gets the <see cref="PartyLocalEndpoint" /> version of this endpoint.
    /// </summary>
    /// <param name="localEndpoint">
    /// The output local version of this endpoint object, or nullptr if this is not a local endpoint.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocal(
        _Outptr_result_maybenull_ PartyLocalEndpoint ** localEndpoint
        ) const party_no_throw;

    /// <summary>
    /// Gets the PlayFab Entity ID of the user associated with this endpoint.
    /// </summary>
    /// <remarks>
    /// The returned string is the same value as the one found in the <em>ID</em> field of a <c>PFEntityKey</c>
    /// structure representing the entity. The value returned from <see cref="GetEntityType()" /> would represent the
    /// other half of a <c>PFEntityKey</c>.
    /// <para>
    /// If this endpoint isn't associated with a user, <paramref name="entityId" /> is set to nullptr.
    /// </para>
    /// <para>
    /// The memory for the Entity ID string remains valid for the life of the endpoint, which is until its
    /// <see cref="PartyEndpointDestroyedStateChange" /> and/or <see cref="PartyDestroyEndpointCompletedStateChange" />,
    /// depending on the type of destruction that occurred, has been provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> and all state changes referencing the endpoint have
    /// been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityId">
    /// The output Entity ID of the user associated with this endpoint, or nullptr.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpoint::GetEntityType" />
    PartyError GetEntityId(
        _Outptr_result_maybenull_ PartyString * entityId
        ) const party_no_throw;

    /// <summary>
    /// Gets the PlayFab entity type of the user associated with this endpoint.
    /// </summary>
    /// <remarks>
    /// This returned string is the same value as the one found in the <em>type</em> field of a <c>PFEntityKey</c>
    /// structure representing the entity. The value returned from <see cref="GetEntityId()" /> would represent the
    /// other half of a <c>PFEntityKey</c>.
    /// <para>
    /// If this endpoint isn't associated with a user, <paramref name="entityType" /> is set to nullptr.
    /// </para>
    /// <para>
    /// The memory for the entity type string remains valid for the life of the endpoint, which is until its
    /// <see cref="PartyEndpointDestroyedStateChange" /> and/or <see cref="PartyDestroyEndpointCompletedStateChange" />,
    /// depending on the type of destruction that occurred, has been provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> and all state changes referencing the endpoint have
    /// been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityType">
    /// The output entity type name string of the user associated with this endpoint, or nullptr.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpoint::GetEntityId" />
    PartyError GetEntityType(
        _Outptr_result_maybenull_ PartyString * entityType
        ) const party_no_throw;

    /// <summary>
    /// Gets the network associated with this endpoint.
    /// </summary>
    /// <param name="network">
    /// The output network associated with this endpoint.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetNetwork(
        _Outptr_ PartyNetwork ** network
        ) const party_no_throw;

    /// <summary>
    /// Gets the device associated with this endpoint.
    /// </summary>
    /// <param name="device">
    /// The output device associated with this endpoint.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetDevice(
        _Outptr_ PartyDevice ** device
        ) const party_no_throw;

    /// <summary>
    /// Gets the network-unique identifier for this endpoint.
    /// </summary>
    /// <remarks>
    /// This identifier is unique within the endpoint's network and is consistent across all endpoints in a network. It
    /// is intended to be used as a compact option for titles to refer to specific endpoints in
    /// <see cref="PartyLocalEndpoint::SendMessage()" /> data buffers without much bandwidth overhead or the need to
    /// manually negotiate identifiers. This identifier is not unique across networks.
    /// <para>
    /// For local endpoints, this method will fail until the <see cref="PartyEndpointCreatedStateChange" /> has been
    /// provided by <see cref="PartyManager::StartProcessingStateChanges()" />. For remote endpoints, this method will
    /// always succeed.
    /// </para>
    /// <para>
    /// All devices in a network will agree on a given endpoint's unique identifier, but different devices may not see
    /// the same endpoints at a given moment. For example, it's possible for endpoint A to send a message to endpoint B
    /// that references a newly-created endpoint C's unique identifier, but that message between A and B may arrive
    /// before the PartyEndpointCreatedStateChange for endpoint C is generated on endpoint B's device.
    /// </para>
    /// </remarks>
    /// <param name="uniqueIdentifier">
    /// The output unique identifier.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpointCreatedStateChange" />
    /// <seealso_nyi cref="PartyManager::SynchronizeMessagesBetweenEndpoints" />
    /// <seealso cref="PartyNetwork::FindEndpointByUniqueIdentifier" />
    PartyError GetUniqueIdentifier(
        _Out_ uint16_t * uniqueIdentifier
        ) const party_no_throw;

    /// <summary>
    /// Gets the value of a shared property.
    /// </summary>
    /// <remarks>
    /// If there is no property associated with the key, the resulting value is nullptr. The returned value is only
    /// valid until the next call to <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="key">
    /// The key of the shared property to retrieve.
    /// </param>
    /// <param name="value">
    /// An output struct that receives a pointer to the property's value and the length in bytes of the value data. If
    /// the property was not found, the call succeeds, but the value is nullptr and the byte length is 0.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetwork::CreateEndpoint" />
    /// <seealso_nyi cref="PartyEndpointPropertiesChangedStateChange" />
    PartyError GetSharedProperty(
        PartyString key,
        _Out_ PartyDataBuffer * value
        ) const party_no_throw;

    /// <summary>
    /// Gets an array of all property keys.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// The returned array is only valid until the next call to
    /// <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The output number of shared properties.
    /// </param>
    /// <param name="keys">
    /// A library-allocated output array containing the keys of all shared properties.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedPropertyKeys(
        _Out_ uint32_t * propertyCount,
        _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this endpoint object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpoint::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this endpoint object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the endpoint without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the endpoint object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpoint::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyEndpoint() = delete;
    PartyEndpoint(const PartyEndpoint&) = delete;
    PartyEndpoint(PartyEndpoint&&) = delete;
    PartyEndpoint& operator=(const PartyEndpoint&) = delete;
    PartyEndpoint& operator=(PartyEndpoint&&) = delete;
    ~PartyEndpoint() = delete;
};

/// <summary>
/// Represents a local endpoint.
/// </summary>
/// <seealso cref="PartyEndpoint" />
/// <seealso cref="PartyEndpoint::GetLocal" />
class PartyLocalEndpoint : public PartyEndpoint
{
public:
    /// <summary>
    /// Gets the local user associated with this local endpoint.
    /// </summary>
    /// <remarks>
    /// If this endpoint isn't associated with a user, <paramref name="localUser" /> is set to nullptr.
    /// </remarks>
    /// <param name="localUser">
    /// The output local user associated with this local endpoint, or nullptr.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocalUser(
        _Outptr_result_maybenull_ PartyLocalUser ** localUser
        ) const party_no_throw;

    /// <summary>
    /// Sends a message to other endpoints in the network.
    /// </summary>
    /// <remarks>
    /// Sending messages to local endpoints is not currently supported. If the array of target endpoints includes any
    /// local targets, this call will synchronously fail.
    /// <para>
    /// All targeted endpoints on a given device will be provided a single
    /// <see cref="PartyEndpointMessageReceivedStateChange" /> with every targeted local endpoint provided in the
    /// PartyEndpointMessageReceivedStateChange <em>receiverEndpoints</em> array.
    /// </para>
    /// <para>
    /// If the array of target endpoints is specified as having zero entries, then the message is broadcast to all
    /// remote endpoints currently in the network.
    /// </para>
    /// <para>
    /// Callers provide 1 or more PartyDataBuffer structures in the <paramref name="dataBuffers" /> array. The memory
    /// that the structures reference does not have to be contiguous, making it easy to have a fixed header buffer
    /// followed by a variable payload, for example. The buffers will be assembled in order, transmitted, and delivered
    /// to the targeted endpoints as a single contiguous data block in a PartyEndpointMessageReceivedStateChange. The
    /// Party library does not expend bandwidth transmitting metadata to describe the original PartyDataBuffer
    /// segmentation.
    /// </para>
    /// <para>
    /// By default, the buffers described in the caller's <paramref name="dataBuffers" /> array are copied to an
    /// allocated buffer before SendMessage() returns. Specifying
    /// <see cref="PartySendMessageOptions::DontCopyDataBuffers" /> will avoid this extra copy step and instead require
    /// the caller to keep the memory specified in each buffer valid and unmodified until a
    /// <see cref="PartyDataBuffersReturnedStateChange" /> returns ownership of the memory to the caller. The
    /// PartyDataBuffer structures themselves do not need to remain valid after the SendMessage() call returns, only the
    /// memory that they reference.
    /// </para>
    /// <para>
    /// Callers that use PartySendMessageOptions::DontCopyDataBuffers may provide a caller-specific
    /// <paramref name="messageIdentifier" /> context. This pointer-sized value will be included with all
    /// PartyDataBuffersReturnedStateChanges so that the caller can easily access its own private message tracking
    /// information. The actual value is treated as opaque and is not interpreted by the Party library, nor transmitted
    /// remotely. It's the caller's responsibility to ensure any of its own memory that
    /// <paramref name="messageIdentifier" /> might represent remains valid until the final requested state change
    /// associated with the message and associated <paramref name="messageIdentifier" /> has been processed and returned
    /// via <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// <para>
    /// Messages may not be transmitted to target endpoints right away based on factors such as connection quality and
    /// receiver responsiveness. The local send queue will grow if you are sending faster than the connection to an
    /// endpoint is estimated to currently support. This increases memory usage and may result in increases in perceived
    /// message latency, so callers are strongly recommended to monitor and manage the local send queues. You can
    /// retrieve information about the send queue using <see cref="PartyLocalEndpoint::GetEndpointStatistics()" />. You
    /// can manage the send queue by reducing the size and/or frequency of sending, by using the
    /// <paramref name="queuingConfiguration" /> optional settings to configure timeouts that automatically expire
    /// messages that have been queued for too long, or by using PartyLocalEndpoint::CancelMessages() to explicitly
    /// remove some or all queued messages.
    /// </para>
    /// <para>
    /// When this method returns success, the message has begun transmitting or has successfully been enqueued for
    /// future transmission. In particular, a successful return from this method does not imply that the message was
    /// successfully delivered to any recipients. The Party API does not currently provide a way to track the delivery
    /// and processing of individual messages. The <see cref="PartyNetwork::GetNetworkStatistics()" /> and
    /// GetEndpointStatistics() methods can be used to query aggregate statistics for the network as a whole or for an
    /// individual local endpoint, respectively.
    /// </para>
    /// <para>
    /// If <paramref name="options" /> includes <see cref="PartySendMessageOptions::GuaranteedDelivery" /> and the
    /// message could not be successfully delivered to the transparent cloud relay server for forwarding to the target
    /// endpoints, then a <see cref="PartyNetworkDestroyedStateChange" /> will be generated. In other words, messages
    /// with a guaranteed delivery requirement will either be delivered or the sending client will be disconnected from
    /// the network. When the transparent cloud relay server forwards the guaranteed delivery message to each remote
    /// device containing one or more target endpoints, if the message could not be delivered, the remote device will
    /// likewise be disconnected from the network, indicated by a PartyNetworkDestroyedStateChange. In other words, a
    /// device that fails to receive a message with a guaranteed delivery requirement will be disconnected from the
    /// network.
    /// </para>
    /// <para>
    /// The Party library automatically fragments and reassembles large messages that exceed the maximum size supported
    /// by the environment so that callers are not required to manage this. However, there is a small amount of overhead
    /// associated with fragmentation. Callers that are able to send smaller messages or otherwise naturally break up
    /// large state payloads efficiently themselves may wish to do so.
    /// </para>
    /// <para>
    /// If SendMessage() is invoked with a zero entry target endpoint array prior to successfully authenticating a first
    /// user into the network, then even though no remote endpoints have been reported via
    /// <see cref="PartyEndpointCreatedStateChange" /> state changes (and therefore known to exist in the network), the
    /// message will still be queued. Once the first user has successfully authenticated and this sending local endpoint
    /// has been successfully created, the queued message will then target all remote endpoints that exist in the
    /// network at that later time. Because the future state of the network and the set of ultimately receiving
    /// endpoints isn't known at SendMessage() time in this case, titles should exercise caution regarding what content
    /// is placed in such deferred broadcast messages, or simply refrain from submitting them at all until this local
    /// device and endpoint are fully participating in the network.
    /// </para>
    /// </remarks>
    /// <param name="targetEndpointCount">
    /// The number of target endpoints in the <paramref name="targetEndpoints" /> array. May be zero to broadcast to all
    /// remote endpoints in the network. A device that receives a broadcast message will have the target endpoint fields
    /// of the <see cref="PartyEndpointMessageReceivedStateChange" /> populated with all of the device's local
    /// endpoints.
    /// </param>
    /// <param name="targetEndpoints">
    /// The <paramref name="targetEndpointCount" /> entry array of target PartyEndpoint object pointers to which the
    /// message should be sent. This is ignored when <paramref name="targetEndpointCount" /> is zero.
    /// </param>
    /// <param name="options">
    /// Zero or more option flags describing how to send the message.
    /// </param>
    /// <param name="queuingConfiguration">
    /// An optional structure describing how the message should behave while locally queued and waiting for an
    /// opportunity to transmit. May be nullptr to use default queuing behavior.
    /// </param>
    /// <param name="dataBufferCount">
    /// The number of buffer structures provided in the <paramref name="dataBuffers" /> array. Must be greater than 0.
    /// </param>
    /// <param name="dataBuffers">
    /// The <paramref name="dataBufferCount" /> entry array of PartyDataBuffer structures describing the message payload
    /// to send.
    /// </param>
    /// <param name="messageIdentifier">
    /// An opaque, caller-specific context pointer the Party library will include in any state changes referring to this
    /// message. It is not interpreted or transmitted remotely. May be nullptr if no message identification context is
    /// needed.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if enqueuing the message for transmission succeeded or an error code otherwise. If
    /// this method fails, no related state changes will be generated. The human-readable form of the error code can be
    /// retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartySendMessageOptions" />
    /// <seealso cref="PartySendMessageQueuingConfiguration" />
    /// <seealso cref="PartyDataBuffersReturnedStateChange" />
    /// <seealso cref="PartyEndpointMessageReceivedStateChange" />
    /// <seealso_nyi cref="PartyLocalEndpoint::CancelMessages" />
    /// <seealso cref="PartyNetwork::GetNetworkStatistics" />
    /// <seealso cref="PartyLocalEndpoint::GetEndpointStatistics" />
    /// <seealso cref="PartyLocalEndpoint::FlushMessages" />
    PartyError SendMessage(
        uint32_t targetEndpointCount,
        _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints,
        PartySendMessageOptions options,
        _In_opt_ const PartySendMessageQueuingConfiguration * queuingConfiguration,
        uint32_t dataBufferCount,
        _In_reads_(dataBufferCount) const PartyDataBuffer * dataBuffers,
        _In_opt_ void * messageIdentifier
        ) party_no_throw;

    /// <summary>
    /// Cancels all or a filterable subset of messages previously submitted using <see cref="SendMessage()" /> that have
    /// not yet started transmitting.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// Matching messages in this local endpoint's send queue to the specified target endpoint(s) that have not started
    /// transmitting will be removed from the queue without being sent.
    /// </para>
    /// <para>
    /// When using <see cref="PartyCancelMessagesFilterExpression::None" /> for the <paramref name="filterExpression" />
    /// parameter, CancelMessages() will cancel all messages from this local endpoint to the specified target endpoints.
    /// Callers can instead specify a different PartyCancelMessagesFilterExpression value along with specific masks and
    /// values to match in the <paramref name="messageIdentityFilterMask" /> and
    /// <paramref name="filteredMessageIdentitiesToMatch" /> parameters in order to only match a subset of messages
    /// based on how the filter expressions evaluate the <em>identityForCancelFilters</em> field provided with each
    /// message in its <see cref="PartySendMessageQueuingConfiguration" /> structure.
    /// </para>
    /// </remarks>
    /// <param name="targetEndpointCount">
    /// The number of target endpoints in the <paramref name="targetEndpoints" /> array. May be 0 to cancel messages to
    /// all endpoints currently in the network, including this local one.
    /// </param>
    /// <param name="targetEndpoints">
    /// The <paramref name="targetEndpointCount" /> entry array of target PartyEndpoint object pointers for which
    /// messages should be canceled. This is ignored when <paramref name="targetEndpointCount" /> is zero.
    /// </param>
    /// <param name="filterExpression">
    /// Zero or more option flags describing how to perform cancelation.
    /// </param>
    /// <param name="messageIdentityFilterMask">
    /// A caller-specific value used to mask bits in messages' identity values.
    /// </param>
    /// <param name="filteredMessageIdentitiesToMatch">
    /// A caller-specific value that must match the result of the filter expression in order for a message to be
    /// selected to be canceled.
    /// </param>
    /// <param name="canceledMessagesCount">
    /// Optional output parameter to receive the number of messages that were actually canceled.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if canceling messages succeeded or an error code otherwise. The human-readable form
    /// of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyCancelMessagesFilterExpression" />
    /// <seealso cref="PartySendMessageQueuingConfiguration" />
    /// <seealso cref="PartyLocalEndpoint::SendMessage" />
    /// <nyi />
    PartyError CancelMessages(
        uint32_t targetEndpointCount,
        _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints,
        PartyCancelMessagesFilterExpression filterExpression,
        uint32_t messageIdentityFilterMask,
        uint32_t filteredMessageIdentitiesToMatch,
        _Out_opt_ uint32_t * canceledMessagesCount
        ) party_no_throw;

    /// <summary>
    /// Forces all queued messages to the specified endpoints from this local endpoint to be sent as soon as possible
    /// regardless of their coalesce settings.
    /// </summary>
    /// <remarks>
    /// When <see cref="PartySendMessageOptions::CoalesceOpportunistically" /> or
    /// <see cref="PartySendMessageOptions::AlwaysCoalesceUntilFlushed" /> is specified in a call to
    /// <see cref="SendMessage()" />, the message may not be transmitted immediately and instead be combined with other
    /// messages. This coalescing can improve bandwidth efficiency at the potential expense of perceived latency. This
    /// method allows the title to manually force such coalesced messages to begin transmitting as soon as possible.
    /// <para>
    /// Currently this method ignores the <paramref name="targetEndpoints" /> parameter and forces all queued messages
    /// from this local endpoint to be transmitted as soon as possible.
    /// </para>
    /// </remarks>
    /// <param name="targetEndpointCount">
    /// This parameter is currently ignored. The number of target endpoints in the <paramref name="targetEndpoints" />
    /// array.
    /// </param>
    /// <param name="targetEndpoints">
    /// This parameter is currently ignored. A <paramref name="targetEndpointCount" /> entry array of target
    /// PartyEndpoint object pointers. Messages from this local endpoint, up through and including the most recent
    /// message to any endpoint in the array, will be transmitted as soon as possible.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if flushing messages succeeded or an error code otherwise. The human-readable form of
    /// the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalEndpoint::SendMessage" />
    /// <seealso cref="PartySendMessageOptions" />
    PartyError FlushMessages(
        uint32_t targetEndpointCount,
        _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints
        ) party_no_throw;

    /// <summary>
    /// Gets one or more statistic counter values for the specified target endpoints.
    /// </summary>
    /// <remarks>
    /// This method is used to retrieve performance counters, queue lengths, historical usage metrics, or other
    /// statistical information recorded between this local endpoint and a target endpoint.
    /// <para>
    /// Alternatively, multiple target endpoints can be provided, or a zero-entry array to retrieve statistics for all
    /// target endpoints currently in the network, including this local one. If multiple target endpoints are requested,
    /// then the values returned are the combined statistics for those endpoints. The particular method used to combine
    /// multiple endpoint statistics into a single value depends on and is described by the specific
    /// PartyEndpointStatistic type.
    /// </para>
    /// <para>
    /// A given PartyEndpointStatistic type may appear in any order in the <paramref name="statisticTypes" /> array, but
    /// must not be specified more than once. Each corresponding statistic value will be written to the
    /// <paramref name="statisticValues" /> array in the same order.
    /// </para>
    /// <para>
    /// The returned statistic values are always the most current ones available. There is no guarantee they will report
    /// the same value from one GetEndpointStatistics() call to the next, even if there were no intervening calls to
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="targetEndpointCount">
    /// The number of target endpoints in the <paramref name="targetEndpoints" /> array. May be 0 to retrieve statistics
    /// for all endpoints currently in the network, including this local one.
    /// </param>
    /// <param name="targetEndpoints">
    /// The <paramref name="targetEndpointCount" /> entry array of target PartyEndpoint object pointers for which
    /// statistics should be retrieved. This is ignored when <paramref name="targetEndpointCount" /> is zero.
    /// </param>
    /// <param name="statisticCount">
    /// The number of statistics in the input <paramref name="statisticTypes" /> array and to be written in the
    /// <paramref name="statisticValues" /> output array. This must be at least 1.
    /// </param>
    /// <param name="statisticTypes">
    /// The <paramref name="statisticCount" /> entry input array of unique PartyEndpointStatistic types to retrieve.
    /// </param>
    /// <param name="statisticValues">
    /// The <paramref name="statisticCount" /> entry output array where the statistic values should be written. Each
    /// statistic value will be written at the same entry index corresponding to where the requested
    /// PartyEndpointStatistic appears in the <paramref name="statisticTypes" /> input array.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the endpoint statistics succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpointStatistic" />
    /// <seealso cref="PartyNetwork::GetNetworkStatistics" />
    PartyError GetEndpointStatistics(
        uint32_t targetEndpointCount,
        _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints,
        uint32_t statisticCount,
        _In_reads_(statisticCount) const PartyEndpointStatistic * statisticTypes,
        _Out_writes_all_(statisticCount) uint64_t * statisticValues
        ) const party_no_throw;

    /// <summary>
    /// Sets or removes multiple shared properties associated with this object.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If a nullptr value is passed in the array, the corresponding key will be removed. Existing properties not
    /// specified in the call will remain unchanged.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The number of properties to set or remove.
    /// </param>
    /// <param name="keys">
    /// An array of size <paramref name="propertyCount" /> containing the keys to set or remove.
    /// </param>
    /// <param name="values">
    /// An array of size <paramref name="propertyCount" /> containing the values for each key in
    /// <paramref name="keys" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError SetSharedProperties(
        uint32_t propertyCount,
        _In_reads_(propertyCount) const PartyString * keys,
        _In_reads_(propertyCount) const PartyDataBuffer * values
        ) party_no_throw;

private:
    PartyLocalEndpoint() = delete;
    PartyLocalEndpoint(const PartyLocalEndpoint&) = delete;
    PartyLocalEndpoint(PartyLocalEndpoint&&) = delete;
    PartyLocalEndpoint& operator=(const PartyLocalEndpoint&) = delete;
    PartyLocalEndpoint& operator=(PartyLocalEndpoint&&) = delete;
    ~PartyLocalEndpoint() = delete;
};

/// <summary>
/// Represents a device across networks.
/// </summary>
/// <seealso cref="PartyLocalDevice" />
class PartyDevice
{
public:
    /// <summary>
    /// Gets the <see cref="PartyLocalDevice" /> version of this device.
    /// </summary>
    /// <param name="localDevice">
    /// The output local version of this device object, or nullptr if this is not a local device.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocal(
        _Outptr_result_maybenull_ PartyLocalDevice ** localDevice
        ) const party_no_throw;

    /// <summary>
    /// Gets the chat controls on this device.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyChatControlDestroyedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the chat control will no longer be present in the
    /// array returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges(). If this is the local device, the memory for the array is also
    /// invalidated when <see cref="PartyLocalDevice::CreateChatControl()" /> returns success.
    /// </para>
    /// </remarks>
    /// <param name="chatControlCount">
    /// The output number of chat controls on this device.
    /// </param>
    /// <param name="chatControls">
    /// A library-allocated output array containing the chat controls on this device.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalDevice::CreateChatControl" />
    /// <seealso cref="PartyLocalDevice::DestroyChatControl" />
    /// <seealso cref="PartyChatControlDestroyedStateChange" />
    PartyError GetChatControls(
        _Out_ uint32_t * chatControlCount,
        _Outptr_result_buffer_(*chatControlCount) PartyChatControlArray * chatControls
        ) const party_no_throw;

    /// <summary>
    /// Gets the value of a shared property.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If there is no property associated with the key, the resulting value is nullptr. The returned value is only
    /// valid until the next call to <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="key">
    /// The key of the shared property to retrieve.
    /// </param>
    /// <param name="value">
    /// An output struct that receives a pointer to the property's value and the length in bytes of the value data. If
    /// the property was not found, the call succeeds, but the value is nullptr and the byte length is 0.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedProperty(
        PartyString key,
        _Out_ PartyDataBuffer * value
        ) const party_no_throw;

    /// <summary>
    /// Gets an array of all property keys.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// The returned array is only valid until the next call to
    /// <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The output number of shared properties.
    /// </param>
    /// <param name="keys">
    /// A library-allocated output array containing the keys of all shared properties.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedPropertyKeys(
        _Out_ uint32_t * propertyCount,
        _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this device object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDevice::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this device object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the device without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the device object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDevice::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyDevice() = delete;
    PartyDevice(const PartyDevice&) = delete;
    PartyDevice(PartyDevice&&) = delete;
    PartyDevice& operator=(const PartyDevice&) = delete;
    PartyDevice& operator=(PartyDevice&&) = delete;
    ~PartyDevice() = delete;
};

/// <summary>
/// Represents the local device.
/// </summary>
/// <seealso cref="PartyDevice" />
/// <seealso cref="PartyManager::GetLocalDevice" />
class PartyLocalDevice : public PartyDevice
{
public:
    /// <summary>
    /// Queues an asynchronous operation to create a local chat control for the specified user.
    /// </summary>
    /// <remarks>
    /// If this method returns success, a <see cref="PartyCreateChatControlCompletedStateChange" /> will be generated to
    /// provide the result of the asynchronous operation. If the asynchronous operation succeeds, a
    /// <see cref="PartyChatControlCreatedStateChange" /> will be generated. If the operation fails, a
    /// <see cref="PartyChatControlDestroyedStateChange" /> will be generated. This method itself does not make the chat
    /// control visible to any remote devices. To announce this chat control to remote devices, the title should call
    /// <see cref="PartyNetwork::ConnectChatControl()" />.
    /// <para>
    /// On successful return, this method invalidates the memory for any array previously returned by
    /// <see cref="PartyManager::GetChatControls()" /> or <see cref="PartyDevice::GetChatControls()" /> for the local
    /// device, as it synchronously adds the new chat control to the arrays.
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> also invalidates the memory for these arrays. The
    /// returned <paramref name="localChatControl" /> object will be valid until a PartyChatControlDestroyedStateChange
    /// has been generated and all state changes referencing the object have been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// <para>
    /// The language associated with this chat control can optionally be specified via the
    /// <paramref name="languageCode" /> parameter. If no language is specified, the user's default, as determined by
    /// the platform, will be used. The language is used as the spoken language associated with this chat control for
    /// transcription and the target language for incoming translations. If the language code specified is en-US, for
    /// instance, the input audio to this chat control will be treated as the English (United States) language and
    /// transcribed as such. If translation is enabled via either
    /// <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> or
    /// <see cref="PartyLocalChatControl::SetTextChatOptions()" />, the incoming voice chat transcriptions and/or text
    /// chat will be translated to English (United States).
    /// </para>
    /// <para>
    /// The language code should be in BCP 47 format; supported language codes are enumerated in
    /// <see xref="https://learn.microsoft.com/azure/ai-services/speech-service/language-support" name="Language support" />.
    /// Specifying an unsupported or invalid language code will not cause this method to fail, but will result in
    /// failure to generate transcriptions associated with this chat control. The language code used with this method
    /// can be queried via <see cref="PartyLocalChatControl::GetLanguage()" />.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    // The local user for the new local chat control.
    /// </param>
    /// <param name="languageCode">
    /// The optional language to use.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <param name="localChatControl">
    /// The optional, output local chat control.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to create a chat control began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyCreateChatControlCompletedStateChange" />
    /// <seealso cref="PartyChatControlCreatedStateChange" />
    /// <seealso cref="PartyChatControlDestroyedStateChange" />
    /// <seealso cref="PartyManager::GetChatControls" />
    /// <seealso cref="PartyDevice::GetChatControls" />
    /// <seealso cref="PartyNetwork::ConnectChatControl" />
    /// <seealso cref="PartyLocalChatControl::GetLanguage" />
    /// <seealso_nyi cref="PartyLocalChatControl::SetLanguage" />
    /// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
    /// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
    PartyError CreateChatControl(
        const PartyLocalUser * localUser,
        _In_opt_ PartyString languageCode,
        _In_opt_ void * asyncIdentifier,
        _Outptr_opt_ PartyLocalChatControl ** localChatControl
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to destroy a local chat control.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous attempt to destroy a local chat control. A
    /// <see cref="PartyDestroyChatControlCompletedStateChange" /> will be provided upon completion of the operation,
    /// indicating success or failure. Before successful completion of the operation, the local chat control will be
    /// disconnected from all networks it was previously connected to (each indicated by a
    /// <see cref="PartyChatControlLeftNetworkStateChange" />). Memory for the local chat control will remain valid
    /// until all state changes referencing the chat control have been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </remarks>
    /// <param name="localChatControl">
    /// The local chat control to destroy.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to destroy the chat control began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDestroyChatControlCompletedStateChange" />
    /// <seealso cref="PartyChatControlDestroyedStateChange" />
    /// <seealso cref="PartyChatControlLeftNetworkStateChange" />
    PartyError DestroyChatControl(
        _In_ PartyLocalChatControl * localChatControl,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Sets or removes multiple shared properties associated with this object.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If a nullptr value is passed in the array, the corresponding key will be removed. Existing properties not
    /// specified in the call will remain unchanged.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The number of properties to set or remove.
    /// </param>
    /// <param name="keys">
    /// An array of size <paramref name="propertyCount" /> containing the keys to set or remove.
    /// </param>
    /// <param name="values">
    /// An array of size <paramref name="propertyCount" /> containing the values for each key in
    /// <paramref name="keys" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError SetSharedProperties(
        uint32_t propertyCount,
        _In_reads_(propertyCount) const PartyString * keys,
        _In_reads_(propertyCount) const PartyDataBuffer * values
        ) party_no_throw;

private:
    PartyLocalDevice() = delete;
    PartyLocalDevice(const PartyLocalDevice&) = delete;
    PartyLocalDevice(PartyLocalDevice&&) = delete;
    PartyLocalDevice& operator=(const PartyLocalDevice&) = delete;
    PartyLocalDevice& operator=(PartyLocalDevice&&) = delete;
    ~PartyLocalDevice() = delete;
};

/// <summary>
/// Represents an invitation used to authenticate a user into a network.
/// </summary>
/// <seealso cref="PartyManager::CreateNewNetwork" />
/// <seealso cref="PartyNetwork::CreateInvitation" />
/// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
/// <seealso cref="PartyInvitationConfiguration" />
class PartyInvitation
{
public:
    /// <summary>
    /// Gets the PlayFab Entity ID of the user that created this invitation or nullptr if this invitation was generated
    /// as part of a <see cref="PartyManager::CreateNewNetwork()" /> operation.
    /// </summary>
    /// <param name="entityId">
    /// The output Entity ID of the user that created this invitation, or nullptr.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetCreatorEntityId(
        _Outptr_result_maybenull_ PartyString * entityId
        ) const party_no_throw;

    /// <summary>
    /// Gets this invitation's configuration.
    /// </summary>
    /// <remarks>
    /// The memory for the <see cref="PartyInvitationConfiguration" /> and all memory it references are valid for the
    /// lifetime of the invitation object.
    /// </remarks>
    /// <param name="configuration">
    /// The output configuration of this invitation.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyInvitationConfiguration" />
    PartyError GetInvitationConfiguration(
        _Outptr_ const PartyInvitationConfiguration ** configuration
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this invitation
    /// object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyInvitation::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this invitation object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the invitation without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the invitation object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyInvitation::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyInvitation() = delete;
    PartyInvitation(const PartyInvitation&) = delete;
    PartyInvitation(PartyInvitation&&) = delete;
    PartyInvitation& operator=(const PartyInvitation&) = delete;
    PartyInvitation& operator=(PartyInvitation&&) = delete;
    ~PartyInvitation() = delete;
};

/// <summary>
/// Represents a network.
/// </summary>
class PartyNetwork
{
public:
    /// <summary>
    /// Queues an asynchronous operation to authenticate the specified local user into the network.
    /// </summary>
    /// <remarks>
    /// While any device with the appropriate network descriptor can establish a connection to the network, no useful
    /// actions can be performed, and no useful information can be gleaned, without first authenticating a local user on
    /// the device into the network. As such, authentication is an important security measure to ensure the integrity of
    /// the network and the privacy of the devices and users participating in the network. To authenticate a user is to
    /// have the transparent cloud relay server validate the user's identity and that the user is allowed to join the
    /// network.
    /// <para>
    /// Upon successful completion of the asynchronous operation started by this call, a user is considered
    /// authenticated into the network. Completion is indicated by the
    /// <see cref="PartyAuthenticateLocalUserCompletedStateChange" />. Successful authentication requires that the local
    /// user's PlayFab Entity Token is valid and is associated with the local user's PlayFab Entity ID. Successful
    /// authentication also requires that the provided invitation allows the provided local user to join the network.
    /// Authentication will fail if authenticating the provided user would violate one of the limits in the network's
    /// configuration. These requirements are enforced by the transparent cloud relay server. If the asynchronous
    /// operation fails, the PartyAuthenticateLocalUserCompletedStateChange will provide a diagnostic result and error
    /// detail, and a <see cref="PartyLocalUserRemovedStateChange" /> will be generated. Because being connected to a
    /// network with no authenticated users is not a useful state outside of transition periods, the Party library will
    /// automatically disconnect a device that doesn't have an authenticated user for an extended period.
    /// </para>
    /// <para>
    /// Users need an <paramref name="invitationIdentifier" /> to call this method and authenticate into the network.
    /// Most commonly this means that another device will have to share the identifier of the initial invitation
    /// generated via <see cref="PartyManager::CreateNewNetwork()" /> or share the identifier of an invitation it has
    /// generated via <see cref="CreateInvitation()" />.
    /// </para>
    /// <para>
    /// User authentication also determines device authentication. A device is considered authenticated into the network
    /// if at least one local user is authenticated into the network. If a device is authenticated into the network, it
    /// will be visible to all other authenticated devices. If a device is connected to the network but not
    /// authenticated, no remote devices, endpoints, or chat controls, will be visible. Similarly, the device won't be
    /// visible to any other devices connected to the network.
    /// </para>
    /// <para>
    /// Once this method is called, all other methods that queue asynchronous operations but require the local user or
    /// local device to be authenticated into the network can be called. Such operations will be queued until completion
    /// of the authentication operation. Once the authentication operation completes, the queued operations will be
    /// processed. If the authentication operation failed, the queued operations will also fail and their respective
    /// completion state changes will indicate such.
    /// </para>
    /// <para>
    /// If the local device enters a state in which there are no authenticated users and no authentication operations in
    /// progress, then all endpoints, including those that aren't fully created, will be destroyed automatically. This
    /// will be signaled by <see cref="PartyEndpointDestroyedStateChange" />s.
    /// </para>
    /// <para>
    /// On successful return, this method invalidates the memory for any array previously returned by
    /// <see cref="PartyNetwork::GetLocalUsers()" />, as it synchronously adds the new user to the array.
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> also invalidates the memory for the array.
    /// </para>
    /// <para>
    /// This method will fail if the specified <paramref name="localUser" /> is in the process of authenticating into
    /// the network, is already authenticated into the network, is authenticated but in the process of being removed due
    /// to a previous call to <see cref="PartyNetwork::RemoveLocalUser()" />, or if the device overall is in the process
    /// of becoming unauthenticated due to its last authenticated user being removed from the network.
    /// </para>
    /// <h>Retrying on failure</h>
    /// <para>
    /// If the PartyAuthenticateLocalUserCompletedStateChange associated with the completion of this call indicates a
    /// failure, the operation may be retried after a delay. The proper retry behavior depends on the
    /// <see cref="PartyStateChangeResult" /> provided in the state change.
    /// </para>
    /// <para>
    /// `` | Result | Retry behavior |
    /// ` | --- | --- |
    /// ` | InternetConnectivityError | Retry with a small delay of no less than 10 seconds. For your app, it may be
    ///         more appropriate to display the error to the user immediately, rather than retrying automatically. |
    /// ` | PartyServiceError | Retry with an exponential backoff. Start with a minimum delay of no less than 10
    ///         seconds, doubling the delay with each retry. |
    /// ` | UserNotAuthorized | This result can mean that the user's entity token was invalid, expired, or that the user
    ///         wasn't authorized for other reasons. It could also mean that specified invitation is no longer valid, or
    ///         the invitation doesn't contain this user. Retry no more than one time, and only after getting a new
    ///         entity token for the user and calling <see cref="PartyLocalUser::UpdateEntityToken()" />.|
    /// ` | FailedToBindToLocalUdpSocket | This result means that the library couldn't bind to the local UDP socket
    ///         specified in the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option. The title must clean up
    ///         its instance of the library, update the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option to
    ///         a valid, available bind address, and reinitialize the library.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    /// The local user to authenticate into the network.
    /// </param>
    /// <param name="invitationIdentifier">
    /// The identifier of the invitation that allows the <paramref name="localUser" /> to authenticate into the network.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to authenticate the local user began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAuthenticateLocalUserCompletedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyLocalUserRemovedStateChange" />
    /// <seealso cref="PartyNetworkConfiguration" />
    /// <seealso cref="PartyManager::CreateLocalUser" />
    /// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
    /// <seealso cref="PartyNetwork::RemoveLocalUser" />
    /// <seealso cref="PartyNetwork::GetLocalUsers" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    PartyError AuthenticateLocalUser(
        const PartyLocalUser * localUser,
        PartyString invitationIdentifier,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to remove a local user from the network.
    /// </summary>
    /// <remarks>
    /// Upon completion of the asynchronous operation, a user is no longer considered authenticated into the network.
    /// Completion is indicated by a <see cref="PartyRemoveLocalUserCompletedStateChange" />. The asynchronous operation
    /// will always succeed, and a <see cref="PartyLocalUserRemovedStateChange" /> will always be provided before the
    /// PartyRemoveLocalUserCompletedStateChange. Before successful completion of the operation, all endpoints
    /// associated with that user will be destroyed (each indicated by a
    /// <see cref="PartyEndpointDestroyedStateChange" />) and all chat controls associated with that user will be
    /// disconnected from the network (each indicated by a <see cref="PartyChatControlLeftNetworkStateChange" />).
    /// <para>
    /// When all local users are removed from the network, the device is no longer considered authenticated. All
    /// endpoints are destroyed and all remote devices will appear to leave the network, because they're no longer
    /// visible to the local device. Similarly, all remote devices will see the local device as having left the network.
    /// The device won't immediately be disconnected and can again be authenticated by a new call to
    /// <see cref="AuthenticateLocalUser()" />. However, because being connected to a network with no authenticated
    /// users isn't a useful state outside of transition periods, the Party library will automatically disconnect a
    /// device that is unauthenticated for more than one minute.
    /// </para>
    /// <para>
    /// This method fails if the specified <paramref name="localUser" /> is already in the process of being removed from
    /// the network due to a previous call to this method.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    /// The local user to remove from the network.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to remove the local user began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyRemoveLocalUserCompletedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyChatControlLeftNetworkStateChange" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyNetwork::GetLocalUsers" />
    PartyError RemoveLocalUser(
        const PartyLocalUser * localUser,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous attempt to create an invitation for the network.
    /// </summary>
    /// <remarks>
    /// To join a network, a user must provide an invitation to <see cref="AuthenticateLocalUser()" />.
    /// <para>
    /// This method queues an asynchronous attempt to create an invitation for the network. A
    /// <see cref="PartyCreateInvitationCompletedStateChange" /> will be provided upon completion of the asynchronous
    /// operation, indicating success or failure. On success, a <see cref="PartyInvitationCreatedStateChange" /> will
    /// have been generated beforehand to indicate that the transparent cloud relay server is ready to accept
    /// authentications using the invitation's identifier. On failure, a
    /// <see cref="PartyInvitationDestroyedStateChange" /> will have been generated beforehand to indicate that the
    /// invitation object is no longer valid and will no longer be queryable via <see cref="GetInvitations()" />.
    /// </para>
    /// <para>
    /// The lifetime of the invitation is tied to the owning local user's membership in the network, therefore
    /// <paramref name="localUser" /> must be authenticated or in the process of authenticating. If
    /// <paramref name="localUser" /> leaves the network for any reason the invitation will be automatically revoked and
    /// subsequent attempts to use that invitation's identifier to join the network will fail until a new invitation is
    /// created using that identifier.
    /// </para>
    /// <para>
    /// Invitations created with this method will only be queryable via GetInvitations() on the local device where the
    /// invitation was created.
    /// </para>
    /// <para>
    /// On successful return, this method invalidates the memory for any array previously returned by GetInvitations(),
    /// as it synchronously adds the new invitation to the array.
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> also invalidates the memory for the array. The
    /// returned <paramref name="invitation" /> object will be valid until a PartyInvitationDestroyedStateChange has
    /// been generated and all state changes referencing the object have been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    /// The local user that owns the invitation in the network. If this local user leaves the network for any reason the
    /// invitation will be automatically revoked.
    /// </param>
    /// <param name="invitationConfiguration">
    /// An optionally specified configuration for the newly created invitation.
    /// <para>
    /// If this value is null, the default configuration values will be used. By default, PlayFab Party will generate a
    /// unique invitation identifier for the title, the revocability will be
    /// <see cref="PartyInvitationRevocability::Creator" />, and the PlayFab Entity ID list will be empty, allowing any
    /// user to join using the invitation.
    /// </para>
    /// <para>
    /// If a configuration is provided, the title may optionally specify the identifier on the configuration. If the
    /// identifier is nullptr or an empty string, the PlayFab Party library will generate an identifier for the title.
    /// It is guaranteed that this generated identifier will be different from all identifiers that the PlayFab Party
    /// library has already generated for invitations on this network across all devices. Titles may specify their own
    /// identifier by providing a non-null, non-empty value in the configuration. If the title specifies the identifier,
    /// it is the title's responsibility to ensure that this identifier does not collide with the identifiers of other
    /// invitations created on this network via <see cref="PartyManager::CreateNewNetwork()" /> or CreateInvitation() on
    /// any device. If the title attempts to create an invitation with an identifier that would collide with a
    /// pre-existing invitation, then the operation will fail asynchronously and the title will receive a
    /// PartyInvitationDestroyedStateChange followed by a <see cref="PartyCreateInvitationCompletedStateChange" /> with
    /// a failure result.
    /// </para>
    /// <para>
    /// If a configuration is provided, its revocability must be PartyInvitationRevocability::Creator.
    /// </para>
    /// <para>
    /// If a configuration is provided and the list of PlayFab Entity IDs is empty, all users will be allowed to join
    /// using the new invitation.
    /// </para>
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <param name="invitation">
    /// The optional, output invitation that may immediately be used to queue user authentications on a network via
    /// <see cref="AuthenticateLocalUser()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to create an invitation began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyInvitationRevocability" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyInvitationCreatedStateChange" />
    /// <seealso cref="PartyCreateInvitationCompletedStateChange" />
    /// <seealso cref="PartyInvitationDestroyedStateChange" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    PartyError CreateInvitation(
        const PartyLocalUser * localUser,
        _In_opt_ const PartyInvitationConfiguration * invitationConfiguration,
        void * asyncIdentifier,
        _Outptr_opt_ PartyInvitation ** invitation
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous attempt to revoke an invitation from the network.
    /// </summary>
    /// <remarks>
    /// Titles may revoke an invitation to prevent further users from authenticating into a network using the
    /// invitation's identifier.
    /// <para>
    /// This method queues an asynchronous attempt to revoke an invitation from the network. A
    /// <see cref="PartyRevokeInvitationCompletedStateChange" /> will be provided upon completion of the asynchronous
    /// operation, indicating success or failure. On success, a <see cref="PartyInvitationDestroyedStateChange" /> will
    /// have been generated beforehand with <see cref="PartyDestroyedReason::Requested" />, to indicate that the network
    /// will no longer accept authentications using the invitation's identifier until a new invitation is created with
    /// that same identifier. It also indicates that the invitation will no longer be queryable via
    /// <see cref="GetInvitations()" />.
    /// </para>
    /// <para>
    /// This operation will only succeed if the invitation was created with its revocability specified such that the
    /// <paramref name="localUser" /> on the local device may revoke the invitation. This might not be the case if the
    /// revocability is set to <see cref="PartyInvitationRevocability::Creator" /> and the <paramref name="localUser" />
    /// is authenticated on a different device than the creator or if the <paramref name="localUser" /> refers to a
    /// different user than the creator.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    /// The local user attempting to revoke the invitation.
    /// </param>
    /// <param name="invitation">
    /// The invitation to revoke.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to revoke the invitation began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyRevokeInvitationCompletedStateChange" />
    /// <seealso cref="PartyInvitationDestroyedStateChange" />
    PartyError RevokeInvitation(
        const PartyLocalUser * localUser,
        PartyInvitation * invitation,
        void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets an array containing all invitations exposed to the local device.
    /// </summary>
    /// <remarks>
    /// The array is backed by the library's internal memory. The array is only valid until the next call to either
    /// <see cref="CreateInvitation()" /> or <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// <para>
    /// A new invitation is exposed to the local device whenever the local device calls CreateInvitation() or a
    /// <see cref="PartyInvitationCreatedStateChange" /> is provided via PartyManager::StartProcessingStateChanges().
    /// </para>
    /// <para>
    /// Invitations created via CreateInvitation() will only be exposed to the local device. The initial invitation will
    /// be exposed to all devices in the network via a PartyInvitationCreatedStateChange.
    /// </para>
    /// <para>
    /// Once a <see cref="PartyInvitationDestroyedStateChange" /> has been provided by
    /// PartyManager::StartProcessingStateChanges(), the invitation will no longer be present in the array returned by
    /// this method.
    /// </para>
    /// </remarks>
    /// <param name="invitationCount">
    /// An output value indicating the number of invitations provided in <paramref name="invitations" />.
    /// </param>
    /// <param name="invitations">
    /// A library-allocated output array containing the invitations that have been created for the network.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    /// <seealso cref="PartyNetwork::RevokeInvitation" />
    /// <seealso cref="PartyInvitationCreatedStateChange" />
    /// <seealso cref="PartyInvitationDestroyedStateChange" />
    PartyError GetInvitations(
        _Out_ uint32_t* invitationCount,
        _Outptr_result_buffer_(*invitationCount) PartyInvitationArray * invitations
        ) const party_no_throw;

    /// <summary>
    /// Queues an asynchronous attempt to create a local endpoint.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous attempt to create an endpoint associated with the local device on this
    /// network. A <see cref="PartyCreateEndpointCompletedStateChange" /> will be provided upon completion of the
    /// operation, indicating success or failure. On success, a <see cref="PartyEndpointCreatedStateChange" /> will be
    /// generated, and the endpoint will be fully created, connected to the network, and visible to all authenticated
    /// devices in the network. On failure, a <see cref="PartyEndpointDestroyedStateChange" /> will be generated.
    /// <para>
    /// This method optionally provides <paramref name="localEndpoint" /> as output that can immediately be used to
    /// perform asynchronous endpoint operations, such as <see cref="PartyLocalEndpoint::SendMessage()" /> and
    /// PartyLocalEndpoint::SetSharedProperties(). These asynchronous operations are internally queued until the
    /// endpoint creation completes, at which point they will be processed.
    /// <see cref="PartyEndpoint::GetUniqueIdentifier()" /> returns a failure until the endpoint creation completes.
    /// This <paramref name="localEndpoint" /> will also be provided in the resulting
    /// PartyCreateEndpointCompletedStateChange.
    /// </para>
    /// <para>
    /// A local user may optionally be provided as the owner of an endpoint. If an owning local user is provided, it
    /// must be authenticated into the network or in the process of authenticating into the network. If its
    /// authentication fails, the endpoint creation will fail as well. If the owning local user is removed from the
    /// network while this endpoint exists, the endpoint will be automatically destroyed. This will be signaled via a
    /// PartyEndpointDestroyedStateChange.
    /// </para>
    /// <para>
    /// If the local device enters a state in which there are no authenticated users and no authentication operations in
    /// progress, then all endpoints, including those that are not fully created, are destroyed automatically. This will
    /// be signaled by PartyEndpointDestroyedStateChanges.
    /// </para>
    /// <para>
    /// On successful return, this method invalidates the memory for any array previously returned by
    /// <see cref="GetEndpoints()" />, as it synchronously adds the new endpoint to the array.
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> also invalidates the memory for the array. The
    /// returned <paramref name="localEndpoint" /> object remains valid until a PartyEndpointDestroyedStateChange has
    /// been generated and all state changes referencing the object have been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// <para>
    /// The property bag is a collection of title-specific values associated with an endpoint. The initial property bag
    /// is queryable as soon as the endpoint becomes visible.
    /// </para>
    /// <para>
    /// If a client would violate the <see cref="PartyNetworkConfiguration::maxEndpointsPerDeviceCount" /> limit by
    /// calling this method after the network configuration was made available, this operation fails synchronously. If
    /// the client queues a violating number of endpoint creations before the network configuration becomes available,
    /// the client will be kicked from the network and a <see cref="PartyNetworkDestroyedStateChange" /> will be
    /// generated when the network configuration becomes available.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    /// An optional local user to associate as the owner of this endpoint. When this endpoint becomes visible on remote
    /// devices, the user's identifier is tied to this endpoint. The endpoint will be destroyed if the user becomes
    /// unauthenticated because they were voluntarily removed via <see cref="RemoveLocalUser()" /> or kicked via
    /// PartyNetwork::KickUser().
    /// </param>
    /// <param name="propertyCount">
    /// The number of properties in the input <paramref name="keys" /> and <paramref name="values" /> arrays.
    /// </param>
    /// <param name="keys">
    /// The <paramref name="propertyCount" /> entry array of property bag keys. The nth key in this array maps to the
    /// nth value in the <paramref name="values" /> array. This parameter must be nullptr if
    /// <paramref name="propertyCount" /> is 0.
    /// </param>
    /// <param name="values">
    /// The <paramref name="propertyCount" /> entry array of property bag values. The nth value in this array is mapped
    /// by the nth key in the <paramref name="keys" /> array. This parameter must be nullptr if
    /// <paramref name="propertyCount" /> is 0.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <param name="localEndpoint">
    /// The optional, output local endpoint object on which to queue endpoint operations.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to create the endpoint began, or an error code
    /// otherwise. If this method fails, no related state changes are generated. The human-readable form of the error
    /// code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyCreateEndpointCompletedStateChange" />
    /// <seealso cref="PartyEndpointCreatedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyNetworkDestroyedStateChange" />
    /// <seealso cref="PartyNetworkConfiguration" />
    /// <seealso cref="PartyNetwork::GetEndpoints" />
    /// <seealso cref="PartyLocalEndpoint::SendMessage" />
    /// <seealso_nyi cref="PartyLocalEndpoint::SetSharedProperties" />
    /// <seealso cref="PartyEndpoint::GetUniqueIdentifier" />
    /// <seealso cref="PartyEndpoint::GetEntityId" />
    /// <seealso cref="PartyEndpoint::GetSharedProperty" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyNetwork::RemoveLocalUser" />
    /// <seealso_nyi cref="PartyNetwork::KickUser" />
    PartyError CreateEndpoint(
        _In_opt_ const PartyLocalUser * localUser,
        uint32_t propertyCount,
        _In_reads_opt_(propertyCount) const PartyString * keys,
        _In_reads_opt_(propertyCount) const PartyDataBuffer * values,
        _In_opt_ void * asyncIdentifier,
        _Outptr_opt_ PartyLocalEndpoint ** localEndpoint
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to destroy a local endpoint.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous operation to destroy an endpoint associated with the local device on this
    /// network. A <see cref="PartyDestroyEndpointCompletedStateChange" /> will be provided upon completion of the
    /// operation, indicating success or failure. On success, the endpoint is no longer connected to the network or
    /// visible to any devices. Memory for the endpoint will remain valid until all state changes referencing the
    /// endpoint have been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// If this call returns success, asynchronous endpoint destruction has begun and methods that attempt to send data
    /// from this local endpoint to the Party network, such as <see cref="PartyLocalEndpoint::SendMessage()" /> and
    /// <see cref="PartyLocalEndpoint::FlushMessages()" />, will fail. Methods that retrieve cached state, such as
    /// <see cref="PartyEndpoint::GetCustomContext()" /> and <see cref="PartyEndpoint::GetEntityId()" />, will continue
    /// to succeed.
    /// </para>
    /// <para>
    /// When the local device begins destroying an endpoint, every remote device will be alerted via a
    /// <see cref="PartyEndpointDestroyedStateChange" />. Those remote devices have the opportunity to send the
    /// destructing endpoint any final messages they wish before returning the PartyEndpointDestroyedStateChange to
    /// PartyManager::FinishProcessingStateChanges(). By returning the PartyEndpointDestroyedStateChange, the remote
    /// device acknowledges the endpoint's destruction and may no longer target the endpoint in
    /// PartyLocalEndpoint::SendMessage() calls. Because the endpoint will not be able to complete its destruction on
    /// the local device until all remote devices have acknowledged it, it is recommended to return this state change as
    /// quickly as possible. If the remote device does not return the PartyEndpointDestroyedStateChange within two
    /// seconds, the remote device will automatically acknowledge the endpoint destruction and the endpoint will no
    /// longer be targetable in PartyLocalEndpoint::SendMessage() calls. The destructing endpoint can receive
    /// <see cref="PartyEndpointMessageReceivedStateChange" />s up until all remote devices have acknowledged the
    /// endpoint's destruction, which is indicated by a PartyEndpointDestroyedStateChange generated on the local device.
    /// </para>
    /// </remarks>
    /// <param name="localEndpoint">
    /// The local endpoint to begin destroying.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to destroy the endpoint began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDestroyEndpointCompletedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyEndpointMessageReceivedStateChange" />
    /// <seealso cref="PartyLocalEndpoint::SendMessage" />
    /// <seealso cref="PartyEndpoint::GetCustomContext" />
    /// <seealso cref="PartyEndpoint::GetEntityId" />
    PartyError DestroyEndpoint(
        _In_ PartyLocalEndpoint * localEndpoint,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to leave the network gracefully.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous operation to gracefully leave the network. A
    /// <see cref="PartyLeaveNetworkCompletedStateChange" /> will be provided upon completion of the operation,
    /// indicating success or failure. Memory for the network will remain valid until all state changes referencing the
    /// network have been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// Before successful completion of the operation, all in-flight inbound and outbound traffic to and from all local
    /// endpoints on this device will be delivered, state changes will be generated on this device showing all local and
    /// remote endpoints being destroyed (each indicated by a <see cref="PartyEndpointDestroyedStateChange" />), all
    /// chat controls being disconneced from this network (each indicated by a
    /// <see cref="PartyChatControlLeftNetworkStateChange" />), all remote devices leaving the network (each indicated
    /// by a <see cref="PartyRemoteDeviceLeftNetworkStateChange" />), and all local users being removed from this
    /// network (each indicated by a <see cref="PartyLocalUserRemovedStateChange" />).
    /// </para>
    /// <para>
    /// This object will be valid until a <see cref="PartyNetworkDestroyedStateChange" /> has been generated and all
    /// state changes referencing the object have been returned to PartyManager::FinishProcessingStateChanges().
    /// </para>
    /// </remarks>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to leave the network began, or an error code otherwise.
    /// If this method fails, no related state changes will be generated. The human-readable form of the error code can
    /// be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetworkDestroyedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyChatControlLeftNetworkStateChange" />
    /// <seealso cref="PartyRemoteDeviceLeftNetworkStateChange" />
    /// <seealso cref="PartyLocalUserRemovedStateChange" />
    PartyError LeaveNetwork(
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the endpoints in this network.
    /// </summary>
    /// <remarks>
    /// This method gets all endpoints currently associated with the network and visible to the local device.
    /// <para>
    /// All local endpoints that have successfully been created or are in the process of being created will be present
    /// in <paramref name="endpoints" />. All remote endpoints that have been successfully created will also be present.
    /// </para>
    /// <para>
    /// Once a <see cref="PartyEndpointDestroyedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the endpoint will no longer be present in the array
    /// returned by this method.
    /// </para>
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or <see cref="CreateEndpoint()" /> returns success.
    /// </para>
    /// </remarks>
    /// <param name="endpointCount">
    /// The output number of PartyEndpoint entries in the <paramref name="endpoints" /> array.
    /// </param>
    /// <param name="endpoints">
    /// A library-allocated output array containing the endpoints in this network.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpointCreatedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyNetwork::CreateEndpoint" />
    /// <seealso cref="PartyNetwork::GetEndpointsByUserType" />
    PartyError GetEndpoints(
        _Out_ uint32_t * endpointCount,
        _Outptr_result_buffer_(*endpointCount) PartyEndpointArray * endpoints
        ) const party_no_throw;

    /// <summary>
    /// Gets a subset of endpoints in this network filtered by user type and owning device location.
    /// </summary>
    /// <remarks>
    /// This method gets the subset of endpoints that are currently associated with the network, visible to the local
    /// device, and that match specified user type and owning device location filters.
    /// <para>
    /// If <paramref name="endpointLocationFilter" /> is PartyEndpointLocationFilter::LocalOrRemote or
    /// PartyEndpointLocationFilter::LocalOnly, then all matching local endpoints that have successfully been created or
    /// are in the process of being created will be present in <paramref name="endpoints" />. If
    /// <paramref name="endpointLocationFilter" /> is PartyEndpointLocationFilter::LocalOrRemote or
    /// PartyEndpointLocationFilter::RemoteOnly, then all matching remote endpoints that have been successfully created
    /// will be present.
    /// </para>
    /// <para>
    /// Once a <see cref="PartyEndpointDestroyedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the endpoint will no longer be present in the array
    /// returned by this method with applicable filters.
    /// </para>
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or <see cref="CreateEndpoint()" /> returns success.
    /// </para>
    /// </remarks>
    /// <param name="endpointUserTypeFilter">
    /// The specific type of users whose PartyEndpoints should be included in the output array.
    /// </param>
    /// <param name="endpointLocationFilter">
    /// Specifies whether to include PartyEndpoints created by the local device, remote devices, or both in the output
    /// array.
    /// </param>
    /// <param name="endpointCount">
    /// The output number of PartyEndpoint entries in the <paramref name="endpoints" /> array.
    /// </param>
    /// <param name="endpoints">
    /// A library-allocated output array containing the endpoints in this network.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpointUserTypeFilter" />
    /// <seealso cref="PartyEndpointLocationFilter" />
    /// <seealso cref="PartyEndpointCreatedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    /// <seealso cref="PartyNetwork::CreateEndpoint" />
    /// <seealso cref="PartyNetwork::GetEndpoints" />
    PartyError GetEndpointsByUserType(
        PartyEndpointUserTypeFilter endpointUserTypeFilter,
        PartyEndpointLocationFilter endpointLocationFilter,
        _Out_ uint32_t * endpointCount,
        _Outptr_result_buffer_(*endpointCount) PartyEndpointArray * endpoints
        ) const party_no_throw;

    /// <summary>
    /// Finds the endpoint with the corresponding network-unique identifier in this network, if it exists.
    /// </summary>
    /// <remarks>
    /// This method returns an error if the endpoint is not valid on the local device, that is, if the
    /// <see cref="PartyEndpointCreatedStateChange" /> for the endpoint associated with
    /// <paramref name="uniqueIdentifier" /> has not yet been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or if the
    /// <see cref="PartyEndpointDestroyedStateChange" /> has been generated and all state changes referencing the
    /// endpoint have been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// <para>
    /// All devices in a network will agree on a given endpoint's unique identifier, but different devices may not see
    /// the same endpoints at a given moment. For example, it's possible for endpoint A to send a message to endpoint B
    /// that references a newly-created endpoint C's unique identifier, but that message between A and B may arrive
    /// before the PartyEndpointCreatedStateChange for endpoint C is generated on endpoint B's device. In that
    /// situation, this method will return an error when called on endpoint B's device because endpoint C is not yet
    /// known on that device.
    /// </para>
    /// </remarks>
    /// <param name="uniqueIdentifier">
    /// The network-unique identifier of an endpoint.
    /// </param>
    /// <param name="endpoint">
    /// The output endpoint with a network-unique identifier matching <paramref name="uniqueIdentifier" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if an endpoint with a matching identifier was found on this network or an error code
    /// otherwise. The human-readable form of the error code can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyEndpoint::GetUniqueIdentifier" />
    /// <seealso cref="PartyEndpointCreatedStateChange" />
    /// <seealso cref="PartyEndpointDestroyedStateChange" />
    PartyError FindEndpointByUniqueIdentifier(
        uint16_t uniqueIdentifier,
        _Outptr_ PartyEndpoint ** endpoint
        ) const party_no_throw;

    /// <summary>
    /// Gets the devices in this network.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyRemoteDeviceLeftNetworkStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the remote device will no longer be present in the
    /// array returned by this method. The local device is always in the array.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges().
    /// </para>
    /// </remarks>
    /// <param name="deviceCount">
    /// The output number of devices in this network.
    /// </param>
    /// <param name="devices">
    /// A library-allocated output array containing the devices in this network.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyRemoteDeviceLeftNetworkStateChange" />
    PartyError GetDevices(
        _Out_ uint32_t * deviceCount,
        _Outptr_result_buffer_(*deviceCount) PartyDeviceArray * devices
        ) const party_no_throw;

    /// <summary>
    /// Gets an array containing the local users that are authenticated or authenticating into the network.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyLocalUserRemovedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the local user will no longer be present in the
    /// array returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges() or <see cref="AuthenticateLocalUser()" /> returns success.
    /// </para>
    /// </remarks>
    /// <param name="userCount">
    /// An output value indicating the number of users provided in <paramref name="localUsers" />.
    /// </param>
    /// <param name="localUsers">
    /// A library-allocated output array containing the local users that are authenticated or authenticating into the
    /// network.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyNetwork::RemoveLocalUser" />
    /// <seealso cref="PartyLocalUserRemovedStateChange" />
    PartyError GetLocalUsers(
        _Out_ uint32_t * userCount,
        _Outptr_result_buffer_(*userCount) PartyLocalUserArray * localUsers
        ) const party_no_throw;

    /// <summary>
    /// Gets the network descriptor for this network.
    /// </summary>
    /// <remarks>
    /// A network descriptor contains all the information required for a device to connect to the network to which the
    /// descriptor is associated. <see cref="PartyManager::SerializeNetworkDescriptor()" /> can be used to get the
    /// serialized form of the descriptor, which is safe to exchange over title and platform-specific communication
    /// channels. However, the network descriptor provided by this call is not serializable until the asynchronous
    /// creation operation associated with this network successfully completes, signaled by a
    /// <see cref="PartyCreateNewNetworkCompletedStateChange" /> with a success result code.
    /// <para>
    /// PartyNetworkDescriptors work in tandem with PartyInvitations to facilitate inviting remote users to join the
    /// network. First, obtain the network descriptor via GetNetworkDescriptor() and serialize it via
    /// PartyManager::SerializeNetworkDescriptor(). Next, create an invitation via <see cref="CreateInvitation()" /> or
    /// query for a preexisting invitation via <see cref="GetInvitations()" /> and obtain the invitation's identifier
    /// from its configuration via <see cref="PartyInvitation::GetInvitationConfiguration()" />. Last, include both the
    /// serialized network descriptor and the invitation identifier in the payload of the platform-specific invite
    /// mechanism. When the remote user receives the invite, they deserialize the network descriptor from the
    /// platform-invite payload via <see cref="PartyManager::DeserializeNetworkDescriptor()" /> and pass the
    /// deserialized descriptor to <see cref="PartyManager::ConnectToNetwork()" />. After connecting, the remote user
    /// joins by authenticating into the network via <see cref="AuthenticateLocalUser()" /> with the invitation
    /// identifier in the platform-invite payload.
    /// </para>
    /// </remarks>
    /// <param name="networkDescriptor">
    /// The output network descriptor.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyCreateNewNetworkCompletedStateChange" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    /// <seealso cref="PartyManager::SerializeNetworkDescriptor" />
    /// <seealso cref="PartyManager::DeserializeNetworkDescriptor" />
    /// <seealso cref="PartyManager::ConnectToNetwork" />
    /// <seealso cref="PartyInvitation" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    /// <seealso cref="PartyNetwork::GetInvitations" />
    /// <seealso cref="PartyInvitation::GetInvitationConfiguration" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    PartyError GetNetworkDescriptor(
        _Out_ PartyNetworkDescriptor * networkDescriptor
        ) const party_no_throw;

    /// <summary>
    /// Gets the network configuration which was set when creating the network.
    /// </summary>
    /// <remarks>
    /// This method returns an error until <see cref="PartyManager::StartProcessingStateChanges()" /> provides a
    /// <see cref="PartyNetworkConfigurationMadeAvailableStateChange" />.
    /// </remarks>
    /// <param name="networkConfiguration">
    /// The output network configuration.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetworkConfigurationMadeAvailableStateChange" />
    PartyError GetNetworkConfiguration(
        _Outptr_ const PartyNetworkConfiguration ** networkConfiguration
        ) const party_no_throw;

    /// <summary>
    /// Kicks a device from the network.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// </remarks>
    /// <param name="targetDevice">
    /// The device to kick.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to kick the device began, or an error code otherwise.
    /// If this method fails, no related state changes will be generated. The human-readable form of the error code can
    /// be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError KickDevice(
        const PartyDevice * targetDevice,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Kicks a user from the network along with all devices that user is on.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// </remarks>
    /// <param name="targetEntityId">
    /// The Entity ID of the user to kick.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to kick the user began, or an error code otherwise. If
    /// this method fails, no related state changes will be generated. The human-readable form of the error code can be
    /// retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError KickUser(
        PartyString targetEntityId,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the value of a shared property.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If there is no property associated with the key, the resulting value is nullptr. The returned value is only
    /// valid until the next call to <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="key">
    /// The key of the shared property to retrieve.
    /// </param>
    /// <param name="value">
    /// An output struct that receives a pointer to the property's value and the length in bytes of the value data. If
    /// the property was not found, the call succeeds, but the value is nullptr and the byte length is 0.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedProperty(
        PartyString key,
        _Out_ PartyDataBuffer * value
        ) const party_no_throw;

    /// <summary>
    /// Gets an array of all property keys.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// The returned array is only valid until the next call to
    /// <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The output number of shared properties.
    /// </param>
    /// <param name="keys">
    /// A library-allocated output array containing the keys of all shared properties.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedPropertyKeys(
        _Out_ uint32_t * propertyCount,
        _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
        ) const party_no_throw;

    /// <summary>
    /// Sets or removes multiple shared properties associated with this object.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If a nullptr value is passed in the array, the corresponding key will be removed. Existing properties not
    /// specified in the call will remain unchanged.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The number of properties to set or remove.
    /// </param>
    /// <param name="keys">
    /// An array of size <paramref name="propertyCount" /> containing the keys to set or remove.
    /// </param>
    /// <param name="values">
    /// An array of size <paramref name="propertyCount" /> containing the values for each key in
    /// <paramref name="keys" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError SetSharedProperties(
        uint32_t propertyCount,
        _In_reads_(propertyCount) const PartyString * keys,
        _In_reads_(propertyCount) const PartyDataBuffer * values
        ) party_no_throw;

    /// <summary>
    /// Connects the local chat control to the network.
    /// </summary>
    /// <remarks>
    /// On successful return, this method invalidates the memory for any array previously returned by
    /// <see cref="GetChatControls()" /> or <see cref="PartyChatControl::GetNetworks()" /> for
    /// <paramref name="chatControl" />, as it synchronously updates those arrays.
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> also invalidates the memory for those arrays.
    /// </remarks>
    /// <param name="chatControl">
    /// The chat control to connect to this network.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to connect the chat control began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError ConnectChatControl(
        _In_ PartyLocalChatControl * chatControl,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Disconnects the local chat control from the network.
    /// </summary>
    /// <param name="chatControl">
    /// The chat control to disconnect from this network.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to disconnect the chat control began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError DisconnectChatControl(
        _In_ PartyLocalChatControl * chatControl,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the chat controls connected to this network.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyChatControlLeftNetworkStateChange" /> has been generated for a chat control by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the chat control will no longer be present in the
    /// array returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges() or <see cref="ConnectChatControl()" /> returns success.
    /// </para>
    /// </remarks>
    /// <param name="chatControlCount">
    /// The output number of chat controls in this network.
    /// </param>
    /// <param name="chatControls">
    /// A library-allocated output array containing the chat controls in this network.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetwork::ConnectChatControl" />
    /// <seealso cref="PartyNetwork::DisconnectChatControl" />
    /// <seealso cref="PartyChatControlLeftNetworkStateChange" />
    PartyError GetChatControls(
        _Out_ uint32_t * chatControlCount,
        _Outptr_result_buffer_(*chatControlCount) PartyChatControlArray * chatControls
        ) const party_no_throw;

    /// <summary>
    /// Gets one or more statistic counter values for the network.
    /// </summary>
    /// <remarks>
    /// This method is used to retrieve performance counters, queue lengths, historical usage metrics, or other
    /// statistical information recorded for this network.
    /// <para>
    /// A given PartyNetworkStatistic type may appear in any order in the <paramref name="statisticTypes" /> array, but
    /// must not be specified more than once. Each corresponding statistic value will be written to the
    /// <paramref name="statisticValues" /> array in the same order.
    /// </para>
    /// <para>
    /// The returned statistic values are always the most current ones available. There is no guarantee they will report
    /// the same value from one GetNetworkStatistics() call to the next, even if there were no intervening calls to
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="statisticCount">
    /// The number of statistics in the input <paramref name="statisticTypes" /> array and to be written in the
    /// <paramref name="statisticValues" /> output array. This must be at least 1.
    /// </param>
    /// <param name="statisticTypes">
    /// The <paramref name="statisticCount" /> entry input array of unique PartyNetworkStatistic types to retrieve.
    /// </param>
    /// <param name="statisticValues">
    /// The <paramref name="statisticCount" /> entry output array where the statistic values should be written. Each
    /// statistic value will be written at the same entry index corresponding to where the requested
    /// PartyNetworkStatistic appears in the <paramref name="statisticTypes" /> input array.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the network statistics succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetworkStatistic" />
    /// <seealso cref="PartyLocalEndpoint::GetEndpointStatistics" />
    PartyError GetNetworkStatistics(
        uint32_t statisticCount,
        _In_reads_(statisticCount) const PartyNetworkStatistic * statisticTypes,
        _Out_writes_all_(statisticCount) uint64_t * statisticValues
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this network object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetwork::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this network object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the network without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the network object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetwork::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

    /// <summary>
    /// Retrieves the type of connection used by the local device for transmitting messages or chat data to the
    /// specified target device in this network.
    /// </summary>
    /// <remarks>
    /// Connection types are determined when devices first authenticate an initial user into the network. If this
    /// network permitted direct peer-to-peer connectivity via the
    /// <see cref="PartyNetworkConfiguration::directPeerConnectivityOptions" /> field, neither device had excluded such
    /// connectivity via the <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> option, and a
    /// direct peer connection was successfully established at that time, then this function will report a value of
    /// <see cref="PartyDeviceConnectionType::DirectPeerConnection" />. Otherwise it will report
    /// <see cref="PartyDeviceConnectionType::RelayServer" />. The value won't change for as long as the target
    /// PartyDevice object remains in this network, even if <see cref="PartyNetwork::RemoveLocalUser()" /> is called for
    /// that initially authenticating user.
    /// <para>
    /// If the target device is the local device, <see cref="PartyDeviceConnectionType::RelayServer" /> will always be
    /// reported regardless of <see cref="PartyNetworkConfiguration" /> settings or
    /// <see cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" /> option.
    /// </para>
    /// <para>
    /// If the target device object isn't participating in this network, an error is returned.
    /// </para>
    /// <para>
    /// If the local device is participating in more networks with the target device object, you shouldn't assume that
    /// calling this same function on those other network objects will report the same value.
    /// </para>
    /// </remarks>
    /// <param name="targetDevice">
    /// The device whose connection type should be retrieved.
    /// </param>
    /// <param name="deviceConnectionType">
    /// The output device connection type.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the connection type succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDeviceConnectionType" />
    /// <seealso cref="PartyNetworkConfiguration" />
    /// <seealso cref="PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask" />
    PartyError GetDeviceConnectionType(
        const PartyDevice * targetDevice,
        _Out_ PartyDeviceConnectionType * deviceConnectionType
        ) const party_no_throw;

private:
    PartyNetwork() = delete;
    PartyNetwork(const PartyNetwork&) = delete;
    PartyNetwork(PartyNetwork&&) = delete;
    PartyNetwork& operator=(const PartyNetwork&) = delete;
    PartyNetwork& operator=(PartyNetwork&&) = delete;
    ~PartyNetwork() = delete;
};

/// <summary>
/// The management class for obtaining audio from an audio source stream.
/// </summary>
class PartyAudioManipulationSourceStream
{
public:
    /// <summary>
    /// Retrieves the source stream configuration.
    /// </summary>
    /// <remarks>
    /// The stream configuration matches the configuration provided to the call to
    /// <see cref="PartyChatControl::ConfigureAudioManipulationVoiceStream()" /> used to create this stream.
    /// </remarks>
    /// <param name="configuration">
    /// The stream configuration.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatControl::ConfigureAudioManipulationVoiceStream" />
    PartyError GetConfiguration(
        _Out_ PartyAudioManipulationSourceStreamConfiguration * configuration
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the format of the buffers that will be provided by
    /// <see cref="PartyAudioManipulationSourceStream::GetNextBuffer()" />.
    /// </summary>
    /// <remarks>
    /// If a format is specified in the configuration provided by
    /// <see cref="PartyAudioManipulationSourceStream::GetConfiguration()" />, this format will match. Otherwise, this
    /// format will be the most efficient format automatically selected by the library.
    /// </remarks>
    /// <param name="format">
    /// The format of the buffers that will be provided by
    /// <see cref="PartyAudioManipulationSourceStream::GetNextBuffer()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatControl::ConfigureAudioManipulationVoiceStream" />
    /// <seealso cref="PartyAudioManipulationSourceStream::GetNextBuffer" />
    /// <seealso cref="PartyAudioManipulationSourceStream::GetConfiguration" />
    PartyError GetFormat(
        _Out_ PartyAudioFormat * format
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the total number of buffers available to retrieve from this stream via
    /// <see cref="PartyAudioManipulationSourceStream::GetNextBuffer()" />.
    /// </summary>
    /// <remarks>
    /// This can be useful if the caller prefers to send audio through their pipeline in batches of buffers. Because
    /// this buffer count is limited by the max audio queue size specified via
    /// <see cref="PartyChatControl::ConfigureAudioManipulationVoiceStream()" />, callers should give their audio
    /// processing pipeline ample time to process the buffers and return them to
    /// <see cref="PartyAudioManipulationSourceStream::ReturnBuffer()" /> to prevent dropped audio.
    /// </remarks>
    /// <param name="count">
    /// The output count of available buffers.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetAvailableBufferCount(
        _Out_ uint32_t * count
        ) const party_no_throw;

    /// <summary>
    /// Gets the next buffer available in the stream.
    /// </summary>
    /// <remarks>
    /// When voice activity is detected, a new buffer is available every 40 ms. Otherwise, no buffers are available.
    /// Buffers retrieved by this method must be returned to the library via
    /// <see cref="PartyAudioManipulationSourceStream::ReturnBuffer()" /> when they're done being used.
    /// <para>
    /// The total number of buffers instantaneously available can be retrieved via
    /// <see cref="PartyAudioManipulationSourceStream::GetAvailableBufferCount()" />. Multiple buffers can be retrieved
    /// in succession before any are returned.
    /// </para>
    /// <para>
    /// Each buffer is in the format specified by <see cref="PartyAudioManipulationSourceStream::GetFormat()" />.
    /// </para>
    /// <para>
    /// A mutable data buffer is provided so that the app can optionally modify the audio in place.
    /// </para>
    /// </remarks>
    /// <param name="buffer">
    /// The output buffer. If no buffer is available, the PartyMutableDataBuffer's <em>bufferByteCount</em> field is 0,
    /// and its <em>buffer</em> field is nullptr.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAudioManipulationSourceStream::GetFormat" />
    /// <seealso cref="PartyAudioManipulationSourceStream::ReturnBuffer" />
    PartyError GetNextBuffer(
        _Out_ PartyMutableDataBuffer * buffer
        ) party_no_throw;

    /// <summary>
    /// Tells the library it can reclaim memory associated with this buffer.
    /// </summary>
    /// <param name="buffer">
    /// The buffer to return, which is the <em>buffer</em> field of a PartyDataBuffer previously retrieved from this
    /// source stream <see cref="PartyAudioManipulationSourceStream::GetNextBuffer()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAudioManipulationSourceStream::GetNextBuffer" />
    PartyError ReturnBuffer(
        _Post_invalid_ void * buffer
        ) party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this source stream
    /// object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAudioManipulationSourceStream::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this source stream object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the network without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the network object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAudioManipulationSourceStream::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyAudioManipulationSourceStream() = delete;
    PartyAudioManipulationSourceStream(const PartyAudioManipulationSourceStream&) = delete;
    PartyAudioManipulationSourceStream(PartyAudioManipulationSourceStream&&) = delete;
    PartyAudioManipulationSourceStream& operator=(const PartyAudioManipulationSourceStream&) = delete;
    PartyAudioManipulationSourceStream& operator=(PartyAudioManipulationSourceStream&&) = delete;
    ~PartyAudioManipulationSourceStream() = delete;
};

/// <summary>
/// The management class for submitting audio to an audio sink stream.
/// </summary>
class PartyAudioManipulationSinkStream
{
public:
    /// <summary>
    /// Retrieves the sink stream configuration.
    /// </summary>
    /// <remarks>
    /// The stream configuration matches the configuration provided to the configuration method used to create this
    /// stream.
    /// </remarks>
    /// <param name="configuration">
    /// The stream configuration.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationCaptureStream" />
    /// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationRenderStream" />
    PartyError GetConfiguration(
        _Out_ PartyAudioManipulationSinkStreamConfiguration * configuration
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the format of the buffers that will be submitted to
    /// <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" />.
    /// </summary>
    /// <remarks>
    /// If a format is specified in the configuration provided by
    /// <see cref="PartyAudioManipulationSinkStream::GetConfiguration()" />, this format will match. Otherwise, this
    /// format will be the most efficient format automatically selected by the library.
    /// </remarks>
    /// <param name="format">
    /// The format of the buffers that will be submitted to
    /// <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationCaptureStream" />
    /// <seealso cref="PartyLocalChatControl::ConfigureAudioManipulationRenderStream" />
    /// <seealso cref="PartyAudioManipulationSinkStream::SubmitBuffer" />
    /// <seealso cref="PartyAudioManipulationSinkStream::GetConfiguration" />
    PartyError GetFormat(
        _Out_ PartyAudioFormat * format
        ) const party_no_throw;

    /// <summary>
    /// Submits audio to be processed by this sink. Depending on the type of sink, this audio may be transmitted to
    /// other chat controls or rendered to the audio output.
    /// </summary>
    /// <remarks>
    /// Every 40 ms, the next 40 ms of audio from this stream is processed. To prevent audio hiccups, buffers for audio
    /// that should be heard continuously should be submitted to this stream at a constant rate.
    /// <para>
    /// The buffer is copied to an allocated buffer before PartyAudioManipulationSinkStream::SubmitBuffer() returns and
    /// can be immediately freed afterwards.
    /// <para>
    /// When applying chat permissions and determining which chat controls should receive audio, audio submitted to a
    /// capture sink via this method is treated as microphone audio.
    /// </para>
    /// </remarks>
    /// <param name="buffer">
    /// The audio buffer. Typically this audio buffer is generated by retrieving the next buffer available from each
    /// incoming source stream, and then processing and mixing each buffer based on game logic. This buffer must have
    /// the format specified by <see cref="PartyAudioManipulationSinkStream::GetFormat()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatPermissionOptions" />
    PartyError SubmitBuffer(
        const PartyDataBuffer * buffer
        ) party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this sink stream
    /// object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAudioManipulationSinkStream::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this sink stream object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the network without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the network object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAudioManipulationSinkStream::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyAudioManipulationSinkStream() = delete;
    PartyAudioManipulationSinkStream(const PartyAudioManipulationSinkStream&) = delete;
    PartyAudioManipulationSinkStream(PartyAudioManipulationSinkStream&&) = delete;
    PartyAudioManipulationSinkStream& operator=(const PartyAudioManipulationSinkStream&) = delete;
    PartyAudioManipulationSinkStream& operator=(PartyAudioManipulationSinkStream&&) = delete;
    ~PartyAudioManipulationSinkStream() = delete;
};

/// <summary>
/// The management class for chat operations.
/// </summary>
/// <seealso cref="PartyLocalChatControl" />
class PartyChatControl
{
public:
    /// <summary>
    /// Gets the <see cref="PartyLocalChatControl" /> version of this chat control.
    /// </summary>
    /// <param name="localChatControl">
    /// The local version of this chat control object, or nullptr if this is not a local chat control.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocal(
        _Outptr_result_maybenull_ PartyLocalChatControl ** localChatControl
        ) const party_no_throw;

    /// <summary>
    /// Returns the device this chat control is associated with.
    /// </summary>
    /// <param name="device">
    /// The output device this chat control is associated with.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetDevice(
        _Outptr_ PartyDevice ** device
        ) const party_no_throw;

    /// <summary>
    /// Gets the PlayFab Entity ID of the user associated with this chat control.
    /// </summary>
    /// <remarks>
    /// The returned string is the same value as the one found in the <em>id</em> field of a <c>PFEntityKey</c>
    /// structure representing the entity. The value returned from <see cref="GetEntityType()" /> would represent the
    /// other half of a <c>PFEntityKey</c>.
    /// <para>
    /// The memory for the Entity ID string remains valid for the life of the chat control, which is until its
    /// <see cref="PartyChatControlDestroyedStateChange" /> and/or
    /// <see cref="PartyDestroyChatControlCompletedStateChange" />, depending on the type of destruction that occurred,
    /// has been provided via <see cref="PartyManager::StartProcessingStateChanges()" /> and all state changes
    /// referencing the chat control have been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityId">
    /// The output Entity ID.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatControl::GetEntityType" />
    PartyError GetEntityId(
        _Outptr_ PartyString * entityId
        ) const party_no_throw;

    /// <summary>
    /// Gets the PlayFab entity type of the user associated with this chat control.
    /// </summary>
    /// <remarks>
    /// This returned string is the same value as the one found in the <em>type</em> field of a <c>PFEntityKey</c>
    /// structure representing the entity. The value returned from <see cref="GetEntityId()" /> would represent the
    /// other half of a <c>PFEntityKey</c>.
    /// <para>
    /// The memory for the entity type string remains valid for the life of the chat control, which is until its
    /// <see cref="PartyChatControlDestroyedStateChange" /> and/or
    /// <see cref="PartyDestroyChatControlCompletedStateChange" />, depending on the type of destruction that occurred,
    /// has been provided via <see cref="PartyManager::StartProcessingStateChanges()" /> and all state changes
    /// referencing the chat control have been returned to <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityType">
    /// The output entity type name string of the user associated with this chat control, or nullptr.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatControl::GetEntityId" />
    PartyError GetEntityType(
        _Outptr_ PartyString * entityType
        ) const party_no_throw;

    /// <summary>
    /// Gets the value of a shared property.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If there is no property associated with the key, the resulting value is nullptr. The returned value is only
    /// valid until the next call to <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="key">
    /// The key of the shared property to retrieve.
    /// </param>
    /// <param name="value">
    /// An output struct that receives a pointer to the property's value and the length in bytes of the value data. If
    /// the property was not found, the call succeeds, but the value is nullptr and the byte length is 0.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedProperty(
        PartyString key,
        _Out_ PartyDataBuffer * value
        ) const party_no_throw;

    /// <summary>
    /// Gets an array of all property keys.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// The returned array is only valid until the next call to
    /// <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The output number of shared properties.
    /// </param>
    /// <param name="keys">
    /// A library-allocated output array containing the keys of all shared properties.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetSharedPropertyKeys(
        _Out_ uint32_t * propertyCount,
        _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
        ) const party_no_throw;

    /// <summary>
    /// Gets the networks to which this chat control is connected.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyChatControlLeftNetworkStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the network will no longer be present in the array
    /// returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges() or <see cref="PartyNetwork::ConnectChatControl()" /> returns
    /// success.
    /// </para>
    /// </remarks>
    /// <param name="networkCount">
    /// The output number of networks to which this chat control is connected.
    /// </param>
    /// <param name="networks">
    /// A library-allocated output array containing the networks to which this chat control is connected.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyNetwork::ConnectChatControl" />
    /// <seealso cref="PartyNetwork::DisconnectChatControl" />
    /// <seealso cref="PartyChatControlLeftNetworkStateChange" />
    PartyError GetNetworks(
        _Out_ uint32_t * networkCount,
        _Outptr_result_buffer_(*networkCount) PartyNetworkArray * networks
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this chat control
    /// object.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatControl::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this chat control object.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the chat control without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the chat control object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatControl::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to configure the audio manipulation voice stream associated with this chat
    /// control.
    /// </summary>
    /// <remarks>
    /// If the configuration is non-null, an audio manipulation voice stream will be created for this chat control. Such
    /// a stream redirects output for the voice audio associated with this chat control; instead of the library
    /// automatically handling the voice audio and routing it, the app can use the source stream to retrieve the voice
    /// audio and route it via game logic. If the configuration is null, and a voice stream has previously been
    /// configured, the voice stream will be destroyed.
    /// <para>
    /// Upon completion of the asynchronous operation, when a non-null configuration was specified, a voice stream for
    /// this chat control can be queried via <see cref="PartyChatControl::GetAudioManipulationVoiceStream()" />.
    /// Completion is indicated by a <see cref="PartyConfigureAudioManipulationVoiceStreamCompletedStateChange" />.
    /// </para>
    /// <h>Platform support and supported formats</h>
    /// <para>
    /// This function is only supported on Windows, Xbox, and PlayStationÂ® 5. Calls on other platforms will fail.
    /// </para>
    /// <para>
    /// The following format options are supported for Windows and Xbox.
    /// </para>
    /// <para>
    /// `` | Format option | Supported value for local chat controls | Supported value for remote chat controls |
    /// ` | --- | --- | --- |
    /// ` | Samples per second | 24 kHz | 24 kHz |
    /// ` | Channel mask | 0 | 0 |
    /// ` | Channel count | 1 | 1 |
    /// ` | Bits per sample | 32 | 16 |
    /// ` | Sample type | <c>PartyAudioSampleType::Float</c> | <c>PartyAudioSampleType::Integer</c> |
    /// ` | Interleaved | false | false |
    /// </para>
    /// <para>
    /// For a list of supported format options for PlayStationÂ® 5, please refer to the
    /// README-RealTimeAudioManipulation.md document distributed with the Party library package.
    /// </para>
    /// </remarks>
    /// <param name="configuration">
    /// The stream configuration.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError ConfigureAudioManipulationVoiceStream(
        _In_opt_ PartyAudioManipulationSourceStreamConfiguration * configuration,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Retrieves the audio manipulation voice stream associated with this chat control.
    /// </summary>
    /// <remarks>
    /// In a local chat control, the stream represents the voice audio detected by the local audio input. Audio provided
    /// by this stream has already been preprocessed with Voice Activity Detection (VAD) and Automatic Gain Control
    /// (AGC). Audio is only provided when voice activity is detected. Typically, the app retrieves audio from this
    /// stream via PartyAudioManipulationSourceStream::GetNextBuffer(), process the audio using app logic, and then
    /// submits the audio back to the library. The audio is submitted back to the library by retrieving the voice sink
    /// stream via PartyLocalChatControl::GetAudioManipulationCaptureStream() and then submitting the buffer via
    /// <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" />.
    /// <para>
    /// Audio generated via <see cref="PartyLocalChatControl::SynthesizeTextToSpeech()" /> of type
    /// <see cref="PartySynthesizeTextToSpeechType::VoiceChat" /> is provided via this source stream, because such audio
    /// acts as the associated user's voice.
    /// </para>
    /// <para>
    /// Audio retrieved via this stream hasn't been transcribed via speech-to-text for voice chat transcription. Audio
    /// that is submitted to a sink stream via PartyAudioManipulationSinkStream::SubmitBuffer() is transcribed, if
    /// transcription options configured via <see cref="PartyLocalChatControl::SetTranscriptionOptions" /> indicates
    /// that audio associated the sink's chat control should be.
    /// </para>
    /// <para>
    /// In a remote chat control, the stream represents the chat control's incoming voice audio. Typically, the app
    /// retrieves audio from the voice streams associated with all remote chat controls via
    /// PartyAudioManipulationSourceStream::GetNextBuffer(), process and mix each buffer into a single audio stream, and
    /// then submits the mixed stream to be rendered by each appropriate sink stream. Each render stream can be
    /// retrieved via PartyLocalChatControl::GetAudioManipulationRenderStream().
    /// </para>
    /// </remarks>
    /// <param name="sourceStream">
    /// The source stream.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetAudioManipulationVoiceStream(
        _Outptr_ PartyAudioManipulationSourceStream ** sourceStream
        ) const party_no_throw;

private:
    PartyChatControl() = delete;
    PartyChatControl(const PartyChatControl&) = delete;
    PartyChatControl(PartyChatControl&&) = delete;
    PartyChatControl& operator=(const PartyChatControl&) = delete;
    PartyChatControl& operator=(PartyChatControl&&) = delete;
    ~PartyChatControl() = delete;
};

/// <summary>
/// The management class for chat operations related to the local device.
/// </summary>
/// <seealso cref="PartyChatControl" />
/// <seealso cref="PartyChatControl::GetLocal" />
class PartyLocalChatControl : public PartyChatControl
{
public:
    /// <summary>
    /// Gets the local user associated with this local chat control.
    /// </summary>
    /// <param name="localUser">
    /// The output local user.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocalUser(
        _Outptr_ PartyLocalUser ** localUser
        ) const party_no_throw;

    /// <summary>
    /// Sets the chat permissions between the local chat control and a target chat control.
    /// </summary>
    /// <remarks>
    /// Chat permissions allow you to control incoming and outgoing voice communication, as well as incoming text
    /// communication, between any pair of chat controls. The chat permissions should be configured to reflect your
    /// target scenario. For example, if the target chat control is a member of the same team, you'd likely allow audio
    /// in both directions. If the target chat control is a member of an opposing team, you likely wouldn't allow audio
    /// in either direction. Regardless of team, you might allow receiving text.
    /// <para>
    /// The target chat control must be remote. The default permission is
    /// <see cref="PartyChatPermissionOptions::None" />.
    /// </para>
    /// <para>
    /// This setting is local only. If a local chat control is configured to send audio to a remote chat control, no
    /// audio will be sent to the remote chat control unless the remote chat control has also been configured to receive
    /// audio from the local chat control in their instance of the library. Similarly, if a local chat control is
    /// configured to receive audio from a remote chat control, no audio will be received from the remote chat control
    /// unless the remote chat control has also been configured to send audio to the local chat control in their
    /// instance of the library. Therefore, both chat controls must agree that the communication is allowed; one chat
    /// control can't force another to accept or transmit undesired chat.
    /// </para>
    /// <para>
    /// There is no permission associated with sending chat text because each call to <see cref="SendText()" /> requires
    /// an explicit list of target chat controls. Including or omitting a target is equivalent to granting or denying
    /// send permission for that text message. The target still must have set
    /// <see cref="PartyChatPermissionOptions::ReceiveText" /> in their instance of the library for it to actually be
    /// delivered.
    /// </para>
    /// <para>
    /// The Party library does not enforce platform restrictions tied to the user's identity, such as a platform setting
    /// that restricts chat to "friends only". Platform restrictions should be considered when configuring the chat
    /// permissions between two chat controls.
    /// </para>
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="chatPermissionOptions">
    /// The chat permission options.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetPermissions(
        const PartyChatControl * targetChatControl,
        PartyChatPermissionOptions chatPermissionOptions
        ) party_no_throw;

    /// <summary>
    /// Gets the voice communication relationship between the local chat control and a target chat control.
    /// </summary>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="chatPermissionOptions">
    /// The output chat permission options.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetPermissions(
        const PartyChatControl * targetChatControl,
        _Out_ PartyChatPermissionOptions * chatPermissionOptions
        ) const party_no_throw;

    /// <summary>
    /// Transmits a chat text string to each specified target chat control.
    /// </summary>
    /// <remarks>
    /// Sending chat text to local chat controls is not currently supported. If the array of target chat controls
    /// includes any local targets, this call will synchronously fail.
    /// <para>
    /// There is no guaranteed translation, localization, or offensive language filtering of the text content; the chat
    /// text string will be presented with the text as is.
    /// </para>
    /// <para>
    /// The text string will only be delivered to target chat controls that have configured their own instances to
    /// receive text messages from the local chat control via <see cref="PartyLocalChatControl::SetPermissions()" />.
    /// </para>
    /// <para>
    /// If a target chat control is not connected to at least one network in common with the source chat control, the
    /// text string will not be delivered. This is possible if there are two local chat controls. A target chat control
    /// connected to one of the local chat controls may not be connected to all other local chat controls.
    /// </para>
    /// <para>
    /// The PartyDataBuffer structures in the <paramref name="dataBuffers" /> array are for auxiliary data associated
    /// with the chat text, such as metadata indicating the color that should be used to render the text or a binary
    /// blob containing image data for an icon to render along with the chat text. For sending game data that is
    /// unrelated to text chat, see <see cref="PartyLocalEndpoint::SendMessage()" />, a messaging option that exposes
    /// finer control over message transmission behavior.
    /// </para>
    /// <para>
    /// Callers provide 0 or more PartyDataBuffer structures in the <paramref name="dataBuffers" /> array. The memory
    /// that the structures reference does not have to be contiguous, making it easy to have a fixed header buffer
    /// followed by a variable payload, for example. The buffers will be assembled in order, transmitted, and delivered
    /// to the targeted chat controls as a single contiguous data block, separate from the chat text, in a
    /// <see cref="PartyChatTextReceivedStateChange" />. The Party library does not expend bandwidth transmitting
    /// metadata to describe the original PartyDataBuffer segmentation.
    /// </para>
    /// </remarks>
    /// <param name="targetChatControlCount">
    /// The number of target chat controls in the <paramref name="targetChatControls" /> array.
    /// </param>
    /// <param name="targetChatControls">
    /// The <paramref name="targetChatControlCount" /> entry array of target PartyChatControls.
    /// </param>
    /// <param name="chatText">
    /// The text to send.
    /// </param>
    /// <param name="dataBufferCount">
    /// The number of buffer structures provided in the <paramref name="dataBuffers" /> array.
    /// </param>
    /// <param name="dataBuffers">
    /// The <paramref name="dataBufferCount" /> entry array of PartyDataBuffer structures describing an auxiliary
    /// message payload to send. This is ignored when <paramref name="dataBufferCount" /> is zero.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyChatTextReceivedStateChange" />
    PartyError SendText(
        uint32_t targetChatControlCount,
        _In_reads_(targetChatControlCount) PartyChatControlArray targetChatControls,
        PartyString chatText,
        uint32_t dataBufferCount,
        _In_reads_(dataBufferCount) const PartyDataBuffer * dataBuffers
        ) party_no_throw;

    /// <summary>
    /// Configures the preferred microphone or recording device that the chat control will use for audio input.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous operation to configure the preferred recording device associated with this
    /// local chat control. If the method succeeds, a <see cref="PartyLocalChatAudioInputChangedStateChange" /> will be
    /// provided by <see cref="PartyManager::StartProcessingStateChanges()" /> with details about the input device
    /// status and a <see cref="PartySetChatAudioInputCompletedStateChange" /> will be provided upon completion of the
    /// operation, indicating success or failure. After completion, an additional
    /// <see cref="PartyLocalChatAudioInputChangedStateChange" /> will be provided each time the audio device status
    /// changes, such as due to device removal.
    /// <para>
    /// If the specified device isn't present, the chat control will subscribe to audio device changes and use the
    /// device when it does appear.
    /// </para>
    /// <para>
    /// When using the <c>PlatformUserDefault</c> option on the Xbox platform, the Xbox User Identifier (XUID) must be
    /// passed as the <c>audioDeviceSelectionContext</c> value.
    /// </para>
    /// </remarks>
    /// <param name="audioDeviceSelectionType">
    /// If <see cref="PartyAudioDeviceSelectionType::None" /> is specified, the audio input will be cleared. If
    /// <see cref="PartyAudioDeviceSelectionType::SystemDefault" /> is specified, the Party library will attempt to use
    /// the system's default communication device. If <see cref="PartyAudioDeviceSelectionType::PlatformUserDefault" />
    /// is specified, the Party library will attempt to use the default communication device associated with
    /// <paramref name="audioDeviceSelectionContext" />. If <see cref="PartyAudioDeviceSelectionType::Manual" /> is
    /// specified, the Party library will attempt to use the communication device whose device identifier matches. Note
    /// that <c>PartyAudioDeviceSelectionType::Manual</c> is not supported on Android, iOS, or macOS.
    /// </param>
    /// <param name="audioDeviceSelectionContext">
    /// When using <see cref="PartyAudioDeviceSelectionType::None" /> or
    /// <see cref="PartyAudioDeviceSelectionType::SystemDefault" />, <paramref name="audioDeviceSelectionContext" />
    /// will be ignored. When using <see cref="PartyAudioDeviceSelectionType::PlatformUserDefault" />,
    /// <paramref name="audioDeviceSelectionContext" /> must be the non-null, non-empty platform-specific user context
    /// that the chat control should use when selecting the audio device. When using
    /// <see cref="PartyAudioDeviceSelectionType::Manual" />, <paramref name="audioDeviceSelectionContext" /> must be
    /// the non-null, non-empty identifier of the audio device that the chat control should use.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the audio input began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::GetAudioInput" />
    /// <seealso cref="PartyLocalChatControl::SetAudioOutput" />
    PartyError SetAudioInput(
        PartyAudioDeviceSelectionType audioDeviceSelectionType,
        _In_opt_ PartyString audioDeviceSelectionContext,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Provides the preferred microphone or recording device that the chat control has been configured to use for audio
    /// input.
    /// </summary>
    /// <remarks>
    /// An empty device identifier string indicates that no input has been selected.
    /// </remarks>
    /// <param name="audioDeviceSelectionType">
    /// An output value indicating the selection type that was used to select the provided <paramref name="deviceId" />.
    /// </param>
    /// <param name="audioDeviceSelectionContext">
    /// When using <see cref="PartyAudioDeviceSelectionType::None" /> or
    /// <see cref="PartyAudioDeviceSelectionType::SystemDefault" />, <paramref name="audioDeviceSelectionContext" />
    /// will be empty. When using <see cref="PartyAudioDeviceSelectionType::PlatformUserDefault" /> or
    /// <see cref="PartyAudioDeviceSelectionType::Manual" />, <paramref name="audioDeviceSelectionContext" /> will be
    /// the value provided in a previous call to <see cref="SetAudioInput()" />. The memory for the string remains valid
    /// until the next <see cref="PartyLocalChatAudioInputChangedStateChange" /> is provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or the chat control is destroyed.
    /// </param>
    /// <param name="deviceId">
    /// An output value indicating the selected audio input device's identifier. The memory for the string remains valid
    /// until the next <see cref="PartyLocalChatAudioInputChangedStateChange" /> is provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or the chat control is destroyed.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::SetAudioInput" />
    /// <seealso cref="PartyLocalChatControl::GetAudioOutput" />
    PartyError GetAudioInput(
        _Out_ PartyAudioDeviceSelectionType * audioDeviceSelectionType,
        _Outptr_ PartyString * audioDeviceSelectionContext,
        _Outptr_ PartyString * deviceId
        ) const party_no_throw;

    /// <summary>
    /// Configures the preferred speakers or headset device that the chat control will use for audio output.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous operation to configure the preferred speakers or headset device associated
    /// with this local chat control. If the method succeeds, a
    /// <see cref="PartyLocalChatAudioOutputChangedStateChange" /> will be provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> with details about the output device status and a
    /// <see cref="PartySetChatAudioOutputCompletedStateChange" /> will be provided upon completion of the operation,
    /// indicating success or failure. After completion, an additional
    /// <see cref="PartyLocalChatAudioOutputChangedStateChange" /> will be provided each time the audio device status
    /// changes, such as due to device removal.
    /// <para>
    /// If the specified device isn't present, the chat control will subscribe to audio device changes and use the
    /// device when it does appear.
    /// </para>
    /// <para>
    /// When using the <c>PlatformUserDefault</c> option on the Xbox platform, the Xbox User Identifier (XUID) must be
    /// passed as the <c>audioDeviceSelectionContext</c> value.
    /// </para>
    /// </remarks>
    /// <param name="audioDeviceSelectionType">
    /// If <see cref="PartyAudioDeviceSelectionType::None" /> is specified, the audio output will be cleared. If
    /// <see cref="PartyAudioDeviceSelectionType::SystemDefault" /> is specified, the Party library will attempt to use
    /// the system's default communication device. If <see cref="PartyAudioDeviceSelectionType::PlatformUserDefault" />
    /// is specified, the Party library will attempt to use the default communication device associated with
    /// <paramref name="audioDeviceSelectionContext" />. If <see cref="PartyAudioDeviceSelectionType::Manual" /> is
    /// specified, the Party library will attempt to use the communication device whose device identifier matches
    /// <paramref name="audioDeviceSelectionContext" />.
    /// </param>
    /// <param name="audioDeviceSelectionContext">
    /// When using <see cref="PartyAudioDeviceSelectionType::None" /> or
    /// <see cref="PartyAudioDeviceSelectionType::SystemDefault" />, <paramref name="audioDeviceSelectionContext" />
    /// will be ignored. When using <see cref="PartyAudioDeviceSelectionType::PlatformUserDefault" />,
    /// <paramref name="audioDeviceSelectionContext" /> must be the non-null, non-empty platform-specific user context
    /// that the chat control should use when selecting the audio device. When using
    /// <see cref="PartyAudioDeviceSelectionType::Manual" />, <paramref name="audioDeviceSelectionContext" /> must be
    /// the non-null, non-empty identifier of the audio device that the chat control should use.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the audio output began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::GetAudioOutput" />
    /// <seealso cref="PartyLocalChatControl::SetAudioInput" />
    PartyError SetAudioOutput(
        PartyAudioDeviceSelectionType audioDeviceSelectionType,
        _In_opt_ PartyString audioDeviceSelectionContext,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Provides the preferred speakers or headset device that the chat control has been configured to use for audio
    /// output.
    /// </summary>
    /// <remarks>
    /// An empty device identifier string indicates that no output has been selected.
    /// </remarks>
    /// <param name="audioDeviceSelectionType">
    /// An output value indicating the selection type that was used to select the provided <paramref name="deviceId" />.
    /// </param>
    /// <param name="audioDeviceSelectionContext">
    /// When using <see cref="PartyAudioDeviceSelectionType::None" /> or
    /// <see cref="PartyAudioDeviceSelectionType::SystemDefault" />, <paramref name="audioDeviceSelectionContext" />
    /// will be empty. When using <see cref="PartyAudioDeviceSelectionType::PlatformUserDefault" /> or
    /// <see cref="PartyAudioDeviceSelectionType::Manual" />, <paramref name="audioDeviceSelectionContext" /> will be
    /// the value provided in a previous call to <see cref="SetAudioOutput()" />. The memory for the string remains
    /// valid until the next <see cref="PartyLocalChatAudioOutputChangedStateChange" /> is provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or the chat control is destroyed.
    /// </param>
    /// <param name="deviceId">
    /// An output value indicating the selected audio output device's identifier. The memory for the string remains
    /// valid until the next <see cref="PartyLocalChatAudioOutputChangedStateChange" /> is provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> or the chat control is destroyed.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::SetAudioOutput" />
    /// <seealso cref="PartyLocalChatControl::GetAudioInput" />
    PartyError GetAudioOutput(
        _Out_ PartyAudioDeviceSelectionType * audioDeviceSelectionType,
        _Outptr_ PartyString * audioDeviceSelectionContext,
        _Outptr_ PartyString * deviceId
        ) const party_no_throw;

    /// <summary>
    /// Populates the local chat control's list of supported text-to-speech profiles.
    /// </summary>
    /// <remarks>
    /// This is an asynchronous operation; a
    /// <see cref="PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange" /> is provided via
    /// <see cref="PartyManager::StartProcessingStateChanges()" /> on completion.
    /// <para>
    /// The asynchronous operation must complete successfully before GetAvailableTextToSpeechProfiles() can be called.
    /// </para>
    /// <para>
    /// Text-to-speech synthesis functionality internally uses available region and latency measurement estimates to
    /// optimize service usage. If the <see cref="PartyOption::RegionUpdateConfiguration" /> option was used to
    /// configure an update mode of <see cref="PartyRegionUpdateMode::Deferred" />, then retrieving the set of available
    /// regions and measuring connection quality to them may not have started yet, or the last update may have exceeded
    /// the configured refresh interval age. If the local device isn't currently connecting or connected to any
    /// networks, PopulateAvailableTextToSpeechProfiles() ensures any deferred region update has started and the
    /// associated <see cref="PartyRegionsChangedStateChange" /> is provided prior to this call's
    /// <see cref="PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange" /> completion.
    /// </para>
    /// </remarks>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation began, or an error code otherwise. If this method
    /// fails, no related state changes are generated. The human-readable form of the error code can be retrieved via
    /// <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::GetAvailableTextToSpeechProfiles" />
    /// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
    /// <seealso cref="PartyLocalChatControl::GetTextToSpeechProfile" />
    PartyError PopulateAvailableTextToSpeechProfiles(
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the text-to-speech profiles for this chat control.
    /// </summary>
    /// <remarks>
    /// This method synchronously returns the result cached by the last successful
    /// <see cref="PopulateAvailableTextToSpeechProfiles()" /> operation.
    /// <para>
    /// The returned array is only valid until the next call to
    /// <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="profileCount">
    /// The output number of available profiles.
    /// </param>
    /// <param name="profiles">
    /// A library-allocated output array containing the available profiles.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles" />
    PartyError GetAvailableTextToSpeechProfiles(
        _Out_ uint32_t * profileCount,
        _Outptr_result_buffer_(*profileCount) PartyTextToSpeechProfileArray * profiles
        ) const party_no_throw;

    /// <summary>
    /// Configures the profile to use for a specified type of text-to-speech operation.
    /// </summary>
    /// <remarks>
    /// The profile provided is used for all subsequent calls to <see cref="SynthesizeTextToSpeech()" /> that specify
    /// the same value for <paramref name="type" />. No profile is configured until SetTextToSpeechProfile() is called
    /// at least once. Thus, this method must be called at least once before any calls to SynthesizeTextToSpeech()
    /// succeed.
    /// <para>
    /// This method accepts a profile identifier to indicate the profile selection so that titles may either pass in the
    /// result of <see cref="PartyTextToSpeechProfile::GetIdentifier()" /> or provide a profile identifier cached from a
    /// previous Party library session.
    /// </para>
    /// <para>
    /// Multiple SetTextToSpeechProfile() operations can be initiated, and they are asynchronously queued. Each
    /// operation is processed and completed in order.
    /// </para>
    /// <para>
    /// This is an asynchronous operation; a <see cref="PartySetTextToSpeechProfileCompletedStateChange" /> is provided
    /// via <see cref="PartyManager::StartProcessingStateChanges()" /> on completion.
    /// </para>
    /// <para>
    /// Text-to-speech synthesis functionality internally uses available region and latency measurement estimates to
    /// optimize service usage. If the <see cref="PartyOption::RegionUpdateConfiguration" /> option was used to
    /// configure an update mode of <see cref="PartyRegionUpdateMode::Deferred" />, then retrieving the set of available
    /// regions and measuring connection quality to them may not have started yet, or the last update may have exceeded
    /// the configured refresh interval age. If the local device isn't currently connecting or connected to any
    /// networks, SetTextToSpeechProfile() ensures any deferred region update has started and the associated
    /// <see cref="PartyRegionsChangedStateChange" /> is provided prior to this call's
    /// <see cref="PartySetTextToSpeechProfileCompletedStateChange" /> completion.
    /// </para>
    /// </remarks>
    /// <param name="type">
    /// The type of text-to-speech operations for which the specified profile should be used.
    /// </param>
    /// <param name="profileIdentifier">
    /// The identifier of the profile that text-to-speech operations of the specified type should use.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the text-to-speech profile began, or an error
    /// code otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::GetTextToSpeechProfile" />
    PartyError SetTextToSpeechProfile(
        PartySynthesizeTextToSpeechType type,
        PartyString profileIdentifier,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the profile that was last configured from a successfully completed <see cref="SetTextToSpeechProfile()" />
    /// operation for a specified type of text-to-speech operation.
    /// </summary>
    /// <remarks>
    /// If no profile has been successfully configured for the specified type of text-to-speech operation, the retrieved
    /// profile will be null.
    /// <para>
    /// The retrieved profile will update when a <see cref="PartySetTextToSpeechProfileCompletedStateChange" />
    /// associated with a call to SetTextToSpeechProfile(), which specified the same value for <paramref name="type" />,
    /// is provided via <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="type">
    /// The type of text-to-speech operations for which the profile should be retrieved.
    /// </param>
    /// <param name="profile">
    /// The output profile. The profile may be nullptr if none has been set for the specified type. The pointer is only
    /// valid until the next call to <see cref="PartyManager::StartProcessingStateChanges()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
    PartyError GetTextToSpeechProfile(
        PartySynthesizeTextToSpeechType type,
        _Outptr_result_maybenull_ PartyTextToSpeechProfile ** profile
        ) const party_no_throw;

    /// <summary>
    /// Generates text-to-speech audio that is used to either displace the microphone audio associated with this chat
    /// control or render directly to the chat control's audio output.
    /// </summary>
    /// <remarks>
    /// SynthesizeTextToSpeech() cannot be called for a particular <paramref name="type" /> until a text-to-speech
    /// profile has been configured for the specified <paramref name="type" /> via
    /// <see cref="SetTextToSpeechProfile()" />.
    /// <para>
    /// SynthesizeTextToSpeech() can be called immediately after configuring a text-to-speech profile via a call to
    /// SetTextToSpeechProfile() with the same <paramref name="type" />. In such a case, the text-to-speech operation
    /// will be queued behind the completion of the SetTextToSpeechProfile() operation.
    /// </para>
    /// </remarks>
    /// <param name="type">
    /// The type of text-to-speech operation.
    /// </param>
    /// <param name="textToSynthesize">
    /// The text from which to synthesize audio.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to synthesize the text to speech began, or an error
    /// code otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
    PartyError SynthesizeTextToSpeech(
        PartySynthesizeTextToSpeechType type,
        PartyString textToSynthesize,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Configures the language associated with this chat control.
    /// </summary>
    /// <remarks>
    /// If no language is specified, the user's default, as determined by the platform, will be used. The language is
    /// used as the spoken language associated with this chat control for transcription and the target language for
    /// incoming translations. If the language code specified is en-US, for instance, the input audio to this chat
    /// control will be treated as the English (United States) language and transcribed as such. If translation is
    /// enabled via either <see cref="SetTranscriptionOptions()" /> or <see cref="SetTextChatOptions()" />, the incoming
    /// voice chat transcriptions and/or text chat will be translated to English (United States).
    /// <para>
    /// The language code should be in BCP 47 format; supported language codes are enumerated in
    /// <see xref="https://learn.microsoft.com/azure/ai-services/speech-service/language-support" name="Language support" />.
    /// Specifying an unsupported or invalid language code will not cause this method to fail, but will result in
    /// failure to generate transcriptions associated with this chat control. The language code can be queried via
    /// <see cref="GetLanguage()" />.
    /// </para>
    /// </remarks>
    /// <param name="languageCode">
    /// The new BCP 47 language code the chat control should use with future communication. If this value is nullptr,
    /// the user's default, as determined by the platform, will be used.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the language began, or an error code otherwise.
    /// If this method fails, no related state changes will be generated. The human-readable form of the error code can
    /// be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::GetLanguage" />
    /// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
    /// <nyi />
    PartyError SetLanguage(
        _In_opt_ PartyString languageCode,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the language associated with this chat control.
    /// </summary>
    /// <remarks>
    /// The language is used as the spoken language associated with this chat control for transcription and the target
    /// language for incoming translations. If the language code specified is en-US, for instance, the input audio to
    /// this chat control will be treated as the English (United States) language and transcribed as such. If
    /// translation is enabled via either <see cref="SetTranscriptionOptions()" /> or
    /// <see cref="SetTextChatOptions()" />, the incoming voice chat transcriptions and/or text chat will be translated
    /// to English (United States).
    /// <para>
    /// The language code should be in BCP 47 format; supported language codes are enumerated in
    /// <see xref="https://learn.microsoft.com/azure/ai-services/speech-service/language-support" name="Language support" />.
    /// Specifying an unsupported or invalid language code will not cause this method to fail, but will result in
    /// failure to generate transcriptions associated with this chat control. The language code can be queried via
    /// GetLanguage().
    /// </para>
    /// </remarks>
    /// <param name="languageCode">
    /// The output BCP 47 language code used by the chat control future communication. The memory for the string remains
    /// valid until the chat control is destroyed.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso_nyi cref="PartySetLanguageCompletedStateChange" />
    /// <seealso cref="PartyLocalDevice::CreateChatControl" />
    /// <seealso_nyi cref="PartyLocalChatControl::SetLanguage" />
    /// <seealso cref="PartyLocalChatControl::SetTranscriptionOptions" />
    PartyError GetLanguage(
        _Outptr_ PartyString * languageCode
        ) const party_no_throw;

    /// <summary>
    /// Configures the transcription options associated with voice chat audio for this chat control.
    /// </summary>
    /// <remarks>
    /// Transcription is the process of generating strings representing spoken phrases in voice chat. The options
    /// specified via <paramref name="options" /> specify the chat controls that should generate these transcription
    /// strings, which are subsequently provided to the local chat control via
    /// <see cref="PartyVoiceChatTranscriptionReceivedStateChange" />s.
    /// <para>
    /// Only chat controls configured to use a language that supports transcription, via
    /// <see cref="PartyLocalDevice::CreateChatControl()" />, will provide transcriptions.
    /// </para>
    /// <para>
    /// Speech-to-text transcription functionality internally uses available region and latency measurement estimates to
    /// optimize service usage. If the <see cref="PartyOption::RegionUpdateConfiguration" /> option was used to
    /// configure an update mode of <see cref="PartyRegionUpdateMode::Deferred" />, then retrieving the set of available
    /// regions and measuring connection quality to them may not have started yet, or the last update may have exceeded
    /// the configured refresh interval age. If the local device isn't currently connecting or connected to any
    /// networks, and the application specifies option flags that include
    /// <see cref="PartyVoiceChatTranscriptionOptions::TranscribeSelfRegardlessOfNetworkState" />, then
    /// SetTranscriptionOptions() ensures any deferred region update has started and the associated
    /// <see cref="PartyRegionsChangedStateChange" /> is provided prior to this call's
    /// <see cref="PartySetTranscriptionOptionsCompletedStateChange" /> completion.
    /// </para>
    /// </remarks>
    /// <param name="options">
    /// Options associated with voice chat transcription, such as which chat controls should generate transcriptions for
    /// the local chat control, and whether those transcriptions should be translated to the local chat control's
    /// language.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the transcription options began, or an error
    /// code otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetTranscriptionOptions(
        PartyVoiceChatTranscriptionOptions options,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Provides the transcription options associated with voice chat audio for this chat control.
    /// </summary>
    /// <remarks>
    /// Transcription is the process of generating strings representing spoken phrases in voice chat. The options
    /// specified via <paramref name="options" /> specify the chat controls that should generate these transcription
    /// strings, which are subsequently provided to the local chat control via
    /// <see cref="PartyVoiceChatTranscriptionReceivedStateChange" />s.
    /// <para>
    /// Only chat controls configured to use a language that supports transcription, via
    /// <see cref="PartyLocalDevice::CreateChatControl()" />, will provide transcriptions.
    /// </para>
    /// </remarks>
    /// <param name="options">
    /// Options associated with voice chat transcription, such as whether it should be enabled only for the local chat
    /// control, remote chat controls, or none.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetTranscriptionOptions(
        _Out_ PartyVoiceChatTranscriptionOptions * options
        ) const party_no_throw;

    /// <summary>
    /// Configures the text chat options associated with text chat for this chat control.
    /// </summary>
    /// <param name="options">
    /// Options associated with text chat, such as whether incoming text chat should be translated.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the chat text options began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetTextChatOptions(
        PartyTextChatOptions options,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Provides the text chat options associated with text chat for this chat control.
    /// </summary>
    /// <param name="options">
    /// Options associated with text chat.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetTextChatOptions(
        _Out_ PartyTextChatOptions * options
        ) const party_no_throw;

    /// <summary>
    /// Configures the volume setting for audio received from a target chat control that will be used for the local chat
    /// control.
    /// </summary>
    /// <remarks>
    /// The volume is a fractional percentage between 0.0 (quietest possible) and 1.0 (the standard chat volume).
    /// <para>
    /// This is a local setting that takes effect immediately.
    /// </para>
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="volume">
    /// The volume.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetAudioRenderVolume(
        const PartyChatControl * targetChatControl,
        float volume
        ) party_no_throw;

    /// <summary>
    /// Provides the volume setting for audio received from a target chat control that will be used for the local chat
    /// control.
    /// </summary>
    /// <remarks>
    /// The volume is a fractional percentage between 0.0 (quietest possible) and 1.0 (the standard chat volume).
    /// <para>
    /// This is a local setting that takes effect immediately.
    /// </para>
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="volume">
    /// The volume.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetAudioRenderVolume(
        const PartyChatControl* targetChatControl,
        _Out_ float * volume
        ) const party_no_throw;

    /// <summary>
    /// Configures whether the user's audio input is in the mute state.
    /// </summary>
    /// <remarks>
    /// Muting determines whether audio data will be captured from the chat control's audio input and sent to the
    /// appropriate chat controls according to the configured relationships. When the audio input is muted, no audio
    /// data will be captured, regardless of the relationships that have been configured. Muting does not stop outgoing
    /// text messages or audio that has been generated by a call to <see cref="SynthesizeTextToSpeech()" />.
    /// </remarks>
    /// <param name="muted">
    /// True to mute, false otherwise.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetAudioInputMuted(
        PartyBool muted
        ) party_no_throw;

    /// <summary>
    /// Provides whether the user's audio input is in the mute state.
    /// </summary>
    /// <remarks>
    /// Muting determines whether audio data will be captured from the chat control's audio input and sent to the
    /// appropriate chat controls according to the configured relationships. When the audio input is muted, no audio
    /// data will be captured, regardless of the relationships that have been configured. Muting does not stop outgoing
    /// text messages or audio that has been generated by a call to SynthesizeTextToSpeech().
    /// </remarks>
    /// <param name="muted">
    /// True if muted, false otherwise.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetAudioInputMuted(
        _Out_ PartyBool * muted
        ) const party_no_throw;

    /// <summary>
    /// Configures whether the incoming audio from the target chat control, in relation to the local chat control, is
    /// muted.
    /// </summary>
    /// <remarks>
    /// Audio muting determines whether incoming voice audio from the target chat control will be rendered to the local
    /// chat control.
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="muted">
    /// True to mute, false otherwise.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetIncomingAudioMuted(
        const PartyChatControl * targetChatControl,
        PartyBool muted
        ) party_no_throw;

    /// <summary>
    /// Provides whether the incoming audio from the target chat control, in relation to the local chat control, has
    /// previously been muted.
    /// </summary>
    /// <remarks>
    /// Audio muting determines whether incoming voice audio from the target chat control will be rendered to the local
    /// chat control.
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="muted">
    /// True if muted, false otherwise.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetIncomingAudioMuted(
        const PartyChatControl * targetChatControl,
        _Out_ PartyBool * muted
        ) const party_no_throw;

    /// <summary>
    /// Configures whether the incoming text messages from the target chat control, in relation to the local chat
    /// control, is muted.
    /// </summary>
    /// <remarks>
    /// Chat text muting determines whether incoming chat text messages from the target chat control will be provided to
    /// the local chat control.
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="muted">
    /// True to mute, false otherwise.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetIncomingTextMuted(
        const PartyChatControl * targetChatControl,
        PartyBool muted
        ) party_no_throw;

    /// <summary>
    /// Provides whether the incoming text messages from the target chat control, in relation to the local chat control,
    /// has previously been muted.
    /// </summary>
    /// <remarks>
    /// Chat text muting determines whether incoming chat text messages from the target chat control will be provided to
    /// the local chat control.
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="muted">
    /// True if muted, false otherwise.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetIncomingTextMuted(
        const PartyChatControl * targetChatControl,
        _Out_ PartyBool * muted
        ) const party_no_throw;

    /// <summary>
    /// Configures the voice audio options for this chat control.
    /// </summary>
    /// <param name="options">
    /// The voice audio options.
    /// </param>
    /// <remarks>
    /// This function is only supported on Android and iOS. Calls on other platforms will fail.
    /// <para>
    /// By default, a chat control is created with <see cref="PartyVoiceAudioOptions::None" /> audio options.
    /// </para>
    /// </remarks>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError SetVoiceAudioOptions(
        PartyVoiceAudioOptions options
        ) party_no_throw;

    /// <summary>
    /// Provides the voice audio options associated with this chat control.
    /// </summary>
    /// <param name="options">
    /// The voice audio options.
    /// </param>
    /// <remarks>
    /// This function is only supported on Android and iOS. Calls on other platforms will fail.
    /// </remarks>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetVoiceAudioOptions(
        _Out_ PartyVoiceAudioOptions* options
        ) const party_no_throw;

    /// <summary>
    /// Provides a chat indicator specifying the audio state of the local chat control.
    /// </summary>
    /// <remarks>
    /// This indicator is intended to assist UI iconic representation so that users can determine whether their voice
    /// audio is being captured and, if not, then why not.
    /// <para>
    /// This value changes frequently and is typically polled every graphics frame.
    /// </para>
    /// </remarks>
    /// <param name="chatIndicator">
    /// The chat indicator.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocalChatIndicator(
        _Out_ PartyLocalChatControlChatIndicator * chatIndicator
        ) const party_no_throw;

    /// <summary>
    /// Provides a chat indicator specifying the audio state of a target chat control in relation to the local chat
    /// control.
    /// </summary>
    /// <remarks>
    /// This indicator is intended to assist UI iconic representation so that users can determine why they're hearing/
    /// seeing what they are ("who's that talking?"), or why not.
    /// <para>
    /// This value changes frequently and is typically polled every graphics frame.
    /// </para>
    /// </remarks>
    /// <param name="targetChatControl">
    /// The target chat control.
    /// </param>
    /// <param name="chatIndicator">
    /// The chat indicator.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetChatIndicator(
        const PartyChatControl * targetChatControl,
        _Out_ PartyChatControlChatIndicator * chatIndicator
        ) const party_no_throw;

    /// <summary>
    /// Configures the bitrate used to encode audio generated by this chat control.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// </remarks>
    /// <param name="bitrate">
    /// The bitrate.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to set the encoder bitrate began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError SetAudioEncoderBitrate(
        uint32_t bitrate,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Provides the bitrate used to encode audio generated by this chat control.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// </remarks>
    /// <param name="bitrate">
    /// The bitrate.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError GetAudioEncoderBitrate(
        _Out_ uint32_t * bitrate
        ) const party_no_throw;

    /// <summary>
    /// Sets or removes multiple shared properties associated with this object.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// If a nullptr value is passed in the array, the corresponding key will be removed. Existing properties not
    /// specified in the call will remain unchanged.
    /// </para>
    /// </remarks>
    /// <param name="propertyCount">
    /// The number of properties to set or remove.
    /// </param>
    /// <param name="keys">
    /// An array of size <paramref name="propertyCount" /> containing the keys to set or remove.
    /// </param>
    /// <param name="values">
    /// An array of size <paramref name="propertyCount" /> containing the values for each key in
    /// <paramref name="keys" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError SetSharedProperties(
        uint32_t propertyCount,
        _In_reads_(propertyCount) const PartyString * keys,
        _In_reads_(propertyCount) const PartyDataBuffer * values
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to configure the audio manipulation capture stream.
    /// </summary>
    /// <remarks>
    /// If the configuration is non-null, a capture stream will be created for this chat control. Such a stream acts as
    /// the voice input for this chat control that is sent to all other chat controls to which this chat control is
    /// configured to communicate. If the configuration is null, and a capture stream has previously been configured,
    /// the capture will be destroyed.
    /// <para>
    /// Upon completion of the asynchronous operation, when a non-null configuration was specified, a capture stream for
    /// this chat control can be queried via <see cref="PartyLocalChatControl::GetAudioManipulationCaptureStream()" />.
    /// Completion is indicated by a <see cref="PartyConfigureAudioManipulationCaptureStreamCompletedStateChange" />.
    /// </para>
    /// <h>Platform support and supported formats</h>
    /// <para>
    /// This function is only supported on Windows, Xbox, and PlayStationÂ® 5. Calls on other platforms will fail.
    /// </para>
    /// <para>
    /// The following format options are supported for Windows and Xbox.
    /// </para>
    /// <para>
    /// `` | Format option | Supported value |
    /// ` | --- | --- |
    /// ` | Samples per second | 24 kHz |
    /// ` | Channel mask | 0 |
    /// ` | Channel count | 1 |
    /// ` | Bits per sample | 32 |
    /// ` | Sample type | <c>PartyAudioSampleType::Float</c> |
    /// ` | Interleaved | false |
    /// </para>
    /// <para>
    /// For a list of supported format options for PlayStationÂ® 5, please refer to the
    /// README-RealTimeAudioManipulation.md document distributed with the Party library package.
    /// </para>
    /// </remarks>
    /// <param name="configuration">
    /// The stream configuration.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError ConfigureAudioManipulationCaptureStream(
        _In_opt_ PartyAudioManipulationSinkStreamConfiguration * configuration,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Retrieves the audio manipulation capture stream associated with this chat control.
    /// </summary>
    /// <remarks>
    /// This stream represents acts as the associated chat control's audio input, i.e. the audio that will be treated as
    /// the local chat control's voice and sent to other chat controls. Typically, the app will retrieve audio from the
    /// voice manipulation stream stream via PartyAudioManipulationSourceStream::GetNextBuffer(), process the audio
    /// using app logic, and then submit the audio back to the library via this stream.
    /// <para>
    /// Audio that is submitted to this stream via <see cref="PartyAudioManipulationSinkStream::SubmitBuffer()" /> will
    /// be transcribed via speech-to-text, if transcription options configured via
    /// <see cref="PartyLocalChatControl::SetTranscriptionOptions" /> indicates that audio associated the sink's chat
    /// control should be.
    /// </para>
    /// </remarks>
    /// <param name="stream">
    /// The output stream.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetAudioManipulationCaptureStream(
        _Outptr_ PartyAudioManipulationSinkStream ** stream
        ) const party_no_throw;

    /// <summary>
    /// Queues an asynchronous operation to configure the audio manipulation render stream.
    /// </summary>
    /// <remarks>
    /// If the configuration is non-null, an audio manipulation render stream will be created for this chat control.
    /// Such a stream acts as the render pipeline for audio that will be rendered to this chat control. If the
    /// configuration is null, and a stream has previously been configured, the stream will be destroyed.
    /// <para>
    /// Upon completion of the asynchronous operation, when a non-null configuration was specified, a render stream for
    /// this chat control can be queried via <see cref="PartyLocalChatControl::GetAudioManipulationRenderStream()" />.
    /// Completion is indicated by a <see cref="PartyConfigureAudioManipulationRenderStreamCompletedStateChange" />.
    /// </para>
    /// <h>Platform support and supported formats</h>
    /// <para>
    /// This function is only supported on Windows, Xbox, and PlayStationÂ® 5. Calls on other platforms will fail.
    /// </para>
    /// <para>
    /// The following format options are supported for Windows and Xbox.
    /// </para>
    /// <para>
    /// `` | Format option | Supported value(s) |
    /// ` | --- | --- |
    /// ` | Samples per second | Any value between 8 kHz and 48 kHz, inclusive. |
    /// ` | Channel mask | Any value |
    /// ` | Channel count | Any value between 1 and 64, inclusive. |
    /// ` | Bits per sample | If sample type is <c>PartyAudioSampleType::Float</c>, 32. If sample type is
    ///         <c>PartyAudioSampleType::Integer</c>, 16 or 32. |
    /// ` | Sample type | <c>PartyAudioSampleType::Float</c> or <c>PartyAudioSampleType::Integer</c> |
    /// ` | Interleaved | true or false |
    /// </para>
    /// <para>
    /// For a list of supported format options for PlayStationÂ® 5, please refer to the
    /// README-RealTimeAudioManipulation.md document distributed with the Party library package.
    /// </para>
    /// </remarks>
    /// <param name="configuration">
    /// The stream configuration.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError ConfigureAudioManipulationRenderStream(
        _In_opt_ PartyAudioManipulationSinkStreamConfiguration * configuration,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Retrieves the manipulation render stream associated with this chat control.
    /// </summary>
    /// <remarks>
    /// This stream represents the audio that will be rendered to the chat control's audio output. Typically, the app
    /// will retrieve audio from the voice streams associated with all remote chat controls via
    /// PartyAudioManipulationSourceStream::GetNextBuffer(), process and mix each buffer into a single audio stream, and
    /// then submit the mixed stream to be rendered by each appropriate render stream. Each render stream can be
    /// retrieved via PartyLocalChatControl::GetAudioManipulationRenderStream().
    /// </remarks>
    /// <param name="stream">
    /// The output stream.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetAudioManipulationRenderStream(
        _Outptr_ PartyAudioManipulationSinkStream ** stream
        ) const party_no_throw;

private:
    PartyLocalChatControl() = delete;
    PartyLocalChatControl(const PartyLocalChatControl&) = delete;
    PartyLocalChatControl(PartyLocalChatControl&&) = delete;
    PartyLocalChatControl& operator=(const PartyLocalChatControl&) = delete;
    PartyLocalChatControl& operator=(PartyLocalChatControl&&) = delete;
    ~PartyLocalChatControl() = delete;
};

/// <summary>
/// Represents the voice profile used to synthesize speech.
/// </summary>
/// <seealso cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles" />
/// <seealso cref="PartyLocalChatControl::GetAvailableTextToSpeechProfiles" />
/// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
/// <seealso cref="PartyLocalChatControl::GetTextToSpeechProfile" />
class PartyTextToSpeechProfile
{
public:
    /// <summary>
    /// Gets the unique identifier associated with this profile.
    /// </summary>
    /// <param name="identifier">
    /// The output unique identifier. The memory for the string remains valid for the lifetime of the profile.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalChatControl::SetTextToSpeechProfile" />
    PartyError GetIdentifier(
        _Outptr_ PartyString * identifier
        ) const party_no_throw;

    /// <summary>
    /// Gets the human-readable name of this profile.
    /// </summary>
    /// <remarks>
    /// This name is intended for use in title UI and logs, but is not localized.
    /// </remarks>
    /// <param name="name">
    /// The output profile name. The memory for the string remains valid for the lifetime of the profile.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetName(
        _Outptr_ PartyString * name
        ) const party_no_throw;

    /// <summary>
    /// Gets the language code associated with this profile.
    /// </summary>
    /// <param name="languageCode">
    /// The output profile language. The memory for the string remains valid for the lifetime of the profile.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetLanguageCode(
        _Outptr_ PartyString * languageCode
        ) const party_no_throw;

    /// <summary>
    /// Gets the gender associated with this profile.
    /// </summary>
    /// <param name="gender">
    /// The output profile gender.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    PartyError GetGender(
        _Out_ PartyGender * gender
        ) const party_no_throw;

    /// <summary>
    /// Retrieves the app's private, custom pointer-sized context value previously associated with this profile.
    /// </summary>
    /// <remarks>
    /// If no custom context has been set yet, the value pointed to by <paramref name="customContext" /> is set to
    /// nullptr.
    /// </remarks>
    /// <param name="customContext">
    /// The output custom context.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if retrieving the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyTextToSpeechProfile::SetCustomContext" />
    PartyError GetCustomContext(
        _Outptr_result_maybenull_ void ** customContext
        ) const party_no_throw;

    /// <summary>
    /// Configures an optional, custom pointer-sized context value with this profile.
    /// </summary>
    /// <remarks>
    /// The custom context is typically used as a "shortcut" that simplifies accessing local, title-specific memory
    /// associated with the profile without requiring a mapping lookup. The value is retrieved using the
    /// <see cref="GetCustomContext()" /> method.
    /// <para>
    /// Any configured value is treated as opaque by the library, and is only valid on the local device; it is not
    /// transmitted over the network.
    /// </para>
    /// </remarks>
    /// <param name="customContext">
    /// An arbitrary, pointer-sized value to store with the profile object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if configuring the custom context succeeded or an error code otherwise. The
    /// human-readable form of the error code can be retrieved via <see cref="PartyManager::GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyTextToSpeechProfile::GetCustomContext" />
    PartyError SetCustomContext(
        _In_opt_ void * customContext
        ) party_no_throw;

private:
    PartyTextToSpeechProfile() = delete;
    PartyTextToSpeechProfile(const PartyTextToSpeechProfile&) = delete;
    PartyTextToSpeechProfile(PartyTextToSpeechProfile&&) = delete;
    PartyTextToSpeechProfile& operator=(const PartyTextToSpeechProfile&) = delete;
    PartyTextToSpeechProfile& operator=(PartyTextToSpeechProfile&&) = delete;
    ~PartyTextToSpeechProfile() = delete;
};

/// <summary>
/// The primary management class for interacting with the Party library.
/// </summary>
/// <remarks>
/// Only a single instance of the class is permitted.
/// </remarks>
class PartyManager
{
public:
    /// <summary>
    /// Retrieves a reference to the PartyManager singleton instance.
    /// </summary>
    /// <returns>
    /// The PartyManager singleton instance.
    /// </returns>
    static PartyManager& GetSingleton(
        ) party_no_throw;

    /// <summary>
    /// Configures an option to fine-tune Party library functionality.
    /// </summary>
    /// <param name="object">
    /// The Party library object that may be required as context for different <see cref="PartyOption" /> values.
    /// </param>
    /// <param name="option">
    /// The Party library option to configure.
    /// </param>
    /// <param name="value">
    /// A pointer to the value used to override <paramref name="option" />. If <paramref name="value" /> is null, the
    /// option will be reset to its default value.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise.
    /// </returns>
    /// <seealso cref="PartyOption" />
    /// <seealso cref="PartyManager::GetOption" />
    static PartyError SetOption(
        _In_opt_ void* object,
        PartyOption option,
        _In_opt_ const void* value
        ) party_no_throw;

    /// <summary>
    /// Retrieves an option used to fine-tune Party library functionality.
    /// </summary>
    /// <param name="object">
    /// The Party library object that may be required as context for different <see cref="PartyOption" /> values.
    /// </param>
    /// <param name="option">
    /// The Party library option to retrieve.
    /// </param>
    /// <param name="value">
    /// An output value to fill with the current <paramref name="option" /> setting. If this option has not yet been
    /// overridden by a call to <see cref="PartyManager::SetOption()" />, this method will retrieve that option's
    /// default value.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise.
    /// </returns>
    /// <seealso cref="PartyOption" />
    /// <seealso cref="PartyManager::SetOption" />
    static PartyError GetOption(
        _In_opt_ const void* object,
        PartyOption option,
        _Out_ void* value
        ) party_no_throw;

    /// <summary>
    /// Get the human-readable form of an error generated by the Party library.
    /// </summary>
    /// <remarks>
    /// These error messages are not localized and are only intended for developers, i.e. these error messages are not
    /// intended to be shown to users via UI.
    /// </remarks>
    /// <param name="error">
    /// An error code.
    /// </param>
    /// <param name="errorMessage">
    /// The output, human-readable error message. The memory for the returned string remains valid for the lifetime of
    /// the process.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise.
    /// </returns>
    static PartyError GetErrorMessage(
        PartyError error,
        _Outptr_ PartyString * errorMessage
        ) party_no_throw;

    /// <summary>
    /// Serializes a network descriptor structure into an opaque string that is safe to communicate over title and
    /// platform-specific communication channels.
    /// </summary>
    /// <remarks>
    /// A network descriptor contains all the information required for a device to connect to the network to which the
    /// descriptor is associated. <paramref name="serializedNetworkDescriptorString" /> is the serialized form of the
    /// descriptor specified by <paramref name="networkDescriptor" /> and is safe to communicate over title and
    /// platform-specific communication channels. The string may be up to
    /// <c>c_maxSerializedNetworkDescriptorStringLength</c> characters long, not including the null terminator, and will
    /// never contain non-ASCII, control, or other characters that would require JSON or XML escaping.
    /// <para>
    /// This method will fail if the network descriptor is not serializable. For example, the descriptor provided
    /// synchronously by a call to <see cref="CreateNewNetwork()" /> is not serializable because it does not contain
    /// enough information for a remote client to connect to the new network. The network descriptor changes and becomes
    /// serializable when the <see cref="PartyCreateNewNetworkCompletedStateChange" /> is provided and indicates
    /// success. The updated network descriptor is provided as a field in the PartyCreateNewNetworkCompletedStateChange.
    /// Once connected to the network, the descriptor can be retrieved using
    /// <see cref="PartyNetwork::GetNetworkDescriptor()" />.
    /// </para>
    /// <para>
    /// PartyNetworkDescriptors work in tandem with PartyInvitations to facilitate inviting remote users to join the
    /// network. First, obtain the network descriptor via PartyNetwork::GetNetworkDescriptor() and serialize it via
    /// SerializeNetworkDescriptor(). Next, create an invitation via <see cref="PartyNetwork::CreateInvitation()" /> or
    /// query for a preexisting invitation via <see cref="PartyNetwork::GetInvitations()" /> and obtain the invitation's
    /// identifier from its configuration via <see cref="PartyInvitation::GetInvitationConfiguration()" />. Last,
    /// include both the serialized network descriptor and the invitation identifier in the payload of the
    /// platform-specific invite mechanism. When the remote user receives the invite, they deserialize the network
    /// descriptor from the platform-invite payload via <see cref="DeserializeNetworkDescriptor()" /> and pass the
    /// deserialized descriptor to <see cref="ConnectToNetwork()" />. After connecting, the remote user joins by
    /// authenticating into the network via <see cref="PartyNetwork::AuthenticateLocalUser()" /> with the invitation
    /// identifier in the platform-invite payload.
    /// </para>
    /// <para>
    /// This method can be called prior to the <see cref="Initialize()" /> method.
    /// </para>
    /// </remarks>
    /// <param name="networkDescriptor">
    /// The network descriptor to serialize.
    /// </param>
    /// <param name="serializedNetworkDescriptorString">
    /// The output buffer to which the serialized network descriptor string is written. The serialized network
    /// descriptor string will never contain non-ASCII, control, or other characters that would require JSON or XML
    /// escaping.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the descriptor is serializable and was successfully serialized, or an error code
    /// otherwise.
    /// </returns>
    /// <seealso cref="PartyManager::DeserializeNetworkDescriptor" />
    /// <seealso cref="PartyNetwork::GetNetworkDescriptor" />
    /// <seealso cref="PartyManager::ConnectToNetwork" />
    /// <seealso cref="PartyInvitation" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    /// <seealso cref="PartyNetwork::GetInvitations" />
    /// <seealso cref="PartyInvitation::GetInvitationConfiguration" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    static PartyError SerializeNetworkDescriptor(
        const PartyNetworkDescriptor * networkDescriptor,
        _Out_writes_z_(c_maxSerializedNetworkDescriptorStringLength + 1) char * serializedNetworkDescriptorString
        ) party_no_throw;

    /// <summary>
    /// Deserializes a network descriptor structure from an opaque string serialized via a prior call to
    /// <see cref="SerializeNetworkDescriptor()" />.
    /// </summary>
    /// <remarks>
    /// The resulting <paramref name="networkDescriptor" /> can be used to connect to a network via
    /// <see cref="ConnectToNetwork()" />.
    /// <para>
    /// This method can be called prior to the <see cref="Initialize()" /> method.
    /// </para>
    /// </remarks>
    /// <param name="serializedNetworkDescriptorString">
    /// The serialized network descriptor string.
    /// </param>
    /// <param name="networkDescriptor">
    /// The output network descriptor. <paramref name="serializedNetworkDescriptorString" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the descriptor string is a valid serialized network descriptor string and was
    /// successfully deserialized, or an error code otherwise.
    /// </returns>
    /// <seealso cref="PartyManager::SerializeNetworkDescriptor" />
    /// <seealso cref="PartyManager::ConnectToNetwork" />
    static PartyError DeserializeNetworkDescriptor(
        PartyString serializedNetworkDescriptorString,
        _Out_ PartyNetworkDescriptor * networkDescriptor
        ) party_no_throw;

    /// <summary>
    /// Optionally configures the memory allocation and freeing callbacks the Party library should use.
    /// </summary>
    /// <remarks>
    /// Using **PFMemSetFunctions** from the PlayFab Core SDK will establish universal custom memory allocation and
    /// freeing callbacks across all PlayFab SDK components, including Party. Using this function allows the title to
    /// implement custom memory allocation and freeing callbacks that override the respective memory callbacks of the
    /// PlayFab Core SDK. This method allows the title to install custom memory allocation functions in order to service
    /// all requests by the Party library for new memory buffers instead of using its default allocation functions.
    /// <para>
    /// The <paramref name="allocateMemoryCallback" /> and <paramref name="freeMemoryCallback" /> parameters must both
    /// be non-null.
    /// </para>
    /// <para>
    /// To use this method, it must be called before any other Party method except for
    /// PartyManager::GetMemoryCallbacks(). This method cannot be called again for the lifetime of this process.
    /// </para>
    /// </remarks>
    /// <param name="allocateMemoryCallback">
    /// A pointer to the custom allocation callback to use.
    /// </param>
    /// <param name="freeMemoryCallback">
    /// A pointer to the custom freeing callback to use.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAllocateMemoryCallback" />
    /// <seealso cref="PartyFreeMemoryCallback" />
    /// <seealso cref="PartyManager::GetMemoryCallbacks" />
    static PartyError SetMemoryCallbacks(
        _In_ PartyAllocateMemoryCallback allocateMemoryCallback,
        _In_ PartyFreeMemoryCallback freeMemoryCallback
        ) party_no_throw;

    /// <summary>
    /// Retrieves the memory allocation and freeing callbacks the Party library is using.
    /// </summary>
    /// <remarks>
    /// This retrieves the memory allocation functions servicing requests by the Party library for new memory.
    /// <para>
    /// This method does not require the <see cref="Initialize()" /> method to have been called first.
    /// </para>
    /// </remarks>
    /// <param name="allocateMemoryCallback">
    /// A place to store a pointer to the memory allocation callback currently used.
    /// </param>
    /// <param name="freeMemoryCallback">
    /// A place to store a pointer to the memory freeing callback currently used.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyAllocateMemoryCallback" />
    /// <seealso cref="PartyFreeMemoryCallback" />
    /// <seealso cref="PartyManager::SetMemoryCallbacks" />
    static PartyError GetMemoryCallbacks(
        _Out_ PartyAllocateMemoryCallback * allocateMemoryCallback,
        _Out_ PartyFreeMemoryCallback * freeMemoryCallback
        ) party_no_throw;

    /// <summary>
    /// Optionally configures the profiling event callbacks the Party library will make when entering or exiting
    /// instrumented methods.
    /// </summary>
    /// <remarks>
    /// This method allows the title to install custom profiling callback functions in order to record and visualize
    /// Party library performance metrics in external profiling tools.
    /// <para>
    /// This method can only be called when the Party library is uninitialized. Calling while Party is initialized will
    /// fail and return an error.
    /// </para>
    /// <para>
    /// Setting an optional callback equal to <c>nullptr</c> will cause the Party library to not make any profiling
    /// callbacks for that event type.
    /// </para>
    /// <para>
    /// In order to minimize the impact of profiling on title performance, callbacks for these events should be kept as
    /// lightweight as possible, as they are expected to fire hundreds or thousands of times per second.
    /// </para>
    /// <para>
    /// This method is only supported on the Windows and Microsoft Game Core versions of the library. Calls on other
    /// platforms will fail.
    /// </para>
    /// </remarks>
    /// <param name="profilingMethodEntranceCallback">
    /// The callback to be made when the Party library enters an internal method which is instrumented for profiling.
    /// </param>
    /// <param name="profilingMethodExitCallback">
    /// The callback to be made when the Party library is about to exit an internal method which is instrumented for
    /// profiling.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyProfilingMethodEntranceCallback" />
    /// <seealso cref="PartyProfilingMethodExitCallback" />
    /// <seealso cref="PartyManager::GetProfilingCallbacksForMethodEntryExit" />
    static PartyError SetProfilingCallbacksForMethodEntryExit(
        _In_opt_ PartyProfilingMethodEntranceCallback profilingMethodEntranceCallback,
        _In_opt_ PartyProfilingMethodExitCallback profilingMethodExitCallback
        ) party_no_throw;

    /// <summary>
    /// Retrieves the profiling event callbacks the Party library is configured to use when entering or exiting
    /// instrumented methods.
    /// </summary>
    /// <remarks>
    /// This method retrieves the profiling callback functions the Party library is calling for the instrumented event
    /// types.
    /// <para>
    /// A callback equal to <c>nullptr</c> indicates that the Party library will not make any profiling callbacks for
    /// that event type.
    /// </para>
    /// <para>
    /// This method is only supported on the Windows and Microsoft Game Core versions of the library. Calls on other
    /// platforms will fail.
    /// </para>
    /// </remarks>
    /// <param name="profilingMethodEntranceCallback">
    /// A pointer to the callback made when the Party library enters an internal method which is instrumented for
    /// profiling.
    /// </param>
    /// <param name="profilingMethodExitCallback">
    /// A pointer to the callback made when the Party library is about to exit an internal method which is instrumented
    /// for profiling.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyProfilingMethodEntranceCallback" />
    /// <seealso cref="PartyProfilingMethodExitCallback" />
    /// <seealso cref="PartyManager::SetProfilingCallbacksForMethodEntryExit" />
    static PartyError GetProfilingCallbacksForMethodEntryExit(
        _Outptr_result_maybenull_ PartyProfilingMethodEntranceCallback * profilingMethodEntranceCallback,
        _Outptr_result_maybenull_ PartyProfilingMethodExitCallback * profilingMethodExitCallback
        ) party_no_throw;

    /// <summary>
    /// Optionally configures the processor on which internal Party library threads run.
    /// </summary>
    /// <remarks>
    /// This method enables the title to configure the processor affinity for internal Party library threads of a given
    /// type. On Windows, the <c>Audio</c> type affects both the PlayFab Party library owned threads and threads owned
    /// by XAudio2. For more information, see <see cref="PartyThreadId" />.
    /// <para>
    /// For all platforms, <b><i>except PlayStationÂ®</i></b>, this method may be called at any time before or after
    /// <see cref="Initialize()" /> and takes effect immediately. <b>On PlayStationÂ®, attempting to set the processor
    /// affinity for the <see cref="PartyThreadId" /> Audio thread after calling Initialize() can result in audio
    /// rendering failures</b>. It's currently recommended to call this method prior to calling Initialize().
    /// </para>
    /// <para>
    /// Thread processor settings are persisted across calls to <see cref="Cleanup()" /> and Initialize(). When there
    /// are more than 64 cores present, this method always applies to processor group 0.
    /// </para>
    /// <para>
    /// In order to specify any processor, pass <c>c_anyProcessor</c> as the <paramref name="threadAffinityMask" />
    /// parameter. <c>c_anyProcessor</c> is also the default value the Party library uses if this method is never
    /// called.
    /// </para>
    /// </remarks>
    /// <param name="threadId">
    /// The type of internal Party library thread to configure processor affinity.
    /// </param>
    /// <param name="threadAffinityMask">
    /// The affinity mask for this type of Party library thread.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyThreadId" />
    /// <seealso cref="PartyManager::GetThreadAffinityMask" />
    static PartyError SetThreadAffinityMask(
        PartyThreadId threadId,
        uint64_t threadAffinityMask
        ) party_no_throw;

    /// <summary>
    /// Retrieves the current set of processors on which internal Party library threads will run or are running as an
    /// affinity mask.
    /// </summary>
    /// <remarks>
    /// This retrieves the current processor affinity for internal Party library threads of a given type.
    /// <para>
    /// This method does not require <see cref="Initialize()" /> to have been called first.
    /// </para>
    /// <para>
    /// A reported value of <c>c_anyProcessor</c> written to <paramref name="threadAffinityMask" /> indicates that the
    /// thread is free to run on any processor.
    /// </para>
    /// </remarks>
    /// <param name="threadId">
    /// The type of internal Party library thread for which processor affinity should be retrieved.
    /// </param>
    /// <param name="threadAffinityMask">
    /// The output affinity mask for this type of Party library thread.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyThreadId" />
    /// <seealso cref="PartyManager::SetThreadAffinityMask" />
    static PartyError GetThreadAffinityMask(
        PartyThreadId threadId,
        _Out_ uint64_t * threadAffinityMask
        ) party_no_throw;

    /// <summary>
    /// Configures the work mode of the Party library processing task associated with <paramref name="threadId" />.
    /// </summary>
    /// <remarks>
    /// If the work mode is set as <see cref="PartyWorkMode::Automatic" />, the processing task associated with
    /// <paramref name="threadId" /> is handled internally by the Party library. If the work mode is configured as
    /// <see cref="PartyWorkMode::Manual" />, the title must instead perform the processing task associated with
    /// <paramref name="threadId" /> via periodic calls to <see cref="DoWork()" />.
    /// <para>
    /// This method can only be called when the Party library is uninitialized. Calling while initialized will fail and
    /// return an error.
    /// </para>
    /// <para>
    /// Configuration of the work mode associated with <paramref name="threadId" /> persists across subsequent calls to
    /// <see cref="Cleanup()" /> and <see cref="Initialize()" />.
    /// </para>
    /// </remarks>
    /// <param name="threadId">
    /// A high-level categorization of the Party library's internal processing task.
    /// </param>
    /// <param name="workMode">
    /// The target work mode for the associated processing task.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::GetWorkMode" />
    /// <seealso cref="PartyManager::DoWork" />
    static PartyError SetWorkMode(
        PartyThreadId threadId,
        PartyWorkMode workMode
        ) party_no_throw;

    /// <summary>
    /// Retrieves the current work mode of the Party library's internal processing task associated with
    /// <paramref name="threadId" />.
    /// </summary>
    /// <param name="threadId">
    /// A high-level categorization of the Party library's internal processing task.
    /// </param>
    /// <param name="workMode">
    /// The current work mode for the associated processing task.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c>
    /// </returns>
    /// <seealso cref="PartyManager::SetWorkMode" />
    /// <seealso cref="PartyManager::DoWork" />
    static PartyError GetWorkMode(
        PartyThreadId threadId,
        _Out_ PartyWorkMode * workMode
        ) party_no_throw;

    /// <summary>
    /// Initializes the PartyManager object instance.
    /// </summary>
    /// <remarks>
    /// This must be called before any other method, aside from the static methods <see cref="GetSingleton()" />,
    /// <see cref="SetMemoryCallbacks()" />, <see cref="GetMemoryCallbacks()" />,
    /// <see cref="SetThreadAffinityMask()" />, <see cref="GetThreadAffinityMask()" />,
    /// <see cref="SerializeNetworkDescriptor()" />, <see cref="DeserializeNetworkDescriptor()" />. Initialize() cannot
    /// be called again without a subsequent <see cref="Cleanup()" /> call.
    /// <para>
    /// Every call to Initialize() should have a corresponding Cleanup() call.
    /// </para>
    /// <para>
    /// Apps using the Microsoft Game Core version of the Party library will need to wait for the Game Core Networking
    /// stack to be initialized prior to calling this method. Determining the status of the network stack can be done
    /// using the Game Core XNetworkingGetConnectivityHint API.
    /// </para>
    /// <para>
    /// Apps using the Microsoft Game Core version of the Party library must listen for app state notifications via the
    /// RegisterAppStateChangeNotification API. When the app is suspended, the app must call PartyManager::Cleanup().
    /// When the app is resumed, the title must wait for the Game Core networking stack to ready and then re-initialize
    /// the Party library by calling PartyManager::Initialize().
    /// </para>
    /// <para>
    /// The Title ID provided to <paramref name="initializationConfiguration" /> must be the same Title ID used to
    /// acquire the PlayFab Entity Handle that will be passed to <see cref="CreateLocalUser()" />.
    /// </para>
    /// </remarks>
    /// <param name="initializationConfiguration">
    /// The Party library initialization configuration.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::CreateLocalUser" />
    /// <seealso cref="PartyManager::Cleanup" />
    /// <seealso cref="PartyManager::GetSingleton" />
    /// <seealso cref="PartyManager::SetMemoryCallbacks" />
    /// <seealso cref="PartyManager::GetMemoryCallbacks" />
    /// <seealso cref="PartyManager::SetThreadAffinityMask" />
    /// <seealso cref="PartyManager::GetThreadAffinityMask" />
    /// <seealso cref="PartyManager::SerializeNetworkDescriptor" />
    /// <seealso cref="PartyManager::DeserializeNetworkDescriptor" />
    /// <nyi />
    PartyError Initialize(
        const PartyInitializationConfiguration * initializationConfiguration
        ) party_no_throw;

    /// <summary>
    /// Immediately reclaims all resources associated with all Party library objects.
    /// </summary>
    /// <remarks>
    /// If local users were participating in a PartyNetwork, they're removed ungracefully (it appears to remote devices
    /// as if network connectivity to this device is lost), so best practice is to call
    /// <see cref="PartyNetwork::LeaveNetwork()" /> first on all networks returned from a call to
    /// <see cref="GetNetworks()" /> and wait for the corresponding <see cref="PartyLeaveNetworkCompletedStateChange" />
    /// to have the local users exit any existing PartyNetworks gracefully.
    /// <para>
    /// This method isn't thread-safe and may not be called concurrently with other non-static Party library methods.
    /// After calling this method, all Party library state is invalidated.
    /// </para>
    /// <para>
    /// Titles using the Microsoft Game Core version of the Party library must listen for app state notifications via
    /// the RegisterAppStateChangeNotification API. When the app is suspended, the title must call
    /// PartyManager::Cleanup(). When the app is resumed, the title must wait for the Game Core networking stack to be
    /// ready and then reinitialize the Party library by calling PartyManager::Initialize().
    /// </para>
    /// <para>
    /// Every call to <see cref="Initialize()" /> should have a corresponding Cleanup() call.
    /// </para>
    /// </remarks>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::Initialize" />
    /// <seealso cref="PartyNetwork::LeaveNetwork" />
    /// <seealso cref="PartyManager::GetNetworks" />
    PartyError Cleanup(
        ) party_no_throw;

    /// <summary>
    /// Retrieves an array of all PartyStateChanges to process since the last such call.
    /// </summary>
    /// <remarks>
    /// This method provides the Party library an opportunity to synchronize state with remote devices or services, and
    /// retrieves a list of all changes currently available for the title since the last call to this method. The title
    /// should use the provided array of 0 or more changes to update its own state or UI, and then call
    /// <see cref="FinishProcessingStateChanges()" /> with them in a timely manner.
    /// <para>
    /// Party library state exposed by the library can change during this call, so you must be thread-safe in your use
    /// of it. For example, invoking StartProcessingStateChanges() on your UI thread at the same time a separate worker
    /// thread is looping through the list of endpoints returned by <see cref="PartyNetwork::GetEndpoints()" /> may
    /// result in crashes because StartProcessingStateChanges() can alter the memory associated with the endpoint list.
    /// StartProcessingStateChanges() should be called frequently-- at least once per graphics frame. It's designed to
    /// execute and return quickly such that it can be called on your main UI thread with negligible impact. For best
    /// results, you should also minimize the time you spend handling state changes before calling
    /// FinishProcessingStateChanges().
    /// </para>
    /// <para>
    /// Each state change returned by StartProcessingStateChanges() must be returned to FinishProcessingStateChanges()
    /// exactly once, but may be returned out of order and may be interleaved with state changes from other calls to
    /// StartProcessingStateChanges(). Any resources associated with a specific state change are guaranteed to stay
    /// valid until the state change is returned to FinishProcessingStateChanges().
    /// </para>
    /// </remarks>
    /// <param name="stateChangeCount">
    /// The output number of PartyStateChange entries for the title to handle in the <paramref name="stateChanges" />
    /// array.
    /// </param>
    /// <param name="stateChanges">
    /// A library-allocated output array of all PartyStateChange entries for the title to handle and then pass to
    /// <see cref="FinishProcessingStateChanges()" />.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyStateChange" />
    /// <seealso cref="PartyManager::FinishProcessingStateChanges" />
    PartyError StartProcessingStateChanges(
        _Out_ uint32_t * stateChangeCount,
        _Outptr_result_buffer_(*stateChangeCount) PartyStateChangeArray * stateChanges
        ) party_no_throw;

    /// <summary>
    /// Returns an array of PartyStateChanges that were being processed.
    /// </summary>
    /// <remarks>
    /// This method informs the Party library that the state changes reported by a previous call to
    /// <see cref="StartProcessingStateChanges()" /> have now been handled by the title, so their associated resources
    /// can be reclaimed. You may call FinishProcessingStateChanges() with any number of state changes. Each state
    /// change returned by StartProcessingStateChanges() must be returned to FinishProcessingStateChanges() exactly
    /// once, but may be returned out of order and may be interleaved with state changes from other calls to
    /// StartProcessingStateChanges(). Even if state changes are held across subsequent calls to
    /// StartProcessingStateChanges(), the Party library state returned by all getters will advance and may no longer
    /// reflect the same state that the held state changes refer to.
    /// <para>
    /// Any resources associated with a specific state change are guaranteed to stay valid until the state change is
    /// returned to FinishProcessingStateChanges().
    /// </para>
    /// <para>
    /// For best results, you should minimize the time you spend handling state changes before calling
    /// FinishProcessingStateChanges().
    /// </para>
    /// </remarks>
    /// <param name="stateChangeCount">
    /// The number of changes, provided in the list specified by <paramref name="stateChanges" />, previously returned
    /// by <see cref="StartProcessingStateChanges()" /> that have now been handled by the title.
    /// </param>
    /// <param name="stateChanges">
    /// The array of changes previously returned by <see cref="StartProcessingStateChanges()" /> that have now been
    /// handled by the title.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyStateChange" />
    /// <seealso cref="PartyManager::StartProcessingStateChanges" />
    PartyError FinishProcessingStateChanges(
        uint32_t stateChangeCount,
        _In_reads_(stateChangeCount) PartyStateChangeArray stateChanges
        ) party_no_throw;

    /// <summary>
    /// Synchronously performs the processing task associated with <paramref name="threadId" />.
    /// </summary>
    /// <remarks>
    /// This method fails and returns an error if the work mode of <paramref name="threadId" /> hasn't previously been
    /// set to <see cref="PartyWorkMode::Manual" /> via a call to <see cref="SetWorkMode()" />. Additionally, on the
    /// Windows and Microsoft Game Core versions of the library, this method fails and returns an error if the title
    /// thread calling this method doesn't exist in a COM multithreaded apartment when <paramref name="threadId" /> is
    /// <see cref="PartyThreadId::Networking" />.
    /// <para>
    /// By default, the Party library will internally manage the processing task associated with
    /// <paramref name="threadId" />. However, if the work mode of <paramref name="threadId" /> is configured as
    /// <see cref="PartyWorkMode::Manual" /> via a call to SetWorkMode(), the title must periodically call this method
    /// to perform the processing task instead.
    /// </para>
    /// <para>
    /// The processing task associated with <paramref name="threadId" /> dictates the frequency at which this method
    /// should be called. The processing task associated with <see cref="PartyThreadId::Audio" /> should be performed
    /// every 40 ms, while the processing task associated with <see cref="PartyThreadId::Networking" /> should be
    /// performed every 50 ms to 100 ms. Internally, processing tasks attempt to handle small variations in timing, but
    /// issues arise if timing strays too far from what is expected (for example, audio stutter, network state
    /// desynchronization).
    /// </para>
    /// <para>
    /// For more information on processing task types and timing expectations, see <see cref="PartyThreadId" />.
    /// </para>
    /// </remarks>
    /// <param name="threadId">
    /// A high-level categorization of the Party library's internal processing task.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::SetWorkMode" />
    /// <seealso cref="PartyManager::GetWorkMode" />
    PartyError DoWork(
        PartyThreadId threadId
        ) party_no_throw;

    /// <summary>
    /// Gets an array containing the set of regions for which your title is configured, along with round trip latency
    /// information.
    /// </summary>
    /// <remarks>
    /// The array provided by this method isn't populated until the first <see cref="PartyRegionsChangedStateChange" />
    /// is provided. Each subsequent PartyRegionsChangedStateChange indicates an update to this set of regions.
    /// Background operations populating this set begin when <see cref="Initialize()" /> is called, unless the
    /// <see cref="PartyOption::RegionUpdateConfiguration" /> option was used to configure an update mode of
    /// <see cref="PartyRegionUpdateMode::Deferred" />. The deferred mode means region retrieval and connection quality
    /// measurement doesn't begin until the application calls <see cref="CreateNewNetwork()" /> with a zero entry region
    /// array, or isn't connected to any network and calls
    /// <see cref="PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles()" />,
    /// <see cref="PartyLocalChatControl::SetTextToSpeechProfile()" />, or
    /// <see cref="PartyLocalChatControl::SetTranscriptionOptions()" /> with option flags that include
    /// <see cref="PartyVoiceChatTranscriptionOptions::TranscribeSelfRegardlessOfNetworkState" />. The
    /// <see cref="PartyOption::RegionUpdateConfiguration" /> option can also be used to configure the interval after
    /// which the regions array is to be refreshed.
    /// <para>
    /// The returned array is sorted in order of increasing latency. Latency for a region is defined as the end-to-end
    /// time it takes to send a UDP datagram to the region's PlayFab Quality of Service beacon and receive a response.
    /// The library measures latency for each region several times in order to calculate high fidelity results.
    /// Applications can alter low-level aspects of this connection quality measurement procedure using the
    /// <see cref="PartyOption::RegionQualityMeasurementConfiguration" /> option.
    /// </para>
    /// <para>
    /// A latency equivalent to UINT32_MAX indicates that the region is supported but latency information couldn't be
    /// determined, either due to failures internal to the library or a timeout was reached without receiving a response
    /// from the beacon. This can be used for diagnostic purposes (a UINT32_MAX latency to every region likely indicates
    /// a local configuration problem) or used to filter out regions that are unlikely to be viable during region
    /// selection.
    /// </para>
    /// <para>
    /// The returned names in the <see cref="PartyRegion" /> structures aren't localized to the current user's language,
    /// and showing the strings directly in UI isn't recommended outside of troubleshooting.
    /// </para>
    /// <para>
    /// Don't assume the set of regions returned remain the same over the life of your title. The PlayFab Party library
    /// will automatically take advantage of additions and changes to available regions over time to continually improve
    /// the experience for end users.
    /// </para>
    /// </remarks>
    /// <param name="regionCount">
    /// The output number of regions provided in <paramref name="regions" />.
    /// </param>
    /// <param name="regions">
    /// A library-allocated output array containing the set of regions.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyRegionsChangedStateChange" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    PartyError GetRegions(
        _Out_ uint32_t * regionCount,
        _Outptr_result_buffer_(*regionCount) const PartyRegion ** regions
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous attempt to create a new network.
    /// </summary>
    /// <remarks>
    /// A PartyNetwork is a set of devices that are connected via a client-server topology to a transparent cloud relay
    /// server. Each device in the network contains a collection of endpoints associated with the network; directed
    /// messages can be sent from an endpoint on the local device to any set of endpoints, or broadcast to all
    /// endpoints. This method queues an attempt to allocate a relay, thus creating a new network, on behalf of the user
    /// represented by <paramref name="localUser" /> but does not connect the local device to the network.
    /// <para>
    /// If no devices connect to the network within 10 minutes of the relay's creation, it will shut down. The network
    /// stays active indefinitely while at least one device is connected, migrating to a new relay if required. If no
    /// devices are connected to the network, the relay becomes inactive and shuts down after 1 minute of inactivity.
    /// </para>
    /// <para>
    /// The local device can queue an attempt to connect to the new network by immediately passing
    /// <paramref name="networkDescriptor" /> to <see cref="ConnectToNetwork()" />. Using this descriptor with
    /// SerializeNetworkDescriptor() fails because the descriptor doesn't contain enough information for a remote device
    /// to connect to the network. The network descriptor changes and becomes serializable when the
    /// <see cref="PartyCreateNewNetworkCompletedStateChange" /> is provided and indicates success. The updated network
    /// descriptor is provided as a field in the PartyCreateNewNetworkCompletedStateChange. The descriptor can also be
    /// retrieved after connecting to the network using <see cref="PartyNetwork::GetNetworkDescriptor()" />.
    /// </para>
    /// <para>
    /// The network is created in the first available region using the order specified in <paramref name="regions" />.
    /// If none of the specified regions are available, the network will not be created. Specifying 0 for
    /// <paramref name="regionCount" /> defaults to using all regions in which the title is configured, ordered by
    /// lowest latency to this device, which is the same array returned by GetRegions().
    /// </para>
    /// <para>
    /// The default region selection only includes latency measurements from this device and not from any other devices.
    /// Titles that have a set of participants for the session known up front should gather measurements from all
    /// devices prior to creating the network and construct a new <paramref name="regions" /> array ordered by lowest
    /// aggregate latency for the whole group.
    /// </para>
    /// <para>
    /// For titles that support join-in-progress, the region with the best overall latency for the group of connected
    /// participants may change as devices join and leave. Titles should take advantage of Party's support for being
    /// connected to multiple networks simultaneously to migrate devices seamlessly to a network created in a region
    /// with better aggregate latency for the group. The title can gather region latency measurements via messages over
    /// the original Party network or information uploaded to an external roster service, create a new Party network in
    /// a region with lower aggregate latency, and instruct all devices to connect to the more favorable network and
    /// disconnect from the original one.
    /// </para>
    /// <para>
    /// If the <see cref="PartyOption::RegionUpdateConfiguration" /> option was used to configure an update mode of
    /// <see cref="PartyRegionUpdateMode::Deferred" />, then retrieving the set of available regions and measuring
    /// connection quality to them may not have started yet, or the last update may have exceeded the configured refresh
    /// interval age. Specifying 0 for <paramref name="regionCount" /> to CreateNewNetwork() ensures any deferred region
    /// update has started and its associated <see cref="PartyRegionsChangedStateChange" /> is provided prior to this
    /// call's <see cref="PartyCreateNewNetworkCompletedStateChange" /> completion.
    /// </para>
    /// <para>
    /// The initial invitation for the newly created network isn't owned by any user. Therefore calling
    /// <see cref="PartyInvitation::GetCreatorEntityId()" /> returns nullptr for the initial invitation. The initial
    /// invitation also persists for the lifetime of the network until specifically revoked via
    /// <see cref="PartyNetwork::RevokeInvitation()" />. New invitations can be created for the network via
    /// <see cref="PartyNetwork::CreateInvitation()" /> by local users authenticated into the network, and those
    /// invitations will persist until their creating local users are removed from the network. Users join the network
    /// via <see cref="PartyNetwork::AuthenticateLocalUser()" /> by providing the identifier of an invitation that was
    /// successfully created, has not been revoked, and allows them to join.
    /// </para>
    /// <h>Retrying on failure</h>
    /// <para>
    /// When CreateNewNetwork() fails asynchronously, a PartyCreateNewNetworkCompletedStateChange is provided by
    /// <see cref="StartProcessingStateChanges" /> indicating the result. The operation may be retried after a delay,
    /// depending on the <see cref="PartyStateChangeResult" /> provided.
    /// </para>
    /// <para>
    /// `` | Result | Retry behavior |
    /// ` | --- | --- |
    /// ` | InternetConnectivityError | Retry with a small delay of no less than 10 seconds. For your app, it may be
    ///         more appropriate to display the error to the user immediately, rather than retrying automatically. |
    /// ` | PartyServiceError | Retry with an exponential backoff. Start with a minimum delay of no less than 10
    ///         seconds, doubling the delay with each retry. |
    /// ` | NoServersAvailable | Retry with an exponential backoff. Start with a minimum delay of no less than 30
    ///         seconds, doubling the delay with each retry. For your app, it may be more appropriate to display the
    ///         error to the user immediately, rather than retrying automatically. |
    /// ` | UserNotAuthorized | This result can mean that the user's entity token was invalid, expired, or that the user
    ///         was not authorized for other reasons. Retry no more than one time, and only after getting a new entity
    ///         token for the user and calling <see cref="PartyLocalUser::UpdateEntityToken()" />.|
    /// ` | UserCreateNetworkThrottled | Don't retry automatically. Instead, display a message to the user and wait for
    ///         the user to initiate another attempt. |
    /// ` | FailedToBindToLocalUdpSocket | This result means that the library couldn't bind to the local UDP socket
    ///         specified in the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option. The title must clean up
    ///         its instance of the library, update the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option to
    ///         a valid, available bind address, and reinitialize the library.
    /// </para>
    /// </remarks>
    /// <param name="localUser">
    /// The local user to which the network creation and relay allocation is attributed.
    /// </param>
    /// <param name="networkConfiguration">
    /// Network configuration properties such as max user count and max device count. These properties last for the
    /// lifetime of the network and can't be changed.
    /// </param>
    /// <param name="regionCount">
    /// The number of regions provided in the array of preferred regions specified via <paramref name="regions" />. If
    /// this is zero, the library uses all regions in which the title is configured, ordered by lowest round trip
    /// latency from this device.
    /// </param>
    /// <param name="regions">
    /// The array of preferred regions in which the network should be created. The network is created in the first
    /// available region.
    /// </param>
    /// <param name="initialInvitationConfiguration">
    /// An optionally specified configuration for the initial invitation.
    /// <para>
    /// If this value is null, then default configuration values are used. By default, PlayFab Party generates a unique
    /// invitation identifier for the title, the revocability is <see cref="PartyInvitationRevocability::Anyone" />, and
    /// the PlayFab Entity ID list is empty, allowing any user to join using the invitation.
    /// </para>
    /// <para>
    /// If a configuration is provided, the title may optionally specify the identifier on the configuration. If the
    /// identifier is nullptr or an empty string, the PlayFab Party library generates an identifier for the title that's
    /// guaranteed to be different from all identifiers that the PlayFab Party library will generate for all future
    /// invitations on this network across all devices. Titles may specify their own identifier by providing a non-null,
    /// nonempty value in the configuration. If the title specifies the identifier, it's the title's responsibility to
    /// ensure that this identifier doesn't collide with the identifiers of future invitations created on this network
    /// via <see cref="PartyNetwork::CreateInvitation()" /> on any device.
    /// </para>
    /// <para>
    /// If a configuration is provided, its revocability must be PartyInvitationRevocability::Anyone.
    /// </para>
    /// <para>
    /// If a configuration is provided and the list of PlayFab Entity IDs is empty, all users are allowed to join using
    /// the new invitation.
    /// </para>
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <param name="networkDescriptor">
    /// The optional, output network descriptor that can be used to immediately queue a connection of the local device
    /// to the newly created network via <see cref="ConnectToNetwork()" />.
    /// </param>
    /// <param name="appliedInitialInvitationIdentifier">
    /// The optional, output buffer to which the initial invitation's identifier is written. If
    /// <paramref name="initialInvitationConfiguration" /> is provided with a non-null, nonempty identifier, then this
    /// buffer will be filled with the same identifier. If an initial configuration isn't provided or the provided
    /// configuration has an empty or null identifier, the PlayFab Party library generates one and returns it to the
    /// title in this buffer.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to create a new network began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyCreateNewNetworkCompletedStateChange" />
    /// <seealso cref="PartyManager::ConnectToNetwork" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyNetwork::LeaveNetwork" />
    /// <seealso cref="PartyNetwork::RevokeInvitation" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    /// <seealso cref="PartyInvitation" />
    /// <seealso cref="PartyInvitationConfiguration" />
    PartyError CreateNewNetwork(
        const PartyLocalUser * localUser,
        const PartyNetworkConfiguration * networkConfiguration,
        uint32_t regionCount,
        const PartyRegion * regions,
        _In_opt_ const PartyInvitationConfiguration * initialInvitationConfiguration,
        _In_opt_ void * asyncIdentifier,
        _Out_opt_ PartyNetworkDescriptor * networkDescriptor,
        _Out_writes_opt_z_(c_maxInvitationIdentifierStringLength + 1) char * appliedInitialInvitationIdentifier
        ) party_no_throw;

    /// <summary>
    /// Queues an asynchronous attempt to connect to a network.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous attempt to connect to the network associated with
    /// <paramref name="networkDescriptor" /> by negotiating and establishing a secure DTLS connection from the local
    /// device to the transparent cloud relay server. A <see cref="PartyConnectToNetworkCompletedStateChange" /> will be
    /// provided upon completion of the asynchronous operation, indicating success or failure. On success, the local
    /// device will have established a connection to the transparent cloud relay server. On failure, a
    /// <see cref="PartyNetworkDestroyedStateChange" /> will be generated. No other devices will become visible, and the
    /// local device won't be visible to any remote devices, until at least one local user is successfully authenticated
    /// via <see cref="PartyNetwork::AuthenticateLocalUser()" />.
    /// <para>
    /// After the device successfully connects to the network, it must authenticate into the network via
    /// PartyNetwork::AuthenticateLocalUser(). If the device is connected to the network but unauthenticated for more
    /// than one minute, the device will be disconnected and a PartyNetworkDestroyedStateChange will be generated. This
    /// will occur regardless of whether the device never authenticated or lost authentication after calling
    /// <see cref="PartyNetwork::RemoveLocalUser()" />.
    /// </para>
    /// <para>
    /// This method optionally provides <paramref name="network" /> as output that can immediately be used to perform
    /// asynchronous network operations, such as <see cref="PartyNetwork::CreateInvitation()" /> and
    /// <see cref="PartyNetwork::CreateEndpoint()" />. These asynchronous operations will be internally queued until the
    /// connection completes, at which point they'll be processed. This <paramref name="network" /> will also be
    /// provided on the resulting PartyConnectToNetworkCompletedStateChange where it will be fully connected and
    /// associated with the provided <paramref name="asyncIdentifier" />.
    /// </para>
    /// <para>
    /// This operation will asynchronously fail with <see cref="PartyConnectToNetworkCompletedStateChange::result" />
    /// set to <see cref="PartyStateChangeResult::NetworkLimitReached" /> if the limit of connected devices for the
    /// network has been reached as specified in the network's configuration when it was created via
    /// <see cref="CreateNewNetwork()" />.
    /// </para>
    /// <para>
    /// On successful return, this method invalidates the memory for any array previously returned by
    /// <see cref="GetNetworks()" />, as it synchronously adds the new network to the array.
    /// <see cref="StartProcessingStateChanges()" /> also invalidates the memory for the array. The returned
    /// <paramref name="network" /> object will be valid until a PartyNetworkDestroyedStateChange has been generated and
    /// all state changes referencing the object have been returned to <see cref="FinishProcessingStateChanges()" />.
    /// </para>
    /// </para>
    /// <h>Retrying on failure</h>
    /// <para>
    /// When ConnectToNetwork() fails asynchronously, a PartyConnectToNetworkCompletedStateChange is provided by
    /// StartProcessingStateChanges() indicating the result. The operation may be retried after a delay, depending on
    /// the <see cref="PartyStateChangeResult" /> provided.
    /// </para>
    /// <para>
    /// `` | Result | Retry behavior |
    /// ` | --- | --- |
    /// ` | InternetConnectivityError | Retry with a small delay of no less than 10 seconds. For your app, it may be
    ///         more appropriate to display the error to the user immediately, rather than retrying automatically. |
    /// ` | NetworkLimitReached | Don't retry automatically. Instead, display a message to the user and wait for the
    ///         user to initiate another attempt. |
    /// ` | NetworkNoLongerExists | Don't retry. |
    /// ` | VersionMismatch | Don't retry. |
    /// ` | FailedToBindToLocalUdpSocket | This result means that the library couldn't bind to the local UDP socket
    ///         specified in the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option. The title must clean up
    ///         its instance of the library, update the <see cref="PartyOption::LocalUdpSocketBindAddress" /> option to
    ///         a valid, available bind address, and reinitialize the library.
    /// </para>
    /// </remarks>
    /// <param name="networkDescriptor">
    /// The descriptor associated with the network to which a connection will be established.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <param name="network">
    /// The optional, output network object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to connect to a new network began, or an error code
    /// otherwise. If this method fails, no related state changes will be generated. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyConnectToNetworkCompletedStateChange" />
    /// <seealso cref="PartyNetworkDestroyedStateChange" />
    /// <seealso cref="PartyNetworkConfiguration" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    /// <seealso cref="PartyManager::GetNetworks" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyNetwork::RemoveLocalUser" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    /// <seealso cref="PartyNetwork::RevokeInvitation" />
    /// <seealso cref="PartyNetwork::CreateEndpoint" />
    /// <seealso cref="PartyNetwork::LeaveNetwork" />
    PartyError ConnectToNetwork(
        const PartyNetworkDescriptor * networkDescriptor,
        _In_opt_ void * asyncIdentifier,
        _Outptr_opt_ PartyNetwork ** network
        ) party_no_throw;

    /// <summary>
    /// Queues an attempt to synchronize the sends to specified endpoints, and optionally all shared properties.
    /// </summary>
    /// <remarks>
    /// This method is currently unimplemented and will always fail.
    /// <para>
    /// This method is about synchronizing how the endpoints listed receive data. This method guarantees that listed
    /// endpoints will receive all data sent to them over their respective networks from the local device before
    /// receiving any data sent after this method was called.
    /// </para>
    /// </remarks>
    /// <param name="endpointCount">
    /// The count of endpoints, specified by <paramref name="endpoints" />, to which data sends should be synchronized.
    /// </param>
    /// <param name="endpoints">
    /// The list of endpoints to which data sends should be synchronized.
    /// </param>
    /// <param name="options">
    /// Additional message synchronization options.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the asynchronous operation to synchronize messages began, or an error code
    /// otherwise. The human-readable form of the error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <nyi />
    PartyError SynchronizeMessagesBetweenEndpoints(
        uint32_t endpointCount,
        _In_reads_(endpointCount) PartyEndpointArray endpoints,
        PartySynchronizeMessagesBetweenEndpointsOptions options,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets the local device.
    /// </summary>
    /// <remarks>
    /// The local device represents the physical device on which the title is executing, such as an Xbox One or PC. Once
    /// the library is initialized, this method will always succeed in returning a local device. The local device will
    /// also appear in the device list of any network to which the local device has connected via
    /// <see cref="ConnectToNetwork()" />.
    /// </remarks>
    /// <param name="localDevice">
    /// The output local device.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    PartyError GetLocalDevice(
        _Outptr_ PartyLocalDevice ** localDevice
        ) const party_no_throw;

    /// <summary>
    /// Creates a local user object that is used to represent a local user when performing networking and chat
    /// operations.
    /// </summary>
    /// <remarks>
    /// This method takes a PlayFab Entity handle as <paramref name="entityHandle" />. No synchronous validation is
    /// performed on this value. When the library performs operations that require user authentication or authorization,
    /// such as creating a network, authenticating into a network, or performing speech-to-text transcription, the Party
    /// service will validate that the token associated with this entityHandle is valid, is not expired, and is
    /// authorized to perform the operation. If these conditions aren't met, the operation will fail.
    /// <para>
    /// A PlayFab Entity handle can be obtained from the result of a PlayFab Core SDK login operation and then provided
    /// as input to this method.
    /// </para>
    /// <para>
    /// The provided <paramref name="entityHandle" /> must have been acquired using the same Title ID that was passed to
    /// <see cref="Initialize()" />.
    /// </para>
    /// <para>
    /// The Party library will automatically obtain a valid token from the PlayFab Core SDK library when authenticating
    /// the user for the Party service.
    /// </para>
    /// <para>
    /// Only <c>c_maxLocalUsersPerDeviceCount</c> PartyLocalUser objects may exist simultaneously at any given time.
    /// This method will synchronously fail if creating another local user would exceed that limit.
    /// </para>
    /// <para>
    /// On successful return, this method invalidates the memory for any array previously returned by
    /// <see cref="GetLocalUsers()" />, as it synchronously adds the new user to the array.
    /// <see cref="StartProcessingStateChanges()" /> also invalidates the memory for the array. The returned
    /// <paramref name="localUser" /> object will be valid until a
    /// <see cref="PartyDestroyLocalUserCompletedStateChange" /> has been generated and all state changes referencing
    /// the object have been returned to <see cref="FinishProcessingStateChanges()" />.
    /// </para>
    /// </remarks>
    /// <param name="entityHandle">
    /// The PlayFab Entity handle to associate with the local user.
    /// </param>
    /// <param name="localUser">
    /// The output local user object.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if creating the local user succeeded or an error code otherwise. The human-readable
    /// form of the error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDestroyLocalUserCompletedStateChange" />
    /// <seealso cref="PartyManager::Initialize" />
    /// <seealso cref="PartyManager::GetLocalUsers" />
    /// <seealso cref="PartyManager::DestroyLocalUser" />
    /// <seealso cref="PartyManager::CreateNewNetwork" />
    /// <seealso cref="PartyNetwork::AuthenticateLocalUser" />
    /// <seealso cref="PartyNetwork::RemoveLocalUser" />
    /// <seealso cref="PartyNetwork::CreateInvitation" />
    /// <seealso cref="PartyNetwork::RevokeInvitation" />
    /// <seealso cref="PartyNetwork::CreateEndpoint" />
    /// <seealso cref="PartyLocalUser::UpdateEntityToken" />
    /// <seealso cref="PartyLocalDevice::CreateChatControl" />
    /// <nyi />
    PartyError CreateLocalUser(
        PFEntityHandle entityHandle,
        _Outptr_ PartyLocalUser** localUser
        ) party_no_throw;

    /// <summary>
    /// Starts an asynchronous operation to destroy a local user.
    /// </summary>
    /// <remarks>
    /// This method queues an asynchronous operation to destroy a local user. A
    /// <see cref="PartyDestroyLocalUserCompletedStateChange" /> will be provided upon completion of the operation,
    /// indicating success or failure. Before successful completion of the operation, the local user will be removed
    /// from all networks it has authenticated into (each indicated by a
    /// <see cref="PartyLocalUserRemovedStateChange" />) and any local chat control associated with the user will be
    /// destroyed (indicated by a <see cref="PartyChatControlDestroyedStateChange" />). Memory for the local user will
    /// remain valid until all state changes referencing the local user have been returned to
    /// <see cref="PartyManager::FinishProcessingStateChanges()" />.
    /// </remarks>
    /// <param name="localUser">
    /// The local user to destroy.
    /// </param>
    /// <param name="asyncIdentifier">
    /// An optional, app-defined, pointer-sized context value that can be used to associate the completion state change
    /// with this call.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the operation to destroy the local user started or an error code otherwise. If
    /// this method fails, no related state changes will be generated. The human-readable form of the error code can be
    /// retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyDestroyLocalUserCompletedStateChange" />
    /// <seealso cref="PartyChatControlDestroyedStateChange" />
    /// <seealso cref="PartyLocalUserRemovedStateChange" />
    /// <seealso cref="PartyManager::GetLocalUsers" />
    PartyError DestroyLocalUser(
        const PartyLocalUser * localUser,
        _In_opt_ void * asyncIdentifier
        ) party_no_throw;

    /// <summary>
    /// Gets an array containing all local users created by <see cref="CreateLocalUser()" /> or
    /// <see cref="CreateLocalUserWithEntityType()" />.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyDestroyLocalUserCompletedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the local user will no longer be present in the
    /// array returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges(), or when CreateLocalUser() or CreateLocalUserWithEntityType()
    /// returns success.
    /// </para>
    /// </remarks>
    /// <param name="userCount">
    /// The output number of local users provided in <paramref name="localUsers" />.
    /// </param>
    /// <param name="localUsers">
    /// A library-allocated output array containing the local users.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::CreateLocalUser" />
    /// <seealso cref="PartyManager::CreateLocalUserWithEntityType" />
    /// <seealso cref="PartyManager::DestroyLocalUser" />
    /// <seealso cref="PartyDestroyLocalUserCompletedStateChange" />
    PartyError GetLocalUsers(
        _Out_ uint32_t * userCount,
        _Outptr_result_buffer_(*userCount) PartyLocalUserArray * localUsers
        ) const party_no_throw;

    /// <summary>
    /// Gets an array of all networks to which the local device is connected or connecting.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyNetworkDestroyedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the network will no longer be present in the array
    /// returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges() or <see cref="ConnectToNetwork()" /> returns success.
    /// </para>
    /// </remarks>
    /// <param name="networkCount">
    /// The output number of networks to which the local device is connected or connecting provided in
    /// <paramref name="networks" />.
    /// </param>
    /// <param name="networks">
    /// A library-allocated output array containing the networks to which the local device is connected or connecting.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyManager::ConnectToNetwork" />
    /// <seealso cref="PartyNetwork::LeaveNetwork" />
    /// <seealso cref="PartyNetworkDestroyedStateChange" />
    PartyError GetNetworks(
        _Out_ uint32_t * networkCount,
        _Outptr_result_buffer_(*networkCount) PartyNetworkArray * networks
        ) const party_no_throw;

    /// <summary>
    /// Gets an array that contains a combined list of all chat controls on the local device and all remote devices.
    /// </summary>
    /// <remarks>
    /// Once a <see cref="PartyChatControlDestroyedStateChange" /> has been provided by
    /// <see cref="PartyManager::StartProcessingStateChanges()" />, the chat control will no longer be present in the
    /// array returned by this method.
    /// <para>
    /// The memory for the returned array is invalidated whenever the title calls
    /// PartyManager::StartProcessingStateChanges() or <see cref="PartyLocalDevice::CreateChatControl()" /> returns
    /// success.
    /// </para>
    /// </remarks>
    /// <param name="chatControlCount">
    /// The output number of chat controls provided in <paramref name="chatControls" />.
    /// </param>
    /// <param name="chatControls">
    /// A library-allocated output array containing the list of all chat controls.
    /// </param>
    /// <returns>
    /// <c>c_partyErrorSuccess</c> if the call succeeded or an error code otherwise. The human-readable form of the
    /// error code can be retrieved via <see cref="GetErrorMessage()" />.
    /// </returns>
    /// <seealso cref="PartyLocalDevice::CreateChatControl" />
    /// <seealso cref="PartyLocalDevice::DestroyChatControl" />
    /// <seealso cref="PartyChatControlDestroyedStateChange" />
    PartyError GetChatControls(
        _Out_ uint32_t * chatControlCount,
        _Outptr_result_buffer_(*chatControlCount) PartyChatControlArray * chatControls
        ) const party_no_throw;

private:
    PartyManager();
    ~PartyManager();
    PartyManager(const PartyManager&) = delete;
    PartyManager(PartyManager&&) = delete;
    PartyManager& operator=(const PartyManager&) = delete;
    PartyManager& operator=(PartyManager&&) = delete;
    void * _handle;
};
}; // Party

#pragma pop_macro("DEFINE_ENUM_FLAG_OPERATORS")
