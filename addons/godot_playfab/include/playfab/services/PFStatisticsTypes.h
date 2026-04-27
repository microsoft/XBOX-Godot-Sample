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
/// StatisticAggregationMethod enum.
/// </summary>
enum class PFStatisticsStatisticAggregationMethod : uint32_t
{
    Last,
    Min,
    Max,
    Sum
};

/// <summary>
/// PFStatisticsStatisticColumn data model.
/// </summary>
typedef struct PFStatisticsStatisticColumn
{
    /// <summary>
    /// Aggregation method for calculating new value of a statistic.
    /// </summary>
    PFStatisticsStatisticAggregationMethod aggregationMethod;

    /// <summary>
    /// Name of the statistic column, as originally configured.
    /// </summary>
    _Null_terminated_ const char* name;

} PFStatisticsStatisticColumn;

/// <summary>
/// PFStatisticsCreateStatisticDefinitionRequest data model.
/// </summary>
typedef struct PFStatisticsCreateStatisticDefinitionRequest
{
    /// <summary>
    /// (Optional) [In Preview]: The list of statistic definition names whose scores must be aggregated
    /// towards this stat. If AggregationSource is specified, the entityType of this definition MUST be
    /// Title (making it a CommunityStat). Currently, only one aggregation source can be specified.
    /// </summary>
    _Maybenull_ _Field_size_(aggregationSourcesCount) const char* const* aggregationSources;

    /// <summary>
    /// Count of aggregationSources
    /// </summary>
    uint32_t aggregationSourcesCount;

    /// <summary>
    /// (Optional) The columns for the statistic defining the aggregation method for each column.
    /// </summary>
    _Maybenull_ _Field_size_(columnsCount) PFStatisticsStatisticColumn const* const* columns;

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
    /// (Optional) The entity type allowed to have score(s) for this statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* entityType;

    /// <summary>
    /// Name of the statistic. Must be less than 150 characters. Restricted to a-Z, 0-9, '(', ')', '_',
    /// '-' and '.'.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) The version reset configuration for the statistic definition.
    /// </summary>
    _Maybenull_ PFVersionConfiguration const* versionConfiguration;

} PFStatisticsCreateStatisticDefinitionRequest;

/// <summary>
/// PFStatisticsDeleteStatisticDefinitionRequest data model.
/// </summary>
typedef struct PFStatisticsDeleteStatisticDefinitionRequest
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
    /// Name of the statistic to delete.
    /// </summary>
    _Null_terminated_ const char* name;

} PFStatisticsDeleteStatisticDefinitionRequest;

/// <summary>
/// PFStatisticsStatisticDelete data model.
/// </summary>
typedef struct PFStatisticsStatisticDelete
{
    /// <summary>
    /// Name of the statistic, as originally configured.
    /// </summary>
    _Null_terminated_ const char* name;

} PFStatisticsStatisticDelete;

/// <summary>
/// PFStatisticsDeleteStatisticsRequest data model.
/// </summary>
typedef struct PFStatisticsDeleteStatisticsRequest
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
    /// Collection of statistics to remove from this entity.
    /// </summary>
    _Field_size_(statisticsCount) PFStatisticsStatisticDelete const* const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

} PFStatisticsDeleteStatisticsRequest;

/// <summary>
/// PFStatisticsDeleteStatisticsResponse data model.
/// </summary>
typedef struct PFStatisticsDeleteStatisticsResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

} PFStatisticsDeleteStatisticsResponse;

/// <summary>
/// PFStatisticsGetStatisticDefinitionRequest data model.
/// </summary>
typedef struct PFStatisticsGetStatisticDefinitionRequest
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
    /// Name of the statistic. Must be less than 150 characters.
    /// </summary>
    _Null_terminated_ const char* name;

} PFStatisticsGetStatisticDefinitionRequest;

