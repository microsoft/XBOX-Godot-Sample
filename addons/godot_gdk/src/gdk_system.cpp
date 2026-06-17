#include "gdk_system.h"

#include <cstdio>

#include <godot_cpp/variant/utility_functions.hpp>

#include <XGame.h>
#include <XGameRuntimeFeature.h>
#include <XSystem.h>

#include "gdk.h"
#include "gdk_result.h"
#include "gdk_xbox_services.h"

namespace godot {

namespace {

String _format_title_id_hex(uint32_t p_title_id) {
    char buffer[11];
    std::snprintf(buffer, sizeof(buffer), "0x%08X", p_title_id);
    return String(buffer);
}

struct FeatureNameEntry {
    const char *name;
    XGameRuntimeFeature feature;
};

const FeatureNameEntry kFeatureNames[] = {
    { "XAccessibility", XGameRuntimeFeature::XAccessibility },
    { "XAppCapture", XGameRuntimeFeature::XAppCapture },
    { "XAsync", XGameRuntimeFeature::XAsync },
    { "XAsyncProvider", XGameRuntimeFeature::XAsyncProvider },
    { "XDisplay", XGameRuntimeFeature::XDisplay },
    { "XGame", XGameRuntimeFeature::XGame },
    { "XGameInvite", XGameRuntimeFeature::XGameInvite },
    { "XGameSave", XGameRuntimeFeature::XGameSave },
    { "XGameUI", XGameRuntimeFeature::XGameUI },
    { "XLauncher", XGameRuntimeFeature::XLauncher },
    { "XNetworking", XGameRuntimeFeature::XNetworking },
    { "XPackage", XGameRuntimeFeature::XPackage },
    { "XPersistentLocalStorage", XGameRuntimeFeature::XPersistentLocalStorage },
    { "XSpeechSynthesizer", XGameRuntimeFeature::XSpeechSynthesizer },
    { "XStore", XGameRuntimeFeature::XStore },
    { "XSystem", XGameRuntimeFeature::XSystem },
    { "XTaskQueue", XGameRuntimeFeature::XTaskQueue },
    { "XThread", XGameRuntimeFeature::XThread },
    { "XUser", XGameRuntimeFeature::XUser },
    { "XError", XGameRuntimeFeature::XError },
    { "XGameEvent", XGameRuntimeFeature::XGameEvent },
    { "XGameStreaming", XGameRuntimeFeature::XGameStreaming },
};

bool _resolve_feature(const String &p_feature_name, XGameRuntimeFeature *r_feature) {
    const String normalized = p_feature_name.strip_edges().to_lower();
    if (normalized.is_empty()) {
        return false;
    }

    for (const FeatureNameEntry &entry : kFeatureNames) {
        if (normalized == String(entry.name).to_lower()) {
            *r_feature = entry.feature;
            return true;
        }
    }
    return false;
}

} // namespace

void GDKSystem::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_title_id"), &GDKSystem::get_title_id);
    ClassDB::bind_method(D_METHOD("get_title_id_hex"), &GDKSystem::get_title_id_hex);
    ClassDB::bind_method(D_METHOD("get_sandbox_id"), &GDKSystem::get_sandbox_id);
    ClassDB::bind_method(D_METHOD("get_service_configuration_id"), &GDKSystem::get_service_configuration_id);
    ClassDB::bind_method(D_METHOD("is_xbox_services_initialized"), &GDKSystem::is_xbox_services_initialized);
    ClassDB::bind_method(D_METHOD("is_feature_available", "feature_name"), &GDKSystem::is_feature_available);
}

void GDKSystem::set_owner(GDK *p_owner) {
    m_owner = p_owner;
}

GDKXboxServices *GDKSystem::_get_xbox_services() const {
    if (m_owner == nullptr) {
        return nullptr;
    }

    return m_owner->get_xbox_services();
}

Ref<GDKResult> GDKSystem::get_title_id() const {
    uint32_t title_id = 0;
    HRESULT hr = XGameGetXboxTitleId(&title_id);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(
            hr,
            "Xbox title ID is unavailable.",
            "xbox_title_id_unavailable");
    }

    return GDKResult::ok_result(static_cast<int64_t>(title_id));
}

Ref<GDKResult> GDKSystem::get_title_id_hex() const {
    Ref<GDKResult> title_id_result = get_title_id();
    if (title_id_result.is_null() || !title_id_result->is_ok()) {
        return title_id_result;
    }

    const uint32_t title_id = static_cast<uint32_t>(int64_t(title_id_result->get_data()));
    return GDKResult::ok_result(_format_title_id_hex(title_id));
}

Ref<GDKResult> GDKSystem::get_sandbox_id() const {
    char sandbox_id[XSystemXboxLiveSandboxIdMaxBytes] = {};
    size_t sandbox_id_used = 0;
    const HRESULT hr = XSystemGetXboxLiveSandboxId(sizeof(sandbox_id), sandbox_id, &sandbox_id_used);
    if (FAILED(hr)) {
        return GDKResult::hresult_error(
            hr,
            "Xbox Live sandbox ID is unavailable.",
            "sandbox_id_unavailable");
    }

    sandbox_id[sizeof(sandbox_id) - 1] = '\0';
    if (sandbox_id_used == 0 || sandbox_id[0] == '\0') {
        return GDKResult::error_result(
            E_FAIL,
            "sandbox_id_unavailable",
            "Sandbox ID is unavailable.");
    }

    return GDKResult::ok_result(String::utf8(sandbox_id));
}

Ref<GDKResult> GDKSystem::get_service_configuration_id() const {
    GDKXboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr || !xbox_services->is_initialized()) {
        return GDKResult::error_result(
            E_FAIL,
            "xbox_services_uninitialized",
            "Xbox services are not initialized.");
    }

    const String scid = xbox_services->get_scid();
    if (scid.is_empty()) {
        return GDKResult::error_result(
            E_FAIL,
            "service_configuration_id_unavailable",
            "Service configuration ID is unavailable.");
    }

    return GDKResult::ok_result(scid);
}

bool GDKSystem::is_xbox_services_initialized() const {
    GDKXboxServices *xbox_services = _get_xbox_services();
    if (xbox_services == nullptr) {
        return false;
    }

    return xbox_services->is_initialized();
}

bool GDKSystem::is_feature_available(const String &p_feature_name) const {
    XGameRuntimeFeature feature = {};
    if (!_resolve_feature(p_feature_name, &feature)) {
        UtilityFunctions::push_warning(
                "GDK.system.is_feature_available: unknown feature name '" + p_feature_name + "'.");
        return false;
    }

    return XGameRuntimeIsFeatureAvailable(feature);
}

} // namespace godot
