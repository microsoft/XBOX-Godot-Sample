// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <XUser.h>

extern "C"
{

#ifndef PF_ENABLE_XAL_AUTH
#define PF_PLATFORM_USER
struct PFLocalUserPlatformContext
{
    XUserHandle user;
};
#endif // PF_ENABLE_XAL_AUTH

}