/// <summary>
/// PFStatisticsGetStatisticDefinitionResponse data model.
/// </summary>
typedef struct PFStatisticsGetStatisticDefinitionResponse
{
    /// <summary>
    /// (Optional) The list of statistic definitions names this definition aggregates to. .
    /// </summary>
    _Maybenull_ _Field_size_(aggregationDestinationsCount) const char* const* aggregationDestinations;

    /// <summary>
    /// Count of aggregationDestinations
    /// </summary>
    uint32_t aggregationDestinationsCount;

    /// <summary>
    /// (Optional) The list of statistic definitions names whose values must be aggregated towards this
    /// stat. If AggregationSource is specified, the entityType of this definition MUST be Title (making
    /// it a CommunityStat). Currently, only one aggregation source can be specified.
    /// </summary>
    _Maybenull_ _Field_size_(aggregationSourcesCount) const char* const* aggregationSources;

    /// <summary>
    /// Count of aggregationSources
    /// </summary>
    uint32_t aggregationSourcesCount;

    /// <summary>
    /// (Optional) The columns for the statistic defining the aggregation method for each column.
    /// </summary>
    _Maybenull_ _Field_size_(columnsCount) PFStatisticsStatisticColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

    /// <summary>
    /// Created time, in UTC.
    /// </summary>
    time_t created;

    /// <summary>
    /// (Optional) The entity type that can have this statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* entityType;

    /// <summary>
    /// (Optional) Last time, in UTC, statistic version was incremented.
    /// </summary>
    _Maybenull_ time_t const* lastResetTime;

    /// <summary>
    /// (Optional) The list of leaderboards that are linked to this statistic definition.
    /// </summary>
    _Maybenull_ _Field_size_(linkedLeaderboardNamesCount) const char* const* linkedLeaderboardNames;

    /// <summary>
    /// Count of linkedLeaderboardNames
    /// </summary>
    uint32_t linkedLeaderboardNamesCount;

    /// <summary>
    /// (Optional) Name of the statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// Statistic version.
    /// </summary>
    uint32_t version;

    /// <summary>
    /// (Optional) The version reset configuration for the leaderboard definition.
    /// </summary>
    _Maybenull_ PFVersionConfiguration const* versionConfiguration;

} PFStatisticsGetStatisticDefinitionResponse;

/// <summary>
/// PFStatisticsGetStatisticsRequest data model.
/// </summary>
typedef struct PFStatisticsGetStatisticsRequest
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

} PFStatisticsGetStatisticsRequest;

/// <summary>
/// PFStatisticsStatisticColumnCollection data model.
/// </summary>
typedef struct PFStatisticsStatisticColumnCollection
{
    /// <summary>
    /// (Optional) Columns for the statistic defining the aggregation method for each column.
    /// </summary>
    _Maybenull_ _Field_size_(columnsCount) PFStatisticsStatisticColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

} PFStatisticsStatisticColumnCollection;

/// <summary>
/// PFStatisticsEntityStatisticValue data model.
/// </summary>
typedef struct PFStatisticsEntityStatisticValue
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

} PFStatisticsEntityStatisticValue;

/// <summary>
/// PFStatisticsGetStatisticsResponse data model.
/// </summary>
typedef struct PFStatisticsGetStatisticsResponse
{
    /// <summary>
    /// (Optional) A mapping of statistic name to the columns defined in the corresponding definition.
    /// </summary>
    _Maybenull_ _Field_size_(columnDetailsCount) struct PFStatisticsStatisticColumnCollectionDictionaryEntry const* columnDetails;

    /// <summary>
    /// Count of columnDetails
    /// </summary>
    uint32_t columnDetailsCount;

    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) List of statistics keyed by Name. Only the latest version of a statistic is returned.
    /// </summary>
    _Maybenull_ _Field_size_(statisticsCount) struct PFStatisticsEntityStatisticValueDictionaryEntry const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

} PFStatisticsGetStatisticsResponse;

/// <summary>
/// PFStatisticsGetStatisticsForEntitiesRequest data model.
/// </summary>
typedef struct PFStatisticsGetStatisticsForEntitiesRequest
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
    /// Collection of Entity IDs to retrieve statistics for.
    /// </summary>
    _Field_size_(entitiesCount) PFEntityKey const* const* entities;

    /// <summary>
    /// Count of entities
    /// </summary>
    uint32_t entitiesCount;

} PFStatisticsGetStatisticsForEntitiesRequest;

/// <summary>
/// PFStatisticsEntityStatistics data model.
/// </summary>
typedef struct PFStatisticsEntityStatistics
{
    /// <summary>
    /// (Optional) Entity key.
    /// </summary>
    _Maybenull_ PFEntityKey const* entityKey;

    /// <summary>
    /// (Optional) All statistics for the given entitykey.
    /// </summary>
    _Maybenull_ _Field_size_(statisticsCount) PFStatisticsEntityStatisticValue const* const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

} PFStatisticsEntityStatistics;

