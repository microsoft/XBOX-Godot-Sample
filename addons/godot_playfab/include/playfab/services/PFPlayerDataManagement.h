// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFPlayerDataManagementTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

#if 0
/// <summary>
/// Deletes title-specific custom properties for a player
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Deletes custom properties for the specified player. The list of provided property names must be non-empty.
/// See also ClientGetPlayerCustomPropertyAsync, ClientListPlayerCustomPropertiesAsync, ClientUpdatePlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientDeletePlayerCustomPropertiesGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientDeletePlayerCustomPropertiesGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientDeletePlayerCustomPropertiesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientDeletePlayerCustomProperties call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientDeletePlayerCustomPropertiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientDeletePlayerCustomPropertiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFPlayerDataManagementClientDeletePlayerCustomPropertiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Retrieves a title-specific custom property value for a player.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ClientDeletePlayerCustomPropertiesAsync, ClientListPlayerCustomPropertiesAsync, ClientUpdatePlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientGetPlayerCustomPropertyGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientGetPlayerCustomPropertyGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientGetPlayerCustomPropertyAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementClientGetPlayerCustomPropertyRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetPlayerCustomProperty call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientGetPlayerCustomPropertyGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientGetPlayerCustomPropertyAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFPlayerDataManagementClientGetPlayerCustomPropertyGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientGetPlayerCustomPropertyResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Retrieves the title-specific custom data for the user which is readable and writable by the client
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Data is stored as JSON key-value pairs. Every time the data is updated via any source, the version
/// counter is incremented. If the Version parameter is provided, then this call will only return data
/// if the current version on the system is greater than the value provided. If the Keys parameter is
/// provided, the data object returned will only contain the data specific to the indicated Keys. Otherwise,
/// the full set of custom user data will be returned. See also ClientGetUserReadOnlyDataAsync, ClientUpdateUserDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientGetUserDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientGetUserDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientGetUserDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetUserData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientGetUserDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientGetUserDataAsync call.
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
PF_API PFPlayerDataManagementClientGetUserDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves the publisher-specific custom data for the user which is readable and writable by the client
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ClientGetUserPublisherReadOnlyDataAsync, ClientUpdateUserPublisherDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientGetUserPublisherDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientGetUserPublisherDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientGetUserPublisherDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetUserPublisherData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientGetUserPublisherDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientGetUserPublisherDataAsync call.
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
PF_API PFPlayerDataManagementClientGetUserPublisherDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves the publisher-specific custom data for the user which can only be read by the client
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ClientGetUserPublisherDataAsync, ClientUpdateUserPublisherDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientGetUserPublisherReadOnlyDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientGetUserPublisherReadOnlyDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientGetUserPublisherReadOnlyDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetUserPublisherReadOnlyData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientGetUserPublisherReadOnlyDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientGetUserPublisherReadOnlyDataAsync call.
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
PF_API PFPlayerDataManagementClientGetUserPublisherReadOnlyDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves the title-specific custom data for the user which can only be read by the client
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Data is stored as JSON key-value pairs. Every time the data is updated via any source, the version
/// counter is incremented. If the Version parameter is provided, then this call will only return data
/// if the current version on the system is greater than the value provided. If the Keys parameter is
/// provided, the data object returned will only contain the data specific to the indicated Keys. Otherwise,
/// the full set of custom user data will be returned. See also ClientGetUserDataAsync, ClientUpdateUserDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientGetUserReadOnlyDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientGetUserReadOnlyDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientGetUserReadOnlyDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientGetUserReadOnlyData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientGetUserReadOnlyDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientGetUserReadOnlyDataAsync call.
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
PF_API PFPlayerDataManagementClientGetUserReadOnlyDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Retrieves title-specific custom property values for a player.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ClientDeletePlayerCustomPropertiesAsync, ClientGetPlayerCustomPropertyAsync, ClientUpdatePlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientListPlayerCustomPropertiesGetResultSize"/>
/// and <see cref="PFPlayerDataManagementClientListPlayerCustomPropertiesGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientListPlayerCustomPropertiesAsync(
    _In_ PFEntityHandle entityHandle,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientListPlayerCustomProperties call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientListPlayerCustomPropertiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientListPlayerCustomPropertiesAsync call.
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
PF_API PFPlayerDataManagementClientListPlayerCustomPropertiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementClientListPlayerCustomPropertiesResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Updates the title-specific custom property values for a player
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Performs an additive update of the custom properties for the specified player. In updating the player's
/// custom properties, properties which already exist will have their values overwritten. No other properties
/// will be changed apart from those specified in the call. See also ClientDeletePlayerCustomPropertiesAsync,
/// ClientGetPlayerCustomPropertyAsync, ClientListPlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientUpdatePlayerCustomPropertiesGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientUpdatePlayerCustomPropertiesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientUpdatePlayerCustomPropertiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_DUPLICATE_PROPERTY_NAME,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_COUNT_TOO_HIGH, E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_NAME_IS_INVALID,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_NAME_TOO_LONG, E_PF_PLAYER_CUSTOM_PROPERTIES_STRING_PROPERTY_VALUE_TOO_LONG,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_VALUE_IS_INVALID_TYPE, E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientUpdatePlayerCustomPropertiesGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult* result
) noexcept;
#endif

