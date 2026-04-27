// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFCatalogTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Creates a new item in the working catalog using provided metadata. Note: SAS tokens provided are
/// valid for 1 hour.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// The item will not be published to the public catalog until the PublishItem API is called for the
/// item.
///
/// When the asynchronous task is complete, call <see cref="PFCatalogCreateDraftItemGetResultSize"/>
/// and <see cref="PFCatalogCreateDraftItemGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogCreateDraftItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogCreateDraftItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a CreateDraftItem call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INVALID_REQUEST, E_PF_ITEM_NOT_FOUND, E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCatalogCreateDraftItemGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogCreateDraftItemAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INVALID_REQUEST, E_PF_ITEM_NOT_FOUND, E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogCreateDraftItemGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogCreateDraftItemResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Creates one or more upload URLs which can be used by the client to upload raw file data. Content
/// URls and uploaded content will be garbage collected after 24 hours if not attached to a draft or published
/// item. Detailed pricing info around uploading content can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/pricing/meters/catalog-meters
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Upload URLs point to Azure Blobs; clients must follow the Microsoft Azure Storage Blob Service REST
/// API pattern for uploading content. The response contains upload URLs and IDs for each file. The IDs
/// and URLs returned must be added to the item metadata and committed using the CreateDraftItem or UpdateDraftItem
/// Item APIs.
///
/// When the asynchronous task is complete, call <see cref="PFCatalogCreateUploadUrlsGetResultSize"/>
/// and <see cref="PFCatalogCreateUploadUrlsGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogCreateUploadUrlsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogCreateUploadUrlsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a CreateUploadUrls call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCatalogCreateUploadUrlsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogCreateUploadUrlsAsync call.
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
PF_API PFCatalogCreateUploadUrlsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogCreateUploadUrlsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Deletes all reviews, helpfulness votes, and ratings submitted by the entity specified.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_PROFILE_DOES_NOT_EXIST or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogDeleteEntityItemReviewsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogDeleteEntityItemReviewsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Removes an item from working catalog and all published versions from the public catalog.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED, E_PF_ITEM_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogDeleteItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogDeleteItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#if 0
/// <summary>
/// Gets the configuration for the catalog. Only Title Entities can call this API. There is a limit of
/// 100 requests in 10 seconds for this API. More information about the Catalog Config can be found here:
/// https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/settings
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// When the asynchronous task is complete, call <see cref="PFCatalogGetCatalogConfigGetResultSize"/>
/// and <see cref="PFCatalogGetCatalogConfigGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetCatalogConfigAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetCatalogConfigRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetCatalogConfig call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_BILLING_INFORMATION_REQUIRED,
/// E_PF_INVALID_ENTITY_TYPE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFCatalogGetCatalogConfigGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetCatalogConfigAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_BILLING_INFORMATION_REQUIRED,
/// E_PF_INVALID_ENTITY_TYPE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogGetCatalogConfigGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetCatalogConfigResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Retrieves an item from the working catalog. This item represents the current working state of the
/// item. GetDraftItem does not work off a cache of the Catalog and should be used when trying to get
/// recent item updates. However, please note that item references data is cached and may take a few moments
/// for changes to propagate. Note: SAS tokens provided are valid for 1 hour.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetDraftItemGetResultSize"/> and
/// <see cref="PFCatalogGetDraftItemGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetDraftItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetDraftItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetDraftItem call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND, E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCatalogGetDraftItemGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetDraftItemAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND, E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogGetDraftItemGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetDraftItemResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves a paginated list of the items from the draft catalog. Up to 50 IDs can be retrieved in
/// a single request. GetDraftItems does not work off a cache of the Catalog and should be used when trying
/// to get recent item updates. Note: SAS tokens provided are valid for 1 hour.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetDraftItemsGetResultSize"/> and
/// <see cref="PFCatalogGetDraftItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetDraftItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetDraftItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetDraftItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetDraftItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetDraftItemsAsync call.
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
PF_API PFCatalogGetDraftItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetDraftItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves a paginated list of the items from the draft catalog created by the Entity. Up to 50 items
/// can be returned at once. You can use continuation tokens to paginate through results that return greater
/// than the limit. GetEntityDraftItems does not work off a cache of the Catalog and should be used when
/// trying to get recent item updates.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetEntityDraftItemsGetResultSize"/>
/// and <see cref="PFCatalogGetEntityDraftItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetEntityDraftItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetEntityDraftItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetEntityDraftItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetEntityDraftItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetEntityDraftItemsAsync call.
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
PF_API PFCatalogGetEntityDraftItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetEntityDraftItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Gets the submitted review for the specified item by the authenticated entity. Individual ratings
/// and reviews data update in near real time with delays within a few seconds.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetEntityItemReviewGetResultSize"/>
/// and <see cref="PFCatalogGetEntityItemReviewGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetEntityItemReviewAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetEntityItemReviewRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetEntityItemReview call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetEntityItemReviewGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetEntityItemReviewAsync call.
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
PF_API PFCatalogGetEntityItemReviewGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetEntityItemReviewResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves an item from the public catalog. GetItem does not work off a cache of the Catalog and should
/// be used when trying to get recent item updates. However, please note that item references data is
/// cached and may take a few moments for changes to propagate.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemGetResultSize"/> and <see
/// cref="PFCatalogGetItemGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItem call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetItemGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemAsync call.
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
PF_API PFCatalogGetItemGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Search for a given item and return a set of bundles and stores containing the item. Up to 50 items
/// can be returned at once. You can use continuation tokens to paginate through results that return greater
/// than the limit. This API is intended for tooling/automation scenarios and has a reduced RPS with Player
/// Entities limited to 30 requests in 300 seconds and Title Entities limited to 100 requests in 10 seconds.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Given an item, return a set of bundles and stores containing the item.
///
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemContainersGetResultSize"/>
/// and <see cref="PFCatalogGetItemContainersGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemContainersAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemContainersRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItemContainers call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ITEM_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFCatalogGetItemContainersGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemContainersAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ITEM_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogGetItemContainersGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemContainersResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Gets the moderation state for an item, including the concern category and string reason. More information
/// about moderation states can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/ugc/moderation
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemModerationStateGetResultSize"/>
/// and <see cref="PFCatalogGetItemModerationStateGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemModerationStateAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemModerationStateRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItemModerationState call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetItemModerationStateGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemModerationStateAsync call.
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
PF_API PFCatalogGetItemModerationStateGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemModerationStateResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;
#endif

