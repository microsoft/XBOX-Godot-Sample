using System.Threading.Tasks;
using Godot;
using GodotGdk.Internal;
using GodotGdk.Types;

namespace GodotGdk.Services;

/// <summary><c>GDK.game_ui</c> — system dialogs, profile cards, and player pickers.</summary>
public sealed class GdkGameUi : GdkServiceBase
{
    internal GdkGameUi(GodotObject o) : base(o) { }

    public Task<GdkResult> ShowMessageDialogAsync(
        string title, string message, string firstButton,
        string secondButton = "", string thirdButton = "",
        string defaultButton = "", string cancelButton = "") =>
        CallResultAsync("show_message_dialog_async", title, message, firstButton,
            secondButton, thirdButton, defaultButton, cancelButton);

    public GdkResult SetNotificationPositionHint(string position) =>
        GdkResult.From(Call("set_notification_position_hint", position).AsGodotObject());

    public Task<GdkResult> ShowPlayerProfileCardAsync(GdkUser requestingUser, string targetXuid) =>
        CallResultAsync("show_player_profile_card_async", requestingUser?.Raw, targetXuid);

    public Task<GdkResult> ShowPlayerPickerAsync(
        GdkUser requestingUser, string prompt, string[] selectableXuids,
        string[] preselectedXuids, int minSelectionCount, int maxSelectionCount) =>
        CallResultAsync("show_player_picker_async", requestingUser?.Raw, prompt,
            selectableXuids, preselectedXuids, minSelectionCount, maxSelectionCount);

    public Task<GdkResult> ResolvePrivilegeWithUiAsync(GdkUser user, int privilege) =>
        CallResultAsync("resolve_privilege_with_ui_async", user?.Raw, privilege);

    public Task<GdkResult> ShowAchievementsAsync(GdkUser requestingUser) =>
        CallResultAsync("show_achievements_async", requestingUser?.Raw);

    public Task<GdkResult> ShowErrorDialogAsync(int errorCode, string context) =>
        CallResultAsync("show_error_dialog_async", errorCode, context);

    public Task<GdkResult> ShowSendGameInviteAsync(
        GdkUser requestingUser, string sessionConfigurationId, string sessionTemplateName,
        string sessionId, string invitationText, string customActivationContext) =>
        CallResultAsync("show_send_game_invite_async", requestingUser?.Raw, sessionConfigurationId,
            sessionTemplateName, sessionId, invitationText, customActivationContext);

    public Task<GdkResult> ShowTextEntryAsync(
        string titleText, string descriptionText, string defaultText, string inputScope, int maxTextLength) =>
        CallResultAsync("show_text_entry_async", titleText, descriptionText, defaultText, inputScope, maxTextLength);
}
