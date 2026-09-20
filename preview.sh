#!/usr/bin/env bash
# Build and run the desktop preview without touching the tablet.
#
#   ./preview.sh
#   ./preview.sh --panel 954x1696 --shot /tmp/word-study-move.png
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
BUILD="$HERE/build-preview"

PREFIX=()
if command -v brew >/dev/null 2>&1 && brew --prefix qt >/dev/null 2>&1; then
    PREFIX=(-DCMAKE_PREFIX_PATH="$(brew --prefix qt)")
fi

cmake -S "$HERE" -B "$BUILD" -G Ninja -DCMAKE_BUILD_TYPE=Release "${PREFIX[@]}"
cmake --build "$BUILD"

exec "$BUILD/word_study" "$@"
