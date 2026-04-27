// Copyright (c) Microsoft Corporation
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#if !defined(__cplusplus)
#error C++11 required
#endif

#pragma once

#include <playfab/services/PFGroupsTypes.h>
#include <playfab/core/PFEntity.h>

extern "C"
{

/// <summary>
/// Accepts an outstanding invitation to to join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Accepts an outstanding invitation to to join a group if the invited entity is not blocked by the
/// group. Nothing is returned in the case of success. See also GroupApplyToGroupAsync, GroupListGroupApplicationsAsync,
/// GroupRemoveGroupApplicationAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ENTITY_BLOCKED_BY_GROUP, E_PF_ENTITY_IS_ALREADY_MEMBER, E_PF_GROUP_APPLICATION_NOT_FOUND
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </remarks>
PF_API PFGroupsAcceptGroupApplicationAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsAcceptGroupApplicationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Accepts an invitation to join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Accepts an outstanding invitation to join the group if the invited entity is not blocked by the group.
/// Only the invited entity or a parent in its chain (e.g. title) may accept the invitation on the invited
/// entity's behalf. Nothing is returned in the case of success. See also GroupInviteToGroupAsync, GroupListGroupInvitationsAsync,
/// GroupListMembershipOpportunitiesAsync, GroupRemoveGroupInvitationAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ENTITY_BLOCKED_BY_GROUP, E_PF_ENTITY_IS_ALREADY_MEMBER, E_PF_GROUP_INVITATION_NOT_FOUND,
/// E_PF_ROLE_DOES_NOT_EXIST or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </remarks>
PF_API PFGroupsAcceptGroupInvitationAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsAcceptGroupInvitationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Adds members to a group or role.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Adds members to a group or role. Existing members of the group will added to roles within the group,
/// but if the user is not already a member of the group, only title claimants may add them to the group,
/// and others must use the group application or invite system to add new members to a group. Returns
/// nothing if successful. See also GroupApplyToGroupAsync, GroupInviteToGroupAsync, GroupListGroupMembersAsync,
/// GroupRemoveMembersAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ROLE_DOES_NOT_EXIST or any of the global PlayFab Service errors. See
/// doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFGroupsAddMembersAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsAddMembersRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Applies to join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Creates an application to join a group. Calling this while a group application already exists will
/// return the same application instead of an error and will not refresh the time before the application
/// expires. By default, if the entity has an invitation to join the group outstanding, this will accept
/// the invitation to join the group instead and return an error indicating such, rather than creating
/// a duplicate application to join that will need to be cleaned up later. Returns information about the
/// application or an error indicating an invitation was accepted instead. See also GroupAcceptGroupApplicationAsync,
/// GroupListGroupApplicationsAsync, GroupRemoveGroupApplicationAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsApplyToGroupGetResultSize"/> and
/// <see cref="PFGroupsApplyToGroupGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsApplyToGroupAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsApplyToGroupRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ApplyToGroup call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_BLOCKED_BY_GROUP,
/// E_PF_ENTITY_IS_ALREADY_MEMBER, E_PF_OUTSTANDING_INVITATION_ACCEPTED_INSTEAD, E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFGroupsApplyToGroupGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsApplyToGroupAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_BLOCKED_BY_GROUP,
/// E_PF_ENTITY_IS_ALREADY_MEMBER, E_PF_OUTSTANDING_INVITATION_ACCEPTED_INSTEAD, E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsApplyToGroupGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsApplyToGroupResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Blocks a list of entities from joining a group.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Blocks a list of entities from joining a group. Blocked entities may not create new applications
/// to join, be invited to join, accept an invitation, or have an application accepted. Failure due to
/// being blocked does not clean up existing applications or invitations to the group. No data is returned
/// in the case of success. See also GroupListGroupBlocksAsync, GroupUnblockEntityAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFGroupsBlockEntityAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsBlockEntityRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Changes the role membership of a list of entities from one role to another.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Changes the role membership of a list of entities from one role to another in in a single operation.
/// The destination role must already exist. This is equivalent to adding the entities to the destination
/// role and removing from the origin role. Returns nothing if successful. See also GroupAddMembersAsync,
/// GroupCreateRoleAsync, GroupRemoveMembersAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ROLE_DOES_NOT_EXIST or any of the global PlayFab Service errors. See
/// doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFGroupsChangeMemberRoleAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsChangeMemberRoleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Creates a new group.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Creates a new group, as well as administration and member roles, based off of a title's group template.
/// Returns information about the group that was created. See also GroupAddMembersAsync, GroupApplyToGroupAsync,
/// GroupDeleteGroupAsync, GroupInviteToGroupAsync, GroupListGroupMembersAsync, GroupRemoveMembersAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsCreateGroupGetResultSize"/> and <see
/// cref="PFGroupsCreateGroupGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsCreateGroupAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsCreateGroupRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a CreateGroup call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_GROUP_NAME_NOT_AVAILABLE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFGroupsCreateGroupGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsCreateGroupAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_GROUP_NAME_NOT_AVAILABLE
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsCreateGroupGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsCreateGroupResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Creates a new group role.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Creates a new role within an existing group, with no members. Both the role ID and role name must
/// be unique within the group, but the name can be the same as the ID. The role ID is set at creation
/// and cannot be changed. Returns information about the role that was created. See also GroupDeleteRoleAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsCreateRoleGetResultSize"/> and <see
/// cref="PFGroupsCreateRoleGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsCreateRoleAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsCreateGroupRoleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a CreateRole call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DUPLICATE_ROLE_ID,
/// E_PF_ROLE_NAME_NOT_AVAILABLE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsCreateRoleGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsCreateRoleAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_DUPLICATE_ROLE_ID,
/// E_PF_ROLE_NAME_NOT_AVAILABLE or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsCreateRoleGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsCreateGroupRoleResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Deletes a group and all roles, invitations, join requests, and blocks associated with it.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Deletes a group and all roles, invitations, join requests, and blocks associated with it. Permission
/// to delete is only required the group itself to execute this action. The group and data cannot be cannot
/// be recovered once removed, but any abuse reports about the group will remain. No data is returned
/// in the case of success. See also GroupCreateGroupAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFGroupsDeleteGroupAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsDeleteGroupRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Deletes an existing role in a group.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns information about the role See also GroupCreateRoleAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ROLE_DOES_NOT_EXIST, E_PF_ROLE_IS_GROUP_ADMIN, E_PF_ROLE_IS_GROUP_DEFAULT_MEMBER
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </remarks>
PF_API PFGroupsDeleteRoleAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsDeleteRoleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets information about a group and its roles
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Returns the ID, name, role list and other non-membership related information about a group. See also
/// GroupUpdateGroupAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsGetGroupGetResultSize"/> and <see
/// cref="PFGroupsGetGroupGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsGetGroupAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsGetGroupRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a GetGroup call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsGetGroupGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsGetGroupAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsGetGroupGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsGetGroupResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Invites a player to join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Invites a player to join a group, if they are not blocked by the group. An optional role can be provided
/// to automatically assign the player to the role if they accept the invitation. By default, if the entity
/// has an application to the group outstanding, this will accept the application instead and return an
/// error indicating such, rather than creating a duplicate invitation to join that will need to be cleaned
/// up later. Returns information about the new invitation or an error indicating an existing application
/// to join was accepted. See also GroupAcceptGroupInvitationAsync, GroupListGroupInvitationsAsync, GroupListMembershipOpportunitiesAsync,
/// GroupRemoveGroupInvitationAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsInviteToGroupGetResultSize"/> and
/// <see cref="PFGroupsInviteToGroupGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsInviteToGroupAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsInviteToGroupRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a InviteToGroup call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_BLOCKED_BY_GROUP,
/// E_PF_ENTITY_IS_ALREADY_MEMBER, E_PF_OUTSTANDING_APPLICATION_ACCEPTED_INSTEAD, E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFGroupsInviteToGroupGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsInviteToGroupAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ENTITY_BLOCKED_BY_GROUP,
/// E_PF_ENTITY_IS_ALREADY_MEMBER, E_PF_OUTSTANDING_APPLICATION_ACCEPTED_INSTEAD, E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsInviteToGroupGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsInviteToGroupResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Checks to see if an entity is a member of a group or role within the group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Checks to see if an entity is a member of a group or role within the group. A result indicating if
/// the entity is a member of the group is returned, or a permission error if the caller does not have
/// permission to read the group's member list. See also GroupGetGroupAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsIsMemberGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsIsMemberAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsIsMemberRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsIsMemberAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="result">PFGroupsIsMemberResponse object that will be populated with the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFGroupsIsMemberGetResult(
    _Inout_ XAsyncBlock* async,
    _Out_ PFGroupsIsMemberResponse* result
) noexcept;

