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
/// EffectType enum.
/// </summary>
enum class PFProfilesEffectType : uint32_t
{
    Allow,
    Deny
};

/// <summary>
/// PFProfilesGetEntityProfileRequest data model. Given an entity type and entity identifier will retrieve
/// the profile from the entity store. If the profile being retrieved is the caller's, then the read operation
/// is consistent, if not it is an inconsistent read. An inconsistent read means that we do not guarantee
/// all committed writes have occurred before reading the profile, allowing for a stale read. If consistency
/// is important the Version Number on the result can be used to compare which version of the profile
/// any reader has.
/// </summary>
typedef struct PFProfilesGetEntityProfileRequest
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
    /// (Optional) Determines whether the objects will be returned as an escaped JSON string or as a
    /// un-escaped JSON object. Default is JSON string.
    /// </summary>
    _Maybenull_ bool const* dataAsObject;

    /// <summary>
    /// (Optional) The optional entity to perform this action on. Defaults to the currently logged in
    /// entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

} PFProfilesGetEntityProfileRequest;

/// <summary>
/// PFProfilesEntityProfileFileMetadata data model. An entity file's meta data. To get a download URL
/// call File/GetFiles API.
/// </summary>
typedef struct PFProfilesEntityProfileFileMetadata
{
    /// <summary>
    /// (Optional) Checksum value for the file, can be used to check if the file on the server has changed.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* checksum;

    /// <summary>
    /// (Optional) Name of the file.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* fileName;

    /// <summary>
    /// Last UTC time the file was modified.
    /// </summary>
    time_t lastModified;

    /// <summary>
    /// Storage service's reported byte count.
    /// </summary>
    int32_t size;

} PFProfilesEntityProfileFileMetadata;

/// <summary>
/// PFProfilesEntityDataObject data model. An entity object and its associated meta data.
/// </summary>
typedef struct PFProfilesEntityDataObject
{
    /// <summary>
    /// (Optional) Un-escaped JSON object, if DataAsObject is true.
    /// </summary>
    PFJsonObject dataObject;

    /// <summary>
    /// (Optional) Escaped string JSON body of the object, if DataAsObject is default or false.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* escapedDataObject;

    /// <summary>
    /// (Optional) Name of this object.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* objectName;

} PFProfilesEntityDataObject;

/// <summary>
/// PFProfilesEntityPermissionStatement data model.
/// </summary>
typedef struct PFProfilesEntityPermissionStatement
{
    /// <summary>
    /// The action this statement effects. May be 'Read', 'Write' or '*' for both read and write.
    /// </summary>
    _Null_terminated_ const char* action;

    /// <summary>
    /// (Optional) A comment about the statement. Intended solely for bookkeeping and debugging.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* comment;

    /// <summary>
    /// (Optional) Additional conditions to be applied for entity resources.
    /// </summary>
    PFJsonObject condition;

    /// <summary>
    /// The effect this statement will have. It may be either Allow or Deny.
    /// </summary>
    PFProfilesEffectType effect;

    /// <summary>
    /// The principal this statement will effect.
    /// </summary>
    PFJsonObject principal;

    /// <summary>
    /// The resource this statements effects. Similar to 'pfrn:data--title![Title ID]/Profile/*'.
    /// </summary>
    _Null_terminated_ const char* resource;

} PFProfilesEntityPermissionStatement;

/// <summary>
/// PFEntityStatisticValue data model.
/// </summary>
typedef struct PFEntityStatisticValue
{
    /// <summary>
    /// (Optional) Metadata associated with the Statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* metadata;

    /// <summary>
    /// (Optional) Statistic name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) Statistic scores.
    /// </summary>
    _Maybenull_ _Field_size_(scoresCount) const char* const* scores;

    /// <summary>
    /// Count of scores
    /// </summary>
    uint32_t scoresCount;

    /// <summary>
    /// Statistic version.
    /// </summary>
    int32_t version;

} PFEntityStatisticValue;

