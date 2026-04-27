// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFEventsTypes.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if 0
/// <summary>
/// Deletes a Data Connection from a title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also EventGetDataConnectionAsync, EventSetDataConnectionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFEventsDeleteDataConnectionGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFEventsDeleteDataConnectionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsDeleteDataConnectionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsDeleteDataConnectionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFEventsDeleteDataConnectionResponse object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVENT_SINK_NOT_FOUND,
/// E_PF_SERVICE_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFEventsDeleteDataConnectionGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEventsDeleteDataConnectionResponse* result
) noexcept;

#endif

#if 0
/// <summary>
/// Retrieves a single Data Connection associated with a title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also EventSetDataConnectionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFEventsGetDataConnectionGetResultSize"/>
/// and <see cref="PFEventsGetDataConnectionGetResult"/> to get the result.
/// </remarks>
PF_API PFEventsGetDataConnectionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsGetDataConnectionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetDataConnection call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVENT_SINK_NOT_FOUND,
/// E_PF_SERVICE_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFEventsGetDataConnectionGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsGetDataConnectionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEventsGetDataConnectionGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFEventsGetDataConnectionResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Retrieves the list of Data Connections associated with a title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also EventGetDataConnectionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFEventsListDataConnectionsGetResultSize"/>
/// and <see cref="PFEventsListDataConnectionsGetResult"/> to get the result.
/// </remarks>
PF_API PFEventsListDataConnectionsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsListDataConnectionsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListDataConnections call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFEventsListDataConnectionsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsListDataConnectionsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEventsListDataConnectionsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFEventsListDataConnectionsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Creates or updates a Data Connection on a title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also EventGetDataConnectionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFEventsSetDataConnectionGetResultSize"/>
/// and <see cref="PFEventsSetDataConnectionGetResult"/> to get the result.
/// </remarks>
PF_API PFEventsSetDataConnectionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsSetDataConnectionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a SetDataConnection call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_BILLING_INFORMATION_REQUIRED,
/// E_PF_EVENT_SINK_CONNECTION_INVALID, E_PF_EVENT_SINK_CONNECTION_UNAUTHORIZED, E_PF_EVENT_SINK_DATABASE_NOT_FOUND,
/// E_PF_EVENT_SINK_INSUFFICIENT_ROLE_ASSIGNMENT, E_PF_EVENT_SINK_LIMIT_EXCEEDED, E_PF_EVENT_SINK_REGION_INVALID,
/// E_PF_EVENT_SINK_SAS_TOKEN_INVALID, E_PF_EVENT_SINK_SAS_TOKEN_PERMISSION_INVALID, E_PF_EVENT_SINK_SECRET_INVALID,
/// E_PF_EVENT_SINK_TENANT_ID_INVALID, E_PF_EVENT_SINK_TITLE_UNAUTHORIZED, E_PF_SERVICE_UNAVAILABLE or
/// any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFEventsSetDataConnectionGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsSetDataConnectionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEventsSetDataConnectionGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFEventsSetDataConnectionResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Sets a Data Connection for the title to either the active or deactivated state.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also EventSetDataConnectionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFEventsSetDataConnectionActiveGetResultSize"/>
/// and <see cref="PFEventsSetDataConnectionActiveGetResult"/> to get the result.
/// </remarks>
PF_API PFEventsSetDataConnectionActiveAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsSetDataConnectionActiveRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a SetDataConnectionActive call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_BILLING_INFORMATION_REQUIRED,
/// E_PF_EVENT_SINK_CONNECTION_INVALID, E_PF_EVENT_SINK_CONNECTION_UNAUTHORIZED, E_PF_EVENT_SINK_DATABASE_NOT_FOUND,
/// E_PF_EVENT_SINK_INSUFFICIENT_ROLE_ASSIGNMENT, E_PF_EVENT_SINK_LIMIT_EXCEEDED, E_PF_EVENT_SINK_NOT_FOUND,
/// E_PF_EVENT_SINK_REGION_INVALID, E_PF_EVENT_SINK_SAS_TOKEN_INVALID, E_PF_EVENT_SINK_SAS_TOKEN_PERMISSION_INVALID,
/// E_PF_EVENT_SINK_SECRET_INVALID, E_PF_EVENT_SINK_TENANT_ID_INVALID, E_PF_EVENT_SINK_TITLE_UNAUTHORIZED,
/// E_PF_SERVICE_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFEventsSetDataConnectionActiveGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsSetDataConnectionActiveAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEventsSetDataConnectionActiveGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFEventsSetDataConnectionActiveResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

/// <summary>
/// Write batches of entity based events to PlayStream. The namespace of the Event must be 'custom' or
/// start with 'custom.'.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFEventsWriteEventsGetResultSize"/> and <see
/// cref="PFEventsWriteEventsGetResult"/> to get the result.
/// </remarks>
PF_API PFEventsWriteEventsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsWriteEventsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a WriteEvents call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_BROKEN,
/// E_PF_EVENT_ENTITY_NOT_ALLOWED, E_PF_EVENT_NAMESPACE_NOT_ALLOWED, E_PF_INVALID_EVENT_CONTENTS, E_PF_INVALID_JSON_CONTENT,
/// E_PF_PER_ENTITY_EVENT_RATE_LIMIT_EXCEEDED or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFEventsWriteEventsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsWriteEventsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEventsWriteEventsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFEventsWriteEventsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


/// <summary>
/// Write batches of entity based events to as Telemetry events (bypass PlayStream). The namespace must
/// be 'custom' or start with 'custom.'
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This API must be called with either X-EntityToken or X-TelemetryKey headers. If sending X-EntityToken
/// the value must be a valid EntityToken for your title. If using X-TelemetryKey the value must be a
/// Telemetry Key configured for your title set to 'Active'. If both are provided, X-TelemetryKey will
/// be ignored.
///
/// When the asynchronous task is complete, call <see cref="PFEventsWriteTelemetryEventsGetResultSize"/>
/// and <see cref="PFEventsWriteTelemetryEventsGetResult"/> to get the result.
/// </remarks>
PF_API PFEventsWriteTelemetryEventsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFEventsWriteEventsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a WriteTelemetryEvents call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_BROKEN,
/// E_PF_EVENT_ENTITY_NOT_ALLOWED, E_PF_EVENT_NAMESPACE_NOT_ALLOWED, E_PF_INVALID_JSON_CONTENT, E_PF_PER_ENTITY_EVENT_RATE_LIMIT_EXCEEDED,
/// E_PF_TELEMETRY_KEY_DEACTIVATED, E_PF_TELEMETRY_KEY_INVALID or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFEventsWriteTelemetryEventsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEventsWriteTelemetryEventsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEventsWriteTelemetryEventsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFEventsWriteEventsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}
