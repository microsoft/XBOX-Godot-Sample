#include "playfab_leaderboards.h"

#include <algorithm>
#include <string>
#include <vector>

#include "playfab_runtime.h"

#include <playfab/services/PFLeaderboards.h>

#include "playfab.h"
#include "playfab_pending_signal.h"
#include "playfab_result.h"
#include "playfab_signal_xasync_context.h"
#include "playfab_user.h"
#include "playfab_users.h"

namespace godot {

namespace {

Signal make_leaderboards_error_signal(
        PlayFabRuntime *p_runtime,
        HRESULT p_hresult,
        const String &p_code,
        const String &p_message,
        const Variant &p_data = Variant()) {
    if (p_runtime != nullptr) {
        return p_runtime->make_error_signal(p_hresult, p_code, p_message, p_data);
    }

    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    pending_signal->complete_deferred(PlayFabResult::error_result(p_hresult, p_code, p_message, p_data));
    return pending_signal->get_completed_signal();
}

bool validate_playfab_user(const Ref<PlayFabUser> &p_user, String *r_error_message) {
    if (!p_user.is_valid()) {
        if (r_error_message != nullptr) {
            *r_error_message = "Leaderboard operations require a valid PlayFabUser.";
        }
        return false;
    }

    const Dictionary entity_key = p_user->get_entity_key();
    const String entity_id = String(entity_key.get("id", String()));
    const String entity_type = String(entity_key.get("type", String()));
    if (entity_id.is_empty() || entity_type.is_empty() || p_user->get_entity_handle() == nullptr) {
        if (r_error_message != nullptr) {
            *r_error_message = "Leaderboard operations require a signed-in PlayFabUser with a valid entity key.";
        }
        return false;
    }

    return true;
}

bool validate_friend_sources(int64_t p_friend_sources, String *r_error_message) {
    if (p_friend_sources >= 0 && (p_friend_sources <= 0x0f || p_friend_sources == 0x10)) {
        return true;
    }

    if (r_error_message != nullptr) {
        *r_error_message =
                "friend_sources must contain only FRIEND_SOURCE_STEAM, FRIEND_SOURCE_FACEBOOK, "
                "FRIEND_SOURCE_XBOX, and FRIEND_SOURCE_PSN (0x00-0x0f), or FRIEND_SOURCE_ALL "
                "(0x10) by itself.";
    }
    return false;
}

bool friend_sources_require_xbox(PFExternalFriendSources p_friend_sources) {
    const uint32_t sources = static_cast<uint32_t>(p_friend_sources);
    return p_friend_sources == PFExternalFriendSources::All ||
            (sources & static_cast<uint32_t>(PFExternalFriendSources::Xbox)) != 0;
}

static_assert(
        static_cast<int64_t>(PlayFabLeaderboards::FRIEND_SOURCE_NONE) ==
        static_cast<int64_t>(PFExternalFriendSources::None));
static_assert(
        static_cast<int64_t>(PlayFabLeaderboards::FRIEND_SOURCE_STEAM) ==
        static_cast<int64_t>(PFExternalFriendSources::Steam));
static_assert(
        static_cast<int64_t>(PlayFabLeaderboards::FRIEND_SOURCE_FACEBOOK) ==
        static_cast<int64_t>(PFExternalFriendSources::Facebook));
static_assert(
        static_cast<int64_t>(PlayFabLeaderboards::FRIEND_SOURCE_XBOX) ==
        static_cast<int64_t>(PFExternalFriendSources::Xbox));
static_assert(
        static_cast<int64_t>(PlayFabLeaderboards::FRIEND_SOURCE_PSN) ==
        static_cast<int64_t>(PFExternalFriendSources::Psn));
static_assert(
        static_cast<int64_t>(PlayFabLeaderboards::FRIEND_SOURCE_ALL) ==
        static_cast<int64_t>(PFExternalFriendSources::All));

Dictionary make_leaderboard_response(const PFLeaderboardsGetEntityLeaderboardResponse *p_response) {
    Dictionary response;
    if (p_response == nullptr) {
        return response;
    }

    response["entry_count"] = static_cast<int64_t>(p_response->entryCount);
    response["version"] = static_cast<int64_t>(p_response->version);
    if (p_response->nextReset != nullptr) {
        response["next_reset"] = static_cast<int64_t>(*p_response->nextReset);
    }

    Array rankings;
    for (uint32_t i = 0; i < p_response->rankingsCount; ++i) {
        const PFLeaderboardsEntityLeaderboardEntry *entry =
                p_response->rankings != nullptr ? p_response->rankings[i] : nullptr;
        if (entry == nullptr) {
            continue;
        }

        Dictionary row;
        row["display_name"] = entry->displayName != nullptr ? String::utf8(entry->displayName) : String();
        row["rank"] = static_cast<int64_t>(entry->rank);
        row["last_updated"] = static_cast<int64_t>(entry->lastUpdated);
        row["metadata"] = entry->metadata != nullptr ? String::utf8(entry->metadata) : String();

        Dictionary entity;
        if (entry->entity != nullptr) {
            entity["id"] = entry->entity->id != nullptr ? String::utf8(entry->entity->id) : String();
            entity["type"] = entry->entity->type != nullptr ? String::utf8(entry->entity->type) : String();
        } else {
            entity["id"] = String();
            entity["type"] = String();
        }
        row["entity"] = entity;

        PackedStringArray scores;
        for (uint32_t score_index = 0; score_index < entry->scoresCount; ++score_index) {
            scores.push_back(entry->scores != nullptr && entry->scores[score_index] != nullptr
                                     ? String::utf8(entry->scores[score_index])
                                     : String());
        }
        row["scores"] = scores;

        rankings.push_back(row);
    }

    response["rankings"] = rankings;
    return response;
}

using LeaderboardResultSizeFn = HRESULT (*)(XAsyncBlock *, size_t *);
using LeaderboardResultFn = HRESULT (*)(XAsyncBlock *, size_t, void *, PFLeaderboardsGetEntityLeaderboardResponse **, size_t *);

class LeaderboardQueryContext final : public PlayFabSignalXAsyncContext {
public:
    enum class Mode {
        Global,
        Around,
        Friends,
    };

private:
    Mode m_mode = Mode::Global;
    Ref<PlayFabUser> m_user;
    std::string m_leaderboard_name_utf8;
    std::string m_entity_id_utf8;
    std::string m_entity_type_utf8;
    std::string m_xbox_token_utf8;
    PFEntityKey m_entity_key = {};
    uint32_t m_page_size = 10;
    uint32_t m_start_position = 1;
    uint32_t m_version = 0;
    uint32_t m_max_surrounding_entries = 10;
    bool m_use_start_position = false;
    bool m_use_version = false;
    PFExternalFriendSources m_external_friend_sources = PFExternalFriendSources::None;

