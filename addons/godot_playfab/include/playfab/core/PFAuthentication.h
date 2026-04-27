// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFAuthenticationTypes.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if 0
/// <summary>
/// Signs the user in using the Android device identifier, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// On Android devices, the recommendation is to use the Settings.Secure.ANDROID_ID as the AndroidDeviceId,
/// as described in this blog post (http://android-developers.blogspot.com/2011/03/identifying-app-installations.html).
/// More information on this identifier can be found in the Android documentation (http://developer.android.com/reference/android/provider/Settings.Secure.html).
/// If this is the first time a user has signed in with the Android device and CreateAccount is set to
/// true, a new PlayFab account will be created and linked to the Android device ID. In this case, no
/// email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is
/// linked to the Android device, an error indicating this will be returned, so that the title can guide
/// the user through creation of a PlayFab account. Please note that while multiple devices of this type
/// can be linked to a single user account, only the one most recently used to login (or most recently
/// linked) will be reflected in the user's account information. We will be updating to show all linked
/// devices in a future release. See also ClientLinkAndroidDeviceIDAsync, ClientLoginWithIOSDeviceIDAsync,
/// ClientUnlinkAndroidDeviceIDAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithAndroidDeviceIDGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithAndroidDeviceIDAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithAndroidDeviceIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithAndroidDeviceIDAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_CREATION_DISABLED,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_SIGNED_REQUEST_NOT_ALLOWED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithAndroidDeviceIDGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithAndroidDeviceIDAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_CREATION_DISABLED,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_SIGNED_REQUEST_NOT_ALLOWED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithAndroidDeviceIDAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithAndroidDeviceIDGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithAndroidDeviceIDAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithAndroidDeviceIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_IOS || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Signs in the user with a Sign in with Apple identity token.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on iOS and macOS.
/// See also ClientLinkAppleAsync, ClientUnlinkAppleAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithAppleGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithAppleAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithAppleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithAppleAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_APPLE_NOT_ENABLED_FOR_TITLE,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_TOKEN_SIGNING_KEY_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithAppleGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithAppleAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_APPLE_NOT_ENABLED_FOR_TITLE,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_TOKEN_SIGNING_KEY_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithAppleAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithAppleGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithAppleAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithAppleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Sign in the user with a Battle.net identity token
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ClientLinkBattleNetAsync, ClientUnlinkBattleNetAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithBattleNetGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithBattleNetAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithBattleNetRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithBattleNetAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_BATTLE_NET_NOT_ENABLED_FOR_TITLE,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithBattleNetGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithBattleNetAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_BATTLE_NET_NOT_ENABLED_FOR_TITLE,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithBattleNetAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithBattleNetGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithBattleNetAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithBattleNetRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

/// <summary>
/// Signs the user in using a custom unique identifier generated by the title, returning a session identifier
/// that can subsequently be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// It is highly recommended that developers ensure that it is extremely unlikely that a customer could
/// generate an ID which is already in use by another customer. If this is the first time a user has signed
/// in with the Custom ID and CreateAccount is set to true, a new PlayFab account will be created and
/// linked to the Custom ID. In this case, no email or username will be associated with the PlayFab account.
/// Otherwise, if no PlayFab account is linked to the Custom ID, an error indicating this will be returned,
/// so that the title can guide the user through creation of a PlayFab account. See also ClientLinkCustomIDAsync,
/// ClientUnlinkCustomIDAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithCustomIDGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithCustomIDAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithCustomIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithCustomIDAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTED_REQUEST_NOT_ALLOWED,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_CREATION_DISABLED, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_SIGNED_REQUEST_NOT_ALLOWED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithCustomIDGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithCustomIDAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTED_REQUEST_NOT_ALLOWED,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_CREATION_DISABLED, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_SIGNED_REQUEST_NOT_ALLOWED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithCustomIDAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithCustomIDGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithCustomIDAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithCustomIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;


