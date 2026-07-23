using Godot;
using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using GodotGdk;
using GodotGdk.Types;

/// <summary>
/// GDK Tutorial 4 reference scene — Multiplayer Activity + presence.
///
/// GDK-only, no PlayFab. This scene drives the Xbox Multiplayer Activity
/// surface directly: set the local user's activity with a title-defined
/// connection string, list friends via the Xbox Social Manager, filter them
/// by the play_multiplayer permission, send targeted invites or open the
/// system invite picker, and track friend activity + presence.
/// </summary>
public partial class G04Mpa : Control
{
    // Title-defined connection string. A shipping title encodes the data a
    // peer needs to rejoin (e.g. a session id); here we use a static token so
    // the MPA round-trip works without any networking backend.
    private const string ConnectionString = "gdk-tutorial-session-1";
    private const int MaxPlayers = 4;

    private RichTextLabel _log;
    private ItemList _friendsList;
    private Button _activityBtn, _refreshBtn, _inviteBtn, _pickerBtn, _trackBtn, _stopBtn, _backBtn;

    private GdkAuth _auth;
    private bool _socialGraphStarted;
    private GdkSocialGroup _friendsGroup;
    private bool _activitySet;
    private string[] _watchedXuids = Array.Empty<string>();

    public override async void _Ready()
    {
        _log = GetNode<RichTextLabel>("Root/LogPanel/Log");
        _friendsList = GetNode<ItemList>("Root/Friends");
        _activityBtn = GetNode<Button>("Root/Buttons/SetActivityBtn");
        _refreshBtn = GetNode<Button>("Root/Buttons/RefreshBtn");
        _inviteBtn = GetNode<Button>("Root/Buttons/InviteBtn");
        _pickerBtn = GetNode<Button>("Root/Buttons/PickerBtn");
        _trackBtn = GetNode<Button>("Root/Buttons/TrackBtn");
        _stopBtn = GetNode<Button>("Root/Buttons/StopBtn");
        _backBtn = GetNode<Button>("Root/Buttons/BackBtn");
        _backBtn.Pressed += OnBackPressed;
        _activityBtn.Pressed += async () => await OnSetActivityPressed();
        _refreshBtn.Pressed += async () => await OnRefreshPressed();
        _inviteBtn.Pressed += async () => await OnInvitePressed();
        _pickerBtn.Pressed += async () => await OpenInvitePickerAsync();
        _trackBtn.Pressed += async () => await OnTrackPressed();
        _stopBtn.Pressed += OnStopPressed;

        _auth = GetNodeOrNull<GdkAuth>("/root/GdkAuth");
        if (_auth == null || !Gdk.IsAvailable)
        {
            Append("[color=red]GdkAuth autoload or GDK extension missing.[/color]");
            SetButtonsEnabled(false);
            return;
        }

        // Forward incoming MPA invites so the developer can see the payload.
        Gdk.MultiplayerActivity.PendingInviteReceived += OnPendingInviteReceived;
        Gdk.MultiplayerActivity.InviteAccepted += OnInviteAccepted;

        SetButtonsEnabled(false);
        Append("Waiting for sign-in…");
        bool signedIn = await _auth.SignInAsync();
        if (!IsInsideTree()) return;
        if (!signedIn)
        {
            Append($"[color=red]Sign-in failed at {_auth.GetLastErrorStage()}: {_auth.GetLastErrorMessage()}[/color]");
            return;
        }
        Append("Signed in. Set your activity first, then invite or track friends.");
        SetButtonsEnabled(true);
        await OnRefreshPressed();
    }

    public override void _ExitTree()
    {
        // Release the social-graph group so the Social Manager stops issuing
        // background work after the scene is gone.
        if (!Gdk.IsAvailable) return;
        Gdk.MultiplayerActivity.PendingInviteReceived -= OnPendingInviteReceived;
        Gdk.MultiplayerActivity.InviteAccepted -= OnInviteAccepted;
        if (_friendsGroup != null)
        {
            Gdk.Social.DestroySocialGroup(_friendsGroup);
            _friendsGroup = null;
        }
        if (_socialGraphStarted)
        {
            GdkUser user = _auth?.XboxUser;
            if (user != null) Gdk.Social.StopSocialGraph(user);
            _socialGraphStarted = false;
        }
    }

    // --- Step 1: set the local activity ---

