#include "gdk_game_ui.h"

#include <cerrno>
#include <cstdlib>
#include <string>
#include <unordered_set>
#include <vector>

#include <XGame.h>
#include <XGameUI.h>

#include "gdk.h"
#include "gdk_pending_signal.h"
#include "gdk_request_parsing.h"
#include "gdk_result.h"
#include "gdk_runtime.h"
#include "gdk_signal_xasync_context.h"
#include "gdk_user.h"

namespace godot {

namespace {

bool _try_parse_xuid(const String &p_xuid, uint64_t *r_xuid) {
    return gdk_request_parsing::try_parse_xuid(p_xuid, r_xuid, /*p_reject_zero=*/true);
}

Ref<GDKResult> _parse_xuid_list(
        const PackedStringArray &p_xuids,
        std::vector<uint64_t> *r_xuids,
        PackedStringArray *r_normalized_xuids,
        const String &p_empty_code,
        const String &p_invalid_code,
        const String &p_empty_message,
        const String &p_invalid_message) {
    if (r_xuids == nullptr) {
        return GDKResult::error_result(E_POINTER, "internal_error", "Missing XUID list output.");
    }

    if (p_xuids.is_empty()) {
        return GDKResult::error_result(E_INVALIDARG, p_empty_code, p_empty_message);
    }

    r_xuids->clear();
    r_xuids->reserve(static_cast<size_t>(p_xuids.size()));

    if (r_normalized_xuids != nullptr) {
        r_normalized_xuids->clear();
    }

    for (int64_t i = 0; i < p_xuids.size(); ++i) {
        uint64_t parsed = 0;
        if (!_try_parse_xuid(p_xuids[i], &parsed)) {
            return GDKResult::error_result(E_INVALIDARG, p_invalid_code, p_invalid_message);
        }

        r_xuids->push_back(parsed);
        if (r_normalized_xuids != nullptr) {
            r_normalized_xuids->push_back(String::num_uint64(parsed));
        }
    }

    return GDKResult::ok_result();
}

bool _try_parse_message_dialog_button(const String &p_name, XGameUiMessageDialogButton *r_button, String *r_normalized_name = nullptr) {
    if (r_button == nullptr) {
        return false;
    }

    const String normalized = p_name.strip_edges().to_lower();
    if (normalized == "first" || normalized == "0") {
        *r_button = XGameUiMessageDialogButton::First;
        if (r_normalized_name != nullptr) {
            *r_normalized_name = "first";
        }
        return true;
    }
    if (normalized == "second" || normalized == "1") {
        *r_button = XGameUiMessageDialogButton::Second;
        if (r_normalized_name != nullptr) {
            *r_normalized_name = "second";
        }
        return true;
    }
    if (normalized == "third" || normalized == "2") {
        *r_button = XGameUiMessageDialogButton::Third;
        if (r_normalized_name != nullptr) {
            *r_normalized_name = "third";
        }
        return true;
    }

    return false;
}

String _message_dialog_button_to_string(XGameUiMessageDialogButton p_button) {
    if (p_button == XGameUiMessageDialogButton::Second) {
        return "second";
    }
    if (p_button == XGameUiMessageDialogButton::Third) {
        return "third";
    }
    return "first";
}

bool _try_parse_notification_position(
        const String &p_position,
        XGameUiNotificationPositionHint *r_hint,
        String *r_normalized_position = nullptr) {
    if (r_hint == nullptr) {
        return false;
    }

    const String normalized = p_position.strip_edges().to_lower();
    if (normalized == "bottom_center") {
        *r_hint = XGameUiNotificationPositionHint::BottomCenter;
        if (r_normalized_position != nullptr) {
            *r_normalized_position = normalized;
        }
        return true;
    }
    if (normalized == "bottom_left") {
        *r_hint = XGameUiNotificationPositionHint::BottomLeft;
        if (r_normalized_position != nullptr) {
            *r_normalized_position = normalized;
        }
        return true;
    }
    if (normalized == "bottom_right") {
        *r_hint = XGameUiNotificationPositionHint::BottomRight;
        if (r_normalized_position != nullptr) {
            *r_normalized_position = normalized;
        }
        return true;
    }
    if (normalized == "top_center") {
        *r_hint = XGameUiNotificationPositionHint::TopCenter;
        if (r_normalized_position != nullptr) {
            *r_normalized_position = normalized;
        }
        return true;
    }
    if (normalized == "top_left") {
        *r_hint = XGameUiNotificationPositionHint::TopLeft;
        if (r_normalized_position != nullptr) {
            *r_normalized_position = normalized;
        }
        return true;
    }
    if (normalized == "top_right") {
        *r_hint = XGameUiNotificationPositionHint::TopRight;
        if (r_normalized_position != nullptr) {
            *r_normalized_position = normalized;
        }
        return true;
    }

    return false;
}

class MessageDialogAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Message dialog cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        XGameUiMessageDialogButton selected_button = XGameUiMessageDialogButton::First;
        HRESULT result_hr = XGameUiShowMessageDialogResult(p_async_block, &selected_button);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Message dialog cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the message dialog flow.",
                    "game_ui_message_dialog_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary data;
        data["selected_button"] = _message_dialog_button_to_string(selected_button);
        data["selected_button_index"] = static_cast<int64_t>(selected_button);

        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    MessageDialogAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

class PlayerProfileCardAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Player profile card UI cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT result_hr = XGameUiShowPlayerProfileCardResult(p_async_block);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Player profile card UI cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the player profile card UI flow.",
                    "game_ui_player_profile_card_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        get_pending_signal()->complete(GDKResult::ok_result());
    }

public:
    PlayerProfileCardAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

class PlayerPickerAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<GDKResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = GDKResult::cancelled("Player picker UI cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        uint32_t selected_count = 0;
        HRESULT count_hr = XGameUiShowPlayerPickerResultCount(p_async_block, &selected_count);
        if (count_hr == E_ABORT) {
            result = GDKResult::cancelled("Player picker UI cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(count_hr)) {
            result = GDKResult::hresult_error(
                    count_hr,
                    "Failed to get the player picker result count.",
                    "game_ui_player_picker_count_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<uint64_t> selected_players(selected_count);
        uint32_t selected_used = 0;
        HRESULT result_hr = XGameUiShowPlayerPickerResult(
                p_async_block,
                selected_count,
                selected_players.data(),
                &selected_used);
        if (result_hr == E_ABORT) {
            result = GDKResult::cancelled("Player picker UI cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(result_hr)) {
            result = GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the player picker UI flow.",
                    "game_ui_player_picker_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        PackedStringArray selected_xuids;
        for (uint32_t i = 0; i < selected_used; ++i) {
            selected_xuids.push_back(String::num_uint64(selected_players[i]));
        }

        Dictionary data;
        data["selected_xuids"] = selected_xuids;
        data["selection_count"] = static_cast<int64_t>(selected_xuids.size());

        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    PlayerPickerAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

class AchievementsUiAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Achievements UI cancelled."));
            return;
        }

        HRESULT result_hr = XGameUiShowAchievementsResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Achievements UI cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the achievements UI flow.",
                    "game_ui_achievements_result_failed"));
            return;
        }

        get_pending_signal()->complete(GDKResult::ok_result());
    }

public:
    AchievementsUiAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

class ErrorDialogAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Error dialog cancelled."));
            return;
        }

