'use strict';

const assert = require('node:assert/strict');
const fs = require('node:fs');
const os = require('node:os');
const path = require('node:path');
const test = require('node:test');
const { execFileSync } = require('node:child_process');

const scope = require('../pr_gate_scope.cjs');

const REPO_ROOT = path.resolve(__dirname, '..', '..', '..');

function classify(...paths) {
  return scope.classifyPaths(paths);
}

function assertNoNative(r) {
  assert.deepEqual(r.components, []);
  assert.equal(r.native, false);
  assert.equal(r.doctest, false);
  assert.deepEqual(r.fuzz_targets, []);
}

// ── Routing table ───────────────────────────────────────────────────────────

test('docs-only change selects no expensive gates', () => {
  const r = classify('docs/ci/pr-gates.md', 'README.md', 'spec/gdext-gdk.md', '.github/copilot-instructions.md');
  assert.equal(r.full, false);
  assertNoNative(r);
  assert.equal(r.editortools, false);
  assert.equal(r.csharp, false);
  assert.equal(r.parse.enabled, false);
  assert.deepEqual(r.fallback, []);
});

test('empty change list is a valid empty selection', () => {
  const r = classify();
  assert.equal(r.full, false);
  assertNoNative(r);
  assert.equal(r.parse.enabled, false);
  scope.validateResult(r);
});

test('PlayFab change selects only PlayFab', () => {
  const r = classify('addons/godot_playfab/src/playfab_users.cpp', 'tests/godot/playfab/tests/test_users.gd');
  assert.deepEqual(r.components, ['playfab']);
  assert.equal(r.editortools, false);
  assert.equal(r.doctest, false);
  assert.deepEqual(r.fuzz_targets, []);
  assert.ok(r.parse.paths.includes('addons/godot_playfab/'));
  assert.ok(!r.parse.paths.some((p) => p.startsWith('addons/godot_gdk')));
  assert.ok(!r.parse.paths.includes('tests/godot/gdk/'));
});

test('GameInput change selects only GameInput', () => {
  const r = classify('addons/godot_gameinput/src/gameinput_device.cpp', 'sample/tutorial_gameinput/main.gd');
  assert.deepEqual(r.components, ['gameinput']);
  assert.equal(r.editortools, false);
  assert.deepEqual(r.parse.paths, scope.PARSE_SCOPES.gameinput.slice().sort());
});

test('GDK change selects GDK without PlayFab or GameInput', () => {
  const r = classify('addons/godot_gdk/src/xbox_users.cpp', 'tests/godot/gdk/tests/test_users.gd');
  assert.deepEqual(r.components, ['gdk']);
  assert.equal(r.csharp, false);
});

test('editor tools changes are native-free', () => {
  for (const p of [
    'addons/godot_gdk_editortools/editor/service.gd',
    'tests/godot/gdk/tests/editortools/test_editortools_service.gd',
    'tests/godot/gdk/tests/test_editortools.gd',
    'tests/godot/gdk/tests/fixtures/packaging/MicrosoftGame.config',
  ]) {
    const r = classify(p);
    assertNoNative(r);
    assert.equal(r.editortools, true, p);
    assert.equal(r.full, false, p);
  }
});

test('tutorials route to the addons their project enables', () => {
  assert.deepEqual(classify('sample/tutorial_gdk/main.gd').components, ['gdk']);
  assert.equal(classify('sample/tutorial_gdk_csharp/Main.cs').editortools, true);
  const integrated = classify('sample/tutorial_integrated/main.gd');
  assert.deepEqual(integrated.components, ['gdk', 'playfab']);
  assert.equal(integrated.editortools, true);
  assert.deepEqual(classify('sample/tutorial_integrated_csharp/Main.cs').components, ['gdk', 'playfab']);
  const playfab = classify('sample/tutorial_playfab_csharp/Main.cs');
  assert.deepEqual(playfab.components, ['playfab']);
  assert.equal(playfab.editortools, false);
  assert.deepEqual(classify('sample/tutorial_gameinput_csharp/Main.cs').components, ['gameinput']);
});

