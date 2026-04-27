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
/// PFLocalizationGetLanguageListRequest data model.
/// </summary>
typedef struct PFLocalizationGetLanguageListRequest
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

} PFLocalizationGetLanguageListRequest;

/// <summary>
/// PFLocalizationGetLanguageListResponse data model.
/// </summary>
typedef struct PFLocalizationGetLanguageListResponse
{
    /// <summary>
    /// (Optional) The list of allowed languages, in BCP47 two-letter format.
    /// </summary>
    _Maybenull_ _Field_size_(languageListCount) const char* const* languageList;

    /// <summary>
    /// Count of languageList
    /// </summary>
    uint32_t languageListCount;

} PFLocalizationGetLanguageListResponse;

#pragma pop_macro("IN")

}
