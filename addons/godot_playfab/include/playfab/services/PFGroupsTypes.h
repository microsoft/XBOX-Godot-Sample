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
/// PFGroupsAcceptGroupApplicationRequest data model. Accepts an outstanding invitation to to join a
/// group if the invited entity is not blocked by the group. Nothing is returned in the case of success.
/// </summary>
typedef struct PFGroupsAcceptGroupApplicationRequest
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
    /// Type of the entity to accept as. Must be the same entity as the claimant or an entity that is
    /// a child of the claimant entity.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsAcceptGroupApplicationRequest;

/// <summary>
/// PFGroupsAcceptGroupInvitationRequest data model. Accepts an outstanding invitation to join the group
/// if the invited entity is not blocked by the group. Only the invited entity or a parent in its chain
/// (e.g. title) may accept the invitation on the invited entity's behalf. Nothing is returned in the
/// case of success.
/// </summary>
typedef struct PFGroupsAcceptGroupInvitationRequest
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

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsAcceptGroupInvitationRequest;

/// <summary>
/// PFGroupsAddMembersRequest data model. Adds members to a group or role. Existing members of the group
/// will added to roles within the group, but if the user is not already a member of the group, only title
/// claimants may add them to the group, and others must use the group application or invite system to
/// add new members to a group. Returns nothing if successful.
/// </summary>
typedef struct PFGroupsAddMembersRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// List of entities to add to the group. Only entities of type title_player_account and character
    /// may be added to groups.
    /// </summary>
    _Field_size_(membersCount) PFEntityKey const* const* members;

    /// <summary>
    /// Count of members
    /// </summary>
    uint32_t membersCount;

    /// <summary>
    /// (Optional) Optional: The ID of the existing role to add the entities to. If this is not specified,
    /// the default member role for the group will be used. Role IDs must be between 1 and 64 characters
    /// long.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsAddMembersRequest;

/// <summary>
/// PFGroupsApplyToGroupRequest data model. Creates an application to join a group. Calling this while
/// a group application already exists will return the same application instead of an error and will not
/// refresh the time before the application expires. By default, if the entity has an invitation to join
/// the group outstanding, this will accept the invitation to join the group instead and return an error
/// indicating such, rather than creating a duplicate application to join that will need to be cleaned
/// up later. Returns information about the application or an error indicating an invitation was accepted
/// instead.
/// </summary>
typedef struct PFGroupsApplyToGroupRequest
{
    /// <summary>
    /// (Optional) Optional, default true. Automatically accept an outstanding invitation if one exists
    /// instead of creating an application.
    /// </summary>
    _Maybenull_ bool const* autoAcceptOutstandingInvite;

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

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsApplyToGroupRequest;

/// <summary>
/// PFGroupsEntityWithLineage data model. Entity wrapper class that contains the entity key and the entities
/// that make up the lineage of the entity.
/// </summary>
typedef struct PFGroupsEntityWithLineage
{
    /// <summary>
    /// (Optional) The entity key for the specified entity.
    /// </summary>
    _Maybenull_ PFEntityKey const* key;

    /// <summary>
    /// (Optional) Dictionary of entity keys for related entities. Dictionary key is entity type.
    /// </summary>
    _Maybenull_ _Field_size_(lineageCount) struct PFEntityKeyDictionaryEntry const* lineage;

    /// <summary>
    /// Count of lineage
    /// </summary>
    uint32_t lineageCount;

} PFGroupsEntityWithLineage;

/// <summary>
/// PFGroupsApplyToGroupResponse data model. Describes an application to join a group.
/// </summary>
typedef struct PFGroupsApplyToGroupResponse
{
    /// <summary>
    /// (Optional) Type of entity that requested membership.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* entity;

    /// <summary>
    /// When the application to join will expire and be deleted.
    /// </summary>
    time_t expires;

    /// <summary>
    /// (Optional) ID of the group that the entity requesting membership to.
    /// </summary>
    _Maybenull_ PFEntityKey const* group;

} PFGroupsApplyToGroupResponse;

/// <summary>
/// PFGroupsBlockEntityRequest data model. Blocks a list of entities from joining a group. Blocked entities
/// may not create new applications to join, be invited to join, accept an invitation, or have an application
/// accepted. Failure due to being blocked does not clean up existing applications or invitations to the
/// group. No data is returned in the case of success.
/// </summary>
typedef struct PFGroupsBlockEntityRequest
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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsBlockEntityRequest;

/// <summary>
/// PFGroupsChangeMemberRoleRequest data model. Changes the role membership of a list of entities from
/// one role to another in in a single operation. The destination role must already exist. This is equivalent
/// to adding the entities to the destination role and removing from the origin role. Returns nothing
/// if successful.
/// </summary>
typedef struct PFGroupsChangeMemberRoleRequest
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
    /// (Optional) The ID of the role that the entities will become a member of. This must be an existing
    /// role. Role IDs must be between 1 and 64 characters long.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* destinationRoleId;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// List of entities to move between roles in the group. All entities in this list must be members
    /// of the group and origin role.
    /// </summary>
    _Field_size_(membersCount) PFEntityKey const* const* members;

    /// <summary>
    /// Count of members
    /// </summary>
    uint32_t membersCount;

    /// <summary>
    /// The ID of the role that the entities currently are a member of. Role IDs must be between 1 and
    /// 64 characters long.
    /// </summary>
    _Null_terminated_ const char* originRoleId;

} PFGroupsChangeMemberRoleRequest;