test('shared test bases select the hosts that inherit them', () => {
  const base = 'addons/godot_gdk/tests_support/bases/';
  // playfab_test_base.gd extends gdk_test_base.gd, so the GDK base feeds both hosts.
  assert.deepEqual(classify(`${base}gdk_test_base.gd`).components, ['gdk', 'playfab']);
  assert.deepEqual(classify(`${base}playfab_test_base.gd`).components, ['playfab']);
  assert.deepEqual(classify(`${base}gameinput_test_base.gd`).components, ['gameinput']);
  assert.deepEqual(classify(`${base}test_env.gd`).components, ['gdk', 'playfab', 'gameinput']);
  // The editor-tool host stages the GDK base + TestEnv; other bases do not touch it.
  assert.equal(classify(`${base}gdk_test_base.gd`).editortools, true);
  assert.equal(classify(`${base}test_env.gd`).editortools, true);
  assert.equal(classify(`${base}playfab_test_base.gd`).editortools, false);
  assert.equal(classify(`${base}gameinput_test_base.gd`).editortools, false);
});

test('BASE_HOSTS covers every base that inherits another base', () => {
  const dir = path.join(REPO_ROOT, 'addons', 'godot_gdk', 'tests_support', 'bases');
  const bases = fs.readdirSync(dir).filter((f) => f.endsWith('.gd'));
  for (const file of bases) {
    const m = /^extends\s+"res:\/\/addons\/godot_gdk_tests\/([^"]+)"/m.exec(fs.readFileSync(path.join(dir, file), 'utf8'));
    if (!m) continue;
    const parentHosts = classify(`addons/godot_gdk/tests_support/bases/${m[1]}`).components;
    const childHosts = classify(`addons/godot_gdk/tests_support/bases/${file}`).components;
    for (const h of childHosts) assert.ok(parentHosts.includes(h), `${m[1]} must also select ${h} (inherited by ${file})`);
  }
});

test('PlayFab multiplayer tooling stays PlayFab-only', () => {
  for (const p of [
    'tests/godot/mp_orchestrator/scenarios/c1.gd',
    'tests/godot/mp_test_client/main.gd',
    'tools/run_mp_orchestrator.ps1',
  ]) {
    assert.deepEqual(classify(p).components, ['playfab'], p);
  }
});

test('native doc_classes also select the managed parity gate', () => {
  const r = classify('addons/godot_playfab/doc_classes/PlayFab.xml');
  assert.deepEqual(r.components, ['playfab']);
  assert.equal(r.csharp, true);
  const managed = classify('addons/godot_gdk_csharp/Gdk.cs', 'tests/csharp/FacadeParity.Tests/ParityTests.cs');
  assertNoNative(managed);
  assert.equal(managed.csharp, true);
});

test('doctest/fuzz production sources select shared C++ gates plus the owner', () => {
  const r = classify('addons/godot_gdk/src/xbox_result_codes_internal.h');
  assert.deepEqual(r.components, ['gdk']);
  assert.equal(r.doctest, true);
  assert.deepEqual(r.fuzz_targets, [
    'gdk_fuzz_request_parsing',
    'gdk_fuzz_result_factories',
    'gdk_fuzz_result_formatting',
  ]);
  const key = classify('addons/godot_playfab/src/playfab_request_key.cpp');
  assert.deepEqual(key.components, ['playfab']);
  assert.equal(key.doctest, true);
  assert.deepEqual(key.fuzz_targets, ['playfab_fuzz_key_lookup']);
});

test('C++ test inputs select only doctest/fuzz work', () => {
  const dt = classify('tests/cpp/result_codes/test_gdk_result_codes.cpp');
  assert.equal(dt.doctest, true);
  assert.deepEqual(dt.components, []);
  assert.deepEqual(dt.fuzz_targets, []);

  const corpus = classify('tests/cpp/fuzz/corpus/playfab_fuzz_party_codec/seed1');
  assert.equal(corpus.doctest, false);
  assert.deepEqual(corpus.fuzz_targets, ['playfab_fuzz_party_codec']);

  assert.deepEqual(classify('tests/cpp/fuzz/fuzz_gdk_request_parsing.cpp').fuzz_targets, ['gdk_fuzz_request_parsing']);
  assert.deepEqual(classify('tests/cpp/fuzz_harness/stub_string.cpp').fuzz_targets, scope.HARNESSED_FUZZ_TARGETS);
  assert.deepEqual(classify('tools/run_fuzz_replay.ps1').fuzz_targets, scope.FUZZ_TARGETS);
  const cm = classify('tests/cpp/CMakeLists.txt');
  assert.equal(cm.doctest, true);
  assert.deepEqual(cm.fuzz_targets, scope.FUZZ_TARGETS);
});

