// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>
#include <playfab/core/PFTypes.h>
#include <playfab/services/PFTypes.h>

extern "C"
{

#pragma push_macro("IN")
#undef IN

/// <summary>
/// PFInventoryAlternateId data model.
/// </summary>
typedef struct PFInventoryAlternateId
{
    /// <summary>
    /// (Optional) Type of the alternate ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* type;

    /// <summary>
    /// (Optional) Value of the alternate ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* value;

} PFInventoryAlternateId;

/// <summary>
/// PFInventoryInventoryItemReference data model.
/// </summary>
typedef struct PFInventoryInventoryItemReference
{
    /// <summary>
    /// (Optional) The inventory item alternate id the request applies to.
    /// </summary>
    _Maybenull_ PFInventoryAlternateId const* alternateId;

    /// <summary>
    /// (Optional) The inventory item id the request applies to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* id;

    /// <summary>
    /// (Optional) The inventory stack id the request should redeem to. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* stackId;

} PFInventoryInventoryItemReference;

/// <summary>
/// PFInventoryInitialValues data model.
/// </summary>
typedef struct PFInventoryInitialValues
{
    /// <summary>
    /// (Optional) Game specific properties for display purposes. The Display Properties field has a
    /// 1000 byte limit.
    /// </summary>
    PFJsonObject displayProperties;

} PFInventoryInitialValues;

/// <summary>
/// PFInventoryAddInventoryItemsRequest data model. Given an entity type, entity identifier and container
/// details, will add the specified inventory items.
/// </summary>
typedef struct PFInventoryAddInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The amount to add for the current item.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// The number of inventory collections is unlimited.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The duration to add to the current item expiration date.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The Idempotency ID for this request. Idempotency IDs can be used to prevent operation
    /// replay in the medium term but will be garbage collected eventually.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The inventory item the request applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

    /// <summary>
    /// (Optional) The values to apply to a stack newly created by this request.
    /// </summary>
    _Maybenull_ PFInventoryInitialValues const* newStackValues;

} PFInventoryAddInventoryItemsRequest;

/// <summary>
/// PFInventoryAddInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryAddInventoryItemsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The idempotency id used in the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryAddInventoryItemsResponse;

/// <summary>
/// PFInventoryDeleteInventoryCollectionRequest data model. Delete an Inventory Collection by the specified
/// Id for an Entity.
/// </summary>
typedef struct PFInventoryDeleteInventoryCollectionRequest
{
    /// <summary>
    /// (Optional) The inventory collection id the request applies to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity the request is about. Set to the caller by default.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

} PFInventoryDeleteInventoryCollectionRequest;

/// <summary>
/// PFInventoryDeleteInventoryItemsRequest data model. Given an entity type, entity identifier and container
/// details, will delete the entity's inventory items.
/// </summary>
typedef struct PFInventoryDeleteInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// The number of inventory collections is unlimited.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The Idempotency ID for this request. Idempotency IDs can be used to prevent operation
    /// replay in the medium term but will be garbage collected eventually.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The inventory item the request applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

} PFInventoryDeleteInventoryItemsRequest;

/// <summary>
/// PFInventoryDeleteInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryDeleteInventoryItemsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The idempotency id used in the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryDeleteInventoryItemsResponse;

/// <summary>
/// PFInventoryAddInventoryItemsOperation data model.
/// </summary>
typedef struct PFInventoryAddInventoryItemsOperation
{
    /// <summary>
    /// (Optional) The amount to add to the current item amount.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) The duration to add to the current item expiration date.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The inventory item the operation applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

    /// <summary>
    /// (Optional) The values to apply to a stack newly created by this operation.
    /// </summary>
    _Maybenull_ PFInventoryInitialValues const* newStackValues;

} PFInventoryAddInventoryItemsOperation;

/// <summary>
/// PFInventoryDeleteInventoryItemsOperation data model.
/// </summary>
typedef struct PFInventoryDeleteInventoryItemsOperation
{
    /// <summary>
    /// (Optional) The inventory item the operation applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

} PFInventoryDeleteInventoryItemsOperation;

/// <summary>
/// PFInventoryPurchasePriceAmount data model.
/// </summary>
typedef struct PFInventoryPurchasePriceAmount
{
    /// <summary>
    /// The amount of the inventory item to use in the purchase .
    /// </summary>
    int32_t amount;

    /// <summary>
    /// (Optional) The inventory item id to use in the purchase .
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemId;

    /// <summary>
    /// (Optional) The inventory stack id the to use in the purchase. Set to "default" by default.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* stackId;

} PFInventoryPurchasePriceAmount;

