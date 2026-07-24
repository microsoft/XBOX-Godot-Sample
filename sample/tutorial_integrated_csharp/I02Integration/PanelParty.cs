using Godot;
using GodotPlayFab.Types;
using GodotPlayFab;

public partial class PanelParty : VBoxContainer
{
    private Label _peerList;
    private RichTextLabel _chatLog;
    private LineEdit _chatInput;
    private Button _send;
    private CheckButton _muteRemotes;
    private Auth _auth;
    private Party _partyNode;
    private PlayFabPartyNetwork _network;
    private PlayFabPartyPeer _peer;
    private PlayFabPartyChat _chat;
    private bool _initialized;
    public override async void _Ready()
    {
        _peerList = GetNode<Label>("PeerList"); _chatLog = GetNode<RichTextLabel>("ChatLog"); _chatInput = GetNode<LineEdit>("ChatInput"); _send = GetNode<Button>("Send"); _muteRemotes = GetNode<CheckButton>("MuteRemotes");
        _auth = GetNodeOrNull<Auth>("/root/Auth"); _partyNode = GetNodeOrNull<Party>("/root/Party"); if (_auth == null || _partyNode == null) { _peerList.Text = "[ERR] Auth/Party autoload missing"; return; }
        _auth.StateChanged += OnAuthStateChanged;
        if (_auth.IsSignedIn()) InitializeAfterSignIn(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) InitializeAfterSignIn(); }
    }
    public override void _ExitTree()
    {
        if (_auth != null) _auth.StateChanged -= OnAuthStateChanged;
        if (!_initialized) return;
        if (_partyNode != null)
        {
            _partyNode.NetworkJoined -= AttachNetwork;
            _partyNode.NetworkLeft -= OnNetworkLeft;
            _partyNode.NetworkDestroyed -= OnNetworkDestroyed;
            _partyNode.StateChanged -= OnPartyStateChanged;
        }
        if (_chat != null)
        {
            _chat.TextMessageReceived -= OnTextReceived;
            _chat.ChatControlAdded -= OnChatControlAdded;
            _chat.ChatControlRemoved -= OnChatControlRemoved;
        }
        AttachNetwork(null);
    }
    private void OnAuthStateChanged(Auth.State state)
    {
        if (!_initialized && _auth != null && _auth.IsSignedIn()) InitializeAfterSignIn();
    }
    private void InitializeAfterSignIn()
    {
        if (_initialized) return; _initialized = true; _send.Pressed += async () => await OnSendPressed(); _muteRemotes.Toggled += OnMuteRemotesToggled; _partyNode.NetworkJoined += AttachNetwork; _partyNode.NetworkLeft += OnNetworkLeft; _partyNode.NetworkDestroyed += OnNetworkDestroyed; _partyNode.StateChanged += OnPartyStateChanged;
        // Chat is meshed on the persistent PlayFab.party.chat surface (reused
        // across networks), so wire its signals once here. The peer list still
        // comes from the per-network transport peer via AttachNetwork/RefreshPeers.
        _chat = PlayFab.Party.GetChat(); _chat.TextMessageReceived += OnTextReceived; _chat.ChatControlAdded += OnChatControlAdded; _chat.ChatControlRemoved += OnChatControlRemoved;
        AttachNetwork(_partyNode.Network);
    }
    private void AttachNetwork(PlayFabPartyNetwork network)
    {
        if (network == _network) return;
        if (_network != null) _network.StateChanged -= OnNetworkStateChanged;
        _network = network;
        if (_network == null) { _peer = null; RefreshPeers(); return; }
        _peer = _network.LocalPeer; _network.StateChanged += OnNetworkStateChanged; RefreshPeers();
    }
    private void OnNetworkLeft() => AttachNetwork(null);
    private void OnNetworkDestroyed() { AttachNetwork(null); _chatLog.AppendText("[i]Party network destroyed (lobby host left, network error, or shutdown)[/i]\n"); }
    private void OnPartyStateChanged(Party.State state) { if (state == Party.State.Hosting) _peerList.Text = "Bringing up Party network…"; else if (state == Party.State.Joining) _peerList.Text = "Joining Party network…"; else if (state == Party.State.Leaving) _peerList.Text = "Leaving Party network…"; }
    private async System.Threading.Tasks.Task OnSendPressed()
    {
        string text = _chatInput.Text.StripEdges(); if (string.IsNullOrEmpty(text) || _peer == null || _chat == null) return; PlayFabResult result = await _chat.SendTextAsync(text, new Godot.Collections.Array()); if (!IsInsideTree()) return; if (result.Ok) { _chatLog.AppendText($"[me] {text}\n"); _chatInput.Text = string.Empty; } else _chatLog.AppendText($"[i]send_text_async failed: {result.Message}[/i]\n");
    }
    private void OnMuteRemotesToggled(bool pressed) { if (_peer == null || _chat == null) return; foreach (Variant id in _peer.GetPeers()) { Godot.Collections.Dictionary entityKey = _peer.GetPeerEntityKey(id.AsInt32()); _ = _chat.SetAudioMutedAsync(entityKey, pressed); } }
    private void OnNetworkStateChanged(PlayFabPartyNetworkStateChange change) => RefreshPeers();
    private void OnTextReceived(Godot.Collections.Dictionary entityKey, PlayFabPartyChatMessage message) { string id = TutorialSupport.DictString(entityKey, "id", "?"); string label = id.Length > 8 ? id[..8] : id; _chatLog.AppendText($"[{label}] {message.Text}\n"); }
    private void OnChatControlAdded(Godot.Collections.Dictionary entityKey, PlayFabPartyChatControl control) => RefreshPeers();
    private void OnChatControlRemoved(Godot.Collections.Dictionary entityKey) => RefreshPeers();
    private void RefreshPeers()
    {
        if (_peer == null) { _peerList.Text = "Not connected"; return; } var lines = new System.Collections.Generic.List<string>(); foreach (Variant idValue in _peer.GetPeers()) { int peerId = idValue.AsInt32(); string id = TutorialSupport.DictString(_peer.GetPeerEntityKey(peerId), "id", "?"); lines.Add($"- {(id.Length > 8 ? id[..8] : id)} (peer {peerId})"); } if (lines.Count == 0) lines.Add("- (waiting for remote peers)"); _peerList.Text = string.Join("\n", lines);
    }
}
