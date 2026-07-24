using Godot;
using System.Threading.Tasks;
using GodotGdk.Types;

/// <summary>
/// GDK Tutorial 1 reference scene — Xbox sign-in status panel.
///
/// Reads the <c>GdkAuth</c> autoload and renders the current sign-in state via
/// <c>GdkAuth.StateChanged</c>. Pressing <b>Sign in</b> re-runs the
/// check → silent → UI fallback; <b>Back</b> returns to the picker. GDK-only:
/// there is no PlayFab identity to show.
/// </summary>
public partial class G01Signin : Control
{
    private Label _identity;
    private Label _status;
    private Button _signInButton;
    private Button _backButton;
    private GdkAuth _auth;

    public override void _Ready()
    {
        _identity = GetNode<Label>("Root/Identity");
        _status = GetNode<Label>("Root/Status");
        _signInButton = GetNode<Button>("Root/Buttons/SignIn");
        _backButton = GetNode<Button>("Root/Buttons/Back");
        _backButton.Pressed += OnBackPressed;
        _signInButton.Pressed += async () => await OnSignInPressed();
        _auth = GetNodeOrNull<GdkAuth>("/root/GdkAuth");
        if (_auth == null)
        {
            _status.Text = "GdkAuth autoload missing — register Autoload/GdkAuth.cs in project.godot.";
            _signInButton.Disabled = true;
            return;
        }
        _auth.StateChanged += OnAuthStateChanged;
        Refresh();
    }

    public override void _ExitTree()
    {
        if (_auth != null) _auth.StateChanged -= OnAuthStateChanged;
    }

    private void OnAuthStateChanged(GdkAuth.State state) => Refresh();

    private void Refresh()
    {
        if (_auth == null) return;
        if (_auth.IsSignedIn())
        {
            RefreshIdentity(_auth.XboxUser);
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

    private void RefreshIdentity(GdkUser xboxUser)
    {
        _identity.Text = xboxUser == null
            ? "Xbox: (not signed in)"
            : $"Xbox: {xboxUser.Gamertag} ({xboxUser.Xuid})";
    }

    private async Task OnSignInPressed()
    {
        // SignInAsync() is idempotent — if already signed in returns immediately;
        // if signing in joins the in-flight attempt; otherwise starts fresh.
        if (_auth != null) await _auth.SignInAsync();
    }

    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}
