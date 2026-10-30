# SDK component initialization telemetry

Status: native local ETW emission is implemented for successful eligible
development initialization in both addons. A local ETW consumer must enable the
provider; this implementation does not upload events or configure a cloud
destination. The standard Microsoft ProductAndServiceUsage privacy tag
classifies the event but does not enable collection or transmission. See
Microsoft's [Privacy Statement](https://privacy.microsoft.com/privacystatement)
for general privacy information; this implementation's actual local behavior
and data fields are described below.

This is the shared design for `godot_gdk` and `godot_playfab`. It does not change
their public initialization API. The design is engine/language neutral; its
implementation scope is only these two native Windows addons.

## Contract and scope

Use native ETW/TraceLogging, not an HTTP uploader, PlayFab analytics, title-facing
`GDK.events`, a managed facade emitter, or a new engine singleton. Do not introduce
a telemetry framework, spool, upload thread, retry, backfill, or shutdown flush.
PlayFab must work independently of the GDK addon; each DLL owns its provider and
loaded-image cycle counter.

The event is `SdkComponentInitialized`. It measures successful eligible
development initialization cycles, not attempts, success rates, unique developers,
devices, installs, or process launches. Count the two components independently.
A native Gaming Runtime reference acquisition is not another addon success.

At full addon success, after GDK's rollback guard commits or PlayFab's users
initialize, advance the cycle counter and attempt at most one write **before** the
existing `initialized` signal can reenter shutdown/initialization:

- `InitializationSequence`: UInt64 successful-cycle counter for that image.
  Advance on each full success, even when collection is suppressed; not on
  failure, rollback, rejected `already_initialized`, or shutdown.
  Preserve it across ordinary shutdown/reinitialization; actual DLL unload/reload
  resets it. No session GUID or globally unique deduplication key is emitted.
  Use ETW process/timestamp metadata and ComponentId to interpret captured cycles;
  distinct legitimate cycles must not be collapsed.

Provider registration belongs at safe extension registration boundaries, respecting
each `register_types.cpp` duplicate-load reference count, never `DllMain`.
Registration/metadata/write failures cannot change addon success, rollback, or
runtime ownership. Use bounded local diagnostics for unexpected errors without
printing identity values, paths, credentials, or payloads. Disabled consumers and
missing optional metadata are expected, not failures to retry.