    PFLeaderboardsGetEntityLeaderboardRequest m_global_request = {};
    PFLeaderboardsGetLeaderboardAroundEntityRequest m_around_request = {};
    PFLeaderboardsGetFriendLeaderboardForEntityRequest m_friend_request = {};

    LeaderboardResultSizeFn get_result_size_fn() const {
        switch (m_mode) {
            case Mode::Global:
                return PFLeaderboardsGetLeaderboardGetResultSize;
            case Mode::Around:
                return PFLeaderboardsGetLeaderboardAroundEntityGetResultSize;
            case Mode::Friends:
                return PFLeaderboardsGetFriendLeaderboardForEntityGetResultSize;
        }

        return PFLeaderboardsGetLeaderboardGetResultSize;
    }

    LeaderboardResultFn get_result_fn() const {
        switch (m_mode) {
            case Mode::Global:
                return PFLeaderboardsGetLeaderboardGetResult;
            case Mode::Around:
                return PFLeaderboardsGetLeaderboardAroundEntityGetResult;
            case Mode::Friends:
                return PFLeaderboardsGetFriendLeaderboardForEntityGetResult;
        }

        return PFLeaderboardsGetLeaderboardGetResult;
    }

public:
    LeaderboardQueryContext(
            Mode p_mode,
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_mode(p_mode),
            m_user(p_user),
            m_leaderboard_name_utf8(p_leaderboard_name.utf8().get_data()) {
        const Dictionary entity_key = p_user->get_entity_key();
        m_entity_id_utf8 = String(entity_key.get("id", String())).utf8().get_data();
        m_entity_type_utf8 = String(entity_key.get("type", String())).utf8().get_data();
        m_entity_key.id = m_entity_id_utf8.c_str();
        m_entity_key.type = m_entity_type_utf8.c_str();
    }

    void configure_global(uint32_t p_page_size, uint32_t p_start_position, bool p_use_start_position, int64_t p_version) {
        m_page_size = p_page_size;
        m_start_position = p_start_position;
        m_use_start_position = p_use_start_position;
        m_use_version = p_version >= 0;
        m_version = m_use_version ? static_cast<uint32_t>(p_version) : 0;

        m_global_request.leaderboardName = m_leaderboard_name_utf8.c_str();
        m_global_request.pageSize = m_page_size;
        m_global_request.startingPosition = m_use_start_position ? &m_start_position : nullptr;
        m_global_request.version = m_use_version ? &m_version : nullptr;
    }

    void configure_around(uint32_t p_max_surrounding_entries, int64_t p_version) {
        m_max_surrounding_entries = p_max_surrounding_entries;
        m_use_version = p_version >= 0;
        m_version = m_use_version ? static_cast<uint32_t>(p_version) : 0;

        m_around_request.entity = &m_entity_key;
        m_around_request.leaderboardName = m_leaderboard_name_utf8.c_str();
        m_around_request.maxSurroundingEntries = m_max_surrounding_entries;
        m_around_request.version = m_use_version ? &m_version : nullptr;
    }

    void configure_friends(
            const String &p_xbox_token,
            int64_t p_version,
            PFExternalFriendSources p_friend_sources) {
        m_xbox_token_utf8 = p_xbox_token.utf8().get_data();
        m_use_version = p_version >= 0;
        m_version = m_use_version ? static_cast<uint32_t>(p_version) : 0;
        m_external_friend_sources = p_friend_sources;

        m_friend_request.entity = &m_entity_key;
        m_friend_request.externalFriendSources =
                p_friend_sources != PFExternalFriendSources::None ? &m_external_friend_sources : nullptr;
        m_friend_request.leaderboardName = m_leaderboard_name_utf8.c_str();
        m_friend_request.version = m_use_version ? &m_version : nullptr;
        m_friend_request.xboxToken =
                friend_sources_require_xbox(p_friend_sources) && !m_xbox_token_utf8.empty()
                ? m_xbox_token_utf8.c_str()
                : nullptr;
    }

#ifdef GODOT_PLAYFAB_TEST_HOOKS
    const PFLeaderboardsGetFriendLeaderboardForEntityRequest &get_friend_request() const {
        return m_friend_request;
    }
#endif

    HRESULT start() {
        switch (m_mode) {
            case Mode::Global:
                return PFLeaderboardsGetLeaderboardAsync(
                        m_user->get_entity_handle(),
                        &m_global_request,
                        get_async_block());
            case Mode::Around:
                return PFLeaderboardsGetLeaderboardAroundEntityAsync(
                        m_user->get_entity_handle(),
                        &m_around_request,
                        get_async_block());
            case Mode::Friends:
                return PFLeaderboardsGetFriendLeaderboardForEntityAsync(
                        m_user->get_entity_handle(),
                        &m_friend_request,
                        get_async_block());
        }

        return E_FAIL;
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled("Leaderboard request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled("Leaderboard request cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "Failed to query the PlayFab leaderboard.", "leaderboard_query_failed");
            get_pending_signal()->complete(result);
            return;
        }

        size_t buffer_size = 0;
        HRESULT size_hr = get_result_size_fn()(p_async_block, &buffer_size);
        if (FAILED(size_hr)) {
            result = PlayFabResult::hresult_error(size_hr, "Failed to get the leaderboard result size.", "leaderboard_result_size_failed");
            get_pending_signal()->complete(result);
            return;
        }

        std::vector<char> buffer(buffer_size);
        PFLeaderboardsGetEntityLeaderboardResponse *response = nullptr;
        HRESULT result_hr = get_result_fn()(
                p_async_block,
                buffer.size(),
                buffer.data(),
                &response,
                nullptr);
        if (FAILED(result_hr)) {
            result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve the leaderboard result payload.", "leaderboard_result_failed");
            get_pending_signal()->complete(result);
            return;
        }

        get_pending_signal()->complete(PlayFabResult::ok_result(make_leaderboard_response(response)));
    }
};

class UpdateLeaderboardEntriesContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    std::string m_leaderboard_name_utf8;
    std::string m_entity_id_utf8;
    std::string m_metadata_utf8;
    std::vector<std::string> m_score_strings;
    std::vector<const char *> m_score_ptrs;
    PFLeaderboardsLeaderboardEntryUpdate m_entry = {};
    const PFLeaderboardsLeaderboardEntryUpdate *m_entry_ptrs[1] = { &m_entry };
    PFLeaderboardsUpdateLeaderboardEntriesRequest m_request = {};

public:
    UpdateLeaderboardEntriesContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            const String &p_leaderboard_name,
            const std::vector<std::string> &p_scores,
            const String &p_metadata) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_leaderboard_name_utf8(p_leaderboard_name.utf8().get_data()),
            m_metadata_utf8(p_metadata.utf8().get_data()),
            m_score_strings(p_scores) {
        const Dictionary entity_key = p_user->get_entity_key();
        m_entity_id_utf8 = String(entity_key.get("id", String())).utf8().get_data();
        m_score_ptrs.reserve(m_score_strings.size());
        for (const std::string &score : m_score_strings) {
            m_score_ptrs.push_back(score.c_str());
        }

        m_entry.entityId = m_entity_id_utf8.c_str();
        m_entry.metadata = m_metadata_utf8.empty() ? nullptr : m_metadata_utf8.c_str();
        m_entry.scores = m_score_ptrs.empty() ? nullptr : m_score_ptrs.data();
        m_entry.scoresCount = static_cast<uint32_t>(m_score_ptrs.size());

        m_request.entries = m_entry_ptrs;
        m_request.entriesCount = 1;
        m_request.leaderboardName = m_leaderboard_name_utf8.c_str();
    }

