'use strict';

// Deterministic helpers for the Microsoft GDK release watcher
// (.github/workflows/gdk-release-watch.yml). GitHub cannot deliver a `release`
// event from microsoft/GDK to this repository, so a scheduled job polls the
// upstream release list, keeps a durable bot-owned issue ledger, and dispatches
// the agentic assessor (.github/workflows/gdk-release-assess.md) for one queued
// release at a time.
//
// A release is assessed automatically exactly once. The resulting report is a
// snapshot of the repository at the commit it was produced from, not a
// maintained compatibility claim, so nothing here re-queues a release because
// main moved or upstream edited its notes. A maintainer who wants a fresh
// answer asks for one with `release_tag` + `retry`.
//
// Everything that decides eligibility, identity, ordering, or state lives here
// so it can be unit tested (tools/ci/tests/gdk_release_watch.test.cjs). The
// model never participates in discovery.

const fs = require('node:fs');
const path = require('node:path');

const UPSTREAM_OWNER = 'microsoft';
const UPSTREAM_REPO = 'GDK';
const TARGET_REPO = 'microsoft/XBOX-Godot-Sample';
const TRUSTED_REF = 'refs/heads/main';
const ASSESS_WORKFLOW = 'gdk-release-assess.lock.yml';

// Oldest edition this automation will ever queue. The October 2025 (2510)
// family is deliberately excluded: those editions are missing GDK features the
// addons depend on, so adding them is not a simple supported-list change. Any
// 2510 edition already in the supported lists stays there; the watcher just
// never proposes a new one. Keep in sync with docs/ci/gdk-release-watch.md.
const MINIMUM_EDITION = 260400;

const MARKER_NAME = 'xbox-godot-gdk-release';
const STATE_MARKER_NAME = 'xbox-godot-gdk-release-state';
const DEFAULT_BOT_LOGIN = 'github-actions[bot]';
const TRACKING_LABEL = 'gdk-release';

const LIMITS = Object.freeze({
  maxReleasePages: 5,
  releasesPerPage: 100,
  maxIssueBodyChars: 60000,
  maxNotesChars: 40000,
  maxQueuedPerRun: 1,
  // An assessment that has not reported back well inside this window is dead:
  // the assessor finishes in minutes, and the watcher polls weekly. Detecting
  // that only closes the ledger entry out so the failure is visible; it never
  // starts another model run.
  assessmentTimeoutMs: 6 * 60 * 60 * 1000,
});

const MONTHS = Object.freeze({
  january: 1,
  february: 2,
  march: 3,
  april: 4,
  may: 5,
  june: 6,
  july: 7,
  august: 8,
  september: 9,
  october: 10,
  november: 11,
  december: 12,
});

// `April-2026-Update-5-v2604.5.7903` / `April-2026-v2604.0.7822`.
const TAG_PATTERN = /^([A-Za-z]+)-(20\d{2})(?:-Update-(\d{1,2}))?-v(\d{4})\.(\d{1,2})\.(\d{1,6})$/;
// `GDK_2604.5.7903.zip` is the SDK archive; other assets (remote iteration
// tooling, for example) ship alongside it and are ignored.
const SDK_ASSET_PATTERN = /^GDK_(\d{4})\.(\d{1,2})\.(\d{1,6})\.zip$/i;

// Upstream release titles are free-form text controlled by the publisher and are
// rendered into Markdown tables and into the assessor's prompt. Collapse them to
// a single bounded line so a crafted title cannot break out of a table row or
// introduce a line that reads like an instruction to the agent. Backticks become
// apostrophes so the result is always safe inside a code span; pipes survive and
// are escaped by `tableCell` at the one site that renders a table.
const MAX_TITLE_CHARS = 200;

