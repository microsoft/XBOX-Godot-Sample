using Godot;
using System.Threading.Tasks;
using GodotPlayFab.Types;

/// <summary>PlayFab Tutorial 1 reference scene — custom-id sign-in status panel.</summary>
public partial class P01Signin : Control
{
    private Label _identity;
    private Label _subtitle;
    private Label _status;
    private Button _signInButton;
    private Button _backButton;
    private PlayFabAuth _auth;

    public override void _Ready()
    {
        _identity = GetNode<Label>("Root/Identity");
        _subtitle = GetNode<Label>("Root/Subtitle");
        _status = GetNode<Label>("Root/Status");
        _signInButton = GetNode<Button>("Root/Buttons/SignIn");
        _backButton = GetNode<Button>("Root/Buttons/Back");
        _backButton.Pressed += OnBackPressed;
        _signInButton.Pressed += async () => await OnSignInPressed();

        _auth = GetNodeOrNull<PlayFabAuth>("/root/PlayFabAuth");
        if (_auth == null)
        {
            _status.Text = "PlayFabAuth autoload missing — register Autoload/PlayFabAuth.cs in project.godot.";
            _signInButton.Disabled = true;
            return;
        }

        _subtitle.Text = $"Signs in as custom id '{_auth.GetCustomId()}'. Override per instance with --pf-user=<name>.";
        _auth.StateChanged += OnAuthStateChanged;
        Refresh();
    }

    public override void _ExitTree()
    {
        if (_auth != null) _auth.StateChanged -= OnAuthStateChanged;
    }

    private void Refresh()
    {
        if (_auth == null) return;
        if (_auth.IsSignedIn())
        {
            RefreshIdentity(_auth.PlayFabUser);
            _status.Text = "Signed in.";
        }
        else if (_auth.IsSigningIn())
        {
            RefreshIdentity(null);
            _status.Text = "Signing in…";
        }
        else if (_auth.IsFailed())
        {
            RefreshIdentity(null);
            _status.Text = $"Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}";
        }
        else
        {
            RefreshIdentity(null);
            _status.Text = "Not signed in.";
        }
    }

    private void RefreshIdentity(PlayFabUser playFabUser)
    {
        if (playFabUser == null)
        {
            _identity.Text = "PlayFab: (not signed in)";
            return;
        }

        Godot.Collections.Dictionary key = playFabUser.EntityKey;
        _identity.Text = $"PlayFab: {TutorialSupport.DictString(key, "type")}:{TutorialSupport.DictString(key, "id")}";
    }

    private async Task OnSignInPressed()
    {
        if (_auth != null) await _auth.SignInAsync();
    }

    private void OnAuthStateChanged(PlayFabAuth.State _state) => Refresh();
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}
