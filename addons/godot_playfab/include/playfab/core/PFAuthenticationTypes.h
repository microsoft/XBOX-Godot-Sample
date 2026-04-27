// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>

#include <playfab/core/PFTypes.h>

#if HC_PLATFORM == HC_PLATFORM_GDK
#include <XUser.h>
#endif

extern "C"
{

#pragma push_macro("IN")
#undef IN

/// <summary>
/// IdentifiedDeviceType enum.
/// </summary>
enum class PFAuthenticationIdentifiedDeviceType : uint32_t
{
    Unknown,
    XboxOne,
    Scarlett,
    WindowsOneCore,
    WindowsOneCoreMobile,
    Win32,
    android,
    iOS,
    PlayStation,
    Nintendo
};

/// <summary>
/// PFAuthenticationLoginWithAndroidDeviceIDRequest data model. On Android devices, the recommendation
/// is to use the Settings.Secure.ANDROID_ID as the AndroidDeviceId, as described in this blog post (http://android-developers.blogspot.com/2011/03/identifying-app-installations.html).
/// More information on this identifier can be found in the Android documentation (http://developer.android.com/reference/android/provider/Settings.Secure.html).
/// If this is the first time a user has signed in with the Android device and CreateAccount is set to
/// true, a new PlayFab account will be created and linked to the Android device ID. In this case, no
/// email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is
/// linked to the Android device, an error indicating this will be returned, so that the title can guide
/// the user through creation of a PlayFab account. Please note that while multiple devices of this type
/// can be linked to a single user account, only the one most recently used to login (or most recently
/// linked) will be reflected in the user's account information. We will be updating to show all linked
/// devices in a future release.
/// </summary>
typedef struct PFAuthenticationLoginWithAndroidDeviceIDRequest
{
    /// <summary>
    /// (Optional) Specific model of the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* androidDevice;

    /// <summary>
    /// Android device identifier for the user's device.
    /// </summary>
    _Null_terminated_ const char* androidDeviceId;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Specific Operating System version for the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* OS;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithAndroidDeviceIDRequest;

/// <summary>
/// PFAuthenticationUserSettings data model.
/// </summary>
typedef struct PFAuthenticationUserSettings
{
    /// <summary>
    /// Boolean for whether this player is eligible for gathering device info.
    /// </summary>
    bool gatherDeviceInfo;

    /// <summary>
    /// Boolean for whether this player should report OnFocus play-time tracking.
    /// </summary>
    bool gatherFocusInfo;

    /// <summary>
    /// Boolean for whether this player is eligible for ad tracking.
    /// </summary>
    bool needsAttribution;

} PFAuthenticationUserSettings;

/// <summary>
/// PFAuthenticationLoginResult data model.
/// </summary>
typedef struct PFAuthenticationLoginResult
{
    /// <summary>
    /// (Optional) Results for requested info.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoResultPayload const* infoResultPayload;

    /// <summary>
    /// (Optional) The time of this user's previous login. If there was no previous login, then it's
    /// DateTime.MinValue.
    /// </summary>
    _Maybenull_ time_t const* lastLoginTime;

    /// <summary>
    /// True if the master_player_account was newly created on this login.
    /// </summary>
    bool newlyCreated;

    /// <summary>
    /// (Optional) Player's unique PlayFabId.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// (Optional) Settings specific to this user.
    /// </summary>
    _Maybenull_ PFAuthenticationUserSettings const* settingsForUser;

    /// <summary>
    /// (Optional) The experimentation treatments for this user at the time of login.
    /// </summary>
    _Maybenull_ PFTreatmentAssignment const* treatmentAssignment;

} PFAuthenticationLoginResult;

/// <summary>
/// PFAuthenticationLoginWithAppleRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithAppleRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// The JSON Web token (JWT) returned by Apple after login. Represented as the identityToken field
    /// in the authorization credential payload. If you choose to ignore the expiration date for identity
    /// tokens, you will receive an NotAuthorized error if Apple rotates the signing key. In this case,
    /// users have to login to provide a fresh identity token.
    /// </summary>
    _Null_terminated_ const char* identityToken;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithAppleRequest;

/// <summary>
/// PFAuthenticationLoginWithBattleNetRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithBattleNetRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// The JSON Web Token (JWT) returned by Battle.net after login.
    /// </summary>
    _Null_terminated_ const char* identityToken;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithBattleNetRequest;

/// <summary>
/// PFAuthenticationLoginWithCustomIDRequest data model. It is highly recommended that developers ensure
/// that it is extremely unlikely that a customer could generate an ID which is already in use by another
/// customer. If this is the first time a user has signed in with the Custom ID and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the Custom ID. In this case, no email
/// or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is linked
/// to the Custom ID, an error indicating this will be returned, so that the title can guide the user
/// through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationLoginWithCustomIDRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

    /// <summary>
    /// Custom unique identifier for the user, generated by the title.
    /// </summary>
    _Null_terminated_ const char* customId;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithCustomIDRequest;

/// <summary>
/// PFAuthenticationLoginWithEmailAddressRequest data model. Email address and password lengths are provided
/// for information purposes. The server will validate that data passed in conforms to the field definition
/// and report errors appropriately. It is recommended that developers not perform this validation locally,
/// so that future updates do not require client updates.
/// </summary>
typedef struct PFAuthenticationLoginWithEmailAddressRequest
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
    /// Email address for the account.
    /// </summary>
    _Null_terminated_ const char* email;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// Password for the PlayFab account (6-100 characters).
    /// </summary>
    _Null_terminated_ const char* password;

} PFAuthenticationLoginWithEmailAddressRequest;

/// <summary>
/// PFAuthenticationLoginWithFacebookRequest data model. Facebook sign-in is accomplished using the Facebook
/// User Access Token. More information on the Token can be found in the Facebook developer documentation
/// (https://developers.facebook.com/docs/facebook-login/access-tokens/). In Unity, for example, the Token
/// is available as AccessToken in the Facebook SDK ScriptableObject FB. If this is the first time a user
/// has signed in with the Facebook account and CreateAccount is set to true, a new PlayFab account will
/// be created and linked to the provided account's Facebook ID. In this case, no email or username will
/// be associated with the PlayFab account. Otherwise, if no PlayFab account is linked to the Facebook
/// account, an error indicating this will be returned, so that the title can guide the user through creation
/// of a PlayFab account. Note that titles should never re-use the same Facebook applications between
/// PlayFab Title IDs, as Facebook provides unique user IDs per application and doing so can result in
/// issues with the Facebook ID for the user in their PlayFab account information. If you must re-use
/// an application in a new PlayFab Title ID, please be sure to first unlink all accounts from Facebook,
/// or delete all users in the first Title ID. Note: If the user is authenticated with AuthenticationToken,
/// instead of AccessToken, the GetFriendsList API will return an empty list.
/// </summary>
typedef struct PFAuthenticationLoginWithFacebookRequest
{
    /// <summary>
    /// Unique identifier from Facebook for the user.
    /// </summary>
    _Null_terminated_ const char* accessToken;

    /// <summary>
    /// (Optional) Token used for limited login authentication.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* authenticationToken;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithFacebookRequest;

/// <summary>
/// PFAuthenticationLoginWithFacebookInstantGamesIdRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithFacebookInstantGamesIdRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// Facebook Instant Games signature for the user.
    /// </summary>
    _Null_terminated_ const char* facebookInstantGamesSignature;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithFacebookInstantGamesIdRequest;

/// <summary>
/// PFAuthenticationLoginWithGameCenterRequest data model. The Game Center player identifier (https://developer.apple.com/library/ios/documentation/Accounts/Reference/ACAccountClassRef/index.html#//apple_ref/occ/instp/ACAccount/identifier)
/// is a generated string which is stored on the local device. As with device identifiers, care must be
/// taken to never expose a player's Game Center identifier to end users, as that could result in a user's
/// account being compromised. If this is the first time a user has signed in with Game Center and CreateAccount
/// is set to true, a new PlayFab account will be created and linked to the Game Center identifier. In
/// this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the Game Center account, an error indicating this will be returned, so that the
/// title can guide the user through creation of a PlayFab account. If an invalid iOS Game Center player
/// identifier is used, an error indicating this will be returned.
/// </summary>
typedef struct PFAuthenticationLoginWithGameCenterRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Unique Game Center player id.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerId;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// (Optional) The URL for the public encryption key that will be used to verify the signature.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* publicKeyUrl;

    /// <summary>
    /// (Optional) A random value used to compute the hash and keep it randomized.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* salt;

    /// <summary>
    /// (Optional) The verification signature of the authentication payload.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* signature;

    /// <summary>
    /// (Optional) The integer representation of date and time that the signature was created on. PlayFab
    /// will reject authentication signatures not within 10 minutes of the server's current time.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* timestamp;

} PFAuthenticationLoginWithGameCenterRequest;

/// <summary>
/// PFAuthenticationLoginWithGoogleAccountRequest data model. Google sign-in is accomplished by obtaining
/// a Google OAuth 2.0 credential using the Google sign-in for Android APIs on the device and passing
/// it to this API. If this is the first time a user has signed in with the Google account and CreateAccount
/// is set to true, a new PlayFab account will be created and linked to the Google account. Otherwise,
/// if no PlayFab account is linked to the Google account, an error indicating this will be returned,
/// so that the title can guide the user through creation of a PlayFab account. The current (recommended)
/// method for obtaining a Google account credential in an Android application is to call GoogleSignInAccount.getServerAuthCode()
/// and send the auth code as the ServerAuthCode parameter of this API. Before doing this, you must create
/// an OAuth 2.0 web application client ID in the Google API Console and configure its client ID and secret
/// in the PlayFab Game Manager Google Add-on for your title. This method does not require prompting of
/// the user for additional Google account permissions, resulting in a user experience with the least
/// possible friction. For more information about obtaining the server auth code, see https://developers.google.com/identity/sign-in/android/offline-access.
/// The previous (deprecated) method was to obtain an OAuth access token by calling GetAccessToken() on
/// the client and passing it as the AccessToken parameter to this API. for the with the Google OAuth
/// 2.0 Access Token. More information on this change can be found in the Google developer documentation
/// (https://android-developers.googleblog.com/2016/01/play-games-permissions-are-changing-in.html).
/// </summary>
typedef struct PFAuthenticationLoginWithGoogleAccountRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// OAuth 2.0 server authentication code obtained on the client by calling the getServerAuthCode()
    /// (https://developers.google.com/identity/sign-in/android/offline-access) Google client API.
    /// </summary>
    _Null_terminated_ const char* serverAuthCode;

    /// <summary>
    /// (Optional) Optional boolean to opt out of setting the MPA email when creating a Google account,
    /// defaults to true.
    /// </summary>
    _Maybenull_ bool const* setEmail;

} PFAuthenticationLoginWithGoogleAccountRequest;

