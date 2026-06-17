using Godot;
using System.Threading.Tasks;

public partial class TutorialPicker : Control
{
    private readonly (string Label, string Scene, bool NeedsAuth)[] _tutorials =
    {
        ("T1 — Sign in", "res://t01_signin.tscn", false),
        ("T2 — Unlock an achievement", "res://t02_achievement.tscn", true),
        ("T3 — PlayFab leaderboard", "res://t03_leaderboard.tscn", true),
        ("T4 — Game Saves", "res://t04_game_saves.tscn", true),
        ("T5 — Multiplayer lobby", "res://t05_lobby.tscn", true),
        ("T6 — Multiplayer Activity", "res://t06_mpa.tscn", true),
        ("T7 — PlayFab Party", "res://t07_party.tscn", true),
        ("T8 — Integration tech demo", "res://T08Integration/t08_integration.tscn", true),
    };
    private Label _status;
    private RichTextLabel _problems;
    private VBoxContainer _buttons;
    private Auth _auth;

    public override async void _Ready()
    {
        _status = GetNode<Label>("Root/Status");
        _problems = GetNode<RichTextLabel>("Root/Problems");
        _buttons = GetNode<VBoxContainer>("Root/Buttons");
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        PopulateButtons();
        _status.Text = "Signing in…";
        SetSigninGated(true);
        Godot.Collections.Array problems = DetectConfigProblems();
        if (problems.Count > 0)
        {
            ShowConfigProblems(problems);
            if (HasProblemOfKind(problems, "game_config")) SetAllGated(true);
            return;
        }
        if (_auth == null) { _status.Text = "Auth autoload missing — register Autoload/Auth.cs in project.godot."; SetSigninGated(true); return; }
        SetAllGated(true);
        await ReleaseOrphanedSessionsAsync();
        SetAllGated(false);
        SetSigninGated(true);
        _auth.StateChanged += OnAuthStateChanged;
        OnAuthStateChanged(_auth.GetState());
    }

