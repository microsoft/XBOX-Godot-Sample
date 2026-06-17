using Godot;
using System.Threading.Tasks;
using GodotPlayFab.Types;

public partial class T07Party : Control
{
    private Label _statusLabel, _networkLabel;
    private Button _hostButton, _joinButton, _leaveButton, _sendButton, _pingButton, _backButton;
    private LineEdit _joinLobbyId, _chatInput;
    private TextEdit _chatLog;
    private Auth _auth;
    private Lobby _lobbyNode;
    private Party _partyNode;

    public override async void _Ready()
    {
        _statusLabel = GetNode<Label>("Root/Status");
        _networkLabel = GetNode<Label>("Root/NetworkLabel");
        _hostButton = GetNode<Button>("Root/Buttons/Host");
        _joinLobbyId = GetNode<LineEdit>("Root/JoinRow/LobbyId");
        _joinButton = GetNode<Button>("Root/JoinRow/Join");
        _leaveButton = GetNode<Button>("Root/Buttons/Leave");
        _chatInput = GetNode<LineEdit>("Root/ChatRow/Message");
        _sendButton = GetNode<Button>("Root/ChatRow/Send");
        _pingButton = GetNode<Button>("Root/ChatRow/Ping");
        _chatLog = GetNode<TextEdit>("Root/ChatLog");
        _backButton = GetNode<Button>("Root/Back");
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        _lobbyNode = GetNodeOrNull<Lobby>("/root/Lobby");
        _partyNode = GetNodeOrNull<Party>("/root/Party");
        _hostButton.Pressed += async () => await OnHostPressed();
        _joinButton.Pressed += async () => await OnJoinPressed();
        _leaveButton.Pressed += async () => await OnLeavePressed();
        _sendButton.Pressed += async () => await OnSendPressed();
        _pingButton.Pressed += OnPingPressed;
        _backButton.Pressed += OnBackPressed;
        SetButtonsForState(false);
        _statusLabel.Text = "Sign-in pending.";
        _networkLabel.Text = "Network: (none)";
        _sendButton.Disabled = true; _pingButton.Disabled = true;
        if (_auth == null || _lobbyNode == null || _partyNode == null) { _statusLabel.Text = "[ERR] Auth/Lobby/Party autoload missing"; return; }
        if (!await _auth.SignInAsync()) { _statusLabel.Text = $"Sign-in failed ({_auth.GetLastErrorStage()}): {_auth.GetLastErrorMessage()}"; return; }
        _statusLabel.Text = "Signed in. Host or join a lobby to bring up the Party network.";
        _lobbyNode.LobbyJoined += OnLobbyJoined;
        _lobbyNode.LobbyLeft += OnLobbyLeft;
        _lobbyNode.LobbyDisconnected += OnLobbyDisconnected;
        _lobbyNode.StateChanged += OnLobbyStateChanged;
        _partyNode.NetworkJoined += OnNetworkJoined;
        _partyNode.NetworkLeft += OnNetworkLeft;
        _partyNode.NetworkDestroyed += OnNetworkDestroyed;
        _partyNode.PeerConnected += id => AppendLog($"[peer connected] id={id}");
        _partyNode.PeerDisconnected += id => AppendLog($"[peer left] id={id}");
        _partyNode.ChatReceived += (id, text) => AppendLog($"peer {id}> {text}");
        _partyNode.RpcReceived += (id, text) => AppendLog($"peer {id} (rpc)> {text}");
        _partyNode.StateChanged += OnPartyStateChanged;
        SetButtonsForState(true);
    }