/// <summary>
/// PFAuthenticationLoginWithGooglePlayGamesServicesRequest data model. Google Play Games sign-in is
/// accomplished by obtaining a Google OAuth 2.0 credential using the Google Play Games sign-in for Android
/// APIs on the device and passing it to this API. If this is the first time a user has signed in with
/// the Google Play Games account and CreateAccount is set to true, a new PlayFab account will be created
/// and linked to the Google Play Games account. Otherwise, if no PlayFab account is linked to the Google
/// Play Games account, an error indicating this will be returned, so that the title can guide the user
/// through creation of a PlayFab account. The current (recommended) method for obtaining a Google Play
/// Games account credential in an Android application is to call GamesSignInClient.requestServerSideAccess()
/// and send the auth code as the ServerAuthCode parameter of this API. Before doing this, you must create
/// an OAuth 2.0 web application client ID in the Google API Console and configure its client ID and secret
/// in the PlayFab Game Manager Google Add-on for your title. This method does not require prompting of
/// the user for additional Google account permissions, resulting in a user experience with the least
/// possible friction. For more information about obtaining the server auth code, see https://developers.google.com/games/services/android/signin.
/// </summary>
typedef struct PFAuthenticationLoginWithGooglePlayGamesServicesRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// OAuth 2.0 server authentication code obtained on the client by calling the requestServerSideAccess()
    /// (https://developers.google.com/games/services/android/signin) Google Play Games client API.
    /// </summary>
    _Null_terminated_ const char* serverAuthCode;

} PFAuthenticationLoginWithGooglePlayGamesServicesRequest;