        HRESULT result_hr = XGameUiShowErrorDialogResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Error dialog cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the error dialog flow.",
                    "game_ui_error_dialog_result_failed"));
            return;
        }

        get_pending_signal()->complete(GDKResult::ok_result());
    }

public:
    ErrorDialogAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

class SendGameInviteAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Send game invite UI cancelled."));
            return;
        }

        HRESULT result_hr = XGameUiShowSendGameInviteResult(p_async_block);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Send game invite UI cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to complete the send game invite UI flow.",
                    "game_ui_send_game_invite_result_failed"));
            return;
        }

        get_pending_signal()->complete(GDKResult::ok_result());
    }

public:
    SendGameInviteAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

class TextEntryAsyncContext final : public GDKSignalXAsyncContext {
protected:
    void finalize(XAsyncBlock *p_async_block) override {
        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            get_pending_signal()->complete(GDKResult::cancelled("Text entry UI cancelled."));
            return;
        }

        uint32_t buffer_size = 0;
        HRESULT size_hr = XGameUiShowTextEntryResultSize(p_async_block, &buffer_size);
        if (size_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Text entry UI cancelled."));
            return;
        }
        if (FAILED(size_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    size_hr,
                    "Failed to size the text entry UI result.",
                    "game_ui_text_entry_size_failed"));
            return;
        }

        std::vector<char> buffer(buffer_size > 0 ? buffer_size : 1, '\0');
        uint32_t used = 0;
        HRESULT result_hr = XGameUiShowTextEntryResult(p_async_block, static_cast<uint32_t>(buffer.size()), buffer.data(), &used);
        if (result_hr == E_ABORT) {
            get_pending_signal()->complete(GDKResult::cancelled("Text entry UI cancelled."));
            return;
        }
        if (FAILED(result_hr)) {
            get_pending_signal()->complete(GDKResult::hresult_error(
                    result_hr,
                    "Failed to read the text entry UI result.",
                    "game_ui_text_entry_result_failed"));
            return;
        }

        Dictionary data;
        data["text"] = String::utf8(buffer.data());
        get_pending_signal()->complete(GDKResult::ok_result(data));
    }

