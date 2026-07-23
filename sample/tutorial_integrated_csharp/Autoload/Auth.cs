using Godot;
using System;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;
using GodotPlayFab;
using GodotPlayFab.Types;

public partial class Auth : Node
{
    public enum State { Uninitialized, SigningInXbox, SigningInPlayFab, SignedIn, Failed }

    public event Action<State> StateChanged;

    private State _state = State.Uninitialized;
    private GdkUser _xboxUser;
    private PlayFabUser _playFabUser;
    private string _lastErrorStage = string.Empty;
    private string _lastErrorMessage = string.Empty;
    private Task<bool> _inFlight;

    public State CurrentState => _state;
    public GdkUser XboxUser => _state == State.SignedIn ? _xboxUser : null;
    public PlayFabUser PlayFabUser => _state == State.SignedIn ? _playFabUser : null;

    public override void _Ready() => _ = SignInAsync();

    public State GetState() => _state;
    public bool IsSignedIn() => _state == State.SignedIn;
    public bool IsSigningIn() => _state == State.SigningInXbox || _state == State.SigningInPlayFab;
    public bool IsFailed() => _state == State.Failed;
    public string GetLastErrorStage() => _lastErrorStage;
    public string GetLastErrorMessage() => _lastErrorMessage;

    public Task<bool> SignInAsync()
    {
        if (_state == State.SignedIn) return Task.FromResult(true);
        if (_inFlight != null && !_inFlight.IsCompleted) return _inFlight;
        _inFlight = DoSignInAsync();
        return _inFlight;
    }

    private async Task<bool> DoSignInAsync()
    {
        _lastErrorStage = string.Empty;
        _lastErrorMessage = string.Empty;
        _xboxUser = null;
        _playFabUser = null;

        SetState(State.SigningInXbox);
        GdkUser xbox = await EnsureXboxUserAsync();
        if (xbox == null)
        {
            SetState(State.Failed);
            return false;
        }
        _xboxUser = xbox;
        GD.Print($"[Auth] Xbox primary user: {xbox.Gamertag}");

        SetState(State.SigningInPlayFab);
        PlayFabUser pf = await EnsurePlayFabUserAsync(xbox);
        if (pf == null)
        {
            SetState(State.Failed);
            return false;
        }
        _playFabUser = pf;
        Godot.Collections.Dictionary key = pf.EntityKey;
        GD.Print($"[Auth] PlayFab session: {TutorialSupport.DictString(key, "type")}:{TutorialSupport.DictString(key, "id")}");
        GD.Print("[Auth] Sign-in complete.");
        SetState(State.SignedIn);
        return true;
    }

    private void SetState(State next)
    {
        if (_state == next) return;
        _state = next;
        StateChanged?.Invoke(_state);
    }

    private void SetError(string stage, string message)
    {
        _lastErrorStage = stage;
        _lastErrorMessage = message ?? string.Empty;
        GD.PushWarning($"[Auth] sign-in failed at {stage}: {_lastErrorMessage}");
    }

    private async Task<GdkUser> EnsureXboxUserAsync()
    {
        if (!Gdk.IsAvailable)
        {
            SetError("gdk.missing", "godot_gdk extension is not loaded");
            return null;
        }

        if (!Gdk.IsInitialized)
        {
            GdkResult init = Gdk.Initialize();
            if (!init.Ok)
            {
                SetError("gdk.initialize", init.Message);
                return null;
            }
        }

        GdkUser primary = Gdk.Users.GetPrimaryUser();
        if (primary != null && primary.IsSignedIn) return primary;

        GdkResult silent = await Gdk.Users.AddDefaultUserAsync();
        GdkUser silentUser = silent?.DataAs<GdkUser>();
        if (silent != null && silent.Ok && silentUser != null && silentUser.IsSignedIn) return silentUser;

        GD.Print($"[Auth] Silent sign-in failed ({silent?.Message}) — falling back to UI.");
        GdkResult ui = await Gdk.Users.AddUserWithUiAsync();
        GdkUser uiUser = ui?.DataAs<GdkUser>();
        if (ui != null && ui.Ok && uiUser != null && uiUser.IsSignedIn) return uiUser;

        SetError("gdk.add_user_with_ui", ui?.Message ?? "Unknown GDK sign-in failure");
        return null;
    }

    private async Task<PlayFabUser> EnsurePlayFabUserAsync(GdkUser xbox)
    {
        if (!PlayFab.IsAvailable)
        {
            SetError("playfab.missing", "godot_playfab extension is not loaded");
            return null;
        }
        if (xbox == null || !xbox.IsSignedIn)
        {
            SetError("playfab.sign_in", "Xbox user is not signed in");
            return null;
        }
        if (!PlayFab.IsInitialized)
        {
            PlayFabResult init = PlayFab.Initialize();
            if (!init.Ok)
            {
                SetError("playfab.initialize", init.Message);
                return null;
            }
        }

        PlayFabResult result = await PlayFab.Users.SignInWithXUserAsync(xbox);
        if (!result.Ok)
        {
            SetError("playfab.sign_in", result.Message);
            return null;
        }
        return result.DataAs<PlayFabUser>();
    }
}

