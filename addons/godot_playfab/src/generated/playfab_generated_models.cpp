// GENERATED FILE - DO NOT EDIT BY HAND.
#include "generated/playfab_generated_models.h"

namespace godot { namespace playfab_generated {

Variant to_variant_PFStringDictionaryEntry(const PFStringDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = p_value->value != nullptr ? String::utf8(p_value->value) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementAddOrUpdateContactEmailRequest(const PFAccountManagementAddOrUpdateContactEmailRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["email_address"] = p_value->emailAddress != nullptr ? String::utf8(p_value->emailAddress) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementBattleNetAccountPlayFabIdPair(const PFAccountManagementBattleNetAccountPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["battle_net_account_id"] = p_value->battleNetAccountId != nullptr ? String::utf8(p_value->battleNetAccountId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementClientLinkBattleNetAccountRequest(const PFAccountManagementClientLinkBattleNetAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    dictionary["identity_token"] = p_value->identityToken != nullptr ? String::utf8(p_value->identityToken) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementClientLinkNintendoServiceAccountRequest(const PFAccountManagementClientLinkNintendoServiceAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    dictionary["identity_token"] = p_value->identityToken != nullptr ? String::utf8(p_value->identityToken) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementClientLinkPSNAccountRequest(const PFAccountManagementClientLinkPSNAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["auth_code"] = p_value->authCode != nullptr ? String::utf8(p_value->authCode) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    if (p_value->issuerId != nullptr) dictionary["issuer_id"] = static_cast<int64_t>(*p_value->issuerId);
    dictionary["redirect_uri"] = p_value->redirectUri != nullptr ? String::utf8(p_value->redirectUri) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementClientLinkXboxAccountRequest(const PFAccountManagementClientLinkXboxAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    dictionary["user"] = Variant();
    return dictionary;
}

Variant to_variant_PFAccountManagementClientUnlinkBattleNetAccountRequest(const PFAccountManagementClientUnlinkBattleNetAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementClientUnlinkNintendoServiceAccountRequest(const PFAccountManagementClientUnlinkNintendoServiceAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementClientUnlinkPSNAccountRequest(const PFAccountManagementClientUnlinkPSNAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementClientUnlinkXboxAccountRequest(const PFAccountManagementClientUnlinkXboxAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementClientUpdateAvatarUrlRequest(const PFAccountManagementClientUpdateAvatarUrlRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["image_url"] = p_value->imageUrl != nullptr ? String::utf8(p_value->imageUrl) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementFacebookInstantGamesPlayFabIdPair(const PFAccountManagementFacebookInstantGamesPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["facebook_instant_games_id"] = p_value->facebookInstantGamesId != nullptr ? String::utf8(p_value->facebookInstantGamesId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementFacebookPlayFabIdPair(const PFAccountManagementFacebookPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["facebook_id"] = p_value->facebookId != nullptr ? String::utf8(p_value->facebookId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGameCenterPlayFabIdPair(const PFAccountManagementGameCenterPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["game_center_id"] = p_value->gameCenterId != nullptr ? String::utf8(p_value->gameCenterId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetAccountInfoRequest(const PFAccountManagementGetAccountInfoRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["email"] = p_value->email != nullptr ? String::utf8(p_value->email) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["title_display_name"] = p_value->titleDisplayName != nullptr ? String::utf8(p_value->titleDisplayName) : String();
    dictionary["username"] = p_value->username != nullptr ? String::utf8(p_value->username) : String();
    return dictionary;
}

Variant to_variant_PFUserAndroidDeviceInfo(const PFUserAndroidDeviceInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["android_device_id"] = p_value->androidDeviceId != nullptr ? String::utf8(p_value->androidDeviceId) : String();
    return dictionary;
}

Variant to_variant_PFUserAppleIdInfo(const PFUserAppleIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["apple_subject_id"] = p_value->appleSubjectId != nullptr ? String::utf8(p_value->appleSubjectId) : String();
    return dictionary;
}

Variant to_variant_PFUserBattleNetInfo(const PFUserBattleNetInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["battle_net_account_id"] = p_value->battleNetAccountId != nullptr ? String::utf8(p_value->battleNetAccountId) : String();
    dictionary["battle_net_battle_tag"] = p_value->battleNetBattleTag != nullptr ? String::utf8(p_value->battleNetBattleTag) : String();
    return dictionary;
}

Variant to_variant_PFUserCustomIdInfo(const PFUserCustomIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["custom_id"] = p_value->customId != nullptr ? String::utf8(p_value->customId) : String();
    return dictionary;
}

Variant to_variant_PFUserFacebookInfo(const PFUserFacebookInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["facebook_id"] = p_value->facebookId != nullptr ? String::utf8(p_value->facebookId) : String();
    dictionary["full_name"] = p_value->fullName != nullptr ? String::utf8(p_value->fullName) : String();
    return dictionary;
}

Variant to_variant_PFUserFacebookInstantGamesIdInfo(const PFUserFacebookInstantGamesIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["facebook_instant_games_id"] = p_value->facebookInstantGamesId != nullptr ? String::utf8(p_value->facebookInstantGamesId) : String();
    return dictionary;
}

Variant to_variant_PFUserGameCenterInfo(const PFUserGameCenterInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["game_center_id"] = p_value->gameCenterId != nullptr ? String::utf8(p_value->gameCenterId) : String();
    return dictionary;
}

Variant to_variant_PFUserGoogleInfo(const PFUserGoogleInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["google_email"] = p_value->googleEmail != nullptr ? String::utf8(p_value->googleEmail) : String();
    dictionary["google_gender"] = p_value->googleGender != nullptr ? String::utf8(p_value->googleGender) : String();
    dictionary["google_id"] = p_value->googleId != nullptr ? String::utf8(p_value->googleId) : String();
    dictionary["google_locale"] = p_value->googleLocale != nullptr ? String::utf8(p_value->googleLocale) : String();
    dictionary["google_name"] = p_value->googleName != nullptr ? String::utf8(p_value->googleName) : String();
    return dictionary;
}

Variant to_variant_PFUserGooglePlayGamesInfo(const PFUserGooglePlayGamesInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["google_play_games_player_avatar_image_url"] = p_value->googlePlayGamesPlayerAvatarImageUrl != nullptr ? String::utf8(p_value->googlePlayGamesPlayerAvatarImageUrl) : String();
    dictionary["google_play_games_player_display_name"] = p_value->googlePlayGamesPlayerDisplayName != nullptr ? String::utf8(p_value->googlePlayGamesPlayerDisplayName) : String();
    dictionary["google_play_games_player_id"] = p_value->googlePlayGamesPlayerId != nullptr ? String::utf8(p_value->googlePlayGamesPlayerId) : String();
    return dictionary;
}

Variant to_variant_PFUserIosDeviceInfo(const PFUserIosDeviceInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["ios_device_id"] = p_value->iosDeviceId != nullptr ? String::utf8(p_value->iosDeviceId) : String();
    return dictionary;
}

Variant to_variant_PFUserKongregateInfo(const PFUserKongregateInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["kongregate_id"] = p_value->kongregateId != nullptr ? String::utf8(p_value->kongregateId) : String();
    dictionary["kongregate_name"] = p_value->kongregateName != nullptr ? String::utf8(p_value->kongregateName) : String();
    return dictionary;
}

Variant to_variant_PFUserNintendoSwitchAccountIdInfo(const PFUserNintendoSwitchAccountIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["nintendo_switch_account_subject_id"] = p_value->nintendoSwitchAccountSubjectId != nullptr ? String::utf8(p_value->nintendoSwitchAccountSubjectId) : String();
    return dictionary;
}

Variant to_variant_PFUserNintendoSwitchDeviceIdInfo(const PFUserNintendoSwitchDeviceIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["nintendo_switch_device_id"] = p_value->nintendoSwitchDeviceId != nullptr ? String::utf8(p_value->nintendoSwitchDeviceId) : String();
    return dictionary;
}

Variant to_variant_PFUserOpenIdInfo(const PFUserOpenIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["connection_id"] = p_value->connectionId != nullptr ? String::utf8(p_value->connectionId) : String();
    dictionary["issuer"] = p_value->issuer != nullptr ? String::utf8(p_value->issuer) : String();
    dictionary["subject"] = p_value->subject != nullptr ? String::utf8(p_value->subject) : String();
    return dictionary;
}

Variant to_variant_PFUserPrivateAccountInfo(const PFUserPrivateAccountInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["email"] = p_value->email != nullptr ? String::utf8(p_value->email) : String();
    return dictionary;
}

Variant to_variant_PFUserPsnInfo(const PFUserPsnInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["psn_account_id"] = p_value->psnAccountId != nullptr ? String::utf8(p_value->psnAccountId) : String();
    dictionary["psn_online_id"] = p_value->psnOnlineId != nullptr ? String::utf8(p_value->psnOnlineId) : String();
    return dictionary;
}

Variant to_variant_PFUserServerCustomIdInfo(const PFUserServerCustomIdInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["custom_id"] = p_value->customId != nullptr ? String::utf8(p_value->customId) : String();
    return dictionary;
}

Variant to_variant_PFUserSteamInfo(const PFUserSteamInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->steamActivationStatus != nullptr) dictionary["steam_activation_status"] = static_cast<int64_t>(*p_value->steamActivationStatus);
    dictionary["steam_country"] = p_value->steamCountry != nullptr ? String::utf8(p_value->steamCountry) : String();
    if (p_value->steamCurrency != nullptr) dictionary["steam_currency"] = static_cast<int64_t>(*p_value->steamCurrency);
    dictionary["steam_id"] = p_value->steamId != nullptr ? String::utf8(p_value->steamId) : String();
    dictionary["steam_name"] = p_value->steamName != nullptr ? String::utf8(p_value->steamName) : String();
    return dictionary;
}

Variant to_variant_PFEntityKey(const PFEntityKey *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    return dictionary;
}

Variant to_variant_PFUserTitleInfo(const PFUserTitleInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["avatar_url"] = p_value->avatarUrl != nullptr ? String::utf8(p_value->avatarUrl) : String();
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    if (p_value->firstLogin != nullptr) dictionary["first_login"] = static_cast<int64_t>(*p_value->firstLogin);
    if (p_value->isBanned != nullptr) dictionary["is_banned"] = static_cast<bool>(*p_value->isBanned);
    if (p_value->lastLogin != nullptr) dictionary["last_login"] = static_cast<int64_t>(*p_value->lastLogin);
    if (p_value->origination != nullptr) dictionary["origination"] = static_cast<int64_t>(*p_value->origination);
    dictionary["title_player_account"] = to_variant_PFEntityKey(p_value->titlePlayerAccount);
    return dictionary;
}

Variant to_variant_PFUserTwitchInfo(const PFUserTwitchInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["twitch_id"] = p_value->twitchId != nullptr ? String::utf8(p_value->twitchId) : String();
    dictionary["twitch_user_name"] = p_value->twitchUserName != nullptr ? String::utf8(p_value->twitchUserName) : String();
    return dictionary;
}

Variant to_variant_PFUserXboxInfo(const PFUserXboxInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["xbox_user_id"] = p_value->xboxUserId != nullptr ? String::utf8(p_value->xboxUserId) : String();
    dictionary["xbox_user_sandbox"] = p_value->xboxUserSandbox != nullptr ? String::utf8(p_value->xboxUserSandbox) : String();
    return dictionary;
}

Variant to_variant_PFUserAccountInfo(const PFUserAccountInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["android_device_info"] = to_variant_PFUserAndroidDeviceInfo(p_value->androidDeviceInfo);
    dictionary["apple_account_info"] = to_variant_PFUserAppleIdInfo(p_value->appleAccountInfo);
    dictionary["battle_net_account_info"] = to_variant_PFUserBattleNetInfo(p_value->battleNetAccountInfo);
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["custom_id_info"] = to_variant_PFUserCustomIdInfo(p_value->customIdInfo);
    dictionary["facebook_info"] = to_variant_PFUserFacebookInfo(p_value->facebookInfo);
    dictionary["facebook_instant_games_id_info"] = to_variant_PFUserFacebookInstantGamesIdInfo(p_value->facebookInstantGamesIdInfo);
    dictionary["game_center_info"] = to_variant_PFUserGameCenterInfo(p_value->gameCenterInfo);
    dictionary["google_info"] = to_variant_PFUserGoogleInfo(p_value->googleInfo);
    dictionary["google_play_games_info"] = to_variant_PFUserGooglePlayGamesInfo(p_value->googlePlayGamesInfo);
    dictionary["ios_device_info"] = to_variant_PFUserIosDeviceInfo(p_value->iosDeviceInfo);
    dictionary["kongregate_info"] = to_variant_PFUserKongregateInfo(p_value->kongregateInfo);
    dictionary["nintendo_switch_account_info"] = to_variant_PFUserNintendoSwitchAccountIdInfo(p_value->nintendoSwitchAccountInfo);
    dictionary["nintendo_switch_device_id_info"] = to_variant_PFUserNintendoSwitchDeviceIdInfo(p_value->nintendoSwitchDeviceIdInfo);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->openIdInfoCount; ++i) {
            values.push_back(to_variant_PFUserOpenIdInfo(p_value->openIdInfo != nullptr ? p_value->openIdInfo[i] : nullptr));
        }
        dictionary["open_id_info"] = values;
    }
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["private_info"] = to_variant_PFUserPrivateAccountInfo(p_value->privateInfo);
    dictionary["psn_info"] = to_variant_PFUserPsnInfo(p_value->psnInfo);
    dictionary["server_custom_id_info"] = to_variant_PFUserServerCustomIdInfo(p_value->serverCustomIdInfo);
    dictionary["steam_info"] = to_variant_PFUserSteamInfo(p_value->steamInfo);
    dictionary["title_info"] = to_variant_PFUserTitleInfo(p_value->titleInfo);
    dictionary["twitch_info"] = to_variant_PFUserTwitchInfo(p_value->twitchInfo);
    dictionary["username"] = p_value->username != nullptr ? String::utf8(p_value->username) : String();
    dictionary["xbox_info"] = to_variant_PFUserXboxInfo(p_value->xboxInfo);
    return dictionary;
}

Variant to_variant_PFAccountManagementGetAccountInfoResult(const PFAccountManagementGetAccountInfoResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["account_info"] = to_variant_PFUserAccountInfo(p_value->accountInfo);
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsRequest(const PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->battleNetAccountIdsCount; ++i) {
            values.push_back(p_value->battleNetAccountIds != nullptr && p_value->battleNetAccountIds[i] != nullptr ? String::utf8(p_value->battleNetAccountIds[i]) : String());
        }
        dictionary["battle_net_account_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsResult(const PFAccountManagementGetPlayFabIDsFromBattleNetAccountIdsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementBattleNetAccountPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromFacebookIDsRequest(const PFAccountManagementGetPlayFabIDsFromFacebookIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->facebookIDsCount; ++i) {
            values.push_back(p_value->facebookIDs != nullptr && p_value->facebookIDs[i] != nullptr ? String::utf8(p_value->facebookIDs[i]) : String());
        }
        dictionary["facebook_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromFacebookIDsResult(const PFAccountManagementGetPlayFabIDsFromFacebookIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementFacebookPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromFacebookInstantGamesIdsRequest(const PFAccountManagementGetPlayFabIDsFromFacebookInstantGamesIdsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->facebookInstantGamesIdsCount; ++i) {
            values.push_back(p_value->facebookInstantGamesIds != nullptr && p_value->facebookInstantGamesIds[i] != nullptr ? String::utf8(p_value->facebookInstantGamesIds[i]) : String());
        }
        dictionary["facebook_instant_games_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromFacebookInstantGamesIdsResult(const PFAccountManagementGetPlayFabIDsFromFacebookInstantGamesIdsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementFacebookInstantGamesPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromGameCenterIDsRequest(const PFAccountManagementGetPlayFabIDsFromGameCenterIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->gameCenterIDsCount; ++i) {
            values.push_back(p_value->gameCenterIDs != nullptr && p_value->gameCenterIDs[i] != nullptr ? String::utf8(p_value->gameCenterIDs[i]) : String());
        }
        dictionary["game_center_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromGameCenterIDsResult(const PFAccountManagementGetPlayFabIDsFromGameCenterIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementGameCenterPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromGoogleIDsRequest(const PFAccountManagementGetPlayFabIDsFromGoogleIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->googleIDsCount; ++i) {
            values.push_back(p_value->googleIDs != nullptr && p_value->googleIDs[i] != nullptr ? String::utf8(p_value->googleIDs[i]) : String());
        }
        dictionary["google_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGooglePlayFabIdPair(const PFAccountManagementGooglePlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["google_id"] = p_value->googleId != nullptr ? String::utf8(p_value->googleId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromGoogleIDsResult(const PFAccountManagementGetPlayFabIDsFromGoogleIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementGooglePlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromGooglePlayGamesPlayerIDsRequest(const PFAccountManagementGetPlayFabIDsFromGooglePlayGamesPlayerIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->googlePlayGamesPlayerIDsCount; ++i) {
            values.push_back(p_value->googlePlayGamesPlayerIDs != nullptr && p_value->googlePlayGamesPlayerIDs[i] != nullptr ? String::utf8(p_value->googlePlayGamesPlayerIDs[i]) : String());
        }
        dictionary["google_play_games_player_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGooglePlayGamesPlayFabIdPair(const PFAccountManagementGooglePlayGamesPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["google_play_games_player_id"] = p_value->googlePlayGamesPlayerId != nullptr ? String::utf8(p_value->googlePlayGamesPlayerId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromGooglePlayGamesPlayerIDsResult(const PFAccountManagementGetPlayFabIDsFromGooglePlayGamesPlayerIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementGooglePlayGamesPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromKongregateIDsRequest(const PFAccountManagementGetPlayFabIDsFromKongregateIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->kongregateIDsCount; ++i) {
            values.push_back(p_value->kongregateIDs != nullptr && p_value->kongregateIDs[i] != nullptr ? String::utf8(p_value->kongregateIDs[i]) : String());
        }
        dictionary["kongregate_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementKongregatePlayFabIdPair(const PFAccountManagementKongregatePlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["kongregate_id"] = p_value->kongregateId != nullptr ? String::utf8(p_value->kongregateId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromKongregateIDsResult(const PFAccountManagementGetPlayFabIDsFromKongregateIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementKongregatePlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromNintendoServiceAccountIdsRequest(const PFAccountManagementGetPlayFabIDsFromNintendoServiceAccountIdsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->nintendoAccountIdsCount; ++i) {
            values.push_back(p_value->nintendoAccountIds != nullptr && p_value->nintendoAccountIds[i] != nullptr ? String::utf8(p_value->nintendoAccountIds[i]) : String());
        }
        dictionary["nintendo_account_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementNintendoServiceAccountPlayFabIdPair(const PFAccountManagementNintendoServiceAccountPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["nintendo_service_account_id"] = p_value->nintendoServiceAccountId != nullptr ? String::utf8(p_value->nintendoServiceAccountId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromNintendoServiceAccountIdsResult(const PFAccountManagementGetPlayFabIDsFromNintendoServiceAccountIdsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementNintendoServiceAccountPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromNintendoSwitchDeviceIdsRequest(const PFAccountManagementGetPlayFabIDsFromNintendoSwitchDeviceIdsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->nintendoSwitchDeviceIdsCount; ++i) {
            values.push_back(p_value->nintendoSwitchDeviceIds != nullptr && p_value->nintendoSwitchDeviceIds[i] != nullptr ? String::utf8(p_value->nintendoSwitchDeviceIds[i]) : String());
        }
        dictionary["nintendo_switch_device_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementNintendoSwitchPlayFabIdPair(const PFAccountManagementNintendoSwitchPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["nintendo_switch_device_id"] = p_value->nintendoSwitchDeviceId != nullptr ? String::utf8(p_value->nintendoSwitchDeviceId) : String();
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromNintendoSwitchDeviceIdsResult(const PFAccountManagementGetPlayFabIDsFromNintendoSwitchDeviceIdsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementNintendoSwitchPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromPSNAccountIDsRequest(const PFAccountManagementGetPlayFabIDsFromPSNAccountIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->issuerId != nullptr) dictionary["issuer_id"] = static_cast<int64_t>(*p_value->issuerId);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->PSNAccountIDsCount; ++i) {
            values.push_back(p_value->PSNAccountIDs != nullptr && p_value->PSNAccountIDs[i] != nullptr ? String::utf8(p_value->PSNAccountIDs[i]) : String());
        }
        dictionary["psn_account_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementPSNAccountPlayFabIdPair(const PFAccountManagementPSNAccountPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["psn_account_id"] = p_value->PSNAccountId != nullptr ? String::utf8(p_value->PSNAccountId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromPSNAccountIDsResult(const PFAccountManagementGetPlayFabIDsFromPSNAccountIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementPSNAccountPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromPSNOnlineIDsRequest(const PFAccountManagementGetPlayFabIDsFromPSNOnlineIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->issuerId != nullptr) dictionary["issuer_id"] = static_cast<int64_t>(*p_value->issuerId);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->PSNOnlineIDsCount; ++i) {
            values.push_back(p_value->PSNOnlineIDs != nullptr && p_value->PSNOnlineIDs[i] != nullptr ? String::utf8(p_value->PSNOnlineIDs[i]) : String());
        }
        dictionary["psn_online_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementPSNOnlinePlayFabIdPair(const PFAccountManagementPSNOnlinePlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["psn_online_id"] = p_value->PSNOnlineId != nullptr ? String::utf8(p_value->PSNOnlineId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromPSNOnlineIDsResult(const PFAccountManagementGetPlayFabIDsFromPSNOnlineIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementPSNOnlinePlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromSteamIDsRequest(const PFAccountManagementGetPlayFabIDsFromSteamIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->steamStringIDsCount; ++i) {
            values.push_back(p_value->steamStringIDs != nullptr && p_value->steamStringIDs[i] != nullptr ? String::utf8(p_value->steamStringIDs[i]) : String());
        }
        dictionary["steam_string_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementSteamPlayFabIdPair(const PFAccountManagementSteamPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["steam_string_id"] = p_value->steamStringId != nullptr ? String::utf8(p_value->steamStringId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromSteamIDsResult(const PFAccountManagementGetPlayFabIDsFromSteamIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementSteamPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromSteamNamesRequest(const PFAccountManagementGetPlayFabIDsFromSteamNamesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->steamNamesCount; ++i) {
            values.push_back(p_value->steamNames != nullptr && p_value->steamNames[i] != nullptr ? String::utf8(p_value->steamNames[i]) : String());
        }
        dictionary["steam_names"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementSteamNamePlayFabIdPair(const PFAccountManagementSteamNamePlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["steam_name"] = p_value->steamName != nullptr ? String::utf8(p_value->steamName) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromSteamNamesResult(const PFAccountManagementGetPlayFabIDsFromSteamNamesResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementSteamNamePlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromTwitchIDsRequest(const PFAccountManagementGetPlayFabIDsFromTwitchIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->twitchIdsCount; ++i) {
            values.push_back(p_value->twitchIds != nullptr && p_value->twitchIds[i] != nullptr ? String::utf8(p_value->twitchIds[i]) : String());
        }
        dictionary["twitch_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementTwitchPlayFabIdPair(const PFAccountManagementTwitchPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["twitch_id"] = p_value->twitchId != nullptr ? String::utf8(p_value->twitchId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromTwitchIDsResult(const PFAccountManagementGetPlayFabIDsFromTwitchIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementTwitchPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromXboxLiveIDsRequest(const PFAccountManagementGetPlayFabIDsFromXboxLiveIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["sandbox"] = p_value->sandbox != nullptr ? String::utf8(p_value->sandbox) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->xboxLiveAccountIDsCount; ++i) {
            values.push_back(p_value->xboxLiveAccountIDs != nullptr && p_value->xboxLiveAccountIDs[i] != nullptr ? String::utf8(p_value->xboxLiveAccountIDs[i]) : String());
        }
        dictionary["xbox_live_account_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementXboxLiveAccountPlayFabIdPair(const PFAccountManagementXboxLiveAccountPlayFabIdPair *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["xbox_live_account_id"] = p_value->xboxLiveAccountId != nullptr ? String::utf8(p_value->xboxLiveAccountId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayFabIDsFromXboxLiveIDsResult(const PFAccountManagementGetPlayFabIDsFromXboxLiveIDsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            values.push_back(to_variant_PFAccountManagementXboxLiveAccountPlayFabIdPair(p_value->data != nullptr ? p_value->data[i] : nullptr));
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFPlayerProfileViewConstraints(const PFPlayerProfileViewConstraints *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["show_avatar_url"] = static_cast<bool>(p_value->showAvatarUrl);
    dictionary["show_banned_until"] = static_cast<bool>(p_value->showBannedUntil);
    dictionary["show_campaign_attributions"] = static_cast<bool>(p_value->showCampaignAttributions);
    dictionary["show_contact_email_addresses"] = static_cast<bool>(p_value->showContactEmailAddresses);
    dictionary["show_created"] = static_cast<bool>(p_value->showCreated);
    dictionary["show_display_name"] = static_cast<bool>(p_value->showDisplayName);
    dictionary["show_experiment_variants"] = static_cast<bool>(p_value->showExperimentVariants);
    dictionary["show_last_login"] = static_cast<bool>(p_value->showLastLogin);
    dictionary["show_linked_accounts"] = static_cast<bool>(p_value->showLinkedAccounts);
    dictionary["show_locations"] = static_cast<bool>(p_value->showLocations);
    dictionary["show_memberships"] = static_cast<bool>(p_value->showMemberships);
    dictionary["show_origination"] = static_cast<bool>(p_value->showOrigination);
    dictionary["show_push_notification_registrations"] = static_cast<bool>(p_value->showPushNotificationRegistrations);
    dictionary["show_statistics"] = static_cast<bool>(p_value->showStatistics);
    dictionary["show_tags"] = static_cast<bool>(p_value->showTags);
    dictionary["show_total_value_to_date_in_usd"] = static_cast<bool>(p_value->showTotalValueToDateInUsd);
    dictionary["show_values_to_date"] = static_cast<bool>(p_value->showValuesToDate);
    return dictionary;
}

Variant to_variant_PFGetPlayerCombinedInfoRequestParams(const PFGetPlayerCombinedInfoRequestParams *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["get_character_inventories"] = static_cast<bool>(p_value->getCharacterInventories);
    dictionary["get_character_list"] = static_cast<bool>(p_value->getCharacterList);
    dictionary["get_player_profile"] = static_cast<bool>(p_value->getPlayerProfile);
    dictionary["get_player_statistics"] = static_cast<bool>(p_value->getPlayerStatistics);
    dictionary["get_title_data"] = static_cast<bool>(p_value->getTitleData);
    dictionary["get_user_account_info"] = static_cast<bool>(p_value->getUserAccountInfo);
    dictionary["get_user_data"] = static_cast<bool>(p_value->getUserData);
    dictionary["get_user_inventory"] = static_cast<bool>(p_value->getUserInventory);
    dictionary["get_user_read_only_data"] = static_cast<bool>(p_value->getUserReadOnlyData);
    dictionary["get_user_virtual_currency"] = static_cast<bool>(p_value->getUserVirtualCurrency);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->playerStatisticNamesCount; ++i) {
            values.push_back(p_value->playerStatisticNames != nullptr && p_value->playerStatisticNames[i] != nullptr ? String::utf8(p_value->playerStatisticNames[i]) : String());
        }
        dictionary["player_statistic_names"] = values;
    }
    dictionary["profile_constraints"] = to_variant_PFPlayerProfileViewConstraints(p_value->profileConstraints);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->titleDataKeysCount; ++i) {
            values.push_back(p_value->titleDataKeys != nullptr && p_value->titleDataKeys[i] != nullptr ? String::utf8(p_value->titleDataKeys[i]) : String());
        }
        dictionary["title_data_keys"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->userDataKeysCount; ++i) {
            values.push_back(p_value->userDataKeys != nullptr && p_value->userDataKeys[i] != nullptr ? String::utf8(p_value->userDataKeys[i]) : String());
        }
        dictionary["user_data_keys"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->userReadOnlyDataKeysCount; ++i) {
            values.push_back(p_value->userReadOnlyDataKeys != nullptr && p_value->userReadOnlyDataKeys[i] != nullptr ? String::utf8(p_value->userReadOnlyDataKeys[i]) : String());
        }
        dictionary["user_read_only_data_keys"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayerCombinedInfoRequest(const PFAccountManagementGetPlayerCombinedInfoRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["info_request_parameters"] = to_variant_PFGetPlayerCombinedInfoRequestParams(p_value->infoRequestParameters);
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFItemInstance(const PFItemInstance *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["annotation"] = p_value->annotation != nullptr ? String::utf8(p_value->annotation) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->bundleContentsCount; ++i) {
            values.push_back(p_value->bundleContents != nullptr && p_value->bundleContents[i] != nullptr ? String::utf8(p_value->bundleContents[i]) : String());
        }
        dictionary["bundle_contents"] = values;
    }
    dictionary["bundle_parent"] = p_value->bundleParent != nullptr ? String::utf8(p_value->bundleParent) : String();
    dictionary["catalog_version"] = p_value->catalogVersion != nullptr ? String::utf8(p_value->catalogVersion) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customDataCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customData[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_data"] = values;
    }
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    if (p_value->expiration != nullptr) dictionary["expiration"] = static_cast<int64_t>(*p_value->expiration);
    dictionary["item_class"] = p_value->itemClass != nullptr ? String::utf8(p_value->itemClass) : String();
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["item_instance_id"] = p_value->itemInstanceId != nullptr ? String::utf8(p_value->itemInstanceId) : String();
    if (p_value->purchaseDate != nullptr) dictionary["purchase_date"] = static_cast<int64_t>(*p_value->purchaseDate);
    if (p_value->remainingUses != nullptr) dictionary["remaining_uses"] = static_cast<int64_t>(*p_value->remainingUses);
    dictionary["unit_currency"] = p_value->unitCurrency != nullptr ? String::utf8(p_value->unitCurrency) : String();
    dictionary["unit_price"] = static_cast<int64_t>(p_value->unitPrice);
    if (p_value->usesIncrementedBy != nullptr) dictionary["uses_incremented_by"] = static_cast<int64_t>(*p_value->usesIncrementedBy);
    return dictionary;
}

Variant to_variant_PFCharacterInventory(const PFCharacterInventory *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["character_id"] = p_value->characterId != nullptr ? String::utf8(p_value->characterId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->inventoryCount; ++i) {
            values.push_back(to_variant_PFItemInstance(p_value->inventory != nullptr ? p_value->inventory[i] : nullptr));
        }
        dictionary["inventory"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCharacterResult(const PFCharacterResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["character_id"] = p_value->characterId != nullptr ? String::utf8(p_value->characterId) : String();
    dictionary["character_name"] = p_value->characterName != nullptr ? String::utf8(p_value->characterName) : String();
    dictionary["character_type"] = p_value->characterType != nullptr ? String::utf8(p_value->characterType) : String();
    return dictionary;
}

Variant to_variant_PFAdCampaignAttributionModel(const PFAdCampaignAttributionModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["attributed_at"] = static_cast<int64_t>(p_value->attributedAt);
    dictionary["campaign_id"] = p_value->campaignId != nullptr ? String::utf8(p_value->campaignId) : String();
    dictionary["platform"] = p_value->platform != nullptr ? String::utf8(p_value->platform) : String();
    return dictionary;
}

Variant to_variant_PFContactEmailInfoModel(const PFContactEmailInfoModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["email_address"] = p_value->emailAddress != nullptr ? String::utf8(p_value->emailAddress) : String();
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    if (p_value->verificationStatus != nullptr) dictionary["verification_status"] = static_cast<int64_t>(*p_value->verificationStatus);
    return dictionary;
}

Variant to_variant_PFLinkedPlatformAccountModel(const PFLinkedPlatformAccountModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["email"] = p_value->email != nullptr ? String::utf8(p_value->email) : String();
    if (p_value->platform != nullptr) dictionary["platform"] = static_cast<int64_t>(*p_value->platform);
    dictionary["platform_user_id"] = p_value->platformUserId != nullptr ? String::utf8(p_value->platformUserId) : String();
    dictionary["username"] = p_value->username != nullptr ? String::utf8(p_value->username) : String();
    return dictionary;
}

Variant to_variant_PFLocationModel(const PFLocationModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["city"] = p_value->city != nullptr ? String::utf8(p_value->city) : String();
    if (p_value->continentCode != nullptr) dictionary["continent_code"] = static_cast<int64_t>(*p_value->continentCode);
    if (p_value->countryCode != nullptr) dictionary["country_code"] = static_cast<int64_t>(*p_value->countryCode);
    if (p_value->latitude != nullptr) dictionary["latitude"] = static_cast<double>(*p_value->latitude);
    if (p_value->longitude != nullptr) dictionary["longitude"] = static_cast<double>(*p_value->longitude);
    return dictionary;
}

Variant to_variant_PFSubscriptionModel(const PFSubscriptionModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["expiration"] = static_cast<int64_t>(p_value->expiration);
    dictionary["initial_subscription_time"] = static_cast<int64_t>(p_value->initialSubscriptionTime);
    dictionary["is_active"] = static_cast<bool>(p_value->isActive);
    if (p_value->status != nullptr) dictionary["status"] = static_cast<int64_t>(*p_value->status);
    dictionary["subscription_id"] = p_value->subscriptionId != nullptr ? String::utf8(p_value->subscriptionId) : String();
    dictionary["subscription_item_id"] = p_value->subscriptionItemId != nullptr ? String::utf8(p_value->subscriptionItemId) : String();
    dictionary["subscription_provider"] = p_value->subscriptionProvider != nullptr ? String::utf8(p_value->subscriptionProvider) : String();
    return dictionary;
}

Variant to_variant_PFMembershipModel(const PFMembershipModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["is_active"] = static_cast<bool>(p_value->isActive);
    dictionary["membership_expiration"] = static_cast<int64_t>(p_value->membershipExpiration);
    dictionary["membership_id"] = p_value->membershipId != nullptr ? String::utf8(p_value->membershipId) : String();
    if (p_value->overrideExpiration != nullptr) dictionary["override_expiration"] = static_cast<int64_t>(*p_value->overrideExpiration);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->subscriptionsCount; ++i) {
            values.push_back(to_variant_PFSubscriptionModel(p_value->subscriptions != nullptr ? p_value->subscriptions[i] : nullptr));
        }
        dictionary["subscriptions"] = values;
    }
    return dictionary;
}

Variant to_variant_PFPushNotificationRegistrationModel(const PFPushNotificationRegistrationModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["notification_endpoint_arn"] = p_value->notificationEndpointARN != nullptr ? String::utf8(p_value->notificationEndpointARN) : String();
    if (p_value->platform != nullptr) dictionary["platform"] = static_cast<int64_t>(*p_value->platform);
    return dictionary;
}

Variant to_variant_PFStatisticModel(const PFStatisticModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["value"] = static_cast<int64_t>(p_value->value);
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    return dictionary;
}

Variant to_variant_PFTagModel(const PFTagModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["tag_value"] = p_value->tagValue != nullptr ? String::utf8(p_value->tagValue) : String();
    return dictionary;
}

Variant to_variant_PFValueToDateModel(const PFValueToDateModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["currency"] = p_value->currency != nullptr ? String::utf8(p_value->currency) : String();
    dictionary["total_value"] = static_cast<int64_t>(p_value->totalValue);
    dictionary["total_value_as_decimal"] = p_value->totalValueAsDecimal != nullptr ? String::utf8(p_value->totalValueAsDecimal) : String();
    return dictionary;
}

Variant to_variant_PFPlayerProfileModel(const PFPlayerProfileModel *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->adCampaignAttributionsCount; ++i) {
            values.push_back(to_variant_PFAdCampaignAttributionModel(p_value->adCampaignAttributions != nullptr ? p_value->adCampaignAttributions[i] : nullptr));
        }
        dictionary["ad_campaign_attributions"] = values;
    }
    dictionary["avatar_url"] = p_value->avatarUrl != nullptr ? String::utf8(p_value->avatarUrl) : String();
    if (p_value->bannedUntil != nullptr) dictionary["banned_until"] = static_cast<int64_t>(*p_value->bannedUntil);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->contactEmailAddressesCount; ++i) {
            values.push_back(to_variant_PFContactEmailInfoModel(p_value->contactEmailAddresses != nullptr ? p_value->contactEmailAddresses[i] : nullptr));
        }
        dictionary["contact_email_addresses"] = values;
    }
    if (p_value->created != nullptr) dictionary["created"] = static_cast<int64_t>(*p_value->created);
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->experimentVariantsCount; ++i) {
            values.push_back(p_value->experimentVariants != nullptr && p_value->experimentVariants[i] != nullptr ? String::utf8(p_value->experimentVariants[i]) : String());
        }
        dictionary["experiment_variants"] = values;
    }
    if (p_value->lastLogin != nullptr) dictionary["last_login"] = static_cast<int64_t>(*p_value->lastLogin);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->linkedAccountsCount; ++i) {
            values.push_back(to_variant_PFLinkedPlatformAccountModel(p_value->linkedAccounts != nullptr ? p_value->linkedAccounts[i] : nullptr));
        }
        dictionary["linked_accounts"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->locationsCount; ++i) {
            values.push_back(to_variant_PFLocationModel(p_value->locations != nullptr ? p_value->locations[i] : nullptr));
        }
        dictionary["locations"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->membershipsCount; ++i) {
            values.push_back(to_variant_PFMembershipModel(p_value->memberships != nullptr ? p_value->memberships[i] : nullptr));
        }
        dictionary["memberships"] = values;
    }
    if (p_value->origination != nullptr) dictionary["origination"] = static_cast<int64_t>(*p_value->origination);
    dictionary["player_id"] = p_value->playerId != nullptr ? String::utf8(p_value->playerId) : String();
    dictionary["publisher_id"] = p_value->publisherId != nullptr ? String::utf8(p_value->publisherId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->pushNotificationRegistrationsCount; ++i) {
            values.push_back(to_variant_PFPushNotificationRegistrationModel(p_value->pushNotificationRegistrations != nullptr ? p_value->pushNotificationRegistrations[i] : nullptr));
        }
        dictionary["push_notification_registrations"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            values.push_back(to_variant_PFStatisticModel(p_value->statistics != nullptr ? p_value->statistics[i] : nullptr));
        }
        dictionary["statistics"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(to_variant_PFTagModel(p_value->tags != nullptr ? p_value->tags[i] : nullptr));
        }
        dictionary["tags"] = values;
    }
    dictionary["title_id"] = p_value->titleId != nullptr ? String::utf8(p_value->titleId) : String();
    if (p_value->totalValueToDateInUSD != nullptr) dictionary["total_value_to_date_in_usd"] = static_cast<int64_t>(*p_value->totalValueToDateInUSD);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->valuesToDateCount; ++i) {
            values.push_back(to_variant_PFValueToDateModel(p_value->valuesToDate != nullptr ? p_value->valuesToDate[i] : nullptr));
        }
        dictionary["values_to_date"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticValue(const PFStatisticValue *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["statistic_name"] = p_value->statisticName != nullptr ? String::utf8(p_value->statisticName) : String();
    dictionary["value"] = static_cast<int64_t>(p_value->value);
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    return dictionary;
}

Variant to_variant_PFUserDataRecord(const PFUserDataRecord *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["last_updated"] = static_cast<int64_t>(p_value->lastUpdated);
    if (p_value->permission != nullptr) dictionary["permission"] = static_cast<int64_t>(*p_value->permission);
    dictionary["value"] = p_value->value != nullptr ? String::utf8(p_value->value) : String();
    return dictionary;
}

Variant to_variant_PFUserDataRecordDictionaryEntry(const PFUserDataRecordDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFUserDataRecord(p_value->value);
    return dictionary;
}

Variant to_variant_PFInt32DictionaryEntry(const PFInt32DictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = static_cast<int64_t>(p_value->value);
    return dictionary;
}

Variant to_variant_PFVirtualCurrencyRechargeTime(const PFVirtualCurrencyRechargeTime *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["recharge_max"] = static_cast<int64_t>(p_value->rechargeMax);
    dictionary["recharge_time"] = static_cast<int64_t>(p_value->rechargeTime);
    dictionary["seconds_to_recharge"] = static_cast<int64_t>(p_value->secondsToRecharge);
    return dictionary;
}

Variant to_variant_PFVirtualCurrencyRechargeTimeDictionaryEntry(const PFVirtualCurrencyRechargeTimeDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFVirtualCurrencyRechargeTime(p_value->value);
    return dictionary;
}

Variant to_variant_PFGetPlayerCombinedInfoResultPayload(const PFGetPlayerCombinedInfoResultPayload *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["account_info"] = to_variant_PFUserAccountInfo(p_value->accountInfo);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->characterInventoriesCount; ++i) {
            values.push_back(to_variant_PFCharacterInventory(p_value->characterInventories != nullptr ? p_value->characterInventories[i] : nullptr));
        }
        dictionary["character_inventories"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->characterListCount; ++i) {
            values.push_back(to_variant_PFCharacterResult(p_value->characterList != nullptr ? p_value->characterList[i] : nullptr));
        }
        dictionary["character_list"] = values;
    }
    dictionary["player_profile"] = to_variant_PFPlayerProfileModel(p_value->playerProfile);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->playerStatisticsCount; ++i) {
            values.push_back(to_variant_PFStatisticValue(p_value->playerStatistics != nullptr ? p_value->playerStatistics[i] : nullptr));
        }
        dictionary["player_statistics"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->titleDataCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->titleData[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["title_data"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->userDataCount; ++i) {
            const Variant entry_variant = to_variant_PFUserDataRecordDictionaryEntry(&p_value->userData[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["user_data"] = values;
    }
    dictionary["user_data_version"] = static_cast<int64_t>(p_value->userDataVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->userInventoryCount; ++i) {
            values.push_back(to_variant_PFItemInstance(p_value->userInventory != nullptr ? p_value->userInventory[i] : nullptr));
        }
        dictionary["user_inventory"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->userReadOnlyDataCount; ++i) {
            const Variant entry_variant = to_variant_PFUserDataRecordDictionaryEntry(&p_value->userReadOnlyData[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["user_read_only_data"] = values;
    }
    dictionary["user_read_only_data_version"] = static_cast<int64_t>(p_value->userReadOnlyDataVersion);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->userVirtualCurrencyCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->userVirtualCurrency[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["user_virtual_currency"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->userVirtualCurrencyRechargeTimesCount; ++i) {
            const Variant entry_variant = to_variant_PFVirtualCurrencyRechargeTimeDictionaryEntry(&p_value->userVirtualCurrencyRechargeTimes[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["user_virtual_currency_recharge_times"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayerCombinedInfoResult(const PFAccountManagementGetPlayerCombinedInfoResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["info_result_payload"] = to_variant_PFGetPlayerCombinedInfoResultPayload(p_value->infoResultPayload);
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayerProfileRequest(const PFAccountManagementGetPlayerProfileRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    dictionary["profile_constraints"] = to_variant_PFPlayerProfileViewConstraints(p_value->profileConstraints);
    return dictionary;
}

Variant to_variant_PFAccountManagementGetPlayerProfileResult(const PFAccountManagementGetPlayerProfileResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["player_profile"] = to_variant_PFPlayerProfileModel(p_value->playerProfile);
    return dictionary;
}

Variant to_variant_PFEntityLineage(const PFEntityLineage *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["character_id"] = p_value->characterId != nullptr ? String::utf8(p_value->characterId) : String();
    dictionary["group_id"] = p_value->groupId != nullptr ? String::utf8(p_value->groupId) : String();
    dictionary["master_player_account_id"] = p_value->masterPlayerAccountId != nullptr ? String::utf8(p_value->masterPlayerAccountId) : String();
    dictionary["namespace_id"] = p_value->namespaceId != nullptr ? String::utf8(p_value->namespaceId) : String();
    dictionary["title_id"] = p_value->titleId != nullptr ? String::utf8(p_value->titleId) : String();
    dictionary["title_player_account_id"] = p_value->titlePlayerAccountId != nullptr ? String::utf8(p_value->titlePlayerAccountId) : String();
    return dictionary;
}

Variant to_variant_PFEntityLineageDictionaryEntry(const PFEntityLineageDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFEntityLineage(p_value->value);
    return dictionary;
}

Variant to_variant_PFAccountManagementGetTitlePlayersFromProviderIDsResponse(const PFAccountManagementGetTitlePlayersFromProviderIDsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->titlePlayerAccountsCount; ++i) {
            const Variant entry_variant = to_variant_PFEntityLineageDictionaryEntry(&p_value->titlePlayerAccounts[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["title_player_accounts"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementGetTitlePlayersFromXboxLiveIDsRequest(const PFAccountManagementGetTitlePlayersFromXboxLiveIDsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["sandbox"] = p_value->sandbox != nullptr ? String::utf8(p_value->sandbox) : String();
    dictionary["title_id"] = p_value->titleId != nullptr ? String::utf8(p_value->titleId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->xboxLiveIdsCount; ++i) {
            values.push_back(p_value->xboxLiveIds != nullptr && p_value->xboxLiveIds[i] != nullptr ? String::utf8(p_value->xboxLiveIds[i]) : String());
        }
        dictionary["xbox_live_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementLinkCustomIDRequest(const PFAccountManagementLinkCustomIDRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["custom_id"] = p_value->customId != nullptr ? String::utf8(p_value->customId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    return dictionary;
}

Variant to_variant_PFAccountManagementLinkOpenIdConnectRequest(const PFAccountManagementLinkOpenIdConnectRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["connection_id"] = p_value->connectionId != nullptr ? String::utf8(p_value->connectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    dictionary["id_token"] = p_value->idToken != nullptr ? String::utf8(p_value->idToken) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementLinkSteamAccountRequest(const PFAccountManagementLinkSteamAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->forceLink != nullptr) dictionary["force_link"] = static_cast<bool>(*p_value->forceLink);
    dictionary["steam_ticket"] = p_value->steamTicket != nullptr ? String::utf8(p_value->steamTicket) : String();
    if (p_value->ticketIsServiceSpecific != nullptr) dictionary["ticket_is_service_specific"] = static_cast<bool>(*p_value->ticketIsServiceSpecific);
    return dictionary;
}

Variant to_variant_PFAccountManagementRemoveContactEmailRequest(const PFAccountManagementRemoveContactEmailRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementReportPlayerClientRequest(const PFAccountManagementReportPlayerClientRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["comment"] = p_value->comment != nullptr ? String::utf8(p_value->comment) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["reportee_id"] = p_value->reporteeId != nullptr ? String::utf8(p_value->reporteeId) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementReportPlayerClientResult(const PFAccountManagementReportPlayerClientResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["submissions_remaining"] = static_cast<int64_t>(p_value->submissionsRemaining);
    return dictionary;
}

Variant to_variant_PFAccountManagementSetDisplayNameRequest(const PFAccountManagementSetDisplayNameRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    if (p_value->expectedVersion != nullptr) dictionary["expected_version"] = static_cast<int64_t>(*p_value->expectedVersion);
    return dictionary;
}

Variant to_variant_PFAccountManagementSetDisplayNameResponse(const PFAccountManagementSetDisplayNameResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->operationResult != nullptr) dictionary["operation_result"] = static_cast<int64_t>(*p_value->operationResult);
    if (p_value->versionNumber != nullptr) dictionary["version_number"] = static_cast<int64_t>(*p_value->versionNumber);
    return dictionary;
}

Variant to_variant_PFAccountManagementUnlinkCustomIDRequest(const PFAccountManagementUnlinkCustomIDRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["custom_id"] = p_value->customId != nullptr ? String::utf8(p_value->customId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementUnlinkOpenIdConnectRequest(const PFAccountManagementUnlinkOpenIdConnectRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["connection_id"] = p_value->connectionId != nullptr ? String::utf8(p_value->connectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementUnlinkSteamAccountRequest(const PFAccountManagementUnlinkSteamAccountRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFAccountManagementUpdateUserTitleDisplayNameRequest(const PFAccountManagementUpdateUserTitleDisplayNameRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    return dictionary;
}

Variant to_variant_PFAccountManagementUpdateUserTitleDisplayNameResult(const PFAccountManagementUpdateUserTitleDisplayNameResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCatalogAlternateId(const PFCatalogCatalogAlternateId *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    dictionary["value"] = p_value->value != nullptr ? String::utf8(p_value->value) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCatalogSpecificConfig(const PFCatalogCatalogSpecificConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->contentTypesCount; ++i) {
            values.push_back(p_value->contentTypes != nullptr && p_value->contentTypes[i] != nullptr ? String::utf8(p_value->contentTypes[i]) : String());
        }
        dictionary["content_types"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogDeepLinkFormat(const PFCatalogDeepLinkFormat *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["format"] = p_value->format != nullptr ? String::utf8(p_value->format) : String();
    dictionary["platform"] = p_value->platform != nullptr ? String::utf8(p_value->platform) : String();
    return dictionary;
}

Variant to_variant_PFCatalogDisplayPropertyIndexInfo(const PFCatalogDisplayPropertyIndexInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    if (p_value->type != nullptr) dictionary["type"] = static_cast<int64_t>(*p_value->type);
    return dictionary;
}

Variant to_variant_PFCatalogFileConfig(const PFCatalogFileConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->contentTypesCount; ++i) {
            values.push_back(p_value->contentTypes != nullptr && p_value->contentTypes[i] != nullptr ? String::utf8(p_value->contentTypes[i]) : String());
        }
        dictionary["content_types"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogImageConfig(const PFCatalogImageConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogCategoryRatingConfig(const PFCatalogCategoryRatingConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    return dictionary;
}

Variant to_variant_PFCatalogReviewConfig(const PFCatalogReviewConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->categoryRatingsCount; ++i) {
            values.push_back(to_variant_PFCatalogCategoryRatingConfig(p_value->categoryRatings != nullptr ? p_value->categoryRatings[i] : nullptr));
        }
        dictionary["category_ratings"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogUserGeneratedContentSpecificConfig(const PFCatalogUserGeneratedContentSpecificConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->contentTypesCount; ++i) {
            values.push_back(p_value->contentTypes != nullptr && p_value->contentTypes[i] != nullptr ? String::utf8(p_value->contentTypes[i]) : String());
        }
        dictionary["content_types"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogCatalogConfig(const PFCatalogCatalogConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->adminEntitiesCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->adminEntities != nullptr ? p_value->adminEntities[i] : nullptr));
        }
        dictionary["admin_entities"] = values;
    }
    dictionary["catalog"] = to_variant_PFCatalogCatalogSpecificConfig(p_value->catalog);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->deepLinkFormatsCount; ++i) {
            values.push_back(to_variant_PFCatalogDeepLinkFormat(p_value->deepLinkFormats != nullptr ? p_value->deepLinkFormats[i] : nullptr));
        }
        dictionary["deep_link_formats"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->displayPropertyIndexInfosCount; ++i) {
            values.push_back(to_variant_PFCatalogDisplayPropertyIndexInfo(p_value->displayPropertyIndexInfos != nullptr ? p_value->displayPropertyIndexInfos[i] : nullptr));
        }
        dictionary["display_property_index_infos"] = values;
    }
    dictionary["file"] = to_variant_PFCatalogFileConfig(p_value->file);
    dictionary["image"] = to_variant_PFCatalogImageConfig(p_value->image);
    dictionary["is_catalog_enabled"] = static_cast<bool>(p_value->isCatalogEnabled);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->platformsCount; ++i) {
            values.push_back(p_value->platforms != nullptr && p_value->platforms[i] != nullptr ? String::utf8(p_value->platforms[i]) : String());
        }
        dictionary["platforms"] = values;
    }
    dictionary["review"] = to_variant_PFCatalogReviewConfig(p_value->review);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->reviewerEntitiesCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->reviewerEntities != nullptr ? p_value->reviewerEntities[i] : nullptr));
        }
        dictionary["reviewer_entities"] = values;
    }
    dictionary["user_generated_content"] = to_variant_PFCatalogUserGeneratedContentSpecificConfig(p_value->userGeneratedContent);
    return dictionary;
}

Variant to_variant_PFCatalogContent(const PFCatalogContent *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["max_client_version"] = p_value->maxClientVersion != nullptr ? String::utf8(p_value->maxClientVersion) : String();
    dictionary["min_client_version"] = p_value->minClientVersion != nullptr ? String::utf8(p_value->minClientVersion) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    dictionary["url"] = p_value->url != nullptr ? String::utf8(p_value->url) : String();
    return dictionary;
}

Variant to_variant_PFCatalogDeepLink(const PFCatalogDeepLink *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["platform"] = p_value->platform != nullptr ? String::utf8(p_value->platform) : String();
    dictionary["url"] = p_value->url != nullptr ? String::utf8(p_value->url) : String();
    return dictionary;
}

Variant to_variant_PFJsonObject(const PFJsonObject *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["string_value"] = p_value->stringValue != nullptr ? String::utf8(p_value->stringValue) : String();
    return dictionary;
}

Variant to_variant_PFCatalogImage(const PFCatalogImage *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["tag"] = p_value->tag != nullptr ? String::utf8(p_value->tag) : String();
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    dictionary["url"] = p_value->url != nullptr ? String::utf8(p_value->url) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCatalogPriceAmount(const PFCatalogCatalogPriceAmount *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["amount"] = static_cast<int64_t>(p_value->amount);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCatalogPrice(const PFCatalogCatalogPrice *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->amountsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogPriceAmount(p_value->amounts != nullptr ? p_value->amounts[i] : nullptr));
        }
        dictionary["amounts"] = values;
    }
    if (p_value->unitAmount != nullptr) dictionary["unit_amount"] = static_cast<int64_t>(*p_value->unitAmount);
    if (p_value->unitDurationInSeconds != nullptr) dictionary["unit_duration_in_seconds"] = static_cast<double>(*p_value->unitDurationInSeconds);
    return dictionary;
}

Variant to_variant_PFCatalogCatalogPriceOptions(const PFCatalogCatalogPriceOptions *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->pricesCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogPrice(p_value->prices != nullptr ? p_value->prices[i] : nullptr));
        }
        dictionary["prices"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogCatalogItemReference(const PFCatalogCatalogItemReference *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["price_options"] = to_variant_PFCatalogCatalogPriceOptions(p_value->priceOptions);
    return dictionary;
}

Variant to_variant_PFCatalogKeywordSet(const PFCatalogKeywordSet *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->valuesCount; ++i) {
            values.push_back(p_value->values != nullptr && p_value->values[i] != nullptr ? String::utf8(p_value->values[i]) : String());
        }
        dictionary["values"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogKeywordSetDictionaryEntry(const PFCatalogKeywordSetDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFCatalogKeywordSet(p_value->value);
    return dictionary;
}

Variant to_variant_PFCatalogModerationState(const PFCatalogModerationState *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->lastModifiedDate != nullptr) dictionary["last_modified_date"] = static_cast<int64_t>(*p_value->lastModifiedDate);
    dictionary["reason"] = p_value->reason != nullptr ? String::utf8(p_value->reason) : String();
    if (p_value->status != nullptr) dictionary["status"] = static_cast<int64_t>(*p_value->status);
    return dictionary;
}

Variant to_variant_PFCatalogRating(const PFCatalogRating *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->average != nullptr) dictionary["average"] = static_cast<double>(*p_value->average);
    if (p_value->count1Star != nullptr) dictionary["count1_star"] = static_cast<int64_t>(*p_value->count1Star);
    if (p_value->count2Star != nullptr) dictionary["count2_star"] = static_cast<int64_t>(*p_value->count2Star);
    if (p_value->count3Star != nullptr) dictionary["count3_star"] = static_cast<int64_t>(*p_value->count3Star);
    if (p_value->count4Star != nullptr) dictionary["count4_star"] = static_cast<int64_t>(*p_value->count4Star);
    if (p_value->count5Star != nullptr) dictionary["count5_star"] = static_cast<int64_t>(*p_value->count5Star);
    return dictionary;
}

Variant to_variant_PFCatalogRealMoneyPriceDetails(const PFCatalogRealMoneyPriceDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->appleAppStorePricesCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->appleAppStorePrices[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["apple_app_store_prices"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->googlePlayPricesCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->googlePlayPrices[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["google_play_prices"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->microsoftStorePricesCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->microsoftStorePrices[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["microsoft_store_prices"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->nintendoEShopPricesCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->nintendoEShopPrices[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["nintendo_e_shop_prices"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->playStationStorePricesCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->playStationStorePrices[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["play_station_store_prices"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->steamPricesCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->steamPrices[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["steam_prices"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogFilterOptions(const PFCatalogFilterOptions *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["filter"] = p_value->filter != nullptr ? String::utf8(p_value->filter) : String();
    if (p_value->includeAllItems != nullptr) dictionary["include_all_items"] = static_cast<bool>(*p_value->includeAllItems);
    return dictionary;
}

Variant to_variant_PFCatalogPermissions(const PFCatalogPermissions *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->segmentIdsCount; ++i) {
            values.push_back(p_value->segmentIds != nullptr && p_value->segmentIds[i] != nullptr ? String::utf8(p_value->segmentIds[i]) : String());
        }
        dictionary["segment_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogCatalogPriceAmountOverride(const PFCatalogCatalogPriceAmountOverride *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->fixedValue != nullptr) dictionary["fixed_value"] = static_cast<int64_t>(*p_value->fixedValue);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    if (p_value->multiplier != nullptr) dictionary["multiplier"] = static_cast<double>(*p_value->multiplier);
    return dictionary;
}

Variant to_variant_PFCatalogCatalogPriceOverride(const PFCatalogCatalogPriceOverride *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->amountsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogPriceAmountOverride(p_value->amounts != nullptr ? p_value->amounts[i] : nullptr));
        }
        dictionary["amounts"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogCatalogPriceOptionsOverride(const PFCatalogCatalogPriceOptionsOverride *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->pricesCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogPriceOverride(p_value->prices != nullptr ? p_value->prices[i] : nullptr));
        }
        dictionary["prices"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogStoreDetails(const PFCatalogStoreDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["filter_options"] = to_variant_PFCatalogFilterOptions(p_value->filterOptions);
    dictionary["permissions"] = to_variant_PFCatalogPermissions(p_value->permissions);
    dictionary["price_options_override"] = to_variant_PFCatalogCatalogPriceOptionsOverride(p_value->priceOptionsOverride);
    return dictionary;
}

Variant to_variant_PFCatalogCatalogItem(const PFCatalogCatalogItem *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->alternateIdsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogAlternateId(p_value->alternateIds != nullptr ? p_value->alternateIds[i] : nullptr));
        }
        dictionary["alternate_ids"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->contentsCount; ++i) {
            values.push_back(to_variant_PFCatalogContent(p_value->contents != nullptr ? p_value->contents[i] : nullptr));
        }
        dictionary["contents"] = values;
    }
    dictionary["content_type"] = p_value->contentType != nullptr ? String::utf8(p_value->contentType) : String();
    if (p_value->creationDate != nullptr) dictionary["creation_date"] = static_cast<int64_t>(*p_value->creationDate);
    dictionary["creator_entity"] = to_variant_PFEntityKey(p_value->creatorEntity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->deepLinksCount; ++i) {
            values.push_back(to_variant_PFCatalogDeepLink(p_value->deepLinks != nullptr ? p_value->deepLinks[i] : nullptr));
        }
        dictionary["deep_links"] = values;
    }
    dictionary["default_stack_id"] = p_value->defaultStackId != nullptr ? String::utf8(p_value->defaultStackId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->descriptionCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->description[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["description"] = values;
    }
    dictionary["display_properties"] = p_value->displayProperties.stringValue != nullptr ? String::utf8(p_value->displayProperties.stringValue) : String();
    dictionary["display_version"] = p_value->displayVersion != nullptr ? String::utf8(p_value->displayVersion) : String();
    if (p_value->endDate != nullptr) dictionary["end_date"] = static_cast<int64_t>(*p_value->endDate);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->imagesCount; ++i) {
            values.push_back(to_variant_PFCatalogImage(p_value->images != nullptr ? p_value->images[i] : nullptr));
        }
        dictionary["images"] = values;
    }
    if (p_value->isHidden != nullptr) dictionary["is_hidden"] = static_cast<bool>(*p_value->isHidden);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->itemReferencesCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogItemReference(p_value->itemReferences != nullptr ? p_value->itemReferences[i] : nullptr));
        }
        dictionary["item_references"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->keywordsCount; ++i) {
            const Variant entry_variant = to_variant_PFCatalogKeywordSetDictionaryEntry(&p_value->keywords[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["keywords"] = values;
    }
    if (p_value->lastModifiedDate != nullptr) dictionary["last_modified_date"] = static_cast<int64_t>(*p_value->lastModifiedDate);
    dictionary["moderation"] = to_variant_PFCatalogModerationState(p_value->moderation);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->platformsCount; ++i) {
            values.push_back(p_value->platforms != nullptr && p_value->platforms[i] != nullptr ? String::utf8(p_value->platforms[i]) : String());
        }
        dictionary["platforms"] = values;
    }
    dictionary["price_options"] = to_variant_PFCatalogCatalogPriceOptions(p_value->priceOptions);
    dictionary["rating"] = to_variant_PFCatalogRating(p_value->rating);
    dictionary["real_money_price_details"] = to_variant_PFCatalogRealMoneyPriceDetails(p_value->realMoneyPriceDetails);
    if (p_value->startDate != nullptr) dictionary["start_date"] = static_cast<int64_t>(*p_value->startDate);
    dictionary["store_details"] = to_variant_PFCatalogStoreDetails(p_value->storeDetails);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->titleCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->title[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["title"] = values;
    }
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCreateDraftItemRequest(const PFCatalogCreateDraftItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["item"] = to_variant_PFCatalogCatalogItem(p_value->item);
    dictionary["publish"] = static_cast<bool>(p_value->publish);
    return dictionary;
}

Variant to_variant_PFCatalogCreateDraftItemResponse(const PFCatalogCreateDraftItemResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item"] = to_variant_PFCatalogCatalogItem(p_value->item);
    return dictionary;
}

Variant to_variant_PFCatalogUploadInfo(const PFCatalogUploadInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["file_name"] = p_value->fileName != nullptr ? String::utf8(p_value->fileName) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCreateUploadUrlsRequest(const PFCatalogCreateUploadUrlsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->filesCount; ++i) {
            values.push_back(to_variant_PFCatalogUploadInfo(p_value->files != nullptr ? p_value->files[i] : nullptr));
        }
        dictionary["files"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogUploadUrlMetadata(const PFCatalogUploadUrlMetadata *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["file_name"] = p_value->fileName != nullptr ? String::utf8(p_value->fileName) : String();
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["url"] = p_value->url != nullptr ? String::utf8(p_value->url) : String();
    return dictionary;
}

Variant to_variant_PFCatalogCreateUploadUrlsResponse(const PFCatalogCreateUploadUrlsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->uploadUrlsCount; ++i) {
            values.push_back(to_variant_PFCatalogUploadUrlMetadata(p_value->uploadUrls != nullptr ? p_value->uploadUrls[i] : nullptr));
        }
        dictionary["upload_urls"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogDeleteEntityItemReviewsRequest(const PFCatalogDeleteEntityItemReviewsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFCatalogDeleteItemRequest(const PFCatalogDeleteItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetCatalogConfigRequest(const PFCatalogGetCatalogConfigRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogGetCatalogConfigResponse(const PFCatalogGetCatalogConfigResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["config"] = to_variant_PFCatalogCatalogConfig(p_value->config);
    return dictionary;
}

Variant to_variant_PFCatalogGetDraftItemRequest(const PFCatalogGetDraftItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetDraftItemResponse(const PFCatalogGetDraftItemResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item"] = to_variant_PFCatalogCatalogItem(p_value->item);
    return dictionary;
}

Variant to_variant_PFCatalogGetDraftItemsRequest(const PFCatalogGetDraftItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->alternateIdsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogAlternateId(p_value->alternateIds != nullptr ? p_value->alternateIds[i] : nullptr));
        }
        dictionary["alternate_ids"] = values;
    }
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    if (p_value->count != nullptr) dictionary["count"] = static_cast<int64_t>(*p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->idsCount; ++i) {
            values.push_back(p_value->ids != nullptr && p_value->ids[i] != nullptr ? String::utf8(p_value->ids[i]) : String());
        }
        dictionary["ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogGetDraftItemsResponse(const PFCatalogGetDraftItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->itemsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogItem(p_value->items != nullptr ? p_value->items[i] : nullptr));
        }
        dictionary["items"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogGetEntityDraftItemsRequest(const PFCatalogGetEntityDraftItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["filter"] = p_value->filter != nullptr ? String::utf8(p_value->filter) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetEntityDraftItemsResponse(const PFCatalogGetEntityDraftItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->itemsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogItem(p_value->items != nullptr ? p_value->items[i] : nullptr));
        }
        dictionary["items"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogGetEntityItemReviewRequest(const PFCatalogGetEntityItemReviewRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogReview(const PFCatalogReview *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->categoryRatingsCount; ++i) {
            const Variant entry_variant = to_variant_PFInt32DictionaryEntry(&p_value->categoryRatings[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["category_ratings"] = values;
    }
    dictionary["helpful_negative"] = static_cast<int64_t>(p_value->helpfulNegative);
    dictionary["helpful_positive"] = static_cast<int64_t>(p_value->helpfulPositive);
    dictionary["is_installed"] = static_cast<bool>(p_value->isInstalled);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["item_version"] = p_value->itemVersion != nullptr ? String::utf8(p_value->itemVersion) : String();
    dictionary["locale"] = p_value->locale != nullptr ? String::utf8(p_value->locale) : String();
    dictionary["rating"] = static_cast<int64_t>(p_value->rating);
    dictionary["reviewer_entity"] = to_variant_PFEntityKey(p_value->reviewerEntity);
    dictionary["review_id"] = p_value->reviewId != nullptr ? String::utf8(p_value->reviewId) : String();
    dictionary["review_text"] = p_value->reviewText != nullptr ? String::utf8(p_value->reviewText) : String();
    dictionary["submitted"] = static_cast<int64_t>(p_value->submitted);
    dictionary["title"] = p_value->title != nullptr ? String::utf8(p_value->title) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetEntityItemReviewResponse(const PFCatalogGetEntityItemReviewResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["review"] = to_variant_PFCatalogReview(p_value->review);
    return dictionary;
}

Variant to_variant_PFCatalogGetItemContainersRequest(const PFCatalogGetItemContainersRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemContainersResponse(const PFCatalogGetItemContainersResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->containersCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogItem(p_value->containers != nullptr ? p_value->containers[i] : nullptr));
        }
        dictionary["containers"] = values;
    }
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemModerationStateRequest(const PFCatalogGetItemModerationStateRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemModerationStateResponse(const PFCatalogGetItemModerationStateResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["state"] = to_variant_PFCatalogModerationState(p_value->state);
    return dictionary;
}

Variant to_variant_PFCatalogGetItemPublishStatusRequest(const PFCatalogGetItemPublishStatusRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemPublishStatusResponse(const PFCatalogGetItemPublishStatusResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->result != nullptr) dictionary["result"] = static_cast<int64_t>(*p_value->result);
    dictionary["status_message"] = p_value->statusMessage != nullptr ? String::utf8(p_value->statusMessage) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemRequest(const PFCatalogGetItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemResponse(const PFCatalogGetItemResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item"] = to_variant_PFCatalogCatalogItem(p_value->item);
    return dictionary;
}

Variant to_variant_PFCatalogGetItemReviewSummaryRequest(const PFCatalogGetItemReviewSummaryRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemReviewSummaryResponse(const PFCatalogGetItemReviewSummaryResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["least_favorable_review"] = to_variant_PFCatalogReview(p_value->leastFavorableReview);
    dictionary["most_favorable_review"] = to_variant_PFCatalogReview(p_value->mostFavorableReview);
    dictionary["rating"] = to_variant_PFCatalogRating(p_value->rating);
    return dictionary;
}

Variant to_variant_PFCatalogGetItemReviewsRequest(const PFCatalogGetItemReviewsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["order_by"] = p_value->orderBy != nullptr ? String::utf8(p_value->orderBy) : String();
    return dictionary;
}

Variant to_variant_PFCatalogGetItemReviewsResponse(const PFCatalogGetItemReviewsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->reviewsCount; ++i) {
            values.push_back(to_variant_PFCatalogReview(p_value->reviews != nullptr ? p_value->reviews[i] : nullptr));
        }
        dictionary["reviews"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogGetItemsRequest(const PFCatalogGetItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->alternateIdsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogAlternateId(p_value->alternateIds != nullptr ? p_value->alternateIds[i] : nullptr));
        }
        dictionary["alternate_ids"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->idsCount; ++i) {
            values.push_back(p_value->ids != nullptr && p_value->ids[i] != nullptr ? String::utf8(p_value->ids[i]) : String());
        }
        dictionary["ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogGetItemsResponse(const PFCatalogGetItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->itemsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogItem(p_value->items != nullptr ? p_value->items[i] : nullptr));
        }
        dictionary["items"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogPublishDraftItemRequest(const PFCatalogPublishDraftItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogReportItemRequest(const PFCatalogReportItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    if (p_value->concernCategory != nullptr) dictionary["concern_category"] = static_cast<int64_t>(*p_value->concernCategory);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["reason"] = p_value->reason != nullptr ? String::utf8(p_value->reason) : String();
    return dictionary;
}

Variant to_variant_PFCatalogReportItemReviewRequest(const PFCatalogReportItemReviewRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    if (p_value->concernCategory != nullptr) dictionary["concern_category"] = static_cast<int64_t>(*p_value->concernCategory);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["reason"] = p_value->reason != nullptr ? String::utf8(p_value->reason) : String();
    dictionary["review_id"] = p_value->reviewId != nullptr ? String::utf8(p_value->reviewId) : String();
    return dictionary;
}

Variant to_variant_PFCatalogReviewItemRequest(const PFCatalogReviewItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["review"] = to_variant_PFCatalogReview(p_value->review);
    return dictionary;
}

Variant to_variant_PFCatalogReviewTakedown(const PFCatalogReviewTakedown *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["review_id"] = p_value->reviewId != nullptr ? String::utf8(p_value->reviewId) : String();
    return dictionary;
}

Variant to_variant_PFCatalogStoreReference(const PFCatalogStoreReference *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    return dictionary;
}

Variant to_variant_PFCatalogSearchItemsRequest(const PFCatalogSearchItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["filter"] = p_value->filter != nullptr ? String::utf8(p_value->filter) : String();
    dictionary["language"] = p_value->language != nullptr ? String::utf8(p_value->language) : String();
    dictionary["order_by"] = p_value->orderBy != nullptr ? String::utf8(p_value->orderBy) : String();
    dictionary["search"] = p_value->search != nullptr ? String::utf8(p_value->search) : String();
    dictionary["select"] = p_value->select != nullptr ? String::utf8(p_value->select) : String();
    dictionary["store"] = to_variant_PFCatalogStoreReference(p_value->store);
    return dictionary;
}

Variant to_variant_PFCatalogSearchItemsResponse(const PFCatalogSearchItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->itemsCount; ++i) {
            values.push_back(to_variant_PFCatalogCatalogItem(p_value->items != nullptr ? p_value->items[i] : nullptr));
        }
        dictionary["items"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogSetItemModerationStateRequest(const PFCatalogSetItemModerationStateRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["reason"] = p_value->reason != nullptr ? String::utf8(p_value->reason) : String();
    if (p_value->status != nullptr) dictionary["status"] = static_cast<int64_t>(*p_value->status);
    return dictionary;
}

Variant to_variant_PFCatalogSubmitItemReviewVoteRequest(const PFCatalogSubmitItemReviewVoteRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFCatalogCatalogAlternateId(p_value->alternateId);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["review_id"] = p_value->reviewId != nullptr ? String::utf8(p_value->reviewId) : String();
    if (p_value->vote != nullptr) dictionary["vote"] = static_cast<int64_t>(*p_value->vote);
    return dictionary;
}

Variant to_variant_PFCatalogTakedownItemReviewsRequest(const PFCatalogTakedownItemReviewsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->reviewsCount; ++i) {
            values.push_back(to_variant_PFCatalogReviewTakedown(p_value->reviews != nullptr ? p_value->reviews[i] : nullptr));
        }
        dictionary["reviews"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogUpdateCatalogConfigRequest(const PFCatalogUpdateCatalogConfigRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["config"] = to_variant_PFCatalogCatalogConfig(p_value->config);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFCatalogUpdateDraftItemRequest(const PFCatalogUpdateDraftItemRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["item"] = to_variant_PFCatalogCatalogItem(p_value->item);
    dictionary["publish"] = static_cast<bool>(p_value->publish);
    return dictionary;
}

Variant to_variant_PFCatalogUpdateDraftItemResponse(const PFCatalogUpdateDraftItemResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item"] = to_variant_PFCatalogCatalogItem(p_value->item);
    return dictionary;
}

Variant to_variant_PFCloudScriptExecuteCloudScriptRequest(const PFCloudScriptExecuteCloudScriptRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["function_name"] = p_value->functionName != nullptr ? String::utf8(p_value->functionName) : String();
    dictionary["function_parameter"] = p_value->functionParameter.stringValue != nullptr ? String::utf8(p_value->functionParameter.stringValue) : String();
    if (p_value->generatePlayStreamEvent != nullptr) dictionary["generate_play_stream_event"] = static_cast<bool>(*p_value->generatePlayStreamEvent);
    if (p_value->revisionSelection != nullptr) dictionary["revision_selection"] = static_cast<int64_t>(*p_value->revisionSelection);
    if (p_value->specificRevision != nullptr) dictionary["specific_revision"] = static_cast<int64_t>(*p_value->specificRevision);
    return dictionary;
}

Variant to_variant_PFCloudScriptScriptExecutionError(const PFCloudScriptScriptExecutionError *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["error"] = p_value->error != nullptr ? String::utf8(p_value->error) : String();
    dictionary["message"] = p_value->message != nullptr ? String::utf8(p_value->message) : String();
    dictionary["stack_trace"] = p_value->stackTrace != nullptr ? String::utf8(p_value->stackTrace) : String();
    return dictionary;
}

Variant to_variant_PFCloudScriptLogStatement(const PFCloudScriptLogStatement *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["data"] = p_value->data.stringValue != nullptr ? String::utf8(p_value->data.stringValue) : String();
    dictionary["level"] = p_value->level != nullptr ? String::utf8(p_value->level) : String();
    dictionary["message"] = p_value->message != nullptr ? String::utf8(p_value->message) : String();
    return dictionary;
}

Variant to_variant_PFCloudScriptExecuteCloudScriptResult(const PFCloudScriptExecuteCloudScriptResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["a_pi_requests_issued"] = static_cast<int64_t>(p_value->aPIRequestsIssued);
    dictionary["error"] = to_variant_PFCloudScriptScriptExecutionError(p_value->error);
    dictionary["execution_time_seconds"] = static_cast<double>(p_value->executionTimeSeconds);
    dictionary["function_name"] = p_value->functionName != nullptr ? String::utf8(p_value->functionName) : String();
    dictionary["function_result"] = p_value->functionResult.stringValue != nullptr ? String::utf8(p_value->functionResult.stringValue) : String();
    if (p_value->functionResultTooLarge != nullptr) dictionary["function_result_too_large"] = static_cast<bool>(*p_value->functionResultTooLarge);
    dictionary["http_requests_issued"] = static_cast<int64_t>(p_value->httpRequestsIssued);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->logsCount; ++i) {
            values.push_back(to_variant_PFCloudScriptLogStatement(p_value->logs != nullptr ? p_value->logs[i] : nullptr));
        }
        dictionary["logs"] = values;
    }
    if (p_value->logsTooLarge != nullptr) dictionary["logs_too_large"] = static_cast<bool>(*p_value->logsTooLarge);
    dictionary["memory_consumed_bytes"] = static_cast<int64_t>(p_value->memoryConsumedBytes);
    dictionary["processor_time_seconds"] = static_cast<double>(p_value->processorTimeSeconds);
    dictionary["revision"] = static_cast<int64_t>(p_value->revision);
    return dictionary;
}

Variant to_variant_PFCloudScriptExecuteEntityCloudScriptRequest(const PFCloudScriptExecuteEntityCloudScriptRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["function_name"] = p_value->functionName != nullptr ? String::utf8(p_value->functionName) : String();
    dictionary["function_parameter"] = p_value->functionParameter.stringValue != nullptr ? String::utf8(p_value->functionParameter.stringValue) : String();
    if (p_value->generatePlayStreamEvent != nullptr) dictionary["generate_play_stream_event"] = static_cast<bool>(*p_value->generatePlayStreamEvent);
    if (p_value->revisionSelection != nullptr) dictionary["revision_selection"] = static_cast<int64_t>(*p_value->revisionSelection);
    if (p_value->specificRevision != nullptr) dictionary["specific_revision"] = static_cast<int64_t>(*p_value->specificRevision);
    return dictionary;
}

Variant to_variant_PFCloudScriptExecuteFunctionRequest(const PFCloudScriptExecuteFunctionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["function_name"] = p_value->functionName != nullptr ? String::utf8(p_value->functionName) : String();
    dictionary["function_parameter"] = p_value->functionParameter.stringValue != nullptr ? String::utf8(p_value->functionParameter.stringValue) : String();
    if (p_value->generatePlayStreamEvent != nullptr) dictionary["generate_play_stream_event"] = static_cast<bool>(*p_value->generatePlayStreamEvent);
    return dictionary;
}

Variant to_variant_PFCloudScriptFunctionExecutionError(const PFCloudScriptFunctionExecutionError *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["error"] = p_value->error != nullptr ? String::utf8(p_value->error) : String();
    dictionary["message"] = p_value->message != nullptr ? String::utf8(p_value->message) : String();
    dictionary["stack_trace"] = p_value->stackTrace != nullptr ? String::utf8(p_value->stackTrace) : String();
    return dictionary;
}

Variant to_variant_PFCloudScriptExecuteFunctionResult(const PFCloudScriptExecuteFunctionResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["error"] = to_variant_PFCloudScriptFunctionExecutionError(p_value->error);
    dictionary["execution_time_milliseconds"] = static_cast<int64_t>(p_value->executionTimeMilliseconds);
    dictionary["function_name"] = p_value->functionName != nullptr ? String::utf8(p_value->functionName) : String();
    dictionary["function_result"] = p_value->functionResult.stringValue != nullptr ? String::utf8(p_value->functionResult.stringValue) : String();
    if (p_value->functionResultSize != nullptr) dictionary["function_result_size"] = static_cast<int64_t>(*p_value->functionResultSize);
    if (p_value->functionResultTooLarge != nullptr) dictionary["function_result_too_large"] = static_cast<bool>(*p_value->functionResultTooLarge);
    return dictionary;
}

Variant to_variant_PFDataAbortFileUploadsRequest(const PFDataAbortFileUploadsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->fileNamesCount; ++i) {
            values.push_back(p_value->fileNames != nullptr && p_value->fileNames[i] != nullptr ? String::utf8(p_value->fileNames[i]) : String());
        }
        dictionary["file_names"] = values;
    }
    if (p_value->profileVersion != nullptr) dictionary["profile_version"] = static_cast<int64_t>(*p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataAbortFileUploadsResponse(const PFDataAbortFileUploadsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataDeleteFilesRequest(const PFDataDeleteFilesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->fileNamesCount; ++i) {
            values.push_back(p_value->fileNames != nullptr && p_value->fileNames[i] != nullptr ? String::utf8(p_value->fileNames[i]) : String());
        }
        dictionary["file_names"] = values;
    }
    if (p_value->profileVersion != nullptr) dictionary["profile_version"] = static_cast<int64_t>(*p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataDeleteFilesResponse(const PFDataDeleteFilesResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataFinalizeFileUploadsRequest(const PFDataFinalizeFileUploadsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->fileNamesCount; ++i) {
            values.push_back(p_value->fileNames != nullptr && p_value->fileNames[i] != nullptr ? String::utf8(p_value->fileNames[i]) : String());
        }
        dictionary["file_names"] = values;
    }
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataGetFileMetadata(const PFDataGetFileMetadata *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["checksum"] = p_value->checksum != nullptr ? String::utf8(p_value->checksum) : String();
    dictionary["download_url"] = p_value->downloadUrl != nullptr ? String::utf8(p_value->downloadUrl) : String();
    dictionary["file_name"] = p_value->fileName != nullptr ? String::utf8(p_value->fileName) : String();
    dictionary["last_modified"] = static_cast<int64_t>(p_value->lastModified);
    dictionary["size"] = static_cast<int64_t>(p_value->size);
    return dictionary;
}

Variant to_variant_PFDataGetFileMetadataDictionaryEntry(const PFDataGetFileMetadataDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFDataGetFileMetadata(p_value->value);
    return dictionary;
}

Variant to_variant_PFDataFinalizeFileUploadsResponse(const PFDataFinalizeFileUploadsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->metadataCount; ++i) {
            const Variant entry_variant = to_variant_PFDataGetFileMetadataDictionaryEntry(&p_value->metadata[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["metadata"] = values;
    }
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataGetFilesRequest(const PFDataGetFilesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFDataGetFilesResponse(const PFDataGetFilesResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->metadataCount; ++i) {
            const Variant entry_variant = to_variant_PFDataGetFileMetadataDictionaryEntry(&p_value->metadata[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["metadata"] = values;
    }
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataGetObjectsRequest(const PFDataGetObjectsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    if (p_value->escapeObject != nullptr) dictionary["escape_object"] = static_cast<bool>(*p_value->escapeObject);
    return dictionary;
}

Variant to_variant_PFDataObjectResult(const PFDataObjectResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["data_object"] = p_value->dataObject.stringValue != nullptr ? String::utf8(p_value->dataObject.stringValue) : String();
    dictionary["escaped_data_object"] = p_value->escapedDataObject != nullptr ? String::utf8(p_value->escapedDataObject) : String();
    dictionary["object_name"] = p_value->objectName != nullptr ? String::utf8(p_value->objectName) : String();
    return dictionary;
}

Variant to_variant_PFDataObjectResultDictionaryEntry(const PFDataObjectResultDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFDataObjectResult(p_value->value);
    return dictionary;
}

Variant to_variant_PFDataGetObjectsResponse(const PFDataGetObjectsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->objectsCount; ++i) {
            const Variant entry_variant = to_variant_PFDataObjectResultDictionaryEntry(&p_value->objects[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["objects"] = values;
    }
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataInitiateFileUploadMetadata(const PFDataInitiateFileUploadMetadata *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["file_name"] = p_value->fileName != nullptr ? String::utf8(p_value->fileName) : String();
    dictionary["upload_url"] = p_value->uploadUrl != nullptr ? String::utf8(p_value->uploadUrl) : String();
    return dictionary;
}

Variant to_variant_PFDataInitiateFileUploadsRequest(const PFDataInitiateFileUploadsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->fileNamesCount; ++i) {
            values.push_back(p_value->fileNames != nullptr && p_value->fileNames[i] != nullptr ? String::utf8(p_value->fileNames[i]) : String());
        }
        dictionary["file_names"] = values;
    }
    if (p_value->profileVersion != nullptr) dictionary["profile_version"] = static_cast<int64_t>(*p_value->profileVersion);
    return dictionary;
}

Variant to_variant_PFDataInitiateFileUploadsResponse(const PFDataInitiateFileUploadsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->uploadDetailsCount; ++i) {
            values.push_back(to_variant_PFDataInitiateFileUploadMetadata(p_value->uploadDetails != nullptr ? p_value->uploadDetails[i] : nullptr));
        }
        dictionary["upload_details"] = values;
    }
    return dictionary;
}

Variant to_variant_PFDataSetObject(const PFDataSetObject *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["data_object"] = p_value->dataObject.stringValue != nullptr ? String::utf8(p_value->dataObject.stringValue) : String();
    if (p_value->deleteObject != nullptr) dictionary["delete_object"] = static_cast<bool>(*p_value->deleteObject);
    dictionary["escaped_data_object"] = p_value->escapedDataObject != nullptr ? String::utf8(p_value->escapedDataObject) : String();
    dictionary["object_name"] = p_value->objectName != nullptr ? String::utf8(p_value->objectName) : String();
    return dictionary;
}

Variant to_variant_PFDataSetObjectInfo(const PFDataSetObjectInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["object_name"] = p_value->objectName != nullptr ? String::utf8(p_value->objectName) : String();
    dictionary["operation_reason"] = p_value->operationReason != nullptr ? String::utf8(p_value->operationReason) : String();
    if (p_value->setResult != nullptr) dictionary["set_result"] = static_cast<int64_t>(*p_value->setResult);
    return dictionary;
}

Variant to_variant_PFDataSetObjectsRequest(const PFDataSetObjectsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    if (p_value->expectedProfileVersion != nullptr) dictionary["expected_profile_version"] = static_cast<int64_t>(*p_value->expectedProfileVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->objectsCount; ++i) {
            values.push_back(to_variant_PFDataSetObject(p_value->objects != nullptr ? p_value->objects[i] : nullptr));
        }
        dictionary["objects"] = values;
    }
    return dictionary;
}

Variant to_variant_PFDataSetObjectsResponse(const PFDataSetObjectsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->setResultsCount; ++i) {
            values.push_back(to_variant_PFDataSetObjectInfo(p_value->setResults != nullptr ? p_value->setResults[i] : nullptr));
        }
        dictionary["set_results"] = values;
    }
    return dictionary;
}

Variant to_variant_PFEntityKeyDictionaryEntry(const PFEntityKeyDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFEntityKey(p_value->value);
    return dictionary;
}

Variant to_variant_PFEntityStatisticValue(const PFEntityStatisticValue *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["metadata"] = p_value->metadata != nullptr ? String::utf8(p_value->metadata) : String();
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->scoresCount; ++i) {
            values.push_back(p_value->scores != nullptr && p_value->scores[i] != nullptr ? String::utf8(p_value->scores[i]) : String());
        }
        dictionary["scores"] = values;
    }
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    return dictionary;
}

Variant to_variant_PFEntityStatisticValueDictionaryEntry(const PFEntityStatisticValueDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFEntityStatisticValue(p_value->value);
    return dictionary;
}

Variant to_variant_PFExperimentationGetTreatmentAssignmentRequest(const PFExperimentationGetTreatmentAssignmentRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFVariable(const PFVariable *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["value"] = p_value->value != nullptr ? String::utf8(p_value->value) : String();
    return dictionary;
}

Variant to_variant_PFTreatmentAssignment(const PFTreatmentAssignment *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->variablesCount; ++i) {
            values.push_back(to_variant_PFVariable(p_value->variables != nullptr ? p_value->variables[i] : nullptr));
        }
        dictionary["variables"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->variantsCount; ++i) {
            values.push_back(p_value->variants != nullptr && p_value->variants[i] != nullptr ? String::utf8(p_value->variants[i]) : String());
        }
        dictionary["variants"] = values;
    }
    return dictionary;
}

Variant to_variant_PFExperimentationGetTreatmentAssignmentResult(const PFExperimentationGetTreatmentAssignmentResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["treatment_assignment"] = to_variant_PFTreatmentAssignment(p_value->treatmentAssignment);
    return dictionary;
}

Variant to_variant_PFFriendsAddFriendResult(const PFFriendsAddFriendResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["created"] = static_cast<bool>(p_value->created);
    return dictionary;
}

Variant to_variant_PFFriendsClientAddFriendRequest(const PFFriendsClientAddFriendRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["friend_email"] = p_value->friendEmail != nullptr ? String::utf8(p_value->friendEmail) : String();
    dictionary["friend_play_fab_id"] = p_value->friendPlayFabId != nullptr ? String::utf8(p_value->friendPlayFabId) : String();
    dictionary["friend_title_display_name"] = p_value->friendTitleDisplayName != nullptr ? String::utf8(p_value->friendTitleDisplayName) : String();
    dictionary["friend_username"] = p_value->friendUsername != nullptr ? String::utf8(p_value->friendUsername) : String();
    return dictionary;
}

Variant to_variant_PFFriendsClientGetFriendsListRequest(const PFFriendsClientGetFriendsListRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->externalPlatformFriends != nullptr) dictionary["external_platform_friends"] = static_cast<int64_t>(*p_value->externalPlatformFriends);
    dictionary["profile_constraints"] = to_variant_PFPlayerProfileViewConstraints(p_value->profileConstraints);
    dictionary["user"] = Variant();
    return dictionary;
}

Variant to_variant_PFFriendsClientRemoveFriendRequest(const PFFriendsClientRemoveFriendRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["friend_play_fab_id"] = p_value->friendPlayFabId != nullptr ? String::utf8(p_value->friendPlayFabId) : String();
    return dictionary;
}

Variant to_variant_PFFriendsClientSetFriendTagsRequest(const PFFriendsClientSetFriendTagsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["friend_play_fab_id"] = p_value->friendPlayFabId != nullptr ? String::utf8(p_value->friendPlayFabId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFFriendsFriendInfo(const PFFriendsFriendInfo *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["facebook_info"] = to_variant_PFUserFacebookInfo(p_value->facebookInfo);
    dictionary["friend_play_fab_id"] = p_value->friendPlayFabId != nullptr ? String::utf8(p_value->friendPlayFabId) : String();
    dictionary["game_center_info"] = to_variant_PFUserGameCenterInfo(p_value->gameCenterInfo);
    dictionary["profile"] = to_variant_PFPlayerProfileModel(p_value->profile);
    dictionary["psn_info"] = to_variant_PFUserPsnInfo(p_value->PSNInfo);
    dictionary["steam_info"] = to_variant_PFUserSteamInfo(p_value->steamInfo);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->tagsCount; ++i) {
            values.push_back(p_value->tags != nullptr && p_value->tags[i] != nullptr ? String::utf8(p_value->tags[i]) : String());
        }
        dictionary["tags"] = values;
    }
    dictionary["title_display_name"] = p_value->titleDisplayName != nullptr ? String::utf8(p_value->titleDisplayName) : String();
    dictionary["username"] = p_value->username != nullptr ? String::utf8(p_value->username) : String();
    dictionary["xbox_info"] = to_variant_PFUserXboxInfo(p_value->xboxInfo);
    return dictionary;
}

Variant to_variant_PFFriendsGetFriendsListResult(const PFFriendsGetFriendsListResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->friendsCount; ++i) {
            values.push_back(to_variant_PFFriendsFriendInfo(p_value->friends != nullptr ? p_value->friends[i] : nullptr));
        }
        dictionary["friends"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsAcceptGroupApplicationRequest(const PFGroupsAcceptGroupApplicationRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsAcceptGroupInvitationRequest(const PFGroupsAcceptGroupInvitationRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsAddMembersRequest(const PFGroupsAddMembersRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->membersCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->members != nullptr ? p_value->members[i] : nullptr));
        }
        dictionary["members"] = values;
    }
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsApplyToGroupRequest(const PFGroupsApplyToGroupRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->autoAcceptOutstandingInvite != nullptr) dictionary["auto_accept_outstanding_invite"] = static_cast<bool>(*p_value->autoAcceptOutstandingInvite);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsEntityWithLineage(const PFGroupsEntityWithLineage *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = to_variant_PFEntityKey(p_value->key);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->lineageCount; ++i) {
            const Variant entry_variant = to_variant_PFEntityKeyDictionaryEntry(&p_value->lineage[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["lineage"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsApplyToGroupResponse(const PFGroupsApplyToGroupResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFGroupsEntityWithLineage(p_value->entity);
    dictionary["expires"] = static_cast<int64_t>(p_value->expires);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsBlockEntityRequest(const PFGroupsBlockEntityRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsChangeMemberRoleRequest(const PFGroupsChangeMemberRoleRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["destination_role_id"] = p_value->destinationRoleId != nullptr ? String::utf8(p_value->destinationRoleId) : String();
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->membersCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->members != nullptr ? p_value->members[i] : nullptr));
        }
        dictionary["members"] = values;
    }
    dictionary["origin_role_id"] = p_value->originRoleId != nullptr ? String::utf8(p_value->originRoleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsCreateGroupRequest(const PFGroupsCreateGroupRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group_name"] = p_value->groupName != nullptr ? String::utf8(p_value->groupName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsCreateGroupResponse(const PFGroupsCreateGroupResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["admin_role_id"] = p_value->adminRoleId != nullptr ? String::utf8(p_value->adminRoleId) : String();
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["group_name"] = p_value->groupName != nullptr ? String::utf8(p_value->groupName) : String();
    dictionary["member_role_id"] = p_value->memberRoleId != nullptr ? String::utf8(p_value->memberRoleId) : String();
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->rolesCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->roles[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["roles"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsCreateGroupRoleRequest(const PFGroupsCreateGroupRoleRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    dictionary["role_name"] = p_value->roleName != nullptr ? String::utf8(p_value->roleName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsCreateGroupRoleResponse(const PFGroupsCreateGroupRoleResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    dictionary["role_name"] = p_value->roleName != nullptr ? String::utf8(p_value->roleName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsDeleteGroupRequest(const PFGroupsDeleteGroupRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsDeleteRoleRequest(const PFGroupsDeleteRoleRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsEntityMemberRole(const PFGroupsEntityMemberRole *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->membersCount; ++i) {
            values.push_back(to_variant_PFGroupsEntityWithLineage(p_value->members != nullptr ? p_value->members[i] : nullptr));
        }
        dictionary["members"] = values;
    }
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    dictionary["role_name"] = p_value->roleName != nullptr ? String::utf8(p_value->roleName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsGetGroupRequest(const PFGroupsGetGroupRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["group_name"] = p_value->groupName != nullptr ? String::utf8(p_value->groupName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsGetGroupResponse(const PFGroupsGetGroupResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["admin_role_id"] = p_value->adminRoleId != nullptr ? String::utf8(p_value->adminRoleId) : String();
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["group_name"] = p_value->groupName != nullptr ? String::utf8(p_value->groupName) : String();
    dictionary["member_role_id"] = p_value->memberRoleId != nullptr ? String::utf8(p_value->memberRoleId) : String();
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->rolesCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->roles[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["roles"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsGroupApplication(const PFGroupsGroupApplication *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFGroupsEntityWithLineage(p_value->entity);
    dictionary["expires"] = static_cast<int64_t>(p_value->expires);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsGroupBlock(const PFGroupsGroupBlock *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFGroupsEntityWithLineage(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsGroupInvitation(const PFGroupsGroupInvitation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["expires"] = static_cast<int64_t>(p_value->expires);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["invited_by_entity"] = to_variant_PFGroupsEntityWithLineage(p_value->invitedByEntity);
    dictionary["invited_entity"] = to_variant_PFGroupsEntityWithLineage(p_value->invitedEntity);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsGroupRole(const PFGroupsGroupRole *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    dictionary["role_name"] = p_value->roleName != nullptr ? String::utf8(p_value->roleName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsGroupWithRoles(const PFGroupsGroupWithRoles *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["group_name"] = p_value->groupName != nullptr ? String::utf8(p_value->groupName) : String();
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->rolesCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupRole(p_value->roles != nullptr ? p_value->roles[i] : nullptr));
        }
        dictionary["roles"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsInviteToGroupRequest(const PFGroupsInviteToGroupRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->autoAcceptOutstandingApplication != nullptr) dictionary["auto_accept_outstanding_application"] = static_cast<bool>(*p_value->autoAcceptOutstandingApplication);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsInviteToGroupResponse(const PFGroupsInviteToGroupResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["expires"] = static_cast<int64_t>(p_value->expires);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["invited_by_entity"] = to_variant_PFGroupsEntityWithLineage(p_value->invitedByEntity);
    dictionary["invited_entity"] = to_variant_PFGroupsEntityWithLineage(p_value->invitedEntity);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsIsMemberRequest(const PFGroupsIsMemberRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsIsMemberResponse(const PFGroupsIsMemberResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["is_member"] = static_cast<bool>(p_value->isMember);
    return dictionary;
}

Variant to_variant_PFGroupsListGroupApplicationsRequest(const PFGroupsListGroupApplicationsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsListGroupApplicationsResponse(const PFGroupsListGroupApplicationsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->applicationsCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupApplication(p_value->applications != nullptr ? p_value->applications[i] : nullptr));
        }
        dictionary["applications"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsListGroupBlocksRequest(const PFGroupsListGroupBlocksRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsListGroupBlocksResponse(const PFGroupsListGroupBlocksResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->blockedEntitiesCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupBlock(p_value->blockedEntities != nullptr ? p_value->blockedEntities[i] : nullptr));
        }
        dictionary["blocked_entities"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsListGroupInvitationsRequest(const PFGroupsListGroupInvitationsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsListGroupInvitationsResponse(const PFGroupsListGroupInvitationsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->invitationsCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupInvitation(p_value->invitations != nullptr ? p_value->invitations[i] : nullptr));
        }
        dictionary["invitations"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsListGroupMembersRequest(const PFGroupsListGroupMembersRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsListGroupMembersResponse(const PFGroupsListGroupMembersResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->membersCount; ++i) {
            values.push_back(to_variant_PFGroupsEntityMemberRole(p_value->members != nullptr ? p_value->members[i] : nullptr));
        }
        dictionary["members"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsListMembershipOpportunitiesRequest(const PFGroupsListMembershipOpportunitiesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFGroupsListMembershipOpportunitiesResponse(const PFGroupsListMembershipOpportunitiesResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->applicationsCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupApplication(p_value->applications != nullptr ? p_value->applications[i] : nullptr));
        }
        dictionary["applications"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->invitationsCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupInvitation(p_value->invitations != nullptr ? p_value->invitations[i] : nullptr));
        }
        dictionary["invitations"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsListMembershipRequest(const PFGroupsListMembershipRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFGroupsListMembershipResponse(const PFGroupsListMembershipResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->groupsCount; ++i) {
            values.push_back(to_variant_PFGroupsGroupWithRoles(p_value->groups != nullptr ? p_value->groups[i] : nullptr));
        }
        dictionary["groups"] = values;
    }
    return dictionary;
}

Variant to_variant_PFGroupsRemoveGroupApplicationRequest(const PFGroupsRemoveGroupApplicationRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsRemoveGroupInvitationRequest(const PFGroupsRemoveGroupInvitationRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsRemoveMembersRequest(const PFGroupsRemoveMembersRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->membersCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->members != nullptr ? p_value->members[i] : nullptr));
        }
        dictionary["members"] = values;
    }
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsUnblockEntityRequest(const PFGroupsUnblockEntityRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    return dictionary;
}

Variant to_variant_PFGroupsUpdateGroupRequest(const PFGroupsUpdateGroupRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["admin_role_id"] = p_value->adminRoleId != nullptr ? String::utf8(p_value->adminRoleId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->expectedProfileVersion != nullptr) dictionary["expected_profile_version"] = static_cast<int64_t>(*p_value->expectedProfileVersion);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["group_name"] = p_value->groupName != nullptr ? String::utf8(p_value->groupName) : String();
    dictionary["member_role_id"] = p_value->memberRoleId != nullptr ? String::utf8(p_value->memberRoleId) : String();
    return dictionary;
}

Variant to_variant_PFGroupsUpdateGroupResponse(const PFGroupsUpdateGroupResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["operation_reason"] = p_value->operationReason != nullptr ? String::utf8(p_value->operationReason) : String();
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    if (p_value->setResult != nullptr) dictionary["set_result"] = static_cast<int64_t>(*p_value->setResult);
    return dictionary;
}

Variant to_variant_PFGroupsUpdateGroupRoleRequest(const PFGroupsUpdateGroupRoleRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->expectedProfileVersion != nullptr) dictionary["expected_profile_version"] = static_cast<int64_t>(*p_value->expectedProfileVersion);
    dictionary["group"] = to_variant_PFEntityKey(p_value->group);
    dictionary["role_id"] = p_value->roleId != nullptr ? String::utf8(p_value->roleId) : String();
    dictionary["role_name"] = p_value->roleName != nullptr ? String::utf8(p_value->roleName) : String();
    return dictionary;
}

Variant to_variant_PFGroupsUpdateGroupRoleResponse(const PFGroupsUpdateGroupRoleResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["operation_reason"] = p_value->operationReason != nullptr ? String::utf8(p_value->operationReason) : String();
    dictionary["profile_version"] = static_cast<int64_t>(p_value->profileVersion);
    if (p_value->setResult != nullptr) dictionary["set_result"] = static_cast<int64_t>(*p_value->setResult);
    return dictionary;
}

Variant to_variant_PFInventoryAlternateId(const PFInventoryAlternateId *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    dictionary["value"] = p_value->value != nullptr ? String::utf8(p_value->value) : String();
    return dictionary;
}

Variant to_variant_PFInventoryInventoryItemReference(const PFInventoryInventoryItemReference *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["alternate_id"] = to_variant_PFInventoryAlternateId(p_value->alternateId);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["stack_id"] = p_value->stackId != nullptr ? String::utf8(p_value->stackId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryInitialValues(const PFInventoryInitialValues *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["display_properties"] = p_value->displayProperties.stringValue != nullptr ? String::utf8(p_value->displayProperties.stringValue) : String();
    return dictionary;
}

Variant to_variant_PFInventoryAddInventoryItemsOperation(const PFInventoryAddInventoryItemsOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    dictionary["new_stack_values"] = to_variant_PFInventoryInitialValues(p_value->newStackValues);
    return dictionary;
}

Variant to_variant_PFInventoryAddInventoryItemsRequest(const PFInventoryAddInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    dictionary["new_stack_values"] = to_variant_PFInventoryInitialValues(p_value->newStackValues);
    return dictionary;
}

Variant to_variant_PFInventoryAddInventoryItemsResponse(const PFInventoryAddInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryDeleteInventoryCollectionRequest(const PFInventoryDeleteInventoryCollectionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    return dictionary;
}

Variant to_variant_PFInventoryDeleteInventoryItemsOperation(const PFInventoryDeleteInventoryItemsOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    return dictionary;
}

Variant to_variant_PFInventoryDeleteInventoryItemsRequest(const PFInventoryDeleteInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    return dictionary;
}

Variant to_variant_PFInventoryDeleteInventoryItemsResponse(const PFInventoryDeleteInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryPurchasePriceAmount(const PFInventoryPurchasePriceAmount *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["amount"] = static_cast<int64_t>(p_value->amount);
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["stack_id"] = p_value->stackId != nullptr ? String::utf8(p_value->stackId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryPurchaseInventoryItemsOperation(const PFInventoryPurchaseInventoryItemsOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["delete_empty_stacks"] = static_cast<bool>(p_value->deleteEmptyStacks);
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    dictionary["new_stack_values"] = to_variant_PFInventoryInitialValues(p_value->newStackValues);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->priceAmountsCount; ++i) {
            values.push_back(to_variant_PFInventoryPurchasePriceAmount(p_value->priceAmounts != nullptr ? p_value->priceAmounts[i] : nullptr));
        }
        dictionary["price_amounts"] = values;
    }
    dictionary["store_id"] = p_value->storeId != nullptr ? String::utf8(p_value->storeId) : String();
    return dictionary;
}

Variant to_variant_PFInventorySubtractInventoryItemsOperation(const PFInventorySubtractInventoryItemsOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["delete_empty_stacks"] = static_cast<bool>(p_value->deleteEmptyStacks);
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    return dictionary;
}

Variant to_variant_PFInventoryTransferInventoryItemsOperation(const PFInventoryTransferInventoryItemsOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["delete_empty_stacks"] = static_cast<bool>(p_value->deleteEmptyStacks);
    dictionary["giving_item"] = to_variant_PFInventoryInventoryItemReference(p_value->givingItem);
    dictionary["new_stack_values"] = to_variant_PFInventoryInitialValues(p_value->newStackValues);
    dictionary["receiving_item"] = to_variant_PFInventoryInventoryItemReference(p_value->receivingItem);
    return dictionary;
}

Variant to_variant_PFInventoryInventoryItem(const PFInventoryInventoryItem *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["display_properties"] = p_value->displayProperties.stringValue != nullptr ? String::utf8(p_value->displayProperties.stringValue) : String();
    if (p_value->expirationDate != nullptr) dictionary["expiration_date"] = static_cast<int64_t>(*p_value->expirationDate);
    dictionary["id"] = p_value->id != nullptr ? String::utf8(p_value->id) : String();
    dictionary["stack_id"] = p_value->stackId != nullptr ? String::utf8(p_value->stackId) : String();
    if (p_value->startDate != nullptr) dictionary["start_date"] = static_cast<int64_t>(*p_value->startDate);
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    return dictionary;
}

Variant to_variant_PFInventoryUpdateInventoryItemsOperation(const PFInventoryUpdateInventoryItemsOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item"] = to_variant_PFInventoryInventoryItem(p_value->item);
    return dictionary;
}

Variant to_variant_PFInventoryInventoryOperation(const PFInventoryInventoryOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["add"] = to_variant_PFInventoryAddInventoryItemsOperation(p_value->add);
    dictionary["delete_op"] = to_variant_PFInventoryDeleteInventoryItemsOperation(p_value->deleteOp);
    dictionary["purchase"] = to_variant_PFInventoryPurchaseInventoryItemsOperation(p_value->purchase);
    dictionary["subtract"] = to_variant_PFInventorySubtractInventoryItemsOperation(p_value->subtract);
    dictionary["transfer"] = to_variant_PFInventoryTransferInventoryItemsOperation(p_value->transfer);
    dictionary["update"] = to_variant_PFInventoryUpdateInventoryItemsOperation(p_value->update);
    return dictionary;
}

Variant to_variant_PFInventoryExecuteInventoryOperationsRequest(const PFInventoryExecuteInventoryOperationsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->operationsCount; ++i) {
            values.push_back(to_variant_PFInventoryInventoryOperation(p_value->operations != nullptr ? p_value->operations[i] : nullptr));
        }
        dictionary["operations"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryExecuteInventoryOperationsResponse(const PFInventoryExecuteInventoryOperationsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryExecuteTransferOperationsRequest(const PFInventoryExecuteTransferOperationsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["giving_collection_id"] = p_value->givingCollectionId != nullptr ? String::utf8(p_value->givingCollectionId) : String();
    dictionary["giving_entity"] = to_variant_PFEntityKey(p_value->givingEntity);
    dictionary["giving_e_tag"] = p_value->givingETag != nullptr ? String::utf8(p_value->givingETag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->operationsCount; ++i) {
            values.push_back(to_variant_PFInventoryTransferInventoryItemsOperation(p_value->operations != nullptr ? p_value->operations[i] : nullptr));
        }
        dictionary["operations"] = values;
    }
    dictionary["receiving_collection_id"] = p_value->receivingCollectionId != nullptr ? String::utf8(p_value->receivingCollectionId) : String();
    dictionary["receiving_entity"] = to_variant_PFEntityKey(p_value->receivingEntity);
    return dictionary;
}

Variant to_variant_PFInventoryExecuteTransferOperationsResponse(const PFInventoryExecuteTransferOperationsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["giving_e_tag"] = p_value->givingETag != nullptr ? String::utf8(p_value->givingETag) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->givingTransactionIdsCount; ++i) {
            values.push_back(p_value->givingTransactionIds != nullptr && p_value->givingTransactionIds[i] != nullptr ? String::utf8(p_value->givingTransactionIds[i]) : String());
        }
        dictionary["giving_transaction_ids"] = values;
    }
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["operation_status"] = p_value->operationStatus != nullptr ? String::utf8(p_value->operationStatus) : String();
    dictionary["operation_token"] = p_value->operationToken != nullptr ? String::utf8(p_value->operationToken) : String();
    dictionary["receiving_e_tag"] = p_value->receivingETag != nullptr ? String::utf8(p_value->receivingETag) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->receivingTransactionIdsCount; ++i) {
            values.push_back(p_value->receivingTransactionIds != nullptr && p_value->receivingTransactionIds[i] != nullptr ? String::utf8(p_value->receivingTransactionIds[i]) : String());
        }
        dictionary["receiving_transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryGetInventoryCollectionIdsRequest(const PFInventoryGetInventoryCollectionIdsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFInventoryGetInventoryCollectionIdsResponse(const PFInventoryGetInventoryCollectionIdsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->collectionIdsCount; ++i) {
            values.push_back(p_value->collectionIds != nullptr && p_value->collectionIds[i] != nullptr ? String::utf8(p_value->collectionIds[i]) : String());
        }
        dictionary["collection_ids"] = values;
    }
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    return dictionary;
}

Variant to_variant_PFInventoryGetInventoryItemsRequest(const PFInventoryGetInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["filter"] = p_value->filter != nullptr ? String::utf8(p_value->filter) : String();
    return dictionary;
}

Variant to_variant_PFInventoryGetInventoryItemsResponse(const PFInventoryGetInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->itemsCount; ++i) {
            values.push_back(to_variant_PFInventoryInventoryItem(p_value->items != nullptr ? p_value->items[i] : nullptr));
        }
        dictionary["items"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryGetInventoryOperationStatusRequest(const PFInventoryGetInventoryOperationStatusRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["operation_token"] = p_value->operationToken != nullptr ? String::utf8(p_value->operationToken) : String();
    return dictionary;
}

Variant to_variant_PFInventoryGetInventoryOperationStatusResponse(const PFInventoryGetInventoryOperationStatusResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["operation_status"] = p_value->operationStatus != nullptr ? String::utf8(p_value->operationStatus) : String();
    return dictionary;
}

Variant to_variant_PFInventoryGetTransactionHistoryRequest(const PFInventoryGetTransactionHistoryRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    dictionary["count"] = static_cast<int64_t>(p_value->count);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["filter"] = p_value->filter != nullptr ? String::utf8(p_value->filter) : String();
    dictionary["order_by"] = p_value->orderBy != nullptr ? String::utf8(p_value->orderBy) : String();
    return dictionary;
}

Variant to_variant_PFInventoryTransactionClawbackDetails(const PFInventoryTransactionClawbackDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["transaction_id_clawedback"] = p_value->transactionIdClawedback != nullptr ? String::utf8(p_value->transactionIdClawedback) : String();
    return dictionary;
}

Variant to_variant_PFInventoryTransactionOperation(const PFInventoryTransactionOperation *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["item_friendly_id"] = p_value->itemFriendlyId != nullptr ? String::utf8(p_value->itemFriendlyId) : String();
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["item_type"] = p_value->itemType != nullptr ? String::utf8(p_value->itemType) : String();
    dictionary["stack_id"] = p_value->stackId != nullptr ? String::utf8(p_value->stackId) : String();
    dictionary["type"] = p_value->type != nullptr ? String::utf8(p_value->type) : String();
    return dictionary;
}

Variant to_variant_PFInventoryTransactionPurchaseDetails(const PFInventoryTransactionPurchaseDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["item_friendly_id"] = p_value->itemFriendlyId != nullptr ? String::utf8(p_value->itemFriendlyId) : String();
    dictionary["item_id"] = p_value->itemId != nullptr ? String::utf8(p_value->itemId) : String();
    dictionary["store_friendly_id"] = p_value->storeFriendlyId != nullptr ? String::utf8(p_value->storeFriendlyId) : String();
    dictionary["store_id"] = p_value->storeId != nullptr ? String::utf8(p_value->storeId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryTransactionRedeemDetails(const PFInventoryTransactionRedeemDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["marketplace"] = p_value->marketplace != nullptr ? String::utf8(p_value->marketplace) : String();
    dictionary["marketplace_transaction_id"] = p_value->marketplaceTransactionId != nullptr ? String::utf8(p_value->marketplaceTransactionId) : String();
    dictionary["offer_id"] = p_value->offerId != nullptr ? String::utf8(p_value->offerId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryTransactionTransferDetails(const PFInventoryTransactionTransferDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["giving_collection_id"] = p_value->givingCollectionId != nullptr ? String::utf8(p_value->givingCollectionId) : String();
    dictionary["giving_entity"] = to_variant_PFEntityKey(p_value->givingEntity);
    dictionary["receiving_collection_id"] = p_value->receivingCollectionId != nullptr ? String::utf8(p_value->receivingCollectionId) : String();
    dictionary["receiving_entity"] = to_variant_PFEntityKey(p_value->receivingEntity);
    dictionary["transfer_id"] = p_value->transferId != nullptr ? String::utf8(p_value->transferId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryTransaction(const PFInventoryTransaction *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["api_name"] = p_value->apiName != nullptr ? String::utf8(p_value->apiName) : String();
    dictionary["clawback_details"] = to_variant_PFInventoryTransactionClawbackDetails(p_value->clawbackDetails);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["item_type"] = p_value->itemType != nullptr ? String::utf8(p_value->itemType) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->operationsCount; ++i) {
            values.push_back(to_variant_PFInventoryTransactionOperation(p_value->operations != nullptr ? p_value->operations[i] : nullptr));
        }
        dictionary["operations"] = values;
    }
    dictionary["operation_type"] = p_value->operationType != nullptr ? String::utf8(p_value->operationType) : String();
    dictionary["purchase_details"] = to_variant_PFInventoryTransactionPurchaseDetails(p_value->purchaseDetails);
    dictionary["redeem_details"] = to_variant_PFInventoryTransactionRedeemDetails(p_value->redeemDetails);
    dictionary["timestamp"] = static_cast<int64_t>(p_value->timestamp);
    dictionary["transaction_id"] = p_value->transactionId != nullptr ? String::utf8(p_value->transactionId) : String();
    dictionary["transfer_details"] = to_variant_PFInventoryTransactionTransferDetails(p_value->transferDetails);
    return dictionary;
}

Variant to_variant_PFInventoryGetTransactionHistoryResponse(const PFInventoryGetTransactionHistoryResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["continuation_token"] = p_value->continuationToken != nullptr ? String::utf8(p_value->continuationToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionsCount; ++i) {
            values.push_back(to_variant_PFInventoryTransaction(p_value->transactions != nullptr ? p_value->transactions[i] : nullptr));
        }
        dictionary["transactions"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryGooglePlayProductPurchase(const PFInventoryGooglePlayProductPurchase *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["product_id"] = p_value->productId != nullptr ? String::utf8(p_value->productId) : String();
    dictionary["token"] = p_value->token != nullptr ? String::utf8(p_value->token) : String();
    return dictionary;
}

Variant to_variant_PFInventoryPurchaseInventoryItemsRequest(const PFInventoryPurchaseInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["delete_empty_stacks"] = static_cast<bool>(p_value->deleteEmptyStacks);
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    dictionary["new_stack_values"] = to_variant_PFInventoryInitialValues(p_value->newStackValues);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->priceAmountsCount; ++i) {
            values.push_back(to_variant_PFInventoryPurchasePriceAmount(p_value->priceAmounts != nullptr ? p_value->priceAmounts[i] : nullptr));
        }
        dictionary["price_amounts"] = values;
    }
    dictionary["store_id"] = p_value->storeId != nullptr ? String::utf8(p_value->storeId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryPurchaseInventoryItemsResponse(const PFInventoryPurchaseInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemAppleAppStoreInventoryItemsRequest(const PFInventoryRedeemAppleAppStoreInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["receipt"] = p_value->receipt != nullptr ? String::utf8(p_value->receipt) : String();
    return dictionary;
}

Variant to_variant_PFInventoryRedemptionFailure(const PFInventoryRedemptionFailure *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["failure_code"] = p_value->failureCode != nullptr ? String::utf8(p_value->failureCode) : String();
    dictionary["failure_details"] = p_value->failureDetails != nullptr ? String::utf8(p_value->failureDetails) : String();
    dictionary["marketplace_alternate_id"] = p_value->marketplaceAlternateId != nullptr ? String::utf8(p_value->marketplaceAlternateId) : String();
    dictionary["marketplace_transaction_id"] = p_value->marketplaceTransactionId != nullptr ? String::utf8(p_value->marketplaceTransactionId) : String();
    return dictionary;
}

Variant to_variant_PFInventoryRedemptionSuccess(const PFInventoryRedemptionSuccess *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->expirationTimestamp != nullptr) dictionary["expiration_timestamp"] = static_cast<int64_t>(*p_value->expirationTimestamp);
    dictionary["marketplace_alternate_id"] = p_value->marketplaceAlternateId != nullptr ? String::utf8(p_value->marketplaceAlternateId) : String();
    dictionary["marketplace_transaction_id"] = p_value->marketplaceTransactionId != nullptr ? String::utf8(p_value->marketplaceTransactionId) : String();
    dictionary["success_timestamp"] = static_cast<int64_t>(p_value->successTimestamp);
    return dictionary;
}

Variant to_variant_PFInventoryRedeemAppleAppStoreInventoryItemsResponse(const PFInventoryRedeemAppleAppStoreInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->failedCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionFailure(p_value->failed != nullptr ? p_value->failed[i] : nullptr));
        }
        dictionary["failed"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->succeededCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionSuccess(p_value->succeeded != nullptr ? p_value->succeeded[i] : nullptr));
        }
        dictionary["succeeded"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemGooglePlayInventoryItemsRequest(const PFInventoryRedeemGooglePlayInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->purchasesCount; ++i) {
            values.push_back(to_variant_PFInventoryGooglePlayProductPurchase(p_value->purchases != nullptr ? p_value->purchases[i] : nullptr));
        }
        dictionary["purchases"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemGooglePlayInventoryItemsResponse(const PFInventoryRedeemGooglePlayInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->failedCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionFailure(p_value->failed != nullptr ? p_value->failed[i] : nullptr));
        }
        dictionary["failed"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->succeededCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionSuccess(p_value->succeeded != nullptr ? p_value->succeeded[i] : nullptr));
        }
        dictionary["succeeded"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemMicrosoftStoreInventoryItemsRequest(const PFInventoryRedeemMicrosoftStoreInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["user"] = Variant();
    return dictionary;
}

Variant to_variant_PFInventoryRedeemMicrosoftStoreInventoryItemsResponse(const PFInventoryRedeemMicrosoftStoreInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->failedCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionFailure(p_value->failed != nullptr ? p_value->failed[i] : nullptr));
        }
        dictionary["failed"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->succeededCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionSuccess(p_value->succeeded != nullptr ? p_value->succeeded[i] : nullptr));
        }
        dictionary["succeeded"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemNintendoEShopInventoryItemsRequest(const PFInventoryRedeemNintendoEShopInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["nintendo_service_account_id_token"] = p_value->nintendoServiceAccountIdToken != nullptr ? String::utf8(p_value->nintendoServiceAccountIdToken) : String();
    return dictionary;
}

Variant to_variant_PFInventoryRedeemNintendoEShopInventoryItemsResponse(const PFInventoryRedeemNintendoEShopInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->failedCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionFailure(p_value->failed != nullptr ? p_value->failed[i] : nullptr));
        }
        dictionary["failed"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->succeededCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionSuccess(p_value->succeeded != nullptr ? p_value->succeeded[i] : nullptr));
        }
        dictionary["succeeded"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemPlayStationStoreInventoryItemsRequest(const PFInventoryRedeemPlayStationStoreInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["authorization_code"] = p_value->authorizationCode != nullptr ? String::utf8(p_value->authorizationCode) : String();
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["redirect_uri"] = p_value->redirectUri != nullptr ? String::utf8(p_value->redirectUri) : String();
    dictionary["service_label"] = p_value->serviceLabel != nullptr ? String::utf8(p_value->serviceLabel) : String();
    return dictionary;
}

Variant to_variant_PFInventoryRedeemPlayStationStoreInventoryItemsResponse(const PFInventoryRedeemPlayStationStoreInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->failedCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionFailure(p_value->failed != nullptr ? p_value->failed[i] : nullptr));
        }
        dictionary["failed"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->succeededCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionSuccess(p_value->succeeded != nullptr ? p_value->succeeded[i] : nullptr));
        }
        dictionary["succeeded"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryRedeemSteamInventoryItemsRequest(const PFInventoryRedeemSteamInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFInventoryRedeemSteamInventoryItemsResponse(const PFInventoryRedeemSteamInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->failedCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionFailure(p_value->failed != nullptr ? p_value->failed[i] : nullptr));
        }
        dictionary["failed"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->succeededCount; ++i) {
            values.push_back(to_variant_PFInventoryRedemptionSuccess(p_value->succeeded != nullptr ? p_value->succeeded[i] : nullptr));
        }
        dictionary["succeeded"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventorySubtractInventoryItemsRequest(const PFInventorySubtractInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["delete_empty_stacks"] = static_cast<bool>(p_value->deleteEmptyStacks);
    if (p_value->durationInSeconds != nullptr) dictionary["duration_in_seconds"] = static_cast<double>(*p_value->durationInSeconds);
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["item"] = to_variant_PFInventoryInventoryItemReference(p_value->item);
    return dictionary;
}

Variant to_variant_PFInventorySubtractInventoryItemsResponse(const PFInventorySubtractInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryTransferInventoryItemsRequest(const PFInventoryTransferInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->amount != nullptr) dictionary["amount"] = static_cast<int64_t>(*p_value->amount);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["delete_empty_stacks"] = static_cast<bool>(p_value->deleteEmptyStacks);
    dictionary["giving_collection_id"] = p_value->givingCollectionId != nullptr ? String::utf8(p_value->givingCollectionId) : String();
    dictionary["giving_entity"] = to_variant_PFEntityKey(p_value->givingEntity);
    dictionary["giving_e_tag"] = p_value->givingETag != nullptr ? String::utf8(p_value->givingETag) : String();
    dictionary["giving_item"] = to_variant_PFInventoryInventoryItemReference(p_value->givingItem);
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["new_stack_values"] = to_variant_PFInventoryInitialValues(p_value->newStackValues);
    dictionary["receiving_collection_id"] = p_value->receivingCollectionId != nullptr ? String::utf8(p_value->receivingCollectionId) : String();
    dictionary["receiving_entity"] = to_variant_PFEntityKey(p_value->receivingEntity);
    dictionary["receiving_item"] = to_variant_PFInventoryInventoryItemReference(p_value->receivingItem);
    return dictionary;
}

Variant to_variant_PFInventoryTransferInventoryItemsResponse(const PFInventoryTransferInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["giving_e_tag"] = p_value->givingETag != nullptr ? String::utf8(p_value->givingETag) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->givingTransactionIdsCount; ++i) {
            values.push_back(p_value->givingTransactionIds != nullptr && p_value->givingTransactionIds[i] != nullptr ? String::utf8(p_value->givingTransactionIds[i]) : String());
        }
        dictionary["giving_transaction_ids"] = values;
    }
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["operation_status"] = p_value->operationStatus != nullptr ? String::utf8(p_value->operationStatus) : String();
    dictionary["operation_token"] = p_value->operationToken != nullptr ? String::utf8(p_value->operationToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->receivingTransactionIdsCount; ++i) {
            values.push_back(p_value->receivingTransactionIds != nullptr && p_value->receivingTransactionIds[i] != nullptr ? String::utf8(p_value->receivingTransactionIds[i]) : String());
        }
        dictionary["receiving_transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFInventoryUpdateInventoryItemsRequest(const PFInventoryUpdateInventoryItemsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["collection_id"] = p_value->collectionId != nullptr ? String::utf8(p_value->collectionId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    dictionary["item"] = to_variant_PFInventoryInventoryItem(p_value->item);
    return dictionary;
}

Variant to_variant_PFInventoryUpdateInventoryItemsResponse(const PFInventoryUpdateInventoryItemsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["e_tag"] = p_value->eTag != nullptr ? String::utf8(p_value->eTag) : String();
    dictionary["idempotency_id"] = p_value->idempotencyId != nullptr ? String::utf8(p_value->idempotencyId) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->transactionIdsCount; ++i) {
            values.push_back(p_value->transactionIds != nullptr && p_value->transactionIds[i] != nullptr ? String::utf8(p_value->transactionIds[i]) : String());
        }
        dictionary["transaction_ids"] = values;
    }
    return dictionary;
}

Variant to_variant_PFLocalizationGetLanguageListRequest(const PFLocalizationGetLanguageListRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    return dictionary;
}

Variant to_variant_PFLocalizationGetLanguageListResponse(const PFLocalizationGetLanguageListResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->languageListCount; ++i) {
            values.push_back(p_value->languageList != nullptr && p_value->languageList[i] != nullptr ? String::utf8(p_value->languageList[i]) : String());
        }
        dictionary["language_list"] = values;
    }
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest(const PFPlayerDataManagementClientDeletePlayerCustomPropertiesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->expectedPropertiesVersion != nullptr) dictionary["expected_properties_version"] = static_cast<int64_t>(*p_value->expectedPropertiesVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->propertyNamesCount; ++i) {
            values.push_back(p_value->propertyNames != nullptr && p_value->propertyNames[i] != nullptr ? String::utf8(p_value->propertyNames[i]) : String());
        }
        dictionary["property_names"] = values;
    }
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementDeletedPropertyDetails(const PFPlayerDataManagementDeletedPropertyDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["was_deleted"] = static_cast<bool>(p_value->wasDeleted);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult(const PFPlayerDataManagementClientDeletePlayerCustomPropertiesResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->deletedPropertiesCount; ++i) {
            values.push_back(to_variant_PFPlayerDataManagementDeletedPropertyDetails(p_value->deletedProperties != nullptr ? p_value->deletedProperties[i] : nullptr));
        }
        dictionary["deleted_properties"] = values;
    }
    dictionary["properties_version"] = static_cast<int64_t>(p_value->propertiesVersion);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientGetPlayerCustomPropertyRequest(const PFPlayerDataManagementClientGetPlayerCustomPropertyRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["property_name"] = p_value->propertyName != nullptr ? String::utf8(p_value->propertyName) : String();
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementCustomPropertyDetails(const PFPlayerDataManagementCustomPropertyDetails *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["value"] = p_value->value.stringValue != nullptr ? String::utf8(p_value->value.stringValue) : String();
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientGetPlayerCustomPropertyResult(const PFPlayerDataManagementClientGetPlayerCustomPropertyResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["properties_version"] = static_cast<int64_t>(p_value->propertiesVersion);
    dictionary["property"] = to_variant_PFPlayerDataManagementCustomPropertyDetails(p_value->property);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientGetUserDataResult(const PFPlayerDataManagementClientGetUserDataResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            const Variant entry_variant = to_variant_PFUserDataRecordDictionaryEntry(&p_value->data[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["data"] = values;
    }
    dictionary["data_version"] = static_cast<int64_t>(p_value->dataVersion);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientListPlayerCustomPropertiesResult(const PFPlayerDataManagementClientListPlayerCustomPropertiesResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->propertiesCount; ++i) {
            values.push_back(to_variant_PFPlayerDataManagementCustomPropertyDetails(p_value->properties != nullptr ? p_value->properties[i] : nullptr));
        }
        dictionary["properties"] = values;
    }
    dictionary["properties_version"] = static_cast<int64_t>(p_value->propertiesVersion);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementUpdateProperty(const PFPlayerDataManagementUpdateProperty *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["value"] = p_value->value.stringValue != nullptr ? String::utf8(p_value->value.stringValue) : String();
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest(const PFPlayerDataManagementClientUpdatePlayerCustomPropertiesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->expectedPropertiesVersion != nullptr) dictionary["expected_properties_version"] = static_cast<int64_t>(*p_value->expectedPropertiesVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->propertiesCount; ++i) {
            values.push_back(to_variant_PFPlayerDataManagementUpdateProperty(p_value->properties != nullptr ? p_value->properties[i] : nullptr));
        }
        dictionary["properties"] = values;
    }
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult(const PFPlayerDataManagementClientUpdatePlayerCustomPropertiesResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["properties_version"] = static_cast<int64_t>(p_value->propertiesVersion);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementClientUpdateUserDataRequest(const PFPlayerDataManagementClientUpdateUserDataRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->data[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["data"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->keysToRemoveCount; ++i) {
            values.push_back(p_value->keysToRemove != nullptr && p_value->keysToRemove[i] != nullptr ? String::utf8(p_value->keysToRemove[i]) : String());
        }
        dictionary["keys_to_remove"] = values;
    }
    if (p_value->permission != nullptr) dictionary["permission"] = static_cast<int64_t>(*p_value->permission);
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementGetUserDataRequest(const PFPlayerDataManagementGetUserDataRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->ifChangedFromDataVersion != nullptr) dictionary["if_changed_from_data_version"] = static_cast<int64_t>(*p_value->ifChangedFromDataVersion);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->keysCount; ++i) {
            values.push_back(p_value->keys != nullptr && p_value->keys[i] != nullptr ? String::utf8(p_value->keys[i]) : String());
        }
        dictionary["keys"] = values;
    }
    dictionary["play_fab_id"] = p_value->playFabId != nullptr ? String::utf8(p_value->playFabId) : String();
    return dictionary;
}

Variant to_variant_PFPlayerDataManagementUpdateUserDataResult(const PFPlayerDataManagementUpdateUserDataResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["data_version"] = static_cast<int64_t>(p_value->dataVersion);
    return dictionary;
}

Variant to_variant_PFProfilesEntityDataObject(const PFProfilesEntityDataObject *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["data_object"] = p_value->dataObject.stringValue != nullptr ? String::utf8(p_value->dataObject.stringValue) : String();
    dictionary["escaped_data_object"] = p_value->escapedDataObject != nullptr ? String::utf8(p_value->escapedDataObject) : String();
    dictionary["object_name"] = p_value->objectName != nullptr ? String::utf8(p_value->objectName) : String();
    return dictionary;
}

Variant to_variant_PFProfilesEntityDataObjectDictionaryEntry(const PFProfilesEntityDataObjectDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFProfilesEntityDataObject(p_value->value);
    return dictionary;
}

Variant to_variant_PFProfilesEntityPermissionStatement(const PFProfilesEntityPermissionStatement *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["action"] = p_value->action != nullptr ? String::utf8(p_value->action) : String();
    dictionary["comment"] = p_value->comment != nullptr ? String::utf8(p_value->comment) : String();
    dictionary["condition"] = p_value->condition.stringValue != nullptr ? String::utf8(p_value->condition.stringValue) : String();
    dictionary["effect"] = static_cast<int64_t>(p_value->effect);
    dictionary["principal"] = p_value->principal.stringValue != nullptr ? String::utf8(p_value->principal.stringValue) : String();
    dictionary["resource"] = p_value->resource != nullptr ? String::utf8(p_value->resource) : String();
    return dictionary;
}

Variant to_variant_PFProfilesEntityProfileFileMetadata(const PFProfilesEntityProfileFileMetadata *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["checksum"] = p_value->checksum != nullptr ? String::utf8(p_value->checksum) : String();
    dictionary["file_name"] = p_value->fileName != nullptr ? String::utf8(p_value->fileName) : String();
    dictionary["last_modified"] = static_cast<int64_t>(p_value->lastModified);
    dictionary["size"] = static_cast<int64_t>(p_value->size);
    return dictionary;
}

Variant to_variant_PFProfilesEntityProfileFileMetadataDictionaryEntry(const PFProfilesEntityProfileFileMetadataDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFProfilesEntityProfileFileMetadata(p_value->value);
    return dictionary;
}

Variant to_variant_PFProfilesEntityProfileBody(const PFProfilesEntityProfileBody *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["avatar_url"] = p_value->avatarUrl != nullptr ? String::utf8(p_value->avatarUrl) : String();
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["display_name"] = p_value->displayName != nullptr ? String::utf8(p_value->displayName) : String();
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    dictionary["entity_chain"] = p_value->entityChain != nullptr ? String::utf8(p_value->entityChain) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->experimentVariantsCount; ++i) {
            values.push_back(p_value->experimentVariants != nullptr && p_value->experimentVariants[i] != nullptr ? String::utf8(p_value->experimentVariants[i]) : String());
        }
        dictionary["experiment_variants"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->filesCount; ++i) {
            const Variant entry_variant = to_variant_PFProfilesEntityProfileFileMetadataDictionaryEntry(&p_value->files[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["files"] = values;
    }
    dictionary["language"] = p_value->language != nullptr ? String::utf8(p_value->language) : String();
    dictionary["lineage"] = to_variant_PFEntityLineage(p_value->lineage);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->objectsCount; ++i) {
            const Variant entry_variant = to_variant_PFProfilesEntityDataObjectDictionaryEntry(&p_value->objects[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["objects"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->permissionsCount; ++i) {
            values.push_back(to_variant_PFProfilesEntityPermissionStatement(p_value->permissions != nullptr ? p_value->permissions[i] : nullptr));
        }
        dictionary["permissions"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            const Variant entry_variant = to_variant_PFEntityStatisticValueDictionaryEntry(&p_value->statistics[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["statistics"] = values;
    }
    dictionary["version_number"] = static_cast<int64_t>(p_value->versionNumber);
    return dictionary;
}

Variant to_variant_PFProfilesGetEntityProfileRequest(const PFProfilesGetEntityProfileRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->dataAsObject != nullptr) dictionary["data_as_object"] = static_cast<bool>(*p_value->dataAsObject);
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFProfilesGetEntityProfileResponse(const PFProfilesGetEntityProfileResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["profile"] = to_variant_PFProfilesEntityProfileBody(p_value->profile);
    return dictionary;
}

Variant to_variant_PFProfilesGetEntityProfilesRequest(const PFProfilesGetEntityProfilesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->dataAsObject != nullptr) dictionary["data_as_object"] = static_cast<bool>(*p_value->dataAsObject);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->entitiesCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->entities != nullptr ? p_value->entities[i] : nullptr));
        }
        dictionary["entities"] = values;
    }
    return dictionary;
}

Variant to_variant_PFProfilesGetEntityProfilesResponse(const PFProfilesGetEntityProfilesResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->profilesCount; ++i) {
            values.push_back(to_variant_PFProfilesEntityProfileBody(p_value->profiles != nullptr ? p_value->profiles[i] : nullptr));
        }
        dictionary["profiles"] = values;
    }
    return dictionary;
}

Variant to_variant_PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest(const PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->masterPlayerAccountIdsCount; ++i) {
            values.push_back(p_value->masterPlayerAccountIds != nullptr && p_value->masterPlayerAccountIds[i] != nullptr ? String::utf8(p_value->masterPlayerAccountIds[i]) : String());
        }
        dictionary["master_player_account_ids"] = values;
    }
    dictionary["title_id"] = p_value->titleId != nullptr ? String::utf8(p_value->titleId) : String();
    return dictionary;
}

Variant to_variant_PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse(const PFProfilesGetTitlePlayersFromMasterPlayerAccountIdsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["title_id"] = p_value->titleId != nullptr ? String::utf8(p_value->titleId) : String();
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->titlePlayerAccountsCount; ++i) {
            const Variant entry_variant = to_variant_PFEntityKeyDictionaryEntry(&p_value->titlePlayerAccounts[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["title_player_accounts"] = values;
    }
    return dictionary;
}

Variant to_variant_PFProfilesSetEntityProfilePolicyRequest(const PFProfilesSetEntityProfilePolicyRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statementsCount; ++i) {
            values.push_back(to_variant_PFProfilesEntityPermissionStatement(p_value->statements != nullptr ? p_value->statements[i] : nullptr));
        }
        dictionary["statements"] = values;
    }
    return dictionary;
}

Variant to_variant_PFProfilesSetEntityProfilePolicyResponse(const PFProfilesSetEntityProfilePolicyResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->permissionsCount; ++i) {
            values.push_back(to_variant_PFProfilesEntityPermissionStatement(p_value->permissions != nullptr ? p_value->permissions[i] : nullptr));
        }
        dictionary["permissions"] = values;
    }
    return dictionary;
}

Variant to_variant_PFProfilesSetProfileLanguageRequest(const PFProfilesSetProfileLanguageRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    if (p_value->expectedVersion != nullptr) dictionary["expected_version"] = static_cast<int64_t>(*p_value->expectedVersion);
    dictionary["language"] = p_value->language != nullptr ? String::utf8(p_value->language) : String();
    return dictionary;
}

Variant to_variant_PFProfilesSetProfileLanguageResponse(const PFProfilesSetProfileLanguageResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->operationResult != nullptr) dictionary["operation_result"] = static_cast<int64_t>(*p_value->operationResult);
    if (p_value->versionNumber != nullptr) dictionary["version_number"] = static_cast<int64_t>(*p_value->versionNumber);
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticColumn(const PFStatisticsStatisticColumn *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["aggregation_method"] = static_cast<int64_t>(p_value->aggregationMethod);
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticsUpdateEventConfig(const PFStatisticsStatisticsUpdateEventConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["event_type"] = static_cast<int64_t>(p_value->eventType);
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticsEventEmissionConfig(const PFStatisticsStatisticsEventEmissionConfig *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["update_event_config"] = to_variant_PFStatisticsStatisticsUpdateEventConfig(p_value->updateEventConfig);
    return dictionary;
}

Variant to_variant_PFVersionConfiguration(const PFVersionConfiguration *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["max_queryable_versions"] = static_cast<int64_t>(p_value->maxQueryableVersions);
    dictionary["reset_interval"] = static_cast<int64_t>(p_value->resetInterval);
    return dictionary;
}

Variant to_variant_PFStatisticsCreateStatisticDefinitionRequest(const PFStatisticsCreateStatisticDefinitionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->aggregationSourcesCount; ++i) {
            values.push_back(p_value->aggregationSources != nullptr && p_value->aggregationSources[i] != nullptr ? String::utf8(p_value->aggregationSources[i]) : String());
        }
        dictionary["aggregation_sources"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->columnsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticColumn(p_value->columns != nullptr ? p_value->columns[i] : nullptr));
        }
        dictionary["columns"] = values;
    }
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity_type"] = p_value->entityType != nullptr ? String::utf8(p_value->entityType) : String();
    dictionary["event_emission_config"] = to_variant_PFStatisticsStatisticsEventEmissionConfig(p_value->eventEmissionConfig);
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["version_configuration"] = to_variant_PFVersionConfiguration(p_value->versionConfiguration);
    return dictionary;
}

Variant to_variant_PFStatisticsDeleteStatisticDefinitionRequest(const PFStatisticsDeleteStatisticDefinitionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticDelete(const PFStatisticsStatisticDelete *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsDeleteStatisticsRequest(const PFStatisticsDeleteStatisticsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticDelete(p_value->statistics != nullptr ? p_value->statistics[i] : nullptr));
        }
        dictionary["statistics"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsDeleteStatisticsResponse(const PFStatisticsDeleteStatisticsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    return dictionary;
}

Variant to_variant_PFStatisticsEntityStatisticValue(const PFStatisticsEntityStatisticValue *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["metadata"] = p_value->metadata != nullptr ? String::utf8(p_value->metadata) : String();
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->scoresCount; ++i) {
            values.push_back(p_value->scores != nullptr && p_value->scores[i] != nullptr ? String::utf8(p_value->scores[i]) : String());
        }
        dictionary["scores"] = values;
    }
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    return dictionary;
}

Variant to_variant_PFStatisticsEntityStatisticValueDictionaryEntry(const PFStatisticsEntityStatisticValueDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFStatisticsEntityStatisticValue(p_value->value);
    return dictionary;
}

Variant to_variant_PFStatisticsEntityStatistics(const PFStatisticsEntityStatistics *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["entity_key"] = to_variant_PFEntityKey(p_value->entityKey);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            values.push_back(to_variant_PFStatisticsEntityStatisticValue(p_value->statistics != nullptr ? p_value->statistics[i] : nullptr));
        }
        dictionary["statistics"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsGetStatisticDefinitionRequest(const PFStatisticsGetStatisticDefinitionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsGetStatisticDefinitionResponse(const PFStatisticsGetStatisticDefinitionResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->aggregationDestinationsCount; ++i) {
            values.push_back(p_value->aggregationDestinations != nullptr && p_value->aggregationDestinations[i] != nullptr ? String::utf8(p_value->aggregationDestinations[i]) : String());
        }
        dictionary["aggregation_destinations"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->aggregationSourcesCount; ++i) {
            values.push_back(p_value->aggregationSources != nullptr && p_value->aggregationSources[i] != nullptr ? String::utf8(p_value->aggregationSources[i]) : String());
        }
        dictionary["aggregation_sources"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->columnsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticColumn(p_value->columns != nullptr ? p_value->columns[i] : nullptr));
        }
        dictionary["columns"] = values;
    }
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["entity_type"] = p_value->entityType != nullptr ? String::utf8(p_value->entityType) : String();
    dictionary["event_emission_config"] = to_variant_PFStatisticsStatisticsEventEmissionConfig(p_value->eventEmissionConfig);
    if (p_value->lastResetTime != nullptr) dictionary["last_reset_time"] = static_cast<int64_t>(*p_value->lastResetTime);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->linkedLeaderboardNamesCount; ++i) {
            values.push_back(p_value->linkedLeaderboardNames != nullptr && p_value->linkedLeaderboardNames[i] != nullptr ? String::utf8(p_value->linkedLeaderboardNames[i]) : String());
        }
        dictionary["linked_leaderboard_names"] = values;
    }
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    dictionary["version_configuration"] = to_variant_PFVersionConfiguration(p_value->versionConfiguration);
    return dictionary;
}

Variant to_variant_PFStatisticsGetStatisticsForEntitiesRequest(const PFStatisticsGetStatisticsForEntitiesRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->entitiesCount; ++i) {
            values.push_back(to_variant_PFEntityKey(p_value->entities != nullptr ? p_value->entities[i] : nullptr));
        }
        dictionary["entities"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticNamesCount; ++i) {
            values.push_back(p_value->statisticNames != nullptr && p_value->statisticNames[i] != nullptr ? String::utf8(p_value->statisticNames[i]) : String());
        }
        dictionary["statistic_names"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticColumnCollection(const PFStatisticsStatisticColumnCollection *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->columnsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticColumn(p_value->columns != nullptr ? p_value->columns[i] : nullptr));
        }
        dictionary["columns"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticColumnCollectionDictionaryEntry(const PFStatisticsStatisticColumnCollectionDictionaryEntry *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["key"] = p_value->key != nullptr ? String::utf8(p_value->key) : String();
    dictionary["value"] = to_variant_PFStatisticsStatisticColumnCollection(p_value->value);
    return dictionary;
}

Variant to_variant_PFStatisticsGetStatisticsForEntitiesResponse(const PFStatisticsGetStatisticsForEntitiesResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->columnDetailsCount; ++i) {
            const Variant entry_variant = to_variant_PFStatisticsStatisticColumnCollectionDictionaryEntry(&p_value->columnDetails[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["column_details"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->entitiesStatisticsCount; ++i) {
            values.push_back(to_variant_PFStatisticsEntityStatistics(p_value->entitiesStatistics != nullptr ? p_value->entitiesStatistics[i] : nullptr));
        }
        dictionary["entities_statistics"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsGetStatisticsRequest(const PFStatisticsGetStatisticsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticNamesCount; ++i) {
            values.push_back(p_value->statisticNames != nullptr && p_value->statisticNames[i] != nullptr ? String::utf8(p_value->statisticNames[i]) : String());
        }
        dictionary["statistic_names"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsGetStatisticsResponse(const PFStatisticsGetStatisticsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->columnDetailsCount; ++i) {
            const Variant entry_variant = to_variant_PFStatisticsStatisticColumnCollectionDictionaryEntry(&p_value->columnDetails[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["column_details"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            const Variant entry_variant = to_variant_PFStatisticsEntityStatisticValueDictionaryEntry(&p_value->statistics[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["statistics"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsIncrementStatisticVersionRequest(const PFStatisticsIncrementStatisticVersionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsIncrementStatisticVersionResponse(const PFStatisticsIncrementStatisticVersionResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    return dictionary;
}

Variant to_variant_PFStatisticsListStatisticDefinitionsRequest(const PFStatisticsListStatisticDefinitionsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    if (p_value->pageSize != nullptr) dictionary["page_size"] = static_cast<int64_t>(*p_value->pageSize);
    dictionary["skip_token"] = p_value->skipToken != nullptr ? String::utf8(p_value->skipToken) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticDefinition(const PFStatisticsStatisticDefinition *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->aggregationDestinationsCount; ++i) {
            values.push_back(p_value->aggregationDestinations != nullptr && p_value->aggregationDestinations[i] != nullptr ? String::utf8(p_value->aggregationDestinations[i]) : String());
        }
        dictionary["aggregation_destinations"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->aggregationSourcesCount; ++i) {
            values.push_back(p_value->aggregationSources != nullptr && p_value->aggregationSources[i] != nullptr ? String::utf8(p_value->aggregationSources[i]) : String());
        }
        dictionary["aggregation_sources"] = values;
    }
    {
        Array values;
        for (uint32_t i = 0; i < p_value->columnsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticColumn(p_value->columns != nullptr ? p_value->columns[i] : nullptr));
        }
        dictionary["columns"] = values;
    }
    dictionary["created"] = static_cast<int64_t>(p_value->created);
    dictionary["entity_type"] = p_value->entityType != nullptr ? String::utf8(p_value->entityType) : String();
    dictionary["event_emission_config"] = to_variant_PFStatisticsStatisticsEventEmissionConfig(p_value->eventEmissionConfig);
    if (p_value->lastResetTime != nullptr) dictionary["last_reset_time"] = static_cast<int64_t>(*p_value->lastResetTime);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->linkedLeaderboardNamesCount; ++i) {
            values.push_back(p_value->linkedLeaderboardNames != nullptr && p_value->linkedLeaderboardNames[i] != nullptr ? String::utf8(p_value->linkedLeaderboardNames[i]) : String());
        }
        dictionary["linked_leaderboard_names"] = values;
    }
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["version"] = static_cast<int64_t>(p_value->version);
    dictionary["version_configuration"] = to_variant_PFVersionConfiguration(p_value->versionConfiguration);
    return dictionary;
}

Variant to_variant_PFStatisticsListStatisticDefinitionsResponse(const PFStatisticsListStatisticDefinitionsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["page_size"] = static_cast<int64_t>(p_value->pageSize);
    dictionary["skip_token"] = p_value->skipToken != nullptr ? String::utf8(p_value->skipToken) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticDefinitionsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticDefinition(p_value->statisticDefinitions != nullptr ? p_value->statisticDefinitions[i] : nullptr));
        }
        dictionary["statistic_definitions"] = values;
    }
    return dictionary;
}

Variant to_variant_PFStatisticsStatisticUpdate(const PFStatisticsStatisticUpdate *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["metadata"] = p_value->metadata != nullptr ? String::utf8(p_value->metadata) : String();
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    {
        Array values;
        for (uint32_t i = 0; i < p_value->scoresCount; ++i) {
            values.push_back(p_value->scores != nullptr && p_value->scores[i] != nullptr ? String::utf8(p_value->scores[i]) : String());
        }
        dictionary["scores"] = values;
    }
    if (p_value->version != nullptr) dictionary["version"] = static_cast<int64_t>(*p_value->version);
    return dictionary;
}

Variant to_variant_PFStatisticsUpdateStatisticDefinitionRequest(const PFStatisticsUpdateStatisticDefinitionRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["event_emission_config"] = to_variant_PFStatisticsStatisticsEventEmissionConfig(p_value->eventEmissionConfig);
    dictionary["name"] = p_value->name != nullptr ? String::utf8(p_value->name) : String();
    dictionary["version_configuration"] = to_variant_PFVersionConfiguration(p_value->versionConfiguration);
    return dictionary;
}

Variant to_variant_PFStatisticsUpdateStatisticsRequest(const PFStatisticsUpdateStatisticsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->customTagsCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->customTags[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["custom_tags"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Array values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            values.push_back(to_variant_PFStatisticsStatisticUpdate(p_value->statistics != nullptr ? p_value->statistics[i] : nullptr));
        }
        dictionary["statistics"] = values;
    }
    dictionary["transaction_id"] = p_value->transactionId != nullptr ? String::utf8(p_value->transactionId) : String();
    return dictionary;
}

Variant to_variant_PFStatisticsUpdateStatisticsResponse(const PFStatisticsUpdateStatisticsResponse *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->columnDetailsCount; ++i) {
            const Variant entry_variant = to_variant_PFStatisticsStatisticColumnCollectionDictionaryEntry(&p_value->columnDetails[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["column_details"] = values;
    }
    dictionary["entity"] = to_variant_PFEntityKey(p_value->entity);
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->statisticsCount; ++i) {
            const Variant entry_variant = to_variant_PFStatisticsEntityStatisticValueDictionaryEntry(&p_value->statistics[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["statistics"] = values;
    }
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetPublisherDataRequest(const PFTitleDataManagementGetPublisherDataRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->keysCount; ++i) {
            values.push_back(p_value->keys != nullptr && p_value->keys[i] != nullptr ? String::utf8(p_value->keys[i]) : String());
        }
        dictionary["keys"] = values;
    }
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetPublisherDataResult(const PFTitleDataManagementGetPublisherDataResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->data[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetTimeResult(const PFTitleDataManagementGetTimeResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["time"] = static_cast<int64_t>(p_value->time);
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetTitleDataRequest(const PFTitleDataManagementGetTitleDataRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->keysCount; ++i) {
            values.push_back(p_value->keys != nullptr && p_value->keys[i] != nullptr ? String::utf8(p_value->keys[i]) : String());
        }
        dictionary["keys"] = values;
    }
    if (p_value->overrideLabel != nullptr) dictionary["override_label"] = static_cast<int64_t>(*p_value->overrideLabel);
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetTitleDataResult(const PFTitleDataManagementGetTitleDataResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Dictionary values;
        for (uint32_t i = 0; i < p_value->dataCount; ++i) {
            const Variant entry_variant = to_variant_PFStringDictionaryEntry(&p_value->data[i]);
            if (entry_variant.get_type() == Variant::DICTIONARY) {
                Dictionary entry = entry_variant;
                values[entry.get("key", Variant())] = entry.get("value", Variant());
            }
        }
        dictionary["data"] = values;
    }
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetTitleNewsRequest(const PFTitleDataManagementGetTitleNewsRequest *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    if (p_value->count != nullptr) dictionary["count"] = static_cast<int64_t>(*p_value->count);
    return dictionary;
}

Variant to_variant_PFTitleDataManagementTitleNewsItem(const PFTitleDataManagementTitleNewsItem *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    dictionary["body"] = p_value->body != nullptr ? String::utf8(p_value->body) : String();
    dictionary["news_id"] = p_value->newsId != nullptr ? String::utf8(p_value->newsId) : String();
    dictionary["timestamp"] = static_cast<int64_t>(p_value->timestamp);
    dictionary["title"] = p_value->title != nullptr ? String::utf8(p_value->title) : String();
    return dictionary;
}

Variant to_variant_PFTitleDataManagementGetTitleNewsResult(const PFTitleDataManagementGetTitleNewsResult *p_value) {
    if (p_value == nullptr) { return Variant(); }
    Dictionary dictionary;
    {
        Array values;
        for (uint32_t i = 0; i < p_value->newsCount; ++i) {
            values.push_back(to_variant_PFTitleDataManagementTitleNewsItem(p_value->news != nullptr ? p_value->news[i] : nullptr));
        }
        dictionary["news"] = values;
    }
    return dictionary;
}

} }
