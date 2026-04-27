// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFFriendsTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Adds the PlayFab user, based upon a match against a supplied unique identifier, to the friend list
/// of the local user. At least one of FriendPlayFabId,FriendUsername,FriendEmail, or FriendTitleDisplayName
/// should be initialized.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// See also ClientGetFriendsListAsync, ClientSetFriendTagsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFFriendsClientAddFriendGetResult"/> to get
/// the result.
/// </remarks>
PF_API PFFriendsClientAddFriendAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFFriendsClientAddFriendRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFFriendsClientAddFriendAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFFriendsAddFriendResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_USERS_ALREADY_FRIENDS
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFFriendsClientAddFriendGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFFriendsAddFriendResult* result
) noexcept;

/// <summary>
/// Retrieves the current friend list for the local user, constrained to users who have PlayFab accounts.
/// Friends from linked accounts (Facebook, Steam) are also included. You may optionally exclude some
/// linked services' friends.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// See also ClientAddFriendAsync, ClientGetPlayerProfileAsync, ClientRemoveFriendAsync.
///
/// When the asynchronous task is complete, call <see cref="PFFriendsClientGetFriendsListGetResultSize"/>
/// and <see cref="PFFriendsClientGetFriendsListGetResult"/> to get the result.
/// </remarks>
PF_API PFFriendsClientGetFriendsListAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFFriendsClientGetFriendsListRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetFriendsList call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_FOUND,
/// E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_XBOX_INACCESSIBLE,
/// E_PF_XBOX_SERVICE_TOO_MANY_REQUESTS, E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFFriendsClientGetFriendsListGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFFriendsClientGetFriendsListAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_FOUND,
/// E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_XBOX_INACCESSIBLE,
/// E_PF_XBOX_SERVICE_TOO_MANY_REQUESTS, E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFFriendsClientGetFriendsListGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFFriendsGetFriendsListResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Removes a specified user from the friend list of the local user
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// See also ClientAddFriendAsync, ClientSetFriendTagsAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFFriendsClientRemoveFriendAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFFriendsClientRemoveFriendRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Updates the tag list for a specified user in the friend list of the local user
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This operation is not additive. It will completely replace the tag list for the specified user. Please
/// note that only users in the PlayFab friends list can be assigned tags. Attempting to set a tag on
/// a friend only included in the friends list from a social site integration (such as Facebook or Steam)
/// will return the AccountNotFound error. See also ClientAddFriendAsync, ClientRemoveFriendAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFFriendsClientSetFriendTagsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFFriendsClientSetFriendTagsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Adds the Friend user to the friendlist of the user with PlayFabId. At least one of FriendPlayFabId,FriendUsername,FriendEmail,
/// or FriendTitleDisplayName should be initialized.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also ServerGetFriendsListAsync, ServerRemoveFriendAsync, ServerSetFriendTagsAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_USERS_ALREADY_FRIENDS or any of the global PlayFab Service errors. See
/// doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFFriendsServerAddFriendAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFFriendsServerAddFriendRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the current friends for the user with PlayFabId, constrained to users who have PlayFab
/// accounts. Friends from linked accounts (Facebook, Steam) are also included. You may optionally exclude
/// some linked services' friends.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also ServerAddFriendAsync, ServerGetPlayerProfileAsync, ServerRemoveFriendAsync, ServerSetFriendTagsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFFriendsServerGetFriendsListGetResultSize"/>
/// and <see cref="PFFriendsServerGetFriendsListGetResult"/> to get the result.
/// </remarks>
PF_API PFFriendsServerGetFriendsListAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFFriendsServerGetFriendsListRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetFriendsList call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_FOUND,
/// E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_SERVICE_TOO_MANY_REQUESTS,
/// E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFFriendsServerGetFriendsListGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFFriendsServerGetFriendsListAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_FOUND,
/// E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_SERVICE_TOO_MANY_REQUESTS,
/// E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFFriendsServerGetFriendsListGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFFriendsGetFriendsListResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Removes the specified friend from the the user's friend list
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// See also ServerAddFriendAsync, ServerSetFriendTagsAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ACCOUNT_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFFriendsServerRemoveFriendAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFFriendsServerRemoveFriendRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the tag list for a specified user in the friend list of another user
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This operation is not additive. It will completely replace the tag list for the specified user. Please
/// note that only users in the PlayFab friends list can be assigned tags. Attempting to set a tag on
/// a friend only included in the friends list from a social site integration (such as Facebook or Steam)
/// will return the AccountNotFound error. See also ServerAddFriendAsync, ServerGetFriendsListAsync, ServerRemoveFriendAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFFriendsServerSetFriendTagsAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFFriendsServerSetFriendTagsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}