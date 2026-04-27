// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFSegmentsTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// List all segments that a player currently belongs to at this moment in time.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFSegmentsClientGetPlayerSegmentsGetResultSize"/>
/// and <see cref="PFSegmentsClientGetPlayerSegmentsGetResult"/> to get the result.
/// </remarks>
PF_API PFSegmentsClientGetPlayerSegmentsAsync(
    _In_ PFEntityHandle entityHandle,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetPlayerSegments call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFSegmentsClientGetPlayerSegmentsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFSegmentsClientGetPlayerSegmentsAsync call.
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
PF_API PFSegmentsClientGetPlayerSegmentsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFSegmentsGetPlayerSegmentsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Get all tags with a given Namespace (optional) from a player profile.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This API will return a list of canonical tags which includes both namespace and tag's name. If namespace
/// is not provided, the result is a list of all canonical tags. TagName can be used for segmentation
/// and Namespace is limited to 128 characters.
///
/// When the asynchronous task is complete, call <see cref="PFSegmentsClientGetPlayerTagsGetResultSize"/>
/// and <see cref="PFSegmentsClientGetPlayerTagsGetResult"/> to get the result.
/// </remarks>
PF_API PFSegmentsClientGetPlayerTagsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFSegmentsGetPlayerTagsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetPlayerTags call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFSegmentsClientGetPlayerTagsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFSegmentsClientGetPlayerTagsAsync call.
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
PF_API PFSegmentsClientGetPlayerTagsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFSegmentsGetPlayerTagsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Adds a given tag to a player profile. The tag's namespace is automatically generated based on the
/// source of the tag.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API will trigger a player_tag_added event and add a tag with the given TagName and PlayFabID
/// to the corresponding player profile. TagName can be used for segmentation and it is limited to 256
/// characters. Also there is a limit on the number of tags a title can have. See also ServerGetPlayerTagsAsync,
/// ServerRemovePlayerTagAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_PLAYER_TAG_COUNT_LIMIT_EXCEEDED or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFSegmentsServerAddPlayerTagAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFSegmentsAddPlayerTagRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves an array of player segment definitions. Results from this can be used in subsequent API
/// calls such as GetPlayersInSegment which requires a Segment ID. While segment names can change the
/// ID for that segment will not change.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Request has no paramaters. See also ServerGetPlayersInSegmentAsync.
///
/// When the asynchronous task is complete, call <see cref="PFSegmentsServerGetAllSegmentsGetResultSize"/>
/// and <see cref="PFSegmentsServerGetAllSegmentsGetResult"/> to get the result.
/// </remarks>
PF_API PFSegmentsServerGetAllSegmentsAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetAllSegments call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFSegmentsServerGetAllSegmentsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFSegmentsServerGetAllSegmentsAsync call.
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
PF_API PFSegmentsServerGetAllSegmentsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFSegmentsGetAllSegmentsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// List all segments that a player currently belongs to at this moment in time.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also ServerGetAllSegmentsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFSegmentsServerGetPlayerSegmentsGetResultSize"/>
/// and <see cref="PFSegmentsServerGetPlayerSegmentsGetResult"/> to get the result.
/// </remarks>
PF_API PFSegmentsServerGetPlayerSegmentsAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFSegmentsGetPlayersSegmentsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetPlayerSegments call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFSegmentsServerGetPlayerSegmentsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFSegmentsServerGetPlayerSegmentsAsync call.
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
PF_API PFSegmentsServerGetPlayerSegmentsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFSegmentsGetPlayerSegmentsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Allows for paging through all players in a given segment. This API creates a snapshot of all player
/// profiles that match the segment definition at the time of its creation and lives through the Total
/// Seconds to Live, refreshing its life span on each subsequent use of the Continuation Token. Profiles
/// that change during the course of paging will not be reflected in the results. AB Test segments are
/// currently not supported by this operation. NOTE: This API is limited to being called 30 times in one
/// minute. You will be returned an error if you exceed this threshold.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Initial request must contain at least a Segment ID. Subsequent requests must contain the Segment
/// ID as well as the Continuation Token. Failure to send the Continuation Token will result in a new
/// player segment list being generated. Each time the Continuation Token is passed in the length of the
/// Total Seconds to Live is refreshed. If too much time passes between requests to the point that a subsequent
/// request is past the Total Seconds to Live an error will be returned and paging will be terminated.
/// This API is resource intensive and should not be used in scenarios which might generate high request
/// volumes. Only one request to this API at a time should be made per title. Concurrent requests to the
/// API may be rejected with the APIConcurrentRequestLimitExceeded error. See also ServerGetAllSegmentsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFSegmentsServerGetPlayersInSegmentGetResultSize"/>
/// and <see cref="PFSegmentsServerGetPlayersInSegmentGetResult"/> to get the result.
/// </remarks>
PF_API PFSegmentsServerGetPlayersInSegmentAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFSegmentsGetPlayersInSegmentRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetPlayersInSegment call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EXPIRED_CONTINUATION_TOKEN,
/// E_PF_GET_PLAYERS_IN_SEGMENT_RATE_LIMIT_EXCEEDED, E_PF_INTERNAL_SERVER_ERROR, E_PF_INVALID_CONTINUATION_TOKEN,
/// E_PF_INVALID_SEARCH_TERM, E_PF_INVALID_SEGMENT, E_PF_SEGMENT_NOT_FOUND or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFSegmentsServerGetPlayersInSegmentGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFSegmentsServerGetPlayersInSegmentAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EXPIRED_CONTINUATION_TOKEN,
/// E_PF_GET_PLAYERS_IN_SEGMENT_RATE_LIMIT_EXCEEDED, E_PF_INTERNAL_SERVER_ERROR, E_PF_INVALID_CONTINUATION_TOKEN,
/// E_PF_INVALID_SEARCH_TERM, E_PF_INVALID_SEGMENT, E_PF_SEGMENT_NOT_FOUND or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFSegmentsServerGetPlayersInSegmentGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFSegmentsGetPlayersInSegmentResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Get all tags with a given Namespace (optional) from a player profile.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API will return a list of canonical tags which includes both namespace and tag's name. If namespace
/// is not provided, the result is a list of all canonical tags. TagName can be used for segmentation
/// and Namespace is limited to 128 characters. See also ServerAddPlayerTagAsync, ServerRemovePlayerTagAsync.
///
/// When the asynchronous task is complete, call <see cref="PFSegmentsServerGetPlayerTagsGetResultSize"/>
/// and <see cref="PFSegmentsServerGetPlayerTagsGetResult"/> to get the result.
/// </remarks>
PF_API PFSegmentsServerGetPlayerTagsAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFSegmentsGetPlayerTagsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetPlayerTags call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFSegmentsServerGetPlayerTagsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFSegmentsServerGetPlayerTagsAsync call.
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
PF_API PFSegmentsServerGetPlayerTagsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFSegmentsGetPlayerTagsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Remove a given tag from a player profile. The tag's namespace is automatically generated based on
/// the source of the tag.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API will trigger a player_tag_removed event and remove a tag with the given TagName and PlayFabID
/// from the corresponding player profile. TagName can be used for segmentation and it is limited to 256
/// characters See also ServerAddPlayerTagAsync, ServerGetPlayerTagsAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFSegmentsServerRemovePlayerTagAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFSegmentsRemovePlayerTagRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}