#include "playfab_runtime.h"

#include <algorithm>
#include <atomic>

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include "playfab_pending_signal.h"
#include "playfab_result.h"

namespace godot {

namespace {

#if defined(HC_PLATFORM) && HC_PLATFORM == HC_PLATFORM_GDK
constexpr bool PLAYFAB_GDK_PLATFORM = true;
#else
constexpr bool PLAYFAB_GDK_PLATFORM = false;
#endif

struct RuntimeQueueTerminateContext {
    std::atomic<bool> terminated{false};
};

void wait_for_async_completion(HRESULT p_start_hr, XAsyncBlock *p_async_block) {
    if (SUCCEEDED(p_start_hr)) {
        XAsyncGetStatus(p_async_block, true);
    }
}

constexpr const char *PLAYFAB_TITLE_ID_SETTING = "playfab/runtime/title_id";
constexpr const char *PLAYFAB_ENDPOINT_SETTING = "playfab/runtime/endpoint";

constexpr const char *GAME_CONFIG_RES_PATH = "res://MicrosoftGame.config";

// Process-lifetime storage for the UTF-8 path handed to
// XGameRuntimeInitializeWithOptions. The Microsoft GDK runtime is a
// process-lifetime resource and we initialize it at most once per process, so
// a single static buffer is sufficient. Keeping the bytes alive here removes
// any reliance on the XGameRuntimeInitializeWithOptions copy semantics, which
// the public XGameRuntimeInit.h header does not document either way.
CharString g_xgame_runtime_config_path;

// Resolves res://MicrosoftGame.config to an absolute OS path, or returns an
// empty String when the file is not present on disk. Uses GetFileAttributesExW
// instead of Godot's FileAccess so a config that lives inside a packed .pck is
// not mistaken for a real file (XGameRuntime cannot read from inside a .pck).
String _resolve_game_config_path() {
    ProjectSettings *settings = ProjectSettings::get_singleton();
    if (settings == nullptr) {
        return String();
    }

    String resolved = settings->globalize_path(GAME_CONFIG_RES_PATH);
    if (resolved.is_empty() || resolved.begins_with("res://")) {
        return String();
    }

    Char16String wide_path = resolved.utf16();
    WIN32_FILE_ATTRIBUTE_DATA attrs = {};
    BOOL ok = GetFileAttributesExW(
            reinterpret_cast<LPCWSTR>(wide_path.get_data()),
            GetFileExInfoStandard,
            &attrs);
    if (!ok) {
        return String();
    }
    if ((attrs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return String();
    }
    return resolved;
}

// Calls XGameRuntimeInitializeWithOptions when res://MicrosoftGame.config is
// on disk so unpackaged Godot dev runs (editor or `godot project.godot`) pick
// up the game config explicitly and avoid the "no package identity" HRESULT
// class. Falls back to XGameRuntimeInitialize() for packaged GDK launches,
// where the registered package supplies identity. Mirrors the helper in
// godot_gdk so both addons feed XGameRuntime the same options; XGameRuntime is
// ref-counted, and using the same hardcoded path keeps the first caller's
// options consistent regardless of bootstrap ordering.
//
// r_config_path is set to the absolute path passed into
// XGameRuntimeInitializeWithOptions, or to an empty String when the fall-back
// path was taken; callers include it in their failure message.
HRESULT _initialize_xgame_runtime(String &r_config_path) {
    String resolved = _resolve_game_config_path();
    if (resolved.is_empty()) {
        r_config_path = String();
        return XGameRuntimeInitialize();
    }

    g_xgame_runtime_config_path = resolved.utf8();
    XGameRuntimeOptions options = {};
    options.gameConfigSource = XGameRuntimeGameConfigSource::File;
    options.gameConfig = g_xgame_runtime_config_path.get_data();
    r_config_path = resolved;
    return XGameRuntimeInitializeWithOptions(&options);
}

} // namespace

PlayFabRuntime::PlayFabRuntime() {
}

PlayFabRuntime::~PlayFabRuntime() {
    shutdown();

    // XGameRuntime is process-lifetime state. Keep initialize()/shutdown()
    // re-armable and release this addon's runtime reference only when the
    // extension singleton is torn down.
    if (m_xgame_runtime_initialized) {
        XGameRuntimeUninitialize();
        m_xgame_runtime_initialized = false;
    }
}

Ref<PlayFabResult> PlayFabRuntime::initialize() {
    if (m_initialized) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(E_FAIL, "already_initialized", "PlayFab runtime is already initialized.");
        return result;
    }

