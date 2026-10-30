#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <grdk.h>
#include <XGame.h>
#include <XPackage.h>
#include <XSystem.h>

#include "godot_telemetry.h"
#include "native_telemetry.h"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/xml_parser.hpp>
#include <godot_cpp/core/error_macros.hpp>

#include <cwchar>

namespace initialization_telemetry {
namespace {

ImageSession session;
bool usable = false;
bool warned = false;

void diagnostic(const char *p_message) {
    if (!warned) {
        warned = true;
        WARN_PRINT(godot::String("[Initialization telemetry] ") + p_message);
    }
}

std::wstring wide(const godot::String &p_value) {
    const godot::Char16String value = p_value.utf16();
    return std::wstring(reinterpret_cast<const wchar_t *>(value.get_data()), value.length());
}

void collect_configured_identity(const godot::String &p_selected_path, EventPayload &r_payload) {
    using namespace godot;
    String path = p_selected_path;
    if (path.is_empty()) {
        OS *os = OS::get_singleton();
        if (os == nullptr || os->has_feature("editor")) {
            return;
        }
        // Packed projects can globalize res:// to a relative path. The runtime
        // uses the physical config beside the exported executable instead.
        path = os->get_executable_path().get_base_dir().path_join("MicrosoftGame.config");
    }
    const std::wstring physical_path = wide(path);
    WIN32_FILE_ATTRIBUTE_DATA attributes{};
    if (!GetFileAttributesExW(physical_path.c_str(), GetFileExInfoStandard, &attributes)) {
        const DWORD status = GetLastError();
        if (status != ERROR_FILE_NOT_FOUND && status != ERROR_PATH_NOT_FOUND) {
            diagnostic("Game config identity could not be read; identity fields are unavailable.");
        }
        return;
    }
    if ((attributes.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0 ||
            attributes.nFileSizeHigh != 0 || attributes.nFileSizeLow > 1024 * 1024) {
        diagnostic("Game config identity input is invalid or too large.");
        return;
    }
    Ref<FileAccess> file = FileAccess::open(path, FileAccess::READ);
    if (file.is_null() || file->get_length() > 1024 * 1024) {
        diagnostic("Game config identity could not be read.");
        return;
    }
    const PackedByteArray bytes = file->get_buffer(file->get_length());
    Ref<XMLParser> parser;
    parser.instantiate();
    if (parser->open_buffer(bytes) != OK) {
        diagnostic("Game config identity could not be parsed; identity fields are unavailable.");
        return;
    }
    // Gaming Runtime already accepted the config; only extract identity metadata.
    Error status;
    while ((status = parser->read()) == OK) {
        if (parser->get_node_type() != XMLParser::NODE_ELEMENT || parser->get_node_name() != "Identity") {
            continue;
        }
        const String name = parser->get_named_attribute_value_safe("Name");
        const String publisher = parser->get_named_attribute_value_safe("Publisher");
        if (name.is_empty() || publisher.is_empty()) {
            return;
        }
        const LONG identity_status = configured_package_identity(wide(name), wide(publisher),
                r_payload.configured_title_package_family_name, r_payload.package_publisher_id);
        if (identity_status != ERROR_SUCCESS) {
            diagnostic("Configured package identity could not be derived; identity fields are unavailable.");
        }
        return;
    }
    if (status != ERR_FILE_EOF) {
        diagnostic("Game config identity could not be parsed; identity fields are unavailable.");
    }
}

} // namespace

void initialize() {
    const HRESULT session_status = session.initialize();
    const ULONG provider_status = register_provider();
    usable = SUCCEEDED(session_status) && provider_status == ERROR_SUCCESS;
    if (!usable) {
        diagnostic("Provider/session initialization failed; addon initialization is unaffected.");
    }
}

void shutdown() {
    usable = false;
    unregister_provider();
}

void record_initialized(bool p_with_options, const godot::String &p_config_path,
        const godot::String &p_playfab_title_id) {
    using namespace godot;
    if (!session.record_success()) {
        diagnostic("Cycle correlation is unavailable; this event is suppressed.");
        return;
    }
    const EnvironmentSuppression suppression = process_suppression();
    if (suppression != EnvironmentSuppression::absent_or_empty) {
        if (suppression == EnvironmentSuppression::unavailable) {
            diagnostic("Process suppression state is unknown; collection is suppressed.");
        }
        return;
    }
    if (!usable || !provider_enabled()) {
        return;
    }
    DevelopmentEvidence evidence;
    evidence.gaming_package = XPackageIsPackagedProcess() ? PackageIdentity::packaged : PackageIdentity::unpackaged;
    LONG package_status = ERROR_SUCCESS;
    evidence.windows_package = windows_package_identity(package_status);
    if (evidence.windows_package == PackageIdentity::unknown) {
        diagnostic("Windows package classification is unknown; collection is suppressed.");
    }
    Engine *engine = Engine::get_singleton();
    OS *os = OS::get_singleton();
    evidence.engine_available = engine != nullptr && os != nullptr;
    if (evidence.engine_available) {
        evidence.editor_hint = engine->is_editor_hint();
        evidence.editor_feature = os->has_feature("editor");
        evidence.debug_feature = os->has_feature("debug");
        evidence.release_feature = os->has_feature("release");
    }
    evidence.initialized_with_options = p_with_options;
    const DevelopmentDecision decision = classify_development(evidence);
    if (!is_eligible(decision)) {
        return;
    }
    EventPayload payload;
    payload.component_id = SDK_COMPONENT_ID;
    payload.component_version = SDK_COMPONENT_VERSION;
    payload.engine_version = wide(String(engine->get_version_info().get("string", "")));
    if (!valid_value(payload.engine_version, VERSION_LIMIT)) {
        payload.engine_version.clear();
        diagnostic("Engine version exceeds the telemetry contract; the field is unavailable.");
    }
    payload.development_context = development_context(decision);
    payload.device_type = static_cast<uint32_t>(XSystemGetDeviceType());
    const XVersion runtime_version = XSystemGetRuntimeInfo().runtimeVersion;
    if (runtime_version.major || runtime_version.minor || runtime_version.build || runtime_version.revision) {
        payload.gaming_runtime_version = std::to_wstring(runtime_version.major) + L"." +
                std::to_wstring(runtime_version.minor) + L"." + std::to_wstring(runtime_version.build) +
                L"." + std::to_wstring(runtime_version.revision);
    }
#if defined(_GRDK_EDITION) && _GRDK_EDITION != 999999
    payload.gdk_build_sdk_version = std::to_wstring(_GRDK_EDITION);
#endif
    uint32_t title = 0;
    if (SUCCEEDED(XGameGetXboxTitleId(&title))) {
        wchar_t value[9]{};
        swprintf_s(value, L"%08X", title);
        payload.xbox_title_id = value;
    }
    payload.playfab_title_id = wide(p_playfab_title_id);
    if (!valid_value(payload.playfab_title_id, IDENTIFIER_LIMIT)) {
        payload.playfab_title_id.clear();
        diagnostic("Configured title ID exceeds the telemetry contract; the field is unavailable.");
    }
    collect_configured_identity(p_config_path, payload);
    if (!valid_payload(payload)) {
        diagnostic("Collected metadata does not satisfy the event contract; this event is suppressed.");
        return;
    }
    if (process_suppression() == EnvironmentSuppression::absent_or_empty && provider_enabled()) {
        // TraceLoggingWrite has no result; ETW delivery is best-effort, without replay.
        write_initialized(session, payload);
    }
}

} // namespace initialization_telemetry
