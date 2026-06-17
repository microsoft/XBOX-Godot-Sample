using Godot;
using System.Threading.Tasks;

/// <summary>Tutorial picker — default scene for the PlayFab-only C# tutorial track.</summary>
public partial class TutorialPicker : Control
{
    private readonly (string Label, string Scene, bool NeedsAuth)[] _tutorials =
    {
        ("P1 — Sign in (PlayFab custom id)", "res://p01_signin.tscn", false),
        ("P2 — PlayFab leaderboard", "res://p02_leaderboard.tscn", true),
        ("P3 — Multiplayer lobby", "res://p03_lobby.tscn", true),
        ("P4 — PlayFab Party", "res://p04_party.tscn", true),
    };

    private Label _status;
    private RichTextLabel _problems;
    private VBoxContainer _buttons;
    private PlayFabAuth _auth;

    public override async void _Ready()
    {
        _status = GetNode<Label>("Root/Status");
        _problems = GetNode<RichTextLabel>("Root/Problems");
        _buttons = GetNode<VBoxContainer>("Root/Buttons");
        _auth = GetNodeOrNull<PlayFabAuth>("/root/PlayFabAuth");
        PopulateButtons();
        _status.Text = "Signing in…";
        SetSigninGated(true);

        Godot.Collections.Array problems = DetectConfigProblems();
        if (problems.Count > 0)
        {
            ShowConfigProblems(problems);
            SetAllGated(true);
            return;
        }

        if (_auth == null)
        {
            _status.Text = "PlayFabAuth autoload missing — register Autoload/PlayFabAuth.cs in project.godot.";
            SetSigninGated(true);
            return;
        }

        SetAllGated(true);
        await ReleaseOrphanedSessionsAsync();
        if (!IsInsideTree()) return;
        SetAllGated(false);
        SetSigninGated(true);

        _auth.StateChanged += OnAuthStateChanged;
        OnAuthStateChanged(_auth.GetState());
    }

    public override void _ExitTree()
    {
        if (_auth != null) _auth.StateChanged -= OnAuthStateChanged;
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
        for (int i = 0; i < _tutorials.Length; i++)
            if (_tutorials[i].NeedsAuth) ((Button)_buttons.GetChild(i)).Disabled = gated;
    }

    private void SetAllGated(bool gated)
    {
        for (int i = 0; i < _tutorials.Length; i++) ((Button)_buttons.GetChild(i)).Disabled = gated;
    }

    private Godot.Collections.Array DetectConfigProblems()
    {
        var problems = new Godot.Collections.Array();
        string pfTitle = ProjectSettings.GetSetting("playfab/runtime/title_id", string.Empty).AsString().StripEdges();
        if (string.IsNullOrEmpty(pfTitle))
        {
            problems.Add(new Godot.Collections.Dictionary
            {
                ["kind"] = "pf_title",
                ["title"] = "PlayFab title id is not set.",
                ["fix"] = "Open Project Settings → PlayFab → Runtime → Title Id, paste your PlayFab title id (3–5 hex chars from PlayFab Game Manager), then relaunch this project. The repo script tools\\setup_sample.ps1 can write this for you.",
            });
        }
        return problems;
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

    private void OnAuthStateChanged(PlayFabAuth.State state)
    {
        if (_auth.IsSignedIn())
        {
            Godot.Collections.Dictionary key = _auth.PlayFabUser?.EntityKey;
            string id = key == null ? "(unknown)" : $"{TutorialSupport.DictString(key, "type")}:{TutorialSupport.DictString(key, "id")}";
            _status.Text = $"Signed in as '{_auth.GetCustomId()}' ({id})";
            SetSigninGated(false);
        }
        else if (_auth.IsSigningIn())
        {
            _status.Text = "Signing in…";
            SetSigninGated(true);
        }
        else if (_auth.IsFailed())
        {
            _status.Text = $"Sign-in failed ({_auth.GetLastErrorStage()}): {_auth.GetLastErrorMessage()} — P1 is still available.";
            SetSigninGated(true);
        }
        else
        {
            _status.Text = "Not signed in.";
            SetSigninGated(true);
        }
    }

    private async Task ReleaseOrphanedSessionsAsync()
    {
        Lobby lobby = GetNodeOrNull<Lobby>("/root/Lobby");
        if (lobby != null)
        {
            await DrainLobbyAsync(lobby);
            if (!IsInsideTree()) return;
        }
        Party party = GetNodeOrNull<Party>("/root/Party");
        if (party != null)
        {
            await DrainPartyAsync(party);
            if (!IsInsideTree()) return;
        }
    }

    private async Task DrainLobbyAsync(Lobby lobby)
    {
        if (lobby.IsBusy())
        {
            _status.Text = "Waiting for previous lobby op to finish…";
            ulong deadline = Time.GetTicksMsec() + 5000;
            while (lobby.IsBusy() && Time.GetTicksMsec() < deadline)
            {
                await ToSignal(GetTree().CreateTimer(0.1), SceneTreeTimer.SignalName.Timeout);
                if (!IsInsideTree()) return;
            }
            if (lobby.IsBusy())
            {
                GD.PushWarning("[Picker] Previous lobby op did not settle within 5s; skipping cleanup");
                return;
            }
        }
        if (lobby.IsInLobby())
        {
            _status.Text = "Cleaning up previous lobby…";
            await lobby.LeaveLobbyAsync();
            if (!IsInsideTree()) return;
        }
    }

    private async Task DrainPartyAsync(Party party)
    {
        if (party.IsBusy())
        {
            _status.Text = "Waiting for previous Party network op to finish…";
            ulong deadline = Time.GetTicksMsec() + 5000;
            while (party.IsBusy() && Time.GetTicksMsec() < deadline)
            {
                await ToSignal(GetTree().CreateTimer(0.1), SceneTreeTimer.SignalName.Timeout);
                if (!IsInsideTree()) return;
            }
            if (party.IsBusy())
            {
                GD.PushWarning("[Picker] Previous Party network op did not settle within 5s; skipping cleanup");
                return;
            }
        }
        if (party.IsInNetwork())
        {
            _status.Text = "Cleaning up previous Party network…";
            await party.LeavePartyAsync();
            if (!IsInsideTree()) return;
        }
    }
}