/// <summary>
/// PFInventoryPurchaseInventoryItemsOperation data model.
/// </summary>
typedef struct PFInventoryPurchaseInventoryItemsOperation
{
    /// <summary>
    /// (Optional) The amount to purchase.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// Indicates whether stacks reduced to an amount of 0 during the operation should be deleted from
    /// the inventory. (Default = false).
    /// </summary>
    bool deleteEmptyStacks;

    /// <summary>
    /// (Optional) The duration to purchase.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The inventory item the operation applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

    /// <summary>
    /// (Optional) The values to apply to a stack newly created by this operation.
    /// </summary>
    _Maybenull_ PFInventoryInitialValues const* newStackValues;

    /// <summary>
    /// (Optional) The per-item price the item is expected to be purchased at. This must match a value
    /// configured in the Catalog or specified Store.
    /// </summary>
    _Maybenull_ _Field_size_(priceAmountsCount) PFInventoryPurchasePriceAmount const* const* priceAmounts;

    /// <summary>
    /// Count of priceAmounts
    /// </summary>
    uint32_t priceAmountsCount;

    /// <summary>
    /// (Optional) The id of the Store to purchase the item from.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* storeId;

} PFInventoryPurchaseInventoryItemsOperation;

/// <summary>
/// PFInventorySubtractInventoryItemsOperation data model.
/// </summary>
typedef struct PFInventorySubtractInventoryItemsOperation
{
    /// <summary>
    /// (Optional) The amount to subtract from the current item amount.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// Indicates whether stacks reduced to an amount of 0 during the request should be deleted from
    /// the inventory. (Default = false).
    /// </summary>
    bool deleteEmptyStacks;

    /// <summary>
    /// (Optional) The duration to subtract from the current item expiration date.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The inventory item the operation applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

} PFInventorySubtractInventoryItemsOperation;

/// <summary>
/// PFInventoryTransferInventoryItemsOperation data model.
/// </summary>
typedef struct PFInventoryTransferInventoryItemsOperation
{
    /// <summary>
    /// (Optional) The amount to transfer.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// Indicates whether stacks reduced to an amount of 0 during the operation should be deleted from
    /// the inventory. (Default = false).
    /// </summary>
    bool deleteEmptyStacks;

    /// <summary>
    /// (Optional) The inventory item the operation is transferring from.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* givingItem;

    /// <summary>
    /// (Optional) The values to apply to a stack newly created by this operation.
    /// </summary>
    _Maybenull_ PFInventoryInitialValues const* newStackValues;

    /// <summary>
    /// (Optional) The inventory item the operation is transferring to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* receivingItem;

} PFInventoryTransferInventoryItemsOperation;

/// <summary>
/// PFInventoryInventoryItem data model.
/// </summary>
typedef struct PFInventoryInventoryItem
{
    /// <summary>
    /// (Optional) The amount of the item.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) Game specific properties for display purposes. This is an arbitrary JSON blob. The
    /// Display Properties field has a 1000 byte limit.
    /// </summary>
    PFJsonObject displayProperties;

    /// <summary>
    /// (Optional) Only used for subscriptions. The date of when the item will expire in UTC.
    /// </summary>
    _Maybenull_ time_t const* expirationDate;

    /// <summary>
    /// (Optional) The id of the item. This should correspond to the item id in the catalog.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* id;

    /// <summary>
    /// (Optional) The stack id of the item.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* stackId;

    /// <summary>
    /// (Optional) The type of the item. This should correspond to the item type in the catalog.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* type;

} PFInventoryInventoryItem;

/// <summary>
/// PFInventoryUpdateInventoryItemsOperation data model.
/// </summary>
typedef struct PFInventoryUpdateInventoryItemsOperation
{
    /// <summary>
    /// (Optional) The inventory item to update with the specified values.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItem const* item;

} PFInventoryUpdateInventoryItemsOperation;

/// <summary>
/// PFInventoryInventoryOperation data model.
/// </summary>
typedef struct PFInventoryInventoryOperation
{
    /// <summary>
    /// (Optional) The add operation.
    /// </summary>
    _Maybenull_ PFInventoryAddInventoryItemsOperation const* add;

    /// <summary>
    /// (Optional) The delete operation.
    /// </summary>
    _Maybenull_ PFInventoryDeleteInventoryItemsOperation const* deleteOp;

    /// <summary>
    /// (Optional) The purchase operation.
    /// </summary>
    _Maybenull_ PFInventoryPurchaseInventoryItemsOperation const* purchase;

    /// <summary>
    /// (Optional) The subtract operation.
    /// </summary>
    _Maybenull_ PFInventorySubtractInventoryItemsOperation const* subtract;

    /// <summary>
    /// (Optional) The transfer operation.
    /// </summary>
    _Maybenull_ PFInventoryTransferInventoryItemsOperation const* transfer;

    /// <summary>
    /// (Optional) The update operation.
    /// </summary>
    _Maybenull_ PFInventoryUpdateInventoryItemsOperation const* update;

} PFInventoryInventoryOperation;