    private void PopulateButtons()
    {
        foreach (var entry in _tutorials)
        {
            var button = new Button { Text = entry.Label };
            string scene = entry.Scene;
            button.Pressed += () => GetTree().ChangeSceneToFile(scene);
            _buttons.AddChild(button);
        }
    }
    private void SetSigninGated(bool gated)
    {
        for (int i = 0; i < _tutorials.Length; i++) if (_tutorials[i].NeedsAuth) ((Button)_buttons.GetChild(i)).Disabled = gated;
    }
    private void SetAllGated(bool gated)
    {
        for (int i = 0; i < _tutorials.Length; i++) ((Button)_buttons.GetChild(i)).Disabled = gated;
    }
    private Godot.Collections.Array DetectConfigProblems()
    {
        var problems = new Godot.Collections.Array();
        string pfTitle = ProjectSettings.GetSetting("playfab/runtime/title_id", string.Empty).AsString().StripEdges();
        if (string.IsNullOrEmpty(pfTitle)) problems.Add(new Godot.Collections.Dictionary { ["kind"] = "pf_title", ["title"] = "PlayFab title id is not set.", ["fix"] = "Open Project Settings → PlayFab → Runtime → Title Id, paste your PlayFab title id, then relaunch." });
        const string configPath = "res://MicrosoftGame.config";
        if (!Godot.FileAccess.FileExists(configPath)) problems.Add(new Godot.Collections.Dictionary { ["kind"] = "game_config", ["title"] = "MicrosoftGame.config is missing.", ["fix"] = "Copy MicrosoftGame.config.template to MicrosoftGame.config and fill in Partner Center values." });
        else
        {
            string stale = GameConfigPlaceholderSummary(configPath);
            if (!string.IsNullOrEmpty(stale)) problems.Add(new Godot.Collections.Dictionary { ["kind"] = "game_config", ["title"] = $"MicrosoftGame.config still has placeholder values ({stale}).", ["fix"] = "Edit MicrosoftGame.config and replace placeholder values with Partner Center values." });
        }
        return problems;
    }
    private string GameConfigPlaceholderSummary(string configPath)
    {
        using Godot.FileAccess file = Godot.FileAccess.Open(configPath, Godot.FileAccess.ModeFlags.Read);
        if (file == null) return string.Empty;
        string text = file.GetAsText();
        var stale = new System.Collections.Generic.List<string>();
        if (text.Contains("FFFFFFFF")) stale.Add("TitleId");
        if (text.Contains("9NXXXXXXXXXX")) stale.Add("StoreId");
        if (text.Contains("00000000-0000-0000-0000-000000000000")) stale.Add("MSAAppId");
        if (text.Contains("XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX")) stale.Add("Publisher");
        return string.Join(", ", stale);
    }
    private bool HasProblemOfKind(Godot.Collections.Array problems, string kind)
    {
        foreach (Variant problemValue in problems) if (TutorialSupport.DictString(problemValue.AsGodotDictionary(), "kind") == kind) return true;
        return false;
    }
    private void ShowConfigProblems(Godot.Collections.Array problems)
    {
        _status.Text = "Configuration problems detected — fix the items below and relaunch.";
        var lines = new System.Collections.Generic.List<string>();
        foreach (Variant problemValue in problems)
        {
            Godot.Collections.Dictionary p = problemValue.AsGodotDictionary();
            lines.Add($"[color=#ff8080][b]• {TutorialSupport.DictString(p, "title")}[/b][/color]");
            lines.Add($"    {TutorialSupport.DictString(p, "fix")}");
            lines.Add(string.Empty);
        }
        _problems.Text = string.Join("\n", lines);
        _problems.Visible = true;
    }
    private void OnAuthStateChanged(Auth.State state)
    {
        if (_auth.IsSignedIn()) { _status.Text = $"Signed in as {_auth.XboxUser?.Gamertag ?? "(unknown)"}"; SetSigninGated(false); }
        else if (_auth.IsSigningIn()) { _status.Text = "Signing in…"; SetSigninGated(true); }
        else if (_auth.IsFailed()) { _status.Text = $"Sign-in failed ({_auth.GetLastErrorStage()}): {_auth.GetLastErrorMessage()} — T1 is still available."; SetSigninGated(true); }
        else { _status.Text = "Not signed in."; SetSigninGated(true); }
    }
    private async Task ReleaseOrphanedSessionsAsync()
    {
        Lobby lobby = GetNodeOrNull<Lobby>("/root/Lobby");
        if (lobby != null) await DrainLobbyAsync(lobby);
        Party party = GetNodeOrNull<Party>("/root/Party");
        if (party != null) await DrainPartyAsync(party);
    }
    private async Task DrainLobbyAsync(Lobby lobby)
    {
        if (lobby.IsBusy()) { _status.Text = "Waiting for previous lobby op to finish…"; ulong deadline = Time.GetTicksMsec() + 5000; while (lobby.IsBusy() && Time.GetTicksMsec() < deadline) await ToSignal(GetTree().CreateTimer(0.1), SceneTreeTimer.SignalName.Timeout); if (lobby.IsBusy()) { GD.PushWarning("[Picker] Previous lobby op did not settle within 5s; skipping cleanup"); return; } }
        if (lobby.IsInLobby()) { _status.Text = "Cleaning up previous lobby…"; await lobby.LeaveLobbyAsync(); }
    }
    private async Task DrainPartyAsync(Party party)
    {
        if (party.IsBusy()) { _status.Text = "Waiting for previous Party network op to finish…"; ulong deadline = Time.GetTicksMsec() + 5000; while (party.IsBusy() && Time.GetTicksMsec() < deadline) await ToSignal(GetTree().CreateTimer(0.1), SceneTreeTimer.SignalName.Timeout); if (party.IsBusy()) { GD.PushWarning("[Picker] Previous Party network op did not settle within 5s; skipping cleanup"); return; } }
        if (party.IsInNetwork()) { _status.Text = "Cleaning up previous Party network…"; await party.LeavePartyAsync(); }
    }
}

