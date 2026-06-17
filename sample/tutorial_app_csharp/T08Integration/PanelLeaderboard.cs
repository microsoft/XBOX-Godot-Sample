using Godot;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;

public partial class PanelLeaderboard : VBoxContainer
{
    private const string StatisticName = "high_score";
    private const string LeaderboardName = "high_score";
    private Label _top10, _around, _status;
    private Button _submit, _refresh;
    private Auth _auth;
    private int _scratchScore = 100;
    private bool _initialized;
    public override async void _Ready()
    {
        _top10 = GetNode<Label>("Top10"); _around = GetNode<Label>("AroundUser"); _submit = GetNode<Button>("SubmitScore"); _refresh = GetNode<Button>("Refresh"); _status = GetNode<Label>("Status");
        _auth = GetNodeOrNull<Auth>("/root/Auth"); if (_auth == null) { _status.Text = "[ERR] Auth autoload missing"; return; }
        _auth.StateChanged += state => { if (!_initialized && _auth.IsSignedIn()) _ = InitializeAfterSignInAsync(); };
        if (_auth.IsSignedIn()) await InitializeAfterSignInAsync(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) await InitializeAfterSignInAsync(); }
    }
    private async Task InitializeAfterSignInAsync()
    {
        if (_initialized) return; _initialized = true; _submit.Pressed += async () => await OnSubmitPressed(); _refresh.Pressed += async () => await RefreshViewsAsync(); await RefreshViewsAsync();
    }
    private async Task OnSubmitPressed()
    {
        _scratchScore += 10;
        var request = new Godot.Collections.Dictionary { ["statistics"] = new Godot.Collections.Array { new Godot.Collections.Dictionary { ["name"] = StatisticName, ["scores"] = new Godot.Collections.Array { _scratchScore.ToString() } } } };
        PlayFabResult result = await PlayFab.Statistics.UpdateStatisticsAsync(_auth.PlayFabUser, request);
        if (!IsInsideTree()) return;
        if (result.Ok) { _status.Text = $"Recorded {_scratchScore} to {StatisticName}"; GD.Print($"[Lb] Recorded {_scratchScore} to {StatisticName}"); } else { _status.Text = $"Record failed: {result.Message}"; return; }
        await RefreshViewsAsync();
    }
    private async Task RefreshViewsAsync()
    {
        PlayFabUser user = _auth.PlayFabUser;
        PlayFabResult top = await PlayFab.Leaderboards.GetLeaderboardAsync(user, LeaderboardName, 1, 10);
        if (!IsInsideTree()) return;
        _top10.Text = top.Ok ? Render(TutorialSupport.DictArray(top.Data.AsGodotDictionary(), "rankings")) : $"Top-10 failed: {top.Message}";
        PlayFabResult around = await PlayFab.Leaderboards.GetLeaderboardAroundUserAsync(user, LeaderboardName, 3);
        if (!IsInsideTree()) return;
        _around.Text = around.Ok ? Render(TutorialSupport.DictArray(around.Data.AsGodotDictionary(), "rankings")) : $"Around-user failed: {around.Message}";
    }
    private string Render(Godot.Collections.Array rankings)
    {
        var lines = new System.Collections.Generic.List<string>();
        foreach (Variant entry in rankings) { Godot.Collections.Dictionary row = entry.AsGodotDictionary(); lines.Add($"{TutorialSupport.DictInt(row, "rank")}. {TutorialSupport.DisplayName(row)} — {TutorialSupport.PrimaryScore(row)}"); }
        return lines.Count == 0 ? "(no entries)" : string.Join("\n", lines);
    }
}


