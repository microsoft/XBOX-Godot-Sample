using Godot;
using System.Threading.Tasks;
using GodotXbox;
using GodotXbox.Types;

/// <summary>
/// GDK Tutorial 3 reference scene — Title Storage + user statistics.
///
/// GDK-only surfaces, no PlayFab. Buttons drive each step:
///   - List Title Storage blob metadata, then download an existing blob
///     (GDK.title_storage list/download). Untrusted platforms (PC) can't
///     upload to Title Storage, so writes are omitted here.
///   - Stage and flush a couple of title-managed statistics, then query
///     them back (GDK.stats set/flush/query).
/// </summary>
public partial class G03StorageStats : Control
{
    // Title Storage uses "TrustedPlatform" for binary blobs scoped to the
    // signed-in user. Other valid storage types include "GlobalStorage" and
    // "Universal"; see the XboxTitleStorage reference for the full set.
    private const string StorageType = "TrustedPlatform";
    private const string BlobPath = "tutorial/save.bin";

    // Title-managed statistics declared for the title in Partner Center.
    // Substitute with statistics you registered for your own title.
    private const string StatHighScore = "HighScore";
    private const string StatLevelsCleared = "LevelsCleared";

    private RichTextLabel _log;
    private Button _storageBtn;
    private Button _statsBtn;
    private Button _backBtn;
    private XboxAuth _auth;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _storageBtn = GetNode<Button>("Root/Buttons/StorageBtn");
        _statsBtn = GetNode<Button>("Root/Buttons/StatsBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _backBtn.Pressed += OnBackPressed;
        _storageBtn.Pressed += async () => await OnStoragePressed();
        _statsBtn.Pressed += async () => await OnStatsPressed();

        _auth = GetNodeOrNull<XboxAuth>("/root/XboxAuth");
        if (_auth == null || !Xbox.IsAvailable)
        {
            Append("[color=red]XboxAuth autoload or GDK extension missing.[/color]");
            SetButtonsEnabled(false);
            return;
        }

        // Surface real-time stat changes for the tracked statistics.
        Xbox.Stats.StatChanged += OnStatChanged;

        SetButtonsEnabled(false);
        Append("Waiting for sign-in…");
        bool signedIn = await _auth.SignInAsync();
        if (!IsInsideTree()) return;
        if (signedIn) { Append("Signed in."); SetButtonsEnabled(true); }
        else Append($"[color=red]Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}[/color]");
    }

    public override void _ExitTree()
    {
        if (Xbox.IsAvailable) Xbox.Stats.StatChanged -= OnStatChanged;
    }

    // --- Title Storage (Step 1) ---

    private async Task OnStoragePressed()
    {
        XboxUser user = _auth.XboxUser;
        if (user == null) return;

        // Untrusted platforms (PC) can't write Title Storage — uploads are
        // rejected with HTTP 403. Provision the blob from a console or Partner
        // Center, then read it back with the list + download surfaces below.

        // 1. List blob metadata so the developer can see what's stored.
        // Pass maxItems=25 to mirror the GDScript sample's binding default
        // (the C# facade has no default; the native list forwards it as-is).
        XboxResult list = await Xbox.TitleStorage.ListBlobMetadataAsync(user, StorageType, string.Empty, 0, 25);
        if (!IsInsideTree()) return;
        if (list.Ok) Append($"[Storage] Listed blob metadata for {StorageType}.");
        else Append($"[color=orange][Storage] list failed: {list.Message} ({list.Code})[/color]");

        // 2. Download an existing blob.
        XboxResult down = await Xbox.TitleStorage.DownloadBlobAsync(user, StorageType, BlobPath);
        if (!IsInsideTree()) return;
        if (!down.Ok) { Append($"[color=orange][Storage] download failed: {down.Message}[/color]"); return; }
        Godot.Collections.Dictionary data = down.Data.AsGodotDictionary();
        byte[] payloadBytes = data.ContainsKey("data") ? data["data"].AsByteArray() : System.Array.Empty<byte>();
        Append($"[color=green][Storage] Downloaded {payloadBytes.Length} bytes: \"{System.Text.Encoding.UTF8.GetString(payloadBytes)}\"[/color]");
    }

    // --- Statistics (Step 2) ---

    private async Task OnStatsPressed()
    {
        XboxUser user = _auth.XboxUser;
        if (user == null) return;

        string[] stats = { StatHighScore, StatLevelsCleared };

        // Stage real-time tracking so StatChanged fires once values land.
        Xbox.Stats.TrackStats(user, stats);

        // 1. Stage a couple of title-managed statistics.
        Xbox.Stats.SetStatInteger(user, StatHighScore, 12500);
        Xbox.Stats.SetStatInteger(user, StatLevelsCleared, 7);

        // 2. Flush the staged values to the Xbox service.
        XboxResult flush = await Xbox.Stats.FlushStatsAsync(user);
        if (!IsInsideTree()) return;
        if (!flush.Ok) { Append($"[color=orange][Stats] flush failed: {flush.Message} ({flush.Code})[/color]"); return; }
        Append("[Stats] Flushed HighScore=12500, LevelsCleared=7.");

        // 3. Query them back.
        XboxResult query = await Xbox.Stats.QueryUserStatsAsync(user, stats);
        if (!IsInsideTree()) return;
        if (!query.Ok) { Append($"[color=orange][Stats] query failed: {query.Message}[/color]"); return; }
        Append($"[color=green][Stats] Queried back: {query.Data.AsGodotDictionary()}[/color]");
    }

    private void OnStatChanged(XboxUser user, string statName, Variant value) =>
        Append($"[Stats] tracked change: {statName} = {value}");

    private void SetButtonsEnabled(bool enabled) { _storageBtn.Disabled = !enabled; _statsBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}
