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
/// PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest data model. Deletes custom properties
/// for the specified player. The list of provided property names must be non-empty.
/// </summary>
typedef struct PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest
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
    /// (Optional) Optional field used for concurrency control. One can ensure that the delete operation
    /// will only be performed if the player's properties have not been updated by any other clients since
    /// the last version.
    /// </summary>
    _Maybenull_ int32_t const* expectedPropertiesVersion;

    /// <summary>
    /// A list of property names denoting which properties should be deleted.
    /// </summary>
    _Field_size_(propertyNamesCount) const char* const* propertyNames;

    /// <summary>
    /// Count of propertyNames
    /// </summary>
    uint32_t propertyNamesCount;

} PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest;

/// <summary>
/// PFPlayerDataManagementDeletedPropertyDetails data model.
/// </summary>
typedef struct PFPlayerDataManagementDeletedPropertyDetails
{
    /// <summary>
    /// (Optional) The name of the property which was requested to be deleted.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// Indicates whether or not the property was deleted. If false, no property with that name existed.
    /// </summary>
    bool wasDeleted;

} PFPlayerDataManagementDeletedPropertyDetails;

/// <summary>
/// PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult data model.
/// </summary>
typedef struct PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult
{
    /// <summary>
    /// (Optional) The list of properties requested to be deleted.
    /// </summary>
    _Maybenull_ _Field_size_(deletedPropertiesCount) PFPlayerDataManagementDeletedPropertyDetails const* const* deletedProperties;

    /// <summary>
    /// Count of deletedProperties
    /// </summary>
    uint32_t deletedPropertiesCount;

    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

} PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult;

/// <summary>
/// PFPlayerDataManagementClientGetPlayerCustomPropertyRequest data model.
/// </summary>
typedef struct PFPlayerDataManagementClientGetPlayerCustomPropertyRequest
{
    /// <summary>
    /// Specific property name to search for in the player's properties.
    /// </summary>
    _Null_terminated_ const char* propertyName;

} PFPlayerDataManagementClientGetPlayerCustomPropertyRequest;

/// <summary>
/// PFPlayerDataManagementCustomPropertyDetails data model.
/// </summary>
typedef struct PFPlayerDataManagementCustomPropertyDetails
{
    /// <summary>
    /// (Optional) The custom property's name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) The custom property's value.
    /// </summary>
    PFJsonObject value;

} PFPlayerDataManagementCustomPropertyDetails;

/// <summary>
/// PFPlayerDataManagementClientGetPlayerCustomPropertyResult data model.
/// </summary>
typedef struct PFPlayerDataManagementClientGetPlayerCustomPropertyResult
{
    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

    /// <summary>
    /// (Optional) Player specific property and its corresponding value.
    /// </summary>
    _Maybenull_ PFPlayerDataManagementCustomPropertyDetails const* property;

} PFPlayerDataManagementClientGetPlayerCustomPropertyResult;

/// <summary>
/// PFPlayerDataManagementGetUserDataRequest data model. Data is stored as JSON key-value pairs. Every
/// time the data is updated via any source, the version counter is incremented. If the Version parameter
/// is provided, then this call will only return data if the current version on the system is greater
/// than the value provided. If the Keys parameter is provided, the data object returned will only contain
/// the data specific to the indicated Keys. Otherwise, the full set of custom user data will be returned.
/// </summary>
typedef struct PFPlayerDataManagementGetUserDataRequest
{
    /// <summary>
    /// (Optional) The version that currently exists according to the caller. The call will return the
    /// data for all of the keys if the version in the system is greater than this.
    /// </summary>
    _Maybenull_ uint32_t const* ifChangedFromDataVersion;

    /// <summary>
    /// (Optional) List of unique keys to load from.
    /// </summary>
    _Maybenull_ _Field_size_(keysCount) const char* const* keys;

    /// <summary>
    /// Count of keys
    /// </summary>
    uint32_t keysCount;

    /// <summary>
    /// (Optional) Unique PlayFab identifier of the user to load data for. Optional, defaults to yourself
    /// if not set. When specified to a PlayFab id of another player, then this will only return public
    /// keys for that account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

} PFPlayerDataManagementGetUserDataRequest;

