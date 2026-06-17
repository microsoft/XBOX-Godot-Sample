using Godot;
using System;
using System.Threading.Tasks;
using GodotGdk;
using GodotPlayFab;
using GodotPlayFab.Types;
using GodotPlayFab.Services;

public partial class Party : Node
{
    private const string PartyDescriptorKey = "party_descriptor";
    private const int XuserPrivilegeCommunications = 252;
    private const int XuserPrivilegeCommunicationVoiceIngame = 205;
    private const int PartyChatNone = 0;
    private const int PartyChatSendAudio = 1;
    private const int PartyChatReceiveAudio = 2;
    private const int PartyChatSendText = 4;
    private const int PartyChatReceiveText = 8;

    public enum State { Uninitialized, Ready, Hosting, Joining, InNetwork, Leaving }

    public event Action<State> StateChanged;
    public event Action<PlayFabPartyNetwork> NetworkJoined;
    public event Action NetworkLeft;
    public event Action NetworkDestroyed;
    public event Action<int> PeerConnected;
    public event Action<int> PeerDisconnected;
    public event Action<int, string> ChatReceived;
    public event Action<int, string> RpcReceived;

    private State _state = State.Uninitialized;
    private Auth _auth;
    private Lobby _lobbyNode;
    private PlayFabLobby _lobby;
    private PlayFabPartyNetwork _network;
    private bool _isHost;
    private bool _lobbySignalsConnected;
    private bool _pfPartySignalsConnected;
    private bool _abortPartyOp;
    private bool _teardownInProgress;

    public PlayFabPartyNetwork Network => _state == State.InNetwork ? _network : null;

    public override void _Ready()
    {
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        _lobbyNode = GetNodeOrNull<Lobby>("/root/Lobby");
        if (_auth == null || _lobbyNode == null)
        {
            GD.PushError("[Party] Auth/Lobby autoload missing");
            return;
        }
        _ = EnsureReadyAsync();
    }

    public State GetState() => _state;
    public bool IsReady() => _state == State.Ready;
    public bool IsInNetwork() => _state == State.InNetwork;
    public bool IsBusy() => _state == State.Hosting || _state == State.Joining || _state == State.Leaving;
    public PlayFabPartyNetwork GetCurrentNetwork() => Network;

    private void SetState(State next)
    {
        if (_state == next) return;
        _state = next;
        StateChanged?.Invoke(_state);
    }

    private async Task<bool> EnsureReadyAsync()
    {
        if (_state == State.Ready || _state == State.InNetwork) return true;
        if (IsBusy() || _state != State.Uninitialized) return false;
        if (!await _auth.SignInAsync())
        {
            GD.PushWarning($"[Party] sign-in failed ({_auth.GetLastErrorStage()}) — autoload will not initialize");
            return false;
        }
        if (_state == State.Ready || _state == State.InNetwork) return true;
        if (_state != State.Uninitialized) return false;
        if (!_lobbySignalsConnected)
        {
            _lobbyNode.LobbyJoined += OnLobbyJoinedFromLobbyAutoload;
            _lobbyNode.LobbyLeft += OnLobbyLeftFromLobbyAutoload;
            _lobbyNode.LobbyDisconnected += OnLobbyLeftFromLobbyAutoload;
            _lobbySignalsConnected = true;
        }
        SetState(State.Ready);
        GD.Print("[Party] Lobby wiring connected. PlayFab Party init is lazy.");
        return true;
    }

    private async Task<bool> EnsurePartyInitializedAsync()
    {
        if (!PlayFab.IsAvailable)
        {
            GD.PushError("[Party] PlayFab extension not loaded");
            return false;
        }
        if (!PlayFab.Party.IsInitialized())
        {
            PlayFabPartyConfig cfg = TutorialSupport.PartyConfig(8, PlayFabParty.DIRECTPEERCONNECTIVITYANY, true, true);
            PlayFabResult init = await PlayFab.Party.InitializeAsync(cfg);
            if (!init.Ok)
            {
                GD.PushWarning($"[Party] PlayFab.party init failed: {init.Message} ({init.Code})");
                return false;
            }
            GD.Print("[Party] PlayFab.party initialized lazily (voice=true text=true transcription=false)");
        }
        if (!_pfPartySignalsConnected)
        {
            PlayFab.Party.PartyError += OnPartyError;
            _pfPartySignalsConnected = true;
        }
        return true;
    }

