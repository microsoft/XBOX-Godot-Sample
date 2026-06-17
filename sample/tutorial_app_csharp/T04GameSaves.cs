using Godot;
using System.Threading.Tasks;
using GodotPlayFab;
using GodotPlayFab.Types;
using GodotPlayFab.Services;

public partial class T04GameSaves : Control
{
    private const string ActionKeepCloud = "keep_cloud";
    private const string ActionLastKnownGood = "last_known_good";
    private const string ActionLastConflict = "last_conflict";
    private RichTextLabel _log;
    private Button _addBtn, _writeBtn, _uploadBtn, _stateBtn, _resolveBtn, _backBtn;
    private AcceptDialog _resolveDialog;
    private Auth _auth;
    private string _saveFolder = string.Empty;

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _addBtn = GetNode<Button>("Root/Buttons/AddBtn");
        _writeBtn = GetNode<Button>("Root/Buttons/WriteBtn");
        _uploadBtn = GetNode<Button>("Root/Buttons/UploadBtn");
        _stateBtn = GetNode<Button>("Root/Buttons/StateBtn");
        _resolveBtn = GetNode<Button>("Root/Buttons/ResolveBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _resolveDialog = GetNode<AcceptDialog>("ResolveDialog");
        _backBtn.Pressed += OnBackPressed;
        _addBtn.Pressed += async () => await AddToGameSavesAsync();
        _writeBtn.Pressed += () => WriteSave(1234);
        _uploadBtn.Pressed += async () => await UploadAsync("Tutorial 4 — demo save");
        _stateBtn.Pressed += PrintCloudState;
        _resolveBtn.Pressed += OnResolvePressed;
        _resolveDialog.AddButton("Keep cloud version", true, ActionKeepCloud);
        _resolveDialog.AddButton("Roll back to last known good", true, ActionLastKnownGood);
        _resolveDialog.AddButton("Roll back to last conflict", true, ActionLastConflict);
        _resolveDialog.CustomAction += async action => await OnResolveAction(action);
        _auth = GetNodeOrNull<Auth>("/root/Auth");
        if (_auth == null || !PlayFab.IsAvailable) { Append("[color=red]Auth autoload or PlayFab extension missing.[/color]"); SetButtonsEnabled(false); return; }
        SetButtonsEnabled(false);
        Append("Waiting for sign-in…");
        if (await _auth.SignInAsync()) { Append("Signed in."); SetButtonsEnabled(true); }
        else Append($"[color=red]Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}[/color]");
    }

    private async Task AddToGameSavesAsync()
    {
        PlayFabUser user = _auth.PlayFabUser;
        if (user == null) return;
        if (!user.HasLocalUserHandle) { Append("[color=red][Save] PlayFab session is custom-id; Game Saves needs Xbox.[/color]"); return; }
        PlayFabResult result = await PlayFab.GameSaves.AddUserWithUiAsync(user);
        if (!result.Ok) { Append($"[color=orange][Save] Add user failed: {result.Message} ({result.Code})[/color]"); return; }
        Godot.Collections.Dictionary data = result.Data.AsGodotDictionary();
        _saveFolder = TutorialSupport.DictString(data, "folder");
        Append($"[Save] Game Saves folder: {_saveFolder}");
        Append($"[Save] Cloud connected: {TutorialSupport.DictBool(data, "connected_to_cloud")}, quota left: {TutorialSupport.DictInt(data, "remaining_quota", -1)} bytes");
    }

    private void WriteSave(int highscore)
    {
        if (string.IsNullOrEmpty(_saveFolder)) { Append("[color=red][Save] _save_folder not resolved yet — press 'Add user' first.[/color]"); return; }
        string path = _saveFolder.PathJoin("progress.json");
        using Godot.FileAccess file = Godot.FileAccess.Open(path, Godot.FileAccess.ModeFlags.Write);
        if (file == null) { Append($"[color=red][Save] Open failed: {path}[/color]"); return; }
        var payload = new Godot.Collections.Dictionary { ["highscore"] = highscore, ["saved_at"] = Time.GetDatetimeStringFromSystem(true) };
        file.StoreString(Json.Stringify(payload));
        Append($"[Save] Wrote save: highscore={highscore}");
    }

    private async Task UploadAsync(string description)
    {
        PlayFabUser user = _auth.PlayFabUser;
        if (user == null) return;
        if (!string.IsNullOrEmpty(description))
        {
            PlayFabResult desc = await PlayFab.GameSaves.SetSaveDescriptionAsync(user, description);
            if (!desc.Ok) Append($"[color=orange][Save] Description set failed: {desc.Message}[/color]");
        }
        PlayFabResult result = await PlayFab.GameSaves.UploadWithUiAsync(user, false);
        Append(result.Ok ? "[Save] Upload complete" : $"[color=orange][Save] Upload failed: {result.Message} ({result.Code})[/color]");
    }

    private void PrintCloudState()
    {
        PlayFabUser user = _auth.PlayFabUser;
        if (user == null) return;
        PlayFabResult connected = PlayFab.GameSaves.IsConnectedToCloud(user);
        if (connected.Ok) Append($"[Save] Cloud connected: {connected.Data.AsBool()}");
        PlayFabResult folderSize = PlayFab.GameSaves.GetFolderSize(user);
        if (folderSize.Ok) Append($"[Save] Folder size on disk: {folderSize.Data.AsInt64()} bytes");
        PlayFabResult quota = PlayFab.GameSaves.GetRemainingQuota(user);
        if (quota.Ok) Append($"[Save] Remaining quota: {quota.Data.AsInt64()} bytes");
    }

    private void OnResolvePressed()
    {
        PlayFabUser user = _auth.PlayFabUser;
        if (user == null || !user.HasLocalUserHandle) { Append("[color=red][Save] Resolve unavailable — sign in with an Xbox account first.[/color]"); return; }
        _resolveDialog.PopupCentered();
    }

    private async Task OnResolveAction(StringName action)
    {
        _resolveDialog.Hide();
        int option = PlayFabGameSaves.ADDUSEROPTIONNONE;
        string label = string.Empty;
        string text = action.ToString();
        if (text == ActionKeepCloud) { option = PlayFabGameSaves.ADDUSEROPTIONNONE; label = "keep cloud version"; }
        else if (text == ActionLastKnownGood) { option = PlayFabGameSaves.ADDUSEROPTIONROLLBACKTOLASTKNOWNGOOD; label = "rolled back to last known good"; }
        else if (text == ActionLastConflict) { option = PlayFabGameSaves.ADDUSEROPTIONROLLBACKTOLASTCONFLICT; label = "rolled back to last conflict"; }
        else return;
        await ApplyResolutionAsync(option, label);
    }

    private async Task ApplyResolutionAsync(int option, string label)
    {
        PlayFabResult result = await PlayFab.GameSaves.AddUserWithUiAsync(_auth.PlayFabUser, option);
        if (!result.Ok) { Append($"[color=orange][Save] Resolution failed ({label}): {result.Message}[/color]"); return; }
        _saveFolder = TutorialSupport.DictString(result.Data.AsGodotDictionary(), "folder", _saveFolder);
        Append($"[Save] Resolution complete — {label}. Folder: {_saveFolder}");
    }

    private void SetButtonsEnabled(bool enabled) { _addBtn.Disabled = !enabled; _writeBtn.Disabled = !enabled; _uploadBtn.Disabled = !enabled; _stateBtn.Disabled = !enabled; _resolveBtn.Disabled = !enabled; }
    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}


