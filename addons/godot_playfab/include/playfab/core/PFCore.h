// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>
#include <playfab/core/PFErrors.h>
#include <playfab/core/PFLocalUser.h>
#include <playfab/core/PFServiceConfig.h>
#include <playfab/core/PFEntity.h>
#include <playfab/core/PFAuthentication.h>
#include <playfab/core/PFPlatform.h>
#include <playfab/core/PFEvents.h>
#include <playfab/core/PFEventPipeline.h>
#include <playfab/core/PFTrace.h>

extern "C"
{

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_IOS || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Initialize PlayFabCore global state. Custom platform hooks must be configured prior to calling PFInitialize.
/// </summary>
/// <param name="backgroundQueue">An XTaskQueue that should be used for background work. If no queue is provided then a default (threadpool) queue will be used.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFInitialize(
    _In_opt_ XTaskQueueHandle backgroundQueue
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_ANDROID
/// <summary>
/// Initializes PlayFabCore global state. Custom platform hooks must be configured prior to calling PFInitialize.
/// </summary>
/// <param name="backgroundQueue">An XTaskQueue that should be used for background work. If no queue is provided then a default (threadpool) queue will be used.</param>
/// <param name="args">Args for initializing libHttpClient.</param>
/// <remarks>
/// This will internally call PFInitialize(nullptr) if it hasn't been called already by the
/// title. If control of PFCore background work is needed, the title should explicitly call
/// PFInitialize and PFUninitialize.
/// </remarks>
/// <returns>Result code for this API operation.</returns>
PF_API PFInitialize(
    _In_opt_ XTaskQueueHandle backgroundQueue,
    _In_ JavaVM* javaVm,
    _In_ jobject applicationContext
) noexcept;
#endif

/// <summary>
/// Cleanup PlayFab global state.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>Asynchronous result returned via XAsyncGetStatus.</remarks>
PF_API PFUninitializeAsync(
    _In_ XAsyncBlock* async
) noexcept;

}
