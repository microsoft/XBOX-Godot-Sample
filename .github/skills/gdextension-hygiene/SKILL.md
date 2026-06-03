---
name: gdextension-hygiene
description: >
  Run a Godot/GDExtension finish pass over the current diff: enforce headless GDScript validation,
  public API doc_classes coverage, docs/spec/sample/test synchronization, and registration or
  CMake follow-through. Triggers: hygiene pass, finish pass, validate diff completeness, godot
  addon hygiene, gdextension hygiene, preflight diff, final pass
---

# GDExtension Hygiene Skill

You are the repo-local gdextension-hygiene skill. Your job is to inspect the current diff and make
sure the change is finished cleanly across the Godot-facing surfaces that usually drift apart.

## Core rules

- Start from the current diff or the user-specified files, not from a whole-repo audit.
- Build the checklist from the touched surfaces instead of applying every rule blindly.
- Use the existing repo and path-scoped instructions before inventing new validation steps.
- If the user explicitly wants PR-feedback triage, use `pr-feedback-loop` first.
- If the user explicitly wants risky-diff pressure testing, use `adversarial-review` first.

## Finish-pass checklist

- For `.gd` changes anywhere in the repo, run:

```powershell
pwsh -NoLogo -NoProfile -ExecutionPolicy Bypass -File .\tools\check_gd_scripts_headless.ps1
```

- For `godot_gdk` public contract changes:
  - ensure public classes, methods, properties, signals, and enums stay reflected in
    `addons\godot_gdk\doc_classes\*.xml`
  - ensure the related `docs\godot-gdk-*.md`, `spec\gdext-gdk.md`, sample content, and tests stay
    aligned when behavior or script-visible API changes
- For new native classes or source files:
  - ensure registration and ownership are wired correctly
  - ensure the relevant addon `CMakeLists.txt` includes the new files
- For synced `godot_gdk` addon content under `addons\godot_gdk\`:
  - run the existing debug build step that refreshes the sample copy when required by the
    path-scoped instructions
- For other addons:
  - follow their current build and documentation patterns
  - do not invent `doc_classes` requirements for addons that do not currently ship them

## Workflow

1. Identify the changed surfaces in the diff.
2. Build the minimum finish-pass checklist that applies to those surfaces.
3. Fix the missing follow-through directly when it is concrete and local.
4. Run the narrowest existing validation that proves the change is complete.
5. Report any remaining gaps that need a product or scope decision.

## Output format

Use this structure:

1. **Scope**
2. **Required follow-through**
3. **Gaps fixed**
4. **Remaining gaps**
5. **Validation**
