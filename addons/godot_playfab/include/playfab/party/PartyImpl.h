#pragma once

#include <Party_c.h>

#ifndef __cplusplus
#error "This header requires C++"
#endif // end if not defined __cplusplus

namespace Party {

#define PARTY_SAL_SUPPORT
#ifdef PARTY_SAL_SUPPORT
#pragma prefast(push)
#pragma prefast(disable:__WARNING_DANGEROUS_POINTERCAST, "All of the C++ structures are related to the corresponding C structures")
#endif // PARTY_SAL_SUPPORT

//
// PartyLocalUser class implementation
//

PartyError PartyLocalUser::GetEntityId(
    _Outptr_ PartyString * entityId
    ) const party_no_throw
{
    return PartyLocalUserGetEntityId(
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(this),
        entityId);
}

PartyError PartyLocalUser::GetEntityType(
    _Outptr_ PartyString * entityType
    ) const party_no_throw
{
    return PartyLocalUserGetEntityType(
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(this),
        entityType);
}

PartyError PartyLocalUser::UpdateEntityToken(
    PartyString entityToken
    ) party_no_throw
{
    return PartyLocalUserUpdateEntityToken(
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(this),
        entityToken);
}

PartyError PartyLocalUser::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyLocalUserGetCustomContext(
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(this),
        customContext);
}

PartyError PartyLocalUser::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyLocalUserSetCustomContext(
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(this),
        customContext);
}

//
// PartyLocalEndpoint class implementation
//

PartyError PartyLocalEndpoint::GetLocalUser(
    _Outptr_result_maybenull_ PartyLocalUser ** localUser
    ) const party_no_throw
{
    return PartyEndpointGetLocalUser(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        const_cast<PARTY_LOCAL_USER_HANDLE *>(reinterpret_cast<const PARTY_LOCAL_USER_HANDLE *>(localUser)));
}

PartyError PartyLocalEndpoint::SendMessage(
    uint32_t targetEndpointCount,
    _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints,
    PartySendMessageOptions options,
    _In_opt_ const PartySendMessageQueuingConfiguration * queuingConfiguration,
    uint32_t dataBufferCount,
    _In_reads_(dataBufferCount) const PartyDataBuffer * dataBuffers,
    _In_opt_ void * messageIdentifier
    ) party_no_throw
{
    return PartyEndpointSendMessage(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        targetEndpointCount,
        reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(targetEndpoints),
        static_cast<PARTY_SEND_MESSAGE_OPTIONS>(options),
        reinterpret_cast<const PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION *>(queuingConfiguration),
        dataBufferCount,
        reinterpret_cast<const PARTY_DATA_BUFFER *>(dataBuffers),
        messageIdentifier);
}

PartyError PartyLocalEndpoint::CancelMessages(
    uint32_t targetEndpointCount,
    _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints,
    PartyCancelMessagesFilterExpression filterExpression,
    uint32_t messageIdentityFilterMask,
    uint32_t filteredMessageIdentitiesToMatch,
    _Out_opt_ uint32_t * canceledMessagesCount
    ) party_no_throw
{
    return PartyEndpointCancelMessages(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        targetEndpointCount,
        reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(targetEndpoints),
        static_cast<PARTY_CANCEL_MESSAGES_FILTER_EXPRESSION>(filterExpression),
        messageIdentityFilterMask,
        filteredMessageIdentitiesToMatch,
        canceledMessagesCount);
}

PartyError PartyLocalEndpoint::FlushMessages(
    uint32_t targetEndpointCount,
    _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints
    ) party_no_throw
{
    return PartyEndpointFlushMessages(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        targetEndpointCount,
        reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(targetEndpoints));
}

PartyError PartyLocalEndpoint::GetEndpointStatistics(
    uint32_t targetEndpointCount,
    _In_reads_(targetEndpointCount) PartyEndpointArray targetEndpoints,
    uint32_t statisticCount,
    _In_reads_(statisticCount) const PartyEndpointStatistic * statisticTypes,
    _Out_writes_all_(statisticCount) uint64_t * statisticValues
    ) const party_no_throw
{
    return PartyEndpointGetEndpointStatistics(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        targetEndpointCount,
        reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(targetEndpoints),
        statisticCount,
        reinterpret_cast<const PARTY_ENDPOINT_STATISTIC *>(statisticTypes),
        statisticValues);
}

PartyError PartyLocalEndpoint::SetSharedProperties(
    uint32_t propertyCount,
    _In_reads_(propertyCount) const PartyString * keys,
    _In_reads_(propertyCount) const PartyDataBuffer * values
    ) party_no_throw
{
    return PartyEndpointSetSharedProperties(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        propertyCount,
        keys,
        reinterpret_cast<const PARTY_DATA_BUFFER*>(values));
}

//
// PartyEndpoint class implementation
//

PartyError PartyEndpoint::GetLocal(
    _Outptr_result_maybenull_ PartyLocalEndpoint ** localEndpoint
    ) const party_no_throw
{
    PartyError error;
    PartyBool isLocal;

    error = PartyEndpointIsLocal(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        &isLocal);
    if (PARTY_SUCCEEDED(error))
    {
        if (isLocal != 0)
        {
            *localEndpoint = const_cast<PartyLocalEndpoint *>(static_cast<const PartyLocalEndpoint *>(this));
        }
        else
        {
            *localEndpoint = nullptr;
        }
    }

    return error;
}

PartyError PartyEndpoint::GetEntityId(
    _Outptr_result_maybenull_ PartyString * entityId
    ) const party_no_throw
{
    return PartyEndpointGetEntityId(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        entityId);
}

PartyError PartyEndpoint::GetEntityType(
    _Outptr_result_maybenull_ PartyString * entityType
    ) const party_no_throw
{
    return PartyEndpointGetEntityType(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        entityType);
}

PartyError PartyEndpoint::GetSharedProperty(
    PartyString key,
    _Out_ PartyDataBuffer * value
    ) const party_no_throw
{
    return PartyEndpointGetSharedProperty(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        key,
        reinterpret_cast<PARTY_DATA_BUFFER*>(value));
}

PartyError PartyEndpoint::GetSharedPropertyKeys(
    _Out_ uint32_t * propertyCount,
    _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
    ) const party_no_throw
{
    return PartyEndpointGetSharedPropertyKeys(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        propertyCount,
        keys);
}

PartyError PartyEndpoint::GetNetwork(
    _Outptr_ PartyNetwork ** network
    ) const party_no_throw
{
    return PartyEndpointGetNetwork(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        const_cast<PARTY_NETWORK_HANDLE *>(reinterpret_cast<const PARTY_NETWORK_HANDLE *>(network)));
}

PartyError PartyEndpoint::GetDevice(
    _Outptr_ PartyDevice ** device
    ) const party_no_throw
{
    return PartyEndpointGetDevice(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        const_cast<PARTY_DEVICE_HANDLE *>(reinterpret_cast<const PARTY_DEVICE_HANDLE *>(device)));
}

PartyError PartyEndpoint::GetUniqueIdentifier(
    _Out_ uint16_t * uniqueIdentifier
    ) const party_no_throw
{
    return PartyEndpointGetUniqueIdentifier(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        uniqueIdentifier);
}

PartyError PartyEndpoint::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyEndpointGetCustomContext(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        customContext);
}

PartyError PartyEndpoint::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyEndpointSetCustomContext(
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(this),
        customContext);
}

//
// PartyLocalDevice class implementation
//

PartyError PartyLocalDevice::CreateChatControl(
    const PartyLocalUser * localUser,
    _In_opt_ PartyString languageCode,
    _In_opt_ void * asyncIdentifier,
    _Outptr_opt_ PartyLocalChatControl ** localChatControl
    ) party_no_throw
{
    return PartyDeviceCreateChatControl(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(localUser),
        languageCode,
        asyncIdentifier,
        const_cast<PARTY_CHAT_CONTROL_HANDLE *>(reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE *>(localChatControl)));
}

PartyError PartyLocalDevice::DestroyChatControl(
    _In_ PartyLocalChatControl * localChatControl,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyDeviceDestroyChatControl(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(localChatControl),
        asyncIdentifier);
}

PartyError PartyLocalDevice::SetSharedProperties(
    uint32_t propertyCount,
    _In_reads_(propertyCount) const PartyString * keys,
    _In_reads_(propertyCount) const PartyDataBuffer * values
    ) party_no_throw
{
    return PartyDeviceSetSharedProperties(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        propertyCount,
        keys,
        reinterpret_cast<const PARTY_DATA_BUFFER*>(values));
}

//
// PartyDevice class implementation
//

PartyError PartyDevice::GetLocal(
    _Outptr_result_maybenull_ PartyLocalDevice ** localDevice
    ) const party_no_throw
{
    PartyError error;
    PartyBool isLocal;

    error = PartyDeviceIsLocal(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        &isLocal);
    if (PARTY_SUCCEEDED(error))
    {
        if (isLocal != 0)
        {
            *localDevice = const_cast<PartyLocalDevice *>(static_cast<const PartyLocalDevice *>(this));
        }
        else
        {
            *localDevice = nullptr;
        }
    }

    return error;
}

PartyError PartyDevice::GetChatControls(
    _Out_ uint32_t * chatControlCount,
    _Outptr_result_buffer_(*chatControlCount) PartyChatControlArray * chatControls
    ) const party_no_throw
{
    return PartyDeviceGetChatControls(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        chatControlCount,
        const_cast<const PARTY_CHAT_CONTROL_HANDLE **>(reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE *const*>(chatControls)));
}

PartyError PartyDevice::GetSharedProperty(
    PartyString key,
    _Out_ PartyDataBuffer * value
    ) const party_no_throw
{
    return PartyDeviceGetSharedProperty(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        key,
        reinterpret_cast<PARTY_DATA_BUFFER*>(value));
}

PartyError PartyDevice::GetSharedPropertyKeys(
    _Out_ uint32_t * propertyCount,
    _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
    ) const party_no_throw
{
    return PartyDeviceGetSharedPropertyKeys(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        propertyCount,
        keys);
}

PartyError PartyDevice::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyDeviceGetCustomContext(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        customContext);
}

PartyError PartyDevice::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyDeviceSetCustomContext(
        reinterpret_cast<PARTY_DEVICE_HANDLE>(this),
        customContext);
}

//
// PartyInvitation class implementation
//

PartyError PartyInvitation::GetCreatorEntityId(
    _Outptr_result_maybenull_ PartyString * entityId
    ) const party_no_throw
{
    return PartyInvitationGetCreatorEntityId(
        reinterpret_cast<PARTY_INVITATION_HANDLE>(this),
        entityId);
}

PartyError PartyInvitation::GetInvitationConfiguration(
    _Outptr_ const PartyInvitationConfiguration ** configuration
    ) const party_no_throw
{
    return PartyInvitationGetInvitationConfiguration(
        reinterpret_cast<PARTY_INVITATION_HANDLE>(this),
        reinterpret_cast<const PARTY_INVITATION_CONFIGURATION **>(configuration));
}

PartyError PartyInvitation::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyInvitationGetCustomContext(
        reinterpret_cast<PARTY_INVITATION_HANDLE>(this),
        customContext);
}

PartyError PartyInvitation::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyInvitationSetCustomContext(
        reinterpret_cast<PARTY_INVITATION_HANDLE>(this),
        customContext);
}

//
// PartyNetwork class implementation
//

PartyError PartyNetwork::AuthenticateLocalUser(
    const PartyLocalUser * localUser,
    PartyString invitationIdentifier,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkAuthenticateLocalUser(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(localUser),
        invitationIdentifier,
        asyncIdentifier);
}

PartyError PartyNetwork::RemoveLocalUser(
    const PartyLocalUser * localUser,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkRemoveLocalUser(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(localUser),
        asyncIdentifier);
}

PartyError PartyNetwork::CreateInvitation(
    const PartyLocalUser * localUser,
    _In_opt_ const PartyInvitationConfiguration * invitationConfiguration,
    void* asyncIdentifier,
    _Outptr_opt_ PartyInvitation ** invitation
    ) party_no_throw
{
    return PartyNetworkCreateInvitation(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_LOCAL_USER_HANDLE>(localUser),
        reinterpret_cast<const PARTY_INVITATION_CONFIGURATION *>(invitationConfiguration),
        asyncIdentifier,
        const_cast<PARTY_INVITATION_HANDLE *>(reinterpret_cast<const PARTY_INVITATION_HANDLE *>(invitation)));
}

PartyError PartyNetwork::RevokeInvitation(
    const PartyLocalUser * localUser,
    PartyInvitation * invitation,
    void* asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkRevokeInvitation(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<const PARTY_LOCAL_USER_HANDLE>(localUser),
        reinterpret_cast<const PARTY_INVITATION_HANDLE>(invitation),
        asyncIdentifier);
}

PartyError PartyNetwork::GetInvitations(
    _Out_ uint32_t* invitationCount,
    _Outptr_result_buffer_(*invitationCount) PartyInvitationArray * invitations
    ) const party_no_throw
{
    return PartyNetworkGetInvitations(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        invitationCount,
        const_cast<const PARTY_INVITATION_HANDLE **>(reinterpret_cast<const PARTY_INVITATION_HANDLE *const*>(invitations)));
}

PartyError PartyNetwork::CreateEndpoint(
    _In_opt_ const PartyLocalUser * localUser,
    uint32_t sharedPropertyCount,
    _In_reads_opt_(sharedPropertyCount) const PartyString * keys,
    _In_reads_opt_(sharedPropertyCount) const PartyDataBuffer * values,
    _In_opt_ void * asyncIdentifier,
    _Outptr_opt_ PartyLocalEndpoint ** localEndpoint
    ) party_no_throw
{
    return PartyNetworkCreateEndpoint(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<const PARTY_LOCAL_USER_HANDLE>(localUser),
        sharedPropertyCount,
        keys,
        reinterpret_cast<const PARTY_DATA_BUFFER*>(values),
        asyncIdentifier,
        const_cast<PARTY_ENDPOINT_HANDLE *>(reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(localEndpoint)));
}

PartyError PartyNetwork::DestroyEndpoint(
    _In_ PartyLocalEndpoint * localEndpoint,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkDestroyEndpoint(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_ENDPOINT_HANDLE>(localEndpoint),
        asyncIdentifier);
}

PartyError PartyNetwork::LeaveNetwork(
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkLeaveNetwork(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        asyncIdentifier);
}

PartyError PartyNetwork::GetEndpoints(
    _Out_ uint32_t * endpointCount,
    _Outptr_result_buffer_(*endpointCount) PartyEndpointArray * endpoints
    ) const party_no_throw
{
    return PartyNetworkGetEndpoints(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        endpointCount,
        const_cast<const PARTY_ENDPOINT_HANDLE **>(reinterpret_cast<const PARTY_ENDPOINT_HANDLE *const*>(endpoints)));
}

PartyError PartyNetwork::GetEndpointsByUserType(
    PartyEndpointUserTypeFilter endpointUserTypeFilter,
    PartyEndpointLocationFilter endpointLocationFilter,
    _Out_ uint32_t * endpointCount,
    _Outptr_result_buffer_(*endpointCount) PartyEndpointArray * endpoints
    ) const party_no_throw
{
    return PartyNetworkGetEndpointsByUserType(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        static_cast<PARTY_ENDPOINT_USER_TYPE_FILTER>(endpointUserTypeFilter),
        static_cast<PARTY_ENDPOINT_LOCATION_FILTER>(endpointLocationFilter),
        endpointCount,
        const_cast<const PARTY_ENDPOINT_HANDLE **>(reinterpret_cast<const PARTY_ENDPOINT_HANDLE *const*>(endpoints)));
}

PartyError PartyNetwork::FindEndpointByUniqueIdentifier(
    uint16_t uniqueIdentifier,
    _Outptr_ PartyEndpoint** endpoint
    ) const party_no_throw
{
    return PartyNetworkFindEndpointByUniqueIdentifier(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        uniqueIdentifier,
        const_cast<PARTY_ENDPOINT_HANDLE *>(reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(endpoint)));
}

PartyError PartyNetwork::GetDevices(
    _Out_ uint32_t * deviceCount,
    _Outptr_result_buffer_(*deviceCount) PartyDeviceArray * devices
    ) const party_no_throw
{
    return PartyNetworkGetDevices(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        deviceCount,
        const_cast<const PARTY_DEVICE_HANDLE **>(reinterpret_cast<const PARTY_DEVICE_HANDLE *const*>(devices)));
}

PartyError PartyNetwork::GetLocalUsers(
    _Out_ uint32_t * userCount,
    _Outptr_result_buffer_(*userCount) PartyLocalUserArray * users
    ) const party_no_throw
{
    return PartyNetworkGetLocalUsers(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        userCount,
        const_cast<const PARTY_LOCAL_USER_HANDLE **>(reinterpret_cast<const PARTY_LOCAL_USER_HANDLE *const*>(users)));
}

PartyError PartyNetwork::GetNetworkDescriptor(
    _Out_ PartyNetworkDescriptor * networkDescriptor
    ) const party_no_throw
{
    return PartyNetworkGetNetworkDescriptor(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_NETWORK_DESCRIPTOR *>(networkDescriptor));
}

PartyError PartyNetwork::GetNetworkConfiguration(
    _Outptr_ const PartyNetworkConfiguration ** networkConfiguration
    ) const party_no_throw
{
    return PartyNetworkGetNetworkConfiguration(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<const PARTY_NETWORK_CONFIGURATION **>(networkConfiguration));
}

PartyError PartyNetwork::KickDevice(
    const PartyDevice * targetDevice,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkKickDevice(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_DEVICE_HANDLE>(targetDevice),
        asyncIdentifier);
}

PartyError PartyNetwork::KickUser(
    PartyString targetEntityId,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkKickUser(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        targetEntityId,
        asyncIdentifier);
}

PartyError PartyNetwork::GetSharedProperty(
    PartyString key,
    _Out_ PartyDataBuffer * value
    ) const party_no_throw
{
    return PartyNetworkGetSharedProperty(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        key,
        reinterpret_cast<PARTY_DATA_BUFFER*>(value));
}

PartyError PartyNetwork::GetSharedPropertyKeys(
    _Out_ uint32_t * propertyCount,
    _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
    ) const party_no_throw
{
    return PartyNetworkGetSharedPropertyKeys(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        propertyCount,
        keys);
}