    private async Task OnSetActivityPressed()
    {
        GdkUser user = _auth.XboxUser;
        if (user == null) return;
        GdkResult result = await Gdk.MultiplayerActivity.SetActivityAsync(
            user, ConnectionString, "followed", MaxPlayers, 1, string.Empty, false);
        if (!IsInsideTree()) return;
        if (result.Ok)
        {
            _activitySet = true;
            Append($"[color=green][MPA] Activity set (connection_string={ConnectionString}).[/color]");
        }
        else
        {
            Append($"[color=orange][MPA] set_activity failed: {result.Message} ({result.Code})[/color]");
        }
    }

    // --- Friends list via the Xbox Social Manager ---

    private async Task OnRefreshPressed()
    {
        _friendsList.Clear();
        _friendsList.AddItem("(loading friends…)");
        _friendsList.SetItemDisabled(0, true);
        Godot.Collections.Array friends = await GetFriendsAsync();
        if (!IsInsideTree()) return;
        _friendsList.Clear();
        if (friends.Count == 0)
        {
            _friendsList.AddItem("(no friends found)");
            _friendsList.SetItemDisabled(0, true);
            Append("[i]No friends returned by Social Manager. Confirm sign-in and that the title has the Social capability.[/i]");
            return;
        }
        foreach (Variant f in friends)
        {
            GodotObject friend = f.AsGodotObject();
            string xuid = friend.Get("xuid").AsString();
            int idx = _friendsList.AddItem($"{FormatGamertag(friend)}  —  {xuid}");
            _friendsList.SetItemMetadata(idx, xuid);
        }
        Append($"Loaded {friends.Count} friends.");
    }

    private async Task<Godot.Collections.Array> GetFriendsAsync()
    {
        GdkUser user = _auth.XboxUser;
        if (user == null) return new Godot.Collections.Array();
        if (!_socialGraphStarted)
        {
            GdkResult sg = Gdk.Social.StartSocialGraph(user);
            if (!sg.Ok) { GD.PushWarning($"[MPA] start_social_graph failed: {sg.Message}"); return new Godot.Collections.Array(); }
            _socialGraphStarted = true;
        }
        if (_friendsGroup == null)
        {
            GdkResult f = await Gdk.Social.GetFriendsAsync(user);
            if (!f.Ok) { GD.PushWarning($"[MPA] get_friends failed: {f.Message}"); return new Godot.Collections.Array(); }
            GdkSocialGroup group = f.DataAs<GdkSocialGroup>();
            if (!IsInsideTree())
            {
                // Scene was torn down during the await; _ExitTree already ran
                // and could not see this group. Destroy it now so the native
                // social-group handle does not leak.
                if (group != null) Gdk.Social.DestroySocialGroup(group);
                return new Godot.Collections.Array();
            }
            _friendsGroup = group;
        }
        GdkResult users = Gdk.Social.GetGroupUsers(_friendsGroup);
        if (!users.Ok) { GD.PushWarning($"[MPA] get_group_users failed: {users.Message}"); return new Godot.Collections.Array(); }
        return users.Data.AsGodotArray();
    }

    // --- Step 5: invites ---

    private async Task OnInvitePressed()
    {
        if (!_activitySet)
        {
            Append("[color=orange]Set your activity first (Step 1) so invites carry a connection string.[/color]");
            return;
        }
        string[] xuids = SelectedXuids();
        if (xuids.Length == 0) { Append("[color=orange]Select a friend in the list above first.[/color]"); return; }
        string[] allowed = await FilterInvitableAsync(xuids);
        if (!IsInsideTree()) return;
        if (allowed.Length == 0) { Append("[color=orange]Invite blocked by the play_multiplayer permission.[/color]"); return; }
        // Empty connection_string reuses the cached activity connection string
        // set by SetActivityAsync above.
        GdkResult result = await Gdk.MultiplayerActivity.SendInvitesAsync(_auth.XboxUser, allowed, false, string.Empty);
        if (!IsInsideTree()) return;
        if (result.Ok) Append($"[MPA] Sent invite to {allowed.Length} friend(s).");
        else Append($"[color=orange][MPA] send_invites failed: {result.Message} ({result.Code})[/color]");
    }

    private async Task OpenInvitePickerAsync()
    {
        if (!_activitySet)
        {
            Append("[color=orange]Set your activity first (Step 1) before opening the picker.[/color]");
            return;
        }
        GdkResult result = await Gdk.MultiplayerActivity.ShowInviteUiAsync(_auth.XboxUser);
        if (!IsInsideTree()) return;
        if (!result.Ok) Append($"[color=orange][MPA] show_invite_ui failed: {result.Message}[/color]");
    }