    public async Task<bool> HostPartyAsync()
    {
        if (!await EnsureReadyAsync()) return false;
        if (_state != State.Ready)
        {
            GD.PushWarning($"[Party] host_party rejected — busy or already in network (state={(int)_state})");
            return false;
        }
        SetState(State.Hosting);
        _abortPartyOp = false;
        _isHost = true;
        if (!await EnsurePartyInitializedAsync())
        {
            _isHost = false;
            SetState(State.Ready);
            return false;
        }
        Godot.Collections.Dictionary caps = await ResolveChatCapabilitiesAsync();
        PlayFabPartyConfig cfg = TutorialSupport.PartyConfig(4, PlayFabParty.DIRECTPEERCONNECTIVITYANY,
            TutorialSupport.DictBool(caps, "voice"), TutorialSupport.DictBool(caps, "text"), _lobby?.LobbyId ?? string.Empty);
        PlayFabResult result = await PlayFab.Party.CreateAndJoinNetworkAsync(_auth.PlayFabUser, cfg);
        if (_abortPartyOp || _state != State.Hosting)
        {
            if (result.Ok) _ = result.DataAs<PlayFabPartyNetwork>()?.LeaveAsync();
            _abortPartyOp = false;
            _isHost = false;
            if (_state == State.Hosting) SetState(State.Ready);
            return false;
        }
        if (!result.Ok)
        {
            GD.PushWarning($"[Party] create_and_join failed: {result.Message} ({result.Code})");
            _isHost = false;
            SetState(State.Ready);
            return false;
        }
        PlayFabPartyNetwork net = result.DataAs<PlayFabPartyNetwork>();
        AttachNetwork(net);
        SetState(State.InNetwork);
        GD.Print("[Party] Network created — waiting for descriptor…");
        NetworkJoined?.Invoke(_network);
        if (!string.IsNullOrEmpty(_network.Descriptor)) await PublishDescriptorOnLobbyAsync(_network.Descriptor, net);
        return true;
    }

    private async Task<bool> JoinPartyNetworkAsync(string descriptor)
    {
        if (!await EnsureReadyAsync()) return false;
        if (_state != State.Ready)
        {
            GD.PushWarning($"[Party] join rejected — busy or already in network (state={(int)_state})");
            return false;
        }
        SetState(State.Joining);
        _abortPartyOp = false;
        _isHost = false;
        if (!await EnsurePartyInitializedAsync())
        {
            SetState(State.Ready);
            return false;
        }
        Godot.Collections.Dictionary caps = await ResolveChatCapabilitiesAsync();
        PlayFabPartyConfig cfg = TutorialSupport.PartyConfig(4, PlayFabParty.DIRECTPEERCONNECTIVITYANY,
            TutorialSupport.DictBool(caps, "voice"), TutorialSupport.DictBool(caps, "text"), _lobby?.LobbyId ?? string.Empty);
        PlayFabResult result = await PlayFab.Party.JoinNetworkAsync(_auth.PlayFabUser, descriptor, cfg);
        if (_abortPartyOp || _state != State.Joining)
        {
            if (result.Ok) _ = result.DataAs<PlayFabPartyNetwork>()?.LeaveAsync();
            _abortPartyOp = false;
            if (_state == State.Joining) SetState(State.Ready);
            return false;
        }
        if (!result.Ok)
        {
            GD.PushWarning($"[Party] join_network failed: {result.Message} ({result.Code})");
            SetState(State.Ready);
            return false;
        }
        AttachNetwork(result.DataAs<PlayFabPartyNetwork>());
        SetState(State.InNetwork);
        GD.Print($"[Party] Joined Party network: {_network.NetworkId}");
        NetworkJoined?.Invoke(_network);
        return true;
    }

