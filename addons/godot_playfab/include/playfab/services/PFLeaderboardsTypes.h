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
/// LeaderboardSortDirection enum.
/// </summary>
enum class PFLeaderboardsLeaderboardSortDirection : uint32_t
{
    Descending,
    Ascending
};

/// <summary>
/// ExternalFriendSources enum.
/// </summary>
enum class PFExternalFriendSources : uint32_t
{
    None = 0x0,
    Steam = 0x1,
    Facebook = 0x2,
    Xbox = 0x4,
    Psn = 0x8,
    All = 0x10
};

DEFINE_ENUM_FLAG_OPERATORS(PFExternalFriendSources);

/// <summary>
/// PFLeaderboardsLinkedStatisticColumn data model.
/// </summary>
typedef struct PFLeaderboardsLinkedStatisticColumn
{
    /// <summary>
    /// The name of the statistic column that this leaderboard column is sourced from.
    /// </summary>
    _Null_terminated_ const char* linkedStatisticColumnName;

    /// <summary>
    /// The name of the statistic.
    /// </summary>
    _Null_terminated_ const char* linkedStatisticName;

} PFLeaderboardsLinkedStatisticColumn;

/// <summary>
/// PFLeaderboardsLeaderboardColumn data model.
/// </summary>
typedef struct PFLeaderboardsLeaderboardColumn
{
    /// <summary>
    /// (Optional) If the value for this column is sourced from a statistic, details of the linked column.
    /// Null if the leaderboard is not linked.
    /// </summary>
    _Maybenull_ PFLeaderboardsLinkedStatisticColumn const* linkedStatisticColumn;

    /// <summary>
    /// A name for the leaderboard column, unique per leaderboard definition.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// The sort direction for this column.
    /// </summary>
    PFLeaderboardsLeaderboardSortDirection sortDirection;

} PFLeaderboardsLeaderboardColumn;

/// <summary>
/// PFLeaderboardsCreateLeaderboardDefinitionRequest data model.
/// </summary>
typedef struct PFLeaderboardsCreateLeaderboardDefinitionRequest
{
    /// <summary>
    /// Leaderboard columns describing the sort directions, cannot be changed after creation.
    /// </summary>
    _Field_size_(columnsCount) PFLeaderboardsLeaderboardColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

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
    /// The entity type being represented on the leaderboard. If it doesn't correspond to the PlayFab
    /// entity types, use 'external' as the type.
    /// </summary>
    _Null_terminated_ const char* entityType;

    /// <summary>
    /// A name for the leaderboard, unique per title.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// Maximum number of entries on this leaderboard.
    /// </summary>
    int32_t sizeLimit;

    /// <summary>
    /// The version reset configuration for the leaderboard definition.
    /// </summary>
    PFVersionConfiguration const* versionConfiguration;

} PFLeaderboardsCreateLeaderboardDefinitionRequest;

/// <summary>
/// PFLeaderboardsDeleteLeaderboardDefinitionRequest data model.
/// </summary>
typedef struct PFLeaderboardsDeleteLeaderboardDefinitionRequest
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
    /// The name of the leaderboard definition to delete.
    /// </summary>
    _Null_terminated_ const char* name;

} PFLeaderboardsDeleteLeaderboardDefinitionRequest;

/// <summary>
/// PFLeaderboardsDeleteLeaderboardEntriesRequest data model.
/// </summary>
typedef struct PFLeaderboardsDeleteLeaderboardEntriesRequest
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
    /// (Optional) The unique Ids of the entries to delete from the leaderboard.
    /// </summary>
    _Maybenull_ _Field_size_(entityIdsCount) const char* const* entityIds;

    /// <summary>
    /// Count of entityIds
    /// </summary>
    uint32_t entityIdsCount;

    /// <summary>
    /// The name of the leaderboard.
    /// </summary>
    _Null_terminated_ const char* name;

} PFLeaderboardsDeleteLeaderboardEntriesRequest;

