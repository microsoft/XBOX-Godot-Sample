using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabGroups : PlayFabServiceBase
{
    internal PlayFabGroups(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> AcceptGroupApplicationAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("accept_group_application_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> AcceptGroupInvitationAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("accept_group_invitation_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> AddMembersAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("add_members_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ApplyToGroupAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("apply_to_group_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> BlockEntityAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("block_entity_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ChangeMemberRoleAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("change_member_role_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> CreateGroupAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("create_group_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> CreateRoleAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("create_role_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteGroupAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_group_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> DeleteRoleAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("delete_role_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetGroupAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_group_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> InviteToGroupAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("invite_to_group_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> IsMemberAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("is_member_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListGroupApplicationsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_group_applications_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListGroupBlocksAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_group_blocks_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListGroupInvitationsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_group_invitations_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListGroupMembersAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_group_members_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListMembershipAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_membership_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ListMembershipOpportunitiesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("list_membership_opportunities_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RemoveGroupApplicationAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("remove_group_application_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RemoveGroupInvitationAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("remove_group_invitation_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RemoveMembersAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("remove_members_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UnblockEntityAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("unblock_entity_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateGroupAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_group_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateRoleAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_role_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