    public async Task<bool> LeavePartyAsync()
    {
        if (_state != State.InNetwork)
        {
            GD.PushWarning($"[Party] leave_party rejected — not in a network (state={(int)_state})");
            return false;
        }
        SetState(State.Leaving);
        _teardownInProgress = true;
        if (_isHost && _lobby != null && _lobby.IsOwner(_auth.PlayFabUser))
        {
            PlayFabResult clear = await _lobby.SetPropertiesAsync(new Godot.Collections.Dictionary { [PartyDescriptorKey] = string.Empty });
            if (!clear.Ok) GD.PushWarning($"[Party] descriptor clear failed: {clear.Message}");
        }
        PlayFabResult pf = await _network.LeaveAsync();
        if (!pf.Ok) GD.PushWarning($"[Party] leave failed: {pf.Message}");
        DetachNetwork();
        _isHost = false;
        SetState(State.Ready);
        NetworkLeft?.Invoke();
        _teardownInProgress = false;
        return pf.Ok;
    }

    public async Task<Godot.Collections.Dictionary> ResolveChatCapabilitiesAsync()
    {
        bool textAllowed = await HasPrivilegeAsync(XuserPrivilegeCommunications);
        bool voiceAllowed = textAllowed && await HasPrivilegeAsync(XuserPrivilegeCommunicationVoiceIngame);
        return new Godot.Collections.Dictionary { ["text"] = textAllowed, ["voice"] = voiceAllowed };
    }

    public async Task<bool> ToggleMuteAsync(int peerId, bool muted)
    {
        if (_state != State.InNetwork || _network?.LocalPeer == null)
        {
            GD.PushWarning($"[Party] toggle_mute rejected — not in a network (state={(int)_state})");
            return false;
        }
        PlayFabResult pf = await _network.LocalPeer.SetPeerMutedAsync(peerId, muted);
        if (!pf.Ok) GD.PushWarning($"[Party] mute toggle failed: {pf.Message}");
        return pf.Ok;
    }

    public async Task<bool> SendChatAsync(string text)
    {
        if (_state != State.InNetwork || _network?.LocalPeer == null)
        {
            GD.PushWarning($"[Party] send_chat rejected — not in a network (state={(int)_state})");
            return false;
        }
        PlayFabResult pf = await _network.LocalPeer.SendTextAsync(text);
        if (!pf.Ok) GD.PushWarning($"[Party] send_text failed: {pf.Message}");
        return pf.Ok;
    }

    public bool SendRpcPing(string text)
    {
        if (_state != State.InNetwork || _network == null)
        {
            GD.PushWarning($"[Party] send_rpc_ping rejected — not in a network (state={(int)_state})");
            return false;
        }
        if (Multiplayer.MultiplayerPeer == null)
        {
            GD.PushWarning("[Party] send_rpc_ping rejected — multiplayer peer not bound");
            return false;
        }
        if (Multiplayer.GetPeers().Length == 0)
        {
            GD.PushWarning("[Party] send_rpc_ping rejected — no remote peers connected yet");
            return false;
        }
        Error err = Rpc(MethodName.PingMessage, text);
        if (err != Error.Ok)
        {
            GD.PushWarning($"[Party] send_rpc_ping failed: {err}");
            return false;
        }
        return true;
    }

    [Rpc(MultiplayerApi.RpcMode.AnyPeer, TransferMode = MultiplayerPeer.TransferModeEnum.Reliable)]
    public void HandshakeMessage(string text)
    {
        int sender = Multiplayer.GetRemoteSenderId();
        GD.Print($"[Party] RPC from peer {sender}: \"{text}\"");
    }

    [Rpc(MultiplayerApi.RpcMode.AnyPeer, CallLocal = false, TransferMode = MultiplayerPeer.TransferModeEnum.Reliable)]
    public void PingMessage(string text)
    {
        int sender = Multiplayer.GetRemoteSenderId();
        GD.Print($"[Party] ping RPC from peer {sender}: \"{text}\"");
        RpcReceived?.Invoke(sender, text);
    }

    private async Task<bool> HasPrivilegeAsync(int privilege)
    {
        if (_auth.XboxUser == null) return false;
        GodotGdk.GdkResult pf = await Gdk.Users.CheckPrivilegeAsync(_auth.XboxUser, privilege);
        return pf.Ok && TutorialSupport.DictBool(pf.Data.AsGodotDictionary(), "has_privilege");
    }

    private async Task<bool> CheckPermissionAsync(string permission, string peerXuid)
    {
        if (_auth.XboxUser == null) return false;
        GodotGdk.GdkResult pf = await Gdk.Privacy.CheckPermissionAsync(_auth.XboxUser, permission, peerXuid);
        return pf.Ok && TutorialSupport.DictBool(pf.Data.AsGodotDictionary(), "allowed");
    }

