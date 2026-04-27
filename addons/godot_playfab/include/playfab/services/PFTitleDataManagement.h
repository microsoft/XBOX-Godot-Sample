// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFTitleDataManagementTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Retrieves the key-value store of custom publisher settings
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This API is designed to return publisher-specific values which can be read, but not written to, by
/// the client. This data is shared across all titles assigned to a particular publisher, and can be used
/// for cross-game coordination. Only titles assigned to a publisher can use this API. For more information
/// email helloplayfab@microsoft.com. Note that there may up to a minute delay in between updating title
/// data and this API call returning the newest value.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementClientGetPublisherDataGetResultSize"/>
/// and <see cref="PFTitleDataManagementClientGetPublisherDataGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementClientGetPublisherDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFTitleDataManagementGetPublisherDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetPublisherData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFTitleDataManagementClientGetPublisherDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementClientGetPublisherDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementClientGetPublisherDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetPublisherDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves the current server time
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This query retrieves the current time from one of the servers in PlayFab. Please note that due to
/// clock drift between servers, there is a potential variance of up to 5 seconds.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementClientGetTimeGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFTitleDataManagementClientGetTimeAsync(
    _In_ PFEntityHandle entityHandle,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementClientGetTimeAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFTitleDataManagementGetTimeResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFTitleDataManagementClientGetTimeGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFTitleDataManagementGetTimeResult* result
) noexcept;

/// <summary>
/// Retrieves the key-value store of custom title settings
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This API is designed to return title specific values which can be read, but not written to, by the
/// client. For example, a developer could choose to store values which modify the user experience, such
/// as enemy spawn rates, weapon strengths, movement speeds, etc. This allows a developer to update the
/// title without the need to create, test, and ship a new build. If the player belongs to an experiment
/// variant that uses title data overrides, the overrides are applied automatically and returned with
/// the title data. Note that there may up to a minute delay in between updating title data and this API
/// call returning the newest value.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementClientGetTitleDataGetResultSize"/>
/// and <see cref="PFTitleDataManagementClientGetTitleDataGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementClientGetTitleDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFTitleDataManagementGetTitleDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetTitleData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFTitleDataManagementClientGetTitleDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementClientGetTitleDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementClientGetTitleDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetTitleDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves the title news feed, as configured in the developer portal
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementClientGetTitleNewsGetResultSize"/>
/// and <see cref="PFTitleDataManagementClientGetTitleNewsGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementClientGetTitleNewsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFTitleDataManagementGetTitleNewsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetTitleNews call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFTitleDataManagementClientGetTitleNewsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementClientGetTitleNewsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementClientGetTitleNewsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetTitleNewsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the key-value store of custom publisher settings
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API is designed to return publisher-specific values which can be read, but not written to, by
/// the client. This data is shared across all titles assigned to a particular publisher, and can be used
/// for cross-game coordination. Only titles assigned to a publisher can use this API. For more information
/// email helloplayfab@microsoft.com. Note that there may up to a minute delay in between updating title
/// data and this API call returning the newest value. See also ServerSetPublisherDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementServerGetPublisherDataGetResultSize"/>
/// and <see cref="PFTitleDataManagementServerGetPublisherDataGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementServerGetPublisherDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementGetPublisherDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetPublisherData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFTitleDataManagementServerGetPublisherDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementServerGetPublisherDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementServerGetPublisherDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetPublisherDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the current server time
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This query retrieves the current time from one of the servers in PlayFab. Please note that due to
/// clock drift between servers, there is a potential variance of up to 5 seconds.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementServerGetTimeGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFTitleDataManagementServerGetTimeAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementServerGetTimeAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFTitleDataManagementGetTimeResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFTitleDataManagementServerGetTimeGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFTitleDataManagementGetTimeResult* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the key-value store of custom title settings
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API is designed to return title specific values which can be read, but not written to, by the
/// client. For example, a developer could choose to store values which modify the user experience, such
/// as enemy spawn rates, weapon strengths, movement speeds, etc. This allows a developer to update the
/// title without the need to create, test, and ship a new build. If an override label is specified in
/// the request, the overrides are applied automatically and returned with the title data. Note that there
/// may up to a minute delay in between updating title data and this API call returning the newest value.
/// See also ServerSetTitleDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementServerGetTitleDataGetResultSize"/>
/// and <see cref="PFTitleDataManagementServerGetTitleDataGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementServerGetTitleDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementGetTitleDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetTitleData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFTitleDataManagementServerGetTitleDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementServerGetTitleDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementServerGetTitleDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetTitleDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the key-value store of custom internal title settings
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API is designed to return title specific values which are accessible only to the server. This
/// can be used to tweak settings on game servers and Cloud Scripts without needed to update and re-deploy
/// them. Note that there may up to a minute delay in between updating title data and this API call returning
/// the newest value. See also ServerSetTitleInternalDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementServerGetTitleInternalDataGetResultSize"/>
/// and <see cref="PFTitleDataManagementServerGetTitleInternalDataGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementServerGetTitleInternalDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementGetTitleDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetTitleInternalData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFTitleDataManagementServerGetTitleInternalDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementServerGetTitleInternalDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementServerGetTitleInternalDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetTitleDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the title news feed, as configured in the developer portal
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFTitleDataManagementServerGetTitleNewsGetResultSize"/>
/// and <see cref="PFTitleDataManagementServerGetTitleNewsGetResult"/> to get the result.
/// </remarks>
PF_API PFTitleDataManagementServerGetTitleNewsAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementGetTitleNewsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetTitleNews call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFTitleDataManagementServerGetTitleNewsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFTitleDataManagementServerGetTitleNewsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFTitleDataManagementServerGetTitleNewsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFTitleDataManagementGetTitleNewsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the key-value store of custom publisher settings
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API is designed to store publisher-specific values which can be read, but not written to, by
/// the client. This data is shared across all titles assigned to a particular publisher, and can be used
/// for cross-game coordination. Only titles assigned to a publisher can use this API. This operation
/// is additive. If a Key does not exist in the current dataset, it will be added with the specified Value.
/// If it already exists, the Value for that key will be overwritten with the new Value. For more information
/// email helloplayfab@microsoft.com See also ServerGetPublisherDataAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_PUBLISHER_NOT_SET or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFTitleDataManagementServerSetPublisherDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementSetPublisherDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the key-value store of custom title settings
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API is designed to store title specific values which can be read, but not written to, by the
/// client. For example, a developer could choose to store values which modify the user experience, such
/// as enemy spawn rates, weapon strengths, movement speeds, etc. This allows a developer to update the
/// title without the need to create, test, and ship a new build. This operation is additive. If a Key
/// does not exist in the current dataset, it will be added with the specified Value. If it already exists,
/// the Value for that key will be overwritten with the new Value. See also ServerGetTitleDataAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATA_LENGTH_EXCEEDED, E_PF_TOO_MANY_KEYS or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFTitleDataManagementServerSetTitleDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementSetTitleDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the key-value store of custom title settings
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This API is designed to store title specific values which are accessible only to the server. This
/// can be used to tweak settings on game servers and Cloud Scripts without needed to update and re-deploy
/// them. This operation is additive. If a Key does not exist in the current dataset, it will be added
/// with the specified Value. If it already exists, the Value for that key will be overwritten with the
/// new Value. See also ServerGetTitleInternalDataAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATA_LENGTH_EXCEEDED, E_PF_TOO_MANY_KEYS or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFTitleDataManagementServerSetTitleInternalDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFTitleDataManagementSetTitleDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}