// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFCore.h>
#include <playfab/services/PFFriends.h>
#include <playfab/services/PFAccountManagement.h>
#include <playfab/services/PFPlatformSpecific.h>
#include <playfab/services/PFPlayerDataManagement.h>
#include <playfab/services/PFCloudScript.h>
#include <playfab/services/PFSegments.h>
#include <playfab/services/PFTitleDataManagement.h>
#include <playfab/services/PFPushNotifications.h>
#include <playfab/services/PFData.h>
#include <playfab/services/PFInventory.h>
#include <playfab/services/PFCatalog.h>
#include <playfab/services/PFExperimentation.h>
#include <playfab/services/PFGroups.h>
#include <playfab/services/PFLeaderboards.h>
#include <playfab/services/PFStatistics.h>
#include <playfab/services/PFLocalization.h>
#include <playfab/services/PFMultiplayerServer.h>
#include <playfab/services/PFProfiles.h>
#if HC_PLATFORM != HC_PLATFORM_GDK
#include <httpClient/httpClient.h>
#endif

extern "C"
{

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_IOS || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Initializes PlayFab Services global state
/// </summary>
/// <param name="reserved">Reserved for future use.</param>
/// <remarks>
/// This will internally call PFInitialize(nullptr) if it hasn't been called already by the
/// title. If control of PFCore background work is needed, the title should explicitly call
/// PFInitialize and PFUninitialize.
/// </remarks>
/// <returns>Result code for this API operation.</returns>
PF_API PFServicesInitialize(
    _In_opt_ XTaskQueueHandle reserved
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_ANDROID
/// <summary>
/// Initializes PlayFab Services global state
/// </summary>
/// <param name="backgroundQueue">An XTaskQueue that should be used for background work. If no queue is provided then a default (threadpool) queue will be used.</param>
/// <param name="args">Arguments for initializing libHttpClient. It contains a Java VM and an application context, both can be retrieved through an Android app activity.</param>
/// <remarks>
/// This will internally call PFInitialize(nullptr) if it hasn't been called already by the
/// title. If control of PFCore background work is needed, the title should explicitly call
/// PFInitialize and PFUninitialize.
/// </remarks>
/// <returns>Result code for this API operation.</returns>
PF_API PFServicesInitialize(
    _In_opt_ XTaskQueueHandle backgroundQueue,
    _In_ HCInitArgs* initArgs
) noexcept;
#endif

/// <summary>
/// Cleanup PlayFab Services global state.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This will internally call PFUninitializeAsync() if PFServicesInitialize() needed 
/// to call PFInitialize() interally.
/// Asynchronous result returned via XAsyncGetStatus.</remarks>
PF_API PFServicesUninitializeAsync(
    _In_ XAsyncBlock* async
) noexcept;

}

