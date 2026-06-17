using Godot;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;

public partial class PanelAchievements : VBoxContainer
{
    private const string AchievementId = "1";
    private Label _status;
    private Button _p25, _p50, _p75, _unlock;
    private Auth _auth;
    private bool _initialized;
    public override async void _Ready()
    {
        _status = GetNode<Label>("Status"); _p25 = GetNode<Button>("Progress25"); _p50 = GetNode<Button>("Progress50"); _p75 = GetNode<Button>("Progress75"); _unlock = GetNode<Button>("Unlock");
        _auth = GetNodeOrNull<Auth>("/root/Auth"); if (_auth == null) { _status.Text = "[ERR] Auth autoload missing"; return; }
        _auth.StateChanged += state => { if (!_initialized && _auth.IsSignedIn()) _ = InitializeAfterSignInAsync(); };
        if (_auth.IsSignedIn()) await InitializeAfterSignInAsync(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) await InitializeAfterSignInAsync(); }
    }
    private async Task InitializeAfterSignInAsync()
    {
        if (_initialized) return; _initialized = true;
        Gdk.Achievements.AchievementUnlocked += OnAchievementUnlocked;
        _p25.Pressed += async () => await PushProgressAsync(25); _p50.Pressed += async () => await PushProgressAsync(50); _p75.Pressed += async () => await PushProgressAsync(75); _unlock.Pressed += async () => await PushProgressAsync(100);
        GdkResult result = await Gdk.Achievements.QueryPlayerAchievementsAsync(_auth.XboxUser);
        if (!IsInsideTree()) return;
        if (result.Ok) RefreshStatus(); else { _status.Text = $"Query failed: {result.Message}"; GD.PushWarning($"[Ach] query failed: {result.Message}"); }
    }
    private async Task PushProgressAsync(int percent)
    {
        GdkResult result = await Gdk.Achievements.UpdateAchievementAsync(_auth.XboxUser, AchievementId, percent);
        if (!IsInsideTree()) return;
        if (result.Ok) { _status.Text = $"Pushed {percent}%"; GD.Print($"[Ach] Updated to {percent}%"); } else { _status.Text = $"Update failed: {result.Message}"; GD.PushWarning($"[Ach] {_status.Text}"); }
    }
    private void OnAchievementUnlocked(GdkUser user, string id) { if (id == AchievementId) { _status.Text = $"Unlocked {id} for {user.Gamertag}"; RefreshStatus(); } }
    private void RefreshStatus()
    {
        foreach (Variant entry in Gdk.Achievements.GetCachedAchievements(_auth.XboxUser))
        {
            GodotObject ach = entry.AsGodotObject();
            if (ach.Get("id").AsString() == AchievementId) { int p = ach.Get("progress_percent").AsInt32(); _status.Text = $"{AchievementId}: {p}% — {(p >= 100 ? "Unlocked" : "In progress")}"; return; }
        }
        _status.Text = $"Achievement {AchievementId} not yet in cache";
    }
}


