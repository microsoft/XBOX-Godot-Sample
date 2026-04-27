// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <XAsync.h>
#include <playfab/core/PFPal.h>
#include <playfab/core/PFTypes.h>
#include <playfab/core/PFAuthenticationTypes.h>
#include <playfab/core/PFServiceConfig.h>

extern "C"
{

/// <summary>
/// Handle to an authenticated Entity (TitlePlayer, Title, etc.). Contains the auth tokens needed to make PlayFab service
/// calls. When no longer needed, the Entity handle must be closed with PFEntityCloseHandle.
/// </summary>
typedef struct PFEntity* PFEntityHandle;

/// <summary>
/// Entity type for all TitlePlayer Entities. This const value can be used to populate PFEntityKeys referring to TitlePlayer Entities.
/// </summary>
static const char* PFEntityTitlePlayerEntityType = "title_player_account";

/// <summary>
/// PlayFab EntityToken and its expiration time. Used to authenticate PlayFab service calls.
/// </summary>
typedef struct PFEntityToken
{
    /// <summary>
    /// The token used to set X-EntityToken for all entity based API calls.
    /// </summary>
    const char* token;

    /// <summary>
    /// (Optional) The time the token will expire, if it is an expiring token, in UTC.
    /// </summary>
    time_t const* expiration;

} PFEntityToken;

/// <summary>
/// Duplicates a PFEntityHandle.
/// </summary>
/// <param name="entityHandle">Entity handle to duplicate.</param>
/// <param name="duplicatedEntityHandle">The duplicated handle.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Both the duplicated handle and the original handle need to be closed with PFEntityCloseHandle when they
/// are no longer needed.
/// </remarks>
PF_API PFEntityDuplicateHandle(
    _In_ PFEntityHandle entityHandle,
    _Out_ PFEntityHandle* duplicatedEntityHandle
) noexcept;

/// <summary>
/// Closes a PFEntityHandle.
/// </summary>
/// <param name="entityHandle">Entity handle to close.</param>
/// <returns>Result code for this API operation.</returns>
PF_API_(void) PFEntityCloseHandle(
    _In_ PFEntityHandle entityHandle
) noexcept;

/// <summary>
/// Get the cached PFEntityToken for an Entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a login call.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetEntityTokenAsync(
    _In_ PFEntityHandle entityHandle,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFEntityGetEntityTokenAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetEntityTokenResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFEntityGetEntityTokenAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="entityToken">Pointer to the EntityToken object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// entityToken is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEntityGetEntityTokenResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ const PFEntityToken** entityToken,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_MAC || HC_PLATFORM == HC_PLATFORM_LINUX
/// <summary>
/// Gets the size of the buffer needed to hold the PlayFab secretKey. Will fail with E_PF_NOSECRETKEY if there is no 
/// Secret Key associated with the provided Entity.
/// </summary>
/// <param name="handle">Entity handle.</param>
/// <param name="secretKeySize">Buffer size required for the secretKey (including null terminator).</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetSecretKeySize(
    _In_  PFEntityHandle handle,
    _Out_ size_t* secretKeySize
) noexcept;

/// <summary>
/// Gets the PlayFab secretKey associated with a title Entity. Will fail with E_PF_NOSECRETKEY if there is no Secret 
/// Key associated with the provided Entity.
/// </summary>
/// <param name="handle">Entity handle.</param>
/// <param name="secretKeySize">Size of the provided buffer. Required size can be obtained via PFServiceConfigGetSecretKeySize.</param>
/// <param name="secretKey">Buffer the secretKey will be written to.</param>
/// <param name="secretKeyUsed">The number of bytes used in the buffer including the null terminator.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetSecretKey(
    _In_ PFEntityHandle handle,
    _In_ size_t secretKeySize,
    _Out_writes_(secretKeySize) char* secretKey,
    _Out_opt_ size_t* secretKeyUsed
) noexcept;
#endif


/// <summary>
/// Get the size in bytes needed to store the PFEntityKey for an Entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="bufferSize">The buffer size in bytes required for the EntityKey.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetEntityKeySize(
    _In_ PFEntityHandle entityHandle,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the PFEntityKey for an entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="bufferSize">The size of the buffer for the PFEntityKey. The required size can be obtained from PFEntityGetEntityKeySize.</param>
/// <param name="buffer">Byte buffer used for the PFEntityKey and its fields.</param>
/// <param name="result">Pointer to the PFEntityKey object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// entityKey is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFEntityGetEntityKey(
    _In_ PFEntityHandle entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ const PFEntityKey** entityKey,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Convenience method to check if an Entity is a Title Player. This is equivalent to calling PFEntityGetEntityKey and comparing
/// entityKey.type to PFTitlePlayerEntityType.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="isTitlePlayer">Will be set to true if the entity is a TitlePlayer and false otherwise.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityIsTitlePlayer(
    _In_ PFEntityHandle entityHandle,
    _Out_ bool* isTitlePlayer
) noexcept;


/// <summary>
/// Gets the size of the buffer needed to hold the API endpoint string from an entity handle.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="apiEndpointSize">Buffer size required for the API endpoint string (including null terminator).</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetAPIEndpointSize(
    _In_  PFEntityHandle entityHandle,
    _Out_ size_t* apiEndpointSize
) noexcept;