    HRESULT start() {
        return PFLeaderboardsUpdateLeaderboardEntriesAsync(
                m_user->get_entity_handle(),
                &m_request,
                get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override {
        Ref<PlayFabResult> result;

        if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
            result = PlayFabResult::cancelled("Leaderboard update cancelled.");
            get_pending_signal()->complete(result);
            return;
        }

        HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
        if (status_hr == E_ABORT) {
            result = PlayFabResult::cancelled("Leaderboard update cancelled.");
            get_pending_signal()->complete(result);
            return;
        }
        if (FAILED(status_hr)) {
            result = PlayFabResult::hresult_error(status_hr, "Failed to update the PlayFab leaderboard entry.", "leaderboard_update_failed");
            get_pending_signal()->complete(result);
            return;
        }

        Dictionary data;
        data["leaderboard_name"] = String::utf8(m_leaderboard_name_utf8.c_str());
        data["local_id"] = m_user->get_local_id();
        data["entity_key"] = m_user->get_entity_key();

        PackedStringArray scores;
        for (const std::string &score : m_score_strings) {
            scores.push_back(String::utf8(score.c_str()));
        }
        data["scores"] = scores;
        data["metadata"] = String::utf8(m_metadata_utf8.c_str());

        get_pending_signal()->complete(PlayFabResult::ok_result(data));
    }
};

class FriendLeaderboardTokenContext final : public PlayFabSignalXAsyncContext {
    Ref<PlayFabUser> m_user;
    std::string m_leaderboard_name_utf8;
    int64_t m_version = -1;
    PFExternalFriendSources m_friend_sources = PFExternalFriendSources::None;
    XUserHandle m_user_handle = nullptr;

public:
    FriendLeaderboardTokenContext(
            PlayFabRuntime *p_runtime,
            const Ref<PlayFabPendingSignal> &p_pending_signal,
            const Ref<PlayFabUser> &p_user,
            XUserHandle p_user_handle,
            const String &p_leaderboard_name,
            int64_t p_version,
            PFExternalFriendSources p_friend_sources) :
            PlayFabSignalXAsyncContext(p_runtime, p_pending_signal),
            m_user(p_user),
            m_leaderboard_name_utf8(p_leaderboard_name.utf8().get_data()),
            m_version(p_version),
            m_friend_sources(p_friend_sources),
            m_user_handle(p_user_handle) {}

