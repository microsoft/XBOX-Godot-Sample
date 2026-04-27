#include "gdk_achievements.h"
#include "gdk_core.h"
#include "gdk_user.h"

#include <godot_cpp/variant/utility_functions.hpp>

#include <XAsync.h>

#ifndef HTTP_E_STATUS_NOT_MODIFIED
#define HTTP_E_STATUS_NOT_MODIFIED ((HRESULT)0x80190130L)
#endif

namespace godot {

// ── GDKAchievements ─────────────────────────────────────────────

GDKAchievements *GDKAchievements::singleton = nullptr;

GDKAchievements *GDKAchievements::get_singleton() {
    return singleton;
}

GDKAchievements::GDKAchievements() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

GDKAchievements::~GDKAchievements() {
    shutdown();
    singleton = nullptr;
}

void GDKAchievements::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "scid"), &GDKAchievements::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &GDKAchievements::shutdown);
    ClassDB::bind_method(D_METHOD("unlock", "achievement_id"), &GDKAchievements::unlock);
    ClassDB::bind_method(D_METHOD("update_progress", "achievement_id", "percent"), &GDKAchievements::update_progress);
    ClassDB::bind_method(D_METHOD("check_achievement", "achievement_id"), &GDKAchievements::check_achievement);
    ClassDB::bind_method(D_METHOD("is_initialized"), &GDKAchievements::is_initialized);

    ADD_SIGNAL(MethodInfo("achievement_unlocked", PropertyInfo(Variant::STRING, "achievement_id")));
    ADD_SIGNAL(MethodInfo("achievement_update_failed",
        PropertyInfo(Variant::STRING, "achievement_id"),
        PropertyInfo(Variant::STRING, "error")));
    ADD_SIGNAL(MethodInfo("achievement_checked",
        PropertyInfo(Variant::STRING, "achievement_id"),
        PropertyInfo(Variant::BOOL, "is_unlocked")));
}

// ── Xbox Live context management ────────────────────────────────

bool GDKAchievements::_ensure_context() {
    GDKUserManager *user_mgr = GDKUserManager::get_singleton();
    if (!user_mgr || !user_mgr->is_signed_in()) {
        _destroy_context();
        return false;
    }

    if (m_xbl_context) return true;

    Ref<GDKUserInfo> user = user_mgr->get_current_user();
    if (!user.is_valid() || !user->get_handle()) return false;

    HRESULT hr = XblContextCreateHandle(user->get_handle(), &m_xbl_context);
    if (FAILED(hr)) {
        UtilityFunctions::push_warning("GDK: Failed to create Xbox Live context");
        m_xbl_context = nullptr;
        return false;
    }

    UtilityFunctions::print("GDK: Xbox Live context created");
    return true;
}

void GDKAchievements::_destroy_context() {
    if (m_xbl_context) {
        XblContextCloseHandle(m_xbl_context);
        m_xbl_context = nullptr;
    }
}

// ── Public API ──────────────────────────────────────────────────

Error GDKAchievements::initialize(const String &scid) {
    if (m_initialized) return OK;

    GDKCore *core = GDKCore::get_singleton();
    ERR_FAIL_COND_V_MSG(!core || !core->is_initialized(), ERR_UNCONFIGURED,
        "GDK not initialized. Call GDK.initialize() first.");

    XblInitArgs args = {};
    CharString scid_utf8 = scid.utf8();
    args.scid = scid_utf8.get_data();

    HRESULT hr = XblInitialize(&args);
    if (FAILED(hr)) {
        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        UtilityFunctions::push_error("GDK: XblInitialize failed: ", hex_buf);
        return ERR_CANT_CREATE;
    }

    m_scid = scid;
    m_initialized = true;
    UtilityFunctions::print("GDK: Xbox Live services initialized");
    return OK;
}

void GDKAchievements::shutdown() {
    _destroy_context();
    if (m_initialized) {
        XAsyncBlock async = {};
        if (SUCCEEDED(XblCleanupAsync(&async))) {
            XAsyncGetStatus(&async, true);
        }
        m_initialized = false;
    }
}

bool GDKAchievements::is_initialized() const {
    return m_initialized;
}

// ── Async achievement update ────────────────────────────────────

struct AchievementContext {
    GDKAchievements *manager;
    String achievement_id;
    XAsyncBlock async;
};

static void CALLBACK achievement_update_complete(XAsyncBlock *async) {
    auto *ctx = static_cast<AchievementContext *>(async->context);
    HRESULT hr = XAsyncGetStatus(async, false);
    ctx->manager->_on_achievement_complete(ctx->achievement_id, hr);
    delete ctx;
}

void GDKAchievements::unlock(const String &achievement_id) {
    update_progress(achievement_id, 100);
}