/// <summary>
/// PFLeaderboardsGetFriendLeaderboardForEntityRequest data model.
/// </summary>
typedef struct PFLeaderboardsGetFriendLeaderboardForEntityRequest
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
    /// (Optional) Indicates which other platforms' friends should be included in the response. In HTTP,
    /// it is represented as a comma-separated list of platforms.
    /// </summary>
    _Maybenull_ PFExternalFriendSources const* externalFriendSources;

    /// <summary>
    /// Name of the leaderboard.
    /// </summary>
    _Null_terminated_ const char* leaderboardName;

    /// <summary>
    /// (Optional) Optional version of the leaderboard, defaults to current version.
    /// </summary>
    _Maybenull_ uint32_t const* version;

    /// <summary>
    /// (Optional) Xbox token if Xbox friends should be included. Requires Xbox be configured on PlayFab.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* xboxToken;

} PFLeaderboardsGetFriendLeaderboardForEntityRequest;

/// <summary>
/// PFLeaderboardsEntityLeaderboardEntry data model. Individual rank of an entity in a leaderboard.
/// </summary>
typedef struct PFLeaderboardsEntityLeaderboardEntry
{
    /// <summary>
    /// (Optional) Entity's display name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* displayName;

    /// <summary>
    /// (Optional) Entity identifier.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// The time at which the last update to the entry was recorded on the server.
    /// </summary>
    time_t lastUpdated;

    /// <summary>
    /// (Optional) An opaque blob of data stored on the leaderboard entry. Note that the metadata is
    /// not used for ranking purposes.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* metadata;

    /// <summary>
    /// Position on the leaderboard.
    /// </summary>
    int32_t rank;

    /// <summary>
    /// (Optional) Scores for the entry.
    /// </summary>
    _Maybenull_ _Field_size_(scoresCount) const char* const* scores;

    /// <summary>
    /// Count of scores
    /// </summary>
    uint32_t scoresCount;

} PFLeaderboardsEntityLeaderboardEntry;

/// <summary>
/// PFLeaderboardsGetEntityLeaderboardResponse data model. Leaderboard response.
/// </summary>
typedef struct PFLeaderboardsGetEntityLeaderboardResponse
{
    /// <summary>
    /// (Optional) Leaderboard columns describing the sort directions.
    /// </summary>
    _Maybenull_ _Field_size_(columnsCount) PFLeaderboardsLeaderboardColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

    /// <summary>
    /// The number of entries on the leaderboard.
    /// </summary>
    uint32_t entryCount;

    /// <summary>
    /// (Optional) The time the next scheduled reset will occur. Null if the leaderboard does not reset
    /// on a schedule.
    /// </summary>
    _Maybenull_ time_t const* nextReset;

    /// <summary>
    /// (Optional) Individual entity rankings in the leaderboard, in sorted order by rank.
    /// </summary>
    _Maybenull_ _Field_size_(rankingsCount) PFLeaderboardsEntityLeaderboardEntry const* const* rankings;

    /// <summary>
    /// Count of rankings
    /// </summary>
    uint32_t rankingsCount;

    /// <summary>
    /// Version of the leaderboard being returned.
    /// </summary>
    uint32_t version;

} PFLeaderboardsGetEntityLeaderboardResponse;

/// <summary>
/// PFLeaderboardsGetEntityLeaderboardRequest data model. Request to load a leaderboard.
/// </summary>
typedef struct PFLeaderboardsGetEntityLeaderboardRequest
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
    /// Name of the leaderboard.
    /// </summary>
    _Null_terminated_ const char* leaderboardName;

    /// <summary>
    /// Maximum number of results to return from the leaderboard. Minimum 1, maximum 100.
    /// </summary>
    uint32_t pageSize;

    /// <summary>
    /// (Optional) Index position to start from. 1 is beginning of leaderboard. .
    /// </summary>
    _Maybenull_ uint32_t const* startingPosition;

    /// <summary>
    /// (Optional) Optional version of the leaderboard, defaults to current version.
    /// </summary>
    _Maybenull_ uint32_t const* version;

} PFLeaderboardsGetEntityLeaderboardRequest;

/// <summary>
/// PFLeaderboardsGetLeaderboardAroundEntityRequest data model. Request to load a section of a leaderboard
/// centered on a specific entity.
/// </summary>
typedef struct PFLeaderboardsGetLeaderboardAroundEntityRequest
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
    /// Name of the leaderboard.
    /// </summary>
    _Null_terminated_ const char* leaderboardName;

    /// <summary>
    /// Number of surrounding entries to return (in addition to specified entity). In general, the number
    /// of ranks above and below will be split into half. For example, if the specified value is 10, 5
    /// ranks above and 5 ranks below will be retrieved. However, the numbers will get skewed in either
    /// direction when the specified entity is towards the top or bottom of the leaderboard. Also, the
    /// number of entries returned can be lower than the value specified for entries at the bottom of
    /// the leaderboard.
    /// </summary>
    uint32_t maxSurroundingEntries;

    /// <summary>
    /// (Optional) Optional version of the leaderboard, defaults to current.
    /// </summary>
    _Maybenull_ uint32_t const* version;

} PFLeaderboardsGetLeaderboardAroundEntityRequest;

