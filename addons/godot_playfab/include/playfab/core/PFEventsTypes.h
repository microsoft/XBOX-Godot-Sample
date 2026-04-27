// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>

#include <playfab/core/PFTypes.h>

extern "C"
{

#pragma push_macro("IN")
#undef IN

/// <summary>
/// DataConnectionErrorState enum.
/// </summary>
enum class PFEventsDataConnectionErrorState : uint32_t
{
    OK,
    Error
};

/// <summary>
/// DataConnectionType enum.
/// </summary>
enum class PFEventsDataConnectionType : uint32_t
{
    AzureBlobStorage,
    AzureDataExplorer,
    FabricKQL
};

/// <summary>
/// PFEventsDeleteDataConnectionRequest data model.
/// </summary>
typedef struct PFEventsDeleteDataConnectionRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The name of the data connection to delete.
    /// </summary>
    _Null_terminated_ const char* name;

} PFEventsDeleteDataConnectionRequest;

/// <summary>
/// PFEventsDeleteDataConnectionResponse data model.
/// </summary>
typedef struct PFEventsDeleteDataConnectionResponse
{
    /// <summary>
    /// Indicates whether or not the connection was deleted as part of the request.
    /// </summary>
    bool wasDeleted;

} PFEventsDeleteDataConnectionResponse;

/// <summary>
/// PFEventsGetDataConnectionRequest data model.
/// </summary>
typedef struct PFEventsGetDataConnectionRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The name of the data connection to retrieve.
    /// </summary>
    _Null_terminated_ const char* name;

} PFEventsGetDataConnectionRequest;

/// <summary>
/// PFEventsDataConnectionAzureBlobSettings data model.
/// </summary>
typedef struct PFEventsDataConnectionAzureBlobSettings
{
    /// <summary>
    /// (Optional) Name of the storage account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* accountName;

    /// <summary>
    /// (Optional) Name of the container.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* containerName;

    /// <summary>
    /// (Optional) Azure Entra Tenant Id.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* tenantId;

} PFEventsDataConnectionAzureBlobSettings;

/// <summary>
/// PFEventsDataConnectionAzureDataExplorerSettings data model.
/// </summary>
typedef struct PFEventsDataConnectionAzureDataExplorerSettings
{
    /// <summary>
    /// (Optional) The URI of the ADX cluster.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* clusterUri;

    /// <summary>
    /// (Optional) The database to write to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* database;

    /// <summary>
    /// (Optional) The table to write to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* table;

} PFEventsDataConnectionAzureDataExplorerSettings;

/// <summary>
/// PFEventsDataConnectionFabricKQLSettings data model.
/// </summary>
typedef struct PFEventsDataConnectionFabricKQLSettings
{
    /// <summary>
    /// (Optional) The URI of the Fabric cluster.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* clusterUri;

    /// <summary>
    /// (Optional) The database to write to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* database;

    /// <summary>
    /// (Optional) The table to write to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* table;

} PFEventsDataConnectionFabricKQLSettings;

/// <summary>
/// PFEventsDataConnectionSettings data model.
/// </summary>
typedef struct PFEventsDataConnectionSettings
{
    /// <summary>
    /// (Optional) Settings if the type of connection is AzureBlobStorage.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionAzureBlobSettings const* azureBlobSettings;

    /// <summary>
    /// (Optional) Settings if the type of connection is AzureDataExplorer.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionAzureDataExplorerSettings const* azureDataExplorerSettings;

    /// <summary>
    /// (Optional) Settings if the type of connection is FabricKQL.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionFabricKQLSettings const* azureFabricKQLSettings;

} PFEventsDataConnectionSettings;

/// <summary>
/// PFEventsDataConnectionStatusDetails data model.
/// </summary>
typedef struct PFEventsDataConnectionStatusDetails
{
    /// <summary>
    /// (Optional) The name of the error affecting the data connection, if any.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* error;

    /// <summary>
    /// (Optional) A description of the error affecting the data connection, if any. This may be empty
    /// for some errors.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* errorMessage;

    /// <summary>
    /// (Optional) The most recent time of the error affecting the data connection, if any.
    /// </summary>
    _Maybenull_ time_t const* mostRecentErrorTime;

    /// <summary>
    /// (Optional) Indicates if the connection is in a normal state or error state.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionErrorState const* state;

} PFEventsDataConnectionStatusDetails;

/// <summary>
/// PFEventsDataConnectionDetails data model.
/// </summary>
typedef struct PFEventsDataConnectionDetails
{
    /// <summary>
    /// Settings of the data connection.
    /// </summary>
    PFEventsDataConnectionSettings const* connectionSettings;

    /// <summary>
    /// Whether or not the connection is currently active.
    /// </summary>
    bool isActive;

    /// <summary>
    /// The name of the data connection.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) Current status of the data connection, if any.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionStatusDetails const* status;

    /// <summary>
    /// The type of data connection.
    /// </summary>
    PFEventsDataConnectionType type;

} PFEventsDataConnectionDetails;

/// <summary>
/// PFEventsGetDataConnectionResponse data model.
/// </summary>
typedef struct PFEventsGetDataConnectionResponse
{
    /// <summary>
    /// (Optional) The details of the queried Data Connection.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionDetails const* dataConnection;

} PFEventsGetDataConnectionResponse;

