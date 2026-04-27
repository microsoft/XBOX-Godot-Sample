// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFCloudScriptTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Executes a CloudScript function, with the 'currentPlayerId' set to the PlayFab ID of the authenticated
/// player. The PlayFab ID is the entity ID of the player's master_player_account entity.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCloudScriptClientExecuteCloudScriptGetResultSize"/>
/// and <see cref="PFCloudScriptClientExecuteCloudScriptGetResult"/> to get the result.
/// </remarks>
PF_API PFCloudScriptClientExecuteCloudScriptAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCloudScriptExecuteCloudScriptRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ClientExecuteCloudScript call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_API_REQUEST_COUNT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_API_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_FUNCTION_ARGUMENT_SIZE_EXCEEDED, E_PF_CLOUD_SCRIPT_HTTP_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_NOT_FOUND, E_PF_JAVASCRIPT_EXCEPTION, E_PF_TITLE_DELETED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCloudScriptClientExecuteCloudScriptGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCloudScriptClientExecuteCloudScriptAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_API_REQUEST_COUNT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_API_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_FUNCTION_ARGUMENT_SIZE_EXCEEDED, E_PF_CLOUD_SCRIPT_HTTP_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_NOT_FOUND, E_PF_JAVASCRIPT_EXCEPTION, E_PF_TITLE_DELETED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCloudScriptClientExecuteCloudScriptGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCloudScriptExecuteCloudScriptResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Executes a CloudScript function, with the 'currentPlayerId' set to the PlayFab ID of the authenticated
/// player. The PlayFab ID is the entity ID of the player's master_player_account entity.
/// </summary>
/// <param name="titleEntityHandle">PFEntityHandle for a title Entity obtained using PFAuthenticationGetEntityWithSecretKeyAsync.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// When the asynchronous task is complete, call <see cref="PFCloudScriptServerExecuteCloudScriptGetResultSize"/>
/// and <see cref="PFCloudScriptServerExecuteCloudScriptGetResult"/> to get the result.
/// </remarks>
PF_API PFCloudScriptServerExecuteCloudScriptAsync(
    _In_ PFEntityHandle titleEntityHandle,
    _In_ const PFCloudScriptExecuteCloudScriptServerRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ServerExecuteCloudScript call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_API_REQUEST_COUNT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_API_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_FUNCTION_ARGUMENT_SIZE_EXCEEDED, E_PF_CLOUD_SCRIPT_HTTP_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_NOT_FOUND, E_PF_JAVASCRIPT_EXCEPTION, E_PF_TITLE_DELETED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCloudScriptServerExecuteCloudScriptGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCloudScriptServerExecuteCloudScriptAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_API_REQUEST_COUNT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_API_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_FUNCTION_ARGUMENT_SIZE_EXCEEDED, E_PF_CLOUD_SCRIPT_HTTP_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_NOT_FOUND, E_PF_JAVASCRIPT_EXCEPTION, E_PF_TITLE_DELETED or any of the global PlayFab
/// Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCloudScriptServerExecuteCloudScriptGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCloudScriptExecuteCloudScriptResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Cloud Script is one of PlayFab's most versatile features. It allows client code to request execution
/// of any kind of custom server-side functionality you can implement, and it can be used in conjunction
/// with virtually anything.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Executes CloudScript with the entity profile that is defined in the request.
///
/// When the asynchronous task is complete, call <see cref="PFCloudScriptExecuteEntityCloudScriptGetResultSize"/>
/// and <see cref="PFCloudScriptExecuteEntityCloudScriptGetResult"/> to get the result.
/// </remarks>
PF_API PFCloudScriptExecuteEntityCloudScriptAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCloudScriptExecuteEntityCloudScriptRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ExecuteEntityCloudScript call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_API_REQUEST_COUNT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_API_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_FUNCTION_ARGUMENT_SIZE_EXCEEDED, E_PF_CLOUD_SCRIPT_HTTP_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_NOT_FOUND, E_PF_INVALID_ENTITY_TYPE, E_PF_JAVASCRIPT_EXCEPTION, E_PF_TITLE_DELETED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFCloudScriptExecuteEntityCloudScriptGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCloudScriptExecuteEntityCloudScriptAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_API_REQUEST_COUNT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_API_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_FUNCTION_ARGUMENT_SIZE_EXCEEDED, E_PF_CLOUD_SCRIPT_HTTP_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_NOT_FOUND, E_PF_INVALID_ENTITY_TYPE, E_PF_JAVASCRIPT_EXCEPTION, E_PF_TITLE_DELETED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCloudScriptExecuteEntityCloudScriptGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCloudScriptExecuteCloudScriptResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Cloud Script is one of PlayFab's most versatile features. It allows client code to request execution
/// of any kind of custom server-side functionality you can implement, and it can be used in conjunction
/// with virtually anything.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Executes an Azure Function with the profile of the entity that is defined in the request. See also
/// CloudScriptRegisterHttpFunctionAsync, CloudScriptRegisterQueuedFunctionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFCloudScriptExecuteFunctionGetResultSize"/>
/// and <see cref="PFCloudScriptExecuteFunctionGetResult"/> to get the result.
/// </remarks>
PF_API PFCloudScriptExecuteFunctionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCloudScriptExecuteFunctionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ExecuteFunction call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_ARGUMENT_SIZE_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_EVENT_HUB_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_EXECUTION_TIME_LIMIT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_HTTP_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_QUEUE_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_RETURN_SIZE_EXCEEDED, E_PF_INVALID_ENTITY_TYPE or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCloudScriptExecuteFunctionGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCloudScriptExecuteFunctionAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_ARGUMENT_SIZE_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_EVENT_HUB_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_EXECUTION_TIME_LIMIT_EXCEEDED,
/// E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_HTTP_REQUEST_ERROR, E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_QUEUE_REQUEST_ERROR,
/// E_PF_CLOUD_SCRIPT_AZURE_FUNCTIONS_RETURN_SIZE_EXCEEDED, E_PF_INVALID_ENTITY_TYPE or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCloudScriptExecuteFunctionGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCloudScriptExecuteFunctionResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Lists all currently registered Event Hub triggered Azure Functions for a given title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// See also CloudScriptListFunctionsAsync, CloudScriptListHttpFunctionsAsync, CloudScriptListQueuedFunctionsAsync,
/// CloudScriptRegisterEventHubFunctionAsync.
///
/// When the asynchronous task is complete, call <see cref="PFCloudScriptListEventHubFunctionsGetResultSize"/>
/// and <see cref="PFCloudScriptListEventHubFunctionsGetResult"/> to get the result.
/// </remarks>
PF_API PFCloudScriptListEventHubFunctionsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCloudScriptListFunctionsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListEventHubFunctions call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCloudScriptListEventHubFunctionsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCloudScriptListEventHubFunctionsAsync call.
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
PF_API PFCloudScriptListEventHubFunctionsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCloudScriptListEventHubFunctionsResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if 0
/// <summary>
/// Registers an event hub triggered Azure Function with a title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// A title can have many functions, RegisterEventHubFunction associates a function name with an event
/// hub name and connection string. See also CloudScriptExecuteFunctionAsync, CloudScriptListEventHubFunctionsAsync,
/// CloudScriptListFunctionsAsync, CloudScriptPostFunctionResultForEntityTriggeredActionAsync, CloudScriptPostFunctionResultForFunctionExecutionAsync,
/// CloudScriptPostFunctionResultForPlayerTriggeredActionAsync, CloudScriptPostFunctionResultForScheduledTaskAsync,
/// CloudScriptRegisterHttpFunctionAsync, CloudScriptRegisterQueuedFunctionAsync, CloudScriptUnregisterFunctionAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFCloudScriptRegisterEventHubFunctionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCloudScriptRegisterEventHubFunctionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif


}