public:
    TextEntryAsyncContext(GDKRuntime *p_runtime, const Ref<GDKPendingSignal> &p_pending_signal) :
            GDKSignalXAsyncContext(p_runtime, p_pending_signal) {}
};

XGameUiTextEntryInputScope _parse_text_entry_input_scope(const String &p_input_scope) {
    const String normalized = p_input_scope.strip_edges().to_lower();
    if (normalized.is_empty() || normalized == "default") {
        return XGameUiTextEntryInputScope::Default;
    }
    if (normalized == "url") {
        return XGameUiTextEntryInputScope::Url;
    }
    if (normalized == "email" || normalized == "email_smtp_address") {
        return XGameUiTextEntryInputScope::EmailSmtpAddress;
    }
    if (normalized == "number") {
        return XGameUiTextEntryInputScope::Number;
    }
    if (normalized == "password") {
        return XGameUiTextEntryInputScope::Password;
    }
    if (normalized == "telephone" || normalized == "telephone_number") {
        return XGameUiTextEntryInputScope::TelephoneNumber;
    }
    if (normalized == "alphanumeric") {
        return XGameUiTextEntryInputScope::Alphanumeric;
    }
    if (normalized == "search") {
        return XGameUiTextEntryInputScope::Search;
    }
    if (normalized == "chat" || normalized == "chat_without_emoji") {
        return XGameUiTextEntryInputScope::ChatWithoutEmoji;
    }
    return XGameUiTextEntryInputScope::Default;
}

} // namespace

void GDKGameUI::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("show_message_dialog_async", "title", "message", "first_button", "second_button", "third_button", "default_button", "cancel_button"),
            &GDKGameUI::show_message_dialog_async,
            DEFVAL(String("OK")),
            DEFVAL(String()),
            DEFVAL(String()),
            DEFVAL(String("first")),
            DEFVAL(String("first")));
    ClassDB::bind_method(D_METHOD("set_notification_position_hint", "position"), &GDKGameUI::set_notification_position_hint);
    ClassDB::bind_method(D_METHOD("show_player_profile_card_async", "requesting_user", "target_xuid"), &GDKGameUI::show_player_profile_card_async);
    ClassDB::bind_method(
            D_METHOD("show_player_picker_async", "requesting_user", "prompt", "selectable_xuids", "preselected_xuids", "min_selection_count", "max_selection_count"),
            &GDKGameUI::show_player_picker_async,
            DEFVAL(PackedStringArray()),
            DEFVAL(static_cast<int64_t>(1)),
            DEFVAL(static_cast<int64_t>(1)));
    ClassDB::bind_method(D_METHOD("resolve_privilege_with_ui_async", "user", "privilege"), &GDKGameUI::resolve_privilege_with_ui_async);
    ClassDB::bind_method(D_METHOD("show_achievements_async", "requesting_user"), &GDKGameUI::show_achievements_async);
    ClassDB::bind_method(D_METHOD("show_error_dialog_async", "error_code", "context"), &GDKGameUI::show_error_dialog_async, DEFVAL(String()));
    ClassDB::bind_method(
            D_METHOD("show_send_game_invite_async", "requesting_user", "session_configuration_id", "session_template_name", "session_id", "invitation_text", "custom_activation_context"),
            &GDKGameUI::show_send_game_invite_async,
            DEFVAL(String()),
            DEFVAL(String()));
    ClassDB::bind_method(
            D_METHOD("show_text_entry_async", "title_text", "description_text", "default_text", "input_scope", "max_text_length"),
            &GDKGameUI::show_text_entry_async,
            DEFVAL(String()),
            DEFVAL(String()),
            DEFVAL(String()),
            DEFVAL(String("default")),
            DEFVAL(static_cast<int64_t>(0)));
}

