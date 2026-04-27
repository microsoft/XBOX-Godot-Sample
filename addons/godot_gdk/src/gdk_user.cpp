#include "gdk_user.h"
#include "gdk_core.h"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/image.hpp>
#include <godot_cpp/classes/image_texture.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

#include <XUser.h>
#include <XAsync.h>

#include <vector>

namespace godot {

// ── GDKUserInfo ─────────────────────────────────────────────────

GDKUserInfo::GDKUserInfo() {}

GDKUserInfo::~GDKUserInfo() {
    if (m_user_handle && m_handle_owned) {
        XUserCloseHandle(m_user_handle);
        m_user_handle = nullptr;
    }
}

void GDKUserInfo::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_gamertag"), &GDKUserInfo::get_gamertag);
    ClassDB::bind_method(D_METHOD("get_xuid"), &GDKUserInfo::get_xuid);
    ClassDB::bind_method(D_METHOD("is_valid"), &GDKUserInfo::is_valid);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "gamertag"), "", "get_gamertag");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "xuid"), "", "get_xuid");
}

void GDKUserInfo::set_from_handle(XUserHandle handle) {
    if (m_user_handle && m_handle_owned) {
        XUserCloseHandle(m_user_handle);
    }
    m_user_handle = handle;
    m_handle_owned = (handle != nullptr);
    m_gamertag = "";
    m_xuid = 0;

    if (!handle) return;

    // Retrieve gamertag
    char gamertag[XUserGamertagComponentClassicMaxBytes] = {};
    size_t gamertag_size = sizeof(gamertag);
    HRESULT hr = XUserGetGamertag(
        handle,
        XUserGamertagComponent::Classic,
        gamertag_size,
        gamertag,
        &gamertag_size
    );
    if (SUCCEEDED(hr)) {
        m_gamertag = String::utf8(gamertag);
    } else {
        UtilityFunctions::push_warning("GDK: Failed to retrieve gamertag");
    }

    // Retrieve XUID
    uint64_t xuid = 0;
    hr = XUserGetId(handle, &xuid);
    if (SUCCEEDED(hr)) {
        m_xuid = xuid;
    } else {
        UtilityFunctions::push_warning("GDK: Failed to retrieve XUID");
    }
}

String GDKUserInfo::get_gamertag() const { return m_gamertag; }
uint64_t GDKUserInfo::get_xuid() const { return m_xuid; }
bool GDKUserInfo::is_valid() const { return m_user_handle != nullptr; }

void GDKUserInfo::invalidate() {
    if (m_user_handle && m_handle_owned) {
        XUserCloseHandle(m_user_handle);
    }
    m_user_handle = nullptr;
    m_handle_owned = false;
    m_gamertag = "";
    m_xuid = 0;
}

// ── GDKUserManager ─────────────────────────────────────────────

GDKUserManager *GDKUserManager::singleton = nullptr;

GDKUserManager *GDKUserManager::get_singleton() {
    return singleton;
}

GDKUserManager::GDKUserManager() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

GDKUserManager::~GDKUserManager() {
    _unregister_change_event();
    _clear_user();
    singleton = nullptr;
}

void GDKUserManager::_bind_methods() {
    ClassDB::bind_method(D_METHOD("sign_in"), &GDKUserManager::sign_in);
    ClassDB::bind_method(D_METHOD("sign_in_silently"), &GDKUserManager::sign_in_silently);
    ClassDB::bind_method(D_METHOD("sign_out"), &GDKUserManager::sign_out);
    ClassDB::bind_method(D_METHOD("get_gamer_picture"), &GDKUserManager::get_gamer_picture);
    ClassDB::bind_method(D_METHOD("get_current_user"), &GDKUserManager::get_current_user);
    ClassDB::bind_method(D_METHOD("is_signed_in"), &GDKUserManager::is_signed_in);
    ClassDB::bind_method(D_METHOD("is_sign_in_pending"), &GDKUserManager::is_sign_in_pending);

    ADD_SIGNAL(MethodInfo("user_signed_in", PropertyInfo(Variant::OBJECT, "user")));
    ADD_SIGNAL(MethodInfo("sign_in_failed", PropertyInfo(Variant::STRING, "error")));
    ADD_SIGNAL(MethodInfo("user_signed_out"));
    ADD_SIGNAL(MethodInfo("gamer_picture_loaded", PropertyInfo(Variant::OBJECT, "texture")));
}

// ── Internal state management ───────────────────────────────────

void GDKUserManager::_store_user(Ref<GDKUserInfo> user) {
    _clear_user();
    m_current_user = user;
    _register_change_event();
}

