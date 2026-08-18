using System.Threading.Tasks;
using Godot;
using GodotXbox.Internal;
using GodotXbox.Types;

namespace GodotXbox.Services;

/// <summary><c>GDK.game_ui</c> — system dialogs, profile cards, and player pickers.</summary>
public sealed class XboxGameUi : XboxServiceBase
{
    internal XboxGameUi(GodotObject o) : base(o) { }

    public Task<XboxResult> ShowMessageDialogAsync(
        string title, string message, string firstButton,
        string secondButton = "", string thirdButton = "",
        string defaultButton = "", string cancelButton = "") =>
        CallResultAsync("show_message_dialog_async", title, message, firstButton,
            secondButton, thirdButton, defaultButton, cancelButton);

    public XboxResult SetNotificationPositionHint(string position) =>
        XboxResult.From(Call("set_notification_position_hint", position).AsGodotObject());

    public Task<XboxResult> ShowPlayerProfileCardAsync(XboxUser requestingUser, string targetXuid) =>
        CallResultAsync("show_player_profile_card_async", requestingUser?.Raw, targetXuid);

    public Task<XboxResult> ShowPlayerPickerAsync(
        XboxUser requestingUser, string prompt, string[] selectableXuids,
        string[] preselectedXuids, int minSelectionCount, int maxSelectionCount) =>
        CallResultAsync("show_player_picker_async", requestingUser?.Raw, prompt,
            selectableXuids, preselectedXuids, minSelectionCount, maxSelectionCount);

    public Task<XboxResult> ResolvePrivilegeWithUiAsync(XboxUser user, int privilege) =>
        CallResultAsync("resolve_privilege_with_ui_async", user?.Raw, privilege);

    public Task<XboxResult> ShowAchievementsAsync(XboxUser requestingUser) =>
        CallResultAsync("show_achievements_async", requestingUser?.Raw);

    public Task<XboxResult> ShowErrorDialogAsync(int errorCode, string context) =>
        CallResultAsync("show_error_dialog_async", errorCode, context);

    public Task<XboxResult> ShowSendGameInviteAsync(
        XboxUser requestingUser, string sessionConfigurationId, string sessionTemplateName,
        string sessionId, string invitationText, string customActivationContext) =>
        CallResultAsync("show_send_game_invite_async", requestingUser?.Raw, sessionConfigurationId,
            sessionTemplateName, sessionId, invitationText, customActivationContext);

    public Task<XboxResult> ShowTextEntryAsync(
        string titleText, string descriptionText, string defaultText, string inputScope, int maxTextLength) =>
        CallResultAsync("show_text_entry_async", titleText, descriptionText, defaultText, inputScope, maxTextLength);
}
