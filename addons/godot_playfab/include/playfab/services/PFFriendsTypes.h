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
/// ExternalFriendSources enum.
/// </summary>
enum class PFFriendsExternalFriendSources : uint32_t
{
    None = 0x0,
    Steam = 0x1,
    Facebook = 0x2,
    Xbox = 0x4,
    Psn = 0x8,
    All = 0x10
};

DEFINE_ENUM_FLAG_OPERATORS(PFFriendsExternalFriendSources);

/// <summary>
/// PFFriendsClientAddFriendRequest data model.
/// </summary>
typedef struct PFFriendsClientAddFriendRequest
{
    /// <summary>
    /// (Optional) Email address of the user to attempt to add to the local user's friend list.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendEmail;

    /// <summary>
    /// (Optional) PlayFab identifier of the user to attempt to add to the local user's friend list.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendPlayFabId;

    /// <summary>
    /// (Optional) Title-specific display name of the user to attempt to add to the local user's friend
    /// list.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendTitleDisplayName;

    /// <summary>
    /// (Optional) PlayFab username of the user to attempt to add to the local user's friend list.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendUsername;

} PFFriendsClientAddFriendRequest;

/// <summary>
/// PFFriendsAddFriendResult data model.
/// </summary>
typedef struct PFFriendsAddFriendResult
{
    /// <summary>
    /// True if the friend request was processed successfully.
    /// </summary>
    bool created;

} PFFriendsAddFriendResult;

/// <summary>
/// PFFriendsClientGetFriendsListRequest data model.
/// </summary>
typedef struct PFFriendsClientGetFriendsListRequest
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
    /// (Optional) Indicates which other platforms' friends should be included in the response. In HTTP,
    /// it is represented as a comma-separated list of platforms.
    /// </summary>
    _Maybenull_ PFFriendsExternalFriendSources const* externalPlatformFriends;

    /// <summary>
    /// (Optional) If non-null, this determines which properties of the resulting player profiles to
    /// return. For API calls from the client, only the allowed client profile properties for the title
    /// may be requested. These allowed properties are configured in the Game Manager "Client Profile
    /// Options" tab in the "Settings" section.
    /// </summary>
    _Maybenull_ PFPlayerProfileViewConstraints const* profileConstraints;

#if HC_PLATFORM == HC_PLATFORM_GDK
    /// <summary>
    /// (Optional) XUserHandle if Xbox friends should be included.
    /// </summary>
    XUserHandle user;
#else
    /// <summary>
    /// (Optional) Xbox token if Xbox friends should be included. Requires Xbox be configured on PlayFab.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* xboxToken;
#endif

} PFFriendsClientGetFriendsListRequest;

/// <summary>
/// PFFriendsFriendInfo data model.
/// </summary>
typedef struct PFFriendsFriendInfo
{
    /// <summary>
    /// (Optional) Available Facebook information (if the user and connected Facebook friend both have
    /// PlayFab Accounts in the same title).
    /// </summary>
    _Maybenull_ PFUserFacebookInfo const* facebookInfo;

    /// <summary>
    /// (Optional) PlayFab unique identifier for this friend.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendPlayFabId;

    /// <summary>
    /// (Optional) Available Game Center information (if the user and connected Game Center friend both
    /// have PlayFab Accounts in the same title).
    /// </summary>
    _Maybenull_ PFUserGameCenterInfo const* gameCenterInfo;

    /// <summary>
    /// (Optional) The profile of the user, if requested.
    /// </summary>
    _Maybenull_ PFPlayerProfileModel const* profile;

    /// <summary>
    /// (Optional) Available PlayStation :tm: Network information, if the user connected PlayStation
    /// :tm Network friend both have PlayFab Accounts in the same title.
    /// </summary>
    _Maybenull_ PFUserPsnInfo const* PSNInfo;

    /// <summary>
    /// (Optional) Available Steam information (if the user and connected Steam friend both have PlayFab
    /// Accounts in the same title).
    /// </summary>
    _Maybenull_ PFUserSteamInfo const* steamInfo;

    /// <summary>
    /// (Optional) Tags which have been associated with this friend.
    /// </summary>
    _Maybenull_ _Field_size_(tagsCount) const char* const* tags;

    /// <summary>
    /// Count of tags
    /// </summary>
    uint32_t tagsCount;

    /// <summary>
    /// (Optional) Title-specific display name for this friend.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* titleDisplayName;

    /// <summary>
    /// (Optional) PlayFab unique username for this friend.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* username;

    /// <summary>
    /// (Optional) Available Xbox information, (if the user and connected Xbox Live friend both have
    /// PlayFab Accounts in the same title).
    /// </summary>
    _Maybenull_ PFUserXboxInfo const* xboxInfo;

} PFFriendsFriendInfo;

/// <summary>
/// PFFriendsGetFriendsListResult data model. If any additional services are queried for the user's friends,
/// those friends who also have a PlayFab account registered for the title will be returned in the results.
/// For Facebook, user has to have logged into the title's Facebook app recently, and only friends who
/// also plays this game will be included. Note: If the user authenticated with AuthenticationToken when
/// calling LoginWithFacebook, instead of AcessToken, an empty list will be returned. For Xbox Live, user
/// has to have logged into the Xbox Live recently, and only friends who also play this game will be included.
/// </summary>
typedef struct PFFriendsGetFriendsListResult
{
    /// <summary>
    /// (Optional) Array of friends found.
    /// </summary>
    _Maybenull_ _Field_size_(friendsCount) PFFriendsFriendInfo const* const* friends;

    /// <summary>
    /// Count of friends
    /// </summary>
    uint32_t friendsCount;

} PFFriendsGetFriendsListResult;