void GDKUserManager::_clear_user() {
    _unregister_change_event();
    if (m_current_user.is_valid()) {
        m_current_user->invalidate();
        m_current_user.unref();
    }
}

// ── XUserChangeEvent ────────────────────────────────────────────

static void CALLBACK user_change_callback(
    void *context,
    XUserLocalId user_local_id,
    XUserChangeEvent event
) {
    auto *mgr = static_cast<GDKUserManager *>(context);
    mgr->_on_user_change(user_local_id, event);
}

void GDKUserManager::_register_change_event() {
    if (m_change_registered || !m_current_user.is_valid()) return;

    HRESULT hr = XUserRegisterForChangeEvent(
        nullptr, // default task queue
        this,
        user_change_callback,
        &m_change_token
    );

    if (SUCCEEDED(hr)) {
        m_change_registered = true;
    }
}

void GDKUserManager::_unregister_change_event() {
    if (!m_change_registered) return;
    XUserUnregisterForChangeEvent(m_change_token, false);
    m_change_registered = false;
}

void GDKUserManager::_on_user_change(XUserLocalId user_local_id, XUserChangeEvent event) {
    if (!m_current_user.is_valid()) return;

    switch (event) {
        case XUserChangeEvent::SignedOut:
            UtilityFunctions::print("GDK: User signed out externally");
            _clear_user();
            call_deferred("emit_signal", "user_signed_out");
            break;
        case XUserChangeEvent::SigningOut:
            UtilityFunctions::print("GDK: User signing out...");
            break;
        case XUserChangeEvent::GamerPicture:
            // Re-fetch the gamer picture
            if (m_current_user.is_valid() && m_current_user->get_handle()) {
                get_gamer_picture();
            }
            break;
        case XUserChangeEvent::Gamertag:
            // Refresh user data
            if (m_current_user.is_valid() && m_current_user->get_handle()) {
                m_current_user->set_from_handle(m_current_user->get_handle());
            }
            break;
        default:
            break;
    }
}

// ── Async sign-in ───────────────────────────────────────────────

struct SignInContext {
    GDKUserManager *manager;
    bool was_silent;
    XAsyncBlock async;
};

static void CALLBACK sign_in_complete(XAsyncBlock *async) {
    auto *ctx = static_cast<SignInContext *>(async->context);

    XUserHandle user_handle = nullptr;
    HRESULT hr = XUserAddResult(async, &user_handle);

    ctx->manager->_on_sign_in_complete(user_handle, hr, ctx->was_silent);
    delete ctx;
}

void GDKUserManager::_on_sign_in_complete(XUserHandle handle, HRESULT hr, bool was_silent) {
    if (SUCCEEDED(hr) && handle) {
        Ref<GDKUserInfo> user;
        user.instantiate();
        user->set_from_handle(handle);

        m_sign_in_pending = false;
        _store_user(user);

        // Log without PII — only indicate success
        UtilityFunctions::print("GDK: Xbox user signed in successfully");
        call_deferred("emit_signal", "user_signed_in", user);
    } else {
        if (handle) {
            XUserCloseHandle(handle);
        }

        // If silent attempt failed asynchronously, fall back to UI
        if (was_silent) {
            UtilityFunctions::print("GDK: Silent sign-in failed, falling back to UI");
            m_sign_in_pending = false;
            m_silent_attempt = false;
            sign_in();
            return;
        }

        m_sign_in_pending = false;

        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        String msg = String("Sign-in failed: ") + hex_buf;
        UtilityFunctions::push_error("GDK: ", msg);
        call_deferred("emit_signal", "sign_in_failed", msg);
    }
}

// ── Public API ──────────────────────────────────────────────────

void GDKUserManager::sign_in() {
    GDKCore *core = GDKCore::get_singleton();
    ERR_FAIL_COND_MSG(!core || !core->is_initialized(),
        "GDK not initialized. Call GDK.initialize() first.");

    if (m_sign_in_pending) {
        UtilityFunctions::push_warning("GDK: Sign-in already in progress");
        return;
    }

    m_sign_in_pending = true;
    m_silent_attempt = false;

    auto *ctx = new SignInContext();
    ctx->manager = this;
    ctx->was_silent = false;
    ctx->async = {};
    ctx->async.queue = core->get_task_queue();
    ctx->async.context = ctx;
    ctx->async.callback = sign_in_complete;

    HRESULT hr = XUserAddAsync(
        XUserAddOptions::AddDefaultUserAllowingUI,
        &ctx->async
    );

    if (FAILED(hr)) {
        m_sign_in_pending = false;
        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        String msg = String("Failed to start sign-in: ") + hex_buf;
        UtilityFunctions::push_error("GDK: ", msg);
        emit_signal("sign_in_failed", msg);
        delete ctx;
    }
}