test('unknown fuzz corpus dirs and sources fall back to full', () => {
  for (const p of [
    'tests/cpp/fuzz/corpus/playfab_fuzz_party_codex/seed1',
    'tests/cpp/fuzz/corpus/README.md',
    'tests/cpp/fuzz/fuzz_not_a_real_target.cpp',
  ]) {
    const r = classify(p);
    assert.equal(r.full, true, p);
    assert.deepEqual(r.fallback, [p], p);
  }
});

test('mixed fuzz targets stay mixed', () => {
  const r = classify(
    'tests/cpp/fuzz/corpus/playfab_fuzz_key_lookup/a',
    'tests/cpp/fuzz/corpus/gdk_fuzz_result_formatting/b',
  );
  assert.deepEqual(r.fuzz_targets, ['gdk_fuzz_result_formatting', 'playfab_fuzz_key_lookup']);
});

test('CI inputs fan out to their consumers', () => {
  assert.deepEqual(classify('.github/gdk-versions.json').components, ['gdk', 'playfab']);

  const godot = classify('.github/godot-versions.json');
  assert.deepEqual(godot.components, ['gdk', 'playfab', 'gameinput']);
  assert.equal(godot.editortools, true);
  assert.equal(godot.parse.full, true);
  assert.equal(godot.csharp, false);
  assert.deepEqual(godot.fuzz_targets, []);

  const build = classify('.github/actions/build-addons/action.yml');
  assert.deepEqual(build.components, ['gdk', 'playfab', 'gameinput']);
  assert.equal(build.doctest, true);

  const offline = classify('.github/actions/run-offline-tier/action.yml');
  assert.deepEqual(offline.components, ['gdk', 'playfab', 'gameinput']);
  assert.equal(offline.editortools, false);

  const parse = classify('tools/check_gd_scripts_headless.ps1');
  assertNoNative(parse);
  assert.equal(parse.parse.full, true);

  const gut = classify('third_party/Gut');
  assert.deepEqual(gut.components, ['gdk', 'playfab', 'gameinput']);
  assert.equal(gut.editortools, true);

  const runner = classify('tools/run_all_tests.ps1');
  assert.equal(runner.editortools, true);
  assert.equal(runner.full, false);
});

test('core gate wiring and shared native inputs select everything', () => {
  for (const p of [
    '.github/workflows/pr-gates.yml',
    'tools/ci/pr_gate_scope.cjs',
    'tools/ci/tests/pr_gate_scope.test.cjs',
    'CMakeLists.txt',
    'cmake/GodotExtensionCommon.cmake',
    'godot-cpp',
    'vcpkg.json',
    '.gitmodules',
  ]) {
    const r = classify(p);
    assert.equal(r.full, true, p);
    assert.deepEqual(r.fallback, [], p);
    assert.deepEqual(r.fuzz_targets, scope.FUZZ_TARGETS, p);
    assert.equal(r.parse.full, true, p);
  }
});

test('nightly and triage changes select only lightweight work', () => {
  const nightly = classify('.github/workflows/playfab-live-nightly.yml');
  assert.equal(nightly.full, false);
  assertNoNative(nightly);

  const triage = classify(
    'tools/ci/issue_triage.cjs',
    'tools/ci/tests/issue_triage.test.cjs',
    '.github/workflows/issue-triage.lock.yml',
    '.github/skills/issue-triage/SKILL.md',
    'tests/evals/issue-triage/cases/a.json',
    '.github/aw/actions-lock.json',
  );
  assert.equal(triage.full, false);
  assertNoNative(triage);
  assert.equal(triage.triage, true);
  assert.equal(triage.parse.enabled, false);
});

test('gdk release watch changes select the gdk-watch gate and nothing heavier', () => {
  const watch = classify(
    'tools/ci/gdk_release_watch.cjs',
    'tools/ci/gdk_release_assess.cjs',
    'tools/ci/tests/gdk_release_assess.test.cjs',
    '.github/workflows/gdk-release-watch.yml',
    '.github/workflows/gdk-release-assess.lock.yml',
  );
  assert.equal(watch.full, false, 'the gdk-watch gate covers these paths');
  assert.equal(watch.gdk_watch, true, 'the required aggregate must see a gdk-watch result');
  assertNoNative(watch);
  assert.equal(watch.parse.enabled, false);
  assert.deepEqual(watch.fallback, []);
});

test('the shared gh-aw action lock selects both agentic check gates', () => {
  const aw = classify('.github/aw/actions-lock.json');
  assert.equal(aw.triage, true);
  assert.equal(aw.gdk_watch, true);
  assert.equal(aw.full, false);
});

