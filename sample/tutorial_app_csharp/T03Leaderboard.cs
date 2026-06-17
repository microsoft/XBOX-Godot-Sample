using Godot;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;

public partial class T03Leaderboard : Control
{
    private const string StatisticName = "high_score";
    private const string LeaderboardName = "high_score";
    private const int DemoScore = 1234;
    private RichTextLabel _log;
    private Button _submitBtn, _topBtn, _pagesBtn, _aroundBtn, _friendBtn, _backBtn;
    private Auth _auth;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _submitBtn = GetNode<Button>("Root/Buttons/SubmitBtn");
        _topBtn = GetNode<Button>("Root/Buttons/TopBtn");
        _pagesBtn = GetNode<Button>("Root/Buttons/PagesBtn");
        _aroundBtn = GetNode<Button>("Root/Buttons/AroundBtn");
        _friendBtn = GetNode<Button>("Root/Buttons/FriendBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _backBtn.Pressed += OnBackPressed;
        _submitBtn.Pressed += async () => await RecordScoreAsync(DemoScore);
        _topBtn.Pressed += async () => await PrintGlobalTopAsync();
        _pagesBtn.Pressed += async () => await PrintAllPagesAsync();
        _aroundBtn.Pressed += async () => await PrintAroundUserAsync();
        _friendBtn.Pressed += async () => await PrintXboxFriendLeaderboardAsync();
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        if (_auth == null || !PlayFab.IsAvailable) { Append("[color=red]Auth autoload or PlayFab extension missing.[/color]"); SetButtonsEnabled(false); return; }
        SetButtonsEnabled(false);
        Append("Waiting for sign-in…");
        if (await _auth.SignInAsync()) { Append("Signed in."); SetButtonsEnabled(true); }
        else Append($"[color=red]Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}[/color]");
    }

    private async Task RecordScoreAsync(int score)
    {
        PlayFabUser user = _auth.PlayFabUser;
        if (user == null) return;
        var request = new Godot.Collections.Dictionary { ["statistics"] = new Godot.Collections.Array { new Godot.Collections.Dictionary { ["name"] = StatisticName, ["scores"] = new Godot.Collections.Array { score.ToString() } } } };
        PlayFabResult result = await PlayFab.Statistics.UpdateStatisticsAsync(user, request);
        if (!result.Ok) { Append($"[color=orange][Lead] Record failed: {result.Message}[/color]"); return; }
        Append($"[Lead] Recorded score {score} to statistic \"{StatisticName}\"");
    }

    private async Task PrintGlobalTopAsync()
    {
        PlayFabResult result = await PlayFab.Leaderboards.GetLeaderboardAsync(_auth.PlayFabUser, LeaderboardName, 1, 10);
        if (!result.Ok) { Append($"[color=orange][Lead] get_leaderboard failed: {result.Message}[/color]"); return; }
        PrintPage("Global page 1", result.Data.AsGodotDictionary());
    }

    private async Task PrintAllPagesAsync()
    {
        const int pageSize = 10;
        PlayFabResult first = await PlayFab.Leaderboards.GetLeaderboardAsync(_auth.PlayFabUser, LeaderboardName, 1, pageSize);
        if (!first.Ok) { Append($"[color=orange][Lead] first page failed: {first.Message}[/color]"); return; }
        Godot.Collections.Dictionary page = first.Data.AsGodotDictionary();
        int total = TutorialSupport.DictInt(page, "entry_count");
        int version = TutorialSupport.DictInt(page, "version", -1);
        int next = 1;
        int index = 1;
        while (page != null)
        {
            Godot.Collections.Array rankings = TutorialSupport.DictArray(page, "rankings");
            Append($"[Lead] Page {index}: {rankings.Count} row(s)");
            foreach (Variant entry in rankings) PrintRow(entry.AsGodotDictionary());
            next += rankings.Count;
            if (rankings.Count == 0 || next > total) break;
            PlayFabResult nextPage = await PlayFab.Leaderboards.GetLeaderboardAsync(_auth.PlayFabUser, LeaderboardName, next, pageSize, version);
            if (!nextPage.Ok) { Append($"[color=orange][Lead] page {index + 1} failed: {nextPage.Message}[/color]"); return; }
            page = nextPage.Data.AsGodotDictionary();
            index++;
        }
    }

    private async Task PrintAroundUserAsync()
    {
        PlayFabResult result = await PlayFab.Leaderboards.GetLeaderboardAroundUserAsync(_auth.PlayFabUser, LeaderboardName, 3);
        if (!result.Ok) { Append($"[color=orange][Lead] around_user failed: {result.Message}[/color]"); return; }
        Godot.Collections.Dictionary page = result.Data.AsGodotDictionary();
        Godot.Collections.Array rankings = TutorialSupport.DictArray(page, "rankings");
        Append($"[Lead] Around-user: {rankings.Count} row(s) centered on you");
        string myId = TutorialSupport.DictString(_auth.PlayFabUser.EntityKey, "id");
        foreach (Variant entry in rankings)
        {
            Godot.Collections.Dictionary row = entry.AsGodotDictionary();
            string marker = TutorialSupport.DictString(TutorialSupport.DictDict(row, "entity"), "id") == myId ? " (you)" : string.Empty;
            Append($"[Lead]   #{TutorialSupport.DictInt(row, "rank")}  {TutorialSupport.DisplayName(row)} — {TutorialSupport.PrimaryScore(row)}{marker}");
        }
    }

    private async Task PrintXboxFriendLeaderboardAsync()
    {
        PlayFabResult result = await PlayFab.Leaderboards.GetFriendLeaderboardAsync(_auth.PlayFabUser, LeaderboardName, true);
        if (!result.Ok) { Append($"[color=orange][Lead] friend leaderboard failed: {result.Message}[/color]"); return; }
        PrintPage("Xbox-friend leaderboard", result.Data.AsGodotDictionary());
    }

    private void PrintPage(string label, Godot.Collections.Dictionary page)
    {
        Godot.Collections.Array rankings = TutorialSupport.DictArray(page, "rankings");
        Append($"[Lead] {label}: rank 1..{rankings.Count} of ~{TutorialSupport.DictInt(page, "entry_count")} entries (version {TutorialSupport.DictInt(page, "version", -1)})");
        foreach (Variant entry in rankings) PrintRow(entry.AsGodotDictionary());
    }

    private void PrintRow(Godot.Collections.Dictionary row) => Append($"[Lead]   #{TutorialSupport.DictInt(row, "rank")}  {TutorialSupport.DisplayName(row)} — {TutorialSupport.PrimaryScore(row)}");
    private void SetButtonsEnabled(bool enabled) { _submitBtn.Disabled = !enabled; _topBtn.Disabled = !enabled; _pagesBtn.Disabled = !enabled; _aroundBtn.Disabled = !enabled; _friendBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}