/// <summary>
/// PFPlayerDataManagementClientGetUserDataResult data model.
/// </summary>
typedef struct PFPlayerDataManagementClientGetUserDataResult
{
    /// <summary>
    /// (Optional) User specific data for this title.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFUserDataRecordDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

    /// <summary>
    /// Indicates the current version of the data that has been set. This is incremented with every set
    /// call for that type of data (read-only, internal, etc). This version can be provided in Get calls
    /// to find updated data.
    /// </summary>
    uint32_t dataVersion;

} PFPlayerDataManagementClientGetUserDataResult;

/// <summary>
/// PFPlayerDataManagementClientListPlayerCustomPropertiesResult data model.
/// </summary>
typedef struct PFPlayerDataManagementClientListPlayerCustomPropertiesResult
{
    /// <summary>
    /// (Optional) Player specific properties and their corresponding values for this title.
    /// </summary>
    _Maybenull_ _Field_size_(propertiesCount) PFPlayerDataManagementCustomPropertyDetails const* const* properties;

    /// <summary>
    /// Count of properties
    /// </summary>
    uint32_t propertiesCount;

    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

} PFPlayerDataManagementClientListPlayerCustomPropertiesResult;

/// <summary>
/// PFPlayerDataManagementUpdateProperty data model.
/// </summary>
typedef struct PFPlayerDataManagementUpdateProperty
{
    /// <summary>
    /// Name of the custom property. Can contain Unicode letters and digits. They are limited in size.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// Value of the custom property. Limited to booleans, numbers, and strings.
    /// </summary>
    PFJsonObject value;

} PFPlayerDataManagementUpdateProperty;

/// <summary>
/// PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest data model. Performs an additive
/// update of the custom properties for the specified player. In updating the player's custom properties,
/// properties which already exist will have their values overwritten. No other properties will be changed
/// apart from those specified in the call.
/// </summary>
typedef struct PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest
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
    /// (Optional) Optional field used for concurrency control. One can ensure that the update operation
    /// will only be performed if the player's properties have not been updated by any other clients since
    /// last the version.
    /// </summary>
    _Maybenull_ int32_t const* expectedPropertiesVersion;

    /// <summary>
    /// Collection of properties to be set for a player.
    /// </summary>
    _Field_size_(propertiesCount) PFPlayerDataManagementUpdateProperty const* const* properties;

    /// <summary>
    /// Count of properties
    /// </summary>
    uint32_t propertiesCount;

} PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest;

/// <summary>
/// PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult data model.
/// </summary>
typedef struct PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult
{
    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

} PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult;

/// <summary>
/// PFPlayerDataManagementClientUpdateUserDataRequest data model. This function performs an additive
/// update of the arbitrary strings containing the custom data for the user. In updating the custom data
/// object, keys which already exist in the object will have their values overwritten, while keys with
/// null values will be removed. New keys will be added, with the given values. No other key-value pairs
/// will be changed apart from those specified in the call.
/// </summary>
typedef struct PFPlayerDataManagementClientUpdateUserDataRequest
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
    /// (Optional) Key-value pairs to be written to the custom data. Note that keys are trimmed of whitespace,
    /// are limited in size, and may not begin with a '!' character or be null.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFStringDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

    /// <summary>
    /// (Optional) Optional list of Data-keys to remove from UserData. Some SDKs cannot insert null-values
    /// into Data due to language constraints. Use this to delete the keys directly.
    /// </summary>
    _Maybenull_ _Field_size_(keysToRemoveCount) const char* const* keysToRemove;

    /// <summary>
    /// Count of keysToRemove
    /// </summary>
    uint32_t keysToRemoveCount;

    /// <summary>
    /// (Optional) Permission to be applied to all user data keys written in this request. Defaults to
    /// "private" if not set. This is used for requests by one player for information about another player;
    /// those requests will only return Public keys.
    /// </summary>
    _Maybenull_ PFUserDataPermission const* permission;

} PFPlayerDataManagementClientUpdateUserDataRequest;

/// <summary>
/// PFPlayerDataManagementUpdateUserDataResult data model.
/// </summary>
typedef struct PFPlayerDataManagementUpdateUserDataResult
{
    /// <summary>
    /// Indicates the current version of the data that has been set. This is incremented with every set
    /// call for that type of data (read-only, internal, etc). This version can be provided in Get calls
    /// to find updated data.
    /// </summary>
    uint32_t dataVersion;

} PFPlayerDataManagementUpdateUserDataResult;