    private async Task<string[]> FilterInvitableAsync(string[] xuids)
    {
        GdkUser user = _auth.XboxUser;
        if (user == null || xuids.Length == 0) return Array.Empty<string>();
        GdkResult pf = await Gdk.Privacy.BatchCheckPermissionAsync(user, "play_multiplayer", xuids);
        if (!pf.Ok) { GD.PushWarning($"[MPA] permission batch failed: {pf.Message}"); return Array.Empty<string>(); }
        var allowed = new List<string>();
        foreach (Variant entry in pf.Data.AsGodotArray())
        {
            Godot.Collections.Dictionary row = entry.AsGodotDictionary();
            if (TutorialSupport.DictBool(row, "allowed")) allowed.Add(TutorialSupport.DictString(row, "target_xuid"));
        }
        return allowed.ToArray();
    }

    // --- Steps 6 + 7: friend activity + presence tracking ---

    private async Task OnTrackPressed()
    {
        string[] xuids = SelectedXuids();
        if (xuids.Length == 0) { Append("[color=orange]Select one or more friends in the list above first.[/color]"); return; }
        _watchedXuids = xuids;
        GdkUser user = _auth.XboxUser;

        GdkResult activities = await Gdk.MultiplayerActivity.GetActivitiesAsync(user, xuids);
        if (!IsInsideTree()) return;
        if (!activities.Ok) Append($"[color=orange][MPA] get_activities failed: {activities.Message}[/color]");

        Gdk.Presence.TrackPresence(user, xuids, Array.Empty<long>());
        GdkResult presence = await Gdk.Presence.GetPresenceAsync(xuids);
        if (!IsInsideTree()) return;
        if (!presence.Ok) Append($"[color=orange][Pres] get_presence failed: {presence.Message}[/color]");

        foreach (string xuid in xuids)
        {
            GdkMultiplayerActivityInfo info = Gdk.MultiplayerActivity.GetCachedActivity(xuid);
            if (info != null) Append($"[MPA] {xuid} activity: connection_string={info.ConnectionString}");
            else Append($"[MPA] {xuid} has no cached activity.");
        }
    }

    private void OnStopPressed()
    {
        if (_watchedXuids.Length == 0) return;
        Gdk.Presence.StopTrackingPresence(_auth.XboxUser, _watchedXuids, Array.Empty<long>());
        Append($"Stopped tracking {_watchedXuids.Length} friend(s).");
        _watchedXuids = Array.Empty<string>();
    }

    // --- Incoming invite signals ---

    private void OnPendingInviteReceived(Godot.Collections.Dictionary invite) =>
        Append($"[MPA] Pending invite from {TutorialSupport.DictString(invite, "sender_xuid", "(unknown)")}.");

    private void OnInviteAccepted(Godot.Collections.Dictionary invite)
    {
        string conn = TutorialSupport.DictString(invite, "connection_string");
        if (string.IsNullOrEmpty(conn)) conn = TutorialSupport.DictString(invite, "raw_uri");
        Append($"[color=green][MPA] Invite accepted — join {conn}.[/color]");
    }

    // --- Helpers ---

    private string[] SelectedXuids()
    {
        var list = new List<string>();
        foreach (int idx in _friendsList.GetSelectedItems())
        {
            Variant meta = _friendsList.GetItemMetadata(idx);
            if (meta.VariantType == Variant.Type.String && !string.IsNullOrEmpty(meta.AsString())) list.Add(meta.AsString());
        }
        return list.ToArray();
    }

    private static string FormatGamertag(GodotObject friend)
    {
        string gamertag = friend.Get("gamertag").AsString();
        if (!string.IsNullOrEmpty(gamertag)) return gamertag;
        string display = friend.Get("display_name").AsString();
        return string.IsNullOrEmpty(display) ? "(unknown)" : display;
    }

    private void SetButtonsEnabled(bool enabled)
    {
        _activityBtn.Disabled = !enabled;
        _refreshBtn.Disabled = !enabled;
        _inviteBtn.Disabled = !enabled;
        _pickerBtn.Disabled = !enabled;
        _trackBtn.Disabled = !enabled;
        _stopBtn.Disabled = !enabled;
    }

    private void Append(string line) { _log.AppendText(line + "\n"); GD.Print(line); }
    private void OnBackPressed() => GetTree().ChangeSceneToFile("res://Shared/tutorial_picker.tscn");
}
