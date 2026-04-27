// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>

extern "C"
{

PF_API PFTraceEnableTraceToFile(
    _In_z_ const char* traceFileDirectory
) noexcept;

}
