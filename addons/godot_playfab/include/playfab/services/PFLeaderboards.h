// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFLeaderboardsTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Creates a new leaderboard definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also LeaderboardDeleteLeaderboardDefinitionAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_API_NOT_ENABLED_FOR_TITLE, E_PF_DUPLICATE_COLUMN_NAME_FOUND, E_PF_DUPLICATE_LINKED_STATISTIC_COLUMN_NAME_FOUND,
/// E_PF_EXTERNAL_ENTITY_NOT_ALLOWED_FOR_TIER, E_PF_INVALID_BASE_TIME_FOR_INTERVAL, E_PF_LEADERBOARD_COUNT_LIMIT_EXCEEDED,
/// E_PF_LEADERBOARD_NAME_CONFLICT, E_PF_LEADERBOARD_SIZE_LIMIT_EXCEEDED, E_PF_LINKED_STATISTIC_COLUMN_MISMATCH,
/// E_PF_LINKED_STATISTIC_COLUMN_NOT_FOUND, E_PF_LINKED_STATISTIC_COLUMN_REQUIRED, E_PF_LINKING_STATS_NOT_ALLOWED_FOR_ENTITY_TYPE,
/// E_PF_MAX_QUERYABLE_VERSIONS_VALUE_NOT_ALLOWED_FOR_TIER, E_PF_MULTIPLE_LINKED_STATISTICS_NOT_ALLOWED,
/// E_PF_STAT_DEFINITION_ALREADY_LINKED_TO_LEADERBOARD, E_PF_STATISTIC_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFLeaderboardsCreateLeaderboardDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsCreateLeaderboardDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Deletes a leaderboard definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_LEADERBOARD_DEFINITION_MODIFICATION_NOT_ALLOWED_WHILE_LINKED, E_PF_LEADERBOARD_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </remarks>
PF_API PFLeaderboardsDeleteLeaderboardDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsDeleteLeaderboardDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Deletes the specified entries from the given leaderboard.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also LeaderboardUpdateLeaderboardEntriesAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_API_NOT_ENABLED_FOR_TITLE, E_PF_LEADERBOARD_NOT_FOUND, E_PF_LEADERBOARD_UPDATE_NOT_ALLOWED_WHILE_LINKED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </remarks>
PF_API PFLeaderboardsDeleteLeaderboardEntriesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsDeleteLeaderboardEntriesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Get the friend leaderboard for the specified entity. A maximum of 25 friend entries are listed in
/// the leaderboard.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsGetFriendLeaderboardForEntityGetResultSize"/>
/// and <see cref="PFLeaderboardsGetFriendLeaderboardForEntityGetResult"/> to get the result.
/// </remarks>
PF_API PFLeaderboardsGetFriendLeaderboardForEntityAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsGetFriendLeaderboardForEntityRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetFriendLeaderboardForEntity call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_FOUND,
/// E_PF_API_NOT_ENABLED_FOR_TITLE, E_PF_DOWNSTREAM_SERVICE_UNAVAILABLE, E_PF_EXPIRED_XBOX_LIVE_TOKEN,
/// E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_ENTITY_TYPE, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_LEADERBOARD_NOT_FOUND, E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_STATISTIC_NOT_FOUND, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_SERVICE_TOO_MANY_REQUESTS, E_PF_XBOX_XASS_EXCHANGE_FAILURE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFLeaderboardsGetFriendLeaderboardForEntityGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsGetFriendLeaderboardForEntityAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_FOUND,
/// E_PF_API_NOT_ENABLED_FOR_TITLE, E_PF_DOWNSTREAM_SERVICE_UNAVAILABLE, E_PF_EXPIRED_XBOX_LIVE_TOKEN,
/// E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_ENTITY_TYPE, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_LEADERBOARD_NOT_FOUND, E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_STATISTIC_NOT_FOUND, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_SERVICE_TOO_MANY_REQUESTS, E_PF_XBOX_XASS_EXCHANGE_FAILURE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFLeaderboardsGetFriendLeaderboardForEntityGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFLeaderboardsGetEntityLeaderboardResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Get the leaderboard for a specific entity type and statistic.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsGetLeaderboardGetResultSize"/>
/// and <see cref="PFLeaderboardsGetLeaderboardGetResult"/> to get the result.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsGetEntityLeaderboardRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetLeaderboard call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_LEADERBOARD_NOT_FOUND,
/// E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_SPECIFIED_VERSION_LEADERBOARD_NOT_FOUND, E_PF_STATISTIC_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFLeaderboardsGetLeaderboardGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsGetLeaderboardAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_LEADERBOARD_NOT_FOUND,
/// E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_SPECIFIED_VERSION_LEADERBOARD_NOT_FOUND, E_PF_STATISTIC_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFLeaderboardsGetEntityLeaderboardResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Get the leaderboard around a specific entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsGetLeaderboardAroundEntityGetResultSize"/>
/// and <see cref="PFLeaderboardsGetLeaderboardAroundEntityGetResult"/> to get the result.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardAroundEntityAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsGetLeaderboardAroundEntityRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetLeaderboardAroundEntity call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_LEADERBOARD_NOT_FOUND,
/// E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFLeaderboardsGetLeaderboardAroundEntityGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsGetLeaderboardAroundEntityAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_LEADERBOARD_NOT_FOUND,
/// E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardAroundEntityGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFLeaderboardsGetEntityLeaderboardResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Gets the specified leaderboard definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsGetLeaderboardDefinitionGetResultSize"/>
/// and <see cref="PFLeaderboardsGetLeaderboardDefinitionGetResult"/> to get the result.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsGetLeaderboardDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetLeaderboardDefinition call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_TITLE,
/// E_PF_LEADERBOARD_COLUMN_LENGTH_MISMATCH_WITH_STAT_DEFINITION, E_PF_LEADERBOARD_NOT_FOUND or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
PF_API PFLeaderboardsGetLeaderboardDefinitionGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsGetLeaderboardDefinitionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_TITLE,
/// E_PF_LEADERBOARD_COLUMN_LENGTH_MISMATCH_WITH_STAT_DEFINITION, E_PF_LEADERBOARD_NOT_FOUND or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardDefinitionGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFLeaderboardsGetLeaderboardDefinitionResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Get the leaderboard limited to a set of entities.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsGetLeaderboardForEntitiesGetResultSize"/>
/// and <see cref="PFLeaderboardsGetLeaderboardForEntitiesGetResult"/> to get the result.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardForEntitiesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsGetLeaderboardForEntitiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetLeaderboardForEntities call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_TITLE,
/// E_PF_LEADERBOARD_NOT_FOUND, E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_STATISTIC_NOT_FOUND or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
PF_API PFLeaderboardsGetLeaderboardForEntitiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsGetLeaderboardForEntitiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_TITLE,
/// E_PF_LEADERBOARD_NOT_FOUND, E_PF_NO_LEADERBOARD_FOR_STATISTIC, E_PF_STATISTIC_NOT_FOUND or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFLeaderboardsGetLeaderboardForEntitiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFLeaderboardsGetEntityLeaderboardResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Increment a leaderboard version.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsIncrementLeaderboardVersionGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFLeaderboardsIncrementLeaderboardVersionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsIncrementLeaderboardVersionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsIncrementLeaderboardVersionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFLeaderboardsIncrementLeaderboardVersionResponse object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CONCURRENT_EDIT_ERROR,
/// E_PF_INVALID_VERSION_RESET_FOR_LINKED_LEADERBOARD, E_PF_LEADERBOARD_DEFINITION_MODIFICATION_NOT_ALLOWED_WHILE_LINKED,
/// E_PF_LEADERBOARD_NOT_FOUND, E_PF_VERSION_INCREMENT_RATE_EXCEEDED or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFLeaderboardsIncrementLeaderboardVersionGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFLeaderboardsIncrementLeaderboardVersionResponse* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Lists the leaderboard definitions defined for the Title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also LeaderboardDeleteLeaderboardDefinitionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFLeaderboardsListLeaderboardDefinitionsGetResultSize"/>
/// and <see cref="PFLeaderboardsListLeaderboardDefinitionsGetResult"/> to get the result.
/// </remarks>
PF_API PFLeaderboardsListLeaderboardDefinitionsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsListLeaderboardDefinitionsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListLeaderboardDefinitions call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFLeaderboardsListLeaderboardDefinitionsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFLeaderboardsListLeaderboardDefinitionsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFLeaderboardsListLeaderboardDefinitionsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFLeaderboardsListLeaderboardDefinitionsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Unlinks a leaderboard definition from it's linked statistic definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also LeaderboardCreateLeaderboardDefinitionAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_LEADERBOARD_NOT_FOUND, E_PF_NO_LINKED_STATISTIC_TO_LEADERBOARD or any
/// of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </remarks>
PF_API PFLeaderboardsUnlinkLeaderboardFromStatisticAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsUnlinkLeaderboardFromStatisticRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if 0
/// <summary>
/// Updates a leaderboard definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_LEADERBOARD_SIZE_LIMIT_EXCEEDED, E_PF_MAX_QUERYABLE_VERSIONS_VALUE_NOT_ALLOWED_FOR_TIER,
/// E_PF_RESET_INTERVAL_CANNOT_BE_MODIFIED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFLeaderboardsUpdateLeaderboardDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsUpdateLeaderboardDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Adds or updates entries on the specified leaderboard.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS, E_PF_LEADERBOARD_COLUMN_LENGTH_MISMATCH,
/// E_PF_LEADERBOARD_NOT_FOUND, E_PF_LEADERBOARD_UPDATE_NOT_ALLOWED_WHILE_LINKED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFLeaderboardsUpdateLeaderboardEntriesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFLeaderboardsUpdateLeaderboardEntriesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}