void GDKGameUI::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

Ref<GDKResult> GDKGameUI::on_runtime_initialized() {
    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return GDKResult::error_result(
                E_FAIL,
                "runtime_not_initialized",
                "Cannot initialize the game UI service before the GDK runtime.");
    }

    m_runtime_ready = true;
    return GDKResult::ok_result();
}

void GDKGameUI::shutdown() {
    m_runtime_ready = false;
}

Signal GDKGameUI::show_message_dialog_async(
        const String &p_title,
        const String &p_message,
        const String &p_first_button,
        const String &p_second_button,
        const String &p_third_button,
        const String &p_default_button,
        const String &p_cancel_button) {
    const String title = p_title.strip_edges();
    const String message = p_message.strip_edges();
    if (title.is_empty()) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_title", "A non-empty dialog title is required.");
    }
    if (message.is_empty()) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_message", "A non-empty dialog message is required.");
    }

    const String first_button = p_first_button.strip_edges();
    const String second_button = p_second_button.strip_edges();
    const String third_button = p_third_button.strip_edges();
    if (first_button.is_empty()) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_button_label", "The first dialog button label is required.");
    }

    XGameUiMessageDialogButton default_button = XGameUiMessageDialogButton::First;
    String default_button_name;
    if (!_try_parse_message_dialog_button(p_default_button, &default_button, &default_button_name)) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_default_button", "default_button must be 'first', 'second', or 'third'.");
    }

    XGameUiMessageDialogButton cancel_button = XGameUiMessageDialogButton::First;
    String cancel_button_name;
    if (!_try_parse_message_dialog_button(p_cancel_button, &cancel_button, &cancel_button_name)) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_cancel_button", "cancel_button must be 'first', 'second', or 'third'.");
    }

    const bool has_second_button = !second_button.is_empty();
    const bool has_third_button = !third_button.is_empty();
    if (!has_second_button && (default_button_name == "second" || default_button_name == "third" || cancel_button_name == "second" || cancel_button_name == "third")) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_button_layout", "A second button label is required when selecting second/third as default or cancel.");
    }
    if (!has_third_button && (default_button_name == "third" || cancel_button_name == "third")) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_button_layout", "A third button label is required when selecting third as default or cancel.");
    }

    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new MessageDialogAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    const CharString title_utf8 = title.utf8();
    const CharString message_utf8 = message.utf8();
    const CharString first_button_utf8 = first_button.utf8();
    const CharString second_button_utf8 = second_button.utf8();
    const CharString third_button_utf8 = third_button.utf8();

    HRESULT hr = XGameUiShowMessageDialogAsync(
            context->get_async_block(),
            title_utf8.get_data(),
            message_utf8.get_data(),
            first_button_utf8.get_data(),
            has_second_button ? second_button_utf8.get_data() : nullptr,
            has_third_button ? third_button_utf8.get_data() : nullptr,
            default_button,
            cancel_button);
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the message dialog flow.",
                "game_ui_message_dialog_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Ref<GDKResult> GDKGameUI::set_notification_position_hint(const String &p_position) {
    XGameUiNotificationPositionHint position_hint = XGameUiNotificationPositionHint::BottomCenter;
    String normalized_position;
    if (!_try_parse_notification_position(p_position, &position_hint, &normalized_position)) {
        return GDKResult::error_result(
                E_INVALIDARG,
                "invalid_notification_position",
                "position must be one of: bottom_center, bottom_left, bottom_right, top_center, top_left, top_right.");
    }

    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return GDKResult::error_result(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    HRESULT hr = XGameUiSetNotificationPositionHint(position_hint);
    if (FAILED(hr)) {
        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to set the notification position hint.",
                "game_ui_notification_position_failed");
        return result;
    }

    Dictionary data;
    data["position"] = normalized_position;

    return GDKResult::ok_result(data);
}