function sanitizeReleaseTitle(raw) {
  const collapsed = String(raw ?? '')
    .replace(/[\u0000-\u001f\u007f-\u009f\u2028\u2029]/g, ' ')
    .replace(/`/g, "'")
    .replace(/\s+/g, ' ')
    .trim();
  if (collapsed.length <= MAX_TITLE_CHARS) return collapsed;
  return `${collapsed.slice(0, MAX_TITLE_CHARS - 1).trimEnd()}\u2026`;
}

const ASSESSMENT_STATUSES = Object.freeze([
  'awaiting-assessment',
  'assessment-dispatched',
  'assessment-failed',
  'changes-required',
  'tests-only',
  'needs-review',
]);

class WatchError extends Error {}

function statusOf(error) {
  return error && typeof error.status === 'number' ? error.status : undefined;
}

function editionOf(family, update) {
  return `${family}${String(update).padStart(2, '0')}`;
}

// Parses a modern versioned GDK tag. Returns null when the tag is not of that
// shape at all, and `{ conflict }` when it is but its own components disagree —
// a disagreement means we cannot trust the SDK identity and must not guess.
function parseGdkTag(tag) {
  const match = TAG_PATTERN.exec(String(tag || ''));
  if (!match) return null;
  const [, monthName, yearText, updateText, familyText, updateComponent, buildText] = match;
  const month = MONTHS[monthName.toLowerCase()];
  if (!month) return null;
  const year = Number(yearText);
  const family = Number(familyText);
  const update = Number(updateComponent);
  const build = Number(buildText);
  const labelUpdate = updateText === undefined ? 0 : Number(updateText);
  const expectedFamily = (year % 100) * 100 + month;
  if (family !== expectedFamily) {
    return { conflict: `tag names ${monthName} ${year} but carries version family ${family}` };
  }
  if (update !== labelUpdate) {
    return { conflict: `tag names update ${labelUpdate} but carries version update ${update}` };
  }
  if (update > 99) return { conflict: `update ${update} does not map to a 6-digit edition` };
  return {
    version: `${family}.${update}.${build}`,
    family,
    update,
    build,
    edition: editionOf(family, update),
    releaseLabel: `${monthName} ${year}${labelUpdate ? ` Update ${labelUpdate}` : ''}`,
  };
}

function sdkAssets(release) {
  return (release.assets || [])
    .map((asset) => {
      const match = SDK_ASSET_PATTERN.exec(String((asset && asset.name) || ''));
      if (!match) return null;
      return { name: asset.name, version: `${Number(match[1])}.${Number(match[2])}.${Number(match[3])}` };
    })
    .filter(Boolean);
}

// `ignored` releases are permanently out of scope; `conflict` releases are
// reported so a human looks at them instead of being silently dropped.
function classifyRelease(release) {
  const tag = String((release && release.tag_name) || '');
  if (!release || !tag) return { status: 'conflict', reason: 'release has no tag' };
  if (release.draft) return { status: 'ignored', reason: 'draft release' };
  if (release.prerelease) return { status: 'ignored', reason: 'pre-release' };

  const parsed = parseGdkTag(tag);
  if (!parsed) return { status: 'ignored', reason: 'not a versioned public GDK release tag' };
  if (parsed.conflict) return { status: 'conflict', reason: parsed.conflict };

  const name = sanitizeReleaseTitle(release.name);
  if (!/\bGDK\b/i.test(name)) {
    return { status: 'conflict', reason: `release title ${JSON.stringify(name)} does not identify a GDK release` };
  }

  const assets = sdkAssets(release);
  if (assets.length === 0) {
    return { status: 'conflict', reason: 'release publishes no GDK_<version>.zip SDK archive' };
  }
  if (assets.length > 1) {
    return { status: 'conflict', reason: `release publishes ${assets.length} SDK archives` };
  }
  if (assets[0].version !== parsed.version) {
    return { status: 'conflict', reason: `tag version ${parsed.version} disagrees with asset ${assets[0].name}` };
  }

  if (Number(parsed.edition) < MINIMUM_EDITION) {
    return { status: 'ignored', reason: `edition ${parsed.edition} predates the minimum supported edition ${MINIMUM_EDITION}` };
  }

  return {
    status: 'eligible',
    release: {
      id: release.id,
      tag,
      name,
      url: release.html_url,
      publishedAt: release.published_at,
      asset: assets[0].name,
      ...parsed,
    },
  };
}

function parseSupportedEditions(cmakeText) {
  const match = /set\(GDK_SUPPORTED_VERSIONS\s+"([^"]*)"/.exec(String(cmakeText));
  if (!match) throw new WatchError('GDK_SUPPORTED_VERSIONS was not found in cmake/GDKDependencies.cmake.');
  const editions = match[1]
    .split(';')
    .map((value) => value.trim())
    .filter(Boolean);
  if (!editions.length) throw new WatchError('GDK_SUPPORTED_VERSIONS is empty.');
  for (const edition of editions) {
    if (!/^\d{6}$/.test(edition)) throw new WatchError(`GDK_SUPPORTED_VERSIONS contains a non-edition entry: ${edition}`);
  }
  return editions;
}

function parseHostedMatrix(json) {
  if (!json || typeof json !== 'object') throw new WatchError('.github/gdk-versions.json is not an object.');
  if (typeof json.default !== 'string' || !json.default) throw new WatchError('.github/gdk-versions.json has no "default".');
  if (!Array.isArray(json.supported) || !json.supported.length) {
    throw new WatchError('.github/gdk-versions.json has no "supported" entries.');
  }
  const supported = json.supported.map((entry, index) => {
    if (!entry || typeof entry !== 'object') throw new WatchError(`supported[${index}] is not an object.`);
    const parsed = /^(\d{4})\.(\d{1,2})\.(\d{1,6})$/.exec(String(entry.version || ''));
    if (!parsed) throw new WatchError(`supported[${index}].version is not a YYMM.N.build port version.`);
    const expectedEdition = editionOf(Number(parsed[1]), Number(parsed[2]));
    if (String(entry.edition) !== expectedEdition) {
      throw new WatchError(`supported[${index}] maps ${entry.version} to edition ${entry.edition}, expected ${expectedEdition}.`);
    }
    return { version: entry.version, edition: expectedEdition, release: String(entry.release || '') };
  });
  if (!supported.some((entry) => entry.version === json.default)) {
    throw new WatchError(`.github/gdk-versions.json "default" ${json.default} is not in "supported".`);
  }
  return { default: json.default, supported };
}

function parseRegistryBaseline(json) {
  const registry = json && json['default-registry'];
  const baseline = registry && registry.baseline;
  if (!/^[0-9a-f]{40}$/.test(String(baseline || ''))) {
    throw new WatchError('vcpkg-configuration.json does not pin a 40-character default-registry baseline.');
  }
  return baseline;
}

function readJsonFile(root, relative) {
  const file = path.join(root, relative);
  let text;
  try {
    text = fs.readFileSync(file, 'utf8');
  } catch (error) {
    throw new WatchError(`${relative} could not be read: ${error.message}`);
  }
  try {
    return JSON.parse(text);
  } catch (error) {
    throw new WatchError(`${relative} is not valid JSON: ${error.message}`);
  }
}

// The two support lists are independent and are not interchangeable: the CMake
// allowlist governs an SDK installed on disk, the hosted matrix governs vcpkg
// `ms-gdk` port versions that a GitHub-hosted runner can restore.
function readSupportState(root) {
  const cmakePath = path.join(root, 'cmake', 'GDKDependencies.cmake');
  let cmakeText;
  try {
    cmakeText = fs.readFileSync(cmakePath, 'utf8');
  } catch (error) {
    throw new WatchError(`cmake/GDKDependencies.cmake could not be read: ${error.message}`);
  }
  const installedEditions = parseSupportedEditions(cmakeText);
  const hosted = parseHostedMatrix(readJsonFile(root, path.join('.github', 'gdk-versions.json')));
  const baseline = parseRegistryBaseline(readJsonFile(root, 'vcpkg-configuration.json'));
  return {
    installedEditions,
    hosted,
    baseline,
    minimumEdition: String(MINIMUM_EDITION),
  };
}

// Installed-allowlist membership is what "supported" means for this repository.
// A hosted-matrix gap alone is an availability problem, not missing support.
function supportStatusFor(edition, state) {
  const installed = state.installedEditions.includes(edition);
  const hosted = state.hosted.supported.some((entry) => entry.edition === edition);
  if (!installed) return { supported: false, hosted };
  return { supported: true, hosted };
}

function compareReleases(a, b) {
  const editionDelta = Number(a.edition) - Number(b.edition);
  if (editionDelta !== 0) return editionDelta;
  return String(a.id).localeCompare(String(b.id));
}

// Returns the unsupported eligible releases (oldest edition first), the
// already-supported ones, and everything skipped, so the caller can report all
// three without re-deriving them.
function selectBacklog({ releases, state }) {
  const unsupported = [];
  const supported = [];
  const ignored = [];
  const conflicts = [];
  for (const raw of releases || []) {
    const verdict = classifyRelease(raw);
    if (verdict.status === 'ignored') {
      ignored.push({ tag: String((raw && raw.tag_name) || '(untagged)'), reason: verdict.reason });
      continue;
    }
    if (verdict.status === 'conflict') {
      conflicts.push({ tag: String((raw && raw.tag_name) || '(untagged)'), reason: verdict.reason });
      continue;
    }
    const status = supportStatusFor(verdict.release.edition, state);
    const entry = { ...verdict.release, hostedListed: status.hosted };
    if (status.supported) supported.push(entry);
    else unsupported.push(entry);
  }
  unsupported.sort(compareReleases);
  supported.sort(compareReleases);
  return { unsupported, supported, ignored, conflicts };
}

// The nearest already-approved release below the candidate. Assessment must
// diff against this, not against the previous upstream release, because the
// upstream notes are cumulative.
function findSupportBaselineRelease(candidate, supportedReleases) {
  const older = supportedReleases.filter((entry) => Number(entry.edition) < Number(candidate.edition));
  if (!older.length) return null;
  const sameFamily = older.filter((entry) => entry.family === candidate.family);
  const pool = sameFamily.length ? sameFamily : older;
  return pool[pool.length - 1];
}

function markerPrefix(releaseId) {
  return `<!-- ${MARKER_NAME} id=${releaseId} `;
}

function stateMarkerPrefix(releaseId) {
  return `<!-- ${STATE_MARKER_NAME} id=${releaseId} `;
}

function isBotAuthored(entity, botLogin) {
  return Boolean(entity && entity.user && entity.user.login === botLogin);
}

// A pipe splits a GFM table cell even inside a code span, so the one cell that
// carries free-form upstream text escapes it. GFM renders `\|` as a literal `|`.
function tableCell(text) {
  return String(text ?? '').replace(/\|/g, '\\|');
}

function renderTrackingIssueTitle(release) {
  return `GDK ${release.version} (${release.releaseLabel}): assess addon support`;
}

function renderTrackingIssueBody({ release, baselineRelease, state, runUrl }) {
  const lines = [
    `${markerPrefix(release.id)}tag=${release.tag} version=${release.version} edition=${release.edition} -->`,
    `## Microsoft GDK ${release.version} — support assessment`,
    '',
    '> [!NOTE]',
    '> Opened automatically by the GDK release watcher. Support is **not** claimed by this issue;',
    '> it tracks the assessment of what this repository needs in order to support the release.',
    '',
    '| Field | Value |',
    '| --- | --- |',
    `| Upstream release | \`${tableCell(release.name)}\` |`,
    `| Release page | ${release.url} |`,
    `| Tag | \`${release.tag}\` |`,
    `| Port version | \`${release.version}\` |`,
    `| Edition | \`${release.edition}\` |`,
    `| SDK archive | \`${release.asset}\` |`,
    `| Published | ${release.publishedAt || 'unknown'} |`,
    `| Comparison baseline | ${baselineRelease ? `\`${baselineRelease.version}\` (edition \`${baselineRelease.edition}\`)` : '_none below this edition — cross-family baseline needed_'} |`,
    `| Installed allowlist | \`${state.installedEditions.join(';')}\` |`,
    `| Hosted matrix default | \`${state.hosted.default}\` |`,
    '',
    '### Status',
    '',
    '**Awaiting assessment.** No classification has been made yet.',
    '',
    '### What happens next',
    '',
    '1. The watcher dispatches the read-only assessor for this release, once.',
    '2. The assessor posts a report comment classifying the release as',
    '   `changes_required`, `tests_only`, or `needs_review`, together with the checklist for',
    '   turning that report into a change.',
    '3. Assign this issue to Copilot (or pick it up yourself) to do that work. The report is a',
    '   **snapshot** of the commit it names, not a standing claim: re-check current `main`,',
    '   derive the version-list changes against the repository as it is then, and open a draft',
    '   pull request.',
    '4. A human builds and tests against the real SDK before support is merged. No part of',
    '   this automation can validate an SDK, so nothing here is approved support.',
    '',
    'The assessment is not repeated automatically — not when this repository changes, and not',
    'when upstream edits its notes. To get a fresh one, run the **GDK Release Watch** workflow',
    `with \`release_tag: ${release.tag}\` and \`retry: true\`.`,
    '',
    'Closing this issue tells the watcher the release was handled; it is never reopened automatically.',
    '',
    `<sub>Watcher run: ${runUrl}</sub>`,
    '',
  ];
  return lines.join('\n');
}

