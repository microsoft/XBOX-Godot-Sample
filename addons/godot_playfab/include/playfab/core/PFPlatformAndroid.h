// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include "jni.h"
#ifdef PF_ENABLE_XAL_AUTH
#include <Xal.h>
#endif

extern "C"
{

#define PF_PLATFORM_USER

#ifdef PF_ENABLE_XAL_AUTH
struct PFLocalUserPlatformContext
{
    XalUserHandle userHandle;
};
#else
/// <summary>
/// Creates a LocalUser using the authenticated Google Play Games user. The user must be successfully authenticated prior
/// to calling PFLocalUserCreateHandle or the PlayFab SDK will be unable to retreive the Server auth token it needs.
/// See https://developers.google.com/games/services/android/signin for more details.
/// </summary>
struct PFLocalUserPlatformContext
{
    /// <summary>
    /// Google Play Id for the player. Can be obtained after authenticating with Google Play Games by calling 
    /// PlayGames.getPlayersClient(...).getCurrentPlayerId().
    /// </summary>
    const char* playerId;

    /// <summary>
    /// Google Play Games sign in client that PlayFab will use to obtain a server auth token. The signInClient can be
    /// obtained by calling PlayGames.getGamesSignInClient(...). See https://developer.android.com/games/pgs/android/server-access 
    /// for more details.
    /// </summary>
    jobject signinClient;

    /// <summary>
    /// OAuth Client ID registered for your game server in the Google Play console. See https://developer.android.com/games/pgs/android/server-access
    /// for more details.
    /// </summary>
    const char* OAuth2WebClientId;
};
#endif // PF_ENABLE_XAL_AUTH

}
