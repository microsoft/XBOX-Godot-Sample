// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>
#include <playfab/core/PFTypes.h>
extern "C"
{

#pragma push_macro("IN")
#undef IN

// PlayFab Services HRESULT error codes (FACILITY_XBOX + 0x5A00 + offset)

// Service errors
#define E_PF_SERVICES_NOT_INITIALIZED           MAKE_E_HC(0x5A00L) // 0x89235A00
#define E_PF_SERVICES_ALREADY_INITIALIZED       MAKE_E_HC(0x5A01L) // 0x89235A01

/// <summary>
/// OperationTypes enum.
/// </summary>
enum class PFOperationTypes : uint32_t
{
    Created,
    Updated,
    Deleted,
    None
};

/// <summary>
/// ResetInterval enum.
/// </summary>
enum class PFResetInterval : uint32_t
{
    Manual,
    Hour,
    Day,
    Week,
    Month
};

/// <summary>
/// PFVersionConfiguration data model.
/// </summary>
typedef struct PFVersionConfiguration
{
    /// <summary>
    /// The maximum number of versions of this leaderboard/statistic that can be queried. .
    /// </summary>
    int32_t maxQueryableVersions;

    /// <summary>
    /// Reset interval that statistics or leaderboards will reset on. When using Manual intervalthe reset
    /// can only be increased by calling the Increase version API. When using Hour interval the resetwill
    /// occur at the start of the next hour UTC time. When using Day interval the reset will occur at
    /// thestart of the next day in UTC time. When using the Week interval the reset will occur at the
    /// start ofthe next Monday in UTC time. When using Month interval the reset will occur at the start
    /// of the nextmonth in UTC time.
    /// </summary>
    PFResetInterval resetInterval;

} PFVersionConfiguration;

#pragma pop_macro("IN")

}