Provider name: `Microsoft.Gaming.Middleware`, name-derived GUID
`{D13E8219-218A-51FC-8362-8C90CF602D01}`, informational ETW severity level 4,
Core event tag `0x00080000` and Critical Data category keyword
`0x0000800000000000`. Core is not an ETW severity level. These values match
`MICROSOFT_EVENTTAG_CORE_DATA` and `MICROSOFT_KEYWORD_CRITICAL_DATA` in the
[public WIL definitions](https://github.com/microsoft/wil/blob/master/include/wil/traceloggingconfig.h).
They are event metadata, not additional payload fields or an upload mechanism.
The GUID is the stable provider ID derived from the provider name, not a Core
provider-group ID.

The MIT-licensed `addons\initialization_telemetry\TraceLoggingConfig.h` supplies
the public-build configuration. Provider registration uses its
`TraceLoggingOptionWindowsCoreTelemetry()` macro and event constants come from
the same header. **The macro is intentionally empty:** public builds
remain normal ETW providers with no Microsoft provider-group membership.
Public builds use only this stub; they do not configure an upload destination.
The standard `TelemetryPrivacyDataTag(PDT_ProductAndServiceUsage)` adds
`PartA_PrivTags` as UInt64 with value `0x0000000002000000`. This is
usage-classification metadata, not a collection, consent, or transmission
setting. An ETW consumer must enable the provider; otherwise no event is written.

## Local eligibility and suppression

Check the **process environment** before optional telemetry collection and again
before a write. Any nonempty `GODOT_ADDON_SUPPRESS_TELEMETRY` value suppresses,
including `0`, `false`, or whitespace. Do not parse it as a Boolean or query raw
user/system registry environment values. The shared test runner does not set
this variable automatically; controlled validation can set it explicitly.

After Gaming Runtime initialization, **both addons** must query
`XPackageIsPackagedProcess()`. A true result excludes all installation-identity
processes, including debug packages. False is not sufficient for eligibility.
Supplement it with Windows `GetCurrentPackageFamilyName`; treat
`APPMODEL_ERROR_NO_PACKAGE` as positive unpackaged evidence, confirmed identity as
excluded, and an unexpected result as unknown/suppressed. Packaging is not retail.

Require positive running-engine development evidence and suppress release,
conflicting, or unavailable evidence. Use `Engine::is_editor_hint()` for editor UI,
`OS::has_feature("editor")` for an editor-capable engine running a project, and the
running engine's debug/release features for exports, not the addon's `_DEBUG`.
Editor feature evidence does not establish a parent/launcher relationship.
Keep the actual successful WithOptions selection across reused native runtime
references; do not infer it from current config presence or change initializer
selection for telemetry. Successful WithOptions initialization is positive
development evidence even outside an editor-capable engine and without a debug
feature; classify that case as a development runtime. Package exclusions and
explicit release/conflicting evidence still suppress.

The private shared code is compiled separately into each DLL:
`development_policy.h` contains the pure decision policy,
`native_telemetry.*` contains Windows identity/correlation/TraceLogging code, and
`godot_telemetry.*` collects the actual running engine and Gaming Runtime metadata.
Each successful root initialization calls it before `initialized`. Unknown
package/engine evidence fails closed, and release evidence cannot be overridden
by debug/editor/WithOptions evidence. Sequence advancement precedes suppression;
eligible captured events can therefore have gaps when earlier cycles were suppressed.

Do not query, collect, persist, or use
`XPackageGetCurrentProcessPackageIdentifier` as a package gate. The isolated
experiment established that it can succeed in an unpackaged config-backed process;
its opaque installation/session token is not a PFN.

## Local-capture schema version 1

Exactly 17 application fields plus the standard UInt64 `PartA_PrivTags` field:
18 ETW payload fields total, plus the ETW header timestamp. The privacy field
precedes the application fields below.
This remains the unreleased schema revision 1; earlier captures containing a
runtime session GUID are not the current capture contract.
Empty optional strings retain the fixed schema. No per-field status/source, package/evidence/path,
HRESULT, success flag, redundant SignalType, or current-process PFN fields.
No persistent user/device/install identifiers.

| Field | Native type | Meaning/source |
| --- | --- | --- |
| PartA_PrivTags | UInt64 | Standard privacy metadata: `PDT_ProductAndServiceUsage` = `0x0000000002000000` |
| SchemaVersion | UInt32 | Payload revision 1 |
| ComponentId | UnicodeString | `godot_gdk` or `godot_playfab` |
| ComponentVersion | UnicodeString | Owning addon's plugin.cfg release version, embedded at build time |
| ComponentKind | UInt32 enum | Engine addon = 3; reserved native SDK = 1, managed SDK = 2, language wrapper = 4 |
| InitializationSequence | UInt64 | Successful-cycle number within image |
| EngineName | UnicodeString | Godot for these emitters |
| EngineVersion | UnicodeString | Actual calling `Engine::get_version_info()`, not godot-cpp build constants |
| ExecutionRuntimeName | UnicodeString | Empty; managed/interpreted attribution deferred |
| ExecutionRuntimeVersion | UnicodeString | Empty; do not infer .NET |
| GamingRuntimeVersion | UnicodeString | XSystemGetRuntimeInfo().runtimeVersion, four decimal components; all-zero result is unavailable |
| GdkBuildSdkVersion | UnicodeString | Real selected grdk.h macros; never synthesized edition 999999 |
| DevelopmentContext | UInt32 enum | Editor = 1, editor-runtime = 2, development runtime = 3; tooling = 4 is reserved, not emitted |
| ConfiguredTitlePackageFamilyName | UnicodeString | Explicit valid game config identity, not editor identity |
| PackagePublisherId | UnicodeString | Windows package publisher ID from configured game PFN |
| DeviceType | UInt32 enum | Exact `XSystemDeviceType` numeric value returned by `XSystemGetDeviceType()`, without model flattening or remapping |
| XboxTitleId | UnicodeString | XGameGetXboxTitleId as eight uppercase hex digits, else empty |
| PlayFabTitleId | UnicodeString | Resolved configured PlayFab title ID, else empty |

Underlying PlayFab SDK versions are expressly excluded; SDK HTTP headers already
provide that attribution outside this event. Keep the addon's ComponentVersion.
Do not inspect network traffic or investigate another SDK-version field.

The selected GDK defines device values as Unknown = 0, Pc = 1, XboxOne = 2,
XboxOneS = 3, XboxOneX = 4, XboxOneXDevkit = 5, XboxScarlettLockhart = 6,
XboxScarlettAnaconda = 7 and XboxScarlettDevkit = 8. Preserve future UInt32 values
unchanged rather than coercing them to Unknown or suppressing the event.

Lengths are UTF-16 code units, excluding the terminating NUL. Versions are limited
to 128, component/PlayFab IDs to 256, configured PFN to Windows'
`PACKAGE_FAMILY_NAME_MAX_LENGTH` (64), package publisher ID to 13, and Xbox title ID
to exactly eight uppercase hex digits when present. EngineName is fixed `Godot`;
execution-runtime strings are empty. Reject embedded NUL/control characters and
invalid surrogate pairs. Invalid optional metadata becomes unavailable, not
truncated; an invalid required payload suppresses the event with a bounded local
diagnostic. These local choices still need review for any future production route.

Use `PackageFamilyNameFromId` with explicit valid game config `Identity.Name` and
`Identity.Publisher`, then `PackageNameAndPublisherIdFromFamilyName` to extract
the publisher ID with documented buffer sizing. Leave both empty if unavailable.
No custom hashing, raw publisher DN telemetry, installed-package enumeration,
editor identity substitution, or new setting before using existing config.
This is configured identity, not ownership verification; it is not a Partner
Center seller/organization ID or a PlayFab publisher ID.

For WithOptions, use the actual successfully selected config path preserved by
the owning runtime across shutdown/reinitialize. For plain initialization in a
non-editor engine, use the physical `MicrosoftGame.config` beside the executable.
Do not compare this absolute path with `ProjectSettings.globalize_path("res://...")`:
a packed project's resource root can globalize to a relative path. Never fall
back to a Godot editor executable's config when no successful WithOptions path
was retained. `gdk/packaging/game_config_dir` selects the source config for the
PC exporter to stage; it is not an exported game's runtime lookup directory.
Read at most 1 MiB after successful Gaming Runtime and addon initialization.
Reuse Godot's `XMLParser` to extract `Identity.Name` and `Identity.Publisher`,
then stop. This is metadata extraction from an already accepted config, not a
second schema validator; no separate XML library is needed. Reading/parsing
remains best-effort if the file becomes unavailable or changes after runtime
initialization. No raw publisher distinguished name or config path is emitted.
A missing config/identity leaves the two fields empty.

The Gaming Runtime version source is documented by
[XSystemGetRuntimeInfo](https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/system/xsystem/functions/xsystemgetruntimeinfo)
and [XSystemRuntimeInfo](https://learn.microsoft.com/en-us/gaming/gdk/docs/reference/system/xsystem/structs/xsystemruntimeinfo):
`runtimeVersion` is the runtime service the game is running on; zero means
unavailable. Do not substitute `availableVersion`, an OS version or a build SDK.

Include grdk.h independently of the existing feature-gate wrappers that synthesize
999999. Missing or synthetic metadata is unavailable; preserve existing feature
gates. Recording console device enum values does not add console build support.

Other-engine/direct-SDK/managed consumers may reuse the vocabulary only with an
explicitly reviewed source and eligibility mapping: component version describes
the measured component, engine version describes the actual host, execution
runtime describes the actual executing runtime, and unavailable values stay empty.
A forwarding wrapper must not emit a second native initialization event.

### Contract-only consumer examples

These are illustrative mappings, not implemented emitters or evidence of
additional platform support. Every future adapter needs its own reviewed
development-eligibility and metadata sources.

| Consumer | Component attribution | Engine / execution runtime | Counting |
| --- | --- | --- | --- |
| Native addon for another engine | Its own ComponentId/version; ComponentKind = engine addon (3) | For example, Unreal and the actual running engine version; execution-runtime fields empty for native code | One event per successful eligible initialization of that addon, not its dependencies |
| Direct native SDK | The measured SDK's own identity/version; ComponentKind = native SDK (1) | Engine and execution-runtime fields empty when neither exists | Its own initialization lifecycle and cycle counter |
| Independently initialized managed SDK | Its own identity/version; ComponentKind = managed SDK (2) | Actual engine if present; observed .NET runtime/version, never inferred from build dependencies | Its own initialization, separate from native dependency initialization |
| Managed wrapper forwarding to these Godot addons | The existing native event retains `godot_gdk` / `godot_playfab` identity | The native event remains Godot with empty execution-runtime fields | No additional initialization event; separate wrapper-use attribution remains deferred |

Unavailable optional values stay empty. Xbox and PlayFab title IDs remain
separate namespaces. Publisher grouping identifies configured Windows package
publisher identity, not ownership verification or a universal organization ID.
Use ETW process/timestamp metadata, ComponentId and InitializationSequence to
interpret cycles; these are not a globally unique deduplication key.
Distinct successful cycles remain distinct. Empty publisher identity means
unknown, not a shared organization. This success-only event cannot
provide an initialization success rate without a separate attempt denominator.

## Plan

1. **Establish local contract.** Use the project-owned provider and typed schema
   above for explicitly authorized local capture.
2. **Implement and integrate.** Compile a minimal private helper into both DLLs
   without a shared runtime dependency. Preserve initializer evidence and native
   references. Wire owning emitters, registration, cycle state, typed schema,
   truthful metadata and CMake versions/libraries. Update Markdown addon docs/specs,
   scoped instructions, sample guidance and tests in the same change. Telemetry
   does not belong in XML class docs. Emit no shutdown event; provider teardown
   only unregisters the handle, without writing or flushing.
3. **Verify source locally.** Review the exact contract, Debug build,
   separate-tree Release build, then relevant existing native/offline tests.
   For metadata-only follow-ups, narrow validation to the existing native suites
   and a controlled capture of both addons. Do not add a regression harness,
   failure hooks or an expanded acceptance matrix for this follow-up.
   Run the managed headless validator for any .gd change.
   Suppress ordinary test emissions.
4. **Capture locally.** Enable this provider with an authorized ETW session, run
   controlled synthetic addon lifecycle scenarios, stop the session and decode
   its ETL. Check names/types, counts, cycle sequences and truthful metadata.
   Do not elevate, install/register a package or switch sandbox without permission.

This implementation does not perform live-service writes, change a sandbox,
install or register a package, elevate privileges, or publish events remotely.
It does not configure remote collection or reporting.

## Local capture

Controlled validation can set `GODOT_ADDON_SUPPRESS_TELEMETRY` to a nonempty
value explicitly. For an explicitly authorized synthetic capture, remove that
variable **only from the controlled process environment**, run the game, then
stop/decode:

```powershell
logman start GodotAddonInit -p "{D13E8219-218A-51FC-8362-8C90CF602D01}" 0x0000800000000000 4 -o .\addon-init.etl -ets
# Launch the eligible development game here, with suppression removed from that process.
logman stop GodotAddonInit -ets
tracerpt .\addon-init.etl -o .\addon-init.xml -of XML -y
```

Use a unique session name and stop it in a `finally` block in scripted captures.
Capture requires the appropriate existing ETW permissions; do not automatically
elevate or change membership. ETL files include configured identity values and
ETW header process metadata: handle them as local diagnostic artifacts, not
public repository content. There is no uploader or remote destination.

## Validation record

After removing the runtime session GUID, both Debug addons and the native test
target rebuilt with their explicit `ole32` links removed:

| Command | Result |
| --- | --- |
| `cmake --build build\addon-package --config Debug --target godot_gdk godot_playfab gdk_unit_tests -- /verbosity:minimal` | Passed |
| `.\build\addon-package\bin\Debug\gdk_unit_tests.exe` | 26 cases / 1813 assertions passed |
| `git diff --check` | Passed |

Compiled telemetry objects no longer reference `CoCreateGuid`. A source schema
check confirmed 17 application fields plus the standard privacy field, with
SchemaVersion still 1. No ETW recapture or Release rebuild was performed for this
change. No `.gd` files changed, so the parse gate was not rerun. The orchestrator
was intentionally narrowed to the native targets above; live tests/writes were
skipped. Earlier validation below describes the pre-removal schema.

The implementation was built and tested in Debug and Release with
`GODOT_ADDON_SUPPRESS_TELEMETRY=1`; the existing native suite passed 26 cases and 1816
assertions in each configuration. Focused local ETW captures verified the
provider metadata, privacy tag, payload schema, per-addon cycle correlation,
and suppression behavior. A standalone unpackaged Debug export also confirmed
that DevelopmentContext 3 events contain configured PFN and publisher metadata.
The config lookup uses the physical `MicrosoftGame.config` beside the executable
when no successful WithOptions path is retained; it does not rely on the packed
project's potentially relative resource root or use an editor executable's config.

No repository `.gd` files changed. The full orchestrator was narrowed to
affected targets for the metadata-only follow-up; no live-service tests or writes
were run.

| Command | Result |
| --- | --- |
| `cmake --preset default` | Initially blocked by missing pinned godot-cpp; passed after `git submodule update --init --recursive` restored all three required submodules |
| `cmake --build --preset debug --target godot_gdk godot_playfab gdk_unit_tests` | Passed |
| `cmake --preset default-release` | Passed, separate Release tree |
| `cmake --build --preset release --target godot_gdk godot_playfab gdk_unit_tests` | Passed |
| `.\build\bin\Debug\gdk_unit_tests.exe` | 26 cases / 1816 assertions passed |
| `.\build\release\bin\Release\gdk_unit_tests.exe` | 26 cases / 1816 assertions passed |
| `cmake --preset gdk-only` then `cmake --build --preset debug-gdk --target godot_gdk` | Passed, independent GDK Debug |
| `cmake --preset playfab-only` then `cmake --build --preset debug-playfab --target godot_playfab` | Passed, independent PlayFab Debug |
| `cmake --preset gdk-only-release` then `cmake --build --preset release-gdk --target godot_gdk` | Passed, independent GDK Release |
| `cmake --preset playfab-only-release` then `cmake --build --preset release-playfab --target godot_playfab` | Passed, independent PlayFab Release |
| `node --test .\tools\ci\tests\pr_gate_scope.test.cjs` | 34 tests passed using a checksum-verified portable Node 22 runtime because Node was absent from PATH |

`GODOT_ADDON_SUPPRESS_TELEMETRY=1` was set for configure/build/native test runs.
The repository orchestrator was intentionally scoped to the affected GDK and
PlayFab hosts; GameInput is unchanged. With official Godot 4.6.2 selected using
`GODOT_CONSOLE`, this command passed:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\run_all_tests.ps1 -SkipBuild -Hosts tests\godot\gdk,tests\godot\playfab -ParsePaths addons\godot_gdk,addons\godot_playfab,tests\godot\gdk,tests\godot\playfab
```

The parse gate, native suite and all ten bootstrap runners passed. GDK: 339 tests,
306 passing/33 pending, 3235 validated assertions, zero failures. PlayFab: 151
tests, 128 passing/23 pending, 4966 validated assertions, zero failures. Build was
intentionally skipped in this command because the builds above had completed.
No repository `.gd` file was changed; parse still ran as part of this scoped run.
Live-service tests and writes were skipped; neither `-Live` nor
`-AllowLiveWrites` was used.

Non-elevated local ETW captures used a synthetic config/title and official Godot
4.6.2. Combined GDK-first and PlayFab-first runs each produced four events;
GDK reentrant, PlayFab failed-then-successful, GDK-only and PlayFab-only runs each
produced two. `GODOT_ADDON_SUPPRESS_TELEMETRY` values `0`, `false` and whitespace each
produced zero. Sequences were 1/2 per component. These captures predate removal
of the runtime session GUID; the current schema has not been recaptured.
Captured metadata included
the actual engine string, running Gaming Runtime version, real `_GRDK_EDITION`
value, configured PFN/publisher, canonical Xbox/PlayFab title IDs and the exact
`XSystemDeviceType::Pc` value (1). Console/devkit and future device values have
native payload-validation coverage, not console hardware coverage. ETL and
the synthetic harness remain local session artifacts, not public source.

Limitations: registered/packaged controls and loose debug/release export controls
were not executed in this integration run; the package/release exclusion policy
has offline matrix coverage. Fresh scratch-host editor imports intermittently
crashed during teardown (including before the parser replacement); immediate repeated
imports and all lifecycle scenarios passed. The import crash is unresolved, not
claimed fixed or conclusively attributed. A first parse attempt using an ambient
custom Godot 4.5.1 build failed leak-warning checks; the explicit official 4.6.2
run above passed. Other engine versions and backend delivery remain unverified.