function renderStateComment({ releaseId, state }) {
  if (!ASSESSMENT_STATUSES.includes(state.status)) {
    throw new WatchError(`Unknown assessment status: ${state.status}`);
  }
  const payload = { ...state, release: releaseId };
  return [
    `${stateMarkerPrefix(releaseId)}status=${state.status} -->`,
    `**Watcher state:** \`${state.status}\`${state.note ? ` — ${state.note}` : ''}`,
    '',
    '<details><summary>Machine-readable state</summary>',
    '',
    '```json',
    JSON.stringify(payload, null, 2),
    '```',
    '',
    '</details>',
    '',
  ].join('\n');
}

function parseStateComment(body) {
  const text = String(body || '');
  if (!text.includes(`<!-- ${STATE_MARKER_NAME} `)) return null;
  const match = /```json\n([\s\S]*?)\n```/.exec(text);
  if (!match) return null;
  try {
    const parsed = JSON.parse(match[1]);
    return parsed && typeof parsed === 'object' && !Array.isArray(parsed) ? parsed : null;
  } catch {
    return null;
  }
}

function latestState(comments, releaseId, botLogin) {
  const prefix = stateMarkerPrefix(releaseId);
  for (let i = comments.length - 1; i >= 0; i -= 1) {
    const comment = comments[i];
    if (!isBotAuthored(comment, botLogin)) continue;
    if (typeof comment.body !== 'string' || !comment.body.startsWith(prefix)) continue;
    const parsed = parseStateComment(comment.body);
    if (parsed) return { state: parsed, comment };
  }
  return null;
}