/// <summary>
/// PFStatisticsGetStatisticsForEntitiesResponse data model.
/// </summary>
typedef struct PFStatisticsGetStatisticsForEntitiesResponse
{
    /// <summary>
    /// (Optional) A mapping of statistic name to the columns defined in the corresponding definition.
    /// </summary>
    _Maybenull_ _Field_size_(columnDetailsCount) struct PFStatisticsStatisticColumnCollectionDictionaryEntry const* columnDetails;

    /// <summary>
    /// Count of columnDetails
    /// </summary>
    uint32_t columnDetailsCount;

    /// <summary>
    /// (Optional) List of entities mapped to their statistics. Only the latest version of a statistic
    /// is returned.
    /// </summary>
    _Maybenull_ _Field_size_(entitiesStatisticsCount) PFStatisticsEntityStatistics const* const* entitiesStatistics;

    /// <summary>
    /// Count of entitiesStatistics
    /// </summary>
    uint32_t entitiesStatisticsCount;

} PFStatisticsGetStatisticsForEntitiesResponse;

/// <summary>
/// PFStatisticsIncrementStatisticVersionRequest data model.
/// </summary>
typedef struct PFStatisticsIncrementStatisticVersionRequest
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
    /// Name of the statistic to increment the version of.
    /// </summary>
    _Null_terminated_ const char* name;

} PFStatisticsIncrementStatisticVersionRequest;

/// <summary>
/// PFStatisticsIncrementStatisticVersionResponse data model.
/// </summary>
typedef struct PFStatisticsIncrementStatisticVersionResponse
{
    /// <summary>
    /// New statistic version.
    /// </summary>
    uint32_t version;

} PFStatisticsIncrementStatisticVersionResponse;

/// <summary>
/// PFStatisticsListStatisticDefinitionsRequest data model.
/// </summary>
typedef struct PFStatisticsListStatisticDefinitionsRequest
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

} PFStatisticsListStatisticDefinitionsRequest;

/// <summary>
/// PFStatisticsStatisticDefinition data model.
/// </summary>
typedef struct PFStatisticsStatisticDefinition
{
    /// <summary>
    /// (Optional) The list of statistic definitions names this definition aggregates to. .
    /// </summary>
    _Maybenull_ _Field_size_(aggregationDestinationsCount) const char* const* aggregationDestinations;

    /// <summary>
    /// Count of aggregationDestinations
    /// </summary>
    uint32_t aggregationDestinationsCount;

    /// <summary>
    /// (Optional) The list of statistic definitions names whose values must be aggregated towards this
    /// stat. If AggregationSource is specified, the entityType of this definition MUST be Title (making
    /// it a CommunityStat). Currently, only one aggregation source can be specified.
    /// </summary>
    _Maybenull_ _Field_size_(aggregationSourcesCount) const char* const* aggregationSources;

    /// <summary>
    /// Count of aggregationSources
    /// </summary>
    uint32_t aggregationSourcesCount;

    /// <summary>
    /// (Optional) The columns for the statistic defining the aggregation method for each column.
    /// </summary>
    _Maybenull_ _Field_size_(columnsCount) PFStatisticsStatisticColumn const* const* columns;

    /// <summary>
    /// Count of columns
    /// </summary>
    uint32_t columnsCount;

    /// <summary>
    /// Created time, in UTC.
    /// </summary>
    time_t created;

    /// <summary>
    /// (Optional) The entity type that can have this statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* entityType;

    /// <summary>
    /// (Optional) Last time, in UTC, statistic version was incremented.
    /// </summary>
    _Maybenull_ time_t const* lastResetTime;

    /// <summary>
    /// (Optional) The list of leaderboards that are linked to this statistic definition.
    /// </summary>
    _Maybenull_ _Field_size_(linkedLeaderboardNamesCount) const char* const* linkedLeaderboardNames;

    /// <summary>
    /// Count of linkedLeaderboardNames
    /// </summary>
    uint32_t linkedLeaderboardNamesCount;

    /// <summary>
    /// (Optional) Name of the statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// Statistic version.
    /// </summary>
    uint32_t version;

    /// <summary>
    /// (Optional) The version reset configuration for the leaderboard definition.
    /// </summary>
    _Maybenull_ PFVersionConfiguration const* versionConfiguration;

} PFStatisticsStatisticDefinition;