PartyError PartyNetwork::SetSharedProperties(
    uint32_t propertyCount,
    _In_reads_(propertyCount) const PartyString * keys,
    _In_reads_(propertyCount) const PartyDataBuffer * values
    ) party_no_throw
{
    return PartyNetworkSetSharedProperties(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        propertyCount,
        keys,
        reinterpret_cast<const PARTY_DATA_BUFFER*>(values));
}

PartyError PartyNetwork::ConnectChatControl(
    _In_ PartyLocalChatControl* chatControl,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkConnectChatControl(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(chatControl),
        asyncIdentifier);
}

PartyError PartyNetwork::DisconnectChatControl(
    _In_ PartyLocalChatControl* chatControl,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyNetworkDisconnectChatControl(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(chatControl),
        asyncIdentifier);
}

PartyError PartyNetwork::GetChatControls(
    _Out_ uint32_t * chatControlCount,
    _Outptr_result_buffer_(*chatControlCount) PartyChatControlArray * chatControls
    ) const party_no_throw
{
    return PartyNetworkGetChatControls(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        chatControlCount,
        const_cast<const PARTY_CHAT_CONTROL_HANDLE **>(reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE *const*>(chatControls)));
}

PartyError PartyNetwork::GetNetworkStatistics(
    uint32_t statisticCount,
    _In_reads_(statisticCount) const PartyNetworkStatistic * statisticTypes,
    _Out_writes_all_(statisticCount) uint64_t * statisticValues
    ) const party_no_throw
{
    return PartyNetworkGetNetworkStatistics(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        statisticCount,
        reinterpret_cast<const PARTY_NETWORK_STATISTIC *>(statisticTypes),
        statisticValues);
}

PartyError PartyNetwork::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyNetworkGetCustomContext(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        customContext);
}

PartyError PartyNetwork::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyNetworkSetCustomContext(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        customContext);
}

PartyError PartyNetwork::GetDeviceConnectionType(
    const PartyDevice* targetDevice,
    _Out_ PartyDeviceConnectionType * deviceConnectionType
    ) const party_no_throw
{
    return PartyNetworkGetDeviceConnectionType(
        reinterpret_cast<PARTY_NETWORK_HANDLE>(this),
        reinterpret_cast<PARTY_DEVICE_HANDLE>(targetDevice),
        reinterpret_cast<PARTY_DEVICE_CONNECTION_TYPE*>(deviceConnectionType));
}

//
// PartyLocalChatControl class implementation
//

PartyError PartyLocalChatControl::GetLocalUser(
    _Outptr_ PartyLocalUser ** localUser
    ) const party_no_throw
{
    return PartyChatControlGetLocalUser(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        const_cast<PARTY_LOCAL_USER_HANDLE *>(reinterpret_cast<const PARTY_LOCAL_USER_HANDLE *>(localUser)));
}

PartyError PartyLocalChatControl::SetPermissions(
    const PartyChatControl * targetChatControl,
    PartyChatPermissionOptions chatPermissionOptions
    ) party_no_throw
{
    return PartyChatControlSetPermissions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        static_cast<PARTY_CHAT_PERMISSION_OPTIONS>(chatPermissionOptions));
}

PartyError PartyLocalChatControl::GetPermissions(
    const PartyChatControl * targetChatControl,
    _Out_ PartyChatPermissionOptions * chatPermissionOptions
    ) const party_no_throw
{
    return PartyChatControlGetPermissions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        reinterpret_cast<PARTY_CHAT_PERMISSION_OPTIONS*>(chatPermissionOptions));
}

PartyError PartyLocalChatControl::SendText(
    uint32_t targetChatControlCount,
    _In_reads_(targetChatControlCount) PartyChatControlArray targetChatControls,
    PartyString chatText,
    uint32_t dataBufferCount,
    _In_reads_(dataBufferCount) const PartyDataBuffer * dataBuffers
    ) party_no_throw
{
    return PartyChatControlSendText(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        targetChatControlCount,
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE *>(targetChatControls),
        chatText,
        dataBufferCount,
        reinterpret_cast<const PARTY_DATA_BUFFER *>(dataBuffers));
}