/// <summary>
/// PFGroupsCreateGroupRequest data model. Creates a new group, as well as administration and member
/// roles, based off of a title's group template. Returns information about the group that was created.
/// </summary>
typedef struct PFGroupsCreateGroupRequest
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

    /// <summary>
    /// The name of the group. This is unique at the title level by default.
    /// </summary>
    _Null_terminated_ const char* groupName;

} PFGroupsCreateGroupRequest;

/// <summary>
/// PFGroupsCreateGroupResponse data model.
/// </summary>
typedef struct PFGroupsCreateGroupResponse
{
    /// <summary>
    /// (Optional) The ID of the administrator role for the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* adminRoleId;

    /// <summary>
    /// The server date and time the group was created.
    /// </summary>
    time_t created;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) The name of the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* groupName;

    /// <summary>
    /// (Optional) The ID of the default member role for the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* memberRoleId;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) The list of roles and names that belong to the group.
    /// </summary>
    _Maybenull_ _Field_size_(rolesCount) struct PFStringDictionaryEntry const* roles;

    /// <summary>
    /// Count of roles
    /// </summary>
    uint32_t rolesCount;

} PFGroupsCreateGroupResponse;

/// <summary>
/// PFGroupsCreateGroupRoleRequest data model. Creates a new role within an existing group, with no members.
/// Both the role ID and role name must be unique within the group, but the name can be the same as the
/// ID. The role ID is set at creation and cannot be changed. Returns information about the role that
/// was created.
/// </summary>
typedef struct PFGroupsCreateGroupRoleRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// The ID of the role. This must be unique within the group and cannot be changed. Role IDs must
    /// be between 1 and 64 characters long and are restricted to a-Z, A-Z, 0-9, '(', ')', '_', '-' and
    /// '.'.
    /// </summary>
    _Null_terminated_ const char* roleId;

    /// <summary>
    /// The name of the role. This must be unique within the group and can be changed later. Role names
    /// must be between 1 and 100 characters long.
    /// </summary>
    _Null_terminated_ const char* roleName;

} PFGroupsCreateGroupRoleRequest;

/// <summary>
/// PFGroupsCreateGroupRoleResponse data model.
/// </summary>
typedef struct PFGroupsCreateGroupRoleResponse
{
    /// <summary>
    /// The current version of the group profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) ID for the role.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

    /// <summary>
    /// (Optional) The name of the role.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleName;

} PFGroupsCreateGroupRoleResponse;

/// <summary>
/// PFGroupsDeleteGroupRequest data model. Deletes a group and all roles, invitations, join requests,
/// and blocks associated with it. Permission to delete is only required the group itself to execute this
/// action. The group and data cannot be cannot be recovered once removed, but any abuse reports about
/// the group will remain. No data is returned in the case of success.
/// </summary>
typedef struct PFGroupsDeleteGroupRequest
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
    /// ID of the group or role to remove.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsDeleteGroupRequest;

/// <summary>
/// PFGroupsDeleteRoleRequest data model. Returns information about the role.
/// </summary>
typedef struct PFGroupsDeleteRoleRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) The ID of the role to delete. Role IDs must be between 1 and 64 characters long.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsDeleteRoleRequest;