/// <summary>
/// Gets the API endpoint for the PFServiceConfig associated with an entity handle.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="apiEndpointSize">Size of the provided buffer. Required size can be obtained via PFEntityGetAPIEndpointSize.</param>
/// <param name="apiEndpoint">Buffer the API endpoint string will be written to.</param>
/// <param name="apiEndpointUsed">The number of bytes used in the buffer including the null terminator.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetAPIEndpoint(
    _In_ PFEntityHandle entityHandle,
    _In_ size_t apiEndpointSize,
    _Out_writes_(apiEndpointSize) char* apiEndpoint,
    _Out_opt_ size_t* apiEndpointUsed
) noexcept;

/// <summary>
/// Gets the size of the buffer needed to hold the titleId string from an entity handle.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="apiEndpointSize">Buffer size required for the titleId string (including null terminator).</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetTitleIdSize(
    _In_  PFEntityHandle entityHandle,
    _Out_ size_t* titleIdSize
) noexcept;

/// <summary>
/// Gets the titleId for the PFServiceConfig associated with an entity handle.
/// </summary>
/// <param name="entityHandle">PFEntityHandle returned from a auth call.</param>
/// <param name="titleIdSize">Size of the provided buffer. Required size can be obtained via PFEntityGetAPIEndpointSize.</param>
/// <param name="titleIdBuffer">Buffer the titleId string will be written to.</param>
/// <param name="titleIdUsed">The number of bytes used in the buffer including the null terminator.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityGetTitleId(
    _In_ PFEntityHandle entityHandle,
    _In_ size_t titleIdSize,
    _Out_writes_(titleIdSize) char* titleIdBuffer,
    _Out_opt_ size_t* titleIdUsed
) noexcept;

/// <summary>
/// EntityToken expired event handler. Needed to reauthenticate players in scenarios where the SDK is unable to automatically
/// refresh the cached EntityToken.
/// </summary>
/// <param name="context">Optional context pointer to data used by the event handler.</param>
/// <param name="entityKey">The EntityKey for the Entity whose auth token expired.</param>
/// <returns></returns>
typedef void CALLBACK PFEntityTokenExpiredEventHandler(
    _In_ void* context,
    _In_ const PFEntityKey* entityKey
);

/// <summary>
/// Register a handler for EntityToken expired events.
/// </summary>
/// <param name="queue">The async queue the callback should be invoked on.</param>
/// <param name="context">Optional pointer to data used by the event handler.</param>
/// <param name="handler">The event handler, <see cref="XalUserChangeEventHandler"/>.</param>
/// <param name="token">The token for unregistering this callback</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_FAIL.</returns>
PF_API PFEntityRegisterTokenExpiredEventHandler(
    _In_opt_ XTaskQueueHandle queue,
    _In_opt_ void* context,
    _In_ PFEntityTokenExpiredEventHandler* handler,
    _Out_ PFRegistrationToken* token
) noexcept;

/// <summary>
/// Unregisters a previously registered callback.
/// </summary>
/// <param name="token">The token returned from PFEntityRegisterTokenExpiredEventHandler.</param>
/// <returns></returns>
PF_API_(void) PFEntityUnregisterTokenExpiredEventHandler(
    _In_ PFRegistrationToken token
) noexcept;

/// <summary>
/// A handler invoked every time an Entity is automatically re-authenticated, thus obtaining a new EntityToken. An entity
/// will be automatically re-authenticated prior to its EntityToken expiring.
/// </summary>
/// <param name="context">Optional context pointer to data used by the event handler.</param>
/// <param name="entityKey">The EntityKey for the Entity whose EntityToken expired.</param>
/// <param name="newToken">The new token for the refreshed entity.</param>
/// <returns></returns>
typedef void CALLBACK PFEntityTokenRefreshedEventHandler(
    _In_ void* context,
    _In_ const PFEntityKey* entityKey,
    _In_ const PFEntityToken* newToken
);

/// <summary>
/// Register a handler for EntityToken refreshed events.
/// </summary>
/// <param name="entityHandle">Entity handle for the entity.</param>
/// <param name="queue">The async queue the callback should be invoked on.</param>
/// <param name="callback">The callback, <see cref="PFEntityTokenRefreshedCallback"/>.</param>
/// <param name="context">Optional pointer to data used by the callback.</param>
/// <param name="token">The token for unregistering the callback.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFEntityRegisterTokenRefreshedEventHandler(
    _In_ XTaskQueueHandle queue,
    _In_opt_ void* context,
    _In_ PFEntityTokenRefreshedEventHandler* handler,
    _Out_ PFRegistrationToken* token
) noexcept;

/// <summary>
/// Unregisters a previously registered PFEntityTokenRefreshedCallback.
/// </summary>
/// <param name="entityHandle">Entity handle for the entity.</param>
/// <param name="token">Registration token from PFEntityRegisterTokenRefreshedCallback.</param>
/// <returns>Result code for this API operation.</returns>
PF_API_(void) PFEntityUnregisterTokenRefreshedEventHandler(
    _In_ PFRegistrationToken token
) noexcept;

}
