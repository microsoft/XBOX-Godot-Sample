using Godot;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;

/// <summary>
/// GDK Tutorial 3 reference scene — Title Storage + user statistics.
///
/// GDK-only surfaces, no PlayFab. Buttons drive each step:
///   - Upload a small blob to Title Storage, then list + download it back
///     (GDK.title_storage upload/list/download).
///   - Stage and flush a couple of title-managed statistics, then query
///     them back (GDK.stats set/flush/query).
/// </summary>
public partial class G03StorageStats : Control
{
    // Title Storage uses "TrustedPlatform" for binary blobs scoped to the
    // signed-in user. Other valid storage types include "GlobalStorage" and
    // "Universal"; see the GDKTitleStorage reference for the full set.
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
    private GdkAuth _auth;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _storageBtn = GetNode<Button>("Root/Buttons/StorageBtn");
        _statsBtn = GetNode<Button>("Root/Buttons/StatsBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _backBtn.Pressed += OnBackPressed;
        _storageBtn.Pressed += async () => await OnStoragePressed();
        _statsBtn.Pressed += async () => await OnStatsPressed();

        _auth = GetNodeOrNull<GdkAuth>("/root/GdkAuth");
        if (_auth == null || !Gdk.IsAvailable)
        {
            Append("[color=red]GdkAuth autoload or GDK extension missing.[/color]");
            SetButtonsEnabled(false);
            return;
        }

        // Surface real-time stat changes for the tracked statistics.
        Gdk.Stats.StatChanged += OnStatChanged;

        SetButtonsEnabled(false);
        Append("Waiting for sign-in…");
        bool signedIn = await _auth.SignInAsync();
        if (!IsInsideTree()) return;
        if (signedIn) { Append("Signed in."); SetButtonsEnabled(true); }
        else Append($"[color=red]Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}[/color]");
    }

    public override void _ExitTree()
    {
        if (Gdk.IsAvailable) Gdk.Stats.StatChanged -= OnStatChanged;
    }

    // --- Title Storage (Step 1) ---

    private async Task OnStoragePressed()
    {
        GdkUser user = _auth.XboxUser;
        if (user == null) return;

        // 1. Upload a small blob.
        string payload = $"tutorial-payload @ {(long)Time.GetUnixTimeFromSystem()}";
        byte[] bytes = System.Text.Encoding.UTF8.GetBytes(payload);
        GdkResult up = await Gdk.TitleStorage.UploadBlobAsync(user, StorageType, BlobPath, bytes, "Tutorial Save");
        if (!IsInsideTree()) return;
        if (!up.Ok) { Append($"[color=orange][Storage] upload failed: {up.Message} ({up.Code})[/color]"); return; }
        Append($"[Storage] Uploaded {bytes.Length} bytes to {BlobPath}.");

        // 2. List blob metadata so the developer can see what's stored.
        GdkResult list = await Gdk.TitleStorage.ListBlobMetadataAsync(user, StorageType, string.Empty, 0, 0);
        if (!IsInsideTree()) return;
        if (list.Ok) Append($"[Storage] Listed blob metadata for {StorageType}.");

        // 3. Download the blob back and verify the round-trip.
        GdkResult down = await Gdk.TitleStorage.DownloadBlobAsync(user, StorageType, BlobPath);
        if (!IsInsideTree()) return;
        if (!down.Ok) { Append($"[color=orange][Storage] download failed: {down.Message}[/color]"); return; }
        Godot.Collections.Dictionary data = down.Data.AsGodotDictionary();
        byte[] payloadBytes = data.ContainsKey("data") ? data["data"].AsByteArray() : System.Array.Empty<byte>();
        Append($"[color=green][Storage] Downloaded {payloadBytes.Length} bytes: \"{System.Text.Encoding.UTF8.GetString(payloadBytes)}\"[/color]");
    }

    // --- Statistics (Step 2) ---

    private async Task OnStatsPressed()
    {
        GdkUser user = _auth.XboxUser;
        if (user == null) return;

        string[] stats = { StatHighScore, StatLevelsCleared };

        // Stage real-time tracking so StatChanged fires once values land.
        Gdk.Stats.TrackStats(user, stats);

        // 1. Stage a couple of title-managed statistics.
        Gdk.Stats.SetStatInteger(user, StatHighScore, 12500);
        Gdk.Stats.SetStatInteger(user, StatLevelsCleared, 7);

        // 2. Flush the staged values to the Xbox service.
        GdkResult flush = await Gdk.Stats.FlushStatsAsync(user);
        if (!IsInsideTree()) return;
        if (!flush.Ok) { Append($"[color=orange][Stats] flush failed: {flush.Message} ({flush.Code})[/color]"); return; }
        Append("[Stats] Flushed HighScore=12500, LevelsCleared=7.");

        // 3. Query them back.
        GdkResult query = await Gdk.Stats.QueryUserStatsAsync(user, stats);
        if (!IsInsideTree()) return;
        if (!query.Ok) { Append($"[color=orange][Stats] query failed: {query.Message}[/color]"); return; }
        Append($"[color=green][Stats] Queried back: {query.Data.AsGodotDictionary()}[/color]");
    }

    private void OnStatChanged(GdkUser user, string statName, Variant value) =>
        Append($"[Stats] tracked change: {statName} = {value}");

    private void SetButtonsEnabled(bool enabled) { _storageBtn.Disabled = !enabled; _statsBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}
