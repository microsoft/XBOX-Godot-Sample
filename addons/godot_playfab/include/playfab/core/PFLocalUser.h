// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFEntity.h>
#if HC_PLATFORM == HC_PLATFORM_GDK
#include <playfab/core/PFPlatformGDK.h>
#elif HC_PLATFORM == HC_PLATFORM_NINTENDO_SWITCH
#include <playfab/core/PFPlatformSwitch.h>
#elif HC_PLATFORM == HC_PLATFORM_ANDROID
#include <playfab/core/PFPlatformAndroid.h>
#elif defined(STEAMWORKS_AVAILABLE)
#include <playfab/core/PFPlatformSteam.h>
#endif

extern "C"
{

/// <summary>
/// Handle to a PlayFab Local User.
/// </summary>
typedef struct PFLocalUser* PFLocalUserHandle;

/// <summary>
/// Forward declaration of platform specific context used to create a PlayFab Local User. The platform context 
/// will be used to obtain a localId, as well as to authenticate with PlayFab when the user is logged in
/// with PFLocalUserLoginAsync.
/// </summary>
struct PFLocalUserPlatformContext;

/// <summary>
/// Callback that will be invoked to enable custom login logic. This can be used in cases where a non-standard
/// login provider is desired, or if there isn't a default login provider for the platform. The title is responsible
/// for building the login request object and calling one of the PFAuthenticationLoginWith* APIs.
/// This callback will also be invoked by the SDK to renew an expired token for a previously authenticated user. In that
/// case, the existingEntityHandle will be non-null, and the title should call the appropriate PFAuthenticationReLoginWith*
/// API instead.
/// 
/// The PFServiceConfigHandle and XAsyncBlock passed to this callback are owned by the SDK and should just be passed
/// directly to the login API of choice.
/// </summary>
/// <param name="localUserHandle">The local user to log in.</param>
/// <param name="serviceConfigHandle">The service config to use for login.</param>
/// <param name="existingEntityHandle">The previously authenticated entity if there is one and nullptr otherwise.</param>
/// <param name="async">Async block to be passed to the login API.</param>
/// <returns></returns>
typedef HRESULT CALLBACK PFLocalUserLoginHandler(
    _In_ PFLocalUserHandle localUserHandle,
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_opt_ PFEntityHandle existingEntityHandle,
    _Inout_ XAsyncBlock* async
);

#ifdef PF_PLATFORM_USER
/// <summary>
/// Creates a PlayFab Local User to be used when a PlayFab identity is needed but PlayFab login is unavailable (ex. the device is offline). 
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle that will be used to log this user into PlayFab.</param>
/// <param name="platformContext">Platform specific context that identifies the local user.</param>
/// <param name="customContext">Custom context to be associated with the local user.</param>
/// <param name="localUserHandle">Pointer to a PFLocalUserHandle to write.</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_INVALIDARG.</returns>
PF_API PFLocalUserCreateHandleWithPlatformUser(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ PFLocalUserPlatformContext const* platformContext,
    _In_opt_ void* customContext,
    _Out_ PFLocalUserHandle* localUserHandle
) noexcept;
#endif

/// <summary>
/// Creates a PlayFab Local User to be used when a PlayFab identity is needed but PlayFab login is unavailable (ex. the device is offline).
/// It is the title's responsibility to ensure the localId provided is:
/// 1) Locally Unique. If multiple users play the title on the same device (simultaneously or during different play sessions) they must
///    have different localIds.
/// 2) The same for a given user across multiple play sessions.
/// Additionally, the title must provide a custom PFLocalUserLoginHandler. This handler will be called by the SDK to authenticate the user
/// with PlayFab. See <see cref="PFLocalUserLoginHandler"> for more details.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle that will be used to log this user into PlayFab.</param>
/// <param name="persistedLocalId">Locally unique ID string that identifies the user. ID will be persisted across play sessions.</param>
/// <param name="loginHandle">Custom handler that will be called to log the local user into PlayFab.</param>
/// <param name="customContext">Custom context to be associated with the local user.</param>
/// <param name="localUserHandle">Pointer to a PFLocalUserHandle to write.</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_INVALIDARG.</returns>
PF_API PFLocalUserCreateHandleWithPersistedLocalId(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const char* persistedLocalId,
    _In_ PFLocalUserLoginHandler* loginHandler,
    _In_opt_ void* customContext,
    _Out_ PFLocalUserHandle* localUserHandle
) noexcept;

/// <summary>
/// Duplicates a PFLocalUserHandle.
/// </summary>
/// <param name="localUserHandle">LocalUser handle to duplicate.</param>
/// <param name="duplicatedHandle">The duplicated handle.</param>
/// <returns>Result code for this API operation.</returns> 
/// <remarks>
/// Both the duplicated handle and the original handle need to be closed with PFLocalUserCloseHandle when they
/// are no longer needed.
/// </remarks>
PF_API PFLocalUserDuplicateHandle(
    _In_ PFLocalUserHandle localUserHandle,
    _Out_ PFLocalUserHandle* duplicatedHandle
) noexcept;

/// <summary>
/// Closes a PFLocalUserHandle.
/// </summary>
/// <param name="handle">LocalUser handle to close.</param>
/// <returns>Result code for this API operation.</returns>
PF_API_(void) PFLocalUserCloseHandle(
    _In_ PFLocalUserHandle localUserHandle
) noexcept;

/// <summary>
/// Compares two PFLocalUserHandle objects.
/// </summary>
/// <param name="user1">First user to compare.</param>
/// <param name="user2">Second user to compare.</param>
/// <returns>Returns 0 if the handles are equal, a negative number if user1 is less than user2, and a positive number if user1 is greater than user2.</returns>
PF_API_(int32_t) PFLocalUserHandleCompare(
    _In_opt_ PFLocalUserHandle user1,
    _In_opt_ PFLocalUserHandle user2
    ) noexcept;

/// <summary>
/// Gets the PFServiceConfigHandle associated with the local user.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="serviceConfigHandle">PFServiceConfigHandle to set.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFLocalUserGetServiceConfigHandle(
    _In_ PFLocalUserHandle localUserHandle,
    _Out_ PFServiceConfigHandle* serviceConfigHandle
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the localId for a user.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="bufferSize">The buffer size in bytes required for the localId (including the null terminator).</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFLocalUserGetLocalIdSize(
    _In_ PFLocalUserHandle localUserHandle,
    _Out_ size_t* localIdSize
) noexcept;

/// <summary>
/// Get the localId for a user.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="localIdSize">Size of the provided buffer. Required size can be obtained via PFLocalUserGetLocalIdSize.</param>
/// <param name="localIdBuffer">Buffer the localId will be written to.</param>
/// <param name="localIdUsed">The number of bytes used in the buffer including the null terminator.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFLocalUserGetLocalId(
    _In_ PFLocalUserHandle localUserHandle,
    _In_ size_t localIdSize,
    _Out_writes_(localIdSize) char* localIdBuffer,
    _Out_opt_ size_t* localIdUsed
) noexcept;