Signal GDKGameUI::show_player_profile_card_async(const Ref<GDKUser> &p_requesting_user, const String &p_target_xuid) {
    uint64_t target_xuid = 0;
    if (!_try_parse_xuid(p_target_xuid, &target_xuid)) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_target_xuid", "target_xuid must be a non-empty, non-zero numeric string.");
    }

    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_requesting_user.is_valid() || p_requesting_user->get_handle() == nullptr) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to show profile card UI.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new PlayerProfileCardAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    HRESULT hr = XGameUiShowPlayerProfileCardAsync(
            context->get_async_block(),
            p_requesting_user->get_handle(),
            target_xuid);
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the player profile card UI flow.",
                "game_ui_player_profile_card_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal GDKGameUI::show_player_picker_async(
        const Ref<GDKUser> &p_requesting_user,
        const String &p_prompt,
        const PackedStringArray &p_selectable_xuids,
        const PackedStringArray &p_preselected_xuids,
        int64_t p_min_selection_count,
        int64_t p_max_selection_count) {
    const String prompt = p_prompt.strip_edges();
    if (prompt.is_empty()) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_prompt", "A non-empty player picker prompt is required.");
    }

    std::vector<uint64_t> selectable_xuids;
    Ref<GDKResult> selectable_parse_result = _parse_xuid_list(
            p_selectable_xuids,
            &selectable_xuids,
            nullptr,
            "missing_xuids",
            "invalid_xuids",
            "At least one selectable XUID is required.",
            "Each selectable XUID must be a non-empty, non-zero numeric string.");
    if (!selectable_parse_result->is_ok()) {
        return make_error_signal_internal(
                static_cast<HRESULT>(selectable_parse_result->get_hresult()),
                selectable_parse_result->get_code(),
                selectable_parse_result->get_message());
    }

    std::vector<uint64_t> preselected_xuids;
    if (!p_preselected_xuids.is_empty()) {
        Ref<GDKResult> preselected_parse_result = _parse_xuid_list(
                p_preselected_xuids,
                &preselected_xuids,
                nullptr,
                "invalid_preselected_xuids",
                "invalid_preselected_xuids",
                "Preselected XUIDs must be numeric strings.",
                "Each preselected XUID must be a non-empty, non-zero numeric string.");
        if (!preselected_parse_result->is_ok()) {
            return make_error_signal_internal(
                    static_cast<HRESULT>(preselected_parse_result->get_hresult()),
                    preselected_parse_result->get_code(),
                    preselected_parse_result->get_message());
        }
    }

    if (p_min_selection_count <= 0 || p_max_selection_count <= 0) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_selection_range", "min_selection_count and max_selection_count must be greater than zero.");
    }
    if (p_min_selection_count > p_max_selection_count) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_selection_range", "min_selection_count cannot be greater than max_selection_count.");
    }
    if (p_max_selection_count > static_cast<int64_t>(selectable_xuids.size())) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_selection_range", "max_selection_count cannot exceed the selectable_xuids count.");
    }
    if (preselected_xuids.size() > selectable_xuids.size()) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_preselected_xuids", "preselected_xuids cannot exceed selectable_xuids.");
    }

    std::unordered_set<uint64_t> selectable_xuids_set(selectable_xuids.begin(), selectable_xuids.end());
    for (uint64_t preselected_xuid : preselected_xuids) {
        if (selectable_xuids_set.find(preselected_xuid) == selectable_xuids_set.end()) {
            return make_error_signal_internal(E_INVALIDARG, "invalid_preselected_xuids", "Each preselected_xuid must be present in selectable_xuids.");
        }
    }

    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_requesting_user.is_valid() || p_requesting_user->get_handle() == nullptr) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to show player picker UI.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new PlayerPickerAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    const CharString prompt_utf8 = prompt.utf8();
    const uint64_t *selectable_xuids_ptr = selectable_xuids.empty() ? nullptr : selectable_xuids.data();
    const uint64_t *preselected_xuids_ptr = preselected_xuids.empty() ? nullptr : preselected_xuids.data();
    HRESULT hr = XGameUiShowPlayerPickerAsync(
            context->get_async_block(),
            p_requesting_user->get_handle(),
            prompt_utf8.get_data(),
            static_cast<uint32_t>(selectable_xuids.size()),
            selectable_xuids_ptr,
            static_cast<uint32_t>(preselected_xuids.size()),
            preselected_xuids_ptr,
            static_cast<uint32_t>(p_min_selection_count),
            static_cast<uint32_t>(p_max_selection_count));
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<GDKResult> result = GDKResult::hresult_error(
                hr,
                "Failed to start the player picker UI flow.",
                "game_ui_player_picker_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal GDKGameUI::resolve_privilege_with_ui_async(const Ref<GDKUser> &p_user, int64_t p_privilege) {
    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_user.is_valid() || p_user->get_handle() == nullptr) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to resolve privilege UI.");
    }

    GDKUsers *users = get_users_internal();
    if (users == nullptr) {
        return make_error_signal_internal(E_FAIL, "users_service_unavailable", "GDK users service is unavailable.");
    }

    return users->resolve_privilege_with_ui_async(p_user, p_privilege);
}