/// <summary>
/// PFAuthenticationLoginWithIOSDeviceIDRequest data model. On iOS devices, the identifierForVendor (https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIDevice_Class/index.html#//apple_ref/occ/instp/UIDevice/identifierForVendor)
/// must be used as the DeviceId, as the UIDevice uniqueIdentifier has been deprecated as of iOS 5, and
/// use of the advertisingIdentifier for this purpose will result in failure of Apple's certification
/// process. If this is the first time a user has signed in with the iOS device and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the vendor-specific iOS device ID. In
/// this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the iOS device, an error indicating this will be returned, so that the title
/// can guide the user through creation of a PlayFab account. Please note that while multiple devices
/// of this type can be linked to a single user account, only the one most recently used to login (or
/// most recently linked) will be reflected in the user's account information. We will be updating to
/// show all linked devices in a future release.
/// </summary>
typedef struct PFAuthenticationLoginWithIOSDeviceIDRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// Vendor-specific iOS identifier for the user's device.
    /// </summary>
    _Null_terminated_ const char* deviceId;

    /// <summary>
    /// (Optional) Specific model of the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* deviceModel;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Specific Operating System version for the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* OS;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithIOSDeviceIDRequest;

/// <summary>
/// PFAuthenticationLoginWithKongregateRequest data model. More details regarding Kongregate and their
/// game authentication system can be found at http://developers.kongregate.com/docs/virtual-goods/authentication.
/// Developers must provide the Kongregate user ID and auth token that are generated using the Kongregate
/// client library. PlayFab will combine these identifiers with the title's unique Kongregate app ID to
/// log the player into the Kongregate system. If CreateAccount is set to true and there is not already
/// a user matched to this Kongregate ID, then PlayFab will create a new account for this user and link
/// the ID. In this case, no email or username will be associated with the PlayFab account. If there is
/// already a different PlayFab user linked with this account, then an error will be returned.
/// </summary>
typedef struct PFAuthenticationLoginWithKongregateRequest
{
    /// <summary>
    /// Token issued by Kongregate's client API for the user.
    /// </summary>
    _Null_terminated_ const char* authTicket;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// Numeric user ID assigned by Kongregate.
    /// </summary>
    _Null_terminated_ const char* kongregateId;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithKongregateRequest;

/// <summary>
/// PFAuthenticationLoginWithNintendoServiceAccountRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithNintendoServiceAccountRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// The JSON Web token (JWT) returned by Nintendo after login.
    /// </summary>
    _Null_terminated_ const char* identityToken;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithNintendoServiceAccountRequest;

/// <summary>
/// PFAuthenticationLoginWithNintendoSwitchDeviceIdRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithNintendoSwitchDeviceIdRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Nintendo Switch unique identifier for the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* nintendoSwitchDeviceId;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithNintendoSwitchDeviceIdRequest;

/// <summary>
/// PFAuthenticationLoginWithOpenIdConnectRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithOpenIdConnectRequest
{
    /// <summary>
    /// A name that identifies which configured OpenID Connect provider relationship to use. Maximum
    /// 100 characters.
    /// </summary>
    _Null_terminated_ const char* connectionId;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// The JSON Web token (JWT) returned by the identity provider after login. Represented as the id_token
    /// field in the identity provider's response.
    /// </summary>
    _Null_terminated_ const char* idToken;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithOpenIdConnectRequest;

/// <summary>
/// PFAuthenticationLoginWithPlayFabRequest data model. Username and password lengths are provided for
/// information purposes. The server will validate that data passed in conforms to the field definition
/// and report errors appropriately. It is recommended that developers not perform this validation locally,
/// so that future updates to the username or password do not require client updates.
/// </summary>
typedef struct PFAuthenticationLoginWithPlayFabRequest
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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// Password for the PlayFab account (6-100 characters).
    /// </summary>
    _Null_terminated_ const char* password;

    /// <summary>
    /// PlayFab username for the account.
    /// </summary>
    _Null_terminated_ const char* username;

} PFAuthenticationLoginWithPlayFabRequest;

/// <summary>
/// PFAuthenticationLoginWithPSNRequest data model. If this is the first time a user has signed in with
/// the PlayStation :tm: Network account and CreateAccount is set to true, a new PlayFab account will
/// be created and linked to the PlayStation :tm: Network account. In this case, no email or username
/// will be associated with the PlayFab account. Otherwise, if no PlayFab account is linked to the PlayStation
/// :tm: Network account, an error indicating this will be returned, so that the title can guide the user
/// through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationLoginWithPSNRequest
{
    /// <summary>
    /// Auth code provided by the PlayStation :tm: Network OAuth provider.
    /// </summary>
    _Null_terminated_ const char* authCode;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Id of the PlayStation :tm: Network issuer environment. If null, defaults to production
    /// environment.
    /// </summary>
    _Maybenull_ int32_t const* issuerId;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// (Optional) Redirect URI supplied to PlayStation :tm: Network when requesting an auth code.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* redirectUri;

} PFAuthenticationLoginWithPSNRequest;

/// <summary>
/// PFAuthenticationLoginWithSteamRequest data model. Steam sign-in is accomplished with the Steam Session
/// Ticket. More information on the Ticket can be found in the Steamworks SDK, here: https://partner.steamgames.com/documentation/auth.
/// NOTE: For Steam authentication to work, the title must be configured with the Steam Application ID
/// and Web API Key in the PlayFab Game Manager (under Steam in the Add-ons Marketplace). You can obtain
/// a Web API Key from the Permissions page of any Group associated with your App ID in the Steamworks
/// site. If this is the first time a user has signed in with the Steam account and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the provided account's Steam ID. In this
/// case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account
/// is linked to the Steam account, an error indicating this will be returned, so that the title can guide
/// the user through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationLoginWithSteamRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// Authentication token for the user, returned as a byte array from Steam, and converted to a string
    /// (for example, the byte 0x08 should become "08").
    /// </summary>
    _Null_terminated_ const char* steamTicket;

    /// <summary>
    /// (Optional) True if ticket was generated using ISteamUser::GetAuthTicketForWebAPI() using "AzurePlayFab"
    /// as the identity string. False if the ticket was generated with ISteamUser::GetAuthSessionTicket().
    /// </summary>
    _Maybenull_ bool const* ticketIsServiceSpecific;

} PFAuthenticationLoginWithSteamRequest;

