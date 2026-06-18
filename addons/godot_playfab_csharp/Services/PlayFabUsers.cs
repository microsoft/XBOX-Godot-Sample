using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;
using GodotGdk.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabUsers : PlayFabServiceBase
{
    internal PlayFabUsers(GodotObject o) : base(o)
    {
    }

    public Task<PlayFabResult> SignInWithXUserAsync(GodotGdk.Types.GdkUser user, bool create_account = true) =>
        CallResultAsync("sign_in_with_xuser_async", user?.Raw, create_account);

    public Task<PlayFabResult> SignInWithCustomIdAsync(string custom_id, bool create_account = true) =>
        CallResultAsync("sign_in_with_custom_id_async", custom_id, create_account);

    public Task<PlayFabResult> SignInWithSteamAsync(string steam_ticket, bool create_account = true, bool ticket_is_service_specific = false) =>
        CallResultAsync("sign_in_with_steam_async", steam_ticket, create_account, ticket_is_service_specific);

    public Task<PlayFabResult> SignInWithOpenIdConnectAsync(string connection_id, string id_token, bool create_account = true) =>
        CallResultAsync("sign_in_with_open_id_connect_async", connection_id, id_token, create_account);

    public Task<PlayFabResult> SignInWithBattleNetAsync(string identity_token, bool create_account = true) =>
        CallResultAsync("sign_in_with_battle_net_async", identity_token, create_account);

    public PlayFabUser GetUserByLocalId(int local_id) =>
        PlayFabUser.From(Call("get_user_by_local_id", local_id).AsGodotObject());

    public PlayFabUser GetUserByCustomId(string custom_id) =>
        PlayFabUser.From(Call("get_user_by_custom_id", custom_id).AsGodotObject());

    public PlayFabUser GetUser(Variant user_or_local_id) =>
        PlayFabUser.From(Call("get_user", user_or_local_id).AsGodotObject());

    public Godot.Collections.Array GetUsers() =>
        Call("get_users").AsGodotArray();
}
