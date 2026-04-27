// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFPushNotificationsTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Sends an iOS/Android Push Notification to a specific user, if that user's device has been configured
/// for Push Notifications in PlayFab. If a user has linked both Android and iOS devices, both will be
/// notified.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_PUSH_NOT_ENABLED_FOR_ACCOUNT, E_PF_PUSH_SERVICE_ERROR or any of the
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFPushNotificationsServerSendPushNotificationAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPushNotificationsSendPushNotificationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Sends an iOS/Android Push Notification template to a specific user, if that user's device has been
/// configured for Push Notifications in PlayFab. If a user has linked both Android and iOS devices, both
/// will be notified.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_PUSH_NOT_ENABLED_FOR_ACCOUNT, E_PF_PUSH_NOTIFICATION_TEMPLATE_NOT_FOUND,
/// E_PF_PUSH_SERVICE_ERROR or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </remarks>
PF_API PFPushNotificationsServerSendPushNotificationFromTemplateAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPushNotificationsSendPushNotificationFromTemplateRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}