Signal GDKGameUI::show_achievements_async(const Ref<GDKUser> &p_requesting_user) {
    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_requesting_user.is_valid() || p_requesting_user->get_handle() == nullptr) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to show achievements UI.");
    }

    uint32_t title_id = 0;
    HRESULT title_hr = XGameGetXboxTitleId(&title_id);
    if (FAILED(title_hr)) {
        return make_error_signal_internal(title_hr, "xbox_title_id_unavailable", "Failed to resolve the Xbox title ID for the achievements UI.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new AchievementsUiAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    HRESULT hr = XGameUiShowAchievementsAsync(
            context->get_async_block(),
            p_requesting_user->get_handle(),
            title_id);
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                hr,
                "Failed to start the achievements UI flow.",
                "game_ui_achievements_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKGameUI::show_error_dialog_async(int64_t p_error_code, const String &p_context) {
    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new ErrorDialogAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    const String context_text = p_context.strip_edges();
    const CharString context_utf8 = context_text.utf8();

    HRESULT hr = XGameUiShowErrorDialogAsync(
            context->get_async_block(),
            static_cast<HRESULT>(p_error_code),
            context_text.is_empty() ? nullptr : context_utf8.get_data());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                hr,
                "Failed to start the error dialog flow.",
                "game_ui_error_dialog_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKGameUI::show_send_game_invite_async(
        const Ref<GDKUser> &p_requesting_user,
        const String &p_session_configuration_id,
        const String &p_session_template_name,
        const String &p_session_id,
        const String &p_invitation_text,
        const String &p_custom_activation_context) {
    const String session_configuration_id = p_session_configuration_id.strip_edges();
    const String session_template_name = p_session_template_name.strip_edges();
    const String session_id = p_session_id.strip_edges();
    if (session_configuration_id.is_empty() || session_template_name.is_empty() || session_id.is_empty()) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_session", "session_configuration_id, session_template_name, and session_id are all required.");
    }

    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }
    if (!p_requesting_user.is_valid() || p_requesting_user->get_handle() == nullptr) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_user", "A signed-in GDKUser is required to send a game invite.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new SendGameInviteAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    const String invitation_text = p_invitation_text.strip_edges();
    const String custom_activation_context = p_custom_activation_context.strip_edges();
    const CharString session_configuration_id_utf8 = session_configuration_id.utf8();
    const CharString session_template_name_utf8 = session_template_name.utf8();
    const CharString session_id_utf8 = session_id.utf8();
    const CharString invitation_text_utf8 = invitation_text.utf8();
    const CharString custom_activation_context_utf8 = custom_activation_context.utf8();

    HRESULT hr = XGameUiShowSendGameInviteAsync(
            context->get_async_block(),
            p_requesting_user->get_handle(),
            session_configuration_id_utf8.get_data(),
            session_template_name_utf8.get_data(),
            session_id_utf8.get_data(),
            invitation_text.is_empty() ? nullptr : invitation_text_utf8.get_data(),
            custom_activation_context.is_empty() ? nullptr : custom_activation_context_utf8.get_data());
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                hr,
                "Failed to start the send game invite UI flow.",
                "game_ui_send_game_invite_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