/// <summary>
/// PFInventoryExecuteInventoryOperationsRequest data model. Execute a list of Inventory Operations for
/// an Entity.
/// </summary>
typedef struct PFInventoryExecuteInventoryOperationsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// The number of inventory collections is unlimited.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The Idempotency ID for this request. Idempotency IDs can be used to prevent operation
    /// replay in the medium term but will be garbage collected eventually.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The operations to run transactionally. The operations will be executed in-order sequentially
    /// and will succeed or fail as a batch. Up to 50 operations can be added.
    /// </summary>
    _Maybenull_ _Field_size_(operationsCount) PFInventoryInventoryOperation const* const* operations;

    /// <summary>
    /// Count of operations
    /// </summary>
    uint32_t operationsCount;

} PFInventoryExecuteInventoryOperationsRequest;

/// <summary>
/// PFInventoryExecuteInventoryOperationsResponse data model.
/// </summary>
typedef struct PFInventoryExecuteInventoryOperationsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The idempotency id used in the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The ids of the transactions that occurred as a result of the request.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryExecuteInventoryOperationsResponse;

/// <summary>
/// PFInventoryExecuteTransferOperationsRequest data model. Transfer the specified list of inventory
/// items of an entity's container Id to another entity's container Id.
/// </summary>
typedef struct PFInventoryExecuteTransferOperationsRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The inventory collection id the request is transferring from. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingCollectionId;

    /// <summary>
    /// (Optional) The entity the request is transferring from. Set to the caller by default.
    /// </summary>
    _Maybenull_ PFEntityKey const* givingEntity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingETag;

    /// <summary>
    /// (Optional) The idempotency id for the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The transfer operations to run transactionally. The operations will be executed in-order
    /// sequentially and will succeed or fail as a batch. Up to 50 operations can be added.
    /// </summary>
    _Maybenull_ _Field_size_(operationsCount) PFInventoryTransferInventoryItemsOperation const* const* operations;

    /// <summary>
    /// Count of operations
    /// </summary>
    uint32_t operationsCount;

    /// <summary>
    /// (Optional) The inventory collection id the request is transferring to. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* receivingCollectionId;

    /// <summary>
    /// (Optional) The entity the request is transferring to. Set to the caller by default.
    /// </summary>
    _Maybenull_ PFEntityKey const* receivingEntity;

} PFInventoryExecuteTransferOperationsRequest;

/// <summary>
/// PFInventoryExecuteTransferOperationsResponse data model.
/// </summary>
typedef struct PFInventoryExecuteTransferOperationsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources (before transferring
    /// from). This value will be empty if the operation has not completed yet. More information about
    /// using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingETag;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request's giving action.
    /// </summary>
    _Maybenull_ _Field_size_(givingTransactionIdsCount) const char* const* givingTransactionIds;

    /// <summary>
    /// Count of givingTransactionIds
    /// </summary>
    uint32_t givingTransactionIdsCount;

    /// <summary>
    /// (Optional) The Idempotency ID for this request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The transfer operation status. Possible values are 'InProgress' or 'Completed'. If
    /// the operation has completed, the response code will be 200. Otherwise, it will be 202.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationStatus;

    /// <summary>
    /// (Optional) The token that can be used to get the status of the transfer operation. This will
    /// only have a value if OperationStatus is 'InProgress'.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationToken;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources (before transferring
    /// to). This value will be empty if the operation has not completed yet.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* receivingETag;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request's receiving action.
    /// </summary>
    _Maybenull_ _Field_size_(receivingTransactionIdsCount) const char* const* receivingTransactionIds;

    /// <summary>
    /// Count of receivingTransactionIds
    /// </summary>
    uint32_t receivingTransactionIdsCount;

} PFInventoryExecuteTransferOperationsResponse;

/// <summary>
/// PFInventoryGetInventoryCollectionIdsRequest data model. Get a list of Inventory Collection Ids for
/// the specified Entity.
/// </summary>
typedef struct PFInventoryGetInventoryCollectionIdsRequest
{
    /// <summary>
    /// (Optional) An opaque token used to retrieve the next page of collection ids, if any are available.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* continuationToken;

    /// <summary>
    /// Number of items to retrieve. This value is optional. The default value is 10.
    /// </summary>
    int32_t count;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity the request is about. Set to the caller by default.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

} PFInventoryGetInventoryCollectionIdsRequest;

/// <summary>
/// PFInventoryGetInventoryCollectionIdsResponse data model.
/// </summary>
typedef struct PFInventoryGetInventoryCollectionIdsResponse
{
    /// <summary>
    /// (Optional) The requested inventory collection ids.
    /// </summary>
    _Maybenull_ _Field_size_(collectionIdsCount) const char* const* collectionIds;

    /// <summary>
    /// Count of collectionIds
    /// </summary>
    uint32_t collectionIdsCount;

    /// <summary>
    /// (Optional) An opaque token used to retrieve the next page of collection ids, if any are available.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* continuationToken;

} PFInventoryGetInventoryCollectionIdsResponse;

