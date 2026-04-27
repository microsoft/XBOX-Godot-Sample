// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFInventoryTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Add inventory items. Up to 10,000 stacks of items can be added to a single inventory collection.
/// Stack size is uncapped.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Given an entity type, entity identifier and container details, will add the specified inventory items.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryAddInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryAddInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryAddInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryAddInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a AddInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryAddInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryAddInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryAddInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryAddInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Delete an Inventory Collection. More information about Inventory Collections can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/inventory/collections
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Delete an Inventory Collection by the specified Id for an Entity.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFInventoryDeleteInventoryCollectionAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryDeleteInventoryCollectionRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Delete inventory items
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Given an entity type, entity identifier and container details, will delete the entity's inventory
/// items.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryDeleteInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryDeleteInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryDeleteInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryDeleteInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a DeleteInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFInventoryDeleteInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryDeleteInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryDeleteInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryDeleteInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Execute a list of Inventory Operations. A maximum list of 50 operations can be performed by a single
/// request. There is also a limit to 300 items that can be modified/added in a single request. For example,
/// adding a bundle with 50 items counts as 50 items modified. All operations must be done within a single
/// inventory collection. This API has a reduced RPS compared to an individual inventory operation with
/// Player Entities limited to 60 requests in 90 seconds.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Execute a list of Inventory Operations for an Entity.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryExecuteInventoryOperationsGetResultSize"/>
/// and <see cref="PFInventoryExecuteInventoryOperationsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryExecuteInventoryOperationsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryExecuteInventoryOperationsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ExecuteInventoryOperations call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_ITEM_NOT_FOUND or any of the
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryExecuteInventoryOperationsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryExecuteInventoryOperationsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_ITEM_NOT_FOUND or any of the
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryExecuteInventoryOperationsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryExecuteInventoryOperationsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Transfer a list of inventory items. A maximum list of 50 operations can be performed by a single
/// request. When the response code is 202, one or more operations did not complete within the timeframe
/// of the request. You can identify the pending operations by looking for OperationStatus = 'InProgress'.
/// You can check on the operation status at anytime within 1 day of the request by passing the TransactionToken
/// to the GetInventoryOperationStatus API.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Transfer the specified list of inventory items of an entity's container Id to another entity's container
/// Id.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryExecuteTransferOperationsGetResultSize"/>
/// and <see cref="PFInventoryExecuteTransferOperationsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryExecuteTransferOperationsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryExecuteTransferOperationsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ExecuteTransferOperations call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryExecuteTransferOperationsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryExecuteTransferOperationsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryExecuteTransferOperationsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryExecuteTransferOperationsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Get Inventory Collection Ids. Up to 50 Ids can be returned at once (or 250 with response compression
/// enabled). You can use continuation tokens to paginate through results that return greater than the
/// limit. It can take a few seconds for new collection Ids to show up.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Get a list of Inventory Collection Ids for the specified Entity.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryGetInventoryCollectionIdsGetResultSize"/>
/// and <see cref="PFInventoryGetInventoryCollectionIdsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryGetInventoryCollectionIdsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryGetInventoryCollectionIdsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetInventoryCollectionIds call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryGetInventoryCollectionIdsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryGetInventoryCollectionIdsAsync call.
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
PF_API PFInventoryGetInventoryCollectionIdsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryGetInventoryCollectionIdsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Get current inventory items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Given an entity type, entity identifier and container details, will get the entity's inventory items.
/// .
///
/// When the asynchronous task is complete, call <see cref="PFInventoryGetInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryGetInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryGetInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryGetInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_DELETED,
/// E_PF_DATABASE_THROUGHPUT_EXCEEDED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryGetInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryGetInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_DELETED,
/// E_PF_DATABASE_THROUGHPUT_EXCEEDED or any of the global PlayFab Service errors. See doc page "Handling
/// PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryGetInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryGetInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Get the status of an inventory operation using an OperationToken. You can check on the operation
/// status at anytime within 1 day of the request by passing the TransactionToken to the this API.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Get the status of an Inventory Operation using an OperationToken.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryGetInventoryOperationStatusGetResultSize"/>
/// and <see cref="PFInventoryGetInventoryOperationStatusGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryGetInventoryOperationStatusAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryGetInventoryOperationStatusRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetInventoryOperationStatus call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryGetInventoryOperationStatusGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryGetInventoryOperationStatusAsync call.
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
PF_API PFInventoryGetInventoryOperationStatusGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryGetInventoryOperationStatusResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Gets the access tokens.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, GDK, Linux, and macOS.
/// Gets the access tokens for Microsoft Store authentication.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryGetMicrosoftStoreAccessTokensGetResultSize"/>
/// and <see cref="PFInventoryGetMicrosoftStoreAccessTokensGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryGetMicrosoftStoreAccessTokensAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryGetMicrosoftStoreAccessTokensRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetMicrosoftStoreAccessTokens call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_NOT_IMPLEMENTED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFInventoryGetMicrosoftStoreAccessTokensGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryGetMicrosoftStoreAccessTokensAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_NOT_IMPLEMENTED
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryGetMicrosoftStoreAccessTokensGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryGetMicrosoftStoreAccessTokensResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Get transaction history for a player. Up to 250 Events can be returned at once. You can use continuation
/// tokens to paginate through results that return greater than the limit. Getting transaction history
/// has a lower RPS limit than getting a Player's inventory with Player Entities having a limit of 30
/// requests in 300 seconds.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Linux, and macOS.
/// Get transaction history for specified entity and collection.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryGetTransactionHistoryGetResultSize"/>
/// and <see cref="PFInventoryGetTransactionHistoryGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryGetTransactionHistoryAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryGetTransactionHistoryRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetTransactionHistory call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryGetTransactionHistoryGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryGetTransactionHistoryAsync call.
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
PF_API PFInventoryGetTransactionHistoryGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryGetTransactionHistoryResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Purchase an item or bundle. Up to 10,000 stacks of items can be added to a single inventory collection.
/// Stack size is uncapped.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Purchase a single item or bundle, paying the associated price.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryPurchaseInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryPurchaseInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryPurchaseInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryPurchaseInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a PurchaseInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_ITEM_NOT_FOUND or any of the
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryPurchaseInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryPurchaseInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_ITEM_NOT_FOUND or any of the
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryPurchaseInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryPurchaseInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if HC_PLATFORM == HC_PLATFORM_IOS
/// <summary>
/// Redeem items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on iOS.
/// Redeem items from the Apple App Store.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryRedeemAppleAppStoreInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryRedeemAppleAppStoreInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryRedeemAppleAppStoreInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryRedeemAppleAppStoreInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RedeemAppleAppStoreInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_CATALOG_ITEM_CONFIGURATION
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFInventoryRedeemAppleAppStoreInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryRedeemAppleAppStoreInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_CATALOG_ITEM_CONFIGURATION
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryRedeemAppleAppStoreInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryRedeemAppleAppStoreInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_ANDROID
/// <summary>
/// Redeem items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Android.
/// Redeem items from the Google Play Store.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryRedeemGooglePlayInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryRedeemGooglePlayInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryRedeemGooglePlayInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryRedeemGooglePlayInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RedeemGooglePlayInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_CATALOG_ITEM_CONFIGURATION
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFInventoryRedeemGooglePlayInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryRedeemGooglePlayInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_INVALID_CATALOG_ITEM_CONFIGURATION
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryRedeemGooglePlayInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryRedeemGooglePlayInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Redeem items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, GDK, Linux, and macOS.
/// Redeem items from the Microsoft Store.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryRedeemMicrosoftStoreInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryRedeemMicrosoftStoreInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryRedeemMicrosoftStoreInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryRedeemMicrosoftStoreInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RedeemMicrosoftStoreInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_XASS_EXCHANGE_FAILURE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFInventoryRedeemMicrosoftStoreInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryRedeemMicrosoftStoreInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_INVALID_XBOX_LIVE_TOKEN, E_PF_XBOX_INACCESSIBLE, E_PF_XBOX_XASS_EXCHANGE_FAILURE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryRedeemMicrosoftStoreInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryRedeemMicrosoftStoreInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_NINTENDO_SWITCH || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Redeem items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Nintendo Switch, Linux, and macOS.
/// Redeem items from the Nintendo EShop.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryRedeemNintendoEShopInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryRedeemNintendoEShopInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryRedeemNintendoEShopInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryRedeemNintendoEShopInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RedeemNintendoEShopInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_INVALID_NINTENDO_SWITCH_ACCOUNT_ID or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryRedeemNintendoEShopInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryRedeemNintendoEShopInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_INVALID_NINTENDO_SWITCH_ACCOUNT_ID or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryRedeemNintendoEShopInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryRedeemNintendoEShopInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_SONY_PLAYSTATION_4 || HC_PLATFORM == HC_PLATFORM_SONY_PLAYSTATION_5 || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Redeem items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, Sony PlayStation®, Linux, and macOS.
/// Redeem items from the PlayStation Store.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryRedeemPlayStationStoreInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryRedeemPlayStationStoreInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryRedeemPlayStationStoreInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryRedeemPlayStationStoreInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RedeemPlayStationStoreInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_INVALID_PSN_AUTH_CODE, E_PF_NOT_AUTHORIZED or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
PF_API PFInventoryRedeemPlayStationStoreInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryRedeemPlayStationStoreInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION, E_PF_INVALID_PSN_AUTH_CODE, E_PF_NOT_AUTHORIZED or any of
/// the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error
/// handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryRedeemPlayStationStoreInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryRedeemPlayStationStoreInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