    private void SetButtonsForState(bool signedIn) { _hostButton.Disabled = !signedIn; _joinButton.Disabled = !signedIn; _leaveButton.Disabled = true; }
    private void AppendLog(string line) => _chatLog.Text += line + "\n";
    private async Task OnHostPressed() { _statusLabel.Text = "Hosting lobby + Party network..."; _hostButton.Disabled = true; _joinButton.Disabled = true; await _lobbyNode.HostLobbyAsync(); }
    private async Task OnJoinPressed()
    {
        string connection = _joinLobbyId.Text.StripEdges();
        if (string.IsNullOrEmpty(connection)) { _statusLabel.Text = "Paste a lobby connection string before Join."; return; }
        _statusLabel.Text = "Joining lobby + Party network..."; _hostButton.Disabled = true; _joinButton.Disabled = true; await _lobbyNode.JoinLobbyAsync(connection);
    }
    private async Task OnLeavePressed()
    {
        _statusLabel.Text = "Leaving Party and lobby...";
        if (_partyNode.GetCurrentNetwork() != null) await _partyNode.LeavePartyAsync();
        await _lobbyNode.LeaveLobbyAsync();
        _statusLabel.Text = "Left. Ready to host or join again.";
        _hostButton.Disabled = false; _joinButton.Disabled = false; _leaveButton.Disabled = true; _sendButton.Disabled = true; _pingButton.Disabled = true; _networkLabel.Text = "Network: (none)";
    }
    private async Task OnSendPressed()
    {
        string text = _chatInput.Text;
        if (string.IsNullOrEmpty(text)) return;
        _chatInput.Clear();
        AppendLog(await _partyNode.SendChatAsync(text) ? "you> " + text : "[send failed]");
    }
    private void OnPingPressed()
    {
        string text = $"ping @{Time.GetTicksMsec()}";
        AppendLog(_partyNode.SendRpcPing(text) ? "you (rpc)> " + text : "[ping failed — not in a network]");
    }
    private async void OnLobbyJoined(PlayFabLobby lobby)
    {
        _statusLabel.Text = $"Lobby ready: {lobby.LobbyId}";
        _joinLobbyId.Text = lobby.ConnectionString;
        _joinLobbyId.CaretColumn = lobby.ConnectionString.Length;
        _joinLobbyId.SelectAll(); _joinLobbyId.GrabFocus(); _leaveButton.Disabled = false;
        if (_auth.PlayFabUser != null && lobby.IsOwner(_auth.PlayFabUser)) await _partyNode.HostPartyAsync();
    }
    private void OnLobbyLeft() { _statusLabel.Text = "Lobby ended."; _hostButton.Disabled = false; _joinButton.Disabled = false; _leaveButton.Disabled = true; _sendButton.Disabled = true; _pingButton.Disabled = true; _networkLabel.Text = "Network: (none)"; }
    private void OnLobbyDisconnected() { _statusLabel.Text = "Disconnected from lobby (kicked or network error)."; _hostButton.Disabled = false; _joinButton.Disabled = false; _leaveButton.Disabled = true; _sendButton.Disabled = true; _pingButton.Disabled = true; _networkLabel.Text = "Network: (none)"; }
    private void OnNetworkJoined(PlayFabPartyNetwork network) { _networkLabel.Text = $"Network: {network.NetworkId}"; _sendButton.Disabled = false; _pingButton.Disabled = false; _statusLabel.Text = "Party network up. Voice/text chat active."; }
    private void OnNetworkLeft() { _networkLabel.Text = "Network: (none)"; _sendButton.Disabled = true; _pingButton.Disabled = true; }
    private void OnNetworkDestroyed() { _networkLabel.Text = "Network: (lost)"; _sendButton.Disabled = true; _pingButton.Disabled = true; _statusLabel.Text = "Party network destroyed (lobby host left, network error, or shutdown)."; }
    private void OnLobbyStateChanged(Lobby.State state) { if (state == Lobby.State.Hosting) _statusLabel.Text = "Hosting lobby…"; else if (state == Lobby.State.Joining) _statusLabel.Text = "Joining lobby…"; else if (state == Lobby.State.Leaving) _statusLabel.Text = "Leaving lobby…"; }
    private void OnPartyStateChanged(Party.State state) { if (state == Party.State.Hosting) _statusLabel.Text = "Bringing up Party network…"; else if (state == Party.State.Joining) _statusLabel.Text = "Joining Party network…"; else if (state == Party.State.Leaving) _statusLabel.Text = "Tearing down Party network…"; }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}

