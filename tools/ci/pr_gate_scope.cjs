'use strict';

// PR gate scope classifier.
//
// Maps the files changed by a pull request or push to the PR-gate work they
// actually affect, so a PlayFab-only change does not pay for the GDK host, a
// docs-only change does not build native addons, and so on. The routing table
// below is the contract; docs/ci/pr-gates.md describes it for humans and
// tools/ci/tests/pr_gate_scope.test.cjs pins it (including a check that every
// tracked file is classified and that the doctest/fuzz maps match CMake).
//
// Rules accumulate. "specific" rules always apply; "exempt" rules (docs and
// repo metadata) only apply when no specific rule matched the path. A path that
// matches neither selects the full gate set and records a fallback reason.

const fs = require('node:fs');
const path = require('node:path');
const { execFileSync } = require('node:child_process');

const COMPONENTS = ['gdk', 'playfab', 'gameinput'];

// Existing CI fuzz targets (tests/cpp/fuzz/CMakeLists.txt). playfab_fuzz_api_models
// needs the PlayFab SDK and is not built in CI, so it is intentionally absent.
const FUZZ_TARGETS = [
  'gdk_fuzz_request_parsing',
  'gdk_fuzz_result_factories',
  'gdk_fuzz_result_formatting',
  'playfab_fuzz_key_lookup',
  'playfab_fuzz_party_codec',
  'playfab_fuzz_request_dictionary',
];
// Targets that link tests/cpp/fuzz_harness (all but the pure key lookup).
const HARNESSED_FUZZ_TARGETS = FUZZ_TARGETS.filter((t) => t !== 'playfab_fuzz_key_lookup');

// Production translation units compiled into gdk_unit_tests (tests/cpp/CMakeLists.txt).
// Basenames without extension; the .cpp and its sibling .h both count.
const DOCTEST_PRODUCTION_SOURCES = [
  'addons/godot_gdk/src/xbox_result_codes_internal',
  'addons/godot_playfab/src/playfab_result_codes_internal',
  'addons/godot_playfab/src/playfab_request_key',
];

// Production sources (and their headers) -> fuzz targets that compile them,
// directly or through a header include.
const FUZZ_PRODUCTION_SOURCES = {
  'addons/godot_playfab/src/playfab_request_key': ['playfab_fuzz_key_lookup'],
  'addons/godot_gdk/src/xbox_result_codes_internal': [
    'gdk_fuzz_result_formatting',
    'gdk_fuzz_result_factories',
    'gdk_fuzz_request_parsing',
  ],
  'addons/godot_playfab/src/playfab_result_codes_internal': [
    'gdk_fuzz_result_formatting',
    'gdk_fuzz_result_factories',
  ],
  'addons/godot_gdk/src/xbox_result': ['gdk_fuzz_result_factories', 'gdk_fuzz_request_parsing'],
  'addons/godot_playfab/src/playfab_result': ['gdk_fuzz_result_factories'],
  'addons/godot_playfab/src/playfab_request_value': ['playfab_fuzz_request_dictionary'],
  'addons/godot_playfab/src/playfab_party_codec': ['playfab_fuzz_party_codec'],
  'addons/godot_gdk/src/xbox_request_parsing': ['gdk_fuzz_request_parsing'],
};