/// <summary>
/// PFGroupsGetGroupRequest data model. Returns the ID, name, role list and other non-membership related
/// information about a group.
/// </summary>
typedef struct PFGroupsGetGroupRequest
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
    /// (Optional) The identifier of the group.
    /// </summary>
    _Maybenull_ PFEntityKey const* group;

    /// <summary>
    /// (Optional) The full name of the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* groupName;

} PFGroupsGetGroupRequest;

/// <summary>
/// PFGroupsGetGroupResponse data model.
/// </summary>
typedef struct PFGroupsGetGroupResponse
{
    /// <summary>
    /// (Optional) The ID of the administrator role for the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* adminRoleId;

    /// <summary>
    /// The server date and time the group was created.
    /// </summary>
    time_t created;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) The name of the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* groupName;

    /// <summary>
    /// (Optional) The ID of the default member role for the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* memberRoleId;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) The list of roles and names that belong to the group.
    /// </summary>
    _Maybenull_ _Field_size_(rolesCount) struct PFStringDictionaryEntry const* roles;

    /// <summary>
    /// Count of roles
    /// </summary>
    uint32_t rolesCount;

} PFGroupsGetGroupResponse;

/// <summary>
/// PFGroupsInviteToGroupRequest data model. Invites a player to join a group, if they are not blocked
/// by the group. An optional role can be provided to automatically assign the player to the role if they
/// accept the invitation. By default, if the entity has an application to the group outstanding, this
/// will accept the application instead and return an error indicating such, rather than creating a duplicate
/// invitation to join that will need to be cleaned up later. Returns information about the new invitation
/// or an error indicating an existing application to join was accepted.
/// </summary>
typedef struct PFGroupsInviteToGroupRequest
{
    /// <summary>
    /// (Optional) Optional, default true. Automatically accept an application if one exists instead
    /// of creating an invitation.
    /// </summary>
    _Maybenull_ bool const* autoAcceptOutstandingApplication;

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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) Optional. ID of an existing a role in the group to assign the user to. The group's
    /// default member role is used if this is not specified. Role IDs must be between 1 and 64 characters
    /// long.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsInviteToGroupRequest;