    ~FriendLeaderboardTokenContext() override {
        if (m_user_handle != nullptr) {
            XUserCloseHandle(m_user_handle);
            m_user_handle = nullptr;
        }
    }

#ifdef GODOT_PLAYFAB_TEST_HOOKS
    PFExternalFriendSources get_friend_sources() const {
        return m_friend_sources;
    }
#endif

    HRESULT start() {
        static const char *PLAYFAB_TOKEN_URL = "https://playfabapi.com";
        static const char *PLAYFAB_TOKEN_METHOD = "POST";

        return XUserGetTokenAndSignatureAsync(
                m_user_handle,
                XUserGetTokenAndSignatureOptions::None,
                PLAYFAB_TOKEN_METHOD,
                PLAYFAB_TOKEN_URL,
                0,
                nullptr,
                0,
                nullptr,
                get_async_block());
    }

protected:
    void finalize(XAsyncBlock *p_async_block) override;
};

void start_friend_query(
        PlayFabRuntime *p_runtime,
        const Ref<PlayFabPendingSignal> &p_pending_signal,
        const Ref<PlayFabUser> &p_user,
        const String &p_leaderboard_name,
        const String &p_xbox_token,
        int64_t p_version,
        PFExternalFriendSources p_friend_sources) {
    auto *context = new LeaderboardQueryContext(
            LeaderboardQueryContext::Mode::Friends,
            p_runtime,
            p_pending_signal,
            p_user,
            p_leaderboard_name);
    context->configure_friends(p_xbox_token, p_version, p_friend_sources);
    context->bind_cancel_handler();

    HRESULT hr = context->start();
    if (FAILED(hr)) {
        p_pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the friend leaderboard request.", "friend_leaderboard_start_failed");
        p_pending_signal->complete_deferred(result);
    }
}

void FriendLeaderboardTokenContext::finalize(XAsyncBlock *p_async_block) {
    Ref<PlayFabResult> result;

    if (get_runtime()->is_shutting_down() || get_pending_signal()->was_cancel_requested()) {
        result = PlayFabResult::cancelled("Friend leaderboard token request cancelled.");
        get_pending_signal()->complete(result);
        return;
    }

    HRESULT status_hr = XAsyncGetStatus(p_async_block, false);
    if (status_hr == E_ABORT) {
        result = PlayFabResult::cancelled("Friend leaderboard token request cancelled.");
        get_pending_signal()->complete(result);
        return;
    }
    if (FAILED(status_hr)) {
        result = PlayFabResult::hresult_error(status_hr, "Failed to acquire an Xbox token for the friend leaderboard request.", "friend_leaderboard_token_failed");
        get_pending_signal()->complete(result);
        return;
    }

    size_t buffer_size = 0;
    HRESULT size_hr = XUserGetTokenAndSignatureResultSize(p_async_block, &buffer_size);
    if (FAILED(size_hr)) {
        result = PlayFabResult::hresult_error(size_hr, "Failed to get the Xbox token result size.", "friend_leaderboard_token_result_size_failed");
        get_pending_signal()->complete(result);
        return;
    }

    std::vector<uint8_t> buffer(buffer_size);
    XUserGetTokenAndSignatureData *token_data = nullptr;
    HRESULT result_hr = XUserGetTokenAndSignatureResult(
            p_async_block,
            buffer.size(),
            buffer.empty() ? nullptr : buffer.data(),
            &token_data,
            nullptr);
    if (FAILED(result_hr)) {
        result = PlayFabResult::hresult_error(result_hr, "Failed to retrieve the Xbox token payload.", "friend_leaderboard_token_result_failed");
        get_pending_signal()->complete(result);
        return;
    }

    const String xbox_token = token_data != nullptr && token_data->token != nullptr ? String::utf8(token_data->token) : String();
    if (xbox_token.is_empty()) {
        result = PlayFabResult::error_result(E_FAIL, "friend_leaderboard_token_empty", "Xbox token acquisition succeeded but returned an empty token.");
        get_pending_signal()->complete(result);
        return;
    }

    start_friend_query(
            get_runtime(),
            get_pending_signal(),
            m_user,
            String::utf8(m_leaderboard_name_utf8.c_str()),
            xbox_token,
            m_version,
            m_friend_sources);
}

} // namespace