/// <summary>
/// PFProfilesEntityProfileBody data model.
/// </summary>
typedef struct PFProfilesEntityProfileBody
{
    /// <summary>
    /// (Optional) Avatar URL for the entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* avatarUrl;

    /// <summary>
    /// The creation time of this profile in UTC.
    /// </summary>
    time_t created;

    /// <summary>
    /// (Optional) The display name of the entity. This field may serve different purposes for different
    /// entity types. i.e.: for a title player account it could represent the display name of the player,
    /// whereas on a character it could be character's name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* displayName;

    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The chain of responsibility for this entity. Use Lineage.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* entityChain;

    /// <summary>
    /// (Optional) The experiment variants of this profile.
    /// </summary>
    _Maybenull_ _Field_size_(experimentVariantsCount) const char* const* experimentVariants;

    /// <summary>
    /// Count of experimentVariants
    /// </summary>
    uint32_t experimentVariantsCount;

    /// <summary>
    /// (Optional) The files on this profile.
    /// </summary>
    _Maybenull_ _Field_size_(filesCount) struct PFProfilesEntityProfileFileMetadataDictionaryEntry const* files;

    /// <summary>
    /// Count of files
    /// </summary>
    uint32_t filesCount;

    /// <summary>
    /// (Optional) The language on this profile.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* language;

    /// <summary>
    /// (Optional) The lineage of this profile.
    /// </summary>
    _Maybenull_ PFEntityLineage const* lineage;

    /// <summary>
    /// (Optional) The objects on this profile.
    /// </summary>
    _Maybenull_ _Field_size_(objectsCount) struct PFProfilesEntityDataObjectDictionaryEntry const* objects;

    /// <summary>
    /// Count of objects
    /// </summary>
    uint32_t objectsCount;

    /// <summary>
    /// (Optional) The permissions that govern access to this entity profile and its properties. Only
    /// includes permissions set on this profile, not global statements from titles and namespaces.
    /// </summary>
    _Maybenull_ _Field_size_(permissionsCount) PFProfilesEntityPermissionStatement const* const* permissions;

    /// <summary>
    /// Count of permissions
    /// </summary>
    uint32_t permissionsCount;

    /// <summary>
    /// (Optional) The statistics on this profile.
    /// </summary>
    _Maybenull_ _Field_size_(statisticsCount) struct PFEntityStatisticValueDictionaryEntry const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

    /// <summary>
    /// The version number of the profile in persistent storage at the time of the read. Used for optional
    /// optimistic concurrency during update.
    /// </summary>
    int32_t versionNumber;

} PFProfilesEntityProfileBody;

/// <summary>
/// PFProfilesGetEntityProfileResponse data model.
/// </summary>
typedef struct PFProfilesGetEntityProfileResponse
{
    /// <summary>
    /// (Optional) Entity profile.
    /// </summary>
    _Maybenull_ PFProfilesEntityProfileBody const* profile;

} PFProfilesGetEntityProfileResponse;

/// <summary>
/// PFProfilesGetEntityProfilesRequest data model. Given a set of entity types and entity identifiers
/// will retrieve all readable profiles properties for the caller. Profiles that the caller is not allowed
/// to read will silently not be included in the results.
/// </summary>
typedef struct PFProfilesGetEntityProfilesRequest
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
    /// (Optional) Determines whether the objects will be returned as an escaped JSON string or as a
    /// un-escaped JSON object. Default is JSON string.
    /// </summary>
    _Maybenull_ bool const* dataAsObject;

    /// <summary>
    /// Entity keys of the profiles to load. Must be between 1 and 25.
    /// </summary>
    _Field_size_(entitiesCount) PFEntityKey const* const* entities;

    /// <summary>
    /// Count of entities
    /// </summary>
    uint32_t entitiesCount;

} PFProfilesGetEntityProfilesRequest;

/// <summary>
/// PFProfilesGetEntityProfilesResponse data model.
/// </summary>
typedef struct PFProfilesGetEntityProfilesResponse
{
    /// <summary>
    /// (Optional) Entity profiles.
    /// </summary>
    _Maybenull_ _Field_size_(profilesCount) PFProfilesEntityProfileBody const* const* profiles;

    /// <summary>
    /// Count of profiles
    /// </summary>
    uint32_t profilesCount;

} PFProfilesGetEntityProfilesResponse;