/// <summary>
/// PFGroupsInviteToGroupResponse data model. Describes an invitation to a group.
/// </summary>
typedef struct PFGroupsInviteToGroupResponse
{
    /// <summary>
    /// When the invitation will expire and be deleted.
    /// </summary>
    time_t expires;

    /// <summary>
    /// (Optional) The group that the entity invited to.
    /// </summary>
    _Maybenull_ PFEntityKey const* group;

    /// <summary>
    /// (Optional) The entity that created the invitation.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* invitedByEntity;

    /// <summary>
    /// (Optional) The entity that is invited.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* invitedEntity;

    /// <summary>
    /// (Optional) ID of the role in the group to assign the user to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsInviteToGroupResponse;

/// <summary>
/// PFGroupsIsMemberRequest data model. Checks to see if an entity is a member of a group or role within
/// the group. A result indicating if the entity is a member of the group is returned, or a permission
/// error if the caller does not have permission to read the group's member list.
/// </summary>
typedef struct PFGroupsIsMemberRequest
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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) Optional: ID of the role to check membership of. Defaults to any role (that is, check
    /// to see if the entity is a member of the group in any capacity) if not specified.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsIsMemberRequest;

/// <summary>
/// PFGroupsIsMemberResponse data model.
/// </summary>
typedef struct PFGroupsIsMemberResponse
{
    /// <summary>
    /// A value indicating whether or not the entity is a member.
    /// </summary>
    bool isMember;

} PFGroupsIsMemberResponse;

/// <summary>
/// PFGroupsListGroupApplicationsRequest data model. Lists all outstanding requests to join a group.
/// Returns a list of all requests to join, as well as when the request will expire. To get the group
/// applications for a specific entity, use ListMembershipOpportunities.
/// </summary>
typedef struct PFGroupsListGroupApplicationsRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsListGroupApplicationsRequest;

/// <summary>
/// PFGroupsGroupApplication data model. Describes an application to join a group.
/// </summary>
typedef struct PFGroupsGroupApplication
{
    /// <summary>
    /// (Optional) Type of entity that requested membership.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* entity;

    /// <summary>
    /// When the application to join will expire and be deleted.
    /// </summary>
    time_t expires;

    /// <summary>
    /// (Optional) ID of the group that the entity requesting membership to.
    /// </summary>
    _Maybenull_ PFEntityKey const* group;

} PFGroupsGroupApplication;

/// <summary>
/// PFGroupsListGroupApplicationsResponse data model.
/// </summary>
typedef struct PFGroupsListGroupApplicationsResponse
{
    /// <summary>
    /// (Optional) The requested list of applications to the group.
    /// </summary>
    _Maybenull_ _Field_size_(applicationsCount) PFGroupsGroupApplication const* const* applications;

    /// <summary>
    /// Count of applications
    /// </summary>
    uint32_t applicationsCount;

} PFGroupsListGroupApplicationsResponse;

/// <summary>
/// PFGroupsListGroupBlocksRequest data model. Lists all entities blocked from joining a group. A list
/// of blocked entities is returned.
/// </summary>
typedef struct PFGroupsListGroupBlocksRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsListGroupBlocksRequest;

/// <summary>
/// PFGroupsGroupBlock data model. Describes an entity that is blocked from joining a group.
/// </summary>
typedef struct PFGroupsGroupBlock
{
    /// <summary>
    /// (Optional) The entity that is blocked.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* entity;

    /// <summary>
    /// ID of the group that the entity is blocked from.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsGroupBlock;

/// <summary>
/// PFGroupsListGroupBlocksResponse data model.
/// </summary>
typedef struct PFGroupsListGroupBlocksResponse
{
    /// <summary>
    /// (Optional) The requested list blocked entities.
    /// </summary>
    _Maybenull_ _Field_size_(blockedEntitiesCount) PFGroupsGroupBlock const* const* blockedEntities;

    /// <summary>
    /// Count of blockedEntities
    /// </summary>
    uint32_t blockedEntitiesCount;

} PFGroupsListGroupBlocksResponse;

/// <summary>
/// PFGroupsListGroupInvitationsRequest data model. Lists all outstanding invitations for a group. Returns
/// a list of entities that have been invited, as well as when the invitation will expire. To get the
/// group invitations for a specific entity, use ListMembershipOpportunities.
/// </summary>
typedef struct PFGroupsListGroupInvitationsRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsListGroupInvitationsRequest;

/// <summary>
/// PFGroupsGroupInvitation data model. Describes an invitation to a group.
/// </summary>
typedef struct PFGroupsGroupInvitation
{
    /// <summary>
    /// When the invitation will expire and be deleted.
    /// </summary>
    time_t expires;

    /// <summary>
    /// (Optional) The group that the entity invited to.
    /// </summary>
    _Maybenull_ PFEntityKey const* group;

    /// <summary>
    /// (Optional) The entity that created the invitation.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* invitedByEntity;

    /// <summary>
    /// (Optional) The entity that is invited.
    /// </summary>
    _Maybenull_ PFGroupsEntityWithLineage const* invitedEntity;

    /// <summary>
    /// (Optional) ID of the role in the group to assign the user to.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsGroupInvitation;

/// <summary>
/// PFGroupsListGroupInvitationsResponse data model.
/// </summary>
typedef struct PFGroupsListGroupInvitationsResponse
{
    /// <summary>
    /// (Optional) The requested list of group invitations.
    /// </summary>
    _Maybenull_ _Field_size_(invitationsCount) PFGroupsGroupInvitation const* const* invitations;

    /// <summary>
    /// Count of invitations
    /// </summary>
    uint32_t invitationsCount;

} PFGroupsListGroupInvitationsResponse;

/// <summary>
/// PFGroupsListGroupMembersRequest data model. Gets a list of members and the roles they belong to within
/// the group. If the caller does not have permission to view the role, and the member is in no other
/// role, the member is not displayed. Returns a list of entities that are members of the group.
/// </summary>
typedef struct PFGroupsListGroupMembersRequest
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
    /// ID of the group to list the members and roles for.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsListGroupMembersRequest;

/// <summary>
/// PFGroupsEntityMemberRole data model.
/// </summary>
typedef struct PFGroupsEntityMemberRole
{
    /// <summary>
    /// (Optional) The list of members in the role.
    /// </summary>
    _Maybenull_ _Field_size_(membersCount) PFGroupsEntityWithLineage const* const* members;

    /// <summary>
    /// Count of members
    /// </summary>
    uint32_t membersCount;

    /// <summary>
    /// (Optional) The ID of the role.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

    /// <summary>
    /// (Optional) The name of the role.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleName;

} PFGroupsEntityMemberRole;

/// <summary>
/// PFGroupsListGroupMembersResponse data model.
/// </summary>
typedef struct PFGroupsListGroupMembersResponse
{
    /// <summary>
    /// (Optional) The requested list of roles and member entity IDs.
    /// </summary>
    _Maybenull_ _Field_size_(membersCount) PFGroupsEntityMemberRole const* const* members;

    /// <summary>
    /// Count of members
    /// </summary>
    uint32_t membersCount;

} PFGroupsListGroupMembersResponse;

/// <summary>
/// PFGroupsListMembershipRequest data model. Lists the groups and roles that an entity is a part of,
/// checking to see if group and role metadata and memberships should be visible to the caller. If the
/// entity is not in any roles that are visible to the caller, the group is not returned in the results,
/// even if the caller otherwise has permission to see that the entity is a member of that group.
/// </summary>
typedef struct PFGroupsListMembershipRequest
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

} PFGroupsListMembershipRequest;

/// <summary>
/// PFGroupsGroupRole data model. Describes a group role.
/// </summary>
typedef struct PFGroupsGroupRole
{
    /// <summary>
    /// (Optional) ID for the role.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

    /// <summary>
    /// (Optional) The name of the role.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleName;

} PFGroupsGroupRole;

/// <summary>
/// PFGroupsGroupWithRoles data model. Describes a group and the roles that it contains.
/// </summary>
typedef struct PFGroupsGroupWithRoles
{
    /// <summary>
    /// (Optional) ID for the group.
    /// </summary>
    _Maybenull_ PFEntityKey const* group;

    /// <summary>
    /// (Optional) The name of the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* groupName;

    /// <summary>
    /// The current version of the profile, can be used for concurrency control during updates.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) The list of roles within the group.
    /// </summary>
    _Maybenull_ _Field_size_(rolesCount) PFGroupsGroupRole const* const* roles;

    /// <summary>
    /// Count of roles
    /// </summary>
    uint32_t rolesCount;

} PFGroupsGroupWithRoles;

/// <summary>
/// PFGroupsListMembershipResponse data model.
/// </summary>
typedef struct PFGroupsListMembershipResponse
{
    /// <summary>
    /// (Optional) The list of groups.
    /// </summary>
    _Maybenull_ _Field_size_(groupsCount) PFGroupsGroupWithRoles const* const* groups;

    /// <summary>
    /// Count of groups
    /// </summary>
    uint32_t groupsCount;

} PFGroupsListMembershipResponse;

/// <summary>
/// PFGroupsListMembershipOpportunitiesRequest data model. Lists all outstanding group applications and
/// invitations for an entity. Anyone may call this for any entity, but data will only be returned for
/// the entity or a parent of that entity. To list invitations or applications for a group to check if
/// a player is trying to join, use ListGroupInvitations and ListGroupApplications.
/// </summary>
typedef struct PFGroupsListMembershipOpportunitiesRequest
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

} PFGroupsListMembershipOpportunitiesRequest;

/// <summary>
/// PFGroupsListMembershipOpportunitiesResponse data model.
/// </summary>
typedef struct PFGroupsListMembershipOpportunitiesResponse
{
    /// <summary>
    /// (Optional) The requested list of group applications.
    /// </summary>
    _Maybenull_ _Field_size_(applicationsCount) PFGroupsGroupApplication const* const* applications;

    /// <summary>
    /// Count of applications
    /// </summary>
    uint32_t applicationsCount;

    /// <summary>
    /// (Optional) The requested list of group invitations.
    /// </summary>
    _Maybenull_ _Field_size_(invitationsCount) PFGroupsGroupInvitation const* const* invitations;

    /// <summary>
    /// Count of invitations
    /// </summary>
    uint32_t invitationsCount;

} PFGroupsListMembershipOpportunitiesResponse;

/// <summary>
/// PFGroupsRemoveGroupApplicationRequest data model. Removes an existing application to join the group.
/// This is used for both rejection of an application as well as withdrawing an application. The applying
/// entity or a parent in its chain (e.g. title) may withdraw the application, and any caller with appropriate
/// access in the group may reject an application. No data is returned in the case of success.
/// </summary>
typedef struct PFGroupsRemoveGroupApplicationRequest
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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsRemoveGroupApplicationRequest;

/// <summary>
/// PFGroupsRemoveGroupInvitationRequest data model. Removes an existing invitation to join the group.
/// This is used for both rejection of an invitation as well as rescinding an invitation. The invited
/// entity or a parent in its chain (e.g. title) may reject the invitation by calling this method, and
/// any caller with appropriate access in the group may rescind an invitation. No data is returned in
/// the case of success.
/// </summary>
typedef struct PFGroupsRemoveGroupInvitationRequest
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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsRemoveGroupInvitationRequest;

/// <summary>
/// PFGroupsRemoveMembersRequest data model. Removes members from a group. A member can always remove
/// themselves from a group, regardless of permissions. Returns nothing if successful.
/// </summary>
typedef struct PFGroupsRemoveMembersRequest
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
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// List of entities to remove.
    /// </summary>
    _Field_size_(membersCount) PFEntityKey const* const* members;

    /// <summary>
    /// Count of members
    /// </summary>
    uint32_t membersCount;

    /// <summary>
    /// (Optional) The ID of the role to remove the entities from.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

} PFGroupsRemoveMembersRequest;