void PlayFabLeaderboards::_bind_methods() {
    ClassDB::bind_method(
            D_METHOD("submit_score_async", "user", "leaderboard_name", "score", "additional_scores", "metadata"),
            &PlayFabLeaderboards::submit_score_async,
            DEFVAL(Array()),
            DEFVAL(String()));
    ClassDB::bind_method(
            D_METHOD("get_leaderboard_async", "user", "leaderboard_name", "start_position", "page_size", "version"),
            &PlayFabLeaderboards::get_leaderboard_async,
            DEFVAL(1),
            DEFVAL(10),
            DEFVAL(-1));
    ClassDB::bind_method(
            D_METHOD("get_leaderboard_around_user_async", "user", "leaderboard_name", "max_surrounding_entries", "version"),
            &PlayFabLeaderboards::get_leaderboard_around_user_async,
            DEFVAL(10),
            DEFVAL(-1));
    ClassDB::bind_method(
            D_METHOD("get_friend_leaderboard_async", "user", "leaderboard_name", "include_xbox_friends", "version"),
            &PlayFabLeaderboards::get_friend_leaderboard_async,
            DEFVAL(true),
            DEFVAL(-1));
    ClassDB::bind_method(
            D_METHOD("get_friend_leaderboard_with_sources_async", "user", "leaderboard_name", "friend_sources", "version"),
            &PlayFabLeaderboards::get_friend_leaderboard_with_sources_async,
            DEFVAL(-1));

#ifdef GODOT_PLAYFAB_TEST_HOOKS
    ClassDB::bind_method(
            D_METHOD("_test_friend_leaderboard_request", "friend_sources", "xbox_token", "version"),
            &PlayFabLeaderboards::_test_friend_leaderboard_request,
            DEFVAL(String()),
            DEFVAL(-1));
    ClassDB::bind_method(
            D_METHOD("_test_friend_token_context_sources", "friend_sources"),
            &PlayFabLeaderboards::_test_friend_token_context_sources);
#endif

    BIND_BITFIELD_FLAG(FRIEND_SOURCE_NONE);
    BIND_BITFIELD_FLAG(FRIEND_SOURCE_STEAM);
    BIND_BITFIELD_FLAG(FRIEND_SOURCE_FACEBOOK);
    BIND_BITFIELD_FLAG(FRIEND_SOURCE_XBOX);
    BIND_BITFIELD_FLAG(FRIEND_SOURCE_PSN);
    BIND_BITFIELD_FLAG(FRIEND_SOURCE_ALL);
}

