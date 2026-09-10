#!/usr/bin/env bash
# One-shot build script: locates Qt and builds AppIso with CMake.
# Just run "bash build.sh".
#
# Qt is NOT installed automatically. Install it yourself first (see
# README.md) -- either via your distro's package manager, or under
# ~/Qt/<version>/gcc_64 using the official Qt installer. If it's
# installed somewhere else, set the QT_DIR environment variable to your
# Qt kit folder, e.g.: QT_DIR=/opt/Qt/6.8.3/gcc_64 bash build.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/build"

echo "=== AppIso build ==="
echo

# --- Step 1: find Qt ----------------------------------------------------
QT_PREFIX=""
if [ -n "${QT_DIR:-}" ] && [ -x "$QT_DIR/bin/qmake" ]; then
    QT_PREFIX="$QT_DIR"
else
    for d in "$HOME"/Qt/*/gcc_64; do
        if [ -x "$d/bin/qmake" ]; then
            QT_PREFIX="$d"
        fi
    done
fi

if [ -n "$QT_PREFIX" ]; then
    echo "Found Qt at $QT_PREFIX."
    PREFIX_ARGS=(-DCMAKE_PREFIX_PATH="$QT_PREFIX")
else
    echo "Qt was not found under \$QT_DIR or ~/Qt -- will look for a system installation instead."
    PREFIX_ARGS=()
fi
echo

# --- Step 2: sanity-check the rest of the toolchain --------------------
if ! command -v g++ >/dev/null 2>&1 && ! command -v clang++ >/dev/null 2>&1; then
    echo "ERROR: no C++ compiler found." >&2
    echo "Install one first, e.g.: sudo apt install build-essential" >&2
    exit 1
fi

if [ ! -f /usr/include/GL/glew.h ] && ! pkg-config --exists glew 2>/dev/null; then
    echo "ERROR: GLEW development files were not found." >&2
    echo "Install them first, e.g.:" >&2
    echo "  Debian/Ubuntu : sudo apt install libglew-dev" >&2
    echo "  Fedora        : sudo dnf install glew-devel" >&2
    echo "  Arch          : sudo pacman -S glew" >&2
    exit 1
fi

# --- Step 3: configure and build ---------------------------------------
GENERATOR="Unix Makefiles"
if command -v ninja >/dev/null 2>&1; then
    GENERATOR="Ninja"
fi
echo "Using CMake generator: $GENERATOR"

echo "Configuring..."
if ! cmake -S "$SCRIPT_DIR" -B "$BUILD_DIR" -G "$GENERATOR" "${PREFIX_ARGS[@]}" -DCMAKE_BUILD_TYPE=Release; then
    echo
    echo "ERROR: CMake configuration failed." >&2
    if [ -z "$QT_PREFIX" ]; then
        echo "This is likely because Qt 6 (modules: Core, Gui, Widgets, OpenGL," >&2
        echo "OpenGLWidgets, Svg) could not be found. Install it via your package" >&2
        echo "manager, or with the official Qt installer, then either let this" >&2
        echo "script find it under ~/Qt/, or set QT_DIR to your Qt kit folder." >&2
    fi
    exit 1
fi

echo
echo "Building..."
cmake --build "$BUILD_DIR" --parallel

echo
echo "================================================"
echo " Build succeeded!"
echo " Executable: $BUILD_DIR/AppIso"
echo "================================================"