    if (!PLAYFAB_GDK_PLATFORM) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(E_FAIL, "platform_unsupported", "The refactored PlayFab runtime currently supports GDK platforms only.");
        return result;
    }

    ProjectSettings *project_settings = ProjectSettings::get_singleton();
    if (project_settings == nullptr) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(E_FAIL, "project_settings_unavailable", "ProjectSettings is unavailable.");
        return result;
    }

    const String title_id = String(project_settings->get_setting(PLAYFAB_TITLE_ID_SETTING, "")).strip_edges();
    String endpoint = String(project_settings->get_setting(PLAYFAB_ENDPOINT_SETTING, "")).strip_edges();
    if (title_id.is_empty()) {
        Ref<PlayFabResult> result = PlayFabResult::error_result(E_INVALIDARG, "title_id_required", "PlayFab initialization requires ProjectSettings['playfab/runtime/title_id'] to be set.");
        return result;
    }

    if (endpoint.is_empty()) {
        endpoint = "https://" + title_id + ".playfabapi.com";
    }

    bool playfab_core_initialized = false;
    bool playfab_services_initialized = false;
    bool game_save_files_initialized = false;

    if (!m_xgame_runtime_initialized) {
        String attempted_config_path;
        HRESULT runtime_hr = _initialize_xgame_runtime(attempted_config_path);
        if (FAILED(runtime_hr)) {
            String message = "Failed to initialize the Gaming Runtime.";
            if (!attempted_config_path.is_empty()) {
                message += " Tried game config at: " + attempted_config_path;
            }
            Ref<PlayFabResult> result = PlayFabResult::hresult_error(runtime_hr, message, "runtime_initialize_failed");
            return result;
        }
        m_xgame_runtime_initialized = true;
    }

    HRESULT hr = XTaskQueueCreate(
            XTaskQueueDispatchMode::ThreadPool,
            XTaskQueueDispatchMode::Manual,
            &m_task_queue);
    if (FAILED(hr)) {
        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to create the shared XTaskQueue.", "task_queue_create_failed");
        return result;
    }

    hr = PFInitialize(nullptr);
    if (FAILED(hr)) {
        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to initialize PlayFab Core.", "playfab_core_initialize_failed");
        return result;
    }
    playfab_core_initialized = true;

    hr = PFServicesInitialize(nullptr);
    if (FAILED(hr)) {
        XAsyncBlock async = {};
        wait_for_async_completion(PFUninitializeAsync(&async), &async);

        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to initialize PlayFab Services.", "playfab_services_initialize_failed");
        return result;
    }
    playfab_services_initialized = true;

    PFGameSaveInitArgs game_save_init_args = {};
    game_save_init_args.backgroundQueue = m_task_queue;
    game_save_init_args.options = static_cast<uint64_t>(PFGameSaveInitOptions::None);
    game_save_init_args.saveFolder = nullptr;

    hr = PFGameSaveFilesInitialize(&game_save_init_args);
    if (hr == E_INVALIDARG) {
        // Game Save Files require a packaged GDK title identity and a
        // signed-in Xbox user. Custom-ID / headless sessions and CI runners
        // without a packaged identity legitimately cannot initialize them --
        // PFGameSaveFilesInitialize reports that expected case as E_INVALIDARG.
        // (Confirmed on a headless Windows Server CI runner: the unpackaged /
        // no-signed-in-user session returns 0x80070057 == E_INVALIDARG.)
        // Degrade gracefully ONLY for that HRESULT: keep PlayFab Core/Services
        // running and leave Game Saves unavailable instead of taking the whole
        // runtime down. Game Saves methods still reject non-Xbox-backed users
        // with xbox_user_required.
        //
        // This is surfaced via print() rather than WARN_PRINT on purpose: Game
        // Saves being unavailable is an expected, handled condition for
        // custom-ID / unpackaged sessions, and routing it through the engine
        // error system would make GUT (which fails tests on unexpected engine
        // errors) fail every live test that initializes PlayFab.
        UtilityFunctions::print(String("[PlayFab] Game Save Files unavailable (") + PlayFabResult::format_hresult(hr) + "); continuing without Game Saves. This is expected for custom-ID or unpackaged sessions.");
        game_save_files_initialized = false;
    } else if (FAILED(hr)) {
        // Any other failure is unexpected (e.g. on a packaged title where Game
        // Saves should be available) and must NOT be masked. Log the exact
        // HRESULT for diagnosis, then fail fast and tear down Services + Core
        // (in reverse init order) so the caller sees the real error instead of a
        // silently-degraded runtime.
        UtilityFunctions::print(String("[PlayFab] Unexpected Game Save Files initialization failure (") + PlayFabResult::format_hresult(hr) + "); failing PlayFab initialization.");

        XAsyncBlock services_async = {};
        wait_for_async_completion(PFServicesUninitializeAsync(&services_async), &services_async);

        XAsyncBlock core_async = {};
        wait_for_async_completion(PFUninitializeAsync(&core_async), &core_async);

        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to initialize PlayFab Game Save Files.", "playfab_game_save_initialize_failed");
        return result;
    } else {
        game_save_files_initialized = true;
    }

    m_title_id = title_id;
    m_endpoint = endpoint;

    const CharString endpoint_utf8 = m_endpoint.utf8();
    const CharString title_id_utf8 = m_title_id.utf8();
    hr = PFServiceConfigCreateHandle(
            endpoint_utf8.get_data(),
            title_id_utf8.get_data(),
            &m_service_config_handle);
    if (FAILED(hr)) {
        if (game_save_files_initialized) {
            XAsyncBlock game_save_async = {};
            wait_for_async_completion(PFGameSaveFilesUninitializeAsync(&game_save_async), &game_save_async);
        }
        if (playfab_services_initialized) {
            XAsyncBlock services_async = {};
            wait_for_async_completion(PFServicesUninitializeAsync(&services_async), &services_async);
        }
        if (playfab_core_initialized) {
            XAsyncBlock core_async = {};
            wait_for_async_completion(PFUninitializeAsync(&core_async), &core_async);
        }

        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;
        m_title_id = "";
        m_endpoint = "";

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to create the PlayFab service configuration.", "service_config_create_failed");
        return result;
    }

    m_initialized = true;
    m_shutting_down = false;
    m_game_save_files_initialized = game_save_files_initialized;
    return PlayFabResult::ok_result();
}