    private async Task PublishDescriptorOnLobbyAsync(string descriptor, PlayFabPartyNetwork expectedNetwork)
    {
        if (_state != State.InNetwork || !_isHost || _network != expectedNetwork || _lobby == null) return;
        if (!_lobby.IsOwner(_auth.PlayFabUser)) return;
        GD.Print("[Party] Descriptor ready, publishing on the lobby");
        PlayFabResult pf = await _lobby.SetPropertiesAsync(new Godot.Collections.Dictionary { [PartyDescriptorKey] = descriptor });
        if (!pf.Ok && _state == State.InNetwork && _network == expectedNetwork) GD.PushWarning($"[Party] descriptor publish failed: {pf.Message}");
    }

    private void AttachLobby(PlayFabLobby lobby)
    {
        if (_lobby == lobby) return;
        DetachLobby();
        _lobby = lobby;
        if (_lobby != null) _lobby.StateChanged += OnLobbyStateChanged;
    }

    private void DetachLobby()
    {
        if (_lobby != null) _lobby.StateChanged -= OnLobbyStateChanged;
        _lobby = null;
    }

    private void AttachNetwork(PlayFabPartyNetwork net)
    {
        DetachNetwork();
        _network = net;
        if (_network == null) return;
        _network.StateChanged += OnNetworkStateChanged;
        PlayFabPartyPeer peer = _network.LocalPeer;
        if (peer == null) return;
        Multiplayer.MultiplayerPeer = peer.AsMultiplayerPeer;
        peer.TextMessageReceived += OnPartyTextReceived;
        peer.ConnectionStateChanged += OnPartyConnectionStateChanged;
        peer.ChatControlAdded += OnChatControlAdded;
        foreach (Variant rawId in peer.GetPeers())
        {
            int peerId = rawId.AsInt32();
            peer_connected(peerId, peer);
        }
        if (peer.AsMultiplayerPeer != null && peer.AsMultiplayerPeer.GetConnectionStatus() == MultiplayerPeer.ConnectionStatus.Connected)
        {
            Rpc(MethodName.HandshakeMessage, "ready");
        }
    }

    private void peer_connected(int peerId, PlayFabPartyPeer peer)
    {
        PeerConnected?.Invoke(peerId);
        PlayFabPartyChatControl ctrl = peer.GetPeerChatControl(peerId);
        if (ctrl != null) _ = HandleChatControlAddedAsync(peerId, ctrl);
    }

    private void DetachNetwork()
    {
        if (_network != null)
        {
            _network.StateChanged -= OnNetworkStateChanged;
            PlayFabPartyPeer peer = _network.LocalPeer;
            if (peer != null)
            {
                peer.TextMessageReceived -= OnPartyTextReceived;
                peer.ConnectionStateChanged -= OnPartyConnectionStateChanged;
                peer.ChatControlAdded -= OnChatControlAdded;
            }
        }
        ClearMultiplayerPeer();
        _network = null;
    }

    private void OnLobbyJoinedFromLobbyAutoload(PlayFabLobby lobby)
    {
        AttachLobby(lobby);
        if (_isHost || _state != State.Ready) return;
        string descriptor = TutorialSupport.DictString(lobby.Properties, PartyDescriptorKey);
        if (!string.IsNullOrEmpty(descriptor)) _ = JoinPartyNetworkAsync(descriptor);
    }

    private void OnLobbyLeftFromLobbyAutoload()
    {
        DetachLobby();
        if (IsBusy() && _state != State.Leaving)
        {
            _abortPartyOp = true;
            GD.PushWarning($"[Party] Lobby left while busy (state={(int)_state}); in-flight op will abort on completion");
            return;
        }
        if (_state == State.InNetwork) _ = LeavePartyAsync();
    }

    private void OnLobbyStateChanged(PlayFabLobbyStateChange change)
    {
        if (change.Kind != PlayFabLobby.PROPERTIESUPDATED || _isHost || _state != State.Ready) return;
        string descriptor = TutorialSupport.DictString(change.Lobby?.Properties, PartyDescriptorKey);
        if (!string.IsNullOrEmpty(descriptor)) _ = JoinPartyNetworkAsync(descriptor);
    }