test('unrelated changes leave the gdk-watch gate unselected', () => {
  assert.equal(classify('addons/godot_gdk/src/gdk.cpp').gdk_watch, false);
});

test('specific rules beat documentation exemptions', () => {
  assert.deepEqual(classify('addons/godot_gdk/README.md').components, ['gdk']);
  assert.equal(classify('.github/skills/issue-triage/SKILL.md').triage, true);
  assert.equal(classify('.github/skills/other/SKILL.md').triage, false);
});

test('unknown paths select the full gate set and are reported', () => {
  const r = classify('docs/x.md', 'tools/brand_new_tool.ps1', 'sample/tutorial_netrumble/project.godot');
  assert.equal(r.full, true);
  assert.deepEqual(r.fallback, ['tools/brand_new_tool.ps1', 'sample/tutorial_netrumble/project.godot']);
  assert.match(scope.renderSummary(r), /no routing rule/);
});

test('paths are normalized and deduplicated', () => {
  const r = classify('addons\\godot_playfab\\src\\a.cpp', './addons/godot_playfab/src/a.cpp');
  assert.equal(r.file_count, 1);
  assert.deepEqual(r.components, ['playfab']);
});

// ── Every tracked file has a route ──────────────────────────────────────────

test('every tracked file is classified without fallback', () => {
  const files = execFileSync('git', ['ls-files', '-z'], { cwd: REPO_ROOT, encoding: 'utf8', maxBuffer: 64 * 1024 * 1024 })
    .split('\0')
    .filter(Boolean);
  assert.ok(files.length > 100);
  const unknown = files.filter((f) => scope.classifyPaths([f]).fallback.length > 0);
  assert.deepEqual(unknown, [], 'add a routing rule for these tracked paths');
});

// ── CMake / include-graph sync ──────────────────────────────────────────────

function readRepo(rel) {
  return fs.readFileSync(path.join(REPO_ROOT, rel), 'utf8');
}