/// <summary>
/// Gets the status of a publish of an item.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemPublishStatusGetResultSize"/>
/// and <see cref="PFCatalogGetItemPublishStatusGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemPublishStatusAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemPublishStatusRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItemPublishStatus call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetItemPublishStatusGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemPublishStatusAsync call.
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
PF_API PFCatalogGetItemPublishStatusGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemPublishStatusResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Get a paginated set of reviews associated with the specified item. Individual ratings and reviews
/// data update in near real time with delays within a few seconds.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemReviewsGetResultSize"/> and
/// <see cref="PFCatalogGetItemReviewsGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemReviewsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemReviewsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItemReviews call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetItemReviewsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemReviewsAsync call.
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
PF_API PFCatalogGetItemReviewsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemReviewsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Get a summary of all ratings and reviews associated with the specified item. Summary ratings data
/// is cached with update data coming within 15 minutes.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemReviewSummaryGetResultSize"/>
/// and <see cref="PFCatalogGetItemReviewSummaryGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemReviewSummaryAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemReviewSummaryRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItemReviewSummary call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetItemReviewSummaryGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemReviewSummaryAsync call.
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
PF_API PFCatalogGetItemReviewSummaryGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemReviewSummaryResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Retrieves items from the public catalog. Up to 50 items can be returned at once. GetItems does not
/// work off a cache of the Catalog and should be used when trying to get recent item updates. However,
/// please note that item references data is cached and may take a few moments for changes to propagate.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogGetItemsGetResultSize"/> and <see
/// cref="PFCatalogGetItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogGetItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogGetItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ITEM_NOT_FOUND,
/// E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
PF_API PFCatalogGetItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogGetItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ITEM_NOT_FOUND,
/// E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogGetItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogGetItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Initiates a publish of an item from the working catalog to the public catalog. You can use the GetItemPublishStatus
/// API to track the state of the item publish.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// The call kicks off a workflow to publish the item to the public catalog. The Publish Status API should
/// be used to monitor the publish job.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED, E_PF_ITEM_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogPublishDraftItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogPublishDraftItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Submit a report for an item, indicating in what way the item is inappropriate.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED, E_PF_ITEM_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogReportItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogReportItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Submit a report for a review
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Submit a report for an inappropriate review, allowing the submitting user to specify their concern.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogReportItemReviewAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogReportItemReviewRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Creates or updates a review for the specified item. More information around the caching surrounding
/// item ratings and reviews can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/ratings#ratings-design-and-caching
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED, E_PF_ITEM_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogReviewItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogReviewItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Executes a search against the public catalog using the provided search parameters and returns a set
/// of paginated results. SearchItems uses a cache of the catalog with item updates taking up to a few
/// minutes to propagate. You should use the GetItem API for when trying to immediately get recent item
/// updates. More information about the Search API can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/search
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogSearchItemsGetResultSize"/> and
/// <see cref="PFCatalogSearchItemsGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogSearchItemsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogSearchItemsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a SearchItems call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND, E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCatalogSearchItemsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogSearchItemsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_ITEM_NOT_FOUND, E_PF_NOT_IMPLEMENTED or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogSearchItemsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogSearchItemsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

