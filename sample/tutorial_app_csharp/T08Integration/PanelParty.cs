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
    private bool _initialized;
    public override async void _Ready()
    {
        _peerList = GetNode<Label>("PeerList"); _chatLog = GetNode<RichTextLabel>("ChatLog"); _chatInput = GetNode<LineEdit>("ChatInput"); _send = GetNode<Button>("Send"); _muteRemotes = GetNode<CheckButton>("MuteRemotes");
        _auth = GetNodeOrNull<Auth>("/root/Auth"); _partyNode = GetNodeOrNull<Party>("/root/Party"); if (_auth == null || _partyNode == null) { _peerList.Text = "[ERR] Auth/Party autoload missing"; return; }
        _auth.StateChanged += _ => { if (!_initialized && _auth.IsSignedIn()) InitializeAfterSignIn(); };
        if (_auth.IsSignedIn()) InitializeAfterSignIn(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) InitializeAfterSignIn(); }
    }
    private void InitializeAfterSignIn()
    {
        if (_initialized) return; _initialized = true; _send.Pressed += async () => await OnSendPressed(); _muteRemotes.Toggled += OnMuteRemotesToggled; _partyNode.NetworkJoined += AttachNetwork; _partyNode.NetworkLeft += () => AttachNetwork(null); _partyNode.NetworkDestroyed += () => { AttachNetwork(null); _chatLog.AppendText("[i]Party network destroyed (lobby host left, network error, or shutdown)[/i]\n"); }; _partyNode.StateChanged += OnPartyStateChanged; AttachNetwork(_partyNode.Network);
    }
    private void AttachNetwork(PlayFabPartyNetwork network)
    {
        _network = network; _peer = _network?.LocalPeer; if (_peer != null) { _peer.TextMessageReceived += OnTextReceived; _peer.ChatControlAdded += (_, _) => RefreshPeers(); _peer.ChatControlRemoved += _ => RefreshPeers(); } RefreshPeers();
    }
    private void OnPartyStateChanged(Party.State state) { if (state == Party.State.Hosting) _peerList.Text = "Bringing up Party network…"; else if (state == Party.State.Joining) _peerList.Text = "Joining Party network…"; else if (state == Party.State.Leaving) _peerList.Text = "Leaving Party network…"; }
    private async System.Threading.Tasks.Task OnSendPressed()
    {
        string text = _chatInput.Text.StripEdges(); if (string.IsNullOrEmpty(text) || _peer == null) return; PlayFabResult result = await _peer.SendTextAsync(text); if (!IsInsideTree()) return; if (result.Ok) { _chatLog.AppendText($"[me] {text}\n"); _chatInput.Text = string.Empty; } else _chatLog.AppendText($"[i]send_text_async failed: {result.Message}[/i]\n");
    }
    private void OnMuteRemotesToggled(bool pressed) { if (_peer == null) return; foreach (Variant id in _peer.GetPeers()) _ = _peer.SetPeerMutedAsync(id.AsInt32(), pressed); }
    private void OnTextReceived(int peerId, PlayFabPartyChatMessage message) { string label = "?"; if (_peer != null) { string id = TutorialSupport.DictString(_peer.GetPeerEntityKey(peerId), "id", "?"); label = id.Length > 8 ? id[..8] : id; } _chatLog.AppendText($"[{label}] {message.Text}\n"); }
    private void RefreshPeers()
    {
        if (_peer == null) { _peerList.Text = "Not connected"; return; } var lines = new System.Collections.Generic.List<string>(); foreach (Variant idValue in _peer.GetPeers()) { int peerId = idValue.AsInt32(); string id = TutorialSupport.DictString(_peer.GetPeerEntityKey(peerId), "id", "?"); lines.Add($"- {(id.Length > 8 ? id[..8] : id)} (peer {peerId})"); } if (lines.Count == 0) lines.Add("- (waiting for remote peers)"); _peerList.Text = string.Join("\n", lines);
    }
}