async function listUpstreamReleases(github, core) {
  const releases = [];
  for (let page = 1; page <= LIMITS.maxReleasePages; page += 1) {
    const { data } = await github.rest.repos.listReleases({
      owner: UPSTREAM_OWNER,
      repo: UPSTREAM_REPO,
      per_page: LIMITS.releasesPerPage,
      page,
    });
    if (!Array.isArray(data)) throw new WatchError('Upstream release listing returned an unexpected payload.');
    releases.push(...data);
    if (data.length < LIMITS.releasesPerPage) return releases;
  }
  // The list is not ordered by version, so a truncated listing can hide a newer
  // release entirely. Say so rather than reporting a confidently empty backlog.
  if (core && typeof core.warning === 'function') {
    core.warning(`Upstream release listing hit the ${LIMITS.maxReleasePages}-page cap; older releases were not inspected.`);
  }
  return releases;
}

// Bot-owned tracking issues are the durable queue. Caches and artifacts expire;
// a version watermark cannot express "this one failed" or "a human closed it".
async function listTrackingIssues({ github, owner, repo, botLogin }) {
  const issues = await github.paginate(github.rest.issues.listForRepo, {
    owner,
    repo,
    state: 'all',
    labels: TRACKING_LABEL,
    per_page: 100,
  });
  const records = new Map();
  for (const issue of issues) {
    if (issue.pull_request) continue;
    if (!isBotAuthored(issue, botLogin)) continue;
    const match = new RegExp(`<!-- ${MARKER_NAME} id=(\\d+) `).exec(String(issue.body || ''));
    if (!match) continue;
    const releaseId = match[1];
    const existing = records.get(releaseId);
    if (!existing || issue.number < existing.number) records.set(releaseId, issue);
  }
  return records;
}