void PlayFabLeaderboards::set_owner(PlayFab *p_owner) {
    m_owner = p_owner;
}

PlayFabRuntime *PlayFabLeaderboards::_get_runtime() const {
    return m_owner != nullptr ? m_owner->get_runtime() : nullptr;
}

Signal PlayFabLeaderboards::submit_score_async(
        const Ref<PlayFabUser> &p_user,
        const String &p_leaderboard_name,
        int64_t p_score,
        const Array &p_additional_scores,
        const String &p_metadata) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_leaderboards_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    String user_error;
    if (!validate_playfab_user(p_user, &user_error)) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_playfab_user", user_error);
    }

    const String leaderboard_name = p_leaderboard_name.strip_edges();
    if (leaderboard_name.is_empty()) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_leaderboard_name", "Leaderboard operations require a non-empty leaderboard_name.");
    }

    std::vector<std::string> scores;
    scores.push_back(std::to_string(p_score));
    for (int64_t i = 0; i < p_additional_scores.size(); ++i) {
        scores.push_back(String(p_additional_scores[i]).utf8().get_data());
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new UpdateLeaderboardEntriesContext(runtime, pending_signal, p_user, leaderboard_name, scores, p_metadata);
    context->bind_cancel_handler();

    HRESULT hr = context->start();
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the leaderboard update request.", "leaderboard_update_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabLeaderboards::get_leaderboard_async(
        const Ref<PlayFabUser> &p_user,
        const String &p_leaderboard_name,
        int64_t p_start_position,
        int64_t p_page_size,
        int64_t p_version) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_leaderboards_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    String user_error;
    if (!validate_playfab_user(p_user, &user_error)) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_playfab_user", user_error);
    }

    const String leaderboard_name = p_leaderboard_name.strip_edges();
    if (leaderboard_name.is_empty()) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_leaderboard_name", "Leaderboard operations require a non-empty leaderboard_name.");
    }

    const uint32_t page_size = static_cast<uint32_t>(CLAMP<int64_t>(p_page_size, 1, 100));
    const uint32_t start_position = static_cast<uint32_t>(MAX<int64_t>(p_start_position, 1));

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new LeaderboardQueryContext(
            LeaderboardQueryContext::Mode::Global,
            runtime,
            pending_signal,
            p_user,
            leaderboard_name);
    context->configure_global(page_size, start_position, true, p_version);
    context->bind_cancel_handler();

    HRESULT hr = context->start();
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the leaderboard query request.", "leaderboard_query_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabLeaderboards::get_leaderboard_around_user_async(
        const Ref<PlayFabUser> &p_user,
        const String &p_leaderboard_name,
        int64_t p_max_surrounding_entries,
        int64_t p_version) {
    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_leaderboards_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    String user_error;
    if (!validate_playfab_user(p_user, &user_error)) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_playfab_user", user_error);
    }

    const String leaderboard_name = p_leaderboard_name.strip_edges();
    if (leaderboard_name.is_empty()) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_leaderboard_name", "Leaderboard operations require a non-empty leaderboard_name.");
    }

    const uint32_t max_surrounding_entries = static_cast<uint32_t>(CLAMP<int64_t>(p_max_surrounding_entries, 1, 100));

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();

    auto *context = new LeaderboardQueryContext(
            LeaderboardQueryContext::Mode::Around,
            runtime,
            pending_signal,
            p_user,
            leaderboard_name);
    context->configure_around(max_surrounding_entries, p_version);
    context->bind_cancel_handler();

    HRESULT hr = context->start();
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the around-user leaderboard query request.", "leaderboard_around_user_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