function resolveCMakePath(raw, dir) {
  return raw
    .replace(/\$\{_PF_SRC\}|\$\{_PF_RC_SRC\}/g, 'addons/godot_playfab/src')
    .replace(/\$\{_GDK_SRC\}|\$\{_GDK_RC_SRC\}/g, 'addons/godot_gdk/src')
    .replace(/\$\{CMAKE_SOURCE_DIR\}\//g, '')
    .replace(/\$\{CMAKE_CURRENT_SOURCE_DIR\}/g, dir);
}

function cmakeTargets(rel, fnName) {
  const text = readRepo(rel);
  const dir = path.posix.dirname(rel);
  const targets = {};
  const blockRe = new RegExp(`${fnName}\\(([\\s\\S]*?)\\n\\s*\\)`, 'g');
  for (const m of text.matchAll(blockRe)) {
    const body = m[1];
    const name = /TARGET_NAME\s+(\S+)/.exec(body)[1];
    const sources = [...body.matchAll(/"([^"]+\.cpp)"/g)].map((s) => resolveCMakePath(s[1], dir));
    targets[name] = sources;
  }
  return targets;
}

// Local `#include "x.h"` closure, resolving against the including file's folder
// and the addon src folders.
function includeClosure(files) {
  const searchDirs = ['addons/godot_gdk/src', 'addons/godot_playfab/src', 'tests/cpp/fuzz_harness'];
  const seen = new Set();
  const queue = [...files];
  while (queue.length) {
    const f = queue.pop();
    if (seen.has(f) || !fs.existsSync(path.join(REPO_ROOT, f))) continue;
    seen.add(f);
    for (const m of readRepo(f).matchAll(/#include\s+"([^"]+)"/g)) {
      const candidates = [path.posix.join(path.posix.dirname(f), m[1]), ...searchDirs.map((d) => `${d}/${m[1]}`)];
      const hit = candidates.find((c) => fs.existsSync(path.join(REPO_ROOT, c)));
      if (hit) queue.push(path.posix.normalize(hit));
    }
    if (f.endsWith('.cpp')) {
      const h = f.replace(/\.cpp$/, '.h');
      if (fs.existsSync(path.join(REPO_ROOT, h))) queue.push(h);
    }
  }
  return [...seen];
}

test('doctest production sources match tests/cpp/CMakeLists.txt and their includes', () => {
  const targets = cmakeTargets('tests/cpp/CMakeLists.txt', 'godot_addon_doctest_target');
  assert.deepEqual(Object.keys(targets), ['gdk_unit_tests']);
  const sources = targets.gdk_unit_tests;
  const production = sources.filter((s) => s.startsWith('addons/')).map((s) => s.replace(/\.cpp$/, ''));
  assert.deepEqual(production.sort(), scope.DOCTEST_PRODUCTION_SOURCES.slice().sort());

  const impl = 'tests/cpp/main.cpp';
  for (const f of includeClosure([impl, ...sources])) {
    const r = scope.classifyPaths([f]);
    assert.equal(r.doctest, true, `${f} feeds gdk_unit_tests but does not select doctest`);
  }
});

test('fuzz routing matches tests/cpp/fuzz/CMakeLists.txt and their includes', () => {
  const targets = cmakeTargets('tests/cpp/fuzz/CMakeLists.txt', 'godot_addon_fuzzer_target');
  delete targets.playfab_fuzz_api_models;
  assert.deepEqual(Object.keys(targets).sort(), scope.FUZZ_TARGETS.slice().sort());
  const harnessed = Object.entries(targets)
    .filter(([, srcs]) => includeClosure(srcs).some((f) => f.startsWith('tests/cpp/fuzz_harness/')))
    .map(([t]) => t)
    .sort();
  const harnessText = readRepo('tests/cpp/fuzz/CMakeLists.txt');
  for (const t of scope.HARNESSED_FUZZ_TARGETS) {
    assert.match(harnessText, new RegExp(`TARGET_NAME ${t}[\\s\\S]*?godot_stub_harness`), `${t} links the harness`);
  }
  assert.ok(harnessed.every((t) => scope.HARNESSED_FUZZ_TARGETS.includes(t)));

  for (const [target, sources] of Object.entries(targets)) {
    for (const f of includeClosure(sources)) {
      const r = scope.classifyPaths([f]);
      assert.ok(r.fuzz_targets.includes(target), `${f} feeds ${target} but does not select it`);
    }
    assert.ok(fs.existsSync(path.join(REPO_ROOT, 'tests/cpp/fuzz/corpus', target)), `${target} has a corpus dir`);
  }
});

// ── git diff ingestion ──────────────────────────────────────────────────────

test('parseNameStatusZ handles renames, copies, deletes, and spaces', () => {
  const raw = ['M', 'a b/c.gd', 'R087', 'old/x.cpp', 'new dir/x.cpp', 'D', 'gone.txt', 'C100', 's', 'd', 'A', 'n', ''].join('\0');
  assert.deepEqual(scope.parseNameStatusZ(raw), ['a b/c.gd', 'old/x.cpp', 'new dir/x.cpp', 'gone.txt', 's', 'd', 'n']);
  assert.throws(() => scope.parseNameStatusZ('M\0'), /Truncated/);
  assert.throws(() => scope.parseNameStatusZ('bogus\0x\0'), /Unexpected/);
});

function makeRepo() {
  const dir = fs.mkdtempSync(path.join(os.tmpdir(), 'pr-gate-scope-'));
  const g = (...args) => execFileSync('git', args, { cwd: dir, encoding: 'utf8' }).trim();
  g('init', '-q', '-b', 'main');
  g('config', 'user.email', 't@example.com');
  g('config', 'user.name', 't');
  g('config', 'commit.gpgsign', 'false');
  const write = (rel, text) => {
    fs.mkdirSync(path.dirname(path.join(dir, rel)), { recursive: true });
    fs.writeFileSync(path.join(dir, rel), text);
  };
  return { dir, g, write };
}

test('selectForEvent diffs PRs from the merge base and includes both rename sides', (t) => {
  const { dir, g, write } = makeRepo();
  t.after(() => fs.rmSync(dir, { recursive: true, force: true }));
  write('addons/godot_gameinput/src/a.cpp', 'x\n'.repeat(20));
  write('docs/readme.md', 'd');
  g('add', '-A');
  g('commit', '-qm', 'base');
  const base0 = g('rev-parse', 'HEAD');

  g('checkout', '-qb', 'feature');
  fs.mkdirSync(path.join(dir, 'addons/godot_playfab/src'), { recursive: true });
  g('mv', 'addons/godot_gameinput/src/a.cpp', 'addons/godot_playfab/src/a b.cpp');
  g('commit', '-qm', 'rename');
  const head = g('rev-parse', 'HEAD');

  // Advance main with an unrelated GDK change; the merge-base diff must ignore it.
  g('checkout', '-q', 'main');
  write('addons/godot_gdk/src/z.cpp', 'z');
  g('add', '-A');
  g('commit', '-qm', 'main moves');
  const base = g('rev-parse', 'HEAD');

  const r = scope.selectForEvent({ event: 'pull_request', base, head, cwd: dir });
  assert.deepEqual(r.components, ['playfab', 'gameinput']);
  assert.equal(r.file_count, 2);

  const push = scope.selectForEvent({ event: 'push', before: base0, after: head, cwd: dir });
  assert.deepEqual(push.components, ['playfab', 'gameinput']);
});

test('selectForEvent falls back to full selection for unusable push history', (t) => {
  const { dir, g, write } = makeRepo();
  t.after(() => fs.rmSync(dir, { recursive: true, force: true }));
  write('docs/a.md', 'a');
  g('add', '-A');
  g('commit', '-qm', 'a');
  const head = g('rev-parse', 'HEAD');

  const created = scope.selectForEvent({ event: 'push', before: '0'.repeat(40), after: head, cwd: dir });
  assert.equal(created.full, true);
  assert.match(created.reasons[0], /new branch/);

  const missing = scope.selectForEvent({ event: 'push', before: 'f'.repeat(40), after: head, cwd: dir });
  assert.equal(missing.full, true);
  assert.match(missing.reasons[0], /not resolvable/);

  const dispatch = scope.selectForEvent({ event: 'workflow_dispatch', cwd: dir });
  assert.equal(dispatch.full, true);
  assert.deepEqual(dispatch.components, ['gdk', 'playfab', 'gameinput']);

  assert.throws(() => scope.selectForEvent({ event: 'pull_request', base: 'f'.repeat(40), head, cwd: dir }), /not available/);
  assert.throws(() => scope.selectForEvent({ event: 'issue_comment', cwd: dir }), /Unsupported/);
});

// ── Outputs ─────────────────────────────────────────────────────────────────

test('validateResult rejects malformed selections', () => {
  const good = classify('addons/godot_playfab/src/a.cpp');
  scope.validateResult(good);
  assert.throws(() => scope.validateResult({ ...good, components: ['xbox'] }), /components/);
  assert.throws(() => scope.validateResult({ ...good, doctest: 'yes' }), /doctest/);
  assert.throws(() => scope.validateResult({ ...good, parse: { full: false, paths: ['a,b'] } }), /parse.paths/);
  assert.throws(() => scope.validateResult({ ...good, fuzz_targets: ['nope'] }), /fuzz_targets/);
});

test('main writes GitHub outputs and a step summary', (t) => {
  const tmp = fs.mkdtempSync(path.join(os.tmpdir(), 'pr-gate-out-'));
  t.after(() => fs.rmSync(tmp, { recursive: true, force: true }));
  const list = path.join(tmp, 'files.txt');
  fs.writeFileSync(list, 'addons/godot_playfab/src/playfab_party_codec.cpp\ntests/cpp/fuzz/corpus/playfab_fuzz_key_lookup/x\n');
  const env = { GITHUB_OUTPUT: path.join(tmp, 'out'), GITHUB_STEP_SUMMARY: path.join(tmp, 'sum') };
  const origWrite = process.stdout.write;
  process.stdout.write = () => true;
  try {
    scope.main(['--files-from', list], env);
  } finally {
    process.stdout.write = origWrite;
  }
  const out = fs.readFileSync(env.GITHUB_OUTPUT, 'utf8');
  const get = (k) => new RegExp(`^${k}<<(\\S+)\\n([\\s\\S]*?)\\n\\1$`, 'm').exec(out)[2];
  assert.equal(get('components'), 'playfab');
  assert.equal(get('gdk'), 'false');
  assert.equal(get('playfab'), 'true');
  assert.equal(get('native'), 'true');
  assert.equal(get('doctest'), 'false');
  assert.equal(get('fuzz_targets'), 'playfab_fuzz_key_lookup,playfab_fuzz_party_codec');
  assert.equal(get('parse'), 'true');
  assert.ok(get('parse_paths').split(',').includes('addons/godot_playfab/'));
  assert.equal(JSON.parse(get('scope_json')).components[0], 'playfab');
  assert.match(fs.readFileSync(env.GITHUB_STEP_SUMMARY, 'utf8'), /PR gate scope/);
});
