// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>

extern "C"
{

typedef struct PFServiceConfig* PFServiceConfigHandle;

/// <summary>
/// Creates a PlayFab service configuration.
/// </summary>
/// <param name="apiEndpoint">String used to connect to a PlayFab Service instance.</param>
/// <param name="playFabTitleId">PlayFab TitleId for the title. Found in the Game Manager for your title on the PlayFab Website.</param>
/// <param name="serviceConfigHandle">Pointer to a PFServiceConfigHandle to write.</param>
/// <returns>Result code for this API operation.  Possible values are S_OK, E_PF_NOT_INITIALIZED, or E_INVALIDARG.</returns>
PF_API PFServiceConfigCreateHandle(
    _In_z_ const char* apiEndpoint,
    _In_z_ const char* playFabTitleId,
    _Out_ PFServiceConfigHandle* serviceConfigHandle
) noexcept;

/// <summary>
/// Duplicates a PFServiceConfigHandle.
/// </summary>
/// <param name="handle">ServiceConfig handle to duplicate.</param>
/// <param name="duplicatedHandle">The duplicated handle.</param>
/// <returns>Result code for this API operation.</returns> 
/// <remarks>
/// Both the duplicated handle and the original handle need to be closed with PFServiceConfigCloseHandle when they
/// are no longer needed.
/// </remarks>
PF_API PFServiceConfigDuplicateHandle(
    PFServiceConfigHandle handle,
    PFServiceConfigHandle* duplicatedHandle
) noexcept;

/// <summary>
/// Closes a PFServiceConfigHandle.
/// </summary>
/// <param name="handle">ServiceConfig handle to close.</param>
/// <returns>Result code for this API operation.</returns>
PF_API_(void) PFServiceConfigCloseHandle(
    PFServiceConfigHandle handle
) noexcept;

/// <summary>
/// Gets the size of the buffer needed to hold the API endpoint for a service configuration.
/// </summary>
/// <param name="handle">ServiceConfig handle.</param>
/// <param name="apiEndpointSize">Buffer size required for the API endpoint string (including null terminator).</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFServiceConfigGetAPIEndpointSize(
    _In_  PFServiceConfigHandle handle,
    _Out_ size_t* apiEndpointSize
) noexcept;

/// <summary>
/// Gets the API endpoint for a service configuration.
/// </summary>
/// <param name="handle">ServiceConfig handle.</param>
/// <param name="apiEndpointSize">Size of the provided buffer. Required size can be obtained via PFServiceConfigGetAPIEndpointSize.</param>
/// <param name="apiEndpoint">Buffer the API endpoint string will be written to.</param>
/// <param name="apiEndpointUsed">The number of bytes used in the buffer including the null terminator.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFServiceConfigGetAPIEndpoint(
    _In_ PFServiceConfigHandle handle,
    _In_ size_t apiEndpointSize,
    _Out_writes_(apiEndpointSize) char* apiEndpoint,
    _Out_opt_ size_t* apiEndpointUsed
) noexcept;

/// <summary>
/// Gets the size of the buffer needed to hold the PlayFab titleId for a service configuration.
/// </summary>
/// <param name="handle">ServiceConfig handle.</param>
/// <param name="titleIdSize">Buffer size required for the titleId (including null terminator).</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFServiceConfigGetTitleIdSize(
    _In_  PFServiceConfigHandle handle,
    _Out_ size_t* titleIdSize
) noexcept;

/// <summary>
/// Gets the PlayFab titleId for a service configuration.
/// </summary>
/// <param name="handle">ServiceConfig handle.</param>
/// <param name="titleIdSize">Size of the provided buffer. Required size can be obtained via PFServiceConfigGetTitleIdSize.</param>
/// <param name="titleId">Buffer the titleId will be written to.</param>
/// <param name="titleIdUsed">The number of bytes used in the buffer including the null terminator.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFServiceConfigGetTitleId(
    _In_ PFServiceConfigHandle handle,
    _In_ size_t titleIdSize,
    _Out_writes_(titleIdSize) char* titleId,
    _Out_opt_ size_t* titleIdUsed
) noexcept;

}
