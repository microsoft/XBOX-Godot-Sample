using Godot;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;

public partial class T02Achievement : Control
{
    private const string FirstScoreId = "1";
    private RichTextLabel _log;
    private Button _listBtn;
    private Button _pushBtn;
    private Button _backBtn;
    private Auth _auth;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _listBtn = GetNode<Button>("Root/Buttons/ListBtn");
        _pushBtn = GetNode<Button>("Root/Buttons/PushBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _backBtn.Pressed += OnBackPressed;
        _listBtn.Pressed += async () => await PrintCachedAchievementsAsync();
        _pushBtn.Pressed += async () => { await PushProgressAsync(50); await PushProgressAsync(100); };
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        if (_auth == null || !Gdk.IsAvailable)
        {
            Append("[color=red]Auth autoload or GDK extension missing.[/color]");
            SetButtonsEnabled(false);
            return;
        }
        Gdk.Achievements.AchievementUnlocked += OnAchievementUnlocked;
        Gdk.Achievements.RuntimeError += r => Append($"[color=orange][Ach] Achievements subsystem error: {r.Message} (0x{r.HResult:X8})[/color]");
        SetButtonsEnabled(false);
        Append("Waiting for sign-in…");
        if (await _auth.SignInAsync()) { Append("Signed in."); SetButtonsEnabled(true); }
        else Append($"[color=red]Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}[/color]");
    }

    private async Task PrintCachedAchievementsAsync()
    {
        GdkUser user = _auth.XboxUser;
        if (user == null) return;
        GdkResult result = await Gdk.Achievements.QueryPlayerAchievementsAsync(user);
        if (!result.Ok) { Append($"[color=orange][Ach] query failed: {result.Message}[/color]"); return; }
        Godot.Collections.Array cache = Gdk.Achievements.GetCachedAchievements(user);
        Append($"[Ach] {cache.Count} achievement(s) declared for this title");
        foreach (Variant entry in cache)
        {
            GodotObject ach = entry.AsGodotObject();
            Append($"[Ach]   {ach.Get("id").AsString()} ({ach.Get("name").AsString()}) — {ach.Get("progress_percent").AsInt32()}%");
        }
    }

    private async Task PushProgressAsync(int percent)
    {
        GdkUser user = _auth.XboxUser;
        if (user == null) return;
        GdkResult result = await Gdk.Achievements.UpdateAchievementAsync(user, FirstScoreId, percent);
        Append(result.Ok ? $"[Ach] Updated to {percent}% — result ok" : $"[color=orange][Ach] Update to {percent}% failed: {result.Message} ({result.Code})[/color]");
    }

    private void OnAchievementUnlocked(GdkUser user, string achievementId)
    {
        Godot.Collections.Array cache = Gdk.Achievements.GetCachedAchievements(user);
        foreach (Variant entry in cache)
        {
            GodotObject ach = entry.AsGodotObject();
            if (ach.Get("id").AsString() == achievementId) { Append($"[color=green][Ach] Unlocked: {ach.Get("name").AsString()}[/color]"); return; }
        }
        Append($"[color=green][Ach] Unlocked id={achievementId} (not in cache yet)[/color]");
    }

    private void SetButtonsEnabled(bool enabled) { _listBtn.Disabled = !enabled; _pushBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}