/// <summary>
/// PFStatisticsListStatisticDefinitionsResponse data model.
/// </summary>
typedef struct PFStatisticsListStatisticDefinitionsResponse
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
    /// (Optional) List of statistic definitions for the title.
    /// </summary>
    _Maybenull_ _Field_size_(statisticDefinitionsCount) PFStatisticsStatisticDefinition const* const* statisticDefinitions;

    /// <summary>
    /// Count of statisticDefinitions
    /// </summary>
    uint32_t statisticDefinitionsCount;

} PFStatisticsListStatisticDefinitionsResponse;

/// <summary>
/// PFStatisticsUpdateStatisticDefinitionRequest data model.
/// </summary>
typedef struct PFStatisticsUpdateStatisticDefinitionRequest
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
    /// Name of the statistic. Must be less than 150 characters. Restricted to a-Z, 0-9, '(', ')', '_',
    /// '-' and '.'.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) The version reset configuration for the statistic definition.
    /// </summary>
    _Maybenull_ PFVersionConfiguration const* versionConfiguration;

} PFStatisticsUpdateStatisticDefinitionRequest;

/// <summary>
/// PFStatisticsStatisticUpdate data model.
/// </summary>
typedef struct PFStatisticsStatisticUpdate
{
    /// <summary>
    /// (Optional) Arbitrary metadata to store along side the statistic, will be returned by all Leaderboard
    /// APIs. Must be less than 50 UTF8 encoded characters.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* metadata;

    /// <summary>
    /// Name of the statistic, as originally configured.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) Statistic scores for the entity. This will be used in accordance with the aggregation
    /// method configured for the statistics.The maximum value allowed for each individual score is 9223372036854775807.
    /// The minimum value for each individual score is -9223372036854775807The values are formatted as
    /// strings to avoid interop issues with client libraries unable to handle 64bit integers.
    /// </summary>
    _Maybenull_ _Field_size_(scoresCount) const char* const* scores;

    /// <summary>
    /// Count of scores
    /// </summary>
    uint32_t scoresCount;

    /// <summary>
    /// (Optional) Optional field to indicate the version of the statistic to set. When empty defaults
    /// to the statistic's current version.
    /// </summary>
    _Maybenull_ uint32_t const* version;

} PFStatisticsStatisticUpdate;

/// <summary>
/// PFStatisticsUpdateStatisticsRequest data model.
/// </summary>
typedef struct PFStatisticsUpdateStatisticsRequest
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
    /// Collection of statistics to update, maximum 50.
    /// </summary>
    _Field_size_(statisticsCount) PFStatisticsStatisticUpdate const* const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

    /// <summary>
    /// (Optional) Optional transactionId of this update which can be used to ensure idempotence. Using
    /// this field is still in testing stage.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* transactionId;

} PFStatisticsUpdateStatisticsRequest;

/// <summary>
/// PFStatisticsUpdateStatisticsResponse data model.
/// </summary>
typedef struct PFStatisticsUpdateStatisticsResponse
{
    /// <summary>
    /// (Optional) A mapping of statistic name to the columns defined in the corresponding definition.
    /// </summary>
    _Maybenull_ _Field_size_(columnDetailsCount) struct PFStatisticsStatisticColumnCollectionDictionaryEntry const* columnDetails;

    /// <summary>
    /// Count of columnDetails
    /// </summary>
    uint32_t columnDetailsCount;

    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) Updated entity profile statistics.
    /// </summary>
    _Maybenull_ _Field_size_(statisticsCount) struct PFStatisticsEntityStatisticValueDictionaryEntry const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

} PFStatisticsUpdateStatisticsResponse;

/// <summary>
/// Dictionary entry for an associative array with PFStatisticsStatisticColumnCollection values.
/// </summary>
typedef struct PFStatisticsStatisticColumnCollectionDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFStatisticsStatisticColumnCollection const* value;
} PFStatisticsStatisticColumnCollectionDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFStatisticsEntityStatisticValue values.
/// </summary>
typedef struct PFStatisticsEntityStatisticValueDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFStatisticsEntityStatisticValue const* value;
} PFStatisticsEntityStatisticValueDictionaryEntry;

#pragma pop_macro("IN")

}
