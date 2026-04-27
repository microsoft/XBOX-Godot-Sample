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
/// PFExperimentationGetTreatmentAssignmentRequest data model. Given a title player or a title entity
/// token, returns the treatment variants and variables assigned to the entity across all running experiments.
/// </summary>
typedef struct PFExperimentationGetTreatmentAssignmentRequest
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

} PFExperimentationGetTreatmentAssignmentRequest;

/// <summary>
/// PFExperimentationGetTreatmentAssignmentResult data model.
/// </summary>
typedef struct PFExperimentationGetTreatmentAssignmentResult
{
    /// <summary>
    /// (Optional) Treatment assignment for the entity.
    /// </summary>
    _Maybenull_ PFTreatmentAssignment const* treatmentAssignment;

} PFExperimentationGetTreatmentAssignmentResult;

#pragma pop_macro("IN")

}
