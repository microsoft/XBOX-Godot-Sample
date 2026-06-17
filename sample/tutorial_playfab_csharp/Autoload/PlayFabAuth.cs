using Godot;
using System;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;

/// <summary>
/// PlayFab Tutorial — standalone custom-id sign-in (state-machine autoload).
/// </summary>
public partial class PlayFabAuth : Node
{
    private const string Prefix = "godot-playfab-tutorial-";
    private const string DefaultUser = "player";
    private const string UserArg = "--pf-user";
    private const string UserEnv = "PF_CUSTOM_ID";

    public enum State { Uninitialized, SigningIn, SignedIn, Failed }

    public event Action<State> StateChanged;

    private State _state = State.Uninitialized;
    private PlayFabUser _user;
    private string _customId = string.Empty;
    private string _lastErrorStage = string.Empty;
    private string _lastErrorMessage = string.Empty;
    private Task<bool> _inFlight;

    public State CurrentState => _state;
    public PlayFabUser PlayFabUser => _state == State.SignedIn ? _user : null;
    public string CustomId => GetCustomId();

    public override void _Ready()
    {
        _customId = ResolveCustomId();
        GD.Print($"[PlayFabAuth] custom id: {_customId}");
        _ = SignInAsync();
    }

    public State GetState() => _state;
    public bool IsSignedIn() => _state == State.SignedIn;
    public bool IsSigningIn() => _state == State.SigningIn;
    public bool IsFailed() => _state == State.Failed;
    public string GetLastErrorStage() => _lastErrorStage;
    public string GetLastErrorMessage() => _lastErrorMessage;

    public string GetCustomId()
    {
        if (string.IsNullOrEmpty(_customId)) _customId = ResolveCustomId();
        return _customId;
    }

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
        _user = null;

        SetState(State.SigningIn);
        PlayFabUser user = await EnsurePlayFabUserAsync();
        if (user == null)
        {
            SetState(State.Failed);
            return false;
        }

        _user = user;
        Godot.Collections.Dictionary key = user.EntityKey;
        GD.Print($"[PlayFabAuth] PlayFab session: {TutorialSupport.DictString(key, "type")}:{TutorialSupport.DictString(key, "id")}");
        GD.Print("[PlayFabAuth] Sign-in complete.");
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
        GD.PushWarning($"[PlayFabAuth] sign-in failed at {stage}: {_lastErrorMessage}");
    }

    private async Task<PlayFabUser> EnsurePlayFabUserAsync()
    {
        if (!PlayFab.IsAvailable)
        {
            SetError("playfab.missing", "godot_playfab extension is not loaded");
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

        string customId = GetCustomId();
        PlayFabUser cached = PlayFab.Users.GetUserByCustomId(customId);
        if (cached != null) return cached;

        PlayFabResult result = await PlayFab.Users.SignInWithCustomIdAsync(customId, true);
        if (!result.Ok)
        {
            SetError("playfab.sign_in", result.Message);
            return null;
        }

        return result.DataAs<PlayFabUser>();
    }

    private static string ResolveCustomId()
    {
        string token = ReadUserArg();
        if (string.IsNullOrWhiteSpace(token)) token = OS.GetEnvironment(UserEnv).Trim();
        if (string.IsNullOrWhiteSpace(token)) token = DefaultUser;
        return Prefix + token;
    }

    private static string ReadUserArg()
    {
        var args = new System.Collections.Generic.List<string>();
        args.AddRange(OS.GetCmdlineArgs());
        args.AddRange(OS.GetCmdlineUserArgs());
        for (int i = 0; i < args.Count; i++)
        {
            string arg = args[i];
            if (arg.StartsWith(UserArg + "=", StringComparison.Ordinal))
                return arg[(UserArg.Length + 1)..].Trim();
            if (arg == UserArg && i + 1 < args.Count)
                return args[i + 1].Trim();
        }
        return string.Empty;
    }
}