/// <summary>
/// PFEventsListDataConnectionsRequest data model.
/// </summary>
typedef struct PFEventsListDataConnectionsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

} PFEventsListDataConnectionsRequest;

/// <summary>
/// PFEventsListDataConnectionsResponse data model.
/// </summary>
typedef struct PFEventsListDataConnectionsResponse
{
    /// <summary>
    /// (Optional) The list of existing Data Connections.
    /// </summary>
    _Maybenull_ _Field_size_(dataConnectionsCount) PFEventsDataConnectionDetails const* const* dataConnections;

    /// <summary>
    /// Count of dataConnections
    /// </summary>
    uint32_t dataConnectionsCount;

} PFEventsListDataConnectionsResponse;

/// <summary>
/// PFEventsSetDataConnectionRequest data model.
/// </summary>
typedef struct PFEventsSetDataConnectionRequest
{
    /// <summary>
    /// Settings of the data connection.
    /// </summary>
    PFEventsDataConnectionSettings const* connectionSettings;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// Whether or not the connection is currently active.
    /// </summary>
    bool isActive;

    /// <summary>
    /// The name of the data connection to update or create.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// The type of data connection.
    /// </summary>
    PFEventsDataConnectionType type;

} PFEventsSetDataConnectionRequest;

/// <summary>
/// PFEventsSetDataConnectionResponse data model.
/// </summary>
typedef struct PFEventsSetDataConnectionResponse
{
    /// <summary>
    /// (Optional) The details of the Data Connection to be created or updated.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionDetails const* dataConnection;

} PFEventsSetDataConnectionResponse;

/// <summary>
/// PFEventsSetDataConnectionActiveRequest data model.
/// </summary>
typedef struct PFEventsSetDataConnectionActiveRequest
{
    /// <summary>
    /// Whether to set the data connection to active (true) or deactivated (false).
    /// </summary>
    bool active;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The name of the data connection to update.
    /// </summary>
    _Null_terminated_ const char* name;

} PFEventsSetDataConnectionActiveRequest;

/// <summary>
/// PFEventsSetDataConnectionActiveResponse data model.
/// </summary>
typedef struct PFEventsSetDataConnectionActiveResponse
{
    /// <summary>
    /// (Optional) The most current details about the data connection that was to be updated.
    /// </summary>
    _Maybenull_ PFEventsDataConnectionDetails const* dataConnection;

    /// <summary>
    /// Indicates whether or not the data connection was updated. If false, the data connection was already
    /// in the desired state.
    /// </summary>
    bool wasUpdated;

} PFEventsSetDataConnectionActiveResponse;

/// <summary>
/// PFEventsEventContents data model.
/// </summary>
typedef struct PFEventsEventContents
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the event (e.g. build number, external trace
    /// identifiers, etc.). Before an event is written, this collection and the base request custom tags
    /// will be merged, but not overriden. This enables the caller to specify static tags and per event
    /// tags.
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) Entity associated with the event. If null, the event will apply to the calling entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The namespace in which the event is defined. Allowed namespaces can vary by API.
    /// </summary>
    _Null_terminated_ const char* eventNamespace;

    /// <summary>
    /// The name of this event.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) The original unique identifier associated with this event before it was posted to
    /// PlayFab. The value might differ from the EventId value, which is assigned when the event is received
    /// by the server.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* originalId;

    /// <summary>
    /// (Optional) The time (in UTC) associated with this event when it occurred. If specified, this
    /// value is stored in the OriginalTimestamp property of the PlayStream event.
    /// </summary>
    _Maybenull_ time_t const* originalTimestamp;

    /// <summary>
    /// (Optional) Arbitrary data associated with the event. Only one of Payload or PayloadJSON is allowed.
    /// </summary>
    PFJsonObject payload;

    /// <summary>
    /// (Optional) Arbitrary data associated with the event, represented as a JSON serialized string.
    /// Only one of Payload or PayloadJSON is allowed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* payloadJSON;

} PFEventsEventContents;

/// <summary>
/// PFEventsWriteEventsRequest data model.
/// </summary>
typedef struct PFEventsWriteEventsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// The collection of events to write. Up to 200 events can be written per request.
    /// </summary>
    _Field_size_(eventsCount) PFEventsEventContents const* const* events;

    /// <summary>
    /// Count of events
    /// </summary>
    uint32_t eventsCount;

} PFEventsWriteEventsRequest;

/// <summary>
/// PFEventsWriteEventsResponse data model.
/// </summary>
typedef struct PFEventsWriteEventsResponse
{
    /// <summary>
    /// (Optional) The unique identifiers assigned by the server to the events, in the same order as
    /// the events in the request. Only returned if FlushToPlayStream option is true.
    /// </summary>
    _Maybenull_ _Field_size_(assignedEventIdsCount) const char* const* assignedEventIds;

    /// <summary>
    /// Count of assignedEventIds
    /// </summary>
    uint32_t assignedEventIdsCount;

} PFEventsWriteEventsResponse;

#pragma pop_macro("IN")

}
