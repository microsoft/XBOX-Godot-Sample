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
/// PFTitleDataManagementGetPublisherDataRequest data model. This API is designed to return publisher-specific
/// values which can be read, but not written to, by the client. This data is shared across all titles
/// assigned to a particular publisher, and can be used for cross-game coordination. Only titles assigned
/// to a publisher can use this API. For more information email helloplayfab@microsoft.com. Note that
/// there may up to a minute delay in between updating title data and this API call returning the newest
/// value.
/// </summary>
typedef struct PFTitleDataManagementGetPublisherDataRequest
{
    /// <summary>
    ///  array of keys to get back data from the Publisher data blob, set by the admin tools.
    /// </summary>
    _Field_size_(keysCount) const char* const* keys;

    /// <summary>
    /// Count of keys
    /// </summary>
    uint32_t keysCount;

} PFTitleDataManagementGetPublisherDataRequest;

/// <summary>
/// PFTitleDataManagementGetPublisherDataResult data model.
/// </summary>
typedef struct PFTitleDataManagementGetPublisherDataResult
{
    /// <summary>
    /// (Optional) A dictionary object of key / value pairs.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFStringDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

} PFTitleDataManagementGetPublisherDataResult;

/// <summary>
/// PFTitleDataManagementGetTimeResult data model. Time is always returned as Coordinated Universal Time
/// (UTC).
/// </summary>
typedef struct PFTitleDataManagementGetTimeResult
{
    /// <summary>
    /// Current server time when the request was received, in UTC.
    /// </summary>
    time_t time;

} PFTitleDataManagementGetTimeResult;

/// <summary>
/// PFTitleDataManagementGetTitleDataRequest data model. This API is designed to return title specific
/// values which can be read, but not written to, by the client. For example, a developer could choose
/// to store values which modify the user experience, such as enemy spawn rates, weapon strengths, movement
/// speeds, etc. This allows a developer to update the title without the need to create, test, and ship
/// a new build. If the player belongs to an experiment variant that uses title data overrides, the overrides
/// are applied automatically and returned with the title data. Note that there may up to a minute delay
/// in between updating title data and this API call returning the newest value.
/// </summary>
typedef struct PFTitleDataManagementGetTitleDataRequest
{
    /// <summary>
    /// (Optional) Specific keys to search for in the title data (leave null to get all keys).
    /// </summary>
    _Maybenull_ _Field_size_(keysCount) const char* const* keys;

    /// <summary>
    /// Count of keys
    /// </summary>
    uint32_t keysCount;

    /// <summary>
    /// (Optional) Optional field that specifies the name of an override. This value is ignored when
    /// used by the game client; otherwise, the overrides are applied automatically to the title data.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* overrideLabel;

} PFTitleDataManagementGetTitleDataRequest;

/// <summary>
/// PFTitleDataManagementGetTitleDataResult data model.
/// </summary>
typedef struct PFTitleDataManagementGetTitleDataResult
{
    /// <summary>
    /// (Optional) A dictionary object of key / value pairs.
    /// </summary>
    _Maybenull_ _Field_size_(dataCount) struct PFStringDictionaryEntry const* data;

    /// <summary>
    /// Count of data
    /// </summary>
    uint32_t dataCount;

} PFTitleDataManagementGetTitleDataResult;

/// <summary>
/// PFTitleDataManagementGetTitleNewsRequest data model.
/// </summary>
typedef struct PFTitleDataManagementGetTitleNewsRequest
{
    /// <summary>
    /// (Optional) Limits the results to the last n entries. Defaults to 10 if not set.
    /// </summary>
    _Maybenull_ int32_t const* count;

} PFTitleDataManagementGetTitleNewsRequest;

/// <summary>
/// PFTitleDataManagementTitleNewsItem data model.
/// </summary>
typedef struct PFTitleDataManagementTitleNewsItem
{
    /// <summary>
    /// (Optional) News item text.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* body;

    /// <summary>
    /// (Optional) Unique identifier of news item.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* newsId;

    /// <summary>
    /// Date and time when the news item was posted.
    /// </summary>
    time_t timestamp;

    /// <summary>
    /// (Optional) Title of the news item.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* title;

} PFTitleDataManagementTitleNewsItem;

/// <summary>
/// PFTitleDataManagementGetTitleNewsResult data model.
/// </summary>
typedef struct PFTitleDataManagementGetTitleNewsResult
{
    /// <summary>
    /// (Optional) Array of news items.
    /// </summary>
    _Maybenull_ _Field_size_(newsCount) PFTitleDataManagementTitleNewsItem const* const* news;

    /// <summary>
    /// Count of news
    /// </summary>
    uint32_t newsCount;

} PFTitleDataManagementGetTitleNewsResult;

/// <summary>
/// PFTitleDataManagementSetPublisherDataRequest data model. This API is designed to store publisher-specific
/// values which can be read, but not written to, by the client. This data is shared across all titles
/// assigned to a particular publisher, and can be used for cross-game coordination. Only titles assigned
/// to a publisher can use this API. This operation is additive. If a Key does not exist in the current
/// dataset, it will be added with the specified Value. If it already exists, the Value for that key will
/// be overwritten with the new Value. For more information email helloplayfab@microsoft.com.
/// </summary>
typedef struct PFTitleDataManagementSetPublisherDataRequest
{
    /// <summary>
    /// Key we want to set a value on (note, this is additive - will only replace an existing key's value
    /// if they are the same name.) Keys are trimmed of whitespace. Keys may not begin with the '!' character.
    /// </summary>
    _Null_terminated_ const char* key;

    /// <summary>
    /// (Optional) New value to set. Set to null to remove a value.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* value;

} PFTitleDataManagementSetPublisherDataRequest;

/// <summary>
/// PFTitleDataManagementSetTitleDataRequest data model. This API is designed to store title specific
/// values which can be read, but not written to, by the client. For example, a developer could choose
/// to store values which modify the user experience, such as enemy spawn rates, weapon strengths, movement
/// speeds, etc. This allows a developer to update the title without the need to create, test, and ship
/// a new build. This operation is additive. If a Key does not exist in the current dataset, it will be
/// added with the specified Value. If it already exists, the Value for that key will be overwritten with
/// the new Value.
/// </summary>
typedef struct PFTitleDataManagementSetTitleDataRequest
{
    /// <summary>
    /// Key we want to set a value on (note, this is additive - will only replace an existing key's value
    /// if they are the same name.) Keys are trimmed of whitespace. Keys may not begin with the '!' character.
    /// </summary>
    _Null_terminated_ const char* key;

    /// <summary>
    /// (Optional) New value to set. Set to null to remove a value.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* value;

} PFTitleDataManagementSetTitleDataRequest;

#pragma pop_macro("IN")

}
