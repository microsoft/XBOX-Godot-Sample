using Godot;
using System.Threading.Tasks;
using GodotGdk;

public partial class PanelMpa : VBoxContainer
{
    private Label _state;
    private OptionButton _friends;
    private Button _refresh, _send, _picker;
    private RichTextLabel _log;
    private ConfirmationDialog _inviteDialog;
    private Auth _auth;
    private Lobby _lobbyNode;
    private int _dialogInviteId;
    private bool _initialized;
    public override async void _Ready()
    {
        _state = GetNode<Label>("State"); _friends = GetNode<OptionButton>("Friends"); _refresh = GetNode<Button>("ButtonRow/Refresh"); _send = GetNode<Button>("ButtonRow/Send"); _picker = GetNode<Button>("ButtonRow/Picker"); _log = GetNode<RichTextLabel>("Log"); _inviteDialog = GetNode<ConfirmationDialog>("InviteDialog");
        _auth = GetNodeOrNull<Auth>("/root/Auth"); _lobbyNode = GetNodeOrNull<Lobby>("/root/Lobby"); if (_auth == null || _lobbyNode == null) { _state.Text = "[ERR] Auth/Lobby autoload missing"; return; }
        _auth.StateChanged += state => { if (!_initialized && _auth.IsSignedIn()) _ = InitializeAfterSignInAsync(); };
        if (_auth.IsSignedIn()) await InitializeAfterSignInAsync(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) await InitializeAfterSignInAsync(); }
    }
    private async Task InitializeAfterSignInAsync()
    {
        if (_initialized) return; _initialized = true; _refresh.Pressed += async () => await OnRefreshPressed(); _send.Pressed += async () => await OnSendPressed(); _picker.Pressed += async () => await OnPickerPressed(); _inviteDialog.Confirmed += async () => await OnInviteDialogConfirmed(); _inviteDialog.Canceled += () => _lobbyNode.RejectPendingInvite(_dialogInviteId);
        Gdk.MultiplayerActivity.InviteAccepted += invite => _log.AppendText($"Accepted: {TutorialSupport.DictString(invite, "raw_uri")}\n");
        Gdk.MultiplayerActivity.PendingInviteReceived += invite => _log.AppendText($"Pending: {TutorialSupport.DictString(invite, "raw_uri")}\n");
        _lobbyNode.InvitePendingConfirmation += OnInvitePendingConfirmation; _lobbyNode.InvitePendingCleared += OnInvitePendingCleared; RefreshState(); await OnRefreshPressed();
    }
    private void RefreshState()
    {
        GodotPlayFab.Types.PlayFabLobby current = _lobbyNode.GetCurrentLobby(); _state.Text = current == null ? "No lobby — activity not advertised" : $"Advertising {(current.LobbyId.Length > 8 ? current.LobbyId[..8] : current.LobbyId)} ({current.MemberCount} / {current.MaxMemberCount}, cross=False)";
    }
    private async Task OnRefreshPressed()
    {
        _friends.Clear(); _friends.AddItem("(loading…)"); _friends.SetItemDisabled(0, true); Godot.Collections.Array list = await _lobbyNode.GetFriendsAsync(); if (!IsInsideTree()) return;
        _friends.Clear(); if (list.Count == 0) { _friends.AddItem("(no friends found)"); _friends.SetItemDisabled(0, true); return; }
        foreach (Variant f in list) { GodotObject friend = f.AsGodotObject(); string label = friend.Get("gamertag").AsString(); if (string.IsNullOrEmpty(label)) label = friend.Get("display_name").AsString(); if (string.IsNullOrEmpty(label)) label = friend.Get("xuid").AsString(); int idx = _friends.ItemCount; _friends.AddItem(label); _friends.SetItemMetadata(idx, friend.Get("xuid").AsString()); }
    }
    private async Task OnSendPressed()
    {
        if (_friends.ItemCount == 0 || _friends.IsItemDisabled(_friends.Selected)) { _log.AppendText("[i]Refresh first and pick a friend[/i]\n"); return; }
        Variant xuidVar = _friends.GetItemMetadata(_friends.Selected); if (xuidVar.VariantType != Variant.Type.String || string.IsNullOrEmpty(xuidVar.AsString())) { _log.AppendText("[i]No XUID on selected entry[/i]\n"); return; }
        string xuid = xuidVar.AsString(); bool sent = await _lobbyNode.InviteFriendAsync(xuid); if (!IsInsideTree()) return; _log.AppendText(sent ? $"Sent invite to {xuid}\n" : $"[i]Invite to {xuid} suppressed (no lobby, permission-denied, or send failure)[/i]\n");
    }
    private async Task OnPickerPressed() { await _lobbyNode.OpenInvitePickerAsync(); if (IsInsideTree()) _log.AppendText("Closed system invite picker\n"); }
    private void OnInvitePendingConfirmation(int inviteId, string connectionString) { _dialogInviteId = inviteId; _log.AppendText("[i]Invite received while in a lobby — see prompt[/i]\n"); _inviteDialog.PopupCentered(); }
    private void OnInvitePendingCleared(int inviteId) { if (inviteId == _dialogInviteId && _inviteDialog.Visible) _inviteDialog.Hide(); }
    private async Task OnInviteDialogConfirmed() { if (!await _lobbyNode.ConfirmPendingInviteAsync(_dialogInviteId) && IsInsideTree()) _log.AppendText("[i]Invite accept dropped — pending invite was stale or leave/join failed[/i]\n"); }
}