/// <summary>
/// Lists all outstanding requests to join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Lists all outstanding requests to join a group. Returns a list of all requests to join, as well as
/// when the request will expire. To get the group applications for a specific entity, use ListMembershipOpportunities.
/// See also GroupAcceptGroupApplicationAsync, GroupApplyToGroupAsync, GroupRemoveGroupApplicationAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsListGroupApplicationsGetResultSize"/>
/// and <see cref="PFGroupsListGroupApplicationsGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsListGroupApplicationsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsListGroupApplicationsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListGroupApplications call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsListGroupApplicationsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsListGroupApplicationsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsListGroupApplicationsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsListGroupApplicationsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists all entities blocked from joining a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Lists all entities blocked from joining a group. A list of blocked entities is returned See also
/// GroupBlockEntityAsync, GroupUnblockEntityAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsListGroupBlocksGetResultSize"/> and
/// <see cref="PFGroupsListGroupBlocksGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsListGroupBlocksAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsListGroupBlocksRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListGroupBlocks call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsListGroupBlocksGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsListGroupBlocksAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsListGroupBlocksGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsListGroupBlocksResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists all outstanding invitations for a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Lists all outstanding invitations for a group. Returns a list of entities that have been invited,
/// as well as when the invitation will expire. To get the group invitations for a specific entity, use
/// ListMembershipOpportunities. See also GroupAcceptGroupInvitationAsync, GroupInviteToGroupAsync, GroupListMembershipOpportunitiesAsync,
/// GroupRemoveGroupInvitationAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsListGroupInvitationsGetResultSize"/>
/// and <see cref="PFGroupsListGroupInvitationsGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsListGroupInvitationsAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsListGroupInvitationsRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListGroupInvitations call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsListGroupInvitationsGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsListGroupInvitationsAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsListGroupInvitationsGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsListGroupInvitationsResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists all members for a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Gets a list of members and the roles they belong to within the group. If the caller does not have
/// permission to view the role, and the member is in no other role, the member is not displayed. Returns
/// a list of entities that are members of the group. See also GroupListMembershipAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsListGroupMembersGetResultSize"/>
/// and <see cref="PFGroupsListGroupMembersGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsListGroupMembersAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsListGroupMembersRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListGroupMembers call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsListGroupMembersGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsListGroupMembersAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsListGroupMembersGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsListGroupMembersResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists all groups and roles for an entity
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Lists the groups and roles that an entity is a part of, checking to see if group and role metadata
/// and memberships should be visible to the caller. If the entity is not in any roles that are visible
/// to the caller, the group is not returned in the results, even if the caller otherwise has permission
/// to see that the entity is a member of that group. See also GroupListGroupMembersAsync, GroupListMembershipOpportunitiesAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsListMembershipGetResultSize"/> and
/// <see cref="PFGroupsListMembershipGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsListMembershipAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsListMembershipRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListMembership call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsListMembershipGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsListMembershipAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsListMembershipGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsListMembershipResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Lists all outstanding invitations and group applications for an entity
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Lists all outstanding group applications and invitations for an entity. Anyone may call this for
/// any entity, but data will only be returned for the entity or a parent of that entity. To list invitations
/// or applications for a group to check if a player is trying to join, use ListGroupInvitations and ListGroupApplications.
/// See also GroupListGroupApplicationsAsync, GroupListGroupInvitationsAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsListMembershipOpportunitiesGetResultSize"/>
/// and <see cref="PFGroupsListMembershipOpportunitiesGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsListMembershipOpportunitiesAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsListMembershipOpportunitiesRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a ListMembershipOpportunities call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsListMembershipOpportunitiesGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsListMembershipOpportunitiesAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be one of
/// global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsListMembershipOpportunitiesGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsListMembershipOpportunitiesResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Removes an application to join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Removes an existing application to join the group. This is used for both rejection of an application
/// as well as withdrawing an application. The applying entity or a parent in its chain (e.g. title) may
/// withdraw the application, and any caller with appropriate access in the group may reject an application.
/// No data is returned in the case of success. See also GroupAcceptGroupApplicationAsync, GroupApplyToGroupAsync,
/// GroupListGroupApplicationsAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_GROUP_APPLICATION_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFGroupsRemoveGroupApplicationAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsRemoveGroupApplicationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Removes an invitation join a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Removes an existing invitation to join the group. This is used for both rejection of an invitation
/// as well as rescinding an invitation. The invited entity or a parent in its chain (e.g. title) may
/// reject the invitation by calling this method, and any caller with appropriate access in the group
/// may rescind an invitation. No data is returned in the case of success. See also GroupAcceptGroupInvitationAsync,
/// GroupInviteToGroupAsync, GroupListGroupInvitationsAsync, GroupListMembershipOpportunitiesAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_GROUP_INVITATION_NOT_FOUND or any of the global PlayFab Service errors.
/// See doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFGroupsRemoveGroupInvitationAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsRemoveGroupInvitationRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Removes members from a group.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Removes members from a group. A member can always remove themselves from a group, regardless of permissions.
/// Returns nothing if successful. See also GroupAddMembersAsync, GroupListGroupMembersAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be E_PF_ROLE_DOES_NOT_EXIST or any of the global PlayFab Service errors. See
/// doc page "Handling PlayFab Errors" for more details on error handling.
/// </remarks>
PF_API PFGroupsRemoveMembersAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsRemoveMembersRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Unblocks a list of entities from joining a group
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Unblocks a list of entities from joining a group. No data is returned in the case of success. See
/// also GroupBlockEntityAsync, GroupListGroupBlocksAsync.
///
/// Call <see cref="XAsyncGetStatus"/> to get the status of the operation. If the service call is unsuccessful,
/// the async result will be one of global PlayFab Service errors. See doc page "Handling PlayFab Errors"
/// for more details on error handling.
/// </remarks>
PF_API PFGroupsUnblockEntityAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsUnblockEntityRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Updates non-membership data about a group.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Updates data about a group, such as the name or default member role. Returns information about whether
/// the update was successful. Only title claimants may modify the administration role for a group. See
/// also GroupCreateGroupAsync, GroupDeleteGroupAsync, GroupGetGroupAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsUpdateGroupGetResultSize"/> and <see
/// cref="PFGroupsUpdateGroupGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsUpdateGroupAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsUpdateGroupRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a UpdateGroup call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_GROUP_NAME_NOT_AVAILABLE,
/// E_PF_ROLE_DOES_NOT_EXIST or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
PF_API PFGroupsUpdateGroupGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsUpdateGroupAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_GROUP_NAME_NOT_AVAILABLE,
/// E_PF_ROLE_DOES_NOT_EXIST or any of the global PlayFab Service errors. See doc page "Handling PlayFab
/// Errors" for more details on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsUpdateGroupGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsUpdateGroupResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;