function readWatchInputs(env) {
  const raw = env.GDK_WATCH_INPUTS ? safeParse(env.GDK_WATCH_INPUTS) : {};
  const inputs = raw && typeof raw === 'object' ? raw : {};
  const tag = String(inputs.release_tag || '').trim();
  if (tag && !TAG_PATTERN.test(tag)) throw new WatchError(`release_tag ${JSON.stringify(tag)} is not a GDK release tag.`);
  const retry = inputs.retry === true || inputs.retry === 'true';
  // Retry re-dispatches a paid model run and overrides the terminal-state guard,
  // so it must name exactly one release. Without a tag it would combine with
  // drain_backlog to reassess every already-settled release in the backlog.
  if (retry && !tag) {
    throw new WatchError('retry requires release_tag: name the single release to reassess.');
  }
  return {
    releaseTag: tag || null,
    retry,
    drainBacklog: inputs.drain_backlog === true || inputs.drain_backlog === 'true',
    preview: inputs.preview === true || inputs.preview === 'true',
  };
}

function safeParse(text) {
  try {
    return JSON.parse(text);
  } catch {
    throw new WatchError('GDK_WATCH_INPUTS is not valid JSON.');
  }
}

// Publishing is restricted to the target repository's trusted branch. Forks and
// feature branches still run discovery, but only render a preview.
function evaluateTrustedContext({ context, env }) {
  const repository = `${context.repo.owner}/${context.repo.repo}`;
  const expected = env.GDK_WATCH_TARGET_REPO || TARGET_REPO;
  const ref = env.GITHUB_REF || '';
  if (repository !== expected) return { trusted: false, reason: `repository ${repository} is not ${expected}` };
  if (ref !== TRUSTED_REF) return { trusted: false, reason: `ref ${ref || '(unset)'} is not ${TRUSTED_REF}` };
  return { trusted: true };
}

// `issues.create` does not reliably provision a label that does not exist yet,
// and `listTrackingIssues` finds the durable queue *by* that label. An issue
// created without it is invisible to every later run, which means a duplicate
// issue and a duplicate model run for the same release.
async function ensureTrackingLabel({ github, core, owner, repo }) {
  try {
    await github.rest.issues.getLabel({ owner, repo, name: TRACKING_LABEL });
    return false;
  } catch (error) {
    if (error && error.status !== 404) throw error;
  }
  try {
    await github.rest.issues.createLabel({
      owner,
      repo,
      name: TRACKING_LABEL,
      color: '1d76db',
      description: 'Upstream GDK release tracked by the release watcher',
    });
    core.notice(`Created the \`${TRACKING_LABEL}\` label.`);
    return true;
  } catch (error) {
    // A concurrent run may have won the race; anything else is a real failure.
    if (error && error.status === 422) return false;
    throw error;
  }
}