void PlayFabRuntime::shutdown() {
    if (!m_initialized) {
        return;
    }

    m_shutting_down = true;

    std::vector<Ref<PlayFabPendingSignal>> active_pending_signals = m_active_pending_signals;
    for (const Ref<PlayFabPendingSignal> &pending_signal : active_pending_signals) {
        if (pending_signal.is_valid()) {
            pending_signal->cancel();
        }
    }

    if (m_game_save_files_initialized) {
        XAsyncBlock game_save_async = {};
        wait_for_async_completion(PFGameSaveFilesUninitializeAsync(&game_save_async), &game_save_async);
        m_game_save_files_initialized = false;
    }

    if (m_service_config_handle != nullptr) {
        PFServiceConfigCloseHandle(m_service_config_handle);
        m_service_config_handle = nullptr;
    }

    XAsyncBlock services_async = {};
    wait_for_async_completion(PFServicesUninitializeAsync(&services_async), &services_async);

    XAsyncBlock core_async = {};
    wait_for_async_completion(PFUninitializeAsync(&core_async), &core_async);

    if (m_task_queue != nullptr) {
        // Heap-allocate the terminate context so the wait can safely give up
        // without a stack UAF if the SDK callback fires later. Context is
        // freed only when the callback observably completes; otherwise it is
        // intentionally leaked.
        auto *ctx = new RuntimeQueueTerminateContext();
        HRESULT terminate_hr = XTaskQueueTerminate(m_task_queue, false, ctx, _queue_terminated);
        if (SUCCEEDED(terminate_hr)) {
            constexpr int kMaxDispatchIterations = 500; // ~5 seconds at 10ms each.
            int iterations = 0;
            while (!ctx->terminated.load(std::memory_order_acquire) && iterations < kMaxDispatchIterations) {
                XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 10);
                ++iterations;
            }
            if (ctx->terminated.load(std::memory_order_acquire)) {
                delete ctx;
            } else {
                WARN_PRINT("PlayFabRuntime: XTaskQueueTerminate did not complete within 5s; leaking terminate context to avoid UAF if the SDK callback fires later.");
                // ctx intentionally leaked.
            }
        } else {
            delete ctx;
        }

        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;
    }

    for (Ref<PlayFabPendingSignal> &pending_signal : m_active_pending_signals) {
        if (pending_signal.is_valid()) {
            pending_signal->clear_cancel_handler();
            pending_signal->clear_release_handler();
        }
    }
    m_active_pending_signals.clear();

    // XGameRuntimeUninitialize intentionally does not run here. It is paired
    // with the first successful XGameRuntimeInitialize and released once from
    // ~PlayFabRuntime() during extension teardown.

    m_initialized = false;
    m_shutting_down = false;
    m_game_save_files_initialized = false;
    m_title_id = "";
    m_endpoint = "";
}