Signal PlayFabLeaderboards::get_friend_leaderboard_async(
        const Ref<PlayFabUser> &p_user,
        const String &p_leaderboard_name,
        bool p_include_xbox_friends,
        int64_t p_version) {
    const FriendSources sources = _friend_sources_from_legacy_bool(p_include_xbox_friends);
    return get_friend_leaderboard_with_sources_async(
            p_user,
            p_leaderboard_name,
            BitField<FriendSources>(sources),
            p_version);
}

PlayFabLeaderboards::FriendSources PlayFabLeaderboards::_friend_sources_from_legacy_bool(
        bool p_include_xbox_friends) {
    return p_include_xbox_friends ? FRIEND_SOURCE_XBOX : FRIEND_SOURCE_NONE;
}

Signal PlayFabLeaderboards::get_friend_leaderboard_with_sources_async(
        const Ref<PlayFabUser> &p_user,
        const String &p_leaderboard_name,
        BitField<FriendSources> p_friend_sources,
        int64_t p_version) {
    const int64_t friend_sources_value = p_friend_sources;
    String sources_error;
    if (!validate_friend_sources(friend_sources_value, &sources_error)) {
        return make_leaderboards_error_signal(
                _get_runtime(),
                E_INVALIDARG,
                "invalid_friend_sources",
                sources_error);
    }

    PlayFabRuntime *runtime = _get_runtime();
    if (runtime == nullptr || !runtime->is_initialized()) {
        return make_leaderboards_error_signal(runtime, E_FAIL, "not_initialized", "PlayFab is not initialized. Call PlayFab.initialize() first.");
    }

    String user_error;
    if (!validate_playfab_user(p_user, &user_error)) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_playfab_user", user_error);
    }

    const String leaderboard_name = p_leaderboard_name.strip_edges();
    if (leaderboard_name.is_empty()) {
        return make_leaderboards_error_signal(runtime, E_INVALIDARG, "invalid_leaderboard_name", "Leaderboard operations require a non-empty leaderboard_name.");
    }

    Ref<PlayFabPendingSignal> pending_signal = runtime->make_pending_signal();
    const PFExternalFriendSources friend_sources =
            static_cast<PFExternalFriendSources>(friend_sources_value);

    if (!friend_sources_require_xbox(friend_sources)) {
        start_friend_query(
                runtime,
                pending_signal,
                p_user,
                leaderboard_name,
                String(),
                p_version,
                friend_sources);
        return pending_signal->get_completed_signal();
    }

    XUserLocalId local_id = {};
    local_id.value = p_user->get_local_id();
    if (local_id.value == 0) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(
                E_HANDLE,
                "friend_leaderboard_xuser_not_found",
                "Requested friend sources include Xbox. Sign in with PlayFab.users.sign_in_with_xuser_async(), or select sources without Xbox. ALL includes Xbox.");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserFindUserByLocalId(local_id, &user_handle);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to find an active XUserHandle for the Xbox friends leaderboard request.", "friend_leaderboard_xuser_not_found");
        pending_signal->complete_deferred(result);
        return pending_signal->get_completed_signal();
    }

    auto *token_context = new FriendLeaderboardTokenContext(
            runtime,
            pending_signal,
            p_user,
            user_handle,
            leaderboard_name,
            p_version,
            friend_sources);
    token_context->bind_cancel_handler();

    hr = token_context->start();
    if (FAILED(hr)) {
        pending_signal->clear_cancel_handler();
        delete token_context;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to start the Xbox token request for the friend leaderboard call.", "friend_leaderboard_token_start_failed");
        pending_signal->complete_deferred(result);
    }

    return pending_signal->get_completed_signal();
}