PartyError PartyLocalChatControl::SetAudioInput(
    PartyAudioDeviceSelectionType audioDeviceSelectionType,
    _In_opt_ PartyString audioDeviceSelectionContext,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetAudioInput(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_AUDIO_DEVICE_SELECTION_TYPE>(audioDeviceSelectionType),
        audioDeviceSelectionContext,
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetAudioInput(
    _Out_ PartyAudioDeviceSelectionType * audioDeviceSelectionType,
    _Outptr_ PartyString * audioDeviceSelectionContext,
    _Outptr_ PartyString * deviceId
    ) const party_no_throw
{
    return PartyChatControlGetAudioInput(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_DEVICE_SELECTION_TYPE*>(audioDeviceSelectionType),
        audioDeviceSelectionContext,
        deviceId);
}

PartyError PartyLocalChatControl::SetAudioOutput(
    PartyAudioDeviceSelectionType audioDeviceSelectionType,
    _In_opt_ PartyString audioDeviceSelectionContext,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetAudioOutput(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_AUDIO_DEVICE_SELECTION_TYPE>(audioDeviceSelectionType),
        audioDeviceSelectionContext,
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetAudioOutput(
    _Out_ PartyAudioDeviceSelectionType * audioDeviceSelectionType,
    _Outptr_ PartyString * audioDeviceSelectionContext,
    _Outptr_ PartyString * deviceId
    ) const party_no_throw
{
    return PartyChatControlGetAudioOutput(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_DEVICE_SELECTION_TYPE*>(audioDeviceSelectionType),
        audioDeviceSelectionContext,
        deviceId);
}

PartyError PartyLocalChatControl::PopulateAvailableTextToSpeechProfiles(
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlPopulateAvailableTextToSpeechProfiles(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetAvailableTextToSpeechProfiles(
    _Out_ uint32_t * profileCount,
    _Outptr_result_buffer_(*profileCount) PartyTextToSpeechProfileArray * profiles
    ) const party_no_throw
{
    return PartyChatControlGetAvailableTextToSpeechProfiles(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        profileCount,
        const_cast<const PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE**>(reinterpret_cast<const PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE *const*>(profiles)));
}

PartyError PartyLocalChatControl::SetTextToSpeechProfile(
    PartySynthesizeTextToSpeechType type,
    PartyString profile,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetTextToSpeechProfile(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_SYNTHESIZE_TEXT_TO_SPEECH_TYPE>(type),
        profile,
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetTextToSpeechProfile(
    PartySynthesizeTextToSpeechType type,
    _Outptr_result_maybenull_ PartyTextToSpeechProfile ** profile
    ) const party_no_throw
{
    return PartyChatControlGetTextToSpeechProfile(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_SYNTHESIZE_TEXT_TO_SPEECH_TYPE>(type),
        const_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE *>(reinterpret_cast<const PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE*>(profile)));
}

PartyError PartyLocalChatControl::SynthesizeTextToSpeech(
    PartySynthesizeTextToSpeechType type,
    PartyString textToSynthesize,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSynthesizeTextToSpeech(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_SYNTHESIZE_TEXT_TO_SPEECH_TYPE>(type),
        textToSynthesize,
        asyncIdentifier);
}

PartyError PartyLocalChatControl::SetLanguage(
    _In_opt_ PartyString languageCode,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetLanguage(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        languageCode,
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetLanguage(
    _Outptr_ PartyString * languageCode
    ) const party_no_throw
{
    return PartyChatControlGetLanguage(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        languageCode);
}

PartyError PartyLocalChatControl::SetTranscriptionOptions(
    PartyVoiceChatTranscriptionOptions options,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetTranscriptionOptions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS>(options),
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetTranscriptionOptions(
    _Out_ PartyVoiceChatTranscriptionOptions * options
    ) const party_no_throw
{
    return PartyChatControlGetTranscriptionOptions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS*>(options));
}

PartyError PartyLocalChatControl::SetTextChatOptions(
    PartyTextChatOptions options,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetTextChatOptions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_TEXT_CHAT_OPTIONS>(options),
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetTextChatOptions(
    _Out_ PartyTextChatOptions * options
    ) const party_no_throw
{
    return PartyChatControlGetTextChatOptions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_TEXT_CHAT_OPTIONS*>(options));
}


PartyError PartyLocalChatControl::SetAudioRenderVolume(
    const PartyChatControl * targetChatControl,
    float volume
    ) party_no_throw
{
    return PartyChatControlSetAudioRenderVolume(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        volume);
}

PartyError PartyLocalChatControl::GetAudioRenderVolume(
    const PartyChatControl * targetChatControl,
    _Out_ float * volume
    ) const party_no_throw
{
    return PartyChatControlGetAudioRenderVolume(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        volume);
}

PartyError PartyLocalChatControl::SetAudioInputMuted(
    PartyBool muted
    ) party_no_throw
{
    return PartyChatControlSetAudioInputMuted(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        muted);
}

PartyError PartyLocalChatControl::GetAudioInputMuted(
    _Out_ PartyBool * muted
    ) const party_no_throw
{
    return PartyChatControlGetAudioInputMuted(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        muted);
}

PartyError PartyLocalChatControl::SetIncomingAudioMuted(
    const PartyChatControl * targetChatControl,
    PartyBool muted
    ) party_no_throw
{
    return PartyChatControlSetIncomingAudioMuted(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        muted);
}

PartyError PartyLocalChatControl::GetIncomingAudioMuted(
    const PartyChatControl * targetChatControl,
    _Out_ PartyBool * muted
    ) const party_no_throw
{
    return PartyChatControlGetIncomingAudioMuted(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        muted);
}

PartyError PartyLocalChatControl::SetIncomingTextMuted(
    const PartyChatControl * targetChatControl,
    PartyBool muted
    ) party_no_throw
{
    return PartyChatControlSetIncomingTextMuted(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        muted);
}

PartyError PartyLocalChatControl::GetIncomingTextMuted(
    const PartyChatControl * targetChatControl,
    _Out_ PartyBool * muted
    ) const party_no_throw
{
    return PartyChatControlGetIncomingTextMuted(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        muted);
}

PartyError PartyLocalChatControl::SetVoiceAudioOptions(
    PartyVoiceAudioOptions options
    ) party_no_throw
{
    return PartyChatControlSetVoiceAudioOptions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        static_cast<PARTY_VOICE_AUDIO_OPTIONS>(options));
}

PartyError PartyLocalChatControl::GetVoiceAudioOptions(
    _Out_ PartyVoiceAudioOptions* options
    ) const party_no_throw
{
    return PartyChatControlGetVoiceAudioOptions(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_VOICE_AUDIO_OPTIONS*>(options));
}

PartyError PartyLocalChatControl::GetLocalChatIndicator(
    _Out_ PartyLocalChatControlChatIndicator * chatIndicator
    ) const party_no_throw
{
    return PartyChatControlGetLocalChatIndicator(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_LOCAL_CHAT_CONTROL_CHAT_INDICATOR *>(chatIndicator));
}

PartyError PartyLocalChatControl::GetChatIndicator(
    const PartyChatControl * targetChatControl,
    _Out_ PartyChatControlChatIndicator * chatIndicator
    ) const party_no_throw
{
    return PartyChatControlGetChatIndicator(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE>(targetChatControl),
        reinterpret_cast<PARTY_CHAT_CONTROL_CHAT_INDICATOR *>(chatIndicator));
}

PartyError PartyLocalChatControl::SetAudioEncoderBitrate(
    uint32_t bitrate,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlSetAudioEncoderBitrate(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        bitrate,
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetAudioEncoderBitrate(
    _Out_ uint32_t * bitrate
    ) const party_no_throw
{
    return PartyChatControlGetAudioEncoderBitrate(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        bitrate);
}

PartyError PartyLocalChatControl::SetSharedProperties(
    uint32_t propertyCount,
    _In_reads_(propertyCount) const PartyString * keys,
    _In_reads_(propertyCount) const PartyDataBuffer * values
    ) party_no_throw
{
    return PartyChatControlSetSharedProperties(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        propertyCount,
        keys,
        reinterpret_cast<const PARTY_DATA_BUFFER*>(values));
}

PartyError PartyLocalChatControl::ConfigureAudioManipulationCaptureStream(
    _In_opt_ PartyAudioManipulationSinkStreamConfiguration * configuration,
    _In_opt_ void* asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlConfigureAudioManipulationCaptureStream(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION*>(configuration),
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetAudioManipulationCaptureStream(
    _Outptr_ PartyAudioManipulationSinkStream** stream
    ) const party_no_throw
{
    return PartyChatControlGetAudioManipulationCaptureStream(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        const_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE*>(reinterpret_cast<const PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE*>(stream)));
}

PartyError PartyLocalChatControl::ConfigureAudioManipulationRenderStream(
    _In_opt_ PartyAudioManipulationSinkStreamConfiguration * configuration,
    _In_opt_ void* asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlConfigureAudioManipulationRenderStream(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION*>(configuration),
        asyncIdentifier);
}

PartyError PartyLocalChatControl::GetAudioManipulationRenderStream(
    _Outptr_ PartyAudioManipulationSinkStream** stream
    ) const party_no_throw
{
    return PartyChatControlGetAudioManipulationRenderStream(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        const_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE*>(reinterpret_cast<const PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE*>(stream)));
}

//
// PartyChatControl class implementation
//

PartyError PartyChatControl::GetLocal(
    _Outptr_result_maybenull_ PartyLocalChatControl ** localChatControl
    ) const party_no_throw
{
    PartyError error;
    PartyBool isLocal;

    error = PartyChatControlIsLocal(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        &isLocal);
    if (PARTY_SUCCEEDED(error))
    {
        if (isLocal != 0)
        {
            *localChatControl = const_cast<PartyLocalChatControl *>(static_cast<const PartyLocalChatControl *>(this));
        }
        else
        {
            *localChatControl = nullptr;
        }
    }

    return error;
}

PartyError PartyChatControl::GetDevice(
    _Outptr_ PartyDevice ** device
    ) const party_no_throw
{
    return PartyChatControlGetDevice(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        const_cast<PARTY_DEVICE_HANDLE *>(reinterpret_cast<const PARTY_DEVICE_HANDLE *>(device)));
}

PartyError PartyChatControl::GetEntityId(
    _Outptr_ PartyString * entityId
    ) const party_no_throw
{
    return PartyChatControlGetEntityId(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        entityId);
}

PartyError PartyChatControl::GetEntityType(
    _Outptr_ PartyString * entityType
    ) const party_no_throw
{
    return PartyChatControlGetEntityType(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        entityType);
}

PartyError PartyChatControl::GetSharedProperty(
    PartyString key,
    _Out_ PartyDataBuffer * value
    ) const party_no_throw
{
    return PartyChatControlGetSharedProperty(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        key,
        reinterpret_cast<PARTY_DATA_BUFFER*>(value));
}

PartyError PartyChatControl::GetSharedPropertyKeys(
    _Out_ uint32_t * propertyCount,
    _Outptr_result_buffer_(*propertyCount) const PartyString ** keys
    ) const party_no_throw
{
    return PartyChatControlGetSharedPropertyKeys(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        propertyCount,
        keys);
}

PartyError PartyChatControl::GetNetworks(
    _Out_ uint32_t * networkCount,
    _Outptr_result_buffer_(*networkCount) PartyNetworkArray * networks
    ) const party_no_throw
{
    return PartyChatControlGetNetworks(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        networkCount,
        const_cast<const PARTY_NETWORK_HANDLE **>(reinterpret_cast<const PARTY_NETWORK_HANDLE *const*>(networks)));
}

PartyError PartyChatControl::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyChatControlSetCustomContext(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        customContext);
}

PartyError PartyChatControl::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyChatControlGetCustomContext(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        customContext);
}

PartyError PartyChatControl::ConfigureAudioManipulationVoiceStream(
    _In_opt_ PartyAudioManipulationSourceStreamConfiguration * configuration,
    _In_opt_ void* asyncIdentifier
    ) party_no_throw
{
    return PartyChatControlConfigureAudioManipulationVoiceStream(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION*>(configuration),
        asyncIdentifier);
}

PartyError PartyChatControl::GetAudioManipulationVoiceStream(
    _Outptr_ PartyAudioManipulationSourceStream** sourceStream
    ) const party_no_throw
{
    return PartyChatControlGetAudioManipulationVoiceStream(
        reinterpret_cast<PARTY_CHAT_CONTROL_HANDLE>(this),
        const_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE*>(reinterpret_cast<const PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE*>(sourceStream)));
}

//
// PartyTextToSpeechProfile class implementation
//

PartyError PartyTextToSpeechProfile::GetIdentifier(
    _Outptr_ PartyString * identifier
    ) const party_no_throw
{
    return PartyTextToSpeechProfileGetIdentifier(
        reinterpret_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE>(this),
        identifier);
}

PartyError PartyTextToSpeechProfile::GetName(
    _Outptr_ PartyString * name
    ) const party_no_throw
{
    return PartyTextToSpeechProfileGetName(
        reinterpret_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE>(this),
        name);
}

PartyError PartyTextToSpeechProfile::GetLanguageCode(
    _Outptr_ PartyString * languageCode
    ) const party_no_throw
{
    return PartyTextToSpeechProfileGetLanguageCode(
        reinterpret_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE>(this),
        languageCode);
}

PartyError PartyTextToSpeechProfile::GetGender(
    _Out_ PartyGender * gender
    ) const party_no_throw
{
    return PartyTextToSpeechProfileGetGender(
        reinterpret_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE>(this),
        reinterpret_cast<PARTY_GENDER *>(gender));
}

PartyError PartyTextToSpeechProfile::GetCustomContext(
    _Outptr_result_maybenull_ void ** customContext
    ) const party_no_throw
{
    return PartyTextToSpeechProfileGetCustomContext(
        reinterpret_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE>(this),
        customContext);
}

PartyError PartyTextToSpeechProfile::SetCustomContext(
    _In_opt_ void * customContext
    ) party_no_throw
{
    return PartyTextToSpeechProfileSetCustomContext(
        reinterpret_cast<PARTY_TEXT_TO_SPEECH_PROFILE_HANDLE>(this),
        customContext);
}

//
// PartyAudioManipulationSourceStream class implementation
//

PartyError PartyAudioManipulationSourceStream::GetConfiguration(
    _Out_ PartyAudioManipulationSourceStreamConfiguration* configuration
    ) const party_no_throw
{
    return PartyAudioManipulationSourceStreamGetConfiguration(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION*>(configuration));
}

PartyError PartyAudioManipulationSourceStream::GetFormat(
    _Out_ PartyAudioFormat* format
    ) const party_no_throw
{
    return PartyAudioManipulationSourceStreamGetFormat(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_FORMAT*>(format));
}

PartyError PartyAudioManipulationSourceStream::GetAvailableBufferCount(
    _Out_ uint32_t* count
    ) const party_no_throw
{
    return PartyAudioManipulationSourceStreamGetAvailableBufferCount(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        count);
}

PartyError PartyAudioManipulationSourceStream::GetNextBuffer(
    _Out_ PartyMutableDataBuffer* buffer
    ) party_no_throw
{
    return PartyAudioManipulationSourceStreamGetNextBuffer(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        reinterpret_cast<PARTY_MUTABLE_DATA_BUFFER*>(buffer));
}

PartyError PartyAudioManipulationSourceStream::ReturnBuffer(
    _Post_invalid_ void * buffer
    ) party_no_throw
{
    return PartyAudioManipulationSourceStreamReturnBuffer(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        buffer);
}

PartyError PartyAudioManipulationSourceStream::GetCustomContext(
    _Outptr_result_maybenull_ void** customContext
    ) const party_no_throw
{
    return PartyAudioManipulationSourceStreamGetCustomContext(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        customContext);
}

PartyError PartyAudioManipulationSourceStream::SetCustomContext(
    _In_opt_ void* customContext
    ) party_no_throw
{
    return PartyAudioManipulationSourceStreamSetCustomContext(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE>(this),
        customContext);
}

//
// PartyAudioManipulationSinkStream class implementation
//

PartyError PartyAudioManipulationSinkStream::GetConfiguration(
    _Out_ PartyAudioManipulationSinkStreamConfiguration* configuration
    ) const party_no_throw
{
    return PartyAudioManipulationSinkStreamGetConfiguration(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION*>(configuration));
}

PartyError PartyAudioManipulationSinkStream::GetFormat(
    _Out_ PartyAudioFormat* format
    ) const party_no_throw
{
    return PartyAudioManipulationSinkStreamGetFormat(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE>(this),
        reinterpret_cast<PARTY_AUDIO_FORMAT*>(format));
}

PartyError PartyAudioManipulationSinkStream::SubmitBuffer(
    const PartyDataBuffer* buffer
    ) party_no_throw
{
    return PartyAudioManipulationSinkStreamSubmitBuffer(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE>(this),
        reinterpret_cast<const PARTY_DATA_BUFFER*>(buffer));
}

PartyError PartyAudioManipulationSinkStream::GetCustomContext(
    _Outptr_result_maybenull_ void** customContext
    ) const party_no_throw
{
    return PartyAudioManipulationSinkStreamGetCustomContext(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE>(this),
        customContext);
}

PartyError PartyAudioManipulationSinkStream::SetCustomContext(
    _In_opt_ void* customContext
    ) party_no_throw
{
    return PartyAudioManipulationSinkStreamSetCustomContext(
        reinterpret_cast<PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE>(this),
        customContext);
}

//
// PartyManager class implementation
//

PartyManager::PartyManager() :
    _handle(nullptr)
{
}

PartyManager::~PartyManager()
{
}

PartyManager& PartyManager::GetSingleton() party_no_throw
{
    static PartyManager singleton;
    return singleton;
}

PartyError PartyManager::SetOption(
    _In_opt_ void* object,
    PartyOption option,
    _In_opt_ const void* value
    ) party_no_throw
{
    return PartySetOption(
        object,
        static_cast<PARTY_OPTION>(option),
        value);
}

PartyError PartyManager::GetOption(
    _In_opt_ const void* object,
    PartyOption option,
    _Out_ void* value
    ) party_no_throw
{
    return PartyGetOption(
        object,
        static_cast<PARTY_OPTION>(option),
        value);
}

PartyError PartyManager::GetErrorMessage(
    PartyError error,
    _Outptr_ PartyString* errorMessage
    ) party_no_throw
{
    return PartyGetErrorMessage(error, errorMessage);
}

PartyError PartyManager::SerializeNetworkDescriptor(
    const PartyNetworkDescriptor * networkDescriptor,
    _Out_writes_z_(c_maxSerializedNetworkDescriptorStringLength + 1) char * serializedNetworkDescriptorString
    ) party_no_throw
{
    return PartySerializeNetworkDescriptor(
        reinterpret_cast<const PARTY_NETWORK_DESCRIPTOR *>(networkDescriptor),
        serializedNetworkDescriptorString);
}

PartyError PartyManager::DeserializeNetworkDescriptor(
    PartyString serializedNetworkDescriptorString,
    _Out_ PartyNetworkDescriptor * networkDescriptor
    ) party_no_throw
{
    return PartyDeserializeNetworkDescriptor(
        serializedNetworkDescriptorString,
        reinterpret_cast<PARTY_NETWORK_DESCRIPTOR *>(networkDescriptor));
}

PartyError PartyManager::SetMemoryCallbacks(
    _In_ PartyAllocateMemoryCallback allocateMemoryCallback,
    _In_ PartyFreeMemoryCallback freeMemoryCallback
    ) party_no_throw
{
    return PartySetMemoryCallbacks(allocateMemoryCallback, freeMemoryCallback);
}

PartyError PartyManager::GetMemoryCallbacks(
    _Out_ PartyAllocateMemoryCallback * allocateMemoryCallback,
    _Out_ PartyFreeMemoryCallback * freeMemoryCallback
    ) party_no_throw
{
    return PartyGetMemoryCallbacks(allocateMemoryCallback, freeMemoryCallback);
}

PartyError PartyManager::SetProfilingCallbacksForMethodEntryExit(
    _In_opt_ PartyProfilingMethodEntranceCallback profilingMethodEntranceCallback,
    _In_opt_ PartyProfilingMethodExitCallback profilingMethodExitCallback
    ) party_no_throw
{
    return PartySetProfilingCallbacksForMethodEntryExit(
        reinterpret_cast<PARTY_PROFILING_METHOD_ENTRANCE_FUNC>(profilingMethodEntranceCallback),
        reinterpret_cast<PARTY_PROFILING_METHOD_EXIT_FUNC>(profilingMethodExitCallback));
}

PartyError PartyManager::GetProfilingCallbacksForMethodEntryExit(
    _Outptr_result_maybenull_ PartyProfilingMethodEntranceCallback * profilingMethodEntranceCallback,
    _Outptr_result_maybenull_ PartyProfilingMethodExitCallback * profilingMethodExitCallback
    ) party_no_throw
{
    return PartyGetProfilingCallbacksForMethodEntryExit(
        reinterpret_cast<PARTY_PROFILING_METHOD_ENTRANCE_FUNC*>(profilingMethodEntranceCallback),
        reinterpret_cast<PARTY_PROFILING_METHOD_EXIT_FUNC*>(profilingMethodExitCallback));
}

PartyError PartyManager::SetThreadAffinityMask(
    PartyThreadId threadId,
    uint64_t threadAffinityMask
    ) party_no_throw
{
    return PartySetThreadAffinityMask(
        static_cast<PARTY_THREAD_ID>(threadId),
        threadAffinityMask);
}

PartyError PartyManager::GetThreadAffinityMask(
    PartyThreadId threadId,
    _Out_ uint64_t * threadAffinityMask
    ) party_no_throw
{
    return PartyGetThreadAffinityMask(
        static_cast<PARTY_THREAD_ID>(threadId),
        threadAffinityMask);
}

PartyError PartyManager::SetWorkMode(
    PartyThreadId threadId,
    PartyWorkMode workMode
    ) party_no_throw
{
    return PartySetWorkMode(
        static_cast<PARTY_THREAD_ID>(threadId),
        static_cast<PARTY_WORK_MODE>(workMode));
}

PartyError PartyManager::GetWorkMode(
    PartyThreadId threadId,
    _Out_ PartyWorkMode * workMode
    ) party_no_throw
{
    return PartyGetWorkMode(
        static_cast<PARTY_THREAD_ID>(threadId),
        reinterpret_cast<PARTY_WORK_MODE *>(workMode));
}

PartyError PartyManager::Initialize(
    const PartyInitializationConfiguration * initializationConfiguration
    ) party_no_throw
{
    void* newHandle = nullptr;

    PartyError error = PartyInitialize(
        const_cast<PARTY_INITIALIZATION_CONFIGURATION *>(reinterpret_cast<const PARTY_INITIALIZATION_CONFIGURATION *>(initializationConfiguration)),
        const_cast<PARTY_HANDLE *>(reinterpret_cast<const PARTY_HANDLE *>(&newHandle)));
    if (PARTY_SUCCEEDED(error))
    {
        _handle = newHandle;
    }

    return error;
}

PartyError PartyManager::Cleanup() party_no_throw
{
    PartyError error = PartyCleanup(reinterpret_cast<PARTY_HANDLE>(_handle));
    _handle = nullptr;

    return error;
}

PartyError PartyManager::StartProcessingStateChanges(
    _Out_ uint32_t * stateChangeCount,
    _Outptr_result_buffer_(*stateChangeCount) PartyStateChangeArray * stateChanges
    ) party_no_throw
{
    return PartyStartProcessingStateChanges(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        stateChangeCount,
        reinterpret_cast<const PARTY_STATE_CHANGE * const **>(stateChanges));
}

PartyError PartyManager::FinishProcessingStateChanges(
    uint32_t stateChangeCount,
    _In_reads_(stateChangeCount) PartyStateChangeArray stateChanges
    ) party_no_throw
{
    return PartyFinishProcessingStateChanges(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        stateChangeCount,
        reinterpret_cast<const PARTY_STATE_CHANGE * const *>(stateChanges));
}

PartyError PartyManager::DoWork(
    PartyThreadId threadId
    ) party_no_throw
{
    return PartyDoWork(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        static_cast<PARTY_THREAD_ID>(threadId));
}

PartyError PartyManager::GetRegions(
    _Out_ uint32_t * regionListCount,
    _Outptr_result_buffer_(*regionListCount) const PartyRegion ** regionList
    ) party_no_throw
{
    return PartyGetRegions(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        regionListCount,
        reinterpret_cast<const PARTY_REGION **>(regionList));
}

PartyError PartyManager::CreateNewNetwork(
    const PartyLocalUser * localUser,
    const PartyNetworkConfiguration * networkConfiguration,
    uint32_t regionCount,
    const PartyRegion * regions,
    _In_opt_ const PartyInvitationConfiguration * initialInvitationConfiguration,
    _In_opt_ void * asyncIdentifier,
    _Out_opt_ PartyNetworkDescriptor * networkDescriptor,
    _Out_writes_opt_z_(c_maxInvitationIdentifierStringLength + 1) char * appliedInitialInvitationIdentifier
    ) party_no_throw
{
    return PartyCreateNewNetwork(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        reinterpret_cast<const PARTY_LOCAL_USER_HANDLE>(localUser),
        reinterpret_cast<const PARTY_NETWORK_CONFIGURATION *>(networkConfiguration),
        regionCount,
        reinterpret_cast<const PARTY_REGION *>(regions),
        reinterpret_cast<const PARTY_INVITATION_CONFIGURATION *>(initialInvitationConfiguration),
        asyncIdentifier,
        reinterpret_cast<PARTY_NETWORK_DESCRIPTOR *>(networkDescriptor),
        appliedInitialInvitationIdentifier);
}

PartyError  PartyManager::ConnectToNetwork(
    const PartyNetworkDescriptor * networkDescriptor,
    _In_opt_ void * asyncIdentifier,
    _Outptr_opt_ PartyNetwork ** network
    ) party_no_throw
{
    return PartyConnectToNetwork(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        reinterpret_cast<const PARTY_NETWORK_DESCRIPTOR *>(networkDescriptor),
        asyncIdentifier,
        const_cast<PARTY_NETWORK_HANDLE *>(reinterpret_cast<const PARTY_NETWORK_HANDLE *>(network)));
}

PartyError PartyManager::SynchronizeMessagesBetweenEndpoints(
    uint32_t endpointCount,
    _In_reads_(endpointCount) PartyEndpointArray endpoints,
    PartySynchronizeMessagesBetweenEndpointsOptions options,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartySynchronizeMessagesBetweenEndpoints(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        endpointCount,
        reinterpret_cast<const PARTY_ENDPOINT_HANDLE *>(endpoints),
        static_cast<PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_OPTIONS>(options),
        asyncIdentifier);
}

PartyError PartyManager::GetLocalDevice(
    _Outptr_ PartyLocalDevice ** localDevice
    ) const party_no_throw
{
    return PartyGetLocalDevice(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        const_cast<PARTY_DEVICE_HANDLE *>(reinterpret_cast<const PARTY_DEVICE_HANDLE *>(localDevice)));
}

PartyError PartyManager::CreateLocalUser(
    PFEntityHandle entityHandle,
    _Outptr_ PartyLocalUser ** localUser
    ) party_no_throw
{
    return PartyCreateLocalUser(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        entityHandle,
        const_cast<PARTY_LOCAL_USER_HANDLE *>(reinterpret_cast<const PARTY_LOCAL_USER_HANDLE *>(localUser)));
}

PartyError PartyManager::DestroyLocalUser(
    const PartyLocalUser * localUser,
    _In_opt_ void * asyncIdentifier
    ) party_no_throw
{
    return PartyDestroyLocalUser(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        reinterpret_cast<const PARTY_LOCAL_USER_HANDLE>(localUser),
        asyncIdentifier);
}

PartyError PartyManager::GetLocalUsers(
    _Out_ uint32_t * userCount,
    _Outptr_result_buffer_(*userCount) PartyLocalUserArray * users
    ) const party_no_throw
{
    return PartyGetLocalUsers(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        userCount,
        const_cast<const PARTY_LOCAL_USER_HANDLE **>(reinterpret_cast<const PARTY_LOCAL_USER_HANDLE *const*>(users)));
}

PartyError PartyManager::GetNetworks(
    _Out_ uint32_t * networkCount,
    _Outptr_result_buffer_(*networkCount) PartyNetworkArray * networks
    ) const party_no_throw
{
    return PartyGetNetworks(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        networkCount,
        const_cast<const PARTY_NETWORK_HANDLE **>(reinterpret_cast<const PARTY_NETWORK_HANDLE *const*>(networks)));
}

PartyError PartyManager::GetChatControls(
    _Out_ uint32_t * chatControlCount,
    _Outptr_result_buffer_(*chatControlCount) PartyChatControlArray * chatControls
    ) const party_no_throw
{
    return PartyGetChatControls(
        reinterpret_cast<PARTY_HANDLE>(_handle),
        chatControlCount,
        const_cast<const PARTY_CHAT_CONTROL_HANDLE **>(reinterpret_cast<const PARTY_CHAT_CONTROL_HANDLE *const*>(chatControls)));
}

//
// C to C++ structure and constant verification
//
#pragma push_macro("PARTY_C_ASSERT")
#undef PARTY_C_ASSERT
#define PARTY_C_ASSERT(e) typedef char __PARTY_C_ASSERT__[(e)?1:-1]

// BEGIN GENERATED SECTION: DO NOT EDIT

PARTY_C_ASSERT(PARTY_MAX_NETWORK_CONFIGURATION_MAX_DEVICE_COUNT == c_maxNetworkConfigurationMaxDeviceCount);
PARTY_C_ASSERT(PARTY_MAX_DIRECT_PEER_CONNECTIONS_PER_DEVICE == c_maxDirectPeerConnectionsPerDevice);
PARTY_C_ASSERT(PARTY_MAX_NETWORK_CONFIGURATION_MAX_ENDPOINTS_PER_DEVICE_COUNT == c_maxNetworkConfigurationMaxEndpointsPerDeviceCount);
PARTY_C_ASSERT(PARTY_MAX_LOCAL_USERS_PER_DEVICE_COUNT == c_maxLocalUsersPerDeviceCount);
PARTY_C_ASSERT(PARTY_OPAQUE_CONNECTION_INFORMATION_BYTE_COUNT == c_opaqueConnectionInformationByteCount);
PARTY_C_ASSERT(PARTY_MAX_INVITATION_IDENTIFIER_STRING_LENGTH == c_maxInvitationIdentifierStringLength);
PARTY_C_ASSERT(PARTY_MAX_INVITATION_ENTITY_ID_COUNT == c_maxInvitationEntityIdCount);
PARTY_C_ASSERT(PARTY_MAX_ENTITY_ID_STRING_LENGTH == c_maxEntityIdStringLength);
PARTY_C_ASSERT(PARTY_NETWORK_IDENTIFIER_STRING_LENGTH == c_networkIdentifierStringLength);
PARTY_C_ASSERT(PARTY_MAX_REGION_NAME_STRING_LENGTH == c_maxRegionNameStringLength);
PARTY_C_ASSERT(PARTY_MAX_SERIALIZED_NETWORK_DESCRIPTOR_STRING_LENGTH == c_maxSerializedNetworkDescriptorStringLength);
PARTY_C_ASSERT(PARTY_MAX_AUDIO_DEVICE_IDENTIFIER_STRING_LENGTH == c_maxAudioDeviceIdentifierStringLength);
PARTY_C_ASSERT(PARTY_MAX_LANGUAGE_CODE_STRING_LENGTH == c_maxLanguageCodeStringLength);
PARTY_C_ASSERT(PARTY_MAX_CHAT_TEXT_MESSAGE_STRING_LENGTH == c_maxChatTextMessageStringLength);
PARTY_C_ASSERT(PARTY_MAX_CHAT_TRANSCRIPTION_MESSAGE_STRING_LENGTH == c_maxChatTranscriptionMessageStringLength);
PARTY_C_ASSERT(PARTY_MAX_TEXT_TO_SPEECH_PROFILE_IDENTIFIER_STRING_LENGTH == c_maxTextToSpeechProfileIdentifierStringLength);
PARTY_C_ASSERT(PARTY_MAX_TEXT_TO_SPEECH_PROFILE_NAME_STRING_LENGTH == c_maxTextToSpeechProfileNameStringLength);
PARTY_C_ASSERT(PARTY_MAX_TEXT_TO_SPEECH_INPUT_STRING_LENGTH == c_maxTextToSpeechInputStringLength);
PARTY_C_ASSERT(PARTY_ANY_PROCESSOR == c_anyProcessor);
PARTY_C_ASSERT(PARTY_MIN_SEND_MESSAGE_QUEUING_PRIORITY == c_minSendMessageQueuingPriority);
PARTY_C_ASSERT(PARTY_CHAT_SEND_MESSAGE_QUEUING_PRIORITY == c_chatSendMessageQueuingPriority);
PARTY_C_ASSERT(PARTY_DEFAULT_SEND_MESSAGE_QUEUING_PRIORITY == c_defaultSendMessageQueuingPriority);
PARTY_C_ASSERT(PARTY_MAX_SEND_MESSAGE_QUEUING_PRIORITY == c_maxSendMessageQueuingPriority);

PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REGIONS_CHANGED == static_cast<uint32_t>(PartyStateChangeType::RegionsChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_DESTROY_LOCAL_USER_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::DestroyLocalUserCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CREATE_NEW_NETWORK_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::CreateNewNetworkCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CONNECT_TO_NETWORK_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::ConnectToNetworkCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_AUTHENTICATE_LOCAL_USER_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::AuthenticateLocalUserCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_NETWORK_CONFIGURATION_MADE_AVAILABLE == static_cast<uint32_t>(PartyStateChangeType::NetworkConfigurationMadeAvailable));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_NETWORK_DESCRIPTOR_CHANGED == static_cast<uint32_t>(PartyStateChangeType::NetworkDescriptorChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_LOCAL_USER_REMOVED == static_cast<uint32_t>(PartyStateChangeType::LocalUserRemoved));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REMOVE_LOCAL_USER_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::RemoveLocalUserCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_LOCAL_USER_KICKED == static_cast<uint32_t>(PartyStateChangeType::LocalUserKicked));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CREATE_ENDPOINT_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::CreateEndpointCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_DESTROY_ENDPOINT_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::DestroyEndpointCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_ENDPOINT_CREATED == static_cast<uint32_t>(PartyStateChangeType::EndpointCreated));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_ENDPOINT_DESTROYED == static_cast<uint32_t>(PartyStateChangeType::EndpointDestroyed));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REMOTE_DEVICE_CREATED == static_cast<uint32_t>(PartyStateChangeType::RemoteDeviceCreated));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REMOTE_DEVICE_DESTROYED == static_cast<uint32_t>(PartyStateChangeType::RemoteDeviceDestroyed));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REMOTE_DEVICE_JOINED_NETWORK == static_cast<uint32_t>(PartyStateChangeType::RemoteDeviceJoinedNetwork));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REMOTE_DEVICE_LEFT_NETWORK == static_cast<uint32_t>(PartyStateChangeType::RemoteDeviceLeftNetwork));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_DEVICE_PROPERTIES_CHANGED == static_cast<uint32_t>(PartyStateChangeType::DevicePropertiesChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_LEAVE_NETWORK_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::LeaveNetworkCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_NETWORK_DESTROYED == static_cast<uint32_t>(PartyStateChangeType::NetworkDestroyed));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_ENDPOINT_MESSAGE_RECEIVED == static_cast<uint32_t>(PartyStateChangeType::EndpointMessageReceived));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_DATA_BUFFERS_RETURNED == static_cast<uint32_t>(PartyStateChangeType::DataBuffersReturned));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_ENDPOINT_PROPERTIES_CHANGED == static_cast<uint32_t>(PartyStateChangeType::EndpointPropertiesChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SynchronizeMessagesBetweenEndpointsCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_NETWORK_PROPERTIES_CHANGED == static_cast<uint32_t>(PartyStateChangeType::NetworkPropertiesChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_KICK_DEVICE_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::KickDeviceCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_KICK_USER_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::KickUserCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CREATE_CHAT_CONTROL_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::CreateChatControlCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_DESTROY_CHAT_CONTROL_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::DestroyChatControlCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CHAT_CONTROL_CREATED == static_cast<uint32_t>(PartyStateChangeType::ChatControlCreated));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CHAT_CONTROL_DESTROYED == static_cast<uint32_t>(PartyStateChangeType::ChatControlDestroyed));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetChatAudioEncoderBitrateCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CHAT_TEXT_RECEIVED == static_cast<uint32_t>(PartyStateChangeType::ChatTextReceived));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_VOICE_CHAT_TRANSCRIPTION_RECEIVED == static_cast<uint32_t>(PartyStateChangeType::VoiceChatTranscriptionReceived));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_CHAT_AUDIO_INPUT_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetChatAudioInputCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_CHAT_AUDIO_OUTPUT_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetChatAudioOutputCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_LOCAL_CHAT_AUDIO_INPUT_CHANGED == static_cast<uint32_t>(PartyStateChangeType::LocalChatAudioInputChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED == static_cast<uint32_t>(PartyStateChangeType::LocalChatAudioOutputChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetTextToSpeechProfileCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SynthesizeTextToSpeechCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CHAT_CONTROL_PROPERTIES_CHANGED == static_cast<uint32_t>(PartyStateChangeType::ChatControlPropertiesChanged));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CHAT_CONTROL_JOINED_NETWORK == static_cast<uint32_t>(PartyStateChangeType::ChatControlJoinedNetwork));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CHAT_CONTROL_LEFT_NETWORK == static_cast<uint32_t>(PartyStateChangeType::ChatControlLeftNetwork));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CONNECT_CHAT_CONTROL_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::ConnectChatControlCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_DISCONNECT_CHAT_CONTROL_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::DisconnectChatControlCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::PopulateAvailableTextToSpeechProfilesCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CREATE_INVITATION_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::CreateInvitationCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_REVOKE_INVITATION_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::RevokeInvitationCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_INVITATION_CREATED == static_cast<uint32_t>(PartyStateChangeType::InvitationCreated));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_INVITATION_DESTROYED == static_cast<uint32_t>(PartyStateChangeType::InvitationDestroyed));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_LANGUAGE_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetLanguageCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_TRANSCRIPTION_OPTIONS_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetTranscriptionOptionsCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_SET_TEXT_CHAT_OPTIONS_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::SetTextChatOptionsCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::ConfigureAudioManipulationVoiceStreamCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::ConfigureAudioManipulationCaptureStreamCompleted));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_TYPE_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED == static_cast<uint32_t>(PartyStateChangeType::ConfigureAudioManipulationRenderStreamCompleted));

PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_SUCCEEDED == static_cast<uint32_t>(PartyStateChangeResult::Succeeded));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_UNKNOWN_ERROR == static_cast<uint32_t>(PartyStateChangeResult::UnknownError));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_CANCELED_BY_TITLE == static_cast<uint32_t>(PartyStateChangeResult::CanceledByTitle));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_INTERNET_CONNECTIVITY_ERROR == static_cast<uint32_t>(PartyStateChangeResult::InternetConnectivityError));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_PARTY_SERVICE_ERROR == static_cast<uint32_t>(PartyStateChangeResult::PartyServiceError));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_NO_SERVERS_AVAILABLE == static_cast<uint32_t>(PartyStateChangeResult::NoServersAvailable));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_USER_NOT_AUTHORIZED == static_cast<uint32_t>(PartyStateChangeResult::UserNotAuthorized));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_USER_CREATE_NETWORK_THROTTLED == static_cast<uint32_t>(PartyStateChangeResult::UserCreateNetworkThrottled));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_TITLE_NOT_ENABLED_FOR_PARTY == static_cast<uint32_t>(PartyStateChangeResult::TitleNotEnabledForParty));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_NETWORK_LIMIT_REACHED == static_cast<uint32_t>(PartyStateChangeResult::NetworkLimitReached));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_NETWORK_NO_LONGER_EXISTS == static_cast<uint32_t>(PartyStateChangeResult::NetworkNoLongerExists));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_NETWORK_NOT_JOINABLE == static_cast<uint32_t>(PartyStateChangeResult::NetworkNotJoinable));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_VERSION_MISMATCH == static_cast<uint32_t>(PartyStateChangeResult::VersionMismatch));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_LEAVE_NETWORK_CALLED == static_cast<uint32_t>(PartyStateChangeResult::LeaveNetworkCalled));
PARTY_C_ASSERT(PARTY_STATE_CHANGE_RESULT_FAILED_TO_BIND_TO_LOCAL_UDP_SOCKET == static_cast<uint32_t>(PartyStateChangeResult::FailedToBindToLocalUdpSocket));

PARTY_C_ASSERT(PARTY_LOCAL_USER_REMOVED_REASON_AUTHENTICATION_FAILED == static_cast<uint32_t>(PartyLocalUserRemovedReason::AuthenticationFailed));
PARTY_C_ASSERT(PARTY_LOCAL_USER_REMOVED_REASON_REMOVE_LOCAL_USER == static_cast<uint32_t>(PartyLocalUserRemovedReason::RemoveLocalUser));
PARTY_C_ASSERT(PARTY_LOCAL_USER_REMOVED_REASON_DESTROY_LOCAL_USER == static_cast<uint32_t>(PartyLocalUserRemovedReason::DestroyLocalUser));
PARTY_C_ASSERT(PARTY_LOCAL_USER_REMOVED_REASON_DESTROY_NETWORK == static_cast<uint32_t>(PartyLocalUserRemovedReason::DestroyNetwork));

PARTY_C_ASSERT(PARTY_DESTROYED_REASON_REQUESTED == static_cast<uint32_t>(PartyDestroyedReason::Requested));
PARTY_C_ASSERT(PARTY_DESTROYED_REASON_DISCONNECTED == static_cast<uint32_t>(PartyDestroyedReason::Disconnected));
PARTY_C_ASSERT(PARTY_DESTROYED_REASON_KICKED == static_cast<uint32_t>(PartyDestroyedReason::Kicked));
PARTY_C_ASSERT(PARTY_DESTROYED_REASON_DEVICE_LOST_AUTHENTICATION == static_cast<uint32_t>(PartyDestroyedReason::DeviceLostAuthentication));
PARTY_C_ASSERT(PARTY_DESTROYED_REASON_CREATION_FAILED == static_cast<uint32_t>(PartyDestroyedReason::CreationFailed));

PARTY_C_ASSERT(PARTY_TEXT_CHAT_FILTER_LEVEL_FAMILY_FRIENDLY == static_cast<uint32_t>(PartyTextChatFilterLevel::FamilyFriendly));
PARTY_C_ASSERT(PARTY_TEXT_CHAT_FILTER_LEVEL_MEDIUM == static_cast<uint32_t>(PartyTextChatFilterLevel::Medium));
PARTY_C_ASSERT(PARTY_TEXT_CHAT_FILTER_LEVEL_MATURE == static_cast<uint32_t>(PartyTextChatFilterLevel::Mature));

PARTY_C_ASSERT(PARTY_OPTION_LOCAL_UDP_SOCKET_BIND_ADDRESS == static_cast<uint32_t>(PartyOption::LocalUdpSocketBindAddress));
PARTY_C_ASSERT(PARTY_OPTION_LOCAL_DEVICE_DIRECT_PEER_CONNECTIVITY_OPTIONS_MASK == static_cast<uint32_t>(PartyOption::LocalDeviceDirectPeerConnectivityOptionsMask));
PARTY_C_ASSERT(PARTY_OPTION_TEXT_CHAT_FILTER_LEVEL == static_cast<uint32_t>(PartyOption::TextChatFilterLevel));
PARTY_C_ASSERT(PARTY_OPTION_LOCAL_DEVICE_MAX_DIRECT_PEER_CONNECTIONS == static_cast<uint32_t>(PartyOption::LocalDeviceMaxDirectPeerConnections));
PARTY_C_ASSERT(PARTY_OPTION_REGION_UPDATE_CONFIGURATION == static_cast<uint32_t>(PartyOption::RegionUpdateConfiguration));
PARTY_C_ASSERT(PARTY_OPTION_REGION_QUALITY_MEASUREMENT_CONFIGURATION == static_cast<uint32_t>(PartyOption::RegionQualityMeasurementConfiguration));

PARTY_C_ASSERT(PARTY_THREAD_ID_AUDIO == static_cast<uint32_t>(PartyThreadId::Audio));
PARTY_C_ASSERT(PARTY_THREAD_ID_NETWORKING == static_cast<uint32_t>(PartyThreadId::Networking));

PARTY_C_ASSERT(PARTY_WORK_MODE_AUTOMATIC == static_cast<uint32_t>(PartyWorkMode::Automatic));
PARTY_C_ASSERT(PARTY_WORK_MODE_MANUAL == static_cast<uint32_t>(PartyWorkMode::Manual));

PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_DEFAULT == static_cast<uint32_t>(PartySendMessageOptions::Default));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_GUARANTEED_DELIVERY == static_cast<uint32_t>(PartySendMessageOptions::GuaranteedDelivery));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_BEST_EFFORT_DELIVERY == static_cast<uint32_t>(PartySendMessageOptions::BestEffortDelivery));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_SEQUENTIAL_DELIVERY == static_cast<uint32_t>(PartySendMessageOptions::SequentialDelivery));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_NONSEQUENTIAL_DELIVERY == static_cast<uint32_t>(PartySendMessageOptions::NonsequentialDelivery));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_COPY_DATA_BUFFERS == static_cast<uint32_t>(PartySendMessageOptions::CopyDataBuffers));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_DONT_COPY_DATA_BUFFERS == static_cast<uint32_t>(PartySendMessageOptions::DontCopyDataBuffers));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_COALESCE_OPPORTUNISTICALLY == static_cast<uint32_t>(PartySendMessageOptions::CoalesceOpportunistically));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_ALWAYS_COALESCE_UNTIL_FLUSHED == static_cast<uint32_t>(PartySendMessageOptions::AlwaysCoalesceUntilFlushed));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_REQUIRE_TIMELY_ACKNOWLEDGEMENT == static_cast<uint32_t>(PartySendMessageOptions::RequireTimelyAcknowledgement));
PARTY_C_ASSERT(PARTY_SEND_MESSAGE_OPTIONS_ALLOW_LAZY_ACKNOWLEDGEMENT == static_cast<uint32_t>(PartySendMessageOptions::AllowLazyAcknowledgement));

PARTY_C_ASSERT(PARTY_MESSAGE_RECEIVED_OPTIONS_NONE == static_cast<uint32_t>(PartyMessageReceivedOptions::None));
PARTY_C_ASSERT(PARTY_MESSAGE_RECEIVED_OPTIONS_GUARANTEED_DELIVERY == static_cast<uint32_t>(PartyMessageReceivedOptions::GuaranteedDelivery));
PARTY_C_ASSERT(PARTY_MESSAGE_RECEIVED_OPTIONS_SEQUENTIAL_DELIVERY == static_cast<uint32_t>(PartyMessageReceivedOptions::SequentialDelivery));
PARTY_C_ASSERT(PARTY_MESSAGE_RECEIVED_OPTIONS_REQUIRED_FRAGMENTATION == static_cast<uint32_t>(PartyMessageReceivedOptions::RequiredFragmentation));

PARTY_C_ASSERT(PARTY_CANCEL_MESSAGES_FILTER_EXPRESSION_NONE == static_cast<uint32_t>(PartyCancelMessagesFilterExpression::None));
PARTY_C_ASSERT(PARTY_CANCEL_MESSAGES_FILTER_EXPRESSION_IDENTITY_AND_MASK_EQUALS_MATCH_VALUE == static_cast<uint32_t>(PartyCancelMessagesFilterExpression::IdentityAndMaskEqualsMatchValue));
PARTY_C_ASSERT(PARTY_CANCEL_MESSAGES_FILTER_EXPRESSION_IDENTITY_AND_MASK_DOES_NOT_EQUAL_MATCH_VALUE == static_cast<uint32_t>(PartyCancelMessagesFilterExpression::IdentityAndMaskDoesNotEqualMatchValue));

PARTY_C_ASSERT(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_OPTIONS_NONE == static_cast<uint32_t>(PartySynchronizeMessagesBetweenEndpointsOptions::None));
PARTY_C_ASSERT(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_OPTIONS_SYNCHRONIZE_MESSAGES_WITH_SHARED_PROPERTIES == static_cast<uint32_t>(PartySynchronizeMessagesBetweenEndpointsOptions::SynchronizeMessagesWithSharedProperties));

PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_AVERAGE_RELAY_SERVER_ROUND_TRIP_LATENCY_IN_MILLISECONDS == static_cast<uint32_t>(PartyNetworkStatistic::AverageRelayServerRoundTripLatencyInMilliseconds));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_SENT_PROTOCOL_PACKETS == static_cast<uint32_t>(PartyNetworkStatistic::SentProtocolPackets));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_SENT_PROTOCOL_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::SentProtocolBytes));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_RETRIED_PROTOCOL_PACKETS == static_cast<uint32_t>(PartyNetworkStatistic::RetriedProtocolPackets));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_RETRIED_PROTOCOL_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::RetriedProtocolBytes));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_DROPPED_PROTOCOL_PACKETS == static_cast<uint32_t>(PartyNetworkStatistic::DroppedProtocolPackets));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_RECEIVED_PROTOCOL_PACKETS == static_cast<uint32_t>(PartyNetworkStatistic::ReceivedProtocolPackets));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_RECEIVED_PROTOCOL_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::ReceivedProtocolBytes));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_CURRENTLY_QUEUED_SEND_MESSAGES == static_cast<uint32_t>(PartyNetworkStatistic::CurrentlyQueuedSendMessages));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_CURRENTLY_QUEUED_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::CurrentlyQueuedSendMessageBytes));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_CURRENTLY_ACTIVE_SEND_MESSAGES == static_cast<uint32_t>(PartyNetworkStatistic::CurrentlyActiveSendMessages));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_CURRENTLY_ACTIVE_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::CurrentlyActiveSendMessageBytes));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_TIMED_OUT_SEND_MESSAGES == static_cast<uint32_t>(PartyNetworkStatistic::TimedOutSendMessages));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_TIMED_OUT_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::TimedOutSendMessageBytes));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_CANCELED_SEND_MESSAGES == static_cast<uint32_t>(PartyNetworkStatistic::CanceledSendMessages));
PARTY_C_ASSERT(PARTY_NETWORK_STATISTIC_CANCELED_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyNetworkStatistic::CanceledSendMessageBytes));

PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_CURRENTLY_QUEUED_SEND_MESSAGES == static_cast<uint32_t>(PartyEndpointStatistic::CurrentlyQueuedSendMessages));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_CURRENTLY_QUEUED_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyEndpointStatistic::CurrentlyQueuedSendMessageBytes));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_CURRENTLY_ACTIVE_SEND_MESSAGES == static_cast<uint32_t>(PartyEndpointStatistic::CurrentlyActiveSendMessages));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_CURRENTLY_ACTIVE_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyEndpointStatistic::CurrentlyActiveSendMessageBytes));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_TIMED_OUT_SEND_MESSAGES == static_cast<uint32_t>(PartyEndpointStatistic::TimedOutSendMessages));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_TIMED_OUT_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyEndpointStatistic::TimedOutSendMessageBytes));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_CANCELED_SEND_MESSAGES == static_cast<uint32_t>(PartyEndpointStatistic::CanceledSendMessages));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_CANCELED_SEND_MESSAGE_BYTES == static_cast<uint32_t>(PartyEndpointStatistic::CanceledSendMessageBytes));
PARTY_C_ASSERT(PARTY_ENDPOINT_STATISTIC_AVERAGE_DEVICE_ROUND_TRIP_LATENCY_IN_MILLISECONDS == static_cast<uint32_t>(PartyEndpointStatistic::AverageDeviceRoundTripLatencyInMilliseconds));

PARTY_C_ASSERT(PARTY_INVITATION_REVOCABILITY_CREATOR == static_cast<uint32_t>(PartyInvitationRevocability::Creator));
PARTY_C_ASSERT(PARTY_INVITATION_REVOCABILITY_ANYONE == static_cast<uint32_t>(PartyInvitationRevocability::Anyone));

PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_NONE == static_cast<uint32_t>(PartyChatPermissionOptions::None));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_SEND_MICROPHONE_AUDIO == static_cast<uint32_t>(PartyChatPermissionOptions::SendMicrophoneAudio));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_SEND_TEXT_TO_SPEECH_AUDIO == static_cast<uint32_t>(PartyChatPermissionOptions::SendTextToSpeechAudio));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_SEND_AUDIO == static_cast<uint32_t>(PartyChatPermissionOptions::SendAudio));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_RECEIVE_MICROPHONE_AUDIO == static_cast<uint32_t>(PartyChatPermissionOptions::ReceiveMicrophoneAudio));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_RECEIVE_TEXT_TO_SPEECH_AUDIO == static_cast<uint32_t>(PartyChatPermissionOptions::ReceiveTextToSpeechAudio));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_RECEIVE_AUDIO == static_cast<uint32_t>(PartyChatPermissionOptions::ReceiveAudio));
PARTY_C_ASSERT(PARTY_CHAT_PERMISSION_OPTIONS_RECEIVE_TEXT == static_cast<uint32_t>(PartyChatPermissionOptions::ReceiveText));

PARTY_C_ASSERT(PARTY_AUDIO_DEVICE_SELECTION_TYPE_NONE == static_cast<uint32_t>(PartyAudioDeviceSelectionType::None));
PARTY_C_ASSERT(PARTY_AUDIO_DEVICE_SELECTION_TYPE_SYSTEM_DEFAULT == static_cast<uint32_t>(PartyAudioDeviceSelectionType::SystemDefault));
PARTY_C_ASSERT(PARTY_AUDIO_DEVICE_SELECTION_TYPE_PLATFORM_USER_DEFAULT == static_cast<uint32_t>(PartyAudioDeviceSelectionType::PlatformUserDefault));
PARTY_C_ASSERT(PARTY_AUDIO_DEVICE_SELECTION_TYPE_MANUAL == static_cast<uint32_t>(PartyAudioDeviceSelectionType::Manual));

PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_NO_INPUT == static_cast<uint32_t>(PartyAudioInputState::NoInput));
PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_INITIALIZED == static_cast<uint32_t>(PartyAudioInputState::Initialized));
PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_NOT_FOUND == static_cast<uint32_t>(PartyAudioInputState::NotFound));
PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_USER_CONSENT_DENIED == static_cast<uint32_t>(PartyAudioInputState::UserConsentDenied));
PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_UNSUPPORTED_FORMAT == static_cast<uint32_t>(PartyAudioInputState::UnsupportedFormat));
PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_ALREADY_IN_USE == static_cast<uint32_t>(PartyAudioInputState::AlreadyInUse));
PARTY_C_ASSERT(PARTY_AUDIO_INPUT_STATE_UNKNOWN_ERROR == static_cast<uint32_t>(PartyAudioInputState::UnknownError));

PARTY_C_ASSERT(PARTY_AUDIO_OUTPUT_STATE_NO_OUTPUT == static_cast<uint32_t>(PartyAudioOutputState::NoOutput));
PARTY_C_ASSERT(PARTY_AUDIO_OUTPUT_STATE_INITIALIZED == static_cast<uint32_t>(PartyAudioOutputState::Initialized));
PARTY_C_ASSERT(PARTY_AUDIO_OUTPUT_STATE_NOT_FOUND == static_cast<uint32_t>(PartyAudioOutputState::NotFound));
PARTY_C_ASSERT(PARTY_AUDIO_OUTPUT_STATE_UNSUPPORTED_FORMAT == static_cast<uint32_t>(PartyAudioOutputState::UnsupportedFormat));
PARTY_C_ASSERT(PARTY_AUDIO_OUTPUT_STATE_ALREADY_IN_USE == static_cast<uint32_t>(PartyAudioOutputState::AlreadyInUse));
PARTY_C_ASSERT(PARTY_AUDIO_OUTPUT_STATE_UNKNOWN_ERROR == static_cast<uint32_t>(PartyAudioOutputState::UnknownError));

PARTY_C_ASSERT(PARTY_LOCAL_CHAT_CONTROL_CHAT_INDICATOR_SILENT == static_cast<uint32_t>(PartyLocalChatControlChatIndicator::Silent));
PARTY_C_ASSERT(PARTY_LOCAL_CHAT_CONTROL_CHAT_INDICATOR_TALKING == static_cast<uint32_t>(PartyLocalChatControlChatIndicator::Talking));
PARTY_C_ASSERT(PARTY_LOCAL_CHAT_CONTROL_CHAT_INDICATOR_AUDIO_INPUT_MUTED == static_cast<uint32_t>(PartyLocalChatControlChatIndicator::AudioInputMuted));
PARTY_C_ASSERT(PARTY_LOCAL_CHAT_CONTROL_CHAT_INDICATOR_NO_AUDIO_INPUT == static_cast<uint32_t>(PartyLocalChatControlChatIndicator::NoAudioInput));

PARTY_C_ASSERT(PARTY_CHAT_CONTROL_CHAT_INDICATOR_SILENT == static_cast<uint32_t>(PartyChatControlChatIndicator::Silent));
PARTY_C_ASSERT(PARTY_CHAT_CONTROL_CHAT_INDICATOR_TALKING == static_cast<uint32_t>(PartyChatControlChatIndicator::Talking));
PARTY_C_ASSERT(PARTY_CHAT_CONTROL_CHAT_INDICATOR_INCOMING_VOICE_DISABLED == static_cast<uint32_t>(PartyChatControlChatIndicator::IncomingVoiceDisabled));
PARTY_C_ASSERT(PARTY_CHAT_CONTROL_CHAT_INDICATOR_INCOMING_COMMUNICATIONS_MUTED == static_cast<uint32_t>(PartyChatControlChatIndicator::IncomingCommunicationsMuted));
PARTY_C_ASSERT(PARTY_CHAT_CONTROL_CHAT_INDICATOR_NO_REMOTE_INPUT == static_cast<uint32_t>(PartyChatControlChatIndicator::NoRemoteInput));
PARTY_C_ASSERT(PARTY_CHAT_CONTROL_CHAT_INDICATOR_REMOTE_AUDIO_INPUT_MUTED == static_cast<uint32_t>(PartyChatControlChatIndicator::RemoteAudioInputMuted));

PARTY_C_ASSERT(PARTY_GENDER_NEUTRAL == static_cast<uint32_t>(PartyGender::Neutral));
PARTY_C_ASSERT(PARTY_GENDER_FEMALE == static_cast<uint32_t>(PartyGender::Female));
PARTY_C_ASSERT(PARTY_GENDER_MALE == static_cast<uint32_t>(PartyGender::Male));

PARTY_C_ASSERT(PARTY_AUDIO_SOURCE_TYPE_MICROPHONE == static_cast<uint32_t>(PartyAudioSourceType::Microphone));
PARTY_C_ASSERT(PARTY_AUDIO_SOURCE_TYPE_TEXT_TO_SPEECH == static_cast<uint32_t>(PartyAudioSourceType::TextToSpeech));

PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_PHRASE_TYPE_HYPOTHESIS == static_cast<uint32_t>(PartyVoiceChatTranscriptionPhraseType::Hypothesis));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_PHRASE_TYPE_FINAL == static_cast<uint32_t>(PartyVoiceChatTranscriptionPhraseType::Final));

PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_NONE == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::None));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_TRANSCRIBE_SELF == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::TranscribeSelf));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_TRANSCRIBE_OTHER_CHAT_CONTROLS_WITH_MATCHING_LANGUAGES == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::TranscribeOtherChatControlsWithMatchingLanguages));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_TRANSCRIBE_OTHER_CHAT_CONTROLS_WITH_NON_MATCHING_LANGUAGES == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::TranscribeOtherChatControlsWithNonMatchingLanguages));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_DISABLE_HYPOTHESIS_PHRASES == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::DisableHypothesisPhrases));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_TRANSLATE_TO_LOCAL_LANGUAGE == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::TranslateToLocalLanguage));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_DISABLE_PROFANITY_MASKING == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::DisableProfanityMasking));
PARTY_C_ASSERT(PARTY_VOICE_CHAT_TRANSCRIPTION_OPTIONS_TRANSCRIBE_SELF_REGARDLESS_OF_NETWORK_STATE == static_cast<uint32_t>(PartyVoiceChatTranscriptionOptions::TranscribeSelfRegardlessOfNetworkState));

PARTY_C_ASSERT(PARTY_TEXT_CHAT_OPTIONS_NONE == static_cast<uint32_t>(PartyTextChatOptions::None));
PARTY_C_ASSERT(PARTY_TEXT_CHAT_OPTIONS_TRANSLATE_TO_LOCAL_LANGUAGE == static_cast<uint32_t>(PartyTextChatOptions::TranslateToLocalLanguage));
PARTY_C_ASSERT(PARTY_TEXT_CHAT_OPTIONS_FILTER_OFFENSIVE_TEXT == static_cast<uint32_t>(PartyTextChatOptions::FilterOffensiveText));

PARTY_C_ASSERT(PARTY_TRANSLATION_RECEIVED_OPTIONS_NONE == static_cast<uint32_t>(PartyTranslationReceivedOptions::None));
PARTY_C_ASSERT(PARTY_TRANSLATION_RECEIVED_OPTIONS_TRUNCATED == static_cast<uint32_t>(PartyTranslationReceivedOptions::Truncated));

PARTY_C_ASSERT(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_TYPE_NARRATION == static_cast<uint32_t>(PartySynthesizeTextToSpeechType::Narration));
PARTY_C_ASSERT(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_TYPE_VOICE_CHAT == static_cast<uint32_t>(PartySynthesizeTextToSpeechType::VoiceChat));

PARTY_C_ASSERT(PARTY_AUDIO_SAMPLE_TYPE_INTEGER == static_cast<uint32_t>(PartyAudioSampleType::Integer));
PARTY_C_ASSERT(PARTY_AUDIO_SAMPLE_TYPE_FLOAT == static_cast<uint32_t>(PartyAudioSampleType::Float));

PARTY_C_ASSERT(PARTY_VOICE_AUDIO_OPTIONS_NONE == static_cast<uint32_t>(PartyVoiceAudioOptions::None));
PARTY_C_ASSERT(PARTY_VOICE_AUDIO_OPTIONS_NOISE_SUPPRESSION == static_cast<uint32_t>(PartyVoiceAudioOptions::NoiseSuppression));

PARTY_C_ASSERT(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_OPTIONS_NONE == static_cast<uint32_t>(PartyLocalUdpSocketBindAddressOptions::None));
PARTY_C_ASSERT(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_OPTIONS_EXCLUDE_GAME_CORE_PREFERRED_UDP_MULTIPLAYER_PORT == static_cast<uint32_t>(PartyLocalUdpSocketBindAddressOptions::ExcludeGameCorePreferredUdpMultiplayerPort));

PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_NONE == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::None));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_SAME_PLATFORM_TYPE == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::SamePlatformType));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_DIFFERENT_PLATFORM_TYPE == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::DifferentPlatformType));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_ANY_PLATFORM_TYPE == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::AnyPlatformType));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_SAME_ENTITY_LOGIN_PROVIDER == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::SameEntityLoginProvider));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_DIFFERENT_ENTITY_LOGIN_PROVIDER == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::DifferentEntityLoginProvider));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_ANY_ENTITY_LOGIN_PROVIDER == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::AnyEntityLoginProvider));
PARTY_C_ASSERT(PARTY_DIRECT_PEER_CONNECTIVITY_OPTIONS_ONLY_SERVERS == static_cast<uint32_t>(PartyDirectPeerConnectivityOptions::OnlyServers));

PARTY_C_ASSERT(PARTY_DEVICE_CONNECTION_TYPE_RELAY_SERVER == static_cast<uint32_t>(PartyDeviceConnectionType::RelayServer));
PARTY_C_ASSERT(PARTY_DEVICE_CONNECTION_TYPE_DIRECT_PEER_CONNECTION == static_cast<uint32_t>(PartyDeviceConnectionType::DirectPeerConnection));

PARTY_C_ASSERT(PARTY_CHAT_TEXT_RECEIVED_OPTIONS_NONE == static_cast<uint32_t>(PartyChatTextReceivedOptions::None));
PARTY_C_ASSERT(PARTY_CHAT_TEXT_RECEIVED_OPTIONS_FILTERED_OFFENSIVE_TERMS == static_cast<uint32_t>(PartyChatTextReceivedOptions::FilteredOffensiveTerms));
PARTY_C_ASSERT(PARTY_CHAT_TEXT_RECEIVED_OPTIONS_FILTERED_ENTIRE_MESSAGE == static_cast<uint32_t>(PartyChatTextReceivedOptions::FilteredEntireMessage));
PARTY_C_ASSERT(PARTY_CHAT_TEXT_RECEIVED_OPTIONS_FILTERED_DUE_TO_ERROR == static_cast<uint32_t>(PartyChatTextReceivedOptions::FilteredDueToError));

PARTY_C_ASSERT(PARTY_REGION_UPDATE_MODE_IMMEDIATE == static_cast<uint32_t>(PartyRegionUpdateMode::Immediate));
PARTY_C_ASSERT(PARTY_REGION_UPDATE_MODE_DEFERRED == static_cast<uint32_t>(PartyRegionUpdateMode::Deferred));

PARTY_C_ASSERT(PARTY_ENDPOINT_USER_TYPE_FILTER_NO_USER_FILTER == static_cast<uint32_t>(PartyEndpointUserTypeFilter::NoUserFilter));
PARTY_C_ASSERT(PARTY_ENDPOINT_USER_TYPE_FILTER_PLAYERS == static_cast<uint32_t>(PartyEndpointUserTypeFilter::Players));
PARTY_C_ASSERT(PARTY_ENDPOINT_USER_TYPE_FILTER_SERVERS == static_cast<uint32_t>(PartyEndpointUserTypeFilter::Servers));

PARTY_C_ASSERT(PARTY_ENDPOINT_LOCATION_FILTER_NO_LOCATION_FILTER == static_cast<uint32_t>(PartyEndpointLocationFilter::NoLocationFilter));
PARTY_C_ASSERT(PARTY_ENDPOINT_LOCATION_FILTER_LOCAL_ONLY == static_cast<uint32_t>(PartyEndpointLocationFilter::LocalOnly));
PARTY_C_ASSERT(PARTY_ENDPOINT_LOCATION_FILTER_REMOTE_ONLY == static_cast<uint32_t>(PartyEndpointLocationFilter::RemoteOnly));

PARTY_C_ASSERT(sizeof(PARTY_INITIALIZATION_CONFIGURATION) == sizeof(PartyInitializationConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_INITIALIZATION_CONFIGURATION::titleId) == sizeof(PartyInitializationConfiguration::titleId));
PARTY_C_ASSERT(offsetof(PARTY_INITIALIZATION_CONFIGURATION, titleId) == offsetof(PartyInitializationConfiguration, titleId));
PARTY_C_ASSERT(sizeof(PARTY_INITIALIZATION_CONFIGURATION::audioTaskQueue) == sizeof(PartyInitializationConfiguration::audioTaskQueue));
PARTY_C_ASSERT(offsetof(PARTY_INITIALIZATION_CONFIGURATION, audioTaskQueue) == offsetof(PartyInitializationConfiguration, audioTaskQueue));
PARTY_C_ASSERT(sizeof(PARTY_INITIALIZATION_CONFIGURATION::networkingTaskQueue) == sizeof(PartyInitializationConfiguration::networkingTaskQueue));
PARTY_C_ASSERT(offsetof(PARTY_INITIALIZATION_CONFIGURATION, networkingTaskQueue) == offsetof(PartyInitializationConfiguration, networkingTaskQueue));

PARTY_C_ASSERT(sizeof(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_CONFIGURATION) == sizeof(PartyLocalUdpSocketBindAddressConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_CONFIGURATION::options) == sizeof(PartyLocalUdpSocketBindAddressConfiguration::options));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_CONFIGURATION, options) == offsetof(PartyLocalUdpSocketBindAddressConfiguration, options));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_CONFIGURATION::port) == sizeof(PartyLocalUdpSocketBindAddressConfiguration::port));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_UDP_SOCKET_BIND_ADDRESS_CONFIGURATION, port) == offsetof(PartyLocalUdpSocketBindAddressConfiguration, port));

PARTY_C_ASSERT(sizeof(PARTY_REGION_UPDATE_CONFIGURATION) == sizeof(PartyRegionUpdateConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_REGION_UPDATE_CONFIGURATION::mode) == sizeof(PartyRegionUpdateConfiguration::mode));
PARTY_C_ASSERT(offsetof(PARTY_REGION_UPDATE_CONFIGURATION, mode) == offsetof(PartyRegionUpdateConfiguration, mode));
PARTY_C_ASSERT(sizeof(PARTY_REGION_UPDATE_CONFIGURATION::refreshIntervalInSeconds) == sizeof(PartyRegionUpdateConfiguration::refreshIntervalInSeconds));
PARTY_C_ASSERT(offsetof(PARTY_REGION_UPDATE_CONFIGURATION, refreshIntervalInSeconds) == offsetof(PartyRegionUpdateConfiguration, refreshIntervalInSeconds));

PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION) == sizeof(PartyRegionQualityMeasurementConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION::totalMeasurementTimeoutInMilliseconds) == sizeof(PartyRegionQualityMeasurementConfiguration::totalMeasurementTimeoutInMilliseconds));
PARTY_C_ASSERT(offsetof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION, totalMeasurementTimeoutInMilliseconds) == offsetof(PartyRegionQualityMeasurementConfiguration, totalMeasurementTimeoutInMilliseconds));
PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION::highLatencyHintInMilliseconds) == sizeof(PartyRegionQualityMeasurementConfiguration::highLatencyHintInMilliseconds));
PARTY_C_ASSERT(offsetof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION, highLatencyHintInMilliseconds) == offsetof(PartyRegionQualityMeasurementConfiguration, highLatencyHintInMilliseconds));
PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION::minRequiredSuccessfulResponses) == sizeof(PartyRegionQualityMeasurementConfiguration::minRequiredSuccessfulResponses));
PARTY_C_ASSERT(offsetof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION, minRequiredSuccessfulResponses) == offsetof(PartyRegionQualityMeasurementConfiguration, minRequiredSuccessfulResponses));
PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION::idealNumberOfSuccessfulResponses) == sizeof(PartyRegionQualityMeasurementConfiguration::idealNumberOfSuccessfulResponses));
PARTY_C_ASSERT(offsetof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION, idealNumberOfSuccessfulResponses) == offsetof(PartyRegionQualityMeasurementConfiguration, idealNumberOfSuccessfulResponses));
PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION::maxRetriesWithNoResponse) == sizeof(PartyRegionQualityMeasurementConfiguration::maxRetriesWithNoResponse));
PARTY_C_ASSERT(offsetof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION, maxRetriesWithNoResponse) == offsetof(PartyRegionQualityMeasurementConfiguration, maxRetriesWithNoResponse));
PARTY_C_ASSERT(sizeof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION::maxTimeoutsAfterResponse) == sizeof(PartyRegionQualityMeasurementConfiguration::maxTimeoutsAfterResponse));
PARTY_C_ASSERT(offsetof(PARTY_REGION_QUALITY_MEASUREMENT_CONFIGURATION, maxTimeoutsAfterResponse) == offsetof(PartyRegionQualityMeasurementConfiguration, maxTimeoutsAfterResponse));

PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESCRIPTOR) == sizeof(PartyNetworkDescriptor));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESCRIPTOR::networkIdentifier) == sizeof(PartyNetworkDescriptor::networkIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESCRIPTOR, networkIdentifier) == offsetof(PartyNetworkDescriptor, networkIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESCRIPTOR::regionName) == sizeof(PartyNetworkDescriptor::regionName));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESCRIPTOR, regionName) == offsetof(PartyNetworkDescriptor, regionName));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESCRIPTOR::opaqueConnectionInformation) == sizeof(PartyNetworkDescriptor::opaqueConnectionInformation));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESCRIPTOR, opaqueConnectionInformation) == offsetof(PartyNetworkDescriptor, opaqueConnectionInformation));

PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION) == sizeof(PartyNetworkConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION::maxUserCount) == sizeof(PartyNetworkConfiguration::maxUserCount));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION, maxUserCount) == offsetof(PartyNetworkConfiguration, maxUserCount));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION::maxDeviceCount) == sizeof(PartyNetworkConfiguration::maxDeviceCount));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION, maxDeviceCount) == offsetof(PartyNetworkConfiguration, maxDeviceCount));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION::maxUsersPerDeviceCount) == sizeof(PartyNetworkConfiguration::maxUsersPerDeviceCount));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION, maxUsersPerDeviceCount) == offsetof(PartyNetworkConfiguration, maxUsersPerDeviceCount));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION::maxDevicesPerUserCount) == sizeof(PartyNetworkConfiguration::maxDevicesPerUserCount));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION, maxDevicesPerUserCount) == offsetof(PartyNetworkConfiguration, maxDevicesPerUserCount));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION::maxEndpointsPerDeviceCount) == sizeof(PartyNetworkConfiguration::maxEndpointsPerDeviceCount));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION, maxEndpointsPerDeviceCount) == offsetof(PartyNetworkConfiguration, maxEndpointsPerDeviceCount));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION::directPeerConnectivityOptions) == sizeof(PartyNetworkConfiguration::directPeerConnectivityOptions));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION, directPeerConnectivityOptions) == offsetof(PartyNetworkConfiguration, directPeerConnectivityOptions));

PARTY_C_ASSERT(sizeof(PARTY_REGION) == sizeof(PartyRegion));
PARTY_C_ASSERT(sizeof(PARTY_REGION::regionName) == sizeof(PartyRegion::regionName));
PARTY_C_ASSERT(offsetof(PARTY_REGION, regionName) == offsetof(PartyRegion, regionName));
PARTY_C_ASSERT(sizeof(PARTY_REGION::roundTripLatencyInMilliseconds) == sizeof(PartyRegion::roundTripLatencyInMilliseconds));
PARTY_C_ASSERT(offsetof(PARTY_REGION, roundTripLatencyInMilliseconds) == offsetof(PartyRegion, roundTripLatencyInMilliseconds));

PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CONFIGURATION) == sizeof(PartyInvitationConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CONFIGURATION::identifier) == sizeof(PartyInvitationConfiguration::identifier));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_CONFIGURATION, identifier) == offsetof(PartyInvitationConfiguration, identifier));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CONFIGURATION::revocability) == sizeof(PartyInvitationConfiguration::revocability));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_CONFIGURATION, revocability) == offsetof(PartyInvitationConfiguration, revocability));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CONFIGURATION::entityIdCount) == sizeof(PartyInvitationConfiguration::entityIdCount));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_CONFIGURATION, entityIdCount) == offsetof(PartyInvitationConfiguration, entityIdCount));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CONFIGURATION::entityIds) == sizeof(PartyInvitationConfiguration::entityIds));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_CONFIGURATION, entityIds) == offsetof(PartyInvitationConfiguration, entityIds));

PARTY_C_ASSERT(sizeof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION) == sizeof(PartySendMessageQueuingConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION::priority) == sizeof(PartySendMessageQueuingConfiguration::priority));
PARTY_C_ASSERT(offsetof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION, priority) == offsetof(PartySendMessageQueuingConfiguration, priority));
PARTY_C_ASSERT(sizeof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION::identityForCancelFilters) == sizeof(PartySendMessageQueuingConfiguration::identityForCancelFilters));
PARTY_C_ASSERT(offsetof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION, identityForCancelFilters) == offsetof(PartySendMessageQueuingConfiguration, identityForCancelFilters));
PARTY_C_ASSERT(sizeof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION::timeoutInMilliseconds) == sizeof(PartySendMessageQueuingConfiguration::timeoutInMilliseconds));
PARTY_C_ASSERT(offsetof(PARTY_SEND_MESSAGE_QUEUING_CONFIGURATION, timeoutInMilliseconds) == offsetof(PartySendMessageQueuingConfiguration, timeoutInMilliseconds));

PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFER) == sizeof(PartyDataBuffer));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFER::buffer) == sizeof(PartyDataBuffer::buffer));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFER, buffer) == offsetof(PartyDataBuffer, buffer));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFER::bufferByteCount) == sizeof(PartyDataBuffer::bufferByteCount));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFER, bufferByteCount) == offsetof(PartyDataBuffer, bufferByteCount));

PARTY_C_ASSERT(sizeof(PARTY_MUTABLE_DATA_BUFFER) == sizeof(PartyMutableDataBuffer));
PARTY_C_ASSERT(sizeof(PARTY_MUTABLE_DATA_BUFFER::buffer) == sizeof(PartyMutableDataBuffer::buffer));
PARTY_C_ASSERT(offsetof(PARTY_MUTABLE_DATA_BUFFER, buffer) == offsetof(PartyMutableDataBuffer, buffer));
PARTY_C_ASSERT(sizeof(PARTY_MUTABLE_DATA_BUFFER::bufferByteCount) == sizeof(PartyMutableDataBuffer::bufferByteCount));
PARTY_C_ASSERT(offsetof(PARTY_MUTABLE_DATA_BUFFER, bufferByteCount) == offsetof(PartyMutableDataBuffer, bufferByteCount));

PARTY_C_ASSERT(sizeof(PARTY_TRANSLATION) == sizeof(PartyTranslation));
PARTY_C_ASSERT(sizeof(PARTY_TRANSLATION::result) == sizeof(PartyTranslation::result));
PARTY_C_ASSERT(offsetof(PARTY_TRANSLATION, result) == offsetof(PartyTranslation, result));
PARTY_C_ASSERT(sizeof(PARTY_TRANSLATION::errorDetail) == sizeof(PartyTranslation::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_TRANSLATION, errorDetail) == offsetof(PartyTranslation, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_TRANSLATION::languageCode) == sizeof(PartyTranslation::languageCode));
PARTY_C_ASSERT(offsetof(PARTY_TRANSLATION, languageCode) == offsetof(PartyTranslation, languageCode));
PARTY_C_ASSERT(sizeof(PARTY_TRANSLATION::options) == sizeof(PartyTranslation::options));
PARTY_C_ASSERT(offsetof(PARTY_TRANSLATION, options) == offsetof(PartyTranslation, options));
PARTY_C_ASSERT(sizeof(PARTY_TRANSLATION::translation) == sizeof(PartyTranslation::translation));
PARTY_C_ASSERT(offsetof(PARTY_TRANSLATION, translation) == offsetof(PartyTranslation, translation));

PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT) == sizeof(PartyAudioFormat));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT::samplesPerSecond) == sizeof(PartyAudioFormat::samplesPerSecond));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_FORMAT, samplesPerSecond) == offsetof(PartyAudioFormat, samplesPerSecond));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT::channelMask) == sizeof(PartyAudioFormat::channelMask));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_FORMAT, channelMask) == offsetof(PartyAudioFormat, channelMask));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT::channelCount) == sizeof(PartyAudioFormat::channelCount));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_FORMAT, channelCount) == offsetof(PartyAudioFormat, channelCount));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT::bitsPerSample) == sizeof(PartyAudioFormat::bitsPerSample));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_FORMAT, bitsPerSample) == offsetof(PartyAudioFormat, bitsPerSample));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT::sampleType) == sizeof(PartyAudioFormat::sampleType));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_FORMAT, sampleType) == offsetof(PartyAudioFormat, sampleType));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_FORMAT::interleaved) == sizeof(PartyAudioFormat::interleaved));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_FORMAT, interleaved) == offsetof(PartyAudioFormat, interleaved));

PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION) == sizeof(PartyAudioManipulationSourceStreamConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION::format) == sizeof(PartyAudioManipulationSourceStreamConfiguration::format));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION, format) == offsetof(PartyAudioManipulationSourceStreamConfiguration, format));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION::maxTotalAudioBufferSizeInMilliseconds) == sizeof(PartyAudioManipulationSourceStreamConfiguration::maxTotalAudioBufferSizeInMilliseconds));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_CONFIGURATION, maxTotalAudioBufferSizeInMilliseconds) == offsetof(PartyAudioManipulationSourceStreamConfiguration, maxTotalAudioBufferSizeInMilliseconds));

PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION) == sizeof(PartyAudioManipulationSinkStreamConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION::format) == sizeof(PartyAudioManipulationSinkStreamConfiguration::format));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION, format) == offsetof(PartyAudioManipulationSinkStreamConfiguration, format));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION::maxTotalAudioBufferSizeInMilliseconds) == sizeof(PartyAudioManipulationSinkStreamConfiguration::maxTotalAudioBufferSizeInMilliseconds));
PARTY_C_ASSERT(offsetof(PARTY_AUDIO_MANIPULATION_SINK_STREAM_CONFIGURATION, maxTotalAudioBufferSizeInMilliseconds) == offsetof(PartyAudioManipulationSinkStreamConfiguration, maxTotalAudioBufferSizeInMilliseconds));

PARTY_C_ASSERT(sizeof(PARTY_STATE_CHANGE) == sizeof(PartyStateChange));
PARTY_C_ASSERT(sizeof(PARTY_STATE_CHANGE::stateChangeType) == sizeof(PartyStateChange::stateChangeType));
PARTY_C_ASSERT(offsetof(PARTY_STATE_CHANGE, stateChangeType) == offsetof(PartyStateChange, stateChangeType));

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Winvalid-offsetof"
#endif // __clang__

PARTY_C_ASSERT(sizeof(PARTY_REGIONS_CHANGED_STATE_CHANGE) == sizeof(PartyRegionsChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REGIONS_CHANGED_STATE_CHANGE::result) == sizeof(PartyRegionsChangedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_REGIONS_CHANGED_STATE_CHANGE, result) == offsetof(PartyRegionsChangedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_REGIONS_CHANGED_STATE_CHANGE::errorDetail) == sizeof(PartyRegionsChangedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_REGIONS_CHANGED_STATE_CHANGE, errorDetail) == offsetof(PartyRegionsChangedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE) == sizeof(PartyCreateNewNetworkCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::result) == sizeof(PartyCreateNewNetworkCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, result) == offsetof(PartyCreateNewNetworkCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyCreateNewNetworkCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyCreateNewNetworkCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyCreateNewNetworkCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyCreateNewNetworkCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::networkConfiguration) == sizeof(PartyCreateNewNetworkCompletedStateChange::networkConfiguration));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, networkConfiguration) == offsetof(PartyCreateNewNetworkCompletedStateChange, networkConfiguration));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::regionCount) == sizeof(PartyCreateNewNetworkCompletedStateChange::regionCount));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, regionCount) == offsetof(PartyCreateNewNetworkCompletedStateChange, regionCount));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::regions) == sizeof(PartyCreateNewNetworkCompletedStateChange::regions));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, regions) == offsetof(PartyCreateNewNetworkCompletedStateChange, regions));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyCreateNewNetworkCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyCreateNewNetworkCompletedStateChange, asyncIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::networkDescriptor) == sizeof(PartyCreateNewNetworkCompletedStateChange::networkDescriptor));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, networkDescriptor) == offsetof(PartyCreateNewNetworkCompletedStateChange, networkDescriptor));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE::appliedInitialInvitationIdentifier) == sizeof(PartyCreateNewNetworkCompletedStateChange::appliedInitialInvitationIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_NEW_NETWORK_COMPLETED_STATE_CHANGE, appliedInitialInvitationIdentifier) == offsetof(PartyCreateNewNetworkCompletedStateChange, appliedInitialInvitationIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE) == sizeof(PartyConnectToNetworkCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE::result) == sizeof(PartyConnectToNetworkCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE, result) == offsetof(PartyConnectToNetworkCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyConnectToNetworkCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyConnectToNetworkCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE::networkDescriptor) == sizeof(PartyConnectToNetworkCompletedStateChange::networkDescriptor));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE, networkDescriptor) == offsetof(PartyConnectToNetworkCompletedStateChange, networkDescriptor));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyConnectToNetworkCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyConnectToNetworkCompletedStateChange, asyncIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE::network) == sizeof(PartyConnectToNetworkCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_TO_NETWORK_COMPLETED_STATE_CHANGE, network) == offsetof(PartyConnectToNetworkCompletedStateChange, network));

PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE) == sizeof(PartyAuthenticateLocalUserCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE::result) == sizeof(PartyAuthenticateLocalUserCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE, result) == offsetof(PartyAuthenticateLocalUserCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyAuthenticateLocalUserCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyAuthenticateLocalUserCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE::network) == sizeof(PartyAuthenticateLocalUserCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE, network) == offsetof(PartyAuthenticateLocalUserCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyAuthenticateLocalUserCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyAuthenticateLocalUserCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE::invitationIdentifier) == sizeof(PartyAuthenticateLocalUserCompletedStateChange::invitationIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE, invitationIdentifier) == offsetof(PartyAuthenticateLocalUserCompletedStateChange, invitationIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyAuthenticateLocalUserCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_AUTHENTICATE_LOCAL_USER_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyAuthenticateLocalUserCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION_MADE_AVAILABLE_STATE_CHANGE) == sizeof(PartyNetworkConfigurationMadeAvailableStateChange));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION_MADE_AVAILABLE_STATE_CHANGE::network) == sizeof(PartyNetworkConfigurationMadeAvailableStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION_MADE_AVAILABLE_STATE_CHANGE, network) == offsetof(PartyNetworkConfigurationMadeAvailableStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_CONFIGURATION_MADE_AVAILABLE_STATE_CHANGE::networkConfiguration) == sizeof(PartyNetworkConfigurationMadeAvailableStateChange::networkConfiguration));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_CONFIGURATION_MADE_AVAILABLE_STATE_CHANGE, networkConfiguration) == offsetof(PartyNetworkConfigurationMadeAvailableStateChange, networkConfiguration));

PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESCRIPTOR_CHANGED_STATE_CHANGE) == sizeof(PartyNetworkDescriptorChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESCRIPTOR_CHANGED_STATE_CHANGE::network) == sizeof(PartyNetworkDescriptorChangedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESCRIPTOR_CHANGED_STATE_CHANGE, network) == offsetof(PartyNetworkDescriptorChangedStateChange, network));

PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE) == sizeof(PartyLocalUserRemovedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE::network) == sizeof(PartyLocalUserRemovedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE, network) == offsetof(PartyLocalUserRemovedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE::localUser) == sizeof(PartyLocalUserRemovedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE, localUser) == offsetof(PartyLocalUserRemovedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE::removedReason) == sizeof(PartyLocalUserRemovedStateChange::removedReason));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_USER_REMOVED_STATE_CHANGE, removedReason) == offsetof(PartyLocalUserRemovedStateChange, removedReason));