#if 0
/// <summary>
/// Signs the user into the PlayFab account, returning a session identifier that can subsequently be
/// used for API calls which require an authenticated user. Unlike most other login API calls, LoginWithEmailAddress
/// does not permit the creation of new accounts via the CreateAccountFlag. Email addresses may be used
/// to create accounts via RegisterPlayFabUser.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Email address and password lengths are provided for information purposes. The server will validate
/// that data passed in conforms to the field definition and report errors appropriately. It is recommended
/// that developers not perform this validation locally, so that future updates do not require client
/// updates. See also ClientLoginWithPlayFabAsync, ClientRegisterPlayFabUserAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithEmailAddressGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithEmailAddressAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithEmailAddressRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithEmailAddressAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_EMAIL_OR_PASSWORD,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithEmailAddressGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithEmailAddressAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_EMAIL_OR_PASSWORD,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithEmailAddressAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithEmailAddressGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithEmailAddressAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithEmailAddressRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_ANDROID || HC_PLATFORM == HC_PLATFORM_IOS
/// <summary>
/// Signs the user in using a Facebook access token, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Android and iOS.
/// Facebook sign-in is accomplished using the Facebook User Access Token. More information on the Token
/// can be found in the Facebook developer documentation (https://developers.facebook.com/docs/facebook-login/access-tokens/).
/// In Unity, for example, the Token is available as AccessToken in the Facebook SDK ScriptableObject
/// FB. If this is the first time a user has signed in with the Facebook account and CreateAccount is
/// set to true, a new PlayFab account will be created and linked to the provided account's Facebook ID.
/// In this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the Facebook account, an error indicating this will be returned, so that the
/// title can guide the user through creation of a PlayFab account. Note that titles should never re-use
/// the same Facebook applications between PlayFab Title IDs, as Facebook provides unique user IDs per
/// application and doing so can result in issues with the Facebook ID for the user in their PlayFab account
/// information. If you must re-use an application in a new PlayFab Title ID, please be sure to first
/// unlink all accounts from Facebook, or delete all users in the first Title ID. Note: If the user is
/// authenticated with AuthenticationToken, instead of AccessToken, the GetFriendsList API will return
/// an empty list. See also ClientLinkFacebookAccountAsync, ClientUnlinkFacebookAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithFacebookGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithFacebookAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithFacebookRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithFacebookAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_FACEBOOK_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithFacebookGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithFacebookAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FACEBOOK_API_ERROR, E_PF_INVALID_FACEBOOK_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithFacebookAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithFacebookGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithFacebookAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithFacebookRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using a Facebook Instant Games ID, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user. Requires Facebook Instant Games to be configured.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ClientLinkFacebookInstantGamesIdAsync, ClientUnlinkFacebookInstantGamesIdAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithFacebookInstantGamesIdGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithFacebookInstantGamesIdAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithFacebookInstantGamesIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithFacebookInstantGamesIdAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FACEBOOK_INSTANT_GAMES_AUTH_NOT_CONFIGURED_FOR_TITLE,
/// E_PF_INVALID_FACEBOOK_INSTANT_GAMES_SIGNATURE, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithFacebookInstantGamesIdGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithFacebookInstantGamesIdAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FACEBOOK_INSTANT_GAMES_AUTH_NOT_CONFIGURED_FOR_TITLE,
/// E_PF_INVALID_FACEBOOK_INSTANT_GAMES_SIGNATURE, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithFacebookInstantGamesIdAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithFacebookInstantGamesIdGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithFacebookInstantGamesIdAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithFacebookInstantGamesIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_IOS
/// <summary>
/// Signs the user in using an iOS Game Center player identifier, returning a session identifier that
/// can subsequently be used for API calls which require an authenticated user. Logging in with a Game
/// Center ID is insecure if you do not include the optional PublicKeyUrl, Salt, Signature, and Timestamp
/// parameters in this request. It is recommended you require these parameters on all Game Center calls
/// by going to the Apple Add-ons page in the PlayFab Game Manager and enabling the 'Require secure authentication
/// only for this app' option.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on iOS.
/// The Game Center player identifier (https://developer.apple.com/library/ios/documentation/Accounts/Reference/ACAccountClassRef/index.html#//apple_ref/occ/instp/ACAccount/identifier)
/// is a generated string which is stored on the local device. As with device identifiers, care must be
/// taken to never expose a player's Game Center identifier to end users, as that could result in a user's
/// account being compromised. If this is the first time a user has signed in with Game Center and CreateAccount
/// is set to true, a new PlayFab account will be created and linked to the Game Center identifier. In
/// this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the Game Center account, an error indicating this will be returned, so that the
/// title can guide the user through creation of a PlayFab account. If an invalid iOS Game Center player
/// identifier is used, an error indicating this will be returned. See also ClientLoginWithIOSDeviceIDAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithGameCenterGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithGameCenterAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithGameCenterRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithGameCenterAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_GAME_CENTER_AUTHENTICATION_FAILED, E_PF_INVALID_GAME_CENTER_AUTH_REQUEST,
/// E_PF_INVALID_GAME_CENTER_ID, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithGameCenterGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithGameCenterAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_GAME_CENTER_AUTHENTICATION_FAILED, E_PF_INVALID_GAME_CENTER_AUTH_REQUEST,
/// E_PF_INVALID_GAME_CENTER_ID, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithGameCenterAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithGameCenterGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithGameCenterAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithGameCenterRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_ANDROID
/// <summary>
/// Signs the user in using their Google account credentials
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Android.
/// Google sign-in is accomplished by obtaining a Google OAuth 2.0 credential using the Google sign-in
/// for Android APIs on the device and passing it to this API. If this is the first time a user has signed
/// in with the Google account and CreateAccount is set to true, a new PlayFab account will be created
/// and linked to the Google account. Otherwise, if no PlayFab account is linked to the Google account,
/// an error indicating this will be returned, so that the title can guide the user through creation of
/// a PlayFab account. The current (recommended) method for obtaining a Google account credential in an
/// Android application is to call GoogleSignInAccount.getServerAuthCode() and send the auth code as the
/// ServerAuthCode parameter of this API. Before doing this, you must create an OAuth 2.0 web application
/// client ID in the Google API Console and configure its client ID and secret in the PlayFab Game Manager
/// Google Add-on for your title. This method does not require prompting of the user for additional Google
/// account permissions, resulting in a user experience with the least possible friction. For more information
/// about obtaining the server auth code, see https://developers.google.com/identity/sign-in/android/offline-access.
/// The previous (deprecated) method was to obtain an OAuth access token by calling GetAccessToken() on
/// the client and passing it as the AccessToken parameter to this API. for the with the Google OAuth
/// 2.0 Access Token. More information on this change can be found in the Google developer documentation
/// (https://android-developers.googleblog.com/2016/01/play-games-permissions-are-changing-in.html).
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithGoogleAccountGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithGoogleAccountAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithGoogleAccountRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithGoogleAccountAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_GOOGLE_O_AUTH_ERROR, E_PF_GOOGLE_O_AUTH_NO_ID_TOKEN_INCLUDED_IN_RESPONSE,
/// E_PF_GOOGLE_O_AUTH_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_GOOGLE_TOKEN, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED,
/// E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithGoogleAccountGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithGoogleAccountAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_GOOGLE_O_AUTH_ERROR, E_PF_GOOGLE_O_AUTH_NO_ID_TOKEN_INCLUDED_IN_RESPONSE,
/// E_PF_GOOGLE_O_AUTH_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_GOOGLE_TOKEN, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED,
/// E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithGoogleAccountAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithGoogleAccountGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithGoogleAccountAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithGoogleAccountRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_ANDROID
/// <summary>
/// Signs the user in using their Google Play Games account credentials
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Android.
/// Google Play Games sign-in is accomplished by obtaining a Google OAuth 2.0 credential using the Google
/// Play Games sign-in for Android APIs on the device and passing it to this API. If this is the first
/// time a user has signed in with the Google Play Games account and CreateAccount is set to true, a new
/// PlayFab account will be created and linked to the Google Play Games account. Otherwise, if no PlayFab
/// account is linked to the Google Play Games account, an error indicating this will be returned, so
/// that the title can guide the user through creation of a PlayFab account. The current (recommended)
/// method for obtaining a Google Play Games account credential in an Android application is to call GamesSignInClient.requestServerSideAccess()
/// and send the auth code as the ServerAuthCode parameter of this API. Before doing this, you must create
/// an OAuth 2.0 web application client ID in the Google API Console and configure its client ID and secret
/// in the PlayFab Game Manager Google Add-on for your title. This method does not require prompting of
/// the user for additional Google account permissions, resulting in a user experience with the least
/// possible friction. For more information about obtaining the server auth code, see https://developers.google.com/games/services/android/signin.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithGooglePlayGamesServicesGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithGooglePlayGamesServicesAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithGooglePlayGamesServicesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithGooglePlayGamesServicesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_GOOGLE_O_AUTH_ERROR, E_PF_GOOGLE_O_AUTH_NOT_CONFIGURED_FOR_TITLE,
/// E_PF_INVALID_GOOGLE_PLAY_GAMES_SERVER_AUTH_CODE, E_PF_INVALID_GOOGLE_TOKEN, E_PF_INVALID_GOOGLE_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithGooglePlayGamesServicesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithGooglePlayGamesServicesAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_GOOGLE_O_AUTH_ERROR, E_PF_GOOGLE_O_AUTH_NOT_CONFIGURED_FOR_TITLE,
/// E_PF_INVALID_GOOGLE_PLAY_GAMES_SERVER_AUTH_CODE, E_PF_INVALID_GOOGLE_TOKEN, E_PF_INVALID_GOOGLE_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithGooglePlayGamesServicesAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithGooglePlayGamesServicesGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithGooglePlayGamesServicesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithGooglePlayGamesServicesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using the vendor-specific iOS device identifier, returning a session identifier
/// that can subsequently be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// On iOS devices, the identifierForVendor (https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIDevice_Class/index.html#//apple_ref/occ/instp/UIDevice/identifierForVendor)
/// must be used as the DeviceId, as the UIDevice uniqueIdentifier has been deprecated as of iOS 5, and
/// use of the advertisingIdentifier for this purpose will result in failure of Apple's certification
/// process. If this is the first time a user has signed in with the iOS device and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the vendor-specific iOS device ID. In
/// this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the iOS device, an error indicating this will be returned, so that the title
/// can guide the user through creation of a PlayFab account. Please note that while multiple devices
/// of this type can be linked to a single user account, only the one most recently used to login (or
/// most recently linked) will be reflected in the user's account information. We will be updating to
/// show all linked devices in a future release. See also ClientLinkIOSDeviceIDAsync, ClientUnlinkIOSDeviceIDAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithIOSDeviceIDGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithIOSDeviceIDAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithIOSDeviceIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithIOSDeviceIDAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_CREATION_DISABLED,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_SIGNED_REQUEST_NOT_ALLOWED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithIOSDeviceIDGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithIOSDeviceIDAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_CREATION_DISABLED,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_SIGNED_REQUEST_NOT_ALLOWED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithIOSDeviceIDAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithIOSDeviceIDGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithIOSDeviceIDAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithIOSDeviceIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using a Kongregate player account.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// More details regarding Kongregate and their game authentication system can be found at http://developers.kongregate.com/docs/virtual-goods/authentication.
/// Developers must provide the Kongregate user ID and auth token that are generated using the Kongregate
/// client library. PlayFab will combine these identifiers with the title's unique Kongregate app ID to
/// log the player into the Kongregate system. If CreateAccount is set to true and there is not already
/// a user matched to this Kongregate ID, then PlayFab will create a new account for this user and link
/// the ID. In this case, no email or username will be associated with the PlayFab account. If there is
/// already a different PlayFab user linked with this account, then an error will be returned. See also
/// ClientLinkKongregateAsync, ClientUnlinkKongregateAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithKongregateGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithKongregateAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithKongregateRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithKongregateAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_KONGREGATE_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithKongregateGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithKongregateAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_KONGREGATE_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithKongregateAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithKongregateGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithKongregateAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithKongregateRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_NINTENDO_SWITCH
/// <summary>
/// Signs in the user with a Nintendo service account token.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Nintendo Switch.
/// See also ClientLinkNintendoServiceAccountAsync, ClientUnlinkNintendoServiceAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithNintendoServiceAccountGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithNintendoServiceAccountAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithNintendoServiceAccountRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithNintendoServiceAccountAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_NINTENDO_SWITCH_NOT_ENABLED_FOR_TITLE,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithNintendoServiceAccountGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithNintendoServiceAccountAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_NINTENDO_SWITCH_NOT_ENABLED_FOR_TITLE,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithNintendoServiceAccountAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithNintendoServiceAccountGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithNintendoServiceAccountAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithNintendoServiceAccountRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using a Nintendo Switch Device ID, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ClientLinkNintendoSwitchDeviceIdAsync, ClientUnlinkNintendoSwitchDeviceIdAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithNintendoSwitchDeviceIdGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithNintendoSwitchDeviceIdAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithNintendoSwitchDeviceIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithNintendoSwitchDeviceIdAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithNintendoSwitchDeviceIdGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithNintendoSwitchDeviceIdAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithNintendoSwitchDeviceIdAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithNintendoSwitchDeviceIdGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithNintendoSwitchDeviceIdAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithNintendoSwitchDeviceIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