#ifdef GODOT_PLAYFAB_TEST_HOOKS
Ref<PlayFabResult> PlayFabLeaderboards::_test_friend_leaderboard_request(
        BitField<FriendSources> p_friend_sources,
        const String &p_xbox_token,
        int64_t p_version) const {
    const int64_t friend_sources_value = p_friend_sources;
    String sources_error;
    if (!validate_friend_sources(friend_sources_value, &sources_error)) {
        return PlayFabResult::error_result(
                E_INVALIDARG,
                "invalid_friend_sources",
                sources_error);
    }

    Ref<PlayFabUser> blank_user;
    blank_user.instantiate();
    LeaderboardQueryContext context(
            LeaderboardQueryContext::Mode::Friends,
            nullptr,
            Ref<PlayFabPendingSignal>(),
            blank_user,
            "test_friend_leaderboard");
    const PFExternalFriendSources friend_sources =
            static_cast<PFExternalFriendSources>(friend_sources_value);
    context.configure_friends(p_xbox_token, p_version, friend_sources);

    const PFLeaderboardsGetFriendLeaderboardForEntityRequest &request =
            context.get_friend_request();
    Dictionary snapshot;
    if (request.externalFriendSources != nullptr) {
        snapshot["external_friend_sources"] =
                static_cast<int64_t>(*request.externalFriendSources);
    } else {
        snapshot["external_friend_sources"] = Variant();
    }
    snapshot["requires_xbox_token"] =
            friend_sources_require_xbox(friend_sources);
    if (request.xboxToken != nullptr) {
        snapshot["xbox_token"] = String::utf8(request.xboxToken);
    } else {
        snapshot["xbox_token"] = Variant();
    }
    if (request.version != nullptr) {
        snapshot["version"] = static_cast<int64_t>(*request.version);
    } else {
        snapshot["version"] = Variant();
    }
    return PlayFabResult::ok_result(snapshot);
}

int64_t PlayFabLeaderboards::_test_friend_token_context_sources(
        BitField<FriendSources> p_friend_sources) const {
    Ref<PlayFabUser> blank_user;
    blank_user.instantiate();
    FriendLeaderboardTokenContext context(
            nullptr,
            Ref<PlayFabPendingSignal>(),
            blank_user,
            nullptr,
            "test_friend_leaderboard",
            -1,
            static_cast<PFExternalFriendSources>(
                    static_cast<int64_t>(p_friend_sources)));
    return static_cast<int64_t>(context.get_friend_sources());
}
#endif

} // namespace godot
