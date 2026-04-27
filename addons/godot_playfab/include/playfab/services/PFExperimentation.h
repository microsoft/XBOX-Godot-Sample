// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFExperimentationTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Gets the treatment assignments for a player for every running experiment in the title.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// When the asynchronous task is complete, call <see cref="PFExperimentationGetTreatmentAssignmentGetResultSize"/>
/// and <see cref="PFExperimentationGetTreatmentAssignmentGetResult"/> to get the result.
/// </remarks>
PF_API PFExperimentationGetTreatmentAssignmentAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFExperimentationGetTreatmentAssignmentRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetTreatmentAssignment call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EXPERIMENTATION_CLIENT_TIMEOUT,
/// E_PF_EXPERIMENTATION_TREATMENT_ASSIGNMENT_DISABLED, E_PF_EXPERIMENTATION_TREATMENT_ASSIGNMENT_FAILED,
/// E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_ENTITY_TYPE or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFExperimentationGetTreatmentAssignmentGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFExperimentationGetTreatmentAssignmentAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_EXPERIMENTATION_CLIENT_TIMEOUT,
/// E_PF_EXPERIMENTATION_TREATMENT_ASSIGNMENT_DISABLED, E_PF_EXPERIMENTATION_TREATMENT_ASSIGNMENT_FAILED,
/// E_PF_FEATURE_NOT_CONFIGURED_FOR_TITLE, E_PF_INVALID_ENTITY_TYPE or any of the global PlayFab Service
/// errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFExperimentationGetTreatmentAssignmentGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFExperimentationGetTreatmentAssignmentResult** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}