// GDScript parse scopes (repo-relative prefixes) per consumer area.
const PARSE_SCOPES = {
  gdk: [
    'addons/godot_gdk/',
    'sample/tutorial_gdk/',
    'sample/tutorial_gdk_csharp/',
    'sample/tutorial_integrated/',
    'sample/tutorial_integrated_csharp/',
    'tests/godot/gdk/',
  ],
  playfab: [
    'addons/godot_playfab/',
    'sample/tutorial_playfab/',
    'sample/tutorial_playfab_csharp/',
    'sample/tutorial_integrated/',
    'sample/tutorial_integrated_csharp/',
    'tests/godot/playfab/',
    'tests/godot/mp_orchestrator/',
    'tests/godot/mp_test_client/',
  ],
  gameinput: [
    'addons/godot_gameinput/',
    'sample/tutorial_gameinput/',
    'sample/tutorial_gameinput_csharp/',
    'tests/godot/gameinput/',
  ],
  editortools: [
    'addons/godot_gdk_editortools/',
    'sample/tutorial_gdk/',
    'sample/tutorial_gdk_csharp/',
    'sample/tutorial_integrated/',
    'sample/tutorial_integrated_csharp/',
    'tests/godot/gdk/',
  ],
  loadcheck: ['tools/ci/gdextension_load_check.gd'],
};

function emptySelection() {
  return {
    full: false,
    components: new Set(),
    editortools: false,
    doctest: false,
    fuzz: new Set(),
    csharp: false,
    triage: false,
    gdkWatch: false,
    parseFull: false,
    parsePaths: new Set(),
    reasons: [],
    fallback: [],
  };
}

// ── Selection helpers used by rules ─────────────────────────────────────────

function component(sel, name) {
  sel.components.add(name);
  parse(sel, name);
}

function editortools(sel) {
  sel.editortools = true;
  parse(sel, 'editortools');
}

function parse(sel, scope) {
  for (const p of PARSE_SCOPES[scope]) sel.parsePaths.add(p);
}

function allHosts(sel) {
  for (const c of COMPONENTS) component(sel, c);
}

function allFuzz(sel) {
  for (const t of FUZZ_TARGETS) sel.fuzz.add(t);
}

function godotConsumers(sel) {
  allHosts(sel);
  editortools(sel);
  sel.parseFull = true;
}

function full(sel) {
  sel.full = true;
}

function stem(p) {
  return p.replace(/\.(cpp|h)$/, '');
}

const SRC_RE = /^addons\/godot_(gdk|playfab)\/src\/[^/]+\.(cpp|h)$/;

function sharedNativeSources(p, sel) {
  const m = SRC_RE.exec(p);
  if (!m) return;
  const base = stem(p);
  if (DOCTEST_PRODUCTION_SOURCES.includes(base)) sel.doctest = true;
  for (const t of FUZZ_PRODUCTION_SOURCES[base] || []) sel.fuzz.add(t);
}

// playfab_test_base.gd extends gdk_test_base.gd, so the GDK base feeds both hosts.
const BASE_HOSTS = {
  'gdk_test_base.gd': ['gdk', 'playfab'],
  'playfab_test_base.gd': ['playfab'],
  'gameinput_test_base.gd': ['gameinput'],
};

const EDITORTOOLS_HOST_BASES = ['gdk_test_base.gd', 'test_env.gd'];

const EDITORTOOLS_SUITE_RE =
  /^tests\/godot\/gdk\/tests\/(editortools\/|test_editortools\.gd$|fixtures\/packaging\/)/;

function fuzzSourceTarget(file) {
  // fuzz_<area>_<rest>.cpp -> <area>_fuzz_<rest>
  const m = /^fuzz_(gdk|playfab)_(.+)\.cpp$/.exec(file);
  if (!m) return null;
  const target = `${m[1]}_fuzz_${m[2]}`;
  return FUZZ_TARGETS.includes(target) ? target : null;
}

// ── Routing table ───────────────────────────────────────────────────────────
// Each rule: { name, kind, test(path) -> bool, apply(sel, path) }.
// Order only matters for readability; specific rules accumulate.

const pre = (prefix) => (p) => p.startsWith(prefix);
const eq = (...names) => (p) => names.includes(p);
const re = (regex) => (p) => regex.test(p);