/// <summary>
/// Logs in a user with an Open ID Connect JWT created by an existing relationship between a title and
/// an Open ID Connect provider.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// See also ClientLinkOpenIdConnectAsync, ClientUnlinkOpenIdConnectAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithOpenIdConnectGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithOpenIdConnectAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithOpenIdConnectRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithOpenIdConnectAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_IDENTITY_PROVIDER_ID, E_PF_INVALID_SIGNATURE,
/// E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithOpenIdConnectGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithOpenIdConnectAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_IDENTITY_PROVIDER_ID, E_PF_INVALID_SIGNATURE,
/// E_PF_INVALID_SIGNATURE_TIME, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithOpenIdConnectAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithOpenIdConnectGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithOpenIdConnectAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithOpenIdConnectRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;


#if 0
/// <summary>
/// Signs the user into the PlayFab account, returning a session identifier that can subsequently be
/// used for API calls which require an authenticated user. Unlike most other login API calls, LoginWithPlayFab
/// does not permit the creation of new accounts via the CreateAccountFlag. Username/Password credentials
/// may be used to create accounts via RegisterPlayFabUser, or added to existing accounts using AddUsernamePassword.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Username and password lengths are provided for information purposes. The server will validate that
/// data passed in conforms to the field definition and report errors appropriately. It is recommended
/// that developers not perform this validation locally, so that future updates to the username or password
/// do not require client updates. See also ClientAddUsernamePasswordAsync, ClientLoginWithEmailAddressAsync,
/// ClientRegisterPlayFabUserAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithPlayFabGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithPlayFabAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithPlayFabRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithPlayFabAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_SIGNATURE,
/// E_PF_INVALID_USERNAME_OR_PASSWORD, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithPlayFabGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithPlayFabAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_SIGNATURE,
/// E_PF_INVALID_USERNAME_OR_PASSWORD, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithPlayFabAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithPlayFabGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithPlayFabAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithPlayFabRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_SONY_PLAYSTATION_4 || HC_PLATFORM == HC_PLATFORM_SONY_PLAYSTATION_5
/// <summary>
/// Signs the user in using a PlayStation :tm: Network authentication code, returning a session identifier
/// that can subsequently be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Sony PlayStation®.
/// If this is the first time a user has signed in with the PlayStation :tm: Network account and CreateAccount
/// is set to true, a new PlayFab account will be created and linked to the PlayStation :tm: Network account.
/// In this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the PlayStation :tm: Network account, an error indicating this will be returned,
/// so that the title can guide the user through creation of a PlayFab account. See also ClientLinkPSNAccountAsync,
/// ClientUnlinkPSNAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithPSNGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithPSNAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithPSNRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithPSNAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_PSN_AUTH_CODE,
/// E_PF_INVALID_PSN_ISSUER_ID, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_PSN_INACCESSIBLE, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithPSNGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithPSNAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_PSN_AUTH_CODE,
/// E_PF_INVALID_PSN_ISSUER_ID, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_PSN_INACCESSIBLE, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithPSNAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithPSNGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithPSNAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithPSNRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Signs the user in using a Steam authentication ticket, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, GDK, Linux, and macOS.
/// Steam sign-in is accomplished with the Steam Session Ticket. More information on the Ticket can be
/// found in the Steamworks SDK, here: https://partner.steamgames.com/documentation/auth. NOTE: For Steam
/// authentication to work, the title must be configured with the Steam Application ID and Web API Key
/// in the PlayFab Game Manager (under Steam in the Add-ons Marketplace). You can obtain a Web API Key
/// from the Permissions page of any Group associated with your App ID in the Steamworks site. If this
/// is the first time a user has signed in with the Steam account and CreateAccount is set to true, a
/// new PlayFab account will be created and linked to the provided account's Steam ID. In this case, no
/// email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is
/// linked to the Steam account, an error indicating this will be returned, so that the title can guide
/// the user through creation of a PlayFab account. See also ClientLinkSteamAccountAsync, ClientUnlinkSteamAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithSteamGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithSteamAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithSteamRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithSteamAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_STEAM_TICKET, E_PF_INVALID_STEAM_USERNAME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_STEAM_NOT_ENABLED_FOR_TITLE, E_PF_STEAM_USER_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithSteamGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithSteamAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_STEAM_TICKET, E_PF_INVALID_STEAM_USERNAME,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_STEAM_NOT_ENABLED_FOR_TITLE, E_PF_STEAM_USER_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithSteamAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithSteamGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithSteamAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithSteamRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using a Twitch access token.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// More details regarding Twitch and their authentication system can be found at https://github.com/justintv/Twitch-API/blob/master/authentication.md.
/// Developers must provide the Twitch access token that is generated using one of the Twitch authentication
/// flows. PlayFab will use the title's unique Twitch Client ID to authenticate the token and log in to
/// the PlayFab system. If CreateAccount is set to true and there is not already a user matched to the
/// Twitch username that generated the token, then PlayFab will create a new account for this user and
/// link the ID. In this case, no email or username will be associated with the PlayFab account. If there
/// is already a different PlayFab user linked with this account, then an error will be returned. See
/// also ClientLinkTwitchAsync, ClientUnlinkTwitchAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithTwitchGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithTwitchAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithTwitchRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithTwitchAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_TWITCH_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_TWITCH_RESPONSE_ERROR or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithTwitchGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithTwitchAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_TWITCH_TOKEN,
/// E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_TWITCH_RESPONSE_ERROR or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithTwitchAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithTwitchGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithTwitchAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithTwitchRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Signs the user in using a Xbox Live Token, returning a session identifier that can subsequently be
/// used for API calls which require an authenticated user. If possible, PFAuthenticationLoginWithXUserAsync
/// should be preferred, as it will more seamlessly handle automatic token refresh.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, GDK, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithXboxGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithXboxAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithXboxRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithXboxAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_INVALID_XBOX_LIVE_TOKEN,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_SIGNED_REQUEST_NOT_ALLOWED, E_PF_XBOX_INACCESSIBLE,
/// E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST, E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithXboxGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithXboxAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_INVALID_XBOX_LIVE_TOKEN,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_SIGNED_REQUEST_NOT_ALLOWED, E_PF_XBOX_INACCESSIBLE,
/// E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST, E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithXboxAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithXboxGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle. Used to address situations where the EntityToken expired and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithXboxAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithXboxRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_GDK
/// <summary>
/// Signs the user in using an XUserHandle, returning a session identifier that can subsequently be
/// used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on GDK.
/// If this is the first time a user has signed in with the Xbox Live account and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the Xbox Live account. In this case,
/// no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account
/// is linked to the Xbox Live account, an error indicating this will be returned, so that the title can
/// guide the user through creation of a PlayFab account. See also ClientLinkXboxAccountAsync, ClientUnlinkXboxAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationLoginWithXUserGetResult"/> to get the result.
/// </remarks>
PF_API PFAuthenticationLoginWithXUserAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationLoginWithXUserRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationLoginWithXUserAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_INVALID_XBOX_LIVE_TOKEN,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_SIGNED_REQUEST_NOT_ALLOWED, E_PF_XBOX_INACCESSIBLE,
/// E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST, E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationLoginWithXUserGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationLoginWithXUserAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_INVALID_XBOX_LIVE_TOKEN,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_SIGNED_REQUEST_NOT_ALLOWED, E_PF_XBOX_INACCESSIBLE,
/// E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST, E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationLoginWithXUserAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationLoginWithXUserGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_opt_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Reauthenticates an existing PFEntityHandle using an XUserHandle. Used to address situations where the EntityToken expired
/// and the PlayFab SDK is unable to refresh it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to re-login.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the result of the operation.
/// If successful, the cached EntityToken for the PFEntityHandle will be updated in place.
/// </remarks>
PF_API PFAuthenticationReLoginWithXUserAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationLoginWithXUserRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if 0
/// <summary>
/// Registers a new Playfab user account, returning a session identifier that can subsequently be used
/// for API calls which require an authenticated user. You must supply a username and an email address.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ClientLoginWithEmailAddressAsync, ClientLoginWithPlayFabAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationRegisterPlayFabUserGetResultSize"/>
/// and <see cref="PFAuthenticationRegisterPlayFabUserGetResult"/> to get the result.
/// </remarks>
PF_API PFAuthenticationRegisterPlayFabUserAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_ const PFAuthenticationRegisterPlayFabUserRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RegisterPlayFabUser call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EMAIL_ADDRESS_NOT_AVAILABLE,
/// E_PF_ENCRYPTION_KEY_MISSING, E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_INVALID_EMAIL_ADDRESS,
/// E_PF_INVALID_PARTNER_RESPONSE, E_PF_INVALID_PASSWORD, E_PF_INVALID_USERNAME, E_PF_NAME_NOT_AVAILABLE,
/// E_PF_PROFANE_DISPLAY_NAME, E_PF_SIGNED_REQUEST_NOT_ALLOWED, E_PF_USERNAME_NOT_AVAILABLE or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
PF_API PFAuthenticationRegisterPlayFabUserGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFAuthenticationRegisterPlayFabUserAsync call.
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
PF_API PFAuthenticationRegisterPlayFabUserGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFAuthenticationRegisterPlayFabUserResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using the Android device identifier, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// On Android devices, the recommendation is to use the Settings.Secure.ANDROID_ID as the AndroidDeviceId,
/// as described in this blog post (http://android-developers.blogspot.com/2011/03/identifying-app-installations.html).
/// More information on this identifier can be found in the Android documentation (http://developer.android.com/reference/android/provider/Settings.Secure.html).
/// If this is the first time a user has signed in with the Android device and CreateAccount is set to
/// true, a new PlayFab account will be created and linked to the Android device ID. In this case, no
/// email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is
/// linked to the Android device, an error indicating this will be returned, so that the title can guide
/// the user through creation of a PlayFab account. Please note that while multiple devices of this type
/// can be linked to a single user account, only the one most recently used to login (or most recently
/// linked) will be reflected in the user's account information. We will be updating to show all linked
/// devices in a future release.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithAndroidDeviceIDGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithAndroidDeviceIDAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationServerLoginWithAndroidDeviceIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithAndroidDeviceIDAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithAndroidDeviceIDGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithAndroidDeviceIDAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithAndroidDeviceIDAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithAndroidDeviceIDGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using a custom unique identifier generated by the title, returning a session identifier
/// that can subsequently be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// It is highly recommended that developers ensure that it is extremely unlikely that a customer could
/// generate an ID which is already in use by another customer. If this is the first time a user has signed
/// in with the Custom ID and CreateAccount is set to true, a new PlayFab account will be created and
/// linked to the Custom ID. In this case, no email or username will be associated with the PlayFab account.
/// Otherwise, if no PlayFab account is linked to the Custom ID, an error indicating this will be returned,
/// so that the title can guide the user through creation of a PlayFab account.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithCustomIDGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithCustomIDAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationServerLoginWithCustomIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithCustomIDAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithCustomIDGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithCustomIDAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithCustomIDAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithCustomIDGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using the iOS device identifier, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// On iOS devices, the identifierForVendor (https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIDevice_Class/index.html#//apple_ref/occ/instp/UIDevice/identifierForVendor)
/// must be used as the DeviceId, as the UIDevice uniqueIdentifier has been deprecated as of iOS 5, and
/// use of the advertisingIdentifier for this purpose will result in failure of Apple's certification
/// process. If this is the first time a user has signed in with the iOS device and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the vendor-specific iOS device ID. In
/// this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the iOS device, an error indicating this will be returned, so that the title
/// can guide the user through creation of a PlayFab account.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithIOSDeviceIDGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithIOSDeviceIDAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationServerLoginWithIOSDeviceIDRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithIOSDeviceIDAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithIOSDeviceIDGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithIOSDeviceIDAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithIOSDeviceIDAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithIOSDeviceIDGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if 0
/// <summary>
/// Signs the user in using a PlayStation :tm: Network authentication code, returning a session identifier
/// that can subsequently be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// If this is the first time a user has signed in with the PlayStation :tm: Network account and CreateAccount
/// is set to true, a new PlayFab account will be created and linked to the PlayStation :tm: Network account.
/// In this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the PlayStation :tm: Network account, an error indicating this will be returned,
/// so that the title can guide the user through creation of a PlayFab account. See also ServerLinkPSNAccountAsync,
/// ServerUnlinkPSNAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithPSNGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithPSNAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationServerLoginWithPSNRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithPSNAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_PSN_AUTH_CODE,
/// E_PF_INVALID_PSN_ISSUER_ID, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_PSN_INACCESSIBLE, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithPSNGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithPSNAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_MISSING,
/// E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED, E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_PSN_AUTH_CODE,
/// E_PF_INVALID_PSN_ISSUER_ID, E_PF_PLAYER_SECRET_ALREADY_CONFIGURED, E_PF_PLAYER_SECRET_NOT_CONFIGURED,
/// E_PF_PSN_INACCESSIBLE, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithPSNAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithPSNGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Securely login a game client from an external server backend using a custom identifier for that player.
/// Server Custom ID and Client Custom ID are mutually exclusive and cannot be used to retrieve the same
/// player account.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithServerCustomIdGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithServerCustomIdAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationLoginWithServerCustomIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithServerCustomIdAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithServerCustomIdGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithServerCustomIdAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithServerCustomIdAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithServerCustomIdGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Signs the user in using an Steam ID, returning a session identifier that can subsequently be used
/// for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// If this is the first time a user has signed in with the Steam ID and CreateAccount is set to true,
/// a new PlayFab account will be created and linked to the Steam account. In this case, no email or username
/// will be associated with the PlayFab account. Otherwise, if no PlayFab account is linked to the Steam
/// account, an error indicating this will be returned, so that the title can guide the user through creation
/// of a PlayFab account. Steam users that are not logged into the Steam Client app will only have their
/// Steam username synced, other data, such as currency and country will not be available until they login
/// while the Client is open.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithSteamIdGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithSteamIdAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationLoginWithSteamIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithSteamIdAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_INVALID_STEAM_USERNAME, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_STEAM_NOT_ENABLED_FOR_TITLE, E_PF_STEAM_USER_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithSteamIdGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithSteamIdAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_INVALID_SIGNATURE, E_PF_INVALID_SIGNATURE_TIME, E_PF_INVALID_STEAM_USERNAME, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_STEAM_NOT_ENABLED_FOR_TITLE, E_PF_STEAM_USER_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithSteamIdAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithSteamIdGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Signs the user in using a Xbox Live Token from an external server backend, returning a session identifier
/// that can subsequently be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// If this is the first time a user has signed in with the Xbox Live account and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the Xbox Live account. In this case,
/// no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account
/// is linked to the Xbox Live account, an error indicating this will be returned, so that the title can
/// guide the user through creation of a PlayFab account. See also ServerLinkXboxAccountAsync, ServerUnlinkXboxAccountAsync.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithXboxGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithXboxAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationServerLoginWithXboxRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithXboxAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST, E_PF_XBOX_XASS_EXCHANGE_FAILURE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithXboxGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithXboxAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_EXPIRED_XBOX_LIVE_TOKEN, E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED,
/// E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST, E_PF_XBOX_XASS_EXCHANGE_FAILURE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithXboxAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithXboxGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Signs the user in using an Xbox ID and Sandbox ID, returning a session identifier that can subsequently
/// be used for API calls which require an authenticated user
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// If this is the first time a user has signed in with the Xbox ID and CreateAccount is set to true,
/// a new PlayFab account will be created and linked to the Xbox Live account. In this case, no email
/// or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is linked
/// to the Xbox Live account, an error indicating this will be returned, so that the title can guide the
/// user through creation of a PlayFab account.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationServerLoginWithXboxIdGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationServerLoginWithXboxIdAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationLoginWithXboxIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PFAuthenticationServerLoginWithXboxIdAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST,
/// E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationServerLoginWithXboxIdGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationServerLoginWithXboxIdAsync call. The PFEntityHandle will always be returned, but the additional info
/// in the PFAuthenticationLoginResult is only returned if a buffer is provided.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the Login result value and its fields.</param>
/// <param name="result">Pointer to the LoginResult object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EVALUATION_MODE_PLAYER_COUNT_EXCEEDED,
/// E_PF_REQUEST_VIEW_CONSTRAINT_PARAMS_NOT_ALLOWED, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_REJECTED_XSTS_EXCHANGE_REQUEST,
/// E_PF_XBOX_XASS_EXCHANGE_FAILURE or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationServerLoginWithXboxIdAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed. If returned, 'result' is a pointer within 'buffer' and does not need to be freed separately.
/// </remarks>
PF_API PFAuthenticationServerLoginWithXboxIdGetResult(
    _Inout_ XAsyncBlock* async,
    _Outptr_ PFAuthenticationEntityTokenResponse const** entityTokenResponse,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_opt_ PFAuthenticationLoginResult const** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Create a game_server entity token and return a new or existing game_server entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Create or return a game_server entity token. Caller must be a title entity.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationAuthenticateGameServerWithCustomIdGetResultSize"/>
/// and <see cref="PFAuthenticationAuthenticateGameServerWithCustomIdGetResult"/> to get the result.
/// </remarks>
PF_API PFAuthenticationAuthenticateGameServerWithCustomIdAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationAuthenticateCustomIdRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the result from a PFAuthenticationAuthenticateGameServerWithCustomIdAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <param name="newlyCreated">Will be set to true if the account was newly created on this authentication and false otherwise</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationAuthenticateGameServerWithCustomIdAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed.
/// </remarks>
PF_API PFAuthenticationAuthenticateGameServerWithCustomIdGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle,
    _Out_opt_ bool* newlyCreated
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Delete a game_server entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Delete a game_server entity. The caller can be the game_server entity attempting to delete itself.
/// Or a title entity attempting to delete game_server entities for this title.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFAuthenticationDeleteAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationDeleteRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Method to exchange a legacy AuthenticationTicket or title SecretKey for an Entity Token or to refresh
/// a still valid Entity Token.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API must be called with X-SecretKey, X-Authentication or X-EntityToken headers. An optional
/// EntityKey may be included to attempt to set the resulting EntityToken to a specific entity, however
/// the entity must be a relation of the caller, such as the master_player_account of a character. If
/// sending X-EntityToken the account will be marked as freshly logged in and will issue a new token.
/// If using X-Authentication or X-EntityToken the header must still be valid and cannot be expired or
/// revoked.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationGetEntityGetResult"/> to
/// get the result.
/// </remarks>
PF_API PFAuthenticationGetEntityAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationGetEntityRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFAuthenticationGetEntityAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_BROKEN,
/// E_PF_NO_VALID_CERTIFICATE_FOR_AAD or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationGetEntityAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed.
/// </remarks>
PF_API PFAuthenticationGetEntityGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Method to exchange a legacy AuthenticationTicket or title SecretKey for an Entity Token or to refresh
/// a still valid Entity Token.
/// </summary>
/// <param name="serviceConfigHandle">PFServiceConfigHandle returned from PFServiceConfigCreateHandle call.</param>
/// <param name="secretKey">Title Secret Key used to authenticate the service request.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API must be called with X-SecretKey, X-Authentication or X-EntityToken headers. An optional
/// EntityKey may be included to attempt to set the resulting EntityToken to a specific entity, however
/// the entity must be a relation of the caller, such as the master_player_account of a character. If
/// sending X-EntityToken the account will be marked as freshly logged in and will issue a new token.
/// If using X-Authentication or X-EntityToken the header must still be valid and cannot be expired or
/// revoked.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationGetEntityWithSecretKeyGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFAuthenticationGetEntityWithSecretKeyAsync(
    _In_ PFServiceConfigHandle serviceConfigHandle,
    _In_z_ const char* secretKey,
    _In_ const PFAuthenticationGetEntityRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFAuthenticationGetEntityWithSecretKeyAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="entityHandle">PFEntityHandle which can be used to authenticate other PlayFab API calls.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENCRYPTION_KEY_BROKEN,
/// E_PF_NO_VALID_CERTIFICATE_FOR_AAD or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// If the PFAuthenticationGetEntityWithSecretKeyAsync call fails, entityHandle with be null. Otherwise, the handle must be closed with PFEntityCloseHandle
/// when it is no longer needed.
/// </remarks>
PF_API PFAuthenticationGetEntityWithSecretKeyGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFEntityHandle* entityHandle
) noexcept;

#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Method for a server to validate a client provided EntityToken. Only callable by the title entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Given an entity token, validates that it hasn't expired or been revoked and will return details of
/// the owner.
///
/// When the asynchronous task is complete, call <see cref="PFAuthenticationValidateEntityTokenGetResultSize"/>
/// and <see cref="PFAuthenticationValidateEntityTokenGetResult"/> to get the result.
/// </remarks>
PF_API PFAuthenticationValidateEntityTokenAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFAuthenticationValidateEntityTokenRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ValidateEntityToken call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFAuthenticationValidateEntityTokenGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFAuthenticationValidateEntityTokenAsync call.
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
PF_API PFAuthenticationValidateEntityTokenGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFAuthenticationValidateEntityTokenResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#endif

}
