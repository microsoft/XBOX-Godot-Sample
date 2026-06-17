using Godot;
using System.Threading.Tasks;
using GodotPlayFab.Types;

public partial class T05Lobby : Control
{
    private RichTextLabel _log;
    private LineEdit _connEdit;
    private Button _hostBtn, _joinBtn, _leaveBtn, _loadoutBtn, _mapBtn, _backBtn;
    private ItemList _members;
    private Lobby _lobbyNode;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _connEdit = GetNode<LineEdit>("Root/JoinRow/ConnEdit");
        _hostBtn = GetNode<Button>("Root/Buttons/HostBtn");
        _joinBtn = GetNode<Button>("Root/Buttons/JoinBtn");
        _leaveBtn = GetNode<Button>("Root/Buttons/LeaveBtn");
        _loadoutBtn = GetNode<Button>("Root/Buttons/LoadoutBtn");
        _mapBtn = GetNode<Button>("Root/Buttons/MapBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _members = GetNode<ItemList>("Root/MembersPanel/Members");
        _backBtn.Pressed += OnBackPressed;
        _hostBtn.Pressed += async () => await _lobbyNode.HostLobbyAsync();
        _joinBtn.Pressed += async () => await OnJoinPressed();
        _leaveBtn.Pressed += async () => await _lobbyNode.LeaveLobbyAsync();
        _loadoutBtn.Pressed += async () => await _lobbyNode.PushLoadoutChangeAsync("smg");
        _mapBtn.Pressed += async () => await _lobbyNode.ChangeMapAsync("docks");
        _lobbyNode = GetNodeOrNull<Lobby>("/root/Lobby");
        if (_lobbyNode == null) { Append("[color=red]Lobby autoload missing.[/color]"); SetButtonsEnabled(false); return; }
        _lobbyNode.LobbyJoined += OnLobbyJoined;
        _lobbyNode.LobbyLeft += OnLobbyLeft;
        _lobbyNode.LobbyDisconnected += OnLobbyDisconnected;
        _lobbyNode.StateChanged += OnLobbyStateChanged;
        Auth auth = GetNodeOrNull<Auth>("/root/Auth");
        if (auth == null || !await auth.SignInAsync()) { Append("[color=red]Sign-in failed.[/color]"); SetButtonsEnabled(false); return; }
        Append("Signed in. Click Host or Join to bring up multiplayer.");
        SetButtonsEnabled(true);
        _leaveBtn.Disabled = true;
    }

    private void OnLobbyJoined(PlayFabLobby lobby)
    {
        Append($"[color=green]Joined lobby {lobby.LobbyId}[/color]");
        _connEdit.Text = lobby.ConnectionString;
        _connEdit.CaretColumn = lobby.ConnectionString.Length;
        _connEdit.SelectAll();
        _connEdit.GrabFocus();
        RefreshMembers(lobby);
        _leaveBtn.Disabled = false;
        lobby.StateChanged += _ => RefreshMembers(lobby);
    }
    private void OnLobbyLeft() { Append("Left lobby."); _members.Clear(); _leaveBtn.Disabled = true; }
    private void OnLobbyDisconnected() { Append("[color=orange]Disconnected from lobby (kicked or network error).[/color]"); _members.Clear(); _leaveBtn.Disabled = true; }
    private void OnLobbyStateChanged(Lobby.State state)
    {
        if (state == Lobby.State.Hosting) Append("[color=yellow]Hosting lobby…[/color]");
        else if (state == Lobby.State.Joining) Append("[color=yellow]Joining lobby…[/color]");
        else if (state == Lobby.State.Leaving) Append("[color=yellow]Leaving lobby…[/color]");
        bool busy = state == Lobby.State.Hosting || state == Lobby.State.Joining || state == Lobby.State.Leaving;
        bool inLobby = state == Lobby.State.InLobby;
        _hostBtn.Disabled = busy || inLobby;
        _joinBtn.Disabled = busy || inLobby;
        _leaveBtn.Disabled = !inLobby;
        _loadoutBtn.Disabled = !inLobby;
        _mapBtn.Disabled = !inLobby;
    }
    private void RefreshMembers(PlayFabLobby lobby)
    {
        _members.Clear();
        foreach (Variant v in lobby.Members)
        {
            PlayFabLobbyMember m = PlayFabLobbyMember.From(v.AsGodotObject());
            _members.AddItem($"{m.UserId}{(m.IsLocal ? " (local)" : string.Empty)}");
        }
    }
    private async Task OnJoinPressed()
    {
        string conn = _connEdit.Text.StripEdges();
        if (string.IsNullOrEmpty(conn)) { Append("[color=orange]Paste a connection string into the field first.[/color]"); return; }
        await _lobbyNode.JoinLobbyAsync(conn);
    }
    private void SetButtonsEnabled(bool enabled) { _hostBtn.Disabled = !enabled; _joinBtn.Disabled = !enabled; _loadoutBtn.Disabled = !enabled; _mapBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}