async function ensureTrackingIssue({ github, core, owner, repo, release, baselineRelease, state, runUrl, existing, publish }) {
  if (existing) return { issue: existing, created: false };
  if (!publish) {
    core.info(`Preview: would open a tracking issue for GDK ${release.version}.`);
    return { issue: null, created: false };
  }
  await ensureTrackingLabel({ github, core, owner, repo });
  const { data } = await github.rest.issues.create({
    owner,
    repo,
    title: renderTrackingIssueTitle(release),
    body: renderTrackingIssueBody({ release, baselineRelease, state, runUrl }),
    labels: [TRACKING_LABEL],
  });
  core.notice(`Opened GDK ${release.version} tracking issue: ${data.html_url}`);
  return { issue: data, created: true };
}

// A release is assessed automatically once: when it has never been assessed, or
// when a human explicitly asks for another look. A completed report is a
// snapshot of the commit it names, so a later change to this repository or to
// the upstream notes does not silently buy another model run — the maintainer
// decides whether a fresh answer is worth one.
function assessmentDecision({ record, retry }) {
  if (!record) return { dispatch: true, reason: 'no tracking issue yet' };
  if (record.issue.state === 'closed') return { dispatch: false, reason: 'tracking issue is closed' };
  const state = record.state;
  if (!state) return { dispatch: true, reason: 'tracking issue has no recorded state' };
  // The in-flight check comes before the retry lever on purpose. Two live
  // attempts for one release race each other through a ledger that has no
  // compare-and-swap: the older one can read the ledger before the retry is
  // recorded, publish, and leave its own terminal state as the latest entry.
  // The retry then finds the release settled, refuses to publish, and nothing
  // ages it out -- the ledger looks finished. Keeping one attempt in flight at
  // a time is what makes that unreachable.
  if (state.status === 'assessment-dispatched') {
    // An attempt whose run already finished without posting is dead, not in
    // flight. Say so on the issue so the failure is visible, but do not start
    // another assessment on our own: a run that died because the agent crashed
    // on this release would otherwise fail the same way every week, unattended.
    // A maintainer asking for it explicitly is a different matter.
    if (record.staleAttempt) {
      if (retry) return { dispatch: true, reason: `explicit retry after ${record.staleAttempt}` };
      return { dispatch: false, reason: `the previous attempt ended as ${record.staleAttempt}`, stalled: record.staleAttempt };
    }
    const inFlight = `an assessment is already in flight (run ${state.runId || 'unknown'})`;
    if (retry) {
      return {
        dispatch: false,
        refusedRetry: true,
        reason: `${inFlight}; retry once it has finished or timed out`,
      };
    }
    return { dispatch: false, reason: inFlight };
  }
  if (retry) return { dispatch: true, reason: 'explicit retry requested' };
  if (state.status === 'assessment-failed') {
    return { dispatch: false, reason: 'the previous assessment failed; dispatch a retry to try again' };
  }
  return { dispatch: false, reason: `already assessed as ${state.status}` };
}

// An `assessment-dispatched` record is only in flight while its attempt could
// still be running. Past that window the attempt failed somewhere the watcher
// cannot observe (dispatch rejected, agent crashed, detection blocked the safe
// output), and the ledger has to stop claiming an assessment is coming.
function staleAttemptStatus(state, now) {
  if (!state || state.status !== 'assessment-dispatched') return null;
  const startedAt = Date.parse(state.at || '');
  if (!Number.isFinite(startedAt)) return 'an attempt with no recorded start time';
  return now - startedAt > LIMITS.assessmentTimeoutMs ? 'a timed-out attempt' : null;
}

async function loadRecords({ github, owner, repo, botLogin, backlog, now = Date.now() }) {
  const issues = await listTrackingIssues({ github, owner, repo, botLogin });
  const records = new Map();
  for (const release of backlog) {
    const issue = issues.get(String(release.id));
    if (!issue) continue;
    const comments = await github.paginate(github.rest.issues.listComments, {
      owner,
      repo,
      issue_number: issue.number,
      per_page: 100,
    });
    const found = latestState(comments, release.id, botLogin);
    const state = found ? found.state : null;
    records.set(String(release.id), { issue, state, staleAttempt: staleAttemptStatus(state, now) });
  }
  return records;
}

// Identifies one dispatch attempt. A retry re-assesses the same release, so
// nothing about the evidence tells two attempts apart; the watcher run that
// queued the work does. The run attempt is part of the key because re-running a
// watcher workflow preserves GITHUB_RUN_ID and only increments
// GITHUB_RUN_ATTEMPT, so the run id alone would hand a re-run the previous
// attempt's identity. This is computed once, at dispatch, and then travels as
// an input so that neither side has to re-derive it from a ledger another run
// may have moved on -- an assessor re-run keeps the id it was dispatched with.
const ATTEMPT_PATTERN = /^[A-Za-z0-9._-]{1,96}$/;

