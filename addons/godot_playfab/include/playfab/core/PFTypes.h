// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/core/PFPal.h>

extern "C"
{

#pragma push_macro("IN")
#undef IN

/// <summary>
/// TitleActivationStatus enum.
/// </summary>
enum class PFTitleActivationStatus : uint32_t
{
    None,
    ActivatedTitleKey,
    PendingSteam,
    ActivatedSteam,
    RevokedSteam
};

#ifdef HC_PLATFORM_IS_PLAYSTATION
#pragma push_macro("BSD")
#endif
#undef BSD 
/// <summary>
/// Currency enum.
/// </summary>
enum class PFCurrency : uint32_t
{
    AED,
    AFN,
    ALL,
    AMD,
    ANG,
    AOA,
    ARS,
    AUD,
    AWG,
    AZN,
    BAM,
    BBD,
    BDT,
    BGN,
    BHD,
    BIF,
    BMD,
    BND,
    BOB,
    BRL,
    BSD,
    BTN,
    BWP,
    BYR,
    BZD,
    CAD,
    CDF,
    CHF,
    CLP,
    CNY,
    COP,
    CRC,
    CUC,
    CUP,
    CVE,
    CZK,
    DJF,
    DKK,
    DOP,
    DZD,
    EGP,
    ERN,
    ETB,
    EUR,
    FJD,
    FKP,
    GBP,
    GEL,
    GGP,
    GHS,
    GIP,
    GMD,
    GNF,
    GTQ,
    GYD,
    HKD,
    HNL,
    HRK,
    HTG,
    HUF,
    IDR,
    ILS,
    IMP,
    INR,
    IQD,
    IRR,
    ISK,
    JEP,
    JMD,
    JOD,
    JPY,
    KES,
    KGS,
    KHR,
    KMF,
    KPW,
    KRW,
    KWD,
    KYD,
    KZT,
    LAK,
    LBP,
    LKR,
    LRD,
    LSL,
    LYD,
    MAD,
    MDL,
    MGA,
    MKD,
    MMK,
    MNT,
    MOP,
    MRO,
    MUR,
    MVR,
    MWK,
    MXN,
    MYR,
    MZN,
    NAD,
    NGN,
    NIO,
    NOK,
    NPR,
    NZD,
    OMR,
    PAB,
    PEN,
    PGK,
    PHP,
    PKR,
    PLN,
    PYG,
    QAR,
    RON,
    RSD,
    RUB,
    RWF,
    SAR,
    SBD,
    SCR,
    SDG,
    SEK,
    SGD,
    SHP,
    SLL,
    SOS,
    SPL,
    SRD,
    STD,
    SVC,
    SYP,
    SZL,
    THB,
    TJS,
    TMT,
    TND,
    TOP,
    TRY,
    TTD,
    TVD,
    TWD,
    TZS,
    UAH,
    UGX,
    USD,
    UYU,
    UZS,
    VEF,
    VND,
    VUV,
    WST,
    XAF,
    XCD,
    XDR,
    XOF,
    XPF,
    YER,
    ZAR,
    ZMW,
    ZWD
};
#ifdef HC_PLATFORM_IS_PLAYSTATION
#pragma pop_macro("BSD")
#endif

/// <summary>
/// UserOrigination enum.
/// </summary>
enum class PFUserOrigination : uint32_t
{
    Organic,
    Steam,
    Google,
    Amazon,
    Facebook,
    Kongregate,
    GamersFirst,
    Unknown,
    IOS,
    LoadTest,
    Android,
    PSN,
    GameCenter,
    CustomId,
    XboxLive,
    Parse,
    Twitch,
    ServerCustomId,
    NintendoSwitchDeviceId,
    FacebookInstantGamesId,
    OpenIdConnect,
    Apple,
    NintendoSwitchAccount,
    GooglePlayGames,
    XboxMobileStore,
    King,
    BattleNet
};

/// <summary>
/// UserDataPermission enum.
/// </summary>
enum class PFUserDataPermission : uint32_t
{
    Private,
    Public
};

/// <summary>
/// EmailVerificationStatus enum.
/// </summary>
enum class PFEmailVerificationStatus : uint32_t
{
    Unverified,
    Pending,
    Confirmed
};

/// <summary>
/// LoginIdentityProvider enum.
/// </summary>
enum class PFLoginIdentityProvider : uint32_t
{
    Unknown,
    PlayFab,
    Custom,
    GameCenter,
    GooglePlay,
    Steam,
    XBoxLive,
    PSN,
    Kongregate,
    Facebook,
    IOSDevice,
    AndroidDevice,
    Twitch,
    WindowsHello,
    GameServer,
    CustomServer,
    NintendoSwitch,
    FacebookInstantGames,
    OpenIdConnect,
    Apple,
    NintendoSwitchAccount,
    GooglePlayGames,
    XboxMobileStore,
    King,
    BattleNet
};

/// <summary>
/// ContinentCode enum.
/// </summary>
enum class PFContinentCode : uint32_t
{
    AF,
    AN,
    AS,
    EU,
    NA,
    OC,
    SA,
    Unknown
};

/// <summary>
/// CountryCode enum.
/// </summary>
enum class PFCountryCode : uint32_t
{
    AF,
    AX,
    AL,
    DZ,
    AS,
    AD,
    AO,
    AI,
    AQ,
    AG,
    AR,
    AM,
    AW,
    AU,
    AT,
    AZ,
    BS,
    BH,
    BD,
    BB,
    BY,
    BE,
    BZ,
    BJ,
    BM,
    BT,
    BO,
    BQ,
    BA,
    BW,
    BV,
    BR,
    IO,
    BN,
    BG,
    BF,
    BI,
    KH,
    CM,
    CA,
    CV,
    KY,
    CF,
    TD,
    CL,
    CN,
    CX,
    CC,
    CO,
    KM,
    CG,
    CD,
    CK,
    CR,
    CI,
    HR,
    CU,
    CW,
    CY,
    CZ,
    DK,
    DJ,
    DM,
    DO,
    EC,
    EG,
    SV,
    GQ,
    ER,
    EE,
    ET,
    FK,
    FO,
    FJ,
    FI,
    FR,
    GF,
    PF,
    TF,
    GA,
    GM,
    GE,
    DE,
    GH,
    GI,
    GR,
    GL,
    GD,
    GP,
    GU,
    GT,
    GG,
    GN,
    GW,
    GY,
    HT,
    HM,
    VA,
    HN,
    HK,
    HU,
    IS,
    IN,
    ID,
    IR,
    IQ,
    IE,
    IM,
    IL,
    IT,
    JM,
    JP,
    JE,
    JO,
    KZ,
    KE,
    KI,
    KP,
    KR,
    KW,
    KG,
    LA,
    LV,
    LB,
    LS,
    LR,
    LY,
    LI,
    LT,
    LU,
    MO,
    MK,
    MG,
    MW,
    MY,
    MV,
    ML,
    MT,
    MH,
    MQ,
    MR,
    MU,
    YT,
    MX,
    FM,
    MD,
    MC,
    MN,
    ME,
    MS,
    MA,
    MZ,
    MM,
    NA,
    NR,
    NP,
    NL,
    NC,
    NZ,
    NI,
    NE,
    NG,
    NU,
    NF,
    MP,
    NO,
    OM,
    PK,
    PW,
    PS,
    PA,
    PG,
    PY,
    PE,
    PH,
    PN,
    PL,
    PT,
    PR,
    QA,
    RE,
    RO,
    RU,
    RW,
    BL,
    SH,
    KN,
    LC,
    MF,
    PM,
    VC,
    WS,
    SM,
    ST,
    SA,
    SN,
    RS,
    SC,
    SL,
    SG,
    SX,
    SK,
    SI,
    SB,
    SO,
    ZA,
    GS,
    SS,
    ES,
    LK,
    SD,
    SR,
    SJ,
    SZ,
    SE,
    CH,
    SY,
    TW,
    TJ,
    TZ,
    TH,
    TL,
    TG,
    TK,
    TO,
    TT,
    TN,
    TR,
    TM,
    TC,
    TV,
    UG,
    UA,
    AE,
    GB,
    US,
    UM,
    UY,
    UZ,
    VU,
    VE,
    VN,
    VG,
    VI,
    WF,
    EH,
    YE,
    ZM,
    ZW,
    Unknown
};

/// <summary>
/// SubscriptionProviderStatus enum.
/// </summary>
enum class PFSubscriptionProviderStatus : uint32_t
{
    NoError,
    Cancelled,
    UnknownError,
    BillingError,
    ProductUnavailable,
    CustomerDidNotAcceptPriceChange,
    FreeTrial,
    PaymentPending
};

/// <summary>
/// PushNotificationPlatform enum.
/// </summary>
enum class PFPushNotificationPlatform : uint32_t
{
    ApplePushNotificationService,
    GoogleCloudMessaging
};

/// <summary>
/// A token returned when registering a callback to identify the registration. This token is later used 
/// to unregister the callback.
/// </summary>
typedef uint64_t PFRegistrationToken;

/// <summary>
/// String representation of a Json Object
/// </summary>
typedef struct PFJsonObject
{
    _Maybenull_ _Null_terminated_ const char* stringValue;
} PFJsonObject;

/// <summary>
/// PFItemInstance data model. A unique instance of an item in a user's inventory. Note, to retrieve
/// additional information for an item such as Tags, Description that are the same across all instances
/// of the item, a call to GetCatalogItems is required. The ItemID of can be matched to a catalog entry,
/// which contains the additional information. Also note that Custom Data is only set when the User's
/// specific instance has updated the CustomData via a call to UpdateUserInventoryItemCustomData. Other
/// fields such as UnitPrice and UnitCurrency are only set when the item was granted via a purchase.
/// </summary>
typedef struct PFItemInstance
{
    /// <summary>
    /// (Optional) Game specific comment associated with this instance when it was added to the user
    /// inventory.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* annotation;

    /// <summary>
    /// (Optional) Array of unique items that were awarded when this catalog item was purchased.
    /// </summary>
    _Maybenull_ _Field_size_(bundleContentsCount) const char* const* bundleContents;

    /// <summary>
    /// Count of bundleContents
    /// </summary>
    uint32_t bundleContentsCount;

    /// <summary>
    /// (Optional) Unique identifier for the parent inventory item, as defined in the catalog, for object
    /// which were added from a bundle or container.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* bundleParent;

    /// <summary>
    /// (Optional) Catalog version for the inventory item, when this instance was created.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* catalogVersion;

    /// <summary>
    /// (Optional) A set of custom key-value pairs on the instance of the inventory item, which is not
    /// to be confused with the catalog item's custom data.
    /// </summary>
    _Maybenull_ _Field_size_(customDataCount) struct PFStringDictionaryEntry const* customData;

    /// <summary>
    /// Count of customData
    /// </summary>
    uint32_t customDataCount;

    /// <summary>
    /// (Optional) CatalogItem.DisplayName at the time this item was purchased.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* displayName;

    /// <summary>
    /// (Optional) Timestamp for when this instance will expire.
    /// </summary>
    _Maybenull_ time_t const* expiration;

    /// <summary>
    /// (Optional) Class name for the inventory item, as defined in the catalog.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemClass;

    /// <summary>
    /// (Optional) Unique identifier for the inventory item, as defined in the catalog.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemId;

    /// <summary>
    /// (Optional) Unique item identifier for this specific instance of the item.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* itemInstanceId;

    /// <summary>
    /// (Optional) Timestamp for when this instance was purchased.
    /// </summary>
    _Maybenull_ time_t const* purchaseDate;

    /// <summary>
    /// (Optional) Total number of remaining uses, if this is a consumable item.
    /// </summary>
    _Maybenull_ int32_t const* remainingUses;

    /// <summary>
    /// (Optional) Currency type for the cost of the catalog item. Not available when granting items.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* unitCurrency;

    /// <summary>
    /// Cost of the catalog item in the given currency. Not available when granting items.
    /// </summary>
    uint32_t unitPrice;

    /// <summary>
    /// (Optional) The number of uses that were added or removed to this item in this call.
    /// </summary>
    _Maybenull_ int32_t const* usesIncrementedBy;

} PFItemInstance;

/// <summary>
/// PFUserAndroidDeviceInfo data model.
/// </summary>
typedef struct PFUserAndroidDeviceInfo
{
    /// <summary>
    /// (Optional) Android device ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* androidDeviceId;

} PFUserAndroidDeviceInfo;

/// <summary>
/// PFUserAppleIdInfo data model.
/// </summary>
typedef struct PFUserAppleIdInfo
{
    /// <summary>
    /// (Optional) Apple subject ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* appleSubjectId;

} PFUserAppleIdInfo;

/// <summary>
/// PFUserBattleNetInfo data model.
/// </summary>
typedef struct PFUserBattleNetInfo
{
    /// <summary>
    /// (Optional) Battle.net identifier.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* battleNetAccountId;

    /// <summary>
    /// (Optional) Battle.net display name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* battleNetBattleTag;

} PFUserBattleNetInfo;

/// <summary>
/// PFUserCustomIdInfo data model.
/// </summary>
typedef struct PFUserCustomIdInfo
{
    /// <summary>
    /// (Optional) Custom ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* customId;

} PFUserCustomIdInfo;

/// <summary>
/// PFUserFacebookInfo data model.
/// </summary>
typedef struct PFUserFacebookInfo
{
    /// <summary>
    /// (Optional) Facebook identifier.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* facebookId;

    /// <summary>
    /// (Optional) Facebook full name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* fullName;

} PFUserFacebookInfo;

/// <summary>
/// PFUserFacebookInstantGamesIdInfo data model.
/// </summary>
typedef struct PFUserFacebookInstantGamesIdInfo
{
    /// <summary>
    /// (Optional) Facebook Instant Games ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* facebookInstantGamesId;

} PFUserFacebookInstantGamesIdInfo;

/// <summary>
/// PFUserGameCenterInfo data model.
/// </summary>
typedef struct PFUserGameCenterInfo
{
    /// <summary>
    /// (Optional) Gamecenter identifier.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* gameCenterId;

} PFUserGameCenterInfo;

/// <summary>
/// PFUserGoogleInfo data model.
/// </summary>
typedef struct PFUserGoogleInfo
{
    /// <summary>
    /// (Optional) Email address of the Google account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googleEmail;

    /// <summary>
    /// (Optional) Gender information of the Google account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googleGender;

    /// <summary>
    /// (Optional) Google ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googleId;

    /// <summary>
    /// (Optional) Locale of the Google account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googleLocale;

    /// <summary>
    /// (Optional) Name of the Google account user.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googleName;

} PFUserGoogleInfo;

/// <summary>
/// PFUserGooglePlayGamesInfo data model.
/// </summary>
typedef struct PFUserGooglePlayGamesInfo
{
    /// <summary>
    /// (Optional) Avatar image url of the Google Play Games player.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googlePlayGamesPlayerAvatarImageUrl;

    /// <summary>
    /// (Optional) Display name of the Google Play Games player.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googlePlayGamesPlayerDisplayName;

    /// <summary>
    /// (Optional) Google Play Games player ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* googlePlayGamesPlayerId;

} PFUserGooglePlayGamesInfo;

/// <summary>
/// PFUserIosDeviceInfo data model.
/// </summary>
typedef struct PFUserIosDeviceInfo
{
    /// <summary>
    /// (Optional) IOS device ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* iosDeviceId;

} PFUserIosDeviceInfo;

/// <summary>
/// PFUserKongregateInfo data model.
/// </summary>
typedef struct PFUserKongregateInfo
{
    /// <summary>
    /// (Optional) Kongregate ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* kongregateId;

    /// <summary>
    /// (Optional) Kongregate Username.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* kongregateName;

} PFUserKongregateInfo;

/// <summary>
/// PFUserNintendoSwitchAccountIdInfo data model.
/// </summary>
typedef struct PFUserNintendoSwitchAccountIdInfo
{
    /// <summary>
    /// (Optional) Nintendo Switch account subject ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* nintendoSwitchAccountSubjectId;

} PFUserNintendoSwitchAccountIdInfo;

/// <summary>
/// PFUserNintendoSwitchDeviceIdInfo data model.
/// </summary>
typedef struct PFUserNintendoSwitchDeviceIdInfo
{
    /// <summary>
    /// (Optional) Nintendo Switch Device ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* nintendoSwitchDeviceId;

} PFUserNintendoSwitchDeviceIdInfo;

/// <summary>
/// PFUserOpenIdInfo data model.
/// </summary>
typedef struct PFUserOpenIdInfo
{
    /// <summary>
    /// (Optional) OpenID Connection ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* connectionId;

    /// <summary>
    /// (Optional) OpenID Issuer.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* issuer;

    /// <summary>
    /// (Optional) OpenID Subject.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* subject;

} PFUserOpenIdInfo;

/// <summary>
/// PFUserPrivateAccountInfo data model.
/// </summary>
typedef struct PFUserPrivateAccountInfo
{
    /// <summary>
    /// (Optional) User email address.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* email;

} PFUserPrivateAccountInfo;

/// <summary>
/// PFUserPsnInfo data model.
/// </summary>
typedef struct PFUserPsnInfo
{
    /// <summary>
    /// (Optional) PlayStation :tm: Network account ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* psnAccountId;

    /// <summary>
    /// (Optional) PlayStation :tm: Network online ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* psnOnlineId;

} PFUserPsnInfo;

/// <summary>
/// PFUserServerCustomIdInfo data model.
/// </summary>
typedef struct PFUserServerCustomIdInfo
{
    /// <summary>
    /// (Optional) Custom ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* customId;

} PFUserServerCustomIdInfo;

/// <summary>
/// PFUserSteamInfo data model.
/// </summary>
typedef struct PFUserSteamInfo
{
    /// <summary>
    /// (Optional) What stage of game ownership the user is listed as being in, from Steam.
    /// </summary>
    _Maybenull_ PFTitleActivationStatus const* steamActivationStatus;

    /// <summary>
    /// (Optional) The country in which the player resides, from Steam data.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* steamCountry;

    /// <summary>
    /// (Optional) Currency type set in the user Steam account.
    /// </summary>
    _Maybenull_ PFCurrency const* steamCurrency;

    /// <summary>
    /// (Optional) Steam identifier.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* steamId;

    /// <summary>
    /// (Optional) Steam display name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* steamName;

} PFUserSteamInfo;

#ifndef PF_ENTITY_KEY_DEFINED
#define PF_ENTITY_KEY_DEFINED
/// <summary>
/// PFEntityKey data model. Combined entity type and ID structure which uniquely identifies a single
/// entity.
/// </summary>
typedef struct PFEntityKey
{
    /// <summary>
    /// Unique ID of the entity.
    /// </summary>
    _Null_terminated_ const char* id;

    /// <summary>
    /// (Optional) Entity type. See https://docs.microsoft.com/gaming/playfab/features/data/entities/available-built-in-entity-types.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* type;

} PFEntityKey;
#endif // PF_ENTITY_KEY_DEFINED

/// <summary>
/// PFUserTitleInfo data model.
/// </summary>
typedef struct PFUserTitleInfo
{
    /// <summary>
    /// (Optional) URL to the player's avatar.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* avatarUrl;

    /// <summary>
    /// Timestamp indicating when the user was first associated with this game (this can differ significantly
    /// from when the user first registered with PlayFab).
    /// </summary>
    time_t created;

    /// <summary>
    /// (Optional) Name of the user, as it is displayed in-game.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* displayName;

    /// <summary>
    /// (Optional) Timestamp indicating when the user first signed into this game (this can differ from
    /// the Created timestamp, as other events, such as issuing a beta key to the user, can associate
    /// the title to the user).
    /// </summary>
    _Maybenull_ time_t const* firstLogin;

    /// <summary>
    /// (Optional) Boolean indicating whether or not the user is currently banned for a title.
    /// </summary>
    _Maybenull_ bool const* isBanned;

    /// <summary>
    /// (Optional) Timestamp for the last user login for this title.
    /// </summary>
    _Maybenull_ time_t const* lastLogin;

    /// <summary>
    /// (Optional) Source by which the user first joined the game, if known.
    /// </summary>
    _Maybenull_ PFUserOrigination const* origination;

    /// <summary>
    /// (Optional) Title player account entity for this user.
    /// </summary>
    _Maybenull_ PFEntityKey const* titlePlayerAccount;

} PFUserTitleInfo;

/// <summary>
/// PFUserTwitchInfo data model.
/// </summary>
typedef struct PFUserTwitchInfo
{
    /// <summary>
    /// (Optional) Twitch ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* twitchId;

    /// <summary>
    /// (Optional) Twitch Username.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* twitchUserName;

} PFUserTwitchInfo;

/// <summary>
/// PFUserXboxInfo data model.
/// </summary>
typedef struct PFUserXboxInfo
{
    /// <summary>
    /// (Optional) XBox user ID.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* xboxUserId;

    /// <summary>
    /// (Optional) XBox user sandbox.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* xboxUserSandbox;

} PFUserXboxInfo;

/// <summary>
/// PFUserAccountInfo data model.
/// </summary>
typedef struct PFUserAccountInfo
{
    /// <summary>
    /// (Optional) User Android device information, if an Android device has been linked.
    /// </summary>
    _Maybenull_ PFUserAndroidDeviceInfo const* androidDeviceInfo;

    /// <summary>
    /// (Optional) Sign in with Apple account information, if an Apple account has been linked.
    /// </summary>
    _Maybenull_ PFUserAppleIdInfo const* appleAccountInfo;

    /// <summary>
    /// (Optional) Battle.net account information, if a Battle.net account has been linked.
    /// </summary>
    _Maybenull_ PFUserBattleNetInfo const* battleNetAccountInfo;

    /// <summary>
    /// Timestamp indicating when the user account was created.
    /// </summary>
    time_t created;

    /// <summary>
    /// (Optional) Custom ID information, if a custom ID has been assigned.
    /// </summary>
    _Maybenull_ PFUserCustomIdInfo const* customIdInfo;

    /// <summary>
    /// (Optional) User Facebook information, if a Facebook account has been linked.
    /// </summary>
    _Maybenull_ PFUserFacebookInfo const* facebookInfo;

    /// <summary>
    /// (Optional) Facebook Instant Games account information, if a Facebook Instant Games account has
    /// been linked.
    /// </summary>
    _Maybenull_ PFUserFacebookInstantGamesIdInfo const* facebookInstantGamesIdInfo;

    /// <summary>
    /// (Optional) User Gamecenter information, if a Gamecenter account has been linked.
    /// </summary>
    _Maybenull_ PFUserGameCenterInfo const* gameCenterInfo;

    /// <summary>
    /// (Optional) User Google account information, if a Google account has been linked.
    /// </summary>
    _Maybenull_ PFUserGoogleInfo const* googleInfo;

    /// <summary>
    /// (Optional) User Google Play Games account information, if a Google Play Games account has been
    /// linked.
    /// </summary>
    _Maybenull_ PFUserGooglePlayGamesInfo const* googlePlayGamesInfo;

    /// <summary>
    /// (Optional) User iOS device information, if an iOS device has been linked.
    /// </summary>
    _Maybenull_ PFUserIosDeviceInfo const* iosDeviceInfo;

    /// <summary>
    /// (Optional) User Kongregate account information, if a Kongregate account has been linked.
    /// </summary>
    _Maybenull_ PFUserKongregateInfo const* kongregateInfo;

    /// <summary>
    /// (Optional) Nintendo Switch account information, if a Nintendo Switch account has been linked.
    /// </summary>
    _Maybenull_ PFUserNintendoSwitchAccountIdInfo const* nintendoSwitchAccountInfo;

    /// <summary>
    /// (Optional) Nintendo Switch device information, if a Nintendo Switch device has been linked.
    /// </summary>
    _Maybenull_ PFUserNintendoSwitchDeviceIdInfo const* nintendoSwitchDeviceIdInfo;

    /// <summary>
    /// (Optional) OpenID Connect information, if any OpenID Connect accounts have been linked.
    /// </summary>
    _Maybenull_ _Field_size_(openIdInfoCount) PFUserOpenIdInfo const* const* openIdInfo;

    /// <summary>
    /// Count of openIdInfo
    /// </summary>
    uint32_t openIdInfoCount;

    /// <summary>
    /// (Optional) Unique identifier for the user account.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playFabId;

    /// <summary>
    /// (Optional) Personal information for the user which is considered more sensitive.
    /// </summary>
    _Maybenull_ PFUserPrivateAccountInfo const* privateInfo;

    /// <summary>
    /// (Optional) User PlayStation :tm: Network account information, if a PlayStation :tm: Network account
    /// has been linked.
    /// </summary>
    _Maybenull_ PFUserPsnInfo const* psnInfo;

    /// <summary>
    /// (Optional) Server Custom ID information, if a server custom ID has been assigned.
    /// </summary>
    _Maybenull_ PFUserServerCustomIdInfo const* serverCustomIdInfo;

    /// <summary>
    /// (Optional) User Steam information, if a Steam account has been linked.
    /// </summary>
    _Maybenull_ PFUserSteamInfo const* steamInfo;

    /// <summary>
    /// (Optional) Title-specific information for the user account.
    /// </summary>
    _Maybenull_ PFUserTitleInfo const* titleInfo;

    /// <summary>
    /// (Optional) User Twitch account information, if a Twitch account has been linked.
    /// </summary>
    _Maybenull_ PFUserTwitchInfo const* twitchInfo;

    /// <summary>
    /// (Optional) User account name in the PlayFab service.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* username;

    /// <summary>
    /// (Optional) User XBox account information, if a XBox account has been linked.
    /// </summary>
    _Maybenull_ PFUserXboxInfo const* xboxInfo;

} PFUserAccountInfo;

/// <summary>
/// PFCharacterResult data model.
/// </summary>
typedef struct PFCharacterResult
{
    /// <summary>
    /// (Optional) The id for this character on this player.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* characterId;

    /// <summary>
    /// (Optional) The name of this character.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* characterName;

    /// <summary>
    /// (Optional) The type-string that was given to this character on creation.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* characterType;

} PFCharacterResult;

/// <summary>
/// PFUserDataRecord data model.
/// </summary>
typedef struct PFUserDataRecord
{
    /// <summary>
    /// Timestamp for when this data was last updated.
    /// </summary>
    time_t lastUpdated;

    /// <summary>
    /// (Optional) Indicates whether this data can be read by all users (public) or only the user (private).
    /// This is used for GetUserData requests being made by one player about another player.
    /// </summary>
    _Maybenull_ PFUserDataPermission const* permission;

    /// <summary>
    /// (Optional) Data stored for the specified user data key.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* value;

} PFUserDataRecord;

/// <summary>
/// PFVirtualCurrencyRechargeTime data model.
/// </summary>
typedef struct PFVirtualCurrencyRechargeTime
{
    /// <summary>
    /// Maximum value to which the regenerating currency will automatically increment. Note that it can
    /// exceed this value through use of the AddUserVirtualCurrency API call. However, it will not regenerate
    /// automatically until it has fallen below this value.
    /// </summary>
    int32_t rechargeMax;

    /// <summary>
    /// Server timestamp in UTC indicating the next time the virtual currency will be incremented.
    /// </summary>
    time_t rechargeTime;

    /// <summary>
    /// Time remaining (in seconds) before the next recharge increment of the virtual currency.
    /// </summary>
    int32_t secondsToRecharge;

} PFVirtualCurrencyRechargeTime;

/// <summary>
/// PFPlayerProfileViewConstraints data model.
/// </summary>
typedef struct PFPlayerProfileViewConstraints
{
    /// <summary>
    /// Whether to show player's avatar URL. Defaults to false.
    /// </summary>
    bool showAvatarUrl;

    /// <summary>
    /// Whether to show the banned until time. Defaults to false.
    /// </summary>
    bool showBannedUntil;

    /// <summary>
    /// Whether to show campaign attributions. Defaults to false.
    /// </summary>
    bool showCampaignAttributions;

    /// <summary>
    /// Whether to show contact email addresses. Defaults to false.
    /// </summary>
    bool showContactEmailAddresses;

    /// <summary>
    /// Whether to show the created date. Defaults to false.
    /// </summary>
    bool showCreated;

    /// <summary>
    /// Whether to show the display name. Defaults to false.
    /// </summary>
    bool showDisplayName;

    /// <summary>
    /// Whether to show player's experiment variants. Defaults to false.
    /// </summary>
    bool showExperimentVariants;

    /// <summary>
    /// Whether to show the last login time. Defaults to false.
    /// </summary>
    bool showLastLogin;

    /// <summary>
    /// Whether to show the linked accounts. Defaults to false.
    /// </summary>
    bool showLinkedAccounts;

    /// <summary>
    /// Whether to show player's locations. Defaults to false.
    /// </summary>
    bool showLocations;

    /// <summary>
    /// Whether to show player's membership information. Defaults to false.
    /// </summary>
    bool showMemberships;

    /// <summary>
    /// Whether to show origination. Defaults to false.
    /// </summary>
    bool showOrigination;

    /// <summary>
    /// Whether to show push notification registrations. Defaults to false.
    /// </summary>
    bool showPushNotificationRegistrations;

    /// <summary>
    /// Reserved for future development.
    /// </summary>
    bool showStatistics;

    /// <summary>
    /// Whether to show tags. Defaults to false.
    /// </summary>
    bool showTags;

    /// <summary>
    /// Whether to show the total value to date in usd. Defaults to false.
    /// </summary>
    bool showTotalValueToDateInUsd;

    /// <summary>
    /// Whether to show the values to date. Defaults to false.
    /// </summary>
    bool showValuesToDate;

} PFPlayerProfileViewConstraints;

/// <summary>
/// PFAdCampaignAttributionModel data model.
/// </summary>
typedef struct PFAdCampaignAttributionModel
{
    /// <summary>
    /// UTC time stamp of attribution.
    /// </summary>
    time_t attributedAt;

    /// <summary>
    /// (Optional) Attribution campaign identifier.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* campaignId;

    /// <summary>
    /// (Optional) Attribution network name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* platform;

} PFAdCampaignAttributionModel;

/// <summary>
/// PFContactEmailInfoModel data model.
/// </summary>
typedef struct PFContactEmailInfoModel
{
    /// <summary>
    /// (Optional) The email address.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* emailAddress;

    /// <summary>
    /// (Optional) The name of the email info data.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) The verification status of the email.
    /// </summary>
    _Maybenull_ PFEmailVerificationStatus const* verificationStatus;

} PFContactEmailInfoModel;

/// <summary>
/// PFLinkedPlatformAccountModel data model.
/// </summary>
typedef struct PFLinkedPlatformAccountModel
{
    /// <summary>
    /// (Optional) Linked account email of the user on the platform, if available.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* email;

    /// <summary>
    /// (Optional) Authentication platform.
    /// </summary>
    _Maybenull_ PFLoginIdentityProvider const* platform;

    /// <summary>
    /// (Optional) Unique account identifier of the user on the platform.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* platformUserId;

    /// <summary>
    /// (Optional) Linked account username of the user on the platform, if available.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* username;

} PFLinkedPlatformAccountModel;

/// <summary>
/// PFLocationModel data model.
/// </summary>
typedef struct PFLocationModel
{
    /// <summary>
    /// (Optional) City name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* city;

    /// <summary>
    /// (Optional) The two-character continent code for this location.
    /// </summary>
    _Maybenull_ PFContinentCode const* continentCode;

    /// <summary>
    /// (Optional) The two-character ISO 3166-1 country code for the country associated with the location.
    /// </summary>
    _Maybenull_ PFCountryCode const* countryCode;

    /// <summary>
    /// (Optional) Latitude coordinate of the geographic location.
    /// </summary>
    _Maybenull_ double const* latitude;

    /// <summary>
    /// (Optional) Longitude coordinate of the geographic location.
    /// </summary>
    _Maybenull_ double const* longitude;

} PFLocationModel;

/// <summary>
/// PFSubscriptionModel data model.
/// </summary>
typedef struct PFSubscriptionModel
{
    /// <summary>
    /// When this subscription expires.
    /// </summary>
    time_t expiration;

    /// <summary>
    /// The time the subscription was orignially purchased.
    /// </summary>
    time_t initialSubscriptionTime;

    /// <summary>
    /// Whether this subscription is currently active. That is, if Expiration > now.
    /// </summary>
    bool isActive;

    /// <summary>
    /// (Optional) The status of this subscription, according to the subscription provider.
    /// </summary>
    _Maybenull_ PFSubscriptionProviderStatus const* status;

    /// <summary>
    /// (Optional) The id for this subscription.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* subscriptionId;

    /// <summary>
    /// (Optional) The item id for this subscription from the primary catalog.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* subscriptionItemId;

    /// <summary>
    /// (Optional) The provider for this subscription. Apple or Google Play are supported today.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* subscriptionProvider;

} PFSubscriptionModel;

/// <summary>
/// PFMembershipModel data model.
/// </summary>
typedef struct PFMembershipModel
{
    /// <summary>
    /// Whether this membership is active. That is, whether the MembershipExpiration time has been reached.
    /// </summary>
    bool isActive;

    /// <summary>
    /// The time this membership expires.
    /// </summary>
    time_t membershipExpiration;

    /// <summary>
    /// (Optional) The id of the membership.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* membershipId;

    /// <summary>
    /// (Optional) Membership expirations can be explicitly overridden (via game manager or the admin
    /// api). If this membership has been overridden, this will be the new expiration time.
    /// </summary>
    _Maybenull_ time_t const* overrideExpiration;

    /// <summary>
    /// (Optional) The list of subscriptions that this player has for this membership.
    /// </summary>
    _Maybenull_ _Field_size_(subscriptionsCount) PFSubscriptionModel const* const* subscriptions;

    /// <summary>
    /// Count of subscriptions
    /// </summary>
    uint32_t subscriptionsCount;

} PFMembershipModel;

/// <summary>
/// PFPushNotificationRegistrationModel data model.
/// </summary>
typedef struct PFPushNotificationRegistrationModel
{
    /// <summary>
    /// (Optional) Notification configured endpoint.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* notificationEndpointARN;

    /// <summary>
    /// (Optional) Push notification platform.
    /// </summary>
    _Maybenull_ PFPushNotificationPlatform const* platform;

} PFPushNotificationRegistrationModel;

/// <summary>
/// PFStatisticModel data model.
/// </summary>
typedef struct PFStatisticModel
{
    /// <summary>
    /// (Optional) Statistic name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* name;

    /// <summary>
    /// Statistic value.
    /// </summary>
    int32_t value;

    /// <summary>
    /// Statistic version (0 if not a versioned statistic).
    /// </summary>
    int32_t version;

} PFStatisticModel;

/// <summary>
/// PFTagModel data model.
/// </summary>
typedef struct PFTagModel
{
    /// <summary>
    /// (Optional) Full value of the tag, including namespace.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* tagValue;

} PFTagModel;

/// <summary>
/// PFValueToDateModel data model.
/// </summary>
typedef struct PFValueToDateModel
{
    /// <summary>
    /// (Optional) ISO 4217 code of the currency used in the purchases.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* currency;

    /// <summary>
    /// Total value of the purchases in a whole number of 1/100 monetary units. For example, 999 indicates
    /// nine dollars and ninety-nine cents when Currency is 'USD').
    /// </summary>
    uint32_t totalValue;

    /// <summary>
    /// (Optional) Total value of the purchases in a string representation of decimal monetary units.
    /// For example, '9.99' indicates nine dollars and ninety-nine cents when Currency is 'USD'.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* totalValueAsDecimal;

} PFValueToDateModel;

/// <summary>
/// PFPlayerProfileModel data model.
/// </summary>
typedef struct PFPlayerProfileModel
{
    /// <summary>
    /// (Optional) List of advertising campaigns the player has been attributed to.
    /// </summary>
    _Maybenull_ _Field_size_(adCampaignAttributionsCount) PFAdCampaignAttributionModel const* const* adCampaignAttributions;

    /// <summary>
    /// Count of adCampaignAttributions
    /// </summary>
    uint32_t adCampaignAttributionsCount;

    /// <summary>
    /// (Optional) URL of the player's avatar image.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* avatarUrl;

    /// <summary>
    /// (Optional) If the player is currently banned, the UTC Date when the ban expires.
    /// </summary>
    _Maybenull_ time_t const* bannedUntil;

    /// <summary>
    /// (Optional) List of all contact email info associated with the player account.
    /// </summary>
    _Maybenull_ _Field_size_(contactEmailAddressesCount) PFContactEmailInfoModel const* const* contactEmailAddresses;

    /// <summary>
    /// Count of contactEmailAddresses
    /// </summary>
    uint32_t contactEmailAddressesCount;

    /// <summary>
    /// (Optional) Player record created.
    /// </summary>
    _Maybenull_ time_t const* created;

    /// <summary>
    /// (Optional) Player display name.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* displayName;

    /// <summary>
    /// (Optional) List of experiment variants for the player. Note that these variants are not guaranteed
    /// to be up-to-date when returned during login because the player profile is updated only after login.
    /// Instead, use the LoginResult.TreatmentAssignment property during login to get the correct variants
    /// and variables.
    /// </summary>
    _Maybenull_ _Field_size_(experimentVariantsCount) const char* const* experimentVariants;

    /// <summary>
    /// Count of experimentVariants
    /// </summary>
    uint32_t experimentVariantsCount;

    /// <summary>
    /// (Optional) UTC time when the player most recently logged in to the title.
    /// </summary>
    _Maybenull_ time_t const* lastLogin;

    /// <summary>
    /// (Optional) List of all authentication systems linked to this player account.
    /// </summary>
    _Maybenull_ _Field_size_(linkedAccountsCount) PFLinkedPlatformAccountModel const* const* linkedAccounts;

    /// <summary>
    /// Count of linkedAccounts
    /// </summary>
    uint32_t linkedAccountsCount;

    /// <summary>
    /// (Optional) List of geographic locations from which the player has logged in to the title.
    /// </summary>
    _Maybenull_ _Field_size_(locationsCount) PFLocationModel const* const* locations;

    /// <summary>
    /// Count of locations
    /// </summary>
    uint32_t locationsCount;

    /// <summary>
    /// (Optional) List of memberships for the player, along with whether are expired.
    /// </summary>
    _Maybenull_ _Field_size_(membershipsCount) PFMembershipModel const* const* memberships;

    /// <summary>
    /// Count of memberships
    /// </summary>
    uint32_t membershipsCount;

    /// <summary>
    /// (Optional) Player account origination.
    /// </summary>
    _Maybenull_ PFLoginIdentityProvider const* origination;

    /// <summary>
    /// (Optional) PlayFab player account unique identifier.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* playerId;

    /// <summary>
    /// (Optional) Publisher this player belongs to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* publisherId;

    /// <summary>
    /// (Optional) List of configured end points registered for sending the player push notifications.
    /// </summary>
    _Maybenull_ _Field_size_(pushNotificationRegistrationsCount) PFPushNotificationRegistrationModel const* const* pushNotificationRegistrations;

    /// <summary>
    /// Count of pushNotificationRegistrations
    /// </summary>
    uint32_t pushNotificationRegistrationsCount;

    /// <summary>
    /// (Optional) List of leaderboard statistic values for the player.
    /// </summary>
    _Maybenull_ _Field_size_(statisticsCount) PFStatisticModel const* const* statistics;

    /// <summary>
    /// Count of statistics
    /// </summary>
    uint32_t statisticsCount;

    /// <summary>
    /// (Optional) List of player's tags for segmentation.
    /// </summary>
    _Maybenull_ _Field_size_(tagsCount) PFTagModel const* const* tags;

    /// <summary>
    /// Count of tags
    /// </summary>
    uint32_t tagsCount;

    /// <summary>
    /// (Optional) Title ID this player profile applies to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* titleId;

    /// <summary>
    /// (Optional) Sum of the player's purchases made with real-money currencies, converted to US dollars
    /// equivalent and represented as a whole number of cents (1/100 USD). For example, 999 indicates
    /// nine dollars and ninety-nine cents.
    /// </summary>
    _Maybenull_ uint32_t const* totalValueToDateInUSD;

    /// <summary>
    /// (Optional) List of the player's lifetime purchase totals, summed by real-money currency.
    /// </summary>
    _Maybenull_ _Field_size_(valuesToDateCount) PFValueToDateModel const* const* valuesToDate;

    /// <summary>
    /// Count of valuesToDate
    /// </summary>
    uint32_t valuesToDateCount;

} PFPlayerProfileModel;

/// <summary>
/// PFGetPlayerCombinedInfoRequestParams data model.
/// </summary>
typedef struct PFGetPlayerCombinedInfoRequestParams
{
    /// <summary>
    /// Whether to get character inventories. Defaults to false.
    /// </summary>
    bool getCharacterInventories;

    /// <summary>
    /// Whether to get the list of characters. Defaults to false.
    /// </summary>
    bool getCharacterList;

    /// <summary>
    /// Whether to get player profile. Defaults to false. Has no effect for a new player.
    /// </summary>
    bool getPlayerProfile;

    /// <summary>
    /// Whether to get player statistics. Defaults to false.
    /// </summary>
    bool getPlayerStatistics;

    /// <summary>
    /// Whether to get title data. Defaults to false.
    /// </summary>
    bool getTitleData;

    /// <summary>
    /// Whether to get the player's account Info. Defaults to false.
    /// </summary>
    bool getUserAccountInfo;

    /// <summary>
    /// Whether to get the player's custom data. Defaults to false.
    /// </summary>
    bool getUserData;

    /// <summary>
    /// Whether to get the player's inventory. Defaults to false.
    /// </summary>
    bool getUserInventory;

    /// <summary>
    /// Whether to get the player's read only data. Defaults to false.
    /// </summary>
    bool getUserReadOnlyData;

    /// <summary>
    /// Whether to get the player's virtual currency balances. Defaults to false.
    /// </summary>
    bool getUserVirtualCurrency;

    /// <summary>
    /// (Optional) Specific statistics to retrieve. Leave null to get all keys. Has no effect if GetPlayerStatistics
    /// is false.
    /// </summary>
    _Maybenull_ _Field_size_(playerStatisticNamesCount) const char* const* playerStatisticNames;

    /// <summary>
    /// Count of playerStatisticNames
    /// </summary>
    uint32_t playerStatisticNamesCount;

    /// <summary>
    /// (Optional) Specifies the properties to return from the player profile. Defaults to returning
    /// the player's display name.
    /// </summary>
    _Maybenull_ PFPlayerProfileViewConstraints const* profileConstraints;

    /// <summary>
    /// (Optional) Specific keys to search for in the custom data. Leave null to get all keys. Has no
    /// effect if GetTitleData is false.
    /// </summary>
    _Maybenull_ _Field_size_(titleDataKeysCount) const char* const* titleDataKeys;

    /// <summary>
    /// Count of titleDataKeys
    /// </summary>
    uint32_t titleDataKeysCount;

    /// <summary>
    /// (Optional) Specific keys to search for in the custom data. Leave null to get all keys. Has no
    /// effect if GetUserData is false.
    /// </summary>
    _Maybenull_ _Field_size_(userDataKeysCount) const char* const* userDataKeys;

    /// <summary>
    /// Count of userDataKeys
    /// </summary>
    uint32_t userDataKeysCount;

    /// <summary>
    /// (Optional) Specific keys to search for in the custom data. Leave null to get all keys. Has no
    /// effect if GetUserReadOnlyData is false.
    /// </summary>
    _Maybenull_ _Field_size_(userReadOnlyDataKeysCount) const char* const* userReadOnlyDataKeys;

    /// <summary>
    /// Count of userReadOnlyDataKeys
    /// </summary>
    uint32_t userReadOnlyDataKeysCount;

} PFGetPlayerCombinedInfoRequestParams;

/// <summary>
/// PFCharacterInventory data model.
/// </summary>
typedef struct PFCharacterInventory
{
    /// <summary>
    /// (Optional) The id of this character.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* characterId;

    /// <summary>
    /// (Optional) The inventory of this character.
    /// </summary>
    _Maybenull_ _Field_size_(inventoryCount) PFItemInstance const* const* inventory;

    /// <summary>
    /// Count of inventory
    /// </summary>
    uint32_t inventoryCount;

} PFCharacterInventory;

/// <summary>
/// PFStatisticValue data model.
/// </summary>
typedef struct PFStatisticValue
{
    /// <summary>
    /// (Optional) Unique name of the statistic.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* statisticName;

    /// <summary>
    /// Statistic value for the player.
    /// </summary>
    int32_t value;

    /// <summary>
    /// For updates to an existing statistic value for a player, the version of the statistic when it
    /// was loaded.
    /// </summary>
    uint32_t version;

} PFStatisticValue;

/// <summary>
/// PFGetPlayerCombinedInfoResultPayload data model.
/// </summary>
typedef struct PFGetPlayerCombinedInfoResultPayload
{
    /// <summary>
    /// (Optional) Account information for the user. This is always retrieved.
    /// </summary>
    _Maybenull_ PFUserAccountInfo const* accountInfo;

    /// <summary>
    /// (Optional) Inventories for each character for the user.
    /// </summary>
    _Maybenull_ _Field_size_(characterInventoriesCount) PFCharacterInventory const* const* characterInventories;

    /// <summary>
    /// Count of characterInventories
    /// </summary>
    uint32_t characterInventoriesCount;

    /// <summary>
    /// (Optional) List of characters for the user.
    /// </summary>
    _Maybenull_ _Field_size_(characterListCount) PFCharacterResult const* const* characterList;

    /// <summary>
    /// Count of characterList
    /// </summary>
    uint32_t characterListCount;

    /// <summary>
    /// (Optional) The profile of the players. This profile is not guaranteed to be up-to-date. For a
    /// new player, this profile will not exist.
    /// </summary>
    _Maybenull_ PFPlayerProfileModel const* playerProfile;

    /// <summary>
    /// (Optional) List of statistics for this player.
    /// </summary>
    _Maybenull_ _Field_size_(playerStatisticsCount) PFStatisticValue const* const* playerStatistics;

    /// <summary>
    /// Count of playerStatistics
    /// </summary>
    uint32_t playerStatisticsCount;

    /// <summary>
    /// (Optional) Title data for this title.
    /// </summary>
    _Maybenull_ _Field_size_(titleDataCount) struct PFStringDictionaryEntry const* titleData;

    /// <summary>
    /// Count of titleData
    /// </summary>
    uint32_t titleDataCount;

    /// <summary>
    /// (Optional) User specific custom data.
    /// </summary>
    _Maybenull_ _Field_size_(userDataCount) struct PFUserDataRecordDictionaryEntry const* userData;

    /// <summary>
    /// Count of userData
    /// </summary>
    uint32_t userDataCount;

    /// <summary>
    /// The version of the UserData that was returned.
    /// </summary>
    uint32_t userDataVersion;

    /// <summary>
    /// (Optional) Array of inventory items in the user's current inventory.
    /// </summary>
    _Maybenull_ _Field_size_(userInventoryCount) PFItemInstance const* const* userInventory;

    /// <summary>
    /// Count of userInventory
    /// </summary>
    uint32_t userInventoryCount;

    /// <summary>
    /// (Optional) User specific read-only data.
    /// </summary>
    _Maybenull_ _Field_size_(userReadOnlyDataCount) struct PFUserDataRecordDictionaryEntry const* userReadOnlyData;

    /// <summary>
    /// Count of userReadOnlyData
    /// </summary>
    uint32_t userReadOnlyDataCount;

    /// <summary>
    /// The version of the Read-Only UserData that was returned.
    /// </summary>
    uint32_t userReadOnlyDataVersion;

    /// <summary>
    /// (Optional) Dictionary of virtual currency balance(s) belonging to the user.
    /// </summary>
    _Maybenull_ _Field_size_(userVirtualCurrencyCount) struct PFInt32DictionaryEntry const* userVirtualCurrency;

    /// <summary>
    /// Count of userVirtualCurrency
    /// </summary>
    uint32_t userVirtualCurrencyCount;

    /// <summary>
    /// (Optional) Dictionary of remaining times and timestamps for virtual currencies.
    /// </summary>
    _Maybenull_ _Field_size_(userVirtualCurrencyRechargeTimesCount) struct PFVirtualCurrencyRechargeTimeDictionaryEntry const* userVirtualCurrencyRechargeTimes;

    /// <summary>
    /// Count of userVirtualCurrencyRechargeTimes
    /// </summary>
    uint32_t userVirtualCurrencyRechargeTimesCount;

} PFGetPlayerCombinedInfoResultPayload;

/// <summary>
/// PFVariable data model.
/// </summary>
typedef struct PFVariable
{
    /// <summary>
    /// Name of the variable.
    /// </summary>
    _Null_terminated_ const char* name;

    /// <summary>
    /// (Optional) Value of the variable.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* value;

} PFVariable;

/// <summary>
/// PFTreatmentAssignment data model.
/// </summary>
typedef struct PFTreatmentAssignment
{
    /// <summary>
    /// (Optional) List of the experiment variables.
    /// </summary>
    _Maybenull_ _Field_size_(variablesCount) PFVariable const* const* variables;

    /// <summary>
    /// Count of variables
    /// </summary>
    uint32_t variablesCount;

    /// <summary>
    /// (Optional) List of the experiment variants.
    /// </summary>
    _Maybenull_ _Field_size_(variantsCount) const char* const* variants;

    /// <summary>
    /// Count of variants
    /// </summary>
    uint32_t variantsCount;

} PFTreatmentAssignment;

/// <summary>
/// PFEntityLineage data model.
/// </summary>
typedef struct PFEntityLineage
{
    /// <summary>
    /// (Optional) The Character Id of the associated entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* characterId;

    /// <summary>
    /// (Optional) The Group Id of the associated entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* groupId;

    /// <summary>
    /// (Optional) The Master Player Account Id of the associated entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* masterPlayerAccountId;

    /// <summary>
    /// (Optional) The Namespace Id of the associated entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* namespaceId;

    /// <summary>
    /// (Optional) The Title Id of the associated entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* titleId;

    /// <summary>
    /// (Optional) The Title Player Account Id of the associated entity.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* titlePlayerAccountId;

} PFEntityLineage;

/// <summary>
/// Dictionary entry for an associative array with string values.
/// </summary>
typedef struct PFStringDictionaryEntry
{
    _Null_terminated_ const char* key;
    _Null_terminated_ const char* value;
} PFStringDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with int32_t values.
/// </summary>
typedef struct PFInt32DictionaryEntry
{
    _Null_terminated_ const char* key;
    int32_t value;
} PFInt32DictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with uint32_t values.
/// </summary>
typedef struct PFUint32DictionaryEntry
{
    _Null_terminated_ const char* key;
    uint32_t value;
} PFUint32DictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with time_t values.
/// </summary>
typedef struct PFDateTimeDictionaryEntry
{
    _Null_terminated_ const char* key;
    time_t value;
} PFDateTimeDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFUserDataRecord values.
/// </summary>
typedef struct PFUserDataRecordDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFUserDataRecord const* value;
} PFUserDataRecordDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFVirtualCurrencyRechargeTime values.
/// </summary>
typedef struct PFVirtualCurrencyRechargeTimeDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFVirtualCurrencyRechargeTime const* value;
} PFVirtualCurrencyRechargeTimeDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFEntityKey values.
/// </summary>
typedef struct PFEntityKeyDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFEntityKey const* value;
} PFEntityKeyDictionaryEntry;

/// <summary>
/// Dictionary entry for an associative array with PFEntityLineage values.
/// </summary>
typedef struct PFEntityLineageDictionaryEntry
{
    _Null_terminated_ const char* key;
    PFEntityLineage const* value;
} PFEntityLineageDictionaryEntry;

#pragma pop_macro("IN")

}