/// <summary>
/// PFInventoryGetInventoryItemsRequest data model. Given an entity type, entity identifier and container
/// details, will get the entity's inventory items. .
/// </summary>
typedef struct PFInventoryGetInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) An opaque token used to retrieve the next page of items in the inventory, if any are
    /// available. Should be null on initial request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* continuationToken;

    /// <summary>
    /// Number of items to retrieve. This value is optional. Maximum page size is 50. The default value
    /// is 10.
    /// </summary>
    int32_t count;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) OData Filter to refine the items returned. InventoryItem properties 'type', 'id',
    /// and 'stackId' can be used in the filter. For example: "type eq 'currency'".
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* filter;

} PFInventoryGetInventoryItemsRequest;

/// <summary>
/// PFInventoryGetInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryGetInventoryItemsResponse
{
    /// <summary>
    /// (Optional) An opaque token used to retrieve the next page of items, if any are available.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* continuationToken;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The requested inventory items.
    /// </summary>
    _Maybenull_ _Field_size_(itemsCount) PFInventoryInventoryItem const* const* items;

    /// <summary>
    /// Count of items
    /// </summary>
    uint32_t itemsCount;

} PFInventoryGetInventoryItemsResponse;

/// <summary>
/// PFInventoryGetInventoryOperationStatusRequest data model. Get the status of an Inventory Operation
/// using an OperationToken.
/// </summary>
typedef struct PFInventoryGetInventoryOperationStatusRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

} PFInventoryGetInventoryOperationStatusRequest;

/// <summary>
/// PFInventoryGetInventoryOperationStatusResponse data model.
/// </summary>
typedef struct PFInventoryGetInventoryOperationStatusResponse
{
    /// <summary>
    /// (Optional) The inventory operation status.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationStatus;

} PFInventoryGetInventoryOperationStatusResponse;

/// <summary>
/// PFInventoryGetMicrosoftStoreAccessTokensRequest data model. Gets the access tokens for Microsoft
/// Store authentication.
/// </summary>
typedef struct PFInventoryGetMicrosoftStoreAccessTokensRequest
{
    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

} PFInventoryGetMicrosoftStoreAccessTokensRequest;

/// <summary>
/// PFInventoryGetMicrosoftStoreAccessTokensResponse data model.
/// </summary>
typedef struct PFInventoryGetMicrosoftStoreAccessTokensResponse
{
    /// <summary>
    /// (Optional) The collections access token for calling https://onestore.microsoft.com/b2b/keys/create/collections
    /// to obtain a CollectionsIdKey for the user.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionsAccessToken;

    /// <summary>
    /// The date the collections access token expires.
    /// </summary>
    time_t collectionsAccessTokenExpirationDate;

} PFInventoryGetMicrosoftStoreAccessTokensResponse;

/// <summary>
/// PFInventoryGetTransactionHistoryRequest data model. Get transaction history for specified entity
/// and collection.
/// </summary>
typedef struct PFInventoryGetTransactionHistoryRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) An opaque token used to retrieve the next page of items, if any are available. Should
    /// be null on initial request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* continuationToken;

    /// <summary>
    /// Number of items to retrieve. This value is optional. The default value is 10.
    /// </summary>
    int32_t count;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) An OData filter used to refine the TransactionHistory. Transaction properties 'timestamp',
    /// 'transactionid', 'apiname' and 'operationtype' can be used in the filter. Properties 'transactionid',
    /// 'apiname', and 'operationtype' cannot be used together in a single request. The 'timestamp' property
    /// can be combined with 'apiname' or 'operationtype' in a single request. For example: "timestamp
    /// ge 2023-06-20T23:30Z" or "transactionid eq '10'" or "(timestamp ge 2023-06-20T23:30Z) and (apiname
    /// eq 'AddInventoryItems')". By default, a 6 month timespan from the current date is used.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* filter;

    /// <summary>
    /// (Optional) An OData orderby to order TransactionHistory results. The only supported values are
    /// 'timestamp asc' or 'timestamp desc'. Default orderby is 'timestamp asc'.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* orderBy;

} PFInventoryGetTransactionHistoryRequest;

/// <summary>
/// PFInventoryTransactionOperation data model.
/// </summary>
typedef struct PFInventoryTransactionOperation
{
    /// <summary>
    /// (Optional) The amount of items in this transaction.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) The duration modified in this transaction.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The friendly id of the items in this transaction.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemFriendlyId;

    /// <summary>
    /// (Optional) The item id of the items in this transaction.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemId;

    /// <summary>
    /// (Optional) The type of item that the operation occurred on.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemType;

    /// <summary>
    /// (Optional) The stack id of the items in this transaction.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* stackId;

    /// <summary>
    /// (Optional) The type of the operation that occurred.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* type;

} PFInventoryTransactionOperation;

/// <summary>
/// PFInventoryTransactionPurchaseDetails data model.
/// </summary>
typedef struct PFInventoryTransactionPurchaseDetails
{
    /// <summary>
    /// (Optional) The friendly id of the Store the item was purchased from or null.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* storeFriendlyId;

    /// <summary>
    /// (Optional) The id of the Store the item was purchased from or null.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* storeId;

} PFInventoryTransactionPurchaseDetails;