/// <summary>
/// PFLeaderboardsGetLeaderboardDefinitionRequest data model.
/// </summary>
typedef struct PFLeaderboardsGetLeaderboardDefinitionRequest
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
    /// The name of the leaderboard to retrieve the definition for.
    /// </summary>
    _Null_terminated_ const char* name;

} PFLeaderboardsGetLeaderboardDefinitionRequest;

/// <summary>
/// PFLeaderboardsGetLeaderboardDefinitionResponse data model.
/// </summary>
typedef struct PFLeaderboardsGetLeaderboardDefinitionResponse
{
    /// <summary>
    /// Sort direction of the leaderboard columns, cannot be changed after creation.
    /// </summary>
    _Field_size_(columnsCount) PFLeaderboardsLeaderboardColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

    /// <summary>
    /// Created time, in UTC.
    /// </summary>
    time_t created;

    /// <summary>
    /// The entity type being represented on the leaderboard. If it doesn't correspond to the PlayFab
    /// entity types, use 'external' as the type.
    /// </summary>
    _Null_terminated_ const char* entityType;

    /// <summary>
    /// (Optional) Last time, in UTC, leaderboard version was incremented.
    /// </summary>
    _Maybenull_ time_t const* lastResetTime;

    /// <summary>
    /// A name for the leaderboard, unique per title.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// Maximum number of entries on this leaderboard.
    /// </summary>
    int32_t sizeLimit;

    /// <summary>
    /// Latest Leaderboard version.
    /// </summary>
    uint32_t version;

    /// <summary>
    /// The version reset configuration for the leaderboard definition.
    /// </summary>
    PFVersionConfiguration const* versionConfiguration;

} PFLeaderboardsGetLeaderboardDefinitionResponse;

/// <summary>
/// PFLeaderboardsGetLeaderboardForEntitiesRequest data model. Request a leaderboard limited to a collection
/// of entities.
/// </summary>
typedef struct PFLeaderboardsGetLeaderboardForEntitiesRequest
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
    /// Collection of Entity IDs to include in the leaderboard.
    /// </summary>
    _Field_size_(entityIdsCount) const char* const* entityIds;

    /// <summary>
    /// Count of entityIds
    /// </summary>
    uint32_t entityIdsCount;

    /// <summary>
    /// Name of the leaderboard.
    /// </summary>
    _Null_terminated_ const char* leaderboardName;

    /// <summary>
    /// (Optional) Optional version of the leaderboard, defaults to current.
    /// </summary>
    _Maybenull_ uint32_t const* version;

} PFLeaderboardsGetLeaderboardForEntitiesRequest;

/// <summary>
/// PFLeaderboardsIncrementLeaderboardVersionRequest data model.
/// </summary>
typedef struct PFLeaderboardsIncrementLeaderboardVersionRequest
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
    /// The name of the leaderboard to increment the version for.
    /// </summary>
    _Null_terminated_ const char* name;

} PFLeaderboardsIncrementLeaderboardVersionRequest;

/// <summary>
/// PFLeaderboardsIncrementLeaderboardVersionResponse data model.
/// </summary>
typedef struct PFLeaderboardsIncrementLeaderboardVersionResponse
{
    /// <summary>
    /// New Leaderboard version.
    /// </summary>
    uint32_t version;

} PFLeaderboardsIncrementLeaderboardVersionResponse;

/// <summary>
/// PFLeaderboardsListLeaderboardDefinitionsRequest data model.
/// </summary>
typedef struct PFLeaderboardsListLeaderboardDefinitionsRequest
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

} PFLeaderboardsListLeaderboardDefinitionsRequest;

