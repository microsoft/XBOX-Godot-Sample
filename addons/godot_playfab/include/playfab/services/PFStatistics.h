// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFStatisticsTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Create a new entity statistic definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also StatisticDeleteStatisticDefinitionAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_AGGREGATION_TYPE_NOT_ALLOWED_FOR_MULTI_COLUMN_STATISTIC, E_PF_API_NOT_ENABLED_FOR_TITLE,
/// E_PF_DUPLICATE_COLUMN_NAME_FOUND, E_PF_DUPLICATE_STATISTIC_NAME, E_PF_EXTERNAL_ENTITY_NOT_ALLOWED_FOR_TIER,
/// E_PF_INVALID_BASE_TIME_FOR_INTERVAL, E_PF_MAX_QUERYABLE_VERSIONS_VALUE_NOT_ALLOWED_FOR_TIER, E_PF_STATISTIC_COUNT_LIMIT_EXCEEDED,
/// E_PF_STATISTIC_DEFINITION_HAS_NULL_OR_EMPTY_VERSION_CONFIGURATION, E_PF_STATISTIC_NAME_CONFLICT or
/// any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </remarks>
PF_API PFStatisticsCreateStatisticDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsCreateStatisticDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Delete an entity statistic definition. Will delete all statistics on entity profiles and leaderboards.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also StatisticCreateStatisticDefinitionAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_STATISTIC_DEFINITION_MODIFICATION_NOT_ALLOWED_WHILE_LINKED, E_PF_STATISTIC_NOT_FOUND,
/// E_PF_STATISTIC_UPDATE_IN_PROGRESS or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFStatisticsDeleteStatisticDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsDeleteStatisticDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Delete statistics on an entity profile. This will remove all rankings from associated leaderboards.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFStatisticsDeleteStatisticsGetResultSize"/>
/// and <see cref="PFStatisticsDeleteStatisticsGetResult"/> to get the result.
/// </remarks>
PF_API PFStatisticsDeleteStatisticsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsDeleteStatisticsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a DeleteStatistics call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFStatisticsDeleteStatisticsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsDeleteStatisticsAsync call.
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
PF_API PFStatisticsDeleteStatisticsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFStatisticsDeleteStatisticsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Get current statistic definition information
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also StatisticCreateStatisticDefinitionAsync, StatisticDeleteStatisticDefinitionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFStatisticsGetStatisticDefinitionGetResultSize"/>
/// and <see cref="PFStatisticsGetStatisticDefinitionGetResult"/> to get the result.
/// </remarks>
PF_API PFStatisticsGetStatisticDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsGetStatisticDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetStatisticDefinition call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_STATISTIC_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFStatisticsGetStatisticDefinitionGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsGetStatisticDefinitionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_STATISTIC_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFStatisticsGetStatisticDefinitionGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFStatisticsGetStatisticDefinitionResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Gets statistics for the specified entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFStatisticsGetStatisticsGetResultSize"/>
/// and <see cref="PFStatisticsGetStatisticsGetResult"/> to get the result.
/// </remarks>
PF_API PFStatisticsGetStatisticsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsGetStatisticsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetStatistics call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_INVALID_ENTITY_TYPE, E_PF_NOT_AUTHORIZED, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFStatisticsGetStatisticsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsGetStatisticsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_INVALID_ENTITY_TYPE, E_PF_NOT_AUTHORIZED, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFStatisticsGetStatisticsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFStatisticsGetStatisticsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Gets statistics for the specified collection of entities.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFStatisticsGetStatisticsForEntitiesGetResultSize"/>
/// and <see cref="PFStatisticsGetStatisticsForEntitiesGetResult"/> to get the result.
/// </remarks>
PF_API PFStatisticsGetStatisticsForEntitiesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsGetStatisticsForEntitiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetStatisticsForEntities call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_ENTITY_TYPE,
/// E_PF_NOT_AUTHORIZED, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFStatisticsGetStatisticsForEntitiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsGetStatisticsForEntitiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_ENTITY_TYPE,
/// E_PF_NOT_AUTHORIZED, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFStatisticsGetStatisticsForEntitiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFStatisticsGetStatisticsForEntitiesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Increment an entity statistic definition version.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFStatisticsIncrementStatisticVersionGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFStatisticsIncrementStatisticVersionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsIncrementStatisticVersionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsIncrementStatisticVersionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFStatisticsIncrementStatisticVersionResponse object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_STATISTIC_NOT_FOUND,
/// E_PF_STATISTIC_UPDATE_IN_PROGRESS, E_PF_STATISTIC_VERSION_INCREMENT_RATE_EXCEEDED, E_PF_VERSION_INCREMENT_RATE_EXCEEDED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFStatisticsIncrementStatisticVersionGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFStatisticsIncrementStatisticVersionResponse* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Get all current statistic definitions information
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also StatisticCreateStatisticDefinitionAsync, StatisticDeleteStatisticDefinitionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFStatisticsListStatisticDefinitionsGetResultSize"/>
/// and <see cref="PFStatisticsListStatisticDefinitionsGetResult"/> to get the result.
/// </remarks>
PF_API PFStatisticsListStatisticDefinitionsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsListStatisticDefinitionsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListStatisticDefinitions call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_STATISTIC_COUNT_LIMIT_EXCEEDED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFStatisticsListStatisticDefinitionsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsListStatisticDefinitionsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_STATISTIC_COUNT_LIMIT_EXCEEDED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFStatisticsListStatisticDefinitionsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFStatisticsListStatisticDefinitionsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Update an existing entity statistic definition.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_API_NOT_ENABLED_FOR_TITLE, E_PF_INVALID_BASE_TIME_FOR_INTERVAL, E_PF_MAX_QUERYABLE_VERSIONS_VALUE_NOT_ALLOWED_FOR_TIER,
/// E_PF_RESET_INTERVAL_CANNOT_BE_MODIFIED, E_PF_STATISTIC_NOT_FOUND or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFStatisticsUpdateStatisticDefinitionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsUpdateStatisticDefinitionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Update statistics on an entity profile. Depending on the statistic definition, this may result in
/// entity being ranked on various leaderboards.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// See also StatisticDeleteStatisticsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFStatisticsUpdateStatisticsGetResultSize"/>
/// and <see cref="PFStatisticsUpdateStatisticsGetResult"/> to get the result.
/// </remarks>
PF_API PFStatisticsUpdateStatisticsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFStatisticsUpdateStatisticsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a UpdateStatistics call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_ENTITY_TYPE_MISMATCH_WITH_STAT_DEFINITION, E_PF_INVALID_ENTITY_TYPE, E_PF_INVALID_EXTERNAL_ENTITY_ID,
/// E_PF_NOT_AUTHORIZED, E_PF_STATISTIC_COLUMN_LENGTH_MISMATCH, E_PF_STATISTIC_NOT_FOUND, E_PF_TRANSACTION_ALREADY_APPLIED,
/// E_PF_UPDATING_STATISTICS_USING_TRANSACTION_ID_NOT_AVAILABLE_FOR_FREE_TIER or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFStatisticsUpdateStatisticsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFStatisticsUpdateStatisticsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_API_NOT_ENABLED_FOR_GAME_CLIENT_ACCESS,
/// E_PF_ENTITY_TYPE_MISMATCH_WITH_STAT_DEFINITION, E_PF_INVALID_ENTITY_TYPE, E_PF_INVALID_EXTERNAL_ENTITY_ID,
/// E_PF_NOT_AUTHORIZED, E_PF_STATISTIC_COLUMN_LENGTH_MISMATCH, E_PF_STATISTIC_NOT_FOUND, E_PF_TRANSACTION_ALREADY_APPLIED,
/// E_PF_UPDATING_STATISTICS_USING_TRANSACTION_ID_NOT_AVAILABLE_FOR_FREE_TIER or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFStatisticsUpdateStatisticsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFStatisticsUpdateStatisticsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}