/// <summary>
/// PFInventoryTransactionRedeemDetails data model.
/// </summary>
typedef struct PFInventoryTransactionRedeemDetails
{
    /// <summary>
    /// (Optional) The marketplace that the offer is being redeemed from.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* marketplace;

    /// <summary>
    /// (Optional) The transaction Id returned from the marketplace.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* marketplaceTransactionId;

    /// <summary>
    /// (Optional) The offer Id of the item being redeemed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* offerId;

} PFInventoryTransactionRedeemDetails;

/// <summary>
/// PFInventoryTransactionTransferDetails data model.
/// </summary>
typedef struct PFInventoryTransactionTransferDetails
{
    /// <summary>
    /// (Optional) The collection id the items were transferred from or null if it was the current collection.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingCollectionId;

    /// <summary>
    /// (Optional) The entity the items were transferred from or null if it was the current entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* givingEntity;

    /// <summary>
    /// (Optional) The collection id the items were transferred to or null if it was the current collection.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* receivingCollectionId;

    /// <summary>
    /// (Optional) The entity the items were transferred to or null if it was the current entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* receivingEntity;

    /// <summary>
    /// (Optional) The id of the transfer that occurred.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* transferId;

} PFInventoryTransactionTransferDetails;

/// <summary>
/// PFInventoryTransaction data model.
/// </summary>
typedef struct PFInventoryTransaction
{
    /// <summary>
    /// (Optional) The API call that caused this transaction.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* apiName;

    /// <summary>
    /// (Optional) The type of item that the the operation occurred on.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemType;

    /// <summary>
    /// (Optional) The operations that occurred.
    /// </summary>
    _Maybenull_ _Field_size_(operationsCount) PFInventoryTransactionOperation const* const* operations;

    /// <summary>
    /// Count of operations
    /// </summary>
    uint32_t operationsCount;

    /// <summary>
    /// (Optional) The type of operation that was run.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationType;

    /// <summary>
    /// (Optional) Additional details about the transaction. Null if it was not a purchase operation.
    /// </summary>
    _Maybenull_ PFInventoryTransactionPurchaseDetails const* purchaseDetails;

    /// <summary>
    /// (Optional) Additional details about the transaction. Null if it was not a redeem operation.
    /// </summary>
    _Maybenull_ PFInventoryTransactionRedeemDetails const* redeemDetails;

    /// <summary>
    /// The time this transaction occurred in UTC.
    /// </summary>
    time_t timestamp;

    /// <summary>
    /// (Optional) The id of the transaction. This should be treated like an opaque token.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* transactionId;

    /// <summary>
    /// (Optional) Additional details about the transaction. Null if it was not a transfer operation.
    /// </summary>
    _Maybenull_ PFInventoryTransactionTransferDetails const* transferDetails;

} PFInventoryTransaction;

/// <summary>
/// PFInventoryGetTransactionHistoryResponse data model.
/// </summary>
typedef struct PFInventoryGetTransactionHistoryResponse
{
    /// <summary>
    /// (Optional) An opaque token used to retrieve the next page of items, if any are available. Should
    /// be null on initial request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* continuationToken;

    /// <summary>
    /// (Optional) The requested inventory transactions.
    /// </summary>
    _Maybenull_ _Field_size_(transactionsCount) PFInventoryTransaction const* const* transactions;

    /// <summary>
    /// Count of transactions
    /// </summary>
    uint32_t transactionsCount;

} PFInventoryGetTransactionHistoryResponse;

/// <summary>
/// PFInventoryPurchaseInventoryItemsRequest data model. Purchase a single item or bundle, paying the
/// associated price.
/// </summary>
typedef struct PFInventoryPurchaseInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The amount to purchase.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// The number of inventory collections is unlimited.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// Indicates whether stacks reduced to an amount of 0 during the request should be deleted from
    /// the inventory. (Default=false).
    /// </summary>
    bool deleteEmptyStacks;

    /// <summary>
    /// (Optional) The duration to purchase.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The Idempotency ID for this request. Idempotency IDs can be used to prevent operation
    /// replay in the medium term but will be garbage collected eventually.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The inventory item the request applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

    /// <summary>
    /// (Optional) The values to apply to a stack newly created by this request.
    /// </summary>
    _Maybenull_ PFInventoryInitialValues const* newStackValues;

    /// <summary>
    /// (Optional) The per-item price the item is expected to be purchased at. This must match a value
    /// configured in the Catalog or specified Store. .
    /// </summary>
    _Maybenull_ _Field_size_(priceAmountsCount) PFInventoryPurchasePriceAmount const* const* priceAmounts;

    /// <summary>
    /// Count of priceAmounts
    /// </summary>
    uint32_t priceAmountsCount;

    /// <summary>
    /// (Optional) The id of the Store to purchase the item from.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* storeId;

} PFInventoryPurchaseInventoryItemsRequest;

