using System;
using System.Threading.Tasks;
using Godot;
using GodotPlayFab.Internal;
using GodotPlayFab.Types;

namespace GodotPlayFab.Services;

public sealed class PlayFabParty : PlayFabServiceBase
{
    internal PlayFabParty(GodotObject o) : base(o)
    {
        _o.Connect("party_error", Callable.From((Variant a0) =>
            PartyError?.Invoke(PlayFabResult.From(a0.AsGodotObject()))));
    }

    public event Action<PlayFabResult> PartyError;

    public bool IsInitialized() =>
        Call("is_initialized").AsBool();

    public Task<PlayFabResult> InitializeAsync(PlayFabPartyConfig config = null, int local_udp_port = -1) =>
        CallResultAsync("initialize_async", config?.Raw, local_udp_port);

    public Task<PlayFabResult> ShutdownAsync() =>
        CallResultAsync("shutdown_async");

    public Task<PlayFabResult> CreateAndJoinNetworkAsync(PlayFabUser user, PlayFabPartyConfig config = null) =>
        CallResultAsync("create_and_join_network_async", user?.Raw, config?.Raw);

    public Task<PlayFabResult> JoinNetworkAsync(PlayFabUser user, string descriptor, PlayFabPartyConfig config = null) =>
        CallResultAsync("join_network_async", user?.Raw, descriptor, config?.Raw);

    public Task<PlayFabResult> LeaveNetworkAsync(PlayFabPartyNetwork network) =>
        CallResultAsync("leave_network_async", network?.Raw);

    public PlayFabPartyChat GetChat() =>
        PlayFabPartyChat.From(Call("get_chat").AsGodotObject());

    public Godot.Collections.Array GetNetworks() =>
        Call("get_networks").AsGodotArray();

    public const int DIRECTPEERCONNECTIVITYNONE = 0;

    public const int DIRECTPEERCONNECTIVITYSAMEPLATFORMTYPE = 1;

    public const int DIRECTPEERCONNECTIVITYDIFFERENTPLATFORMTYPE = 2;

    public const int DIRECTPEERCONNECTIVITYANYPLATFORMTYPE = 3;

    public const int DIRECTPEERCONNECTIVITYSAMEENTITYLOGINPROVIDER = 4;

    public const int DIRECTPEERCONNECTIVITYDIFFERENTENTITYLOGINPROVIDER = 8;

    public const int DIRECTPEERCONNECTIVITYANYENTITYLOGINPROVIDER = 12;

    public const int DIRECTPEERCONNECTIVITYANY = 15;

    public const int DIRECTPEERCONNECTIVITYONLYSERVERS = 16;

    public const int NETWORKSTATECREATING = 0;

    public const int NETWORKSTATECONNECTING = 1;

    public const int NETWORKSTATEAUTHENTICATING = 2;

    public const int NETWORKSTATECONNECTED = 3;

    public const int NETWORKSTATEDISCONNECTING = 4;

    public const int NETWORKSTATEDISCONNECTED = 5;

    public const int NETWORKSTATEFAILED = 6;

    public const int NETWORKCHANGESTATE = 0;

    public const int NETWORKCHANGEPEERJOINED = 1;

    public const int NETWORKCHANGEPEERLEFT = 2;

    public const int NETWORKCHANGEDESCRIPTORUPDATED = 3;

    public const int NETWORKCHANGEDESTROYED = 4;

    public const int NETWORKCHANGEERROR = 5;

    public const int CHATCHANGECREATED = 0;

    public const int CHATCHANGEDESTROYED = 1;

    public const int CHATCHANGEPERMISSIONSCHANGED = 2;

    public const int CHATCHANGEMUTEDCHANGED = 3;

    public const int CHATPERMISSIONNONE = 0;

    public const int CHATPERMISSIONSENDAUDIO = 1;

    public const int CHATPERMISSIONRECEIVEAUDIO = 2;

    public const int CHATPERMISSIONRECEIVETEXT = 4;
}