/// <summary>
/// PFGroupsUnblockEntityRequest data model. Unblocks a list of entities from joining a group. No data
/// is returned in the case of success.
/// </summary>
typedef struct PFGroupsUnblockEntityRequest
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
    /// The entity to perform this action on.
    /// </summary>
    PFEntityKey const* entity;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

} PFGroupsUnblockEntityRequest;

/// <summary>
/// PFGroupsUpdateGroupRequest data model. Updates data about a group, such as the name or default member
/// role. Returns information about whether the update was successful. Only title claimants may modify
/// the administration role for a group.
/// </summary>
typedef struct PFGroupsUpdateGroupRequest
{
    /// <summary>
    /// (Optional) Optional: the ID of an existing role to set as the new administrator role for the
    /// group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* adminRoleId;

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
    /// (Optional) Optional field used for concurrency control. By specifying the previously returned
    /// value of ProfileVersion from the GetGroup API, you can ensure that the group data update will
    /// only be performed if the group has not been updated by any other clients since the version you
    /// last loaded.
    /// </summary>
    _Maybenull_ int32_t const* expectedProfileVersion;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) Optional: the new name of the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* groupName;

    /// <summary>
    /// (Optional) Optional: the ID of an existing role to set as the new member role for the group.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* memberRoleId;

} PFGroupsUpdateGroupRequest;