/// <summary>
/// PFInventoryPurchaseInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryPurchaseInventoryItemsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The idempotency id used in the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryPurchaseInventoryItemsResponse;

/// <summary>
/// PFInventoryRedeemAppleAppStoreInventoryItemsRequest data model. Redeem items from the Apple App Store.
/// </summary>
typedef struct PFInventoryRedeemAppleAppStoreInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The receipt provided by the Apple marketplace upon successful purchase.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* receipt;

} PFInventoryRedeemAppleAppStoreInventoryItemsRequest;

/// <summary>
/// PFInventoryRedemptionFailure data model.
/// </summary>
typedef struct PFInventoryRedemptionFailure
{
    /// <summary>
    /// (Optional) The marketplace failure code.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* failureCode;

    /// <summary>
    /// (Optional) The marketplace error details explaining why the offer failed to redeem.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* failureDetails;

    /// <summary>
    /// (Optional) The Marketplace Alternate ID being redeemed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* marketplaceAlternateId;

    /// <summary>
    /// (Optional) The transaction id in the external marketplace.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* marketplaceTransactionId;

} PFInventoryRedemptionFailure;

/// <summary>
/// PFInventoryRedemptionSuccess data model.
/// </summary>
typedef struct PFInventoryRedemptionSuccess
{
    /// <summary>
    /// (Optional) The Marketplace Alternate ID being redeemed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* marketplaceAlternateId;

    /// <summary>
    /// (Optional) The transaction id in the external marketplace.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* marketplaceTransactionId;

    /// <summary>
    /// The timestamp for when the redeem was completed.
    /// </summary>
    time_t successTimestamp;

} PFInventoryRedemptionSuccess;

/// <summary>
/// PFInventoryRedeemAppleAppStoreInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryRedeemAppleAppStoreInventoryItemsResponse
{
    /// <summary>
    /// (Optional) The list of failed redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(failedCount) PFInventoryRedemptionFailure const* const* failed;

    /// <summary>
    /// Count of failed
    /// </summary>
    uint32_t failedCount;

    /// <summary>
    /// (Optional) The list of successful redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(succeededCount) PFInventoryRedemptionSuccess const* const* succeeded;

    /// <summary>
    /// Count of succeeded
    /// </summary>
    uint32_t succeededCount;

    /// <summary>
    /// (Optional) The Transaction IDs associated with the inventory modifications.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryRedeemAppleAppStoreInventoryItemsResponse;

/// <summary>
/// PFInventoryGooglePlayProductPurchase data model.
/// </summary>
typedef struct PFInventoryGooglePlayProductPurchase
{
    /// <summary>
    /// (Optional) The Product ID (SKU) of the InApp product purchased from the Google Play store.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* productId;

    /// <summary>
    /// (Optional) The token provided to the player's device when the product was purchased.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* token;

} PFInventoryGooglePlayProductPurchase;

/// <summary>
/// PFInventoryRedeemGooglePlayInventoryItemsRequest data model. Redeem items from the Google Play Store.
/// </summary>
typedef struct PFInventoryRedeemGooglePlayInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The list of purchases to redeem.
    /// </summary>
    _Maybenull_ _Field_size_(purchasesCount) PFInventoryGooglePlayProductPurchase const* const* purchases;

    /// <summary>
    /// Count of purchases
    /// </summary>
    uint32_t purchasesCount;

} PFInventoryRedeemGooglePlayInventoryItemsRequest;

/// <summary>
/// PFInventoryRedeemGooglePlayInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryRedeemGooglePlayInventoryItemsResponse
{
    /// <summary>
    /// (Optional) The list of failed redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(failedCount) PFInventoryRedemptionFailure const* const* failed;

    /// <summary>
    /// Count of failed
    /// </summary>
    uint32_t failedCount;

    /// <summary>
    /// (Optional) The list of successful redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(succeededCount) PFInventoryRedemptionSuccess const* const* succeeded;

    /// <summary>
    /// Count of succeeded
    /// </summary>
    uint32_t succeededCount;

    /// <summary>
    /// (Optional) The Transaction IDs associated with the inventory modifications.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryRedeemGooglePlayInventoryItemsResponse;

/// <summary>
/// PFInventoryRedeemMicrosoftStoreInventoryItemsRequest data model. Redeem items from the Microsoft
/// Store.
/// </summary>
typedef struct PFInventoryRedeemMicrosoftStoreInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The OneStore Collections Id Key used for AAD authentication.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionsIdKey;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

#if HC_PLATFORM == HC_PLATFORM_GDK
    /// <summary>
    /// XUserHandle used for delegated Microsoft Store authentication.
    /// </summary>
    XUserHandle user;
#else
    /// <summary>
    /// (Optional) Xbox Token used for delegated business partner authentication. Token provided by the
    /// Xbox Live SDK method GetTokenAndSignatureAsync("POST", "https://playfabapi.com/", "").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* xboxToken;
#endif

} PFInventoryRedeemMicrosoftStoreInventoryItemsRequest;