/// <summary>
/// Get the PFLocalUserPlatformContext used to create the user if there was one. If the user was created with PFLocalUserCreateHandleWithPersistedLocalId,
/// there isn't a PFLocalUserPlatformContext and E_PF_CORE_LOCAL_USER_NO_PLATFORM_CONTEXT will be returned.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="localUserPlatformContext">Title owned PFLocalUserPlatformContext struct to populate.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// The populated localUserPlatformContext is owned by the title. After calling this API, it is the title's responsibility to make sure it
/// and any of its members are appropriately cleaned up (i.e. calling XUserCloseHandle on GDK).
/// </remarks>
PF_API PFLocalUserTryGetPlatformContext(
    _In_ PFLocalUserHandle localUserHandle,
    _Out_ PFLocalUserPlatformContext* localUserPlatformContext
) noexcept;

/// <summary>
/// Get the custom context that was associated with a local user when it was created.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="customContext">Custom context pointer to write.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFLocalUserGetCustomContext(
    _In_ PFLocalUserHandle localUserHandle,
    _Out_ void** customContext
) noexcept;

/// <summary>
/// Get the PFEntityHandle associated with the local user if there is one. If the user isn't logged into PlayFab and there isn't an
/// associated handle, E_PF_CORE_LOCAL_USER_NOT_LOGGED_IN will be returned.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="entityHandle">Entity handle to set.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// If the call succeeds, the populated PFEntityHandle is owned by the title. After calling this API, it is the title's responsibility
/// to close it when it is no longer needed using PFEntityCloseHandle.
/// </remarks>
PF_API PFLocalUserTryGetEntityHandle(
    _In_ PFLocalUserHandle localUserHandle,
    _Out_ PFEntityHandle* entityHandle
) noexcept;

/// <summary>
/// Attempts to login the local user with the default PlayFab login provider, or the custom login handler provided when the local
/// user was created.
/// </summary>
/// <param name="localUserHandle">PFLocalUserHandle for the user.</param>
/// <param name="createAccount">Whether or not to automatically create a PlayFab account if one doesn't yet exist for the user.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// The resulting PFEntityHandle will automatically be associated with the local user who is logging in.
/// </remarks>
PF_API PFLocalUserLoginAsync(
    _In_ PFLocalUserHandle localUserHandle,
    _In_ bool createAccount,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFLocalUserLoginAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFLocalUserLoginGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFLocalUserLoginAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// If the PFLocalUserLoginAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFLocalUserLoginGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

} // extern "C"