/// <summary>
/// PFAuthenticationLoginWithTwitchRequest data model. More details regarding Twitch and their authentication
/// system can be found at https://github.com/justintv/Twitch-API/blob/master/authentication.md. Developers
/// must provide the Twitch access token that is generated using one of the Twitch authentication flows.
/// PlayFab will use the title's unique Twitch Client ID to authenticate the token and log in to the PlayFab
/// system. If CreateAccount is set to true and there is not already a user matched to the Twitch username
/// that generated the token, then PlayFab will create a new account for this user and link the ID. In
/// this case, no email or username will be associated with the PlayFab account. If there is already a
/// different PlayFab user linked with this account, then an error will be returned.
/// </summary>
typedef struct PFAuthenticationLoginWithTwitchRequest
{
    /// <summary>
    /// Token issued by Twitch's API for the user.
    /// </summary>
    _Null_terminated_ const char* accessToken;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

} PFAuthenticationLoginWithTwitchRequest;

/// <summary>
/// PFAuthenticationLoginWithXboxRequest data model. If this is the first time a user has signed in with
/// the Xbox Live account and CreateAccount is set to true, a new PlayFab account will be created and
/// linked to the Xbox Live account. In this case, no email or username will be associated with the PlayFab
/// account. Otherwise, if no PlayFab account is linked to the Xbox Live account, an error indicating
/// this will be returned, so that the title can guide the user through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationLoginWithXboxRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// Token provided by the Xbox Live SDK/XDK method GetTokenAndSignatureAsync("POST", "https://playfabapi.com/",
    /// "").
    /// </summary>
    _Null_terminated_ const char* xboxToken;

} PFAuthenticationLoginWithXboxRequest;

