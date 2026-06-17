using Godot;
using System.Threading.Tasks;

public partial class T06Mpa : Control
{
    private RichTextLabel _log;
    private ItemList _friendsList;
    private Button _refreshBtn, _inviteBtn, _pickerBtn, _trackBtn, _stopBtn, _backBtn;
    private ConfirmationDialog _inviteDialog;
    private Lobby _lobbyNode;
    private int _dialogInviteId;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _friendsList = GetNode<ItemList>("Root/Friends");
        _refreshBtn = GetNode<Button>("Root/Buttons/RefreshBtn");
        _inviteBtn = GetNode<Button>("Root/Buttons/InviteBtn");
        _pickerBtn = GetNode<Button>("Root/Buttons/PickerBtn");
        _trackBtn = GetNode<Button>("Root/Buttons/TrackBtn");
        _stopBtn = GetNode<Button>("Root/Buttons/StopBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _inviteDialog = GetNode<ConfirmationDialog>("InviteDialog");
        _backBtn.Pressed += OnBackPressed;
        _refreshBtn.Pressed += async () => await OnRefreshPressed();
        _inviteBtn.Pressed += async () => await OnInvitePressed();
        _pickerBtn.Pressed += async () => await _lobbyNode.OpenInvitePickerAsync();
        _trackBtn.Pressed += async () => await OnTrackPressed();
        _stopBtn.Pressed += () => _lobbyNode.StopTrackingFriends();
        _inviteDialog.Confirmed += async () => await OnInviteDialogConfirmed();
        _inviteDialog.Canceled += OnInviteDialogCanceled;
        _lobbyNode = GetNodeOrNull<Lobby>("/root/Lobby");
        if (_lobbyNode == null) { Append("[color=red]Lobby autoload missing.[/color]"); SetButtonsEnabled(false); return; }
        _lobbyNode.InvitePendingConfirmation += OnInvitePendingConfirmation;
        _lobbyNode.InvitePendingCleared += OnInvitePendingCleared;
        Auth auth = GetNodeOrNull<Auth>("/root/Auth");
        if (auth == null || !await auth.SignInAsync()) { Append("[color=red]Sign-in failed.[/color]"); SetButtonsEnabled(false); return; }
        Append("Signed in. Host a lobby in T5 first, then invite or track friends here.");
        SetButtonsEnabled(true);
        await OnRefreshPressed();
    }

    private async Task OnRefreshPressed()
    {
        _friendsList.Clear(); _friendsList.AddItem("(loading friends…)"); _friendsList.SetItemDisabled(0, true);
        Godot.Collections.Array friends = await _lobbyNode.GetFriendsAsync();
        _friendsList.Clear();
        if (friends.Count == 0) { _friendsList.AddItem("(no friends found)"); _friendsList.SetItemDisabled(0, true); Append("[i]No friends returned by Social Manager.[/i]"); return; }
        foreach (Variant f in friends)
        {
            GodotObject friend = f.AsGodotObject();
            string gamertag = friend.Get("gamertag").AsString();
            string display = friend.Get("display_name").AsString();
            string xuid = friend.Get("xuid").AsString();
            int idx = _friendsList.AddItem($"{(string.IsNullOrEmpty(gamertag) ? (string.IsNullOrEmpty(display) ? "(unknown)" : display) : gamertag)}  —  {xuid}");
            _friendsList.SetItemMetadata(idx, xuid);
        }
        Append($"Loaded {friends.Count} friends.");
    }

    private async Task OnInvitePressed()
    {
        string[] xuids = SelectedXuids();
        if (xuids.Length == 0) { Append("[color=orange]Select a friend in the list above first.[/color]"); return; }
        foreach (string xuid in xuids) await _lobbyNode.InviteFriendAsync(xuid);
    }
    private async Task OnTrackPressed()
    {
        string[] xuids = SelectedXuids();
        if (xuids.Length == 0) { Append("[color=orange]Select one or more friends in the list above first.[/color]"); return; }
        await _lobbyNode.TrackFriendActivitiesAsync(xuids);
    }
    private string[] SelectedXuids()
    {
        var list = new System.Collections.Generic.List<string>();
        foreach (int idx in _friendsList.GetSelectedItems())
        {
            Variant meta = _friendsList.GetItemMetadata(idx);
            if (meta.VariantType == Variant.Type.String && !string.IsNullOrEmpty(meta.AsString())) list.Add(meta.AsString());
        }
        return list.ToArray();
    }
    private void OnInvitePendingConfirmation(int inviteId, string connectionString) { _dialogInviteId = inviteId; Append("[i]Invite received while in a lobby — see prompt.[/i]"); _inviteDialog.PopupCentered(); }
    private void OnInvitePendingCleared(int inviteId) { if (inviteId == _dialogInviteId && _inviteDialog.Visible) _inviteDialog.Hide(); }
    private async Task OnInviteDialogConfirmed() { Append("[i]Accepting invite — leaving current lobby and joining the invited one…[/i]"); if (!await _lobbyNode.ConfirmPendingInviteAsync(_dialogInviteId)) Append("[i]Invite accept dropped — pending invite was stale or leave/join failed.[/i]"); }
    private void OnInviteDialogCanceled() => _lobbyNode.RejectPendingInvite(_dialogInviteId);
    private void SetButtonsEnabled(bool enabled) { _refreshBtn.Disabled = !enabled; _inviteBtn.Disabled = !enabled; _pickerBtn.Disabled = !enabled; _trackBtn.Disabled = !enabled; _stopBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}