PARTY_C_ASSERT(sizeof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE) == sizeof(PartyRemoveLocalUserCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE::result) == sizeof(PartyRemoveLocalUserCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE, result) == offsetof(PartyRemoveLocalUserCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyRemoveLocalUserCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyRemoveLocalUserCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE::network) == sizeof(PartyRemoveLocalUserCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE, network) == offsetof(PartyRemoveLocalUserCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyRemoveLocalUserCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyRemoveLocalUserCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyRemoveLocalUserCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_REMOVE_LOCAL_USER_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyRemoveLocalUserCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE) == sizeof(PartyDestroyLocalUserCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE::result) == sizeof(PartyDestroyLocalUserCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE, result) == offsetof(PartyDestroyLocalUserCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyDestroyLocalUserCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyDestroyLocalUserCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyDestroyLocalUserCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyDestroyLocalUserCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyDestroyLocalUserCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_LOCAL_USER_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyDestroyLocalUserCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_KICKED_STATE_CHANGE) == sizeof(PartyLocalUserKickedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_KICKED_STATE_CHANGE::network) == sizeof(PartyLocalUserKickedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_USER_KICKED_STATE_CHANGE, network) == offsetof(PartyLocalUserKickedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_KICKED_STATE_CHANGE::localUser) == sizeof(PartyLocalUserKickedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_USER_KICKED_STATE_CHANGE, localUser) == offsetof(PartyLocalUserKickedStateChange, localUser));

PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE) == sizeof(PartyCreateEndpointCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE::result) == sizeof(PartyCreateEndpointCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE, result) == offsetof(PartyCreateEndpointCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyCreateEndpointCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyCreateEndpointCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE::network) == sizeof(PartyCreateEndpointCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE, network) == offsetof(PartyCreateEndpointCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyCreateEndpointCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyCreateEndpointCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyCreateEndpointCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyCreateEndpointCompletedStateChange, asyncIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE::localEndpoint) == sizeof(PartyCreateEndpointCompletedStateChange::localEndpoint));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_ENDPOINT_COMPLETED_STATE_CHANGE, localEndpoint) == offsetof(PartyCreateEndpointCompletedStateChange, localEndpoint));

PARTY_C_ASSERT(sizeof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE) == sizeof(PartyDestroyEndpointCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE::result) == sizeof(PartyDestroyEndpointCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE, result) == offsetof(PartyDestroyEndpointCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyDestroyEndpointCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyDestroyEndpointCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE::network) == sizeof(PartyDestroyEndpointCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE, network) == offsetof(PartyDestroyEndpointCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE::localEndpoint) == sizeof(PartyDestroyEndpointCompletedStateChange::localEndpoint));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE, localEndpoint) == offsetof(PartyDestroyEndpointCompletedStateChange, localEndpoint));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyDestroyEndpointCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_ENDPOINT_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyDestroyEndpointCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_CREATED_STATE_CHANGE) == sizeof(PartyEndpointCreatedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_CREATED_STATE_CHANGE::network) == sizeof(PartyEndpointCreatedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_CREATED_STATE_CHANGE, network) == offsetof(PartyEndpointCreatedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_CREATED_STATE_CHANGE::endpoint) == sizeof(PartyEndpointCreatedStateChange::endpoint));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_CREATED_STATE_CHANGE, endpoint) == offsetof(PartyEndpointCreatedStateChange, endpoint));

PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE) == sizeof(PartyEndpointDestroyedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE::network) == sizeof(PartyEndpointDestroyedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE, network) == offsetof(PartyEndpointDestroyedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE::endpoint) == sizeof(PartyEndpointDestroyedStateChange::endpoint));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE, endpoint) == offsetof(PartyEndpointDestroyedStateChange, endpoint));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE::reason) == sizeof(PartyEndpointDestroyedStateChange::reason));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE, reason) == offsetof(PartyEndpointDestroyedStateChange, reason));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE::errorDetail) == sizeof(PartyEndpointDestroyedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_DESTROYED_STATE_CHANGE, errorDetail) == offsetof(PartyEndpointDestroyedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_CREATED_STATE_CHANGE) == sizeof(PartyRemoteDeviceCreatedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_CREATED_STATE_CHANGE::device) == sizeof(PartyRemoteDeviceCreatedStateChange::device));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_CREATED_STATE_CHANGE, device) == offsetof(PartyRemoteDeviceCreatedStateChange, device));

PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_DESTROYED_STATE_CHANGE) == sizeof(PartyRemoteDeviceDestroyedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_DESTROYED_STATE_CHANGE::device) == sizeof(PartyRemoteDeviceDestroyedStateChange::device));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_DESTROYED_STATE_CHANGE, device) == offsetof(PartyRemoteDeviceDestroyedStateChange, device));

PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_JOINED_NETWORK_STATE_CHANGE) == sizeof(PartyRemoteDeviceJoinedNetworkStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_JOINED_NETWORK_STATE_CHANGE::device) == sizeof(PartyRemoteDeviceJoinedNetworkStateChange::device));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_JOINED_NETWORK_STATE_CHANGE, device) == offsetof(PartyRemoteDeviceJoinedNetworkStateChange, device));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_JOINED_NETWORK_STATE_CHANGE::network) == sizeof(PartyRemoteDeviceJoinedNetworkStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_JOINED_NETWORK_STATE_CHANGE, network) == offsetof(PartyRemoteDeviceJoinedNetworkStateChange, network));

PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE) == sizeof(PartyRemoteDeviceLeftNetworkStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE::reason) == sizeof(PartyRemoteDeviceLeftNetworkStateChange::reason));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE, reason) == offsetof(PartyRemoteDeviceLeftNetworkStateChange, reason));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE::errorDetail) == sizeof(PartyRemoteDeviceLeftNetworkStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE, errorDetail) == offsetof(PartyRemoteDeviceLeftNetworkStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE::device) == sizeof(PartyRemoteDeviceLeftNetworkStateChange::device));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE, device) == offsetof(PartyRemoteDeviceLeftNetworkStateChange, device));
PARTY_C_ASSERT(sizeof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE::network) == sizeof(PartyRemoteDeviceLeftNetworkStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_REMOTE_DEVICE_LEFT_NETWORK_STATE_CHANGE, network) == offsetof(PartyRemoteDeviceLeftNetworkStateChange, network));

PARTY_C_ASSERT(sizeof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE) == sizeof(PartyDevicePropertiesChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE::device) == sizeof(PartyDevicePropertiesChangedStateChange::device));
PARTY_C_ASSERT(offsetof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE, device) == offsetof(PartyDevicePropertiesChangedStateChange, device));
PARTY_C_ASSERT(sizeof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE::propertyCount) == sizeof(PartyDevicePropertiesChangedStateChange::propertyCount));
PARTY_C_ASSERT(offsetof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE, propertyCount) == offsetof(PartyDevicePropertiesChangedStateChange, propertyCount));
PARTY_C_ASSERT(sizeof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE::keys) == sizeof(PartyDevicePropertiesChangedStateChange::keys));
PARTY_C_ASSERT(offsetof(PARTY_DEVICE_PROPERTIES_CHANGED_STATE_CHANGE, keys) == offsetof(PartyDevicePropertiesChangedStateChange, keys));

PARTY_C_ASSERT(sizeof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE) == sizeof(PartyLeaveNetworkCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE::result) == sizeof(PartyLeaveNetworkCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE, result) == offsetof(PartyLeaveNetworkCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyLeaveNetworkCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyLeaveNetworkCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE::network) == sizeof(PartyLeaveNetworkCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE, network) == offsetof(PartyLeaveNetworkCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyLeaveNetworkCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_LEAVE_NETWORK_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyLeaveNetworkCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESTROYED_STATE_CHANGE) == sizeof(PartyNetworkDestroyedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESTROYED_STATE_CHANGE::reason) == sizeof(PartyNetworkDestroyedStateChange::reason));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESTROYED_STATE_CHANGE, reason) == offsetof(PartyNetworkDestroyedStateChange, reason));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESTROYED_STATE_CHANGE::errorDetail) == sizeof(PartyNetworkDestroyedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESTROYED_STATE_CHANGE, errorDetail) == offsetof(PartyNetworkDestroyedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_DESTROYED_STATE_CHANGE::network) == sizeof(PartyNetworkDestroyedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_DESTROYED_STATE_CHANGE, network) == offsetof(PartyNetworkDestroyedStateChange, network));

PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE) == sizeof(PartyEndpointMessageReceivedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::network) == sizeof(PartyEndpointMessageReceivedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, network) == offsetof(PartyEndpointMessageReceivedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::senderEndpoint) == sizeof(PartyEndpointMessageReceivedStateChange::senderEndpoint));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, senderEndpoint) == offsetof(PartyEndpointMessageReceivedStateChange, senderEndpoint));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::receiverEndpointCount) == sizeof(PartyEndpointMessageReceivedStateChange::receiverEndpointCount));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, receiverEndpointCount) == offsetof(PartyEndpointMessageReceivedStateChange, receiverEndpointCount));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::receiverEndpoints) == sizeof(PartyEndpointMessageReceivedStateChange::receiverEndpoints));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, receiverEndpoints) == offsetof(PartyEndpointMessageReceivedStateChange, receiverEndpoints));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::options) == sizeof(PartyEndpointMessageReceivedStateChange::options));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, options) == offsetof(PartyEndpointMessageReceivedStateChange, options));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::messageSize) == sizeof(PartyEndpointMessageReceivedStateChange::messageSize));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, messageSize) == offsetof(PartyEndpointMessageReceivedStateChange, messageSize));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE::messageBuffer) == sizeof(PartyEndpointMessageReceivedStateChange::messageBuffer));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_MESSAGE_RECEIVED_STATE_CHANGE, messageBuffer) == offsetof(PartyEndpointMessageReceivedStateChange, messageBuffer));

PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE) == sizeof(PartyDataBuffersReturnedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE::network) == sizeof(PartyDataBuffersReturnedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE, network) == offsetof(PartyDataBuffersReturnedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE::localSenderEndpoint) == sizeof(PartyDataBuffersReturnedStateChange::localSenderEndpoint));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE, localSenderEndpoint) == offsetof(PartyDataBuffersReturnedStateChange, localSenderEndpoint));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE::dataBufferCount) == sizeof(PartyDataBuffersReturnedStateChange::dataBufferCount));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE, dataBufferCount) == offsetof(PartyDataBuffersReturnedStateChange, dataBufferCount));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE::dataBuffers) == sizeof(PartyDataBuffersReturnedStateChange::dataBuffers));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE, dataBuffers) == offsetof(PartyDataBuffersReturnedStateChange, dataBuffers));
PARTY_C_ASSERT(sizeof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE::messageIdentifier) == sizeof(PartyDataBuffersReturnedStateChange::messageIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_DATA_BUFFERS_RETURNED_STATE_CHANGE, messageIdentifier) == offsetof(PartyDataBuffersReturnedStateChange, messageIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE) == sizeof(PartyEndpointPropertiesChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE::endpoint) == sizeof(PartyEndpointPropertiesChangedStateChange::endpoint));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE, endpoint) == offsetof(PartyEndpointPropertiesChangedStateChange, endpoint));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE::propertyCount) == sizeof(PartyEndpointPropertiesChangedStateChange::propertyCount));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE, propertyCount) == offsetof(PartyEndpointPropertiesChangedStateChange, propertyCount));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE::keys) == sizeof(PartyEndpointPropertiesChangedStateChange::keys));
PARTY_C_ASSERT(offsetof(PARTY_ENDPOINT_PROPERTIES_CHANGED_STATE_CHANGE, keys) == offsetof(PartyEndpointPropertiesChangedStateChange, keys));

PARTY_C_ASSERT(sizeof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE) == sizeof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE::endpointCount) == sizeof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange::endpointCount));
PARTY_C_ASSERT(offsetof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE, endpointCount) == offsetof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange, endpointCount));
PARTY_C_ASSERT(sizeof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE::endpoints) == sizeof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange::endpoints));
PARTY_C_ASSERT(offsetof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE, endpoints) == offsetof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange, endpoints));
PARTY_C_ASSERT(sizeof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE::options) == sizeof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange::options));
PARTY_C_ASSERT(offsetof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE, options) == offsetof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange, options));
PARTY_C_ASSERT(sizeof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SYNCHRONIZE_MESSAGES_BETWEEN_ENDPOINTS_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySynchronizeMessagesBetweenEndpointsCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE) == sizeof(PartyCreateInvitationCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE::result) == sizeof(PartyCreateInvitationCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE, result) == offsetof(PartyCreateInvitationCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyCreateInvitationCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyCreateInvitationCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE::network) == sizeof(PartyCreateInvitationCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE, network) == offsetof(PartyCreateInvitationCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyCreateInvitationCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyCreateInvitationCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyCreateInvitationCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyCreateInvitationCompletedStateChange, asyncIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE::invitation) == sizeof(PartyCreateInvitationCompletedStateChange::invitation));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_INVITATION_COMPLETED_STATE_CHANGE, invitation) == offsetof(PartyCreateInvitationCompletedStateChange, invitation));

PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE) == sizeof(PartyRevokeInvitationCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE::result) == sizeof(PartyRevokeInvitationCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE, result) == offsetof(PartyRevokeInvitationCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyRevokeInvitationCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyRevokeInvitationCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE::network) == sizeof(PartyRevokeInvitationCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE, network) == offsetof(PartyRevokeInvitationCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyRevokeInvitationCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyRevokeInvitationCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE::invitation) == sizeof(PartyRevokeInvitationCompletedStateChange::invitation));
PARTY_C_ASSERT(offsetof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE, invitation) == offsetof(PartyRevokeInvitationCompletedStateChange, invitation));
PARTY_C_ASSERT(sizeof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyRevokeInvitationCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_REVOKE_INVITATION_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyRevokeInvitationCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CREATED_STATE_CHANGE) == sizeof(PartyInvitationCreatedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CREATED_STATE_CHANGE::network) == sizeof(PartyInvitationCreatedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_CREATED_STATE_CHANGE, network) == offsetof(PartyInvitationCreatedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_CREATED_STATE_CHANGE::invitation) == sizeof(PartyInvitationCreatedStateChange::invitation));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_CREATED_STATE_CHANGE, invitation) == offsetof(PartyInvitationCreatedStateChange, invitation));

PARTY_C_ASSERT(sizeof(PARTY_INVITATION_DESTROYED_STATE_CHANGE) == sizeof(PartyInvitationDestroyedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_DESTROYED_STATE_CHANGE::network) == sizeof(PartyInvitationDestroyedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_DESTROYED_STATE_CHANGE, network) == offsetof(PartyInvitationDestroyedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_DESTROYED_STATE_CHANGE::invitation) == sizeof(PartyInvitationDestroyedStateChange::invitation));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_DESTROYED_STATE_CHANGE, invitation) == offsetof(PartyInvitationDestroyedStateChange, invitation));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_DESTROYED_STATE_CHANGE::reason) == sizeof(PartyInvitationDestroyedStateChange::reason));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_DESTROYED_STATE_CHANGE, reason) == offsetof(PartyInvitationDestroyedStateChange, reason));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_DESTROYED_STATE_CHANGE::errorDetail) == sizeof(PartyInvitationDestroyedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_INVITATION_DESTROYED_STATE_CHANGE, errorDetail) == offsetof(PartyInvitationDestroyedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE) == sizeof(PartyNetworkPropertiesChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE::network) == sizeof(PartyNetworkPropertiesChangedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE, network) == offsetof(PartyNetworkPropertiesChangedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE::propertyCount) == sizeof(PartyNetworkPropertiesChangedStateChange::propertyCount));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE, propertyCount) == offsetof(PartyNetworkPropertiesChangedStateChange, propertyCount));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE::keys) == sizeof(PartyNetworkPropertiesChangedStateChange::keys));
PARTY_C_ASSERT(offsetof(PARTY_NETWORK_PROPERTIES_CHANGED_STATE_CHANGE, keys) == offsetof(PartyNetworkPropertiesChangedStateChange, keys));

PARTY_C_ASSERT(sizeof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE) == sizeof(PartyKickDeviceCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE::result) == sizeof(PartyKickDeviceCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE, result) == offsetof(PartyKickDeviceCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyKickDeviceCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyKickDeviceCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE::network) == sizeof(PartyKickDeviceCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE, network) == offsetof(PartyKickDeviceCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE::kickedDevice) == sizeof(PartyKickDeviceCompletedStateChange::kickedDevice));
PARTY_C_ASSERT(offsetof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE, kickedDevice) == offsetof(PartyKickDeviceCompletedStateChange, kickedDevice));
PARTY_C_ASSERT(sizeof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyKickDeviceCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_KICK_DEVICE_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyKickDeviceCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE) == sizeof(PartyKickUserCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE::result) == sizeof(PartyKickUserCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE, result) == offsetof(PartyKickUserCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyKickUserCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyKickUserCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE::network) == sizeof(PartyKickUserCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE, network) == offsetof(PartyKickUserCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE::kickedEntityId) == sizeof(PartyKickUserCompletedStateChange::kickedEntityId));
PARTY_C_ASSERT(offsetof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE, kickedEntityId) == offsetof(PartyKickUserCompletedStateChange, kickedEntityId));
PARTY_C_ASSERT(sizeof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyKickUserCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_KICK_USER_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyKickUserCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE) == sizeof(PartyCreateChatControlCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::result) == sizeof(PartyCreateChatControlCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, result) == offsetof(PartyCreateChatControlCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyCreateChatControlCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyCreateChatControlCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localDevice) == sizeof(PartyCreateChatControlCompletedStateChange::localDevice));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localDevice) == offsetof(PartyCreateChatControlCompletedStateChange, localDevice));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localUser) == sizeof(PartyCreateChatControlCompletedStateChange::localUser));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localUser) == offsetof(PartyCreateChatControlCompletedStateChange, localUser));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::languageCode) == sizeof(PartyCreateChatControlCompletedStateChange::languageCode));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, languageCode) == offsetof(PartyCreateChatControlCompletedStateChange, languageCode));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyCreateChatControlCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyCreateChatControlCompletedStateChange, asyncIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyCreateChatControlCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_CREATE_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyCreateChatControlCompletedStateChange, localChatControl));

PARTY_C_ASSERT(sizeof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE) == sizeof(PartyDestroyChatControlCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE::result) == sizeof(PartyDestroyChatControlCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE, result) == offsetof(PartyDestroyChatControlCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyDestroyChatControlCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyDestroyChatControlCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localDevice) == sizeof(PartyDestroyChatControlCompletedStateChange::localDevice));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localDevice) == offsetof(PartyDestroyChatControlCompletedStateChange, localDevice));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyDestroyChatControlCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyDestroyChatControlCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyDestroyChatControlCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_DESTROY_CHAT_CONTROL_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyDestroyChatControlCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_CREATED_STATE_CHANGE) == sizeof(PartyChatControlCreatedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_CREATED_STATE_CHANGE::chatControl) == sizeof(PartyChatControlCreatedStateChange::chatControl));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_CREATED_STATE_CHANGE, chatControl) == offsetof(PartyChatControlCreatedStateChange, chatControl));

PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE) == sizeof(PartyChatControlDestroyedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE::chatControl) == sizeof(PartyChatControlDestroyedStateChange::chatControl));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE, chatControl) == offsetof(PartyChatControlDestroyedStateChange, chatControl));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE::reason) == sizeof(PartyChatControlDestroyedStateChange::reason));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE, reason) == offsetof(PartyChatControlDestroyedStateChange, reason));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE::errorDetail) == sizeof(PartyChatControlDestroyedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_DESTROYED_STATE_CHANGE, errorDetail) == offsetof(PartyChatControlDestroyedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE) == sizeof(PartySetChatAudioEncoderBitrateCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetChatAudioEncoderBitrateCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetChatAudioEncoderBitrateCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetChatAudioEncoderBitrateCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetChatAudioEncoderBitrateCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetChatAudioEncoderBitrateCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetChatAudioEncoderBitrateCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE::bitrate) == sizeof(PartySetChatAudioEncoderBitrateCompletedStateChange::bitrate));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE, bitrate) == offsetof(PartySetChatAudioEncoderBitrateCompletedStateChange, bitrate));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetChatAudioEncoderBitrateCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_ENCODER_BITRATE_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetChatAudioEncoderBitrateCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE) == sizeof(PartyChatTextReceivedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::senderChatControl) == sizeof(PartyChatTextReceivedStateChange::senderChatControl));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, senderChatControl) == offsetof(PartyChatTextReceivedStateChange, senderChatControl));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::receiverChatControlCount) == sizeof(PartyChatTextReceivedStateChange::receiverChatControlCount));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, receiverChatControlCount) == offsetof(PartyChatTextReceivedStateChange, receiverChatControlCount));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::receiverChatControls) == sizeof(PartyChatTextReceivedStateChange::receiverChatControls));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, receiverChatControls) == offsetof(PartyChatTextReceivedStateChange, receiverChatControls));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::languageCode) == sizeof(PartyChatTextReceivedStateChange::languageCode));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, languageCode) == offsetof(PartyChatTextReceivedStateChange, languageCode));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::chatText) == sizeof(PartyChatTextReceivedStateChange::chatText));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, chatText) == offsetof(PartyChatTextReceivedStateChange, chatText));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::dataSize) == sizeof(PartyChatTextReceivedStateChange::dataSize));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, dataSize) == offsetof(PartyChatTextReceivedStateChange, dataSize));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::data) == sizeof(PartyChatTextReceivedStateChange::data));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, data) == offsetof(PartyChatTextReceivedStateChange, data));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::translationCount) == sizeof(PartyChatTextReceivedStateChange::translationCount));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, translationCount) == offsetof(PartyChatTextReceivedStateChange, translationCount));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::translations) == sizeof(PartyChatTextReceivedStateChange::translations));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, translations) == offsetof(PartyChatTextReceivedStateChange, translations));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::options) == sizeof(PartyChatTextReceivedStateChange::options));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, options) == offsetof(PartyChatTextReceivedStateChange, options));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::originalChatText) == sizeof(PartyChatTextReceivedStateChange::originalChatText));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, originalChatText) == offsetof(PartyChatTextReceivedStateChange, originalChatText));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE::errorDetail) == sizeof(PartyChatTextReceivedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_TEXT_RECEIVED_STATE_CHANGE, errorDetail) == offsetof(PartyChatTextReceivedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::result) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, result) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::errorDetail) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, errorDetail) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::senderChatControl) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::senderChatControl));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, senderChatControl) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, senderChatControl));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::receiverChatControlCount) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::receiverChatControlCount));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, receiverChatControlCount) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, receiverChatControlCount));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::receiverChatControls) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::receiverChatControls));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, receiverChatControls) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, receiverChatControls));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::sourceType) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::sourceType));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, sourceType) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, sourceType));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::languageCode) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::languageCode));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, languageCode) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, languageCode));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::transcription) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::transcription));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, transcription) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, transcription));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::type) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::type));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, type) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, type));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::translationCount) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::translationCount));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, translationCount) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, translationCount));
PARTY_C_ASSERT(sizeof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE::translations) == sizeof(PartyVoiceChatTranscriptionReceivedStateChange::translations));
PARTY_C_ASSERT(offsetof(PARTY_VOICE_CHAT_TRANSCRIPTION_RECEIVED_STATE_CHANGE, translations) == offsetof(PartyVoiceChatTranscriptionReceivedStateChange, translations));

PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE) == sizeof(PartySetChatAudioInputCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetChatAudioInputCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetChatAudioInputCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetChatAudioInputCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetChatAudioInputCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetChatAudioInputCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetChatAudioInputCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE::audioDeviceSelectionType) == sizeof(PartySetChatAudioInputCompletedStateChange::audioDeviceSelectionType));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE, audioDeviceSelectionType) == offsetof(PartySetChatAudioInputCompletedStateChange, audioDeviceSelectionType));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE::audioDeviceSelectionContext) == sizeof(PartySetChatAudioInputCompletedStateChange::audioDeviceSelectionContext));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE, audioDeviceSelectionContext) == offsetof(PartySetChatAudioInputCompletedStateChange, audioDeviceSelectionContext));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetChatAudioInputCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_INPUT_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetChatAudioInputCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE) == sizeof(PartySetChatAudioOutputCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetChatAudioOutputCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetChatAudioOutputCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetChatAudioOutputCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetChatAudioOutputCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetChatAudioOutputCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetChatAudioOutputCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE::audioDeviceSelectionType) == sizeof(PartySetChatAudioOutputCompletedStateChange::audioDeviceSelectionType));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE, audioDeviceSelectionType) == offsetof(PartySetChatAudioOutputCompletedStateChange, audioDeviceSelectionType));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE::audioDeviceSelectionContext) == sizeof(PartySetChatAudioOutputCompletedStateChange::audioDeviceSelectionContext));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE, audioDeviceSelectionContext) == offsetof(PartySetChatAudioOutputCompletedStateChange, audioDeviceSelectionContext));
PARTY_C_ASSERT(sizeof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetChatAudioOutputCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_CHAT_AUDIO_OUTPUT_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetChatAudioOutputCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE) == sizeof(PartyLocalChatAudioInputChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE::localChatControl) == sizeof(PartyLocalChatAudioInputChangedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE, localChatControl) == offsetof(PartyLocalChatAudioInputChangedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE::state) == sizeof(PartyLocalChatAudioInputChangedStateChange::state));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE, state) == offsetof(PartyLocalChatAudioInputChangedStateChange, state));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE::errorDetail) == sizeof(PartyLocalChatAudioInputChangedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_CHAT_AUDIO_INPUT_CHANGED_STATE_CHANGE, errorDetail) == offsetof(PartyLocalChatAudioInputChangedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE) == sizeof(PartyLocalChatAudioOutputChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE::localChatControl) == sizeof(PartyLocalChatAudioOutputChangedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE, localChatControl) == offsetof(PartyLocalChatAudioOutputChangedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE::state) == sizeof(PartyLocalChatAudioOutputChangedStateChange::state));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE, state) == offsetof(PartyLocalChatAudioOutputChangedStateChange, state));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE::errorDetail) == sizeof(PartyLocalChatAudioOutputChangedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_LOCAL_CHAT_AUDIO_OUTPUT_CHANGED_STATE_CHANGE, errorDetail) == offsetof(PartyLocalChatAudioOutputChangedStateChange, errorDetail));

PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE) == sizeof(PartySetTextToSpeechProfileCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetTextToSpeechProfileCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetTextToSpeechProfileCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetTextToSpeechProfileCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetTextToSpeechProfileCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetTextToSpeechProfileCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetTextToSpeechProfileCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE::type) == sizeof(PartySetTextToSpeechProfileCompletedStateChange::type));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE, type) == offsetof(PartySetTextToSpeechProfileCompletedStateChange, type));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE::profileIdentifier) == sizeof(PartySetTextToSpeechProfileCompletedStateChange::profileIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE, profileIdentifier) == offsetof(PartySetTextToSpeechProfileCompletedStateChange, profileIdentifier));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetTextToSpeechProfileCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_TO_SPEECH_PROFILE_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetTextToSpeechProfileCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE::result) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE, result) == offsetof(PartySynthesizeTextToSpeechCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySynthesizeTextToSpeechCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySynthesizeTextToSpeechCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE::type) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange::type));
PARTY_C_ASSERT(offsetof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE, type) == offsetof(PartySynthesizeTextToSpeechCompletedStateChange, type));
PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE::textToSynthesize) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange::textToSynthesize));
PARTY_C_ASSERT(offsetof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE, textToSynthesize) == offsetof(PartySynthesizeTextToSpeechCompletedStateChange, textToSynthesize));
PARTY_C_ASSERT(sizeof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySynthesizeTextToSpeechCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SYNTHESIZE_TEXT_TO_SPEECH_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySynthesizeTextToSpeechCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE) == sizeof(PartySetLanguageCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetLanguageCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetLanguageCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetLanguageCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetLanguageCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetLanguageCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetLanguageCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE::languageCode) == sizeof(PartySetLanguageCompletedStateChange::languageCode));
PARTY_C_ASSERT(offsetof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE, languageCode) == offsetof(PartySetLanguageCompletedStateChange, languageCode));
PARTY_C_ASSERT(sizeof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetLanguageCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_LANGUAGE_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetLanguageCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE) == sizeof(PartySetTranscriptionOptionsCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetTranscriptionOptionsCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetTranscriptionOptionsCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetTranscriptionOptionsCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetTranscriptionOptionsCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetTranscriptionOptionsCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetTranscriptionOptionsCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE::options) == sizeof(PartySetTranscriptionOptionsCompletedStateChange::options));
PARTY_C_ASSERT(offsetof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE, options) == offsetof(PartySetTranscriptionOptionsCompletedStateChange, options));
PARTY_C_ASSERT(sizeof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetTranscriptionOptionsCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_TRANSCRIPTION_OPTIONS_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetTranscriptionOptionsCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE) == sizeof(PartySetTextChatOptionsCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE::result) == sizeof(PartySetTextChatOptionsCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE, result) == offsetof(PartySetTextChatOptionsCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartySetTextChatOptionsCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartySetTextChatOptionsCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartySetTextChatOptionsCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartySetTextChatOptionsCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE::options) == sizeof(PartySetTextChatOptionsCompletedStateChange::options));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE, options) == offsetof(PartySetTextChatOptionsCompletedStateChange, options));
PARTY_C_ASSERT(sizeof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartySetTextChatOptionsCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_SET_TEXT_CHAT_OPTIONS_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartySetTextChatOptionsCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE) == sizeof(PartyChatControlPropertiesChangedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE::chatControl) == sizeof(PartyChatControlPropertiesChangedStateChange::chatControl));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE, chatControl) == offsetof(PartyChatControlPropertiesChangedStateChange, chatControl));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE::propertyCount) == sizeof(PartyChatControlPropertiesChangedStateChange::propertyCount));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE, propertyCount) == offsetof(PartyChatControlPropertiesChangedStateChange, propertyCount));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE::keys) == sizeof(PartyChatControlPropertiesChangedStateChange::keys));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_PROPERTIES_CHANGED_STATE_CHANGE, keys) == offsetof(PartyChatControlPropertiesChangedStateChange, keys));

PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_JOINED_NETWORK_STATE_CHANGE) == sizeof(PartyChatControlJoinedNetworkStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_JOINED_NETWORK_STATE_CHANGE::network) == sizeof(PartyChatControlJoinedNetworkStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_JOINED_NETWORK_STATE_CHANGE, network) == offsetof(PartyChatControlJoinedNetworkStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_JOINED_NETWORK_STATE_CHANGE::chatControl) == sizeof(PartyChatControlJoinedNetworkStateChange::chatControl));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_JOINED_NETWORK_STATE_CHANGE, chatControl) == offsetof(PartyChatControlJoinedNetworkStateChange, chatControl));

PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE) == sizeof(PartyChatControlLeftNetworkStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE::reason) == sizeof(PartyChatControlLeftNetworkStateChange::reason));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE, reason) == offsetof(PartyChatControlLeftNetworkStateChange, reason));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE::errorDetail) == sizeof(PartyChatControlLeftNetworkStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE, errorDetail) == offsetof(PartyChatControlLeftNetworkStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE::network) == sizeof(PartyChatControlLeftNetworkStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE, network) == offsetof(PartyChatControlLeftNetworkStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE::chatControl) == sizeof(PartyChatControlLeftNetworkStateChange::chatControl));
PARTY_C_ASSERT(offsetof(PARTY_CHAT_CONTROL_LEFT_NETWORK_STATE_CHANGE, chatControl) == offsetof(PartyChatControlLeftNetworkStateChange, chatControl));

PARTY_C_ASSERT(sizeof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE) == sizeof(PartyConnectChatControlCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::result) == sizeof(PartyConnectChatControlCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, result) == offsetof(PartyConnectChatControlCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyConnectChatControlCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyConnectChatControlCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::network) == sizeof(PartyConnectChatControlCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, network) == offsetof(PartyConnectChatControlCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyConnectChatControlCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyConnectChatControlCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyConnectChatControlCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyConnectChatControlCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE) == sizeof(PartyDisconnectChatControlCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::result) == sizeof(PartyDisconnectChatControlCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, result) == offsetof(PartyDisconnectChatControlCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyDisconnectChatControlCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyDisconnectChatControlCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::network) == sizeof(PartyDisconnectChatControlCompletedStateChange::network));
PARTY_C_ASSERT(offsetof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, network) == offsetof(PartyDisconnectChatControlCompletedStateChange, network));
PARTY_C_ASSERT(sizeof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyDisconnectChatControlCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyDisconnectChatControlCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyDisconnectChatControlCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_DISCONNECT_CHAT_CONTROL_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyDisconnectChatControlCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE) == sizeof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE::result) == sizeof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE, result) == offsetof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_POPULATE_AVAILABLE_TEXT_TO_SPEECH_PROFILES_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyPopulateAvailableTextToSpeechProfilesCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE) == sizeof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE::result) == sizeof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE, result) == offsetof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE::chatControl) == sizeof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange::chatControl));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE, chatControl) == offsetof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange, chatControl));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE::configuration) == sizeof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange::configuration));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE, configuration) == offsetof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange, configuration));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_VOICE_STREAM_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyConfigureAudioManipulationVoiceStreamCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE) == sizeof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE::result) == sizeof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE, result) == offsetof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE::configuration) == sizeof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange::configuration));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE, configuration) == offsetof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange, configuration));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_CAPTURE_STREAM_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyConfigureAudioManipulationCaptureStreamCompletedStateChange, asyncIdentifier));

PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE) == sizeof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE::result) == sizeof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange::result));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE, result) == offsetof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange, result));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE::errorDetail) == sizeof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange::errorDetail));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE, errorDetail) == offsetof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange, errorDetail));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE::localChatControl) == sizeof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange::localChatControl));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE, localChatControl) == offsetof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange, localChatControl));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE::configuration) == sizeof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange::configuration));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE, configuration) == offsetof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange, configuration));
PARTY_C_ASSERT(sizeof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE::asyncIdentifier) == sizeof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange::asyncIdentifier));
PARTY_C_ASSERT(offsetof(PARTY_CONFIGURE_AUDIO_MANIPULATION_RENDER_STREAM_COMPLETED_STATE_CHANGE, asyncIdentifier) == offsetof(PartyConfigureAudioManipulationRenderStreamCompletedStateChange, asyncIdentifier));

#ifdef __clang__
#pragma clang diagnostic pop
#endif // __clang__

PARTY_C_ASSERT(sizeof(PARTY_PROFILING_METHOD_ENTRANCE_EVENT_DATA) == sizeof(PartyProfilingMethodEntranceEventData));
PARTY_C_ASSERT(sizeof(PARTY_PROFILING_METHOD_ENTRANCE_EVENT_DATA::methodName) == sizeof(PartyProfilingMethodEntranceEventData::methodName));
PARTY_C_ASSERT(offsetof(PARTY_PROFILING_METHOD_ENTRANCE_EVENT_DATA, methodName) == offsetof(PartyProfilingMethodEntranceEventData, methodName));

PARTY_C_ASSERT(sizeof(PARTY_PROFILING_METHOD_EXIT_EVENT_DATA) == sizeof(PartyProfilingMethodExitEventData));
PARTY_C_ASSERT(sizeof(PARTY_PROFILING_METHOD_EXIT_EVENT_DATA::methodName) == sizeof(PartyProfilingMethodExitEventData::methodName));
PARTY_C_ASSERT(offsetof(PARTY_PROFILING_METHOD_EXIT_EVENT_DATA, methodName) == offsetof(PartyProfilingMethodExitEventData, methodName));

PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_HANDLE) == sizeof(PartyEndpoint*));
PARTY_C_ASSERT(sizeof(PARTY_ENDPOINT_HANDLE) == sizeof(PartyLocalEndpoint*));
PARTY_C_ASSERT(sizeof(PARTY_DEVICE_HANDLE) == sizeof(PartyDevice*));
PARTY_C_ASSERT(sizeof(PARTY_DEVICE_HANDLE) == sizeof(PartyLocalDevice*));
PARTY_C_ASSERT(sizeof(PARTY_INVITATION_HANDLE) == sizeof(PartyInvitation*));
PARTY_C_ASSERT(sizeof(PARTY_NETWORK_HANDLE) == sizeof(PartyNetwork*));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_HANDLE) == sizeof(PartyChatControl*));
PARTY_C_ASSERT(sizeof(PARTY_CHAT_CONTROL_HANDLE) == sizeof(PartyLocalChatControl*));
PARTY_C_ASSERT(sizeof(PARTY_LOCAL_USER_HANDLE) == sizeof(PartyLocalUser*));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SOURCE_STREAM_HANDLE) == sizeof(PartyAudioManipulationSourceStream*));
PARTY_C_ASSERT(sizeof(PARTY_AUDIO_MANIPULATION_SINK_STREAM_HANDLE) == sizeof(PartyAudioManipulationSinkStream*));

PARTY_C_ASSERT(sizeof(PartyStateChangeType) == sizeof(uint32_t));

// END GENERATED SECTION

#pragma pop_macro("PARTY_C_ASSERT")

#ifdef PARTY_SAL_SUPPORT
#pragma prefast(pop)
#endif // PARTY_SAL_SUPPORT

}; // Party