void GDKUserManager::sign_in_silently() {
    GDKCore *core = GDKCore::get_singleton();
    ERR_FAIL_COND_MSG(!core || !core->is_initialized(),
        "GDK not initialized. Call GDK.initialize() first.");

    if (m_sign_in_pending) {
        UtilityFunctions::push_warning("GDK: Sign-in already in progress");
        return;
    }

    m_sign_in_pending = true;
    m_silent_attempt = true;

    auto *ctx = new SignInContext();
    ctx->manager = this;
    ctx->was_silent = true;
    ctx->async = {};
    ctx->async.queue = core->get_task_queue();
    ctx->async.context = ctx;
    ctx->async.callback = sign_in_complete;

    HRESULT hr = XUserAddAsync(
        XUserAddOptions::AddDefaultUserSilently,
        &ctx->async
    );

    if (FAILED(hr)) {
        // Immediate failure on silent — fall back to UI
        m_sign_in_pending = false;
        m_silent_attempt = false;
        UtilityFunctions::print("GDK: Silent sign-in unavailable, falling back to UI");
        delete ctx;
        sign_in();
    }
}

// ── Async gamer picture ─────────────────────────────────────────

struct GamerPictureContext {
    GDKUserManager *manager;
    XAsyncBlock async;
};

static void CALLBACK gamer_picture_complete(XAsyncBlock *async) {
    auto *ctx = static_cast<GamerPictureContext *>(async->context);
    ctx->manager->_on_gamer_picture_complete(async);
    delete ctx;
}

void GDKUserManager::get_gamer_picture() {
    GDKCore *core = GDKCore::get_singleton();
    ERR_FAIL_COND_MSG(!core || !core->is_initialized(),
        "GDK not initialized. Call GDK.initialize() first.");
    ERR_FAIL_COND_MSG(!is_signed_in(), "No user signed in.");

    if (m_picture_pending) {
        UtilityFunctions::push_warning("GDK: Gamer picture fetch already in progress");
        return;
    }

    m_picture_pending = true;

    auto *ctx = new GamerPictureContext();
    ctx->manager = this;
    ctx->async = {};
    ctx->async.queue = core->get_task_queue();
    ctx->async.context = ctx;
    ctx->async.callback = gamer_picture_complete;

    HRESULT hr = XUserGetGamerPictureAsync(
        m_current_user->get_handle(),
        XUserGamerPictureSize::Medium, // 208x208
        &ctx->async
    );

    if (FAILED(hr)) {
        m_picture_pending = false;
        UtilityFunctions::push_warning("GDK: Failed to start gamer picture fetch");
        delete ctx;
    }
}

void GDKUserManager::_on_gamer_picture_complete(XAsyncBlock *async) {
    m_picture_pending = false;

    size_t buffer_size = 0;
    HRESULT hr = XUserGetGamerPictureResultSize(async, &buffer_size);
    if (FAILED(hr) || buffer_size == 0) {
        UtilityFunctions::push_warning("GDK: Failed to get gamer picture size");
        return;
    }

    std::vector<uint8_t> buffer(buffer_size);
    size_t buffer_used = 0;
    hr = XUserGetGamerPictureResult(async, buffer.size(), buffer.data(), &buffer_used);
    if (FAILED(hr)) {
        UtilityFunctions::push_warning("GDK: Failed to get gamer picture data");
        return;
    }

    // Convert PNG bytes to Godot ImageTexture
    PackedByteArray png_data;
    png_data.resize(buffer_used);
    memcpy(png_data.ptrw(), buffer.data(), buffer_used);

    Ref<Image> image;
    image.instantiate();
    Error err = image->load_png_from_buffer(png_data);
    if (err != OK) {
        UtilityFunctions::push_warning("GDK: Failed to decode gamer picture PNG");
        return;
    }

    Ref<ImageTexture> texture = ImageTexture::create_from_image(image);
    call_deferred("emit_signal", "gamer_picture_loaded", texture);
}

void GDKUserManager::sign_out() {
    if (!m_current_user.is_valid()) {
        return;
    }

    UtilityFunctions::print("GDK: Signing out user");
    _clear_user();
    emit_signal("user_signed_out");
}

Ref<GDKUserInfo> GDKUserManager::get_current_user() const {
    return m_current_user;
}

bool GDKUserManager::is_signed_in() const {
    return m_current_user.is_valid() && m_current_user->is_valid();
}

bool GDKUserManager::is_sign_in_pending() const {
    return m_sign_in_pending;
}

} // namespace godot