/// <summary>
/// PFFriendsClientRemoveFriendRequest data model.
/// </summary>
typedef struct PFFriendsClientRemoveFriendRequest
{
    /// <summary>
    /// PlayFab identifier of the friend account which is to be removed.
    /// </summary>
    _Null_terminated_ const char* friendPlayFabId;

} PFFriendsClientRemoveFriendRequest;

/// <summary>
/// PFFriendsClientSetFriendTagsRequest data model. This operation is not additive. It will completely
/// replace the tag list for the specified user. Please note that only users in the PlayFab friends list
/// can be assigned tags. Attempting to set a tag on a friend only included in the friends list from a
/// social site integration (such as Facebook or Steam) will return the AccountNotFound error.
/// </summary>
typedef struct PFFriendsClientSetFriendTagsRequest
{
    /// <summary>
    /// PlayFab identifier of the friend account to which the tag(s) should be applied.
    /// </summary>
    _Null_terminated_ const char* friendPlayFabId;

    /// <summary>
    /// Array of tags to set on the friend account.
    /// </summary>
    _Field_size_(tagsCount) const char* const* tags;

    /// <summary>
    /// Count of tags
    /// </summary>
    uint32_t tagsCount;

} PFFriendsClientSetFriendTagsRequest;

/// <summary>
/// PFFriendsServerAddFriendRequest data model.
/// </summary>
typedef struct PFFriendsServerAddFriendRequest
{
    /// <summary>
    /// (Optional) Email address of the user being added.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendEmail;

    /// <summary>
    /// (Optional) The PlayFab identifier of the user being added.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendPlayFabId;

    /// <summary>
    /// (Optional) Title-specific display name of the user to being added.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendTitleDisplayName;

    /// <summary>
    /// (Optional) The PlayFab username of the user being added.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* friendUsername;

    /// <summary>
    /// PlayFab identifier of the player to add a new friend.
    /// </summary>
    _Null_terminated_ const char* playFabId;

} PFFriendsServerAddFriendRequest;

/// <summary>
/// PFFriendsServerGetFriendsListRequest data model.
/// </summary>
typedef struct PFFriendsServerGetFriendsListRequest
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
    /// (Optional) Indicates which other platforms' friends should be included in the response. In HTTP,
    /// it is represented as a comma-separated list of platforms.
    /// </summary>
    _Maybenull_ PFFriendsExternalFriendSources const* externalPlatformFriends;

    /// <summary>
    /// PlayFab identifier of the player whose friend list to get.
    /// </summary>
    _Null_terminated_ const char* playFabId;

    /// <summary>
    /// (Optional) If non-null, this determines which properties of the resulting player profiles to
    /// return. For API calls from the client, only the allowed client profile properties for the title
    /// may be requested. These allowed properties are configured in the Game Manager "Client Profile
    /// Options" tab in the "Settings" section.
    /// </summary>
    _Maybenull_ PFPlayerProfileViewConstraints const* profileConstraints;

    /// <summary>
    /// (Optional) Xbox token if Xbox friends should be included. Requires Xbox be configured on PlayFab.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* xboxToken;

} PFFriendsServerGetFriendsListRequest;

/// <summary>
/// PFFriendsServerRemoveFriendRequest data model.
/// </summary>
typedef struct PFFriendsServerRemoveFriendRequest
{
    /// <summary>
    /// PlayFab identifier of the friend account which is to be removed.
    /// </summary>
    _Null_terminated_ const char* friendPlayFabId;

    /// <summary>
    /// Unique PlayFab assigned ID of the user on whom the operation will be performed.
    /// </summary>
    _Null_terminated_ const char* playFabId;

} PFFriendsServerRemoveFriendRequest;

/// <summary>
/// PFFriendsServerSetFriendTagsRequest data model. This operation is not additive. It will completely
/// replace the tag list for the specified user. Please note that only users in the PlayFab friends list
/// can be assigned tags. Attempting to set a tag on a friend only included in the friends list from a
/// social site integration (such as Facebook or Steam) will return the AccountNotFound error.
/// </summary>
typedef struct PFFriendsServerSetFriendTagsRequest
{
    /// <summary>
    /// PlayFab identifier of the friend account to which the tag(s) should be applied.
    /// </summary>
    _Null_terminated_ const char* friendPlayFabId;

    /// <summary>
    /// PlayFab identifier of the player whose friend is to be updated.
    /// </summary>
    _Null_terminated_ const char* playFabId;

    /// <summary>
    /// Array of tags to set on the friend account.
    /// </summary>
    _Field_size_(tagsCount) const char* const* tags;

    /// <summary>
    /// Count of tags
    /// </summary>
    uint32_t tagsCount;

} PFFriendsServerSetFriendTagsRequest;

#pragma pop_macro("IN")

}