    private void OnNetworkStateChanged(PlayFabPartyNetworkStateChange change)
    {
        switch (change.Kind)
        {
            case PlayFabParty.NETWORKCHANGEDESCRIPTORUPDATED:
                if (_isHost && _state == State.InNetwork && _network != null && !string.IsNullOrEmpty(_network.Descriptor))
                    _ = PublishDescriptorOnLobbyAsync(_network.Descriptor, _network);
                break;
            case PlayFabParty.NETWORKCHANGEPEERJOINED:
                GD.Print($"[Party] Peer connected: id={change.PeerId}");
                PeerConnected?.Invoke(change.PeerId);
                break;
            case PlayFabParty.NETWORKCHANGEPEERLEFT:
                GD.Print($"[Party] Peer {change.PeerId} left");
                PeerDisconnected?.Invoke(change.PeerId);
                break;
            case PlayFabParty.NETWORKCHANGESTATE:
                GD.Print($"[Party] State → {change.State} ({change.Reason})");
                break;
            case PlayFabParty.NETWORKCHANGEERROR:
                GD.PushWarning($"[Party] network error: {change.Reason}");
                break;
            case PlayFabParty.NETWORKCHANGEDESTROYED:
                HandleNetworkDestroyed(change.Reason);
                break;
        }
    }

    private void HandleNetworkDestroyed(string reason)
    {
        if (_teardownInProgress || _state == State.Leaving) return;
        if (_state != State.InNetwork || !IsInsideTree()) return;
        GD.Print($"[Party] Network destroyed ({reason})");
        DetachNetwork();
        _isHost = false;
        SetState(State.Ready);
        NetworkDestroyed?.Invoke();
    }

    private void ClearMultiplayerPeer()
    {
        if (!IsInsideTree() || Multiplayer == null) return;
        Multiplayer.MultiplayerPeer = null;
    }

    private void OnPartyTextReceived(int peerId, PlayFabPartyChatMessage message)
    {
        GD.Print($"[Party] Text from peer {peerId}: \"{message.Text}\"");
        ChatReceived?.Invoke(peerId, message.Text);
    }

    private void OnPartyConnectionStateChanged(int status)
    {
        if (status == (int)MultiplayerPeer.ConnectionStatus.Disconnected) GD.Print("[Party] Multiplayer peer disconnected");
    }

    private void OnChatControlAdded(int peerId, PlayFabPartyChatControl control) => _ = HandleChatControlAddedAsync(peerId, control);

    private async Task HandleChatControlAddedAsync(int peerId, PlayFabPartyChatControl control)
    {
        string peerXuid = XuidForPeer(peerId);
        if (string.IsNullOrEmpty(peerXuid)) return;
        bool allowVoice = await CheckPermissionAsync("communicate_using_voice", peerXuid);
        bool allowText = await CheckPermissionAsync("communicate_using_text", peerXuid);
        int permissions = PartyChatNone;
        if (allowVoice) permissions |= PartyChatSendAudio | PartyChatReceiveAudio;
        if (allowText) permissions |= PartyChatSendText | PartyChatReceiveText;
        if (_state != State.InNetwork || _network?.LocalPeer == null) return;
        PlayFabResult pf = await _network.LocalPeer.SetPeerChatPermissionsAsync(peerId, permissions);
        if (!pf.Ok) GD.PushWarning($"[Party] chat permissions for peer {peerId} failed: {pf.Message}");
    }

    private string XuidForPeer(int peerId)
    {
        if (_network?.LocalPeer == null || _lobby == null) return string.Empty;
        Godot.Collections.Dictionary key = _network.LocalPeer.GetPeerEntityKey(peerId);
        string entityId = TutorialSupport.DictString(key, "id");
        if (string.IsNullOrEmpty(entityId)) return string.Empty;
        foreach (Variant memberValue in _lobby.Members)
        {
            PlayFabLobbyMember member = PlayFabLobbyMember.From(memberValue.AsGodotObject());
            if (TutorialSupport.DictString(member.EntityKey, "id") == entityId)
                return TutorialSupport.DictString(member.Properties, "xuid");
        }
        return string.Empty;
    }

    private void OnPartyError(PlayFabResult result) => GD.PushWarning($"[Party] party error: {result.Message} ({result.Code})");
}