/// <summary>
/// PFLeaderboardsLeaderboardDefinition data model.
/// </summary>
typedef struct PFLeaderboardsLeaderboardDefinition
{
    /// <summary>
    /// Sort direction of the leaderboard columns, cannot be changed after creation.
    /// </summary>
    _Field_size_(columnsCount) PFLeaderboardsLeaderboardColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

    /// <summary>
    /// Created time, in UTC.
    /// </summary>
    time_t created;

    /// <summary>
    /// The entity type being represented on the leaderboard. If it doesn't correspond to the PlayFab
    /// entity types, use 'external' as the type.
    /// </summary>
    _Null_terminated_ const char* entityType;

    /// <summary>
    /// (Optional) Last time, in UTC, leaderboard version was incremented.
    /// </summary>
    _Maybenull_ time_t const* lastResetTime;

    /// <summary>
    /// A name for the leaderboard, unique per title.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// Maximum number of entries on this leaderboard.
    /// </summary>
    int32_t sizeLimit;

    /// <summary>
    /// Latest Leaderboard version.
    /// </summary>
    uint32_t version;

    /// <summary>
    /// The version reset configuration for the leaderboard definition.
    /// </summary>
    PFVersionConfiguration const* versionConfiguration;

} PFLeaderboardsLeaderboardDefinition;

/// <summary>
/// PFLeaderboardsListLeaderboardDefinitionsResponse data model.
/// </summary>
typedef struct PFLeaderboardsListLeaderboardDefinitionsResponse
{
    /// <summary>
    /// (Optional) List of leaderboard definitions for the title.
    /// </summary>
    _Maybenull_ _Field_size_(leaderboardDefinitionsCount) PFLeaderboardsLeaderboardDefinition const* const* leaderboardDefinitions;

    /// <summary>
    /// Count of leaderboardDefinitions
    /// </summary>
    uint32_t leaderboardDefinitionsCount;

} PFLeaderboardsListLeaderboardDefinitionsResponse;

/// <summary>
/// PFLeaderboardsUnlinkLeaderboardFromStatisticRequest data model.
/// </summary>
typedef struct PFLeaderboardsUnlinkLeaderboardFromStatisticRequest
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
    /// The name of the leaderboard definition to unlink.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// The name of the statistic definition to unlink.
    /// </summary>
    _Null_terminated_ const char* statisticName;

} PFLeaderboardsUnlinkLeaderboardFromStatisticRequest;

/// <summary>
/// PFLeaderboardsUpdateLeaderboardDefinitionRequest data model.
/// </summary>
typedef struct PFLeaderboardsUpdateLeaderboardDefinitionRequest
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
    /// The name of the leaderboard to update the definition for.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) Maximum number of entries on this leaderboard.
    /// </summary>
    _Maybenull_ int32_t const* sizeLimit;

    /// <summary>
    /// (Optional) The version reset configuration for the leaderboard definition.
    /// </summary>
    _Maybenull_ PFVersionConfiguration const* versionConfiguration;

} PFLeaderboardsUpdateLeaderboardDefinitionRequest;

/// <summary>
/// PFLeaderboardsLeaderboardEntryUpdate data model.
/// </summary>
typedef struct PFLeaderboardsLeaderboardEntryUpdate
{
    /// <summary>
    /// The unique Id for the entry. If using PlayFab Entities, this would be the entityId of the entity.
    /// </summary>
    _Null_terminated_ const char* entityId;

    /// <summary>
    /// (Optional) Arbitrary metadata to store along side the leaderboard entry, will be returned by
    /// all Leaderboard APIs. Must be less than 50 UTF8 encoded characters.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* metadata;

    /// <summary>
    /// (Optional) The scores for the leaderboard. The number of values provided here must match the
    /// number of columns in the Leaderboard definition.
    /// </summary>
    _Maybenull_ _Field_size_(scoresCount) const char* const* scores;

    /// <summary>
    /// Count of scores
    /// </summary>
    uint32_t scoresCount;

} PFLeaderboardsLeaderboardEntryUpdate;

/// <summary>
/// PFLeaderboardsUpdateLeaderboardEntriesRequest data model.
/// </summary>
typedef struct PFLeaderboardsUpdateLeaderboardEntriesRequest
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
    /// (Optional) The entries to add or update on the leaderboard.
    /// </summary>
    _Maybenull_ _Field_size_(entriesCount) PFLeaderboardsLeaderboardEntryUpdate const* const* entries;

    /// <summary>
    /// Count of entries
    /// </summary>
    uint32_t entriesCount;

    /// <summary>
    /// The name of the leaderboard.
    /// </summary>
    _Null_terminated_ const char* leaderboardName;

} PFLeaderboardsUpdateLeaderboardEntriesRequest;

#pragma pop_macro("IN")

}