/// <summary>
/// PFPlayerDataManagementServerDeletePlayerCustomPropertiesRequest data model. Deletes custom properties
/// for the specified player. The list of provided property names must be non-empty.
/// </summary>
typedef struct PFPlayerDataManagementServerDeletePlayerCustomPropertiesRequest
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
    /// (Optional) Optional field used for concurrency control. One can ensure that the delete operation
    /// will only be performed if the player's properties have not been updated by any other clients since
    /// the last version.
    /// </summary>
    _Maybenull_ int32_t const* expectedPropertiesVersion;

    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

    /// <summary>
    /// A list of property names denoting which properties should be deleted.
    /// </summary>
    _Field_size_(propertyNamesCount) const char* const* propertyNames;

    /// <summary>
    /// Count of propertyNames
    /// </summary>
    uint32_t propertyNamesCount;

} PFPlayerDataManagementServerDeletePlayerCustomPropertiesRequest;

/// <summary>
/// PFPlayerDataManagementServerDeletePlayerCustomPropertiesResult data model.
/// </summary>
typedef struct PFPlayerDataManagementServerDeletePlayerCustomPropertiesResult
{
    /// <summary>
    /// (Optional) The list of properties requested to be deleted.
    /// </summary>
    _Maybenull_ _Field_size_(deletedPropertiesCount) PFPlayerDataManagementDeletedPropertyDetails const* const* deletedProperties;

    /// <summary>
    /// Count of deletedProperties
    /// </summary>
    uint32_t deletedPropertiesCount;

    /// <summary>
    /// (Optional) PlayFab unique identifier of the user whose properties were deleted.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

} PFPlayerDataManagementServerDeletePlayerCustomPropertiesResult;

/// <summary>
/// PFPlayerDataManagementServerGetPlayerCustomPropertyRequest data model.
/// </summary>
typedef struct PFPlayerDataManagementServerGetPlayerCustomPropertyRequest
{
    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

    /// <summary>
    /// Specific property name to search for in the player's properties.
    /// </summary>
    _Null_terminated_ const char* propertyName;

} PFPlayerDataManagementServerGetPlayerCustomPropertyRequest;

/// <summary>
/// PFPlayerDataManagementServerGetPlayerCustomPropertyResult data model.
/// </summary>
typedef struct PFPlayerDataManagementServerGetPlayerCustomPropertyResult
{
    /// <summary>
    /// (Optional) PlayFab unique identifier of the user whose properties are being returned.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

    /// <summary>
    /// (Optional) Player specific property and its corresponding value.
    /// </summary>
    _Maybenull_ PFPlayerDataManagementCustomPropertyDetails const* property;

} PFPlayerDataManagementServerGetPlayerCustomPropertyResult;

/// <summary>
/// PFPlayerDataManagementServerGetUserDataResult data model.
/// </summary>
typedef struct PFPlayerDataManagementServerGetUserDataResult
{
    /// <summary>
    /// (Optional) User specific data for this title.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFUserDataRecordDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

    /// <summary>
    /// Indicates the current version of the data that has been set. This is incremented with every set
    /// call for that type of data (read-only, internal, etc). This version can be provided in Get calls
    /// to find updated data.
    /// </summary>
    uint32_t dataVersion;

    /// <summary>
    /// (Optional) PlayFab unique identifier of the user whose custom data is being returned.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

} PFPlayerDataManagementServerGetUserDataResult;

/// <summary>
/// PFPlayerDataManagementListPlayerCustomPropertiesRequest data model.
/// </summary>
typedef struct PFPlayerDataManagementListPlayerCustomPropertiesRequest
{
    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

} PFPlayerDataManagementListPlayerCustomPropertiesRequest;

/// <summary>
/// PFPlayerDataManagementServerListPlayerCustomPropertiesResult data model.
/// </summary>
typedef struct PFPlayerDataManagementServerListPlayerCustomPropertiesResult
{
    /// <summary>
    /// (Optional) PlayFab unique identifier of the user whose properties are being returned.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// (Optional) Player specific properties and their corresponding values for this title.
    /// </summary>
    _Maybenull_ _Field_size_(propertiesCount) PFPlayerDataManagementCustomPropertyDetails const* const* properties;

    /// <summary>
    /// Count of properties
    /// </summary>
    uint32_t propertiesCount;

    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

} PFPlayerDataManagementServerListPlayerCustomPropertiesResult;

