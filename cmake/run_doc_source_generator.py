"""Tiny dispatcher around godot-cpp's doc_source_generator.

Used by the godot_addon_doc_sources CMake helper so we don't have to pass a
multi-line python program through MSBuild's Command attribute (the VS 2026
generator quietly drops `python -c "..."` invocations on Windows). Instead
we invoke this file with positional args:

    python run_doc_source_generator.py <godot_cpp_dir> <output_cpp> <xml> [xml ...]
"""

from __future__ import annotations

import sys
from pathlib import Path


def main(argv: list[str]) -> int:
    if len(argv) < 4:
        print(
            "usage: run_doc_source_generator.py <godot_cpp_dir> <output_cpp> "
            "<xml> [xml ...]",
            file=sys.stderr,
        )
        return 2

    godot_cpp_dir = Path(argv[1])
    output_path = argv[2]
    xml_sources = argv[3:]

    sys.path.insert(0, str(godot_cpp_dir))
    from doc_source_generator import generate_doc_source  # type: ignore

    generate_doc_source(output_path, xml_sources)
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