const RULES = [
  // Core gate wiring: exercise every gate.
  {
    name: 'PR gate workflow / classifier',
    kind: 'specific',
    test: (p) =>
      p === '.github/workflows/pr-gates.yml' ||
      /^tools\/ci\/pr_gate_[^/]+$/.test(p) ||
      /^tools\/ci\/tests\/pr_gate_[^/]+$/.test(p),
    apply: full,
  },

  // Repo-wide native build inputs: conservative full fan-out.
  {
    name: 'shared native build inputs',
    kind: 'specific',
    test: eq(
      '.gitmodules',
      'godot-cpp',
      'vcpkg.json',
      'vcpkg-configuration.json',
      'CMakeLists.txt',
      'CMakePresets.json',
      'cmake/GodotExtensionCommon.cmake',
      'cmake/AssertGodotCppConfig.cmake',
      'cmake/GodotCppConfigGuard.cmake',
      'cmake/run_doc_source_generator.py',
    ),
    apply: full,
  },
  {
    name: 'GDK SDK version / dependency wiring',
    kind: 'specific',
    test: eq('.github/gdk-versions.json', 'cmake/GDKDependencies.cmake'),
    apply: (sel) => {
      component(sel, 'gdk');
      component(sel, 'playfab');
    },
  },
  {
    name: 'Godot version manifest / setup action',
    kind: 'specific',
    test: (p) => p === '.github/godot-versions.json' || p.startsWith('.github/actions/setup-godot/'),
    apply: godotConsumers,
  },
  {
    name: 'native build action',
    kind: 'specific',
    test: pre('.github/actions/build-addons/'),
    apply: (sel) => {
      allHosts(sel);
      sel.doctest = true;
    },
  },
  {
    name: 'offline tier action / load check',
    kind: 'specific',
    test: (p) =>
      p.startsWith('.github/actions/run-offline-tier/') || p === 'tools/ci/gdextension_load_check.gd',
    apply: (sel) => {
      allHosts(sel);
      parse(sel, 'loadcheck');
    },
  },
  {
    name: 'GDScript parse validator',
    kind: 'specific',
    test: eq('tools/check_gd_scripts_headless.ps1'),
    apply: (sel) => {
      sel.parseFull = true;
    },
  },
  {
    name: 'test orchestrator',
    kind: 'specific',
    test: eq('tools/run_all_tests.ps1'),
    apply: (sel) => {
      allHosts(sel);
      editortools(sel);
    },
  },
  {
    name: 'editor-tool lane host builder',
    kind: 'specific',
    test: eq('tools/ci/prepare_editortools_host.ps1'),
    apply: editortools,
  },
  {
    name: 'GUT pins',
    kind: 'specific',
    test: eq('third_party/Gut', 'third_party/Gut-4.5'),
    apply: (sel) => {
      allHosts(sel);
      editortools(sel);
    },
  },

  // Shared test bases (physically under the GDK addon, mirrored into every host).
  {
    name: 'shared GUT test bases',
    kind: 'specific',
    test: pre('addons/godot_gdk/tests_support/'),
    apply: (sel, p) => {
      const file = path.posix.basename(p);
      const hosts = BASE_HOSTS[file] || COMPONENTS;
      for (const h of hosts) component(sel, h);
      // The native-free editor-tool host stages these two (test_editortools.gd extends the GDK base).
      if (EDITORTOOLS_HOST_BASES.includes(file)) editortools(sel);
    },
  },

  // Addons.
  {
    name: 'GDK addon',
    kind: 'specific',
    test: (p) => p.startsWith('addons/godot_gdk/') && !p.startsWith('addons/godot_gdk/tests_support/'),
    apply: (sel, p) => {
      component(sel, 'gdk');
      if (p.startsWith('addons/godot_gdk/doc_classes/')) sel.csharp = true;
      sharedNativeSources(p, sel);
    },
  },
  {
    name: 'PlayFab addon',
    kind: 'specific',
    test: pre('addons/godot_playfab/'),
    apply: (sel, p) => {
      component(sel, 'playfab');
      if (p.startsWith('addons/godot_playfab/doc_classes/')) sel.csharp = true;
      sharedNativeSources(p, sel);
    },
  },
  {
    name: 'GameInput addon',
    kind: 'specific',
    test: pre('addons/godot_gameinput/'),
    apply: (sel, p) => {
      component(sel, 'gameinput');
      if (p.startsWith('addons/godot_gameinput/doc_classes/')) sel.csharp = true;
    },
  },
  {
    name: 'editor tools addon',
    kind: 'specific',
    test: pre('addons/godot_gdk_editortools/'),
    apply: editortools,
  },
  {
    name: 'managed facades / parity tests',
    kind: 'specific',
    test: (p) =>
      /^addons\/godot_(gdk|playfab|gameinput)_csharp\//.test(p) ||
      p.startsWith('tests/csharp/') ||
      p === 'tools/run_csharp_tests.ps1',
    apply: (sel) => {
      sel.csharp = true;
    },
  },

  // Godot test hosts.
  {
    name: 'editor tool suites / packaging fixtures',
    kind: 'specific',
    test: re(EDITORTOOLS_SUITE_RE),
    apply: editortools,
  },
  {
    name: 'GDK test host',
    kind: 'specific',
    test: (p) => p.startsWith('tests/godot/gdk/') && !EDITORTOOLS_SUITE_RE.test(p),
    apply: (sel, p) => {
      component(sel, 'gdk');
      // Root logos are staged into the editor-tool host too.
      if (/^tests\/godot\/gdk\/[^/]+\.png$/.test(p)) editortools(sel);
    },
  },
  {
    name: 'PlayFab test hosts / multiplayer tooling',
    kind: 'specific',
    test: (p) =>
      p.startsWith('tests/godot/playfab/') ||
      p.startsWith('tests/godot/mp_orchestrator/') ||
      p.startsWith('tests/godot/mp_test_client/') ||
      p === 'tools/run_mp_orchestrator.ps1' ||
      p === 'tools/configure_playfab_test_title.ps1' ||
      p === 'tools/reset_player_data.ps1',
    apply: (sel) => component(sel, 'playfab'),
  },
  {
    name: 'GameInput test host',
    kind: 'specific',
    test: pre('tests/godot/gameinput/'),
    apply: (sel) => component(sel, 'gameinput'),
  },

  // Tutorials (consumers of the addons they enable in project.godot).
  {
    name: 'GDK tutorial',
    kind: 'specific',
    test: re(/^sample\/tutorial_gdk(_csharp)?\//),
    apply: (sel) => {
      component(sel, 'gdk');
      editortools(sel);
    },
  },
  {
    name: 'integrated tutorial',
    kind: 'specific',
    test: re(/^sample\/tutorial_integrated(_csharp)?\//),
    apply: (sel) => {
      component(sel, 'gdk');
      component(sel, 'playfab');
      editortools(sel);
    },
  },
  {
    name: 'PlayFab tutorial',
    kind: 'specific',
    test: re(/^sample\/tutorial_playfab(_csharp)?\//),
    apply: (sel) => component(sel, 'playfab'),
  },
  {
    name: 'GameInput tutorial',
    kind: 'specific',
    test: re(/^sample\/tutorial_gameinput(_csharp)?\//),
    apply: (sel) => component(sel, 'gameinput'),
  },

  // C++ doctest and fuzz inputs.
  {
    name: 'C++ test CMake',
    kind: 'specific',
    test: eq('tests/cpp/CMakeLists.txt'),
    apply: (sel) => {
      sel.doctest = true;
      allFuzz(sel);
    },
  },
  {
    name: 'C++ doctest suites',
    kind: 'specific',
    test: (p) =>
      p === 'tests/cpp/main.cpp' ||
      p.startsWith('tests/cpp/result_codes/') ||
      p.startsWith('tests/cpp/request_key/') ||
      p.startsWith('tests/cpp/third_party/'),
    apply: (sel) => {
      sel.doctest = true;
    },
  },
  {
    name: 'fuzz build / replay wiring',
    kind: 'specific',
    test: eq('tests/cpp/fuzz/CMakeLists.txt', 'tools/run_fuzz_replay.ps1'),
    apply: allFuzz,
  },
  {
    name: 'fuzz harness',
    kind: 'specific',
    test: pre('tests/cpp/fuzz_harness/'),
    apply: (sel) => {
      for (const t of HARNESSED_FUZZ_TARGETS) sel.fuzz.add(t);
    },
  },
  {
    name: 'fuzz corpus',
    kind: 'specific',
    // Only known target dirs match; anything else falls through to the full fallback.
    test: (p) => {
      const m = /^tests\/cpp\/fuzz\/corpus\/([^/]+)\/./.exec(p);
      return !!m && FUZZ_TARGETS.includes(m[1]);
    },
    apply: (sel, p) => sel.fuzz.add(p.split('/')[4]),
  },
  {
    name: 'fuzz target source',
    kind: 'specific',
    test: (p) =>
      /^tests\/cpp\/fuzz\/[^/]+\.cpp$/.test(p) && fuzzSourceTarget(path.posix.basename(p)) !== null,
    apply: (sel, p) => sel.fuzz.add(fuzzSourceTarget(path.posix.basename(p))),
  },

  // Issue triage: covered by the Issue Triage Checks workflow.
  {
    name: 'issue triage helpers / workflows',
    kind: 'specific',
    test: (p) =>
      /^tools\/ci\/issue_triage[^/]*$/.test(p) ||
      /^tools\/ci\/tests\/issue_triage[^/]*$/.test(p) ||
      /^\.github\/workflows\/issue-triage[^/]*$/.test(p) ||
      p.startsWith('.github/skills/issue-triage/') ||
      p.startsWith('tests/evals/issue-triage/') ||
      p.startsWith('.github/aw/'),
    apply: (sel) => {
      sel.triage = true;
    },
  },

  // GDK release watch: selects the `gdk-watch` PR gate, which runs the helper
  // suites and the gh-aw compile-drift check. It is part of the required
  // aggregate, so a failure here cannot hide behind a green `PR gates`.
  {
    name: 'gdk release watch helpers / workflows',
    kind: 'specific',
    test: (p) =>
      /^tools\/ci\/gdk_release_[^/]*$/.test(p) ||
      /^tools\/ci\/tests\/gdk_[^/]*$/.test(p) ||
      /^\.github\/workflows\/gdk-release-[^/]*$/.test(p) ||
      p.startsWith('.github/aw/'),
    apply: (sel) => {
      sel.gdkWatch = true;
    },
  },

  // Scheduled live coverage keeps its own schedule; PRs only lint it.
  {
    name: 'nightly live workflow',
    kind: 'specific',
    test: eq('.github/workflows/playfab-live-nightly.yml'),
    apply: () => {},
  },

  // ── Exemptions: lightweight checks only ───────────────────────────────────
  {
    name: 'documentation / repo metadata',
    kind: 'exempt',
    test: (p) =>
      p.startsWith('docs/') ||
      p.startsWith('spec/') ||
      p.startsWith('tests/baselines/') ||
      p === 'tests/godot/README.md' ||
      p === 'sample/tutorial_netrumble/README.md' ||
      /^[^/]+\.md$/.test(p) ||
      p === 'LICENSE' ||
      p.startsWith('.github/acl/') ||
      p.startsWith('.github/compliance/') ||
      p.startsWith('.github/policies/') ||
      p.startsWith('.github/ISSUE_TEMPLATE/') ||
      p.startsWith('.github/instructions/') ||
      p.startsWith('.github/skills/') ||
      p === '.github/PULL_REQUEST_TEMPLATE.md' ||
      p === '.github/copilot-instructions.md' ||
      p.startsWith('.githooks/') ||
      p.startsWith('.vscode/') ||
      p === '.gitattributes' ||
      p === '.gitignore',
    apply: () => {},
  },
  {
    name: 'local-only developer tools',
    kind: 'exempt',
    test: eq(
      'tools/build_addons.ps1',
      'tools/package_addons.ps1',
      'tools/export_samples.ps1',
      'tools/clean_repo.ps1',
      'tools/migrate_gdk_to_xbox.ps1',
      'tools/setup_sample.ps1',
    ),
    apply: () => {},
  },
];

// ── Classification ──────────────────────────────────────────────────────────

function normalizePath(p) {
  return String(p).replace(/\\/g, '/').replace(/^\.\//, '');
}

function classifyPaths(paths) {
  const sel = emptySelection();
  const seen = new Set();
  for (const raw of paths) {
    const p = normalizePath(raw);
    if (!p || seen.has(p)) continue;
    seen.add(p);
    const specific = RULES.filter((r) => r.kind === 'specific' && r.test(p));
    const matched = specific.length > 0 ? specific : RULES.filter((r) => r.kind === 'exempt' && r.test(p)).slice(0, 1);
    if (matched.length === 0) {
      sel.full = true;
      sel.fallback.push(p);
      sel.reasons.push(`${p}: no routing rule (full fallback)`);
      continue;
    }
    for (const rule of matched) {
      rule.apply(sel, p);
      sel.reasons.push(`${p}: ${rule.name}`);
    }
  }
  return finalize(sel, seen.size);
}

function fullSelection(reason) {
  const sel = emptySelection();
  sel.full = true;
  sel.reasons.push(reason);
  return finalize(sel, 0);
}

function finalize(sel, fileCount) {
  if (sel.full) {
    for (const c of COMPONENTS) sel.components.add(c);
    sel.editortools = true;
    sel.doctest = true;
    allFuzz(sel);
    sel.csharp = true;
    sel.triage = true;
    sel.gdkWatch = true;
    sel.parseFull = true;
  }
  const components = COMPONENTS.filter((c) => sel.components.has(c));
  const parsePaths = sel.parseFull ? [] : [...sel.parsePaths].sort();
  const result = {
    full: sel.full,
    components,
    editortools: sel.editortools,
    doctest: sel.doctest,
    native: components.length > 0 || sel.doctest,
    fuzz_targets: FUZZ_TARGETS.filter((t) => sel.fuzz.has(t)),
    csharp: sel.csharp,
    triage: sel.triage,
    gdk_watch: sel.gdkWatch,
    parse: {
      full: sel.parseFull,
      paths: parsePaths,
    },
    file_count: fileCount,
    fallback: sel.fallback,
    reasons: sel.reasons,
  };
  result.parse.enabled = result.parse.full || result.parse.paths.length > 0;
  return result;
}

// ── git diff ingestion ──────────────────────────────────────────────────────

// Parses `git diff --name-status -z` output (optionally with -M). Rename and copy
// records carry two paths; both sides count as changed.
function parseNameStatusZ(buffer) {
  const parts = String(buffer).split('\0');
  const files = [];
  let i = 0;
  while (i < parts.length) {
    const status = parts[i];
    if (status === '') {
      i += 1;
      continue;
    }
    if (!/^[ACDMRTUXB][0-9]*$/.test(status)) {
      throw new Error(`Unexpected git name-status token: ${JSON.stringify(status)}`);
    }
    const twoPaths = status[0] === 'R' || status[0] === 'C';
    const count = twoPaths ? 2 : 1;
    for (let k = 1; k <= count; k += 1) {
      const p = parts[i + k];
      if (p === undefined || p === '') throw new Error(`Truncated git name-status record for ${status}`);
      files.push(p);
    }
    i += 1 + count;
  }
  return files;
}

function git(args, cwd) {
  return execFileSync('git', args, { cwd, encoding: 'utf8', maxBuffer: 256 * 1024 * 1024, stdio: ['ignore', 'pipe', 'pipe'] });
}

function commitExists(sha, cwd) {
  try {
    git(['cat-file', '-e', `${sha}^{commit}`], cwd);
    return true;
  } catch {
    return false;
  }
}

const ZERO_SHA = /^0+$/;

function diffFiles(base, head, cwd) {
  return parseNameStatusZ(git(['diff', '--name-status', '-z', '-M', base, head, '--'], cwd));
}

function selectForEvent({ event, base, head, before, after, cwd = process.cwd() }) {
  if (event === 'workflow_dispatch' || event === 'dispatch' || event === 'schedule') {
    return fullSelection(`${event}: full gate set`);
  }
  if (event === 'pull_request') {
    if (!base || !head) throw new Error('pull_request needs --base and --head');
    for (const sha of [base, head]) {
      if (!commitExists(sha, cwd)) throw new Error(`pull_request commit ${sha} is not available locally`);
    }
    const mergeBase = git(['merge-base', base, head], cwd).trim();
    return classifyPaths(diffFiles(mergeBase, head, cwd));
  }
  if (event === 'push') {
    if (!after) throw new Error('push needs --after');
    if (!before || ZERO_SHA.test(before)) {
      return fullSelection('push: no previous commit (new branch); full gate set');
    }
    if (!commitExists(before, cwd) || !commitExists(after, cwd)) {
      return fullSelection(`push: ${before}..${after} is not resolvable locally; full gate set`);
    }
    return classifyPaths(diffFiles(before, after, cwd));
  }
  throw new Error(`Unsupported event: ${event}`);
}

// ── Output ──────────────────────────────────────────────────────────────────

function validateResult(r) {
  const fail = (m) => {
    throw new Error(`Malformed selection: ${m}`);
  };
  if (typeof r !== 'object' || r === null) fail('not an object');
  for (const k of ['full', 'editortools', 'doctest', 'native', 'csharp', 'triage', 'gdk_watch']) {
    if (typeof r[k] !== 'boolean') fail(`${k} must be boolean`);
  }
  if (!Array.isArray(r.components) || r.components.some((c) => !COMPONENTS.includes(c))) fail('components');
  if (!Array.isArray(r.fuzz_targets) || r.fuzz_targets.some((t) => !FUZZ_TARGETS.includes(t))) fail('fuzz_targets');
  if (typeof r.parse !== 'object' || typeof r.parse.full !== 'boolean' || !Array.isArray(r.parse.paths)) fail('parse');
  if (r.parse.paths.some((p) => typeof p !== 'string' || p.includes(',') || /[\r\n]/.test(p))) fail('parse.paths');
  if (!Array.isArray(r.reasons)) fail('reasons');
  return r;
}

function toOutputs(r) {
  return {
    full: String(r.full),
    native: String(r.native),
    components: r.components.join(','),
    gdk: String(r.components.includes('gdk')),
    playfab: String(r.components.includes('playfab')),
    gameinput: String(r.components.includes('gameinput')),
    editortools: String(r.editortools),
    doctest: String(r.doctest),
    fuzz: String(r.fuzz_targets.length > 0),
    fuzz_targets: r.fuzz_targets.join(','),
    csharp: String(r.csharp),
    gdk_watch: String(r.gdk_watch),
    parse: String(r.parse.enabled),
    parse_paths: r.parse.full ? '' : r.parse.paths.join(','),
    scope_json: JSON.stringify(r),
  };
}

function renderSummary(r) {
  const yes = (b) => (b ? '✅ selected' : '— skipped');
  const lines = [
    '## PR gate scope',
    '',
    r.full ? '**Full gate set selected.**' : `Classified ${r.file_count} changed file(s).`,
    '',
    '| Gate | Selection |',
    '| --- | --- |',
    `| Native build | ${r.native ? `✅ ${r.components.join(', ') || '(doctest only)'}` : '— skipped'} |`,
    `| Offline GUT hosts | ${r.components.length ? `✅ ${r.components.join(', ')}` : '— skipped'} |`,
    `| Editor tools lane | ${yes(r.editortools)} |`,
    `| C++ doctest | ${yes(r.doctest)} |`,
    `| Fuzz replay | ${r.fuzz_targets.length ? `✅ ${r.fuzz_targets.join(', ')}` : '— skipped'} |`,
    `| C# facade parity | ${yes(r.csharp)} |`,
    `| GDK release watch checks | ${yes(r.gdk_watch)} |`,
    `| GDScript parse | ${r.parse.full ? '✅ all projects' : r.parse.paths.length ? `✅ ${r.parse.paths.length} scope(s)` : '— skipped'} |`,
    `| Issue triage checks | ${r.triage ? 'covered by the Issue Triage Checks workflow' : '— not affected'} |`,
    '',
  ];
  if (r.fallback.length) {
    lines.push('> [!WARNING]', '> These paths have no routing rule, so the full gate set was selected:', '>');
    for (const p of r.fallback) lines.push(`> - \`${p}\``);
    lines.push('', 'Add a rule in `tools/ci/pr_gate_scope.cjs` (see `docs/ci/pr-gates.md`).', '');
  }
  if (r.reasons.length) {
    lines.push('<details><summary>Routing reasons</summary>', '', '```text', ...r.reasons, '```', '', '</details>', '');
  }
  return lines.join('\n');
}

function writeGithubOutputs(outputs, file) {
  const chunks = [];
  for (const [k, v] of Object.entries(outputs)) {
    const delim = `EOF_${k}_${Math.random().toString(36).slice(2)}`;
    chunks.push(`${k}<<${delim}\n${v}\n${delim}\n`);
  }
  fs.appendFileSync(file, chunks.join(''));
}

function parseArgs(argv) {
  const out = {};
  for (let i = 0; i < argv.length; i += 1) {
    const a = argv[i];
    if (!a.startsWith('--')) throw new Error(`Unexpected argument: ${a}`);
    const key = a.slice(2);
    const value = argv[i + 1];
    if (value === undefined || value.startsWith('--')) throw new Error(`Missing value for ${a}`);
    out[key] = value;
    i += 1;
  }
  return out;
}

function main(argv = process.argv.slice(2), env = process.env) {
  const args = parseArgs(argv);
  let result;
  if (args['files-from'] !== undefined) {
    const text = fs.readFileSync(args['files-from'], 'utf8');
    result = classifyPaths(text.split(/\r?\n|\0/).filter(Boolean));
  } else {
    result = selectForEvent({
      event: args.event,
      base: args.base,
      head: args.head,
      before: args.before,
      after: args.after,
    });
  }
  validateResult(result);
  const outputs = toOutputs(result);
  if (env.GITHUB_OUTPUT) writeGithubOutputs(outputs, env.GITHUB_OUTPUT);
  if (env.GITHUB_STEP_SUMMARY) fs.appendFileSync(env.GITHUB_STEP_SUMMARY, `${renderSummary(result)}\n`);
  process.stdout.write(`${JSON.stringify(result, null, 2)}\n`);
  return result;
}

module.exports = {
  COMPONENTS,
  FUZZ_TARGETS,
  HARNESSED_FUZZ_TARGETS,
  DOCTEST_PRODUCTION_SOURCES,
  FUZZ_PRODUCTION_SOURCES,
  PARSE_SCOPES,
  RULES,
  classifyPaths,
  fullSelection,
  parseNameStatusZ,
  selectForEvent,
  validateResult,
  toOutputs,
  renderSummary,
  main,
};

if (require.main === module) {
  try {
    main();
  } catch (err) {
    process.stderr.write(`::error::pr_gate_scope: ${err.message}\n`);
    process.exit(1);
  }
}
