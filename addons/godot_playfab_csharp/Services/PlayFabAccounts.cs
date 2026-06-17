using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabAccounts : PlayFabServiceBase
{
    internal PlayFabAccounts(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> AddOrUpdateContactEmailAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("add_or_update_contact_email_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetAccountInfoAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_account_info_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayerCombinedInfoAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_player_combined_info_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayerProfileAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_player_profile_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayFabIdsFromBattleNetAccountIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_play_fab_ids_from_battle_net_account_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayFabIdsFromGoogleIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_play_fab_ids_from_google_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayFabIdsFromKongregateIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_play_fab_ids_from_kongregate_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayFabIdsFromSteamIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_play_fab_ids_from_steam_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayFabIdsFromSteamNamesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_play_fab_ids_from_steam_names_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetPlayFabIdsFromXboxLiveIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_play_fab_ids_from_xbox_live_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> LinkBattleNetAccountAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("link_battle_net_account_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> LinkCustomIdAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("link_custom_id_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> LinkOpenIdConnectAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("link_open_id_connect_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> LinkSteamAccountAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("link_steam_account_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> LinkXboxAccountAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("link_xbox_account_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> RemoveContactEmailAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("remove_contact_email_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> ReportPlayerAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("report_player_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UnlinkBattleNetAccountAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("unlink_battle_net_account_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UnlinkCustomIdAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("unlink_custom_id_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UnlinkOpenIdConnectAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("unlink_open_id_connect_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UnlinkSteamAccountAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("unlink_steam_account_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UnlinkXboxAccountAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("unlink_xbox_account_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateAvatarUrlAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_avatar_url_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> UpdateUserTitleDisplayNameAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("update_user_title_display_name_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetTitlePlayersFromXboxLiveIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_title_players_from_xbox_live_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetDisplayNameAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("set_display_name_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetProfileAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_profile_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetProfilesAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_profiles_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> GetTitlePlayersFromMasterPlayerAccountIdsAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("get_title_players_from_master_player_account_ids_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetProfileLanguageAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("set_profile_language_async", user?.Raw, request ?? new Godot.Collections.Dictionary());

    public Task<PlayFabResult> SetProfilePolicyAsync(PlayFabUser user, Godot.Collections.Dictionary request = null) =>
        CallResultAsync("set_profile_policy_async", user?.Raw, request ?? new Godot.Collections.Dictionary());
}
