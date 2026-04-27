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
/// PFPushNotificationsAdvancedPushPlatformMsg data model.
/// </summary>
typedef struct PFPushNotificationsAdvancedPushPlatformMsg
{
    /// <summary>
    /// (Optional) Stops GoogleCloudMessaging notifications from including both notification and data
    /// properties and instead only sends the data property.
    /// </summary>
    _Maybenull_ bool const* gCMDataOnly;

    /// <summary>
    /// The Json the platform should receive.
    /// </summary>
    _Null_terminated_ const char* json;

    /// <summary>
    /// The platform that should receive the Json.
    /// </summary>
    PFPushNotificationPlatform platform;

} PFPushNotificationsAdvancedPushPlatformMsg;

/// <summary>
/// PFPushNotificationsPushNotificationPackage data model.
/// </summary>
typedef struct PFPushNotificationsPushNotificationPackage
{
    /// <summary>
    /// Numerical badge to display on App icon (iOS only).
    /// </summary>
    int32_t badge;

    /// <summary>
    /// (Optional) This must be a JSON formatted object. For use with developer-created custom Push Notification
    /// plugins.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* customData;

    /// <summary>
    /// (Optional) Icon file to display with the message (Not supported for iOS).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* icon;

    /// <summary>
    /// Content of the message (all platforms).
    /// </summary>
    _Null_terminated_ const char* message;

    /// <summary>
    /// (Optional) Sound file to play with the message (all platforms).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* sound;

    /// <summary>
    /// Title/Subject of the message. Not supported for iOS.
    /// </summary>
    _Null_terminated_ const char* title;

} PFPushNotificationsPushNotificationPackage;

/// <summary>
/// PFPushNotificationsSendPushNotificationRequest data model.
/// </summary>
typedef struct PFPushNotificationsSendPushNotificationRequest
{
    /// <summary>
    /// (Optional) Allows you to provide precisely formatted json to target devices. This is an advanced
    /// feature, allowing you to deliver to custom plugin logic, fields, or functionality not natively
    /// supported by PlayFab.
    /// </summary>
    _Maybenull_ _Field_size_(advancedPlatformDeliveryCount) PFPushNotificationsAdvancedPushPlatformMsg const* const* advancedPlatformDelivery;

    /// <summary>
    /// Count of advancedPlatformDelivery
    /// </summary>
    uint32_t advancedPlatformDeliveryCount;

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
    /// (Optional) Text of message to send.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* message;

    /// <summary>
    /// (Optional) Defines all possible push attributes like message, title, icon, etc. Some parameters
    /// are device specific - please see the PushNotificationPackage documentation for details.
    /// </summary>
    _Maybenull_ PFPushNotificationsPushNotificationPackage const* package;

    /// <summary>
    /// PlayFabId of the recipient of the push notification.
    /// </summary>
    _Null_terminated_ const char* recipient;

    /// <summary>
    /// (Optional) Subject of message to send (may not be displayed in all platforms).
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* subject;

    /// <summary>
    /// (Optional) Target Platforms that should receive the Message or Package. If omitted, we will send
    /// to all available platforms.
    /// </summary>
    _Maybenull_ _Field_size_(targetPlatformsCount) PFPushNotificationPlatform const* targetPlatforms;

    /// <summary>
    /// Count of targetPlatforms
    /// </summary>
    uint32_t targetPlatformsCount;

} PFPushNotificationsSendPushNotificationRequest;

/// <summary>
/// PFPushNotificationsSendPushNotificationFromTemplateRequest data model. Represents the request for
/// sending a push notification template to a recipient.
/// </summary>
typedef struct PFPushNotificationsSendPushNotificationFromTemplateRequest
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
    /// Id of the push notification template.
    /// </summary>
    _Null_terminated_ const char* pushNotificationTemplateId;

    /// <summary>
    /// PlayFabId of the push notification recipient.
    /// </summary>
    _Null_terminated_ const char* recipient;

} PFPushNotificationsSendPushNotificationFromTemplateRequest;

#pragma pop_macro("IN")

}