/// <summary>
/// PFPlayerDataManagementServerUpdatePlayerCustomPropertiesRequest data model. Performs an additive
/// update of the custom properties for the specified player. In updating the player's custom properties,
/// properties which already exist will have their values overwritten. No other properties will be changed
/// apart from those specified in the call.
/// </summary>
typedef struct PFPlayerDataManagementServerUpdatePlayerCustomPropertiesRequest
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
    /// (Optional) Optional field used for concurrency control. One can ensure that the update operation
    /// will only be performed if the player's properties have not been updated by any other clients since
    /// last the version.
    /// </summary>
    _Maybenull_ int32_t const* expectedPropertiesVersion;

    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

    /// <summary>
    /// Collection of properties to be set for a player.
    /// </summary>
    _Field_size_(propertiesCount) PFPlayerDataManagementUpdateProperty const* const* properties;

    /// <summary>
    /// Count of properties
    /// </summary>
    uint32_t propertiesCount;

} PFPlayerDataManagementServerUpdatePlayerCustomPropertiesRequest;

/// <summary>
/// PFPlayerDataManagementServerUpdatePlayerCustomPropertiesResult data model.
/// </summary>
typedef struct PFPlayerDataManagementServerUpdatePlayerCustomPropertiesResult
{
    /// <summary>
    /// (Optional) PlayFab unique identifier of the user whose properties were updated.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// Indicates the current version of a player's properties that have been set. This is incremented
    /// after updates and deletes. This version can be provided in update and delete calls for concurrency
    /// control.
    /// </summary>
    int32_t propertiesVersion;

} PFPlayerDataManagementServerUpdatePlayerCustomPropertiesResult;

/// <summary>
/// PFPlayerDataManagementServerUpdateUserDataRequest data model. This function performs an additive
/// update of the arbitrary JSON object containing the custom data for the user. In updating the custom
/// data object, keys which already exist in the object will have their values overwritten, while keys
/// with null values will be removed. No other key-value pairs will be changed apart from those specified
/// in the call.
/// </summary>
typedef struct PFPlayerDataManagementServerUpdateUserDataRequest
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
    /// (Optional) Key-value pairs to be written to the custom data. Note that keys are trimmed of whitespace,
    /// are limited in size, and may not begin with a '!' character or be null.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFStringDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

    /// <summary>
    /// (Optional) Optional list of Data-keys to remove from UserData. Some SDKs cannot insert null-values
    /// into Data due to language constraints. Use this to delete the keys directly.
    /// </summary>
    _Maybenull_ _Field_size_(keysToRemoveCount) const char* const* keysToRemove;

    /// <summary>
    /// Count of keysToRemove
    /// </summary>
    uint32_t keysToRemoveCount;

    /// <summary>
    /// (Optional) Permission to be applied to all user data keys written in this request. Defaults to
    /// "private" if not set.
    /// </summary>
    _Maybenull_ PFUserDataPermission const* permission;

    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

} PFPlayerDataManagementServerUpdateUserDataRequest;

/// <summary>
/// PFPlayerDataManagementUpdateUserInternalDataRequest data model. This function performs an additive
/// update of the arbitrary JSON object containing the custom data for the user. In updating the custom
/// data object, keys which already exist in the object will have their values overwritten, keys with
/// null values will be removed. No other key-value pairs will be changed apart from those specified in
/// the call.
/// </summary>
typedef struct PFPlayerDataManagementUpdateUserInternalDataRequest
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
    /// (Optional) Key-value pairs to be written to the custom data. Note that keys are trimmed of whitespace,
    /// are limited in size, and may not begin with a '!' character or be null.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFStringDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

    /// <summary>
    /// (Optional) Optional list of Data-keys to remove from UserData. Some SDKs cannot insert null-values
    /// into Data due to language constraints. Use this to delete the keys directly.
    /// </summary>
    _Maybenull_ _Field_size_(keysToRemoveCount) const char* const* keysToRemove;

    /// <summary>
    /// Count of keysToRemove
    /// </summary>
    uint32_t keysToRemoveCount;

    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

} PFPlayerDataManagementUpdateUserInternalDataRequest;

#pragma pop_macro("IN")

}
