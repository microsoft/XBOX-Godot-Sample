<!--
This template is auto-applied by GitHub when a PR is opened. Fill it in honestly
— the agent workflow expects each section to be present and substantive. If a
section truly does not apply, write "Not applicable: <reason>" rather than
deleting it.
-->

## Summary

<!-- 1–3 lines. Name the branch and the user-visible change. -->

## Public API changes

<!--
Every renamed/added GDScript-visible class, method, property, signal, or enum
goes here, with a short GDScript usage example for each. If the PR adds no
public API, write "None".
-->

```gdscript
# Example for a new API:
# var result := await PlayFab.users.sign_in_with_custom_id_async("my-id").completed
```

## Spec / docs / samples updated

<!-- Tick the boxes that apply to the touched surfaces. -->

- [ ] `spec\gdext-<feature>.md` updated (Plan / Progress sections if multi-session work)
- [ ] `docs\godot-<addon>-*.md` updated
- [ ] `addons\<addon>\doc_classes\*.xml` updated for any public API change
- [ ] Sample content (`sample\<host>\`) updated when public addon behaviour changed
- [ ] Path-scoped instruction file (`.github\instructions\<addon>.instructions.md`) updated if conventions changed
- [ ] Not applicable — no public addon behaviour changed

## Test coverage delta

<!--
State how the test pipeline moved. Use the per-tier counts from the orchestrator
summary (`build\test-results\run-summary.md`).
-->

- Contract (offline) tests added/removed: …
- Live-read tests added/removed: …
- Live-write tests added/removed: …
- Live title id used (if any): …

## Validation run

<!--
The commands actually executed for this change. Paste the trailing summary
line(s) where possible. Do not skip — reviewers should not have to guess what
was run locally.
-->

```text
# Example:
# pwsh ...\check_gd_scripts_headless.ps1   -> PASS (52 files)
# pwsh ...\run_all_tests.ps1               -> PASS (parse, build, doctest, GUT, bootstrap)
# pwsh ...\run_all_tests.ps1 -Live         -> PASS (live reads only; live writes pending)
```

## Migration notes

<!--
Only if this PR breaks the public surface or changes a Project Setting key.
Otherwise write "None".
-->

None.
