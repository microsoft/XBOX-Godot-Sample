// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFPlatformSpecificTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if HC_PLATFORM == HC_PLATFORM_ANDROID
/// <summary>
/// Registers the Android device to receive push notifications
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Android.
/// More information can be found on configuring your game for the Google Cloud Messaging service in
/// the Google developer documentation, here: http://developer.android.com/google/gcm/client.html. The
/// steps to configure and send Push Notifications is described in the PlayFab tutorials, here: https://docs.microsoft.com/gaming/playfab/features/engagement/push-notifications/quickstart.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_NO_PUSH_NOTIFICATION_ARN_FOR_TITLE, E_PF_PUSH_SERVICE_ERROR or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </remarks>
PF_API PFPlatformSpecificClientAndroidDevicePushNotificationRegistrationAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlatformSpecificAndroidDevicePushNotificationRegistrationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if 0
/// <summary>
/// Uses the supplied OAuth code to refresh the internally cached player PlayStation :tm: Network auth
/// token
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_INVALID_PSN_AUTH_CODE, E_PF_PSN_INACCESSIBLE or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFPlatformSpecificClientRefreshPSNAuthTokenAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlatformSpecificRefreshPSNAuthTokenRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_IOS
/// <summary>
/// Registers the iOS device to receive push notifications
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on iOS.
/// The steps to configure and send Push Notifications is described in the PlayFab tutorials, here: https://docs.microsoft.com/gaming/playfab/features/engagement/push-notifications/quickstart.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_NO_PUSH_NOTIFICATION_ARN_FOR_TITLE, E_PF_PUSH_SERVICE_ERROR or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </remarks>
PF_API PFPlatformSpecificClientRegisterForIOSPushNotificationAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlatformSpecificRegisterForIOSPushNotificationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Awards the specified users the specified Steam achievements
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFPlatformSpecificServerAwardSteamAchievementGetResultSize"/>
/// and <see cref="PFPlatformSpecificServerAwardSteamAchievementGetResult"/> to get the result.
/// </remarks>
PF_API PFPlatformSpecificServerAwardSteamAchievementAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlatformSpecificAwardSteamAchievementRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerAwardSteamAchievement call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_USERIS_NOT_VALID
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlatformSpecificServerAwardSteamAchievementGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlatformSpecificServerAwardSteamAchievementAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_USERIS_NOT_VALID
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFPlatformSpecificServerAwardSteamAchievementGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlatformSpecificAwardSteamAchievementResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif


}