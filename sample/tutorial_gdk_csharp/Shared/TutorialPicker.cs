using Godot;

/// <summary>
/// Tutorial picker — default scene for the GDK-only tutorial track.
///
/// Plain Control with one button per tutorial. Pressing a button changes the
/// current scene to that tutorial's reference end-state scene. Disables
/// auth-gated buttons while sign-in is in-flight so first-time opens don't
/// race the <c>GdkAuth</c> autoload's silent sign-in.
///
/// This is the GDK-only track: it signs into Xbox via <c>GdkAuth</c> and never
/// touches PlayFab, so the config pre-flight only checks MicrosoftGame.config.
/// </summary>
public partial class TutorialPicker : Control
{
    private readonly (string Label, string Scene, bool NeedsAuth)[] _tutorials =
    {
        ("G1 — Sign in (Xbox)", "res://g01_signin.tscn", false),
        ("G2 — Unlock an achievement", "res://g02_achievement.tscn", true),
        ("G3 — Title storage & stats", "res://g03_storage_stats.tscn", true),
        ("G4 — Multiplayer Activity", "res://g04_mpa.tscn", true),
        ("G5 — Text-to-Speech", "res://g05_speech.tscn", false),
    };
    private Label _status;
    private RichTextLabel _problems;
    private VBoxContainer _buttons;
    private GdkAuth _auth;

    public override void _Ready()
    {
        _status = GetNode<Label>("Root/Status");
        _problems = GetNode<RichTextLabel>("Root/Problems");
        _buttons = GetNode<VBoxContainer>("Root/Buttons");
        _auth = GetNodeOrNull<GdkAuth>("/root/GdkAuth");
        PopulateButtons();
        _status.Text = "Signing in…";
        SetSigninGated(true);

        // Pre-flight the per-developer config so the picker shows actionable
        // errors instead of a vague "Sign-in failed (gdk.initialize)" line.
        Godot.Collections.Array problems = DetectConfigProblems();
        if (problems.Count > 0)
        {
            ShowConfigProblems(problems);
            // GDK initialize fails without a real MicrosoftGame.config — even
            // G1 can't run, so lock everything down.
            SetAllGated(true);
            return;
        }

        if (_auth == null)
        {
            _status.Text = "GdkAuth autoload missing — register Autoload/GdkAuth.cs in project.godot.";
            SetSigninGated(true);
            return;
        }

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
        const string configPath = "res://MicrosoftGame.config";
        if (!Godot.FileAccess.FileExists(configPath))
        {
            problems.Add(new Godot.Collections.Dictionary
            {
                ["title"] = "MicrosoftGame.config is missing.",
                ["fix"] = "Copy MicrosoftGame.config.template to MicrosoftGame.config and fill in Partner Center values, OR run pwsh -File tools\\setup_sample.ps1 from the repo root.",
            });
        }
        else
        {
            string stale = GameConfigPlaceholderSummary(configPath);
            if (!string.IsNullOrEmpty(stale))
            {
                problems.Add(new Godot.Collections.Dictionary
                {
                    ["title"] = $"MicrosoftGame.config still has placeholder values ({stale}).",
                    ["fix"] = "Edit MicrosoftGame.config and replace the placeholder TitleId / StoreId / MSAAppId / Publisher values with Partner Center values.",
                });
            }
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

    private void OnAuthStateChanged(GdkAuth.State state)
    {
        if (_auth.IsSignedIn())
        {
            _status.Text = $"Signed in as {_auth.XboxUser?.Gamertag ?? "(unknown)"}";
            SetSigninGated(false);
        }
        else if (_auth.IsSigningIn())
        {
            _status.Text = "Signing in…";
            SetSigninGated(true);
        }
        else if (_auth.IsFailed())
        {
            _status.Text = $"Sign-in failed ({_auth.GetLastErrorStage()}): {_auth.GetLastErrorMessage()} — G1 is still available.";
            SetSigninGated(true);
        }
        else
        {
            _status.Text = "Not signed in.";
            SetSigninGated(true);
        }
    }
}
