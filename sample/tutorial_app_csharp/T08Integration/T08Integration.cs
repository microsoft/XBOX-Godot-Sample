using Godot;
using GodotGdk;
using GodotPlayFab;

public partial class T08Integration : Control
{
    private Label _identity;
    private Label _error;
    private Button _retry;
    private Button _back;
    private Auth _auth;

    public override async void _Ready()
    {
        _identity = GetNode<Label>("Root/Hud/IdentityLabel");
        _error = GetNode<Label>("Root/Hud/ErrorLabel");
        _retry = GetNode<Button>("Root/Hud/SignInRetry");
        _back = GetNode<Button>("Root/Hud/Back");
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        _identity.Text = "Signing in…";
        _error.Text = string.Empty;
        _retry.Pressed += async () => await OnRetryPressed();
        _back.Pressed += () => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
        if (_auth == null) { _identity.Text = "[ERR] Auth autoload missing"; return; }
        _auth.StateChanged += OnAuthStateChanged;
        OnAuthStateChanged(_auth.GetState());
        if (Gdk.IsAvailable)
        {
            Gdk.RuntimeError += r => OnRuntimeError(r.Message, "gdk");
            Gdk.Achievements.RuntimeError += r => OnRuntimeError(r.Message, "achievements");
        }
        if (PlayFab.IsAvailable)
        {
            PlayFab.Multiplayer.MultiplayerError += r => OnRuntimeError(r.Message, "multiplayer");
            PlayFab.Party.PartyError += r => OnRuntimeError(r.Message, "party");
        }
        await _auth.SignInAsync();
    }

    private void OnAuthStateChanged(Auth.State state)
    {
        if (_auth == null) return;
        if (_auth.IsSignedIn())
        {
            string entityId = TutorialSupport.DictString(_auth.PlayFabUser.EntityKey, "id");
            _identity.Text = $"{_auth.XboxUser.Gamertag} ↔ PlayFab:{(entityId.Length > 8 ? entityId[..8] : entityId)}";
            _error.Text = string.Empty;
        }
        else if (_auth.IsSigningIn()) _identity.Text = "Signing in…";
        else if (_auth.IsFailed()) { _identity.Text = "(not signed in)"; _error.Text = $"Sign-in failed ({_auth.GetLastErrorStage()}): {_auth.GetLastErrorMessage()}"; GD.PushWarning($"[Hud] {_error.Text}"); }
        else _identity.Text = "(not signed in)";
    }
    private async System.Threading.Tasks.Task OnRetryPressed() { _error.Text = string.Empty; if (_auth != null) await _auth.SignInAsync(); }
    private void OnRuntimeError(string message, string source) { _error.Text = $"[{source}] {message}"; GD.PushWarning($"[Hud] runtime error from {source}: {message}"); }
}