#if HC_PLATFORM == HC_PLATFORM_GDK
/// <summary>
/// PFAuthenticationLoginWithXUserRequest data model. If this is the first time a user has signed in with
/// the Xbox Live account and CreateAccount is set to true, a new PlayFab account will be created and
/// linked to the Xbox Live account. In this case, no email or username will be associated with the PlayFab
/// account. Otherwise, if no PlayFab account is linked to the Xbox Live account, an error indicating
/// this will be returned, so that the title can guide the user through creation of a PlayFab account.
/// Request object for PFAuthenticationLoginWithXUserAsync.
/// </summary>
typedef struct PFAuthenticationLoginWithXUserRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// XUserHandle returned from XUserAddAsync or XUserAddByIdWithUiAsync.
    /// </summary>
    XUserHandle user;

} PFAuthenticationLoginWithXUserRequest;
#endif

/// <summary>
/// PFAuthenticationRegisterPlayFabUserRequest data model.
/// </summary>
typedef struct PFAuthenticationRegisterPlayFabUserRequest
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
    /// (Optional) An optional parameter for setting the display name for this title (3-25 characters).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* displayName;

    /// <summary>
    /// (Optional) User email address attached to their account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* email;

    /// <summary>
    /// (Optional) Base64 encoded body that is encrypted with the Title's public RSA key (Enterprise
    /// Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* encryptedRequest;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Password for the PlayFab account (6-100 characters).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* password;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// (Optional) An optional parameter that specifies whether both the username and email parameters
    /// are required. If true, both parameters are required; if false, the user must supply either the
    /// username or email parameter. The default value is true.
    /// </summary>
    _Maybenull_ bool const* requireBothUsernameAndEmail;

    /// <summary>
    /// Unique identifier for the title, found in the Settings > Game Properties section of the PlayFab
    /// developer site when a title has been selected.
    /// </summary>
    _Null_terminated_ const char* titleId;

    /// <summary>
    /// (Optional) PlayFab username for the account (3-20 characters).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* username;

} PFAuthenticationRegisterPlayFabUserRequest;

