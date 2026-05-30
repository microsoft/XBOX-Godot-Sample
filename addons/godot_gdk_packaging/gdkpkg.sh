#!/usr/bin/env bash
# ============================================================================
#  gdkpkg.sh — Bash shell forwarder for the godot_gdk_packaging runner.
#
#  Usage:
#      addons/godot_gdk_packaging/gdkpkg.sh <verb> [--flag value] [...]
#
#  Behaviour:
#    * Locates a Godot 4 executable via:
#        1. GODOT_CONSOLE / GODOT_BIN / GODOT environment variables
#        2. <script-dir>/../../sample/Godot* (repo dev layout)
#        3. $PWD/Godot* (current project directory)
#        4. `which godot` / `which godot4`
#    * Defaults the Godot project to the current working directory. Pass
#      `--path <dir>` (passthrough flag) or `--godot <path>` (consumed
#      here) to override.
#    * Forwards every other argument to `run.gd` via Godot's `-s` switch
#      so the script works even when the consumer hasn't yet run
#      `godot --headless --import`.
#    * Propagates the child Godot exit code (which mirrors the verb's
#      PackagingResult.exit_code).
# ============================================================================

set -u

script_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]:-$0}")" >/dev/null 2>&1 && pwd)"
godot_exe=""
project_path="$PWD"
forward_args=()

# ── Argument scan: pull off --godot and --path; forward everything else. ──
while [ $# -gt 0 ]; do
    case "$1" in
        --godot)
            shift
            godot_exe="${1:-}"
            shift || true
            ;;
        --path)
            shift
            project_path="${1:-$project_path}"
            # --path is consumed here and passed to Godot as its own --path flag
            # below. It is intentionally NOT forwarded to run.gd's user args.
            shift || true
            ;;
        *)
            forward_args+=("$1")
            shift
            ;;
    esac
done

try_candidate() {
    local candidate="$1"
    if [ -n "$candidate" ] && [ -x "$candidate" ]; then
        godot_exe="$candidate"
        return 0
    fi
    if [ -n "$candidate" ] && [ -f "$candidate" ]; then
        godot_exe="$candidate"
        return 0
    fi
    return 1
}

if [ -z "$godot_exe" ]; then
    for env_name in GODOT_CONSOLE GODOT_BIN GODOT; do
        candidate="${!env_name:-}"
        if try_candidate "$candidate"; then break; fi
    done
fi

if [ -z "$godot_exe" ]; then
    for search_dir in "$script_dir/../../sample" "$PWD"; do
        if [ -d "$search_dir" ]; then
            # shellcheck disable=SC2012
            for candidate in "$search_dir"/Godot* "$search_dir"/godot*; do
                if try_candidate "$candidate"; then break 2; fi
            done
        fi
    done
fi

if [ -z "$godot_exe" ]; then
    for cmd_name in godot godot4; do
        resolved="$(command -v "$cmd_name" 2>/dev/null || true)"
        if try_candidate "$resolved"; then break; fi
    done
fi

if [ -z "$godot_exe" ]; then
    echo "[gdkpkg] error: could not find a Godot 4 executable." >&2
    echo "[gdkpkg] set GODOT_CONSOLE / GODOT_BIN / GODOT, or pass --godot <path>." >&2
    exit 3
fi

exec "$godot_exe" --headless --path "$project_path" \
    -s res://addons/godot_gdk_packaging/run.gd -- "${forward_args[@]}"