function assessmentAttemptKey({ runId, runAttempt }) {
  const run = String(runId || '').trim();
  const attempt = String(runAttempt || '').trim();
  if (!run) throw new WatchError('An assessment attempt id requires the watcher run id (GITHUB_RUN_ID).');
  const key = attempt ? `${run}.${attempt}` : run;
  if (!ATTEMPT_PATTERN.test(key)) throw new WatchError(`Unusable assessment attempt id: ${key}`);
  return key;
}

async function dispatchAssessment({ github, core, owner, repo, release, issue, attempt, env }) {
  const ref = (env.GITHUB_REF_NAME || 'main').trim();
  await github.rest.actions.createWorkflowDispatch({
    owner,
    repo,
    workflow_id: ASSESS_WORKFLOW,
    ref,
    inputs: {
      release_id: String(release.id),
      release_tag: release.tag,
      issue_number: String(issue.number),
      attempt,
    },
  });
  core.notice(`Dispatched the assessor for GDK ${release.version} (issue #${issue.number}).`);
}

function renderSummary({ backlog, decisions, trusted, preview }) {
  const lines = [
    `- Unsupported releases: ${backlog.unsupported.length}`,
    `- Already supported: ${backlog.supported.length}`,
    `- Skipped: ${backlog.ignored.length}`,
    `- Metadata conflicts: ${backlog.conflicts.length}`,
    `- Publishing: ${trusted && !preview ? 'enabled' : 'preview only'}`,
    '',
  ];
  if (backlog.unsupported.length) {
    // "Hosted matrix" is membership in .github/gdk-versions.json, which is all
    // this column knows. The watcher performs no registry lookup, so it must not
    // imply the port is or is not published in vcpkg; the support step resolves
    // that for the one release it actually proposes.
    lines.push('| Release | Edition | Hosted matrix | Action |', '| --- | --- | --- | --- |');
    for (const release of backlog.unsupported) {
      const decision = decisions.get(String(release.id));
      lines.push(
        `| \`${release.tag}\` | \`${release.edition}\` | ${release.hostedListed ? 'listed' : 'not listed'} | ${decision ? decision.reason : 'pending'} |`,
      );
    }
    lines.push('');
  }
  if (backlog.conflicts.length) {
    lines.push('### Metadata conflicts', '');
    for (const conflict of backlog.conflicts) lines.push(`- \`${conflict.tag}\`: ${conflict.reason}`);
    lines.push('');
  }
  return lines.join('\n');
}

