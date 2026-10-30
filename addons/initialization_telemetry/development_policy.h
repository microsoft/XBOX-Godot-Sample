#ifndef INITIALIZATION_TELEMETRY_DEVELOPMENT_POLICY_H
#define INITIALIZATION_TELEMETRY_DEVELOPMENT_POLICY_H

#include <string_view>

namespace initialization_telemetry {

// Local evidence and decisions only; these enums are not the event's wire enums.
enum class PackageIdentity {
    unknown,
    unpackaged,
    packaged,
};

enum class DevelopmentDecision {
    suppressed,
    packaged,
    unavailable,
    conflicting,
    release,
    editor,
    editor_runtime,
    development_runtime,
};

struct DevelopmentEvidence {
    bool suppressed = false;
    PackageIdentity gaming_package = PackageIdentity::unknown;
    PackageIdentity windows_package = PackageIdentity::unknown;
    bool engine_available = false;
    bool editor_hint = false;
    bool editor_feature = false;
    bool debug_feature = false;
    bool release_feature = false;
    bool initialized_with_options = false;
};

constexpr bool suppression_requested(std::wstring_view p_process_value) {
    return !p_process_value.empty();
}

constexpr DevelopmentDecision classify_development(const DevelopmentEvidence &p_evidence) {
    if (p_evidence.suppressed) {
        return DevelopmentDecision::suppressed;
    }
    if (p_evidence.gaming_package == PackageIdentity::packaged ||
            p_evidence.windows_package == PackageIdentity::packaged) {
        return DevelopmentDecision::packaged;
    }
    if (p_evidence.gaming_package != PackageIdentity::unpackaged ||
            p_evidence.windows_package != PackageIdentity::unpackaged ||
            !p_evidence.engine_available) {
        return DevelopmentDecision::unavailable;
    }
    const bool development_evidence = p_evidence.editor_hint || p_evidence.editor_feature ||
            p_evidence.debug_feature || p_evidence.initialized_with_options;
    if (p_evidence.release_feature) {
        return development_evidence ? DevelopmentDecision::conflicting : DevelopmentDecision::release;
    }
    if (p_evidence.editor_hint && !p_evidence.editor_feature) {
        return DevelopmentDecision::conflicting;
    }
    if (p_evidence.editor_hint) {
        return DevelopmentDecision::editor;
    }
    if (p_evidence.editor_feature) {
        return DevelopmentDecision::editor_runtime;
    }
    if (p_evidence.debug_feature || p_evidence.initialized_with_options) {
        return DevelopmentDecision::development_runtime;
    }
    return DevelopmentDecision::unavailable;
}

constexpr bool is_eligible(DevelopmentDecision p_decision) {
    return p_decision == DevelopmentDecision::editor ||
            p_decision == DevelopmentDecision::editor_runtime ||
            p_decision == DevelopmentDecision::development_runtime;
}

} // namespace initialization_telemetry

#endif // INITIALIZATION_TELEMETRY_DEVELOPMENT_POLICY_H
