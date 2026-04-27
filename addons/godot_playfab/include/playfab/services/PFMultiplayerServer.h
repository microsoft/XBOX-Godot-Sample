// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFMultiplayerServerTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if 0
/// <summary>
/// Deletes a multiplayer server game secret.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Deletes a multiplayer server game secret. See also MultiplayerServerListSecretSummariesAsync, MultiplayerServerUploadSecretAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS, E_PF_MULTIPLAYER_SERVER_BAD_REQUEST,
/// E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR, E_PF_MULTIPLAYER_SERVER_NO_CONTENT,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFMultiplayerServerDeleteSecretAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerDeleteSecretRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Lists details of all build aliases for a title. Accepts tokens for title and if game client access
/// is enabled, allows game client to request list of builds with player entity token.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns a list of summarized details of all multiplayer server builds for a title. See also MultiplayerServerCreateBuildWithManagedContainerAsync,
/// MultiplayerServerDeleteBuildAsync, MultiplayerServerGetBuildAsync.
///
/// When the asynchronous task is complete, call <see cref="PFMultiplayerServerListBuildAliasesGetResultSize"/>
/// and <see cref="PFMultiplayerServerListBuildAliasesGetResult"/> to get the result.
/// </remarks>
PF_API PFMultiplayerServerListBuildAliasesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerListBuildAliasesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListBuildAliases call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED, E_PF_MULTIPLAYER_SERVER_UNAVAILABLE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFMultiplayerServerListBuildAliasesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFMultiplayerServerListBuildAliasesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED, E_PF_MULTIPLAYER_SERVER_UNAVAILABLE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFMultiplayerServerListBuildAliasesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFMultiplayerServerListBuildAliasesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists summarized details of all multiplayer server builds for a title. Accepts tokens for title and
/// if game client access is enabled, allows game client to request list of builds with player entity
/// token.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns a list of summarized details of all multiplayer server builds for a title. See also MultiplayerServerCreateBuildWithManagedContainerAsync,
/// MultiplayerServerDeleteBuildAsync, MultiplayerServerGetBuildAsync.
///
/// When the asynchronous task is complete, call <see cref="PFMultiplayerServerListBuildSummariesV2GetResultSize"/>
/// and <see cref="PFMultiplayerServerListBuildSummariesV2GetResult"/> to get the result.
/// </remarks>
PF_API PFMultiplayerServerListBuildSummariesV2Async(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerListBuildSummariesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListBuildSummariesV2 call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFMultiplayerServerListBuildSummariesV2GetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFMultiplayerServerListBuildSummariesV2Async call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFMultiplayerServerListBuildSummariesV2GetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFMultiplayerServerListBuildSummariesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists quality of service servers for the title. By default, servers are only returned for regions
/// where a Multiplayer Servers build has been deployed.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns a list of quality of service servers for a title.
///
/// When the asynchronous task is complete, call <see cref="PFMultiplayerServerListQosServersForTitleGetResultSize"/>
/// and <see cref="PFMultiplayerServerListQosServersForTitleGetResult"/> to get the result.
/// </remarks>
PF_API PFMultiplayerServerListQosServersForTitleAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerListQosServersForTitleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListQosServersForTitle call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED, E_PF_MULTIPLAYER_SERVER_UNAVAILABLE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFMultiplayerServerListQosServersForTitleGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFMultiplayerServerListQosServersForTitleAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED, E_PF_MULTIPLAYER_SERVER_UNAVAILABLE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFMultiplayerServerListQosServersForTitleGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFMultiplayerServerListQosServersForTitleResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Lists multiplayer server game secrets for a title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Returns a list of multiplayer server game secrets for a title. See also MultiplayerServerDeleteSecretAsync,
/// MultiplayerServerUploadSecretAsync.
///
/// When the asynchronous task is complete, call <see cref="PFMultiplayerServerListSecretSummariesGetResultSize"/>
/// and <see cref="PFMultiplayerServerListSecretSummariesGetResult"/> to get the result.
/// </remarks>
PF_API PFMultiplayerServerListSecretSummariesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerListSecretSummariesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListSecretSummaries call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFMultiplayerServerListSecretSummariesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFMultiplayerServerListSecretSummariesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFMultiplayerServerListSecretSummariesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFMultiplayerServerListSecretSummariesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Request a multiplayer server session. Accepts tokens for title and if game client access is enabled,
/// allows game client to request a server with player entity token.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Requests a multiplayer server session from a particular build in any of the given preferred regions.
/// See also MultiplayerServerGetMultiplayerServerDetailsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFMultiplayerServerRequestMultiplayerServerGetResultSize"/>
/// and <see cref="PFMultiplayerServerRequestMultiplayerServerGetResult"/> to get the result.
/// </remarks>
PF_API PFMultiplayerServerRequestMultiplayerServerAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerRequestMultiplayerServerRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RequestMultiplayerServer call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFMultiplayerServerRequestMultiplayerServerGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFMultiplayerServerRequestMultiplayerServerAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_MULTIPLAYER_SERVER_BAD_REQUEST, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS, E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED,
/// E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFMultiplayerServerRequestMultiplayerServerGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFMultiplayerServerRequestMultiplayerServerResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Request a party session.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Requests a party session from a particular set of builds if build alias params is provided, in any
/// of the given preferred regions.
///
/// When the asynchronous task is complete, call <see cref="PFMultiplayerServerRequestPartyServiceGetResultSize"/>
/// and <see cref="PFMultiplayerServerRequestPartyServiceGetResult"/> to get the result.
/// </remarks>
PF_API PFMultiplayerServerRequestPartyServiceAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerRequestPartyServiceRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RequestPartyService call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PARTY_BAD_REQUEST,
/// E_PF_PARTY_INTERNAL_SERVER_ERROR, E_PF_PARTY_NOT_FOUND, E_PF_PARTY_REQUESTS_THROTTLED_FROM_RATE_LIMITER,
/// E_PF_PARTY_TOO_MANY_REQUESTS, E_PF_PARTY_UNAUTHORIZED, E_PF_PARTY_UNAVAILABLE or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFMultiplayerServerRequestPartyServiceGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFMultiplayerServerRequestPartyServiceAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PARTY_BAD_REQUEST,
/// E_PF_PARTY_INTERNAL_SERVER_ERROR, E_PF_PARTY_NOT_FOUND, E_PF_PARTY_REQUESTS_THROTTLED_FROM_RATE_LIMITER,
/// E_PF_PARTY_TOO_MANY_REQUESTS, E_PF_PARTY_UNAUTHORIZED, E_PF_PARTY_UNAVAILABLE or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFMultiplayerServerRequestPartyServiceGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFMultiplayerServerRequestPartyServiceResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Uploads a multiplayer server game secret.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Uploads a multiplayer server game secret. See also MultiplayerServerDeleteSecretAsync, MultiplayerServerListSecretSummariesAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS, E_PF_MULTIPLAYER_SERVER_BAD_REQUEST,
/// E_PF_MULTIPLAYER_SERVER_CONFLICT, E_PF_MULTIPLAYER_SERVER_FORBIDDEN, E_PF_MULTIPLAYER_SERVER_INTERNAL_SERVER_ERROR,
/// E_PF_MULTIPLAYER_SERVER_NO_CONTENT, E_PF_MULTIPLAYER_SERVER_NOT_FOUND, E_PF_MULTIPLAYER_SERVER_TOO_MANY_REQUESTS,
/// E_PF_MULTIPLAYER_SERVER_UNAUTHORIZED, E_PF_MULTIPLAYER_SERVER_UNAVAILABLE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFMultiplayerServerUploadSecretAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFMultiplayerServerUploadSecretRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}