/// <summary>
/// Updates metadata about a role.
/// </summary>
/// <param name="entityHandle">PFEntityHandle to use for authentication.</param>
/// <param name="request">Populated request object.</param>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <returns>Result code for this API operation.</returns>
/// <remarks>
/// This API is available on all platforms.
/// Updates the role name. Returns information about whether the update was successful. See also GroupCreateRoleAsync,
/// GroupDeleteRoleAsync.
///
/// When the asynchronous task is complete, call <see cref="PFGroupsUpdateRoleGetResultSize"/> and <see
/// cref="PFGroupsUpdateRoleGetResult"/> to get the result.
/// </remarks>
PF_API PFGroupsUpdateRoleAsync(
    _In_ PFEntityHandle entityHandle,
    _In_ const PFGroupsUpdateGroupRoleRequest* request,
    _Inout_ XAsyncBlock* async
) noexcept;

/// <summary>
/// Get the size in bytes needed to store the result of a UpdateRole call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The buffer size in bytes required for the result.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
PF_API PFGroupsUpdateRoleGetResultSize(
    _Inout_ XAsyncBlock* async,
    _Out_ size_t* bufferSize
) noexcept;

/// <summary>
/// Gets the result of a successful PFGroupsUpdateRoleAsync call.
/// </summary>
/// <param name="async">XAsyncBlock for the async operation.</param>
/// <param name="bufferSize">The size of the buffer for the result object.</param>
/// <param name="buffer">Byte buffer used for the result value and its fields.</param>
/// <param name="result">Pointer to the result object.</param>
/// <param name="bufferUsed">The number of bytes in the provided buffer that were used.</param>
/// <returns>
/// Result code for this API operation. If the service call is unsuccessful, the result will be E_PF_ROLE_DOES_NOT_EXIST
/// or any of the global PlayFab Service errors. See doc page "Handling PlayFab Errors" for more details
/// on error handling.
/// </returns>
/// <remarks>
/// result is a pointer within buffer and does not need to be freed separately.
/// </remarks>
PF_API PFGroupsUpdateRoleGetResult(
    _Inout_ XAsyncBlock* async,
    _In_ size_t bufferSize,
    _Out_writes_bytes_to_(bufferSize, *bufferUsed) void* buffer,
    _Outptr_ PFGroupsUpdateGroupRoleResponse** result,
    _Out_opt_ size_t* bufferUsed
) noexcept;


}