Signal GDKGameUI::show_text_entry_async(
        const String &p_title_text,
        const String &p_description_text,
        const String &p_default_text,
        const String &p_input_scope,
        int64_t p_max_text_length) {
    if (p_max_text_length < 0) {
        return make_error_signal_internal(E_INVALIDARG, "invalid_max_length", "max_text_length must be zero or greater.");
    }

    GDKRuntime *runtime = get_runtime_internal();
    if (runtime == nullptr || !runtime->is_initialized() || !m_runtime_ready) {
        return make_error_signal_internal(E_FAIL, "not_initialized", "GDK is not initialized. Call GDK.initialize() first.");
    }

    Ref<GDKPendingSignal> pending_signal = runtime->make_pending_signal();
    auto *context = new TextEntryAsyncContext(runtime, pending_signal);
    context->bind_cancel_handler();

    const String title_text = p_title_text.strip_edges();
    const String description_text = p_description_text.strip_edges();
    const CharString title_text_utf8 = title_text.utf8();
    const CharString description_text_utf8 = description_text.utf8();
    const CharString default_text_utf8 = p_default_text.utf8();

    HRESULT hr = XGameUiShowTextEntryAsync(
            context->get_async_block(),
            title_text.is_empty() ? nullptr : title_text_utf8.get_data(),
            description_text.is_empty() ? nullptr : description_text_utf8.get_data(),
            p_default_text.is_empty() ? nullptr : default_text_utf8.get_data(),
            _parse_text_entry_input_scope(p_input_scope),
            static_cast<uint32_t>(p_max_text_length));
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;
        pending_signal->complete_deferred(GDKResult::hresult_error(
                hr,
                "Failed to start the text entry UI flow.",
                "game_ui_text_entry_start_failed"));
    }

    return pending_signal->get_completed_signal();
}

GDKRuntime *GDKGameUI::get_runtime_internal() const {
    if (m_owner == nullptr) {
        return nullptr;
    }

    return m_owner->get_runtime();
}

GDKUsers *GDKGameUI::get_users_internal() const {
    if (m_owner == nullptr) {
        return nullptr;
    }

    Ref<GDKUsers> users = m_owner->get_users();
    return users.ptr();
}

Signal GDKGameUI::make_completed_signal_internal(const Ref<GDKResult> &p_result) const {
    GDKRuntime *runtime = get_runtime_internal();
    Ref<GDKPendingSignal> pending_signal = runtime != nullptr ? runtime->make_pending_signal() : Ref<GDKPendingSignal>();
    if (pending_signal.is_null()) {
        pending_signal.instantiate();
    }
    pending_signal->complete_deferred(p_result);
    return pending_signal->get_completed_signal();
}

Signal GDKGameUI::make_error_signal_internal(
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message) const {
    GDKRuntime *runtime = get_runtime_internal();
    if (runtime != nullptr) {
        return runtime->make_error_signal(p_hresult, p_code, p_message);
    }

    Ref<GDKPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(GDKResult::error_result(p_hresult, p_code, p_message));
    return pending_signal->get_completed_signal();
}

} // namespace godot
