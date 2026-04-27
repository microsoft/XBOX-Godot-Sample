#include "gdk_core.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <XGameRuntimeInit.h>
#include <XTaskQueue.h>
#include <XGameRuntimeFeature.h>

namespace godot {

GDKCore *GDKCore::singleton = nullptr;

GDKCore *GDKCore::get_singleton() {
    return singleton;
}

GDKCore::GDKCore() {
    ERR_FAIL_COND(singleton != nullptr);
    singleton = this;
}

GDKCore::~GDKCore() {
    if (m_initialized) {
        shutdown();
    }
    singleton = nullptr;
}

void GDKCore::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize"), &GDKCore::initialize);
    ClassDB::bind_method(D_METHOD("shutdown"), &GDKCore::shutdown);
    ClassDB::bind_method(D_METHOD("is_initialized"), &GDKCore::is_initialized);
    ClassDB::bind_method(D_METHOD("get_version"), &GDKCore::get_version);
    ClassDB::bind_method(D_METHOD("tick"), &GDKCore::tick);

    ADD_SIGNAL(MethodInfo("initialized"));
    ADD_SIGNAL(MethodInfo("shutdown_completed"));
    ADD_SIGNAL(MethodInfo("error_occurred", PropertyInfo(Variant::STRING, "message")));
}

Error GDKCore::initialize() {
    if (m_initialized) {
        UtilityFunctions::push_warning("GDK already initialized");
        return ERR_ALREADY_EXISTS;
    }

    // Initialize the GDK runtime first — required before any other GDK API
    HRESULT hr = XGameRuntimeInitialize();

    if (FAILED(hr)) {
        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        String msg = String("Failed to initialize GDK runtime: ") + hex_buf;
        UtilityFunctions::push_error(msg);
        emit_signal("error_occurred", msg);
        return ERR_CANT_CREATE;
    }

    // Create task queue for async GDK operations
    hr = XTaskQueueCreate(
        XTaskQueueDispatchMode::ThreadPool,  // work dispatched to thread pool
        XTaskQueueDispatchMode::Manual,      // completion dispatched manually (on our tick)
        &m_task_queue
    );

    if (FAILED(hr)) {
        XGameRuntimeUninitialize();
        char hex_buf[16];
        snprintf(hex_buf, sizeof(hex_buf), "0x%08X", (unsigned int)hr);
        String msg = String("Failed to create XTaskQueue: ") + hex_buf;
        UtilityFunctions::push_error(msg);
        emit_signal("error_occurred", msg);
        return ERR_CANT_CREATE;
    }

    m_initialized = true;
    UtilityFunctions::print("GDK runtime initialized successfully");
    emit_signal("initialized");
    return OK;
}

void GDKCore::shutdown() {
    if (!m_initialized) {
        return;
    }

    XGameRuntimeUninitialize();

    if (m_task_queue) {
        XTaskQueueCloseHandle(m_task_queue);
        m_task_queue = nullptr;
    }

    m_initialized = false;
    UtilityFunctions::print("GDK runtime shut down");
    emit_signal("shutdown_completed");
}

bool GDKCore::is_initialized() const {
    return m_initialized;
}

void GDKCore::tick() {
    if (!m_initialized || !m_task_queue) {
        return;
    }

    // Dispatch completed async callbacks on the main thread
    while (XTaskQueueDispatch(m_task_queue, XTaskQueuePort::Completion, 0)) {
        // Keep dispatching until no more completions
    }
}

String GDKCore::get_version() const {
    return "GodotGDK v0.1.0-dev";
}

} // namespace godot
