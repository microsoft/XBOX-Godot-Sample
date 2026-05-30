#include "playfab_runtime.h"

#include <algorithm>

#include <godot_cpp/classes/project_settings.hpp>

#include "playfab_pending_signal.h"
#include "playfab_result.h"

namespace godot {

namespace {

#if defined(HC_PLATFORM) && HC_PLATFORM == HC_PLATFORM_GDK
constexpr bool PLAYFAB_GDK_PLATFORM = true;
#else
constexpr bool PLAYFAB_GDK_PLATFORM = false;
#endif

void wait_for_async_completion(HRESULT p_start_hr, XAsyncBlock *p_async_block) {
    if (SUCCEEDED(p_start_hr)) {
        XAsyncGetStatus(p_async_block, true);
    }
}

constexpr const char *PLAYFAB_TITLE_ID_SETTING = "playfab/runtime/title_id";
constexpr const char *PLAYFAB_ENDPOINT_SETTING = "playfab/runtime/endpoint";

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
        HRESULT runtime_hr = XGameRuntimeInitialize();
        if (FAILED(runtime_hr)) {
            Ref<PlayFabResult> result = PlayFabResult::hresult_error(runtime_hr, "Failed to initialize the Gaming Runtime.", "runtime_initialize_failed");
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
    if (FAILED(hr)) {
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

        Ref<PlayFabResult> result = PlayFabResult::hresult_error(hr, "Failed to initialize PlayFab Game Save Files.", "playfab_gamesave_initialize_failed");
        return result;
    }
    game_save_files_initialized = true;

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
    m_game_save_files_initialized = true;
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
        bool terminated = false;
        HRESULT terminate_hr = XTaskQueueTerminate(m_task_queue, false, &terminated, _queue_terminated);
        if (SUCCEEDED(terminate_hr)) {
            while (!terminated) {
                XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 10);
            }
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
    bool *terminated = static_cast<bool *>(p_context);
    if (terminated != nullptr) {
        *terminated = true;
    }
}

} // namespace godot