int PlayFabRuntime::dispatch() {
    if (!m_initialized || m_task_queue == nullptr) {
        return 0;
    }

    int dispatched = 0;
    while (XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 0)) {
        ++dispatched;
    }

    return dispatched;
}

void CALLBACK PlayFabRuntime::_dispatch_probe_completed(void *p_context, bool p_cancelled) {
    // Runs on the thread that dispatches the Completion port (the Godot main
    // thread, via the frame-callback auto-pump or a manual PlayFab.dispatch()).
    // A cancelled invocation means the queue was terminating, not that the
    // pump drained the port, so it must not count toward the probe.
    if (p_cancelled || p_context == nullptr) {
        return;
    }

    PlayFabRuntime *runtime = static_cast<PlayFabRuntime *>(p_context);
    runtime->m_dispatch_probe_count.fetch_add(1, std::memory_order_relaxed);
}

bool PlayFabRuntime::submit_dispatch_probe() {
    // Diagnostics/test hook: enqueue a no-op callback on the shared task queue's
    // Completion port so callers can verify the per-frame auto-pump actually
    // drains that port -- without a network round-trip or a signed-in user.
    // The callback only runs when the Completion port is dispatched, i.e. by
    // PlayFab.dispatch() (manual) or the extension frame callback (auto).
    if (!m_initialized || m_task_queue == nullptr) {
        return false;
    }

    HRESULT hr = XTaskQueueSubmitCallback(
            m_task_queue,
            XTaskQueuePort::Completion,
            this,
            &PlayFabRuntime::_dispatch_probe_completed);
    return SUCCEEDED(hr);
}

uint64_t PlayFabRuntime::get_dispatch_probe_count() const {
    return m_dispatch_probe_count.load(std::memory_order_relaxed);
}

bool PlayFabRuntime::is_initialized() const {
    return m_initialized;
}

bool PlayFabRuntime::is_shutting_down() const {
    return m_shutting_down;
}

bool PlayFabRuntime::is_available() const {
    return PLAYFAB_GDK_PLATFORM;
}

XTaskQueueHandle PlayFabRuntime::get_task_queue() const {
    return m_task_queue;
}

PFServiceConfigHandle PlayFabRuntime::get_service_config_handle() const {
    return m_service_config_handle;
}

String PlayFabRuntime::get_title_id() const {
    return m_title_id;
}

String PlayFabRuntime::get_endpoint() const {
    return m_endpoint;
}

void PlayFabRuntime::retain_pending_signal(const Ref<PlayFabPendingSignal> &p_pending_signal) {
    if (!p_pending_signal.is_valid() || p_pending_signal->is_done()) {
        return;
    }

    p_pending_signal->set_release_handler([this](PlayFabPendingSignal *p_completed_signal) {
        release_pending_signal(p_completed_signal);
    });
    m_active_pending_signals.push_back(p_pending_signal);
}

void PlayFabRuntime::release_pending_signal(PlayFabPendingSignal *p_pending_signal) {
    m_active_pending_signals.erase(
            std::remove_if(
                    m_active_pending_signals.begin(),
                    m_active_pending_signals.end(),
                    [p_pending_signal](const Ref<PlayFabPendingSignal> &candidate) {
                        return candidate.is_null() || candidate.operator->() == p_pending_signal;
                    }),
            m_active_pending_signals.end());
}

Ref<PlayFabPendingSignal> PlayFabRuntime::make_pending_signal() {
    Ref<PlayFabPendingSignal> pending_signal;
    pending_signal.instantiate();
    retain_pending_signal(pending_signal);
    return pending_signal;
}

Signal PlayFabRuntime::make_error_signal(HRESULT p_hresult, const String &p_code, const String &p_message, const Variant &p_data) {
    Ref<PlayFabPendingSignal> pending_signal = make_pending_signal();
    Ref<PlayFabResult> result = PlayFabResult::error_result(p_hresult, p_code, p_message, p_data);
    pending_signal->complete_deferred(result);
    return pending_signal->get_completed_signal();
}

void CALLBACK PlayFabRuntime::_queue_terminated(void *p_context) {
    auto *ctx = static_cast<RuntimeQueueTerminateContext *>(p_context);
    if (ctx != nullptr) {
        ctx->terminated.store(true, std::memory_order_release);
    }
}

} // namespace godot