/// <summary>
/// PFAuthenticationRegisterPlayFabUserResult data model. Each account must have a unique email address
/// in the PlayFab service. Once created, the account may be associated with additional accounts (Steam,
/// Facebook, Game Center, etc.), allowing for added social network lists and achievements systems.
/// </summary>
typedef struct PFAuthenticationRegisterPlayFabUserResult
{
    /// <summary>
    /// (Optional) PlayFab unique identifier for this newly created account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// (Optional) Settings specific to this user.
    /// </summary>
    _Maybenull_ PFAuthenticationUserSettings const* settingsForUser;

    /// <summary>
    /// (Optional) PlayFab unique user name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* username;

} PFAuthenticationRegisterPlayFabUserResult;

/// <summary>
/// PFAuthenticationServerLoginWithAndroidDeviceIDRequest data model. On Android devices, the recommendation
/// is to use the Settings.Secure.ANDROID_ID as the AndroidDeviceId, as described in this blog post (http://android-developers.blogspot.com/2011/03/identifying-app-installations.html).
/// More information on this identifier can be found in the Android documentation (http://developer.android.com/reference/android/provider/Settings.Secure.html).
/// If this is the first time a user has signed in with the Android device and CreateAccount is set to
/// true, a new PlayFab account will be created and linked to the Android device ID. In this case, no
/// email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is
/// linked to the Android device, an error indicating this will be returned, so that the title can guide
/// the user through creation of a PlayFab account. Please note that while multiple devices of this type
/// can be linked to a single user account, only the one most recently used to login (or most recently
/// linked) will be reflected in the user's account information. We will be updating to show all linked
/// devices in a future release.
/// </summary>
typedef struct PFAuthenticationServerLoginWithAndroidDeviceIDRequest
{
    /// <summary>
    /// (Optional) Specific model of the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* androidDevice;

    /// <summary>
    /// Android device identifier for the user's device.
    /// </summary>
    _Null_terminated_ const char* androidDeviceId;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Specific Operating System version for the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* OS;

} PFAuthenticationServerLoginWithAndroidDeviceIDRequest;

/// <summary>
/// PFAuthenticationServerLoginWithCustomIDRequest data model. It is highly recommended that developers
/// ensure that it is extremely unlikely that a customer could generate an ID which is already in use
/// by another customer. If this is the first time a user has signed in with the Custom ID and CreateAccount
/// is set to true, a new PlayFab account will be created and linked to the Custom ID. In this case, no
/// email or username will be associated with the PlayFab account. Otherwise, if no PlayFab account is
/// linked to the Custom ID, an error indicating this will be returned, so that the title can guide the
/// user through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationServerLoginWithCustomIDRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

    /// <summary>
    /// Custom unique identifier for the user, generated by the title.
    /// </summary>
    _Null_terminated_ const char* customId;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

} PFAuthenticationServerLoginWithCustomIDRequest;

/// <summary>
/// PFAuthenticationServerLoginWithIOSDeviceIDRequest data model. On iOS devices, the identifierForVendor
/// (https://developer.apple.com/library/ios/documentation/UIKit/Reference/UIDevice_Class/index.html#//apple_ref/occ/instp/UIDevice/identifierForVendor)
/// must be used as the DeviceId, as the UIDevice uniqueIdentifier has been deprecated as of iOS 5, and
/// use of the advertisingIdentifier for this purpose will result in failure of Apple's certification
/// process. If this is the first time a user has signed in with the iOS device and CreateAccount is set
/// to true, a new PlayFab account will be created and linked to the vendor-specific iOS device ID. In
/// this case, no email or username will be associated with the PlayFab account. Otherwise, if no PlayFab
/// account is linked to the iOS device, an error indicating this will be returned, so that the title
/// can guide the user through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationServerLoginWithIOSDeviceIDRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// Vendor-specific iOS identifier for the user's device.
    /// </summary>
    _Null_terminated_ const char* deviceId;

    /// <summary>
    /// (Optional) Specific model of the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* deviceModel;

    /// <summary>
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Specific Operating System version for the user's device.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* OS;

} PFAuthenticationServerLoginWithIOSDeviceIDRequest;