#if 0
/// <summary>
/// Sets the moderation state for an item, including the concern category and string reason. More information
/// about moderation states can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/ugc/moderation
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED, E_PF_ITEM_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogSetItemModerationStateAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogSetItemModerationStateRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Submit a vote for a review, indicating whether the review was helpful or unhelpful.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogSubmitItemReviewVoteAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogSubmitItemReviewVoteRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

#if 0
/// <summary>
/// Submit a request to takedown one or more reviews.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Submit a request to takedown one or more reviews, removing them from public view. Authors will still
/// be able to see their reviews after being taken down.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED, E_PF_ITEM_NOT_FOUND or any of the global
/// PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFCatalogTakedownItemReviewsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogTakedownItemReviewsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

#if 0
/// <summary>
/// Updates the configuration for the catalog. Only Title Entities can call this API. There is a limit
/// of 10 requests in 10 seconds for this API. More information about the Catalog Config can be found
/// here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/settings
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_BILLING_INFORMATION_REQUIRED, E_PF_CATALOG_CONFIG_INVALID, E_PF_INVALID_ENTITY_TYPE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </remarks>
PF_API PFCatalogUpdateCatalogConfigAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogUpdateCatalogConfigRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;
#endif

/// <summary>
/// Update the metadata for an item in the working catalog. Note: SAS tokens provided are valid for 1
/// hour.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFCatalogUpdateDraftItemGetResultSize"/>
/// and <see cref="PFCatalogUpdateDraftItemGetResult"/> to get the result.
/// </remarks>
PF_API PFCatalogUpdateDraftItemAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFCatalogUpdateDraftItemRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a UpdateDraftItem call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INVALID_REQUEST, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFCatalogUpdateDraftItemGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFCatalogUpdateDraftItemAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DATABASE_THROUGHPUT_EXCEEDED,
/// E_PF_INVALID_REQUEST, E_PF_ITEM_NOT_FOUND or any of the global PlayFab Service errors. See doc page
/// "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFCatalogUpdateDraftItemGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFCatalogUpdateDraftItemResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}