/// <summary>
/// PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest data model. Given a master player account
/// id (PlayFab ID), returns all title player accounts associated with it.
/// </summary>
typedef struct PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest
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
    /// Master player account ids.
    /// </summary>
    _Field_size_(masterPlayerAccountIdsCount) const char* const* masterPlayerAccountIds;

    /// <summary>
    /// Count of masterPlayerAccountIds
    /// </summary>
    uint32_t masterPlayerAccountIdsCount;

    /// <summary>
    /// (Optional) Id of title to get players from.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* titleId;

} PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest;

/// <summary>
/// PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse data model.
/// </summary>
typedef struct PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse
{
    /// <summary>
    /// (Optional) Optional id of title to get players from, required if calling using a master_player_account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* titleId;

    /// <summary>
    /// (Optional) Dictionary of master player ids mapped to title player entity keys and id pairs.
    /// </summary>
    _Maybenull_ _Field_size_(titlePlayerAccountsCount) struct PFEntityKeyDictionaryEntry const* titlePlayerAccounts;

    /// <summary>
    /// Count of titlePlayerAccounts
    /// </summary>
    uint32_t titlePlayerAccountsCount;

} PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse;

/// <summary>
/// PFProfilesSetProfileLanguageRequest data model. Given an entity profile, will update its language
/// to the one passed in if the profile's version is equal to the one passed in.
/// </summary>
typedef struct PFProfilesSetProfileLanguageRequest
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
    /// (Optional) The optional entity to perform this action on. Defaults to the currently logged in
    /// entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The expected version of a profile to perform this update on.
    /// </summary>
    _Maybenull_ int32_t const* expectedVersion;

    /// <summary>
    /// (Optional) The language to set on the given entity. Deletes the profile's language if passed
    /// in a null string.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* language;

} PFProfilesSetProfileLanguageRequest;

/// <summary>
/// PFProfilesSetProfileLanguageResponse data model.
/// </summary>
typedef struct PFProfilesSetProfileLanguageResponse
{
    /// <summary>
    /// (Optional) The type of operation that occured on the profile's language.
    /// </summary>
    _Maybenull_ PFOperationTypes const* operationResult;

    /// <summary>
    /// (Optional) The updated version of the profile after the language update.
    /// </summary>
    _Maybenull_ int32_t const* versionNumber;

} PFProfilesSetProfileLanguageResponse;

/// <summary>
/// PFProfilesSetEntityProfilePolicyRequest data model. This will set the access policy statements on
/// the given entity profile. This is not additive, any existing statements will be replaced with the
/// statements in this request.
/// </summary>
typedef struct PFProfilesSetEntityProfilePolicyRequest
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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The statements to include in the access policy.
    /// </summary>
    _Field_size_(statementsCount) PFProfilesEntityPermissionStatement const* const* statements;

    /// <summary>
    /// Count of statements
    /// </summary>
    uint32_t statementsCount;

} PFProfilesSetEntityProfilePolicyRequest;

/// <summary>
/// PFProfilesSetEntityProfilePolicyResponse data model.
/// </summary>
typedef struct PFProfilesSetEntityProfilePolicyResponse
{
    /// <summary>
    /// (Optional) The permissions that govern access to this entity profile and its properties. Only
    /// includes permissions set on this profile, not global statements from titles and namespaces.
    /// </summary>
    _Maybenull_ _Field_size_(permissionsCount) PFProfilesEntityPermissionStatement const* const* permissions;

    /// <summary>
    /// Count of permissions
    /// </summary>
    uint32_t permissionsCount;

} PFProfilesSetEntityProfilePolicyResponse;

/// <summary>
/// Dictionary entry for an associative array with PFProfilesEntityProfileFileMetadata values.
/// </summary>
typedef struct PFProfilesEntityProfileFileMetadataDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFProfilesEntityProfileFileMetadata const* value;
} PFProfilesEntityProfileFileMetadataDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFProfilesEntityDataObject values.
/// </summary>
typedef struct PFProfilesEntityDataObjectDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFProfilesEntityDataObject const* value;
} PFProfilesEntityDataObjectDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFProfilesEntityStatisticValue values.
/// </summary>
typedef struct PFEntityStatisticValueDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFEntityStatisticValue const* value;
} PFEntityStatisticValueDictionaryEntry;

#pragma pop_macro("IN")

}
