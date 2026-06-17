using Godot;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;
using GodotPlayFab.Types;

public partial class T01Signin : Control
{
    private Label _identity;
    private Label _status;
    private Button _signInButton;
    private Button _backButton;
    private Auth _auth;

    public override void _Ready()
    {
        _identity = GetNode<Label>("Root/Identity");
        _status = GetNode<Label>("Root/Status");
        _signInButton = GetNode<Button>("Root/Buttons/SignIn");
        _backButton = GetNode<Button>("Root/Buttons/Back");
        _backButton.Pressed += OnBackPressed;
        _signInButton.Pressed += async () => await OnSignInPressed();
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        if (_auth == null)
        {
            _status.Text = "Auth autoload missing — register Autoload/Auth.cs in project.godot.";
            _signInButton.Disabled = true;
            return;
        }
        _auth.StateChanged += _ => Refresh();
        Refresh();
    }

    private void Refresh()
    {
        if (_auth == null) return;
        if (_auth.IsSignedIn())
        {
            RefreshIdentity(_auth.XboxUser, _auth.PlayFabUser);
            _status.Text = "Signed in.";
        }
        else if (_auth.IsSigningIn())
        {
            RefreshIdentity(null, null);
            _status.Text = "Signing in…";
        }
        else if (_auth.IsFailed())
        {
            RefreshIdentity(null, null);
            _status.Text = $"Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}";
        }
        else
        {
            RefreshIdentity(null, null);
            _status.Text = "Not signed in.";
        }
    }

    private void RefreshIdentity(GdkUser xboxUser, PlayFabUser playFabUser)
    {
        if (xboxUser == null)
        {
            _identity.Text = "Xbox: (not signed in)";
            return;
        }
        string pfText = string.Empty;
        if (playFabUser != null)
        {
            Godot.Collections.Dictionary key = playFabUser.EntityKey;
            pfText = $"\nPlayFab: {TutorialSupport.DictString(key, "type")}:{TutorialSupport.DictString(key, "id")}";
        }
        _identity.Text = $"Xbox: {xboxUser.Gamertag} ({xboxUser.Xuid}){pfText}";
    }

    private async Task OnSignInPressed()
    {
        if (_auth != null) await _auth.SignInAsync();
    }

    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}

