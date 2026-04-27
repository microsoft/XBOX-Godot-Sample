// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

extern "C"
{
/// <summary>
/// Struct the defines retry settings for PlayFab HTTP requests.
/// </summary>
struct PFHttpRetrySettings
{
    /// <summary>
    /// Controls whether the SDK should automatically retry select errors. There are certain errors that will never be
    /// handled internally and will always be returned directly to the client. Default value is 'true'.
    /// </summary>
    bool allowRetry;

    /// <summary>
    /// The minimum number of seconds the SDK will wait after an HTTP failure before retrying the call. 
    /// The default and minimum value is 2 seconds.
    /// </summary>
    uint32_t minimumRetryDelayInSeconds;

    /// <summary>
    /// The maximum number of seconds the SDK will attempt to retry an HTTP request before returning to the client.
    /// The default value is 20 seconds.
    /// </summary>
    uint32_t timeoutWindowInSeconds;
};

/// <summary>
/// Struct that defines generic Http settings for PlayFab HTTP requests.
/// Currently holds settings to enable gzip compression on all sdk responses.
/// </summary>
struct PFHttpSettings
{
    /// <summary>
    /// Controls whether the SDK currently requests that incoming responses be compressed. 
    /// Response compression is enabled by specifying the Accept-Encoding Header as "application/gzip".
    /// In order to decompress a compressed response provided by an API endpoint HCHttpCallResponseSetGzipCompressed 
    /// must be called prior to calling HCHttpCallPerformAsync.
    /// </summary>
    bool requestResponseCompression;
};


/// <summary>
/// Sets global HTTP retry settings for the SDK.
/// </summary>
/// <param name="settings">Pointer to retry settings.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFSetHttpRetrySettings(
    _In_ PFHttpRetrySettings const* settings
) noexcept;

/// <summary>
/// Gets the current HTTP retry settings for the SDK.
/// </summary>
/// <param name="settings">Pointer to retry settings that will be populated.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFGetHttpRetrySettings(
    _Out_ PFHttpRetrySettings* settings
) noexcept;

/// <summary>
/// Sets global generic HTTP settings for the SDK.
/// </summary>
/// <param name="settings">Pointer to generic settings.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFSetHttpSettings(
    _In_ PFHttpSettings const* settings
) noexcept;

/// <summary>
/// Gets the current generic HTTP settings for the SDK.
/// </summary>
/// <param name="settings">Pointer to generic settings that will be populated.</param>
/// <returns>Result code for this API operation.</returns>
PF_API PFGetHttpSettings(
    _Out_ PFHttpSettings* settings
) noexcept;

}