#if HC_PLATFORM == HC_PLATFORM_WIN32 || HC_PLATFORM == HC_PLATFORM_GDK || HC_PLATFORM == HC_PLATFORM_LINUX || HC_PLATFORM == HC_PLATFORM_MAC
/// <summary>
/// Redeem items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on Win32, GDK, Linux, and macOS.
/// Redeem inventory items from Steam.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryRedeemSteamInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryRedeemSteamInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryRedeemSteamInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryRedeemSteamInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a RedeemSteamInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryRedeemSteamInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryRedeemSteamInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ACCOUNT_NOT_LINKED,
/// E_PF_INVALID_CATALOG_ITEM_CONFIGURATION or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryRedeemSteamInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryRedeemSteamInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Subtract inventory items.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Given an entity type, entity identifier and container details, will subtract the specified inventory
/// items. .
///
/// When the asynchronous task is complete, call <see cref="PFInventorySubtractInventoryItemsGetResultSize"/>
/// and <see cref="PFInventorySubtractInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventorySubtractInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventorySubtractInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a SubtractInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventorySubtractInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventorySubtractInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventorySubtractInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventorySubtractInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Transfer inventory items. When transferring across collections, a 202 response indicates that the
/// transfer did not complete within the timeframe of the request. You can identify the pending operations
/// by looking for OperationStatus = 'InProgress'. You can check on the operation status at anytime within
/// 1 day of the request by passing the TransactionToken to the GetInventoryOperationStatus API. More
/// information about item transfer scenarios can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/inventory/?tabs=inventory-game-manager#transfer-inventory-items
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Transfer the specified inventory items of an entity's container Id to another entity's container
/// Id.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryTransferInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryTransferInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryTransferInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryTransferInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a TransferInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFInventoryTransferInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryTransferInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INSUFFICIENT_FUNDS, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc
/// page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryTransferInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryTransferInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Update inventory items
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Given an entity type, entity identifier and container details, will update the entity's inventory
/// items.
///
/// When the asynchronous task is complete, call <see cref="PFInventoryUpdateInventoryItemsGetResultSize"/>
/// and <see cref="PFInventoryUpdateInventoryItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFInventoryUpdateInventoryItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFInventoryUpdateInventoryItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a UpdateInventoryItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFInventoryUpdateInventoryItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFInventoryUpdateInventoryItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFInventoryUpdateInventoryItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFInventoryUpdateInventoryItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}