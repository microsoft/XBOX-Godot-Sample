using Godot;
using System;
using System.Threading.Tasks;
using GodotXbox;
using GodotXbox.Types;

/// <summary>
/// GDK Tutorial — Xbox sign-in (state-machine autoload).
///
/// The <c>XboxAuth</c> autoload is the GDK-only track's identity service.
/// Unlike the integrated track's <c>Auth</c> autoload, it stops at Xbox:
/// there is no PlayFab step. Consumers gate work by awaiting
/// <see cref="SignInAsync"/>, which is idempotent and joins an in-flight
/// attempt instead of starting a new one. The single
/// <see cref="StateChanged"/> event carries the new state; accessors return
/// the current truth.
/// </summary>
public partial class XboxAuth : Node
{
    public enum State { Uninitialized, SigningInXbox, SignedIn, Failed }

    public event Action<State> StateChanged;

    private State _state = State.Uninitialized;
    private XboxUser _xboxUser;
    private string _lastErrorStage = string.Empty;
    private string _lastErrorMessage = string.Empty;
    private Task<bool> _inFlight;

    public State CurrentState => _state;

    // Read-only addon-object accessor. XboxUser is intentionally null unless
    // the chain reached SignedIn so consumers cannot accidentally use a
    // half-completed session.
    public XboxUser XboxUser => _state == State.SignedIn ? _xboxUser : null;

    // Kick off silent sign-in immediately so the first scene to load can
    // simply `await XboxAuth.SignInAsync()` and join the in-flight attempt.
    public override void _Ready() => _ = SignInAsync();

    public State GetState() => _state;
    public bool IsSignedIn() => _state == State.SignedIn;
    public bool IsSigningIn() => _state == State.SigningInXbox;
    public bool IsFailed() => _state == State.Failed;
    public string GetLastErrorStage() => _lastErrorStage;
    public string GetLastErrorMessage() => _lastErrorMessage;

    /// <summary>
    /// Idempotent. Joins an in-flight attempt if one is already running.
    /// Returns <c>true</c> when the local user is signed in, <c>false</c> on
    /// failure (read <see cref="GetLastErrorStage"/> /
    /// <see cref="GetLastErrorMessage"/> for diagnosis).
    /// </summary>
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

        SetState(State.SigningInXbox);
        XboxUser xbox = await EnsureXboxUserAsync();
        if (xbox == null)
        {
            SetState(State.Failed);
            return false;
        }
        _xboxUser = xbox;
        GD.Print($"[XboxAuth] Xbox primary user: {xbox.Gamertag}");
        GD.Print("[XboxAuth] Sign-in complete.");
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
        GD.PushWarning($"[XboxAuth] sign-in failed at {stage}: {_lastErrorMessage}");
    }

    private async Task<XboxUser> EnsureXboxUserAsync()
    {
        if (!Xbox.IsAvailable)
        {
            SetError("gdk.missing", "godot_gdk extension is not loaded");
            return null;
        }

        if (!Xbox.IsInitialized)
        {
            XboxResult init = Xbox.Initialize();
            if (!init.Ok)
            {
                SetError("gdk.initialize", init.Message);
                return null;
            }
        }

        // 1. Already have a primary user (auto-init, prior sign-in)? Use it.
        XboxUser primary = Xbox.Users.GetPrimaryUser();
        if (primary != null && primary.IsSignedIn) return primary;

        // 2. Try the silent path (picks up the Xbox-app account without UI).
        XboxResult silent = await Xbox.Users.AddDefaultUserAsync();
        XboxUser silentUser = silent?.DataAs<XboxUser>();
        if (silent != null && silent.Ok && silentUser != null && silentUser.IsSignedIn) return silentUser;

        GD.Print($"[XboxAuth] Silent sign-in failed ({silent?.Message}) — falling back to UI.");

        // 3. UI fallback. Shows the system account picker.
        XboxResult ui = await Xbox.Users.AddUserWithUiAsync();
        XboxUser uiUser = ui?.DataAs<XboxUser>();
        if (ui != null && ui.Ok && uiUser != null && uiUser.IsSignedIn) return uiUser;

        SetError("gdk.add_user_with_ui", ui?.Message ?? "Unknown GDK sign-in failure");
        return null;
    }
}