/// <summary>
/// PFGroupsUpdateGroupResponse data model.
/// </summary>
typedef struct PFGroupsUpdateGroupResponse
{
    /// <summary>
    /// (Optional) Optional reason to explain why the operation was the result that it was.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationReason;

    /// <summary>
    /// New version of the group data.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) Indicates which operation was completed, either Created, Updated, Deleted or None.
    /// </summary>
    _Maybenull_ PFOperationTypes const* setResult;

} PFGroupsUpdateGroupResponse;

/// <summary>
/// PFGroupsUpdateGroupRoleRequest data model. Updates the role name. Returns information about whether
/// the update was successful.
/// </summary>
typedef struct PFGroupsUpdateGroupRoleRequest
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
    /// (Optional) Optional field used for concurrency control. By specifying the previously returned
    /// value of ProfileVersion from the GetGroup API, you can ensure that the group data update will
    /// only be performed if the group has not been updated by any other clients since the version you
    /// last loaded.
    /// </summary>
    _Maybenull_ int32_t const* expectedProfileVersion;

    /// <summary>
    /// The identifier of the group.
    /// </summary>
    PFEntityKey const* group;

    /// <summary>
    /// (Optional) ID of the role to update. Role IDs must be between 1 and 64 characters long.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* roleId;

    /// <summary>
    /// The new name of the role.
    /// </summary>
    _Null_terminated_ const char* roleName;

} PFGroupsUpdateGroupRoleRequest;

/// <summary>
/// PFGroupsUpdateGroupRoleResponse data model.
/// </summary>
typedef struct PFGroupsUpdateGroupRoleResponse
{
    /// <summary>
    /// (Optional) Optional reason to explain why the operation was the result that it was.
    /// </summary>
    _Maybenull_ _Null_terminated_ const char* operationReason;

    /// <summary>
    /// New version of the role data.
    /// </summary>
    int32_t profileVersion;

    /// <summary>
    /// (Optional) Indicates which operation was completed, either Created, Updated, Deleted or None.
    /// </summary>
    _Maybenull_ PFOperationTypes const* setResult;

} PFGroupsUpdateGroupRoleResponse;

#pragma pop_macro("IN")

}