/// <summary>
/// PFInventoryRedeemMicrosoftStoreInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryRedeemMicrosoftStoreInventoryItemsResponse
{
    /// <summary>
    /// (Optional) The list of failed redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(failedCount) PFInventoryRedemptionFailure const* const* failed;

    /// <summary>
    /// Count of failed
    /// </summary>
    uint32_t failedCount;

    /// <summary>
    /// (Optional) The list of successful redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(succeededCount) PFInventoryRedemptionSuccess const* const* succeeded;

    /// <summary>
    /// Count of succeeded
    /// </summary>
    uint32_t succeededCount;

    /// <summary>
    /// (Optional) The Transaction IDs associated with the inventory modifications.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryRedeemMicrosoftStoreInventoryItemsResponse;

/// <summary>
/// PFInventoryRedeemNintendoEShopInventoryItemsRequest data model. Redeem items from the Nintendo EShop.
/// </summary>
typedef struct PFInventoryRedeemNintendoEShopInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The Nintendo provided token authorizing redemption.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* nintendoServiceAccountIdToken;

} PFInventoryRedeemNintendoEShopInventoryItemsRequest;

/// <summary>
/// PFInventoryRedeemNintendoEShopInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryRedeemNintendoEShopInventoryItemsResponse
{
    /// <summary>
    /// (Optional) The list of failed redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(failedCount) PFInventoryRedemptionFailure const* const* failed;

    /// <summary>
    /// Count of failed
    /// </summary>
    uint32_t failedCount;

    /// <summary>
    /// (Optional) The list of successful redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(succeededCount) PFInventoryRedemptionSuccess const* const* succeeded;

    /// <summary>
    /// Count of succeeded
    /// </summary>
    uint32_t succeededCount;

    /// <summary>
    /// (Optional) The Transaction IDs associated with the inventory modifications.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryRedeemNintendoEShopInventoryItemsResponse;

/// <summary>
/// PFInventoryRedeemPlayStationStoreInventoryItemsRequest data model. Redeem items from the PlayStation
/// Store.
/// </summary>
typedef struct PFInventoryRedeemPlayStationStoreInventoryItemsRequest
{
    /// <summary>
    /// (Optional) Auth code returned by PlayStation :tm: Network OAuth system.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* authorizationCode;

    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Redirect URI supplied to PlayStation :tm: Network when requesting an auth code.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* redirectUri;

    /// <summary>
    /// (Optional) Optional Service Label to pass into the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* serviceLabel;

} PFInventoryRedeemPlayStationStoreInventoryItemsRequest;

/// <summary>
/// PFInventoryRedeemPlayStationStoreInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryRedeemPlayStationStoreInventoryItemsResponse
{
    /// <summary>
    /// (Optional) The list of failed redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(failedCount) PFInventoryRedemptionFailure const* const* failed;

    /// <summary>
    /// Count of failed
    /// </summary>
    uint32_t failedCount;

    /// <summary>
    /// (Optional) The list of successful redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(succeededCount) PFInventoryRedemptionSuccess const* const* succeeded;

    /// <summary>
    /// Count of succeeded
    /// </summary>
    uint32_t succeededCount;

    /// <summary>
    /// (Optional) The Transaction IDs associated with the inventory modifications.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryRedeemPlayStationStoreInventoryItemsResponse;

/// <summary>
/// PFInventoryRedeemSteamInventoryItemsRequest data model. Redeem inventory items from Steam.
/// </summary>
typedef struct PFInventoryRedeemSteamInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

} PFInventoryRedeemSteamInventoryItemsRequest;

/// <summary>
/// PFInventoryRedeemSteamInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryRedeemSteamInventoryItemsResponse
{
    /// <summary>
    /// (Optional) The list of failed redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(failedCount) PFInventoryRedemptionFailure const* const* failed;

    /// <summary>
    /// Count of failed
    /// </summary>
    uint32_t failedCount;

    /// <summary>
    /// (Optional) The list of successful redemptions from the external marketplace.
    /// </summary>
    _Maybenull_ _Field_size_(succeededCount) PFInventoryRedemptionSuccess const* const* succeeded;

    /// <summary>
    /// Count of succeeded
    /// </summary>
    uint32_t succeededCount;

    /// <summary>
    /// (Optional) The Transaction IDs associated with the inventory modifications.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryRedeemSteamInventoryItemsResponse;

/// <summary>
/// PFInventorySubtractInventoryItemsRequest data model. Given an entity type, entity identifier and
/// container details, will subtract the specified inventory items. .
/// </summary>
typedef struct PFInventorySubtractInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The amount to subtract for the current item.
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// The number of inventory collections is unlimited.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// Indicates whether stacks reduced to an amount of 0 during the request should be deleted from
    /// the inventory. (Default=false).
    /// </summary>
    bool deleteEmptyStacks;

    /// <summary>
    /// (Optional) The duration to subtract from the current item expiration date.
    /// </summary>
    _Maybenull_ double const* durationInSeconds;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The Idempotency ID for this request. Idempotency IDs can be used to prevent operation
    /// replay in the medium term but will be garbage collected eventually.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The inventory item the request applies to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* item;

} PFInventorySubtractInventoryItemsRequest;

/// <summary>
/// PFInventorySubtractInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventorySubtractInventoryItemsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The idempotency id used in the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventorySubtractInventoryItemsResponse;

/// <summary>
/// PFInventoryTransferInventoryItemsRequest data model. Transfer the specified inventory items of an
/// entity's container Id to another entity's container Id.
/// </summary>
typedef struct PFInventoryTransferInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The amount to transfer .
    /// </summary>
    _Maybenull_ int32_t const* amount;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// Indicates whether stacks reduced to an amount of 0 during the request should be deleted from
    /// the inventory. (Default = false).
    /// </summary>
    bool deleteEmptyStacks;

    /// <summary>
    /// (Optional) The inventory collection id the request is transferring from. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingCollectionId;

    /// <summary>
    /// (Optional) The entity the request is transferring from. Set to the caller by default.
    /// </summary>
    _Maybenull_ PFEntityKey const* givingEntity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources (before transferring
    /// from). More information about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingETag;

    /// <summary>
    /// (Optional) The inventory item the request is transferring from.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* givingItem;

    /// <summary>
    /// (Optional) The idempotency id for the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The values to apply to a stack newly created by this request.
    /// </summary>
    _Maybenull_ PFInventoryInitialValues const* newStackValues;

    /// <summary>
    /// (Optional) The inventory collection id the request is transferring to. (Default="default").
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* receivingCollectionId;

    /// <summary>
    /// (Optional) The entity the request is transferring to. Set to the caller by default.
    /// </summary>
    _Maybenull_ PFEntityKey const* receivingEntity;

    /// <summary>
    /// (Optional) The inventory item the request is transferring to.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItemReference const* receivingItem;

} PFInventoryTransferInventoryItemsRequest;

/// <summary>
/// PFInventoryTransferInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryTransferInventoryItemsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources (after transferring
    /// from). More information about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* givingETag;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request's giving action.
    /// </summary>
    _Maybenull_ _Field_size_(givingTransactionIdsCount) const char* const* givingTransactionIds;

    /// <summary>
    /// Count of givingTransactionIds
    /// </summary>
    uint32_t givingTransactionIdsCount;

    /// <summary>
    /// (Optional) The idempotency id for the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The transfer operation status. Possible values are 'InProgress' or 'Completed'. If
    /// the operation has completed, the response code will be 200. Otherwise, it will be 202.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationStatus;

    /// <summary>
    /// (Optional) The token that can be used to get the status of the transfer operation. This will
    /// only have a value if OperationStatus is 'InProgress'.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationToken;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request's receiving action.
    /// </summary>
    _Maybenull_ _Field_size_(receivingTransactionIdsCount) const char* const* receivingTransactionIds;

    /// <summary>
    /// Count of receivingTransactionIds
    /// </summary>
    uint32_t receivingTransactionIdsCount;

} PFInventoryTransferInventoryItemsResponse;

