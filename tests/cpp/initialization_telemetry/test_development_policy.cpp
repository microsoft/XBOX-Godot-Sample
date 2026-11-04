#include "doctest.h"
#include "../../../addons/initialization_telemetry/development_policy.h"

using namespace initialization_telemetry;

namespace {

DevelopmentEvidence unpackaged_debug() {
    DevelopmentEvidence evidence;
    evidence.gaming_package = PackageIdentity::unpackaged;
    evidence.engine_available = true;
    evidence.debug_feature = true;
    return evidence;
}

} // namespace

TEST_CASE("initialization telemetry suppression treats any nonempty value as opt out") {
    CHECK_FALSE(suppression_requested(L""));
    for (const auto *value : {L"0", L"false", L" ", L"\t", L"1", L"true"}) {
        CHECK(suppression_requested(value));
    }
}

TEST_CASE("initialization telemetry requires positive development evidence") {
    auto evidence = unpackaged_debug();
    CHECK(classify_development(evidence) == DevelopmentDecision::development_runtime);
    evidence.debug_feature = false;
    CHECK(classify_development(evidence) == DevelopmentDecision::unavailable);
    evidence.release_feature = true;
    CHECK(classify_development(evidence) == DevelopmentDecision::release);
    evidence.editor_feature = true;
    CHECK(classify_development(evidence) == DevelopmentDecision::conflicting);
    evidence.release_feature = false;
    CHECK(classify_development(evidence) == DevelopmentDecision::editor_runtime);
    evidence.editor_hint = true;
    CHECK(classify_development(evidence) == DevelopmentDecision::editor);
    evidence.initialized_with_options = true;
    CHECK(classify_development(evidence) == DevelopmentDecision::editor);
    evidence.editor_hint = false;
    CHECK(classify_development(evidence) == DevelopmentDecision::editor_runtime);
    evidence.editor_feature = false;
    CHECK(classify_development(evidence) == DevelopmentDecision::development_runtime);
    evidence.initialized_with_options = false;
    evidence.editor_hint = true;
    CHECK(classify_development(evidence) == DevelopmentDecision::conflicting);
}

TEST_CASE("initialization telemetry packaging exclusions dominate every development combination") {
    for (const auto gaming : {PackageIdentity::unknown, PackageIdentity::unpackaged, PackageIdentity::packaged}) {
        for (unsigned flags = 0; flags < 64; ++flags) {
            CAPTURE(gaming);
            CAPTURE(flags);
            DevelopmentEvidence evidence;
            evidence.gaming_package = gaming;
            evidence.engine_available = (flags & 1) != 0;
            evidence.editor_hint = (flags & 2) != 0;
            evidence.editor_feature = (flags & 4) != 0;
            evidence.debug_feature = (flags & 8) != 0;
            evidence.release_feature = (flags & 16) != 0;
            evidence.initialized_with_options = (flags & 32) != 0;
            const auto decision = classify_development(evidence);
            if (gaming == PackageIdentity::packaged) {
                CHECK(decision == DevelopmentDecision::packaged);
            } else if (gaming == PackageIdentity::unknown || !evidence.engine_available) {
                CHECK(decision == DevelopmentDecision::unavailable);
            }
            const bool expected_eligible = gaming == PackageIdentity::unpackaged &&
                    evidence.engine_available && !evidence.release_feature &&
                    (!evidence.editor_hint || evidence.editor_feature) &&
                    (evidence.editor_feature || evidence.debug_feature || evidence.initialized_with_options);
            CHECK(is_eligible(decision) == expected_eligible);
            evidence.suppressed = true;
            CHECK(classify_development(evidence) == DevelopmentDecision::suppressed);
        }
    }
}

TEST_CASE("initialization telemetry WithOptions cannot bypass unknown or release evidence") {
    auto evidence = unpackaged_debug();
    evidence.debug_feature = false;
    evidence.initialized_with_options = true;
    CHECK(classify_development(evidence) == DevelopmentDecision::development_runtime);
    evidence.gaming_package = PackageIdentity::unknown;
    CHECK_FALSE(is_eligible(classify_development(evidence)));
    evidence.gaming_package = PackageIdentity::unpackaged;
    evidence.release_feature = true;
    CHECK_FALSE(is_eligible(classify_development(evidence)));
}