async function runWatch({ github, context, core, env, root }) {
  const { owner, repo } = context.repo;
  const botLogin = env.GDK_WATCH_BOT_LOGIN || DEFAULT_BOT_LOGIN;
  const inputs = readWatchInputs(env);
  const trust = evaluateTrustedContext({ context, env });
  const preview = inputs.preview || !trust.trusted;
  if (!trust.trusted) core.notice(`Preview only: ${trust.reason}.`);

  const state = readSupportState(root);
  const releases = await listUpstreamReleases(github, core);
  const backlog = selectBacklog({ releases, state });
  for (const conflict of backlog.conflicts) {
    core.warning(`Upstream release ${conflict.tag} has inconsistent metadata: ${conflict.reason}`);
  }

  let queue = backlog.unsupported;
  if (inputs.releaseTag) {
    queue = queue.filter((release) => release.tag === inputs.releaseTag);
    if (!queue.length) {
      throw new WatchError(`Release ${inputs.releaseTag} is not an unsupported eligible GDK release.`);
    }
  }

  const records = await loadRecords({ github, owner, repo, botLogin, backlog: queue });
  const runUrl = `${env.GITHUB_SERVER_URL || 'https://github.com'}/${owner}/${repo}/actions/runs/${env.GITHUB_RUN_ID}`;
  const decisions = new Map();
  const ready = [];
  const stalled = [];

  for (const release of queue) {
    const baselineRelease = findSupportBaselineRelease(release, backlog.supported);
    const existing = records.get(String(release.id));
    const ensured = await ensureTrackingIssue({
      github,
      core,
      owner,
      repo,
      release,
      baselineRelease,
      state,
      runUrl,
      existing: existing ? existing.issue : null,
      publish: !preview,
    });
    const record = ensured.issue
      ? { issue: ensured.issue, state: existing ? existing.state : null, staleAttempt: existing ? existing.staleAttempt : null }
      : null;
    const decision = assessmentDecision({ record, retry: inputs.retry });
    decisions.set(String(release.id), decision);
    if (decision.refusedRetry) {
      core.warning(
        `GDK ${release.version}: ${decision.reason}. Nothing was dispatched; a second attempt would race the ` +
          `first one through the tracking issue's ledger.`,
      );
    }
    if (decision.stalled && record) stalled.push({ release, issue: record.issue, reason: decision.stalled });
    if (decision.dispatch && record) ready.push({ release, issue: record.issue });
  }

  ready.sort((a, b) => compareReleases(a.release, b.release));
  const limit = inputs.drainBacklog ? ready.length : LIMITS.maxQueuedPerRun;
  const selected = ready.slice(0, limit);

  if (!preview) {
    // Record the dead attempt so the issue stops claiming an assessment is
    // coming. This closes the ledger entry out; it deliberately does not queue
    // a replacement, because an attempt that died on this release would keep
    // dying on it, unattended, every week.
    for (const item of stalled) {
      await github.rest.issues.createComment({
        owner,
        repo,
        issue_number: item.issue.number,
        body: renderStateComment({
          releaseId: item.release.id,
          state: {
            status: 'assessment-failed',
            runId: env.GITHUB_RUN_ID || null,
            runUrl,
            note: `closed out ${item.reason}; re-run GDK Release Watch with release_tag: ${item.release.tag} and retry: true to assess it again`,
            at: new Date().toISOString(),
          },
        }),
      });
      core.warning(
        `GDK ${item.release.version}: ${item.reason} never reported back. Re-run this workflow with ` +
          `release_tag: ${item.release.tag} and retry: true to assess it again.`,
      );
    }
    for (const item of selected) {
      const attempt = assessmentAttemptKey({ runId: env.GITHUB_RUN_ID, runAttempt: env.GITHUB_RUN_ATTEMPT });
      await github.rest.issues.createComment({
        owner,
        repo,
        issue_number: item.issue.number,
        body: renderStateComment({
          releaseId: item.release.id,
          state: {
            status: 'assessment-dispatched',
            attempt,
            runId: env.GITHUB_RUN_ID || null,
            runUrl,
            at: new Date().toISOString(),
          },
        }),
      });
      try {
        await dispatchAssessment({
          github,
          core,
          owner,
          repo,
          release: item.release,
          issue: item.issue,
          attempt,
          env,
        });
      } catch (error) {
        // The ledger already says "in flight". Leaving it that way after a
        // failed dispatch strands the release until a human forces a retry.
        await github.rest.issues.createComment({
          owner,
          repo,
          issue_number: item.issue.number,
          body: renderStateComment({
            releaseId: item.release.id,
            state: {
              status: 'assessment-failed',
              attempt,
              runId: env.GITHUB_RUN_ID || null,
              runUrl,
              note: `dispatch failed: ${error.message}`,
              at: new Date().toISOString(),
            },
          }),
        });
        throw error;
      }
    }
  } else if (selected.length) {
    core.info(`Preview: would dispatch ${selected.length} assessment(s).`);
  }

  const summary = renderSummary({ backlog, decisions, trusted: trust.trusted, preview });
  if (core.summary) await core.summary.addHeading('GDK release watcher', 2).addRaw(`\n\n${summary}\n`).write();
  core.setOutput('unsupported', String(backlog.unsupported.length));
  core.setOutput('dispatched', String(preview ? 0 : selected.length));
  return { backlog, decisions, dispatched: preview ? [] : selected, stalled: preview ? [] : stalled, preview, summary };
}

module.exports = {
  ASSESSMENT_STATUSES,
  ASSESS_WORKFLOW,
  DEFAULT_BOT_LOGIN,
  LIMITS,
  MARKER_NAME,
  MINIMUM_EDITION,
  STATE_MARKER_NAME,
  TARGET_REPO,
  TRACKING_LABEL,
  TRUSTED_REF,
  UPSTREAM_OWNER,
  UPSTREAM_REPO,
  WatchError,
  ATTEMPT_PATTERN,
  assessmentAttemptKey,
  assessmentDecision,
  classifyRelease,
  compareReleases,
  editionOf,
  evaluateTrustedContext,
  findSupportBaselineRelease,
  latestState,
  listTrackingIssues,
  ensureTrackingLabel,
  listUpstreamReleases,
  markerPrefix,
  parseGdkTag,
  parseHostedMatrix,
  parseRegistryBaseline,
  parseStateComment,
  parseSupportedEditions,
  readSupportState,
  readWatchInputs,
  renderStateComment,
  renderTrackingIssueBody,
  renderTrackingIssueTitle,
  runWatch,
  sanitizeReleaseTitle,
  selectBacklog,
  staleAttemptStatus,
  stateMarkerPrefix,
  statusOf,
  supportStatusFor,
  tableCell,
};