/// <summary>
/// PFAuthenticationServerLoginWithPSNRequest data model. If this is the first time a user has signed
/// in with the PlayStation :tm: Network account and CreateAccount is set to true, a new PlayFab account
/// will be created and linked to the PlayStation :tm: Network account. In this case, no email or username
/// will be associated with the PlayFab account. Otherwise, if no PlayFab account is linked to the PlayStation
/// :tm: Network account, an error indicating this will be returned, so that the title can guide the user
/// through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationServerLoginWithPSNRequest
{
    /// <summary>
    /// Auth code provided by the PlayStation :tm: Network OAuth provider.
    /// </summary>
    _Null_terminated_ const char* authCode;

    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Id of the PlayStation :tm: Network issuer environment. If null, defaults to production
    /// environment.
    /// </summary>
    _Maybenull_ int32_t const* issuerId;

    /// <summary>
    /// Redirect URI supplied to PlayStation :tm: Network when requesting an auth code.
    /// </summary>
    _Null_terminated_ const char* redirectUri;

} PFAuthenticationServerLoginWithPSNRequest;

/// <summary>
/// PFAuthenticationLoginWithServerCustomIdRequest data model.
/// </summary>
typedef struct PFAuthenticationLoginWithServerCustomIdRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// (Optional) Player secret that is used to verify API request signatures (Enterprise Only).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerSecret;

    /// <summary>
    /// The backend server identifier for this player.
    /// </summary>
    _Null_terminated_ const char* serverCustomId;

} PFAuthenticationLoginWithServerCustomIdRequest;

/// <summary>
/// PFAuthenticationLoginWithSteamIdRequest data model. If this is the first time a user has signed in
/// with the Steam ID and CreateAccount is set to true, a new PlayFab account will be created and linked
/// to the Steam account. In this case, no email or username will be associated with the PlayFab account.
/// Otherwise, if no PlayFab account is linked to the Steam account, an error indicating this will be
/// returned, so that the title can guide the user through creation of a PlayFab account. Steam users
/// that are not logged into the Steam Client app will only have their Steam username synced, other data,
/// such as currency and country will not be available until they login while the Client is open.
/// </summary>
typedef struct PFAuthenticationLoginWithSteamIdRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// Unique Steam identifier for a user.
    /// </summary>
    _Null_terminated_ const char* steamId;

} PFAuthenticationLoginWithSteamIdRequest;

/// <summary>
/// PFAuthenticationServerLoginWithXboxRequest data model. If this is the first time a user has signed
/// in with the Xbox Live account and CreateAccount is set to true, a new PlayFab account will be created
/// and linked to the Xbox Live account. In this case, no email or username will be associated with the
/// PlayFab account. Otherwise, if no PlayFab account is linked to the Xbox Live account, an error indicating
/// this will be returned, so that the title can guide the user through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationServerLoginWithXboxRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// Token provided by the Xbox Live SDK/XDK method GetTokenAndSignatureAsync("POST", "https://playfabapi.com/",
    /// "").
    /// </summary>
    _Null_terminated_ const char* xboxToken;

} PFAuthenticationServerLoginWithXboxRequest;

/// <summary>
/// PFAuthenticationLoginWithXboxIdRequest data model. If this is the first time a user has signed in
/// with the Xbox ID and CreateAccount is set to true, a new PlayFab account will be created and linked
/// to the Xbox Live account. In this case, no email or username will be associated with the PlayFab account.
/// Otherwise, if no PlayFab account is linked to the Xbox Live account, an error indicating this will
/// be returned, so that the title can guide the user through creation of a PlayFab account.
/// </summary>
typedef struct PFAuthenticationLoginWithXboxIdRequest
{
    /// <summary>
    /// Automatically create a PlayFab account if one is not currently linked to this ID.
    /// </summary>
    bool createAccount;

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
    /// (Optional) Flags for which pieces of info to return for the user.
    /// </summary>
    _Maybenull_ PFGetPlayerCombinedInfoRequestParams const* infoRequestParameters;

    /// <summary>
    /// The id of Xbox Live sandbox.
    /// </summary>
    _Null_terminated_ const char* sandbox;

    /// <summary>
    /// Unique Xbox identifier for a user.
    /// </summary>
    _Null_terminated_ const char* xboxId;

} PFAuthenticationLoginWithXboxIdRequest;