/// <summary>
/// PFInventoryUpdateInventoryItemsRequest data model. Given an entity type, entity identifier and container
/// details, will update the entity's inventory items.
/// </summary>
typedef struct PFInventoryUpdateInventoryItemsRequest
{
    /// <summary>
    /// (Optional) The id of the entity's collection to perform this action on. (Default="default").
    /// The number of inventory collections is unlimited.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* collectionId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

    /// <summary>
    /// (Optional) The entity to perform this action on.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The Idempotency ID for this request. Idempotency IDs can be used to prevent operation
    /// replay in the medium term but will be garbage collected eventually.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The inventory item to update with the specified values.
    /// </summary>
    _Maybenull_ PFInventoryInventoryItem const* item;

} PFInventoryUpdateInventoryItemsRequest;

/// <summary>
/// PFInventoryUpdateInventoryItemsResponse data model.
/// </summary>
typedef struct PFInventoryUpdateInventoryItemsResponse
{
    /// <summary>
    /// (Optional) ETags are used for concurrency checking when updating resources. More information
    /// about using ETags can be found here: https://learn.microsoft.com/en-us/gaming/playfab/features/economy-v2/catalog/etags.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* eTag;

    /// <summary>
    /// (Optional) The idempotency id used in the request.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* idempotencyId;

    /// <summary>
    /// (Optional) The ids of transactions that occurred as a result of the request.
    /// </summary>
    _Maybenull_ _Field_size_(transactionIdsCount) const char* const* transactionIds;

    /// <summary>
    /// Count of transactionIds
    /// </summary>
    uint32_t transactionIdsCount;

} PFInventoryUpdateInventoryItemsResponse;

#pragma pop_macro("IN")

}
