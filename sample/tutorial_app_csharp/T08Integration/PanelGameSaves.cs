using Godot;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;
using GodotPlayFab.Services;

public partial class PanelGameSaves : VBoxContainer
{
    private const string SaveFile = "progress.dat";
    private const string ActionKeepCloud = "keep_cloud";
    private const string ActionLastKnownGood = "last_known_good";
    private const string ActionLastConflict = "last_conflict";
    private Label _status, _lastRead;
    private Button _write, _read, _resolve;
    private AcceptDialog _resolveDialog;
    private Auth _auth;
    private string _saveFolder = string.Empty;
    private bool _initialized;
    public override async void _Ready()
    {
        _status = GetNode<Label>("Status"); _lastRead = GetNode<Label>("LastRead"); _write = GetNode<Button>("Write"); _read = GetNode<Button>("Read"); _resolve = GetNode<Button>("Resolve"); _resolveDialog = GetNode<AcceptDialog>("ResolveDialog");
        _resolveDialog.AddButton("Keep cloud version", true, ActionKeepCloud); _resolveDialog.AddButton("Roll back to last known good", true, ActionLastKnownGood); _resolveDialog.AddButton("Roll back to last conflict", true, ActionLastConflict);
        _auth = GetNodeOrNull<Auth>("/root/Auth"); if (_auth == null) { _status.Text = "[ERR] Auth autoload missing"; return; }
        _auth.StateChanged += state => { if (!_initialized && _auth.IsSignedIn()) _ = InitializeAfterSignInAsync(); };
        if (_auth.IsSignedIn()) await InitializeAfterSignInAsync(); else { await _auth.SignInAsync(); if (IsInsideTree() && _auth.IsSignedIn()) await InitializeAfterSignInAsync(); }
    }
    private async Task InitializeAfterSignInAsync()
    {
        if (_initialized) return; _initialized = true;
        PlayFabResult result = await PlayFab.GameSaves.AddUserWithUiAsync(_auth.PlayFabUser);
        if (!IsInsideTree()) return;
        if (!result.Ok) { _status.Text = $"Add user failed: {result.Message}"; GD.PushWarning($"[Gs] add_user failed: {result.Message}"); return; }
        _saveFolder = TutorialSupport.DictString(result.Data.AsGodotDictionary(), "folder"); _status.Text = $"Folder: {_saveFolder}"; GD.Print($"[Gs] user folder resolved: {_saveFolder}");
        _write.Pressed += async () => await OnWritePressed(); _read.Pressed += OnReadPressed; _resolve.Pressed += () => { if (string.IsNullOrEmpty(_saveFolder)) _status.Text = "Resolve unavailable — folder not yet resolved"; else _resolveDialog.PopupCentered(); }; _resolveDialog.CustomAction += async a => await OnResolveAction(a);
    }
    private async Task OnWritePressed()
    {
        if (string.IsNullOrEmpty(_saveFolder)) return;
        string path = _saveFolder.PathJoin(SaveFile); string gamertag = _auth.XboxUser?.Gamertag ?? "(unknown)"; string payload = $"saved={gamertag} timestamp={Time.GetDatetimeStringFromSystem()}";
        using Godot.FileAccess f = Godot.FileAccess.Open(path, Godot.FileAccess.ModeFlags.Write); if (f == null) { _status.Text = $"Write open failed: {Godot.FileAccess.GetOpenError()}"; return; }
        f.StoreString(payload); PlayFabResult upload = await PlayFab.GameSaves.UploadWithUiAsync(_auth.PlayFabUser, false); if (!IsInsideTree()) return;
        _status.Text = upload.Ok ? $"Wrote {SaveFile} ({payload.Length} bytes), upload synced" : $"Wrote locally, upload failed: {upload.Message}";
    }
    private void OnReadPressed()
    {
        if (string.IsNullOrEmpty(_saveFolder)) return; string path = _saveFolder.PathJoin(SaveFile); using Godot.FileAccess f = Godot.FileAccess.Open(path, Godot.FileAccess.ModeFlags.Read); _lastRead.Text = f == null ? $"Open failed: {Godot.FileAccess.GetOpenError()}" : f.GetAsText();
    }
    private async Task OnResolveAction(StringName action)
    {
        _resolveDialog.Hide(); int option; string label; string text = action.ToString();
        if (text == ActionKeepCloud) { option = PlayFabGameSaves.ADDUSEROPTIONNONE; label = "keep cloud version"; } else if (text == ActionLastKnownGood) { option = PlayFabGameSaves.ADDUSEROPTIONROLLBACKTOLASTKNOWNGOOD; label = "rolled back to last known good"; } else if (text == ActionLastConflict) { option = PlayFabGameSaves.ADDUSEROPTIONROLLBACKTOLASTCONFLICT; label = "rolled back to last conflict"; } else return;
        PlayFabResult result = await PlayFab.GameSaves.AddUserWithUiAsync(_auth.PlayFabUser, option); if (!IsInsideTree()) return;
        if (!result.Ok) { _status.Text = $"Resolution failed ({label}): {result.Message}"; return; }
        _saveFolder = TutorialSupport.DictString(result.Data.AsGodotDictionary(), "folder", _saveFolder); _status.Text = $"Resolved — {label}. Folder: {_saveFolder}";
    }
}



