#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef PF_ENTITY_KEY_DEFINED
#define PF_ENTITY_KEY_DEFINED

/// <summary>
/// PFEntityKey data model. Combined entity type and ID structure which uniquely identifies a single entity.
/// </summary>
/// <remarks>
/// For more information about entities, see https://docs.microsoft.com/gaming/playfab/features/data/entities/.
/// </remarks>
typedef struct PFEntityKey
{
    /// <summary>
    /// Unique ID of the entity.
    /// </summary>
    _Null_terminated_ const char* id;

    /// <summary>
    /// Entity type. See https://docs.microsoft.com/gaming/playfab/features/data/entities/available-built-in-entity-types.
    /// </summary>
    /// <remarks>
    /// Player entities are typically the <c>title_player_account</c> type. For more information, see
    /// https://docs.microsoft.com/gaming/playfab/features/data/entities/available-built-in-entity-types.
    /// </remarks>
    _Null_terminated_ const char* type;
} PFEntityKey;

#endif // PF_ENTITY_KEY_DEFINED

#ifdef __cplusplus
}
#endif