/// <summary>
/// Creates and updates the title-specific custom data for the user which is readable and writable by
/// the client
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This function performs an additive update of the arbitrary strings containing the custom data for
/// the user. In updating the custom data object, keys which already exist in the object will have their
/// values overwritten, while keys with null values will be removed. New keys will be added, with the
/// given values. No other key-value pairs will be changed apart from those specified in the call. See
/// also ClientGetUserDataAsync, ClientGetUserReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientUpdateUserDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientUpdateUserDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementClientUpdateUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientUpdateUserDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientUpdateUserDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;

/// <summary>
/// Creates and updates the publisher-specific custom data for the user which is readable and writable
/// by the client
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// This function performs an additive update of the arbitrary strings containing the custom data for
/// the user. In updating the custom data object, keys which already exist in the object will have their
/// values overwritten, while keys with null values will be removed. New keys will be added, with the
/// given values. No other key-value pairs will be changed apart from those specified in the call. See
/// also ClientGetUserPublisherDataAsync, ClientGetUserPublisherReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementClientUpdateUserPublisherDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementClientUpdateUserPublisherDataAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFPlayerDataManagementClientUpdateUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementClientUpdateUserPublisherDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementClientUpdateUserPublisherDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;

#if 0
/// <summary>
/// Deletes title-specific custom properties for a player
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Deletes custom properties for the specified player. The list of provided property names must be non-empty.
/// See also ServerGetPlayerCustomPropertyAsync, ServerListPlayerCustomPropertiesAsync, ServerUpdatePlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerDeletePlayerCustomPropertiesGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerDeletePlayerCustomPropertiesGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerDeletePlayerCustomPropertiesAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerDeletePlayerCustomPropertiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerDeletePlayerCustomProperties call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerDeletePlayerCustomPropertiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerDeletePlayerCustomPropertiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFPlayerDataManagementServerDeletePlayerCustomPropertiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerDeletePlayerCustomPropertiesResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Retrieves a title-specific custom property value for a player.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ServerDeletePlayerCustomPropertiesAsync, ServerListPlayerCustomPropertiesAsync, ServerUpdatePlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetPlayerCustomPropertyGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetPlayerCustomPropertyGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetPlayerCustomPropertyAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerGetPlayerCustomPropertyRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetPlayerCustomProperty call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetPlayerCustomPropertyGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetPlayerCustomPropertyAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFPlayerDataManagementServerGetPlayerCustomPropertyGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetPlayerCustomPropertyResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the title-specific custom data for the user which is readable and writable by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ServerGetUserInternalDataAsync, ServerGetUserReadOnlyDataAsync, ServerUpdateUserDataAsync,
/// ServerUpdateUserInternalDataAsync, ServerUpdateUserReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetUserDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetUserDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetUserDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetUserData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetUserDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetUserDataAsync call.
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
PF_API PFPlayerDataManagementServerGetUserDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the title-specific custom data for the user which cannot be accessed by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ServerGetUserDataAsync, ServerGetUserReadOnlyDataAsync, ServerUpdateUserDataAsync,
/// ServerUpdateUserInternalDataAsync, ServerUpdateUserReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetUserInternalDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetUserInternalDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetUserInternalDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetUserInternalData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetUserInternalDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetUserInternalDataAsync call.
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
PF_API PFPlayerDataManagementServerGetUserInternalDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the publisher-specific custom data for the user which is readable and writable by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ServerGetUserPublisherInternalDataAsync, ServerGetUserPublisherReadOnlyDataAsync,
/// ServerUpdateUserPublisherDataAsync, ServerUpdateUserPublisherInternalDataAsync, ServerUpdateUserPublisherReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetUserPublisherDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetUserPublisherDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetUserPublisherDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetUserPublisherData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetUserPublisherDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetUserPublisherDataAsync call.
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
PF_API PFPlayerDataManagementServerGetUserPublisherDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the publisher-specific custom data for the user which cannot be accessed by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ServerGetUserPublisherDataAsync, ServerGetUserPublisherReadOnlyDataAsync,
/// ServerUpdateUserPublisherDataAsync, ServerUpdateUserPublisherInternalDataAsync, ServerUpdateUserPublisherReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetUserPublisherInternalDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetUserPublisherInternalDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetUserPublisherInternalDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetUserPublisherInternalData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetUserPublisherInternalDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetUserPublisherInternalDataAsync call.
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
PF_API PFPlayerDataManagementServerGetUserPublisherInternalDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the publisher-specific custom data for the user which can only be read by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ServerGetUserPublisherDataAsync, ServerGetUserPublisherInternalDataAsync,
/// ServerUpdateUserPublisherDataAsync, ServerUpdateUserPublisherInternalDataAsync, ServerUpdateUserPublisherReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetUserPublisherReadOnlyDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetUserPublisherReadOnlyDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetUserPublisherReadOnlyDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetUserPublisherReadOnlyData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetUserPublisherReadOnlyDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetUserPublisherReadOnlyDataAsync call.
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
PF_API PFPlayerDataManagementServerGetUserPublisherReadOnlyDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Retrieves the title-specific custom data for the user which can only be read by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Data is stored as JSON key-value pairs. If the Keys parameter is provided, the data object returned
/// will only contain the data specific to the indicated Keys. Otherwise, the full set of custom user
/// data will be returned. See also ServerGetUserDataAsync, ServerGetUserInternalDataAsync, ServerUpdateUserDataAsync,
/// ServerUpdateUserInternalDataAsync, ServerUpdateUserReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerGetUserReadOnlyDataGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerGetUserReadOnlyDataGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerGetUserReadOnlyDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementGetUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerGetUserReadOnlyData call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerGetUserReadOnlyDataGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerGetUserReadOnlyDataAsync call.
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
PF_API PFPlayerDataManagementServerGetUserReadOnlyDataGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerGetUserDataResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Retrieves title-specific custom property values for a player.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also ServerDeletePlayerCustomPropertiesAsync, ServerGetPlayerCustomPropertyAsync, ServerUpdatePlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerListPlayerCustomPropertiesGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerListPlayerCustomPropertiesGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerListPlayerCustomPropertiesAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementListPlayerCustomPropertiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerListPlayerCustomProperties call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerListPlayerCustomPropertiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerListPlayerCustomPropertiesAsync call.
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
PF_API PFPlayerDataManagementServerListPlayerCustomPropertiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerListPlayerCustomPropertiesResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Updates the title-specific custom property values for a player
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Performs an additive update of the custom properties for the specified player. In updating the player's
/// custom properties, properties which already exist will have their values overwritten. No other properties
/// will be changed apart from those specified in the call. See also ServerDeletePlayerCustomPropertiesAsync,
/// ServerGetPlayerCustomPropertyAsync, ServerListPlayerCustomPropertiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdatePlayerCustomPropertiesGetResultSize"/>
/// and <see cref="PFPlayerDataManagementServerUpdatePlayerCustomPropertiesGetResult"/> to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdatePlayerCustomPropertiesAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerUpdatePlayerCustomPropertiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerUpdatePlayerCustomProperties call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_DUPLICATE_PROPERTY_NAME,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_COUNT_TOO_HIGH, E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_NAME_IS_INVALID,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_NAME_TOO_LONG, E_PF_PLAYER_CUSTOM_PROPERTIES_STRING_PROPERTY_VALUE_TOO_LONG,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_VALUE_IS_INVALID_TYPE, E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdatePlayerCustomPropertiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdatePlayerCustomPropertiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PLAYER_CUSTOM_PROPERTIES_DUPLICATE_PROPERTY_NAME,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_COUNT_TOO_HIGH, E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_NAME_IS_INVALID,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_PROPERTY_NAME_TOO_LONG, E_PF_PLAYER_CUSTOM_PROPERTIES_STRING_PROPERTY_VALUE_TOO_LONG,
/// E_PF_PLAYER_CUSTOM_PROPERTIES_VALUE_IS_INVALID_TYPE, E_PF_PLAYER_CUSTOM_PROPERTIES_VERSION_MISMATCH
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdatePlayerCustomPropertiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFPlayerDataManagementServerUpdatePlayerCustomPropertiesResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the title-specific custom data for the user which is readable and writable by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This function performs an additive update of the arbitrary JSON object containing the custom data
/// for the user. In updating the custom data object, keys which already exist in the object will have
/// their values overwritten, while keys with null values will be removed. No other key-value pairs will
/// be changed apart from those specified in the call. See also ServerGetUserDataAsync, ServerGetUserInternalDataAsync,
/// ServerGetUserReadOnlyDataAsync, ServerUpdateUserInternalDataAsync, ServerUpdateUserReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdateUserDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdateUserDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerUpdateUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdateUserDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdateUserDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the title-specific custom data for the user which cannot be accessed by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This function performs an additive update of the arbitrary JSON object containing the custom data
/// for the user. In updating the custom data object, keys which already exist in the object will have
/// their values overwritten, keys with null values will be removed. No other key-value pairs will be
/// changed apart from those specified in the call. See also ServerGetUserDataAsync, ServerGetUserInternalDataAsync,
/// ServerGetUserReadOnlyDataAsync, ServerUpdateUserDataAsync, ServerUpdateUserReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdateUserInternalDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdateUserInternalDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementUpdateUserInternalDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdateUserInternalDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdateUserInternalDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the publisher-specific custom data for the user which is readable and writable by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This function performs an additive update of the arbitrary JSON object containing the custom data
/// for the user. In updating the custom data object, keys which already exist in the object will have
/// their values overwritten, while keys with null values will be removed. No other key-value pairs will
/// be changed apart from those specified in the call. See also ServerGetUserPublisherDataAsync, ServerGetUserPublisherInternalDataAsync,
/// ServerGetUserPublisherReadOnlyDataAsync, ServerUpdateUserPublisherInternalDataAsync, ServerUpdateUserPublisherReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdateUserPublisherDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdateUserPublisherDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerUpdateUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdateUserPublisherDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdateUserPublisherDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the publisher-specific custom data for the user which cannot be accessed by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This function performs an additive update of the arbitrary JSON object containing the custom data
/// for the user. In updating the custom data object, keys which already exist in the object will have
/// their values overwritten, keys with null values will be removed. No other key-value pairs will be
/// changed apart from those specified in the call. See also ServerGetUserPublisherDataAsync, ServerGetUserPublisherInternalDataAsync,
/// ServerGetUserPublisherReadOnlyDataAsync, ServerUpdateUserPublisherDataAsync, ServerUpdateUserPublisherReadOnlyDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdateUserPublisherInternalDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdateUserPublisherInternalDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementUpdateUserInternalDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdateUserPublisherInternalDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdateUserPublisherInternalDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the publisher-specific custom data for the user which can only be read by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This function performs an additive update of the arbitrary JSON object containing the custom data
/// for the user. In updating the custom data object, keys which already exist in the object will have
/// their values overwritten, keys with null values will be removed. No other key-value pairs will be
/// changed apart from those specified in the call. See also ServerGetUserPublisherDataAsync, ServerGetUserPublisherInternalDataAsync,
/// ServerGetUserPublisherReadOnlyDataAsync, ServerUpdateUserPublisherDataAsync, ServerUpdateUserPublisherInternalDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdateUserPublisherReadOnlyDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdateUserPublisherReadOnlyDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerUpdateUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdateUserPublisherReadOnlyDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_PUBLISHER_NOT_SET
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdateUserPublisherReadOnlyDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Updates the title-specific custom data for the user which can only be read by the client
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// This function performs an additive update of the arbitrary JSON object containing the custom data
/// for the user. In updating the custom data object, keys which already exist in the object will have
/// their values overwritten, keys with null values will be removed. No other key-value pairs will be
/// changed apart from those specified in the call. See also ServerGetUserDataAsync, ServerGetUserInternalDataAsync,
/// ServerGetUserReadOnlyDataAsync, ServerUpdateUserDataAsync, ServerUpdateUserInternalDataAsync.
///
/// When the asynchronous task is complete, call <see cref="PFPlayerDataManagementServerUpdateUserReadOnlyDataGetResult"/>
/// to get the result.
/// </remarks>
PF_API PFPlayerDataManagementServerUpdateUserReadOnlyDataAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFPlayerDataManagementServerUpdateUserDataRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFPlayerDataManagementServerUpdateUserReadOnlyDataAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFPlayerDataManagementUpdateUserDataResult object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFPlayerDataManagementServerUpdateUserReadOnlyDataGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFPlayerDataManagementUpdateUserDataResult* result
) noexcept;
#endif


}