void GDKAchievements::update_progress(const String &achievement_id, uint32_t percent) {
    ERR_FAIL_COND_MSG(!m_initialized,
        "Xbox Live not initialized. Call GDKAchievements.initialize() first.");
    ERR_FAIL_COND_MSG(percent < 1 || percent > 100,
        "Progress must be between 1 and 100.");

    if (!_ensure_context()) {
        String msg = "No Xbox Live context — is a user signed in?";
        UtilityFunctions::push_error("GDK: ", msg);
        call_deferred("emit_signal", "achievement_update_failed", achievement_id, msg);
        return;
    }

    GDKCore *core = GDKCore::get_singleton();
    GDKUserManager *user_mgr = GDKUserManager::get_singleton();
    uint64_t xuid = user_mgr->get_current_user()->get_xuid();

    auto *ctx = new AchievementContext();
    ctx->manager = this;
    ctx->achievement_id = achievement_id;
    ctx->async = {};
    ctx->async.queue = core->get_task_queue();
    ctx->async.context = ctx;
    ctx->async.callback = achievement_update_complete;

    CharString id_utf8 = achievement_id.utf8();

    HRESULT hr = XblAchievementsUpdateAchievementAsync(
        m_xbl_context,
        xuid,
        id_utf8.get_data(),
        percent,
        &ctx->async
    );

    if (FAILED(hr)) {
        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        String msg = String("Failed to start achievement update: ") + hex_buf;
        UtilityFunctions::push_error("GDK: ", msg);
        call_deferred("emit_signal", "achievement_update_failed", achievement_id, msg);
        delete ctx;
    }
}

void GDKAchievements::_on_achievement_complete(const String &achievement_id, HRESULT hr) {
    // HTTP_E_STATUS_NOT_MODIFIED means progress didn't increase (already unlocked)
    if (SUCCEEDED(hr) || hr == HTTP_E_STATUS_NOT_MODIFIED) {
        UtilityFunctions::print("GDK: Achievement updated: ", achievement_id);
        call_deferred("emit_signal", "achievement_unlocked", achievement_id);
    } else {
        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        String msg = String("Achievement update failed: ") + hex_buf;
        UtilityFunctions::push_error("GDK: ", msg);
        call_deferred("emit_signal", "achievement_update_failed", achievement_id, msg);
    }
}

// ── Async achievement status check ──────────────────────────────

struct AchievementCheckContext {
    GDKAchievements *manager;
    String achievement_id;
    XAsyncBlock async;
};

static void CALLBACK achievement_check_complete(XAsyncBlock *async) {
    auto *ctx = static_cast<AchievementCheckContext *>(async->context);
    ctx->manager->_on_achievement_checked(ctx->achievement_id, async);
    delete ctx;
}

void GDKAchievements::check_achievement(const String &achievement_id) {
    ERR_FAIL_COND_MSG(!m_initialized,
        "Xbox Live not initialized. Call GDKAchievements.initialize() first.");

    if (!_ensure_context()) {
        UtilityFunctions::push_warning("GDK: No Xbox Live context for achievement check");
        return;
    }

    GDKCore *core = GDKCore::get_singleton();
    GDKUserManager *user_mgr = GDKUserManager::get_singleton();
    uint64_t xuid = user_mgr->get_current_user()->get_xuid();

    auto *ctx = new AchievementCheckContext();
    ctx->manager = this;
    ctx->achievement_id = achievement_id;
    ctx->async = {};
    ctx->async.queue = core->get_task_queue();
    ctx->async.context = ctx;
    ctx->async.callback = achievement_check_complete;

    CharString id_utf8 = achievement_id.utf8();
    CharString scid_utf8 = m_scid.utf8();

    HRESULT hr = XblAchievementsGetAchievementAsync(
        m_xbl_context,
        xuid,
        scid_utf8.get_data(),
        id_utf8.get_data(),
        &ctx->async
    );

    if (FAILED(hr)) {
        UtilityFunctions::push_warning("GDK: Failed to start achievement check");
        delete ctx;
    }
}

void GDKAchievements::_on_achievement_checked(const String &achievement_id, XAsyncBlock *async) {
    XblAchievementsResultHandle result_handle = nullptr;
    HRESULT hr = XblAchievementsGetAchievementResult(async, &result_handle);
    if (FAILED(hr) || !result_handle) {
        UtilityFunctions::push_warning("GDK: Failed to get achievement check result");
        call_deferred("emit_signal", "achievement_checked", achievement_id, false);
        return;
    }

    const XblAchievement *achievements = nullptr;
    size_t count = 0;
    hr = XblAchievementsResultGetAchievements(result_handle, &achievements, &count);

    bool is_unlocked = false;
    if (SUCCEEDED(hr) && count > 0) {
        is_unlocked = (achievements[0].progressState == XblAchievementProgressState::Achieved);
    }

    XblAchievementsResultCloseHandle(result_handle);

    UtilityFunctions::print("GDK: Achievement '", achievement_id, "' unlocked = ", is_unlocked);
    call_deferred("emit_signal", "achievement_checked", achievement_id, is_unlocked);
}

} // namespace godot