#if HC_PLATFORM != HC_PLATFORM_GDK 
/// <summary>
/// PFAuthenticationAuthenticateCustomIdRequest data model. Create or return a game_server entity token.
/// Caller must be a title entity.
/// </summary>
typedef struct PFAuthenticationAuthenticateCustomIdRequest
{
    /// <summary>
    /// The customId used to create and retrieve game_server entity tokens. This is unique at the title
    /// level. CustomId must be between 32 and 100 characters.
    /// </summary>
    _Null_terminated_ const char* customId;

    /// <summary>
    /// (Optional) The optional custom tags associated with the request (e.g. build number, external
    /// trace identifiers, etc.).
    /// </summary>
    _Maybenull_ _Field_size_(customTagsCount) struct PFStringDictionaryEntry const* customTags;

    /// <summary>
    /// Count of customTags
    /// </summary>
    uint32_t customTagsCount;

} PFAuthenticationAuthenticateCustomIdRequest;
#endif

/// <summary>
/// PFAuthenticationEntityTokenResponse data model.
/// </summary>
typedef struct PFAuthenticationEntityTokenResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The token used to set X-EntityToken for all entity based API calls.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* entityToken;

    /// <summary>
    /// (Optional) The time the token will expire, if it is an expiring token, in UTC.
    /// </summary>
    _Maybenull_ time_t const* tokenExpiration;

} PFAuthenticationEntityTokenResponse;

#if HC_PLATFORM != HC_PLATFORM_GDK 
/// <summary>
/// PFAuthenticationAuthenticateCustomIdResult data model.
/// </summary>
typedef struct PFAuthenticationAuthenticateCustomIdResult
{
    /// <summary>
    /// (Optional) The token generated used to set X-EntityToken for game_server calls.
    /// </summary>
    _Maybenull_ PFAuthenticationEntityTokenResponse const* entityToken;

    /// <summary>
    /// True if the account was newly created on this authentication.
    /// </summary>
    bool newlyCreated;

} PFAuthenticationAuthenticateCustomIdResult;
#endif

/// <summary>
/// PFAuthenticationDeleteRequest data model. Delete a game_server entity. The caller can be the game_server
/// entity attempting to delete itself. Or a title entity attempting to delete game_server entities for
/// this title.
/// </summary>
typedef struct PFAuthenticationDeleteRequest
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
    /// The game_server entity to be removed.
    /// </summary>
    PFEntityKey const* entity;

} PFAuthenticationDeleteRequest;

/// <summary>
/// PFAuthenticationGetEntityRequest data model. This API must be called with X-SecretKey, X-Authentication
/// or X-EntityToken headers. An optional EntityKey may be included to attempt to set the resulting EntityToken
/// to a specific entity, however the entity must be a relation of the caller, such as the master_player_account
/// of a character. If sending X-EntityToken the account will be marked as freshly logged in and will
/// issue a new token. If using X-Authentication or X-EntityToken the header must still be valid and cannot
/// be expired or revoked.
/// </summary>
typedef struct PFAuthenticationGetEntityRequest
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

} PFAuthenticationGetEntityRequest;

/// <summary>
/// PFAuthenticationValidateEntityTokenRequest data model. Given an entity token, validates that it hasn't
/// expired or been revoked and will return details of the owner.
/// </summary>
typedef struct PFAuthenticationValidateEntityTokenRequest
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
    /// Client EntityToken.
    /// </summary>
    _Null_terminated_ const char* entityToken;

} PFAuthenticationValidateEntityTokenRequest;

/// <summary>
/// PFAuthenticationValidateEntityTokenResponse data model.
/// </summary>
typedef struct PFAuthenticationValidateEntityTokenResponse
{
    /// <summary>
    /// (Optional) The entity id and type.
    /// </summary>
    _Maybenull_ PFEntityKey const* entity;

    /// <summary>
    /// (Optional) The authenticated device for this entity, for the given login.
    /// </summary>
    _Maybenull_ PFAuthenticationIdentifiedDeviceType const* identifiedDeviceType;

    /// <summary>
    /// (Optional) The identity provider for this entity, for the given login.
    /// </summary>
    _Maybenull_ PFLoginIdentityProvider const* identityProvider;

    /// <summary>
    /// (Optional) The ID issued by the identity provider, e.g. a XUID on Xbox Live.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* identityProviderIssuedId;

    /// <summary>
    /// (Optional) The lineage of this profile.
    /// </summary>
    _Maybenull_ PFEntityLineage const* lineage;

} PFAuthenticationValidateEntityTokenResponse;

#pragma pop_macro("IN")

}
