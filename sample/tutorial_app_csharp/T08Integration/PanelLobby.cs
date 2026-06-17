using Godot;
using System.Threading.Tasks;
using GodotPlayFab.Types;

public partial class PanelLobby : VBoxContainer
{
    private Button _host, _join, _leave;
    private Label _status, _members;
    private LineEdit _connectionString;
    private Auth _auth;
    private Lobby _lobbyNode;
    private bool _initialized;
    public override async void _Ready()
    {
        _host = GetNode<Button>("Host"); _join = GetNode<Button>("Join"); _leave = GetNode<Button>("Leave"); _status = GetNode<Label>("Status"); _members = GetNode<Label>("Members"); _connectionString = GetNode<LineEdit>("ConnectionString");
        _auth = GetNodeOrNull<Auth>("/root/Auth"); _lobbyNode = GetNodeOrNull<Lobby>("/root/Lobby"); if (_auth == null || _lobbyNode == null) { _status.Text = "[ERR] Auth/Lobby autoload missing"; return; }
        _auth.StateChanged += _ => { if (!_initialized && _auth.IsSignedIn()) InitializeAfterSignIn(); };
        if (_auth.IsSignedIn()) InitializeAfterSignIn(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) InitializeAfterSignIn(); }
    }
    private void InitializeAfterSignIn()
    {
        if (_initialized) return; _initialized = true; _host.Pressed += async () => await OnHostPressed(); _join.Pressed += async () => await OnJoinPressed(); _leave.Pressed += async () => await OnLeavePressed(); _lobbyNode.StateChanged += OnLobbyStateChanged; _lobbyNode.LobbyDisconnected += OnLobbyDisconnected; Refresh();
    }
    private async Task OnHostPressed() { await _lobbyNode.HostLobbyAsync(); if (!IsInsideTree()) return; if (_lobbyNode.GetCurrentLobby() != null) _connectionString.Text = _lobbyNode.GetCurrentLobby().ConnectionString; Refresh(); }
    private async Task OnJoinPressed() { string text = _connectionString.Text.StripEdges(); if (string.IsNullOrEmpty(text)) { _status.Text = "Paste a connection string into the field first"; return; } await _lobbyNode.JoinLobbyWithStringAsync(text); if (IsInsideTree()) Refresh(); }
    private async Task OnLeavePressed() { await _lobbyNode.LeaveLobbyAsync(); if (IsInsideTree()) Refresh(); }
    private void Refresh()
    {
        PlayFabLobby current = _lobbyNode.GetCurrentLobby(); if (current == null) { _status.Text = "Not in a lobby"; _members.Text = string.Empty; return; }
        _status.Text = $"Lobby {(current.LobbyId.Length > 8 ? current.LobbyId[..8] : current.LobbyId)} ({current.MemberCount} / {current.MaxMemberCount})";
        var lines = new System.Collections.Generic.List<string>(); foreach (Variant mValue in current.Members) { PlayFabLobbyMember m = PlayFabLobbyMember.From(mValue.AsGodotObject()); lines.Add($"- {m.UserId}{(m.IsLocal ? " (you)" : string.Empty)}"); } _members.Text = string.Join("\n", lines);
    }
    private void OnLobbyDisconnected() { _status.Text = "Disconnected from lobby (kicked or network error)"; _members.Text = string.Empty; _connectionString.Text = string.Empty; }
    private void OnLobbyStateChanged(Lobby.State state) { if (state == Lobby.State.Hosting) _status.Text = "Hosting lobby…"; else if (state == Lobby.State.Joining) _status.Text = "Joining lobby…"; else if (state == Lobby.State.Leaving) _status.Text = "Leaving lobby…"; else Refresh(); }
}

